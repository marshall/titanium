@echo off

:: tools\scripts\build.cmd

:: after gearing up, builds specified targets

setlocal

call "%~dp0gear-up.cmd"
if errorlevel 1 goto :end

cd %~dp0..\..

set g_params=

:process_params
:: Loop label for iterative processing of parameters
:: does primitive validation (checks for pairs of params, basically)

if [%1] neq [] (
  if [%2] neq [] (
    if [%1] == [BROWSER] (
      if [%g_ms%] == [VC] (
        if [%2] neq [FF] (
          echo Unable to build specified target with Visual C++ Express
          goto :end
        )
      )
    )
    set g_params=%g_params% %1=%2
    shift & shift
    goto :process_params
  )
)

make%g_params%

set g_params=

:end

endlocal
