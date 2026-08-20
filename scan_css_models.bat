@echo off
chcp 65001 >nul
title Quet Danh Sach File Trong Folder Hien Tai
setlocal EnableDelayedExpansion

set OUTPUT_FILE=%~dp0folder_structure.txt

echo ======================================================== > "%OUTPUT_FILE%"
echo THU MUC DANG QUET: %CD% >> "%OUTPUT_FILE%"
echo Thoi gian: %date% %time% >> "%OUTPUT_FILE%"
echo ======================================================== >> "%OUTPUT_FILE%"
echo. >> "%OUTPUT_FILE%"

echo --- DANH SACH TOAN BO FILE TRONG FOLDER NAY (DUONG DAN TUONG DOI) --- >> "%OUTPUT_FILE%"
echo. >> "%OUTPUT_FILE%"

for /r %%F in (*) do (
    set "full_path=%%F"
    set "rel_path=!full_path:%CD%\=!"
    if not "!rel_path!"=="%~nx0" if not "!rel_path!"=="folder_structure.txt" (
        echo !rel_path! >> "%OUTPUT_FILE%"
    )
)

echo.
echo ========================================================
echo DA QUET XONG!
echo File ket qua da luu ngay tai:
echo "%OUTPUT_FILE%"
echo ========================================================
pause
