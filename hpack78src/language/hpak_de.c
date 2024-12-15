/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*						  		  HPACK Messages 							*
*							HPAKTEXT.C  Updated 29/05/92					*
*																			*
*   This program is protected by copyright and as such if you use or copy	*
*   this code for your own purposes directly or indirectly your soul will	*
*   become the property of the author with the right to dispose of it as	*
* 									he wishes.								*
*																			*
*		Copyright 1991 - 1992  Peter C.Gutmann.  All rights reserved		*
*																			*
****************************************************************************/

/* Generated:		04 Dec 1992.
   From file:		HPAKTEXT.DEF
   Language:		German.
   Character set:	ISO 8859:1987 Latin 1 */

/* The following are all the text messages (except for the hidden ones)
   contained in HPACK.  The error messages are stored in a special struct
   which also contains the error exit level.  Note that the messages are
   all stored as seperate strings instead of in one unified array to make
   automatic generation by a (currently experimental) preprocessing tool
   easier */

#include <stdlib.h>
#include "defs.h"
#include "error.h"
#include "errorlvl.h"
#include "hpacklib.h"

/****************************************************************************
*																			*
*									Errors									*
*																			*
****************************************************************************/

/* When changing the following remember to change the defines in ERROR.H
   as well */

#define ERR			ERROR_INFO

			/* Severe errors */
#if defined( __ARC__ )
ERR err00 = { EXIT_INT_ERR,		"Abort on data transfer" };
#else
ERR err00 = { EXIT_INT_ERR,		"Interner Fehler" };
#endif /* __ARC__ */
ERR err01 = { EXIT_NO_MEM,		"Kein Speicher mehr" };
ERR err02 = { EXIT_NO_DISK,		"Kein Plattenspeicher mehr" };
ERR err03 = { EXIT_NO_ARCH,		"Archivdatei %s kann nicht ge�ffnet werden" };
ERR err04 = { EXIT_NO_TEMP,		"Tempor�rdatei %s kann nicht ge�ffnet werden" };
ERR err05 = { EXIT_NO_PATH,		"Suchweg %s nicht gefunden" };
ERR err06 = { EXIT_NO_BASE,		"Kein Zugriff auf Haupverzeichnis" };
ERR err07 = { EXIT_NO_MKDIR,	"Verzeichnis %s kann nicht Erstellt werden" };
ERR err08 = { EXIT_BREAK,		"Abgebrochen auf Bedienerwunsch" };
#if defined( __MSDOS__ )
ERR err09 = { EXIT_FILE_ERR,	"Dateifehler vom typ Nummer %02d" };
#else
ERR err09 = { EXIT_FILE_ERR,	"Dateifehler" };
#endif /* __MSDOS__ */
ERR err10 = { EXIT_DIR_CORRUPT,	"Verzeichnis des Archivs defekt" };

			/* Standard errors */
ERR err11 = { EXIT_LONG_PATH,	"Suchweg %s zu lang" };
ERR err12 = { EXIT_LONG_PATH,	"Suchweg %s... zu lang" };
ERR err13 = { EXIT_LONG_PATH,	"Suchweg %s%s zu lang" };
ERR err14 = { EXIT_LONG_PATH,	"Suchweg %s%s... zu lang" };
ERR err15 = { EXIT_LONG_PATH,	"Suchweg %s/%s zu lang" };
ERR err16 = { EXIT_NO_OVERRIDE,	"Unm�glich, das Hauptverzeichnis so zu �berreiten" };
ERR err17 = { EXIT_NEST,		"Verzeichnisverschachtelung zu gro�" };
ERR err18 = { EXIT_SCRIPT_ERR,	"%s%d Fehler in der Skriptdatei" };
ERR err19 = { EXIT_NOT_ARCH,	"Nicht ein HPACK-Archiv" };
ERR err20 = { EXIT_NOTHING,		"Keine passenden Dateien auf der Disk(ette)" };
ERR err21 = { EXIT_NOTHING,		"Keine passenden Dateien im Archiv" };
ERR err22 = { EXIT_NOTHING,		"Keine passenden Archiven gefunden" };
ERR err23 = { EXIT_BAD_KEYFILE,	"Fehlerhafte Schl�ssel-Datei" };
ERR err24 = { EXIT_COMMAND,		"Unbekannter Befehl \'%s\'" };
ERR err25 = { EXIT_OPTION,		"Unbekannte Verzeichnis-Option \'%c\'" };
ERR err26 = { EXIT_OPTION,		"Unbekannte �berschreibe-Option \'%c\'" };
ERR err27 = { EXIT_OPTION,		"Unbekannte Anseh-Option \'%c\'" };
ERR err28 = { EXIT_OPTION,		"Unbekannte Option \'%c\'" };

			/* Minor errors */
ERR err29 = { EXIT_PASS,		"Fehlender userID f�r verschl�sselung/authentisierung" };
ERR err30 = { EXIT_PASS,		"Geheimschl�ssel f�r userID \'%s\' nicht gefunden" };
ERR err31 = { EXIT_PASS,		"Geheimschl�ssel nicht gefunden - Entschl�sselung nicht m�glich" };
ERR err32 = { EXIT_PASS,		"�ffentlicher Schl�ssel f�r userID \'%s\' nicht gefunden" };
ERR err33 = { EXIT_PASS,		"Lesen der Zufallsdatei unm�glich" };
ERR err34 = { EXIT_PASS,		"Passwort nicht �bereinstimmend" };
ERR err35 = { EXIT_CHANGE,		"�nderung eines gel�schten Archivs unm�glich" };
ERR err36 = { EXIT_CHANGE,		"�nderung eines vielteiligen Archives unm�glich" };
ERR err37 = { EXIT_CHANGE,		"�nderung eines verschl�sselten Archives unm�glich" };
ERR err38 = { EXIT_CHANGE,		"�nderung eines unverschl�sselten Archives unm�glich" };
ERR err39 = { EXIT_CHANGE,		"�nderung eines einheitlich-komprimierten Archives unm�glich" };
ERR err40 = { EXIT_NOLONG,		"Langes Befehlsformat nicht unterst�tzt in dieser Version" };
ERR err41 = { EXIT_BADWILD,		"Fehler im Platzhalterausdruck %s" };
ERR err42 = { EXIT_BADWILD,		"Platzhalterausdruck zu komplex" };
ERR err43 = { EXIT_BADWILD,		"Platzhalterausdruck im suchweg %s nicht erlaubt" };
ERR err44 = { EXIT_SECURITY,	"Gleichzeitige Nutzung von �blicher und\n\t�ffentlicher Verschl�sselung unm�glich" };
ERR err45 = { EXIT_NOCRYPT,		"Verarbeitung von versch�sseltem Archiv unm�glich" };

/****************************************************************************
*																			*
*									Warnings								*
*																			*
****************************************************************************/

char warn00[] =	"Warnung: Abgerundet wurden %u Bytes vom EOF-�berschu�\n";
char warn01[] =	"Warnung: %s. Fortsetzen [j/n] ";
char warn02[] =	"\nWarnung: Datei kann defekt sein";
char warn03[] =	"Warnung: Kein �ffentlicher Schl�ssel gefunden - Authentisierung unm�glich";
char warn04[] =	"Warnung: Authentisierungsdaten fehlerhaft - Authentisierung unm�glich\n";
char warn05[] =	"\nWarnung: Archivteil zu kurz, wird �bersprungen...";
char warn06[] =	"Warnung: Unbekannter Skriptbefehl \'%s\'\n";
char warn07[] =	"Warnung: Datendatei %s kann nicht ge�ffnet werden - wird �bersprungen\n";
char warn08[] =	"Warnung: Skriptdatei %s kann nicht ge�ffnet werden - wird �bersprungen\n";
char warn09[] =	"Warnung: Verzeichnis %s widersteht \n\t existierende Datei - wird �bersprungen\n";
char warn10[] =	"\nWarnung: Eine Datei war defekt";
char warn11[] =	"\nWarnung: %2d dateien waren defekt\n";

/****************************************************************************
*																			*
*								General Messages							*
*																			*
****************************************************************************/

char mesg00[] =	"Soll das vielteilige Archiv authentisiert werden";
char mesg01[] =	"Archiv Authentisierung schlug fehl";
char mesg02[] =	"Am Archiv-authentisieren - bitte warten\n";
char mesg03[] =	"Authentisierungsinformation wird verlorengehen";
char mesg04[] =	"Archivverzeichnis fehlerhaft";
char mesg05[] =	"Archivverzeichnis fehlerhaft, wahrscheinlich durch\n\t fehlerhaftes Passwort";
char mesg06[] =	"Archivverzeichnis wird bearbeitet...";
char mesg07[] =	"%s [j/n] ";
char mesg08[] =	"%s %s [j/n/a] ";
char mesg09[] =	"%s wird �bersprungen\n";
char mesg10[] =	"Daten sind verschl�sselt";
char mesg11[] =	"Bearbeitung der Verschl�sselung unm�glich";
char mesg12[] =	"Datendatei kann nicht ge�ffnet werden";
char mesg13[] =	"Entpacke";
char mesg14[] =	" %s als";
char mesg15[] =	" als %s";
char mesg16[] =	"Unbekannte Archivierungsmethode";
char mesg17[] =	" - %s wird �bersprungen";
char mesg18[] =	"Existierende Datei %s wird nicht �berschrieben\n";
char mesg19[] =	"Datei %s existiert bereits: Neuen Dateinamen eingeben ";
char mesg20[] =	"Suchweg %s... zu lang\n";
char mesg21[] =	"existiert bereits - �berschreiben";
char mesg22[] =	"Entpacke";
char mesg23[] =	"Zeige";
char mesg24[] =	"Teste";
char mesg25[] =	"\n\tDateitest OK";
char mesg26[] =	"\nEine Taste dr�cken...";
char mesg27[] =	"Addiere";
char mesg28[] =	"\nFehler: ";
char mesg29[] =	"Panik: Fehler w�hrend Fehlerbearbeitung";
char mesg30[] =	"Erstellen des Verzeichnisses %s";
char mesg31[] =	"\nBearbeiten der Skriptdatei %s\n";
char mesg32[] =	"Suchweg %s... zu lang in Zeile %d\n";
char mesg33[] =	"^ Unerlaubtes Zeichen im Dateinamen in Zeile %d\n";
char mesg34[] =	"Maximale Tiefe von ";
char mesg35[] =	"Addiere das Verzeichnis %s\n";
char mesg36[] =	"Bearbeite das Verzeichnis %s\n";
char mesg37[] =	"Verlasse das Verzeichnis %s\n";
char mesg38[] =	"Datei bereits im Archiv - wird �bersprungen\n";
char mesg39[] =	"Addiere";
char mesg40[] =	"L�sche";
char mesg41[] =	"%s wird aus dem Archiv gel�scht\n";
char mesg42[] =	"Frische";
char mesg43[] =	"Ersetze";
char mesg44[] =	"\nArchiv ist \'%s\'\n\n";
char mesg45[] =	"Archiv ist auf dem neusten Stand";
char mesg46[] =	"\nFertig";
char mesg47[] =	"\nVerzeichnis: ";
char mesg48[] =	"\n\tErstellt %02d/%02d/%02d, %02d:%02d:%02d\n";
char mesg49[] =	"Datei";
char mesg50[] =	"Dateien";
char mesg51[] =	"\rPasswortl�nge mu� zwischen 8 und 80 Zeichen liegen";
char mesg52[] =	"Bitte ein Passwort eingeben (8..80 Zeichen): ";
char mesg53[] =	"Bitte ein sekund�res (Zusatz-)Passwort eingeben (8..80 Zeichen): ";
char mesg54[] =	"Zur Best�tigung das Passwort nochmal eingeben: ";
char mesg55[] =	"Bitte ein Passwort f�r den Geheimschl�ssel eingeben (8..80 Zeichen): ";
char mesg56[] =	"Passwort stimmt nicht.";
char mesg57[] =	"Warnung: Beglaubugungstests mi�lungen!\nSchlechte";
char mesg58[] =	"Gute";
char mesg59[] =	" Signatur von: %s.\n\tSignatur vom %02d/%02d/%02d, %02d:%02d:%02d\n";
char mesg60[] =	" - Einen Moment...";
char mesg61[] =	"Dies ist Teil %u von einem vielteiligen Archiv\n";
char mesg62[] =	"Bitte die ";
char mesg63[] =	"n�chste Diskette eingeben";
char mesg64[] =	"vorhergehende Diskette eingeben";
char mesg65[] =	"Diskette mit ";
char mesg66[] =	"Teil %u";
char mesg67[] =	"dem letzten Teil";
char mesg68[] =	" von diesem Archiv eingeben";
char mesg69[] =	" und eine Taste dr�cken...";
char mesg70[] =	"Archiv wird fortgesetzt... ";

/****************************************************************************
*																			*
*					Special Text Strings used in Viewfile.C					*
*																			*
****************************************************************************/

/* The following messages *must* have the various columns line up since they
   are used to display archive directory listings:

   System  Length   Size  Ratio   Date     Time    Name
   ------  ------  ------ -----   ----     ----    ----
    Subdirectory                dd/mm/yy hh:mm:ss  sssssssssss...
	sssss ddddddd ddddddd  dd%  dd/mm/yy hh:mm:ss #sssssssssss...
		  ------- ------- -----					   ----
		  ddddddd ddddddd  dd%				       Total: dddd files

   Grand Total:
		  ddddddd ddddddd			         dd archives, dddd files */

char vmsg00[] =	"System   L�nge   Gr��e Ratio   Datum    Zeit    Name\n" \
				"------  ------  ------ -----   -----    ----    ----";
char vmsg01[] =	" Unterverzeichnis            %02d/%02d/%02d %02d:%02d:%02d  %s\n";
char vmsg02[] =	" %s %7lu %7lu  %2d%%  %02d/%02d/%02d %02d:%02d:%02d %c%s\n";
char vmsg03[] =	"       ------- ------- -----                    ----\n" \
				"       %7lu %7lu  %2d%%                     Zusammen: %u %s\n";
char vmsg04[] =	"\nInsgesamt:\n" \
				"       %7lu %7lu                     %2d Archive, %u %s\n";

/****************************************************************************
*																			*
*								OS-Specific Messages						*
*																			*
****************************************************************************/

#if defined( __MSDOS__ )
char osmsg00[] = "Dateinutzungssverletzung (share)";
char osmsg01[] = "Dateizugriffsverletzung (locking)";
char osmsg02[] = "Dateiname entspricht einem Ger�t";
#endif /* __MSDOS__ */

/****************************************************************************
*																			*
*						Display Various (Long) Messages						*
*																			*
****************************************************************************/

#ifndef __MSDOS__

void showTitle( void )
	{
	hputs( "HPACK - Der multi-system Archivierer Version 0.78a0.  Freigabedatum: 1 Sept 1992" );
	hputs( "F�r Amiga, Archimedes, Atari, Macintosh, MSDOS, OS/2, und UNIX" );
#if defined( __AMIGA__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Amiga-Port von N.Little u.P.Gutmann." );
#elif defined( __ARC__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Arc-Port von Jason, Edouard u.Peter." );
#elif defined( __ATARI__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Atari-Port von M.Moffatt u.P.Gutmann." );
#elif defined( __MAC__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Mac-Portierung von Peter Gutmann." );
#elif defined( __OS2__ )
	hputs( "Copyright (c) Peter Gutmanm 1989 - 1992.  OS/2-Portierung von John Burnell." );
#elif defined( __UNIX__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  UNIX-Portierung von Stuart Woolford." );
#else
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  xxxx-Portierung von yyyy zzzz." );
#endif /* Various OS-dependant credits */
	}

void showHelp( void )
	{
	hputs( "\nGebrauch: HPACK Befehl [Optionen] Archiv [Datendateien...][@Skriptdateien...]" );
	hputs( "\nG�ltige HPACK-Befehle:" );
	hputs( " A - Addieren           X - Extrahieren         V - ArchivVerzeichnis ansehen" );
	hputs( " P - Dateien ansehen    D - Dateien l�schen     F - Dateien auffrischen" );
	hputs( " R - Dateien ersetzen   U - Dateien erneuern    T - Archiv auf Fehler testen" );
	hputs( "\nG�ltige HPACK-Optionen:" );
	hputs( " -K - Vorhandene Dateien �berschreiben -T - Zeitstempeln beim Extahieren" );
	hputs( " -R - Bearbeite Unterverzeichnisse     -D - Speichere Unterverzeichnisse" );
	hputs( " -S - Stiller Modus (keine Infos)      -A - Behalte alle Dateiattribute" );
	hputs( " -I - Interaktiver Modus               -L - Authentisierungsinfos hinzuf�gen" );
	hputs( " -W - F�ge Archivkommentar hinzu       -E - F�ge Fehlerr�ckgewinnungsinfo hinzu" );
	hputs( " -U - Packe einheitlich                -C<*> - Verschl�sselungsart s. HPACK.DOC" );
	hputs( " -B<Suchweg> - Hauptsuchwegangabe      -D<*> - Verzeichnisoptionen s. HPACK.DOC" );
	hputs( " -V<F,D,S> - Ansehen(Datei,Verz,Sortn) -Z<*> - Sys.eigene Optionen s. HPACK.DOC" );
	hputs( " -O<N,A,S,P>       - Optionen f�rs �berschreiben (Keine, Alle, Auto, Fragen)" );
#if defined( __ATARI__ ) || defined( __MSDOS__ ) || defined( __OS2__ )
	hputs( " -X<R,Xxx,L,E,P,A> - �bersetzung (auto, CR, Hexadez.,LF,EBCDIC,Prime,ASCII)" );
#elif defined( __MAC__ )
	hputs( " -X<C,Xxx,L,E,P,A> - �bersetzung (auto,CRLF,Hexadez.,LF,EBCDIC,Prime,ASCII)" );
#elif defined( __AMIGA__ ) || defined( __ARC__ ) || defined( __UNIX__ )
	hputs( " -X<R,Xxx,C,E,P,A> - �bersetzung (auto,CR,Hexadez.,CRLF,EBCDIC,Prime,ASCII)" );
#endif /* Various OS-specific defines */
	hputs( "\n\"Archiv\" ist ein HPACK Archiv, mit der voreingestellten Endung .HPK" );
	hputs( "\"Datendateien\" sind die zu archivierenden Dateien; voreingestellt sind alle." );
	hputs( "        Die erweiterten Platzhalterzeichen *, ?, [], ^/!, \\ sind erlaubt." );
	exit( OK );
	}

#endif /* !__MSDOS__ */
