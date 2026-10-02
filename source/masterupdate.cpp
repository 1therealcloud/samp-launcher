/*
* masterupdate.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "masterupdate.h"

#pragma package(smart_init)
#pragma resource "..\\dfm\\masterupdate.dfm"

TfmMasterUpdate* fmMasterUpdate;

__fastcall TfmMasterUpdate::TfmMasterUpdate(TComponent* Owner) : TForm(Owner)
{}
