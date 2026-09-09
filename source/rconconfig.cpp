/*
* rconconfig.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "rconconfig.h"

#pragma package(smart_init)
#pragma resource "..\\dfm\\rconconfig.dfm"

#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

TfmRconConfig *fmRconConfig;

static volatile LONG bQuitApp = 0;
static SOCKET s = INVALID_SOCKET;
static WORD wPort = 0;
static char* szPassword = NULL;

static struct sockaddr_in to;
static struct sockaddr_in s_in;

static HANDLE hConsoleThread = NULL;
static HANDLE hNetworkThread = NULL;
static unsigned char ipblock[4];

static volatile LONG bGotResponse = 0;
static volatile LONG rconSessionActive = 0;

// struct
struct TRconParams {
    AnsiString Host;
    int Port;
    AnsiString Pass;
};

static void logprintf(const char* format, ...)
{
    char buffer[512];
    DWORD written;
    va_list ap;
    HANDLE hOut;

    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(ap);

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    WriteConsoleA(hOut, buffer, lstrlenA(buffer), &written, NULL);
    WriteConsoleA(hOut, "\n", 1, &written, NULL);
}

static BOOL WINAPI CtrlHandler(DWORD type)
{
    if (type == CTRL_C_EVENT || type == CTRL_CLOSE_EVENT)
    {
        logprintf("Wait for the console to close.\n");
        InterlockedExchange(&bQuitApp, 1);
        return TRUE;
    }
    return FALSE;
}

static void SendCommand(const char* szCommand)
{
    if (s == INVALID_SOCKET || !szPassword || !szCommand || !szCommand[0]) return;

    int cmdLen = lstrlenA(szCommand);
    int passLen = lstrlenA(szPassword);
    if (cmdLen > 0xFFFF || passLen > 0xFFFF)
        return;

    int totalLen = 4 + 4 + 2 + 1 + 2 + passLen + 2 + cmdLen;
    char* data = (char*)LocalAlloc(LPTR, totalLen);
    if (!data) return;

    char* ptr = data;

    DWORD signature = 0x504D4153;
    memcpy(ptr, &signature, sizeof(signature)); ptr += sizeof(signature);
    memcpy(ptr, &to.sin_addr.s_addr, sizeof(to.sin_addr.s_addr)); ptr += sizeof(to.sin_addr.s_addr);
    memcpy(ptr, &wPort, sizeof(wPort)); ptr += sizeof(wPort);

    *ptr = 'x'; ptr++;

    WORD passLength = (WORD)passLen;
    memcpy(ptr, &passLength, sizeof(passLength)); ptr += sizeof(passLength);
    memcpy(ptr, szPassword, passLen); ptr += passLen;

    WORD commandLength = (WORD)cmdLen;
    memcpy(ptr, &commandLength, sizeof(commandLength)); ptr += sizeof(commandLength);
    memcpy(ptr, szCommand, cmdLen); ptr += cmdLen;

    sendto(s, data, (int)(ptr - data), 0, (struct sockaddr*)&to, sizeof(to));

    LocalFree(data);
}



DWORD WINAPI NetworkPumpThread(void* pParam)
{
    char buf[1024];
    while (InterlockedCompareExchange(&bQuitApp, 0, 0) == 0)
    {
        sockaddr_in from = {};
        int fromSize = sizeof(from);
        int len = recvfrom(s, buf, sizeof(buf), 0, (sockaddr*)&from, &fromSize);
        if (len < 13)
            continue;

        if (from.sin_addr.s_addr != to.sin_addr.s_addr ||
            from.sin_port != to.sin_port ||
            memcmp(buf, "SAMP", 4) != 0 ||
            buf[10] != 'x')
            continue;

        WORD messageLength = 0;
        memcpy(&messageLength, &buf[11], sizeof(messageLength));
        if ((int)messageLength > len - 13)
            continue;

        InterlockedExchange(&bGotResponse, 1);
        char message[1025];
        int copyLength = (int)messageLength;
        if (copyLength > (int)sizeof(message) - 1)
            copyLength = sizeof(message) - 1;
        memcpy(message, &buf[13], copyLength);
        message[copyLength] = 0;
        logprintf("%s", message);
    }
    return 0;
}

static DWORD WINAPI ConsoleInputThread(void* param)
{
    char buf[512];
    char ansiBuf[512];
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

    while (InterlockedCompareExchange(&bQuitApp, 0, 0) == 0)
    {
        Sleep(50);

        if (hIn == INVALID_HANDLE_VALUE) break;

        DWORD read = 0;
        if (!ReadConsoleA(hIn, buf, sizeof(buf) - 1, &read, NULL) || read == 0)
            continue;

        buf[read] = '\0';

        while (read > 0 && (buf[read - 1] == '\r' || buf[read - 1] == '\n' || buf[read - 1] == ' '))
        {
            buf[read - 1] = '\0';
            read--;
        }

        if (read > 0 && InterlockedCompareExchange(&bQuitApp, 0, 0) == 0)
        {
            OemToCharA(buf, ansiBuf);
            
            SendCommand(ansiBuf);
        }
    }

    return 0;
}

void __stdcall AllocRconConsole(const char* szHostParam, int port, const char* szPass)
{
    WSADATA wsa;
    struct hostent* he;
    struct in_addr in;
    DWORD tid;
    
    InterlockedExchange(&bQuitApp, 0);
    InterlockedExchange(&bGotResponse, 0);

    if (WSAStartup(0x0202, &wsa) != 0)
        return;
    if (!AllocConsole()) {
        WSACleanup();
        return;
    }

    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    logprintf("\n SA:MP Command Line Remote Console Client");
    logprintf(" ----------------------------------------");
    logprintf(" (C) Copyright 2005-2006 SA:MP Team, v1.0\n");
    logprintf("\nPress Ctrl + C to exit\n");

    unsigned long ulAddr = inet_addr(szHostParam);
    if (ulAddr != INADDR_NONE) 
    {
        in.s_addr = ulAddr;
    } 
    else 
    {
        he = gethostbyname(szHostParam);
        if (he) {
            CopyMemory(&in, he->h_addr, he->h_length);
        } else {
            logprintf("ERROR: Bad host.");
            SetConsoleCtrlHandler(CtrlHandler, FALSE);
            FreeConsole();
            WSACleanup();
            return;
        }
    }

    ipblock[0] = in.S_un.S_un_b.s_b1;
    ipblock[1] = in.S_un.S_un_b.s_b2;
    ipblock[2] = in.S_un.S_un_b.s_b3;
    ipblock[3] = in.S_un.S_un_b.s_b4;

    ZeroMemory(&to, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_port = htons((WORD)port);
    to.sin_addr = in;

    wPort = (WORD)port;

    logprintf("Remote Console: %d.%d.%d.%d:%d...", ipblock[0], ipblock[1], ipblock[2], ipblock[3], wPort);

    int passLen = lstrlenA(szPass);
    szPassword = (char*)LocalAlloc(LPTR, passLen + 1);
    if (!szPassword) {
        logprintf("ERROR: Unable to allocate RCON password.");
        SetConsoleCtrlHandler(CtrlHandler, FALSE);
        FreeConsole();
        WSACleanup();
        return;
    }
    lstrcpynA(szPassword, szPass, passLen + 1);

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) {
        logprintf("ERROR: Unable to create RCON socket.");
        LocalFree(szPassword);
        szPassword = NULL;
        SetConsoleCtrlHandler(CtrlHandler, FALSE);
        FreeConsole();
        WSACleanup();
        return;
    }

    DWORD timeout = 500;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    s_in.sin_family = AF_INET;
    s_in.sin_addr.s_addr = INADDR_ANY;
    s_in.sin_port = 0;

    if (bind(s, (struct sockaddr*)&s_in, sizeof(s_in)) == SOCKET_ERROR) {
        logprintf("ERROR: Unable to bind RCON socket.");
        closesocket(s);
        s = INVALID_SOCKET;
        LocalFree(szPassword);
        szPassword = NULL;
        SetConsoleCtrlHandler(CtrlHandler, FALSE);
        FreeConsole();
        WSACleanup();
        return;
    }

    hConsoleThread = CreateThread(NULL, 0, ConsoleInputThread, NULL, 0, &tid);
    hNetworkThread = CreateThread(NULL, 0, NetworkPumpThread, NULL, 0, &tid);
    if (!hConsoleThread || !hNetworkThread) {
        logprintf("ERROR: Unable to start RCON threads.");
        InterlockedExchange(&bQuitApp, 1);
    }

    SendCommand("echo RCON admin connected.");

	const DWORD CONNECT_TIMEOUT_MS = 3000;
	DWORD startTick = GetTickCount();
	while (InterlockedCompareExchange(&bGotResponse, 0, 0) == 0 &&
           InterlockedCompareExchange(&bQuitApp, 0, 0) == 0)
	{
		if (GetTickCount() - startTick > CONNECT_TIMEOUT_MS)
		{
			logprintf("\nERROR: Server did not respond. Check that RCON is enabled and the password/port are correct.\n");
			InterlockedExchange(&bQuitApp, 1);
			break;
		}
		Sleep(50);
	}

    while (InterlockedCompareExchange(&bQuitApp, 0, 0) == 0)
        Sleep(100);

    SetConsoleCtrlHandler(CtrlHandler, FALSE);

    if (s != INVALID_SOCKET) {
        shutdown(s, SD_BOTH);
        closesocket(s);
        s = INVALID_SOCKET;
    }
    FreeConsole();

    if (hConsoleThread) {
        WaitForSingleObject(hConsoleThread, INFINITE);
        CloseHandle(hConsoleThread);
        hConsoleThread = NULL;
    }
    if (hNetworkThread) {
        WaitForSingleObject(hNetworkThread, INFINITE);
        CloseHandle(hNetworkThread);
        hNetworkThread = NULL;
    }

    WSACleanup();

    if (szPassword) {
        LocalFree(szPassword);
        szPassword = NULL;
    }
}

static DWORD WINAPI RconThreadProc(LPVOID lpParam)
{
    TRconParams* p = (TRconParams*)lpParam;
    AllocRconConsole(p->Host.c_str(), p->Port, p->Pass.c_str());
    delete p;
    InterlockedExchange(&rconSessionActive, 0);
    return 0;
}

__fastcall TfmRconConfig::TfmRconConfig(TComponent *Owner)
    : TForm(Owner)
{
}

void __fastcall TfmRconConfig::bnCancelClick(TObject *Sender)
{
    Close();
}

void __fastcall TfmRconConfig::bnConnectClick(TObject *Sender)
{
    if (InterlockedCompareExchange(&rconSessionActive, 1, 0) != 0) {
        MessageDlg("An RCON session is already active.", mtInformation, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    UnicodeString server = edHost->Text;
    UnicodeString addr;
    UnicodeString portStr;

    int pos = server.Pos(L":");
    if (pos != 0) {
        addr = server.SubString(1, pos - 1);
        portStr = server.SubString(pos + 1, 5);
    } else {
        addr = server;
        portStr = L"7777";
    }

    int port = StrToIntDef(portStr, -1);
    if (addr.IsEmpty() || addr.Length() > 253 ||
        port < 1 || port > 65535 ||
        edPassword->Text.Length() > 65535)
    {
        InterlockedExchange(&rconSessionActive, 0);
        MessageDlg("Invalid RCON host, port, or password.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    AnsiString ansiAddr = addr;
    AnsiString ansiPass = edPassword->Text;

    TRconParams* params = new TRconParams();
    params->Host = ansiAddr;
    params->Port = port;
    params->Pass = ansiPass;

    DWORD tid;
    HANDLE thread = CreateThread(NULL, 0, RconThreadProc, params, 0, &tid);
    if (!thread) {
        delete params;
        InterlockedExchange(&rconSessionActive, 0);
        MessageDlg("Unable to start the RCON session.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }
    CloseHandle(thread);

    Close();
}

void __fastcall TfmRconConfig::edHostKeyPress(TObject *Sender, System::WideChar &Key)
{
    if (!((Key >= L'0' && Key <= L'9') ||
          (Key >= L'a' && Key <= L'z') ||
          (Key >= L'A' && Key <= L'Z') ||
          Key == L'.' || Key == L':'  ||
          Key == 8   || Key == 46))
    {
        Key = 0;
    }
}

void __fastcall TfmRconConfig::edPasswordChange(TObject *Sender)
{
    bnConnect->Enabled = (!edHost->Text.IsEmpty()) && (!edPassword->Text.IsEmpty());
}
