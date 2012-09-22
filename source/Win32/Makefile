#
# PCSX Makefile for Win32-mingw
#

MAJ = 1
MIN = 5
VERSION = ${MAJ}.${MIN}

all: pcsx

CPU = ix86
# CPUTYPE=k6-2

CC = gcc
NASM = nasm
RM = rm -f
STRIP = strip
RC = brcc32
RC2 = windres

ifndef CPUTYPE
	CPUOPT = -mcpu=pentiumpro
else
	CPUOPT = -march=${CPUTYPE}
endif
# http://gcc.gnu.org/onlinedocs/gcc-3.1.1/gcc/i386-and-x86-64-Options.html
#  The choices for cpu-type are i386, i486, i586, i686, pentium,
#  pentium-mmx, pentiumpro, pentium2, pentium3, pentium4, k6, k6-2,
#  k6-3, athlon, athlon-tbird, athlon-4, athlon-xp and athlon-mp.
#
#  While picking a specific cpu-type will schedule things
#  appropriately for that particular chip, the compiler will not
#  generate any code that does not run on the i386 without the
#  -march=cpu-type option being used. i586 is equivalent to
#  pentium and i686 is equivalent to pentiumpro.

OPTIMIZE = -O2 -fomit-frame-pointer -finline-functions -ffast-math
FLAGS = -D__WIN32__ -D__MINGW32__ -DPCSX_VERSION=\"${VERSION}\"
RC1FLAGS = -d__MINGW32__
LIBS = -lz -lcomctl32
RESOBJ = pcsxres.o
OBJS = ../PsxBios.o ../CdRom.o ../PsxCounters.o ../PsxDma.o ../DisR3000A.o \
       ../Spu.o ../Sio.o ../PsxHw.o ../Mdec.o ../PsxMem.o ../Misc.o \
       ../plugins.o ../Decode_XA.o ../R3000A.o ../PsxInterpreter.o \
       ../Gte.o ../PsxHLE.o
OBJS+= WndMain.o Plugin.o ConfigurePlugins.o AboutDlg.o ${RESOBJ}

ifeq (${CPU}, ix86)
	CC = gcc
	OPTIMIZE = -O4 -fomit-frame-pointer -finline-functions -ffast-math -fno-exceptions
	OPTIMIZE += ${CPUOPT}
	OBJS+= ../ix86/iR3000A.o ../ix86/ix86.o
	FLAGS+= -D__i386__
endif
CFLAGS = -Wall ${OPTIMIZE} -I. -I.. ${FLAGS} -mwindows

ASMFLAGS = -f elf ${FLAGS} -i./ -i../

pcsx: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o pcsx.exe ${LIBS}
	${STRIP} pcsx.exe

.PHONY: clean pcsx

clean:
	-${RM} *.o
	-${RM} *.res
	-${RM} ../*.o
	-${RM} ../${CPU}/*.o
	-${RM} pcsx.exe

../%.o: ../%.c
	${CC} ${CFLAGS} -c -o $@ $<

../${CPU}/%.o: ../${CPU}/%.asm
	${NASM} ${ASMFLAGS} -o $@ $<

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

../Cpu/ix86/%.o: ../Cpu/ix86/%.c
	${CC} ${CFLAGS} -c -o $@ $<

${RESOBJ}: pcsx.rc
	${RC} $< ${RC1FLAGS} -fo$*.res
	${RC2} -I res -O coff -o $@ -i $*.res


