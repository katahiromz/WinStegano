// main.c --- Steganography for Windows
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
