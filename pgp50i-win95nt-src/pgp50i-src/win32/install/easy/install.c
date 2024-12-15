/*Very minimalist installation program.  Finds the system directory
 *and the PGP 50 application directory.  Asks user if she wants to
 *install Eudora, and where to put it, if so.  Asks user if she
 *wants the Exchange plugin. After all this info is gathered, we
 *copy all the files to their appropriate places, and set up the
 *neccessary reg keys.
 *
 *Yes, I know this doesn't do error checking and isn't very
 *user-friendly.  It is, however, better than typing in all those
 *reg keys and copying all those files, which is what you were going
 *to have to do before I wrote this at the last minute.
 *
 *06/11/97 23:30 Brett A. Thomas <quark@baz.com>
 *
 *$Id$
 *
 *Copyright (C) 1997 Pretty Good Privacy, Inc.  All Rights Reserved.
 */

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

void usage(void);
void get_system_dir(char *system_dir);
void get_install_dir(char *install_dir);
void get_eudora_dir(char *eudora_dir);
void get_source_dir(char *source_dir);
int query_install_eudora(void);
int query_install_exchange(void);
void perform_install(char *source_dir,
					 char *system_dir,
					 char *install_dir,
					 char *eudora_dir,
					 int do_eudora_install,
					 int do_exchange_install);
void get_input(char *string,
			   int max_len,
			   char *query_message,
			   char *default_value);
void banner(void);
void do_regkeys(char *install_dir);
void create_dir(char *install_dir);

int main(int argc, char **argv)
{
	char system_dir[MAX_PATH] = "\0",
		 install_dir[MAX_PATH] = "\0",
		 eudora_dir[MAX_PATH] = "\0",
		 source_dir[MAX_PATH] = "\0";
	int do_eudora_install = 0, do_exchange_install = 0;

	banner();

	if(argc > 1)
	{
		usage();
		return(0);
	}

	printf("\nPrior to installation, you need to answer a few questions.\n");
	printf("\
The default answers are printed in square brackets.  If you don't know the\n\
answer to a given question, it is probably safe to simply hit ENTER and\n\
accept the default.\n\n");

	get_source_dir(source_dir);
	get_system_dir(system_dir);
	get_install_dir(install_dir);
	if((do_eudora_install = query_install_eudora()))
		get_eudora_dir(eudora_dir);
	do_exchange_install = query_install_exchange();
	perform_install(source_dir,
					system_dir,
					install_dir,
					eudora_dir,
					do_eudora_install,
					do_exchange_install);

	return(0);
}

void banner(void)
{
	printf("\
PGP for Personal Privacy Easy Installer, v0.1, by Brett Thomas\n\
Copyright (C) 1997 Pretty Good Privacy, Inc.  All Rights Reserved.\n");
}

void usage(void)
{
	printf("\
Installs PGP 5.0 for Personal Privacy.  Interactively queries the\n\
user about install options.  Creates reg keys.\n");
}

void get_system_dir(char *system_dir)
{
	get_input(system_dir,
			  MAX_PATH,
			  "\n\
Enter the path to your Windows System directory\n[c:\\windows\\system]:  ",
              "c:\\windows\\system");
}

void get_install_dir(char *install_dir)
{
	get_input(install_dir,
			  MAX_PATH,
			  "\n\
Enter the path you wish PGP to be installed in\n\
[c:\\program files\\pgp\\pgp50i]:  ",
              "c:\\program files\\pgp\\pgp50i");
}

void get_eudora_dir(char *eudora_dir)
{
	get_input(eudora_dir,
			  MAX_PATH,
			  "\n\
Enter the directory you want to install the Eudora plugin in\n\
[c:\\eudora\\plugin]:  ",
              "c:\\eudora\\plugin");
}

void get_source_dir(char *source_dir)
{
	get_input(source_dir,
			  MAX_PATH,
			  "\n\
Enter the directory you want to copy the source files from\n\
[..\\release]:  ",
			  "..\\release");
}

int query_install_eudora(void)
{
	int DoInstall = 0;
	char tmp[16];

	get_input(tmp,
			  sizeof(tmp),
			  "\nDo you wish to install the Eudora plugin?\n[Y]/N:  ",
			  "Y");
	if(toupper(tmp[0]) == 'Y')
	{
		DoInstall = 1;
	}

	return(DoInstall);
}

int query_install_exchange(void)
{
	int DoInstall = 0;
	char tmp[16];

	get_input(tmp,
			  sizeof(tmp),
			  "\nDo you wish to install the Exchange plugin?\n[Y]/N:  ",
			  "Y");
	if(toupper(tmp[0]) == 'Y')
	{
		DoInstall = 1;
	}

	return(DoInstall);
}

void get_input(char *string,
			   int max_len,
			   char *query_message,
			   char *default_value)
{
	/*max_len is not used at this point.  Eventually, it would be nice to
	 *have this function use getch() to pull one character at a time and
	 *thus ensure that we don't overwrite the length of the string, but
	 *I don't have time to do this at this point.
	 */
	(void) max_len;

	*string = '\0';
	printf(query_message);
	gets(string);
	if(!*string)
	{
		strncpy(string, default_value, max_len);
	}
}

void perform_install(char *source_dir,
					 char *system_dir,
					 char *install_dir,
					 char *eudora_dir,
					 int do_eudora_install,
					 int do_exchange_install)
{
	char sysbuf[1024];

	printf("Source Dir = %s\n", source_dir);
	printf("System Dir = %s\n", system_dir);
	printf("Install Dir = %s\n", install_dir);
	if(do_eudora_install)
		printf("Eudora Dir = %s\n", eudora_dir);
	else
		printf("Not performing Eudora install.\n");
	if(do_exchange_install)
		printf("Installing Exchange plugin.\n");
	else
		printf("Not installing Exchange plugin.\n");
	printf("\n");

	create_dir(install_dir);

	sprintf(sysbuf, "copy \"%s\\pgpwctx.dll\" \"%s\"",
			source_dir, system_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\simple.dll\" \"%s\"",
			source_dir, system_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\bn.dll\" \"%s\"",
			source_dir, system_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\pgp.dll\" \"%s\"",
			source_dir, system_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\pgprecip.dll\" \"%s\"",
			source_dir, system_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\pgpcmdlg.dll\" \"%s\"",
			source_dir, system_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\keydb.dll\" \"%s\"",
			source_dir, system_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\pgpks.dll\" \"%s\"",
			source_dir, system_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\pgptray.exe\" \"%s\"",
			source_dir, install_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\pgpkeys.exe\" \"%s\"",
			source_dir, install_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\pgp50.hlp\" \"%s\"",
			source_dir, install_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	sprintf(sysbuf, "copy \"%s\\keygen.avi\" \"%s\"",
			source_dir, install_dir);
	printf("%s\n", sysbuf);
	system(sysbuf);
	if(do_eudora_install)
	{
		sprintf(sysbuf, "copy \"%s\\pgpplugin.dll\" \"%s\"",
				source_dir, eudora_dir);
		printf("%s\n", sysbuf);
		system(sysbuf);
	}

	if(do_exchange_install)
	{
		sprintf(sysbuf, "copy \"%s\\pgpexch.dll\" \"%s\"",
				source_dir, system_dir);
		printf("%s\n", sysbuf);
		system(sysbuf);
	}

	do_regkeys(install_dir);
}

void do_regkeys(char *install_dir)
{
	FILE *ouf;
	SHELLEXECUTEINFO *sei;
	char temp_install_dir[1024], *p1, *p2;

	p1 = install_dir;
	p2 = temp_install_dir;

	while(*p1)
	{
		if(*p1 == '\\')
		{
			*p2 = '\\';
			++p2;
		}
		*p2 = *p1;
		++p2;
		++p1;
	}
	*p2 = '\0';

	printf("Writing temporary reg file...");

	if((ouf = fopen("pgp.reg", "w")))
	{
		fprintf(ouf, "REGEDIT4\n\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\.asc]\n\
   @=\"PGP Armored Encrypted File\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\.aexpk]\n\
   @=\"PGP Armored Extracted Public Key\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\.bexpk]\n\
   @=\"PGP Binary Extracted Public Key\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\.pgp]\n\
   @=\"PGP Encrypted File\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\.prvkr]\n\
   @=\"PGP Private Keyring\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\.pubkr]\n\
   @=\"PGP Public Keyring\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\.sig]\n\
   @=\"PGP Detached Signature File\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\.pkr]\n\
   @=\"PGP Public Keyring\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\.skr]\n\
   @=\"PGP Private Keyring\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\*\\shellex\\ContextMenuHandlers\\PGP Shell Extension]\n\
   @=\"{969223c0-26aa-11d0-90ee-444553540000}\"\n");

		/*
		 * PGP File type handlers
		 */

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Encrypted File]\n\
   @=\"PGP Encrypted File\"\n\
   \"DefaultIcon\"=\"pgpwctx.dll,-104\"\n\
   \"shell\\open\\command\"=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Encrypted File\\DefaultIcon]\n\
   @=\"pgpwctx.dll,-104\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Armored Encrypted File]\n\
   @=\"PGP Armored Encrypted File\"\n\
   \"shell\\open\\command\"=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Armored Encrypted File\\DefaultIcon]\n\
   @=\"pgpwctx.dll,-103\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Armored Extracted Public Key]\n\
   @=\"PGP Armored Extracted Public Key\"\n\
   \"shell\\open\\command\"=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Armored Extracted Public Key\\DefaultIcon]\n\
   @=\"pgpwctx.dll,-107\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Binary Extracted Public Key]\n\
   @=\"PGP Binary Extracted Public Key\"\n\
   \"shell\\open\\command\"=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Binary Extracted Public Key\\DefaultIcon]\n\
   @=\"pgpwctx.dll,-107\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Detached Signature File]\n\
   @=\"PGP Detached Signature File\"\n\
   \"shell\\open\\command\"=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Detached Signature File\\DefaultIcon]\n\
   @=\"pgpwctx.dll,-102\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Private Keyring]\n\
   @=\"PGP Private Keyring\"\n\
   \"shell\\open\\command\"=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Private Keyring\\DefaultIcon]\n\
   @=\"pgpwctx.dll,-105\"\n");

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Public Keyring]\n\
   @=\"PGP Public Keyring\"\n\
   \"shell\\open\\command\"=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_CLASSES_ROOT\\PGP Public Keyring\\DefaultIcon]\n\
   @=\"pgpwctx.dll,-106\"\n");

		/*
		 * Current User keys
		 */


		fprintf(ouf, "\
[HKEY_CURRENT_USER\\Software\\PGP\\PGP\\]\n\
   @=\" \"\n\
   \"AutoGet\"=\"0\"\n\
   \"Company Name\"=\"\"\n\
   \"Encode_Flags\"=\"2\"\n\
   \"Port\"=\"11371\"\n\
   \"Serial Number\"=\"\"\n\
   \"Server\"=\"horowitz.surfnet.nl\"\n\
   \"User\"=\"\"\n\
   \"Version\"=\"for Personal Privacy ver. 5.0\"\n");

		fprintf(ouf, "\
[HKEY_CURRENT_USER\\Software\\PGP\\PGP\\PGPkeys\\]\n\
   @=\" \"\n\
   \"ExePath\"=\"%s\\\\PGPkeys.exe\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_CURRENT_USER\\Software\\PGP\\PGPlib\\]\n\
   @=\" \"\n\
   \"MailEncryptPGPMIME\"=\"false\"\n\
   \"MailSignPGPMIME\"=\"false\"\n\
   \"PubRing\"=\"%s\\\\pubring.pkr\"\n\
   \"SecRing\"=\"%s\\\\secring.skr\"\n",
   temp_install_dir, temp_install_dir);

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\CLSID\\");

		fprintf(ouf, "\
{969223c0-26aa-11d0-90ee-444553540000}\\InProcServer32\\]\n\
   @=\"pgpwctx.dll\"\n\
   \"ThreadingModel\"=\"Apartment\"\n");

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\PGP Armored Encrypted File\\");
		fprintf(ouf, "shell\\open\\command\\]\n\
   @=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\");
		fprintf(ouf, "PGP Armored Extracted Public Key\\shell\\");
		fprintf(ouf, "open\\command\\]\n\
   @=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\PGP Binary Extracted Public Key\\");
		fprintf(ouf, "shell\\open\\command\\]\n\
   @=\"%s\\\\pgptray.exe %%1\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\PGP Detached Signature File\\");
		fprintf(ouf, "shell\\open\\command\\]\n\
   @=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\PGP Encrypted File\\");
		fprintf(ouf, "shell\\open\\command\\]\n\
   @=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\PGP Private Keyring\\");
		fprintf(ouf, "shell\\open\\command\\]\n\
   @=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\PGP Public Keyring\\");
		fprintf(ouf, "shell\\open\\command\\]\n\
   @=\"%s\\\\pgptray.exe %%1\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Exchange\\Client\\Extensions\\]\n");
		fprintf(ouf, "   @\" \"\n\
   \"PGP Plugin\"=\"4.0;pgpExch.dll;1;01000111111100;1001110\"\n");

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\");
		fprintf(ouf, "App Paths\\PGPKEYS.EXE\\]\n\
   @=\"%s\\\\PGPKEYS.EXE\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\");
		fprintf(ouf, "App Paths\\PGPKEYS.EXE\\Path]\n\
   @=\"%s\"\n", temp_install_dir);

		fprintf(ouf, "\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\");
		fprintf(ouf, "\
Shell Extensions\\Approved\\]\n\
   @=\"pgpwctx.dll\"\n\
   \"{969223c0-26aa-11d0-90ee-444553540000}\"=\"Shell Extension\"\n\
[HKEY_LOCAL_MACHINE\\SOFTWARE\\PGP\\PGP\\PGPkeys\\ExePath]\n\
   @=\"%s\\\\PGPkeys.exe\"\n", temp_install_dir);

		fclose(ouf);

		printf("Complete.\n");

		sei=(SHELLEXECUTEINFO *)malloc(sizeof(SHELLEXECUTEINFO));
		memset(sei,0x00,sizeof(SHELLEXECUTEINFO));

		sei->cbSize=sizeof(SHELLEXECUTEINFO);
		sei->fMask = SEE_MASK_NOCLOSEPROCESS;
		sei->hwnd=NULL;
		sei->lpVerb=NULL;
		sei->lpFile="pgp.reg";
		sei->lpParameters=NULL;
		sei->lpDirectory=NULL;
		sei->nShow=SW_SHOWNORMAL;

		ShellExecuteEx(sei);

		if(sei->hInstApp <= (HINSTANCE) SE_ERR_DLLNOTFOUND)
		{
			if(sei->hInstApp == (HINSTANCE) SE_ERR_NOASSOC ||
			   sei->hInstApp == (HINSTANCE) SE_ERR_ASSOCINCOMPLETE)
			{
				printf("Unable to execute regedit!\n\n");
			}
		}
		else
			printf("Complete!\n\n");

		free(sei);

		printf("PGP 5.0 Installation Complete!\n");
	}
	else
	{
		printf("Unable to write pgp.reg!  Registry keys not added!\n");
	}
}

void create_dir(char *install_dir)
{
	char segment[MAX_PATH], *p, c, sysbuf[1024];

	strcpy(segment, install_dir);

	printf("Creating directory %s:\n", install_dir);

	p = segment;

	while(*p)
	{
		while(*p && *p != '\\')
			++p;

		if(p > segment && *(p -1) != ':')
		{
			c = *p;
			*p = '\0';
			printf("Creating \"%s\"\n", segment);
			sprintf(sysbuf, "mkdir \"%s\"", segment);
			system(sysbuf);
			*p = c;
			if(*p)
				++p;
		}
		else
			++p;
	}
}
