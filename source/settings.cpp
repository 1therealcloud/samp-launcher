/*
* settings.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "settings.h"
#include "CSettings.h"
#include "main.h"

#include <shlobj.h>

#pragma package(smart_init)
#pragma resource "..\\dfm\\settings.dfm"

TfmSettings* fmSettings;

static bool IsValidProxy(const UnicodeString& value)
{
    UnicodeString proxy = value.Trim();

    // Empty = proxy disabled.
    if (proxy.IsEmpty())
        return true;

    UnicodeString lower = proxy.LowerCase();

    int prefixLength = 0;

    if (lower.Pos(L"http://") == 1)
        prefixLength = 7;
    else if (lower.Pos(L"https://") == 1)
        prefixLength = 8;
    else if (lower.Pos(L"socks5://") == 1)
        prefixLength = 9;
    else
        return false;

    if (proxy.Length() <= prefixLength)
        return false;

    if (proxy.Pos(L" ") != 0 || proxy.Pos(L"\t") != 0)
        return false;

    return true;
}

__fastcall TfmSettings::TfmSettings(TComponent* Owner) : TForm(Owner)
{}

void __fastcall TfmSettings::FormCreate(TObject* Sender)
{
    cbSaveServerPasswords->Checked = CSettings::ReadBool(L"SaveServPasses", false);
    cbSaveRconPasswords->Checked = CSettings::ReadBool(L"SaveRconPasses", false);
    edCacheLoc->Text = CSettings::ReadString(L"model_cache");
    edProxyAddress->Text = CSettings::ReadString(L"artwork_proxy");
    edInstallLoc->Text = ExtractFilePath(CSettings::GetGtaExecutable());
}

void __fastcall TfmSettings::bnSaveClick(TObject* Sender)
{
    UnicodeString proxy = edProxyAddress->Text.Trim();

    if (!IsValidProxy(proxy))
    {
        Application->MessageBox(L"Invalid proxy address.\n\n"
                                L"Use one of these formats:\n"
                                L"http://host:port\n"
                                L"https://host:port\n"
                                L"socks5://host:port",
                                L"Settings", MB_OK | MB_ICONWARNING);

        edProxyAddress->SetFocus();
        return;
    }

    CSettings::WriteBool(L"SaveServPasses", cbSaveServerPasswords->Checked);
    CSettings::WriteBool(L"SaveRconPasses", cbSaveRconPasswords->Checked);
    CSettings::WriteString(L"artwork_proxy", proxy);

    edProxyAddress->Text = proxy;

    // The in-memory server vector contains the master list outside Favorites.
    // Never write that transient list over USERDATA.DAT.
    if (fmMain && MasterFile == 0)
        fmMain->SaveFavoritesNow();

    Close();
}

void __fastcall TfmSettings::bnCancelClick(TObject* Sender)
{
    Close();
}

void __fastcall TfmSettings::sbBrowseClick(TObject* Sender)
{
    if (fmMain)
    {
        fmMain->GetGTAExe(this->Handle);
        edInstallLoc->Text = ExtractFilePath(CSettings::GetGtaExecutable());
    }
}

void __fastcall TfmSettings::sbBrowseCacheClick(TObject* Sender)
{
    wchar_t path[MAX_PATH] = {};
    String startDir;

    if (SHGetSpecialFolderPathW(nullptr, path, CSIDL_PERSONAL, false))
    {
        startDir = String(path) + "\\GTA San Andreas User Files\\SAMP\\cache";
    }

    String directory;

    if (!fmMain || !fmMain->BrowseForFolder(Handle, directory, startDir, "Please locate your model cache..."))
    {
        return;
    }

    edCacheLoc->Text = directory;

    CSettings::WriteString(L"model_cache", directory);
}
