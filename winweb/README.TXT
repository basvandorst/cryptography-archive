WinWeb Version 1.0 A2.1

This is a pre-release of the WinWeb software.  This software is still being tested and enhanced.

This file contains the following sections:

1. Note to Current Users
2. System Requirements
3. Installation
4. Reporting Problems or Suggestions
5. Menu Descriptions
6. Known Bugs and Problems
7. WINWEB.INI Format
8. Adding External Viewers to WinWeb.
8. Changes from Previous Versions
9. Legal Restrictions

1. Note to People who are Upgrading

If you have already installed an earlier version of WinWeb and are upgrading there are two changes in the new version of WinWeb that you should be aware of.

1.	The way the font height information is stored has been redefined.  This may result in font sizes that are incorrect when you first run the new version of WinWeb.  To fix this, simply go to the View Font dialog box and select the Default button for all of the available fonts.  This should reset the font height to the correct size.

2.  The Proxy= entry in the [Options] section is no longer supported.  If you have WinWeb configured to work with a proxy server you will have to recongifure the new version of WinWeb to work with your server.  To do this, there is now a proxy configuration dialog box in the Options menu.

2. System Requirements

To use WinWeb, your system needs the following:

Windows 3.1
4 Megabytes RAM
5 Megabyte free hard disk space

For network access, WinWeb also requires an installed TCP/IP stack with a Windows Socket Library (WINSOCK.DLL) version 1.1 or greater.

Note:  Additional RAM and hard disk space, if available, is used for caching and improves performance.


3. Installation

To install WinWeb, follow these steps:

1.  Create a WINWEB directory on your hard drive.
2.  Copy all of the files except VBRUN300.DLL from the diskette or FTP server into the directory.
3.  Copy the file VBRUN300.DLL into your window's system directory (usually \windows\system)
3.  Run winweb.exe.

The files in this release are:

winweb.exe
readme.wri
readme.txt
winvoke.exe
winexe.inv
wingo.inv
wecj1_2.exe
wecjlib.dll
wingif.exe
wplany.exe
vbrun300.dll

WinWeb automatically creates a default WINWEB.INI file.
	
4. Reporting Problems or Suggestions

The best way to contact EINet is to send E-Mail to winweb@mcc.com.

If you have questions or problems with WinWeb, we may already be aware of them, or already have a fix for them.  Please check Section 5 of this document, Known Problems and Bugs.

Our mailing address is:

EINet Windows Software
MCC
3500 West Balcones Center Driver
Austin, TX 78759-6509
USA

5. Menu Descriptions

File
Open
Displays an open file dialog box which allows you to select a file from your local hard drive as the current document.
Print
Prints the current document.
Print Preview
Shows a preview of how the current document will be printed.
Print Setup
Options for printing the document.
Exit
Exits the program.

Options
Load to File
When selected, this option will load a document to a file instead of to the WinWeb display.  It will prompt you for a file name.
Set Home Page
Allows you to select which document to use as the home page.  You can select either the EINet Galaxy home page, the document currently being viewed, or you can enter a URL specifying the home page.
Load Images
Toggles whether or not to automatically load inline images.  If a page has been loaded without its inline images, they can be loaded by turning this option on and then selecting the Reload option from the Navigate menu.
Proxy Server
Opens a dialog box which allows you to configure WinWeb to work with a proxy server.

View
Fonts
Allows the fonts which are used to display the document to be changed.
Highlight Color
Allows the highlight color of hyperlinks and graphics links to be changed.
Background Color
Allows the background color of the documents to be changed.
Toolbar
Displays or hides the toolbar at the top of the window.
Status Bar
Displays or hides the status bar at the bottom of the window.
URL Display
Displays or hides the URL display at the top of the window

Navigate
Back
Displays the previously loaded document.
Home
Displays the home page.
Load URL
Prompts for a document address in the form of a URL, which is then loaded from the network.
Reload
Reloads the current document.  This option can be used to reload documents that change over time or to load inline images.
Cancel
Cancels the loading of a document.  If a document is being loaded and Cancel is selected, the loading of the document is aborted.  If the document has been loaded and its inline images are being loaded, the document is displayed but without all of its inline images.
Search
Some pages have indexes that can be searched for additional information.  Use this command to enter a search string that is checked against the current document's index.
Search EINet Galaxy
The same as search, except the query is routed to an index maintained by EINet Galaxy.
History List
Displays the list of documents that have been visited.  Previous documents can be viewed again by selecting them from this list.
Hotlist
Displays a list of user-defined documents.  You can can add, delete, edit or jump to entries in this list.

Help
Index
Eventual location of the online help.  It is currently not implemented.
Using Help
Windows online help tutorial.
About WinWeb
Information about the product.


6. Known Bugs, Problems and Feature Requests

The following is a list of known bugs and problems in the current release of WinWeb.


No Way to Specify TELNET or WAIS clients
Problem: The current version of WinWeb does not contain any way to launch  telnet or wais clients.

No Forward Button
Problem: The current version of WinWeb does not contain a forward button similar to Mosaic's

Exiting an External Viewer Switches to Wrong Application
Problem:  When an external viewer is launched from within WinWeb and then exiting, the application that is activated is generally not WinWeb but some other Windows application.

URLs Aren't Editable
Problem:  Unlike MacWeb, EINet's Macintosh-based Web browser, the URLs that are displayed are not editable.  Work Around:  To enter a URL to jump to press Ctrl-U or select Load URL from the menu.

I Can't Find a Way to Invoke a URL from the Command Line or DDE
Problem:  The current version of WinWeb does not support any way of invoking a URL from either the command line or Windows DDE.

I Can't Select Hyperlinks Using My Keyboard
Problem:  The current version of WinWeb does not support selecting hyperlinks using the Keyboard.
Work Around:  Use the mouse to select hyperlinks.

I Can't Tab Between Fields in a Form
Problem:  The current version of WinWeb does not support tabbing between fields in a form.
Work Around:  To move to a new field, select it with the mouse.

Inline Pictures Aren't Being Aligned Properly
Problem:  Some inline images may not appear aligned correctly in the current version of WinWeb.  The ALIGN= attribute of the HTML IMAGE element is not currently supported.

I'm Running NetManage Chameleon and WinWeb is Crashing at Address NMPCIP 5:13b7
Problem:  We have seen a problem when running WinWeb under NetManage Chameleon in which it crashes at the address NMPCIP 5:13b7.  The problem appears to be related to the gethostbyname() function.

Print and Print Preview Sometimes Don't Work
Problem:  Ocassionally, the print and print preview functions will only show a blank page rather than the correct output.
Work Around:  The problem appears to be intermittent.  Simply trying again often results in the correct behavior.

Online Help Doesn't Work
Problem:  The online and context sensitive help files are not included in this release.

No Icons Displayed on FTP and Gopher Menus
Problem:  Typically Gopher and FTP menus display an icon beside each item graphically showing the item's type.  The current version of the program does not support this.

Can't Copy Text and Pictures out of a Document
Problem:  The current release of the software does not support  the cutting of Text and Pictures.
Work Around:  HTML files can be downloaded and the information can be cut using a word processor such as write.


7. WINWEB.INI Format

The following is a description of the entries in the WINWEB.INI located in the Windows directory.  If an entry is not present, the indicated default value is used.

[Options]
The [Options] section stores most of the user-defined values.

Note:  The Proxy= statement in the [Options] section is no longer supported.  It has been replaced by a seperate [Proxy] section which is documented below.

Toolbar=
0 : ToolBar is not displayed
1 : ToolBar is displayed (default)
StatusBar=
0 : Status Bar is not displayed
1 : Status Bar is displayed (default)
URLBar=
0 : URL bar is not displayed (default)
1 : URL bar is displayed
AutoLoadImages=
0: Don't automatically load inline images
1: Automatically load inline images (default)
BackgroundColor=
Main viewing window's background color.  24-bit color value in hex format and in BGR order.  (default is 0xc0c0c0, dark gray)
HighlightColor=
Color of hyperlinks.  24-bit color value in hex format and in BGR order.  (default is 0xff0000, blue)
CustomColor[0..15]=
User-defined custom colors.  24-bit color value in hex format and in BGR order.  (default is 0xffffff, white)
HomePage=
The page to automatically load when the program is started or when the Navigate Home menu is selected.  (default is EINet Galaxy home page at http://galaxy.einet.net/)
History.X=
Horizontal position of the history dialog.
History.Y=
Vertical position of the history dialog.
History.Width
Width position of the history dialog.
History.Height=
Horizontal position of the history dialog.
Hotlist.X=
Horizontal position of the Hotlist dialog.
Hotlist.Y=
Vertical position of the Hotlist dialog.
Hotlist.Width
Width position of the Hotlist dialog.
Hotlist.Height=
Horizontal position of the Hotlist dialog.
Cache=
Allows you to specify the location in which WinWeb stores its cache files.  By default, WinWeb creates a directory called 'cache' inside of the WinWeb home directory.  If you are running WinWeb from a server you may want to specify a cache option which points to the local hard drive.

For Example:

	Cache=d:\temp\wwcache
[Proxy]

Mode=
0 : Disabled
1 : Single server mode
2 : Multiple server mode
Server=
In single server mode (Mode=1) indicates the name of the proxy server.  This entry is ignored in multiple server mode.

For example:

Server=www1.cern.ch:911
http=
ftp=
gopher=
In multiple server mode (Mode=2) these entries indicate which proxy server to use based on the URL type.

For example:

http=www1.cern.ch:911

[Font.description]
Each font in the viewer has a section that defines its metrics.  Valid sections are:

[Font.Normal]
[Font.Proportional]
[Font.Heading1]
[Font.Heading2]
[Font.Heading3]
[Font.Heading4]
[Font.Heading5]
[Font.Heading6]
[Font.Heading7]

Following is the default for the [Font.Proportional] section:

Height=-10
Width=0
Escapement=0
CharSet=0
PitchAndFamily=0
Weight=0
Italic=0
Underline=0
FaceName=Courier New
Color=0x0

[Launch]
This section defines the viewer launched when a file or a particular type is downloaded.

Entry[n]=
The format for each entry is the MIME type followed by a comma, and then the command to be executed to launch the viewer.  For Example:

Entry0=image/gif,winvoke ingo.inv lview31

Note:  WinWeb starts with Entry0, Entry1, Entry2, etc. until a blank entry is found.  If Entry0 is not present in the WINWEB.INI file, WinWeb automatically creates a default [Launch] section.

[HotList]
This section contains the entries in the user-defined hotlist.

Entry[n]=
The format for each entry is the descriptive text followed by a command, and then the URL to be invoked when the hotlist entry is selected.  For example:

Entry0=EINet Galaxy Home Page,http://galaxy.einet.net/

[Extensions]
This section contains the entries that define how file extensions are mapped to MIME types when downloading a file.

	Entry[n]=
The format for each entry is the file extension, followed by the MIME type to which it is mapped, and then a designation of the file contents.  For example:

Entry16=.gif,image/gif,binary

Valid designators for the file contents are "7bit", "8bit" and "binary".

Note:  WinWeb loads starting with Entry0, Entry1, Entry2, etc. until a blank entry is found.  If Entry0 is not present in the WINWEB.INI file, WinWeb automatically creates a default [Extensions] section.


8. Adding External Viewers
This section describes how to add external viewers to WinWeb.  An external viewer is designed to handle files which WinWeb does not handle internally.  For example, when WinWeb downloads an audio file, WinWeb can be congifured to automatically send the data to an external program which will play audio data.

External viewers can be added to WinWeb by editing the [Launch] and [Extensions] sections of the WINWEB.INI file.

When WinWeb requests a file, the server may identify the type of file which is being sent by first sending the file's MIME type.  A MIME type is simply a character string which identifies a file's type.  By convention, certain types of files have specific MIME types.  For example, if WinWeb receives a file with the MIME type "image/gif" this indicates that it is a bitmap file in the GIF format.  

Some servers, however, do not identify the file type by first sending a MIME type.  WinWeb handles these files by mapping the file's extension into a MIME type.  The [Extensions] section of WINWEB.INI defines these mappings.

For example, if a server sends the file "TEST.GIF" to WinWeb without a MIME type, WinWeb will map the file's extension ".GIF" into the MIME type "image/gif" if there is the following entry in the [Extensions] section:

	Entry16=.gif,image/gif,binary

All files whose extensions are not defined in the [Extensions] section are automatically mapped to the MIME type "www/unknown".

Certain MIME types WinWeb is able to handle internally.  The MIME type "text/html", for example, is the MIME type for a typical Web page.  If the MIME type is not handled internally, however, WinWeb will look for an external viewer to pass the information to.  These viewers, and the MIME type which are mapped to them, are defined in the [Launch] section of WINWEB.INI.

For example, if the [Launch] section contains the following entry

	Entry0=image/gif,wingif

and WinWeb receives a file of MIME type "image/gif", it will launch a program called wingif.  In addition, the name of the file that WinWeb has created and that contains the image data will be passed to the program.

One problem with launching programs in this manner, however, is that the file that WinWeb has created to store the image data is never deleted.  Over time, a large number of these files may accumulate taking up a substantial amount of disk space.

To work around this, WinWeb includes a program called winvoke.  Winvoke is a program which reads a script file and can process additional arguments that are passed to it.  For example, WinWeb's default entry for a GIF file is as follows:

	Entry0=image/gif,winvoke wingo.inv wingif

When WinWeb downloads a GIF file, it will execute the winvoke program with the the arguments wingo.inv, wingif and the data file name.  Winvoke interprets the first argument as a script file.  In this case the script file is wingo.inv.

The script file wingo.inv is designed to take the remaining arguments (wingif and the data file name) and to execute them.  When the viewer program is exited, control is returned to the wingo.inv script which then deletes the data file.

In other words, when a file of MIME type "image/gif" is downloaded, WinWeb will

1.  Launch the winvoke program with arguments "wingo.inv wingif <filename.gif>"
2.  Winvoke will interpret the first argument as its script file.
3.  The script will tell winvoke to launch wingif with the argument <filename.gif>.
4.  Upon exiting the viewer, the script file will tell winvoke to delete the file <filename.gif>
5.  Since this is the end of the script file, winvoke will exit as well.

8. Changes from Previous Versions

The following changes were made from WinWeb version 1.0 A2 to 1.0 A2.1

1.  Added support for documents with very large forms.

2.  Fixed crash with inline .BMP files.  .BMP files are not supported for inline imaging but should no longer crash the program.

3.  A lockup when handling certain inline GIF files was fixed.

4.  Crash when downloading files that are not mapped to an external viewer was fixed.

5.  Crash that resulted from nested <SELECT> statements was fixed.  (Technically, <SELECT> statements are not supposed to be nested, but WInWeb won't crash even if they are.)

6.  Crash was fixed if WinWeb was exited when a form element had the input focus.

7.  Fixed interparagraph spacing.

8.  Formatting for a list of anchors has been fixed.

9.  List marks would sometimes be misaligned.  This has been fixed.

10.  Style and white space compression has been fixed.

11.  Formatting in <PRE> sections has been fixed.

12.  Formatting of non-proportional text has been improved.

13.  WinWeb would sometimes not read all of the data from the end of a document.  This has been fixed.

14.  White space was added around images to improve their looks.

15.  Problem with <HR> indents has been fixed.

16.  Proxy support has been much improved.

17.  Local file access has been improved and a File Open command has been added.

18.  Gopher searches has been improved and the interface improved.

19.  Inline GIF files sent without a MIME header are now supported.

20.  Problems with automatic URL redirections have been fixed.

21.  Default Hotlist size was cutting off one of the dialog box buttons.  This has been fixed.

22.  isindex "Stealth" mode is now supported.

23.  <ADDRESS> formatting is now supported.

24.  <EM> and <STRONG> formatting is now supported.

25.  TEXTAREA form elements are now supported.

26.  Vertical scroll increment was changed.

27.  Default selection in OPTIONS form elements was fixed.

28.  Old HTTP server support was improved.


The following changes were made from WinWeb version 1.0 A1 to 1.0 A2.

1.  Dialogs were added to show initialization failures such as the wrong WINSOCK DLL.

2.  An initialization problem that prevented WinWeb from running was fixed.
  
3.  Page Up and Page Down were changed so that the amount the page is scrolled depends on the window size rather than having a fixed scrolling size.

4.  Support for local file access was fixed.

5.  The URL display was changed to show URL tags.

6.  How the HTML <BR> element is handled was changed to not include an additional line feed.

7.  A problem was fixed that was removing blank spaces and making formatting difficult.

8.  The temporary file created in the cache directory are now given the .WWC extension.

9.  The History and Hotlist dialogs can be resized.

10.  The HTML <HR> element is now supported.

11.  Proxy support was added.

12.  Special characters in form related strings are now escaped.

13.  Some problems relating to large forms were fixed.

14.  File downloading is now supported.


8. Legal Restrictions

                        WinWeb License

WinWeb is Copyright © 1994 Microelectronics and Computer Technology
Corporation (MCC).  All Rights Reserved.  EINet is a trademark of MCC.

The MCC EINet WinWeb Version Alpha 1.0 A2 Software, both the binary executable and the source code (hereafter, Software), is copyrighted and licensed. MCC provides the Software and licenses its use as set forth herein. The Software is, and at all times shall remain, the property of MCC, and Licensee shall have no right, title or interest therein except as expressly set forth in this agreement. Source code, if made available, will be licensed separately.

MCC grants to Licensee a license to use, copy and distribute the Software for academic, research and personal-use purposes only, without a fee, provided that (i) the above copyright notice and this permission notice appear in all copies of the Software and related documentation, and (ii) the MCC and EINet name may not be used in any advertising or publicity relating to the Software without the specific, prior written permission of MCC, and (iii) that no charge is associated with copies of the Software distributed by Licensee to third parties. Commercial use of this Software is forbidden without the express written permission of MCC and payment of a negotiated license fee.

Licensee may make derivative works. However, if Licensee wishes to distribute any derivative work based on or derived from the Software, then Licensee will (1) notify MCC regarding potential distribution of the derivative work; (2) acquire permission from MCC to distribute the derivative work; and (3) clearly notify users that such a derivative work is a modified version and not the original Software written and distributed by MCC.

Any Licensee wishing to make commercial use of the Software should contact MCC, to negotiate an appropriate license for such commercial use. Commercial use includes (1) integration of all or part of the Software into a product for sale or license by or on behalf of Licensee to third parties, or (2) distribution of the Software to third parties that need it to utilize a commercial product sold or licensed by or on behalf of Licensee.

THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. MCC DOES NOT WARRANT THAT THE FUNCTIONS CONTAINED IN THE SOFTWARE WILL MEET LICENSEE'S REQUIREMENTS OR THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE SOFTWARE IS WITH LICENSEE.

IN NO EVENT WILL MCC BE LIABLE TO LICENSEE OR ANY THIRD PARTY FOR ANY DAMAGES, INCLUDING ANY LOST PROFITS, LOST SAVINGS OR OTHER INCIDENTAL, CONSEQUENTIAL OR SPECIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE SOFTWARE EVEN IF MCC HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

By using or copying the Software, Licensee agrees to abide by the copyright law and all other applicable laws of the U.S. including, but not limited to, export control laws, and the terms of this license.  MCC shall have the right to terminate this license immediately by written notice upon Licensee's breach of, or non-compliance with, any of its terms.  Licensee may be held legally responsible for any copyright infringement that is caused or encouraged by Licensee's failure to abide by the terms of this license. 

