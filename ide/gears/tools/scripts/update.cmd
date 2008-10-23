@echo off

:: tools\scripts\update.cmd

:: after gearing up, updates working copy from repository

setlocal

call "%~dp0\gear-up.cmd"
if errorlevel 1 goto :end

cd %~dp0..\..

svn update .

:end

endlocal