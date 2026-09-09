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
#include "main.h"

#include <shlobj.h>

#pragma package(smart_init)
#pragma resource "..\\dfm\\settings.dfm"

TfmSettings *fmSettings;


static bool IsValidProxy(const UnicodeString &value)
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


__fastcall TfmSettings::TfmSettings(TComponent *Owner)
    : TForm(Owner)
{
}


void __fastcall TfmSettings::FormCreate(TObject *Sender)
{
    TRegistry *reg = new TRegistry();

    try {
        reg->RootKey = HKEY_CURRENT_USER;

        if (reg->OpenKey(L"SOFTWARE\\SAMP", true)) {
            if (reg->ValueExists(L"SaveServPasses")) {
                cbSaveServerPasswords->Checked =
                    reg->ReadBool(L"SaveServPasses");
            }

            if (reg->ValueExists(L"SaveRconPasses")) {
                cbSaveRconPasswords->Checked =
                    reg->ReadBool(L"SaveRconPasses");
            }

            if (reg->ValueExists(L"model_cache")) {
                edCacheLoc->Text =
                    reg->ReadString(L"model_cache");
            }

            if (reg->ValueExists(L"artwork_proxy")) {
                edProxyAddress->Text =
                    reg->ReadString(L"artwork_proxy");
            }

            reg->CloseKey();
        }
    }
    __finally {
        delete reg;
    }

    edInstallLoc->Text = ExtractFilePath(gta_sa_exe);
}


void __fastcall TfmSettings::bnSaveClick(TObject *Sender)
{
    UnicodeString proxy = edProxyAddress->Text.Trim();

    if (!IsValidProxy(proxy)) {
        Application->MessageBox(
            L"Invalid proxy address.\n\n"
            L"Use one of these formats:\n"
            L"http://host:port\n"
            L"https://host:port\n"
            L"socks5://host:port",
            L"Settings",
            MB_OK | MB_ICONWARNING
        );

        edProxyAddress->SetFocus();
        return;
    }

    TRegistry *reg = new TRegistry();

    try {
        reg->RootKey = HKEY_CURRENT_USER;

        if (reg->OpenKey(L"SOFTWARE\\SAMP", true)) {
            reg->WriteBool(
                L"SaveServPasses",
                cbSaveServerPasswords->Checked
            );

            reg->WriteBool(
                L"SaveRconPasses",
                cbSaveRconPasswords->Checked
            );

            reg->WriteString(
                L"artwork_proxy",
                proxy
            );

            reg->CloseKey();
        }
    }
    __finally {
        delete reg;
    }

    edProxyAddress->Text = proxy;

    // The in-memory server vector contains the master list outside Favorites.
    // Never write that transient list over USERDATA.DAT.
    if (fmMain && MasterFile == 0)
        fmMain->SaveFavoritesNow();

    Close();
}

void __fastcall TfmSettings::bnCancelClick(TObject *Sender)
{
    Close();
}

void __fastcall TfmSettings::sbBrowseClick(TObject *Sender)
{
    if (fmMain) {
        fmMain->GetGTAExe(this->Handle);
        edInstallLoc->Text = ExtractFilePath(gta_sa_exe);
    }
}


void __fastcall TfmSettings::sbBrowseCacheClick(TObject *Sender)
{
    wchar_t path[MAX_PATH] = {};
    String startDir;

    if (SHGetSpecialFolderPathW(
            nullptr,
            path,
            CSIDL_PERSONAL,
            false))
    {
        startDir =
            String(path) +
            "\\GTA San Andreas User Files\\SAMP\\cache";
    }

    String directory;

    if (!fmMain ||
        !fmMain->BrowseForFolder(
            Handle,
            directory,
            startDir,
            "Please locate your model cache..."))
    {
        return;
    }

    edCacheLoc->Text = directory;

    TRegistry *reg = new TRegistry();

    try {
        reg->RootKey = HKEY_CURRENT_USER;

        if (reg->OpenKey(L"SOFTWARE\\SAMP", true)) {
            reg->WriteString(
                L"model_cache",
                directory
            );

            reg->CloseKey();
        }
    }
    __finally {
        delete reg;
    }
}
