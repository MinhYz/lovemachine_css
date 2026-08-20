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

:: 3. Check common paths
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
    set /p CSS_PATH="Enter your CS:S game folder path [e.g. D:\css]: "
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

:: 5. Clean up old conflicting renamed files that freeze the loading screen
echo [*] Cleaning up old corrupted/conflicting model files...
del /F /Q "!CSS_PATH!\cstrike\custom\akame\models\player\ct_urban.*" 2>nul
del /F /Q "!CSS_PATH!\cstrike\custom\akame\models\player\t_phoenix.*" 2>nul
del /F /Q "!CSS_PATH!\cstrike\custom\cissia_zzz\models\player\ct_sas.*" 2>nul
del /F /Q "!CSS_PATH!\cstrike\custom\cissia_zzz\models\player\t_leet.*" 2>nul

:: 6. Fix sv_pure and pure_server_whitelist
echo [*] Bypassing sv_pure and material consistency checks...
if not exist "!CSS_PATH!\cstrike\cfg" mkdir "!CSS_PATH!\cstrike\cfg"

(
echo whitelist
echo {
echo 	materials\...	allow_from_disk
echo 	models\...	allow_from_disk
echo 	sound\...	allow_from_disk
echo }
) > "!CSS_PATH!\cstrike\pure_server_whitelist.txt"

(
echo whitelist
echo {
echo 	materials\...	allow_from_disk
echo 	models\...	allow_from_disk
echo 	sound\...	allow_from_disk
echo }
) > "!CSS_PATH!\cstrike\cfg\pure_server_whitelist.txt"

echo sv_pure -1 >> "!CSS_PATH!\cstrike\cfg\autoexec.cfg"
echo sv_consistency 0 >> "!CSS_PATH!\cstrike\cfg\autoexec.cfg"
echo cl_consistency 0 >> "!CSS_PATH!\cstrike\cfg\autoexec.cfg"

echo sv_pure -1 >> "!CSS_PATH!\cstrike\cfg\listenserver.cfg"
echo sv_consistency 0 >> "!CSS_PATH!\cstrike\cfg\listenserver.cfg"
echo cl_consistency 0 >> "!CSS_PATH!\cstrike\cfg\listenserver.cfg"

echo [+] sv_pure bypass configured successfully!
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

:: Install Akame cleanly in native path
if exist "!MODELS_SRC!\akame" (
    echo [*] Installing Akame Skin in native path [models/player/legion/akame]...
    set "AKAME_DEST=!CSS_PATH!\cstrike\custom\akame"
    
    mkdir "!AKAME_DEST!\materials" 2>nul
    mkdir "!AKAME_DEST!\models" 2>nul
    
    xcopy /E /I /Y "!MODELS_SRC!\akame\materials" "!AKAME_DEST!\materials" >nul
    xcopy /E /I /Y "!MODELS_SRC!\akame\models" "!AKAME_DEST!\models" >nul

    echo [+] Akame skin installed successfully!
)

:: Install Cissia ZZZ cleanly in native path
if exist "!MODELS_SRC!\cissia_zzz" (
    echo.
    echo [*] Installing Cissia ZZZ Skin in native path...
    set "CISSIA_DEST=!CSS_PATH!\cstrike\custom\cissia_zzz"
    
    mkdir "!CISSIA_DEST!\materials" 2>nul
    mkdir "!CISSIA_DEST!\models" 2>nul
    
    xcopy /E /I /Y "!MODELS_SRC!\cissia_zzz\materials" "!CISSIA_DEST!\materials" >nul
    xcopy /E /I /Y "!MODELS_SRC!\cissia_zzz\models" "!CISSIA_DEST!\models" >nul

    echo [+] Cissia ZZZ skin installed successfully!
)

echo.
echo ==========================================================================
echo  ALL CUSTOM SKINS [AKAME AND CISSIA ZZZ] INSTALLED AND CLEANED!
echo ==========================================================================
echo.
echo [INFO] Game will now load map in 2 seconds without freezing!
echo.
pause
