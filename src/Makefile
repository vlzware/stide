# Stide - Makefile

# compiler to use
CC = gcc

# flags to pass compiler
CFLAGS = -g -O0 -Wall -Werror -Wextra -pedantic-errors -std=c11 \
	-D_XOPEN_SOURCE=600

# name for executable
EXE = stide

# space-separated list of libraries, if any,
# each of which should be prefixed with -l
LIBS = -lsqlite3 -lm -lpng

# space-separated list of source files
SRCS = stide.c helpers.c preprocess.c image.c hide.c create.c extract.c

# automatically generated list of object files
OBJS = $(SRCS:.c=.o)

# default target
all: stide stide_gtk

stide: $(OBJS) Makefile
	$(CC) $(CFLAGS) -o ../bin/linux/$@ $(OBJS) $(LIBS)
	cp ../db/stide.db ../bin/linux/

stide_gtk: stide_gtk.c
	gcc -g -rdynamic  `pkg-config --cflags gtk+-3.0` \
	stide_gtk.c -o ../bin/linux/stide_gtk `pkg-config --libs gtk+-3.0`
	cp stide.glade ../bin/linux/
	cp ../media/cat.png ../bin/linux

# housekeeping
clean:
	rm -f core $(EXE) *.o *.gch
	rm -f ../bin/linux/stide*
	rm -f ../bin/linux/*.png
