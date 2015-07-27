@echo off
@set PATH=%PATH%;%~dp0public\bin;

@IF %1 EQU 0 (
    call:myshowhide 8087
  )
@IF %1 EQU 1 (
    call:myshowhide 8088
  )
@IF %1 EQU 2 (
    call:myshowhide 8089
  )
@IF %1 EQU 3 (
    call:myshowhide 8090
  )

REM @echo %1
REM nircmdc.exe wait 10

:myshowhide
@IF EXIST %TMP%\%~1.lock (
  DEL %TMP%\%~1.lock
  nircmdc win show title %~1
  nircmdc win activate title %~1
) ELSE (
  TYPE NUL > %TMP%\%~1.lock
  nircmdc win hide title %~1
)
goto:eof