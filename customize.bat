@echo off
@echo ======== Set environment variables for windows ========

set root_path=E:

set temp_dir="%root_path%\TEMP"
set tool_dir="%root_path%\Tools"
set prog_dir="%root_path%\Projects"
set cache_dir="%root_path%\Cache"
set download_dir="%root_path%\Downloads"


@echo Temp    : %temp_dir%
@echo Tools   : %tool_dir%
@echo Project : %prog_dir%

pause

@if not exist %temp_dir% (
	md %temp_dir%
	md %tool_dir%\bin
)
@if not exist %tool_dir% (
	md %tool_dir%
)
@if not exist %prog_dir% (
	md %prog_dir%
)
@if not exist %cache_dir% (
	md %cache_dir%
	md %cache_dir%\QQ
	md %cache_dir%\VC
)
@if not exist %download_dir% (
	md %download_dir%
)


@setx TEMP %temp_dir%
@setx TMP  %temp_dir%
@setx TEMP %temp_dir% /m
@setx TMP  %temp_dir% /m
@setx TOOL_BASE %tool_dir%

@setx Path %tool_dir%\bin";%tool_dir%\odbg110

@echo ======== End for Setting environment variables ========
@pause
