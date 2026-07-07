@echo off
echo === Clear project ===

:: Удаляем все объектные файлы рекурсивно во всех папках
del /f /s /q *.o 

:: Удаляем бинарники в корне
del *.elf 
del *.sfo 
del *.pbp 
del *.prx

:: Удаляем сгенерированные стабы и временные файлы из папки VRAM хука
del "SP-P_Engine\Misc\vram_hook\VramExt.S"
del "SP-P_Engine\Misc\vram_hook\VramExt_driver.S"

:: Удаляем лог
del "draw_stats_ms.txt"

echo === Clear complete! ===
pause