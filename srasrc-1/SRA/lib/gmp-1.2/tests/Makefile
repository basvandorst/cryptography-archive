# Makefile for tests for GNU MP

# Define ROOT to the path of the sources.
# Define SUB to the path of the compiler library, including a trailing slash.

ROOT = ..
SUB =
TEST_LIBS = $(ROOT)/$(SUB)libgmp.a
SHELL = /bin/sh
CFLAGS += -I$(ROOT) -I$(ROOT)/$(SUB) 

TEST_SRCS = test-mpz_mul.c test-mpz_divmod.c test-mpz_gcd.c test-mpz_sqrtrem.c
TEST_OBJS = test-mpz_mul.o test-mpz_divmod.o test-mpz_gcd.o test-mpz_sqrtrem.o
TESTS = test-mpz_mul test-mpz_divmod test-mpz_gcd test-mpz_sqrtrem

tests: $(TESTS)
	for i in $(TESTS); do echo $$i; $$i; done
	@echo "The tests passed."

test-mpz_mul: test-mpz_mul.o $(TEST_LIBS)
	$(CC) -o $@ $@.o $(TEST_LIBS)
test-mpz_divmod: test-mpz_divmod.o $(TEST_LIBS)
	$(CC) -o $@ $@.o $(TEST_LIBS)
test-mpz_gcd: test-mpz_gcd.o $(TEST_LIBS)
	$(CC) -o $@ $@.o $(TEST_LIBS)
test-mpz_sqrtrem: test-mpz_sqrtrem.o $(TEST_LIBS)
	$(CC) -o $@ $@.o $(TEST_LIBS)

$(TEST_PREFIX)clean:
	rm -f $(TESTS) $(TEST_OBJS)
