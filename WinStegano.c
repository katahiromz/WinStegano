// WinStegano.c --- Steganography for Windows
// Author: katahiromz
// License: MIT
#include <windows.h>
#include <stdio.h>
#include "WinStegano.h"
#include "SaveBitmapToFile.h"

void version(void)
{
    puts("WinStegano Version 1.0.0");
}

void usage(void)
{
    puts("Usage: WinStegano -write-text input.bmp output.bmp TEXT\n"
         "       WinStegano -write-mono input.bmp output.bmp input-mono.bmp\n"
         "       WinStegano -read input.bmp output-mono.bmp");
}

typedef enum OPERATION
{
    OP_HELP,
    OP_VERSION,
    OP_WRITE_TEXT,
    OP_WRITE_MONO,
    OP_READ,
} OPERATION;

typedef struct STEGANO
{
    OPERATION op;
    const char *input;
    const char *output;
    const char *text;
    const char *mono;
} STEGANO;

HFONT CreateSteganoFont(void)
{
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    LOGFONTW lf;
    GetObjectW(hFont, sizeof(lf), &lf);

    lf.lfHeight = 30;
    return CreateFontIndirectW(&lf);
}

HBITMAP CreateMonoBitmapFromText(HFONT hFont, LPCSTR text, INT text_len, INT margin)
{
    if (!text || !text[0])
        return NULL;

    if (!hFont)
        hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    SIZE textSize = { 0, 0 };
    HDC hDC = CreateCompatibleDC(NULL);
    HGDIOBJ hFontOld = SelectObject(hDC, hFont);
    GetTextExtentPoint32A(hDC, text, text_len, &textSize);
    SelectObject(hDC, hFontOld);
    DeleteDC(hDC);

    INT cx = textSize.cx + 2 * margin;
    INT cy = textSize.cy + 2 * margin;
    if (!cx || !cy)
        return NULL;

    HBITMAP hbm = CreateBitmap(cx, cy, 1, 1, NULL);
    if (!hbm)
        return NULL;

    hDC = CreateCompatibleDC(NULL);
    HGDIOBJ hbmOld = SelectObject(hDC, hbm);
    RECT rc = { 0, 0, cx, cy };
    FillRect(hDC, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    hFontOld = SelectObject(hDC, hFont);
    SetTextAlign(hDC, TA_LEFT | TA_TOP);
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(0, 0, 0));
    TextOutA(hDC, margin, margin, text, text_len);
    SelectObject(hDC, hFontOld);
    SelectObject(hDC, hbmOld);
    DeleteDC(hDC);

    return hbm;
}

HBITMAP Make32BppDIB(HBITMAP hbm, LPDWORD *ppdwBits)
{
    BITMAP bm;
    if (!GetObjectW(hbm, sizeof(bm), &bm))
        return NULL;

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = bm.bmWidth;
    bmi.bmiHeader.biHeight = -bm.bmHeight; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;

    HDC hDC1 = CreateCompatibleDC(NULL);
    HDC hDC2 = CreateCompatibleDC(NULL);
    HBITMAP hbmNew = CreateDIBSection(hDC1, &bmi, DIB_RGB_COLORS, (void**)ppdwBits, NULL, 0);
    HGDIOBJ hbmOld1 = SelectObject(hDC1, hbm);
    HGDIOBJ hbmOld2 = SelectObject(hDC2, hbmNew);
    BitBlt(hDC2, 0, 0, bm.bmWidth, bm.bmHeight, hDC1, 0, 0, SRCCOPY);
    SelectObject(hDC2, hbmOld2);
    SelectObject(hDC1, hbmOld1);
    DeleteDC(hDC2);
    DeleteDC(hDC1);

    return hbmNew;
}

HBITMAP WriteStegano(HBITMAP hbm, HBITMAP hbmMono)
{
    BITMAP bm1, bm2;
    if (!GetObjectW(hbm, sizeof(bm1), &bm1) || !GetObjectW(hbmMono, sizeof(bm2), &bm2))
        return NULL;

    DWORD* pdw;
    HBITMAP hbmNew = Make32BppDIB(hbm, &pdw);
    if (!hbmNew || !GetObjectW(hbmNew, sizeof(bm1), &bm1))
        return NULL;

    HDC hDC2 = CreateCompatibleDC(NULL);
    HGDIOBJ hbmOld2 = SelectObject(hDC2, hbmMono);

    for (INT y = 0; y < bm1.bmHeight; ++y)
    {
        for (INT x = 0; x < bm1.bmWidth; ++x)
        {
            DWORD pixel = *pdw;
            pixel &= 0xFFFEFEFE;
            if (GetPixel(hDC2, x % bm2.bmWidth, y % bm2.bmHeight))
                pixel |= 0x00010101;
            *pdw++ = pixel;
        }
    }

    SelectObject(hDC2, hbmOld2);
    DeleteDC(hDC2);

    return hbmNew;
}

HBITMAP ReadStegano(HBITMAP hbm)
{
    DWORD* pdwSrc = NULL;
    HBITMAP hbm32 = Make32BppDIB(hbm, &pdwSrc);
    if (!hbm32)
        return NULL;

    BITMAP bm1;
    if (!GetObjectW(hbm32, sizeof(bm1), &bm1))
    {
        DeleteObject(hbm32);
        return NULL;
    }

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = bm1.bmWidth;
    bmi.bmiHeader.biHeight = -bm1.bmHeight; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 1;
    bmi.bmiColors[0].rgbBlue = bmi.bmiColors[0].rgbGreen = bmi.bmiColors[0].rgbRed = 0;
    bmi.bmiColors[1].rgbBlue = bmi.bmiColors[1].rgbGreen = bmi.bmiColors[1].rgbRed = 255;

    PBYTE pbDest = NULL;
    HDC hdcScreen = GetDC(NULL);
    HBITMAP hbmMono = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void**)&pbDest, NULL, 0);
    ReleaseDC(NULL, hdcScreen);

    BITMAP bm2;
    if (!hbmMono || !pbDest || !GetObjectW(hbmMono, sizeof(bm2), &bm2))
    {
        DeleteObject(hbm32);
        DeleteObject(hbmMono);
        return NULL;
    }

    ZeroMemory(pbDest, bm2.bmHeight * bm2.bmWidthBytes);

    for (INT y = 0; y < bm2.bmHeight; ++y)
    {
        PDWORD pRowSrc = &pdwSrc[y * bm2.bmWidth];
        PBYTE pRowDest = &pbDest[y * bm2.bmWidthBytes];

        for (INT x = 0; x < bm2.bmWidth; ++x)
        {
            if ((pRowSrc[x] & 0x00010101) == 0x00010101)
                pRowDest[x / 8] |= (0x80 >> (x % 8));
        }
    }

    DeleteObject(hbm32);
    return hbmMono;
}

INT parse_command_line(STEGANO *data, int argc, char **argv)
{
    if (argc <= 1)
        return -1;

    const char *op = argv[1];

    if (lstrcmpiA(op, "-help") == 0 || lstrcmpiA(op, "--help") == 0)
    {
        data->op = OP_HELP;
        return 0;
    }

    if (lstrcmpiA(op, "-version") == 0 || lstrcmpiA(op, "--version") == 0)
    {
        data->op = OP_VERSION;
        return 0;
    }

    if (lstrcmpiA(op, "-write-text") == 0 && argc == 5)
    {
        data->op = OP_WRITE_TEXT;
        data->input = argv[2];
        data->output = argv[3];
        data->text = argv[4];
        data->mono = NULL;
        return 0;
    }

    if (lstrcmpiA(op, "-write-mono") == 0 && argc == 5)
    {
        data->op = OP_WRITE_MONO;
        data->input = argv[2];
        data->output = argv[3];
        data->text = NULL;
        data->mono = argv[4];
        return 0;
    }

    if (lstrcmpiA(op, "-read") == 0 && argc == 4)
    {
        data->op = OP_READ;
        data->input = argv[2];
        data->output = argv[3];
        data->text = NULL;
        data->mono = NULL;
        return 0;
    }

    return -1;
}

int main(int argc, char **argv)
{
    STEGANO data;
    int ret = parse_command_line(&data, argc, argv);
    if (ret == -1)
    {
        fprintf(stderr, "WinStegano: Invalid arguments\n");
        usage();
        return 1;
    }

    BOOL bOK = FALSE;

    switch (data.op)
    {
    case OP_HELP:
        usage();
        bOK = TRUE;
        break;
    case OP_VERSION:
        version();
        bOK = TRUE;
        break;
    case OP_WRITE_TEXT:
        {
            HFONT hFont = CreateSteganoFont();
            HBITMAP hbmInput = LoadBitmapFromFileA(data.input);
            HBITMAP hbmMono = CreateMonoBitmapFromText(hFont, data.text, lstrlenA(data.text), 16);
            HBITMAP hbmOutput = WriteStegano(hbmInput, hbmMono);
            if (hFont && hbmInput && hbmMono && hbmOutput && SaveBitmapToFileA(data.output, hbmOutput))
                bOK = TRUE;
            DeleteObject(hFont);
            DeleteObject(hbmInput);
            DeleteObject(hbmOutput);
            DeleteObject(hbmMono);
        }
        break;
    case OP_WRITE_MONO:
        {
            HBITMAP hbmInput = LoadBitmapFromFileA(data.input);
            HBITMAP hbmMono = LoadBitmapFromFileA(data.mono);
            HBITMAP hbmOutput = WriteStegano(hbmInput, hbmMono);
            if (hbmInput && hbmMono && hbmOutput && SaveBitmapToFileA(data.output, hbmOutput))
            {
                bOK = TRUE;
            }
            DeleteObject(hbmInput);
            DeleteObject(hbmOutput);
            DeleteObject(hbmMono);
        }
        break;
    case OP_READ:
        {
            HBITMAP hbmInput = LoadBitmapFromFileA(data.input);
            HBITMAP hbmMono = ReadStegano(hbmInput);
            if (hbmInput && hbmMono && SaveBitmapToFileA(data.output, hbmMono))
            {
                bOK = TRUE;
            }
            DeleteObject(hbmInput);
            DeleteObject(hbmMono);
        }
        break;
    }

    if (!bOK)
    {
        puts("FAILED.");
        return 1;
    }

    puts("OK.");
    return 0;
}
