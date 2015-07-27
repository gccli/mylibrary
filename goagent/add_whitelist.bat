@ECHO OFF

ECHO.
ECHO Demonstration of receiving user input through the TYPE CON command.
ECHO Type in any string and close by pressing Enter, F6 (or Ctrl+Z), Enter.
ECHO Only the last non-empty line will be remembered, leading spaces are ignored.
ECHO.

:: Only one single command line is needed to receive user input
FOR /F "tokens=*" %%A IN ('TYPE CON') DO SET INPUT=%%A
:: Use quotes if you want to display redirection characters as well
ECHO Domain list: "%INPUT%"
set /p yes="Are you sure: (y/n)? "

IF %yes% EQU y (
    CD client
    python27.exe proxystatistics.py -a "%INPUT%"
)

pause
