@echo off

for %%I in ("%~dp0..") do set "ParentPath=%%~fI"
set "RELEASE_FROM=%ParentPath%\build\release"
set "RELEASE_TO=%ParentPath%\release"

if not exist "%RELEASE_FROM%" mkdir "%RELEASE_FROM%"

xcopy "%ParentPath%\assets" "%RELEASE_FROM%\assets" /E /I /H /Y
copy "%ParentPath%\build\program.exe" "%RELEASE_FROM%\assets"

copy "%ParentPath%\build\Launch.bat" "%RELEASE_FROM%\"
copy "%ParentPath%\build\program.exe" "%RELEASE_FROM%\assets"

if not exist "%RELEASE_FROM%\bin" mkdir "%RELEASE_FROM%\bin"
"%ParentPath%\build\dllfinder\dllfinder.exe" "%RELEASE_FROM%\assets\program.exe" "%RELEASE_FROM%\bin" "%ParentPath%\bin"

pause