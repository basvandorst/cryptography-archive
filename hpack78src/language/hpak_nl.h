/****************************************************************************
*																			*
*							HPACK Multi-System Archiver						*
*							===========================						*
*																			*
*						  HPACK Messages Symbolic Defines					*
*							HPAKTEXT.H  Updated 29/05/92					*
*																			*
* This program is protected by copyright and as such any use or copying of	*
*  this code for your own purposes directly or indirectly is highly uncool	*
*  					and if you do so there will be....trubble.				*
* 				And remember: We know where your kids go to school.			*
*																			*
*		Copyright 1991 - 1992  Peter C.Gutmann.  All rights reserved		*
*																			*
****************************************************************************/

/* The following two procedures display the title message and help screen */

void showTitle( void );
void showHelp( void );

/* Below are the interface declarations for the messages in HPAKTEXT.C.
   Note that the message tags in these two files are automatically generated
   by a preprocessor from a text definition file and should not be relied
   upon to be consistent from one version to another - they serve merely as
   tags for the messages */

/****************************************************************************
*																			*
*									Warnings								*
*																			*
****************************************************************************/

#define RESPONSE_YES	'J'
#define RESPONSE_NO		'N'
#define RESPONSE_ALL	'A'

extern char warn00[], warn01[], warn02[], warn03[], warn04[], warn05[];
extern char	warn06[], warn07[], warn08[], warn09[], warn10[], warn11[];

#define WARN_TRUNCATED_u_BYTES_EOF_PADDING		warn00
#define WARN_s_CONTINUE_YN						warn01
#define WARN_FILE_PROBABLY_CORRUPTED			warn02
#define WARN_CANT_FIND_PUBLIC_KEY				warn03
#define WARN_SEC_INFO_CORRUPTED					warn04
#define WARN_ARCHIVE_SECTION_TOO_SHORT			warn05
#define WARN_UNKNOWN_SCRIPT_COMMAND_s			warn06
#define WARN_CANNOT_OPEN_DATAFILE_s				warn07
#define WARN_CANNOT_OPEN_SCRIPTFILE_s			warn08
#define WARN_DIRNAME_s_CONFLICTS_WITH_FILE		warn09
#define WARN_FILE_CORRUPTED						warn10
#define WARN_d_FILES_CORRUPTED					warn11

/****************************************************************************
*																			*
*								General Messages							*
*																			*
****************************************************************************/

extern char mesg00[], mesg01[], mesg02[], mesg03[], mesg04[], mesg05[];
extern char mesg06[], mesg07[], mesg08[], mesg09[], mesg10[], mesg11[];
extern char mesg12[], mesg13[], mesg14[], mesg15[], mesg16[], mesg17[];
extern char mesg18[], mesg19[], mesg20[], mesg21[], mesg22[], mesg23[];
extern char mesg24[], mesg25[], mesg26[], mesg27[], mesg28[], mesg29[];
extern char mesg30[], mesg31[], mesg32[], mesg33[], mesg34[], mesg35[];
extern char mesg36[], mesg37[], mesg38[], mesg39[], mesg40[], mesg41[];
extern char mesg42[], mesg43[], mesg44[], mesg45[], mesg46[], mesg47[];
extern char mesg48[], mesg49[], mesg50[], mesg51[], mesg52[], mesg53[];
extern char mesg54[], mesg55[], mesg56[], mesg57[], mesg58[], mesg59[];
extern char mesg60[], mesg61[], mesg62[], mesg63[], mesg64[], mesg65[];
extern char mesg66[], mesg67[], mesg68[], mesg69[], mesg70[], mesg71[];

#define MESG_VERIFY_SECURITY_INFO				mesg00
#define MESG_AUTH_CHECK_FAILED					mesg01
#define MESG_VERIFYING_ARCHIVE_AUTHENTICITY		mesg02
#define MESG_SECURITY_INFO_WILL_BE_DESTROYED	mesg03
#define MESG_ARCHIVE_DIRECTORY_CORRUPTED		mesg04
#define MESG_ARCHIVE_DIRECTORY_WRONG_PASSWORD	mesg05
#define MESG_PROCESSING_ARCHIVE_DIRECTORY		mesg06
#define MESG_s_YN								mesg07
#define MESG_s_s_YNA							mesg08
#define MESG_SKIPPING_s							mesg09
#define MESG_DATA_IS_ENCRYPTED					mesg10
#define MESG_CANNOT_PROCESS_ENCR_INFO			mesg11
#define MESG_CANNOT_OPEN_DATAFILE				mesg12
#define MESG_EXTRACTING							mesg13
#define MESG_s_AS								mesg14
#define MESG_AS_s								mesg15
#define MESG_UNKNOWN_ARCHIVING_METHOD			mesg16
#define MESG__SKIPPING_s						mesg17
#define MESG_WONT_OVERWRITE_EXISTING_s			mesg18
#define MESG_s_ALREADY_EXISTS_ENTER_NEW_NAME	mesg19
#define MESG_PATH_s__TOO_LONG					mesg20
#define MESG_ALREADY_EXISTS__OVERWRITE			mesg21
#define MESG_EXTRACT							mesg22
#define MESG_DISPLAY							mesg23
#define MESG_TEST								mesg24
#define MESG_FILE_TESTED_OK						mesg25
#define MESG_HIT_A_KEY							mesg26
#define MESG_ADDING								mesg27
#define MESG_ERROR								mesg28
#define MESG_ERROR_DURING_ERROR_RECOVERY		mesg29
#define MESG_CREATING_DIRECTORY_s				mesg30
#define MESG_PROCESSING_SCRIPTFILE_s			mesg31
#define MESG_PATH_s__TOO_LONG_LINE_d			mesg32
#define MESG_BAD_CHAR_IN_FILENAME_LINE_d		mesg33
#define MESG_MAXIMUM_LEVEL_OF					mesg34
#define MESG_ADDING_DIRECTORY_s					mesg35
#define MESG_CHECKING_DIRECTORY_s				mesg36
#define MESG_LEAVING_DIRECTORY_s				mesg37
#define MESG_FILE_s_ALREADY_IN_ARCH__SKIPPING	mesg38
#define MESG_ADD								mesg39
#define MESG_DELETE								mesg40
#define MESG_DELETING_s_FROM_ARCHIVE			mesg41
#define MESG_FRESHEN							mesg42
#define MESG_REPLACE							mesg43
#define	MESG_ARCHIVE_IS_s						mesg44
#define MESG_ARCH_IS_UPTODATE					mesg45
#define MESG_DONE								mesg46
#define MESG_DIRECTORY							mesg47
#define MESG_DIRECTORY_TIME						mesg48
#define MESG_SINGULAR_FILES						mesg49
#define MESG_PLURAL_FILES						mesg50
#define MESG_KEY_INCORRECT_LENGTH				mesg51
#define MESG_ENTER_PASSWORD						mesg52
#define MESG_ENTER_SEC_PASSWORD					mesg53
#define MESG_REENTER_TO_CONFIRM					mesg54
#define MESG_ENTER_SECKEY_PASSWORD				mesg55
#define MESG_PASSWORD_INCORRECT					mesg56
#define MESG_BAD_SIGNATURE						mesg57
#define MESG_GOOD_SIGNATURE						mesg58
#define MESG_SIGNATURE_FROM_s_DATE_dddddd		mesg59
#define MESG_WAIT								mesg60
#define MESG_PART_d_OF_MULTIPART_ARCHIVE		mesg61
#define MESG_PLEASE_INSERT_THE					mesg62
#define MESG_NEXT_DISK							mesg63
#define MESG_PREV_DISK							mesg64
#define MESG_DISK_CONTAINING					mesg65
#define MESG_PART_d								mesg66
#define MESG_THE_LAST_PART						mesg67
#define MESG_OF_THIS_ARCHIVE					mesg68
#define MESG_AND_PRESS_A_KEY					mesg69
#define MESG_CONTINUING							mesg70

/****************************************************************************
*																			*
*					Special Text Strings used in Viewfile.C					*
*																			*
****************************************************************************/

extern char vmsg00[], vmsg01[], vmsg02[], vmsg03[], vmsg04[], vmsg05[];

#define VIEWFILE_TITLE							vmsg00
#define VIEWFILE_SUBDIRECTORY					vmsg01
#define VIEWFILE_MAIN_DISPLAY					vmsg02
#define VIEWFILE_TRAILER						vmsg03
#define VIEWFILE_GRAND_TOTAL					vmsg04

/****************************************************************************
*																			*
*								OS-Specific Messages						*
*																			*
****************************************************************************/

#ifdef __MSDOS__

extern char osmsg00[], osmsg01[], osmsg02[], osmsg03[], osmsg04[], osmsg05[];

#define OSMESG_FILE_SHARING_VIOLATION			osmsg00
#define OSMESG_FILE_LOCKING_VIOLATION			osmsg01
#define OSMESG_FILE_IS_DEVICEDRVR				osmsg02

#endif /* __MSDOS__ */
