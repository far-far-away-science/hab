net start WinRM
Set-Item WSMan:\localhost\Client\TrustedHosts -Value 192.168.1.103
remove-module psreadline -force
Enter-PsSession -ComputerName 192.168.1.103 -Credential 192.168.1.103\Administrator
