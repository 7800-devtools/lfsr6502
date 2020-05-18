#define PROGNAME "lfsr6502 v0.1"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int A, X, Y, CARRY, OVERFLOW, NEGATIVE, ZERO;
int randv[8];
int iterations;
int outformat;
int algorithm;
int arguments;
int periodcheck;

void BIT(int val);
void CMP(int val);
void CPX(int val);
void CPY(int val);

#define BEQ if(ZERO) goto 
#define BNE if(!ZERO) goto 
#define BVS if(OVERFLOW) goto 
#define BVC if(!OVERFLOW) goto 
#define BCS if(CARRY) goto 
#define BCC if(!CARRY) goto 

#define LDA A = 
#define LDY Y = 
#define LDX X = 
#define STA(a) (a) = A
#define STX(a) (a) = X
#define STY(a) (a) = Y

// We wrap these these functions in a macro, to avoid having some "opcodes"
// with pointers, and others without, to look more like 6502 assembly.
void doLSR(int *val);
void doASL(int *val);
void doROR(int *val);
void doROL(int *val);
#define ROR(a) doROR(&a)
#define ROL(a) doROL(&a)
#define ASL(a) doASL(&a)
#define LSR(a) doLSR(&a)

#define TAX X = A
#define TXA A = Y
#define TAY Y = A
#define TYA A = Y
#define EOR A = A ^ 
#define ORA A = A | 
#define AND A = A & 

void usage(char *name);
void outputbyte(int A);
void processargs(int argc, char **argv);

void runalgorithm(void);
void runbatari8(void);
void runbatari8rev(void);
void runbatari16(void);
void runpitfall8left(void);
void runpitfall8right(void);
void runxorshift16(void);
void runxhybrid24(void);
void runoverlap24(void);

#define OUTHEXSPC 0
#define OUTHEXRET 1
#define OUTDECSPC 2
#define OUTDECRET 3
#define OUTRAW    4
#define OUTEND    5

#define ALGBATARI8         0
#define ALGBATARI8REV      1
#define ALGBATARI16        2
#define ALGPITFALL8LEFT    3
#define ALGPITFALL8RIGHT   4
#define ALGXORSHIFT16      5
#define ALGXHYBRID24       6
#define ALGOVERLAP24       7
#define ALGEND             8

int main(int argc, char **argv)
{
    int t;
    long period = 0;
    iterations = -1;		// default is infinite loops
    algorithm = ALGBATARI8;	// default is batari8
    outformat = OUTHEXSPC;	// default is spaced hex output
    arguments = 0;
    periodcheck = 0;

    // default seed is 0x0101010101010101
    for (t=0;t<8;t++)
    	randv[t] = 1;

    processargs(argc, argv);

    if (!arguments)
        usage(argv[0]);

    if(periodcheck)
    {
        int s;
        int sample[16];
        int compare[16];
        printf("Searching for period...\n");
    	for (period = 0;period<16; period++)
        {
            runalgorithm();
            sample[period]=A;
            compare[period]=A;
        }
    	for (;; period++)
        {
            runalgorithm();
            for(s=0;s<15;s++)
                compare[s]=compare[s+1];
            compare[15]=A;
            for(s=0;s<16;s++)
                if(sample[s]!=compare[s])
                    break;
            if(s==16)
                break;
	}
        printf("Period found: %ld\n",period-15);
        exit(0);
    }

    for (t = 0; (iterations < 0) || (t < iterations); t++)
    {
	runalgorithm();
	outputbyte(A);
    }

}

void runbatari8(void)
{
    // galois8 lfsr
    // 6502 implementation by Fred Quimby
    // ram    : 1 byte
    // cycles : 11-12
    // size   : 9 bytes
    // notes  : this is the "rand" routine in batari Basic.

    LDA (randv[0]);		//  LDA randv0
    LSR (A);			//  LSR
    BCC noeor;			//  BCC noeor
    EOR (0xB4);			//    EOR #$B4
  noeor:			//noeor  
    STA (randv[0]);		//  STA randv0
}

void runbatari8rev(void)
{
    // galois8 lfsr
    // 6502 implementation by Fred Quimby
    // reversal by Mike Saarna
    // ram    : 1 byte
    // cycles : 11-12
    // size   : 9 bytes
    // notes  : this is the "rand" routine in batari Basic, run backwards

    LDA (randv[0]);		//  LDA randv0
    ASL (A);			//  ASL
    BCC noeor;			//  BCC noeor
    EOR (0x69);			//    EOR #$69
  noeor:			//noeor  
    STA (randv[0]);		//  STA randv0
}


void runbatari16(void)
{
    // galois16 lfsr
    // 6502 implementation by Fred Quimby
    // ram    : 2 bytes
    // cycles : 19-20
    // size   : 13 bytes
    // notes  : this is "rand16" routine in batari Basic, 
    //          and the default rand routine in 7800basic

    LDA (randv[0]);		//  LDA randv0
    LSR (A);			//  LSR
    ROL (randv[1]);		//  ROL randv1
    BCC noeor;			//  BCC noeor
    EOR (0xB4);			//    EOR #$B4
  noeor:			//noeor  
    STA (randv[0]);		//  STA randv0
    EOR (randv[1]);		//  EOR randv1
}

void runpitfall8left(void)
{
    // fibonacci8 lfsr
    // 6502 implementation by David Crane
    // ram    : 1 byte
    // cycles : 30
    // size   : 16 bytes
    // notes  : this is the lfsr run when pitfall harry goes left

    LDA (randv[0]);		//  LDA randv0
    ASL (A);			//  ASL
    EOR (randv[0]);		//  EOR randv0
    ASL (A);			//  ASL
    EOR (randv[0]);		//  EOR randv0
    ASL (A);			//  ASL
    ASL (A);			//  ASL
    ROL (A);			//  ROL
    EOR (randv[0]);		//  EOR randv0
    LSR (A);			//  LSR
    ROR (randv[0]);		//  ROR randv0
    LDA (randv[0]);		//  LDA randv0
}

void runpitfall8right(void)
{
    // fibonacci8 lfsr
    // 6502 implementation by David Crane
    // ram    : 1 byte
    // cycles : 29
    // size   : 16 bytes
    // notes  : this is the lfsr run when pitfall harry goes right

    LDA (randv[0]);		//  LDA randv0
    ASL (A);			//  ASL
    EOR (randv[0]);		//  EOR randv0
    ASL (A);			//  ASL
    EOR (randv[0]);		//  EOR randv0
    ASL (A);			//  ASL
    ASL (A);			//  ASL
    EOR (randv[0]);		//  EOR randv0
    ASL (A);			//  ASL
    ROL (randv[0]);		//  ROL randv0
    LDA (randv[0]);		//  LDA randv0
}


void runxorshift16(void)
{
    // xorshift16 lfsr
    // 6502 implementation by veikko
    // ram    : 2 bytes
    // cycles : 30
    // size   : 19 bytes

    LDA (randv[1]);		//  LDA randv1
    LSR (A);			//  LSR
    LDA (randv[0]);		//  LDA randv0
    ROR (A);			//  ROR
    EOR (randv[1]);		//  EOR randv1
    STA (randv[1]);		//  STA randv1
    ROR (A);			//  ROR
    EOR (randv[0]);		//  EOR randv0
    STA (randv[0]);		//  STA randv0
    EOR (randv[1]);		//  EOR randv1
    STA (randv[1]);		//  STA randv1

}

void runxhybrid24(void)
{

    BIT (randv[1]);		//  BIT randv1
    BVS skipupperlfsr;		//  BVS skipupperlfsr
    LDA (randv[2]);		//  LDA randv2
    LSR (A);			//  LSR
    BCC skipuppereor;		//  BCC skipuppereor
    EOR (0xB4);			//  EOR #$B4
skipuppereor:			//skipuppereor
    STA (randv[2]);		//  STA randv2
skipupperlfsr:			//skipupperlfsr

    LDA (randv[1]);		//  LDA randv1
    LSR (A);			//  LSR
    LDA (randv[0]);		//  LDA randv0
    ROR (A);			//  ROR
    EOR (randv[1]);		//  EOR randv1
    STA (randv[1]);		//  STA randv1
    ROR (A);			//  ROR
    EOR (randv[0]);		//  EOR randv0
    STA (randv[0]);		//  STA randv0
    EOR (randv[1]);		//  EOR randv1
    STA (randv[1]);		//  STA randv1

    EOR (randv[2]);		//  EOR randv2

}

void runoverlap24(void)
{
    LDY (randv[1]);		//  ldy randv1 
    LDA (randv[2]);		//  lda randv2
    LSR (A);			//  lsr
    LSR (A);			//  lsr
    LSR (A);			//  lsr
    LSR (A);			//  lsr
    STA (randv[1]);		//  sta randv1
    LSR (A);			//  lsr
    LSR (A);			//  lsr
    EOR (randv[1]);		//  eor randv1
    LSR (A);			//  lsr
    EOR (randv[1]);		//  eor randv1
    EOR (randv[0]);		//  eor randv0
    STA (randv[1]);		//  sta randv1
    LDA (randv[2]);		//  lda randv2
    ASL (A);			//  asl
    EOR (randv[2]);		//  eor randv2
    ASL (A);			//  asl
    ASL (A);			//  asl
    EOR (randv[2]);		//  eor randv2
    ASL (A);			//  asl
    EOR (randv[2]);		//  eor randv2
    STY (randv[2]);		//  sty randv2 
    STA (randv[0]);		//  sta randv0
}

void runalgorithm(void)
{
    switch (algorithm)
    {
    case ALGBATARI8:
	runbatari8();
	break;
    case ALGBATARI8REV:
	runbatari8rev();
	break;
    case ALGBATARI16:
	runbatari16();
	break;
    case ALGPITFALL8LEFT:
	runpitfall8left();
	break;
    case ALGPITFALL8RIGHT:
	runpitfall8right();
	break;
    case ALGXORSHIFT16:
	runxorshift16();
	break;
    case ALGXHYBRID24:
	runxhybrid24();
	break;
    case ALGOVERLAP24:
	runoverlap24();
	break;
    }
}


void processargs(int argc, char **argv)
{
    int t, c;
    long seed;
    while ((c = getopt(argc, argv, ":a:i:o:s:hp")) != -1)
    {
	switch (c)
	{
	case 'a':
	    algorithm = strtol(optarg, NULL, 10);
	    if (algorithm >= ALGEND)
	    {
		fprintf(stderr, "ERR: usupported algorithm.\n");
		exit(1);
	    }
            arguments++;
	    break;
	case 'i':
	    iterations = strtol(optarg, NULL, 10);
            arguments++;
	    break;
	case 'o':
	    outformat = strtol(optarg, NULL, 10);
	    if (outformat >= OUTEND)
	    {
		fprintf(stderr, "ERR: usupported output format\n");
		exit(1);
	    }
            arguments++;
	    break;

	case 's':
	    if (strncmp(optarg, "0x", 2) == 0)
		seed = strtol(optarg + 2, NULL, 16);
	    else if (optarg[0] == '$')
		seed = strtol(optarg + 1, NULL, 16);
	    else
		seed = strtol(optarg, NULL, 10);
	    for (t = 0; t < 8; t++)
	    {
		randv[t] = seed & 0xff;
		seed = seed >> 8;
	    }
            arguments++;
	    break;
	case 'h':
	    usage(argv[0]);
	    break;
	case 'p':
	    periodcheck++;
	    break;
	default:
	    fprintf(stderr, "ERR: usupported argument\n");
	    exit(1);
	    break;
	}
    }

}

void outputbyte(int val)
{
    switch (outformat)
    {
    case OUTRAW:
	printf("%c", val);
	break;
    case OUTHEXSPC:
	printf("%02x ", val);
	break;
    case OUTHEXRET:
	printf("%02x\n", val);
	break;
    case OUTDECSPC:
	printf("%d ", val);
	break;
    case OUTDECRET:
	printf("%d\n", val);
	break;
    }
}

inline void BIT(int VAL)
{
    OVERFLOW = (VAL & 0x40) >> 6;
    NEGATIVE = (VAL & 0x80) >> 7;
    ZERO = VAL & A;
}

inline void CMP(int VAL)
{
	int result;
	ZERO = 0; NEGATIVE = 0; CARRY = 0;
	result = A - VAL;
	if(result==0)
		ZERO = 1;
	if(result<0)
		NEGATIVE = 1;
	if(result>=0)
		CARRY = 1;
}

inline void CPX(int VAL)
{
	int result;
	ZERO = 0; NEGATIVE = 0; CARRY = 0;
	result = X - VAL;
	if(result==0)
		ZERO = 1;
	if(result<0)
		NEGATIVE = 1;
	if(result>=0)
		CARRY = 1;
}

inline void CPY(int VAL)
{
	int result;
	ZERO = 0; NEGATIVE = 0; CARRY = 0;
	result = Y - VAL;
	if(result==0)
		ZERO = 1;
	if(result<0)
		NEGATIVE = 1;
	if(result>=0)
		CARRY = 1;
}

inline void doASL(int *VAL)
{
    CARRY = (*VAL & 0x80) >> 7; // CARRY is always 0 or 1
    *VAL = (*VAL << 1) & 0xff;
}

inline void doROL(int *VAL)
{
    *VAL = (*VAL << 1) | CARRY;
    CARRY = (*VAL & 0x100) >> 8; // CARRY is always 0 or 1
    *VAL = *VAL & 0xff;
}

inline void doLSR(int *VAL)
{
    CARRY = *VAL & 1;
    *VAL = *VAL >> 1;
}

inline void doROR(int *VAL)
{
    int savecarry;
    savecarry = CARRY; // save the bit that will be carried
    CARRY = *VAL & 1;
    *VAL = *VAL >> 1;
    if (savecarry)
	*VAL = *VAL | 0x80;
}

void usage(char *programname)
{
    fprintf(stderr, "%s %s %s\n", PROGNAME, __DATE__, __TIME__);
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage: %s [-a #] [-i #] [-o #] [-s #]\n", programname);
    fprintf(stderr, "  -a #  specifies the lfsr algorithm:\n");
    fprintf(stderr, "        0 = batari8 [default]\n");
    fprintf(stderr, "        1 = batari8rev\n");
    fprintf(stderr, "        2 = batari16\n");
    fprintf(stderr, "        3 = pitfall8left\n");
    fprintf(stderr, "        4 = pitfall8right\n");
    fprintf(stderr, "        5 = xorshift16\n");
    fprintf(stderr, "        6 = xhybrid24\n");
    fprintf(stderr, "        7 = overlap24\n");
    fprintf(stderr, "  -i #  specifies the number of iterations.\n");
    fprintf(stderr, "        use -1 for infinite output. [default]\n");
    fprintf(stderr, "  -o #  specifies the output format:\n");
    fprintf(stderr, "        0 = space separated hex values [default]\n");
    fprintf(stderr, "        1 = return separated hex values\n");
    fprintf(stderr, "        2 = space separated dec values\n");
    fprintf(stderr, "        3 = return separated dec values\n");
    fprintf(stderr, "        4 = raw character ouput\n");
    fprintf(stderr, "  -s #  specify a seed value. decimal assumed, unless hex notation is used.\n");
    fprintf(stderr, "        [default:0x0101010101010101]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "examples:\n");
    fprintf(stderr, "  %s -a 5 -i 10 -o 1 -s 0xdb07\n", programname);
    fprintf(stderr, "    ^- runs xorshift16 with seed 0xdb07, and outputs 10 hex values.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  %s -a6 -o4 | dieharder -a -g 200\n", programname);
    fprintf(stderr, "    ^- run a dieharder evaluation on xhybrid24.\n");
    exit(1);
}
