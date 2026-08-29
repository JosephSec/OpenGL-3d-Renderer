@echo off

setlocal enabledelayedexpansion

for %%I in ("%~dp0..") do set "ParentPath=%%~fI"

set "GLOBAL_FOLDER=%ParentPath%\assets\%~1"
shift


@REM DLL FILES
if not exist %GLOBAL_FOLDER%\bin mkdir %GLOBAL_FOLDER%\bin

set DLL_FILE=%ParentPath%\bin\zengine.dll
if not exist %DLL_FILE% (
  echo dll file not found "%DLL_FILE%"
  exit
)

copy /y %DLL_FILE% %GLOBAL_FOLDER%\bin\
echo copied dll to global bin folder
@REM DLL FILES

@REM HPP FILES
:loop
if "%~1"=="" goto :end
  set HPP_FOLDER_FROM=%ParentPath%\include\%~1\
  set HPP_FOLDER_TO=%GLOBAL_FOLDER%\include\%~1\

  xcopy !HPP_FOLDER_FROM! !HPP_FOLDER_TO! /S /E /H /Y /I

  echo copied hpp files from "!HPP_FOLDER_FROM!" to "!HPP_FOLDER_TO!"

  shift
  goto :loop
:end
@REM HPP FILES