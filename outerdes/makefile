CC=cc
CFLAGS=-O
ROUTINES=d3des.o usage.o desmain.o mygetopt.o

all: outerdes testdes

outerdes: $(ROUTINES)
	$(CC) $(CFLAGS) -o outerdes $(ROUTINES)

testdes:
	@echo  "This should display the alphabet: "
	@outerdes -d -k OuterDES -i alphabet.enc
