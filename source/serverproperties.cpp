/*
* serverproperties.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "serverproperties.h"
#include "main.h"

#pragma package(smart_init)
#pragma resource "..\\dfm\\serverproperties.dfm"

TfmServerProperties* fmServerProperties;

__fastcall TfmServerProperties::TfmServerProperties(TComponent* Owner) : TForm(Owner)
{}

void __fastcall TfmServerProperties::bnSaveClick(TObject* Sender)
{
    if (fmMain && fmMain->lbServers->ItemIndex != -1)
    {
        int idx = StrToIntDef(fmMain->lbServers->Items->Strings[fmMain->lbServers->ItemIndex], -1);
        if (idx < 0 || idx >= (int)Servers.size())
        {
            Close();
            return;
        }

        // AnsiString
        Servers[idx].ServerPassword = AnsiString(edServerPassword->Text);
        Servers[idx].RconPassword = AnsiString(edRconPassword->Text);
        fmMain->SaveFavoritesNow();
    }
    Close();
}

void __fastcall TfmServerProperties::bnCancelClick(TObject* Sender)
{
    Close();
}

void __fastcall TfmServerProperties::bnConnectClick(TObject* Sender)
{
    if (fmMain && fmMain->lbServers->ItemIndex != -1)
    {
        int idx = StrToIntDef(fmMain->lbServers->Items->Strings[fmMain->lbServers->ItemIndex], -1);
        if (idx < 0 || idx >= (int)Servers.size())
        {
            Close();
            return;
        }

        // AnsiString
        Servers[idx].ServerPassword = AnsiString(edServerPassword->Text);
        Servers[idx].RconPassword = AnsiString(edRconPassword->Text);
        fmMain->SaveFavoritesNow();

        fmMain->ConnectClick(fmMain);
    }
    Close();
}

void __fastcall TfmServerProperties::pmCopyPopup(TObject* Sender)
{
    piCopy->Enabled = (edAddress->Text != L"- - -");
}

void __fastcall TfmServerProperties::piCopyClick(TObject* Sender)
{
    if (fmMain)
    {
        fmMain->SetClipBoardStr(edAddress->Text);
    }
}
