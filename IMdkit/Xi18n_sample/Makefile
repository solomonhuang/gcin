

CC = cc

CFLAGS = -g   
INC = -I../include -I/usr/X11R6/include
SRC = IC.c  sampleIM.c
OBJ = $(SRC:.c=.o)
LIB = ../lib/libXimd.a -L/usr/X11R6/lib -lXext -lX11

all: sampleIM

sampleIM: $(OBJ)
	$(CC) -o sampleIM $(OBJ) $(LIB)

.c.o:
	$(CC) -c $(CFLAGS) $(INC) $<

install:
	@echo nothing to be installed.

clean:
	rm -f sampleIM *.o *.o *.bak

