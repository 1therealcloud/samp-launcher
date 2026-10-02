/*
* importfavorites.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class TfmImportFavorites : public TForm
{
    __published : TRadioButton* rbAddToCurrent;
    TRadioButton* rbReplaceCurrent;
    TButton* bnOk;

public:
    __fastcall TfmImportFavorites(TComponent* Owner);
};

extern PACKAGE TfmImportFavorites* fmImportFavorites;
