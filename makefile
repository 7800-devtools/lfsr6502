CC     = gcc
CFLAGS = -Wall -g -O0
 
all: lfsr6502
 
lfsr6502: lfsr6502.c
	$(CC) $(CFLAGS) lfsr6502.c -o $@ 
 
clean:
	rm -f lfsr6502

dist:
	make clean
	make distclean
	make -f makefile.xcmp.win-x86
	make -f makefile.xcmp.win-x64
	make -f makefile.linux-x86
	make -f makefile.linux-x64
	make -f makefile.xcmp.osx-x86
	make -f makefile.xcmp.osx-x64
	unix2dos *.txt *.c *.h

distclean:
	make -f makefile.xcmp.win-x86 clean
	make -f makefile.xcmp.win-x64 clean
	make -f makefile.linux-x86 clean
	make -f makefile.linux-x64 clean
	make -f makefile.xcmp.osx-x86 clean
	make -f makefile.xcmp.osx-x64 clean

