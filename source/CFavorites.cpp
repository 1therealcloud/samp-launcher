/*
* CFavorites.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "CFavorites.h"
#include "CServerList.h"
#include "CServerQuery.h"
#include "CSettings.h"
#include "globals.h"
#include "main.h"

#include <set>
#include <string>
#include <vector>

static const AnsiChar FileTag[4] = {'S', 'A', 'M', 'P'};
static const int MAX_FAVORITE_SERVERS = 100000;
static const int MAX_FAVORITE_ADDRESS_LENGTH = 1024;
static const int MAX_FAVORITE_HOSTNAME_LENGTH = 4096;
static const int MAX_FAVORITE_PASSWORD_LENGTH = 4096;
static const LONGLONG MAX_FAVORITES_FILE_SIZE = 64LL * 1024 * 1024;
static bool FavoritesChanged = false;

static bool ReadExact(HANDLE File, void* Buffer, DWORD Size)
{
    DWORD bytesRead = 0;
    return ReadFile(File, Buffer, Size, &bytesRead, nullptr) && bytesRead == Size;
}

static bool ReadFavoriteString(HANDLE File, AnsiString& Value, int MaxLength)
{
    int length = 0;
    if (!ReadExact(File, &length, sizeof(length)) || length < 0 || length > MaxLength)
        return false;

    if (length == 0)
    {
        Value = AnsiString();
        return true;
    }

    LARGE_INTEGER currentPosition = {};
    LARGE_INTEGER zero = {};
    LARGE_INTEGER fileSize = {};
    if (!SetFilePointerEx(File, zero, &currentPosition, FILE_CURRENT) || !GetFileSizeEx(File, &fileSize) ||
        fileSize.QuadPart - currentPosition.QuadPart < length)
        return false;

    Value.SetLength(length);
    return ReadExact(File, &Value[1], (DWORD)length);
}

static bool WriteExact(HANDLE File, const void* Buffer, DWORD Size)
{
    DWORD bytesWritten = 0;
    return WriteFile(File, Buffer, Size, &bytesWritten, nullptr) && bytesWritten == Size;
}

static bool WriteFavoriteString(HANDLE File, const AnsiString& Value)
{
    int length = Value.Length();
    if (!WriteExact(File, &length, sizeof(length)))
        return false;
    return length == 0 || WriteExact(File, Value.c_str(), (DWORD)length);
}

void CFavorites::Import(TfmMain* Form, const String& FileName, bool AddToFavorites)
{
    if (!Form)
        return;

    HANDLE file = CreateFileW(FileName.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    char tag[4] = {};
    if (!ReadExact(file, tag, sizeof(tag)) || memcmp(tag, FileTag, 4) != 0)
    {
        CloseHandle(file);
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    int version = 0;
    if (!ReadExact(file, &version, sizeof(version)))
    {
        CloseHandle(file);
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }
    if (version != FAVORITES_FILE_VERSION)
    {
        CloseHandle(file);
        MessageDlg("Bad SA-MP favorites file version.\n\nYour client may need updating.", mtError,
                   TMsgDlgButtons() << mbOK, 0);
        return;
    }

    int serverCount = 0;
    if (!ReadExact(file, &serverCount, sizeof(serverCount)))
    {
        CloseHandle(file);
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    LARGE_INTEGER fileSize = {};
    LARGE_INTEGER currentPosition = {};
    LARGE_INTEGER zero = {};
    if (serverCount < 0 || serverCount > MAX_FAVORITE_SERVERS || !GetFileSizeEx(file, &fileSize) ||
        fileSize.QuadPart > MAX_FAVORITES_FILE_SIZE || !SetFilePointerEx(file, zero, &currentPosition, FILE_CURRENT) ||
        fileSize.QuadPart - currentPosition.QuadPart < (LONGLONG)serverCount * 20)
    {
        CloseHandle(file);
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    std::vector<TServerInfo> imported;
    imported.reserve(serverCount);
    bool valid = true;

    for (int i = 0; i < serverCount; i++)
    {
        TServerInfo server = {};
        if (!ReadFavoriteString(file, server.Address, MAX_FAVORITE_ADDRESS_LENGTH) ||
            !ReadExact(file, &server.Port, sizeof(server.Port)) ||
            !ReadFavoriteString(file, server.HostName, MAX_FAVORITE_HOSTNAME_LENGTH) ||
            !ReadFavoriteString(file, server.ServerPassword, MAX_FAVORITE_PASSWORD_LENGTH) ||
            !ReadFavoriteString(file, server.RconPassword, MAX_FAVORITE_PASSWORD_LENGTH))
        {
            valid = false;
            break;
        }

        if (!CServerList::IsValidEndpoint(String(server.Address), server.Port))
        {
            valid = false;
            break;
        }

        server.Ping = 9999;
        server.Tag = (WORD)random(0xFFFF);
        imported.push_back(server);
    }

    CloseHandle(file);
    if (!valid)
    {
        MessageDlg("Invalid SA-MP file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    if (!AddToFavorites)
    {
        CServerQuery::ClearQueue();
        Servers.clear();
    }

    std::set<std::string> endpoints;
    for (int i = 0; i < (int)Servers.size(); i++)
        endpoints.insert(CServerList::EndpointKey(Servers[i].Address, Servers[i].Port));

    for (int i = 0; i < (int)imported.size(); i++)
    {
        std::string endpoint = CServerList::EndpointKey(imported[i].Address, imported[i].Port);
        if (!endpoints.insert(endpoint).second)
            continue;

        Servers.push_back(imported[i]);
        int index = (int)Servers.size() - 1;
        CServerQuery::Enqueue(String(Servers[index].Address) + ":" + IntToStr(Servers[index].Port) + "#" +
                              IntToStr(Servers[index].Tag));
    }

    CServerList::MarkOrderDirty();
    CServerList::RebuildLookup();
    Form->UpdateServers();
}

bool CFavorites::Export(const String& FileName, bool ExportPasswords)
{
    if (FileName.IsEmpty())
        return false;

    String tempFileName = FileName + ".tmp";
    HANDLE file =
        CreateFileW(tempFileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        MessageDlg("Unable to save SA-MP favorites file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return false;
    }

    bool saveServerPassword = ExportPasswords && CSettings::ReadBool(L"SaveServPasses", false);
    bool saveRconPassword = ExportPasswords && CSettings::ReadBool(L"SaveRconPasses", false);

    bool success = WriteExact(file, FileTag, 4);
    int version = FAVORITES_FILE_VERSION;
    success = success && WriteExact(file, &version, sizeof(version));
    int count = (int)Servers.size();
    success = success && WriteExact(file, &count, sizeof(count));

    for (int i = 0; success && i < count; i++)
    {
        AnsiString empty;
        success = WriteFavoriteString(file, Servers[i].Address) &&
                  WriteExact(file, &Servers[i].Port, sizeof(Servers[i].Port)) &&
                  WriteFavoriteString(file, Servers[i].HostName) &&
                  WriteFavoriteString(file, saveServerPassword ? Servers[i].ServerPassword : empty) &&
                  WriteFavoriteString(file, saveRconPassword ? Servers[i].RconPassword : empty);
    }

    if (success)
        success = FlushFileBuffers(file) != FALSE;
    CloseHandle(file);

    if (!success ||
        !MoveFileExW(tempFileName.c_str(), FileName.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        DeleteFileW(tempFileName.c_str());
        MessageDlg("Unable to save SA-MP favorites file.", mtError, TMsgDlgButtons() << mbOK, 0);
        return false;
    }

    return true;
}

void CFavorites::SaveNow()
{
    FavoritesChanged = !Export(CSettings::GetUserDataFileName(), true);
}

bool CFavorites::IsChanged()
{
    return FavoritesChanged;
}

void CFavorites::SetChanged(bool Changed)
{
    FavoritesChanged = Changed;
}
