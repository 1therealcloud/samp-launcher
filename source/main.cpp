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
#include "CDnsResolver.h"
#include "CFavorites.h"
#include "CGameLauncher.h"
#include "CMasterServer.h"
#include "CServerList.h"
#include "CServerQuery.h"
#include "CSettings.h"
#include "exportfavorites.h"
#include "importfavorites.h"
#include "main.h"
#include "rconconfig.h"
#include "serverproperties.h"
#include "settings.h"
#include "unit_webrunform.h"

#include <string>
#include <shlobj.h>

#pragma package(smart_init)
#pragma resource "..\\dfm\\main.dfm"

bool Filtered = true;
bool InstanceChecked = false;
int SelServer = -1;
int PingCounter = 0;
int MasterFile = 1;
int ServersTopIndex = -1;

static std::string PingChartEndpoint;

TfmMain* fmMain;

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

static bool SameStrings(TStrings* Left, TStrings* Right)
{
    if (!Left || !Right || Left->Count != Right->Count)
        return false;
    for (int i = 0; i < Left->Count; i++)
    {
        if (Left->Strings[i] != Right->Strings[i])
            return false;
    }
    return true;
}

static void CheckAnotherInstance()
{
    if (!InstanceChecked)
    {
        // Pascal mutex
        CreateMutex(nullptr, false, L"kyeman and spookie woz 'ere, innit.");
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            MessageBoxW(0, L"SA:MP is already running.\n\nYou can only run one instance at a time.", L"SA:MP Error",
                        MB_ICONERROR);
            ExitProcess(0);
        }
        InstanceChecked = true;
    }
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

    if (len > 0)
    {
        while (i <= len)
        {
            if (tokenCount == (ItemIndex - 1))
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
    DWORD bufSize = sizeof(buf);
    HKEY hKey = nullptr;

    // read registry
    HKEY roots[] = {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE};
    for (int r = 0; r < 2 && tmpStr.IsEmpty(); r++)
    {
        if (RegOpenKeyExW(roots[r], L"SOFTWARE\\Rockstar Games\\GTA San Andreas\\Installation", 0, KEY_READ, &hKey) ==
            ERROR_SUCCESS)
        {
            bufSize = sizeof(buf);
            if (RegQueryValueExW(hKey, L"ExePath", nullptr, nullptr, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
                tmpStr = buf;
            RegCloseKey(hKey);
            hKey = nullptr;
        }
    }

    tmpStr = tmpStr.Trim();
    if (tmpStr.Length() >= 2 && tmpStr[1] == '"' && tmpStr[tmpStr.Length()] == '"')
        tmpStr = tmpStr.SubString(2, tmpStr.Length() - 2);
    if (!tmpStr.IsEmpty())
        tmpStr = ExtractFilePath(tmpStr);

    if (BrowseForFolder(Owner, browseExe, tmpStr, "Please locate your GTA: San Andreas installation..."))
    {
        CSettings::SetGtaExecutable(browseExe + "\\gta_sa.exe");
    }
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED)
    {
        SetWindowTextW(hwnd, L"GTA: San Andreas Installation");
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

// forms

__fastcall TfmMain::TfmMain(TComponent* Owner) : TForm(Owner)
{}

void __fastcall TfmMain::FormCreate(TObject* Sender)
{
    String playerName;
    CSettings::Load(playerName);
    edName->Text = playerName;
    CFavorites::SetChanged(false);

    if (CSettings::GetGtaExecutable().IsEmpty())
        GetGTAExe(Handle);

    Randomize();

    if (!CServerQuery::Initialize(Handle))
        MessageDlg("Unable to initialize the server query socket.", mtError, TMsgDlgButtons() << mbOK, 0);

    tbMasterServerUpdate->OnClick = tbMasterServerUpdateClick;
    tmrQueryQueueProcess->Enabled = CServerQuery::IsReady();
    tmrServerListUpdate->Enabled = true;

    CSettings::EnsureUserFilesFolder();

    bool dummy = true;
    tsServerListsChange(this, 0, dummy);
    lbServersClick(this);
    UpdateServers();
}

void __fastcall TfmMain::FormDestroy(TObject* Sender)
{
    tmrQueryQueueProcess->Enabled = false;
    tmrServerListUpdate->Enabled = false;
    tmSIPingUpdate->Enabled = false;

    if (tsServerLists->TabIndex == 0 && CFavorites::IsChanged())
        CFavorites::SaveNow();

    CSettings::SavePlayerName(edName->Text);
    CMasterServer::Shutdown(this);
    CDnsResolver::Shutdown();
    CGameLauncher::Shutdown(this);
    CServerQuery::Shutdown();
}

void __fastcall TfmMain::FormShow(TObject* Sender)
{
    lbServers->DoubleBuffered = true;
    lbPlayers->DoubleBuffered = true;
    lbRules->DoubleBuffered = true;
    lbServers->SetFocus();

    //  samp://host:port/password
    if (ParamCount() > 0)
    {
        String servFull = ParamStr(1);
        String servPass = (ParamCount() > 1) ? ParamStr(2) : "";

        if (servFull.SubString(1, 7).LowerCase() == "samp://")
        {
            servFull = servFull.SubString(8, servFull.Length() - 7);
            int slashPos = servFull.Pos("/");
            if (slashPos > 0)
            {
                if (servPass.IsEmpty())
                    servPass = servFull.SubString(slashPos + 1, servFull.Length() - slashPos);
                servFull = servFull.SubString(1, slashPos - 1);
            }
            String servAddr, servPort;
            int colonPos = servFull.Pos(":");
            if (colonPos > 0)
            {
                servAddr = servFull.SubString(1, colonPos - 1);
                servPort = servFull.SubString(colonPos + 1, servFull.Length() - colonPos);
                servPort = IntToStr(StrToIntDef(servPort, 7777));
            }
            else
            {
                servAddr = servFull;
                servPort = "7777";
            }

            if (wnd_webrunform)
            {
                wnd_webrunform->Label1->Caption = "Do you want to add " + servAddr + ":" + servPort +
                                                  " to your favorites \n or play on this server now?";
                switch (wnd_webrunform->ShowModal())
                {
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
        }
        else
        {
            String servAddr, servPort;
            int colonPos = servFull.Pos(":");
            if (colonPos > 0)
            {
                servAddr = servFull.SubString(1, colonPos - 1);
                servPort = IntToStr(StrToIntDef(servFull.SubString(colonPos + 1, 5), 7777));
            }
            else
            {
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

void __fastcall TfmMain::FormResize(TObject* Sender)
{
    imLogo->Left = Width - imLogo->Width - 2;
    imLogo->Repaint();
}

void __fastcall TfmMain::SaveFavoritesNow()
{
    CFavorites::SaveNow();
}

bool __fastcall TfmMain::BrowseForFolder(HWND Owner, String& Directory, String StartDir, String Title)
{
    BROWSEINFOW bi = {};
    wchar_t dispName[MAX_PATH] = {};
    wchar_t tempPath[MAX_PATH] = {};

    bi.hwndOwner = Owner;
    bi.pszDisplayName = dispName;
    bi.lpszTitle = Title.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lParam = (LPARAM)StartDir.c_str();
    bi.lpfn = BrowseCallbackProc;

    PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
    if (pidl)
    {
        bool selected = SHGetPathFromIDListW(pidl, tempPath);
        CoTaskMemFree(pidl);
        if (selected)
        {
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
    TStringList* NewServs = new TStringList();
    int TopIndexes[3];
    int TopIndexSaved = lbServers->TopIndex;

    lbServers->Items->BeginUpdate();

    ServersTopIndex = lbServers->TopIndex;
    TopIndexes[0] = lbServers->TopIndex;
    TopIndexes[1] = lbPlayers->TopIndex;
    TopIndexes[2] = lbRules->TopIndex;

    Idx = -1;
    ActualIdx = lbServers->ItemIndex;
    if (ActualIdx != -1)
    {
        Idx = StrToIntDef(lbServers->Items->Strings[ActualIdx], -1);
        if (Idx >= 0 && Idx < (int)Servers.size())
            OldServ = String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port);
    }

    CServerList::Sort();
    CServerList::RebuildLookup();

    AnsiString modeFilter = AnsiLowerCase(AnsiString(edFilterMode->Text));
    AnsiString mapFilter = AnsiLowerCase(AnsiString(edFilterMap->Text));
    int first = SortDir == TSortDir::sdDown ? (int)Servers.size() - 1 : 0;
    int last = SortDir == TSortDir::sdDown ? -1 : (int)Servers.size();
    int step = SortDir == TSortDir::sdDown ? -1 : 1;
    for (i = first; i != last; i += step)
    {
        ItemFiltered = Servers[i].MaxPlayers < 1 && MasterFile != 0;
        if (Filtered)
        {
            if (!modeFilter.IsEmpty() && AnsiPos(modeFilter, AnsiLowerCase(Servers[i].Mode)) == 0)
                ItemFiltered = true;
            if (!mapFilter.IsEmpty() && AnsiPos(mapFilter, AnsiLowerCase(Servers[i].Map)) == 0)
                ItemFiltered = true;
            if (cbFilterFull->Checked && Servers[i].Players == Servers[i].MaxPlayers)
                ItemFiltered = true;
            if (cbFilterEmpty->Checked && Servers[i].Players == 0)
                ItemFiltered = true;
            if (cbFilterPassworded->Checked && Servers[i].Passworded)
                ItemFiltered = true;
        }
        if (!ItemFiltered)
            NewServs->Add(IntToStr(i));
    }

    if (!SameStrings(lbServers->Items, NewServs))
    {
        lbServers->Items->Assign(NewServs);
        lbServers->TopIndex = TopIndexes[0];
    }

    int newItemIndex = -1;
    if (!OldServ.IsEmpty())
    {
        for (i = 0; i < lbServers->Items->Count; i++)
        {
            Idx = StrToIntDef(lbServers->Items->Strings[i], -1);
            if (Idx >= 0 && Idx < (int)Servers.size())
            {
                if (String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port) == OldServ)
                {
                    newItemIndex = i;
                    break;
                }
            }
        }
    }
    if (newItemIndex == -1 && lbServers->Items->Count > 0)
        newItemIndex = (ActualIdx >= 0 && ActualIdx < lbServers->Items->Count) ? ActualIdx : 0;
    lbServers->ItemIndex = newItemIndex;

    Idx = -1;
    if (lbServers->ItemIndex != -1)
        Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);

    NewServs->Clear();
    if (Idx >= 0 && Idx < (int)Servers.size())
    {
        for (i = 0; i < (int)Servers[Idx].aPlayers.size(); i++)
            NewServs->Add(IntToStr(i));
        if (!SameStrings(lbPlayers->Items, NewServs))
        {
            lbPlayers->Items->Assign(NewServs);
            lbPlayers->TopIndex = TopIndexes[1];
        }

        NewServs->Clear();
        label_url->Caption = "";
        for (i = 0; i < (int)Servers[Idx].aRules.size(); i++)
        {
            NewServs->Add(IntToStr(i));
            if (Servers[Idx].aRules[i].Rule == "weburl")
                label_url->Caption = String(Servers[Idx].aRules[i].Value).Trim();
        }
        if (!SameStrings(lbRules->Items, NewServs))
        {
            lbRules->Items->Assign(NewServs);
            lbRules->TopIndex = TopIndexes[2];
        }

        edSIAddress->Text = String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port);
        lbSIPlayers->Caption = IntToStr(Servers[Idx].Players) + " / " + IntToStr(Servers[Idx].MaxPlayers);
        lbSIPing->Caption = (Servers[Idx].Ping == 9999) ? "-" : IntToStr(Servers[Idx].Ping);
        lbSIMode->Caption = String(Servers[Idx].Mode);
        lbSIMap->Caption = String(Servers[Idx].Map);
        gbInfo->Caption = " Server Info: " + String(Servers[Idx].HostName) + " ";
    }
    else
    {
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
    for (i = 0; i < lbServers->Items->Count; i++)
    {
        Idx = StrToIntDef(lbServers->Items->Strings[i], -1);
        if (Idx >= 0 && Idx < (int)Servers.size())
        {
            TotSlots += Servers[Idx].MaxPlayers;
            TotPlayers += Servers[Idx].Players;
        }
    }
    sbMain->SimpleText = "Servers: " + IntToStr(TotPlayers) + " players, playing on " + IntToStr(TotServers) +
                         " servers. (" + IntToStr(TotSlots) + " player slots available)";

    if (TopIndexSaved >= 0 && TopIndexSaved < lbServers->Items->Count)
        lbServers->TopIndex = TopIndexSaved;

    if (TrackingChanges)
        lbServers->Items->EndUpdate();
}

void __fastcall TfmMain::AddServer(String Server)
{
    CServerList::Add(this, Server);
}

void __fastcall TfmMain::FilterChange(TObject* Sender)
{
    UpdateServers();
}

void __fastcall TfmMain::lbServersClick(TObject* Sender)
{
    std::string selectedEndpoint;
    if (lbServers->ItemIndex >= 0 && lbServers->ItemIndex < lbServers->Items->Count)
    {
        int selectedIdx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
        if (selectedIdx >= 0 && selectedIdx < (int)Servers.size())
            selectedEndpoint = CServerList::EndpointKey(Servers[selectedIdx].Address, Servers[selectedIdx].Port);
    }
    if (selectedEndpoint != PingChartEndpoint)
    {
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

    if (lbServers->ItemIndex == -1)
    {
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

void __fastcall TfmMain::lbServersContextPopup(TObject* Sender, TPoint& MousePos, bool& Handled)
{
    Handled = (lbServers->ItemIndex == -1);
}

void __fastcall TfmMain::lbServersDrawItem(TWinControl* Control, int Index, TRect& Rect, TOwnerDrawState State)
{
    TListBox* lb = static_cast<TListBox*>(Control);
    if (Index < 0 || Index >= lb->Items->Count)
        return;
    int Idx = StrToIntDef(lb->Items->Strings[Index], -1);
    if (Idx < 0 || Idx >= (int)Servers.size())
        return;

    lb->Canvas->Pen->Color = clBtnHighlight;
    lb->Canvas->Pen->Style = psClear;

    if (State.Contains(odSelected))
    {
        lb->Canvas->Font->Color = clHighlightText;
        lb->Canvas->Brush->Color = clHighlight;
    }
    else
    {
        lb->Canvas->Font->Color = clWindowText;
        lb->Canvas->Brush->Color = (Index % 2) ? clWindow : DarkenColor(clWindow, 10);
    }

    Rect.Right++;
    lb->Canvas->Rectangle(Rect);
    Rect.Right--;
    lb->Canvas->Pen->Style = psSolid;
    lb->Canvas->MoveTo(Rect.Right, Rect.Bottom - 1);
    lb->Canvas->LineTo(Rect.Left, Rect.Bottom - 1);

    for (int i = 0; i < hcServers->Sections->Count; i++)
    {
        lb->Canvas->MoveTo(hcServers->Sections->Items[i]->Right - 1, Rect.Top);
        lb->Canvas->LineTo(hcServers->Sections->Items[i]->Right - 1, Rect.Bottom);
    }

    VirtualImageList1->Draw(lb->Canvas, 7, Rect.Top + 1, Servers[Idx].Passworded ? L"imPadlocked" : L"imPadlock");

    TRect TempRect;
    TempRect = TRect(hcServers->Sections->Items[1]->Left + 2, Rect.Top + 2, hcServers->Sections->Items[1]->Right - 2,
                     Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].HostName).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcServers->Sections->Items[2]->Left + 2, Rect.Top + 2, hcServers->Sections->Items[2]->Right - 2,
                     Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, (IntToStr(Servers[Idx].Players) + " / " + IntToStr(Servers[Idx].MaxPlayers)).c_str(),
             -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcServers->Sections->Items[3]->Left + 2, Rect.Top + 2, hcServers->Sections->Items[3]->Right - 2,
                     Rect.Bottom - 2);
    if (Servers[Idx].Ping == 9999)
        DrawText(lb->Canvas->Handle, L"-", -1, &TempRect, DT_LEFT);
    else
        DrawText(lb->Canvas->Handle, IntToStr(Servers[Idx].Ping).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcServers->Sections->Items[4]->Left + 2, Rect.Top + 2, hcServers->Sections->Items[4]->Right - 2,
                     Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].Mode).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcServers->Sections->Items[5]->Left + 2, Rect.Top + 2, hcServers->Sections->Items[5]->Right - 2,
                     Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].Map).c_str(), -1, &TempRect, DT_LEFT);
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::hcServersSectionResize(THeaderControl*, THeaderSection*)
{
    lbServers->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::hcServersSectionClick(THeaderControl*, THeaderSection* Section)
{
    switch (Section->Index)
    {
        case 1:
            if (SortMode == TSortMode::smHostName)
                SortDir = (SortDir == TSortDir::sdUp) ? TSortDir::sdDown : TSortDir::sdUp;
            else
            {
                SortMode = TSortMode::smHostName;
                SortDir = TSortDir::sdUp;
            }
            break;
        case 2:
            if (SortMode == TSortMode::smPlayers)
                SortDir = (SortDir == TSortDir::sdUp) ? TSortDir::sdDown : TSortDir::sdUp;
            else
            {
                SortMode = TSortMode::smPlayers;
                SortDir = TSortDir::sdDown;
            }
            break;
        case 3:
            if (SortMode == TSortMode::smPing)
                SortDir = (SortDir == TSortDir::sdUp) ? TSortDir::sdDown : TSortDir::sdUp;
            else
            {
                SortMode = TSortMode::smPing;
                SortDir = TSortDir::sdUp;
            }
            break;
        case 4:
            if (SortMode == TSortMode::smMode)
                SortDir = (SortDir == TSortDir::sdUp) ? TSortDir::sdDown : TSortDir::sdUp;
            else
            {
                SortMode = TSortMode::smMode;
                SortDir = TSortDir::sdUp;
            }
            break;
        case 5:
            if (SortMode == TSortMode::smMap)
                SortDir = (SortDir == TSortDir::sdUp) ? TSortDir::sdDown : TSortDir::sdUp;
            else
            {
                SortMode = TSortMode::smMap;
                SortDir = TSortDir::sdUp;
            }
            break;
    }
    UpdateServers();
}

static bool ReRender = false;

void __fastcall TfmMain::hcServersDrawSection(THeaderControl* HeaderControl, THeaderSection* Section, const TRect& Rect,
                                              bool Pressed)
{
    bool DoIt = false;
    TRect TempRect = Rect;
    TempRect.Left += 2;
    TempRect.Top += 1;

    if (Section->Index == 1 && SortMode == TSortMode::smHostName)
        DoIt = true;
    else if (Section->Index == 2 && SortMode == TSortMode::smPlayers)
        DoIt = true;
    else if (Section->Index == 3 && SortMode == TSortMode::smPing)
        DoIt = true;
    else if (Section->Index == 4 && SortMode == TSortMode::smMode)
        DoIt = true;
    else if (Section->Index == 5 && SortMode == TSortMode::smMap)
        DoIt = true;

    if (DoIt)
    {
        UnicodeString arrowName = (SortDir == TSortDir::sdDown) ? L"imDownArrow" : L"imUpArrow";
        VirtualImageList1->Draw(HeaderControl->Canvas, Rect.Left + 2, Rect.Top + 2, arrowName);
        TempRect.Left += 10;
    }

    DrawText(HeaderControl->Canvas->Handle, Section->Text.c_str(), -1, &TempRect, DT_LEFT);

    if (!ReRender)
    {
        ReRender = true;
        HeaderControl->Repaint();
        ReRender = false;
    }
}

void __fastcall TfmMain::tbMainResize(TObject* Sender)
{
    ToolButton1->Width = ((TToolBar*)Sender)->Width - ToolButton1->Left - imLogo->Width;
    imLogo->Repaint();
}

void __fastcall TfmMain::pnBreakableResize(TObject* Sender)
{
    gbInfo->Width = pnBreakable->Width - gbFilter->Width + 1;

    chSIPingChart->Width = gbInfo->ClientWidth - chSIPingChart->Left - 16;

    chSIPingChart->Visible = (chSIPingChart->Width > 50);
}

void __fastcall TfmMain::lbPlayersDrawItem(TWinControl* Control, int Index, TRect& Rect, TOwnerDrawState State)
{
    if (lbServers->ItemIndex == -1)
        return;
    TListBox* lb = static_cast<TListBox*>(Control);
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size())
        return;
    if (Index < 0 || Index >= (int)Servers[Idx].aPlayers.size())
        return;

    lb->Canvas->Pen->Color = clBtnHighlight;
    lb->Canvas->Pen->Style = psClear;

    if (State.Contains(odSelected))
    {
        lb->Canvas->Font->Color = clHighlightText;
        lb->Canvas->Brush->Color = clHighlight;
    }
    else
    {
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
    TempRect = TRect(hcPlayers->Sections->Items[0]->Left + 2, Rect.Top + 2, hcPlayers->Sections->Items[0]->Right - 2,
                     Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].aPlayers[Index].Name).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcPlayers->Sections->Items[1]->Left + 2, Rect.Top + 2, hcPlayers->Sections->Items[1]->Right - 2,
                     Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, IntToStr(Servers[Idx].aPlayers[Index].Score).c_str(), -1, &TempRect, DT_LEFT);
}
//---------------------------------------------------------------------------
void __fastcall TfmMain::hcPlayersSectionResize(THeaderControl*, THeaderSection*)
{
    lbPlayers->Repaint();
}

void __fastcall TfmMain::lbRulesDrawItem(TWinControl* Control, int Index, TRect& Rect, TOwnerDrawState State)
{
    if (lbServers->ItemIndex == -1)
        return;
    TListBox* lb = static_cast<TListBox*>(Control);
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size())
        return;
    if (Index < 0 || Index >= (int)Servers[Idx].aRules.size())
        return;

    lb->Canvas->Pen->Color = clBtnHighlight;
    lb->Canvas->Pen->Style = psClear;

    if (State.Contains(odSelected))
    {
        lb->Canvas->Font->Color = clHighlightText;
        lb->Canvas->Brush->Color = clHighlight;
    }
    else
    {
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
    TempRect = TRect(hcRules->Sections->Items[0]->Left + 2, Rect.Top + 2, hcRules->Sections->Items[0]->Right - 2,
                     Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].aRules[Index].Rule).c_str(), -1, &TempRect, DT_LEFT);

    TempRect = TRect(hcRules->Sections->Items[1]->Left + 2, Rect.Top + 2, hcRules->Sections->Items[1]->Right - 2,
                     Rect.Bottom - 2);
    DrawText(lb->Canvas->Handle, String(Servers[Idx].aRules[Index].Value).c_str(), -1, &TempRect, DT_LEFT);
}

void __fastcall TfmMain::hcRulesSectionResize(THeaderControl*, THeaderSection*)
{
    lbRules->Repaint();
}

void __fastcall TfmMain::lbPlayersExit(TObject* Sender)
{
    lbPlayers->ItemIndex = -1;
}

void __fastcall TfmMain::lbRulesExit(TObject* Sender)
{
    lbRules->ItemIndex = -1;
}

void __fastcall TfmMain::sbMainDrawPanel(TStatusBar* StatusBar, TStatusPanel* Panel, const TRect& Rect)
{
    StatusBar->Canvas->Brush->Color = clBtnFace;
    StatusBar->Canvas->Rectangle(Rect);

    StatusBar->Canvas->Brush->Color = (TColor)0x00804000;
    int Pcnt = (int)(((Rect.Width()) / 100.0) * 75.0);
    StatusBar->Canvas->Rectangle(Rect.Left, Rect.Top, Rect.Left + Pcnt, Rect.Bottom);
}

void __fastcall TfmMain::QueryServerInfo(String Server, bool bPing, bool bInfo, bool bPlayers, bool bRules)
{
    CServerQuery::Query(this, Server, bPing, bInfo, bPlayers, bRules);
}

void __fastcall TfmMain::WMRecv(TMessage& Message)
{
    CServerQuery::HandleRecv(this, Message);
}

void __fastcall TfmMain::WMQueryBatchReady(TMessage& Message)
{
    CServerQuery::HandleBatchReady(this, Message);
}

TGameLaunchResult __fastcall TfmMain::ServerConnect(String Server, String Port, String Password)
{
    return CGameLauncher::Connect(this, Server, Port, Password);
}

void __fastcall TfmMain::WMGameLaunchComplete(TMessage& Message)
{
    CGameLauncher::HandleComplete(this, Message);
}

void __fastcall TfmMain::WMDnsRecv(TMessage& Message)
{
    CDnsResolver::HandleMessage(this, Message);
}

void __fastcall TfmMain::tmSIPingUpdateTimer(TObject* Sender)
{
    if (lbServers->ItemIndex == -1)
        return;
    if (GetForegroundWindow() != Handle)
        return;

    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size())
        return;

    bool PingOnly;
    if (PingCounter == 5)
    {
        PingCounter = 0;
        PingOnly = false;
    }
    else
    {
        PingCounter++;
        PingOnly = true;
    }

    String ServerAddr =
        String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port) + "#" + IntToStr(Servers[Idx].Tag);
    if (PingOnly)
        QueryServerInfo(ServerAddr, true, false, false, false);
    else
        QueryServerInfo(ServerAddr, true, true, true, true);
}

void __fastcall TfmMain::tmrQueryQueueProcessTimer(TObject* Sender)
{
    CServerQuery::ProcessQueue(this);
    CMasterServer::CheckQueryBatchComplete(this);
}

void __fastcall TfmMain::tmServerListUpdate(TObject* Sender)
{
    CMasterServer::Tick(this);
}

// Favorites I/O
void __fastcall TfmMain::ImportFavoritesClick(TObject* Sender)
{
    if (tsServerLists->TabIndex != 0)
        tsServerLists->TabIndex = 0;

    TOpenDialog* OD = new TOpenDialog(this);
    OD->DefaultExt = "fav";
    OD->Filter = "SA-MP Favorites List (*.fav)|*.fav";
    OD->Options << ofEnableSizing << ofFileMustExist;
    OD->Title = "Import Favorites";
    if (!OD->Execute())
    {
        delete OD;
        return;
    }

    TfmImportFavorites* fmImport = new TfmImportFavorites(Application);
    if (fmImport->ShowModal() != mrOk)
    {
        delete fmImport;
        delete OD;
        return;
    }
    bool AddToFavs = fmImport->rbAddToCurrent->Checked;
    delete fmImport;

    ImportFavorites(OD->FileName, AddToFavs);
    delete OD;

    CFavorites::SetChanged(!CFavorites::Export(CSettings::GetUserDataFileName(), true));
}

void __fastcall TfmMain::ImportFavorites(String FileName, bool AddToFavs)
{
    CFavorites::Import(this, FileName, AddToFavs);
}

void __fastcall TfmMain::ExportFavoritesClick(TObject* Sender)
{
    if (tsServerLists->TabIndex != 0)
        tsServerLists->TabIndex = 0;

    TSaveDialog* SD = new TSaveDialog(this);
    SD->DefaultExt = "fav";
    SD->Filter = "SA-MP Favorites List (*.fav)|*.fav";
    SD->Options << ofHideReadOnly << ofEnableSizing;
    SD->Title = "Export Favorites";
    if (!SD->Execute())
    {
        delete SD;
        return;
    }
    if (FileExists(SD->FileName))
    {
        if (MessageDlg("File '" + SD->FileName + "' already exists. Overwrite?", mtConfirmation,
                       TMsgDlgButtons() << mbYes << mbNo, 0) != mrYes)
        {
            delete SD;
            return;
        }
    }

    TfmExportFavorites* fmExport = new TfmExportFavorites(Application);
    if (fmExport->ShowModal() != mrOk)
    {
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
    return CFavorites::Export(FileName, ExportPasswords);
}

void __fastcall TfmMain::ExitClick(TObject* Sender)
{
    Close();
}

void __fastcall TfmMain::miViewClick(TObject* Sender)
{
    miFilterServerInfo->Checked = Filtered;
    miStatusBar->Checked = sbMain->Visible;
}

void __fastcall TfmMain::ToggleFilterServerInfo(TObject* Sender)
{
    if (Filtered)
    {
        Filtered = false;
        pnBreakable->Height = 16;
        gbInfo->Caption = " Server Info ";
    }
    else
    {
        Filtered = true;
        pnBreakable->Height = 100;
        pnBreakable->Top = 0;
        if (lbServers->ItemIndex != -1)
        {
            int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
            if (Idx >= 0 && Idx < (int)Servers.size())
                gbInfo->Caption = " Server Info: " + String(Servers[Idx].HostName) + " ";
        }
    }
    UpdateServers();
}

void __fastcall TfmMain::ToggleStatusBar(TObject* Sender)
{
    sbMain->Visible = !sbMain->Visible;
    sbMain->Top = 10000;
}

void __fastcall TfmMain::ConnectClick(TObject* Sender)
{
    if (lbServers->ItemIndex == -1)
        return;
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size())
        return;

    String SrvPwd = String(Servers[Idx].ServerPassword);
    if (Servers[Idx].Passworded)
    {
        if (!InputQuery("Server Password", "This server requires a password...", SrvPwd))
            return;
    }
    if (edName->Text.IsEmpty())
    {
        String NickName;
        if (!InputQuery("Who are you?", "Enter your nickname/handle...", NickName))
            return;
        if (NickName.IsEmpty())
            return;
        edName->Text = NickName;
    }

    ServerConnect(String(Servers[Idx].Address), IntToStr(Servers[Idx].Port), SrvPwd);
}

void __fastcall TfmMain::AddServerClick(TObject* Sender)
{
    String Server = GetClipBoardStr();
    if (MasterFile != 0 && lbServers->ItemIndex != -1)
    {
        int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
        if (Idx >= 0 && Idx < (int)Servers.size())
            Server = String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port);
        tsServerLists->TabIndex = 0;
        if (MasterFile != 0)
        {
            bool allowChange = true;
            tsServerListsChange(tsServerLists, 0, allowChange);
        }
    }
    if (InputQuery("Add Server", "Enter new server HOST:PORT...", Server))
    {
        if (!Server.IsEmpty())
            AddServer(Server);
    }
}

void __fastcall TfmMain::DeleteServerClick(TObject* Sender)
{
    if (lbServers->ItemIndex == -1 || tsServerLists->TabIndex != 0)
        return;

    int index = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (!CServerList::Remove(index))
        return;

    UpdateServers();
    CFavorites::SetChanged(!CFavorites::Export(CSettings::GetUserDataFileName(), true));
}

void __fastcall TfmMain::RefreshServerClick(TObject* Sender)
{
    if (lbServers->ItemIndex == -1)
        return;
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size())
        return;

    QueryServerInfo(String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port) + "#" + IntToStr(Servers[Idx].Tag),
                    true, true, true, true);
}

// "Master server update" button
void __fastcall TfmMain::tbMasterServerUpdateClick(TObject* Sender)
{
    MasterServerUpdateClick(Sender);
}

void __fastcall TfmMain::MasterServerUpdateClick(TObject* Sender)
{
    CMasterServer::Refresh(this);
}

void __fastcall TfmMain::CopyServerInfoClick(TObject* Sender)
{
    if (lbServers->ItemIndex == -1)
        return;
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size())
        return;

    String Str = "HostName: " + String(Servers[Idx].HostName) + "\r\n" + "Address:  " + String(Servers[Idx].Address) +
                 ":" + IntToStr(Servers[Idx].Port) + "\r\n" + "Players:  " + IntToStr(Servers[Idx].Players) + " / " +
                 IntToStr(Servers[Idx].MaxPlayers) + "\r\n" + "Ping:     " + IntToStr(Servers[Idx].Ping) + "\r\n" +
                 "Mode:     " + String(Servers[Idx].Mode) + "\r\n" + "Language: " + String(Servers[Idx].Map);
    SetClipBoardStr(Str);
}

void __fastcall TfmMain::ServerPropertiesClick(TObject* Sender)
{
    if (lbServers->ItemIndex == -1)
        return;
    int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
    if (Idx < 0 || Idx >= (int)Servers.size())
        return;

    TfmServerProperties* fm = new TfmServerProperties(Application);
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

void __fastcall TfmMain::SettingsClick(TObject* Sender)
{
    TfmSettings* fm = new TfmSettings(Application);
    fm->ShowModal();
    delete fm;
}

void __fastcall TfmMain::RemoteConsoleClick(TObject* Sender)
{
    TfmRconConfig* fm = new TfmRconConfig(Application);
    if (lbServers->ItemIndex != -1)
    {
        int Idx = StrToIntDef(lbServers->Items->Strings[lbServers->ItemIndex], -1);
        if (Idx >= 0 && Idx < (int)Servers.size())
        {
            fm->edHost->Text = String(Servers[Idx].Address) + ":" + IntToStr(Servers[Idx].Port);
            fm->edPassword->Text = String(Servers[Idx].RconPassword);
        }
    }
    fm->ShowModal();
    delete fm;
}

void __fastcall TfmMain::HelpTopicsClick(TObject* Sender)
{
    ShellExecuteW(Handle, L"open", L"https://wiki.sa-mp.com/", nullptr, nullptr, SW_SHOWNORMAL);
}

void __fastcall TfmMain::AboutClick(TObject* Sender)
{
    fmAbout->ShowModal();
}

void __fastcall TfmMain::miSampClick(TObject* Sender)
{
    ShellExecuteW(Handle, L"open", L"https://www.sa-mp.com/", nullptr, nullptr, SW_SHOWNORMAL);
}

void __fastcall TfmMain::label_urlClick(TObject* Sender)
{
    ShellExecuteW(0, L"open", (L"http://" + ((TLabel*)Sender)->Caption).c_str(), L"", L"", SW_SHOWNORMAL);
}

void __fastcall TfmMain::imLogoClick(TObject* Sender)
{
    fmAbout->ShowModal();
}

void __fastcall TfmMain::piCopyClick(TObject* Sender)
{
    SetClipBoardStr(edSIAddress->Text);
}

void __fastcall TfmMain::pmCopyPopup(TObject* Sender)
{
    piCopy->Enabled = (edSIAddress->Text != "- - -");
}

void __fastcall TfmMain::tsServerListsChange(TObject* Sender, int NewTab, bool& AllowChange)
{
    int previousMasterFile = MasterFile;
    CMasterServer::Cancel(this);
    CServerQuery::ClearNetwork();
    CDnsResolver::Clear();
    MasterFile = NewTab;

    if (previousMasterFile == 0 && CFavorites::IsChanged())
        CFavorites::SetChanged(!CFavorites::Export(CSettings::GetUserDataFileName(), true));

    lbServers->Clear();
    lbPlayers->Clear();
    lbRules->Clear();
    Servers.clear();
    CServerList::MarkOrderDirty();

    if (NewTab == 0)
    {
        String userData = CSettings::GetUserDataFileName();
        if (!userData.IsEmpty() && FileExists(userData))
            ImportFavorites(userData, false);
    }

    UpdateServers();

    tbMasterServerUpdate->Enabled = NewTab != 0;
    miMasterServerUpdate->Enabled = NewTab != 0;
    miAddServer->Enabled = NewTab == 0;
    tbDeleteServer->Enabled = NewTab == 0;
    miDeleteServer->Enabled = NewTab == 0;
    piDeleteServer->Visible = NewTab == 0;

    if (lbServers->Items->Count > 0)
        lbServers->ItemIndex = 0;

    lbServersClick(Sender);
    if (NewTab != 0)
        MasterServerUpdateClick(Sender);
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
    catch (Exception& exception)
    {
        Application->ShowException(&exception);
    }
    catch (...)
    {
        try
        {
            throw Exception("");
        }
        catch (Exception& exception)
        {
            Application->ShowException(&exception);
        }
    }
    return 0;
}
