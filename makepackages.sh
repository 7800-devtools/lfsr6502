#!/bin/sh
# makepackages.sh
#   apply the release.dat contents to the release text in various sources
#   and documents, and then generate the individual release packages.

RELEASE=$(cat release.dat)
ERELEASE=$(cat release.dat | sed 's/ /_/g')
YEAR=$(date +%Y)

dos2unix *.txt >/dev/null 2>&1
cat lfsr6502.c | sed 's/define PROGNAME .*/define PROGNAME "lfsr6502 v'"$RELEASE"'"/g' > lfsr6502.c.new
mv lfsr6502.c.new lfsr6502.c
unix2dos lfsr6502.c >/dev/null 2>&1

# cleanup
find . -name .\*.swp -exec rm '{}' \;
rm -fr packages
mkdir packages
make dist
cd packages

for OSARCH in linux@Linux osx@Darwin win@Windows ; do
        for BITS in x64 x86 ; do
                OS=$(echo $OSARCH | cut -d@ -f1)
                ARCH=$(echo $OSARCH| cut -d@ -f2)
		rm -fr lfsr6502
		mkdir lfsr6502
		cp ../*.txt lfsr6502/
		if [ "$OS" = win ] ; then
			cp ../lfsr6502.Win32."$BITS".exe lfsr6502/lfsr6502.exe
			zip -r lfsr6502-$ERELEASE-$OS-$BITS.zip lfsr6502
		else
			cp ../lfsr6502."$ARCH"."$BITS" lfsr6502/lfsr6502
			tar --numeric-owner -cvzf lfsr6502-$ERELEASE-$OS-$BITS.tar.gz lfsr6502
		fi
		rm -fr lfsr6502
	done
done

cd ..		
dos2unix *.txt lfsr6502.c >/dev/null 2>&1
