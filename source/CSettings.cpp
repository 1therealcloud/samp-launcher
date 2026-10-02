/*
* CSettings.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "CSettings.h"
#include <shlobj.h>

static String gta_sa_exe;

void CSettings::Load(String& PlayerName)
{
    HKEY key = nullptr;
    wchar_t buffer[512] = {};
    DWORD size = 0;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\SAMP", 0, KEY_READ, &key) != ERROR_SUCCESS)
        return;

    size = sizeof(buffer) - sizeof(wchar_t);
    if (RegQueryValueExW(key, L"gta_sa_exe", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &size) ==
        ERROR_SUCCESS)
        gta_sa_exe = buffer;

    ZeroMemory(buffer, sizeof(buffer));
    size = sizeof(buffer) - sizeof(wchar_t);
    if (RegQueryValueExW(key, L"PlayerName", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &size) ==
        ERROR_SUCCESS)
        PlayerName = buffer;

    RegCloseKey(key);
}

void CSettings::SavePlayerName(const String& PlayerName)
{
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\SAMP", 0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) !=
        ERROR_SUCCESS)
        return;

    RegSetValueExW(key, L"PlayerName", 0, REG_SZ, reinterpret_cast<const BYTE*>(PlayerName.c_str()),
                   (PlayerName.Length() + 1) * sizeof(wchar_t));
    RegCloseKey(key);
}

String CSettings::GetGtaExecutable()
{
    return gta_sa_exe;
}

void CSettings::SetGtaExecutable(const String& Path, bool Persist)
{
    gta_sa_exe = Path;
    if (Persist)
        WriteString(L"gta_sa_exe", Path);
}

bool CSettings::ReadBool(const String& Name, bool DefaultValue)
{
    bool result = DefaultValue;
    TRegistry* registry = new TRegistry();
    try
    {
        registry->RootKey = HKEY_CURRENT_USER;
        if (registry->OpenKeyReadOnly(L"SOFTWARE\\SAMP"))
        {
            if (registry->ValueExists(Name))
                result = registry->ReadBool(Name);
            registry->CloseKey();
        }
    }
    __finally
    {
        delete registry;
    }
    return result;
}

String CSettings::ReadString(const String& Name, const String& DefaultValue)
{
    String result = DefaultValue;
    TRegistry* registry = new TRegistry();
    try
    {
        registry->RootKey = HKEY_CURRENT_USER;
        if (registry->OpenKeyReadOnly(L"SOFTWARE\\SAMP"))
        {
            if (registry->ValueExists(Name))
                result = registry->ReadString(Name);
            registry->CloseKey();
        }
    }
    __finally
    {
        delete registry;
    }
    return result;
}

void CSettings::WriteBool(const String& Name, bool Value)
{
    TRegistry* registry = new TRegistry();
    try
    {
        registry->RootKey = HKEY_CURRENT_USER;
        if (registry->OpenKey(L"SOFTWARE\\SAMP", true))
        {
            registry->WriteBool(Name, Value);
            registry->CloseKey();
        }
    }
    __finally
    {
        delete registry;
    }
}

void CSettings::WriteString(const String& Name, const String& Value)
{
    TRegistry* registry = new TRegistry();
    try
    {
        registry->RootKey = HKEY_CURRENT_USER;
        if (registry->OpenKey(L"SOFTWARE\\SAMP", true))
        {
            registry->WriteString(Name, Value);
            registry->CloseKey();
        }
    }
    __finally
    {
        delete registry;
    }
}

String CSettings::GetUserFilesPath()
{
    wchar_t path[MAX_PATH] = {};
    if (!SHGetSpecialFolderPathW(nullptr, path, CSIDL_PERSONAL, false))
        return "";
    return String(path) + "\\GTA San Andreas User Files\\SAMP\\";
}

String CSettings::GetUserDataFileName()
{
    String path = GetUserFilesPath();
    return path.IsEmpty() ? String() : path + "USERDATA.DAT";
}

bool CSettings::EnsureUserFilesFolder()
{
    String path = GetUserFilesPath();
    if (path.IsEmpty())
        return false;

    int result = SHCreateDirectoryExW(nullptr, path.c_str(), nullptr);
    return result == ERROR_SUCCESS || result == ERROR_ALREADY_EXISTS || result == ERROR_FILE_EXISTS;
}
