 ############################################################################
#                                                                            #
#                       Makefile for Implementation of                       #
#                       RRC.2 - Ron Rivest Cipher Nr. 2                      #
#                                                                            #
#       Implementation done on 13/15.02.96 by Daniel Vogelheim.              #
#                                                                            #
#       This document is placed into the public domain.                      #
#                                                                            #
#                                                                            #
#       compile all files with:         make all                             #
#       test with:                      make test                            #
#       make source archive with:       make archive                         #
#       clean directory with:           make clean                           #
#       rebuild with:                   make rebuild                         #
#                                                                            #
 ############################################################################

### Parameters: (i386 Linux)

ENDIAN = LITTLE# LITTLE or BIG (No spaces between value and # allowed!)
CFLAGS = -Wall -ansi -D$(ENDIAN)_ENDIAN
CC = gcc $(CFLAGS)

### Rules:

EXEC = rrc2_test rrc2_encrypt rrc2_decrypt
CSOURCE = rrc2.c rrc2.h rrc2_consts.c rrc2_test.c rrc2_crypt.c
SOURCE = $(CSOURCE) makefile readme history rrc2_spec.txt rrc2_news.txt rrc2_news_2.txt

all: $(EXEC)
archive: rrc2.tar.gz

rrc2.o: rrc2.c rrc2.h rrc2_consts.c
	$(CC) -c rrc2.c

rrc2_test: rrc2_test.c rrc2.o rrc2.h
	$(CC) -o rrc2_test rrc2_test.c rrc2.o

rrc2_encrypt: rrc2_crypt.c rrc2.h rrc2.o
	$(CC) -o rrc2_encrypt rrc2_crypt.c rrc2.o

rrc2_decrypt: rrc2_crypt.c rrc2.h rrc2.o
	$(CC) -o rrc2_decrypt rrc2_crypt.c rrc2.o -DDECRYPT

rrc2.tar.gz:  $(SOURCE)
	@echo "Packing source files into archive:"
	@rm -f rrc2.tar.gz
	@tar cvof - $(SOURCE) | gzip >rrc2.tar.gz

rrc2.uue: rrc2.tar.gz
	uuencode rrc2.tar.gz rrc2.tar.gz >rrc2.uue

test: rrc2_test rrc2_encrypt rrc2_decrypt
	# test spec conformance
	rrc2_test
	# test if decrypt.encrypt = id
	cp rrc2_spec.txt test.file
	echo "abc" >>test.file # make length divisible by blocksize
	echo "test.file" >test.cmd
	echo "This_is_a_test-password." >>test.cmd
	cp test.file test.file.org
	rrc2_encrypt < test.cmd
	rm -f test.file
	rrc2_decrypt < test.cmd
	cmp test.file test.file.org
	rm -f test.file test.file.rrc2 test.file.org test.cmd
	# done!
	@echo
	@echo
	@echo "RRC.2 test passed!"
	
clean:
	rm -f *.o *~ $(EXEC)

touch:
	touch $(CSOURCE)

rebuild: touch
	make all

# FINI: makefile
