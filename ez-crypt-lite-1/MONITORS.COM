�:�                                     
                                 
                                                
                                               
                                                     
To view the contents of this file
enter the name of the file from the DOS prompt.
��  �>�~u�6���*��c����lùP ���ֲ QR��  ��<tN<
tJ<~u2�<~t,�<#t#�<0|#�<f�<9~�<A|�<F~�<a|
���FF�¬�	� � �:�ZY����ZYÊЋ�����>��16�F�H r+1���6�����Y���C;���>��1�  r��ֻ  ��C;���>��1� r���%��<u��<~u��<~t�<t�<#u�<t�:�u��Í6A�< t	��  �����  ��6��\<t���>� u�� �  �  �6��< t<0|�<9�$�����< t!< t��
��< td< t`<0|�<9�$�������< t#< t~�
��< t>< t:<0|n<9j$�׊������< tZ< tV�
��< t< t<0|F<9B$��׊��ش �Ë�Ǳ
���´ �d���ƴ �����;.��� |
�� tM�.�� ��6��< t	��  �����  ��6 �Z<t� �> u� �>� � ��� ��u�+���I���6�<a|�<z�, F���Ћ�����>��16�F����Ћ�C����>��1W�M _r��  ����W���C;���>��1W�& _r��׻  ��C;���>��1W� _r�������<u��^�Ê^��<a|<z, :�u��I�>V�� ^�ì<t�<a|<z, G:u���^�Ã.��>� }	��  ���������>��|��������  ��  ����  ��- ������� � �O��� ��  �Ã��� �[��>� t�.��� �I��>� u�H��.��A����>��1�� �3��%���= ��- 9�|������������>��1�� ������������2�Q�O��� � ��  ���
�	�!�6�
�*<t����
� <�  �!s��
��=�!s���ش@�����!�>�!�t��>� }��  á�= ~- 9�~������  ù ���� QC9�Y�KS��>��1�>� t���<t	<
t��� �[CY����ô ��č>�� ��u���ً�K���<;|�<Dۻ ,
<:t�,
ô� � �O�>:��  ��  ��6��P �  Q��	� � ���մ�  �EY��ù
 ��@�  ���0���  ���0���  ���0��  ���0�~�  �
 ��@�  ���0���  ���0���  ���0���  �6;�P � Q�մ�  ���	� � ��EY��ô�� �O�>:�ô�� �O�>:�À<0|[�<fV�<9~�<A|L�<F~�<a|B�|0|<�|f6�|9~�|A|*�|F~�|a|�<9~,$���ج<9~,$؈:� SQRVWU��  ����=�  � �<tV<tU��StJ��KtN��Mtt��Rtg��Gt{��Otv<t{:}�<~ʀ>�u� S�@�	� �p�E�  �մ�[C��^��&�[��� ~��x u�8 u�  MS�  �մ�[K�y��6��q�8|�S�x u�@ ��� � �W��� t�MS�ջ  ��[K�C�]_^ZY[�PVW�� I�+ˋ�G�� t���_^SR��+ӻ  ��Z[�I SR�ջ  ��Z[X�VW�� F��F+˃� t��_^SR��+ӻ  ��Z[� SR�ջ  ��Z[���SU��  ���� Q�	� �p�YVFUQ�  �մ���< t<t�	� �p�EY��QY]^�  �մ�][�PQW�� ��G� ��_YX�      If you want to write to a file enter the file name.
  If you want to print the file enter LPT1 or LPT2

  ENTER: $@                                                                  Esc=Quit,T=Write,F=Find,C=Cont,Keys=,,,,PgUp,PgDn,Home,End,Row=0000,Col=000                      KMGOIQPH!.1                (C)opyright, 1989, EZX Corp. All rights reserved.                  ?                                                                Enter line number:                                                                   Enter string:                                                                         @)� 56The following descriptions are for BWVIDEO, and LE-D-FIX. They are for use on
monitors that cannot display color (monochrome) but whose computer tells DOS
that the video card is CGA, EGA, VGA (color), etc. This includes laptops with
LCD and gas plasma displays.




    NOTE:  THIS  PROGRAM WAS FOUND ON A LOCAL BULLETIN BOARD. IT IS ASSUMED
    TO BE PUBLIC DOMAIN. USE AT YOUR OWN RISK.

                                    BWVIDEO
    What BWVIDEO is:
    ----------------
    BWVIDEO  is  an  acronym  for  "Black  and  White  Video".   It  is  used
    exclusively to correct problems surrounding the use of Black and  White
    monitors with the  CGA (Color Graphics  Adapter) card.  These  problems
    may be characterized by "fuzzy" or unreadable screen displays.  So,  if
    you have  a color  graphics board  in you  system, and  you also have a
    black and  white monitor,  then you  need this  program, otherwise  you
    don't.  Black and "amber" or "green" also apply.

    To Install BWVIDEO
    ------------------
    After the DOS prompt, simply key in

    DOS Prompt> BWVIDEO

    Once installed, Screen  Cleaning may be  invoked by pressing  Alt-PrtSc
    keys simultaneously.  Pressing them a second time will terminate Screen
    Cleaning.   BWVIDEO  will  beep  the  speaker  twice when Screen Cleaning
    begins and once when it ends.


    How BWVIDEO works:
    ----------------
    BWVIDEO uses two different methods to correct fuzzy displays.  The  first
    is called "Filtering" and the second is "Screen Cleaning".  Once  BWVIDEO
    is invoked, filtering is always present.  Screen Cleaning may be turned
    on or off at the users option.

    Filtering

    BWVIDEO interposes itself  between the program  that you are  running and
    the IBM BIOS programs to modify display requests for color.  In effect,
    each time a request is made  for a particular color to be  displayed on
    screen,  BWVIDEO  steps  in  and  changes  the  request  to  White before
    continuing.

    The rule that BWVIDEO follows to make this change is stated as:

    IF foreground and background are the same THEN
       IF they are black or white THEN leave them alone
       ELSE they are another color so force to white
    ELSE
       IF foreground is black OR background is white THEN force "Reverse Video"
       ELSE force "Normal Video"

    Normal video is defined as White characters on Black background
    Reverse video is defined as Black characters on White background

    This  technique   will  remove  the  fuzziness  associated  with   most
    applications programs that  may  be run.   Once installed, the  program
    sits passively  in the  background and  filters all  color requests and
    changes them as necessary, you do nothing else.


    Screen Cleaning

    There are a class  of programs for which  Filtering will not work,  for
    the simple reason that these programs do not use the IBM BIOS routines.
    Instead,  they  write  directly  to  video themselves.  BWVIDEO filtering
    never gets a chance!  In general, they do this to gain speed.

    BWVIDEO provides Screen Cleaning to remedy this additional situation.  Of
    course,  the  price to pay for  this  remedy is the very speed that the
    program was trying to achieve.  But, when faced with being not able  to
    use the program because the screen is unreadable, a slight decrease  in
    its speed doesn't seem so unattractive.

    Speed is important though, and that's why BWVIDEO provides the  user with
    the option of turning Screen Cleaning on or off at will.

    In  essence,  Screen  Cleaning   proceeds  along  after  the delinquent
    application program and  cleans up its  mess.  It does  so by examining
    each element displayed on the screen and correcting it as necessary.

    To  begin  Screen  Cleaning,  you  need  only  press the Alt-PrtSc keys
    simultaneously.  BWVIDEO will announce that it has begun Screen  Cleaning
    with two short beeps of the speaker.  Screen Cleaning may be turned off
    by again pressing the Alt-PrtSc keys.  BWVIDEO will announce that  Screen
    Cleaning  is  no  longer  being  performed  with  one short beep of the
    speaker.

    BWSCDEMO.COM is included to  demonstrate  Screen  Cleaning.   You  need
    only  install  BWVIDEO, then type BWSCDEMO and follow directions to see
    how Screen Cleaning works.


    What BWVIDEO can't remedy
    -------------------------
    Screen Cleaning always works.  It will always create a screen that  can
    be read and understood.  But some problems exist if Screen Cleaning  is
    requested  while   in  a   GRAPHICS  mode.    This  especially  affects
    graphically oriented games.

    This  happens  because  some  programs  (especially  games)  expect the
    display to be exactly the way they left it when they next try to  write
    to it.

    Because  Screen  Cleaning  modifies  the  display  without  telling the
    application, strange  things can  happen.  As  objects move,  ghosts of
    those objects can be left behind.  If this happens, there is no  reason
    to panic,  as only  the screen  is wrong.   No data  has been lost.  To
    remedy,  you  simply   turn  off  Screen   Cleaning  and  restart   the
    application.  For this application, Screen Cleaning is not suitable.
NOTE: THIS PROGRAM WAS FOUND ON A LOCAL BBS. WE ASSUME IT IS PUBLIC DOMAIN.
USER ASSUMES ALL RISK OF USE.

	Leading Edge Model D Mono Screen "color" Fixup

	Background:
	----------
	    The Leading Edge Model D in monochrome emulates a Hercules
	(tm) Monochrome Graphic Card.  I have noticed several programs
	which display areas that are supposed to be text as various
	shades of blank screen.  Many programs allow you to configure
	"colors" even on a MONO screen, which solves the problem if you
	can at least read enough to run the color set program.	The
	public Domain AUTOMENU is one that I was able to get working
	this way.  However, many programs do not allow color
	configuration, or do not give enough flexiblity to get
	everything visible.  Among these are Leading Edge's own Nutshell
	Tutorial (how mbar- rassing for them!) and Paperback Software's
	VP/Planner.

	    I called Leading Edge Tech Support and found them aware of
	the problem, but without a general fix.  They did send me an
	upgrade to Nutshell, but could do nothing about other packages.
	They suggested that perhaps the programs were getting confused
	since the Model D has both color and mono circuitry.  Sounds
	reasonable BUT THIS IS NOT THE CASE.

	    The IBM monochrome graphics adaptor very cleverly maps some
	be color combinations from the Color Graphics Adaptor (CGA) into
	the "attributes" that it supports.  The Mono adaptor supports
	normal (white on black), reverse (black on white), underline
	(blue on black) and invisible (black on black).  It also
	supports the first three with the addition of blink and or
	bright for a total of 13 attributes.  The remaining possible
	attributes, most of which correspond to valid CGA color combina-
	tions, are UNDEFINED.  However, on a "true-blue" IBM monochrome
	display adaptor the undefined codes all display as normal (with
	whatever blink/bright is set).	On the Leading Edge Model D,
	many of the undefined codes come out as white on white.  I do
	not know if this happens on a genuine Hercules Card or if it is
	unique to this one.  The documentation for all three adaptors
	(IBM, Leading Edge and Hercules) DOES NOT SPECIFY what happens
	if you use an undefined combination.

	    So the problem is not that the programs think that the
	Leading Edge is running in color (the Pheonix Rom passes all the
	tests for display type), but rather that lazy programmers didn't
	care.  They "knew" that the illegal combinations would show up
	as normal white on black.  So here we are!

	    The only "correct" fix for this problem is a hardware change
	to the display adaptor to map illegal codes the same as the IBM
	display does.  But I'm not willing to wait for that, so I wrote
	a software solution.  It's not "clean", it takes 3-4% away from
	your CPU when active, but it does work!


	How to use LE-D-FIX:
	-----------------
	    I tried to make this program as easy to use as possible.
	At the A> prompt (and with le-d-fix.com on drive a:) type

		LE-D-FIX		    and press return

	it loads itself and becomes active.  To turn it off (but leave
	it in memory) just type

		LE-D-FIX		    and press return

	again.	Each call to LE-D-FIX toggles the status.  You may add it
	to your autoexec.bat or the .bat for executing a program that
	needs it.  For use in a .bat file, it returns an ERRORLEVEL code
	of 1 if it is now active, and 0 if it isn't, so you can be sure
	you didn't turn it off when you meant to turn it on.  There is a
	sample batch file for VP Planner included with this program.


	How it works:
	------------
	    Since most programs write directly to the screen memory
	rather than calling DOS to do it's writing, there is no easy way
	to intercept the writes.  Therefore I had to take an "after the
	fact" approach.  LE-D-FIX steals the timer interupt vector (in a
	very clean way, so it should not interfere with anything else)
	and every time it is activated (18.2 times per second) it
	examines a portion of screen memory and changes any illegal
	attributes it finds to the attribute a true IBM display would
	show.  It is currently set up to do it 200 bytes at a time (one
	tenth of the screen) which I found to be the best compromise
	between speed and overhead.  Thus it will fix up any error
	within 10/18 of a second.

	    This is yet another one of those "load me last" programs.
	It is only necessary if you want to be able to turn it on and
	off.  The program can't find itself if some other program has
	taken back the timer interupt.	If you don't intend to toggle
	it, there should be no problem loading a program which insists
	more vehemently on being loaded last.

	    If the MONO TEXT MODE is not active, the interupt handler
	will exit without messing up any graphics.  I've tested this
	with REFLEX version 1.1, and would expect it to be alright with
	any graphic program that sets the mode properly.  If you find
	one of your graphics screens self-destructing, you will have to
	disable LE-D-FIX while running it.
�D���T�����.{�b����@B[]��Fikm���9�����Y��MO����Moq���K��35��ac��L���UW��;=��&u���l�����h���% r ~ � � !g!�!"K"�"�"�"�"�"�"#H#�#�#$I$�$�$�$5%x%�%�%�%&J&�&�&'3'5'w'�'�'9(t(�(�(3)u)�)�)5*w*�*�*+!+^+�+�+%,R,T,�,�,-]-�-�-�-�-�-�-1.3.U.W.�.�.�.�.�./[/�/�/#0b0d0f0v0�0�01H1�1�12F2�2�23D3`3b3�3�3"4a4�4�4�45K5�5�5646