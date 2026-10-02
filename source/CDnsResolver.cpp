/*
* CDnsResolver.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "CDnsResolver.h"
#include "CGameLauncher.h"
#include "CServerList.h"
#include "CServerQuery.h"
#include "globals.h"
#include "main.h"

#include <map>
#include <string>
#include <vector>

#include <winsock2.h>

struct TDnsQuerySpec
{
    String Server;
    String Port;
    String Password;
    bool Ping;
    bool Info;
    bool Players;
    bool Rules;
    bool Connect;
};

struct TDnsRequest
{
    HANDLE Task;
    AnsiString Host;
    char Buffer[MAXGETHOSTSTRUCT];
    std::vector<TDnsQuerySpec> Queries;
};

static std::vector<TDnsRequest*> DnsRequests;
static std::map<std::string, String> DnsCache;

static std::string LowerAnsiKey(const String& Value)
{
    AnsiString lower = AnsiLowerCase(AnsiString(Value));
    return std::string(lower.c_str(), lower.Length());
}

static bool QueueRequest(HWND Window, const AnsiString& Host, const TDnsQuerySpec& Query)
{
    for (int i = 0; i < (int)DnsRequests.size(); i++)
    {
        if (AnsiCompareText(DnsRequests[i]->Host, Host) == 0)
        {
            DnsRequests[i]->Queries.push_back(Query);
            return true;
        }
    }

    TDnsRequest* request = new TDnsRequest();
    request->Task = nullptr;
    request->Host = Host;
    ZeroMemory(request->Buffer, sizeof(request->Buffer));
    request->Queries.push_back(Query);
    request->Task = WSAAsyncGetHostByName(Window, WM_DNS_RECV, Host.c_str(), request->Buffer, sizeof(request->Buffer));
    if (!request->Task)
    {
        delete request;
        return false;
    }

    DnsRequests.push_back(request);
    return true;
}

String CDnsResolver::GetCached(const String& HostName)
{
    AnsiString ansiHost = HostName;
    unsigned long numericAddress = inet_addr(ansiHost.c_str());
    if (numericAddress != INADDR_NONE || HostName == "255.255.255.255")
        return HostName;

    std::map<std::string, String>::const_iterator cached = DnsCache.find(LowerAnsiKey(HostName));
    if (cached == DnsCache.end())
        return "";
    return cached->second == "1" ? String() : cached->second;
}

bool CDnsResolver::HasCached(const String& HostName)
{
    AnsiString ansiHost = HostName;
    unsigned long numericAddress = inet_addr(ansiHost.c_str());
    if (numericAddress != INADDR_NONE || HostName == "255.255.255.255")
        return true;
    return DnsCache.find(LowerAnsiKey(HostName)) != DnsCache.end();
}

void CDnsResolver::Cache(const String& HostName, const String& Address)
{
    DnsCache[LowerAnsiKey(HostName)] = Address.IsEmpty() ? String("1") : Address;
}

void CDnsResolver::Clear()
{
    DnsCache.clear();
}

bool CDnsResolver::QueueServerQuery(HWND Window, const AnsiString& Host, const String& Server, bool Ping, bool Info,
                                    bool Players, bool Rules)
{
    TDnsQuerySpec query = {};
    query.Server = Server;
    query.Ping = Ping;
    query.Info = Info;
    query.Players = Players;
    query.Rules = Rules;
    return QueueRequest(Window, Host, query);
}

bool CDnsResolver::QueueConnect(HWND Window, const AnsiString& Host, const String& Server, const String& Port,
                                const String& Password)
{
    TDnsQuerySpec query = {};
    query.Server = Server;
    query.Port = Port;
    query.Password = Password;
    query.Connect = true;
    return QueueRequest(Window, Host, query);
}

void CDnsResolver::HandleMessage(TfmMain* Form, TMessage& Message)
{
    HANDLE task = reinterpret_cast<HANDLE>(Message.WParam);
    int requestIndex = -1;
    for (int i = 0; i < (int)DnsRequests.size(); i++)
    {
        if (DnsRequests[i]->Task == task)
        {
            requestIndex = i;
            break;
        }
    }
    if (requestIndex < 0)
        return;

    TDnsRequest* request = DnsRequests[requestIndex];
    DnsRequests.erase(DnsRequests.begin() + requestIndex);

    String result;
    if (WSAGETASYNCERROR(Message.LParam) == 0)
    {
        hostent* entry = reinterpret_cast<hostent*>(request->Buffer);
        if (entry && entry->h_addr_list && entry->h_addr_list[0])
        {
            in_addr address = {};
            memcpy(&address, entry->h_addr_list[0], sizeof(address));
            result = AnsiString(inet_ntoa(address));
        }
    }

    Cache(String(request->Host), result);
    CServerList::RebuildLookup();

    std::vector<TDnsQuerySpec> queries;
    queries.swap(request->Queries);
    delete request;

    if (!result.IsEmpty())
    {
        for (int i = 0; i < (int)queries.size(); i++)
        {
            if (queries[i].Connect)
                CGameLauncher::ResumeDnsConnect(Form, queries[i].Server, queries[i].Port, queries[i].Password);
            else
                CServerQuery::Query(Form, queries[i].Server, queries[i].Ping, queries[i].Info, queries[i].Players,
                                    queries[i].Rules);
        }
        return;
    }

    for (int i = 0; i < (int)queries.size(); i++)
    {
        if (queries[i].Connect)
        {
            CGameLauncher::CancelDnsConnect();
            MessageDlg("Unable to resolve the server address.", mtError, TMsgDlgButtons() << mbOK, 0);
            break;
        }
    }
}

void CDnsResolver::Shutdown()
{
    for (int i = 0; i < (int)DnsRequests.size(); i++)
    {
        if (DnsRequests[i]->Task)
            WSACancelAsyncRequest(DnsRequests[i]->Task);
        delete DnsRequests[i];
    }
    DnsRequests.clear();
    DnsCache.clear();
}
