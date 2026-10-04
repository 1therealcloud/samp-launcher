/*
* settings.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class TfmSettings : public TForm
{
    __published : TButton* bnSave;
    TButton* bnCancel;
    TGroupBox* gbPasswords;
    TCheckBox* cbSaveServerPasswords;
    TCheckBox* cbSaveRconPasswords;
    TEdit* edInstallLoc;
    TLabel* Label1;
    TLabel* lblModelCacheTag;
    TSpeedButton* sbBrowseCache;
    TLabel* lblProxyAddr;
    TLabel* lblDebugScript;

    TEdit* edCacheLoc;
    TEdit* edProxyAddress;
    TEdit* edDebugScript;
    TSpeedButton* sbBrowse;
    TSpeedButton* sbBrowseDebugScript;
    TButton* bnLaunchDebug;

    void __fastcall bnSaveClick(TObject* Sender);
    void __fastcall bnCancelClick(TObject* Sender);
    void __fastcall FormCreate(TObject* Sender);
    void __fastcall sbBrowseClick(TObject* Sender);
    void __fastcall sbBrowseCacheClick(TObject* Sender);
    void __fastcall sbBrowseDebugScriptClick(TObject* Sender);
    void __fastcall bnLaunchDebugClick(TObject* Sender);

public:
    __fastcall TfmSettings(TComponent* Owner);
};

extern PACKAGE TfmSettings* fmSettings;
