
$xastir=(gc .\n6udp\logs\tnc.txt)+(gc .\n6udp\logs\net.txt)
$aprsfi=(gc .\aprsfi\n6udp.txt)+(gc .\aprsfi\kg7wfr.txt)
$singlexastir = for ($i=0;$i -lt $xastir.Count;$i+=2) { $xastir[$i],$xastir[$i+1] -join ": " }
$objects=($singlexastir+$aprsfi) |%{
    if($_ -match "(?<date>.*): (?<call>\S+)>.*:(=|@\d+z)(?<lat>\S+)\/(?<lon>\S+)(x\/|>.*)(A|a)=(?<alt>\d+)") {
        #$matches.Remove(0)
        New-Object pscustomobject -Property $matches
    } else {
        #$_
    }
} | ?{
    #hack for fix loss
    #[int]$_.alt -ne 11994
    if($_.0 -match "T#\d+,\d+"){
        if($_.0 -match "T#\d+,001") {
            $true
        } else {
            #bad GPS
            $false
        }
    } else {
        $true
    }
} | select @{
    n="alt"
    e={
        if($_.call -eq "KG7WFR-11") {
            #hack for our bad alt
            ([double]$_.alt)*3.280839895
        } else {
            [double]$_.alt
        }
    }
},@{
    n="lon"
    e={
        if($_.lon -match "(?<degrees>\d{2,3})(?<minutes>\d{2})(?<secondsdec>\.\d{2})(?<dir>\S)"){
            $dec=([double]$Matches.degrees)+([double]$Matches.minutes)/60+([double]$Matches.secondsdec)/60
            if($Matches.dir -eq "W"){
                $dec * -1
            } else {
                $dec
            }
        }
    }
},@{
    n="lat"
    e={
        if($_.lat -match "(?<degrees>\d{2})(?<minutes>\d{2})(?<secondsdec>\.\d{2})(?<dir>\S)"){
            $dec=([double]$Matches.degrees)+([double]$Matches.minutes)/60+([double]$Matches.secondsdec)/60
            if($Matches.dir -ne "N"){
                $dec * -1
            } else {
                $dec
            }
        }
    }
},@{
    n="date"
    e={
        #hack for date
        if($_.date -match "# (?<unix>\d+)"){
            (get-date -Format "s" -Date (New-Object "DateTime" -argumentlist @(1970, 1, 1, 0, 0, 0, 0)).ToLocalTime().AddSeconds([int]$Matches.unix).AddHours(1)) + "-07:00"
        } elseif($_.date -match "^(.+) UTC") {
            (get-date -Format "s" -Date ([datetime]$matches[1]).ToLocalTime()) + "-07:00"
        }
    }
},call

$kml=@"
<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://earth.google.com/kml/2.2">
<Document>
<name>APRS Data</name>
<open>1</open>
<description>Track log for V1 Balloon</description>

<StyleMap id="msn_label">
    <Pair>
        <key>normal</key>
        <styleUrl>#sn_label</styleUrl>
    </Pair>
    <Pair>
        <key>highlight</key>
        <styleUrl>#sh_label</styleUrl>
    </Pair>
</StyleMap>

<Style id="sn_label">
    <LabelStyle>
        <scale>0</scale>
    </LabelStyle>
    <Icon><href>https://mrow.org/files/icons/hotairbaloon.png</href></Icon>
</Style>

<Style id="sh_label">
    <LabelStyle>
        <scale>1</scale>
    </LabelStyle>
    <Icon><href>https://mrow.org/files/icons/hotairbaloon.png</href></Icon>
</Style>

<StyleMap id="msn_label_car">
    <Pair>
        <key>normal</key>
        <styleUrl>#sn_label_car</styleUrl>
    </Pair>
    <Pair>
        <key>highlight</key>
        <styleUrl>#sh_label_car</styleUrl>
    </Pair>
</StyleMap>

<Style id="sn_label_car">
    <LabelStyle>
        <scale>0</scale>
    </LabelStyle>
    <Icon><href>https://mrow.org/files/icons/car.png</href></Icon>
</Style>

<Style id="sh_label_car">
    <LabelStyle>
        <scale>1</scale>
    </LabelStyle>
    <Icon><href>https://mrow.org/files/icons/car.png</href></Icon>
</Style>
$(
$objects | sort date | %{
"<Placemark>"
    "<name>$($_.call) at $($_.date)</name>"
    $(if($_.call -ne "N6UDP"){
        "<styleUrl>#msn_label</styleUrl>"
    }else{
        "<styleUrl>#msn_label_car</styleUrl>"
    })
    
    "<TimeStamp><when>$($_.date)</when></TimeStamp>"
    "<Point><altitudeMode>absolute</altitudeMode><coordinates>$($_.lon,$_.lat,($_.alt*0.3048) -join ",")</coordinates></Point>"
"</Placemark>"
}
)

$(
$objects | group call | %{
    "<Placemark><name>$($_.name) (trail)</name>"
    "<LineString><extrude>1</extrude><altitudeMode>absolute</altitudeMode>"
    "<coordinates>$(($_.group | sort date |%{ $_.lon,$_.lat,($_.alt*0.3048) -join "," }) -join " ")</coordinates>"
    "</LineString>"
    "</Placemark>"

}
)
</Document>
</kml>
"@
sc track.kml $kml