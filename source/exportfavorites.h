/*
* exportfavorites.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class TfmExportFavorites : public TForm
{
__published:
    TCheckBox *cbIncludeSavedPasswords;
    TButton   *bnOk;

public:
    __fastcall TfmExportFavorites(TComponent *Owner);
};

extern PACKAGE TfmExportFavorites *fmExportFavorites;
