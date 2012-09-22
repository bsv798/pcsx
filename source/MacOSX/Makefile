MAJ = 1
MIN = 4
VERSION = ${MAJ}.${MIN}

CFLAGS =  -g -Wall $(shell gtk-config --cflags)

# linux
#CPPFLAGS = -IRPC -I. -DPCSX_VERSION=\"${VERSION}\" -D__LINUX__
#LDFLAGS = -ldl $(shell gtk-config --libs) $(shell sdl-config --libs) -lz
#CPU = i386

# macosx
CPPFLAGS = -I$(HOME)/local/include -I/sw/include -IRPC -I. -DPCSX_VERSION=\"${VERSION}\" -D__MACOSX__
LDFLAGS = -framework Foundation -L/sw/lib -ldl $(shell gtk-config --libs) -lz
CPU = powerpc

# sources
srcs = CdRom.c Decode_XA.c DisR3000A.c Gte.c Mdec.c Misc.c PsxBios.c PsxCounters.c PsxDma.c PsxHLE.c PsxHw.c PsxInterpreter.c PsxMem.c R3000A.c Sio.c Spu.c plugins.c
srcs += Linux/Config.c Linux/LnxMain.c Linux/Plugin.c
srcs += Linux/GladeFuncs.c Linux/GladeGui.c Linux/GtkGui.c

# dynamic recompilation
CPPFLAGS += -DPSXREC
# linux
srcs += ix86/iR3000A.c ix86/ix86.c

# macosx
srcs += MacOSX/dlfcn_simple.c

objs := $(srcs:.c=.o)
deps := $(srcs:.c=.d)

all: pcsx

pcsx: $(objs)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


# -------------------------------------------------------------

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ -MD -MF $(patsubst %.o,%.d,$@)

# for macosx plugins
lib%.so: %.o
	$(CC) $(CFLAGS) -bundle -flat_namespace -undefined suppress -o $@ $^ $(LDFLAGS)

-include $(deps)

clean : 
	find . -name "*~" | xargs rm -f
	rm -f $(objs) $(deps) core*

distclean clear: clean
	rm -f pcsx

tar: clear
	(cd .. && tar cvzf PcsxSrc-macosx.tgz PcsxSrc-macosx)

