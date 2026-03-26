#!/bin/bash
./WinStegano.exe -write-text input.bmp output.bmp "This is a test"
./WinStegano.exe -read output.bmp mono.bmp
