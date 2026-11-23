@echo off
setlocal
set "SDR_TEST_ROOT=E:\POCSAG SDR Pro\tests\profil-portable"
if not exist "%SDR_TEST_ROOT%\temp" mkdir "%SDR_TEST_ROOT%\temp"
if not exist "%SDR_TEST_ROOT%\temp" exit /b 1
set "TEMP=%SDR_TEST_ROOT%\temp"
set "TMP=%TEMP%"
"%~dp0POCSAG-SDR-Pro.exe" --isolated-profile "%SDR_TEST_ROOT%"
