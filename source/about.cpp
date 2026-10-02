/*
* about.cpp
* Copyright (C) 2026 1therealcloud
*
* Licensed under the GNU General Public License v3.0.
* See LICENSE file in the project root for full license text.
* https://www.gnu.org/licenses/gpl-3.0.txt
*/

#include <stdafx.h>
#pragma hdrstop

#include "about.h"

#pragma package(smart_init)
#pragma resource "..\\dfm\\about.dfm"

#pragma comment(lib, "winmm.lib") // timeGetTime, timeBeginPeriod

TfmAbout* fmAbout;

static TCreditLine CreditLines[] = {{"San Andreas", COLOR_NAME, true},
                                    {"Multiplayer", COLOR_TITLE, true},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"Coding:", COLOR_TITLE, true},
                                    {"", 666, false},
                                    {"Kalcor", COLOR_NAME, false},
                                    {"spookie", COLOR_NAME, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"Past coders:", COLOR_TITLE, true},
                                    {"", 666, false},
                                    {"Y_Less", COLOR_NAME, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"Beta Testing:", COLOR_TITLE, true},
                                    {"", 666, false},
                                    {"BlueG, cessil, CrazyBob", COLOR_NAME, false},
                                    {"DamianC, dugi, d0", COLOR_NAME, false},
                                    {"Jay, JernejL, kaisersouse", COLOR_NAME, false},
                                    {"KingJ, Matite, Mmartin", COLOR_NAME, false},
                                    {"RayW, Si|ent, Wicko", COLOR_NAME, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"www.sa-mp.com", COLOR_URL, true},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false},
                                    {"", 666, false}};

static const int CREDIT_LINES_COUNT = sizeof(CreditLines) / sizeof(CreditLines[0]);

// global GDI vars
static int CreditsRollY = 300;
static int TempCRY = 0;
static BITMAPINFO bmi;
static HDC hDC1 = NULL, hDC2 = NULL;
static RGBQUAD* Buf = NULL;
static HBITMAP hBmp = NULL;
static HFONT hNormFont = NULL, hBoldFont = NULL;
static HGDIOBJ hOldBitmap = NULL;
static RECT xRect;

static int DIBWidth = 0;
static int DIBHeight = 0;
static DWORD Ticks_About = 0;
static int TimeScale = 1;

// forward declarations
static void Flip();
static void Render();

__fastcall TfmAbout::TfmAbout(TComponent* Owner) : TForm(Owner)
{}

void __fastcall TfmAbout::FormCreate(TObject* Sender)
{
    DIBWidth = ClientWidth - 4;
    DIBHeight = ClientHeight - 4;

    xRect.left = 0;
    xRect.top = 0;
    xRect.right = ClientWidth;
    xRect.bottom = ClientHeight;

    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = DIBWidth;
    bmi.bmiHeader.biHeight = -DIBHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    RenderTimer = new TTimer(this);
    RenderTimer->Enabled = false;
    RenderTimer->Interval = 10;
    RenderTimer->OnTimer = RenderTimerTick;
}

void __fastcall TfmAbout::FormDestroy(TObject* Sender)
{
    RenderTimer = nullptr;
}

void __fastcall TfmAbout::FormShow(TObject* Sender)
{
    CreditsRollY = 300;

    Buf = new RGBQUAD[DIBWidth * DIBHeight];
    hDC1 = GetDC(this->Handle);
    hDC2 = CreateCompatibleDC(hDC1);
    hBmp = CreateCompatibleBitmap(hDC1, DIBWidth, DIBHeight);

    LOGFONTW fontStruct;
    ZeroMemory(&fontStruct, sizeof(fontStruct));
    fontStruct.lfWidth = 0;
    fontStruct.lfHeight = -18;
    fontStruct.lfQuality = ANTIALIASED_QUALITY;

    wcsncpy(fontStruct.lfFaceName, L"Arial", LF_FACESIZE - 1);

    hNormFont = CreateFontIndirectW(&fontStruct);
    fontStruct.lfWeight = FW_BOLD;
    hBoldFont = CreateFontIndirectW(&fontStruct);

    hOldBitmap = SelectObject(hDC2, hBmp);
    SetBkMode(hDC2, TRANSPARENT);

    Ticks_About = timeGetTime();
    RenderTimer->Enabled = true;
}

void __fastcall TfmAbout::FormClose(TObject* Sender, TCloseAction& Action)
{
    RenderTimer->Enabled = false;

    if (hDC2)
        SelectObject(hDC2, GetStockObject(SYSTEM_FONT));
    if (hNormFont)
        DeleteObject(hNormFont);
    if (hBoldFont)
        DeleteObject(hBoldFont);
    hNormFont = NULL;
    hBoldFont = NULL;
    if (hDC2 && hOldBitmap)
    {
        SelectObject(hDC2, hOldBitmap);
        hOldBitmap = NULL;
    }
    if (hBmp)
    {
        DeleteObject(hBmp);
        hBmp = NULL;
    }
    if (hDC2)
        DeleteDC(hDC2);
    if (hDC1)
        ReleaseDC(this->Handle, hDC1);
    hDC2 = NULL;
    hDC1 = NULL;

    if (Buf)
    {
        delete[] Buf;
        Buf = NULL;
    }
}

void __fastcall TfmAbout::RenderTimerTick(TObject* Sender)
{
    Render();
}

void __fastcall TfmAbout::FormClick(TObject* Sender)
{
    Close();
}

static void Flip()
{
    SetDIBits(hDC2, hBmp, 0, DIBHeight, Buf, &bmi, DIB_RGB_COLORS);

    TempCRY += TimeScale;
    if (TempCRY >= 5)
    {
        TempCRY = 0;
        CreditsRollY -= TimeScale;
        if (CreditsRollY < -((CREDIT_LINES_COUNT * 12) + 50))
        {
            CreditsRollY = 300;
        }
    }

    xRect.top = CreditsRollY;
    for (int i = 0; i < CREDIT_LINES_COUNT; ++i)
    {
        if (CreditLines[i].Color != 666 && xRect.top > -12 && xRect.top < 300)
        {
            SetTextColor(hDC2, CreditLines[i].Color);
            SelectObject(hDC2, CreditLines[i].Bold ? hBoldFont : hNormFont);
            DrawTextW(hDC2, CreditLines[i].Line.c_str(), -1, &xRect, DT_NOCLIP | DT_CENTER);
        }
        xRect.top += 20;
    }

    BitBlt(hDC1, 2, 2, DIBWidth, DIBHeight, hDC2, 0, 0, SRCCOPY);
}

static void Render()
{
    timeBeginPeriod(1);
    DWORD t = timeGetTime();
    timeEndPeriod(1);

    DWORD delta = t - Ticks_About;
    if (delta > 0)
    {
        TimeScale = (int)(100.0 / (1000.0 / delta));
    }
    Ticks_About = t;

    ZeroMemory(Buf, DIBWidth * DIBHeight * sizeof(RGBQUAD));

    Flip();
}
