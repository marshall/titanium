@echo off

:: tools\scripts\gear-up.cmd

:: using registry, verifies:
:: * Microsoft Visual Studio or Visual C++ Express installation
:: * Visual Studio 2005 Service Pack 1 installation

:: then, verfies presence of other development tools
:: using simple CSV path->action manifest

:: the manifest file can be then edited independently to update
:: tools requirements

:: also reads custom-paths.txt, a file that is used to provide
:: custom paths for the environments, where the tools are not
:: installed at default locations

:: TODO: consider converting VS detection into an external script
::    that is part of the manifest

:: TODO: implement "success" marker file, which indicates that the
::    requirements were met in prior runs

:: TODO: consider implementing selective initialization, specified
::    as parameters, i.e. "svn vc unx"

:: TODO: rewrite as simple bootstrap to Python, then do the rest of 
::    gearing up in Python

:prepare

:: if previously run in this environment, exit immediately
  if defined g_success goto :eof

:: save PATH, load custom paths, get ready to gear up

  pushd %~dp0
  
  :: initialize custom paths from first line (the only line, hopefully)
  :: of ./custom-paths.txt
  if exist custom-paths.txt set /p g_custom_paths=<custom-paths.txt
  
  :: create log file var
  set g_log=%temp%\req-%random%.log
  :: create misses log file var
  set g_miss=%temp%\req-miss-%random%.log
  :: create action batch file var
  set g_act=%temp%\req-act-%random%.cmd
  
  :: initialize log file
  echo. > %g_log%
  echo Checking requirements: >> %g_log%
  echo. >> %g_log%

  :: initialize action batch file
  :: PATH intentionally truncated to avoid UnxUtils losing end of the path
  :: for especially long paths
  echo set path=%windir%;%windir%\system32>%g_act%
  if defined g_custom_paths (
    echo set path=%%path%%;%g_custom_paths%>%g_act%
  )
  
  :: start positive!
  set g_success=yes
 
:check_ms

:: check for Microsoft visual Studio installation by querying registry:
::    * DevDiv\VS entry only exists when Visual Studio (including
::      VSTS and VSTF) is installed
::    * DevDiv\VC exists when Visual C++ Express is installed, so
::      check "VS" first to eliminate ambiguity
:: as a result, g_ms variable is set to "VC" if Visual C++ Express is
:: installed, or "VS" if Visual Studio or Team System is installed

  :: reset in case script is run multiple times in the same environment
  set g_ms=

  reg query "HKLM\Software\Microsoft\DevDiv\VS" >nul 2>&1
  if errorlevel 1 (
    reg query "HKLM\Software\Microsoft\DevDiv\VC" >nul 2>&1
    if errorlevel 1 (
      call :not_found "Visual Studio 2005 Professional or Visual C++ Express"
      echo "C:\Program Files\Microsoft Visual Studio 8","http://msdn2.microsoft.com/en-us/library/bb964526.aspx","Visual Studio 2005">>%g_miss%
      goto :done_ms
    )
    set g_ms=VC
    call :found "Visual C++ Express 2005"
    goto :check_vsp
  )
  set g_ms=VS
  call :found "Visual Studio 2005"

:check_vsp

:: check for Service Pack 1

  set g_vsp_str="Visual Studio 2005 Service Pack 1 or greater"

  for /F "skip=4 tokens=3 delims=		" %%a in (
    'reg query "HKLM\Software\Microsoft\DevDiv\%g_ms%\Servicing\8.0" /v SP'
  ) do (
    set g_sp=%%a
  )
  
  if defined g_sp (
    if %g_sp:~-1% lss 1 (
      call :not_found %g_vsp_str%
      echo "-","http://msdn2.microsoft.com/en-us/vstudio/bb265237.aspx","%g_vsp_str%">>%g_miss%
      goto :done_ms
    )
  )
    
  call :found %g_vsp_str%
  
  :: initialize VC vars
  call :call_script "%VS80COMNTOOLS%vsvars32.bat"
  
  :: clean up
  set g_vsp_str=
  
:done_ms

:check_tools

:: reads reqs manifest, builds new PATH and 
:: calls scripts, depending on the command (2nd column) of the
:: manifest

:: manifest format is CSV (do not double-quote the values), columns are:
:: 1 - key to be used in selective gear-up
:: 2 - recommended install path (if empty, 3 is used)
:: 3 - path to file to check for existence
:: 4 - command if found (c = call batch file, p = append to PATH)
:: 5 - url to download if not found
:: 6 - description of the tool
  
  :: g - key
  :: h - instal path
  :: i - path
  :: j - command
  :: k - url
  :: l - description
  for /f "tokens=1,2,3,4,5,6* delims=," %%g in (win32-reqs.csv) do (
    if exist "%%i" (
      call :found "%%l"
      if %%j == p (
        call :append_path "%%i"
      ) else (
        call :call_script "%%i"
      )) else (       
        call :locate "%%i" "%%k" "%%l" "%%h"
      ))
      
  call :display_log

  if defined g_success (
    call "%g_act%"
  ) else (
    call :misses
  )
  
  :: return to starting dir
  popd
  
  call :clean_up
  
  :: setting ERRORLEVEL to 1
  if not defined g_success color 00
  
  goto :eof

:display_log

  type %g_log%
  :: cap the log
  echo.
  echo Requirements check complete.
  
  goto :eof

:clean_up

:: clean up environment upon exit
:: remove all transient variables and files

  del /q %g_log% 2>nul
  set g_log=
  del /q %g_miss% 2>nul
  set g_miss=
  del /q %g_act% 2>nul
  set g_act=
  
  set g_which=
  set g_bool=
  set g_sp=
  
  goto :eof

:append_path

:: has to be done as a subroutine, because "for" loop evaluates
:: variables once, at the beginning of the loop

  echo set path=%%path%%;%~dp1>>%g_act%
  goto :eof

:call_script

:: creates an entry in g_act to call specified script
::    %1 - script path

  echo call %1^>nul>>%g_act%
  goto :eof

:found

:: states that the tool was successfully detected

  echo + %~1>>%g_log%
  goto :eof


:not_found

:: states that the tool wasn't found

  :: drop success flag
  set g_success=

  echo - %~1>>%g_log%
  goto :eof

:locate

:: provides an opportunity for the user to locate/download the tool
:: parameters:
::    %1 - path (it is already known at this point that the tool doesn't
::        exist at this path)
::    %2 - url to download from
::    %3 - friendly name/description of the tool
::    %4 - suggested install path

:: find using path varible
  call :which %~nx1
  if defined g_which (
    call :found %3
    call :append_path "%g_which%"
    goto :eof
  )
  
  call :not_found %3
  
  :: create miss entry
  echo "%4",%2,%3>>%g_miss%
  
  goto :eof

:misses


  echo Some of the required tools were not found (items with the "-" prefix)
  echo in the list above)
  echo.
  echo Make sure the path to the tools is in your the PATH variable
  echo.
  echo You can also create a "custom-paths.txt" file in tools\scripts
  echo directory and put all paths to tools there on one line, separated by 
  echo semicolon. This would allow you to not have to make global
  echo changes to your environment.
  echo.
  echo If the tool is not installed, download and install it from 
  echo locations, specified below:
   
  :: g - path
  :: h - url
  :: i - description
  for /f "tokens=1,2,3* delims=," %%g in (%g_miss%) do (
    echo.
    echo %%~i, installed from:
    echo    %%~h
    if %%g neq "-" (
      echo When installing, consider putting it here to avoid the need
      echo to edit "custom-paths.txt" file:
      echo    %%~g
    )
    echo.
    call :ask_to_download %%h
  )
  goto :eof

:ask_to_download

  :: this has to be a subroutine, because FOR only evaluates vars once
  ::    %1 -- url to download
  
    set g_bool=
    set /p g_bool=Would you like to start the download [y/N]?
    if /i [%g_bool%] == [y] rundll32 url,FileProtocolHandler %1
    
    goto :eof
  
:which
 
:: equivalent of UNIX which command, sets g_which var 

  set g_which=%~$PATH:1
  goto :eof

:eof
