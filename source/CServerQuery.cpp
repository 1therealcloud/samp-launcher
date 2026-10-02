/*
* CServerQuery.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "CServerQuery.h"
#include "CDnsResolver.h"
#include "CMasterServer.h"
#include "CServerList.h"
#include "globals.h"
#include "main.h"

#include <deque>
#include <map>
#include <vector>

#include <mmsystem.h>
#include <winsock2.h>

struct TOutgoingQueryPacket
{
    sockaddr_in Address;
    int Length;
    LONG Generation;
    char Data[15];
};

struct TReceivedQueryPacket
{
    DWORD Address;
    WORD Port;
    int Length;
    char Data[2048];
};

struct TPingRequestTime
{
    DWORD Address;
    WORD Port;
    LONGLONG Counter;
};

class TQueryNetworkThread : public TThread
{
private:
    SOCKET FSocket;
    HWND FNotifyWindow;
    CRITICAL_SECTION FLock;
    HANDLE FWakeEvent;
    WSAEVENT FReceiveEvent;
    std::deque<TOutgoingQueryPacket> FOutgoing;
    std::vector<TReceivedQueryPacket> FReceived;
    std::map<DWORD, TPingRequestTime> FPingRequests;
    bool FResultsPosted;
    volatile LONG FGeneration;
    DWORD FNextPingToken;
    LONGLONG FPerformanceFrequency;

protected:
    void __fastcall Execute() override;

public:
    __fastcall TQueryNetworkThread(SOCKET Socket, HWND NotifyWindow)
        : TThread(true), FSocket(Socket), FNotifyWindow(NotifyWindow), FWakeEvent(nullptr),
          FReceiveEvent(WSA_INVALID_EVENT), FResultsPosted(false), FGeneration(0), FNextPingToken(0),
          FPerformanceFrequency(0)
    {
        FreeOnTerminate = false;
        InitializeCriticalSection(&FLock);
        LARGE_INTEGER frequency = {};
        if (QueryPerformanceFrequency(&frequency))
            FPerformanceFrequency = frequency.QuadPart;
        FWakeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        FReceiveEvent = WSACreateEvent();
        if (FReceiveEvent != WSA_INVALID_EVENT)
            WSAEventSelect(FSocket, FReceiveEvent, FD_READ);
    }

    __fastcall ~TQueryNetworkThread()
    {
        if (FWakeEvent)
            CloseHandle(FWakeEvent);
        if (FReceiveEvent != WSA_INVALID_EVENT)
        {
            WSAEventSelect(FSocket, nullptr, 0);
            WSACloseEvent(FReceiveEvent);
        }
        DeleteCriticalSection(&FLock);
    }

    void Queue(const sockaddr_in& Address, const char* Data, int Length);
    void TakeReceived(std::vector<TReceivedQueryPacket>& Packets);
    bool TakePingMilliseconds(DWORD Token, DWORD Address, WORD Port, int& Milliseconds);
    void Clear();
    void Wake()
    {
        if (FWakeEvent)
            SetEvent(FWakeEvent);
    }
};

static SOCKET QuerySocket = INVALID_SOCKET;
static bool WinsockStarted = false;
static TQueryNetworkThread* QueryNetworkThread = nullptr;
static TStringList* QueryQueue = nullptr;
static int QueryQueuePosition = 0;

void TQueryNetworkThread::Queue(const sockaddr_in& Address, const char* Data, int Length)
{
    if (!Data || Length < 1 || Length > 15)
        return;

    TOutgoingQueryPacket packet = {};
    packet.Address = Address;
    packet.Length = Length;
    packet.Generation = InterlockedCompareExchange(&FGeneration, 0, 0);
    memcpy(packet.Data, Data, Length);

    EnterCriticalSection(&FLock);
    if (Length == 15 && packet.Data[10] == 'p')
    {
        do
        {
            ++FNextPingToken;
        }
        while (FNextPingToken == 0 || FPingRequests.find(FNextPingToken) != FPingRequests.end());
        memcpy(&packet.Data[11], &FNextPingToken, sizeof(FNextPingToken));
    }
    FOutgoing.push_back(packet);
    LeaveCriticalSection(&FLock);
    Wake();
}

bool TQueryNetworkThread::TakePingMilliseconds(DWORD Token, DWORD Address, WORD Port, int& Milliseconds)
{
    LARGE_INTEGER now = {};
    if (FPerformanceFrequency <= 0 || !QueryPerformanceCounter(&now))
        return false;

    EnterCriticalSection(&FLock);
    std::map<DWORD, TPingRequestTime>::iterator found = FPingRequests.find(Token);
    if (found == FPingRequests.end() || found->second.Address != Address || found->second.Port != Port)
    {
        LeaveCriticalSection(&FLock);
        return false;
    }

    LONGLONG started = found->second.Counter;
    FPingRequests.erase(found);
    LeaveCriticalSection(&FLock);

    LONGLONG elapsed = now.QuadPart - started;
    if (elapsed < 0)
        return false;

    LONGLONG rounded = (elapsed * 1000 + FPerformanceFrequency / 2) / FPerformanceFrequency;
    if (rounded < 1)
        rounded = 1;
    if (rounded > 60000)
        return false;

    Milliseconds = (int)rounded;
    return true;
}

void TQueryNetworkThread::TakeReceived(std::vector<TReceivedQueryPacket>& Packets)
{
    EnterCriticalSection(&FLock);
    Packets.swap(FReceived);
    FResultsPosted = false;
    LeaveCriticalSection(&FLock);
}

void TQueryNetworkThread::Clear()
{
    EnterCriticalSection(&FLock);
    FOutgoing.clear();
    FReceived.clear();
    FPingRequests.clear();
    FResultsPosted = false;
    InterlockedIncrement(&FGeneration);
    LeaveCriticalSection(&FLock);
}

void __fastcall TQueryNetworkThread::Execute()
{
    while (!Terminated)
    {
        std::deque<TOutgoingQueryPacket> outgoing;
        EnterCriticalSection(&FLock);
        outgoing.swap(FOutgoing);
        LeaveCriticalSection(&FLock);

        while (!outgoing.empty() && !Terminated)
        {
            const TOutgoingQueryPacket& packet = outgoing.front();
            if (packet.Generation == InterlockedCompareExchange(&FGeneration, 0, 0))
            {
                DWORD pingToken = 0;
                bool isPing = packet.Length == 15 && packet.Data[10] == 'p';
                if (isPing && FPerformanceFrequency > 0)
                {
                    LARGE_INTEGER started = {};
                    if (QueryPerformanceCounter(&started))
                    {
                        memcpy(&pingToken, &packet.Data[11], sizeof(pingToken));
                        TPingRequestTime request = {};
                        request.Address = packet.Address.sin_addr.s_addr;
                        request.Port = ntohs(packet.Address.sin_port);
                        request.Counter = started.QuadPart;
                        EnterCriticalSection(&FLock);
                        if (FPingRequests.size() >= 256)
                            FPingRequests.erase(FPingRequests.begin());
                        FPingRequests[pingToken] = request;
                        LeaveCriticalSection(&FLock);
                    }
                }

                int sent = sendto(FSocket, packet.Data, packet.Length, 0,
                                  reinterpret_cast<const sockaddr*>(&packet.Address), sizeof(packet.Address));
                if (sent == SOCKET_ERROR && pingToken != 0)
                {
                    EnterCriticalSection(&FLock);
                    FPingRequests.erase(pingToken);
                    LeaveCriticalSection(&FLock);
                }
            }
            outgoing.pop_front();
        }

        std::vector<TReceivedQueryPacket> received;
        for (;;)
        {
            TReceivedQueryPacket packet = {};
            sockaddr_in from = {};
            int fromLength = sizeof(from);
            int length =
                recvfrom(FSocket, packet.Data, sizeof(packet.Data), 0, reinterpret_cast<sockaddr*>(&from), &fromLength);
            if (length == SOCKET_ERROR)
            {
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK)
                    break;
                break;
            }
            if (length < 11 || memcmp(packet.Data, "SAMP", 4) != 0)
                continue;

            DWORD packetAddress = 0;
            memcpy(&packetAddress, &packet.Data[4], sizeof(packetAddress));
            if (packetAddress != from.sin_addr.s_addr)
                continue;

            packet.Address = from.sin_addr.s_addr;
            packet.Port = ntohs(from.sin_port);
            packet.Length = length;
            received.push_back(packet);
        }

        bool postResults = false;
        if (!received.empty())
        {
            EnterCriticalSection(&FLock);
            FReceived.insert(FReceived.end(), received.begin(), received.end());
            if (!FResultsPosted)
            {
                FResultsPosted = true;
                postResults = true;
            }
            LeaveCriticalSection(&FLock);
        }

        if (postResults)
            PostMessage(FNotifyWindow, WM_QUERY_BATCH_READY, 0, 0);

        if (!Terminated)
        {
            HANDLE events[2] = {FWakeEvent, FReceiveEvent};
            DWORD count = FReceiveEvent == WSA_INVALID_EVENT ? 1 : 2;
            DWORD waitResult = WaitForMultipleObjects(count, events, FALSE, 50);
            if (count == 2 && waitResult == WAIT_OBJECT_0 + 1)
            {
                WSANETWORKEVENTS networkEvents = {};
                WSAEnumNetworkEvents(FSocket, FReceiveEvent, &networkEvents);
            }
        }
    }
}

static String GetToken(const String& TokenData, int ItemIndex, const String& TokenDelimiter)
{
    int tokenCount = 0;
    int i = 1;
    String result;
    int len = TokenData.Length();
    int delimLen = TokenDelimiter.Length();

    while (i <= len)
    {
        if (tokenCount == ItemIndex - 1)
        {
            if (TokenData.SubString(i, delimLen) == TokenDelimiter)
                break;
            result += TokenData[i];
        }
        if (TokenData.SubString(i, delimLen) == TokenDelimiter)
        {
            tokenCount++;
            i += delimLen - 1;
        }
        i++;
    }
    return result;
}

static void DispatchQueryPacket(const sockaddr_in& Address, const char* Data, int Length)
{
    if (QueryNetworkThread)
        QueryNetworkThread->Queue(Address, Data, Length);
    else if (QuerySocket != INVALID_SOCKET)
        sendto(QuerySocket, Data, Length, 0, reinterpret_cast<const sockaddr*>(&Address), sizeof(Address));
}

bool CServerQuery::Initialize(HWND NotifyWindow)
{
    if (!QueryQueue)
        QueryQueue = new TStringList();
    QueryQueuePosition = 0;

    WSADATA data = {};
    WinsockStarted = WSAStartup(0x0202, &data) == 0;
    if (!WinsockStarted)
        return false;

    QuerySocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (QuerySocket != INVALID_SOCKET)
    {
        int receiveBufferSize = 1024 * 1024;
        setsockopt(QuerySocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&receiveBufferSize),
                   sizeof(receiveBufferSize));
    }

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = 0;
    u_long nonBlocking = 1;
    if (QuerySocket == INVALID_SOCKET ||
        bind(QuerySocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR ||
        ioctlsocket(QuerySocket, FIONBIO, &nonBlocking) == SOCKET_ERROR)
    {
        if (QuerySocket != INVALID_SOCKET)
            closesocket(QuerySocket);
        QuerySocket = INVALID_SOCKET;
        return false;
    }

    try
    {
        QueryNetworkThread = new TQueryNetworkThread(QuerySocket, NotifyWindow);
        QueryNetworkThread->Start();
    }
    catch (...)
    {
        delete QueryNetworkThread;
        QueryNetworkThread = nullptr;
        closesocket(QuerySocket);
        QuerySocket = INVALID_SOCKET;
        return false;
    }

    return true;
}

void CServerQuery::Shutdown()
{
    if (QueryNetworkThread)
    {
        QueryNetworkThread->Terminate();
        QueryNetworkThread->Wake();
        QueryNetworkThread->WaitFor();
        delete QueryNetworkThread;
        QueryNetworkThread = nullptr;
    }

    if (QuerySocket != INVALID_SOCKET)
    {
        shutdown(QuerySocket, SD_BOTH);
        closesocket(QuerySocket);
        QuerySocket = INVALID_SOCKET;
    }

    if (WinsockStarted)
    {
        WSACleanup();
        WinsockStarted = false;
    }

    delete QueryQueue;
    QueryQueue = nullptr;
    QueryQueuePosition = 0;
}

bool CServerQuery::IsReady()
{
    return QuerySocket != INVALID_SOCKET;
}

void CServerQuery::Query(TfmMain* Form, const String& Server, bool Ping, bool Info, bool Players, bool Rules)
{
    if (!Form || QuerySocket == INVALID_SOCKET)
        return;

    AnsiString host;
    WORD port = 7777;
    WORD tag = 0;
    int colonPos = Server.Pos(":");
    int tagPos = Server.Pos("#");

    if (colonPos > 0)
    {
        if (tagPos > 0)
        {
            host = AnsiString(Server.SubString(1, colonPos - 1));
            port = (WORD)StrToIntDef(Server.SubString(colonPos + 1, tagPos - colonPos - 1), 7777);
            tag = (WORD)StrToIntDef(Server.SubString(tagPos + 1, Server.Length() - tagPos), 0);
        }
        else
        {
            host = AnsiString(Server.SubString(1, colonPos - 1));
            port = (WORD)StrToIntDef(Server.SubString(colonPos + 1, 5), 7777);
        }
    }
    else
    {
        host = AnsiString(Server);
    }

    if (tag == 0)
        tag = port;

    String hostName = String(host);
    AnsiString ip = AnsiString(CDnsResolver::GetCached(hostName));
    if (ip.IsEmpty())
    {
        if (CDnsResolver::HasCached(hostName))
            return;
        if (!CDnsResolver::QueueServerQuery(Form->Handle, host, Server, Ping, Info, Players, Rules))
            CDnsResolver::Cache(hostName, "");
        return;
    }

    if (ip.Length() < 7 || ip.Length() > 15)
        return;

    sockaddr_in toAddress = {};
    toAddress.sin_family = AF_INET;
    toAddress.sin_port = htons(port);
    toAddress.sin_addr.s_addr = inet_addr(ip.c_str());

    BYTE buffer[15] = {};
    buffer[0] = 'S';
    buffer[1] = 'A';
    buffer[2] = 'M';
    buffer[3] = 'P';
    buffer[4] = (BYTE)StrToIntDef(AnsiString(GetToken(String(ip), 1, ".")), 0);
    buffer[5] = (BYTE)StrToIntDef(AnsiString(GetToken(String(ip), 2, ".")), 0);
    buffer[6] = (BYTE)StrToIntDef(AnsiString(GetToken(String(ip), 3, ".")), 0);
    buffer[7] = (BYTE)StrToIntDef(AnsiString(GetToken(String(ip), 4, ".")), 0);
    memcpy(&buffer[8], &tag, 2);

    if (Info)
    {
        buffer[10] = 'i';
        DispatchQueryPacket(toAddress, reinterpret_cast<char*>(buffer), 11);
    }
    if (Ping)
    {
        buffer[10] = 'p';
        DWORD ticks = timeGetTime();
        memcpy(&buffer[11], &ticks, 4);
        DispatchQueryPacket(toAddress, reinterpret_cast<char*>(buffer), 15);
    }
    if (Players)
    {
        buffer[10] = 'c';
        DispatchQueryPacket(toAddress, reinterpret_cast<char*>(buffer), 11);
    }
    if (Rules)
    {
        buffer[10] = 'r';
        DispatchQueryPacket(toAddress, reinterpret_cast<char*>(buffer), 11);
    }
}

void CServerQuery::Parse(TfmMain* Form, const String& SourceIP, WORD SourcePort, char* Buffer, int DataLength)
{
    if (!Form || DataLength < 11 || memcmp(Buffer, "SAMP", 4) != 0)
        return;

    AnsiString packetIP;
    packetIP.sprintf("%d.%d.%d.%d", (BYTE)Buffer[4], (BYTE)Buffer[5], (BYTE)Buffer[6], (BYTE)Buffer[7]);

    WORD packetPort = 0;
    memcpy(&packetPort, &Buffer[8], 2);
    if (AnsiString(SourceIP) != packetIP)
        return;

    int index = CServerList::FindByPacket(AnsiString(SourceIP), SourcePort, packetPort);
    if (index < 0 || index >= (int)Servers.size())
        return;

    bool repaintServers = false;
    bool repaintPlayers = false;
    bool repaintRules = false;
    bool rebuildServers = false;

    switch (Buffer[10])
    {
        case 'p':
            if (DataLength == 15)
            {
                DWORD pingToken = 0;
                memcpy(&pingToken, &Buffer[11], 4);
                int measuredPing = 0;
                if (QueryNetworkThread)
                {
                    DWORD sourceAddress = inet_addr(AnsiString(SourceIP).c_str());
                    if (!QueryNetworkThread->TakePingMilliseconds(pingToken, sourceAddress, SourcePort, measuredPing))
                        break;
                }
                else
                {
                    measuredPing = (int)(timeGetTime() - pingToken);
                    if (measuredPing < 1)
                        measuredPing = 1;
                }

                Servers[index].Ping = measuredPing;
                int selectedIndex = -1;
                if (Form->lbServers->ItemIndex >= 0 && Form->lbServers->ItemIndex < Form->lbServers->Items->Count)
                    selectedIndex = StrToIntDef(Form->lbServers->Items->Strings[Form->lbServers->ItemIndex], -1);

                if (selectedIndex == index)
                {
                    double value = (double)Servers[index].Ping;
                    int point = Form->chSIPingChart->Series[0]->AddY(value, "", clBlue);
                    if (point > 60)
                    {
                        for (int j = 1; j <= 61; j++)
                            Form->chSIPingChart->Series[0]->YValue[j - 1] = Form->chSIPingChart->Series[0]->YValue[j];
                        Form->chSIPingChart->Series[0]->Delete(61);
                    }

                    int peak = (int)(Form->chSIPingChart->Series[0]->MaxYValue() + 0.999);
                    int target = peak * 5 / 4;
                    if (target < 50)
                        target = 50;
                    int step = target <= 100 ? 10 : target <= 250 ? 25 : target <= 500 ? 50 : 100;
                    Form->chSIPingChart->LeftAxis->Maximum = ((target + step - 1) / step) * step;
                }

                repaintServers = true;
                rebuildServers = SortMode == TSortMode::smPing;
                Servers[index].QueryPingReceived = true;
                if (selectedIndex == index)
                    Form->lbSIPing->Caption = IntToStr(Servers[index].Ping);
            }
            break;

        case 'i':
        {
            int bufferPos = 11;
            BYTE passworded = 0;
            WORD players = 0;
            WORD maxPlayers = 0;
            if (bufferPos + 1 > DataLength)
                break;
            memcpy(&passworded, &Buffer[bufferPos], 1);
            bufferPos += 1;

            if (bufferPos + 2 > DataLength)
                break;
            memcpy(&players, &Buffer[bufferPos], 2);
            bufferPos += 2;
            players = players > 1000 ? 1000 : players;

            if (bufferPos + 2 > DataLength)
                break;
            memcpy(&maxPlayers, &Buffer[bufferPos], 2);
            bufferPos += 2;
            maxPlayers = maxPlayers > 1000 ? 1000 : maxPlayers;

            AnsiString fields[3];
            const DWORD maxLengths[3] = {63, 39, 39};
            bool valid = true;
            for (int field = 0; field < 3; field++)
            {
                DWORD length = 0;
                if (bufferPos + 4 > DataLength)
                {
                    valid = false;
                    break;
                }
                memcpy(&length, &Buffer[bufferPos], 4);
                bufferPos += 4;
                if (length > maxLengths[field] || length > (DWORD)(DataLength - bufferPos))
                {
                    valid = false;
                    break;
                }
                fields[field] = length > 0 ? AnsiString(Buffer + bufferPos, length) : AnsiString("-");
                bufferPos += length;
            }
            if (!valid)
                break;

            Servers[index].Passworded = passworded != 0;
            Servers[index].MaxPlayers = (int)maxPlayers;
            Servers[index].Players = players > maxPlayers ? (int)maxPlayers : (int)players;
            Servers[index].HostName = fields[0];
            Servers[index].Mode = fields[1];
            Servers[index].Map = fields[2];
            Servers[index].QueryInfoReceived = true;

            repaintServers = true;
            rebuildServers = true;
            Query(Form,
                  String(Servers[index].Address) + ":" + IntToStr(Servers[index].Port) + "#" +
                      IntToStr(Servers[index].Tag),
                  true, false, false, false);
            break;
        }

        case 'c':
        {
            int bufferPos = 11;
            if (bufferPos + 2 > DataLength)
                break;
            WORD playerCount = 0;
            memcpy(&playerCount, &Buffer[bufferPos], 2);
            bufferPos += 2;
            if (playerCount > 100)
                playerCount = 100;

            std::vector<TPlayerInfo> players;
            players.reserve(playerCount);
            bool valid = true;
            for (int i = 0; i < (int)playerCount; i++)
            {
                if (bufferPos >= DataLength)
                {
                    valid = false;
                    break;
                }
                BYTE nameLength = 0;
                memcpy(&nameLength, &Buffer[bufferPos], 1);
                bufferPos += 1;
                if (bufferPos + nameLength > DataLength)
                {
                    valid = false;
                    break;
                }

                TPlayerInfo player;
                player.Name = AnsiString(Buffer + bufferPos, nameLength);
                bufferPos += nameLength;
                if (bufferPos + 4 > DataLength)
                {
                    valid = false;
                    break;
                }

                int score = 0;
                memcpy(&score, &Buffer[bufferPos], 4);
                bufferPos += 4;
                if (score > 1000000)
                    score = 1000000;
                if (score < 0)
                    score = 0;
                player.Score = score;
                players.push_back(player);
            }

            if (valid)
            {
                Servers[index].aPlayers.swap(players);
                Servers[index].Players = (int)Servers[index].aPlayers.size();
                repaintPlayers = true;
                rebuildServers = true;
            }
            break;
        }

        case 'r':
        {
            int bufferPos = 11;
            if (bufferPos + 2 > DataLength)
                break;
            WORD ruleCount = 0;
            memcpy(&ruleCount, &Buffer[bufferPos], 2);
            bufferPos += 2;
            if (ruleCount > 30)
                ruleCount = 30;

            std::vector<TRuleInfo> rules;
            rules.reserve(ruleCount);
            bool valid = true;
            for (int i = 0; i < (int)ruleCount; i++)
            {
                if (bufferPos >= DataLength)
                {
                    valid = false;
                    break;
                }

                BYTE length = 0;
                memcpy(&length, &Buffer[bufferPos], 1);
                bufferPos += 1;
                if (bufferPos + length > DataLength)
                {
                    valid = false;
                    break;
                }

                TRuleInfo rule;
                rule.Rule = AnsiString(Buffer + bufferPos, length);
                bufferPos += length;
                if (bufferPos >= DataLength)
                {
                    valid = false;
                    break;
                }

                memcpy(&length, &Buffer[bufferPos], 1);
                bufferPos += 1;
                if (bufferPos + length > DataLength)
                {
                    valid = false;
                    break;
                }

                rule.Value = AnsiString(Buffer + bufferPos, length);
                bufferPos += length;
                rules.push_back(rule);
            }

            if (valid)
            {
                Servers[index].aRules.swap(rules);
                repaintRules = true;
            }
            break;
        }
    }

    if (CMasterServer::IsQueryBatchActive())
    {
        CMasterServer::OnServerQueryProgress(Form, index);
    }
    else
    {
        if (rebuildServers)
        {
            CServerList::MarkOrderDirty();
            CServerList::MarkDirty();
        }
        if (repaintServers)
            Form->lbServers->Invalidate();
        if (repaintPlayers)
            Form->lbPlayers->Invalidate();
        if (repaintRules)
            Form->lbRules->Invalidate();
    }
}

void CServerQuery::HandleRecv(TfmMain* Form, TMessage& Message)
{
    int socketError = WSAGETSELECTERROR(Message.LParam);
    if (socketError != 0)
    {
        if (socketError != WSAEWOULDBLOCK)
        {
            wchar_t error[512] = {};
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, (DWORD)socketError, 0, error, 512, nullptr);
            MessageDlg(String(error), mtError, TMsgDlgButtons() << mbOK, 0);
        }
        return;
    }
    if (WSAGETSELECTEVENT(Message.LParam) != FD_READ)
        return;

    char buffer[2049] = {};
    sockaddr_in from = {};
    int fromLength = sizeof(from);
    int length = recvfrom(QuerySocket, buffer, 2048, 0, reinterpret_cast<sockaddr*>(&from), &fromLength);

    while (length > 0)
    {
        AnsiString sourceIP = inet_ntoa(from.sin_addr);
        WORD sourcePort = ntohs(from.sin_port);
        Parse(Form, String(sourceIP), sourcePort, buffer, length);

        ZeroMemory(buffer, sizeof(buffer));
        ZeroMemory(&from, sizeof(from));
        from.sin_family = AF_INET;
        fromLength = sizeof(from);
        length = recvfrom(QuerySocket, buffer, 2048, 0, reinterpret_cast<sockaddr*>(&from), &fromLength);
    }
}

void CServerQuery::HandleBatchReady(TfmMain* Form, TMessage& Message)
{
    if (!Form || !QueryNetworkThread)
        return;

    std::vector<TReceivedQueryPacket> packets;
    QueryNetworkThread->TakeReceived(packets);
    for (int i = 0; i < (int)packets.size(); i++)
    {
        in_addr address = {};
        address.s_addr = packets[i].Address;
        AnsiString sourceIP = inet_ntoa(address);
        Parse(Form, String(sourceIP), packets[i].Port, packets[i].Data, packets[i].Length);
    }
}

void CServerQuery::Enqueue(const String& Query)
{
    if (!QueryQueue)
        QueryQueue = new TStringList();
    QueryQueue->Add(Query);
}

void CServerQuery::ClearQueue()
{
    if (QueryQueue)
        QueryQueue->Clear();
    QueryQueuePosition = 0;
}

int CServerQuery::PendingCount()
{
    if (!QueryQueue)
        return 0;
    int pending = QueryQueue->Count - QueryQueuePosition;
    return pending > 0 ? pending : 0;
}

void CServerQuery::ProcessQueue(TfmMain* Form, int MaxPerTick)
{
    if (!Form || !QueryQueue)
        return;

    int sent = 0;
    while (PendingCount() > 0 && sent < MaxPerTick)
    {
        Query(Form, QueryQueue->Strings[QueryQueuePosition], false, true, false, false);
        QueryQueuePosition++;
        sent++;
    }

    if (PendingCount() == 0 && QueryQueuePosition > 0)
        ClearQueue();
}

void CServerQuery::ClearNetwork()
{
    if (QueryNetworkThread)
        QueryNetworkThread->Clear();
    ClearQueue();
}
