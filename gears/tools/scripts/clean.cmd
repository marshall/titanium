@echo off

:: tools\scripts\clean.cmd

:: after gearing up, runs "make clean"

setlocal

call "%~dp0gear-up.cmd"
if errorlevel 1 goto :end

cd %~dp0..\..

make clean

:end

endlocal
