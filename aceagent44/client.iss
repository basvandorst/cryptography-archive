; ===========================================================================
; This is an InstallShield silent install file. To use this file you must
; have an uncompressed version of the ACE/Agent installation. Read the
; installation instructions for a method that allows you to accomplish this.
; The only sections that can be modified by the user of this file are:
;   [RootCert-0], [SdConf-0], and [SdFinishReboot-0].
;
; This version of the silent install script will install a minimal Network
; Authentication client without any user documentation, only the required
; components will be installed. If any other component selections are
; desired use the SERVER.ISS file.
; ===========================================================================

; ===========================================================================
; Change the "RootCert=" line to the location where you can retrieve the
; root certificate for this installation. Do not leave this blank!
; ===========================================================================

[RootCert-0]
RootCert=\\installserver\installpath\sdroot.crt

; ===========================================================================
; Change the "SdConf=" line to the location of the ACE/Server configuration
; record for this installation. Do not leave this blank!
; ===========================================================================

[SdConf-0]
SdConf=\\installserver\installpath\sdconf.rec

; ===========================================================================
; Change the "BootOption=" line to "3" if you want to force a reboot after
; the installation completes successfully. Leave it as "0" to reboot later.
; ===========================================================================

[SdFinishReboot-0]
BootOption=0
Result=1

; ===========================================================================
; DO NOT MODIFY THE REST OF THIS FILE!!!
; ===========================================================================

[InstallShield Silent]
Version=v5.00.000
File=Response File

[Application]
Name=RSA ACE/Agent for Windows NT
Version=4.4 [011]
Company=SDTI
Lang=0009

[DlgOrder]
Count=7
Dlg0=MessageBox-0
Dlg1=SdWelcome-0
Dlg2=SdLicense-0
Dlg3=SdComponentDialog2-0
Dlg4=RootCert-0
Dlg5=SdConf-0
Dlg6=SdFinishReboot-0

[MessageBox-0]
Result=1

[SdWelcome-0]
Result=1

[SdLicense-0]
Result=1

[SdComponentDialog2-0]
Component-type=string
Component-count=1
Component-0=Upgrade Network Client
Result=1
