/*
* globals.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

#include <vector>

// defines
#define WM_RECV          (WM_USER + 1)
#define WM_DNS_RECV      (WM_USER + 2)
#define WM_GAME_LAUNCH_COMPLETE (WM_USER + 3)
#define WM_QUERY_BATCH_READY (WM_USER + 4)
#define FAVORITES_FILE_VERSION  1

// structs
struct TPlayerInfo
{
    AnsiString Name;
    int        Score;
};

struct TRuleInfo
{
    AnsiString Rule;
    AnsiString Value;
};

struct TServerInfo
{
    AnsiString  Address;
    AnsiString  DottedAddress;
    bool        HasAddress;
    int         Port;
    WORD        Tag;

    AnsiString  HostName;
    bool        Passworded;
    int         Players;
    int         MaxPlayers;
    int         Ping;
    AnsiString  Mode;
    AnsiString  Map;
    bool        QueryInfoReceived;
    bool        QueryPingReceived;
    bool        QueryCompleted;

    AnsiString  ServerPassword;
    AnsiString  RconPassword;

	std::vector<TPlayerInfo> aPlayers;
	std::vector<TRuleInfo>   aRules;
};

enum class TSortMode { smHostName, smPlayers, smPing, smMode, smMap };
enum class TSortDir  { sdUp, sdDown };
enum class TGameLaunchResult { Failed, DnsPending, Started };

// global vars
extern std::vector<TServerInfo> Servers;
extern bool        Filtered;
extern TSortMode   SortMode;
extern TSortMode   OldSortMode;
extern TSortDir    SortDir;
extern TSortDir    OldSortDir;
extern int         QuerySocket;
extern AnsiChar    FileTag[4];
extern int         SelServer;
extern int         PingCounter;
extern int         MasterFile;
extern String      gta_sa_exe;
extern TStringList *QueryQueue;
extern int         ServersTopIndex;
extern bool        InstanceChecked;
extern TStringList *IPList;
extern bool        FavoritesChanged;
