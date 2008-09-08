@echo off
:: Generates an HTML file containing a visual diff of a GVN changebranch.

if "%~1"=="" (
  echo Usage: gvn_review.bat "my_username/my_change"
  goto :EOF
)

setlocal

:: Look for Python in //googleclient/third_party/, where Google developers would
:: typically have it. Otherwise, run gears-up.cmd which will find/install it for
:: external developers.
set PYTHON_SETUP="%~dp0..\..\..\..\..\third_party\python_24\setup_env.bat"
if exist %PYTHON_SETUP% (
  call %PYTHON_SETUP%
) else (
  call ="%~dp0..\scripts\gear-up.cmd"
)

:: Look for the SVN command line client in //googleclient/third_party/.
:: Otherwise, assume SVN is in the path (you must have gotten this code
:: somehow :)).
set SVN_SETUP="%~dp0..\..\..\..\..\third_party\subversion\setup_env.bat"
if exist %SVN_SETUP% (
  call %SVN_SETUP%
)

python "%~dp0\gvn_review.py" "%~1"

endlocal
