/*
 *  SecuDE Release 4.3 (GMD)
 */

/********************************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                                *
 *                                                                              *
 *                                                                              *
 *                         NOTICE                                               *
 *                                                                              *
 *    Acquisition, use, and distribution of this module                         *
 *    and related materials are subject to restrictions                         *
 *    mentioned in each volume of the documentation.                            *
 *                                                                              *
 *******************************************************************************/
#include "pem.h"

struct mailrecip
{	char *keyfield;
	int  length; };

struct mailrecip mailrecipients[] = {
"To:",3,
"Resent-To:",10,
"Reply-To:",9,
"Resent-Reply-To:",16,
"cc:",3,
"Resent-cc:",10,
NULL,0 };

#ifdef __STDC__
static RC remove_leading_blanks(char **str);
static char * search_at(char *str);
static char * search_brackets(char *str);
static char * search_string(char *str);
static RC remove_brackets(char **str);
static char *find_colon(char **str);
static void my_strncpy(char *to, char *from, int  num);
static RC aux_search_EB(OctetString *bst,int  depth,int *pos);
#else
static RC remove_leading_blanks();
static char * search_at();
static char * search_brackets();
static char * search_string();
static RC remove_brackets();
static char *find_colon();
static void my_strncpy();
static RC aux_search_EB();
#endif




/***************************************************************
 *
 * Procedure pem_scan822hdr
 *
 ***************************************************************/
/* -----------------------------------------------------------  /
   FUNCTION: 	pem_scan822hdr(header,first_recipient)
   PARAMETER:	 *OctetString	  - mail header  
 		 **SET_OF_Name	  - linked list fuer recipient-names 
   
   DESCRIPTION: Filtert aus dem header alle Strings in den
		Schluesselfeldern "to: cc: ...", die mit einem Komma
		getrennt sind, heraus.
		Dabei wird innerhalb eines derartigen Strings der
		<recpient-name> wie folgt ermittelt:
		1) 	Ist ein Teilstring in dem das Zeichen "@"
			vorkommt enthalten,
			so wird dieser als <recipient-name> uebernommen.
			(vollstaendige Internet-Adresse)
			Sonst ...
		2)	Kommt ein Teilstring vor, der durch eckige
			Klammern eingeschlossen wird "<...>",
			so wird dieser als <recipient-name> uebernommen.
			(lokaler Name)
			Sonst ...
		3)	Es wird der erste zusammenhaengende Teilstring 
			als <recipient-name> uebernommen.
			(lokaler Name)
/  ----------------------------------------------------------- */
/***************************************************************
 *
 * Procedure pem_scan822hdr
 *
 ***************************************************************/
#ifdef __STDC__

SET_OF_NAME *pem_scan822hdr(
	char	 *buffer
)

#else

SET_OF_NAME *pem_scan822hdr(
	buffer
)
char	 *buffer;

#endif

{
	char		*proc = "pem_scan822hdr";
	FILE 		*fd;
	Boolean	 	endofhead = FALSE, ready = FALSE, keyfield_found = FALSE;
	char 		*hilf, *next_string, *recipient;
	int		pos;
	SET_OF_NAME	*new, *last;
	SET_OF_NAME     *first_recipient=0;

	while ( *buffer != '\n' )
	{
		/* Ueberpruefen ob relevantes Headerfeld */
		for(pos = 0, keyfield_found = FALSE; !keyfield_found && mailrecipients[pos].keyfield; pos++)
		{
			if(strncmp(buffer, mailrecipients[pos].keyfield, mailrecipients[pos].length) == 0)
			{
				keyfield_found = TRUE;	/* Schleifenabbruch */
		
				/* Buffer ohne Feldbeschreibung zwischenspeichern */
				if((hilf = (char *) malloc(strlen(buffer)+1)) == NULL) {
					aux_add_error(EMALLOC, "hilf", CNULL, 0, proc);
					return(0);
				}
				/* Feldbeschreibung entfernen */
				recipient = (char *)index(buffer,':');
				recipient++; /* Doppelpunkt entfernen */
				strcpy(hilf, recipient);
				       					
				/* Parsen des Strings mit den Recipients -- Anlegen einer Liste  */
				for (ready=FALSE; !ready; )
				{ 
					/* String jeweils beim naechsten Komma trennen */
					if( (next_string = find_colon(&hilf)) == NULL)
					{
						if(LASTERROR == EMALLOC)
						{	AUX_ADD_ERROR;
							return(0);
						}
						if((next_string = (char *) malloc(strlen(hilf)+1)) == NULL) {
							aux_add_error(EMALLOC, "next_string", CNULL, 0, proc);
							return(0);
						}
						strcpy(next_string, hilf);
						ready = TRUE;
						free(hilf);
					}
					remove_leading_blanks(&next_string);
					if( (strlen(next_string)>=1) && ( *next_string != '\n')) 
					{
					/* Reciepient Name herausfiltern: (3 Moeglichkeiten) */
					/* Internetadresse vorhanden ? */
					if((recipient = search_at(next_string))== CNULL)
					{
						if(LASTERROR == EMALLOC)
						{	AUX_ADD_ERROR;
							return(0);
						}
						/* Lokaler Name in Klammern ? */	       	       
						if((recipient = search_brackets(next_string)) == CNULL)
						{
							if(LASTERROR == EMALLOC)
							{	AUX_ADD_ERROR;
								return(0);
							}
							/* Ersten zusammenhaengenden String waehlen */  
							if((recipient = search_string(next_string))==NULL)
							{	AUX_ADD_ERROR;
								return(0);
							}
		
						}
					}       
					free(next_string);

					/* Eventuelle Klammern entfernen */
					if(remove_brackets(&recipient) < 0) {
						AUX_ADD_ERROR;
						return(0);
					}
						
					/* Endgueltigen String in die Liste uebernehmen */
					if((new = (SET_OF_NAME *) malloc(sizeof(SET_OF_NAME))) == NULL) {
							aux_add_error(EMALLOC, "new", CNULL, 0, proc);
							return(0);
						}
					if((new->element = (NAME *) calloc(1, sizeof(NAME))) == NULL) {
							aux_add_error(EMALLOC, "new->element", CNULL, 0, proc);
							return(0);
						}
					if((new->element->alias = (char *) malloc(strlen(recipient)+1)) == NULL) {
							aux_add_error(EMALLOC, "new->element->alias", CNULL, 0, proc);
							return(0);
						}
					strcpy(new->element->alias,recipient);
					new->next = NULL;
					if(first_recipient == NULL)
					{
						first_recipient = new;
						last = new;
					}
					else
					{
						last->next	= new;
						last 		= new;
					}	
                                	free(recipient);
					}
					else
					{
						free(next_string);
						ready = TRUE;
					}
				}
			}				
		}
		while(*buffer != '\n') buffer++;
		buffer++;
	}
	if(!first_recipient)
	{
		aux_add_error(EINVALID,"No recipient found in RFC822-header",CNULL,0,proc);
	}
	return(first_recipient);
}
/***************************************************************
 *
 * Procedure my_strncpy
 *
 ***************************************************************/
#ifdef __STDC__

static void my_strncpy(
	char	 *to,
	char	 *from,
	int	  num
)

#else

static void my_strncpy(
	to,
	from,
	num
)
char	 *to;
char	 *from;
int	  num;

#endif

{
	char *proc = "my_strncpy";

	char *hilf;
	int i;
	
	strncpy(to,from,num);
	for(i = 1, hilf = to; i <= num; hilf++, i++);
	*hilf = '\0';
}

/***************************************************************
 *
 * Procedure remove_leading_blanks
 *
 ***************************************************************/
#ifdef __STDC__

static RC remove_leading_blanks(
	char	**str
)

#else

static RC remove_leading_blanks(
	str
)
char	**str;

#endif

{
	char *proc = "remove_leading_blanks";
	char *string, *result;

	for(string = *str; strncmp(string," ",1) == 0; string++);
	if((result = (char *) malloc(strlen(*str)+1)) == NULL) {
		aux_add_error(EMALLOC, "result", CNULL, 0, proc);
		return(-1);
	}

	strcpy(result, string);
	free(*str);
	*str = result;
	return(0);
}

/***************************************************************
 *
 * Procedure search_at
 *
 ***************************************************************/
#ifdef __STDC__

static char *search_at(
	char	 *str
)

#else

static char *search_at(
	str
)
char	 *str;

#endif

{
	char 	*proc = "search_at";
	Boolean at_found=FALSE;
	int 	anf, end;
	char	*string, *hilf, *result, *help;

	if((string = (char *) malloc(strlen(str)+1)) == NULL) {
		aux_add_error(EMALLOC, "string", CNULL, 0, proc);
		return(CNULL);
	}

	strcpy(string,str);	
	help = string; /* Pointer auf den Anfang des String - wird nachher fuer das "free" benoetigt */

	/* Zusammenhaengenden String der das Zeichen "@" enhaelt suchen */
	for(end=anf=0, at_found=FALSE, hilf=string; !at_found && (strcmp(hilf,"") != 0) && (strncmp(hilf, "\n", 1) != 0); hilf++, end++)
	{
		if(strncmp(hilf, " ", 1)==0)		/* nur zusammenhaengende Strings verwenden */
		{
			anf = end;
			anf++;
			string = hilf;
			string++;	
		}

		if(strncmp(hilf,"@",1)==0)		/* Internetadresse gefunden */
		{
			at_found = TRUE;
			for(; (strncmp(hilf, " ", 1) != 0) && (strcmp(hilf, "") != 0) && (strncmp(hilf, "\n", 1) != 0); hilf++, end++);
			end--;
		}		
	}
	if(!at_found)
	{
		free(help);
		return(NULL);
	}
	else
	{
		if((result = (char *) malloc(end-anf+1)) == NULL) {
			aux_add_error(EMALLOC, "result", CNULL, 0, proc);
			return(CNULL);
		}
		my_strncpy(result, string, end-anf);			
		free(help);
		return(result);
	}
}

/***************************************************************
 *
 * Procedure search_brackets
 *
 ***************************************************************/
#ifdef __STDC__

static char *search_brackets(
	char	 *str
)

#else

static char *search_brackets(
	str
)
char	 *str;

#endif

{
	char	*proc = "search_brackets";
	Boolean brl_found = FALSE, brr_found = FALSE;
	char	*string, *hilf, *result, *help;
	int 	anf, end;

	if((string = (char *) malloc(strlen(str)+1)) == NULL) {
		aux_add_error(EMALLOC, "string", CNULL, 0, proc);
		return(CNULL);
	}
	strcpy(string,str);
	help = string;

	/* Zusammenhaengenden String der die Zeichen "<.....>" enhaelt suchen */
	for(end = anf = 0, brl_found = FALSE, hilf = string ; !brl_found && (strcmp(hilf,"")!=0) && (strncmp(hilf, "\n", 1) != 0); hilf++, end++)
	{
		if(strncmp(hilf,"<",1)==0) /* Klammer gefunden */
		{
			anf = end;
			string = hilf;
			brl_found = TRUE;
			for(brr_found = FALSE; !brr_found && (strcmp(hilf,"")!=0) && (strncmp(hilf, "\n", 1) != 0) ; hilf++, end++)
			{
				if(strncmp(hilf, ">", 1)==0)
					brr_found = TRUE;
			}
		}		
	}
	if(!brr_found)
	{
		free(help);
		return(NULL);
	}
	else
	{
		if((result = (char *) malloc(end-anf+1)) == NULL) {
			aux_add_error(EMALLOC, "result", CNULL, 0, proc);
			return(CNULL);
		}
		my_strncpy(result, string, end-anf);
		free(help);
		return(result);
	}
}

/***************************************************************
 *
 * Procedure search_string
 *
 ***************************************************************/
#ifdef __STDC__

static char *search_string(
	char	 *str
)

#else

static char *search_string(
	str
)
char	 *str;

#endif

{
	char	*proc = "search_string";
	int 	anf, end;
	char	*string, *result, *hilf, *help;

	if((string = (char *) malloc(strlen(str)+1)) == NULL) {
		aux_add_error(EMALLOC, "string", CNULL, 0, proc);
		return(CNULL);
	}
	strcpy(string,str);
	help = string;

	/* Zusammenhaengenden String suchen */
	for(end = anf = 0, hilf = string; (strcmp(hilf, "")!=0) && (strncmp(hilf, "\n", 1)!=0); hilf++, end++)
	{
		if( (strncmp(hilf, " ", 1)==0) && anf != end )
		{
			if((result = (char *) malloc(end-anf+1)) == NULL) {
				aux_add_error(EMALLOC, "result", CNULL, 0, proc);
				return(CNULL);
			}
			my_strncpy(result, string, end-anf);
			free(help);
			return(result);
		}
	}
	if((result = (char *) malloc(end-anf+1)) == NULL) {
		aux_add_error(EMALLOC, "result", CNULL, 0, proc);
		return(CNULL);
	}
	my_strncpy(result, string, end-anf);
	free(help);
	return(result);	
}

/***************************************************************
 *
 * Procedure remove_brackets
 *
 ***************************************************************/
#ifdef __STDC__

static RC remove_brackets(
	char	**str
)

#else

static RC remove_brackets(
	str
)
char	**str;

#endif

{
	char 	*proc = "remove_brackets";
	char	*string, *hilf, *result, *help;
	
	string = *str;
	result = string;
	help   = string;
	if( (strncmp(string, "<", 1)==0) )
	{
		string++;
		  	
		for(hilf = string; (strncmp(hilf, ">", 1) != 0) && (strcmp(hilf, "") !=0 ) && (strncmp(hilf, "\n", 1) != 0); hilf++);
		if(strncmp(hilf,">",1) == 0)
		{	
			*hilf ='\0';
			if((result = (char *) malloc(strlen(string))) == NULL) {
				aux_add_error(EMALLOC, "result", CNULL, 0, proc);
				return(-1);
			}
			strcpy(result, string);
			free(help);
			*str = result;			
		}		
	}	
	return(0);
}


/***************************************************************
 *
 * Procedure find_colon
 *
 ***************************************************************/
#ifdef __STDC__

static char *find_colon(
	char	**str
)

#else

static char *find_colon(
	str
)
char	**str;

#endif

{
	char	*proc = "find_colon";
	char 	*string, *hilf, *result, *help;
	int	anf, end;

	if((string = (char *) malloc(strlen(*str)+1)) == NULL) {
		aux_add_error(EMALLOC, "string", CNULL, 0, proc);
		return(CNULL);
	}
	strcpy(string, *str);
	if(remove_leading_blanks(&string) < 0) {
		AUX_ADD_ERROR;
		return(CNULL);
	}
	for(anf = 0, end = 0, hilf = string; strcmp(hilf,"") !=0 && (strncmp(hilf, "\n", 1) != 0) ; hilf++, end++)
	{  
		if(strncmp(hilf, ",", 1)==0)
		{
			if((result = (char *) malloc(end-anf+1)) == NULL) {
				aux_add_error(EMALLOC, "result", CNULL, 0, proc);
				return(CNULL);
			}
			my_strncpy(result,string,end-anf);
			hilf++;	/* Komma entfernen */
			if((help = (char *) malloc(strlen(hilf)+1)) == NULL) {
				aux_add_error(EMALLOC, "help", CNULL, 0, proc);
				return(CNULL);
			}
			strcpy(help, hilf);
			free((*str));
			(*str) = help;
			free(string);
			return(result);
		}
	}
	free(string);
	return(NULL);
}



