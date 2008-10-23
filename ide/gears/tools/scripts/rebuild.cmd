@echo off

:: tools\scripts\rebuild.cmd

:: combines "clean" and "build"
:: to provide Re-build command in VS project

call "%~dp0clean.cmd"
call "%~dp0build.cmd" %*