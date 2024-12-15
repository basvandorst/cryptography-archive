/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
/*
 * structure to allow the packet to be in different memory segments
 * this functionallity is mainly used in BSD system, where
 * the packet is split over several mbufs.
 */

#define STATIC_MEMSEGS 64  /* Static number, dynamically allocated if more */

struct memseg
{
  u_char *ptr;
  int len;
};

struct memblk
{
  int len;                 /* Length of whole block        */
  int offset;              /* Offset into first block      */
  int dynamiclen;          /* Length of dynamically allocated array */
  struct memseg *dynamic;  /* NULL == no dynamically allocated array */
  struct memseg *ms;	   /* Array of segment descriptors */
};

#define BLKSTART(m)       ((m)->ms->ptr + (m)->offset)
#define BLKINC(m,val)     (m)->offset += (val), (m)->len -= (val)
#define BLKDEC(m,val)     (m)->offset -= (val), (m)->len += (val)
#define BLKADD(m,src,len) MEMCPY(BLKSTART((m)),(src),(len)), BLKINC((m),(len))
#define BLKADDCHAR(m,ch)  *BLKSTART((m)) = (ch), BLKINC((m), 1)
#define BLKGETCHAR(m,ch)  (ch) = *BLKSTART((m)), BLKINC((m), 1)
