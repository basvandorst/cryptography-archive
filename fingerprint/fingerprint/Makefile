CC=cc -O
TIME=time
COMPRESS=compress

all: fingerprint fptocksum fptosnefru fptomd5 snefru md5 cksum

snefru: snefru.o snefrufile.o snefru512.o summain.o getopt.o
	$(CC) -o snefru snefru.o snefrufile.o snefru512.o summain.o getopt.o

cksum: cksum.o crc32file.o summain.o getopt.o
	$(CC) -o cksum cksum.o crc32file.o summain.o getopt.o

md5: md5.o md5file.o summain.o getopt.o
	$(CC) -o md5 md5.o md5file.o summain.o getopt.o

fptosnefru: fptosnefru.o fprintfmt.o
	$(CC) -o fptosnefru fptosnefru.o fprintfmt.o

fptomd5: fptomd5.o fprintfmt.o
	$(CC) -o fptomd5 fptomd5.o fprintfmt.o

fptocksum: fptocksum.o fprintfmt.o
	$(CC) -o fptocksum fptocksum.o fprintfmt.o

fingerprint: fingerprint.o crc32file.o md5file.o snefrufile.o \
snefru512.o fprintfile.o fprintfmt.o getopt.o summain.o
	$(CC) -o fingerprint fingerprint.o crc32file.o md5file.o \
	snefrufile.o snefru512.o fprintfile.o fprintfmt.o getopt.o summain.o

fptosnefru.o: fptosnefru.c
	$(CC) -c fptosnefru.c

fptomd5.o: fptomd5.c
	$(CC) -c fptomd5.c

fptocksum.o: fptocksum.c
	$(CC) -c fptocksum.c

cksum.o: cksum.c crc32file.h
	$(CC) -c cksum.c

md5.o: md5.c md5file.h
	$(CC) -c md5.c

snefru.o: snefru.c snefrufile.h
	$(CC) -c snefru.c

fprintfmt.o: fprintfmt.c fprintfile.h
	$(CC) -c fprintfmt.c

fprintfile.o: fprintfile.c fprintfile.h
	$(CC) -c fprintfile.c

crc32file.o: crc32file.c crc32file.h
	$(CC) -c crc32file.c

md5file.o: md5file.c md5file.h
	$(CC) -c md5file.c

snefrufile.o: snefrufile.c snefrufile.h snefru512.h
	@echo You should get three warnings for snefrufile.c.
	@echo This is my reminder that the code there is not portable.
	$(CC) -c snefrufile.c

snefru512.o: snefru512.c snefru512.h
	$(CC) -c snefru512.c

fingerprint.o: fingerprint.c crc32file.h md5file.h snefrufile.h \
fprintfile.h getopt.h
	$(CC) -c fingerprint.c

summain.o: summain.c getopt.h
	$(CC) -c summain.c

getopt.o: getopt.c getopt.h
	$(CC) -c getopt.c

test:
	@echo 'Trying canned tests... No output (other than seven ./commands) if successful.'
	./fingerprint -c < fingerprint.tests
	./snefru -c < snefru.tests
	./md5 -c < md5.tests
	./cksum -c < cksum.tests
	./fptosnefru < fingerprint.tests | cmp - snefru.tests
	./fptomd5 < fingerprint.tests | cmp - md5.tests
	./fptocksum < fingerprint.tests | cmp - cksum.tests

shar:
	shar `cat FILES` > fingerprint.shar
	chmod 400 fingerprint.shar

clean: OBJECTS BINARIES
	rm -f `cat OBJECTS BINARIES`

speedtest.1M:
	@echo 'Making a one-megabyte file for speed tests...'
	cat `cat FILES` > speedtest.tmp
	dd bs=8k count=8 < speedtest.tmp > speedtest.64k
	cat speedtest.64k speedtest.64k speedtest.64k speedtest.64k > speedtest.256k
	cat speedtest.256k speedtest.256k speedtest.256k speedtest.256k > speedtest.1M
	rm -f speedtest.tmp speedtest.64k speedtest.256k

speedtest: speedtest.1M fingerprint snefru md5 cksum
	@echo 'Trying speed tests...'
	$(TIME) ./fingerprint < speedtest.1M > /dev/null
	$(TIME) ./snefru < speedtest.1M > /dev/null
	$(TIME) ./md5 < speedtest.1M > /dev/null
	$(TIME) ./cksum < speedtest.1M > /dev/null
	$(TIME) $(COMPRESS) < speedtest.1M > /dev/null
