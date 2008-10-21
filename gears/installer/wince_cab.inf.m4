[Version]
Signature="$Windows NT$"
Provider="Google"  ;[naming]
CESignature="$Windows CE$"

[CEStrings]
AppName="Gears"  ;[naming]
InstallDir="%CE1%\PRODUCT_FRIENDLY_NAME_UQ"

[Strings]
Manufacturer="Google"  ;[naming]

[CEDevice]
VersionMin=4.0
VersionMax=6.99
BuildMax=0xE0000000

[DefaultInstall]
CopyFiles=Files.Common1
CESelfRegister=PRODUCT_SHORT_NAME_UQ.dll
CESetupDLL=setup.dll

[SourceDisksNames]
m4_changequote(`^',`^')m4_dnl
m4_ifelse(DEBUG,^1^,^m4_dnl
1=,"Common1",,"bin-dbg\wince-arm\ie\"
^,^1^,^1^,^m4_dnl
1=,"Common1",,"bin-opt\wince-arm\ie\"
^)

[SourceDisksFiles]
"PRODUCT_SHORT_NAME_UQ.dll"=1
"setup.dll"=1

[DestinationDirs]
Shortcuts=0,%CE2%\Start Menu
Files.Common1=0,"%CE1%\PRODUCT_FRIENDLY_NAME_UQ"

[Files.Common1]
"PRODUCT_SHORT_NAME_UQ.dll","PRODUCT_SHORT_NAME_UQ.dll",,0
