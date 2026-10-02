/*
* main.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

#pragma hdrstop

#include "globals.h"

class TfmMain : public TForm
{
    __published :
        // status bar
        TStatusBar* sbMain;
    TMainMenu* mmMain;
    TMenuItem* miFile;
    TMenuItem* miExportFavoritesList;
    TMenuItem* N1;
    TMenuItem* miExit;
    TMenuItem* miServers;
    TMenuItem* miAddServer;
    TMenuItem* miHelp;
    TMenuItem* miAbout;

    // toolbar
    TToolBar* tbMain;
    TToolButton* tbSettings;
    TToolButton* tbCopyServerInfo;
    TToolButton* tbMasterServerUpdate;
    TToolButton* tbAbout;
    TToolButton* tbHelp;
    TToolButton* tbConnect;
    TToolButton* tbRefreshServer;
    TToolButton* tbAddServer;
    TToolButton* tbDeleteServer;
    TToolButton* tbServerProperties;
    TToolButton* tbSpacer1;
    TToolButton* tbSpacer2;
    TToolButton* tbSpacer3;
    TToolButton* tbSpacer4;
    TToolButton* tbSpacer5;
    TToolButton* ToolButton1;

    // other menu
    TMenuItem* miTools;
    TPopupMenu* pmServers;
    TTabSet* tsServerLists;
    TMenuItem* miConnect;
    TMenuItem* N2;
    TMenuItem* miDeleteServer;
    TMenuItem* miRefreshServer;
    TMenuItem* N3;
    TMenuItem* miMasterServerUpdate;
    TMenuItem* N4;
    TMenuItem* miCopyServerInfo;
    TMenuItem* miServerProperties;
    TMenuItem* miSettings;
    TMenuItem* miHelpTopics;
    TMenuItem* N6;
    TMenuItem* miImportFavoritesList;
    TMenuItem* piConnect;
    TMenuItem* N7;
    TMenuItem* piDeleteServer;
    TMenuItem* piRefreshServer;
    TMenuItem* N9;
    TMenuItem* piCopyServerInfo;
    TMenuItem* piServerProperties;
    TMenuItem* miView;
    TMenuItem* miFilterServerInfo;
    TMenuItem* N10;
    TMenuItem* miStatusBar;
    TMenuItem* N11;
    TMenuItem* miSamp;
    TMenuItem* AddtoFavorites1;
    TMenuItem* RemoteConsole;

    // panels and splitters
    TPanel* pnBreakable;
    TPanel* pnLine;
    TPanel* pnRight;
    TPanel* pnPlayers;
    TPanel* pnRules;
    TPanel* pnMain;
    TSplitter* spRight;
    TSplitter* Splitter1;

    // filter
    TGroupBox* gbFilter;
    TLabeledEdit* edFilterMode;
    TLabeledEdit* edFilterMap;
    TCheckBox* cbFilterEmpty;
    TCheckBox* cbFilterPassworded;
    TCheckBox* cbFilterFull;

    // server info
    TGroupBox* gbInfo;
    TLabel* lbSIAddressLab;
    TLabel* lbSIModeLab;
    TLabel* lbSIMapLab;
    TLabel* lbSIPlayersLab;
    TLabel* lbSIPingLab;
    TLabel* lbSIPing;
    TLabel* lbSIPlayers;
    TLabel* lbSIMap;
    TLabel* lbSIMode;
    TEdit* edSIAddress;
    TChart* chSIPingChart;
    TFastLineSeries* chSIPingLineChart;
    TTimer* tmSIPingUpdate;

    // server list
    THeaderControl* hcServers;
    TListBox* lbServers;

    // players
    TListBox* lbPlayers;
    THeaderControl* hcPlayers;

    // rules
    TListBox* lbRules;
    THeaderControl* hcRules;

    // other
    TPopupMenu* pmCopy;
    TMenuItem* piCopy;
    TLabel* lblPlayerName;
    TTimer* tmrQueryQueueProcess;
    TEdit* edName;
    TLabel* label_url;
    TVirtualImage* imLogo;
    TTimer* tmrServerListUpdate;
    TImageCollection* ImageCollection1;
    TVirtualImageList* VirtualImageList1;

    // Event Handlers

    // forms
    void __fastcall FormCreate(TObject* Sender);
    void __fastcall FormDestroy(TObject* Sender);
    void __fastcall FormShow(TObject* Sender);
    void __fastcall FormResize(TObject* Sender);

    // server list
    void __fastcall lbServersDrawItem(TWinControl* Control, int Index, TRect& Rect, TOwnerDrawState State);
    void __fastcall hcServersSectionResize(THeaderControl* HeaderControl, THeaderSection* Section);
    void __fastcall hcServersSectionClick(THeaderControl* HeaderControl, THeaderSection* Section);
    void __fastcall hcServersDrawSection(THeaderControl* HeaderControl, THeaderSection* Section, const TRect& Rect,
                                         bool Pressed);
    void __fastcall lbServersClick(TObject* Sender);
    void __fastcall lbServersContextPopup(TObject* Sender, TPoint& MousePos, bool& Handled);

    // players / rules
    void __fastcall lbPlayersDrawItem(TWinControl* Control, int Index, TRect& Rect, TOwnerDrawState State);
    void __fastcall hcPlayersSectionResize(THeaderControl* HeaderControl, THeaderSection* Section);
    void __fastcall lbRulesDrawItem(TWinControl* Control, int Index, TRect& Rect, TOwnerDrawState State);
    void __fastcall hcRulesSectionResize(THeaderControl* HeaderControl, THeaderSection* Section);
    void __fastcall lbPlayersExit(TObject* Sender);
    void __fastcall lbRulesExit(TObject* Sender);

    // filter
    void __fastcall FilterChange(TObject* Sender);

    // timers
    void __fastcall tmSIPingUpdateTimer(TObject* Sender);
    void __fastcall tmrQueryQueueProcessTimer(TObject* Sender);
    void __fastcall tmServerListUpdate(TObject* Sender);

    // menu / buttons / panels
    void __fastcall tbMasterServerUpdateClick(TObject* Sender); // master server update button

    void __fastcall ImportFavoritesClick(TObject* Sender);
    void __fastcall ExportFavoritesClick(TObject* Sender);
    void __fastcall ExitClick(TObject* Sender);
    void __fastcall miViewClick(TObject* Sender);
    void __fastcall ToggleFilterServerInfo(TObject* Sender);
    void __fastcall ToggleStatusBar(TObject* Sender);
    void __fastcall ConnectClick(TObject* Sender);
    void __fastcall AddServerClick(TObject* Sender);
    void __fastcall DeleteServerClick(TObject* Sender);
    void __fastcall RefreshServerClick(TObject* Sender);
    void __fastcall MasterServerUpdateClick(TObject* Sender);
    void __fastcall CopyServerInfoClick(TObject* Sender);
    void __fastcall ServerPropertiesClick(TObject* Sender);
    void __fastcall SettingsClick(TObject* Sender);
    void __fastcall RemoteConsoleClick(TObject* Sender);
    void __fastcall HelpTopicsClick(TObject* Sender);
    void __fastcall AboutClick(TObject* Sender);
    void __fastcall miSampClick(TObject* Sender);
    void __fastcall label_urlClick(TObject* Sender);
    void __fastcall imLogoClick(TObject* Sender);

    //void __fastcall CreateFASTDesktoplink1Click(TObject *Sender);

    void __fastcall tbMainResize(TObject* Sender);
    void __fastcall pnBreakableResize(TObject* Sender);
    void __fastcall sbMainDrawPanel(TStatusBar* StatusBar, TStatusPanel* Panel, const TRect& Rect);
    void __fastcall piCopyClick(TObject* Sender);
    void __fastcall pmCopyPopup(TObject* Sender);
    void __fastcall tsServerListsChange(TObject* Sender, int NewTab, bool& AllowChange);

public:
    __fastcall TfmMain(TComponent* Owner);

    // utils
    String __fastcall GetToken(String TokenData, int ItemIndex, String TokenDelimiter);
    String __fastcall GetClipBoardStr();
    void __fastcall SetClipBoardStr(String Str);
    void __fastcall GetGTAExe(HWND Owner);
    bool __fastcall BrowseForFolder(HWND Owner, String& Directory, String StartDir, String Title);

    // server logic
    void __fastcall UpdateServers();
    void __fastcall AddServer(String Server);
    void __fastcall ImportFavorites(String FileName, bool AddToFavs);
    bool __fastcall ExportFavorites(String FileName, bool ExportPasswords);
    void __fastcall SaveFavoritesNow();

    // network
    void __fastcall QueryServerInfo(String Server, bool bPing, bool bInfo, bool bPlayers, bool bRules);
    TGameLaunchResult __fastcall ServerConnect(String Server, String Port, String Password);

    // WndPro
    void __fastcall WMRecv(TMessage& Message);
    void __fastcall WMDnsRecv(TMessage& Message);
    void __fastcall WMGameLaunchComplete(TMessage& Message);
    void __fastcall WMQueryBatchReady(TMessage& Message);
    BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_RECV, TMessage, WMRecv)
    MESSAGE_HANDLER(WM_DNS_RECV, TMessage, WMDnsRecv)
    MESSAGE_HANDLER(WM_GAME_LAUNCH_COMPLETE, TMessage, WMGameLaunchComplete)
    MESSAGE_HANDLER(WM_QUERY_BATCH_READY, TMessage, WMQueryBatchReady)
    END_MESSAGE_MAP(TForm)
};

extern PACKAGE TfmMain* fmMain;
