@echo off
setlocal

:: Define OSP2 URL and destination path
set "OSP2_URL=https://scoqx.github.io/assets/zz-osp-pak8be.pk3"
set "OSP2_FILE=%~dp0osp\zz-osp-pak8be.pk3"

cls
echo ==========================================
echo             UPDATING OSP2
echo ==========================================
echo.

:: Ensure the target directory exists
if not exist "%~dp0osp" (
    echo Creating directory "%~dp0osp"... 
    mkdir "%~dp0osp"
)

:: Download the OSP2 file
curl -L --retry 3 --progress-bar -o "%OSP2_FILE%" "%OSP2_URL%"
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: OSP2 download failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo OSP2 updated successfully!
pause
exit /b 0
