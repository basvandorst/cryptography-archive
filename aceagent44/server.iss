; ===========================================================================
; This is an InstallShield silent install file. To use this file you must
; have an uncompressed version of the ACE/Agent installation. Read the
; installation instructions for a method that allows you to accomplish this.
; The only sections that can be modified by the user of this file are:
;   [RootCert-0], [SdConf-0], [SdFinishReboot-0], and [SdComponentDialog2-0].
;
; This version of the silent install script will install a FULL ACE/Agent
; suite with all the components available. This installation is for servers.
; If a reduced set of components is desired modify the [SdComponentDialog2-0]
; section to include only the necessary components.
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
; Change the "Component-count=" line to the total number of components
; listed. Remove any components that are not desired and change the numbers
; of the remaining components so that the sequence starts at 0 to one less
; than the total count of components. The "Network Server" component depends
; on the "Network Client" component and will cause the "Network Client" to be
; installed even if it is not listed. During an upgrade all the components
; that exist in the current installation will be upgraded even if the upgrade
; is not listed.
; ===========================================================================

[SdComponentDialog2-0]
Component-type=string
Component-count=7
Component-0=Upgrade Local
Component-1=Upgrade Remote
Component-2=Upgrade Web
Component-3=Upgrade Network Server
Component-4=Upgrade Network Client
Component-5=Upgrade User Documentation
Component-6=Upgrade Admin Documentation
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
