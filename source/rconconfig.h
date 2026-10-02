/*
* rconconfig.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class TfmRconConfig : public TForm
{
    __published : TEdit* edHost;
    TLabel* lbHost;
    TLabel* lbPassword;
    TEdit* edPassword;
    TButton* bnConnect;
    TButton* bnCancel;

    void __fastcall edHostKeyPress(TObject* Sender, System::WideChar& Key);
    void __fastcall bnCancelClick(TObject* Sender);
    void __fastcall bnConnectClick(TObject* Sender);
    void __fastcall edPasswordChange(TObject* Sender);

public:
    __fastcall TfmRconConfig(TComponent* Owner);
};

extern PACKAGE TfmRconConfig* fmRconConfig;
