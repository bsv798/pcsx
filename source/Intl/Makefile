#
# PCSX Makefile for Linux
#

all: pofile

SRCS = ../*.c ../ix86/*.c ../Linux/*.c ../Win32/*.c

pofile:
	xgettext --keyword=N_ --keyword=_ --output=pcsx.po ${SRCS}

.PHONY: pofile

