/*
* serverproperties.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class TfmServerProperties : public TForm
{
__published:
    TLabel     *lbAddressLab;
    TLabel     *lbPlayersLab;
    TLabel     *lbPingLab;
    TLabel     *lbModeLab;
    TLabel     *lbMapLab;
    TLabel     *lbMap;
    TLabel     *lbMode;
    TLabel     *lbPing;
    TLabel     *lbPlayers;
    TEdit      *edAddress;
    TLabel     *lbHostName;
    TLabel     *lbServerPassword;
    TLabel     *lbRconPassword;
    TEdit      *edServerPassword;
    TEdit      *edRconPassword;
    TButton    *bnSave;
    TButton    *bnCancel;
    TButton    *bnConnect;
    TPopupMenu *pmCopy;
    TMenuItem  *piCopy;

    void __fastcall bnSaveClick(TObject *Sender);
    void __fastcall bnCancelClick(TObject *Sender);
    void __fastcall bnConnectClick(TObject *Sender);
    void __fastcall pmCopyPopup(TObject *Sender);
    void __fastcall piCopyClick(TObject *Sender);

public:
    __fastcall TfmServerProperties(TComponent *Owner);
};

extern PACKAGE TfmServerProperties *fmServerProperties;
