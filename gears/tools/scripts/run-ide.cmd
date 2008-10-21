@echo off

:: tools\scripts\run-ide.cmd

:: after gearing up:
:: * (re)generates gears.vcproj
:: * launches Visual Studio or Visual C++ Express

setlocal

call "%~dp0\gear-up.cmd"
if errorlevel 1 goto :end

cd %~dp0..

:: generate gears.vcproj
python gen_vs_project.py

:: launch the project
gears.vcproj

:end

endlocal
