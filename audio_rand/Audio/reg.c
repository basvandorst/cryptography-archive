static char *_author = "Peter Shipley\n";

#define AUDIO_CHIP
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sun/audioio.h>
#include <sbusdev/audio_79C30.h>

static char * print_byte();

static struct _map {
	char *label;
	char size;
} map[] = {
    "X filter", 16,
    "R filter", 16,
    "GX Gain", 2,
    "GR Gain", 2,
    "GER Gain", 2,
    "Sidetone", 2,
    "Frequency Tone gen", 2,
    "Amplitude Tone gen", 2,
    "MMR1", 1,
    "MMR2", 1
};

#define NMAPS  (sizeof(map) / sizeof(struct _map) )

struct audio_ioctl ai;

main()
{
extern void bzero();
int fd;
int oldmmr2;
int newmmr2;

    if( (fd = open("/dev/audio", O_RDWR)) < 0) {
	perror("open:");
	exit(1);
    }

    dump(fd);

    /*
    bzero(ai, sizeof(struct audio_ioctl) );
    ai.control = AUDIO_MAP_GX;
    if ( ioctl( fd, AUDIOGETREG, &ai ) < 0 ) {
	perror( "AUDIOGETREG MMR2:" );
	exit(1);
    }

    ai.data[0] = ai.data[1] = 0;
    if ( ioctl( fd, AUDIOSETREG, &ai ) < 0 ) {
	perror( "AUDIOSETREG MMR2:" );
	exit(1);
    }

    */

    sleep(5);
    dump(fd);

}


dump(f)
int f;
{
int i,k;

    bzero(ai, sizeof(struct audio_ioctl) );
    ai.control = AUDIO_MAP_ALL;
    if ( ioctl( f, AUDIOGETREG, &ai ) < 0 ) {
	perror( "AUDIOGETREG MMR2:" );
	exit(1);
    }


    k=0;
    for(i = 0 ; i < NMAPS ; i++) {
	int j;
	(void) printf("%s:\n", map[i].label);
	for(j=1; j <= map[i].size; j++,k++) {
	    (void) printf("%10s", print_byte(ai.data[k]));
	    if ( (j % 7) == 0)
		(void) fputc('\n', stdout);
	}
	(void) fputc('\n', stdout);
    }

    (void) fputc('\n', stdout);
}




static char *
print_byte(c)
char c;
{
static char bt[9];

    bt[0] = ( (c & 0x01) ? '1' : '0');
    bt[1] = ( (c & 0x02) ? '1' : '0');
    bt[2] = ( (c & 0x03) ? '1' : '0');
    bt[3] = ( (c & 0x04) ? '1' : '0');
    bt[4] = ( (c & 0x05) ? '1' : '0');
    bt[5] = ( (c & 0x06) ? '1' : '0');
    bt[6] = ( (c & 0x07) ? '1' : '0');
    bt[7] = ( (c & 0x08) ? '1' : '0');
    bt[8] = NULL;

    return bt;
}
