/*
* unit_webrunform.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

class Twnd_webrunform : public TForm
{
    __published : TBitBtn* BitBtn1; // Add to favorites ? mrYes
    TBitBtn* BitBtn2;               // Cancel           ? mrCancel
    TBitBtn* BitBtn3;               // Play now         ? mrOk
    TLabel* Label1;

    void __fastcall BitBtn1Click(TObject* Sender);
    void __fastcall BitBtn2Click(TObject* Sender);
    void __fastcall BitBtn3Click(TObject* Sender);

public:
    __fastcall Twnd_webrunform(TComponent* Owner);
};

extern PACKAGE Twnd_webrunform* wnd_webrunform;
