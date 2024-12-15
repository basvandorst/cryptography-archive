
LD= $(CC)
CC= cc

LDFLAGS=-g
CFLAGS=-g -I/usr/demo/SOUND -I/usr/local/include

LIBS= -L/usr/demo/SOUND/ -laudio

audio_rand: audio_rand.o
	$(LD) $(LDFLAGS) audio_rand.o $(LIBS) -o $@

test: audio_rand
	audio_rand | head -10000 > \#fff
	echo "plot '#fff'" | gnuplot

