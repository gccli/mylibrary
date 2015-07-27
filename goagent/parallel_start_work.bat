@REM Install telnet : pkgmgr /iu:"telnetclient"
@REM ASSOC .js=JSFile

@echo off
@assoc .py=Python.File >nul
@ftype Python.File="C:\Python27\python.exe" "%1" %* >nul

set _SCREENW_=1920
set _SCREENH_=1080

REM ----- LEFT WINDOW PROPERTIES
set /a _WINLEFTX_=30
set /a _WINLEFTY_=30
set /a _WINLEFTW_=(%_SCREENW_% / 2) - 30
set /a _WINLEFTH_=(%_SCREENH_% - 280)

REM ----- RIGHT WINDOW PROPERTIES
set /a _WINRIGHTX_=(%_WINLEFTX_%) + (%_WINLEFTW_%)
set /a _WINRIGHTY_=(%_WINLEFTY_%)
set /a _WINRIGHTW_=(%_WINLEFTW_%)
set /a _WINRIGHTH_=(%_WINLEFTH_%)
set _WAITTIME_=200


@set GEVENT_LOOP=uvent.loop.UVLoop
@set GEVENT_RESOLVER=block
@set GOAGENT_LISTEN_VISIBLE=1
@set GOAGENT_PATH=%~dp0client

set PATH=%PATH%;%~dp0public\bin;
@CD %GOAGENT_PATH%

REM -------- LEFT TOP --------
@START "Goagent" python27.exe proxy.py -p 8087 jingccli pr01linux pr02linux pr03linux
nircmdc.exe wait %_WAITTIME_%
nircmdc.exe win setsize foreground %_WINLEFTX_% %_WINLEFTY_% %_WINLEFTW_% %_WINLEFTH_%

@START "Goagent" python27.exe proxy.py -p 8088 -n pr01linux pr02linux pr03linux
nircmd.exe wait %_WAITTIME_%
nircmd.exe win setsize foreground %_WINRIGHTX_% %_WINRIGHTY_% %_WINRIGHTW_% %_WINRIGHTH_%


REM -------- LEFT BOTTOM --------
set /a _WINLEFTY_=240
set /a _WINRIGHTY_=(%_WINLEFTY_%)

@START "Goagent" python27.exe proxy.py -p 8089 -n pr02linux pr03linux pr01linux
nircmd.exe wait %_WAITTIME_%
nircmdc.exe win setsize foreground %_WINLEFTX_% %_WINLEFTY_% %_WINLEFTW_% %_WINLEFTH_%

@START "Goagent" python27.exe proxy.py -p 8090 -n pr03linux pr01linux pr02linux
nircmd.exe wait %_WAITTIME_%
nircmd.exe win setsize foreground %_WINRIGHTX_% %_WINRIGHTY_% %_WINRIGHTW_% %_WINRIGHTH_%

@tasklist /FI "IMAGENAME eq python27.exe"
@timeout /T 3
@CD %~dp0