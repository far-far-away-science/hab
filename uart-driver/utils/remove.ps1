devcon.exe remove ACPI\BCM2836
$devices = devcon dp_enum
foreach ($d in $devices)
{
    if ($d.StartsWith('oem') -and $d.EndsWith('.inf'))
    {
        devcon.exe dp_delete $d;
    }
}
cmd /c tracing.bat stop
shutdown /r /t 0;
