@echo off

set TRACE_NAME=uart-driver
set TRACE_LOG_PATH=%SystemRoot%\Tracing\%TRACE_NAME%
set PROVIDER_GUID=#37f79f75-a9ac-46ad-b69c-61d2e9aa5bd5
set TMFFILE=uart-driver.tmf
set TRACE_FORMAT_PREFIX=%%2!-20.20s!%%!FUNC!-
set TRACE_FLAGS=0xFFFF
set TRACE_LEVEL=5

set WIN_KITS_PATH=C:\Program Files (x86)\Windows Kits\10\bin\x64

set command=%1

if /I %command% == start (
    echo starting trace
    if not exist "%TRACE_LOG_PATH%" (
        md "%TRACE_LOG_PATH%"
    )
    tracelog.exe -start "%TRACE_NAME%" -seq 10 -rt -guid %PROVIDER_GUID% -flags %TRACE_FLAGS% -level %TRACE_LEVEL% -f "%TRACE_LOG_PATH%\%TRACE_NAME%.etl"
)
if /I %command% == stop (
    echo stopping trace
    tracelog.exe -flush "%TRACE_NAME%"
    tracelog.exe -stop "%TRACE_NAME%"
)
if /I %command% == create-tmf (
    "%WIN_KITS_PATH%\tracepdb.exe" -f ..\ARM\Release\uart-driver.pdb -p . -o "%TMFFILE%"
)
if /I %command% == format (
    "%WIN_KITS_PATH%\tracefmt.exe" "%TRACE_NAME%.etl" -tmf "%TMFFILE%" -nosummary -o "TRACE_NAME.txt"
)
