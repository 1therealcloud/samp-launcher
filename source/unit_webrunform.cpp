/*
* unit_webrunform.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "unit_webrunform.h"

#pragma package(smart_init)
#pragma resource "..\\dfm\\unit_webrunform.dfm"

Twnd_webrunform *wnd_webrunform;

__fastcall Twnd_webrunform::Twnd_webrunform(TComponent *Owner)
    : TForm(Owner)
{
}

void __fastcall Twnd_webrunform::BitBtn1Click(TObject *Sender)
{
    ModalResult = mrYes;
}

void __fastcall Twnd_webrunform::BitBtn2Click(TObject *Sender)
{
    ModalResult = mrCancel;
}

void __fastcall Twnd_webrunform::BitBtn3Click(TObject *Sender)
{
    ModalResult = mrOk;
}
