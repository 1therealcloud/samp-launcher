/*
* CGameLauncher.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "CGameLauncher.h"
#include "CDnsResolver.h"
#include "CSettings.h"
#include "globals.h"
#include "main.h"

#include <mmsystem.h>
#include <vector>

#include <winsock2.h>

enum class TGameLaunchError
{
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
    DWORD FCreationFlags;

protected:
    void __fastcall Execute() override;

public:
    TGameLaunchError Error;

    __fastcall TGameLaunchThread(HWND NotifyWindow, const String& GameExe, const String& CommandLine,
                                 const String& WorkDir, const String& SampDll, DWORD CreationFlags)
        : TThread(true), FNotifyWindow(NotifyWindow), FGameExe(GameExe), FCommandLine(CommandLine), FWorkDir(WorkDir),
          FSampDll(SampDll), FCreationFlags(CreationFlags), Error(TGameLaunchError::None)
    {
        FreeOnTerminate = false;
    }
};

static TGameLaunchThread* GameLaunchThread = nullptr;
static bool GameLaunchShuttingDown = false;
static bool GameLaunchDnsPending = false;

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
    std::vector<wchar_t> mutableCommandLine(static_cast<size_t>(FCommandLine.Length()) + 1);
    memcpy(&mutableCommandLine[0], FCommandLine.c_str(), mutableCommandLine.size() * sizeof(wchar_t));
    si.cb = sizeof(si);

    if (Terminated)
        goto Finish;

    if (!CreateProcessW(FGameExe.c_str(), &mutableCommandLine[0], nullptr, nullptr, FALSE, FCreationFlags, nullptr,
                        FWorkDir.c_str(), &si, &pi))
    {
        Error = TGameLaunchError::Execute;
        goto Finish;
    }
    processCreated = true;
    if (Terminated)
        goto Finish;

    {
        SIZE_T dllBytes = (static_cast<SIZE_T>(FSampDll.Length()) + 1) * sizeof(wchar_t);
        remotePath = VirtualAllocEx(pi.hProcess, nullptr, dllBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!remotePath)
        {
            Error = TGameLaunchError::Allocate;
            goto Finish;
        }
        if (!WriteProcessMemory(pi.hProcess, remotePath, FSampDll.c_str(), dllBytes, &bytesWritten) ||
            bytesWritten != dllBytes)
        {
            Error = TGameLaunchError::WritePath;
            goto Finish;
        }
    }

    if (Terminated)
        goto Finish;

    kernelModule = GetModuleHandleW(L"kernel32.dll");
    loadLibraryW = kernelModule ? GetProcAddress(kernelModule, "LoadLibraryW") : nullptr;
    remoteThread = loadLibraryW ? CreateRemoteThread(pi.hProcess, nullptr, 0,
                                                     reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryW), remotePath,
                                                     0, &threadId)
                                : nullptr;
    if (!remoteThread)
    {
        Error = TGameLaunchError::CreateRemoteThread;
        goto Finish;
    }

    {
        DWORD waitStarted = timeGetTime();
        while (!Terminated)
        {
            DWORD waitResult = WaitForSingleObject(remoteThread, 50);
            if (waitResult == WAIT_OBJECT_0)
            {
                remoteCompleted = true;
                break;
            }
            if (waitResult == WAIT_FAILED || timeGetTime() - waitStarted >= 10000)
                break;
        }
    }

    if (!remoteCompleted || !GetExitCodeThread(remoteThread, &exitCode) || exitCode == 0)
    {
        if (!Terminated)
            Error = TGameLaunchError::LoadLibrary;
        goto Finish;
    }

    if (Terminated)
        goto Finish;

    if (ResumeThread(pi.hThread) == (DWORD)-1)
    {
        Error = TGameLaunchError::Resume;
        goto Finish;
    }
    processCreated = false;

Finish:
    if (processCreated)
    {
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
        PostMessage(FNotifyWindow, WM_GAME_LAUNCH_COMPLETE, static_cast<WPARAM>(Error), 0);
}

String CGameLauncher::GetAbsolutePath(const String& Path)
{
    String clean = Path.Trim();
    if (clean.Length() >= 2 && clean[1] == (wchar_t)34 && clean[clean.Length()] == (wchar_t)34)
        clean = clean.SubString(2, clean.Length() - 2);
    if (clean.IsEmpty())
        return clean;

    DWORD required = GetFullPathNameW(clean.c_str(), 0, nullptr, nullptr);
    if (required == 0)
        return clean;

    std::vector<wchar_t> buffer(static_cast<size_t>(required) + 1);
    DWORD written = GetFullPathNameW(clean.c_str(), static_cast<DWORD>(buffer.size()), &buffer[0], nullptr);
    if (written == 0 || written >= buffer.size())
        return clean;
    return String(&buffer[0]);
}

bool CGameLauncher::HasUnsafeCommandLineCharacters(const String& Value)
{
    for (int i = 1; i <= Value.Length(); i++)
    {
        if (Value[i] <= 32 || Value[i] == '"')
            return true;
    }
    return false;
}

TGameLaunchResult CGameLauncher::Connect(TfmMain* Form, const String& Server, const String& Port,
                                         const String& Password)
{
    if (!Form || GameLaunchShuttingDown)
        return TGameLaunchResult::Failed;
    if (GameLaunchThread || GameLaunchDnsPending)
        return TGameLaunchResult::Started;

    String gtaExe = CSettings::GetGtaExecutable();
    if (!FileExists(gtaExe))
    {
        MessageDlg("GTA: San Andreas executable not found.\n(" + gtaExe + ")\n\nPlease locate it now.", mtError,
                   TMsgDlgButtons() << mbOK, 0);
        Form->GetGTAExe(Form->Handle);
        gtaExe = CSettings::GetGtaExecutable();
    }

    if (!FileExists(gtaExe))
    {
        MessageDlg("GTA: San Andreas executable STILL not found.\n(" + gtaExe + ")\n\nAborting launch.", mtError,
                   TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    String gameExe = GetAbsolutePath(gtaExe);
    CSettings::SetGtaExecutable(gameExe, false);
    String sampDll = GetAbsolutePath(ExtractFilePath(gameExe) + "samp.dll");
    if (!FileExists(sampDll))
    {
        MessageDlg("SA-MP library not found.\n(" + sampDll + ")", mtError, TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    if (HasUnsafeCommandLineCharacters(Form->edName->Text) || HasUnsafeCommandLineCharacters(Password))
    {
        MessageDlg("Nickname and password cannot contain spaces, quotes, or control characters.", mtError,
                   TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    String serverIP = CDnsResolver::GetCached(Server);
    if (serverIP.IsEmpty())
    {
        if (!CDnsResolver::HasCached(Server))
        {
            if (CDnsResolver::QueueConnect(Form->Handle, AnsiString(Server), Server, Port, Password))
            {
                GameLaunchDnsPending = true;
                return TGameLaunchResult::DnsPending;
            }
            CDnsResolver::Cache(Server, "");
        }
        MessageDlg("Unable to resolve the server address.", mtError, TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    String commandLine = " -c -n " + Form->edName->Text + " -h " + serverIP + " -p " + Port;
    if (!Password.IsEmpty())
        commandLine += " -z " + Password;

    String command = "\"" + gameExe + "\"" + commandLine;
    String workDir = ExtractFilePath(gameExe);
    try
    {
        GameLaunchThread = new TGameLaunchThread(
            Form->Handle, gameExe, command, workDir, sampDll,
            CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED);
        GameLaunchThread->Start();
    }
    catch (...)
    {
        delete GameLaunchThread;
        GameLaunchThread = nullptr;
        MessageDlg("Unable to execute.", mtError, TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    return TGameLaunchResult::Started;
}


TGameLaunchResult CGameLauncher::Debug(TfmMain* Form, const String& DebugScript)
{
    if (!Form || GameLaunchShuttingDown)
        return TGameLaunchResult::Failed;
    if (GameLaunchThread || GameLaunchDnsPending)
        return TGameLaunchResult::Started;

    String gtaExe = CSettings::GetGtaExecutable();
    if (!FileExists(gtaExe))
    {
        MessageDlg("GTA: San Andreas executable not found.\n(" + gtaExe + ")\n\nPlease locate it now.", mtError,
                   TMsgDlgButtons() << mbOK, 0);
        Form->GetGTAExe(Form->Handle);
        gtaExe = CSettings::GetGtaExecutable();
    }

    if (!FileExists(gtaExe))
    {
        MessageDlg("GTA: San Andreas executable STILL not found.\n(" + gtaExe + ")\n\nAborting launch.", mtError,
                   TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    String gameExe = GetAbsolutePath(gtaExe);
    CSettings::SetGtaExecutable(gameExe, false);

    String sampDll = GetAbsolutePath(ExtractFilePath(gameExe) + "samp.dll");
    if (!FileExists(sampDll))
    {
        MessageDlg("SA-MP library not found.\n(" + sampDll + ")", mtError, TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    String debugScript = GetAbsolutePath(DebugScript);
    if (!debugScript.IsEmpty())
    {
        if (!FileExists(debugScript))
        {
            MessageDlg("Debug script not found.\n(" + debugScript + ")", mtError, TMsgDlgButtons() << mbOK, 0);
            return TGameLaunchResult::Failed;
        }

        if (debugScript.Pos(L"\"") != 0 || debugScript.Length() > 255)
        {
            MessageDlg("Debug script path is invalid or too long.", mtError, TMsgDlgButtons() << mbOK, 0);
            return TGameLaunchResult::Failed;
        }
    }

    // Match samp_debug.exe: gta_sa.exe receives -d directly as its command line.
    // samp.dll parses -d and the optional -l "<script>" from GetCommandLine().
    String commandLine = L"-d";
    if (!debugScript.IsEmpty())
        commandLine += L" -l \"" + debugScript + L"\"";

    String workDir = ExtractFilePath(gameExe);
    try
    {
        GameLaunchThread = new TGameLaunchThread(
            Form->Handle, gameExe, commandLine, workDir, sampDll, CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED);
        GameLaunchThread->Start();
    }
    catch (...)
    {
        delete GameLaunchThread;
        GameLaunchThread = nullptr;
        MessageDlg("Unable to execute.", mtError, TMsgDlgButtons() << mbOK, 0);
        return TGameLaunchResult::Failed;
    }

    return TGameLaunchResult::Started;
}

void CGameLauncher::HandleComplete(TfmMain* Form, TMessage& Message)
{
    if (!Form || GameLaunchShuttingDown)
        return;

    TGameLaunchError error = static_cast<TGameLaunchError>(Message.WParam);
    TGameLaunchThread* thread = GameLaunchThread;
    GameLaunchThread = nullptr;
    if (thread)
    {
        thread->WaitFor();
        delete thread;
    }

    switch (error)
    {
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

void CGameLauncher::Shutdown(TfmMain* Form)
{
    GameLaunchShuttingDown = true;
    GameLaunchDnsPending = false;

    if (!GameLaunchThread)
        return;

    GameLaunchThread->Terminate();
    GameLaunchThread->WaitFor();
    if (Form)
    {
        MSG pendingMessage = {};
        while (PeekMessageW(&pendingMessage, Form->Handle, WM_GAME_LAUNCH_COMPLETE, WM_GAME_LAUNCH_COMPLETE, PM_REMOVE))
        {
        }
    }
    delete GameLaunchThread;
    GameLaunchThread = nullptr;
}

void CGameLauncher::ResumeDnsConnect(TfmMain* Form, const String& Server, const String& Port, const String& Password)
{
    GameLaunchDnsPending = false;
    Connect(Form, Server, Port, Password);
}

void CGameLauncher::CancelDnsConnect()
{
    GameLaunchDnsPending = false;
}
