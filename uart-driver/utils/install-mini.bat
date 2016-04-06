devcon.exe dp_add .\uart-mini.inf
cmd /c tracing.bat start
devcon.exe disable ACPI\BCM2836
devcon.exe enable ACPI\BCM2836
