/* $Id$ */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "me.h"

FILE *safeopen(name)
     char *name;
{
    FILE *fp;
    int fd = open(name, O_WRONLY | O_CREAT | O_EXCL, 0600);

    if (fd < 0 &&
	(unlink(name) ||
	 (fd = open(name, O_WRONLY | O_CREAT | O_EXCL, 0600)) < 0))
	return NULL;

    if (! (fp = fdopen(fd, "w"))) {
      close(fd);
      unlink(name);
    }
    return fp;
}

FILE *safeopen_rdwr(name)
     char *name;
{
    FILE *fp;
    int fd = open(name, O_RDWR | O_CREAT | O_EXCL, 0600);

    if (fd < 0 &&
	(unlink(name) ||
	 (fd = open(name, O_RDWR | O_CREAT | O_EXCL, 0600)) < 0))
	return NULL;

    if (! (fp = fdopen(fd, "w+"))) {
      close(fd);
      unlink(name);
    }
    return fp;
}

