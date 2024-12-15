#ifndef _BLK_H
#define _BLK_H

#include <linux/major.h>
#include <linux/sched.h>
#include <linux/locks.h>
#include <linux/genhd.h>

/*
 * NR_REQUEST is the number of entries in the request-queue.
 * NOTE that writes may use only the low 2/3 of these: reads
 * take precedence.
 *
 * 32 seems to be a reasonable number: enough to get some benefit
 * from the elevator-mechanism, but not so much as to lock a lot of
 * buffers when they are in the queue. 64 seems to be too many (easily
 * long pauses in reading when heavy writing/syncing is going on)
 */
#define NR_REQUEST	64

/*
 * Ok, this is an expanded form so that we can use the same
 * request for paging requests when that is implemented. In
 * paging, 'bh' is NULL, and the semaphore is used to wait
 * for read/write completion.
 */
struct request {
	int dev;		/* -1 if no request */
	int cmd;		/* READ or WRITE */
	int errors;
	unsigned long sector;
	unsigned long nr_sectors;
	unsigned long current_nr_sectors;
	char * buffer;
	struct semaphore * sem;
	struct buffer_head * bh;
	struct buffer_head * bhtail;
	struct request * next;
};

/*
 * This is used in the elevator algorithm: Note that
 * reads always go before writes. This is natural: reads
 * are much more time-critical than writes.
 */
#define IN_ORDER(s1,s2) \
((s1)->cmd < (s2)->cmd || ((s1)->cmd == (s2)->cmd && \
((s1)->dev < (s2)->dev || (((s1)->dev == (s2)->dev && \
(s1)->sector < (s2)->sector)))))

struct blk_dev_struct {
	void (*request_fn)(void);
	struct request * current_request;
};


struct sec_size {
	unsigned block_size;
	unsigned block_size_bits;
};

/*
 * These will have to be changed to be aware of different buffer
 * sizes etc.. It actually needs a major cleanup.
 */
#define SECTOR_MASK (blksize_size[loop_major] &&     \
	blksize_size[loop_major][MINOR(CURRENT->dev)] ? \
	((blksize_size[loop_major][MINOR(CURRENT->dev)] >> 9) - 1) :  \
	((BLOCK_SIZE >> 9)  -  1))

#define SUBSECTOR(block) (CURRENT->current_nr_sectors > 0)

extern struct sec_size * blk_sec[MAX_BLKDEV];
extern struct blk_dev_struct blk_dev[MAX_BLKDEV];
extern struct wait_queue * wait_for_request;
extern void resetup_one_dev(struct gendisk *dev, int drive);

extern int * blk_size[MAX_BLKDEV];

extern int * blksize_size[MAX_BLKDEV];

extern unsigned long hd_init(unsigned long mem_start, unsigned long mem_end);
extern unsigned long cdu31a_init(unsigned long mem_start, unsigned long mem_end);
extern unsigned long mcd_init(unsigned long mem_start, unsigned long mem_end);
#ifdef CONFIG_SBPCD
extern unsigned long sbpcd_init(unsigned long, unsigned long);
#endif CONFIG_SBPCD
extern int is_read_only(int dev);
extern void set_device_ro(int dev,int flag);

extern void rd_load(void);
extern long rd_init(long mem_start, int length);
extern int ramdisk_size;

extern unsigned long xd_init(unsigned long mem_start, unsigned long mem_end);

#define RO_IOCTLS(dev,where) \
  case BLKROSET: if (!suser()) return -EACCES; \
		 set_device_ro((dev),get_fs_long((long *) (where))); return 0; \
  case BLKROGET: { int __err = verify_area(VERIFY_WRITE, (void *) (where), sizeof(long)); \
		   if (!__err) put_fs_long(0!=is_read_only(dev),(long *) (where)); return __err; }
		 
#define       DEVICE_OFF(device)
#define DEVICE_REQUEST do_lo_request

#ifndef CURRENT
#define CURRENT (blk_dev[loop_major].current_request)
#endif

#define CURRENT_DEV DEVICE_NR(CURRENT->dev)

#ifdef DEVICE_INTR
void (*DEVICE_INTR)(void) = NULL;
#endif
#ifdef DEVICE_TIMEOUT

#define SET_TIMER \
((timer_table[DEVICE_TIMEOUT].expires = jiffies + TIMEOUT_VALUE), \
(timer_active |= 1<<DEVICE_TIMEOUT))

#define CLEAR_TIMER \
timer_active &= ~(1<<DEVICE_TIMEOUT)

#define SET_INTR(x) \
if ((DEVICE_INTR = (x)) != NULL) \
	SET_TIMER; \
else \
	CLEAR_TIMER;

#else

#define SET_INTR(x) (DEVICE_INTR = (x))

#endif
static void (DEVICE_REQUEST)(void);

/* end_request() - SCSI devices have their own version */

static void end_request(int uptodate)
{
	struct request * req;
	struct buffer_head * bh;

	req = CURRENT;
	req->errors = 0;
	if (!uptodate) {
		printk(DEVICE_NAME " I/O error\n");
		printk("dev %04lX, sector %lu\n",
		       (unsigned long)req->dev, req->sector);
		req->nr_sectors--;
		req->nr_sectors &= ~SECTOR_MASK;
		req->sector += (BLOCK_SIZE / 512);
		req->sector &= ~SECTOR_MASK;		
	}

	if ((bh = req->bh) != NULL) {
		req->bh = bh->b_reqnext;
		bh->b_reqnext = NULL;
		bh->b_uptodate = uptodate;		
		if (!uptodate) bh->b_req = 0; /* So no "Weird" errors */
		unlock_buffer(bh);
		if ((bh = req->bh) != NULL) {
			req->current_nr_sectors = bh->b_size >> 9;
			if (req->nr_sectors < req->current_nr_sectors) {
				req->nr_sectors = req->current_nr_sectors;
				printk("end_request: buffer-list destroyed\n");
			}
			req->buffer = bh->b_data;
			return;
		}
	}
	DEVICE_OFF(req->dev);
	CURRENT = req->next;
	if (req->sem != NULL)
		up(req->sem);
	req->dev = -1;
	wake_up(&wait_for_request);
}

#ifdef DEVICE_INTR
#define CLEAR_INTR SET_INTR(NULL)
#else
#define CLEAR_INTR
#endif

#define INIT_REQUEST \
	if (!CURRENT) {\
		CLEAR_INTR; \
		return; \
	} \
	if (MAJOR(CURRENT->dev) != loop_major) \
		panic(DEVICE_NAME ": request list destroyed"); \
	if (CURRENT->bh) { \
		if (!CURRENT->bh->b_lock) \
			panic(DEVICE_NAME ": block not locked"); \
	}

#endif
