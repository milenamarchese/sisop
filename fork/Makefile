CFLAGS := -ggdb3 -O2 -Wall -Wextra -std=c11
CFLAGS += -Wmissing-prototypes -Wvla
CPPFLAGS := -D_DEFAULT_SOURCE

PROGS := pingpong primes find xargs

all: error_utils.* $(PROGS) 

find: find.o
xargs: xargs.o error_utils.o
primes: primes.o error_utils.o
pingpong: pingpong.o error_utils.o

clean:
	rm -f $(PROGS) *.o core vgcore.*

.PHONY: all clean
