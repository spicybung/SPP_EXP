@echo off
echo === Build Kernel PRX (VRAM Hook) ===
cd SP-P_Engine\Misc\vram_hook
C:\pspsdk\bin\make.exe
cd ..\..\..

echo.
echo === Build Main Engine (EBOOT.PBP) for PSP FAT ===
C:\pspsdk\bin\make.exe MODEL=FAT

echo.
echo === Complete! ===
pause