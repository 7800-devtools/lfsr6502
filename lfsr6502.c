#define PROGNAME "lfsr6502 v0.1"
#define REPORTNAME "lfsr6502 LFSR Quality Analsys Report, v0.1"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>


int A, X, Y, CARRY, OVERFLOW, NEGATIVE, ZERO;
int randv[8];
int randvsave[8];
int iterations;
int outformat;
int algorithm;
int arguments;
int reportflag;
int bitmapflag;
long period;

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
void runriverraid16(void);

void generatebitmap(void);
void generatereport(void);
long getperiod(void);
int  detectseedbytes(void);
void saveseed(void);
void restoreseed(void);

void calcdeviation(void);
float calcvar, calcsdev, calccv;	

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
#define ALGRIVERRAID16     8
#define ALGEND             9

char *algorithmnames[10] = {
	"batari8",
	"batari8rev",
	"batari16",
	"pitfall8left",
	"pitfall8right",
	"xorshift16",
	"xhybrid24",
	"overlap24",
	"riverraid16"
	};

int main(int argc, char **argv)
{
    int t;
    iterations = -1;		// default is infinite loops
    algorithm = ALGBATARI8;	// default is batari8
    outformat = OUTHEXSPC;	// default is spaced hex output
    arguments = 0;
    reportflag = 0;
    bitmapflag = 0;
    period = 0;

    // default seed is 0x0101010101010101
    for (t=0;t<8;t++)
    	randv[t] = 1;

    processargs(argc, argv);

    if (!arguments)
        usage(argv[0]);

    if(bitmapflag)
        generatebitmap();

    if(reportflag)
        generatereport();

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

void runriverraid16(void)
{
    LDA (randv[1]);		//  LDA randv1
    ASL (A);			//  ASL
    ASL (A);			//  ASL
    ASL (A);			//  ASL
    EOR (randv[1]);		//  EOR randv1
    ASL (A);			//  ASL
    ROL (randv[0]);		//  ROL randv0
    ROL (randv[1]);		//  ROL randv1
    LDA (randv[1]);		//  LDA randv1
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
    case ALGRIVERRAID16:
	runriverraid16();
	break;
    }
}

void generatereport(void)
{
    int seedbytes;
    char underline[80];

    // Report title...
    memset(underline,'-',strlen(REPORTNAME));
    underline[strlen(REPORTNAME)]=0;
    printf("%s\n",REPORTNAME);
    printf("%s\n",underline);

    // LFSR being analysed...
    printf("LFSR:               %s (%d)\n",algorithmnames[algorithm],algorithm);

    // seed bytes are algorithmically confirmed rather than assumed...
    seedbytes = detectseedbytes();
    printf("Seed Bytes:         %d\n",seedbytes);

    period = getperiod();
    printf("Period:             %ld\n",period);


    calcdeviation();
    printf("Variance:           %f\n",calcvar);
    printf("Std Deviation:      %f\n",calcsdev);
    printf("Coeff of Variation: %f\n",calccv);

    /* 
	IDEAS for testing...
		-representation: identify missing or extra values
		-clumpiness: run standard deviation values on a few different
		 window sizes.
		-repeat above steps on the upper and lower nibbles
    */

    exit(0);
}

void calcdeviation(void)
{
    long i;
    float mean, sd, var, dev, sum = 0.0, sdev = 0.0, cv; 
 
    calcsdev = 0.0;

    period=getperiod();

    saveseed();

    for(i=0;i<period;i++)
    {
        runalgorithm();
        sum = sum + A;
    }

    mean = sum / period;

    restoreseed();

    for(i=0;i<period;i++)
    {
        runalgorithm();
        dev = (A - mean)*(A - mean);
        sdev = sdev + dev;
    }

    var = sdev / (period - 1);
    sd = sqrt(var);
    cv = (sd/mean)*100;

    calcsdev = sd;
    calccv = cv;
    calcvar = var;

    restoreseed();
}

int detectseedbytes(void)
{
	int origseed[8], usedbytes[8];
        int s,t, seedbytecount;

	// save the seed, init storage
	for (t=0;t<8;t++)
        {
		origseed[t]=randv[t];
                usedbytes[t]=0;
        }

	for(t=0;t<349;t++) // not exhastive
        {
            for(s=0;s<8;s++)
                usedbytes[s]=usedbytes[s]|(origseed[s]^randv[s]);
            runalgorithm();
        }

	// restore the original seed
	for (t=0;t<8;t++)
		randv[t]=origseed[t];

        for(s=0;s<8;s++)
        {
            if(usedbytes[s]!=0)
                seedbytecount++;
        }
        return(seedbytecount);
}

long getperiod(void)
{
	int origseed[8];
	int t;
	long locperiod;

	if (period>0)
		return(period);

	// To check the period, we don't actually need to look at the random 
	// sequence. We just need to find when the seed values match the
	// their starting values.

	// save the seed
	for (t=0;t<8;t++)
		origseed[t]=randv[t];


    	for (locperiod=0;locperiod<0xffffffffffff; locperiod++)
        {
            runalgorithm();
            for(t=0;t<8;t++)
            {
                if(origseed[t]!=randv[t])
                    break;
            }
            if(t==8)
                break;
	}

	// restore the seed
	for (t=0;t<8;t++)
		randv[t]=origseed[t];

        return(++locperiod);
}

void processargs(int argc, char **argv)
{
    int t, c;
    long seed;
    while ((c = getopt(argc, argv, ":a:i:o:s:hrb")) != -1)
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
	case 'r':
	    reportflag++;
            arguments++;
	    break;
	case 'b':
	    bitmapflag++;
            arguments++;
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

void saveseed(void)
{
	int t;
	for(t=0;t<8;t++)
		randvsave[t]=randv[t];
}
void restoreseed(void)
{
	int t;
	for(t=0;t<8;t++)
		randv[t]=randvsave[t];
}

void generatebitmap(void)
{
    period = getperiod();

    FILE *f;
    unsigned char *img = NULL;

    int w,h,bit,x,y;
    int r,g,b;

    if(period<256)
    {
        w=48; // 6x8 bits 
        h=48; 
    }
    else
    {
        w=640; // 80x8 bits
        h=640;
    }

    int filesize = 54 + 3*w*h;  //w is your image width, h is image height

    img = (unsigned char *)malloc(3*w*h);
    memset(img,0,3*w*h);

    saveseed();
    for(int j=0; j<h; j++)
    {
        for(int i=0; i<w; i=i+8)
        {
            runalgorithm();
            for(bit=0;bit<8;bit=bit+1)
            {
                x=i; y=(h-1)-j;
                r=255;g=255;b=255;
		if(A&0x80)
		{  
                    r=0;g=0;b=0;
                }
                img[(bit+x+y*w)*3+2] = (unsigned char)(r);
                img[(bit+x+y*w)*3+1] = (unsigned char)(g);
                img[(bit+x+y*w)*3+0] = (unsigned char)(b);
                A=A<<1;
            }
        }
    }

    restoreseed();

    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    unsigned char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (unsigned char)(filesize);
    bmpfileheader[ 3] = (unsigned char)(filesize>>8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(w);
    bmpinfoheader[ 5] = (unsigned char)(w>> 8);
    bmpinfoheader[ 6] = (unsigned char)(w>>16);
    bmpinfoheader[ 7] = (unsigned char)(w>>24);
    bmpinfoheader[ 8] = (unsigned char)(h);
    bmpinfoheader[ 9] = (unsigned char)(h>> 8);
    bmpinfoheader[10] = (unsigned char)(h>>16);
    bmpinfoheader[11] = (unsigned char)(h>>24);

    char filename[256];
    sprintf(filename,"%s.bmp",algorithmnames[algorithm]);

    f = fopen(filename,"wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for(int i=0; i<h; i++)
    {
        fwrite(img+(w*(h-i-1)*3),3,w,f);
        fwrite(bmppad,1,(4-(w*3)%4)%4,f);
    }

    free(img);
    fclose(f);
    exit(0);
}

void usage(char *programname)
{
    fprintf(stderr, "%s %s %s\n", PROGNAME, __DATE__, __TIME__);
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage: %s [-a #] [-i #] [-o #] [-s #] [-r] [-b]\n", programname);
    fprintf(stderr, "  -a #  specifies the lfsr algorithm:\n");
    fprintf(stderr, "        0 = batari8 [default]\n");
    fprintf(stderr, "        1 = batari8rev\n");
    fprintf(stderr, "        2 = batari16\n");
    fprintf(stderr, "        3 = pitfall8left\n");
    fprintf(stderr, "        4 = pitfall8right\n");
    fprintf(stderr, "        5 = xorshift16\n");
    fprintf(stderr, "        6 = xhybrid24\n");
    fprintf(stderr, "        7 = overlap24\n");
    fprintf(stderr, "        8 = riverraid16\n");
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
    fprintf(stderr, "  -b    generate a bitmap from the selected LFSR.\n");
    fprintf(stderr, "  -r    generate a quality report for the selected LFSR.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "examples:\n");
    fprintf(stderr, "  %s -a 5 -i 10 -o 1 -s 0xdb07\n", programname);
    fprintf(stderr, "    ^- runs xorshift16 with seed 0xdb07, and outputs 10 hex values.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  %s -a6 -o4 | dieharder -a -g 200\n", programname);
    fprintf(stderr, "    ^- run a dieharder evaluation on xhybrid24.\n");
    exit(1);
}
