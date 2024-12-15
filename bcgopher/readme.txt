Welcome to BCGopher, the Beta Release!

This is a FREE Microsoft Windows 3.1 (tm) Gopher client compatible with the
Winsock standard 1.1.

NOTE: You must have a TCP/IP package that is compliant with the Windows Socket Library
to be able to use BCGopher.  Contact the vendor of your software to find out if it is.

You can aquire the most recent updated copy by ftping to:

address: bcinfo.bc.edu
username: anonymous
password: guest           (this is not a true anonymous ftp server)
directory: pub/bcgopher

In this archive you should find the following files.

BCGOPHER.EXE    - The Windows Gopher Application
BCGOPHER.HLP    - The Windows Help File.
README.TXT      - This File.
COMMDLG.DLL     - A dynamic link library for common dialogs.

also included for your enjoyment:
L-VIEW.EXE      - Self Extracting Archive of L-View FREEware Image viewer.
WINCODE.EXE     - Self Extracting Archive of WINCODE uuencoder / decoder.
QTW11.EXE       - Self Extracting Archive of Quicktime For Windows version 1.1.
WINAVI.EXE      - Self Extracting Archive of a Video for Windows Player.
FTPSOFTWARE.ZIP - Archive of the WINSOCK.DLL file for FTP's PC/TCP version 2.11.
WINSOCK-FAQ.TXT - Text file containing the Most Frequently asked questions about WinSOCK.
WINSOCK-VENDOR.TXT  - Text file containing vendor information about WINSOCK.

To install place all the files in your windows directory (or appropriate subdirectory), 
create a program manager group (if you want) and create an icon and gopher it!


It has many features available.

* uses specified pager (notepad default)
* uses specified telnet (wntvt [pc/tcp] default)
* automatic connect on startup (default not to)
* Bookmarks
* Full bookmark editing.
* returning to previous menu (from menu, from push button, or keystroke)
* returning to any previous site (in menu item)
* CSO phone book support
* Index Searches (WAIS)
* DOS Binary
* Receives HQX files as text (instead of hiding their existance like others do)
* Custom font and size
* Dialog that gives you detailed information about item (host, port, selector string)
* bookmarks in seperate file from configuration info (bcgbkmrk.ini)
* image file support (with customizable viewer app)
* sound file support (with customizable player app)
* Windows Help file.
* Differentiation between folder and server link.
* support for movie files (with customizable player app)

BCGopher is copyrighted 1993 The Trustees of Boston College.

If you have any problems or suggestions please send them to me.  I won't
guarantee I will do anything but I will definately read it.

Tested on the following Winsocks:
    FTP's PC/TCP Version 2.11 and Version 2.2
    Peter Tattam's WinSock stack.
    Beame & Whitesides Beta WINSOCK.
    Wollongong Pathway
    Novell LWP  

Revision History:

v0.1@ completed.  Most Major features exist
                  This version shipped to list of people for testing with WINSOCK.
v0.1@a. * Fixed some of the canceling functions.
        * Removed Save from bookmark menu.
        * Started to add "Out of Memory" messages.
        * Removed all the destory icons because I think they are deleting things 
          they shouldn't.
        * Changed bookmark "Go To" to act like "OK" also.
        * Changed Gopher_Data->Item_Type to character to support extended 
          gopher types (like "I")
        * added image viewer capability.
v0.1@b  * fixed (hopefully) the pointer scheme, which should
          also fix the back arrow and recent menu.
        * moved bookmarks to seperate file (wngbkmrk.ini).
        * fixed (hopefully) resource leaks.
        * added support for sound.
        * fixed up the wait cursor.
        * fixed most of the keypad keys.
v0.2@r  * change of version for alpha release purposes. (r = release)
        * never released.
v0.3@r  * fixed (sort of) scrolling with keys and such
        * fixed some problems with Recent menu
v0.4@   * later that day...
        * Changed name to BCGopher.
v0.4@r  * fixed a problem with CSO.  Actually and truely released to
          the public for alpha (@) testing.
v0.4@a  * Fixed bug in CSO with no matches.
v0.4@b  * Add temporary file for image and sound.
        * Added ability to arrange bookmarks in dialog box.
        * Fixed bug in bookmark dialog box.  Canceling gave weird results.
        * Really and truely fixed the minimizing?
v0.4@c  * Cleaned up temporary files (kludge, not using callbacks).
        * Added differentiation between directory and link to another server.
        * Fixed bug introduced it 0.4@b with DOSBINARY and BINARY files.
v0.4@d  * Changed wait cursor, and fixed it's flashing in non client areas.
        * fixed problem of large lists having errors scattered through
          the file types.
        * Fixed scrolling with keypad keys.
        * Fixed bug where temporary image files aren't all deleted upon close.
        * Fixed Major Bookmark memory leak which caused crashes.
v0.5ß   * changed to beta, did a little clean up.
        * Fixed problem with recent list.
v0.6ß   * Added Movie support.
        * Added unsupported icon specification.
        * some miscellaneous clean ups.
v0.6ßa  * movie viewer not opening temporary movie file when double clicking
          with left button.
        * Also fixed problem of saving configuration information for movie viewer.
v0.6ßb  * Fixed problem where scroll bar was not appearing in large lists.
v0.6ßc  * Removed problem conflict between ALT-B (menu) and ALT-B (Previous Site).
        * Added support for image type of "g" (GIF only file).
        
Edmund C. Greene
Special Services Consultant
Information Processing Support
Boston College
ed@bcvms.bc.edu
