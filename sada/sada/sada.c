/*******************************************************************************
*
* FILE:           sada.c
*
* DESCRIPTION:    SA/DA-Compiler
*
* AUTHOR:         Richard De Moliner (demoliner@isi.ee.ethz.ch)
*                 Signal and Information Processing Laboratory
*                 Swiss Federal Institute of Technology
*                 CH-8092 Zuerich, Switzerland
*
* DATE:           10.12.1994
*
* CHANGE HISTORY: 03.06.1992 created [RDM]
*                 15.01.1993 generation of RTF-files [Thomas Ernst]
*                 20.06.1994 embedding generation of RTF-files into original
*                            program, extension of syntax to support multiple
*                            laboratories, generation of `XXX_AMIV.TXT' and
*                            `XXX_TAB.RTF' [RDM]
*                 10.12.1994 new keywords: "STUDIEN_DIPLOMARBEIT",
*                            "DIPLOMARBEIT", "FUER", "STUDENTEN",
*                            "ART_DER_ARBEIT", "UEBERSCHRIFT" [RDM]
*
********************************************************************************
*
* USAGE:          sada file_name { file_name }
*
* INPUT-FILES:    file_name     ASCII-file with semester-projects in the format
*                               defined by the syntax below.
*                 DIV_HEAD.RTF  head of RTF-file 'XXX_DIV.RTF'
*                 DIV_TAIL.RTF  tail of RTF-file 'XXX_DIV.RTF'
*                 TAB_HEAD.RTF  head of RTF-file 'XXX_TAB.RTF'
*                 TAB_TAIL.RTF  tail of RTF-file 'XXX_TAB.RTF'
*                               (RTF = Rich Text Format)
*
* OUTPUT-FILES:   XXX_AMIV.TXT  ASCII-file for 'AMIV-Blitz'
*                 XXX_DIV.RTF   Microsoft Word RTF-file for division
*                 XXX_TAB.RTF   Microsoft Word RTF-file for laboratory
*
********************************************************************************

SYNTAX DEFINED IN EBNF (Extended Backus Naur Form):

  Ausschreibung = { Deklaration { [ Ueberschrift ] Arbeit } }.

  Deklaration   = Institut { Professor } { Fachgruppe { Professor } } Ende.
  Institut      = "INSTITUT_PROFESSUR" Text.
  Fachgruppe    = "FACHGRUPPE" Text.
  Professor     = "PROFESSOR" Text.

  Ueberschrift  = "UEBERSCHRIFT" Text Ende.
  Ende          = "ENDE".

  Arbeit        = ( "STUDIENARBEIT" | "STUDIEN_DIPLOMARBEIT" | "DIPLOMARBEIT" )
                    "FUER" Anzahl "STUDENTEN"
                    "TITEL" Text
                    "INHALT" Text
                  [ "ART_DER_ARBEIT" Text ]
                  [ "VORAUSSETZUNGEN" Text ]
                  [ "KONTAKTPERSON" Text ]
                    "BETREUER" Text
                    "PROFESSOR" Text
                    "ENDE".

  Text          = Wort { Wort | Zeilenumbruch }.
  Wort          = Zeichen { Zeichen }.
  Anzahl        = Ziffer [ ".." Ziffer ].
  Ziffer        = "0" .. "9".
  Zeichen       = "!" ... "~" | Sonderzeichen.
  Zeilenumbruch = "\\".
  Sonderzeichen = ( \`a | \`e | \`i | \`o | \`u | \`A | \`E | \`I | \`O | \`U
                  | \'a | \'e | \'i | \'o | \'u | \'A | \'E | \'I | \'O | \'U
                  | \^a | \^e | \^i | \^o | \^u | \^A | \^E | \^I | \^O | \^U
                  | \"a | \"e | \"i | \"o | \"u | \"A | \"E | \"I | \"O | \"U ).

********************************************************************************
*
* TRANSLATION:    German               English
*                 ----------------------------------
*                 Abteilung            division
*                 Ausschreibung        advertisement
*                 Deklaration          declaration
*                 Arbeit               project
*                 Text                 text
*                 Anzahl               Quantity
*
*                 Institut             laboratory
*                 Fachgruppe           group
*                 Ueberschrift         headline
*                 Studienarbeit        s_project
*                 Studien/Diplomarbeit sd_project
*                 Diplomarbeit         d_project
*                 Fuer                 for
*                 Studenten            students
*                 Titel                title
*                 Inhalt               contents
*                 Art der Arbeit       manner
*                 Voraussetzungen      requirements
*                 Kontaktperson        contact
*                 Betreuer             attendant
*                 Professor            professor
*                 Ende                 end
*                 Wort                 word
*                 Zeilenumbruch        new_line
*                 Dateiende            end_of_file
*
*******************************************************************************/

/******************* External Headers *****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************* Local Headers ********************************************/

/******************* Constants ************************************************/
#ifndef TRUE
#define TRUE                     1
#endif
#ifndef FALSE
#define FALSE                    0
#endif

/* general constants */
#define MAX_CHARS_IN_PROJECT   900
#define MAX_CHARS_IN_HEADLINE  100
#define MAX_GROUPS              20
#define MAX_PROFESSORS          20
#define MAX_PROJECTS            93

/* allowed character set */
#define FIRST_CHAR              '!'
#define LAST_CHAR               '~'
#define NOF_SPEC_CHARS          40
#define FIRST_SPEC_CHAR         ((char)(unsigned char)(128 + 1))
#define LAST_SPEC_CHAR          ((char)(unsigned char)(128 + NOF_SPEC_CHARS))

/* length of character arrays */
#define WORD_LEN              1000
#define STRING_LEN           65000

/* symbol table */
#define NONE                    -1
#define LABORATORY               0
#define GROUP                    1
#define HEADLINE                 2
#define S_PROJECT                3
#define SD_PROJECT               4
#define D_PROJECT                5
#define FOR                      6
#define STUDENTS                 7
#define TITLE                    8
#define CONTENTS                 9
#define MANNER                  10
#define REQUIREMENTS            11
#define CONTACT                 12
#define ATTENDANT               13
#define PROFESSOR               14
#define END                     15
#define WORD                    16
#define NEW_LINE                17
#define END_OF_FILE             18
#define NOF_SYMBOLS             19

/******************* Assertions ***********************************************/

/******************* Macros ***************************************************/
 
/******************* Types ****************************************************/

/******************* Module Data **********************************************/

const char symbol_text[NOF_SYMBOLS][24] =
{
    "INSTITUT_PROFESSUR",
    "FACHGRUPPE",
    "UEBERSCHRIFT",
    "STUDIENARBEIT",
    "STUDIEN_DIPLOMARBEIT",
    "DIPLOMARBEIT",
    "FUER",
    "STUDENTEN",
    "TITEL",
    "INHALT",
    "ART_DER_ARBEIT",
    "VORAUSSETZUNGEN",
    "KONTAKTPERSON",
    "BETREUER",
    "PROFESSOR",
    "ENDE",
    "Wort",
    "Zeilenumbruch",
    "Dateiende"
};

#define NOF_TEXTS 30

const char text[NOF_TEXTS][80] =
{
    /*  0 */ "nicht erlaubtes Sonderzeichen",
    /*  1 */ "nicht erlaubter Institutsname",
    /*  2 */ "erlaubt sind",
    /*  3 */ "Institut ist bereits deklariert",
    /*  4 */ "Fachgruppe ist bereits deklariert",
    /*  5 */ "Professor ist bereits deklariert",
    /*  6 */ "Professor ist nicht deklariert",
    /*  7 */ "zu viele Fachgruppen in Institut",
    /*  8 */ "zu viele Professoren in Institut",
    /*  9 */ "zu viele Projekte in Institut",
    /* 10 */ "Text ist zu lang",
    /* 11 */ "Wort ist zu lang",
    /* 12 */ "konnte Datei nicht erzeugen",
    /* 13 */ "konnte Datei nicht oeffnen",
    /* 14 */ "konnte Datei nicht schliessen",
    /* 15 */ "nicht genuegend Arbeitsspeicher",
    /* 16 */ "Zeile",
    /* 17 */ "Fehler",
    /* 18 */ "erwartet",
    /* 19 */ "Lese Datei",
    /* 20 */ "Generiere Datei",
    /* 21 */ "Fehler gefunden",
    /* 22 */ "sollte nicht geschehen, nicht erlaubtes Sonderzeichen",
    /* 23 */ "Beschreibung der Arbeit ist zu umfangreich",
    /* 24 */ "Warnung",
    /* 25 */ "Warnung(en) gefunden",
    /* 26 */ "verdaechtiges Wort",
    /* 27 */ "Zeichen",
    /* 28 */ "nicht erlaubte Anzahl",
    /* 29 */ "Ueberschrift ist zu lang",
};

/*
aaa \'e0 \'e1 \'e2 \'e4 AAA \'c0 \'c1 \'c2 \'c4
eee \'e8 \'e9 \'ea \'eb EEE \'c8 \'c9 \'ca \'cb
iii \'ec \'ed \'ee \'ef III \'cc \'cd \'ce \'cf
ooo \'f2 \'f3 \'f4 \'f6 OOO \'d2 \'d3 \'d4 \'d6
uuu \'f9 \'fa \'fb \'fc UUU \'d9 \'da \'db \'dc
*/

const struct
{
    char txt[4];
    char rtf[5];
}
spec_char[NOF_SPEC_CHARS] =
{
    "\\`a", "\\\'e0", "\\\'a", "\\\'e1", "\\^a", "\\\'e2", "\\\"a", "\\\'e4",
    "\\`e", "\\\'e8", "\\\'e", "\\\'e9", "\\^e", "\\\'ea", "\\\"e", "\\\'eb",
    "\\`i", "\\\'ec", "\\\'i", "\\\'ed", "\\^i", "\\\'ee", "\\\"i", "\\\'ef",
    "\\`o", "\\\'f2", "\\\'o", "\\\'f3", "\\^o", "\\\'f4", "\\\"o", "\\\'f6",
    "\\`u", "\\\'f9", "\\\'u", "\\\'fa", "\\^u", "\\\'fb", "\\\"u", "\\\'fc",
    "\\`A", "\\\'c0", "\\\'A", "\\\'c1", "\\^A", "\\\'c2", "\\\"A", "\\\'c4",
    "\\`E", "\\\'c8", "\\\'E", "\\\'c9", "\\^E", "\\\'ca", "\\\"E", "\\\'cb",
    "\\`I", "\\\'cc", "\\\'I", "\\\'cd", "\\^I", "\\\'ce", "\\\"I", "\\\'cf",
    "\\`O", "\\\'d2", "\\\'O", "\\\'d3", "\\^O", "\\\'d4", "\\\"O", "\\\'d6",
    "\\`U", "\\\'d9", "\\\'U", "\\\'da", "\\^U", "\\\'db", "\\\"U", "\\\'dc",
};

struct block_t
{
    char string[STRING_LEN];
    long string_len;
    struct block_t *prev;
};

struct professor_t
{
    char *name;
    int group_num;
};

struct project_t
{
    int type;
    int text_length;
    char *title;
    char *nof_students;
    char *contents;
    char *manner;
    char *requirements;
    char *contact;
    char *attendant;
    char *headline;
    struct professor_t *professor;
};

#define NOF_LABORATORIES 16

struct laboratory_t
{
    char short_name[4];
    char text[80];
    char name[80];
    char *group_name[MAX_GROUPS];
    struct professor_t professor[MAX_PROFESSORS];
    struct project_t project[MAX_PROJECTS];
    int nof_groups;
    int nof_professors;
    int nof_projects;
    int sort;
}
laboratory[NOF_LABORATORIES] =
{
    {"IFE", "Institut f\\\"ur Elektronik"},
    {"IQE", "Institut f\\\"ur Quantenelektronik"},
    {"IIS", "Institut f\\\"ur Integrierte Systeme"},
    {"IBT", "Institut f\\\"ur Biomedizinische Technik und Medizinische"
                            " Informatik"},
    {"PZT", "Professur f\\\"ur Zuverl\\\"assigkeitstechnik"},
    {"IFH", "Institut f\\\"ur Feldtheorie und H\\\"ochstfrequenztechnik"},
    {"IMS", "Institut f\\\"ur Milit\\\"arische Sicherheitstechnik"},
    {"IKT", "Institut f\\\"ur Kommunikationstechnik"},
    {"ISI", "Institut f\\\"ur Signal- und Informationsverarbeitung"},
    {"TIK", "Institut f\\\"ur Technische Informatik und Kommunikationsnetze"},
    {"IFA", "Institut f\\\"ur Automatik"},
    {"LEM", "Professur f\\\"ur Leistungselektronik und Messtechnik"},
    {"EEH", "Institut f\\\"ur Elektrische Energie\\\"ubertragung und"
                            " Hochspannungstechnik"},
    {"IEM", "Institut f\\\"ur Elektrische Maschinen"},
    {"EEK", "Professur f\\\"ur Elektrotechnische Entwicklungen und"
                            " Konstruktionen"},
    {"EXT", "Extern"}
};

int nof_errors = 0;
int nof_warnings = 0;
FILE *in_file = NULL;
FILE *out_file = NULL;
const char *in_file_name = NULL;
struct block_t *cur_block = NULL;
struct laboratory_t *cur_lab = NULL;

char ch;
char word[WORD_LEN];
int word_len;
int sym;
int next_sym;
int line_num;
int last_line_num;
int rtf_format;
int out_line_len;
int left_margin;
int right_margin;

/******************* Functions ************************************************/

/*******************************************************************************
*                    G E N E R A L - F U N C T I O N S
*******************************************************************************/
void Print_String(const char *str)

{   int i;
    char ch;

    if (str != NULL)
        for (i = 0; (ch = str[i]) != '\0'; i++)
            if (FIRST_SPEC_CHAR <= ch && ch <= LAST_SPEC_CHAR)
                fputs(spec_char[ch - FIRST_SPEC_CHAR].txt, stdout);
            else
                fputc(ch, stdout);
} /* Print_String */

/******************************************************************************/
void Print_Error_Warning(const char *str0, const char *str1, const char *str2)

{   printf("%s: ", str0);
    if (in_file_name != NULL)
        printf("\"%s\", %s %d: ", in_file_name, text[16], last_line_num);
    if (str1 != NULL) fputs(str1, stdout);
    if (str1 != NULL && str2 != NULL) fputs(": ", stdout);
    Print_String(str2); fputc('\n', stdout);
} /* Print_Error_Warning */

/******************************************************************************/
void Error(const char *str1, const char *str2)

{   Print_Error_Warning(text[17], str1, str2);
    nof_errors++;
} /* Error */

/******************************************************************************/
void Warning(const char *str1, const char *str2)

{   Print_Error_Warning(text[24], str1, str2);
    nof_warnings++;
} /* Warning */

/******************************************************************************/
void Init(void)

{   int lab, i, k, j;
    char txt[4];

    if ((cur_block = (struct block_t *)malloc(sizeof(struct block_t))) == NULL)
        Error(text[15], NULL);
    else
    {
        cur_block->string_len = 0;
        cur_block->prev = NULL;
    }
    txt[0] = '\\';
    txt[3] = '\0';
    for (lab = 0; lab < NOF_LABORATORIES; lab++)
    {
        j = k = 0;
        while ((ch = laboratory[lab].text[k++]) != '\0')
        {
            if (ch == '\\')
            {
                txt[1] = laboratory[lab].text[k++];
                txt[2] = laboratory[lab].text[k++];
                for (i = 0;
                     i < NOF_SPEC_CHARS && strcmp(txt, spec_char[i].txt);
                     i++);
                if (i < NOF_SPEC_CHARS)
                    ch = i + FIRST_SPEC_CHAR;
                else
                    Error(text[22], NULL);
            }
            laboratory[lab].name[j++] = ch;
        }
        laboratory[lab].name[j] = '\0';
        laboratory[lab].nof_groups = 0;
        laboratory[lab].nof_professors = 0;
        laboratory[lab].nof_projects = 0;
        laboratory[lab].sort = TRUE;
    }
} /* Init */

/******************************************************************************/
void Free(void)

{   struct block_t *prev_block;

    while (cur_block != NULL)
    {
        prev_block = cur_block->prev;
        free(cur_block);
        cur_block = prev_block;
    }
} /* Free */

/*******************************************************************************
*                    S C A N N E R   A N D   P A R S E R
*******************************************************************************/
void Get_Char(void)

{   ch = fgetc(in_file);
    if (ch == '\n') line_num++;
} /* Get_Char */

/******************************************************************************/
void Get_Sym(void)

{   char txt[4];
    int i;

    last_line_num = line_num;
    if (next_sym != NONE) { sym = next_sym; next_sym = NONE; return; }
    sym = next_sym = NONE;
    while ((ch < FIRST_CHAR || LAST_CHAR < ch) && ch != EOF) Get_Char();
    if (ch == EOF) { sym = END_OF_FILE; return; }
    word_len = 0;
    do
    {
        if (ch != '\\')
            word[word_len++] = ch;
        else
        {
            Get_Char();
            if (ch == '\\')
            {
                if (word_len == 0) { Get_Char(); sym = NEW_LINE; return; }
                else next_sym = NEW_LINE;
            }
            else
            {
                txt[0] = '\\';
                txt[1] = ch;
                Get_Char();
                txt[2] = ch;
                txt[3] = '\0';
                for (i = 0;
                     i < NOF_SPEC_CHARS && strcmp(txt, spec_char[i].txt);
                     i++);
                if (i < NOF_SPEC_CHARS)
                    word[word_len++] = i + FIRST_SPEC_CHAR;
                else
                    Error(text[0], NULL);
            }
        }
        Get_Char();
    }
    while (FIRST_CHAR <= ch && ch <= LAST_CHAR && next_sym == NONE
           && word_len < WORD_LEN);
    if (word_len < WORD_LEN)
        word[word_len] = '\0';
    else
    {
        word[WORD_LEN - 1] = '\0';
        Error(text[11], word);
    }
    for (sym = 0; sym < WORD && strcmp(word, symbol_text[sym]); sym++);
} /* Get_Sym */

/******************************************************************************/
void Skip_Garbage(void)

{   while (sym == WORD || sym == NEW_LINE) Get_Sym();
} /* Skip_Garbage */

/******************************************************************************/
void Skip_To_Sym(int symbol)

{   if (sym != symbol && sym != END_OF_FILE)
    {
        Error(symbol_text[symbol], text[18]);
        while (sym != symbol && sym != END_OF_FILE) Get_Sym();
    }
} /* Skip_To_Sym */

/******************************************************************************/
void Check_Sym(int symbol)

{   if (sym == symbol)
        Get_Sym();
    else
    {
        Error(symbol_text[symbol], text[18]);
        if (symbol != END)
            Get_Sym();
        else
        {
            while (sym != END && sym != S_PROJECT && sym != SD_PROJECT
                   && sym != D_PROJECT && sym != LABORATORY && sym != HEADLINE
                   && sym != END_OF_FILE) Get_Sym();
            if (sym == END) Get_Sym();
        }
    }
} /* Check_Sym */

/******************************************************************************/
void Check_Word(void)

{   int i, n;
    char ch;

    if (sym != WORD) return;
    n = 0;
    for (i = 0; i < word_len; i++) if ('A' <= word[i] && word[i] <= 'Z') n++;
    if (8 < n)
    {
        Warning(text[26], word);
        return;
    }
    for (i = 0; i < word_len - 1; i++)
        if ((ch = word[i]) == '!' || ch == ',' || ch == ':' || ch == ';'
            || ch == '?' || i == 0 && ch == '.' && word[i + 1] != '.'
            || i && (ch == '\"' || ch == '\'' || ch == '`' || ch == '^')
            && ('a' <= (ch = word[i + 1]) && ch <= 'z' || 'A' <= ch 
            && ch <= 'Z'))
        {
            Warning(text[26], word);
            return;
        }
    if (word_len == 1 && ((ch = word[i]) == '!' || ch == ',' || ch == ':'
        || ch == ';' || ch == '?' || ch == '.'))
    {
        Warning(text[26], word);
        return;
    }
} /* Check_Word */

/******************************************************************************/
char *Text(int support_new_line)

{   int old_nof_errors;
    char *string, seperator;
    long i, len, string_len, start_string_len;
    struct block_t *new_block;

    seperator = '\0';
    old_nof_errors = nof_errors;
    start_string_len = string_len = cur_block->string_len;
    string = cur_block->string;
    while (sym == NEW_LINE) Get_Sym();
    if (sym != WORD) Error(symbol_text[WORD], text[18]);
    while (sym == WORD || sym == NEW_LINE)
    {
        if (sym == WORD)
        {
            Check_Word();
            if (STRING_LEN - 2 <= string_len + word_len)
            {
                if (STRING_LEN - 2 <= string_len - start_string_len + word_len)
                    Error(text[10], NULL);
                else if ((new_block 
                         = (struct block_t *)malloc(sizeof(struct block_t)))
                         == NULL)
                    Error(text[15], NULL);
                else
                {
                    new_block->prev = cur_block;
                    new_block->string_len = len = 0;
                    for (i = start_string_len; i < string_len; i++)
                        new_block->string[len++] = string[i];
                    cur_block = new_block;
                    string_len = len;
                    start_string_len = 0;
                    string = cur_block->string;
                }
            }
            if (string_len + word_len < STRING_LEN - 2)
            {
                if (seperator != '\0') string[string_len++] = seperator;
                for (i = 0; i < word_len; i++) string[string_len++] = word[i];
                seperator = ' ';
            }
        }
        else if (support_new_line)
            seperator = '\n';
        else
            seperator = ' ';
        Get_Sym();
        if (nof_errors != old_nof_errors)
            while (sym == WORD || sym == NEW_LINE)
                Get_Sym();
    }
    if (string_len != start_string_len || string_len == 0)
    {
        string[string_len++] = '\0';
        cur_block->string_len = string_len;
    }
    else
        start_string_len--;
    return &string[start_string_len];
} /* Text */

/******************************************************************************/
char *Quantity(void)

{   char *r;

    r = Text(FALSE);
    if (r[0] == '\0' || r[0] <= '0' || '9' < r[0] || r[1] != '\0' 
        && (r[1] != '.' || r[2] != '.' || r[3] <= r[0] || '9' < r[3] 
            || r[4] != '\0'))
        Error(text[28], r);
    return r;
} /* Quantity */

/******************************************************************************/
struct professor_t *Professor(int insert_new_professor)

{   char *professor_name;
    int i;

    Check_Sym(PROFESSOR);
    professor_name = Text(FALSE);
    for (i = cur_lab->nof_professors - 1;
         0 <= i && strcmp(cur_lab->professor[i].name, professor_name);
         i--);
    if (insert_new_professor)
    {
        if (0 <= i)
            Error(text[5], professor_name);
        else if (MAX_PROFESSORS <= cur_lab->nof_professors)
            Error(text[8], cur_lab->name);
        else
        {
            i = cur_lab->nof_professors;
            cur_lab->professor[i].name = professor_name;
            cur_lab->professor[i].group_num = cur_lab->nof_groups - 1;
            cur_lab->nof_professors++;
        }
    }
    else if (i < 0)
        Error(text[6], professor_name);
    if (i < 0)
        return NULL;
    else
        return &cur_lab->professor[i];
} /* Professor */

/******************************************************************************/
void Group(void)

{   char *group_name;
    int i;

    Check_Sym(GROUP);
    group_name = Text(FALSE);
    for (i = cur_lab->nof_groups - 1;
         0 <= i && strcmp(cur_lab->group_name[i], group_name);
         i--);
    if (0 <= i)
        Error(text[4], group_name);
    else if (MAX_GROUPS <= cur_lab->nof_groups)
        Error(text[7], cur_lab->name);
    else
        cur_lab->group_name[cur_lab->nof_groups++] = group_name;
} /* Group */

/******************************************************************************/
void Project(void)

{   struct project_t *project;
    char str[100];
    long len;
    int type;

    if (sym == S_PROJECT || sym == SD_PROJECT || sym == D_PROJECT)
        type = sym;
    else
        type = S_PROJECT;
    Check_Sym(type);
    if (MAX_PROJECTS <= cur_lab->nof_projects)
        Error(text[9], cur_lab->name);
    else
    {
        project = &cur_lab->project[cur_lab->nof_projects++];
        project->type = type;
        Check_Sym(FOR);
        len = strlen(project->nof_students = Quantity());
        Check_Sym(STUDENTS);
        Check_Sym(TITLE);
        len += strlen(project->title = Text(FALSE));
        Check_Sym(CONTENTS);
        len += strlen(project->contents = Text(TRUE));
        if (sym == MANNER)
        {
            Get_Sym();
            len += strlen(project->manner = Text(FALSE));
        }
        else
            project->manner = NULL;
        if (sym == REQUIREMENTS)
        {
            Get_Sym();
            len += strlen(project->requirements = Text(FALSE));
        }
        else
            project->requirements = NULL;
        if (sym == CONTACT)
        {
            Get_Sym();
            len += strlen(project->contact = Text(FALSE));
        }
        else
            project->contact = NULL;
        Check_Sym(ATTENDANT);
        len += strlen(project->attendant = Text(FALSE));
        if ((project->professor = Professor(FALSE)) != NULL)
            len += strlen(project->professor->name);
        project->headline = NULL;
        if (MAX_CHARS_IN_PROJECT < len)
        {
            sprintf(str, "%ld %s", len, text[27]);
            Error(text[23], str);
        }
        project->text_length = (int)len;
        Check_Sym(END);
    }
    Skip_Garbage();
} /* Project */

/******************************************************************************/
void Headline(void)

{   char *headline;
    size_t len;

    Check_Sym(HEADLINE);
    headline = Text(FALSE);
    len = strlen(headline);
    if (MAX_CHARS_IN_HEADLINE < len)
        Error(text[29], headline);
    Check_Sym(END);
    Project();
    if (cur_lab->nof_projects)
        cur_lab->project[cur_lab->nof_projects - 1].headline = headline;
    cur_lab->sort = FALSE;
} /* Headline */

/******************************************************************************/
void Declaration(void)

{   char *name;
    int i;

    Check_Sym(LABORATORY);
    name = Text(FALSE);
    for (i = 0; i < NOF_LABORATORIES && strcmp(name, laboratory[i].name); i++);
    if (NOF_LABORATORIES <= i)
    {
        cur_lab = NULL;
        Error(text[1], text[2]);
        for (i = 0; i < NOF_LABORATORIES; i++)
        {
            Print_String(laboratory[i].name); Print_String("\n");
        }
    }
    else
    {
        cur_lab = &laboratory[i];
        if (laboratory[i].nof_groups || laboratory[i].nof_professors)
            Error(text[3], name);
        else
        {
            while (sym == PROFESSOR) Professor(TRUE);
            while (sym == GROUP)
            {
                Group();
                while (sym == PROFESSOR) Professor(TRUE);
            }
            Check_Sym(END);
        }
    }
    Skip_Garbage();
} /* Declaration */

/******************************************************************************/
void Parse_File(const char *file_name)

{   if (nof_errors) return;
    if ((in_file = fopen(file_name, "r")) == NULL)
    {
        Error(text[13], file_name);
        return;
    }
    printf("%s: \"%s\"\n", text[19], file_name);
    in_file_name = file_name;
    sym = next_sym = NONE;
    last_line_num = line_num = 1;
    Get_Char();
    Get_Sym();
    Skip_Garbage();
    Skip_To_Sym(LABORATORY);
    while (sym != END_OF_FILE)
    {
        Declaration();
        if (cur_lab != NULL)
            while (sym == S_PROJECT || sym == SD_PROJECT || sym == D_PROJECT 
                   || sym == HEADLINE)
                if (sym == HEADLINE)
                    Headline();
                else
                    Project();
        Skip_To_Sym(LABORATORY);
    }
    in_file_name = NULL;
    if (fclose(in_file))
        Error(text[14], file_name);
    in_file = NULL;
} /* Parse_File */

/*******************************************************************************
*                    C O D E - G E N E R A T O R
*******************************************************************************/
void Put_Ln(void)

{   if (rtf_format) fputs("\\par", out_file);
    fputc('\n', out_file);
    out_line_len = 0;
} /* Put_Ln */

/******************************************************************************/
void Put_Word(void)

{   word[word_len] = '\0';
    if (right_margin && right_margin <= out_line_len + word_len)
    {
        fputc('\n', out_file);
        out_line_len = 0;
    }
    while (out_line_len < left_margin)
    {
        fputc(' ', out_file);
        out_line_len++;
    }
    if (out_line_len != left_margin)
    {
        fputc(' ', out_file);
        out_line_len++;
    }
    fputs(word, out_file);
    out_line_len += word_len;
    word_len = 0;
} /* Put_Word */

/******************************************************************************/
void Put_Str(const char *str)

{   int i, k;
    char ch, h;

    if (str == NULL || str[0] == '\0') return;
    word_len = 0;
    for (i = 0; (ch = str[i]) != '\0'; i++)
        if (FIRST_CHAR <= ch && ch <= LAST_CHAR)
            word[word_len++] = ch;
        else if (ch == ' ' && word_len)
            Put_Word();
        else if (ch == '\n')
        {
            if (word_len) Put_Word();
            if (!rtf_format) Put_Str("\\\\");
            Put_Ln();
        }
        else if (ch < FIRST_SPEC_CHAR || LAST_SPEC_CHAR < ch)
            Error(text[22], NULL);
        else if (rtf_format)
            for (k = 0; (h = spec_char[ch-FIRST_SPEC_CHAR].rtf[k]) != '\0'; k++)
                word[word_len++] = h;
        else
            for (k = 0; (h = spec_char[ch-FIRST_SPEC_CHAR].txt[k]) != '\0'; k++)
                word[word_len++] = h;
    if (word_len) Put_Word();
} /* Put_Str */

/******************************************************************************/
void Put_File(const char *file_name)

{   FILE *file;
    int ch;

    if ((file = fopen(file_name, "r")) == NULL)
        Error(text[13], file_name);
    else
    {
        while ((ch = fgetc(file)) != EOF)
            if (ch != '\r')
                fputc(ch, out_file);
        if (fclose(file))
            Error(text[14], file_name);
    }
} /* Put_File */

/******************************************************************************/
void Get_Project_Id(const struct project_t *project, int *proj_num,
                    char *proj_id)

{   sprintf(proj_id, "%s%02d, ", cur_lab->short_name, ++(*proj_num));
    if (project->type == S_PROJECT)
        strcat(proj_id, "SA, ");
    else if (project->type  == SD_PROJECT)
        strcat(proj_id, "SA/DA, ");
    else if (project->type == D_PROJECT)
        strcat(proj_id, "DA, ");
    strcat(proj_id, project->nof_students);
    strcat(proj_id, ":");
} /* Get_Project_Id */

/******************************************************************************/
void Generate_Code(const char *out_name,
                   void (*Put_Laboratory)(const char *, const char *),
                   void (*Put_Group)(const char *, const char *),
                   void (*Put_Headline)(const char *),
                   void (*Put_Project)(const struct project_t *, const char *))

{   char str[WORD_LEN];
    int lab, grp, prof, proj, proj_num, type, i;

    if (nof_errors) return;
    i = -1;
    for (lab = 0; lab < NOF_LABORATORIES; lab++)
        if (laboratory[lab].nof_groups || laboratory[lab].nof_professors)
            if (i == -1) i = lab; else i = -2;
    sprintf(str, "%s_%s.%s", 
            0 <= i ? laboratory[i].short_name : "ALL",
            out_name,
            rtf_format ? "RTF" : "TXT");
    if ((out_file = fopen(str, "w")) == NULL)
    {
        Error(text[12], str);
        return;
    }
    printf("%s: \"%s\"\n", text[20], str);
    left_margin = 0;
    right_margin = 0;
    out_line_len = 0;
    if (rtf_format)
    {
        sprintf(str, "%s_HEAD.RTF", out_name);
        Put_File(str);
    }
    for (lab = 0; lab < NOF_LABORATORIES; lab++)
        if (laboratory[lab].nof_groups || laboratory[lab].nof_professors)
        {
            cur_lab = &laboratory[lab];
            proj_num = 0;
            for (grp = -1; grp < cur_lab->nof_groups; grp++)
            {
                i = 0;
                for (prof = 0; prof < cur_lab->nof_professors; prof++)
                    if (cur_lab->professor[prof].group_num == grp)
                        i++;
                if (i == 0)
                    str[0] = '\0';
                else
                {
                    if (i == 1)
                        strcpy(str, "Prof. ");
                    else
                        strcpy(str, "Proff. ");
                    i = 0;
                    for (prof = 0; prof < cur_lab->nof_professors; prof++)
                        if (cur_lab->professor[prof].group_num == grp)
                        {
                          if (i) strcat(str, ", "); else i = 1;
                          strcat(str, cur_lab->professor[prof].name);
                        }
                }
                if (grp < 0 && Put_Laboratory != NULL)
                    (*Put_Laboratory)(cur_lab->name, str);
                else if (0 <= grp && Put_Group != NULL)
                    (*Put_Group)(cur_lab->group_name[grp], str);
                if (cur_lab->sort)
                {
                    for (prof = 0; prof < cur_lab->nof_professors; prof++)
                        if (cur_lab->professor[prof].group_num == grp)
                            for (type = S_PROJECT; type <= D_PROJECT; type++)
                                for (proj = 0; 
                                     proj < cur_lab->nof_projects;
                                     proj++)
                                    if (cur_lab->project[proj].professor
                                        == &cur_lab->professor[prof]
                                        && cur_lab->project[proj].type == type
                                        && Put_Project != NULL)
                                    {
                                        Get_Project_Id(&cur_lab->project[proj],
                                                       &proj_num, str);
                                        (*Put_Project)(&cur_lab->project[proj],
                                                       str);
                                    }
                }
                else
                {
                    for (proj = 0; proj < cur_lab->nof_projects; proj++)
                        if (cur_lab->project[proj].professor->group_num == grp)
                        {
                            if (cur_lab->project[proj].headline != NULL
                                && Put_Headline != NULL)
                               (*Put_Headline)(cur_lab->project[proj].headline);
                            if (Put_Project != NULL)
                            {
                                Get_Project_Id(&cur_lab->project[proj],
                                               &proj_num, str);
                                (*Put_Project)(&cur_lab->project[proj], str);
                            }
                        }
                }
            }
        }
    if (rtf_format)
    {
        sprintf(str, "%s_TAIL.RTF", out_name);
        Put_File(str);
    }
    if (fclose(out_file))
        Error(text[14], out_name);
    out_file = NULL;
} /* Generate_Code */

/******************************************************************************/
void Put_Amiv_Laboratory(const char *laboratory_name, const char *prof_names)

{   char str[WORD_LEN];
    int grp, prof;

    right_margin = 80;
    for (grp = -1; grp < cur_lab->nof_groups; grp++)
    {
        if (grp < 0)
        {
            left_margin = 0; Put_Str(symbol_text[LABORATORY]);
            left_margin = 4; Put_Str(cur_lab->name); Put_Ln();
        }
        else
        {
            left_margin = 0; Put_Str(symbol_text[GROUP]);
            left_margin = 4; Put_Str(cur_lab->group_name[grp]); Put_Ln();
        }
        for (prof = 0; prof < cur_lab->nof_professors; prof++)
            if (cur_lab->professor[prof].group_num == grp)
            {
                left_margin = 2; Put_Str(symbol_text[PROFESSOR]);
                left_margin = 4; Put_Str(cur_lab->professor[prof].name);
                Put_Ln();
            }
    }
    left_margin = 0; Put_Str(symbol_text[END]);
    sprintf(str, "(%s: %d Arbeiten)", cur_lab->short_name,
                    cur_lab->nof_projects);
    Put_Str(str); Put_Ln(); Put_Ln();
} /* Put_Amiv_Laboratory */

/******************************************************************************/
void Put_Amiv_Group(const char *group_name, const char *prof_names)

{   left_margin = 0;
    Put_Str("Fachgruppe"); Put_Str(group_name); Put_Ln(); Put_Ln();
} /* Put_Amiv_Group */

/******************************************************************************/
void Put_Amiv_Headline(const char *headline)

{   left_margin = 0; Put_Str(symbol_text[HEADLINE]);
    left_margin = 4; Put_Str(headline); Put_Ln();
    left_margin = 0; Put_Str(symbol_text[END]); Put_Ln(); Put_Ln();
} /* Put_Amiv_Headline */

/******************************************************************************/
void Put_Amiv_Project(const struct project_t *project, const char *proj_id)

{   char str[WORD_LEN];

    left_margin = 0; Put_Str(symbol_text[project->type]);
                     Put_Str(symbol_text[FOR]);
                     Put_Str(project->nof_students);
                     Put_Str(symbol_text[STUDENTS]); Put_Ln();
    left_margin = 2; Put_Str(symbol_text[TITLE]);
    left_margin = 4; Put_Str(project->title); Put_Ln();
    left_margin = 2; Put_Str(symbol_text[CONTENTS]);
    left_margin = 4; Put_Str(project->contents); Put_Ln();
    if (project->manner != NULL)
    {
        left_margin = 2; Put_Str(symbol_text[MANNER]);
        left_margin = 4; Put_Str(project->manner); Put_Ln();
    }
    if (project->requirements)
    {
        left_margin = 2; Put_Str(symbol_text[REQUIREMENTS]);
        left_margin = 4; Put_Str(project->requirements); Put_Ln();
    }
    if (project->contact != NULL)
    {
        left_margin = 2; Put_Str(symbol_text[CONTACT]);
        left_margin = 4; Put_Str(project->contact); Put_Ln();
    }
    left_margin = 2; Put_Str(symbol_text[ATTENDANT]);
    left_margin = 4; Put_Str(project->attendant); Put_Ln();
    left_margin = 2; Put_Str(symbol_text[PROFESSOR]);
    left_margin = 4; Put_Str(project->professor->name); Put_Ln();
    left_margin = 0; Put_Str(symbol_text[END]);
    sprintf(str, "(%s %d Zeichen)", proj_id, project->text_length);
    Put_Str(str); Put_Ln(); Put_Ln();
} /* Put_Amiv_Project */

/******************************************************************************/
void Put_Dep_Laboratory(const char *laboratory_name, const char *prof_names)

{   Put_Str("\\pard\\plain \\s254\\qc\\sb360 \\b\\f4\\fs32");
    Put_Str(laboratory_name); Put_Ln();
    if (prof_names[0] != '\0')
    {
        Put_Str(prof_names); Put_Ln();
    }
} /* Put_Dep_Laboratory */

/******************************************************************************/
void Put_Dep_Group(const char *group_name, const char *prof_names)

{   Put_Str("\\pard\\plain \\s253\\qc\\sb360 \\b\\f4\\fs28");
    Put_Str("Fachgruppe"); Put_Str(group_name); Put_Ln();
    if (prof_names[0] != '\0')
    {
        Put_Str("\\pard\\plain \\s2\\qc \\f4");
        Put_Str(prof_names); Put_Ln();
    }
} /* Put_Dep_Group */

/******************************************************************************/
void Put_Dep_Headline(const char *headline)

{   Put_Str("\\pard\\plain \\s252\\qj\\sb360 \\b\\f5");
    Put_Str(headline); Put_Ln();
} /* Put_Dep_Headline */

/******************************************************************************/
void Put_Dep_Project(const struct project_t *project, const char *proj_id)

{   char str[WORD_LEN];

    Put_Str("\\pard\\plain \\s251\\qj\\sb240 \\b\\f5\\fs20");
    Put_Str(proj_id);
    Put_Str(project->title); Put_Ln();
    Put_Str("\\pard\\plain \\qj \\f4\\fs20");
    Put_Str(project->contents); Put_Ln();
    if (project->manner != NULL)
    {
        Put_Str("Art der Arbeit:"); Put_Str(project->manner); Put_Ln();
    }
    if (project->requirements != NULL)
    {
        Put_Str("Voraussetzungen:"); Put_Str(project->requirements); Put_Ln();
    }
    if (project->contact != NULL)
    {
        Put_Str("Kontaktperson:"); Put_Str(project->contact);
    }
    Put_Str("Betreuer:"); Put_Str(project->attendant);
    sprintf(str, "(Prof. %s)", project->professor->name);
    Put_Str(str); Put_Ln();
} /* Put_Dep_Project */

/******************************************************************************/
void Put_Tab_Project(const struct project_t *project, const char *proj_id)

{   Put_Str("\\pard \\intbl");
    Put_Str(proj_id);
    Put_Ln();
    Put_Str("\\cell {\\b");
    Put_Str(project->title);
    Put_Str("} \\cell");
    Put_Str(project->contents);
    Put_Str("\\cell");
    if (project->manner != NULL)
        Put_Str(project->manner);
    Put_Str("\\cell");
    if (project->requirements != NULL)
        Put_Str(project->requirements);
    Put_Str("\\cell");
    if (project->contact != NULL)
        Put_Str(project->contact);
    Put_Str("\\cell");
    Put_Str(project->attendant);
    Put_Str("\\cell");
    Put_Str(project->professor->name);
    Put_Str("\\cell \\cell \\cell \\cell \\cell \\cell \\cell");
    Put_Str("\\cell \\cell \\cell \\cell \\row");
} /* Put_Tab_Project */

/*******************************************************************************
*                    M A I N
*******************************************************************************/
int main (int argc, char *argv[])

{   int i;

    printf("SA/DA-Compiler, Version 1.1\n");
    if (argc < 2)
    {
        printf("\n*** Usage: sada file_name { file_name }\n\n");
        return 1;
    }
    Init();
    for (i = 1; i < argc; i++)
        Parse_File(argv[i]);
    rtf_format = FALSE;
    Generate_Code("AMIV", Put_Amiv_Laboratory, Put_Amiv_Group,
                          Put_Amiv_Headline, Put_Amiv_Project);
    rtf_format = TRUE;
    Generate_Code("DIV", Put_Dep_Laboratory, Put_Dep_Group,
                         Put_Dep_Headline, Put_Dep_Project);
    Generate_Code("TAB", NULL, NULL, NULL, Put_Tab_Project);
    Free();
    if (nof_errors)
        printf("\n*** %d %s", nof_errors, text[21]);
    if (nof_warnings)
        printf("\n*** %d %s", nof_warnings, text[25]);
    if (nof_errors || nof_warnings)
        printf("\n");
    return nof_errors != 0;
} /* main */

/******************************************************************************/
