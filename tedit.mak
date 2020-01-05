#Fairly Universal Makefile
#

CC = gcc
CFLAGS = -m32 -O3 -Wall -g -DDEBUG -DEDITOR
LIBDIR = -L/usr/lib/i386-linux-gnu

ALLEGRO_LIBS = allegro-5 allegro_acodec-5 allegro_image-5 \
allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_font-5 \
allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 \
allegro_ttf-5 allegro_video-5

ALLEGRO_FLAGS = $(shell pkg-config --cflags --libs $(ALLEGRO_LIBS)) -lm -lphysfs -lc

INC = -I/usr/include/ -I/usr/include/i386-linux-gnu

ODIR = obj
SRCDIR = src

BINDIR = bin
OUTFILE = tedit

#Source files.
_SRC = \
%.c
########
SRC = $(patsubst %,$(SRCDIR)/%,$(_SRC))

#Headers.
_DEPS = \
tt_map.h \
tt_edit.h \
tt_thing.h \
jt_util.h
########
#DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

#Object files
_OBJ = \
tt_edit.o \
tt_map.o \
tt_thing.o \
jt_util.o
########
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

#Compile Objects from Source files
$(ODIR)/%.o: $(SRC) $(DEPS)
	$(CC) $(INC) -c -o $@ $< $(CFLAGS)

#Link Objects and output to executable
$(BINDIR)/$(OUTFILE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(ALLEGRO_FLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f $(BINDIR)/$(OUTFILE)