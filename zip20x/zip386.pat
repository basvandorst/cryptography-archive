The following patch has been applied to the sources of zip 2.0
to avoid creating entries for "./" and "../" in zip files with
zip386.exe, compiled with DJGPP 1.10. The patch is not necessary
for the 16 bit version zip.exe.

*** zip20/fileio.c	Tue Sep 14 12:12:40 1993
--- fileio.c	Mon Sep 13 03:26:52 1993
***************
*** 832,847 ****
     file system.  Return an error code in the ZE_ class. */
  {
  # ifndef __GO32__
-   char *a;              /* alloc'ed space for name */
    dstrm *d;             /* stream for reading directory */
    char *e;              /* name found in directory */
!   int f;                /* true if there was a match */
    char *n;              /* constructed name from directory */
    char *p;              /* path */
    char *q;              /* name */
-   int r;                /* temporary variable */
    char v[5];            /* space for device current directory */
- # endif /* __GO32__ */
  
  # ifndef WIN32
    if (volume_label == 1) {
--- 832,847 ----
     file system.  Return an error code in the ZE_ class. */
  {
  # ifndef __GO32__
    dstrm *d;             /* stream for reading directory */
    char *e;              /* name found in directory */
!   int r;                /* temporary variable */
    char *n;              /* constructed name from directory */
+ # endif /* __GO32__ */
+   int f;                /* true if there was a match */
+   char *a;              /* alloc'ed space for name */
    char *p;              /* path */
    char *q;              /* name */
    char v[5];            /* space for device current directory */
  
  # ifndef WIN32
    if (volume_label == 1) {
***************
*** 855,863 ****
      /* "zip -$ foo a:" can be used to force drive name */
    }
  # endif
- # ifdef __GO32__
-   return procname(w); /* expansion already done by DJGPP */
- # else
    /* Allocate and copy pattern */
    if ((p = a = malloc(strlen(w) + 1)) == NULL)
      return ZE_MEM;
--- 855,860 ----
***************
*** 908,914 ****
          err(ZE_PARMS, "cannot remove parent directory");
       return procname(p);
    }
! 
    /* Search that level for matching names */
    if ((d = opend(p)) == NULL)
    {
--- 905,917 ----
          err(ZE_PARMS, "cannot remove parent directory");
       return procname(p);
    }
! # ifdef __GO32__
!   /* expansion already done by DJGPP */
!   f = 1;
!   if (strcmp(q, ".") != 0 && strcmp(q, "..") != 0 && procname(w) != ZE_OK) {
!      f = 0;
!   }
! # else
    /* Search that level for matching names */
    if ((d = opend(p)) == NULL)
    {
***************
*** 950,960 ****
      }
    }
    closed(d);
  
    /* Done */
    free((voidp *)a);
    return f ? ZE_OK : ZE_MISS;
- # endif /* __GO32__ */
  }
  #endif /* MSDOS || OS2 */
  
--- 953,963 ----
      }
    }
    closed(d);
+ # endif /* __GO32__ */
  
    /* Done */
    free((voidp *)a);
    return f ? ZE_OK : ZE_MISS;
  }
  #endif /* MSDOS || OS2 */
  
