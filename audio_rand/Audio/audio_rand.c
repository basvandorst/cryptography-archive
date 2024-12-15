static char *_author = "Peter Shipley\n";

#define AUDIO_CHIP
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sun/audioio.h>
#include <sbusdev/audio_79C30.h>
#include <multimedia/ulaw2linear.h>


/* X:  0-15 
   R:  16-31
   GX: 32-33
   GR: 33-34
*/

#define MMR2_BITS 45

static char *_who_did_it = "Peter M Shipley (1992) [v0.1]\n";



#define GX_GAIN 32


main()
{
struct audio_ioctl ai;
extern void bzero();
int fd;
int j, i;

    if( (fd = open("/dev/audio", O_RDONLY)) < 0) {
	perror("open:");
	exit(1);
    }

    bzero(ai, sizeof(struct audio_ioctl) );

    ai.control = AUDIO_MAP_ALL;

    if ( ioctl( fd, AUDIOGETREG, &ai ) < 0 ) {
	perror( "AUDIOGETREG ALL:" );
	exit(1);
    }
    /* set audio input to a unconnected pin (audio input B) */
    ai.data[MMR2_BITS] = ( ai.data[MMR2_BITS] & ~AUDIO_MMR2_BITS_AINB);

    /* set gain of the GX reg to the max */
    ai.data[GX_GAIN] = 0x7F;
    ai.data[GX_GAIN+1] = 0x0;

    for(i=0;;) {
	char buf[BUFSIZ];

	read(fd, buf, BUFSIZ);

	for(j=0; j<BUFSIZ ; j++)
	    printf("%d %d\n", i++, audio_u2s(buf[j]));
    }
}
