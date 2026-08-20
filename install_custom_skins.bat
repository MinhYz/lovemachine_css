@echo off
setlocal enabledelayedexpansion
title Lovemachine CS:S // 1-Click Custom Skin Installer

echo ==========================================================================
echo        LOVEMACHINE CS:S // 1-CLICK AUTO CUSTOM SKIN INSTALLER
echo ==========================================================================
echo.

set "CSS_PATH="

:: 1. Check if we are inside a CS:S game folder
if exist "hl2.exe" (
    if exist "cstrike" (
        set "CSS_PATH=%CD%"
    )
)

:: 2. Check parent directory
if not defined CSS_PATH (
    if exist "..\hl2.exe" (
        if exist "..\cstrike" (
            pushd ..
            set "CSS_PATH=!CD!"
            popd
        )
    )
)

:: 3. Check common paths (D:\css, C:\css, Steam)
if not defined CSS_PATH (
    if exist "D:\css\hl2.exe" (
        set "CSS_PATH=D:\css"
    ) else if exist "C:\css\hl2.exe" (
        set "CSS_PATH=C:\css"
    ) else if exist "C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Source\hl2.exe" (
        set "CSS_PATH=C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Source"
    ) else if exist "D:\SteamLibrary\steamapps\common\Counter-Strike Source\hl2.exe" (
        set "CSS_PATH=D:\SteamLibrary\steamapps\common\Counter-Strike Source"
    )
)

:: 4. If still not found, ask user
if not defined CSS_PATH (
    echo [!] Auto-detection could not locate CS:S directory.
    set /p CSS_PATH="Enter your CS:S game folder path (e.g. D:\css): "
)

if not exist "!CSS_PATH!\cstrike" (
    echo.
    echo [-] ERROR: Could not find 'cstrike' folder in '!CSS_PATH!'.
    echo [-] Please make sure you entered the correct Counter-Strike: Source folder.
    pause
    exit /b 1
)

echo [+] Target CS:S Directory: !CSS_PATH!\cstrike
echo.

:: Determine scripts/models folder location
set "MODELS_SRC="
if exist "scripts\models" (
    set "MODELS_SRC=scripts\models"
) else if exist "..\scripts\models" (
    set "MODELS_SRC=..\scripts\models"
) else if exist "%~dp0scripts\models" (
    set "MODELS_SRC=%~dp0scripts\models"
)

:: Create custom folder in cstrike
if not exist "!CSS_PATH!\cstrike\custom" mkdir "!CSS_PATH!\cstrike\custom"

:: Install Akame
if exist "!MODELS_SRC!\akame" (
    echo [*] Installing Akame (Akame ga Kill) Skin into custom/akame...
    set "AKAME_DEST=!CSS_PATH!\cstrike\custom\akame"
    
    mkdir "!AKAME_DEST!\models\player" 2>nul
    mkdir "!AKAME_DEST!\materials" 2>nul
    
    xcopy /E /I /Y "!MODELS_SRC!\akame\materials" "!AKAME_DEST!\materials" >nul
    
    :: Copy as native legion path
    mkdir "!AKAME_DEST!\models\player\legion\akame" 2>nul
    xcopy /E /I /Y "!MODELS_SRC!\akame\models" "!AKAME_DEST!\models" >nul
    
    :: Also copy as ct_urban and t_phoenix replacements for 100% offline & server instant compatibility!
    copy /Y "!MODELS_SRC!\akame\models\player\legion\akame\akame_fix.mdl" "!AKAME_DEST!\models\player\ct_urban.mdl" >nul
    copy /Y "!MODELS_SRC!\akame\models\player\legion\akame\akame_fix.dx90.vtx" "!AKAME_DEST!\models\player\ct_urban.dx90.vtx" >nul
    copy /Y "!MODELS_SRC!\akame\models\player\legion\akame\akame_fix.dx80.vtx" "!AKAME_DEST!\models\player\ct_urban.dx80.vtx" >nul
    copy /Y "!MODELS_SRC!\akame\models\player\legion\akame\akame_fix.vvd" "!AKAME_DEST!\models\player\ct_urban.vvd" >nul
    copy /Y "!MODELS_SRC!\akame\models\player\legion\akame\akame_fix.phy" "!AKAME_DEST!\models\player\ct_urban.phy" >nul

    copy /Y "!MODELS_SRC!\akame\models\player\legion\akame\akame_fix.mdl" "!AKAME_DEST!\models\player\t_phoenix.mdl" >nul
    copy /Y "!MODELS_SRC!\akame\models\player\legion\akame\akame_fix.dx90.vtx" "!AKAME_DEST!\models\player\t_phoenix.dx90.vtx" >nul
    copy /Y "!MODELS_SRC!\akame\models\player\legion\akame\akame_fix.dx80.vtx" "!AKAME_DEST!\models\player\t_phoenix.dx80.vtx" >nul
    copy /Y "!MODELS_SRC!\akame\models\player\legion\akame\akame_fix.vvd" "!AKAME_DEST!\models\player\t_phoenix.vvd" >nul
    copy /Y "!MODELS_SRC!\akame\models\player\legion\akame\akame_fix.phy" "!AKAME_DEST!\models\player\t_phoenix.phy" >nul

    echo [SUCCESS] Akame skin installed successfully!
)

:: Install Cissia ZZZ
if exist "!MODELS_SRC!\cissia_zzz" (
    echo.
    echo [*] Installing Cissia ZZZ (Zenless Zone Zero) Skin into custom/cissia_zzz...
    set "CISSIA_DEST=!CSS_PATH!\cstrike\custom\cissia_zzz"
    
    mkdir "!CISSIA_DEST!\models\player" 2>nul
    mkdir "!CISSIA_DEST!\materials" 2>nul
    
    xcopy /E /I /Y "!MODELS_SRC!\cissia_zzz\materials" "!CISSIA_DEST!\materials" >nul
    xcopy /E /I /Y "!MODELS_SRC!\cissia_zzz\models" "!CISSIA_DEST!\models" >nul
    
    :: Copy as ct_sas and t_leet replacements for 100% offline & server instant compatibility!
    set "CISSIA_SRC_MDL=!MODELS_SRC!\cissia_zzz\models\sneaky_holy\neps\powered_by_nidegg\best_zombie_escape_server\thick_snake\cissia_zzz"
    copy /Y "!CISSIA_SRC_MDL!.mdl" "!CISSIA_DEST!\models\player\ct_sas.mdl" >nul
    copy /Y "!CISSIA_SRC_MDL!.dx90.vtx" "!CISSIA_DEST!\models\player\ct_sas.dx90.vtx" >nul
    copy /Y "!CISSIA_SRC_MDL!.dx80.vtx" "!CISSIA_DEST!\models\player\ct_sas.dx80.vtx" >nul
    copy /Y "!CISSIA_SRC_MDL!.vvd" "!CISSIA_DEST!\models\player\ct_sas.vvd" >nul

    copy /Y "!CISSIA_SRC_MDL!.mdl" "!CISSIA_DEST!\models\player\t_leet.mdl" >nul
    copy /Y "!CISSIA_SRC_MDL!.dx90.vtx" "!CISSIA_DEST!\models\player\t_leet.dx90.vtx" >nul
    copy /Y "!CISSIA_SRC_MDL!.dx80.vtx" "!CISSIA_DEST!\models\player\t_leet.dx80.vtx" >nul
    copy /Y "!CISSIA_SRC_MDL!.vvd" "!CISSIA_DEST!\models\player\t_leet.vvd" >nul

    echo [SUCCESS] Cissia ZZZ skin installed successfully!
)

:: If a file or folder was dragged and dropped onto this batch file
if not "%~1"=="" (
    echo.
    echo [*] Processing Drag-and-Drop file/folder: %~1
    if exist "%~1\materials" (
        mkdir "!CSS_PATH!\cstrike\custom\%~n1" 2>nul
        xcopy /E /I /Y "%~1" "!CSS_PATH!\cstrike\custom\%~n1" >nul
        echo [SUCCESS] Drag-and-drop custom mod '%~n1' installed!
    )
)

echo.
echo ==========================================================================
echo  ALL CUSTOM SKINS (AKAME & CISSIA ZZZ) INSTALLED SUCCESSFULLY!
echo ==========================================================================
echo.
echo [INFO] How to view in game:
echo  1. Open CS:S and play Offline or Online.
echo  2. Choose CT (Urban / SAS) or T (Phoenix / Leet).
echo  3. Or open cheat menu [INSERT] -^> Visuals -^> Select [Urban / Seal (CT)] or [Phoenix (T)]!
echo.
pause
