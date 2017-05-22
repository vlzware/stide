# Makefile for HIPS

# compiler to use
#CC = clang
CC = gcc

# flags to pass compiler
#CFLAGS = -ggdb3 -O0 -Qunused-arguments -std=c11 -Wall -Werror
CFLAGS = -ggdb3 -O0 -std=c11 -Wall -Werror

# name for executable
EXE_C = hips_c
EXE_E = hips_e

# space-separated list of header files
HDRS = bmp.h helpers.h

# space-separated list of libraries, if any,
# each of which should be prefixed with -l
LIBS = -l sqlite3

# space-separated list of source files
SRCS_C = hips_c.c helpers.c
SRCS_E = hips_e.c helpers.c

# automatically generated list of object files
OBJS_C = $(SRCS_C:.c=.o)
OBJS_E = $(SRCS_E:.c=.o)


# default target
all: hips_c hips_e

hips_c: $(OBJS_C) $(HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(OBJS_C) $(LIBS)

hips_e: $(OBJS_E) $(HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(OBJS_E) $(LIBS)

# dependencies 
$(OBJS_C): $(HDRS) Makefile
$(OBJS_E): $(HDRS) Makefile

# housekeeping
clean:
	rm -f core $(EXE_C) *.o
	rm -f core $(EXE_E) *.o
