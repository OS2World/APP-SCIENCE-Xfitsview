#-----------------------------------------------------------------------
#  Copyright (C) 1996,1997,1998
#  Associated Universities, Inc. Washington DC, USA.
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of
#  the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, write to the Free
#  Software Foundation, Inc., 675 Massachusetts Ave, Cambridge,
#  MA 02139, USA.
#-----------------------------------------------------------------------
CC = -gcc -m486 -O3 -c

TARGETS= libFITS.a

all:  $(TARGETS)

libFITS.a: axisdesc.o fmem.o imag.o matx.o string.o dskstore.o fwindow.o imagdesc.o matxdesc.o zsubs.o fitsio.o ifitshed.o infoelem.o matxpos.o fitsmem.o ifitsstr.o infolist.o myutil.o gzipread.o position.o dsssubs.o precess.o wpos.o histo.o
	$(ar s ) libFITS.a
 
axisdesc.o: axisdesc.c axisdesc.h mydefs.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) axisdesc.c
	ar r libFITS.a axisdesc.o

dskstore.o: dskstore.c dskstore.h fitsio.h mydefs.h myio.h mystring.h myutil.h zconvrt.h
	$(CC) -c $(CFLAGS) dskstore.c
	ar r libFITS.a dskstore.o

fitsio.o: fitsio.c fitsio.h mydefs.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) fitsio.c
	ar r libFITS.a fitsio.o

fitsmem.o: fitsmem.c fitsmem.h mydefs.h
	$(CC) -c $(CFLAGS) fitsmem.c
	ar r libFITS.a fitsmem.o

fmem.o: fmem.c fmem.h fitsmem.h mydefs.h
	$(CC) -c $(CFLAGS) fmem.c
	ar r libFITS.a fmem.o

fwindow.o: fwindow.c fwindow.h dskstore.h fmem.h mydefs.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) fwindow.c
	ar r libFITS.a fwindow.o

ifitshed.o: ifitshed.c ifitshed.h fitsio.h mydefs.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) ifitshed.c
	ar r libFITS.a ifitshed.o

ifitsstr.o: ifitsstr.c ifitsstr.h fitsio.h mydefs.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) ifitsstr.c
	ar r libFITS.a ifitsstr.o

imag.o: imag.c imag.h mydefs.h dskstore.h fmem.h matx.h imagdesc.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) imag.c
	ar r libFITS.a imag.o

imagdesc.o: imagdesc.c imagdesc.h mydefs.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) imagdesc.c
	ar r libFITS.a imagdesc.o

infoelem.o: infoelem.c infoelem.h mydefs.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) infoelem.c
	ar r libFITS.a infoelem.o

infolist.o: infolist.c infolist.h infoelem.h mydefs.h mystring.h
	$(CC) -c $(CFLAGS) infolist.c
	ar r libFITS.a infolist.o

matx.o: matx.c matx.h mydefs.h dskstore.h fmem.h matxdesc.h matxpos.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) matx.c
	ar r libFITS.a matx.o

matxdesc.o: matxdesc.c matxdesc.h mydefs.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) matxdesc.c
	ar r libFITS.a matxdesc.o

matxpos.o: matxpos.c matxpos.h mydefs.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) matxpos.c
	ar r libFITS.a matxpos.o

myutil.o: myutil.c myutil.h mydefs.h myio.h mystring.h
	$(CC) -c $(CFLAGS) myutil.c
	ar r libFITS.a myutil.o

string.o: string.c mystring.h mydefs.h
	$(CC) -c $(CFLAGS) string.c
	ar r libFITS.a string.o

zsubs.o: zsubs.c zsubs.h mydefs.h myio.h mystring.h myutil.h
	$(CC) -c $(CFLAGS) zsubs.c
	ar r libFITS.a zsubs.o

gzipread.o: gzipread.c gzipread.h gziputil.h fitsmem.h fitsio.h
	$(CC) -c $(CFLAGS) gzipread.c
	ar r libFITS.a gzipread.o

position.o: position.c position.h precess.h dsssubs.h
	$(CC) -c $(CFLAGS) position.c
	ar r libFITS.a position.o

dsssubs.o: dsssubs.c dsssubs.h
	$(CC) -c $(CFLAGS) dsssubs.c
	ar r libFITS.a dsssubs.o

precess.o: precess.c precess.h
	$(CC) -c $(CFLAGS) precess.c
	ar r libFITS.a precess.o

wpos.o: wpos.c wpos.h position.h precess.h dsssubs.h
	$(CC) -c $(CFLAGS) wpos.c
	ar r libFITS.a wpos.o

histo.o: histo.c histo.h
	$(CC) -c $(CFLAGS) histo.c
	ar r libFITS.a histo.o

clobber:
	rm -f *.o *~* *.a 
	rm -f myconfig.h
	rm -f $(TARGETS)



