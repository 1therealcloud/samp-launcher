/*
* CRconClient.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "CRconClient.h"

#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

struct TRconParams
{
    AnsiString Host;
    int Port;
    AnsiString Password;
};

static volatile LONG QuitRequested = 0;
static volatile LONG GotResponse = 0;
static volatile LONG SessionActive = 0;
static SOCKET RconSocket = INVALID_SOCKET;
static WORD RconPort = 0;
static char* RconPassword = nullptr;
static sockaddr_in RemoteAddress = {};
static sockaddr_in LocalAddress = {};
static HANDLE ConsoleThread = nullptr;
static HANDLE NetworkThread = nullptr;
static unsigned char IpBlock[4] = {};

static void Log(const char* Format, ...)
{
    char buffer[512];
    va_list arguments;
    va_start(arguments, Format);
    vsnprintf(buffer, sizeof(buffer), Format, arguments);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(arguments);

    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written = 0;
    WriteConsoleA(output, buffer, lstrlenA(buffer), &written, nullptr);
    WriteConsoleA(output, "\n", 1, &written, nullptr);
}

static BOOL WINAPI CtrlHandler(DWORD Type)
{
    if (Type != CTRL_C_EVENT && Type != CTRL_CLOSE_EVENT)
        return FALSE;

    Log("Wait for the console to close.\n");
    InterlockedExchange(&QuitRequested, 1);
    return TRUE;
}

static void SendCommand(const char* Command)
{
    if (RconSocket == INVALID_SOCKET || !RconPassword || !Command || !Command[0])
        return;

    int commandLength = lstrlenA(Command);
    int passwordLength = lstrlenA(RconPassword);
    if (commandLength > 0xFFFF || passwordLength > 0xFFFF)
        return;

    int totalLength = 4 + 4 + 2 + 1 + 2 + passwordLength + 2 + commandLength;
    char* data = static_cast<char*>(LocalAlloc(LPTR, totalLength));
    if (!data)
        return;

    char* cursor = data;
    DWORD signature = 0x504D4153;
    memcpy(cursor, &signature, sizeof(signature));
    cursor += sizeof(signature);
    memcpy(cursor, &RemoteAddress.sin_addr.s_addr, sizeof(RemoteAddress.sin_addr.s_addr));
    cursor += sizeof(RemoteAddress.sin_addr.s_addr);
    memcpy(cursor, &RconPort, sizeof(RconPort));
    cursor += sizeof(RconPort);

    *cursor++ = 'x';

    WORD passwordSize = static_cast<WORD>(passwordLength);
    memcpy(cursor, &passwordSize, sizeof(passwordSize));
    cursor += sizeof(passwordSize);
    memcpy(cursor, RconPassword, passwordLength);
    cursor += passwordLength;

    WORD commandSize = static_cast<WORD>(commandLength);
    memcpy(cursor, &commandSize, sizeof(commandSize));
    cursor += sizeof(commandSize);
    memcpy(cursor, Command, commandLength);
    cursor += commandLength;

    sendto(RconSocket, data, static_cast<int>(cursor - data), 0, reinterpret_cast<sockaddr*>(&RemoteAddress),
           sizeof(RemoteAddress));
    LocalFree(data);
}

static DWORD WINAPI NetworkPumpThread(void*)
{
    char buffer[1024];
    while (InterlockedCompareExchange(&QuitRequested, 0, 0) == 0)
    {
        sockaddr_in from = {};
        int fromSize = sizeof(from);
        int length = recvfrom(RconSocket, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&from), &fromSize);
        if (length < 13)
            continue;

        if (from.sin_addr.s_addr != RemoteAddress.sin_addr.s_addr || from.sin_port != RemoteAddress.sin_port ||
            memcmp(buffer, "SAMP", 4) != 0 || buffer[10] != 'x')
            continue;

        WORD messageLength = 0;
        memcpy(&messageLength, &buffer[11], sizeof(messageLength));
        if (static_cast<int>(messageLength) > length - 13)
            continue;

        InterlockedExchange(&GotResponse, 1);
        char message[1025];
        int copyLength = static_cast<int>(messageLength);
        if (copyLength > static_cast<int>(sizeof(message)) - 1)
            copyLength = sizeof(message) - 1;
        memcpy(message, &buffer[13], copyLength);
        message[copyLength] = 0;
        Log("%s", message);
    }
    return 0;
}

static DWORD WINAPI ConsoleInputThread(void*)
{
    char buffer[512];
    char ansiBuffer[512];
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);

    while (InterlockedCompareExchange(&QuitRequested, 0, 0) == 0)
    {
        Sleep(50);
        if (input == INVALID_HANDLE_VALUE)
            break;

        DWORD read = 0;
        if (!ReadConsoleA(input, buffer, sizeof(buffer) - 1, &read, nullptr) || read == 0)
            continue;

        buffer[read] = '\0';
        while (read > 0 && (buffer[read - 1] == '\r' || buffer[read - 1] == '\n' || buffer[read - 1] == ' '))
        {
            buffer[--read] = '\0';
        }

        if (read > 0 && InterlockedCompareExchange(&QuitRequested, 0, 0) == 0)
        {
            OemToCharA(buffer, ansiBuffer);
            SendCommand(ansiBuffer);
        }
    }
    return 0;
}

static void RunConsole(const char* Host, int Port, const char* Password)
{
    WSADATA winsock = {};
    hostent* hostEntry = nullptr;
    in_addr address = {};
    DWORD threadId = 0;

    InterlockedExchange(&QuitRequested, 0);
    InterlockedExchange(&GotResponse, 0);

    if (WSAStartup(0x0202, &winsock) != 0)
        return;
    if (!AllocConsole())
    {
        WSACleanup();
        return;
    }

    SetConsoleCtrlHandler(CtrlHandler, TRUE);
    Log("\n SA:MP Command Line Remote Console Client");
    Log(" ----------------------------------------");
    Log(" (C) Copyright 2005-2006 SA:MP Team, v1.0\n");
    Log("\nPress Ctrl + C to exit\n");

    unsigned long numericAddress = inet_addr(Host);
    if (numericAddress != INADDR_NONE)
    {
        address.s_addr = numericAddress;
    }
    else
    {
        hostEntry = gethostbyname(Host);
        if (!hostEntry)
        {
            Log("ERROR: Bad host.");
            SetConsoleCtrlHandler(CtrlHandler, FALSE);
            FreeConsole();
            WSACleanup();
            return;
        }
        CopyMemory(&address, hostEntry->h_addr, hostEntry->h_length);
    }

    IpBlock[0] = address.S_un.S_un_b.s_b1;
    IpBlock[1] = address.S_un.S_un_b.s_b2;
    IpBlock[2] = address.S_un.S_un_b.s_b3;
    IpBlock[3] = address.S_un.S_un_b.s_b4;

    ZeroMemory(&RemoteAddress, sizeof(RemoteAddress));
    RemoteAddress.sin_family = AF_INET;
    RemoteAddress.sin_port = htons(static_cast<WORD>(Port));
    RemoteAddress.sin_addr = address;
    RconPort = static_cast<WORD>(Port);

    Log("Remote Console: %d.%d.%d.%d:%d...", IpBlock[0], IpBlock[1], IpBlock[2], IpBlock[3], RconPort);

    int passwordLength = lstrlenA(Password);
    RconPassword = static_cast<char*>(LocalAlloc(LPTR, passwordLength + 1));
    if (!RconPassword)
    {
        Log("ERROR: Unable to allocate RCON password.");
        SetConsoleCtrlHandler(CtrlHandler, FALSE);
        FreeConsole();
        WSACleanup();
        return;
    }
    lstrcpynA(RconPassword, Password, passwordLength + 1);

    RconSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (RconSocket == INVALID_SOCKET)
    {
        Log("ERROR: Unable to create RCON socket.");
        LocalFree(RconPassword);
        RconPassword = nullptr;
        SetConsoleCtrlHandler(CtrlHandler, FALSE);
        FreeConsole();
        WSACleanup();
        return;
    }

    DWORD timeout = 500;
    setsockopt(RconSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

    ZeroMemory(&LocalAddress, sizeof(LocalAddress));
    LocalAddress.sin_family = AF_INET;
    LocalAddress.sin_addr.s_addr = INADDR_ANY;
    LocalAddress.sin_port = 0;

    if (bind(RconSocket, reinterpret_cast<sockaddr*>(&LocalAddress), sizeof(LocalAddress)) == SOCKET_ERROR)
    {
        Log("ERROR: Unable to bind RCON socket.");
        closesocket(RconSocket);
        RconSocket = INVALID_SOCKET;
        LocalFree(RconPassword);
        RconPassword = nullptr;
        SetConsoleCtrlHandler(CtrlHandler, FALSE);
        FreeConsole();
        WSACleanup();
        return;
    }

    ConsoleThread = CreateThread(nullptr, 0, ConsoleInputThread, nullptr, 0, &threadId);
    NetworkThread = CreateThread(nullptr, 0, NetworkPumpThread, nullptr, 0, &threadId);
    if (!ConsoleThread || !NetworkThread)
    {
        Log("ERROR: Unable to start RCON threads.");
        InterlockedExchange(&QuitRequested, 1);
    }

    SendCommand("echo RCON admin connected.");

    const DWORD CONNECT_TIMEOUT_MS = 3000;
    DWORD startTick = GetTickCount();
    while (InterlockedCompareExchange(&GotResponse, 0, 0) == 0 && InterlockedCompareExchange(&QuitRequested, 0, 0) == 0)
    {
        if (GetTickCount() - startTick > CONNECT_TIMEOUT_MS)
        {
            Log("\nERROR: Server did not respond. Check that RCON is enabled and the password/port are correct.\n");
            InterlockedExchange(&QuitRequested, 1);
            break;
        }
        Sleep(50);
    }

    while (InterlockedCompareExchange(&QuitRequested, 0, 0) == 0)
        Sleep(100);

    SetConsoleCtrlHandler(CtrlHandler, FALSE);
    if (RconSocket != INVALID_SOCKET)
    {
        shutdown(RconSocket, SD_BOTH);
        closesocket(RconSocket);
        RconSocket = INVALID_SOCKET;
    }
    FreeConsole();

    if (ConsoleThread)
    {
        WaitForSingleObject(ConsoleThread, INFINITE);
        CloseHandle(ConsoleThread);
        ConsoleThread = nullptr;
    }
    if (NetworkThread)
    {
        WaitForSingleObject(NetworkThread, INFINITE);
        CloseHandle(NetworkThread);
        NetworkThread = nullptr;
    }

    WSACleanup();
    if (RconPassword)
    {
        LocalFree(RconPassword);
        RconPassword = nullptr;
    }
}

static DWORD WINAPI RconThreadProc(LPVOID Parameter)
{
    TRconParams* params = static_cast<TRconParams*>(Parameter);
    RunConsole(params->Host.c_str(), params->Port, params->Password.c_str());
    delete params;
    InterlockedExchange(&SessionActive, 0);
    return 0;
}

TRconStartResult CRconClient::Start(const AnsiString& Host, int Port, const AnsiString& Password)
{
    if (InterlockedCompareExchange(&SessionActive, 1, 0) != 0)
        return TRconStartResult::AlreadyActive;

    TRconParams* params = new TRconParams();
    params->Host = Host;
    params->Port = Port;
    params->Password = Password;

    DWORD threadId = 0;
    HANDLE thread = CreateThread(nullptr, 0, RconThreadProc, params, 0, &threadId);
    if (!thread)
    {
        delete params;
        InterlockedExchange(&SessionActive, 0);
        return TRconStartResult::Failed;
    }

    CloseHandle(thread);
    return TRconStartResult::Started;
}

bool CRconClient::IsActive()
{
    return InterlockedCompareExchange(&SessionActive, 0, 0) != 0;
}
