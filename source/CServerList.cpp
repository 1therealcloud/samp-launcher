/*
* CServerList.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "CServerList.h"
#include "CDnsResolver.h"
#include "CFavorites.h"
#include "CServerQuery.h"
#include "CSettings.h"
#include "globals.h"
#include "main.h"

#include <algorithm>
#include <map>
#include <string>

#include <winsock2.h>

std::vector<TServerInfo> Servers;
TSortMode SortMode = TSortMode::smHostName;
TSortMode OldSortMode = TSortMode::smHostName;
TSortDir SortDir = TSortDir::sdUp;
TSortDir OldSortDir = TSortDir::sdUp;

static std::map<std::string, int> ServerLookup;
static bool ServerListDirty = false;
static bool ServerOrderDirty = true;

static bool HasUnsafeCommandLineCharacters(const String& Value)
{
    for (int i = 1; i <= Value.Length(); i++)
    {
        if (Value[i] <= 32 || Value[i] == '"')
            return true;
    }
    return false;
}

bool CServerList::Add(TfmMain* Form, const String& Value)
{
    if (!Form)
        return false;

    String serverText = Value.Trim();
    String address;
    int port = 7777;
    int colon = serverText.Pos(":");
    if (colon > 0)
    {
        address = serverText.SubString(1, colon - 1).Trim();
        String portText = serverText.SubString(colon + 1, serverText.Length() - colon).Trim();
        port = StrToIntDef(portText, -1);
    }
    else
    {
        address = serverText;
    }

    if (!IsValidEndpoint(address, port))
    {
        MessageDlg("Invalid server address or port.", mtError, TMsgDlgButtons() << mbOK, 0);
        return false;
    }

    std::string endpoint = EndpointKey(AnsiString(address), port);
    for (int i = 0; i < (int)Servers.size(); i++)
    {
        if (EndpointKey(Servers[i].Address, Servers[i].Port) == endpoint)
        {
            MessageDlg("This server is already on your list.", mtError, TMsgDlgButtons() << mbOK, 0);
            return false;
        }
    }

    TServerInfo server = {};
    server.Address = AnsiString(address);
    server.Port = port;
    server.HostName = AnsiString("(Retrieving info...) " + address + ":" + IntToStr(port));
    server.Ping = 9999;
    server.Tag = (WORD)random(0xFFFF);
    Servers.push_back(server);

    CServerQuery::Enqueue(address + ":" + IntToStr(port) + "#" + IntToStr(server.Tag));
    CFavorites::SetChanged(!CFavorites::Export(CSettings::GetUserDataFileName(), true));
    MarkOrderDirty();
    RebuildLookup();
    Form->UpdateServers();
    return true;
}

bool CServerList::Remove(int Index)
{
    if (Index < 0 || Index >= (int)Servers.size())
        return false;

    Servers.erase(Servers.begin() + Index);
    MarkOrderDirty();
    RebuildLookup();
    return true;
}

std::string CServerList::ServerKey(const AnsiString& Address, WORD Port, WORD Tag)
{
    AnsiString key;
    key.sprintf("%s:%u#%u", Address.c_str(), (unsigned)Port, (unsigned)Tag);
    return std::string(key.c_str(), key.Length());
}

std::string CServerList::EndpointKey(const AnsiString& Address, int Port)
{
    AnsiString lower = AnsiLowerCase(Address);
    AnsiString key;
    key.sprintf("%s:%d", lower.c_str(), Port);
    return std::string(key.c_str(), key.Length());
}

bool CServerList::IsValidEndpoint(const String& Address, int Port)
{
    return !Address.IsEmpty() && Address.Length() <= 253 && Address.Pos("#") == 0 && Address.Pos("/") == 0 &&
           !HasUnsafeCommandLineCharacters(Address) && Port >= 1 && Port <= 65535;
}

void CServerList::RebuildLookup()
{
    ServerLookup.clear();
    for (int i = 0; i < (int)Servers.size(); i++)
    {
        TServerInfo& server = Servers[i];
        WORD tag = server.Tag != 0 ? server.Tag : (WORD)server.Port;
        AnsiString numeric = server.Address;
        unsigned long address = inet_addr(numeric.c_str());
        if (address == INADDR_NONE && numeric != "255.255.255.255")
        {
            String cached = CDnsResolver::GetCached(String(server.Address));
            if (!cached.IsEmpty())
            {
                server.DottedAddress = AnsiString(cached);
                server.HasAddress = true;
                numeric = server.DottedAddress;
            }
        }
        if (!numeric.IsEmpty())
            ServerLookup[ServerKey(numeric, (WORD)server.Port, tag)] = i;
        if (!server.DottedAddress.IsEmpty())
            ServerLookup[ServerKey(server.DottedAddress, (WORD)server.Port, tag)] = i;
    }
}

int CServerList::FindByPacket(const AnsiString& Address, WORD Port, WORD Tag)
{
    std::map<std::string, int>::const_iterator found = ServerLookup.find(ServerKey(Address, Port, Tag));
    return found == ServerLookup.end() ? -1 : found->second;
}

void CServerList::MarkOrderDirty()
{
    ServerOrderDirty = true;
}

bool CServerList::IsOrderDirty()
{
    return ServerOrderDirty;
}

void CServerList::Sort()
{
    if (!ServerOrderDirty && OldSortMode == SortMode && OldSortDir == SortDir)
        return;

    switch (SortMode)
    {
        case TSortMode::smHostName:
            std::sort(Servers.begin(), Servers.end(), [](const TServerInfo& a, const TServerInfo& b) {
                return AnsiCompareText(a.HostName, b.HostName) < 0;
            });
            break;
        case TSortMode::smPlayers:
            std::sort(Servers.begin(), Servers.end(), [](const TServerInfo& a, const TServerInfo& b) {
                return a.Players < b.Players;
            });
            break;
        case TSortMode::smPing:
            std::sort(Servers.begin(), Servers.end(), [](const TServerInfo& a, const TServerInfo& b) {
                return a.Ping < b.Ping;
            });
            break;
        case TSortMode::smMode:
            std::sort(Servers.begin(), Servers.end(), [](const TServerInfo& a, const TServerInfo& b) {
                return AnsiCompareText(a.Mode, b.Mode) < 0;
            });
            break;
        case TSortMode::smMap:
            std::sort(Servers.begin(), Servers.end(), [](const TServerInfo& a, const TServerInfo& b) {
                return AnsiCompareText(a.Map, b.Map) < 0;
            });
            break;
    }

    OldSortMode = SortMode;
    OldSortDir = SortDir;
    ServerOrderDirty = false;
}

void CServerList::MarkDirty()
{
    ServerListDirty = true;
}

bool CServerList::ConsumeDirty()
{
    bool dirty = ServerListDirty;
    ServerListDirty = false;
    return dirty;
}
