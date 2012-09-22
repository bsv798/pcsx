#
# PCSX Makefile for Linux
#

include Makefile.cfg

MAJ = 1
MIN = 5
VERSION = ${MAJ}.${MIN}

# Set to TRUE to compile with the old gtk1 gui
DISABLE_GTK2 = FALSE

all: pcsx

CPU = ix86

OPTIMIZE = -O2 -fomit-frame-pointer -finline-functions -ffast-math
FLAGS = -D__LINUX__ -DPCSX_VERSION=\"${VERSION}\" -DPACKAGE=\"pcsx\"
FLAGS+= -DENABLE_NLS

# this includes the option -rdynamic and we don't want that
ifeq (${DISABLE_GTK2}, FALSE)
	LIBST = $(shell pkg-config gtk+-2.0 --libs)
	LIBS = $(subst -rdynamic,,${LIBST}) -lz
else
	LIBST = $(shell gtk-config --libs)
	LIBS = $(subst -rdynamic,,${LIBST}) -lz
endif

OBJS = ../PsxBios.o ../CdRom.o ../PsxCounters.o ../PsxDma.o ../DisR3000A.o \
       ../Spu.o ../Sio.o ../PsxHw.o ../Mdec.o ../PsxMem.o ../Misc.o \
       ../plugins.o ../Decode_XA.o ../R3000A.o ../PsxInterpreter.o \
       ../Gte.o ../PsxHLE.o
OBJS+= LnxMain.o Plugin.o Config.o

ifeq (${DISABLE_GTK2}, FALSE)
	OBJS+= Gtk2Gui.o Glade2Gui.o Glade2Funcs.o
else
	OBJS+= GtkGui.o GladeGui.o GladeFuncs.o
endif

ifeq (${CPU}, ix86)
	ifneq (${PGCC}, )
		CC = ${PGCC}
	endif
	OPTIMIZE = -O4 -fomit-frame-pointer -finline-functions -ffast-math -fno-exceptions -march=pentiumpro
	OBJS+= ../ix86/iR3000A.o ../ix86/ix86.o
	FLAGS+= -D__i386__
endif

CFLAGS = -Wall ${OPTIMIZE} -I. -I.. ${FLAGS}
ifeq (${DISABLE_GTK2}, FALSE)
	CFLAGS+= $(shell pkg-config gtk+-2.0 --cflags)
else
	CFLAGS+= $(shell gtk-config --cflags)
endif

pcsx: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o pcsx ${LIBS}
	${STRIP} pcsx

.PHONY: clean pcsx pofile

clean:
	${RM} -f *.o ../*.o ../${CPU}/*.o pcsx

../%.o: ../%.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

../${CPU}/%.o: ../${CPU}/%.c
	${CC} ${CFLAGS} -c -o $@ $<

../ix86/iR3000A.o: ../ix86/iR3000A.c ../ix86/iGte.h
