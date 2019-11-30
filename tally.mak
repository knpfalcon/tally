#Fairly Universal Makefile
#

CC = gcc
CFLAGS = -std=c99 -m32 -Wall -g -DDEBUG
LIBDIR = -L/usr/lib/i386-linux-gnu 

ALLEGRO_LIBS = allegro-5 allegro_acodec-5 allegro_image-5 \
allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_font-5 \
allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 \
allegro_ttf-5 allegro_video-5

ALLEGRO_FLAGS = $(shell pkg-config --cflags --libs $(ALLEGRO_LIBS)) -lm -lphysfs

INC = -I/usr/include/

ODIR = obj
SRCDIR = src

BINDIR = bin
OUTFILE = tally

#Source files.
_SRC = \
%.c
########
SRC = $(patsubst %,$(SRCDIR)/%,$(_SRC))

#Headers.
_DEPS = \
%.h
########
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

#Object files
_OBJ = \
tt_main.o \
tt_map.o \
tt_items.o \
tt_player.o \
jt_util.o \
tt_bullet.o \
tt_collision.o


########
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

#Compile Objects from Source files
$(ODIR)/%.o: $(SRC) $(DEPS)
	$(CC) $(INC) -c -o $@ $< $(CFLAGS) $(ALLEGRO_FLAGS)

#Link Objects and output to executable
$(BINDIR)/$(OUTFILE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(ALLEGRO_FLAGS) 

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f $(BINDIR)/$(OUTFILE)