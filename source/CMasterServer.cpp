/*
* CMasterServer.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "CMasterServer.h"
#include "CDnsResolver.h"
#include "CServerList.h"
#include "CServerQuery.h"
#include "globals.h"
#include "main.h"
#include "masterupdate.h"

#include <mmsystem.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

static bool MasterUpdateInProgress = false;
static TThread* MasterUpdateThread = nullptr;
static volatile LONG MasterUpdateCancel = 0;
static bool MasterQueryBatchActive = false;
static int MasterQueryPending = 0;
static DWORD MasterQueryDeadline = 0;
static bool MasterRefreshPending = false;
static TfmMasterUpdate* MasterUpdateProgressForm = nullptr;
static const DWORD MASTER_QUERY_RESPONSE_TIMEOUT_MS = 3000;

static bool MasterUpdateThreadFinished()
{
    return MasterUpdateThread == nullptr || WaitForSingleObject((HANDLE)MasterUpdateThread->Handle, 0) == WAIT_OBJECT_0;
}

bool CMasterServer::Download(TStringList* ServerList, String& ErrorMessage)
{
    const int MAX_RESPONSE_SIZE = 16 * 1024 * 1024;
    const int MAX_MASTER_SERVERS = 100000;

    HINTERNET internet =
        InternetOpenW(L"Mozilla/5.0 (compatible; SA:MP v0.3.7)", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!internet)
    {
        ErrorMessage = "Unable to initialize the master-list connection.";
        return false;
    }

    DWORD timeout = 10000;
    InternetSetOptionW(internet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionW(internet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionW(internet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

    HINTERNET url = InternetOpenUrlW(internet, L"https://api.open.mp/servers", nullptr, 0,
                                     INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!url)
    {
        InternetCloseHandle(internet);
        ErrorMessage = "Unable to download the master server list.";
        return false;
    }

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    if (!HttpQueryInfoW(url, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, nullptr) ||
        statusCode < 200 || statusCode >= 300)
    {
        InternetCloseHandle(url);
        InternetCloseHandle(internet);
        ErrorMessage = "The master server returned an invalid HTTP response.";
        return false;
    }

    char buffer[4096];
    AnsiString raw;
    bool readSucceeded = true;
    while (true)
    {
        DWORD bytesRead = 0;
        if (!InternetReadFile(url, buffer, sizeof(buffer), &bytesRead))
        {
            readSucceeded = false;
            break;
        }
        if (bytesRead == 0)
            break;
        if (raw.Length() > MAX_RESPONSE_SIZE - (int)bytesRead)
        {
            readSucceeded = false;
            ErrorMessage = "The master server response is too large.";
            break;
        }
        raw += AnsiString(buffer, bytesRead);
    }

    InternetCloseHandle(url);
    InternetCloseHandle(internet);

    if (!readSucceeded || raw.IsEmpty())
    {
        if (ErrorMessage.IsEmpty())
            ErrorMessage = "Unable to read the master server response.";
        return false;
    }

    TJSONValue* jsonValue = TJSONObject::ParseJSONValue(String(raw));
    TJSONArray* jsonArray = dynamic_cast<TJSONArray*>(jsonValue);
    if (!jsonArray)
    {
        delete jsonValue;
        ErrorMessage = "The master server returned invalid JSON.";
        return false;
    }

    bool valid = true;
    try
    {
        if (jsonArray->Count > MAX_MASTER_SERVERS)
        {
            ErrorMessage = "The master server returned too many entries.";
            valid = false;
        }
        else
        {
            for (int i = 0; i < jsonArray->Count; i++)
            {
                TJSONObject* server = dynamic_cast<TJSONObject*>(jsonArray->Items[i]);
                if (!server)
                    continue;

                TJSONPair* ipPair = server->Get("ip");
                if (!ipPair || !ipPair->JsonValue)
                    continue;

                String fullAddress = ipPair->JsonValue->Value().Trim();
                int colon = fullAddress.Pos(":");
                if (colon > 1 && fullAddress.Length() <= 300 && ServerList->IndexOf(fullAddress) == -1)
                    ServerList->Add(fullAddress);
            }
        }
    }
    __finally
    {
        delete jsonValue;
    }

    return valid;
}

void CMasterServer::Refresh(TfmMain* Form)
{
    if (!Form || MasterUpdateInProgress)
        return;

    if (MasterUpdateThread)
    {
        if (!MasterUpdateThreadFinished())
        {
            MasterRefreshPending = true;
            return;
        }
        MasterUpdateThread->WaitFor();
        delete MasterUpdateThread;
        MasterUpdateThread = nullptr;
    }

    MasterUpdateInProgress = true;
    MasterRefreshPending = false;
    InterlockedExchange(&MasterUpdateCancel, 0);
    MasterUpdateProgressForm = new TfmMasterUpdate(Application);
    MasterUpdateProgressForm->Show();

    try
    {
        MasterUpdateThread = TThread::CreateAnonymousThread([Form]() {
            TStringList* serverList = nullptr;
            String errorMessage;
            bool success = false;
            try
            {
                serverList = new TStringList();
                success = CMasterServer::Download(serverList, errorMessage);
            }
            catch (Exception& exception)
            {
                errorMessage = exception.Message;
            }
            catch (...)
            {
                errorMessage = "Unexpected error while downloading the master server list.";
            }

            if (InterlockedCompareExchange(&MasterUpdateCancel, 0, 0) != 0)
            {
                delete serverList;
                return;
            }

            TThread::Synchronize(MasterUpdateThread, [Form, serverList, errorMessage, success]() {
                bool keepProgressOpen = false;
                try
                {
                    if (InterlockedCompareExchange(&MasterUpdateCancel, 0, 0) == 0)
                    {
                        if (success)
                        {
                            CMasterServer::Apply(Form, serverList);
                            keepProgressOpen = CMasterServer::IsQueryBatchActive();
                        }
                        else
                        {
                            ShowMessage(errorMessage);
                        }
                    }
                }
                __finally
                {
                    delete serverList;
                    if (!keepProgressOpen)
                    {
                        CMasterServer::CloseProgress();
                        MasterUpdateInProgress = false;
                    }
                }
            });
        });
        MasterUpdateThread->FreeOnTerminate = false;
        MasterUpdateThread->Start();
    }
    catch (...)
    {
        if (MasterUpdateThread)
        {
            delete MasterUpdateThread;
            MasterUpdateThread = nullptr;
        }
        CloseProgress();
        MasterUpdateInProgress = false;
        MessageDlg("Unable to start the master-list update.", mtError, TMsgDlgButtons() << mbOK, 0);
    }
}

void CMasterServer::Apply(TfmMain* Form, TStringList* ServerList)
{
    if (!Form || !ServerList)
        return;

    MasterQueryBatchActive = false;
    MasterQueryPending = 0;
    MasterQueryDeadline = 0;
    CServerQuery::ClearNetwork();
    Form->lbServers->Clear();
    Form->lbPlayers->Clear();
    Form->lbRules->Clear();
    Servers.clear();
    CDnsResolver::Clear();
    Form->tmrQueryQueueProcess->Enabled = false;
    Form->tmrServerListUpdate->Enabled = false;

    for (int i = 0; i < ServerList->Count; i++)
    {
        String fullAddress = ServerList->Strings[i].Trim();
        int colon = fullAddress.Pos(":");
        if (colon <= 1)
            continue;

        String address = fullAddress.SubString(1, colon - 1).Trim();
        String portText = fullAddress.SubString(colon + 1, fullAddress.Length()).Trim();
        int port = StrToIntDef(portText, -1);
        if (address.IsEmpty() || port < 1 || port > 65535)
            continue;

        TServerInfo server = {};
        server.HostName = AnsiString("(Retrieving info...) " + fullAddress);
        server.Address = AnsiString(address);
        server.Port = port;
        server.Ping = 9999;
        server.Tag = (WORD)random(0xFFFF);
        Servers.push_back(server);
        CServerQuery::Enqueue(address + ":" + IntToStr(port) + "#" + IntToStr(server.Tag));
    }

    MasterQueryPending = (int)Servers.size();
    CServerList::RebuildLookup();
    MasterQueryBatchActive = MasterQueryPending > 0 && CServerQuery::IsReady();
    Form->tmrQueryQueueProcess->Enabled = CServerQuery::IsReady();
    Form->tmrServerListUpdate->Enabled = true;

    CServerList::MarkOrderDirty();
    if (!MasterQueryBatchActive)
        Form->UpdateServers();
}

void CMasterServer::Tick(TfmMain* Form)
{
    if (!Form)
        return;

    if (MasterUpdateThread && MasterUpdateThreadFinished())
    {
        MasterUpdateThread->WaitFor();
        delete MasterUpdateThread;
        MasterUpdateThread = nullptr;
    }

    if (MasterRefreshPending && !MasterUpdateInProgress && MasterUpdateThread == nullptr && MasterFile != 0)
    {
        MasterRefreshPending = false;
        Refresh(Form);
        return;
    }

    if (MasterQueryBatchActive)
    {
        CheckQueryBatchComplete(Form);
        return;
    }

    if (CServerList::ConsumeDirty())
        Form->UpdateServers();
}

void CMasterServer::CheckQueryBatchComplete(TfmMain* Form)
{
    if (!Form || !MasterQueryBatchActive)
        return;

    if (CServerQuery::PendingCount() > 0)
    {
        MasterQueryDeadline = 0;
        return;
    }

    if (MasterQueryPending <= 0)
    {
        FinishQueryBatch(Form);
        return;
    }

    DWORD now = timeGetTime();
    if (MasterQueryDeadline == 0)
        MasterQueryDeadline = now + MASTER_QUERY_RESPONSE_TIMEOUT_MS;
    else if ((LONG)(now - MasterQueryDeadline) >= 0)
        FinishQueryBatch(Form);
}

void CMasterServer::OnServerQueryProgress(TfmMain* Form, int ServerIndex)
{
    if (!Form || !MasterQueryBatchActive || ServerIndex < 0 || ServerIndex >= (int)Servers.size())
        return;

    if (Servers[ServerIndex].QueryInfoReceived && Servers[ServerIndex].QueryPingReceived &&
        !Servers[ServerIndex].QueryCompleted)
    {
        Servers[ServerIndex].QueryCompleted = true;
        if (MasterQueryPending > 0)
            MasterQueryPending--;
    }

    CheckQueryBatchComplete(Form);
}

void CMasterServer::FinishQueryBatch(TfmMain* Form)
{
    if (!Form || !MasterQueryBatchActive)
        return;

    MasterQueryBatchActive = false;
    MasterQueryPending = 0;
    MasterQueryDeadline = 0;

    Form->UpdateServers();
    if (Form->lbServers->Items->Count > 0)
        Form->lbServers->ItemIndex = 0;
    Form->lbServersClick(Form);

    CloseProgress();
    MasterUpdateInProgress = false;
}

void CMasterServer::CloseProgress()
{
    TfmMasterUpdate* progress = MasterUpdateProgressForm;
    MasterUpdateProgressForm = nullptr;
    if (progress)
    {
        progress->Close();
        delete progress;
    }
}

void CMasterServer::Cancel(TfmMain* Form)
{
    InterlockedExchange(&MasterUpdateCancel, 1);
    CServerQuery::ClearNetwork();
    MasterQueryBatchActive = false;
    MasterQueryPending = 0;
    MasterQueryDeadline = 0;
    CloseProgress();
    MasterUpdateInProgress = false;
}

void CMasterServer::Shutdown(TfmMain* Form)
{
    MasterRefreshPending = false;
    Cancel(Form);
    if (MasterUpdateThread)
    {
        MasterUpdateThread->Terminate();
        MasterUpdateThread->WaitFor();
        delete MasterUpdateThread;
        MasterUpdateThread = nullptr;
    }
}

bool CMasterServer::IsQueryBatchActive()
{
    return MasterQueryBatchActive;
}
