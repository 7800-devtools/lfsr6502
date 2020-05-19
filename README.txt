lfsr6502 readme
---------------

lfsr6502 is a platform for analysis and experimentation with LFSR designs
for 6502 based systems.

While the utility comes with a few sample LFSRs implemented, it also has a
framework that allows you to easily add your own LFSR code. The LFSR C code 
largely looks and feels like 6502 assembly, which greatly reduces chances of 
translation errors between modern computers and the 6502.

Several output formats are supported for the LFSR data, including one suitable
for testing with the famed "dieharder" randomness test.

Running lfsr6502
----------------
lfsr6502 is a command-line utility. If run without arguments, it will output
it's usage information...

lfsr6502 v0.1 May 19 2020 18:10:02

Usage: ./lfsr6502 [-a #] [-i #] [-o #] [-s #] [-r] [-b]
  -a #  specifies the lfsr algorithm:
        0 = batari8 [default]
        1 = batari8rev
        2 = batari16
        3 = pitfall8left
        4 = pitfall8right
        5 = xorshift16
        6 = xhybrid24
        7 = overlap24
        8 = riverraid16
  -i #  specifies the number of iterations.
        use -1 for infinite output. [default]
  -o #  specifies the output format:
        0 = space separated hex values [default]
        1 = return separated hex values
        2 = space separated dec values
        3 = return separated dec values
        4 = raw character ouput
  -s #  specify a seed value. decimal assumed, unless hex notation is used.
        [default:0x0101010101010101]
  -b    generate a bitmap from the selected LFSR.
  -r    generate a quality report for the selected LFSR.

examples:
  ./lfsr6502 -a 5 -i 10 -o 1 -s 0xdb07
    ^- runs xorshift16 with seed 0xdb07, and outputs 10 hex values.

  ./lfsr6502 -a6 -o4 | dieharder -a -g 200
    ^- run a dieharder evaluation on xhybrid24.


The LFSR algorithms included with lfsr6502
------------------------------------------

batari8 (0)
	Type:         galois8 lfsr
	6502 author:  Fred Quimby
	RAM:          1 byte
	Size:         9 bytes
	Cycles:       11-12
	Period:       255
	References:   https://github.com/batari-Basic
	Notes: 	This is the rand routine included with the 2600 development
		language batari Basic. It's main strength is it's compactness
		and speed.
	Sample:       (2a) 15 be 5f 9b f9 c8 64 32 ...
	Code:
		  LDA randv0
		  LSR
		  BCC noeor
		    EOR #$B4
		noeor:
		  STA randv0


batari8rev (1)
	Type:         galois8 lfsr
	6502 author:  (forward) Fred Quimby, (reversal) Mike Saarna
	RAM:          1 byte
	Size:         9 bytes
	Cycles:       11-12
	Period:       255
	References:   https://github.com/batari-Basic
	Notes: 	This is a reversed version of the LFSR included with batari 
		Basic.  Reversal allows the pseudorandom sequence to be 
		rewound at any point, without requiring additional storage.
	Sample:       (32) 64 c8 f9 9b 5f be 15 2a ...
	Code:
		  LDA randv0
		  ASL
		  BCC noeor
		    EOR #$69
		noeor:
		  STA randv0


batari16 (2)
	Type:         galois8 extended to 16 bits
	6502 author:  Fred Quimby
	RAM:          2 bytes
	Size:         13 bytes
	Cycles:       19-20
	Period:       65535
	References:   https://github.com/batari-Basic
	Notes: 	This is the rand16 routine included with the 2600 development
		language batari Basic, and the default rand routine in 
		7800basic. Its quick and compact for a 16-bit LFSR.
	Sample:       (2a2a) 41 a3 e3 fd d2 d8 ba 19 ...
	Code:
		  LDA randv0
		  LSR
		  ROL randv1
		  BCC noeor
		    EOR #$B4
		noeor:
		  STA randv0
		  EOR randv1


pitfall8left (3)
	Type:         fibonacci8 lfsr
	6502 author:  David Crane
	RAM:          1 byte
	Size:         18 bytes
	Cycles:       30
	Period:       255
	References:   https://samiam.org/blog/20130606.html
	Notes:	This is the left-travelling LFSR from the Atari 2600 
		game Pitfall. It's at the core of the game's algorithmic 
		level generation.
	Sample:       (2a) 95 4a a5 52 29 14 8a 45 ...
	Code:
		  LDA randv0
		  ASL
		  EOR randv0
		  ASL
		  EOR randv0
		  ASL
		  ASL
		  ROL
		  EOR randv0
		  LSR
		  ROR randv0
		  LDA randv0


pitfall8right (4)
	Type:         fibonacci8 lfsr
	6502 author:  David Crane
	RAM:          1 byte
	Size:         17 bytes
	Cycles:       30
	Period:       255
	References:   https://samiam.org/blog/20130606.html
	Notes:	This is the right-travelling LFSR from the Atari 2600 
		game Pitfall. It's at the core of the game's algorithmic 
		level generation.
	Sample:       (45) 8a 14 29 52 a5 4a 95 2a ...
	Code:
		  LDA randv0
		  ASL
		  EOR randv0
		  ASL
		  EOR randv0
		  ASL
		  ASL
		  EOR randv0
		  ASL
		  ROL randv0
		  LDA randv0


xorshift16 (5)
	Type:         xorshift16 lfsr
	6502 author:  Veikko
	RAM:          2 bytes
	Size:         17 bytes
	Cycles:       30
	Period:       65535
	References:   http://www.retroprogramming.com/2017/07/xorshift-pseudorandom-numbers-in-z80.html
	Notes:	This is an efficient 6502 implementation of the xorshift16
		algorithm.
	Sample:       (2a2a) 0a ad 77 8b 50 37 06 75 ...
	Code:
		  LDA randv1
		  LSR
		  LDA randv0
		  ROR
		  EOR randv1
		  STA randv1
		  ROR
		  EOR randv0
		  STA randv0
		  EOR randv1
		  STA randv1


xhybrid24 (6)
	Type:         hybrid xorshift16+galois8
	6502 author:  (hybrid code) Mike Saarna, (base xorshift16) Veikko
	RAM:          3 bytes
	Size:         34 bytes
	Cycles:       48-50
	Period:       16711425
	References:   
	Notes:	A bit sampling on the xorshift16 upper-byte cranks a galois8
		lfsr forward. This results in an atypical lfsr period. 
		xhybrid24 passes and scores well on most "dieharder" 
		randomness quality checks. This level of rand quality probably 
		isn't needed on 6502 based systems, but it's nice that it can
		be done with relative efficiency.
	Sample:       (010101) f4 c5 d0 ae 75 6b 70 d2 ...
	Code:

		  BIT randv1
		  BVS skipupperlfsr
		    LDA randv2
		    LSR
		    BCC skipuppereor
		      EOR #$B4
		skipuppereor:
		    STA randv2
		skipupperlfsr:

		  LDA randv1
		  LSR
		  LDA randv0
		  ROR
		  EOR randv1
		  STA randv1
		  ROR
		  EOR randv0
		  STA randv0
		  EOR randv1
		  STA randv1

		  EOR randv2


overlap24 (7)
	Type:         ??
	6502 author:  ??
	RAM:          3 bytes
	Size:         38 bytes
	Cycles:       73
	Period:       16777215
	References:   https://wiki.nesdev.com/w/index.php/Random_number_generator/Linear_feedback_shift_register_(advanced)
	Notes:	Has an XOR-feedback that contains only four bits, but by 
		shifting and feeding back 8 bits at once, a more complex 
		overlapped result is obtained.
	Sample:       (010203) 1b 36 2d 45 8a d4 81 43 ...
	Code:
		  LDY randv1
		  LDA randv2
		  LSR
		  LSR
		  LSR
		  LSR
		  STA randv1
		  LSR
		  LSR
		  EOR randv1
		  LSR
		  EOR randv1
		  EOR randv0
		  STA randv1
		  LDA randv2
		  ASL
		  EOR randv2
		  ASL
		  ASL
		  EOR randv2
		  ASL
		  EOR randv2
		  STY randv2
		  STA randv0

riverraid16 (8)
	Type:         ??
	6502 author:  Carol Shaw
	RAM:          2 bytes
	Size:         14 bytes
	Cycles:       27
	Period:       57337
	References:   http://www.bjars.com/source/RiverRaid.asm
	Notes:	"randomHi is very random, randomLo is NOT when more than one
		bit is used, because: randomLo[x+1] = randomLo[x]*2 + 0/1, but
		randomLo is used more often [in the game] [...]" 
			--Thomas Jentzsch
	Sample:       (a814) 0a 05 b6 5b 99 f8 7c 3e ...
	Code:
		  LDA randv1
		  ASL
		  ASL
		  ASL
		  EOR randv1
		  ASL
		  ROL randv0
		  ROL randv1
		  LDA randv1


Legal Stuff
-----------
lfsr6502 is created by Mike Saarna, copyright 2020. It's provided here under
the GPL v2 license. 6502 LFSR code is copyright it's respective authors, and
not licensed under the GPL. See the included LICENSE.txt file for GPL details.
