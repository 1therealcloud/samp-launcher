/*
* main.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "about.h"
#include "exportfavorites.h"
#include "findsort.h"
#include "importfavorites.h"
#include "main.h"
#include "masterupdate.h"
//#include "rcon.h"
#include "rconconfig.h"
#include "serverproperties.h"
#include "settings.h"
#include "unit_webrunform.h"


#include <vector>
#include <algorithm>
#include <deque>
#include <map>
#include <set>
#include <string>

#include <shlobj.h>
#include <mmsystem.h>
#include <objbase.h>
#include <shlguid.h>

#include <winsock2.h>

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

#pragma package(smart_init)
#pragma resource "..\\dfm\\main.dfm"




std::vector<TServerInfo> Servers;

TSortMode   SortMode        = TSortMode::smHostName;
TSortMode   OldSortMode     = TSortMode::smHostName;
TSortDir    SortDir         = TSortDir::sdUp;
TSortDir    OldSortDir      = TSortDir::sdUp;

AnsiChar    FileTag[4]      = { 'S', 'A', 'M', 'P' };
String      gta_sa_exe;

TStringList *QueryQueue     = nullptr;
TStringList *IPList         = nullptr;

// bool
bool        Filtered        = true;
bool        InstanceChecked = false;
bool        FavoritesChanged = false;

// int
int         QuerySocket     = INVALID_SOCKET;
int         SelServer       = -1;
int         PingCounter     = 0;
static std::string PingChartEndpoint;
int         MasterFile      = 1;
int         ServersTopIndex = -1;
static bool WinsockStarted  = false;
static bool MasterUpdateInProgress = false;
static TThread *MasterUpdateThread = nullptr;
static volatile LONG MasterUpdateCancel = 0;
static bool ServerListDirty = false;
static bool ServerOrderDirty = true;
static int QueryQueuePosition = 0;
static bool MasterQueryBatchActive = false;
static int MasterQueryPending = 0;
static DWORD MasterQueryDeadline = 0;
static bool MasterRefreshPending = false;
static TfmMasterUpdate *MasterUpdateProgressForm = nullptr;
static const DWORD MASTER_QUERY_RESPONSE_TIMEOUT_MS = 3000;

struct TDnsQuerySpec {
    String Server;
    String Port;
    String Password;
    bool Ping;
    bool Info;
    bool Players;
    bool Rules;
    bool Connect;
};

struct TDnsRequest {
    HANDLE Task;
    AnsiString Host;
    char Buffer[MAXGETHOSTSTRUCT];
    std::vector<TDnsQuerySpec> Queries;
};

static std::vector<TDnsRequest*> DnsRequests;
static std::map<std::string, String> DnsCache;
static std::map<std::string, int> ServerLookup;

static std::string LowerAnsiKey(const String &Value)
{
    AnsiString lower = AnsiLowerCase(AnsiString(Value));
    return std::string(lower.c_str(), lower.Length());
}

static std::string ServerKey(const AnsiString &IP, WORD Port, WORD Tag)
{
    AnsiString key;
    key.sprintf("%s:%u#%u", IP.c_str(), (unsigned)Port, (unsigned)Tag);
    return std::string(key.c_str(), key.Length());
}

static std::string EndpointKey(const AnsiString &Address, int Port)
{
    AnsiString lower = AnsiLowerCase(Address);
    AnsiString key;
    key.sprintf("%s:%d", lower.c_str(), Port);
    return std::string(key.c_str(), key.Length());
}

static bool SameStrings(TStrings *Left, TStrings *Right)
{
    if (!Left || !Right || Left->Count != Right->Count)
        return false;
    for (int i = 0; i < Left->Count; i++) {
        if (Left->Strings[i] != Right->Strings[i])
            return false;
    }
    return true;
}

static int PendingQueryCount()
{
    if (!QueryQueue)
        return 0;
    int pending = QueryQueue->Count - QueryQueuePosition;
    return pending > 0 ? pending : 0;
}

static void ClearQueryQueue()
{
    if (QueryQueue)
        QueryQueue->Clear();
    QueryQueuePosition = 0;
}

static void EnqueueQuery(const String &Query)
{
    if (QueryQueue)
        QueryQueue->Add(Query);
}

struct TOutgoingQueryPacket {
    sockaddr_in Address;
    int Length;
    LONG Generation;
    char Data[15];
};

struct TReceivedQueryPacket {
    DWORD Address;
    WORD Port;
    int Length;
    char Data[2048];
};

struct TPingRequestTime {
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
        : TThread(true), FSocket(Socket), FNotifyWindow(NotifyWindow),
          FWakeEvent(nullptr), FReceiveEvent(WSA_INVALID_EVENT),
          FResultsPosted(false), FGeneration(0), FNextPingToken(0),
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
        if (FReceiveEvent != WSA_INVALID_EVENT) {
            WSAEventSelect(FSocket, nullptr, 0);
            WSACloseEvent(FReceiveEvent);
        }
        DeleteCriticalSection(&FLock);
    }

    void Queue(const sockaddr_in &Address, const char *Data, int Length);
    void TakeReceived(std::vector<TReceivedQueryPacket> &Packets);
    bool TakePingMilliseconds(DWORD Token, DWORD Address, WORD Port,
                              int &Milliseconds);
    void Clear();
    void Wake() { if (FWakeEvent) SetEvent(FWakeEvent); }
};

static TQueryNetworkThread *QueryNetworkThread = nullptr;

void TQueryNetworkThread::Queue(const sockaddr_in &Address,
                                const char *Data, int Length)
{
    if (!Data || Length < 1 || Length > 15)
        return;
    TOutgoingQueryPacket packet = {};
    packet.Address = Address;
    packet.Length = Length;
    packet.Generation = InterlockedCompareExchange(&FGeneration, 0, 0);
    memcpy(packet.Data, Data, Length);
    EnterCriticalSection(&FLock);
    if (Length == 15 && packet.Data[10] == 'p') {
        do {
            ++FNextPingToken;
        } while (FNextPingToken == 0 ||
                 FPingRequests.find(FNextPingToken) != FPingRequests.end());
        memcpy(&packet.Data[11], &FNextPingToken, sizeof(FNextPingToken));
    }
    FOutgoing.push_back(packet);
    LeaveCriticalSection(&FLock);
    Wake();
}

bool TQueryNetworkThread::TakePingMilliseconds(
    DWORD Token, DWORD Address, WORD Port, int &Milliseconds)
{
    LARGE_INTEGER now = {};
    if (FPerformanceFrequency <= 0 || !QueryPerformanceCounter(&now))
        return false;

    EnterCriticalSection(&FLock);
    std::map<DWORD, TPingRequestTime>::iterator found =
        FPingRequests.find(Token);
    if (found == FPingRequests.end() ||
        found->second.Address != Address || found->second.Port != Port) {
        LeaveCriticalSection(&FLock);
        return false;
    }
    LONGLONG started = found->second.Counter;
    FPingRequests.erase(found);
    LeaveCriticalSection(&FLock);

    LONGLONG elapsed = now.QuadPart - started;
    if (elapsed < 0)
        return false;
    LONGLONG rounded =
        (elapsed * 1000 + FPerformanceFrequency / 2) /
        FPerformanceFrequency;
    if (rounded < 1)
        rounded = 1;
    if (rounded > 60000)
        return false;
    Milliseconds = (int)rounded;
    return true;
}

void TQueryNetworkThread::TakeReceived(
    std::vector<TReceivedQueryPacket> &Packets)
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
    while (!Terminated) {
        std::deque<TOutgoingQueryPacket> outgoing;
        EnterCriticalSection(&FLock);
        outgoing.swap(FOutgoing);
        LeaveCriticalSection(&FLock);

        while (!outgoing.empty() && !Terminated) {
            const TOutgoingQueryPacket &packet = outgoing.front();
            if (packet.Generation ==
                InterlockedCompareExchange(&FGeneration, 0, 0)) {
                DWORD pingToken = 0;
                bool isPing = packet.Length == 15 && packet.Data[10] == 'p';
                if (isPing && FPerformanceFrequency > 0) {
                    LARGE_INTEGER started = {};
                    if (QueryPerformanceCounter(&started)) {
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
                                  reinterpret_cast<const sockaddr*>(&packet.Address),
                                  sizeof(packet.Address));
                if (sent == SOCKET_ERROR && pingToken != 0) {
                    EnterCriticalSection(&FLock);
                    FPingRequests.erase(pingToken);
                    LeaveCriticalSection(&FLock);
                }
            }
            outgoing.pop_front();
        }

        std::vector<TReceivedQueryPacket> received;
        for (;;) {
            TReceivedQueryPacket packet = {};
            sockaddr_in from = {};
            int fromLength = sizeof(from);
            int length = recvfrom(FSocket, packet.Data, sizeof(packet.Data), 0,
                                  reinterpret_cast<sockaddr*>(&from),
                                  &fromLength);
            if (length == SOCKET_ERROR) {
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
        if (!received.empty()) {
            EnterCriticalSection(&FLock);
            FReceived.insert(FReceived.end(),
                             received.begin(), received.end());
            if (!FResultsPosted) {
                FResultsPosted = true;
                postResults = true;
            }
            LeaveCriticalSection(&FLock);
        }
        if (postResults)
            PostMessage(FNotifyWindow, WM_QUERY_BATCH_READY, 0, 0);

        if (!Terminated) {
            HANDLE events[2] = { FWakeEvent, FReceiveEvent };
            DWORD count = FReceiveEvent == WSA_INVALID_EVENT ? 1 : 2;
            DWORD waitResult = WaitForMultipleObjects(
                count, events, FALSE, 50);
            if (count == 2 && waitResult == WAIT_OBJECT_0 + 1) {
                WSANETWORKEVENTS networkEvents = {};
                WSAEnumNetworkEvents(FSocket, FReceiveEvent,
                                     &networkEvents);
            }
        }
    }
}

enum class TGameLaunchError {
    None,
    Execute,
    Allocate,
    WritePath,
    CreateRemoteThread,
    LoadLibrary,
    Resume
};

class TGameLaunchThread : public TThread
{
private:
    HWND FNotifyWindow;
    String FGameExe;
    String FCommandLine;
    String FWorkDir;
    String FSampDll;

protected:
    void __fastcall Execute() override;

public:
    TGameLaunchError Error;

    __fastcall TGameLaunchThread(HWND NotifyWindow, const String &GameExe,
        const String &CommandLine, const String &WorkDir,
        const String &SampDll)
        : TThread(true), FNotifyWindow(NotifyWindow), FGameExe(GameExe),
          FCommandLine(CommandLine), FWorkDir(WorkDir), FSampDll(SampDll),
          Error(TGameLaunchError::None)
    {
        FreeOnTerminate = false;
    }
};

static TGameLaunchThread *GameLaunchThread = nullptr;
static bool GameLaunchShuttingDown = false;
static bool GameLaunchDnsPending = false;

TfmMain *fmMain;

static bool MasterUpdateThreadFinished()
{
    return MasterUpdateThread == nullptr ||
        WaitForSingleObject((HANDLE)MasterUpdateThread->Handle, 0) ==
            WAIT_OBJECT_0;
}

// utils

TColor DarkenColor(TColor Color, Byte Percent)
{
    // RGB
    COLORREF rgbColor = ColorToRGB(Color);

    BYTE R = GetRValue(rgbColor);
    BYTE G = GetGValue(rgbColor);
    BYTE B = GetBValue(rgbColor);

    R = static_cast<BYTE>(R * (100 - Percent) / 100);
    G = static_cast<BYTE>(G * (100 - Percent) / 100);
    B = static_cast<BYTE>(B * (100 - Percent) / 100);

    return static_cast<TColor>(RGB(R, G, B));
}

static void CheckAnotherInstance()
{
    if (!InstanceChecked) {
        // Pascal mutex
        CreateMutex(nullptr, false, L"kyeman and spookie woz 'ere, innit.");
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            MessageBoxW(0, L"SA:MP is already running.\n\nYou can only run one instance at a time.", L"SA:MP Error", MB_ICONERROR);
            ExitProcess(0);
        }
        InstanceChecked = true;
    }
}

static String GetUserFilesPath()
{
    wchar_t Path[MAX_PATH];
    if (SHGetSpecialFolderPathW(0, Path, CSIDL_PERSONAL, false))
        return String(Path) + "\\GTA San Andreas User Files\\SAMP\\";
    return "";
}

static String GetUserDataFileName()
{
    String path = GetUserFilesPath();
    return path.IsEmpty() ? String() : path + "USERDATA.DAT";
}

static bool SetupUserFilesFolder(const String &Path)
{
    if (Path.IsEmpty())
        return false;
    int result = SHCreateDirectoryExW(nullptr, Path.c_str(), nullptr);
    return result == ERROR_SUCCESS ||
           result == ERROR_ALREADY_EXISTS ||
           result == ERROR_FILE_EXISTS;
}

static String GetCachedIPFromHost(const String &HostName)
{
    AnsiString ansiHost = HostName;
    unsigned long numericAddress = inet_addr(ansiHost.c_str());
    if (numericAddress != INADDR_NONE || HostName == "255.255.255.255")
        return HostName;

    std::map<std::string, String>::const_iterator cached =
        DnsCache.find(LowerAnsiKey(HostName));
    if (cached == DnsCache.end())
        return "";
    return cached->second == "1" ? String() : cached->second;
}

static bool HasCachedHost(const String &HostName)
{
    AnsiString ansiHost = HostName;
    unsigned long numericAddress = inet_addr(ansiHost.c_str());
    if (numericAddress != INADDR_NONE || HostName == "255.255.255.255")
        return true;
    return DnsCache.find(LowerAnsiKey(HostName)) != DnsCache.end();
}

static void CacheHost(const String &HostName, const String &Address)
{
    String value = Address.IsEmpty() ? String("1") : Address;
    DnsCache[LowerAnsiKey(HostName)] = value;
    if (IPList)
        IPList->Values[HostName] = value;
}

static void RebuildServerLookup()
{
    ServerLookup.clear();
    for (int i = 0; i < (int)Servers.size(); i++) {
        TServerInfo &server = Servers[i];
        WORD tag = server.Tag != 0 ? server.Tag : (WORD)server.Port;
        AnsiString numeric = server.Address;
        unsigned long address = inet_addr(numeric.c_str());
        if (address == INADDR_NONE && numeric != "255.255.255.255") {
            String cached = GetCachedIPFromHost(String(server.Address));
            if (!cached.IsEmpty()) {
                server.DottedAddress = AnsiString(cached);
                server.HasAddress = true;
                numeric = server.DottedAddress;
            }
        }
        if (!numeric.IsEmpty())
            ServerLookup[ServerKey(numeric, (WORD)server.Port, tag)] = i;
        if (!server.DottedAddress.IsEmpty())
            ServerLookup[ServerKey(server.DottedAddress,
                                   (WORD)server.Port, tag)] = i;
    }
}

static bool QueueDnsRequest(HWND Window, const AnsiString &Host,
                            const TDnsQuerySpec &Query)
{
    for (int i = 0; i < (int)DnsRequests.size(); i++) {
        if (AnsiCompareText(DnsRequests[i]->Host, Host) == 0) {
            DnsRequests[i]->Queries.push_back(Query);
            return true;
        }
    }

    TDnsRequest *request = new TDnsRequest();
    request->Task = nullptr;
    request->Host = Host;
    ZeroMemory(request->Buffer, sizeof(request->Buffer));
    request->Queries.push_back(Query);
    request->Task = WSAAsyncGetHostByName(Window, WM_DNS_RECV, Host.c_str(),
                                          request->Buffer, sizeof(request->Buffer));
    if (!request->Task) {
        delete request;
        return false;
    }
    DnsRequests.push_back(request);
    return true;
}

static bool GetSavePasswordSetting(const String &ValueName, bool DefaultValue)
{
    bool result = DefaultValue;
    TRegistry *reg = new TRegistry();
    try {
        reg->RootKey = HKEY_CURRENT_USER;
        if (reg->OpenKeyReadOnly(L"SOFTWARE\\SAMP")) {
            if (reg->ValueExists(ValueName))
                result = reg->ReadBool(ValueName);
            reg->CloseKey();
        }
    }
    __finally {
        delete reg;
    }
    return result;
}

static bool HasUnsafeCommandLineCharacters(const String &Value)
{
    for (int i = 1; i <= Value.Length(); i++) {
        if (Value[i] <= 32 || Value[i] == '"')
            return true;
    }
    return false;
}

static String GetAbsolutePath(const String &Path)
{
    String clean = Path.Trim();
    if (clean.Length() >= 2 && clean[1] == (wchar_t)34 &&
        clean[clean.Length()] == (wchar_t)34)
        clean = clean.SubString(2, clean.Length() - 2);
    if (clean.IsEmpty())
        return clean;

    DWORD required = GetFullPathNameW(clean.c_str(), 0, nullptr, nullptr);
    if (required == 0)
        return clean;

    std::vector<wchar_t> buffer(static_cast<size_t>(required) + 1);
    DWORD written = GetFullPathNameW(clean.c_str(),
        static_cast<DWORD>(buffer.size()), &buffer[0], nullptr);
    if (written == 0 || written >= buffer.size())
        return clean;
    return String(&buffer[0]);
}

static const int MAX_FAVORITE_SERVERS = 100000;
static const int MAX_FAVORITE_ADDRESS_LENGTH = 1024;
static const int MAX_FAVORITE_HOSTNAME_LENGTH = 4096;
static const int MAX_FAVORITE_PASSWORD_LENGTH = 4096;
static const LONGLONG MAX_FAVORITES_FILE_SIZE = 64LL * 1024 * 1024;

static bool IsValidServerEndpoint(const String &Address, int Port)
{
    return !Address.IsEmpty() && Address.Length() <= 253 &&
           Address.Pos("#") == 0 && Address.Pos("/") == 0 &&
           !HasUnsafeCommandLineCharacters(Address) &&
           Port >= 1 && Port <= 65535;
}

static bool ReadExact(HANDLE File, void *Buffer, DWORD Size)
{
    DWORD bytesRead = 0;
    return ReadFile(File, Buffer, Size, &bytesRead, nullptr) && bytesRead == Size;
}

static bool ReadFavoriteString(HANDLE File, AnsiString &Value, int MaxLength)
{
    int length = 0;
    if (!ReadExact(File, &length, sizeof(length)) || length < 0 || length > MaxLength)
        return false;

    if (length == 0) {
        Value = AnsiString();
        return true;
    }

    LARGE_INTEGER currentPosition = {};
    LARGE_INTEGER zero = {};
    LARGE_INTEGER fileSize = {};
    if (!SetFilePointerEx(File, zero, &currentPosition, FILE_CURRENT) ||
        !GetFileSizeEx(File, &fileSize) ||
        fileSize.QuadPart - currentPosition.QuadPart < length)
        return false;

    Value.SetLength(length);
    return ReadExact(File, &Value[1], (DWORD)length);
}

static bool WriteExact(HANDLE File, const void *Buffer, DWORD Size)
{
    DWORD bytesWritten = 0;
    return WriteFile(File, Buffer, Size, &bytesWritten, nullptr) && bytesWritten == Size;
}

static bool WriteFavoriteString(HANDLE File, const AnsiString &Value)
{
    int length = Value.Length();
    if (!WriteExact(File, &length, sizeof(length)))
        return false;
    return length == 0 || WriteExact(File, Value.c_str(), (DWORD)length);
}

/*
void __fastcall TfmMain::CreateFASTDesktoplink1Click(TObject *Sender)
{
    if (lbServers->ItemIndex == -1) return;
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size()) return;

    CreateDesktopShortcut(String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port), String(Servers[Idx].HostName));
}

static void CreateDesktopShortcut(String Arguments, String ShortcutName)
{
    CoInitialize(nullptr);
    IShellLink *psl = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
    if (SUCCEEDED(hr)) {
        IPersistFile *ppf = nullptr;
        psl->SetPath(Application->ExeName.c_str());
        psl->SetWorkingDirectory(ExtractFilePath(Application->ExeName).c_str());
        psl->SetArguments(Arguments.c_str());

        hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
        if (SUCCEEDED(hr)) {
            wchar_t Path[MAX_PATH];
            SHGetSpecialFolderPathW(0, Path, CSIDL_DESKTOP, false);
            String LinkName = String(Path) + "\\" + ShortcutName + ".lnk";
            ppf->Save(LinkName.c_str(), false);
            ppf->Release();
        }
        psl->Release();
    }
    CoUninitialize();
}
*/

String __fastcall TfmMain::GetToken(String TokenData, int ItemIndex, String TokenDelimiter)
{
    int tokenCount = 0, i = 1;
    String result = "";
    int len = TokenData.Length();
    int delimLen = TokenDelimiter.Length();

    if (len > 0) {
        while (i <= len) {
            if (tokenCount == (ItemIndex - 1)) {
                if (TokenData.SubString(i, delimLen) == TokenDelimiter)
                    break;
                result += TokenData[i];
            }
            if (TokenData.SubString(i, delimLen) == TokenDelimiter) {
                tokenCount++;
                i += delimLen - 1;
            }
            i++;
        }
    }
    return result;
}

String __fastcall TfmMain::GetClipBoardStr()
{
    return Clipboard()->AsText;
}

void __fastcall TfmMain::SetClipBoardStr(String Str)
{
    Clipboard()->AsText = Str;
}

void __fastcall TfmMain::GetGTAExe(HWND Owner)
{
    String tmpStr, browseExe;
    wchar_t buf[MAX_PATH] = {};
    DWORD   bufSize = sizeof(buf);
    HKEY    hKey    = nullptr;

    // read registry
    HKEY roots[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
    for (int r = 0; r < 2 && tmpStr.IsEmpty(); r++) {
        if (RegOpenKeyExW(roots[r],
                L"SOFTWARE\\Rockstar Games\\GTA San Andreas\\Installation",
                0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            bufSize = sizeof(buf);
            if (RegQueryValueExW(hKey, L"ExePath", nullptr, nullptr,
                    (LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
                tmpStr = buf;
            RegCloseKey(hKey);
            hKey = nullptr;
        }
    }

    tmpStr = tmpStr.Trim();
    if (tmpStr.Length() >= 2 && tmpStr[1] == '"' &&
        tmpStr[tmpStr.Length()] == '"')
        tmpStr = tmpStr.SubString(2, tmpStr.Length() - 2);
    if (!tmpStr.IsEmpty())
        tmpStr = ExtractFilePath(tmpStr);

    if (BrowseForFolder(Owner, browseExe, tmpStr,
            "Please locate your GTA: San Andreas installation..."))
    {
        gta_sa_exe = browseExe + "\\gta_sa.exe";

        if (RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\SAMP", 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
            RegSetValueExW(hKey, L"gta_sa_exe", 0, REG_SZ, (LPBYTE)gta_sa_exe.c_str(), (gta_sa_exe.Length() + 1) * sizeof(wchar_t));
            RegCloseKey(hKey);
        }
    }
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED) {
        SetWindowTextW(hwnd, L"GTA: San Andreas Installation");
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

// forms

__fastcall TfmMain::TfmMain(TComponent *Owner)
    : TForm(Owner)
{
}

void __fastcall TfmMain::FormCreate(TObject *Sender)
{
    GameLaunchShuttingDown = false;
    GameLaunchDnsPending = false;
    IPList           = new TStringList();
    QueryQueue       = new TStringList();
    FavoritesChanged = false;

    // read registry
    HKEY hKey = nullptr;
    wchar_t buf[512];
    DWORD bufSize;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\SAMP", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        ZeroMemory(buf, sizeof(buf));
        bufSize = sizeof(buf) - sizeof(wchar_t);
        if (RegQueryValueExW(hKey, L"gta_sa_exe", nullptr, nullptr, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
            gta_sa_exe = buf;

        ZeroMemory(buf, sizeof(buf));
        bufSize = sizeof(buf) - sizeof(wchar_t);
        if (RegQueryValueExW(hKey, L"PlayerName", nullptr, nullptr, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
            edName->Text = buf;

        RegCloseKey(hKey);
    }

    // Pascal: GetGTAExe
    if (gta_sa_exe.IsEmpty())
        GetGTAExe(Handle);

    Randomize();

    WSADATA wsData;
    WinsockStarted = WSAStartup(0x0202, &wsData) == 0;
    if (WinsockStarted) {
        QuerySocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (QuerySocket != INVALID_SOCKET) {
            int receiveBufferSize = 1024 * 1024;
            setsockopt(QuerySocket, SOL_SOCKET, SO_RCVBUF,
                       reinterpret_cast<const char*>(&receiveBufferSize),
                       sizeof(receiveBufferSize));
        }
        sockaddr_in s_in = {};
        s_in.sin_family      = AF_INET;
        s_in.sin_addr.s_addr = INADDR_ANY;
        s_in.sin_port        = 0;
        u_long nonBlocking = 1;
        if (QuerySocket == INVALID_SOCKET ||
            bind(QuerySocket, (sockaddr*)&s_in, sizeof(s_in)) == SOCKET_ERROR ||
            ioctlsocket(QuerySocket, FIONBIO, &nonBlocking) == SOCKET_ERROR)
        {
            if (QuerySocket != INVALID_SOCKET)
                closesocket(QuerySocket);
            QuerySocket = INVALID_SOCKET;
        }
    }

    if (QuerySocket != INVALID_SOCKET) {
        try {
            QueryNetworkThread = new TQueryNetworkThread(QuerySocket, Handle);
            QueryNetworkThread->Start();
        }
        catch (...) {
            delete QueryNetworkThread;
            QueryNetworkThread = nullptr;
            closesocket(QuerySocket);
            QuerySocket = INVALID_SOCKET;
        }
    }

    if (QuerySocket == INVALID_SOCKET)
        MessageDlg("Unable to initialize the server query socket.", mtError, TMsgDlgButtons() << mbOK, 0);

    tbMasterServerUpdate->OnClick = tbMasterServerUpdateClick;

    // QueryQueue already created
    tmrQueryQueueProcess->Enabled = QuerySocket != INVALID_SOCKET;
    tmrServerListUpdate->Enabled  = true;

    String userDataPath = GetUserDataFileName();
    if (!userDataPath.IsEmpty() && SetupUserFilesFolder(GetUserFilesPath()) &&
        FileExists(userDataPath))
        ImportFavorites(userDataPath, false);

    bool dummy = true;
    tsServerListsChange(this, 0, dummy);
    lbServersClick(this);
    UpdateServers();
}

void __fastcall TfmMain::FormDestroy(TObject *Sender)
{
    GameLaunchShuttingDown = true;
    GameLaunchDnsPending = false;
    tmrQueryQueueProcess->Enabled = false;
    tmrServerListUpdate->Enabled = false;
    tmSIPingUpdate->Enabled = false;

    if (QueryNetworkThread) {
        QueryNetworkThread->Terminate();
        QueryNetworkThread->Wake();
        QueryNetworkThread->WaitFor();
        delete QueryNetworkThread;
        QueryNetworkThread = nullptr;
    }

    if (GameLaunchThread) {
        GameLaunchThread->Terminate();
        GameLaunchThread->WaitFor();
        MSG pendingMessage = {};
        while (PeekMessageW(&pendingMessage, Handle,
                WM_GAME_LAUNCH_COMPLETE, WM_GAME_LAUNCH_COMPLETE,
                PM_REMOVE)) {
        }
        delete GameLaunchThread;
        GameLaunchThread = nullptr;
    }

    MasterRefreshPending = false;
    CancelMasterServerUpdate();
    if (MasterUpdateThread) {
        MasterUpdateThread->Terminate();
        MasterUpdateThread->WaitFor();
        delete MasterUpdateThread;
        MasterUpdateThread = nullptr;
    }

    for (int i = 0; i < (int)DnsRequests.size(); i++) {
        if (DnsRequests[i]->Task)
            WSACancelAsyncRequest(DnsRequests[i]->Task);
        delete DnsRequests[i];
    }
    DnsRequests.clear();

    HKEY hKey = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\SAMP", 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS)
    {
        RegSetValueExW(hKey, L"PlayerName", 0, REG_SZ, (LPBYTE)edName->Text.c_str(), (edName->Text.Length() + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
    }

    if (QuerySocket != INVALID_SOCKET) {
        shutdown(QuerySocket, SD_BOTH);
        closesocket(QuerySocket);
        QuerySocket = INVALID_SOCKET;
    }
    if (WinsockStarted) {
        WSACleanup();
        WinsockStarted = false;
    }

    delete QueryQueue;  QueryQueue = nullptr;
    delete IPList;      IPList     = nullptr;

    if (tsServerLists->TabIndex == 0 && FavoritesChanged) {
        ExportFavorites(GetUserDataFileName(), true);
        FavoritesChanged = false;
    }
}

void __fastcall TfmMain::FormShow(TObject *Sender)
{
    lbServers->DoubleBuffered = true;
    lbPlayers->DoubleBuffered = true;
    lbRules->DoubleBuffered = true;
    lbServers->SetFocus();

    //  samp://host:port/password
    if (ParamCount() > 0) {
        String servFull = ParamStr(1);
        String servPass = (ParamCount() > 1) ? ParamStr(2) : "";

        if (servFull.SubString(1, 7).LowerCase() == "samp://") {
            servFull = servFull.SubString(8, servFull.Length() - 7);
            int slashPos = servFull.Pos("/");
            if (slashPos > 0) {
                if (servPass.IsEmpty())
                    servPass = servFull.SubString(
                        slashPos + 1, servFull.Length() - slashPos);
                servFull = servFull.SubString(1, slashPos - 1);
            }
            String servAddr, servPort;
            int colonPos = servFull.Pos(":");
            if (colonPos > 0) {
                servAddr = servFull.SubString(1, colonPos - 1);
                servPort = servFull.SubString(colonPos + 1, servFull.Length() - colonPos);
                servPort = IntToStr(StrToIntDef(servPort, 7777));
            } else {
                servAddr = servFull;
                servPort = "7777";
            }

            if (wnd_webrunform) {
                wnd_webrunform->Label1->Caption = "Do you want to add " + servAddr + ":" + servPort + " to your favorites \n or play on this server now?";
                switch (wnd_webrunform->ShowModal()) {
                    case mrOk:
                        ServerConnect(servAddr, servPort, servPass);
                        break;
                    case mrYes:
                        #ifndef _DEBUG
                            CheckAnotherInstance();
                        #endif
                        AddServer(servAddr + ":" + servPort);
                        break;
                    case mrCancel:
                        break;
                }
            }
        } else {
            String servAddr, servPort;
            int colonPos = servFull.Pos(":");
            if (colonPos > 0) {
                servAddr = servFull.SubString(1, colonPos - 1);
                servPort = IntToStr(StrToIntDef(servFull.SubString(colonPos + 1, 5), 7777));
            } else {
                servAddr = servFull;
                servPort = "7777";
            }
            ServerConnect(servAddr, servPort, servPass);
        }
    }

    #ifndef _DEBUG
        CheckAnotherInstance();
    #endif

    // виставляємо правльну позицію для status-bar
    sbMain->Top = 10000;
}

void __fastcall TfmMain::FormResize(TObject *Sender)
{
    imLogo->Left = Width - imLogo->Width - 2;
    imLogo->Repaint();
}

void __fastcall TfmMain::SaveFavoritesNow()
{
    // Негайний, безумовний запис USERDATA.DAT — на відміну від FavoritesChanged,
    // який чекає на зміну вкладки або закриття вікна. Викликається одразу
    // після редагування пароля (ServerProperties), щоб не залежати від того,
    // чи користувач ще щось зробить перед закриттям програми.
    FavoritesChanged = !ExportFavorites(GetUserDataFileName(), true);
}



bool __fastcall TfmMain::BrowseForFolder(HWND Owner, String &Directory, String StartDir, String Title)
{
    BROWSEINFOW bi    = {};
    wchar_t dispName[MAX_PATH] = {};
    wchar_t tempPath[MAX_PATH] = {};

    bi.hwndOwner      = Owner;
    bi.pszDisplayName = dispName;
    bi.lpszTitle      = Title.c_str();
    bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lParam         = (LPARAM)StartDir.c_str();
    bi.lpfn           = BrowseCallbackProc;

    PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
    if (pidl) {
        bool selected = SHGetPathFromIDListW(pidl, tempPath);
        CoTaskMemFree(pidl);
        if (selected) {
            Directory = tempPath;
            return true;
        }
    }
    return false;
}

void __fastcall TfmMain::UpdateServers()
{
    int ActualIdx, Idx, i;
    String OldServ;
    bool ItemFiltered;
    bool TrackingChanges = true;
    int TotServers, TotSlots, TotPlayers;
    TStringList *NewServs = new TStringList();
    int TopIndexes[3];
    int TopIndexSaved = lbServers->TopIndex;

    lbServers->Items->BeginUpdate();

    ServersTopIndex = lbServers->TopIndex;
    TopIndexes[0] = lbServers->TopIndex;
    TopIndexes[1] = lbPlayers->TopIndex;
    TopIndexes[2] = lbRules->TopIndex;

    Idx = -1;
    ActualIdx = lbServers->ItemIndex;
    if (ActualIdx != -1) {
        Idx = StrToIntDef(lbServers->Items->Strings[ActualIdx], -1);
        if (Idx >= 0 && Idx < (int)Servers.size())
            OldServ = String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port);
    }

    if (ServerOrderDirty || OldSortMode != SortMode || OldSortDir != SortDir) {
        switch (SortMode) {
            case TSortMode::smHostName:
                std::sort(Servers.begin(), Servers.end(),
                    [](const TServerInfo &a, const TServerInfo &b) {
                        return AnsiCompareText(a.HostName, b.HostName) < 0;
                    });
                break;
            case TSortMode::smPlayers:
                std::sort(Servers.begin(), Servers.end(),
                    [](const TServerInfo &a, const TServerInfo &b) {
                        return a.Players < b.Players;
                    });
                break;
            case TSortMode::smPing:
                std::sort(Servers.begin(), Servers.end(),
                    [](const TServerInfo &a, const TServerInfo &b) {
                        return a.Ping < b.Ping;
                    });
                break;
            case TSortMode::smMode:
                std::sort(Servers.begin(), Servers.end(),
                    [](const TServerInfo &a, const TServerInfo &b) {
                        return AnsiCompareText(a.Mode, b.Mode) < 0;
                    });
                break;
            case TSortMode::smMap:
                std::sort(Servers.begin(), Servers.end(),
                    [](const TServerInfo &a, const TServerInfo &b) {
                        return AnsiCompareText(a.Map, b.Map) < 0;
                    });
                break;
        }
        OldSortMode = SortMode;
        OldSortDir = SortDir;
        ServerOrderDirty = false;
    }

    RebuildServerLookup();

    AnsiString modeFilter = AnsiLowerCase(AnsiString(edFilterMode->Text));
    AnsiString mapFilter = AnsiLowerCase(AnsiString(edFilterMap->Text));
    int first = SortDir == TSortDir::sdDown
        ? (int)Servers.size() - 1 : 0;
    int last = SortDir == TSortDir::sdDown
        ? -1 : (int)Servers.size();
    int step = SortDir == TSortDir::sdDown ? -1 : 1;
    for (i = first; i != last; i += step) {
        ItemFiltered = Servers[i].MaxPlayers < 1 && MasterFile != 0;
        if (Filtered) {
            if (!modeFilter.IsEmpty() &&
                AnsiPos(modeFilter, AnsiLowerCase(Servers[i].Mode)) == 0)
                ItemFiltered = true;
            if (!mapFilter.IsEmpty() &&
                AnsiPos(mapFilter, AnsiLowerCase(Servers[i].Map)) == 0)
                ItemFiltered = true;
            if (cbFilterFull->Checked &&
                Servers[i].Players == Servers[i].MaxPlayers)
                ItemFiltered = true;
            if (cbFilterEmpty->Checked && Servers[i].Players == 0)
                ItemFiltered = true;
            if (cbFilterPassworded->Checked && Servers[i].Passworded)
                ItemFiltered = true;
        }
        if (!ItemFiltered)
            NewServs->Add(IntToStr(i));
    }

    if (!SameStrings(lbServers->Items, NewServs)) {
        lbServers->Items->Assign(NewServs);
        lbServers->TopIndex = TopIndexes[0];
    }

    int newItemIndex = -1;
    if (!OldServ.IsEmpty()) {
        for (i = 0; i < lbServers->Items->Count; i++) {
            Idx = StrToIntDef(lbServers->Items->Strings[i], -1);
            if (Idx >= 0 && Idx < (int)Servers.size()) {
                if (String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port) == OldServ) {
                    newItemIndex = i;
                    break;
                }
            }
        }
    }
    if (newItemIndex == -1 && lbServers->Items->Count > 0)
        newItemIndex = (ActualIdx >= 0 && ActualIdx < lbServers->Items->Count)
            ? ActualIdx : 0;
    lbServers->ItemIndex = newItemIndex;

    Idx = -1;
    if (lbServers->ItemIndex != -1)
        Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);

    NewServs->Clear();
    if (Idx >= 0 && Idx < (int)Servers.size()) {
        for (i = 0; i < (int)Servers[Idx].aPlayers.size(); i++)
            NewServs->Add(IntToStr(i));
        if (!SameStrings(lbPlayers->Items, NewServs)) {
            lbPlayers->Items->Assign(NewServs);
            lbPlayers->TopIndex = TopIndexes[1];
        }

        NewServs->Clear();
        label_url->Caption = "";
        for (i = 0; i < (int)Servers[Idx].aRules.size(); i++) {
            NewServs->Add(IntToStr(i));
            if (Servers[Idx].aRules[i].Rule == "weburl")
                label_url->Caption = String(Servers[Idx].aRules[i].Value).Trim();
        }
        if (!SameStrings(lbRules->Items, NewServs)) {
            lbRules->Items->Assign(NewServs);
            lbRules->TopIndex = TopIndexes[2];
        }

        edSIAddress->Text = String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port);
        lbSIPlayers->Caption = IntToStr(Servers[Idx].Players) + " / " + IntToStr(Servers[Idx].MaxPlayers);
        lbSIPing->Caption = (Servers[Idx].Ping == 9999) ? "-" : IntToStr(Servers[Idx].Ping);
        lbSIMode->Caption = String(Servers[Idx].Mode);
        lbSIMap->Caption = String(Servers[Idx].Map);
        gbInfo->Caption = " Server Info: " + String(Servers[Idx].HostName) + " ";
    } else {
        lbPlayers->Clear();
        lbRules->Clear();
        label_url->Caption = "";
        edSIAddress->Text = "- - -";
        lbSIPlayers->Caption = "- - -";
        lbSIPing->Caption = "- - -";
        lbSIMode->Caption = "- - -";
        lbSIMap->Caption = "- - -";
        gbInfo->Caption = " Server Info ";
    }
    delete NewServs;

    TotServers = lbServers->Items->Count;
    TotSlots = 0;
    TotPlayers = 0;
    for (i = 0; i < lbServers->Items->Count; i++) {
        Idx = StrToIntDef(lbServers->Items->Strings[i], -1);
        if (Idx >= 0 && Idx < (int)Servers.size()) {
            TotSlots += Servers[Idx].MaxPlayers;
            TotPlayers += Servers[Idx].Players;
        }
    }
    sbMain->SimpleText = "Servers: " + IntToStr(TotPlayers) + " players, playing on " + IntToStr(TotServers) + " servers. (" + IntToStr(TotSlots) + " player slots available)";

    if (TopIndexSaved >= 0 && TopIndexSaved < lbServers->Items->Count)
        lbServers->TopIndex = TopIndexSaved;

    if (TrackingChanges)
        lbServers->Items->EndUpdate();

}

void __fastcall TfmMain::AddServer(String Server)
{
    Server = Server.Trim();
    String address;
    int port = 7777;
    int colonPos = Server.Pos(":");
    if (colonPos > 0) {
        address = Server.SubString(1, colonPos - 1).Trim();
        String portText = Server.SubString(
            colonPos + 1, Server.Length() - colonPos).Trim();
        port = StrToIntDef(portText, -1);
    } else {
        address = Server;
    }

    if (!IsValidServerEndpoint(address, port))
    {
        MessageDlg("Invalid server address or port.", mtError,
                   TMsgDlgButtons() << mbOK, 0);
        return;
    }

    int i = (int)Servers.size();
    Servers.resize(i + 1);
    Servers[i].Address = AnsiString(address);
    Servers[i].Port = port;
    Servers[i].HostName = AnsiString("(Retrieving info...) " + String(Servers[i].Address) + ":" + IntToStr(Servers[i].Port));
    Servers[i].Ping = 9999;
    Servers[i].Tag = (WORD)random(0xFFFF);

    bool Dupe = false;
    std::string endpoint = EndpointKey(Servers[i].Address, Servers[i].Port);
    for (int j = 0; j < i; j++) {
        if (EndpointKey(Servers[j].Address, Servers[j].Port) == endpoint) {
            Servers.resize(i);
            MessageDlg("This server is already on your list.", mtError, TMsgDlgButtons() << mbOK, 0);
            Dupe = true;
            break;
        }
    }
    if (!Dupe) {
        EnqueueQuery(address + ":" + IntToStr(port) + "#" +
                     IntToStr(Servers[i].Tag));
        FavoritesChanged = !ExportFavorites(GetUserDataFileName(), true);
    }
    ServerOrderDirty = true;
    UpdateServers();
}

void __fastcall TfmMain::FilterChange(TObject *Sender)
{
    UpdateServers();
}

void __fastcall TfmMain::lbServersClick(TObject *Sender)
{
    std::string selectedEndpoint;
    if (lbServers->ItemIndex >= 0 &&
        lbServers->ItemIndex < lbServers->Items->Count) {
        int selectedIdx = StrToIntDef(
            lbServers->Items->Strings[lbServers->ItemIndex], -1);
        if (selectedIdx >= 0 && selectedIdx < (int)Servers.size())
            selectedEndpoint = EndpointKey(
                Servers[selectedIdx].Address, Servers[selectedIdx].Port);
    }
    if (selectedEndpoint != PingChartEndpoint) {
        PingChartEndpoint = selectedEndpoint;
        chSIPingChart->Series[0]->Clear();
    }
    SelServer = lbServers->ItemIndex;
    lbPlayers->Clear();
    lbRules->Clear();

    bool Enabled = (lbServers->ItemIndex != -1);

    tbDeleteServer->Enabled = (MasterFile == 0);
    miDeleteServer->Enabled = tbDeleteServer->Enabled;

    tbConnect->Enabled = Enabled;
    miConnect->Enabled = Enabled;
    tbRefreshServer->Enabled = Enabled;
    miRefreshServer->Enabled = Enabled;
    tbCopyServerInfo->Enabled = Enabled;
    miCopyServerInfo->Enabled = Enabled;
    tbServerProperties->Enabled = Enabled;
    miServerProperties->Enabled = Enabled;

    if (lbServers->ItemIndex == -1) {
        edSIAddress->Text = "- - -";
        lbSIPlayers->Caption = "- - -";
        lbSIPing->Caption = "- - -";
        lbSIMode->Caption = "- - -";
        lbSIMap->Caption = "- - -";
        gbInfo->Caption = " Server Info ";
        label_url->Caption = "";
        return;
    }

    RefreshServerClick(Sender);
}

void __fastcall TfmMain::lbServersContextPopup(TObject *Sender, TPoint &MousePos, bool &Handled)
{
    Handled = (lbServers->ItemIndex == -1);
}

void __fastcall TfmMain::lbServersDrawItem(TWinControl *Control, int Index, TRect &Rect, TOwnerDrawState State)
{
    TListBox *lb = static_cast<TListBox*>(Control);
    if (Index < 0 || Index >= lb->Items->Count)
        return;
    int Idx = StrToIntDef(lb->Items->Strings[Index], -1);
    if (Idx < 0 || Idx >= (int)Servers.size()) return;

    lb->Canvas->Pen->Color = clBtnHighlight;
    lb->Canvas->Pen->Style = psClear;

    if (State.Contains(odSelected)) {
        lb->Canvas->Font->Color = clHighlightText;
        lb->Canvas->Brush->Color = clHighlight;
    } else {
        lb->Canvas->Font->Color = clWindowText;
        lb->Canvas->Brush->Color = (Index % 2) ? clWindow : DarkenColor(clWindow, 10);
    }

    Rect.Right++;
    lb->Canvas->Rectangle(Rect);
    Rect.Right--;
    lb->Canvas->Pen->Style = psSolid;
    lb->Canvas->MoveTo(Rect.Right, Rect.Bottom - 1);
    lb->Canvas->LineTo(Rect.Left, Rect.Bottom - 1);

    for (int i = 0; i < hcServers->Sections->Count; i++) {
        lb->Canvas->MoveTo(hcServers->Sections->Items[i]->Right - 1, Rect.Top);
        lb->Canvas->LineTo(hcServers->Sections->Items[i]->Right - 1, Rect.Bottom);
    }

    VirtualImageList1->Draw(lb->Canvas, 7, Rect.Top + 1, Servers[Idx].Passworded ? L"imPadlocked" : L"imPadlock");

    TRect TempRect;
    TempRect = TRect(hcServers->Sections->Items[1]->Left + 2, Rect.Top + 2, hcServers->Sections->Items[1]->Right - 2, Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].HostName).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcServers->Sections->Items[2]->Left + 2, Rect.Top + 2, hcServers->Sections->Items[2]->Right - 2, Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, (IntToStr(Servers[Idx].Players) + " / " + IntToStr(Servers[Idx].MaxPlayers)).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcServers->Sections->Items[3]->Left + 2, Rect.Top + 2, hcServers->Sections->Items[3]->Right - 2, Rect.Bottom - 2);
    if (Servers[Idx].Ping == 9999)
        DrawText(lb->Canvas->Handle, L"-", -1, &TempRect, DT_LEFT);
    else
        DrawText(lb->Canvas->Handle, IntToStr(Servers[Idx].Ping).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcServers->Sections->Items[4]->Left + 2, Rect.Top + 2, hcServers->Sections->Items[4]->Right - 2, Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].Mode).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcServers->Sections->Items[5]->Left + 2, Rect.Top + 2, hcServers->Sections->Items[5]->Right - 2, Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].Map).c_str(), -1, &TempRect, DT_LEFT);
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::hcServersSectionResize(THeaderControl *, THeaderSection *)
{
    lbServers->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::hcServersSectionClick(THeaderControl *, THeaderSection *Section)
{
    switch (Section->Index) {
        case 1:
            if (SortMode == TSortMode::smHostName)
                SortDir = (SortDir == TSortDir::sdUp) ? TSortDir::sdDown : TSortDir::sdUp;
            else {
                SortMode = TSortMode::smHostName;
                SortDir = TSortDir::sdUp;
            }
            break;
        case 2:
            if (SortMode == TSortMode::smPlayers)
                SortDir = (SortDir == TSortDir::sdUp) ? TSortDir::sdDown : TSortDir::sdUp;
            else {
                SortMode = TSortMode::smPlayers;
                SortDir = TSortDir::sdDown;
            }
            break;
        case 3:
            if (SortMode == TSortMode::smPing)
                SortDir = (SortDir == TSortDir::sdUp) ? TSortDir::sdDown : TSortDir::sdUp;
            else {
                SortMode = TSortMode::smPing;
                SortDir = TSortDir::sdUp;
            }
            break;
        case 4:
            if (SortMode == TSortMode::smMode)
                SortDir = (SortDir == TSortDir::sdUp) ? TSortDir::sdDown : TSortDir::sdUp;
            else {
                SortMode = TSortMode::smMode;
                SortDir = TSortDir::sdUp;
            }
            break;
        case 5:
            if (SortMode == TSortMode::smMap)
                SortDir = (SortDir == TSortDir::sdUp) ? TSortDir::sdDown : TSortDir::sdUp;
            else {
                SortMode = TSortMode::smMap;
                SortDir = TSortDir::sdUp;
            }
            break;
    }
    UpdateServers();
}

static bool ReRender = false;

void __fastcall TfmMain::hcServersDrawSection(THeaderControl *HeaderControl, THeaderSection *Section, const TRect &Rect, bool Pressed)
{
    bool DoIt = false;
    TRect TempRect = Rect;
    TempRect.Left += 2;
    TempRect.Top += 1;

    if (Section->Index == 1 && SortMode == TSortMode::smHostName) DoIt = true;
    else if (Section->Index == 2 && SortMode == TSortMode::smPlayers) DoIt = true;
    else if (Section->Index == 3 && SortMode == TSortMode::smPing) DoIt = true;
    else if (Section->Index == 4 && SortMode == TSortMode::smMode) DoIt = true;
    else if (Section->Index == 5 && SortMode == TSortMode::smMap) DoIt = true;

    if (DoIt) {
        UnicodeString arrowName = (SortDir == TSortDir::sdDown) ? L"imDownArrow" : L"imUpArrow";
        VirtualImageList1->Draw(HeaderControl->Canvas, Rect.Left + 2, Rect.Top + 2, arrowName);
        TempRect.Left += 10;
    }

    DrawText(HeaderControl->Canvas->Handle, Section->Text.c_str(), -1, &TempRect, DT_LEFT);

    if (!ReRender) {
        ReRender = true;
        HeaderControl->Repaint();
        ReRender = false;
    }
}

void __fastcall TfmMain::tbMainResize(TObject *Sender)
{
    ToolButton1->Width = ((TToolBar*)Sender)->Width - ToolButton1->Left - imLogo->Width;
    imLogo->Repaint();
}

void __fastcall TfmMain::pnBreakableResize(TObject *Sender)
{
    gbInfo->Width = pnBreakable->Width - gbFilter->Width + 1;

    chSIPingChart->Width =
        gbInfo->ClientWidth - chSIPingChart->Left - 16;

    chSIPingChart->Visible = (chSIPingChart->Width > 50);
}

void __fastcall TfmMain::lbPlayersDrawItem(TWinControl *Control, int Index, TRect &Rect, TOwnerDrawState State)
{
    if (lbServers->ItemIndex == -1) return;
    TListBox *lb = static_cast<TListBox*>(Control);
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size()) return;
    if (Index < 0 || Index >= (int)Servers[Idx].aPlayers.size()) return;

    lb->Canvas->Pen->Color = clBtnHighlight;
    lb->Canvas->Pen->Style = psClear;

    if (State.Contains(odSelected)) {
        lb->Canvas->Font->Color = clHighlightText;
        lb->Canvas->Brush->Color = clHighlight;
    } else {
        lb->Canvas->Font->Color = clWindowText;
        lb->Canvas->Brush->Color = (Index % 2) ? clWindow : DarkenColor(clWindow, 10);
    }

    Rect.Right++;
    lb->Canvas->Rectangle(Rect);
    Rect.Right--;
    lb->Canvas->Pen->Style = psSolid;
    lb->Canvas->MoveTo(Rect.Right, Rect.Bottom - 1);
    lb->Canvas->LineTo(Rect.Left, Rect.Bottom - 1);

    lb->Canvas->MoveTo(hcPlayers->Sections->Items[0]->Right - 2, Rect.Top);
    lb->Canvas->LineTo(hcPlayers->Sections->Items[0]->Right - 2, Rect.Bottom);
    lb->Canvas->MoveTo(hcPlayers->Sections->Items[1]->Right - 2, Rect.Top);
    lb->Canvas->LineTo(hcPlayers->Sections->Items[1]->Right - 2, Rect.Bottom);

    TRect TempRect;
    TempRect = TRect(hcPlayers->Sections->Items[0]->Left + 2, Rect.Top + 2, hcPlayers->Sections->Items[0]->Right - 2, Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].aPlayers[Index].Name).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcPlayers->Sections->Items[1]->Left + 2, Rect.Top + 2, hcPlayers->Sections->Items[1]->Right - 2, Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, IntToStr(Servers[Idx].aPlayers[Index].Score).c_str(), -1, &TempRect, DT_LEFT);
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::hcPlayersSectionResize(THeaderControl *, THeaderSection *)
{
    lbPlayers->Repaint();
}

void __fastcall TfmMain::lbRulesDrawItem(TWinControl *Control, int Index, TRect &Rect, TOwnerDrawState State)
{
    if (lbServers->ItemIndex == -1) return;
    TListBox *lb = static_cast<TListBox*>(Control);
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size()) return;
    if (Index < 0 || Index >= (int)Servers[Idx].aRules.size()) return;

    lb->Canvas->Pen->Color = clBtnHighlight;
    lb->Canvas->Pen->Style = psClear;

    if (State.Contains(odSelected)) {
        lb->Canvas->Font->Color = clHighlightText;
        lb->Canvas->Brush->Color = clHighlight;
    } else {
        lb->Canvas->Font->Color = clWindowText;
        lb->Canvas->Brush->Color = (Index % 2) ? clWindow : DarkenColor(clWindow, 10);
    }

    Rect.Right++;
    lb->Canvas->Rectangle(Rect);
    Rect.Right--;
    lb->Canvas->Pen->Style = psSolid;
    lb->Canvas->MoveTo(Rect.Right, Rect.Bottom - 1);
    lb->Canvas->LineTo(Rect.Left, Rect.Bottom - 1);

    lb->Canvas->MoveTo(hcRules->Sections->Items[0]->Right - 2, Rect.Top);
    lb->Canvas->LineTo(hcRules->Sections->Items[0]->Right - 2, Rect.Bottom);
    lb->Canvas->MoveTo(hcRules->Sections->Items[1]->Right - 2, Rect.Top);
    lb->Canvas->LineTo(hcRules->Sections->Items[1]->Right - 2, Rect.Bottom);

    TRect TempRect;
    TempRect = TRect(hcRules->Sections->Items[0]->Left + 2, Rect.Top + 2, hcRules->Sections->Items[0]->Right - 2, Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].aRules[Index].Rule).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcRules->Sections->Items[1]->Left + 2, Rect.Top + 2, hcRules->Sections->Items[1]->Right - 2, Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].aRules[Index].Value).c_str(), -1, &TempRect, DT_LEFT);
}

void __fastcall TfmMain::hcRulesSectionResize(THeaderControl *, THeaderSection *)
{
    lbRules->Repaint();
}

void __fastcall TfmMain::lbPlayersExit(TObject *Sender)
{
    lbPlayers->ItemIndex = -1;
}

void __fastcall TfmMain::lbRulesExit(TObject *Sender)
{
    lbRules->ItemIndex = -1;
}

void __fastcall TfmMain::sbMainDrawPanel(TStatusBar *StatusBar, TStatusPanel *Panel, const TRect &Rect)
{
    StatusBar->Canvas->Brush->Color = clBtnFace;
    StatusBar->Canvas->Rectangle(Rect);

    StatusBar->Canvas->Brush->Color = (TColor)0x00804000;
    int Pcnt = (int)(((Rect.Width()) / 100.0) * 75.0);
    StatusBar->Canvas->Rectangle(Rect.Left, Rect.Top, Rect.Left + Pcnt, Rect.Bottom);
}

static void DispatchQueryPacket(const sockaddr_in &Address,
                                const char *Data, int Length)
{
    if (QueryNetworkThread)
        QueryNetworkThread->Queue(Address, Data, Length);
    else if (QuerySocket != INVALID_SOCKET)
        sendto(QuerySocket, Data, Length, 0,
               reinterpret_cast<const sockaddr*>(&Address), sizeof(Address));
}

void __fastcall TfmMain::QueryServerInfo(String Server, bool bPing, bool bInfo, bool bPlayers, bool bRules)
{
    if (QuerySocket == INVALID_SOCKET)
        return;
    AnsiString host;
    WORD port;
    WORD tag = 0;

    int colonPos = Server.Pos(":");
    int tagPos = Server.Pos("#");

    if (colonPos > 0) {
        if (tagPos > 0) {
            host = AnsiString(Server.SubString(1, colonPos - 1));
            port = (WORD)StrToIntDef(Server.SubString(colonPos + 1, tagPos - colonPos - 1), 7777);
            tag = (WORD)StrToIntDef(Server.SubString(tagPos + 1, Server.Length() - tagPos), 0);
        } else {
            host = AnsiString(Server.SubString(1, colonPos - 1));
            port = (WORD)StrToIntDef(Server.SubString(colonPos + 1, 5), 7777);
        }
    } else {
        host = AnsiString(Server);
        port = 7777;
    }

    if (tag == 0) tag = port;

    String hostName = String(host);
    AnsiString ip = AnsiString(GetCachedIPFromHost(hostName));
    if (ip.IsEmpty()) {
        if (HasCachedHost(hostName))
            return;

        TDnsQuerySpec query = {};
        query.Server = Server;
        query.Ping = bPing;
        query.Info = bInfo;
        query.Players = bPlayers;
        query.Rules = bRules;
        if (!QueueDnsRequest(Handle, host, query))
            CacheHost(hostName, "");
        return;
    }
    if (ip.Length() < 7 || ip.Length() > 15)
        return;

    sockaddr_in toAddr = {};
    toAddr.sin_family      = AF_INET;
    toAddr.sin_port        = htons(port);
    toAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    BYTE buf[15] = {};
    buf[0] = 'S';
    buf[1] = 'A';
    buf[2] = 'M';
    buf[3] = 'P';

    buf[4] = (BYTE)StrToIntDef(AnsiString(GetToken(String(ip), 1, ".")), 0);
    buf[5] = (BYTE)StrToIntDef(AnsiString(GetToken(String(ip), 2, ".")), 0);
    buf[6] = (BYTE)StrToIntDef(AnsiString(GetToken(String(ip), 3, ".")), 0);
    buf[7] = (BYTE)StrToIntDef(AnsiString(GetToken(String(ip), 4, ".")), 0);

    memcpy(&buf[8], &tag, 2);

    int toLen = sizeof(toAddr);

    if (bInfo) {
        buf[10] = 'i';
        DispatchQueryPacket(toAddr, reinterpret_cast<char*>(buf), 11);
    }

    if (bPing) {
        buf[10] = 'p';
        DWORD ticks = timeGetTime();
        memcpy(&buf[11], &ticks, 4);
        DispatchQueryPacket(toAddr, reinterpret_cast<char*>(buf), 15);
    }

    if (bPlayers) {
        buf[10] = 'c';
        DispatchQueryPacket(toAddr, reinterpret_cast<char*>(buf), 11);
    }

    if (bRules) {
        buf[10] = 'r';
        DispatchQueryPacket(toAddr, reinterpret_cast<char*>(buf), 11);
    }
}

void __fastcall TfmMain::QueryServerInfoParse(String SrcIP, WORD SrcPort, char *Buf, int DataLen)
{
    if (DataLen < 11) return;
    if (memcmp(Buf, "SAMP", 4) != 0) return;

    // IP from package (bytes 4-7)
    AnsiString pktIP;
    pktIP.sprintf("%d.%d.%d.%d",
        (BYTE)Buf[4], (BYTE)Buf[5], (BYTE)Buf[6], (BYTE)Buf[7]);

    WORD pktPort;
    memcpy(&pktPort, &Buf[8], 2);

    // SrcIP compare with IP from package
    if (AnsiString(SrcIP) != pktIP) return;

    std::map<std::string, int>::const_iterator found =
        ServerLookup.find(ServerKey(AnsiString(SrcIP), SrcPort, pktPort));
    if (found == ServerLookup.end())
        return;
    int idx = found->second;
    if (idx < 0 || idx >= (int)Servers.size())
        return;

    bool repaintServers = false;
    bool repaintPlayers = false;
    bool repaintRules   = false;
    bool rebuildServers = false;

    switch (Buf[10])
    {
    case 'p': // Ping
        if (DataLen == 15) {
            DWORD pingToken;
            memcpy(&pingToken, &Buf[11], 4);
            int measuredPing = 0;
            if (QueryNetworkThread) {
                DWORD sourceAddress = inet_addr(AnsiString(SrcIP).c_str());
                if (!QueryNetworkThread->TakePingMilliseconds(
                        pingToken, sourceAddress, SrcPort, measuredPing))
                    break;
            } else {
                measuredPing = (int)(timeGetTime() - pingToken);
                if (measuredPing < 1)
                    measuredPing = 1;
            }
            Servers[idx].Ping = measuredPing;

            int selectedIdx = -1;
            if (lbServers->ItemIndex >= 0 &&
                lbServers->ItemIndex < lbServers->Items->Count)
                selectedIdx = StrToIntDef(
                    lbServers->Items->Strings[lbServers->ItemIndex], -1);

            if (selectedIdx == idx) {
                double val = (double)Servers[idx].Ping;

                int i = chSIPingChart->Series[0]->AddY(val, "", clBlue);
                if (i > 60) {
                    for (int j = 1; j <= 61; j++)
                        chSIPingChart->Series[0]->YValue[j - 1] =
                            chSIPingChart->Series[0]->YValue[j];
                    chSIPingChart->Series[0]->Delete(61);
                }

                int peak = (int)(
                    chSIPingChart->Series[0]->MaxYValue() + 0.999);
                int target = peak * 5 / 4;
                if (target < 50)
                    target = 50;
                int step = target <= 100 ? 10 :
                           target <= 250 ? 25 :
                           target <= 500 ? 50 : 100;
                chSIPingChart->LeftAxis->Maximum =
                    ((target + step - 1) / step) * step;
            }

            repaintServers = true;
            rebuildServers = SortMode == TSortMode::smPing;
            Servers[idx].QueryPingReceived = true;
            if (selectedIdx == idx)
                lbSIPing->Caption = IntToStr(Servers[idx].Ping);
        }
        break;

    case 'i': // Info
        {
            int bufPos = 11;
            BYTE  bPassworded;
            WORD  wPlayers, wMaxPlayers;
            if (bufPos + 1 > DataLen)
                break;
            memcpy(&bPassworded, &Buf[bufPos], 1); bufPos += 1;

            if (bufPos + 2 > DataLen)
                break;
            memcpy(&wPlayers, &Buf[bufPos], 2); bufPos += 2;
            wPlayers = (wPlayers > 1000) ? 1000 : wPlayers;

            if (bufPos + 2 > DataLen)
                break;
            memcpy(&wMaxPlayers, &Buf[bufPos], 2); bufPos += 2;
            wMaxPlayers = (wMaxPlayers > 1000) ? 1000 : wMaxPlayers;

            AnsiString fields[3];
            const DWORD maxLengths[3] = { 63, 39, 39 };
            bool valid = true;
            for (int field = 0; field < 3; field++) {
                DWORD length = 0;
                if (bufPos + 4 > DataLen) {
                    valid = false;
                    break;
                }
                memcpy(&length, &Buf[bufPos], 4);
                bufPos += 4;
                if (length > maxLengths[field] ||
                    length > (DWORD)(DataLen - bufPos))
                {
                    valid = false;
                    break;
                }
                fields[field] = length > 0
                    ? AnsiString(Buf + bufPos, length) : AnsiString("-");
                bufPos += length;
            }
            if (!valid)
                break;

            Servers[idx].Passworded = (bPassworded != 0);
            Servers[idx].MaxPlayers = (int)wMaxPlayers;
            Servers[idx].Players = (wPlayers > wMaxPlayers)
                ? (int)wMaxPlayers : (int)wPlayers;
            Servers[idx].HostName = fields[0];
            Servers[idx].Mode = fields[1];
            Servers[idx].Map = fields[2];
            Servers[idx].QueryInfoReceived = true;

            repaintServers = true;
            rebuildServers = true;
            QueryServerInfo(String(Servers[idx].Address) + ":" + IntToStr(Servers[idx].Port) + "#" + IntToStr(Servers[idx].Tag), true, false, false, false);
        }
        break;

    case 'c': // Players
        {
            int bufPos = 11;
            if (bufPos + 2 > DataLen) break;
            WORD wPlayers;
            memcpy(&wPlayers, &Buf[bufPos], 2); bufPos += 2;
            if (wPlayers > 100) wPlayers = 100;
            std::vector<TPlayerInfo> players;
            players.reserve(wPlayers);
            bool valid = true;

            for (int i = 0; i < (int)wPlayers; i++) {
                if (bufPos >= DataLen) { valid = false; break; }
                BYTE nameLen;
                memcpy(&nameLen, &Buf[bufPos], 1); bufPos += 1;
                if (bufPos + nameLen > DataLen) { valid = false; break; }
                TPlayerInfo player;
                player.Name = AnsiString(Buf + bufPos, nameLen);
                bufPos += nameLen;

                if (bufPos + 4 > DataLen) { valid = false; break; }
                int score;
                memcpy(&score, &Buf[bufPos], 4); bufPos += 4;
                if (score > 1000000) score = 1000000;
                if (score < 0)       score = 0;
                player.Score = score;
                players.push_back(player);
            }
            if (valid) {
                int parsedPlayerCount = (int)players.size();
                Servers[idx].aPlayers.swap(players);
                Servers[idx].Players = parsedPlayerCount;
                repaintPlayers = true;
                rebuildServers = true;
            }
        }
        break;

    case 'r': // Rules
        {
            int bufPos = 11;
            if (bufPos + 2 > DataLen) break;
            WORD wRules;
            memcpy(&wRules, &Buf[bufPos], 2); bufPos += 2;
            if (wRules > 30) wRules = 30;
            std::vector<TRuleInfo> rules;
            rules.reserve(wRules);
            bool valid = true;

            for (int i = 0; i < (int)wRules; i++) {
                if (bufPos >= DataLen) { valid = false; break; }
                BYTE len;
                memcpy(&len, &Buf[bufPos], 1); bufPos += 1;
                if (bufPos + len > DataLen) { valid = false; break; }
                TRuleInfo rule;
                rule.Rule = AnsiString(Buf + bufPos, len);
                bufPos += len;

                if (bufPos >= DataLen) { valid = false; break; }
                memcpy(&len, &Buf[bufPos], 1); bufPos += 1;
                if (bufPos + len > DataLen) { valid = false; break; }
                rule.Value = AnsiString(Buf + bufPos, len);
                bufPos += len;
                rules.push_back(rule);
            }
            if (valid) {
                Servers[idx].aRules.swap(rules);
                repaintRules = true;
            }
        }
        break;
    }

    if (MasterQueryBatchActive) {
        if (Servers[idx].QueryInfoReceived &&
            Servers[idx].QueryPingReceived &&
            !Servers[idx].QueryCompleted)
        {
            Servers[idx].QueryCompleted = true;
            if (MasterQueryPending > 0)
                MasterQueryPending--;
        }
        CheckMasterQueryBatchComplete();
    } else {
        if (rebuildServers) {
            ServerOrderDirty = true;
            ServerListDirty = true;
        }

        if (repaintServers) lbServers->Invalidate();
        if (repaintPlayers) lbPlayers->Invalidate();
        if (repaintRules)   lbRules->Invalidate();
    }
}

void __fastcall TfmMain::QueryServerInfoError(int SocketError)
{
    wchar_t err[512] = {};
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr,
                   (DWORD)SocketError, 0, err, 512, nullptr);
    MessageDlg(String(err), mtError, TMsgDlgButtons() << mbOK, 0);
}

void __fastcall TfmMain::WMRecv(TMessage &Message)
{
    int socketError = WSAGETSELECTERROR(Message.LParam);
    if (socketError != 0) {
        if (socketError != WSAEWOULDBLOCK)
            QueryServerInfoError(socketError);
        return;
    }
    if (WSAGETSELECTEVENT(Message.LParam) != FD_READ)
        return;

    char lpBuffer[2049]; // 2048 + 1
    sockaddr_in fromAddr = {};
    int fromLen = sizeof(fromAddr);

    ZeroMemory(lpBuffer, sizeof(lpBuffer));
    int bufLen = recvfrom(QuerySocket, lpBuffer, 2048, 0,
                          (sockaddr*)&fromAddr, &fromLen);

    while (bufLen > 0) {
        AnsiString srcIP  = AnsiString(inet_ntoa(fromAddr.sin_addr));
        WORD       srcPort = ntohs(fromAddr.sin_port);

        QueryServerInfoParse(String(srcIP), srcPort, lpBuffer, bufLen);

        ZeroMemory(lpBuffer, sizeof(lpBuffer));
        ZeroMemory(&fromAddr, sizeof(fromAddr));
        fromAddr.sin_family = AF_INET;
        fromLen = sizeof(fromAddr);

        bufLen = recvfrom(QuerySocket, lpBuffer, 2048, 0,
                          (sockaddr*)&fromAddr, &fromLen);
    }
}

void __fastcall TfmMain::WMQueryBatchReady(TMessage &Message)
{
    if (!QueryNetworkThread)
        return;
    std::vector<TReceivedQueryPacket> packets;
    QueryNetworkThread->TakeReceived(packets);
    for (int i = 0; i < (int)packets.size(); i++) {
        in_addr address = {};
        address.s_addr = packets[i].Address;
        AnsiString sourceIP = inet_ntoa(address);
        QueryServerInfoParse(String(sourceIP), packets[i].Port,
                             packets[i].Data, packets[i].Length);
    }
}

void __fastcall TGameLaunchThread::Execute()
{
    STARTUPINFOW si = {};
    PROCESS_INFORMATION pi = {};
    LPVOID remotePath = nullptr;
    HANDLE remoteThread = nullptr;
    HMODULE kernelModule = nullptr;
    FARPROC loadLibraryW = nullptr;
    SIZE_T bytesWritten = 0;
    DWORD threadId = 0;
    DWORD exitCode = 0;
    bool processCreated = false;
    bool remoteCompleted = false;
    std::vector<wchar_t> mutableCommandLine(
        static_cast<size_t>(FCommandLine.Length()) + 1);
    memcpy(&mutableCommandLine[0], FCommandLine.c_str(),
           mutableCommandLine.size() * sizeof(wchar_t));
    si.cb = sizeof(si);

    if (Terminated)
        goto Finish;

    if (!CreateProcessW(FGameExe.c_str(), &mutableCommandLine[0], nullptr,
            nullptr, FALSE, CREATE_NEW_PROCESS_GROUP |
                NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED,
            nullptr, FWorkDir.c_str(), &si, &pi)) {
        Error = TGameLaunchError::Execute;
        goto Finish;
    }
    processCreated = true;
    if (Terminated)
        goto Finish;

    {
        SIZE_T dllBytes =
            (static_cast<SIZE_T>(FSampDll.Length()) + 1) * sizeof(wchar_t);
        remotePath = VirtualAllocEx(pi.hProcess, nullptr, dllBytes,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!remotePath) {
            Error = TGameLaunchError::Allocate;
            goto Finish;
        }
        if (!WriteProcessMemory(pi.hProcess, remotePath, FSampDll.c_str(),
                dllBytes, &bytesWritten) || bytesWritten != dllBytes) {
            Error = TGameLaunchError::WritePath;
            goto Finish;
        }
    }
    if (Terminated)
        goto Finish;
    kernelModule = GetModuleHandleW(L"kernel32.dll");
    loadLibraryW = kernelModule
        ? GetProcAddress(kernelModule, "LoadLibraryW") : nullptr;
    remoteThread = loadLibraryW
        ? CreateRemoteThread(pi.hProcess, nullptr, 0,
              reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryW),
              remotePath, 0, &threadId)
        : nullptr;
    if (!remoteThread) {
        Error = TGameLaunchError::CreateRemoteThread;
        goto Finish;
    }

    {
        DWORD waitStarted = timeGetTime();
        while (!Terminated) {
            DWORD waitResult = WaitForSingleObject(remoteThread, 50);
            if (waitResult == WAIT_OBJECT_0) {
                remoteCompleted = true;
                break;
            }
            if (waitResult == WAIT_FAILED ||
                timeGetTime() - waitStarted >= 10000)
                break;
        }
    }

    if (!remoteCompleted ||
        !GetExitCodeThread(remoteThread, &exitCode) || exitCode == 0) {
        if (!Terminated)
            Error = TGameLaunchError::LoadLibrary;
        goto Finish;
    }
    if (Terminated)
        goto Finish;
    if (ResumeThread(pi.hThread) == (DWORD)-1) {
        Error = TGameLaunchError::Resume;
        goto Finish;
    }
    processCreated = false;

Finish:
    if (processCreated) {
        TerminateProcess(pi.hProcess, 0);
        WaitForSingleObject(pi.hProcess, 5000);
    }
    if (remoteThread)
        CloseHandle(remoteThread);
    if (remotePath)
        VirtualFreeEx(pi.hProcess, remotePath, 0, MEM_RELEASE);
    if (pi.hThread)
        CloseHandle(pi.hThread);
    if (pi.hProcess)
        CloseHandle(pi.hProcess);
    if (!Terminated)
        PostMessage(FNotifyWindow, WM_GAME_LAUNCH_COMPLETE,
                    static_cast<WPARAM>(Error), 0);
}

TGameLaunchResult __fastcall TfmMain::ServerConnect(String Server, String Port, String Password)
{
    if (GameLaunchShuttingDown)
        return TGameLaunchResult::Failed;
    if (GameLaunchThread || GameLaunchDnsPending)
        return TGameLaunchResult::Started;
    if (!FileExists(gta_sa_exe)) {
        MessageDlg("GTA: San Andreas executable not found.\n(" + gta_sa_exe + ")\n\nPlease locate it now.", mtError, TMsgDlgButtons() << mbOK, 0);
        GetGTAExe(Handle);
    }
    if (!FileExists(gta_sa_exe)) {
        MessageDlg("GTA: San Andreas executable STILL not found.\n(" + gta_sa_exe + ")\n\nAborting launch.", mtError, TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    String gameExe = GetAbsolutePath(gta_sa_exe);
    gta_sa_exe = gameExe;
    String sampDll = GetAbsolutePath(ExtractFilePath(gameExe) + "samp.dll");
    if (!FileExists(sampDll)) {
        MessageDlg("SA-MP library not found.\n(" + sampDll + ")", mtError, TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    if (HasUnsafeCommandLineCharacters(edName->Text) ||
        HasUnsafeCommandLineCharacters(Password))
    {
        MessageDlg("Nickname and password cannot contain spaces, quotes, or control characters.",
                   mtError, TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    String serverIP = GetCachedIPFromHost(Server);
    if (serverIP.IsEmpty()) {
        if (!HasCachedHost(Server)) {
            TDnsQuerySpec query = {};
            query.Server = Server;
            query.Port = Port;
            query.Password = Password;
            query.Connect = true;
            if (QueueDnsRequest(Handle, AnsiString(Server), query)) {
                GameLaunchDnsPending = true;
                return TGameLaunchResult::DnsPending;
            }
            CacheHost(Server, "");
        }
        MessageDlg("Unable to resolve the server address.", mtError, TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    String CmdLine = " -c -n " + edName->Text + " -h " + serverIP + " -p " + Port;
    if (!Password.IsEmpty())
        CmdLine += " -z " + Password;

    String cmd = "\"" + gta_sa_exe + "\"" + CmdLine;
    String workDir = ExtractFilePath(gta_sa_exe);
    try {
        GameLaunchThread = new TGameLaunchThread(
            Handle, gameExe, cmd, workDir, sampDll);
        GameLaunchThread->Start();
    }
    catch (...) {
        delete GameLaunchThread;
        GameLaunchThread = nullptr;
        MessageDlg("Unable to execute.", mtError, TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    return TGameLaunchResult::Started;
}

void __fastcall TfmMain::WMGameLaunchComplete(TMessage &Message)
{
    if (GameLaunchShuttingDown)
        return;

    TGameLaunchError error =
        static_cast<TGameLaunchError>(Message.WParam);
    TGameLaunchThread *thread = GameLaunchThread;
    GameLaunchThread = nullptr;
    if (thread) {
        thread->WaitFor();
        delete thread;
    }

    switch (error) {
        case TGameLaunchError::Execute:
            MessageDlg("Unable to execute.", mtError, TMsgDlgButtons() << mbOK, 0);
            break;
        case TGameLaunchError::Allocate:
            MessageDlg("Failed to allocate memory in target process.", mtError, TMsgDlgButtons() << mbOK, 0);
            break;
        case TGameLaunchError::WritePath:
            MessageDlg("Failed to write DLL path into target process.", mtError, TMsgDlgButtons() << mbOK, 0);
            break;
        case TGameLaunchError::CreateRemoteThread:
            MessageDlg("Failed to create remote thread.", mtError, TMsgDlgButtons() << mbOK, 0);
            break;
        case TGameLaunchError::LoadLibrary:
            MessageDlg("Failed to load SA-MP library into the game.", mtError, TMsgDlgButtons() << mbOK, 0);
            break;
        case TGameLaunchError::Resume:
            MessageDlg("Failed to start the game process.", mtError, TMsgDlgButtons() << mbOK, 0);
            break;
        default:
            break;
    }
}

void __fastcall TfmMain::WMDnsRecv(TMessage &Message)
{
    HANDLE task = (HANDLE)Message.WParam;
    int requestIndex = -1;
    for (int i = 0; i < (int)DnsRequests.size(); i++) {
        if (DnsRequests[i]->Task == task) {
            requestIndex = i;
            break;
        }
    }
    if (requestIndex < 0)
        return;

    TDnsRequest *request = DnsRequests[requestIndex];
    DnsRequests.erase(DnsRequests.begin() + requestIndex);
    String result;
    if (WSAGETASYNCERROR(Message.LParam) == 0) {
        hostent *entry = reinterpret_cast<hostent*>(request->Buffer);
        if (entry && entry->h_addr_list && entry->h_addr_list[0]) {
            in_addr address;
            memcpy(&address, entry->h_addr_list[0], sizeof(address));
            result = AnsiString(inet_ntoa(address));
        }
    }

    CacheHost(String(request->Host), result);
    RebuildServerLookup();

    std::vector<TDnsQuerySpec> queries;
    queries.swap(request->Queries);
    delete request;

    if (!result.IsEmpty()) {
        for (int i = 0; i < (int)queries.size(); i++) {
            if (queries[i].Connect) {
                GameLaunchDnsPending = false;
                ServerConnect(queries[i].Server, queries[i].Port, queries[i].Password);
            } else
                QueryServerInfo(queries[i].Server, queries[i].Ping, queries[i].Info,
                                queries[i].Players, queries[i].Rules);
        }
    } else {
        for (int i = 0; i < (int)queries.size(); i++) {
            if (queries[i].Connect) {
                GameLaunchDnsPending = false;
                MessageDlg("Unable to resolve the server address.", mtError,
                           TMsgDlgButtons() << mbOK, 0);
                break;
            }
        }
    }
}

void __fastcall TfmMain::tmSIPingUpdateTimer(TObject *Sender)
{
    if (lbServers->ItemIndex == -1) return;
    if (GetForegroundWindow() != Handle) return;

    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size()) return;

    bool PingOnly;
    if (PingCounter == 5) {
        PingCounter = 0;
        PingOnly = false;
    } else {
        PingCounter++;
        PingOnly = true;
    }

    String ServerAddr = String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port) + "#" + IntToStr(Servers[Idx].Tag);
    if (PingOnly)
        QueryServerInfo(ServerAddr, true, false, false, false);
    else
        QueryServerInfo(ServerAddr, true, true, true, true);
}

void __fastcall TfmMain::tmrQueryQueueProcessTimer(TObject *Sender)
{
    const int QUERIES_PER_TICK = 24;
    int sent = 0;
    while (PendingQueryCount() > 0 && sent < QUERIES_PER_TICK) {
        QueryServerInfo(QueryQueue->Strings[QueryQueuePosition],
                        false, true, false, false);
        QueryQueuePosition++;
        sent++;
    }
    if (PendingQueryCount() == 0 && QueryQueuePosition > 0)
        ClearQueryQueue();
    CheckMasterQueryBatchComplete();
}

void __fastcall TfmMain::tmServerListUpdate(TObject *Sender)
{
    if (MasterUpdateThread && MasterUpdateThreadFinished()) {
        MasterUpdateThread->WaitFor();
        delete MasterUpdateThread;
        MasterUpdateThread = nullptr;
    }

    if (MasterRefreshPending && !MasterUpdateInProgress &&
        MasterUpdateThread == nullptr && MasterFile != 0)
    {
        MasterRefreshPending = false;
        MasterServerUpdateClick(this);
        return;
    }

    if (MasterQueryBatchActive) {
        CheckMasterQueryBatchComplete();
        return;
    }

    if (ServerListDirty) {
        ServerListDirty = false;
        UpdateServers();
    }
}

// Favorites I/O
void __fastcall TfmMain::ImportFavoritesClick(TObject *Sender)
{
    if (tsServerLists->TabIndex != 0)
        tsServerLists->TabIndex = 0;

    TOpenDialog *OD = new TOpenDialog(this);
    OD->DefaultExt = "fav";
    OD->Filter = "SA-MP Favorites List (*.fav)|*.fav";
    OD->Options << ofEnableSizing << ofFileMustExist;
    OD->Title = "Import Favorites";
    if (!OD->Execute()) {
        delete OD;
        return;
    }

    TfmImportFavorites *fmImport = new TfmImportFavorites(Application);
    if (fmImport->ShowModal() != mrOk) {
        delete fmImport;
        delete OD;
        return;
    }
    bool AddToFavs = fmImport->rbAddToCurrent->Checked;
    delete fmImport;

    ImportFavorites(OD->FileName, AddToFavs);
    delete OD;

    FavoritesChanged = !ExportFavorites(GetUserDataFileName(), true);
}

void __fastcall TfmMain::ImportFavorites(String FileName, bool AddToFavs)
{
    HANDLE hFile = CreateFileW(FileName.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    char Tag[4];
    if (!ReadExact(hFile, Tag, sizeof(Tag))) {
        CloseHandle(hFile);
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }
    if (memcmp(Tag, FileTag, 4) != 0) {
        CloseHandle(hFile);
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    int Version;
    if (!ReadExact(hFile, &Version, sizeof(Version))) {
        CloseHandle(hFile);
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }
    if (Version != FAVORITES_FILE_VERSION) {
        CloseHandle(hFile);
        MessageDlg("Bad SA-MP favorites file version.\n\nYour client may need updating.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    int ServerCount;
    if (!ReadExact(hFile, &ServerCount, sizeof(ServerCount))) {
        CloseHandle(hFile);
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    LARGE_INTEGER fileSize = {};
    LARGE_INTEGER currentPosition = {};
    LARGE_INTEGER zero = {};
    if (ServerCount < 0 || ServerCount > MAX_FAVORITE_SERVERS ||
        !GetFileSizeEx(hFile, &fileSize) ||
        fileSize.QuadPart > MAX_FAVORITES_FILE_SIZE ||
        !SetFilePointerEx(hFile, zero, &currentPosition, FILE_CURRENT) ||
        fileSize.QuadPart - currentPosition.QuadPart < (LONGLONG)ServerCount * 20)
    {
        CloseHandle(hFile);
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    std::vector<TServerInfo> imported;
    imported.reserve(ServerCount);
    bool valid = true;

    for (int j = 0; j < ServerCount; j++) {
        TServerInfo server = {};
        if (!ReadFavoriteString(hFile, server.Address, MAX_FAVORITE_ADDRESS_LENGTH) ||
            !ReadExact(hFile, &server.Port, sizeof(server.Port)) ||
            !ReadFavoriteString(hFile, server.HostName, MAX_FAVORITE_HOSTNAME_LENGTH) ||
            !ReadFavoriteString(hFile, server.ServerPassword, MAX_FAVORITE_PASSWORD_LENGTH) ||
            !ReadFavoriteString(hFile, server.RconPassword, MAX_FAVORITE_PASSWORD_LENGTH))
        {
            valid = false;
            break;
        }
        if (!IsValidServerEndpoint(String(server.Address), server.Port)) {
            valid = false;
            break;
        }

        server.Ping = 9999;
        server.Tag = (WORD)random(0xFFFF);
        imported.push_back(server);
    }

    CloseHandle(hFile);
    if (!valid) {
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    if (!AddToFavs) {
        ClearQueryQueue();
        Servers.clear();
    }

    std::set<std::string> endpoints;
    for (int i = 0; i < (int)Servers.size(); i++)
        endpoints.insert(EndpointKey(Servers[i].Address, Servers[i].Port));

    for (int j = 0; j < (int)imported.size(); j++) {
        std::string endpoint =
            EndpointKey(imported[j].Address, imported[j].Port);
        if (endpoints.insert(endpoint).second) {
            Servers.push_back(imported[j]);
            int i = (int)Servers.size() - 1;
            EnqueueQuery(String(Servers[i].Address) + ":" + IntToStr(Servers[i].Port) + "#" + IntToStr(Servers[i].Tag));
        }
    }

    ServerOrderDirty = true;
    UpdateServers();
}

void __fastcall TfmMain::ExportFavoritesClick(TObject *Sender)
{
    if (tsServerLists->TabIndex != 0)
        tsServerLists->TabIndex = 0;

    TSaveDialog *SD = new TSaveDialog(this);
    SD->DefaultExt = "fav";
    SD->Filter = "SA-MP Favorites List (*.fav)|*.fav";
    SD->Options << ofHideReadOnly << ofEnableSizing;
    SD->Title = "Export Favorites";
    if (!SD->Execute()) {
        delete SD;
        return;
    }
    if (FileExists(SD->FileName)) {
        if (MessageDlg("File '" + SD->FileName + "' already exists. Overwrite?", mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, 0) != mrYes) {
            delete SD;
            return;
        }
    }

    TfmExportFavorites *fmExport = new TfmExportFavorites(Application);
    if (fmExport->ShowModal() != mrOk) {
        delete fmExport;
        delete SD;
        return;
    }
    bool ExportPasswords = fmExport->cbIncludeSavedPasswords->Checked;
    delete fmExport;

    ExportFavorites(SD->FileName, ExportPasswords);
    delete SD;
}

bool __fastcall TfmMain::ExportFavorites(String FileName, bool ExportPasswords)
{
    if (FileName.IsEmpty())
        return false;

    String tempFileName = FileName + ".tmp";
    HANDLE hFile = CreateFileW(tempFileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageDlg("Unable to save SA-MP favorites file.", mtError,
                   TMsgDlgButtons() << mbOK, 0);
        return false;
    }

    // Пароль зберігається у файл лише якщо це дозволено в реєстрі
    // (HKCU\SOFTWARE\SAMP\SaveServPasses / SaveRconPasses, керується вікном Settings).
    // Якщо ключа ще нема — за замовчуванням false (заборонено).
    bool SaveServPass = ExportPasswords && GetSavePasswordSetting(L"SaveServPasses", false);
    bool SaveRconPass = ExportPasswords && GetSavePasswordSetting(L"SaveRconPasses", false);

    bool success = WriteExact(hFile, FileTag, 4);
    int Version = FAVORITES_FILE_VERSION;
    success = success && WriteExact(hFile, &Version, sizeof(Version));
    int Count = (int)Servers.size();
    success = success && WriteExact(hFile, &Count, sizeof(Count));

    for (int i = 0; success && i < Count; i++) {
        AnsiString empty;
        success = WriteFavoriteString(hFile, Servers[i].Address) &&
                  WriteExact(hFile, &Servers[i].Port, sizeof(Servers[i].Port)) &&
                  WriteFavoriteString(hFile, Servers[i].HostName) &&
                  WriteFavoriteString(hFile, SaveServPass ? Servers[i].ServerPassword : empty) &&
                  WriteFavoriteString(hFile, SaveRconPass ? Servers[i].RconPassword : empty);
    }

    if (success)
        success = FlushFileBuffers(hFile) != FALSE;
    CloseHandle(hFile);

    if (!success || !MoveFileExW(tempFileName.c_str(), FileName.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(tempFileName.c_str());
        MessageDlg("Unable to save SA-MP favorites file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return false;
    }
    return true;
}

void __fastcall TfmMain::ExitClick(TObject *Sender)
{
    Close();
}

void __fastcall TfmMain::miViewClick(TObject *Sender)
{
    miFilterServerInfo->Checked = Filtered;
    miStatusBar->Checked = sbMain->Visible;
}

void __fastcall TfmMain::ToggleFilterServerInfo(TObject *Sender)
{
    if (Filtered) {
        Filtered = false;
        pnBreakable->Height = 16;
        gbInfo->Caption = " Server Info ";
    } else {
        Filtered = true;
        pnBreakable->Height = 100;
        pnBreakable->Top = 0;
        if (lbServers->ItemIndex != -1) {
            int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
            if (Idx >= 0 && Idx < (int)Servers.size())
                gbInfo->Caption = " Server Info: " + String(Servers[Idx].HostName) + " ";
        }
    }
    UpdateServers();
}

void __fastcall TfmMain::ToggleStatusBar(TObject *Sender)
{
    sbMain->Visible = !sbMain->Visible;
    sbMain->Top = 10000;
}

void __fastcall TfmMain::ConnectClick(TObject *Sender)
{
    if (lbServers->ItemIndex == -1) return;
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size()) return;

    String SrvPwd = String(Servers[Idx].ServerPassword);
    if (Servers[Idx].Passworded) {
        if (!InputQuery("Server Password", "This server requires a password...", SrvPwd))
            return;
    }
    if (edName->Text.IsEmpty()) {
        String NickName;
        if (!InputQuery("Who are you?", "Enter your nickname/handle...", NickName))
            return;
        if (NickName.IsEmpty())
            return;
        edName->Text = NickName;
    }

    ServerConnect(String(Servers[Idx].Address), IntToStr(Servers[Idx].Port), SrvPwd);
}

void __fastcall TfmMain::AddServerClick(TObject *Sender)
{
    String Server = GetClipBoardStr();
    if (MasterFile != 0 && lbServers->ItemIndex != -1) {
        int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
        if (Idx >= 0 && Idx < (int)Servers.size())
            Server = String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port);
        tsServerLists->TabIndex = 0;
        if (MasterFile != 0) {
            bool allowChange = true;
            tsServerListsChange(tsServerLists, 0, allowChange);
        }
    }
    if (InputQuery("Add Server", "Enter new server HOST:PORT...", Server)) {
        if (!Server.IsEmpty())
            AddServer(Server);
    }
}

void __fastcall TfmMain::DeleteServerClick(TObject *Sender)
{
    if (lbServers->ItemIndex == -1 || tsServerLists->TabIndex != 0) return;

    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size()) return;

    lbServers->Items->Delete(lbServers->ItemIndex);
    for (int i = Idx; i < (int)Servers.size() - 1; i++)
        Servers[i] = Servers[i + 1];
    Servers.resize(Servers.size() - 1);
    ServerOrderDirty = true;
    UpdateServers();

    FavoritesChanged = !ExportFavorites(GetUserDataFileName(), true);
}

void __fastcall TfmMain::RefreshServerClick(TObject *Sender)
{
    if (lbServers->ItemIndex == -1) return;
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size()) return;

    QueryServerInfo(String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port) + "#" + IntToStr(Servers[Idx].Tag), true, true, true, true);
}

static bool DownloadMasterServerList(TStringList *ServerList, String &ErrorMessage)
{
    const int MAX_RESPONSE_SIZE = 16 * 1024 * 1024;
    const int MAX_MASTER_SERVERS = 100000;

    HINTERNET hInet = InternetOpenW(L"Mozilla/5.0 (compatible; SA:MP v0.3.7)",
        INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hInet) {
        ErrorMessage = "Unable to initialize the master-list connection.";
        return false;
    }

    DWORD timeout = 10000;
    InternetSetOptionW(hInet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionW(hInet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionW(hInet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

    HINTERNET hUrl = InternetOpenUrlW(hInet, L"https://api.open.mp/servers", nullptr, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hUrl) {
        InternetCloseHandle(hInet);
        ErrorMessage = "Unable to download the master server list.";
        return false;
    }

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    if (!HttpQueryInfoW(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                        &statusCode, &statusSize, nullptr) ||
        statusCode < 200 || statusCode >= 300)
    {
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInet);
        ErrorMessage = "The master server returned an invalid HTTP response.";
        return false;
    }

    char buffer[4096];
    AnsiString raw;
    bool readSucceeded = true;
    while (true) {
        DWORD bytesRead = 0;
        if (!InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead)) {
            readSucceeded = false;
            break;
        }
        if (bytesRead == 0)
            break;
        if (raw.Length() > MAX_RESPONSE_SIZE - (int)bytesRead) {
            readSucceeded = false;
            ErrorMessage = "The master server response is too large.";
            break;
        }
        raw += AnsiString(buffer, bytesRead);
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInet);

    if (!readSucceeded || raw.IsEmpty()) {
        if (ErrorMessage.IsEmpty())
            ErrorMessage = "Unable to read the master server response.";
        return false;
    }

    TJSONValue *jsonValue = TJSONObject::ParseJSONValue(String(raw));
    TJSONArray *jsonArray = dynamic_cast<TJSONArray*>(jsonValue);
    if (!jsonArray) {
        delete jsonValue;
        ErrorMessage = "The master server returned invalid JSON.";
        return false;
    }

    bool valid = true;
    try {
        if (jsonArray->Count > MAX_MASTER_SERVERS) {
            ErrorMessage = "The master server returned too many entries.";
            valid = false;
        } else {
            for (int i = 0; i < jsonArray->Count; i++) {
                TJSONObject *serverObj = dynamic_cast<TJSONObject*>(jsonArray->Items[i]);
                if (!serverObj)
                    continue;

                TJSONPair *ipPair = serverObj->Get("ip");
                if (ipPair && ipPair->JsonValue) {
                    String fullAddress = ipPair->JsonValue->Value().Trim();
                    int colonPosition = fullAddress.Pos(":");
                    if (colonPosition > 1 && fullAddress.Length() <= 300 &&
                        ServerList->IndexOf(fullAddress) == -1)
                        ServerList->Add(fullAddress);
                }
            }
        }
    }
    __finally {
        delete jsonValue;
    }

    return valid;
}

// "Master server update" button
void __fastcall TfmMain::tbMasterServerUpdateClick(TObject *Sender)
{
    MasterServerUpdateClick(Sender);
}

void __fastcall TfmMain::MasterServerUpdateClick(TObject *Sender)
{
    if (MasterUpdateInProgress)
        return;

    if (MasterUpdateThread) {
        if (!MasterUpdateThreadFinished()) {
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
    try {
      MasterUpdateThread = TThread::CreateAnonymousThread([this]() {
        TStringList *serverList = nullptr;
        String errorMessage;
        bool success = false;
        try {
            serverList = new TStringList();
            success = DownloadMasterServerList(serverList, errorMessage);
        }
        catch (Exception &exception) {
            errorMessage = exception.Message;
        }
        catch (...) {
            errorMessage = "Unexpected error while downloading the master server list.";
        }

        if (InterlockedCompareExchange(&MasterUpdateCancel, 0, 0) != 0) {
            delete serverList;
            return;
        }

        TThread::Synchronize(MasterUpdateThread, [this, serverList, errorMessage, success]() {
            bool keepProgressOpen = false;
            try {
                if (InterlockedCompareExchange(&MasterUpdateCancel, 0, 0) == 0) {
                    if (success) {
                        ApplyMasterServerList(serverList);
                        keepProgressOpen = MasterQueryBatchActive;
                    } else
                        ShowMessage(errorMessage);
                }
            }
            __finally {
                delete serverList;
                if (!keepProgressOpen) {
                    CloseMasterUpdateProgress();
                    MasterUpdateInProgress = false;
                }
            }
        });
      });
      MasterUpdateThread->FreeOnTerminate = false;
      MasterUpdateThread->Start();
    }
    catch (...) {
        if (MasterUpdateThread) {
            delete MasterUpdateThread;
            MasterUpdateThread = nullptr;
        }
        CloseMasterUpdateProgress();
        MasterUpdateInProgress = false;
        MessageDlg("Unable to start the master-list update.", mtError,
                   TMsgDlgButtons() << mbOK, 0);
    }
}

void __fastcall TfmMain::ApplyMasterServerList(TStringList *SL)
{
    MasterQueryBatchActive = false;
    MasterQueryPending = 0;
    MasterQueryDeadline = 0;
    ServerListDirty = false;
    if (QueryNetworkThread)
        QueryNetworkThread->Clear();
    ClearQueryQueue();
    lbServers->Clear();
    lbPlayers->Clear();
    lbRules->Clear();
    Servers.clear();
    IPList->Clear();
    DnsCache.clear();
    tmrQueryQueueProcess->Enabled = false;
    tmrServerListUpdate->Enabled = false;

    for (int i = 0; i < SL->Count; i++) {
        String FullAddr = SL->Strings[i].Trim();
        int ColonPos = FullAddr.Pos(":");
        if (ColonPos <= 1)
            continue;

        String Address = FullAddr.SubString(1, ColonPos - 1).Trim();
        String PortText = FullAddr.SubString(ColonPos + 1, FullAddr.Length()).Trim();
        int Port = StrToIntDef(PortText, -1);
        if (Address.IsEmpty() || Port < 1 || Port > 65535)
            continue;

        TServerInfo server = {};
        server.HostName = AnsiString("(Retrieving info...) " + FullAddr);
        server.Address  = AnsiString(Address);
        server.Port     = Port;
        server.Ping     = 9999;
        server.Tag      = (WORD)random(0xFFFF);
        Servers.push_back(server);
        EnqueueQuery(Address + ":" + IntToStr(Port) + "#" + IntToStr(server.Tag));
    }

    MasterQueryPending = (int)Servers.size();
    RebuildServerLookup();
    MasterQueryBatchActive =
        MasterQueryPending > 0 && QuerySocket != INVALID_SOCKET;
    tmrQueryQueueProcess->Enabled = QuerySocket != INVALID_SOCKET;
    tmrServerListUpdate->Enabled = true;

    ServerOrderDirty = true;
    if (!MasterQueryBatchActive)
        UpdateServers();
}

void __fastcall TfmMain::CheckMasterQueryBatchComplete()
{
    if (!MasterQueryBatchActive)
        return;
    if (PendingQueryCount() > 0) {
        MasterQueryDeadline = 0;
        return;
    }
    if (MasterQueryPending <= 0) {
        FinishMasterQueryBatch();
        return;
    }

    DWORD now = timeGetTime();
    if (MasterQueryDeadline == 0)
        MasterQueryDeadline = now + MASTER_QUERY_RESPONSE_TIMEOUT_MS;
    else if ((LONG)(now - MasterQueryDeadline) >= 0)
        FinishMasterQueryBatch();
}

void __fastcall TfmMain::FinishMasterQueryBatch()
{
    if (!MasterQueryBatchActive)
        return;

    MasterQueryBatchActive = false;
    MasterQueryPending = 0;
    MasterQueryDeadline = 0;
    ServerListDirty = false;

    UpdateServers();
    if (lbServers->Items->Count > 0)
        lbServers->ItemIndex = 0;
    lbServersClick(this);

    CloseMasterUpdateProgress();
    MasterUpdateInProgress = false;
}

void __fastcall TfmMain::CloseMasterUpdateProgress()
{
    TfmMasterUpdate *progressForm = MasterUpdateProgressForm;
    MasterUpdateProgressForm = nullptr;
    if (progressForm) {
        progressForm->Close();
        delete progressForm;
    }
}

void __fastcall TfmMain::CancelMasterServerUpdate()
{
    InterlockedExchange(&MasterUpdateCancel, 1);
    if (QueryNetworkThread)
        QueryNetworkThread->Clear();
    ClearQueryQueue();
    MasterQueryBatchActive = false;
    MasterQueryPending = 0;
    MasterQueryDeadline = 0;
    ServerListDirty = false;
    CloseMasterUpdateProgress();
    MasterUpdateInProgress = false;
}

void __fastcall TfmMain::CopyServerInfoClick(TObject *Sender)
{
    if (lbServers->ItemIndex == -1) return;
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size()) return;

    String Str = "HostName: " + String(Servers[Idx].HostName) + "\r\n" +
                 "Address:  " + String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port) + "\r\n" +
                 "Players:  " + IntToStr(Servers[Idx].Players) + " / " + IntToStr(Servers[Idx].MaxPlayers) + "\r\n" +
                 "Ping:     " + IntToStr(Servers[Idx].Ping) + "\r\n" +
                 "Mode:     " + String(Servers[Idx].Mode) + "\r\n" +
                 "Language: " + String(Servers[Idx].Map);
    SetClipBoardStr(Str);
}

void __fastcall TfmMain::ServerPropertiesClick(TObject *Sender)
{
    if (lbServers->ItemIndex == -1) return;
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size()) return;

    TfmServerProperties *fm = new TfmServerProperties(Application);
    fm->lbHostName->Caption = String(Servers[Idx].HostName);
    fm->edAddress->Text = String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port);
    fm->lbPlayers->Caption = IntToStr(Servers[Idx].Players) + " / " + IntToStr(Servers[Idx].MaxPlayers);
    fm->lbPing->Caption = IntToStr(Servers[Idx].Ping);
    fm->lbMode->Caption = String(Servers[Idx].Mode);
    fm->lbMap->Caption = String(Servers[Idx].Map);
    fm->edServerPassword->Text = String(Servers[Idx].ServerPassword);
    fm->edRconPassword->Text = String(Servers[Idx].RconPassword);
    fm->edServerPassword->Enabled = Servers[Idx].Passworded;
    if (!Servers[Idx].Passworded)
        fm->edServerPassword->Color = clBtnFace;
    fm->ShowModal();
    delete fm;
}

void __fastcall TfmMain::SettingsClick(TObject *Sender)
{
    TfmSettings *fm = new TfmSettings(Application);
    fm->ShowModal();
    delete fm;
}

void __fastcall TfmMain::RemoteConsoleClick(TObject *Sender)
{
    TfmRconConfig *fm = new TfmRconConfig(Application);
    if (lbServers->ItemIndex != -1) {
        int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
        if (Idx >= 0 && Idx < (int)Servers.size()) {
            fm->edHost->Text = String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port);
            fm->edPassword->Text = String(Servers[Idx].RconPassword);
        }
    }
    fm->ShowModal();
    delete fm;
}

void __fastcall TfmMain::HelpTopicsClick(TObject *Sender)
{
    ShellExecuteW(Handle, L"open", L"https://wiki.sa-mp.com/", nullptr, nullptr, SW_SHOWNORMAL);
}

void __fastcall TfmMain::AboutClick(TObject *Sender)
{
    fmAbout->ShowModal();
}

void __fastcall TfmMain::miSampClick(TObject *Sender)
{
    ShellExecuteW(Handle, L"open", L"https://www.sa-mp.com/", nullptr, nullptr, SW_SHOWNORMAL);
}

void __fastcall TfmMain::label_urlClick(TObject *Sender)
{
    ShellExecuteW(0, L"open", (L"http://" + ((TLabel*)Sender)->Caption).c_str(), L"", L"", SW_SHOWNORMAL);
}

void __fastcall TfmMain::imLogoClick(TObject *Sender)
{
    fmAbout->ShowModal();
}



void __fastcall TfmMain::piCopyClick(TObject *Sender)
{
    SetClipBoardStr(edSIAddress->Text);
}

void __fastcall TfmMain::pmCopyPopup(TObject *Sender)
{
    piCopy->Enabled = (edSIAddress->Text != "- - -");
}

void __fastcall TfmMain::tsServerListsChange(TObject *Sender, int NewTab, bool &AllowChange)
{
    // Update the list mode before importing or repainting. Favorites are allowed
    // to remain visible while their server query is still pending.
    int previousMasterFile = MasterFile;
    MasterRefreshPending = false;
    if (MasterUpdateInProgress || MasterQueryBatchActive)
        CancelMasterServerUpdate();
    MasterFile = NewTab;

    if (previousMasterFile == 0 && FavoritesChanged) {
        FavoritesChanged = !ExportFavorites(GetUserDataFileName(), true);
    }

    ClearQueryQueue();
    lbServers->Clear();
    lbPlayers->Clear();
    lbRules->Clear();

    if (NewTab == 0) {
        String userData = GetUserDataFileName();
        Servers.clear();
        ServerOrderDirty = true;
        if (!userData.IsEmpty() && FileExists(userData))
            ImportFavorites(userData, false);
    } else {
        Servers.clear();
        ServerOrderDirty = true;
    }

    UpdateServers();

    tbMasterServerUpdate->Enabled = (NewTab != 0);
    miMasterServerUpdate->Enabled = (NewTab != 0);

    miAddServer->Enabled = (NewTab == 0);
    tbDeleteServer->Enabled = (NewTab == 0);
    miDeleteServer->Enabled = (NewTab == 0);
    piDeleteServer->Visible = (NewTab == 0);

    if (lbServers->Items->Count > 0)
        lbServers->ItemIndex = 0;

    lbServersClick(Sender);
    if (NewTab != 0)
        MasterServerUpdateClick(Sender);
    IPList->Clear();
    DnsCache.clear();
}

// C++Builder's Win32 startup object expects the ANSI WinMain symbol even
// when the VCL application itself is Unicode-aware.
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        Application->Initialize();
        Application->MainFormOnTaskBar = true;
        Application->CreateForm(__classid(TfmMain), &fmMain);
        Application->CreateForm(__classid(TfmAbout), &fmAbout);
        Application->CreateForm(__classid(Twnd_webrunform), &wnd_webrunform);
        Application->Run();
    }
    catch (Exception &exception)
    {
        Application->ShowException(&exception);
    }
    catch (...)
    {
        try
        {
            throw Exception("");
        }
        catch (Exception &exception)
        {
            Application->ShowException(&exception);
        }
    }
    return 0;
}
