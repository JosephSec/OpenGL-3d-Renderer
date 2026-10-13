@echo off

for %%I in ("%~dp0..") do set "ParentPath=%%~fI"

set PATH=%ParentPath%\bin;%PATH%
program.exe