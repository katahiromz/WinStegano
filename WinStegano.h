// WinStegano.h --- Steganography for Windows
// Author: katahiromz
// License: MIT
#pragma once

HFONT CreateSteganoFont(void);
HBITMAP CreateMonoBitmapFromText(HFONT hFont, LPCSTR text, INT text_len, INT margin);
HBITMAP WriteStegano(HBITMAP hbm, HBITMAP hbmMono);
HBITMAP ReadStegano(HBITMAP hbm);
HBITMAP Make32BppDIB(HBITMAP hbm, LPDWORD *pdwBits);
