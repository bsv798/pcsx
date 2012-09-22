#
# KallistiGL test program
# (c)2001 Benoit Miller
#   

TARGET = pcsx.elf
OBJS = ../PsxBios.o ../Decode_XA.o ../Mdec.o plugins.o ../Spu.o \
../CdRom.o ../DisR3000A.o ../PsxHw.o ../PsxMem.o ../R3000A.o ../PsxHLE.o \
../PsxCounters.o ../PsxDma.o ../PsxInterpreter.o ../Misc.o ../Sio.o \
DcastMain.o Plugin.o PlugCD.o PlugSPU.o PlugPAD.o Compat.o \
../sh4/sh4.o ../sh4/sR3000A.o ../sh4/sGte.o ../sh4/Gte.o texture.o \
3dutils.o gui.o filelst.o ConfigGUI.o
# PlugGPU.o

MAJ = 1
MIN = 3
VERSION = ${MAJ}.${MIN}

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

KOS_CFLAGS += -Wcast-align -D__DREAMCAST__ -DPCSX_VERSION=\"${VERSION}\" -DLOG_STDOUT -I. -I.. -I../Cpu/sh4

clean:
	-rm -f $(TARGET) $(OBJS) romdisk.*

rm-elf:
	-rm -f $(TARGET) romdisk.*

$(TARGET): $(OBJS) romdisk.o
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) \
		$(OBJS) romdisk.o $(OBJEXTRA) -L$(KOS_BASE)/lib -lgpu -lz -lgl -lpng -ldcutils -lm $(KOS_LIBS)

romdisk.img:
	$(KOS_GENROMFS) -f romdisk.img -d romdisk -v

romdisk.o: romdisk.img
	$(KOS_BASE)/utils/bin2o/bin2o romdisk.img romdisk romdisk.o

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist:
	rm -f $(OBJS) romdisk.o romdisk.img
	$(KOS_STRIP) $(TARGET)

