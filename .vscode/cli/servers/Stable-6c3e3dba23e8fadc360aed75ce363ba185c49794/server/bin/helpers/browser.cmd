@echo off
setlocal
set ROOT_DIR=%~dp0..\..
call "%ROOT_DIR%\node.exe" "%ROOT_DIR%\out\server-cli.js" "code" "1.81.1" "6c3e3dba23e8fadc360aed75ce363ba185c49794" "code.cmd" "--openExternal" %*
endlocal
