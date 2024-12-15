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
   Language:		Dutch.
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
ERR err00 = { EXIT_INT_ERR,		"Interne fout" };
#endif /* __ARC__ */
ERR err01 = { EXIT_NO_MEM,		"Geheugen vol" };
ERR err02 = { EXIT_NO_DISK,		"Geen ruimte op de diskette" };
ERR err03 = { EXIT_NO_ARCH,		"Archief bestand %s can niet geopend worden" };
ERR err04 = { EXIT_NO_TEMP,		"Tijdelijk bestand %s kan niet geopend worden" };
ERR err05 = { EXIT_NO_PATH,		"Pad %s niet gevonden" };
ERR err06 = { EXIT_NO_BASE,		"Geen toegang tot het basis pad" };
ERR err07 = { EXIT_NO_MKDIR,	"Pad %s kan niet gemaakt worden" };
ERR err08 = { EXIT_BREAK,		"Gestopt op aanvraag van de gebruiker" };
#if defined( __MSDOS__ )
ERR err09 = { EXIT_FILE_ERR,	"Bestand fout nummer %02d" };
#else
ERR err09 = { EXIT_FILE_ERR,	"Bestand fout" };
#endif /* __MSDOS__ */
ERR err10 = { EXIT_DIR_CORRUPT,	"Archief pad is korrupt" };

			/* Standard errors */
ERR err11 = { EXIT_LONG_PATH,	"Pad %s is te lang" };
ERR err12 = { EXIT_LONG_PATH,	"Pad %s... is te lang" };
ERR err13 = { EXIT_LONG_PATH,	"Pad %s%s is te lang" };
ERR err14 = { EXIT_LONG_PATH,	"Pad %s%s... is te lang" };
ERR err15 = { EXIT_LONG_PATH,	"Pad %s/%s is te lang" };
ERR err16 = { EXIT_NO_OVERRIDE,	"Basis pad kan niet overschreven worden" };
ERR err17 = { EXIT_NEST,		"Te veel niveaus in elkaar voor het pad" };
ERR err18 = { EXIT_SCRIPT_ERR,	"%s%d fouten gevonden in script bestand" };
ERR err19 = { EXIT_NOT_ARCH,	"Niet een HPACK archief" };
ERR err20 = { EXIT_NOTHING,		"Geen dezelfde bestanden op de diskette" };
ERR err21 = { EXIT_NOTHING,		"Geen dezelfde bestanden in het archief" };
ERR err22 = { EXIT_NOTHING,		"Geen dezelfde archieven gevonden" };
ERR err23 = { EXIT_BAD_KEYFILE,	"Foutief sleutel bestand" };
ERR err24 = { EXIT_COMMAND,		"Onbekend kommando \'%s\'" };
ERR err25 = { EXIT_OPTION,		"Onbekende pad optie \'%c'\" };
ERR err26 = { EXIT_OPTION,		"Onbekende overschrijf optie \'%c'\" };
ERR err27 = { EXIT_OPTION,		"Onbekende bekijk optie \'%c'\" };
ERR err28 = { EXIT_OPTION,		"Onbekende optie \'%c'\" };

			/* Minor errors */
ERR err29 = { EXIT_PASS,		"Missende userID voor encriptie/autenticieteit" };
ERR err30 = { EXIT_PASS,		"Kan geheime sleutel niet vinden voor userID \'%s\'" };
ERR err31 = { EXIT_PASS,		"Kan geheime sleutel niet vinden - kan informatie niet ontcijferen" };
ERR err32 = { EXIT_PASS,		"Kan publieke sleutel niet vinden voor userID \'%s\'" };
ERR err33 = { EXIT_PASS,		"Kan het random seed bestand niet lezen" };
ERR err34 = { EXIT_PASS,		"Paswoord niet het zelfde" };
ERR err35 = { EXIT_CHANGE,		"Een verwijderd archief kan niet veranderd worden" };
ERR err36 = { EXIT_CHANGE,		"Een archief uit meerdere delen kan niet veranderd worden" };
ERR err37 = { EXIT_CHANGE,		"Een cijfer bestand kan niet veranderd worden" };
ERR err38 = { EXIT_CHANGE,		"Een ontcijfered bestand kan niet veranderd worden" };
ERR err39 = { EXIT_CHANGE,		"Een eenheid gekompreseerd bestand kan niet veranderd worden" };
ERR err40 = { EXIT_NOLONG,		"Lang argument formaat niet ondersteund in deze versie" };
ERR err41 = { EXIT_BADWILD,		"Slechte algemeene uitdrukking in %s" };
ERR err42 = { EXIT_BADWILD,		"Algemeene uitdrukken te complex" };
ERR err43 = { EXIT_BADWILD,		"Een algemeene uitdrukking kan niet gebruikt worden\n      in extern pad naam %s" };
ERR err44 = { EXIT_SECURITY,	"Kan niet beide konventioneel en publieke-sleutel enkriptie gebruiken" };
ERR err45 = { EXIT_NOCRYPT,		"Kan cijfer archief niet verwerken" };

/****************************************************************************
*																			*
*									Warnings								*
*																			*
****************************************************************************/

char warn00[] =	"Waarschuwing: Afgerond werden %u bytes van de EOF-overschot";
char warn01[] =	"Waarschuwing: %s. Verder gaan [j/n] ";
char warn02[] =	"\nWaarschuwing: Bestand kan defect zijn";
char warn03[] =	"Waarschuwing: Publieke sleutel niet gevonden - kan geen autent. kontrole doen\n";
char warn04[] =	"Waarschuwing: Autenticiteits info. is corrupt - kan geen autent. kontrole doen\n";
char warn05[] =	"\nWaarschuwing: Archief sektie is te groot, wordt overgeslagen...";
char warn06[] =	"Waarschuwing: Onbekende script instructie \'%s\'\n";
char warn07[] =	"Waarschuwing: Kan bestand %s niet openen - overgeslagen\n";
char warn08[] =	"Waarschuwing: Script bestand %s kan niet geopend worden - overgeslagen\n";
char warn09[] =	"Waarschuwing: Pad %s is in konflict met\n\t      bestaand bestand - overgeslagen\n";
char warn10[] =	"\nWaarschuwing: !!!!!!!!!!!!!!!!!!";
char warn11[] =	"\nWaarschuwing: %2d !!!!!!!!!!!!!!!!!\n";

/****************************************************************************
*																			*
*								General Messages							*
*																			*
****************************************************************************/

char mesg00[] =	"Kontroleer veeldelig bestands autenticiteit";
char mesg01[] =	"Archief faalt autenticiteits kontrole";
char mesg02[] =	"Archief autenticiteits kontrole - aub. wachten\n";
char mesg03[] =	"Autenticiteits informatie zal vernietigd worden";
char mesg04[] =	"Archief pad is fout";
char mesg05[] =	"Archief pad is fout, waarschijnlijk door\n\t      een inkorect paswoord";
char mesg06[] =	"Archief pad aan het verwerken...";
char mesg07[] =	"%s [j/n] ";
char mesg08[] =	"%s %s [j/n/a] ";
char mesg09[] =	"%s wordt overgeslagen";
char mesg10[] =	"Informatie is versleuteld";
char mesg11[] =	"Verwerken van de versleutelde informatie is onmogelijk";
char mesg12[] =	"Informatie bestand kan niet geopend worden";
char mesg13[] =	"Aan het ontpakken";
char mesg14[] =	" %s als";
char mesg15[] =	" als %s";
char mesg16[] =	"Onbekende Archief metode";
char mesg17[] =	" - %s wordt overgeslagen";
char mesg18[] =	"Bestaand bestand %s word niet overschreven\n";
char mesg19[] =	"Bestand %s bestaat al: Geef een nieuwe bestandnaam ";
char mesg20[] =	"Pad %s... te lang\n";
char mesg21[] =	"bestaat al - overschrijven";
char mesg22[] =	"Ontpakken";
char mesg23[] =	"Laten zien";
char mesg24[] =	"Testen";
char mesg25[] =	"\n\tBestand OK getest";
char mesg26[] =	"\nDruk een toets";
char mesg27[] =	"Bijvoegen";
char mesg28[] =	"\nFout: ";
char mesg29[] =	"Paniek: Fout gedurende foutbewerkingen";
char mesg30[] =	"Pad %s aan het maken";
char mesg31[] =	"\nScript bestand %s aan het verwerken\n";
char mesg32[] =	"Pad %s... te lang in regel %d\n";
char mesg33[] =	"^ Ongeldig karakter in de bestandnaam in regel %d\n";
char mesg34[] =	"Maximale diepte van ";
char mesg35[] =	"Pad %s aan het bijvoegen\n";
char mesg36[] =	"Pad %s aan het kontroleren\n";
char mesg37[] =	"Pad %s aan het verlaten\n";
char mesg38[] =	"Bestand %s is al in het archief - wordt overgeslagen\n";
char mesg39[] =	"Toevoegen";
char mesg40[] =	"Verwijderen";
char mesg41[] =	"%s wordt verwijderd van het archief\n";
char mesg42[] =	"Verfrissen";
char mesg43[] =	"Verplaatsen";
char mesg44[] =	"\nArchief is \'%s\'\n\n";
char mesg45[] =	"Archief is bijgewerkt";
char mesg46[] =	"\nKlaar";
char mesg47[] =	"\nPad: ";
char mesg48[] =	"\n\tGemaakt %02d/%02d/%02d, %02d:%02d:%02d\n";
char mesg49[] =	"bestand";
char mesg50[] =	"bestanden";
char mesg51[] =	"\rPaswoord moet tussen 8 en 80 karakters lang zijn";
char mesg52[] =	"Aub paswoord invoeren (8..80 karakters): ";
char mesg53[] =	"Aub tweede paswoord invoeren (8..80 karakters): ";
char mesg54[] =	"Paswoord weer invoeren voor verificatie: ";
char mesg55[] =	"Aub. een paswoord invoeren voor de geheime sleutel (8.80 karakters): ";
char mesg56[] =	"Paswoord klopt niet.";
char mesg57[] =	"Waarschuwing: Informatie faalt autenticiteit!\nSlecht";
char mesg58[] =	"Goed";
char mesg59[] =	" Ondertekend door: %s.\n\tOndertekend op %02d/%02d/%02d, %02d:%02d:%02d\n";
char mesg60[] =	" - Een momentje aub...";
char mesg61[] =	"Dit is deel %u van een veeldelig archief\n";
char mesg62[] =	"Aub leg in de ";
char mesg63[] =	"volgende schijf";
char mesg64[] =	"voorgaande schijf";
char mesg65[] =	"schijf bevat ";
char mesg66[] =	"deel %u";
char mesg67[] =	"het laatste deel";
char mesg68[] =	" van dit archief";
char mesg69[] =	" en druk een toets";
char mesg70[] =	"Aan het voortzetten... ";

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

char vmsg00[] =	"Systeem  Lengte  Groote Ratio   Datum     Tijd    Naam\n" \
				"-------  ------  ------ -----   -----     ----    ----";
char vmsg01[] =	" onderpad                    %02d/%02d/%02d %02d:%02d:%02d  %s\n";
char vmsg02[] =	" %s %7lu %7lu  %2d%%  %02d/%02d/%02d %02d:%02d:%02d %c%s\n";
char vmsg03[] =	"       ------- ------- -----                     ----\n" \
				"       %7lu %7lu  %2d%%                     Totaal: %u %s\n";
char vmsg04[] =	"\nGroote Totaal:\n" \
				"       %7lu %7lu                    %2d archieven, %u %s\n";

/****************************************************************************
*																			*
*								OS-Specific Messages						*
*																			*
****************************************************************************/

#if defined( __MSDOS__ )
char osmsg00[] = "Bestand samendeling overschreden";
char osmsg01[] = "Bestand toegang overschreden";
char osmsg02[] = "Bestand naam is het zelfde als een apparaat naam";
#endif /* __MSDOS__ */

/****************************************************************************
*																			*
*						Display Various (Long) Messages						*
*																			*
****************************************************************************/

#ifndef __MSDOS__

void showTitle( void )
	{
	hputs( "HPACK - Het multie-systeem archief programma V.0.78a0. Uitgevings dd:1 Sept 1992" );
	hputs( "Voor Amiga, Archimedes, Atari, Macintosh, MSDOS, OS/2, en UNIX" );
#if defined( __AMIGA__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Amiga port door N.Little & P.Gutmann." );
#elif defined( __ARC__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Arc port door Jason, Edouard & Peter." );
#elif defined( __ATARI__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Atari port door M.Moffatt en P.Gutmann" );
#elif defined( __MAC__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Macintosh port door Peter Gutmann." );
#elif defined( __OS2__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  OS/2 port door John Burnell." );
#elif defined( __UNIX__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Unix port door Stuart Woolford." );
#else
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  xxxx port door yyyy zzzz." );
#endif /* Various OS-dependant credits */
	}

void showHelp( void )
	{
	hputs( "\nGebruik: HPACK kommando [opties] archief [informat.best...][@script best... ]" );
	hputs( "\nValiede HPACK kommandos zijn:" );
	hputs( " A - Bestanden toevoegen   X - Bestanden extracteren V - Pad van de bestanden" );
	hputs( " P - Bestanden bekijken    D - Best. verwijderen F - Best.in het arch.verversen" );
	hputs( " R - Bestanden verplaatsen U - Bestanden bijwerken   T - Test arch. integriteit" );
	hputs( "\nGeldige HPACK opties zijn:" );
	hputs( " -K - Bestaande archief overschrijven  -T - Bestanden aanraken op extractie" );
	hputs( " -R - Recursief werken door padden     -D - Sla padden van recursie op" );
	hputs( " -S - Stille zetting (geen informatie) -A - Opslaan van bestand atributen" );
	hputs( " -I - Interactieve zetting             -L - Authenticitatie informat. bijvoegen" );
	hputs( " -W - Archief komentaar bijvoegen      -E - Fout verwerkings informat.bijvoegen" );
	hputs( " -U - Eenheid kompressie zetting       -C<*> - Encryptie opties: zie HPACK.DOC" );
	hputs( " -B<path>  - Geef basis pad            -D<*> - Pad opties: zie HPACK.DOC" );
	hputs( " -V<F,D,S> - Zie opties(Best,Padden,Sort) -Z<*> - Sys-spec.opties:zie HPACK.DOC" );
	hputs( " -O<N,A,S,P>       - Overschrijf opties (Geen, Alles, Slim, Vraag eerst)" );
#if defined( __ATARI__ ) || defined( __MSDOS__ ) || defined( __OS2__ )
	hputs( " -X<R,Xxx,L,E,P,A> - vertaal opties (slim, CR, Hexadec., LF,EBCDIC,Prime,ASCII)" );
#elif defined( __MAC__ )
	hputs( " -X<C,Xxx,L,E,P,A> - vertaal opties (slim, CRLF,Hexadec.,LF,EBCDIC,Prime,ASCII)" );
#elif defined( __AMIGA__ ) || defined( __ARC__ ) || defined( __UNIX__ )
	hputs( " -X<R,Xxx,C,E,P,A> - vertaal opties (slim, CR,Hexadec.,CRLF,EBCDIC,Prime,ASCII)" );
#endif /* Various OS-specific defines */
	hputs( "\n\"archief\" is het HPACK archief, met een standaard extensie van .HPK" );
	hputs( "\"informatie bestanden\" zijn de te archieverende bastanden, alle bestanden als" );
	hputs( "geen gespecificeerd. Algemene karakters *,?, [], ^/!, \\ kunnen gebruikt worden." );
	exit( OK );
	}

#endif /* !__MSDOS__ */
