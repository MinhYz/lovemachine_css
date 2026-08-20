@echo off
chcp 65001 >nul
title Quet Cau Truc Thu Muc Game CS:S & Skin
echo ========================================================
echo   DANG QUET CAU TRUC THU MUC CS:S & CAC MODEL TRONG MAY...
echo ========================================================
echo.

set OUTPUT_FILE=css_folder_structure.txt

echo ======================================================== > %OUTPUT_FILE%
echo   CAU TRUC THU MUC & DANH SACH MODEL CS:S CUA BAN >> %OUTPUT_FILE%
echo   Thoi gian quet: %date% %time% >> %OUTPUT_FILE%
echo ======================================================== >> %OUTPUT_FILE%
echo. >> %OUTPUT_FILE%

echo [1] THU MUC HIEN TAI: %CD% >> %OUTPUT_FILE%
echo. >> %OUTPUT_FILE%

echo [2] DANH SACH CAC FILE .MDL TRONG THU MUC HIEN TAI VA THU MUC CON: >> %OUTPUT_FILE%
dir /s /b *.mdl >> %OUTPUT_FILE% 2>nul
echo. >> %OUTPUT_FILE%

echo [3] CAY THU MUC TONG QUAT (3 CAP DO): >> %OUTPUT_FILE%
tree /f /a | findstr /i /v ".tga .wav .mp3" >> %OUTPUT_FILE% 2>nul

echo ========================================================
echo DA QUET XONG!
echo File ket qua da duoc luu tai: %CD%\%OUTPUT_FILE%
echo Ban chi can mo file "%OUTPUT_FILE%" len va copy noi dung gui cho toi nhe!
echo ========================================================
pause
