/*
* importfavorites.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "importfavorites.h"

#pragma package(smart_init)
#pragma resource "..\\dfm\\importfavorites.dfm"

TfmImportFavorites *fmImportFavorites;

__fastcall TfmImportFavorites::TfmImportFavorites(TComponent *Owner)
    : TForm(Owner)
{
}
