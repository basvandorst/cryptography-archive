/*
 * The author of this software is John Ioannidis, ji@cs.columbia.edu.
 * Copyright (C) 1993, by John Ioannidis.
 *	
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTY. IN PARTICULAR, THE AUTHOR DOES MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE
 * MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 */
 
/*
 * Modloadable wrapper for the swIPe interface.
 * Originally written as a "fake" interface driver by ji, by using
 * some code from the 4.1.3 AnswerBook and making educated guesses
 * about system files.
 */

#include <sys/types.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sundev/mbvar.h>
#include <sun/autoconf.h>
#include <sun/vddrv.h>
#if defined(sun4c) || defined(sun4m)
#include <sun/openprom.h>
#endif

extern int swopen(), swclose(), swioctl();
extern int nulldev();

struct cdevsw swdevsw = 
{
	swopen, swclose, nulldev, nulldev,
	swioctl, nulldev, nulldev, nulldev,
	0, nulldev,
};


#ifdef sun4m
struct vdldrv vd = 
{
	VDMAGIC_PSEUDO,
	"sw",
	NULL,
	NULL,
	&swdevsw,
	0,
	0,
	NULL,
	NULL,
	NULL,
	0,
	1,
};
#else /* sun4m */
struct vdldrv vd =
{
	VDMAGIC_PSEUDO,
	"sw",
#ifdef sun4c
	NULL,
#else
	NULL,
	NULL,
	NULL,
	0,
	1,
#endif /* sun4c */
	NULL,
	&swdevsw,
	0,
	0,
};
#endif /* sun4m */

xxxinit(fc, vdp, vdi, vds)
	unsigned int fc;
	struct vddrv *vdp;
	addr_t vdi;
	struct vdstat *vds;
{
	switch (fc)
	{
	      case VDLOAD:
		vdp->vdd_vdtab = (struct vdlinkage *)&vd;
		return swinit(0);
		
	      case VDUNLOAD:
		printf("cannot unload yet -- sorry!\n");
		return EBUSY;
		
	      case VDSTAT:
		return 0;

	      default:
		return EIO;
	}
}

