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
   Language:		Italian.
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
ERR err00 = { EXIT_INT_ERR,		"Errore interno" };
#endif /* __ARC__ */
ERR err01 = { EXIT_NO_MEM,		"Memoria insufficiente" };
ERR err02 = { EXIT_NO_DISK,		"Spazio su disco insufficiente" };
ERR err03 = { EXIT_NO_ARCH,		"Impossibile aprire l' archivio %s" };
ERR err04 = { EXIT_NO_TEMP,		"Impossibile aprire il file temporaneo" };
ERR err05 = { EXIT_NO_PATH,		"Path %s inesistente" };
ERR err06 = { EXIT_NO_BASE,		"Impossibile accedere alla directory di base" };
ERR err07 = { EXIT_NO_MKDIR,	"Impossibile creare directory %s" };
ERR err08 = { EXIT_BREAK,		"Uscita su richesta utente" };
#if defined( __MSDOS__ )
ERR err09 = { EXIT_FILE_ERR,	"Errore file numero %02d" };
#else
ERR err09 = { EXIT_FILE_ERR,	"Errore file" };
#endif /* __MSDOS__ */
ERR err10 = { EXIT_DIR_CORRUPT,	"Directory dell' archivio danneggiata" };

			/* Standard errors */
ERR err11 = { EXIT_LONG_PATH,	"Lunghezza path %s eccessiva" };
ERR err12 = { EXIT_LONG_PATH,	"Lunghezza path %s... eccessiva" };
ERR err13 = { EXIT_LONG_PATH,	"Lunghezza path %s%s eccessiva" };
ERR err14 = { EXIT_LONG_PATH,	"Lunghezza path %s%s... eccessiva" };
ERR err15 = { EXIT_LONG_PATH,	"Lunghezza path %s/%s... eccessiva" };
ERR err16 = { EXIT_NO_OVERRIDE,	"Impossibile modificare path di base" };
ERR err17 = { EXIT_NEST,		"Troppi livelli di directory nesting" };
ERR err18 = { EXIT_SCRIPT_ERR,	"%s%d errori trovati in script file" };
ERR err19 = { EXIT_NOT_ARCH,	"Non è un' archivio HPACK" };
ERR err20 = { EXIT_NOTHING,		"Non ci sono files corrispondenti sul disco" };
ERR err21 = { EXIT_NOTHING,		"Non ci sono files corrispondenti nell' archivio" };
ERR err22 = { EXIT_NOTHING,		"Non ci sono archivi corrispondenti" };
ERR err23 = { EXIT_BAD_KEYFILE,	"Keyfile invalido" };
ERR err24 = { EXIT_COMMAND,		"Comando sconosciuto \'%s\'" };
ERR err25 = { EXIT_OPTION,		"Opzione per directory sconosciuta: \'%s\'" };
ERR err26 = { EXIT_OPTION,		"Opzione per modifica sconosciuta: \'%\'" };
ERR err27 = { EXIT_OPTION,		"Opzione per visualizzazione sconosciuta: \'%\'" };
ERR err28 = { EXIT_OPTION,		"Opzione sconosciuta: \'%s\'" };

			/* Minor errors */
ERR err29 = { EXIT_PASS,		"Manca User ID per crittografare/convalidare" };
ERR err30 = { EXIT_PASS,		"Chiave segreta introvabile per User ID \'%s\'" };
ERR err31 = { EXIT_PASS,		"Chiave segreta introvabile - impossibile decrittografare i dati" };
ERR err32 = { EXIT_PASS,		"Chiave pubblica introvabile per User ID \'%s'\" };
ERR err33 = { EXIT_PASS,		"File di numeri casuali illeggibile" };
ERR err34 = { EXIT_PASS,		"Parole chiave non identiche" };
ERR err35 = { EXIT_CHANGE,		"Impossibile modificare un' archivio cancellato" };
ERR err36 = { EXIT_CHANGE,		"Impossibile modificare un' archivio multiplo" };
ERR err37 = { EXIT_CHANGE,		"Impossibile modificare un' archivio crittografato" };
ERR err38 = { EXIT_CHANGE,		"Impossibile modificare un' archivio in chiaro" };
ERR err39 = { EXIT_CHANGE,		"Impossibile modificare un' archivio compresso per unità" };
ERR err40 = { EXIT_NOLONG,		"Il formato esteso per le opzioni non è disponibile" };
ERR err41 = { EXIT_BADWILD,		"Formato wildcard in %s invalido" };
ERR err42 = { EXIT_BADWILD,		"Espressione wildcard eccessivamente complessa" };
ERR err43 = { EXIT_BADWILD,		"L' utilizzo delle wildcard nella path esterna\n\t %s non è possibile" };
ERR err44 = { EXIT_SECURITY,	"Non è possibile utilizzare sia la crittografia a \n\t chiave pubblica che convenzionale" };
ERR err45 = { EXIT_NOCRYPT,		"Impossibile procedere con file crittografato" };

/****************************************************************************
*																			*
*									Warnings								*
*																			*
****************************************************************************/

char warn00[] =	"Avviso: Troncati %s bytes di padding EOF\n"";
char warn01[] =	"Avviso: %s. Continuo [s/n] ";
char warn02[] =	"\nAvviso: File potrebbe essere danneggiato";
char warn03[] =	"Avviso: Chiave pubblica introvabile - impossibile controllare autenticità\n";
char warn04[] =	"Avviso: Dati autenticazione danneggiati - impossibile controllare autenticità\n";
char warn05[] =	"\nAvviso: Sezione dell' archivio troppo corta, ignorata...";
char warn06[] =	"Avviso: Comando di script \'%s\' sconosciuto\n";
char warn07[] =	"Avviso: Apertura del file di dati %s impossibile - ignorato\n";
char warn08[] =	"Avviso: Apertura del file di script %s impossibile - ignorato\n";
char warn09[] =	"Avviso: Directory %s in conflitto con\n\tfile esistente - ignorata\n"";
char warn10[] =	"\nAvviso: Un file è danneggiato";
char warn11[] =	"\nAvviso: %2d file sono danneggiati\n";

/****************************************************************************
*																			*
*								General Messages							*
*																			*
****************************************************************************/

char mesg00[] =	"Verifica dell' autenticità dell' archivio multiplo";
char mesg01[] =	"L' archivio non è autenticato";
char mesg02[] =	"Verifca autenticità dell' archivio - attendere prego\n";
char mesg03[] =	"Informazioni per l' autentica verranno distrutte";
char mesg04[] =	"Directory dell' archivio danneggiata";
char mesg05[] =	"Directory dell' archivio danneggiata, probabilmente\n\tdovuto ad una parola chiave errata";
char mesg06[] =	"Analisi della directory dell' archivio...";
char mesg07[] =	"%s [s/n] ";
char mesg08[] =	"%s %s [s/n/t] ";
char mesg09[] =	"%s ignorato\n";
char mesg10[] =	"I dati sono crittografati";
char mesg11[] =	"Impossibile analizzare informazione crittografica";
char mesg12[] =	"Impossibile aprire file di dati";
char mesg13[] =	"Estraendo";
char mesg14[] =	" %s come";
char mesg15[] =	" come %s";
char mesg16[] =	"Metodo di compressione sconosciuto";
char mesg17[] =	" - %s ignorato";
char mesg18[] =	"Il file %s già esistente non verrà sovrascritto\n";
char mesg19[] =	"%s è già esistente: Inserire nuovo nome per il file ";
char mesg20[] =	"Path %s... lunghezza eccessiva\n";
char mesg21[] =	"già esistente - sovrascrivo";
char mesg22[] =	"Estrarre";
char mesg23[] =	"Visualizzare";
char mesg24[] =	"Controllare";
char mesg25[] =	"\n\tFile controllato";
char mesg26[] =	"\nPremere un tasto...";
char mesg27[] =	"Aggiungendo";
char mesg28[] =	"\nErrore: ";
char mesg29[] =	"Panico: Errore durante correzione dell' errore";
char mesg30[] =	"Creando directory %s";
char mesg31[] =	"\nAnalisi del file di script %s\n";
char mesg32[] =	"Directory %s... lunghezza eccessiva alla riga %d\n";
char mesg33[] =	"^ Carattere invalido nel nome del file alla riga %d\n";
char mesg34[] =	"Livello massimo di ";
char mesg35[] =	"Aggiungendo directory %s\n";
char mesg36[] =	"Analizzando directory %s\n";
char mesg37[] =	"Ignorando directory %s\n";
char mesg38[] =	"File %s già in archivio - ignorato\n";
char mesg39[] =	"Aggiungere";
char mesg40[] =	"Cancellare";
char mesg41[] =	"Cancellando %s dall' archivio\n";
char mesg42[] =	"Rinfrescare";
char mesg43[] =	"Sostituire";
char mesg44[] =	"\nArchivio è \'%s\'\n\n";
char mesg45[] =	"\nTerminato";
char mesg46[] =	"\nDirectory: ";
char mesg47[] =	"\n\tCreato il %02d/%02d/%02d, %02d:%02d:%02d\n";
char mesg48[] =	"file";
char mesg49[] =	"file";
char mesg50[] =	"\rLa parola chiave deve essere tra gli 8 e gli 80 caratteri di lunghezza";
char mesg51[] =	"Inserire parola chiave (8..80 caratteri): ";
char mesg52[] =	"Inserire parola chiave secondaria (8..80 caratteri): ";
char mesg53[] =	"Reinserire parola chiave per confermare: ";
char mesg54[] =	"Inserire parola chiave per chiave segreta (8..80 caratteri): ";
char mesg55[] =	"Parola chiave errata.";
char mesg56[] =	"Avviso: Dati non passano controllo autenticità!\nErrata";
char mesg57[] =	"Corretta";
char mesg58[] =	" firma da: %s.\n\tFirma del %02d/%02d/%02d, %02d:%02d:%02d\n";
char mesg59[] =	" - Attendere prego...";
char mesg60[] =	"Questa è la sezione %u di un' archivio multiplo\n";
char mesg61[] =	"Inserire il ";
char mesg62[] =	"disco successivo";
char mesg63[] =	"disco precedente";
char mesg64[] =	"disco contenente ";
char mesg65[] =	"parte %s";
char mesg66[] =	"parte finale";
char mesg67[] =	" di questo archivio";
char mesg68[] =	" e premere un tasto";
char mesg69[] =	"Continuando... ";

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

char vmsg00[] =	"Sistema  Lungh.   Dim.  Prop.   Data      Ora    Nome\n" \
				"-------  ------  ------ -----   ----      ---    ----";
char vmsg01[] =	" Subdirectory                %02d/%02d/%02d %02d:%02d:%02d  %s\n";
char vmsg02[] =	" %s %7lu %7lu  %2d%%  %02d/%02d/%02d %02d:%02d:%02d %c%s\n";
char vmsg03[] =	"       ------- ------- -----                    ----\n" \
				"       %7lu %7lu  %2d%%                     Totale: %u %s\n";
char vmsg04[] =	"\nGran Totale:\n" \
				"       %7lu %7lu                    %2d archivi, %u %s\n";

/****************************************************************************
*																			*
*								OS-Specific Messages						*
*																			*
****************************************************************************/

#if defined( __MSDOS__ )
char osmsg00[] = "File sharing violation";
char osmsg01[] = "File locking violation";
char osmsg02[] = "File corrisponde a device driver";
#endif /* __MSDOS__ */

/****************************************************************************
*																			*
*						Display Various (Long) Messages						*
*																			*
****************************************************************************/

#ifndef __MSDOS__

void showTitle( void )
	{
	hputs( "HPACK - L' archiviatiore multi-sistema Versione 0.78a0.  Del: 1 Sett 1992" );
	hputs( "Per Amiga, Archimedes, Atari, Macintosh, MSDOS, OS/2 e UNIX" );
#if defined( __AMIGA__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Amiga port di N.Little & P.Gutmann." );
#elif defined( __ARC__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Arc port di Jason, Edouard & Peter." );
#elif defined( __ATARI__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Atari port di M.Moffatt and P.Gutmann." );
#elif defined( __MAC__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Macintosh port di Peter Gutmann." );
#elif defined( __OS2__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  OS/2 port di John Burnell." );
#elif defined( __UNIX__ )
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  Unix port di Stuart Woolford." );
#else
	hputs( "Copyright (c) Peter Gutmann 1989 - 1992.  xxxx port di yyyy zzzz." );
#endif /* Various OS-dependant credits */
	}

void showHelp( void )
	{
	hputs( "\nUso: HPACK comando [opzione] archivio [file dati ...][@file_di_script... ]" );
	hputs( "\nI comandi riconosciuti da HPACK sono:" );
	hputs( " A - Aggiungere file    X - Estrarre file       V - Lista dei file" );
	hputs( " P - Visualizza file    D - Cancella file       F - Rinfresca i files" );
	hputs( " R - Sostituisci file   U - Aggiorna file       T - Controlla integrità" );
	hputs( "\nLe opzioni riconosciute da HPACK sono:" );
	hputs( " -K - Sovrascrivi archivio esistente   -T - Aggiorna la data estraendo" );
	hputs( " -R - Visita tutte le subdirectories   -D - Salva le directories vistate" );
	hputs( " -S - Silenzioso (Niente messaggi)     -A - Salva/Estrai attributi dei file" );
	hputs( " -I - Interattivo (Messagi e domande)  -L - Aggiungi informazioni autentica" );
	hputs( " -W - Aggiungi commento                -E - Aggiungi correzione errori" );
	hputs( " -U - Comprimi per unità               -C<*> - Opzioni crittogr.: in HPACK.DOC" );
	hputs( " -B<path>  - Specifica base path       -D<*> - Opzioni directory: in HPACK.DOC" );
	hputs( " -V<F,D,S> - Opz.Vis.(Files,Dirs,Sort) -Z<*> - Opzioni sistema: in HPACK.DOC" );
	hputs( " -O<N,A,S,P>       - Opzioni sovrascrittura (None, All, Smart, Prompt)" );
#if defined( __ATARI__ ) || defined( __MSDOS__ ) || defined( __OS2__ )
	hputs( " -X<R,Xxx,L,E,P,A> - Opz. traduzione (smart, CR, Hex , LF,EBCDIC, Prime, ASCII)" );
#elif defined( __MAC__ )
	hputs( " -X<C,Xxx,L,E,P,A> - Opz. traduzione (smart, CRLF, Hex, LF,EBCDIC,Prime,ASCII)" );
#elif defined( __AMIGA__ ) || defined( __ARC__ ) || defined( __UNIX__ )
	hputs( " -X<R,Xxx,C,E,P,A> - Opz. traduzione (smart, CR, Hex, CRLF, EBCDIC,Prime,ASCII)" );
#endif /* Various OS-specific defines */
	hputs( "\n\"archivio\" è l' archivio HPACK, con file extension .HPK" );
	hputs( "\"file dati\" sono i file da archiviare, con default di tutti i file in" );
	hputs( "     mancanza di nomi. Le wildcards utilizzabili sono *, ?, [], ^/!, \\." );
	exit( OK );
	}

#endif /* !__MSDOS__ */
