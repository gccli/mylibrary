@echo off
@set PATH=%PATH%;%~dp0public\bin
nircmdc shortcut "%~dp0ShowWindow.bat" "~$folder.programs$\MySelf" "ShowWindow[0]" "0" "" "" "" "" "Ctrl+Alt+0"
nircmdc.exe wait 200
nircmdc shortcut "%~dp0ShowWindow.bat" "~$folder.programs$\MySelf" "ShowWindow[1]" "1" "" "" "" "" "Ctrl+Alt+1"
nircmdc.exe wait 200
nircmdc shortcut "%~dp0ShowWindow.bat" "~$folder.programs$\MySelf" "ShowWindow[2]" "2" "" "" "" "" "Ctrl+Alt+2"
nircmdc.exe wait 200
nircmdc shortcut "%~dp0ShowWindow.bat" "~$folder.programs$\MySelf" "ShowWindow[3]" "3" "" "" "" "" "Ctrl+Alt+3"
nircmdc.exe wait 200

nircmdc shortcut "%~dp0parallel_start_work.bat" "~$folder.programs$\MySelf" "GoAgent Start All" "" "" "" "" "" "Ctrl+Alt+S"
nircmdc.exe wait 200
nircmdc shortcut "%~dp0parallel_stop.bat" "~$folder.programs$\MySelf" "GoAgent Stop All" "" "" "" "" "" "Ctrl+Alt+Q"
nircmdc.exe wait 200
