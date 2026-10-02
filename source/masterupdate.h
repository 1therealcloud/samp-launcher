/*
* masterupdate.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class TfmMasterUpdate : public TForm
{
    __published : TLabel* lblPleaseWait;
    TLabel* lblUpdating;

public:
    __fastcall TfmMasterUpdate(TComponent* Owner);
};

extern PACKAGE TfmMasterUpdate* fmMasterUpdate;
