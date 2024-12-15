/******************************************************************************/
/*                                                                            */
/*   C H A N G E   U S E R   A N D   G R O U P - I D ' S   O F   F I L E S    */
/*                                                                            */
/******************************************************************************/
/* Author:       Richard De Moliner (demoliner@isi.ethz.ch)                   */
/*               Signal and Information Processing Laboratory                 */
/*               Swiss Federal Institute of Technology                        */
/*               CH-8092 Zuerich, Switzerland                                 */
/* Last Edition: 26. March 1993                                               */
/* System:       SUN SPARCstation, SUN acc C-Compiler, SUN-OS 4.1.3           */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

/*----------------------------------------------------------------------------*/
/* CONSTANTS                                                                  */
/*----------------------------------------------------------------------------*/
#define userFileName      "table_user_id"
#define groupFileName     "table_group_id"
#define oldPasswdFileName "passwd_old"
#define newPasswdFileName "passwd_new"
#define oldGroupFileName  "group_old"
#define newGroupFileName  "group_new"

#define maxPathLen         4096
#define maxUserId         10000
#define maxGroupId        10000

#define FALSE               (0)
#define TRUE                (1)

/*----------------------------------------------------------------------------*/
/* SYSTEM CALLS                                                               */
/*----------------------------------------------------------------------------*/
int            chdir    (char *path);
int            lstat    (char *path, struct stat *buf);
int            chown    (char *path, int owner, int group);

/*----------------------------------------------------------------------------*/
/* C LIBRARY FUNCTIONS                                                        */
/*----------------------------------------------------------------------------*/
char          *getcwd   (char *buf, int size);
DIR           *opendir  (char *filename);
struct dirent *readdir  (DIR *dirp);
int            closedir (DIR *dirp);
struct passwd *getpwuid (int uid);
struct group  *getgrgid (int gid);

/*----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                           */
/*----------------------------------------------------------------------------*/
int ok;
int nofChanges;
int newUserId[maxUserId];
int newGroupId[maxGroupId];

/*----------------------------------------------------------------------------*/
void PrintUserName (int userId)

{ struct passwd *password;

  password = getpwuid(userId);
  if (password != NULL )
    printf("%s", password->pw_name);
  else
    printf("???????");
} /* PrintUserName */

/*----------------------------------------------------------------------------*/
void PrintGroupName (int groupId)

{ struct group *group;

  group = getgrgid(groupId);
  if (group != NULL )
    printf("%s", group->gr_name);
  else
    printf("???????");
} /* PrintGroupName */

/*----------------------------------------------------------------------------*/
void ReadIdArray(int isUserId, char *fileName, int id[], int len)

{ FILE *inFile;
  int i, oldId, newId;

  if (!ok) return;
  for (i = 0; i < len; i++) id[i] = -1;
  if ((inFile = fopen(fileName, "r")) == NULL) {
    perror(fileName); ok = FALSE; return;
  }
  while (fscanf(inFile, "%d %d\n", &oldId, &newId) == 2) {
    if (oldId < 0 || len <= oldId || newId < 0 || len <= newId) {
      printf("*** old (%d) or new (%d) id out of range in %s\n",
             oldId, newId, fileName);
      ok = FALSE;
    }
    else if (oldId == newId) {
      printf("*** old and new id (%d) are equal in %s\n", oldId, fileName);
      ok = FALSE;
    }
    else if (id[oldId] != -1) {
      printf("*** multiple defined old id (%d) in %s\n", oldId, fileName);
      ok = FALSE;
    }
    else
      id[oldId] = newId;
  }
  if (fclose(inFile)) { perror(fileName); ok = FALSE; return; }
  if (isUserId)
    printf("old uid -> new uid\n");
  else
    printf("old gid -> new gid\n");
  for (i = 0; i < len; i++)
    if (id[i] != -1) {
      printf("%7d -> %7d  ", i, id[i]);
      if (isUserId) PrintUserName(i); else PrintGroupName(i);
      printf("\n");
    }
  printf("\n");
  fflush(stdout);
} /* ReadIdArray */

/*----------------------------------------------------------------------------*/
void Init ()

{ char answer[128];

  ok = TRUE;
  nofChanges = 0;
  ReadIdArray(TRUE, userFileName, newUserId, maxUserId);
  ReadIdArray(FALSE, groupFileName, newGroupId, maxGroupId);
  if (ok) {
    printf("ok to use this translation table [y/n]: "); fflush(stdout);
    scanf("%s", answer);
    ok = !strcmp(answer, "y");
  }
} /* Init */

/*----------------------------------------------------------------------------*/
void GenerateNewPasswordFile ()

{ FILE *inFile, *outFile;
  int ch, cnt, id;

  if (!ok) return;
  if ((inFile = fopen(oldPasswdFileName, "r")) == NULL) {
    perror(oldPasswdFileName); ok = FALSE; return;
  }
  if ((outFile = fopen(newPasswdFileName, "w")) == NULL) {
    perror(newPasswdFileName); ok = FALSE; return;
  }
  cnt = 0;
  for (ch = fgetc(inFile); ch != EOF; ch = fgetc(inFile)) {
    if (ch == '\n') cnt = 0;
    if (cnt != 2 && cnt != 3) fputc(ch, outFile);
    if (ch == ':') {
      if (cnt == 2) {
        if (0 <= id && id < maxUserId && newUserId[id] != -1) id=newUserId[id];
        fprintf(outFile, "%d:", id);
      }
      else if (cnt == 3) {
        if (0 <= id && id < maxGroupId && newGroupId[id]!=-1) id=newGroupId[id];
        fprintf(outFile, "%d:", id);
      }
      cnt++;
      id = 0;
    }
    else if (cnt == 2 || cnt == 3)
      id = id * 10 + (ch - '0');
  }
  fclose(inFile);
  fclose(outFile);
  printf("\npassword file is translated: %s -> %s\n",
         oldPasswdFileName, newPasswdFileName);
  fflush(stdout);
} /* GenerateNewPasswordFile */

/*----------------------------------------------------------------------------*/
void GenerateNewGroupFile ()

{ FILE *inFile, *outFile;
  int ch, cnt, id;

  if (!ok) return;
  if ((inFile = fopen(oldGroupFileName, "r")) == NULL) {
    perror(oldGroupFileName); ok = FALSE; return;
  }
  if ((outFile = fopen(newGroupFileName, "w")) == NULL) {
    perror(newGroupFileName); ok = FALSE; return;
  }
  cnt = 0;
  for (ch = fgetc(inFile); ch != EOF; ch = fgetc(inFile)) {
    if (ch == '\n') cnt = 0;
    if (cnt != 2) fputc(ch, outFile);
    if (ch == ':') {
      if (cnt == 2) {
        if (0 <= id && id < maxGroupId && newGroupId[id]!=-1) id=newGroupId[id];
        fprintf(outFile, "%d:", id);
      }
      cnt++;
      id = 0;
    }
    else if (cnt == 2)
      id = id * 10 + (ch - '0');
  }
  fclose(inFile);
  fclose(outFile);
  printf("\ngroup file is translated: %s -> %s\n",
         oldGroupFileName, newGroupFileName);
  fflush(stdout);
} /* GenerateNewGroupFile */

/*----------------------------------------------------------------------------*/
int IsPathOk (char *path)

{ char currPath[maxPathLen];

  return !chdir(path) && getcwd(currPath, maxPathLen) != NULL &&
         !strcmp(path,currPath);
} /* IsPathOk */

/*----------------------------------------------------------------------------*/
void Traverse (char *path, int level)

{ char fileName[maxPathLen];
  DIR *directory;
  struct dirent *file;
  struct stat status;
  int userId, groupId, isRoot, nofChangesOld;

  if (!ok) return;
  if (!IsPathOk(path) || (directory = opendir(path)) == NULL) {
    printf("*** not analyzed: %s\n", path); return;
  }
  if (level <= 2) {
    printf("translating    %s\n", path); fflush(stdout);
    nofChangesOld = nofChanges;
  }
  for (file = readdir(directory); file != NULL; file = readdir(directory)) {
    isRoot = level == 0 && !strcmp(file->d_name, ".");
    if (isRoot || strcmp(file->d_name, ".") && strcmp(file->d_name, "..")) {
      if (isRoot)
        strcpy(fileName, path);
      else if (!strcmp(path, "/"))
        sprintf(fileName, "/%s", file->d_name);
      else
        sprintf(fileName, "%s/%s", path, file->d_name);
      if (lstat(fileName, &status))
        printf("*** could not read status of %s\n", fileName);
      else {
        if (!isRoot && (status.st_mode & S_IFMT) == S_IFDIR)
          Traverse(fileName, level + 1);
        userId = groupId = -1;
        if (0 <= status.st_uid && status.st_uid < maxUserId)
          userId = newUserId[status.st_uid];
        if (0 <= status.st_gid && status.st_gid < maxGroupId)
          groupId = newGroupId[status.st_gid];
        if (userId != -1 || groupId != -1) {
          nofChanges++;
          if (chown(fileName, userId, groupId))
            printf("*** chown: uid %4d -> %4d, gid %4d -> %4d failed for %s\n",
                   status.st_uid, userId, status.st_gid, groupId, fileName);
        }
      }
    }
  }
  closedir (directory);
  if (level <= 2 && nofChanges - nofChangesOld)
    printf("%5d chown in %s\n", nofChanges - nofChangesOld, path);
  fflush(stdout);
} /* Traverse */

/*----------------------------------------------------------------------------*/
main ()

{ char root[maxPathLen];

  Init(); if (!ok) return -1;
  GenerateNewPasswordFile(); if (!ok) return -1;
  GenerateNewGroupFile(); if (!ok) return -1;
  printf("\ndirectory to start with translation   : "); fflush(stdout);
  scanf("%s", root);
  if (root[0] != '/' || !IsPathOk(root)) {
    printf("*** wrong absolute directory name\n"); return -1;
  }
  printf("\ntranslation started\n\n");
  Traverse(root, 0); if (!ok) return -1;
  printf("\ntranslation done\n");
  return 0;
} /* main */

/*----------------------------------------------------------------------------*/
