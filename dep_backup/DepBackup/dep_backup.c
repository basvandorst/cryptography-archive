/******************************************************************************/
/*                                                                            */
/*      E N C R Y P T E D    B A C K U P    T O    D E P A R T M E N T        */
/*                                                                            */
/******************************************************************************/
/* Author:       Richard De Moliner (demoliner@isi.ee.ethz.ch)                */
/*               Signal and Information Processing Laboratory                 */
/*               Swiss Federal Institute of Technology                        */
/*               CH-8092 Zuerich, Switzerland                                 */
/* Created:      March 8, 1994                                                */
/* System:       SUN SPARCstation, SUN acc ANSI-C-Compiler, SUN-OS 4.1.3      */
/******************************************************************************/
/* Change this definitions:                                                   */

#ifdef SHOWPASSWORD
#define ftpPassword  "mypasswd"
#endif
#define ftpUser      "isibup"
#define ftpCommand   "/usr/ucb/ftp -in roseg 1037"

#define SET_MASTER_KEY                                            \
  uk[0] = 0x0000; uk[1] = 0x0001; uk[2] = 0x0002; uk[3] = 0x0003; \
  uk[4] = 0x0004; uk[5] = 0x0005; uk[6] = 0x0006; uk[7] = 0x0007;

#define SET_ENCRYPTED_FTP_PASSWORD                                \
  da[0] = 0x6FEB; da[1] = 0xA4FB; da[2] = 0x4C1C; da[3] = 0xE7C0;

/******************************************************************************/
/* Do not change the lines below.                                             */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "idea.c"

#define maxInterleave                   2  /* maximal interleave factor + 1   */
#define stringLen                    1024  /* long enough to hold all strings */
#define maxBufLen   (1024 * Idea_dataSize) /* size of input and output buffer */
#define passwordLen (1 + 2 * Idea_dataLen) /* maximal password len is 8 char  */

static Idea_Key  key;                   /* expanded key with 832 bits         */
static Idea_Data state[maxInterleave];  /* state informations for interleaving*/
static u_int32   inputLen  = 0;         /* current number of bytes read stdin */
static int       intTime   = 0;         /* time for interleaving mode         */
static int       inBufLen  = maxBufLen; /* current length of data in 'inBuf'  */
static int       inBufPos  = maxBufLen; /* current read position of 'inBuf'   */
static int       outBufLen = 0;         /* current write position of 'outBuf' */
static u_int8    inBuf[maxBufLen];      /* buffer for file read               */
static u_int8    outBuf[maxBufLen];     /* buffer for file write              */
static char      password[passwordLen]; /* password for remote machine        */
static char      **environment;         /* pointer to environment of program  */

/******************************************************************************/

#ifdef ANSI_C
  static void UnInit(void)
#else
  UnInit()
#endif

{ int i, k;

  for (i = 0; i < Idea_keyLen; i++) key[i] = 0;
  for (i = 0; i < passwordLen; i++) password[i] = '\0';
  for (i = 0; i < maxInterleave; i++)
    for (k = 0; k < Idea_dataLen; k++) state[i][k] = 0;
} /* UnInit */

/******************************************************************************/
/*                          E R R O R - H A N D L I N G                       */
/******************************************************************************/
/* write usage error message and terminate program                            */

#ifdef ANSI_C
  static void UsageError(void)
#else
  UsageError()
#endif

{ fprintf(stderr, "usage error:\n\n\
Usage:   dep_backup ( -e | -d | -c { command } |                \n\
                      -dump dumpLevel dumpDevice fileName |     \n\
                      -restore fileName )                       \n\n\
Example: dep_backup -restore backup/940307.0100-isidor.sd8c.home-L1\n");
  UnInit();
  exit(1);
} /* UsageError */

/******************************************************************************/
/* write error message and terminate program                                  */

#ifdef ANSI_C
  static void Error(char *str)
#else
  Error(str)
  char *str;
#endif

{  fprintf(stderr, "error: %s\n", str); UnInit(); exit(1); } /* Error */

/******************************************************************************/
/*                          I N I T I A L I Z A T I O N                       */
/******************************************************************************/
/* initialize global variables                                                */

#ifdef ANSI_C
  static void Init(void)
#else
  Init()
#endif

{ int i, k;
  Idea_Data da;
  Idea_UserKey uk;

  for (i = 0; i < Idea_keyLen; i++) key[i] = 0;
  for (i = 0; i < Idea_dataLen; i++) da[i] = 0;
  for (i = 0; i < Idea_userKeyLen; i++) uk[i] = 0;
  for (i = 0; i < passwordLen; i++) password[i] = '\0';
  for (i = 0; i < maxInterleave; i++)
    for (k = 0; k < Idea_dataLen; k++) state[i][k] = 0;

  SET_MASTER_KEY
  SET_ENCRYPTED_FTP_PASSWORD

  Idea_ExpandUserKey(uk, key);

#ifdef SHOWPASSWORD
  printf("\n  idea -K ");
  for (i = 0; i < Idea_userKeyLen; i++) printf("%04X", uk[i]);
  printf("\n");
  Idea_InvertKey(key, key);
  strcpy(password, ftpPassword);
  for (i = 0; i < Idea_dataLen; i++) 
    da[i] = (u_int16)(password[2 * i] & 0xFF) |
            (u_int16)(password[2 * i + 1] & 0xFF) << 8;
  Idea_Crypt(da, da, key);
  printf(" ");
  for (i = 0; i < Idea_dataLen; i++) printf(" da[%d] = 0x%04X;", i, da[i]);
  printf("\n\n");
  for (i = 0; i < Idea_userKeyLen; i++) uk[i] = 0;
  for (i = 0; i < Idea_dataLen; i++) da[i] = 0;
  UnInit();
  exit(0);
#endif

  Idea_Crypt(da, da, key);
  for (i = 0; i < Idea_dataLen; i++) {
    password[2 * i] = (char)(da[i] & 0xFF);
    password[2 * i + 1] = (char)(da[i] >> 8 & 0xFF);
  }
  for (i = 0; i < Idea_userKeyLen; i++) uk[i] = 0;
  for (i = 0; i < Idea_dataLen; i++) da[i] = 0;
} /* Init */

/******************************************************************************/
/*                          D E C R Y P T I O N  /  E N C R Y P T I O N       */
/******************************************************************************/
/* read one data-block from 'stdin'                                          */

#ifdef ANSI_C
  static int GetData(Idea_Data data)
#else
  int GetData(data)
  Idea_Data data;
#endif

{ register int i, len;
  register u_int16 h;
  register u_int8 *inPtr;

  if (inBufPos >= inBufLen) {
    if (inBufLen != maxBufLen) return 0;
    inBufLen = fread(inBuf, 1, maxBufLen, stdin);
    inBufPos = 0;
    if (inBufLen == 0) return 0;
    if (inBufLen % Idea_dataSize != 0)
      for (i = inBufLen; i % Idea_dataSize != 0; i++) inBuf[i] = 0;
  }
  inPtr = &inBuf[inBufPos];
  for (i = 0; i < Idea_dataLen; i++) {
    h = ((u_int16)*inPtr++ & 0xFF) << 8;
    data[i] = h | (u_int16)*inPtr++ & 0xFF;
  }
  inBufPos += Idea_dataSize;
  if (inBufPos <= inBufLen) len = Idea_dataSize;
  else len = inBufLen + Idea_dataSize - inBufPos;
  inputLen += len;
  return len;
} /* GetData */

/******************************************************************************/
/* write one data-block to 'stdout'                                           */

#ifdef ANSI_C
  static void PutData(Idea_Data data, int len)
#else
  PutData(data, len)
  Idea_Data data;
  int len;
#endif

{ register int i;
  register u_int16 h;
  register u_int8 *outPtr;

  outPtr = &outBuf[outBufLen];
  for (i = 0; i < Idea_dataLen; i++) {
    h = data[i];
    *outPtr++ = h >> 8 & 0xFF;
    *outPtr++ = h & 0xFF;
  }
  outBufLen += len;
  if (outBufLen >= maxBufLen) {
    if (fwrite(outBuf, maxBufLen, 1, stdout) != 1)
      Error("crypt: could not write result (1)");
    outBufLen = 0;
  }
} /* PutData */

/******************************************************************************/
/* write last block to 'stdout' and close 'stdout'                            */

#ifdef ANSI_C
  static void CloseOutput(void)
#else
  CloseOutput()
#endif

{ if (outBufLen > 0) {
    if (fwrite(outBuf, outBufLen, 1, stdout) != 1)
      Error("crypt: could not write result (2)");
    outBufLen = 0;
  }
  fclose(stdout);
} /* CloseOutput */

/******************************************************************************/
/* encrypt one data-block                                                     */

#ifdef ANSI_C
  static void EncryptData(Idea_Data data)
#else
  EncryptData(data)
  Idea_Data data;
#endif

{ int i;

  for (i = Idea_dataLen - 1; i >= 0; i--) data[i] ^= state[intTime][i];
  Idea_Crypt(data, data, key);
  intTime ^= 1;
  for (i = Idea_dataLen - 1; i >= 0; i--) state[intTime][i] = data[i];
} /* EncryptData */

/******************************************************************************/
/* decrypt one data-block                                                     */

#ifdef ANSI_C
  static void DecryptData(Idea_Data data)
#else
  DecryptData(data)
  Idea_Data data;
#endif

{ int i;

  for (i = Idea_dataLen - 1; i >= 0; i--) state[intTime][i] = data[i];
  Idea_Crypt(data, data, key);
  intTime ^= 1;
  for (i = Idea_dataLen - 1; i >= 0; i--) data[i] ^= state[intTime][i];
} /* DecryptData */

/******************************************************************************/
/* store integer 'value' in 'data'                                            */

#ifdef ANSI_C
  static void PlainLenToData(u_int32 value, Idea_Data data)
#else
  PlainLenToData(value, data)
  u_int32 value;
  Idea_Data data;
#endif

{ data[3] = (u_int16)(value << 3 & 0xFFFF);
  data[2] = (u_int16)(value >> 13 & 0xFFFF);
  data[1] = (u_int16)(value >> 29 & 0x0007);
  data[0] = 0;
} /* PlainLenToData */

/******************************************************************************/
/* extract integer 'value' from 'data'                                        */

#ifdef ANSI_C
  static void DataToPlainLen(Idea_Data data, u_int32 *value)
#else
  DataToPlainLen(data, value)
  Idea_Data data;
  u_int32 *value;
#endif

{ if (data[0] || data[1] > 7 || data[3] & 7)
    Error("input is not a valid cryptogram (1)");
  *value = (u_int32)data[3] >> 3 & 0x1FFF |
           (u_int32)data[2] << 13 |
           (u_int32)data[1] << 29;
} /* DataToPlainLen */

/******************************************************************************/
/* encrypt complete data-stream                                               */

#ifdef ANSI_C
  static void EncryptDataStream(void)
#else
  EncryptDataStream()
#endif

{ u_int32 len;
  Idea_Data data;

  while ((len = GetData(data)) == Idea_dataSize) {
    EncryptData(data); 
    PutData(data, Idea_dataSize); 
  }
  if (len) { EncryptData(data); PutData(data, Idea_dataSize); }
  PlainLenToData(inputLen, data);
  EncryptData(data);
  PutData(data, Idea_dataSize);
  CloseOutput();
} /* EncryptDataStream */

/******************************************************************************/
/* decrypt complete data-stream                                               */
 
#ifdef ANSI_C
  static void DecryptDataStream(void)
#else
  DecryptDataStream()
#endif

{ int t;
  u_int32 len;
  Idea_Data dat[4];

  Idea_InvertKey(key, key);
  if ((len = GetData(dat[0])) != Idea_dataSize) {
    if (len) Error("input is not a valid cryptogram (2)");
    else Error("there are no data to decrypt");
  }
  DecryptData(dat[0]);
  if ((len = GetData(dat[1])) != Idea_dataSize) {
    if (len) Error("input is not a valid cryptogram (3)");
    DataToPlainLen(dat[0], &len);
    if (len) Error("input is not a valid cryptogram (4)");
  }
  else {
    DecryptData(dat[1]);
    t = 2;
    while ((len = GetData(dat[t])) == Idea_dataSize) {
      DecryptData(dat[t]);
      PutData(dat[(t + 2) & 3], Idea_dataSize);
      t = (t + 1) & 3;
    }
    if (len) Error("input is not a valid cryptogram (4)");
    DataToPlainLen(dat[(t + 3) & 3], &len);
    len += 2 * Idea_dataSize;
    if (inputLen < len && len <= inputLen + Idea_dataSize) {
      len -= inputLen;
      PutData(dat[(t + 2) & 3], (int)len);
    }
    else Error("input is not a valid cryptogram (5)");
  }
  CloseOutput();
} /* DecryptDataStream */

/******************************************************************************/
/*                          R E M O T E - M A C H I N E                       */
/******************************************************************************/
/* login to remote machine and execute a list of commands                     */

#ifdef ANSI_C
  static void SendCommands(int nofCommands, char *cmd[])
#else
  SendCommands(nofCommands, cmd)
  int nofCommands;
  char *cmd[];
#endif

{ int i, j, pid, wr[2];
  char *argv[32];
  char str[stringLen];

  strcpy(str, ftpCommand);
  i = 0; j = 0;
  while (str[i] != 0) {
    argv[j++] = &str[i++];
    while ((str[i] != ' ') && (str[i] != 0)) i++;
    if (str[i] = ' ') str[i++] = 0;
  }
  argv[j] = NULL;
  if (pipe(wr) == -1) Error("could not create pipe");
  if ((pid = fork()) == -1) Error("could not fork");
  if (pid == 0) { /* child process */
    if (dup2(wr[0], 0) == -1) Error("could not initialize stdin");
    if (close(wr[0]) == -1) Error("could not close pipe (1)");
    if (close(wr[1]) == -1) Error("could not close pipe (2)");
    execve(argv[0], argv, environment);
    Error("could not run execve");
  }
  else { /* parent process */
    sleep(1);
    if (close(wr[0]) == -1) Error("could not close pipe (3)");
    sprintf(str, "user %s %s\n", ftpUser, password);
    write(wr[1], str, strlen(str));
    sprintf(str, "bin\n"); write(wr[1], str, strlen(str));
    for (i = 0; i < nofCommands; i++) {
      sprintf(str, "%s\n", cmd[i]); write(wr[1], str, strlen(str));
    }
    if (close(wr[1]) == -1) Error("could not close pipe (4)");
    while (wait(NULL) != -1);
  }
} /* SendCommands */

/******************************************************************************/
/* split path from filename and generate a change directory command           */

#ifdef ANSI_C
  static void SplitFileName(char *fileName, char *cdCmd, char **file)
#else
  SplitFileName(fileName, cdCmd, file)
  char *fileName;
  char *cdCmd;
  char **file;
#endif

{ int i, pos;
  char ch;

  pos = -1;
  for (i = 0; (ch = fileName[i]) != '\0'; i++)
    if ((ch < '-') || ('z' < ch)) Error("wrong character in file name");
    else if (ch == '/') pos = i;
  if (0 <= pos) {
    strcpy(cdCmd, "cd ");
    strncat(cdCmd, fileName, pos + 1);
  }
  else
    cdCmd[0] = '\0';
  *file = &fileName[pos + 1];
} /* SplitFileName */

/******************************************************************************/
/* find first executable program in a list of programs                        */

#ifdef ANSI_C
  static void GetExecutable(char *prog1, char *prog2, char *executable)
#else
  GetExecutable(prog1, prog2, executable)
  char *prog1;
  char *prog2;
  char *executable;
#endif

{ struct stat b;
  char str[stringLen];
 
  if (!stat(prog1, &b) && b.st_mode & 0111 && (b.st_mode & S_IFMT) == S_IFREG)
    strcpy(executable, prog1);
  else
  if (!stat(prog2, &b) && b.st_mode & 0111 && (b.st_mode & S_IFMT) == S_IFREG)
    strcpy(executable, prog2);
  else {
    sprintf(str, "executable not found (%s, %s)", prog1, prog2);
    Error(str);
  }
} /* ExecutableIndex */

/******************************************************************************/
/* execute the dump command and send result to remote machine                 */

#ifdef ANSI_C
  static void Dump(char *myProgName, char *level, char *device, char *fileName)
#else
  Dump(myProgName, level, device, fileName)
  char *myProgName;
  char *level;
  char *device;
  char *fileName;
#endif

{ char str1[stringLen];
  char str2[stringLen];
  char *ptr[2];
  char *file;
  char dump[64];
  char compress[64];
  int nofCmds = 0;

  if (device[0] != '/') Error("dump device does not start with a '/'");
  if (level[0] == '\0' || level[1] != '\0') Error("wrong dump level");
  if (*level < '0' || '9' < *level) Error("wrong dump level");
  GetExecutable("/usr/etc/dump",  "/usr/sbin/ufsdump", dump);
  GetExecutable("/usr/ucb/compress", "/usr/bin/compress", compress);
  SplitFileName(fileName, str1, &file);
  if (str1[0] != '\0') ptr[nofCmds++] = &str1[0];
  sprintf(str2, "put \"|%s %suf - %s|%s|%s -e\" %s",
          dump, level, device, compress, myProgName, file);
  ptr[nofCmds++] = &str2[0];
  SendCommands(nofCmds, ptr);
} /* Dump */

/******************************************************************************/
/* receive data from remote machine and execute restore command               */

#ifdef ANSI_C
  static void Restore(char *myProgName, char *fileName)
#else
  Restore(myProgName, fileName)
  char *myProgName;
  char *fileName;
#endif

{ char str1[stringLen];
  char str2[stringLen];
  char *ptr[2];
  char *file;
  char restore[64];
  char uncompress[64];
  int nofCmds = 0;

  GetExecutable("/usr/etc/restore", "/usr/sbin/ufsrestore", restore);
  GetExecutable("/usr/ucb/uncompress", "/usr/bin/uncompress", uncompress);
  SplitFileName(fileName, str1, &file);
  if (str1[0] != '\0') ptr[nofCmds++] = &str1[0];
  sprintf(str2, "get %s \"|%s -d|%s|%s if -\"",
          file, myProgName, uncompress, restore);
  ptr[nofCmds++] = &str2[0];
  SendCommands(nofCmds, ptr);
} /* Restore */

/******************************************************************************/
/*                          M A I N - P R O C E D U R E                       */
/******************************************************************************/

#ifdef ANSI_C
  int main(int argc, char *argv[], char *envp[])
#else
  int main(argc, argv, envp)
  int argc;
  char *argv[];
  char *envp[];
#endif

{ if (geteuid() != 0) Error("you are not super-user");
  if (argc <= 1 || argv[1][0] != '-') UsageError();
  environment = envp;
  Init();
  if (geteuid() != 0) Error("you are not super-user");
  if (!strcmp(argv[1], "-c") && argc > 2) {
    argv++; argv++; argc--; argc--;
    SendCommands(argc, argv);
  }
  else if (!strcmp(argv[1], "-e") && argc == 2)
    EncryptDataStream();
  else if (!strcmp(argv[1], "-d") && argc == 2)
    DecryptDataStream();
  else if (!strcmp(argv[1], "-dump") && argc == 5)
    Dump(argv[0], argv[2], argv[3], argv[4]);
  else if (!strcmp(argv[1], "-restore") && argc == 3)
    Restore(argv[0], argv[2]);
  else
    UsageError();
  UnInit();
  return 0;
}
