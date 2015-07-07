net start WinRM
Set-Item WSMan:\localhost\Client\TrustedHosts -Value hab-iot -force
remove-module psreadline -force
Enter-PsSession -ComputerName hab-iot -Credential hab-iot\Administrator
