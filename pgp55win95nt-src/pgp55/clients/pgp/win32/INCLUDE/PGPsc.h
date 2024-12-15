#ifndef _PGPSCH
#define _PGPSCH

typedef struct _FILELIST
{
	char name[MAX_PATH];
	BOOL IsDirectory;
	struct _FILELIST *next;
} FILELIST;

#ifdef __cplusplus
extern "C" {
#endif

UINT PGPExport AddToFileList(FILELIST **filelist,char *filename);

UINT PGPExport FreeFileList(FILELIST *FileList);

UINT PGPExport PGPscQueryCacheSecsRemaining (void);

UINT PGPExport PGPscPreferences(HWND hwnd,void *PGPsc);

UINT PGPExport PGPscAbout (HWND hwnd);

UINT PGPExport AddKeyFileList(HWND hwnd,void *PGPsc,FILELIST *ListHead);

UINT PGPExport DecryptFileList(HWND hwnd,void *PGPsc,FILELIST *ListHead);

UINT PGPExport EncryptFileList(HWND hwnd,
					 void *PGPsc,
					 FILELIST *ListHead,
					 BOOL bEncrypt, 
					 BOOL bSign);

BOOL PGPExport FreewareWipeCheck(HWND hwnd);

UINT PGPExport WipeFileList(HWND hwnd,FILELIST *ListHead);

char PGPExport *JustFile(char *filename);

BOOL PGPExport GetValidityDrawPrefs(BOOL *pbInputBool);
BOOL PGPExport GetMarginalInvalidPref(BOOL *pbInputBool);

void PGPExport LaunchInternalViewer(HWND hwnd);

BOOL PGPExport InitPGPsc(HWND hwnd,void **PGPsc);
BOOL PGPExport UninitPGPsc(HWND hwnd,void *PGPsc);

unsigned long PGPExport DoEncrypt(HWND hwnd,void *PGPsc,BOOL Encrypt,BOOL Sign);
unsigned long PGPExport DoDecrypt(HWND hwnd,void *PGPsc);
unsigned long PGPExport DoAddKey(HWND hwnd,void *PGPsc);
unsigned long PGPExport DoLaunchKeys(HWND hwnd);
unsigned long PGPExport DoLaunchTools(HWND hwnd);

BOOL PGPExport FileWipe(char *FileName);
BOOL PGPExport ClipboardWipe(HWND hwnd);

BOOL PGPExport WindowExists (LPSTR lpszClassName, LPSTR lpszWindowTitle);

int PGPExport SendPGPlogMsg(char* szMsg);

void PGPExport PGPpath(char *szPath);

void PGPExport FreewareScreen(HINSTANCE hInstance);

void PGPExport FreewareSetParent(HWND hwnd);

#ifdef WIN32
#define DDElogServer "PGPlog"
#else
#define DDElogServer "PGPlog16"
#endif
#define DDElogTopic "Operation_Results"

#ifdef __cplusplus
}
#endif

#endif // PGPSCH