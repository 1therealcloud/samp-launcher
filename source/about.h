/*
* about.h
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#pragma once

#include <mmsystem.h>

// colors
#define COLOR_TITLE   0x000080FF
#define COLOR_NAME    0x00FFFFFF
#define COLOR_URL     0x00FFAA00

struct TCreditLine {
    String   Line;
    COLORREF Color;
    bool     Bold;
};

class TfmAbout : public TForm
{
__published:
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall FormClick(TObject *Sender);

private:
    TTimer *RenderTimer;
    void __fastcall RenderTimerTick(TObject *Sender);

public:
    __fastcall TfmAbout(TComponent *Owner);
};

extern PACKAGE TfmAbout *fmAbout;
