@echo off

set TRACE_NAME=uart
set TRACE_LOG_PATH=%SystemRoot%\Tracing\%TRACE_NAME%
set PROVIDER_GUID=#37f79f75-a9ac-46ad-b69c-61d2e9aa5bd5
set TMFFILE=uart.tmf
set TRACE_FLAGS=0xFFFF
set TRACE_LEVEL=5

set WIN_KITS_PATH=C:\Program Files (x86)\Windows Kits\10\bin\x64

set command=%1

if /I %command% == start (
    echo starting trace
    if not exist "%TRACE_LOG_PATH%" (
        md "%TRACE_LOG_PATH%"
    )
    tracelog.exe -start "%TRACE_NAME%" -seq 10 -rt -guid %PROVIDER_GUID% -flags %TRACE_FLAGS% -level %TRACE_LEVEL% -f "%TRACE_LOG_PATH%\%TRACE_NAME%.etl" -UseSystemTime
)
if /I %command% == stop (
    echo stopping trace
    tracelog.exe -flush "%TRACE_NAME%"
    tracelog.exe -stop "%TRACE_NAME%"
)
if /I %command% == format (
    mkdir ./!tracing-out!
    "%WIN_KITS_PATH%\tracepdb.exe" -f ..\ARM\Release\uart-*.pdb -p ./!tracing-out! -o ./!tracing-out!/"%TMFFILE%"
    "%WIN_KITS_PATH%\tracefmt.exe" ./!tracing-out!/"%TRACE_NAME%.etl" -tmf ./!tracing-out!/"%TMFFILE%" -p ./!tracing-out! -nosummary -o ./!tracing-out!/"TRACE_NAME.txt"
)
