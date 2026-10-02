/*
* rconconfig.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "CRconClient.h"
#include "rconconfig.h"

#pragma package(smart_init)
#pragma resource "..\\dfm\\rconconfig.dfm"

TfmRconConfig* fmRconConfig;

__fastcall TfmRconConfig::TfmRconConfig(TComponent* Owner) : TForm(Owner)
{}

void __fastcall TfmRconConfig::bnCancelClick(TObject* Sender)
{
    Close();
}

void __fastcall TfmRconConfig::bnConnectClick(TObject* Sender)
{
    UnicodeString server = edHost->Text;
    UnicodeString address;
    UnicodeString portText;

    int colon = server.Pos(L":");
    if (colon != 0)
    {
        address = server.SubString(1, colon - 1);
        portText = server.SubString(colon + 1, 5);
    }
    else
    {
        address = server;
        portText = L"7777";
    }

    int port = StrToIntDef(portText, -1);
    if (address.IsEmpty() || address.Length() > 253 || port < 1 || port > 65535 || edPassword->Text.Length() > 65535)
    {
        MessageDlg("Invalid RCON host, port, or password.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    TRconStartResult result = CRconClient::Start(AnsiString(address), port, AnsiString(edPassword->Text));
    if (result == TRconStartResult::AlreadyActive)
    {
        MessageDlg("An RCON session is already active.", mtInformation, TMsgDlgButtons() << mbOK, 0);
        return;
    }
    if (result == TRconStartResult::Failed)
    {
        MessageDlg("Unable to start the RCON session.", mtError, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    Close();
}

void __fastcall TfmRconConfig::edHostKeyPress(TObject* Sender, System::WideChar& Key)
{
    if (!((Key >= L'0' && Key <= L'9') || (Key >= L'a' && Key <= L'z') || (Key >= L'A' && Key <= L'Z') || Key == L'.' ||
          Key == L':' || Key == 8 || Key == 46))
    {
        Key = 0;
    }
}

void __fastcall TfmRconConfig::edPasswordChange(TObject* Sender)
{
    bnConnect->Enabled = (!edHost->Text.IsEmpty()) && (!edPassword->Text.IsEmpty());
}
