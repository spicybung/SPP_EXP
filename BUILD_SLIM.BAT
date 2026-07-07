@echo off
echo === Build Kernel PRX (VRAM Hook) ===
cd SP-P_Engine\Misc\vram_hook
C:\pspsdk\bin\make.exe
cd ..\..\..

echo.
echo === Build Main Engine (EBOOT.PBP) for PSP SLIM ===
C:\pspsdk\bin\make.exe MODEL=SLIM

echo.
echo === Complete! ===
pause