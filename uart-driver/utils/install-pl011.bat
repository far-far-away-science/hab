devcon.exe dp_add .\uart-pl011.inf
cmd /c tracing.bat start
devcon.exe disable ACPI\BCM2837
devcon.exe enable ACPI\BCM2837
