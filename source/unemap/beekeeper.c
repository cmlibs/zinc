/*******************************************************************************
FILE : beekeeper.c

LAST MODIFIED : 18 March 2001

DESCRIPTION :
Functions for reading Beekeeper data files (EEG signals).  The format was
determined from BEEKEEPER_README and B2A_SOURCE that were provided by Peng.

There are 4 types of Beekeeper data files
	DTM.  64 channel, 200 Hz
	CTE 64 channel.  64 channel, 200 Hz
	CTE 128 channel.  128 channel, 200 Hz
	CTE 64 channel 400 Hz.  64 channel, 400 Hz
For unemap, the first two are the same.  The signal data is in records, the
records differ between 64 channel and 128 channel.  There is information other
than signal data records in the files.

In SAMPLE.EEG there are 2826 valid CTE64 records and no valid records for the
other types.  Characters in CTE64 records = 2826*106 = 299556 (SAMPLE.EEG has
300000 characters).
==============================================================================*/

/* does linear interpolation to fill in gaps in data so that times are evenly
	spaced */
#define FILL_IN_DATA_GAPS

/******************************************************************************/
#if defined (BEEKEEPER_README)
B2MICRO.EXE reads Beekeeper data files and converts the data to floating
point values in microvolts.  Syntax is:

B2MICRO [drive][path]filename.ext

If you start the program without specifying a file the program will
request one.  This was added to allow the program to be run from the AuxProgs
menu of Beekeeper.

The program starts by determining the type of data (DTM, CTE 64 channel,
CTE 128 channel or CTE 64 channel 400Hz).  It will then try to read the
calibration factors.  For most files it will have to read through several
seconds worth of data to get a calibration table.  The exception being the
spike files which are created in the Beekeepers Zoom mode.  These files
start with four bytes that equal the time stamp for the center point of
the spike file, this is followed by a copy of the full calibration table
which would be 128 bytes for a 64 channel file and 256 bytes for a 128
channel file.  If the program can not read the calibration factors it will
ask you whether the data was recorded using the 2mV or 4mV range.


The program will print out the file type and a dump of the calibration
table in hex.  I used hex to make it easier to squeeze a 128 channel table
on an 80 column screen.

At this point the program will allow you to specify which channels to
convert.  This allows you to ignore unused channels and thus reduce the
size of the ASCII output file.

In the following samples the recording was 64 channel CTE but only the
first 32 channels were actually used.  Which is why half the calibration
table and half the data are garbage.

Here is a sample of the output to the screen:
E:\>B2MICRO E:\DEMO\A.EEG

Reading calibration factors 
File is CTE 64 channel.
Calibration factors:
 0: 3FA2 16: 3FE7 32: FFFF 48: FFFF 
 1: 3EBF 17: 3FC3 33: FFFF 49: FFFF 
 2: 3F5F 18: 3F9D 34: FFFF 50: FFFF 
 3: 3F48 19: 4005 35: FFFF 51: FFFF 
 4: 3F75 20: 3FA4 36: FFFF 52: FFFF 
 5: 3F60 21: 3F7C 37: FFFF 53: FFFF 
 6: 3F7F 22: 3FA6 38: FFFF 54: FFFF 
 7: 3F4E 23: 3FD3 39: FFFF 55: FFFF 
 8: 3FC8 24: 3F21 40: FFFF 56: FFFF 
 9: 3F8B 25: 3ECE 41: FFFF 57: FFFF 
10: 3F80 26: 3E4E 42: FFFF 58: FFFF 
11: 3FCF 27: 3E4B 43: FFFF 59: FFFF 
12: 3F4E 28: 3E7E 44: FFFF 60: FFFF 
13: 3FC6 29: 3EC2 45: FFFF 61: FFFF 
14: 3F96 30: 3F00 46: FFFF 62: FFFF 
15: 3FAE 31: 3F02 47: FFFF 63: FFFF 

File starts at: 17:54:34.154  and ends at: 17:54:36.105 
Enter output file name: a.fp

Hit ESC to abort

Time: 17:54:34.154 17:54:35.000 17:54:36.000 

Done.

-----------------------------------------------------------------------
In the times the decimal represent the sample number; 0 to 199 for data
sampled at 200Hz and 0 to 399 for 400Hz data.

Here is a sample of the format of the output file:

1 17:54:34.154    ; sample number and time stamp
 -65.917969       ; channel 1
 -21.972656       ; channel 2
 18.554688        ; etc....
 -12.207031
 -0.976563
 -71.289063
 -50.292969
 -55.175781
 19.531250
 25.390625
 39.550781
 9.277344
 56.640625
 24.414063
 2.929688
 -23.437500
 -40.527344
 -27.832031
 -27.343750
 -25.878906
 -24.414063
 43.945313
 55.175781
 -13.183594
 -40.527344
 -45.410156
 -15.625000
 -41.015625
 -28.808594
 -14.648438
 -61.523438
 6.347656
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
 6997.558594
#endif /* defined (BEEKEEPER_README) */
/*============================================================================*/

/******************************************************************************/
#if defined (B2A_SOURCE)
#if defined (SGI)
/*???DB.  So that structures are packed on SGI */
#pragma pack(1)
#endif /* defined (SGI) */

#if !defined (SGI)
/*???DB.  Not present on SGI */
#include <conio.h>
#include <dos.h>
#endif /* !defined (SGI) */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*???DB.  Needed for isdigit, etc */
#include <ctype.h>

/******************************************************************************

  For Microsoft C, compile like this:

    cl -Zp b2a.c

  The -Zp option packs structures that contain byte size variables.  With out
  this option the compiler will insure that the integer variables are aligned
  on word boundries.  Since the SAMPLE_64 and SAMPLE_128 structures contain
  a couple of byte size variables, if you don't use this option the compiler
  will make the structure two bytes bigger.

  On SGI
    cc -n32 -DSGI bta.c -o bta
  ???DB.  Never got fully going on SGI, because there needs to be byte swapping
  for the integers

******************************************************************************/
#define BUF_SIZE  32000
/* CAL_TEMP_SIZE has room for 128 16 bit cal factors, 1 sync byte & 1 byte checksum */
#define CAL_TEMP_SIZE  258
#define BYTE  unsigned char

#define BLACK          0
#define BLUE           1
#define GREEN          2
#define CYAN           3
#define RED            4
#define MAGENTA        5
#define BROWN          6
#define LIGHTGRAY      7

#define DARKGRAY       8
#define BRIGHTBLUE     9
#define BRIGHTGREEN   10
#define BRIGHTCYAN    11
#define BRIGHTRED     12
#define BRIGHTMAGENTA 13
#define YELLOW        14
#define WHITE         15


typedef struct {
  BYTE sync;   /* 0xEF for CTE 64 */
  BYTE alarm;
  unsigned long timeval;
  BYTE alpha1;
  struct {
    BYTE msb1;
    BYTE msb2;
    BYTE lsb;
  } db[32];
  unsigned short  check;
  BYTE alpha2;
} SAMPLE_64;

typedef struct {
  BYTE sync;   /* 0xDF for CTE 128 */
  BYTE alarm;
  unsigned long timeval;
  BYTE alpha1;
  struct {
    BYTE msb1;
    BYTE msb2;
    BYTE lsb;
  } db[64];
  unsigned short check;
  BYTE alpha2;
} SAMPLE_128;

typedef union {
  BYTE *B;
  SAMPLE_64 *S64;
  SAMPLE_128 *S128;
} SamplePtr;

short main (short, char **);
void ReadCalTable (FILE *);
void OutputASCII (FILE *, FILE *);
void FileType (SamplePtr, size_t);
short CHECKSUM (BYTE *);
void UnPack (BYTE *, int);
void TimeStamp (long, char *);
int CalCheckSum (BYTE *, int);
int CalCheckSum2 (BYTE *);
int CompareTables (BYTE *, BYTE *, int);
int ValidFileName (char *);
int ValidTimeString (char *);
long TimeString2Long (char *);
int ReallyDone (void);
int ReadCh (void);
void BackupCursor (char *);
void ClrScr (void);

void *Mem;
int Channels, OutputChannels;
int XPos, YPos;
int Key, Scale, Samples;
long StartTime = -1L;
long EndTime = -1L;
long FirstTime;
long NumberOfSamples;
BYTE SyncByte;
int Select[128];
unsigned short CalGain[128]; /* Unity Gain is 16384 for 2mV; 32768 for 4mV */
unsigned short EEG[128];


short main (short argc, char **argv)
{
  int Done=0, i;
  int Start, End;
  FILE *InputFile, *OutputFile;
#if defined (OLD_CODE)
/*???DB.  Not used */
  char *OutFileName;
#endif /* defined (OLD_CODE) */
  char *Ptr;
  char TimeString[14];
  char UserTimeString[14];
  char InputFileName[81];
  char FileName[81];
  char String[81];

  if(sizeof(SAMPLE_64) != 106 || sizeof(SAMPLE_128) != 202) {
    printf("\nAn error was made when this program was compiled.");
    printf("\n\nSince the structures for the sample blocks contain an odd number");
    printf("\nof byte wide fields you must use the compiler option which will");
    printf("\npack these structures.");
    printf("\n\nFor Microsoft C use the -Zp switch");
    printf("\n\nFor Borland C packed arrays are the default so you must have");
    printf("\nword alignment enabled.  If you are compiling from the command");
    printf("\nline you must be using the -a switch to do word alignment.");
    printf("\nIf you are using the IDE check the under: Options; Compiler;");
    printf("\nCode generation; Word alignment; and make sure it's not enabled.");
    exit(0);
  }
  if(argc<2) {
    printf("File to convert: ");
    String[0]=80;
#if defined (SGI)
/*???DB.  cgets is not ANSI */
    Ptr = gets(String);
#else
    Ptr = cgets(String);
#endif /* defined (SGI) */
    strcpy(InputFileName, Ptr);
  }
  else
    strcpy(InputFileName, argv[1]);
  if((InputFile = fopen(InputFileName, "r+b")) == NULL) {
    printf("\nError opening input file: %s", InputFileName);
    exit(1);
  }
  if((Mem = malloc(BUF_SIZE)) == NULL) {
    printf("\nBuffer allocation failed.");
    fclose(InputFile);
    exit(3);
  }
  ReadCalTable(InputFile); /* also determines file type & number of channels */
  TimeString[0] = UserTimeString[0] = 0;
  do {
    do {
      TimeStamp(StartTime, TimeString);
      printf("\n\nFile starts at: %s", TimeString);
      if(strlen(UserTimeString)==0)
        strcpy(UserTimeString, TimeString);
      TimeStamp(EndTime, TimeString);
      printf(" and ends at: %s", TimeString);
      printf("\nEnter output file name: ");
      String[0]=80;
#if defined (SGI)
/*???DB.  cgets is not ANSI */
      Ptr = gets(String);
#else
      Ptr = cgets(String);
#endif /* defined (SGI) */
      strcpy(FileName, Ptr);
      if(strlen(Ptr)==0)
        Done = ReallyDone();
    } while(!Done && !ValidFileName(FileName));
    if(!Done) {
      do {
        printf("\nEnter time: %s", UserTimeString);
        BackupCursor(UserTimeString);
#if defined (SGI)
/*???DB.  cgets is not ANSI */
        Ptr = gets(String);
#else
        Ptr = cgets(String);
#endif /* defined (SGI) */
        if(strlen(Ptr)==0)
          strcpy(Ptr, UserTimeString);
        else
          strcpy(UserTimeString, Ptr);
        FirstTime = TimeString2Long(Ptr);
      } while(!ValidTimeString(Ptr));
      if(!Done) {
        do {
          ClrScr();
          printf("File name: %s", FileName);
          printf("\nStart Time: %s", UserTimeString);
          printf("\n\nSelected Channels:");
          for(i=0; i<16; i++) {
            printf("\n%2d: %d  ", i+1, Select[i]);
            printf("%2d: %d  ", i+17, Select[i+16]);
            printf("%2d: %d  ", i+33, Select[i+32]);
            printf("%2d: %d  ", i+49, Select[i+48]);
            if(Channels==128) {
              printf("%2d: %d  ", i+65, Select[i+64]);
              printf("%2d: %d  ", i+81, Select[i+80]);
              printf("%3d: %d  ", i+97, Select[i+96]);
              printf("%3d: %d", i+113, Select[i+112]);
            }
          }
          printf("\n\nEnter individual channel number to select or deselect,");
          printf("\n'A' for all, or specify a set '1-16' (0 when done) : ");

#if defined (SGI)
/*???DB.  cgets is not ANSI */
          Ptr = gets(String);
#else
          Ptr = cgets(String);
#endif /* defined (SGI) */
          OutputChannels = atoi(Ptr);
          if(OutputChannels != 0) {
            if((Ptr = strchr(Ptr, '-')) != NULL) {
              Ptr++;
              Start = OutputChannels;
              End = atoi(Ptr);
              if(End<Start) {
                i = End;
                End = Start;
                Start = i;
              }
              if(Start<1)
                Start = 1;
              if(End>Channels)
                End = Channels;
              for(i=Start; i<=End; i++) {
                Select[i-1] ^= 1;
              }
            }
            else if(OutputChannels <= Channels) {
              Select[OutputChannels-1] ^= 1;
            }
          }
          else if(*Ptr == 'a' || *Ptr == 'A') {
            OutputChannels++; /* just so we stay in the while loop */
            for(i=0; i<Channels; i++) {
              Select[i] ^= 1;
            }
          }
        } while(OutputChannels != 0);
      }
      if(!Done) {
        printf("\nEnter number of samples: ");
#if defined (SGI)
/*???DB.  cgets is not ANSI */
        Ptr = gets(String);
#else
        Ptr = cgets(String);
#endif /* defined (SGI) */
        NumberOfSamples = atol(Ptr);
        if(NumberOfSamples==0L)
          Done = ReallyDone();
        i = strlen(Ptr);
/*
        if(i<4)
          Format[2] = '4';
        else if(i<10)
          Format[2] = '0' + i;
        else
          Format[2] = 9;
*/
      }
      if(!Done) {
        if((OutputFile = fopen(FileName, "w+b")) == NULL) {
          printf("\nError opening output file: %s", FileName);
          fclose(InputFile);
          exit(4);
        }
        OutputASCII(InputFile, OutputFile);
        fclose(OutputFile);
      }
    }
  } while (!Done);
  fclose(InputFile);
  free(Mem);
  printf("\n\nDone.");
  return(0);
}


void ReadCalTable (FILE *InFile)
{
  int i, j;
  int BytesLeft, CalDone, CalIndex, Count, TableSize, Result;
  unsigned int Default;
  size_t bytes;
  SamplePtr P;
  BYTE Alpha1;
  BYTE CalTemp1[CAL_TEMP_SIZE];
  BYTE CalTemp2[CAL_TEMP_SIZE];

  printf("\nReading calibration factors ");
  CalDone=0;
  fseek(InFile, 0L, SEEK_END);  /* a very portable way to get the file size */
  if(ftell(InFile) < 100000L) { /* probably a spike file */
    fseek(InFile, 4L, SEEK_SET);
    bytes = fread(&CalGain[0], 1, sizeof(CalGain), InFile);
    CalDone=1;
  }

  fseek(InFile, 0L, SEEK_SET);  /* go to beginning of file */
  memset(CalTemp1, 0, CAL_TEMP_SIZE);
  memset(CalTemp2, 0, CAL_TEMP_SIZE);
  bytes = fread(Mem, 1, BUF_SIZE, InFile);
  P.B = Mem;
  FileType(P, BUF_SIZE);
  Channels = (SyncByte == 0xDF) ? 128 : 64;
  TableSize = Channels * 2 + 2;
  CalIndex = Count = 0;
  BytesLeft = bytes;
  do {
    if(P.S64->sync == SyncByte && P.S64->alarm < 4) {
      if(CHECKSUM(P.B) == 0) {  /* 0 when checksums agree */
        if(StartTime == -1L)
          StartTime = P.S64->timeval;
        Alpha1 = P.S64->alpha1; /* offset is same for both 64 & 128 structs */
        if(CalIndex == 0) {
          if(Alpha1 == 0x77)
            CalTemp1[CalIndex++] = Alpha1;
        }
        else {
          CalTemp1[CalIndex++] = Alpha1;
          if(CalIndex == TableSize) {
            Result = CalCheckSum(CalTemp1, TableSize);
            if(Result != 0 && Channels == 128)
              Result = CalCheckSum2(CalTemp1);
            if(Result == 0) {
              if(CompareTables(CalTemp1, CalTemp2, TableSize) == TableSize) {
                for(i=0, j=1; i<Channels; i++) {
                  CalGain[i] = CalTemp1[j++];
                  CalGain[i] += CalTemp1[j++]<<8;
                }
                CalDone = 1;
              }
              else {
                memmove(CalTemp2, CalTemp1, TableSize);
                CalIndex=0;
              }
            }
            else {
              CalIndex = 0;
              for(i=1; i<TableSize; i++) {
                if(CalTemp1[i] == 0x77) {
                  CalIndex = TableSize - i;
                  memmove(CalTemp1, &CalTemp1[i], CalIndex);
                  break;
                }
              }
            }
          }
        }
        if(Channels==128) {
          P.S128++;
          BytesLeft -= sizeof(SAMPLE_128);
        }
        else {
          P.S64++;
          BytesLeft -= sizeof(SAMPLE_64);
        }
      }
      else {
        P.B++;
        BytesLeft--;
      }
    }
    else {
      P.B++;
      BytesLeft--;
    }
    if(BytesLeft < 500 && bytes == BUF_SIZE) {
      if(Count++&1) {
#if defined (SGI)
/*???DB.  putch is not ANSI */
        putchar('\b');
        putchar('=DB');
#else
        putch('\b');
        putch('=DB');
#endif /* defined (SGI) */
      }
      else
#if defined (SGI)
/*???DB.  putch is not ANSI */
        putchar(' ');
#else
        putch(' ');
#endif /* defined (SGI) */
      memmove(Mem, P.B, BytesLeft);
      bytes = fread((char *)Mem+BytesLeft, 1, BUF_SIZE-BytesLeft, InFile);
      bytes += BytesLeft;
      BytesLeft = bytes;
      P.B = Mem;
    }
  } while(BytesLeft>300 && (StartTime==-1L || !CalDone));


  if(!CalDone) {
    printf("\nCouldn't read calibration table, use 2mV or 4mv? [2]/[4]: ");
    do {
      i = ReadCh();
      if(i>32)
        printf("%c\b", i);
      if(i=='2')
        Default = 16384;
      else if(i=='4')
        Default = 32768;
      else
        printf(" \b");
    } while(i != '2' && i != '4');
    for(i=0; i<128; i++)
      CalGain[i] = Default;
  }
  fseek(InFile, 0L-BUF_SIZE, SEEK_END);  /* go toward end of file */
  bytes = fread(Mem, 1, BUF_SIZE, InFile);
  BytesLeft = BUF_SIZE;
  /*???DB.  Was getting access violation without resetting P.B */
  P.B = Mem;
  while(BytesLeft>300) {
    if(P.S64->sync == SyncByte && P.S64->alarm < 4) {
      if(CHECKSUM(P.B) == 0) {  /* 0 when checksums agree */
        EndTime = P.S64->timeval;
        if(Channels==128) {
          P.S128++;
          BytesLeft -= sizeof(SAMPLE_128);
        }
        else {
          P.S64++;
          BytesLeft -= sizeof(SAMPLE_64);
        }
      }
      else {
        P.B++;
        BytesLeft--;
      }
    }
    else {
      P.B++;
      BytesLeft--;
    }
  }
  /* dump cal table in hex */
  printf("\nCalibration factors:");
  for(i=0; i<16; i++) {
    printf("\n%2d: %04X ", i, CalGain[i]);
    printf("%2d: %04X ", i+16, CalGain[i+16]);
    printf("%2d: %04X ", i+32, CalGain[i+32]);
    printf("%2d: %04X ", i+48, CalGain[i+48]);
    if(Channels==128) {
      printf("%2d: %04X ", i+64, CalGain[i+64]);
      printf("%2d: %04X ", i+80, CalGain[i+80]);
      printf("%3d: %04X ", i+96, CalGain[i+96]);
      printf("%3d: %04X", i+112, CalGain[i+112]);
    }
  }
}


void OutputASCII (FILE *InFile, FILE *OutFile)
{
  int BytesLeft, y;
  unsigned long LastTime = 17280000L;
  unsigned long x;
  int Value;
  int Started;
  short result;
  size_t bytes;
  float Range;
  SamplePtr P;
  char TimeString[14];
  char OldTimeString[14];

  /* to convert output to microvolts: */
  if(CalGain[0] > 0x6000)
    Range = (float)(4000.0/4096.0);  /* full range of 12 bit D/A is 4000 microvolts */
      /*???DB.  Added cast to float */
  else
    Range = (float)(2000.0/4096.0);  /* full range of 12 bit D/A is 2000 microvolts */
      /*???DB.  Added cast to float */

  fseek(InFile, 0L, SEEK_SET);  /* go to beginning of file */
  bytes = fread(Mem, 1, BUF_SIZE, InFile);
  P.B = Mem;
  x = XPos = Samples = Started = 0;
  Scale = 16;
  BytesLeft = bytes;
  printf("\n\nHit ESC to abort");
  printf("\n\nTime: ");
  while(BytesLeft>300) {
    if(P.S64->sync == SyncByte && P.S64->alarm < 4) {
      result = CHECKSUM(P.B);
      if(result == 0) {   /* result == 0 when checksums agree */
        if(!Started) {
          if((long)(P.S64->timeval) <= FirstTime)
            /*???DB.  Added cast to long */
            Started = 1;
        }
        if(Started && (long)(P.S64->timeval)>=FirstTime) {
          /*???DB.  Added cast to long */
          x++;
          if((long)x>=NumberOfSamples)
            /*???DB.  Added cast to long */
            break;
          TimeStamp((long)(P.S64->timeval), TimeString);
            /*???DB.  Added cast to long */
          if(strncmp(TimeString, OldTimeString, 8) != 0) {
            printf(TimeString);
            strcpy(OldTimeString, TimeString);
            for(y=0; y<(int)strlen(TimeString); y++)
              /*???DB.  Added cast to int */
#if defined (SGI)
/*???DB.  putch is not ANSI */
              putchar('\b');
#else
              putch('\b');
#endif /* defined (SGI) */
          }
          fprintf(OutFile, "\r\n%lu %s", x, TimeString);
          if(LastTime != 17280000L && (LastTime+1)<P.S64->timeval) {
            while(LastTime+1 < P.S64->timeval) {
              /* Missing data
              fprintf(OutFile, "\r\n* missing sample.");*/
                /*???DB.  Changed to C comments */
              fprintf(OutFile, "\r\n0");
              LastTime++;
            }
          }
          LastTime = P.S64->timeval;
          UnPack((BYTE *)&P.S64->db[0], Channels);
          for(y=0; y<Channels; y++) {
            if(Select[y]) {
              /* data in the EEG array is 12 bit unsigned */
                /*???DB.  Changed to C comments */
              Value = (int)(((long)EEG[y] * (long)CalGain[y])>>14) - 2048;
              /* Value is now a calibrated 12 bit signed number */
                /*???DB.  Changed to C comments */
              fprintf(OutFile, "\r\n %f", (float)(Value*Range));
            }
          }
        }
        if(Key == 27)
          break;
        if(Channels==128) {
          P.S128++;
          BytesLeft -= sizeof(SAMPLE_128);
        }
        else {
          P.S64++;
          BytesLeft -= sizeof(SAMPLE_64);
        }
      }
      else {
        P.B++;
        BytesLeft--;
      }
    }
    else {
      P.B++;
      BytesLeft--;
    }
    if(BytesLeft < 500 && bytes == BUF_SIZE) {
#if !defined (SGI)
/*???DB.  Watch for ESC.  OK to skip */
      /* this keyboard input stuff is kind of IBM-PC/DOS specific */
      /* I put it in here to reduce the overhead a bit. */
      if(kbhit()) {
        if((Key = getch()) == 0)
          Key = getch() + 0x100;
        if(Key == 27)
          return;
      }
#endif /* !defined (SGI) */
      memmove(Mem, P.B, BytesLeft);
      bytes = fread((char *)Mem+BytesLeft, 1, BUF_SIZE-BytesLeft, InFile);
      bytes += BytesLeft;
      BytesLeft = bytes;
      P.B = Mem;
    }
  }
}


/*
  This routine determines the file type by doing trial checksums for
  each valid sync byte in the buffer, then comparing the totals for
  good checksums for each type.  It would probably work just as well
  to stop at the first valid checksum.  But, what the heck the data's
  there and we got the time... so let's make real sure.
*/
void FileType (SamplePtr P, size_t BufSize)
{
  short x;
  short SampleDTM, Sample64, Sample128, Sample400;
  unsigned char Sync, Alarm;

  SampleDTM = Sample64 = Sample128 = Sample400 = 0;
  for(x=0; x<(short)BufSize; x++) {
    /*???DB.  Added cast to short */
    Sync = P.S64->sync;
    Alarm = P.S64->alarm;
    if(Sync==0xDF && Alarm<4) {
      if(CHECKSUM(&P.S64->sync) == 0) Sample128++;
    }
    else if(Sync==0xEF && Alarm<4) {
      if(CHECKSUM(&P.S64->sync) == 0) Sample64++;
    }
    else if(Sync==0xFF && Alarm<4) {
      if(CHECKSUM(&P.S64->sync) == 0) SampleDTM++;
    }
    else if(Sync==0xE0 && Alarm<4) {
      if(CHECKSUM(&P.S64->sync) == 0) Sample400++;
    }
    P.B++;
  }
  printf("\nFile is ");
  if(SampleDTM>Sample64 && SampleDTM>Sample128 && SampleDTM>Sample400) {
    SyncByte = 0xFF;
    printf("DTM.");
  }
  else if(Sample64>SampleDTM && Sample64>Sample128 && Sample64>Sample400){
    SyncByte = 0xEF;
    printf("CTE 64 channel.");
  }
  else if(Sample128>SampleDTM && Sample128>Sample64 && Sample128>Sample400) {
    SyncByte = 0xDF;
    printf("CTE 128 channel.");
  }
  else if(Sample400>SampleDTM && Sample400>Sample64 && Sample400>Sample128) {
    SyncByte = 0xE0;
    printf("CTE 64 channel 400 Hz.");
  }
}


short CHECKSUM (BYTE *P)
{
  int Length, x;
  unsigned short Carry;
  unsigned short CheckSum = 0x5a5a;
  unsigned short PacketCheckSum;

  Length = (*P == 0xDF) ? 199 : 103; /* sync == 0xDF for 128 channel data */
  for(x=0; x<Length; x++) {
    CheckSum += (unsigned short)*P++;
    Carry = ((CheckSum&0x8000)!=0);
    CheckSum = (CheckSum << 1) + Carry;
  }
  /* checksum is stored low byte, high byte in the finest Intel tradition */
  PacketCheckSum = *P++;
  PacketCheckSum += *P<<8;
  return(PacketCheckSum - CheckSum);
}


/*
  Packed data is stored as follows:

    Byte1, Byte2, Byte3...

  where Byte 1 contains the 8 MSBs of the first channel
        Byte 2 contains the 8 MSBs of the second channel
        Byt3 3 contains the LSBs of the two channels
               the high nibble belongs to the first channel
               the low nibble belong to the second channel

  this format is the same for all file types.
*/
void UnPack (BYTE *Packed, int Channels)
{
  int Pairs, x;
  unsigned short *UnPacked;
  unsigned short Sample1, Sample2;

  UnPacked = EEG;
  Pairs = Channels/2;
  for(x=0; x<Pairs; x++) {
    Sample1 = (unsigned short)*Packed++;
    Sample2 = (unsigned short)*Packed++;
    Sample1 = (Sample1<<4) + (unsigned short)(*Packed >> 4);
    Sample2 = (Sample2<<4) + (unsigned short)(*Packed++ & 0xf);
    *UnPacked++ = Sample1;
    *UnPacked++ = Sample2;
  }
}


/*
  This routine fills a string with the samples time stamp in the
  following format:

    HH:MM:SS.CCC

  where HH is the hour (0-23)
        MM is the minute (0-59)
        SS is the second (0-59)
        CCC is the count (0-199 for 200hz data, 0-399 for 400hz data)
*/
void TimeStamp (long Time, char *String)
{
  int Hour, Minute, Second, Sample;
  long SamplesPerSecond;

  /* for 400Hz data, sync==0xE0, all other file types are 200Hz data */
  SamplesPerSecond = (SyncByte == 0xE0) ? 400L : 200L;
  Sample = (int)(Time%SamplesPerSecond);
  Time /= SamplesPerSecond;
  Hour = (int)(Time/3600L);
  Time = Time%3600L;
  Minute = (int)(Time/60L);
  Time = Time%60L;
  Second = (int)(Time);
  sprintf(String, "%02d:%02d:%02d.%03d ", Hour, Minute, Second, Sample);
}


/*
  This checksum is largely pointless since it shifts instead of rotates.
  As a result it only contains information from the last seven bytes of data.
  It does, however, serve as kind of a trailing sync byte that helps to
  insure that no bytes are missing from the table.
*/
int CalCheckSum (BYTE *Table, int TableSize)
{
  int x;
  BYTE CheckSum=0;

  for(x=0; x<TableSize; x++) {
    CheckSum ^= *Table++;
    CheckSum <<= 1;
  }
  return((int)(CheckSum - *Table));
}


/*
  In Beehive versions 2.20 to 2.22 when doing a 128 channel recording
  it would only checksum the first 64 channels... see if that works.
*/
int CalCheckSum2 (BYTE *Table)
{
  int x;
  BYTE *Ptr;
  BYTE CheckSum=0;

  Ptr = Table;
  for(x=0; x<129; x++) {
    CheckSum ^= *Ptr++;
    CheckSum <<= 1;
  }
  return((int)(CheckSum - Table[257]));
}


/*
  Since the checksum for the calibration table isn't very useful, I try
  to verify the accuracy by getting two consecutive identical copies of
  the table.
*/
int CompareTables (BYTE *Table1, BYTE *Table2, int TableSize)
{
  int x;

  for(x=0; x<TableSize; x++) {
    if(*Table1++ != *Table2++)
      return(x);
  }
  return(x);
}


int ValidFileName (char *FileName)
{
  unsigned char Chr;
  static char *SpecialChars = "$#&@!%()-{}'_`^~\\.:";

  if(strlen(FileName)==0)
    return(0);
  while(*FileName != '\0') {
    Chr = *FileName++;
    if(!isalnum(Chr) && strchr(SpecialChars, Chr)==NULL) {
      printf("\nCan't use '%c' in a file name. ", Chr);
      return(0);
    }
  }
  return(1);
}


int ValidTimeString (char *S)
{
  static char *Template = "dd:dd:dd.ddd";
  char *T;

  T = Template;
  while(*T != '\0') {
    if(*T=='d') {
      if(!isdigit(*S))
        return(0);
    }
    else if(*T != *S)
      return(0);
    T++;
    S++;
  }
  return(1);
}


long TimeString2Long (char *Str)
{
  int Hour, Minute, Second, Sample;
  long SampleNumber;

  Hour   = Str[0]  - '0';
  Hour   = Str[1]  - '0' + Hour * 10;
  Minute = Str[3]  - '0';
  Minute = Str[4]  - '0' + Minute * 10;
  Second = Str[6]  - '0';
  Second = Str[7]  - '0' + Second * 10;
  Sample = Str[9]  - '0';
  Sample = Str[10] - '0' + Sample * 10;
  Sample = Str[11] - '0' + Sample * 10;
  SampleNumber = Hour * 3600L + Minute * 60L + Second;
  SampleNumber *= (SyncByte == 0xE0) ? 400L : 200L;
  SampleNumber += Sample;
  return(SampleNumber);
}


int ReallyDone ()
{
  int Response;

  do {
    printf("\nDo you want to quit? [Y]/[N]: ");
    Response = ReadCh();
  } while(Response!='Y' && Response!='N');
  return(Response=='Y');
}


int ReadCh ()
{
  int Key;

#if defined (SGI)
/*???DB.  getch is not ANSI */
  if((Key = getchar()) == 0)
    Key = getchar() + 0x100;
#else
  if((Key = getch()) == 0)
    Key = getch() + 0x100;
#endif /* defined (SGI) */
  Key = toupper(Key);
  return(Key);
}


void BackupCursor (char *Str)
{
  while(*Str++ != '\0')
#if defined (SGI)
/*???DB.  putch is not ANSI */
    putchar('\b');
#else
    putch('\b');
#endif /* defined (SGI) */
}


void ClrScr ()
{
#if defined (OLD_CODE)
/*???DB.  DOS/Borland ? */
  union REGS R;

  R.x.ax = 3;
  int86(0x10, &R, &R);
#endif /* defined (OLD_CODE) */
}
#endif /* defined (B2A_SOURCE) */
/*============================================================================*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*???debug */
#include <stdio.h>

#include "general/debug.h"
#include "general/myio.h"
#include "unemap/beekeeper.h"
#include "unemap/rig.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
#define BUFFER_SIZE 32000
#define CALIBRATION_TABLE_SIZE 258
#define SAMPLE_128_RECORD_SIZE 202
#define SAMPLE_64_RECORD_SIZE 106

/*
Module functions
----------------
*/
static int check_sum(unsigned char *record)
{
	unsigned char *entry;
	int count,return_code;
	unsigned short record_sum,sum;

	/* check arguments */
	if (record)
	{
		if (0xDF==record[0])
		{
			count=199;
		}
		else
		{
			count=103;
		}
		sum=0x5a5a;
		entry=record;
		while (count>0)
		{
			sum += (unsigned short)(*entry);
			if (sum&0x8000)
			{
				sum=(sum<<1)+1;
			}
			else
			{
				sum=sum<<1;
			}
			entry++;
			count--;
		}
		record_sum= *entry;
		entry++;
		record_sum += (*entry)<<8;
		if (sum==record_sum)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}

	return (return_code);
} /* check_sum */

static int calibration_check_sum(unsigned char *calibration,
	int calibration_size)
{
	unsigned char *entry;
	int i,return_code;
	unsigned short record_sum,sum;

	/* check arguments */
	if (calibration&&(0<calibration_size))
	{
		sum=0;
		entry=calibration;
		for (i=calibration_size;i>0;i--)
		{
			sum ^= *entry;
			sum=sum<<1;
			entry++;
		}
		if (sum== *entry)
		{
			return_code=1;
		}
		else
		{
			if (258==calibration_size)
			{
				sum=0;
				entry=calibration;
				for (i=129;i>0;i--)
				{
					sum ^= *entry;
					sum=sum<<1;
					entry++;
				}
				if (sum==calibration[257])
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				return_code=0;
			}
		}
		record_sum= *entry;
		entry++;
		record_sum += (*entry)<<8;
		if (sum==record_sum)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}

	return (return_code);
} /* calibration_check_sum */

/*
Global functions
----------------
*/
int read_beekeeper_eeg_file(char *file_name,void *rig_pointer)
/*******************************************************************************
LAST MODIFIED : 18 March 2001

DESCRIPTION :
Reads the signal data from a Beekeeper EEG file and creates a rig with a default
configuration (electrode information is not available).
==============================================================================*/
{
	char buffer[BUFFER_SIZE+SAMPLE_128_RECORD_SIZE],
		calibration_table1[CALIBRATION_TABLE_SIZE],
		calibration_table2[CALIBRATION_TABLE_SIZE],*record,sync_byte,temp_char;
	FILE *beekeeper_file;
	float frequency,full_range;
	int buffer_size,calibration_index,calibration_size,*electrodes_in_row,found,i,
		index,j,number_of_channels,number_of_data_gaps,number_of_devices,
		number_of_DTM,number_of_rows,number_of_samples,number_of_128,number_of_400,
		number_of_64,record_size,return_code,*signal_time;
	short *signal_value;
	struct Device **device;
	struct Rig *rig;
	struct Signal_buffer *signal_buffer;
	unsigned char *packed;
	unsigned long data_gap_length,end_time,start_time,time;
	unsigned short gain[128],*unpacked,value[128],value_1,value_2;
#if defined (FILL_IN_DATA_GAPS)
	int max_data_gap_length;
#endif /* defined (FILL_IN_DATA_GAPS) */

	ENTER(read_beekeeper_eeg_file);
/*???debug */
printf("enter read_beekeeper_eeg_file\n");
	if (file_name&&rig_pointer)
	{
		/* read the rig configuration from the electrode file */
		if (beekeeper_file=fopen(file_name,"rb"))
		{
			/* determine the file type */
			number_of_128=0;
			number_of_64=0;
			number_of_DTM=0;
			number_of_400=0;
			record=buffer;
			buffer_size=fread(buffer,sizeof(char),BUFFER_SIZE+SAMPLE_128_RECORD_SIZE,
				beekeeper_file);
			while (SAMPLE_128_RECORD_SIZE<=buffer_size)
			{
				while (SAMPLE_128_RECORD_SIZE<=buffer_size)
				{
					if (((unsigned char)(record[1])<4)&&(((char)0xDF==record[0])||
						((char)0xEF==record[0])||((char)0xFF==record[0])||
						((char)0xE0==record[0]))&&check_sum((unsigned char *)record))
					{
						switch (record[0])
						{
							case (char)0xDF:
							{
								number_of_128++;
							} break;
							case (char)0xEF:
							{
								number_of_64++;
							} break;
							case (char)0xFF:
							{
								number_of_DTM++;
							} break;
							case (char)0xE0:
							{
								number_of_400++;
							} break;
						}
					}
					record++;
					buffer_size--;
				}
				memmove(buffer,record,buffer_size);
				record=buffer;
				buffer_size += fread(record+buffer_size,sizeof(char),BUFFER_SIZE,
					beekeeper_file);
			}
			while (SAMPLE_64_RECORD_SIZE<=buffer_size)
			{
				if (((unsigned char)(record[1])<4)&&(((char)0xEF==record[0])||
					((char)0xFF==record[0])||((char)0xE0==record[0]))&&
					check_sum((unsigned char *)record))
				{
					switch (record[0])
					{
						case (char)0xEF:
						{
							number_of_64++;
						} break;
						case (char)0xFF:
						{
							number_of_DTM++;
						} break;
						case (char)0xE0:
						{
							number_of_400++;
						} break;
					}
				}
				record++;
				buffer_size--;
			}
/*???debug */
printf("number_of_128=%d\n",number_of_128);
printf("number_of_64=%d\n",number_of_64);
printf("number_of_DTM=%d\n",number_of_DTM);
printf("number_of_400=%d\n",number_of_400);
			if (number_of_128>number_of_64)
			{
				if (number_of_128>number_of_DTM)
				{
					if (number_of_128>number_of_400)
					{
						number_of_samples=number_of_128;
						sync_byte=(char)0xDF;
					}
					else
					{
						number_of_samples=number_of_400;
						sync_byte=(char)0xE0;
					}
				}
				else
				{
					if (number_of_DTM>number_of_400)
					{
						number_of_samples=number_of_DTM;
						sync_byte=(char)0xFF;
					}
					else
					{
						number_of_samples=number_of_400;
						sync_byte=(char)0xE0;
					}
				}
			}
			else
			{
				if (number_of_64>number_of_DTM)
				{
					if (number_of_64>number_of_400)
					{
						number_of_samples=number_of_64;
						sync_byte=(char)0xEF;
					}
					else
					{
						number_of_samples=number_of_400;
						sync_byte=(char)0xE0;
					}
				}
				else
				{
					if (number_of_DTM>number_of_400)
					{
						number_of_samples=number_of_DTM;
						sync_byte=(char)0xFF;
					}
					else
					{
						number_of_samples=number_of_400;
						sync_byte=(char)0xE0;
					}
				}
			}
			if (0<number_of_samples)
			{
				if ((char)0xDF==sync_byte)
				{
					record_size=SAMPLE_128_RECORD_SIZE;
					number_of_channels=128;
				}
				else
				{
					record_size=SAMPLE_64_RECORD_SIZE;
					number_of_channels=64;
				}
				/* retrieve the start and end times */
				rewind(beekeeper_file);
				found=0;
				number_of_data_gaps=0;
				buffer_size=fread(buffer,sizeof(char),BUFFER_SIZE+record_size,
					beekeeper_file);
				record=buffer;
#if defined (FILL_IN_DATA_GAPS)
				max_data_gap_length=number_of_samples;
#endif /* defined (FILL_IN_DATA_GAPS) */
				while (record_size<=buffer_size)
				{
					while (record_size<=buffer_size)
					{
						if (((unsigned char)(record[1])<4)&&(sync_byte==record[0])&&
							check_sum((unsigned char *)record))
						{
#if !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER)
							/* convert from big to little endian */
							temp_char=record[2];
							record[2]=record[5];
							record[5]=temp_char;
							temp_char=record[3];
							record[3]=record[4];
							record[4]=temp_char;
#endif /* !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER) */
							memcpy((char *)&time,record+2,sizeof(unsigned long));
							if (found)
							{
								if (end_time+1!=time)
								{
									number_of_data_gaps++;
									data_gap_length += time-end_time-1;
#if defined (FILL_IN_DATA_GAPS)
									if (time-end_time-1<max_data_gap_length)
									{
										number_of_samples += time-end_time-1;
									}
									else
									{
										number_of_samples += max_data_gap_length;
									}
#endif /* defined (FILL_IN_DATA_GAPS) */
								}
							}
							else
							{
								start_time=time;
							}
							end_time=time;
							found++;
							record += record_size;
							buffer_size -= record_size;
						}
						else
						{
							record++;
							buffer_size--;
						}
					}
					memmove(buffer,record,buffer_size);
					record=buffer;
					buffer_size += fread(record+buffer_size,sizeof(char),BUFFER_SIZE,
						beekeeper_file);
				}
/*???debug */
printf("number of records = %d\n",found);
printf("number_of_data_gaps = %d\n",number_of_data_gaps);
printf("start=%lu, end=%lu\n",start_time,end_time);
printf("data length=%lu, no data length=%lu\n",
	end_time-start_time+1-data_gap_length,data_gap_length);
#if defined (FILL_IN_DATA_GAPS)
printf("Filling in data gaps\n");
#else /* defined (FILL_IN_DATA_GAPS) */
printf("Not filling in data gaps\n");
#endif /* defined (FILL_IN_DATA_GAPS) */
				/* extract the calibration table */
				/* 2 bytes for each channel, plus a sync byte and a check sum byte */
				calibration_size=2*number_of_channels+2;
				calibration_index=0;
				found=0;
				rewind(beekeeper_file);
				buffer_size=fread(buffer,sizeof(char),BUFFER_SIZE+record_size,
					beekeeper_file);
				record=buffer;
				while ((record_size<=buffer_size)&&!found)
				{
					while ((record_size<=buffer_size)&&!found)
					{
						if (((unsigned char)(record[1])<4)&&(sync_byte==record[0])&&
							check_sum((unsigned char *)record))
						{
							temp_char=record[6];
							if (0==calibration_index)
							{
								/* check for sync byte */
								if (0x77==temp_char)
								{
									calibration_table1[0]=0x77;
									calibration_index=1;
								}
							}
							else
							{
								calibration_table1[calibration_index]=(unsigned char)temp_char;
								calibration_index++;
								if (calibration_index==calibration_size)
								{
									/* have read calibration table */
									if (calibration_check_sum((unsigned char *)calibration_table1,
										calibration_size))
									{
										/* check that same as previous */
										if (memcmp(calibration_table1,calibration_table2,
											calibration_size))
										{
											found=1;
											j=1;
											for (i=0;i<number_of_channels;i++)
											{
												gain[i]=(unsigned char)(calibration_table1[j]);
												j++;
												gain[i] += ((unsigned char)(calibration_table1[j]))<<8;
												j++;
											}
										}
										else
										{
											memmove(calibration_table2,calibration_table1,
												calibration_size);
											calibration_index=0;
										}
									}
									else
									{
										/* look for another sync byte */
										i=1;
										while ((i<calibration_size)&&(0x77!=calibration_table1[i]))
										{
											i++;
										}
										if (i<calibration_size)
										{
											memmove(calibration_table1,calibration_table1+i,
												calibration_size-i);
										}
									}
								}
							}
							record += record_size;
							buffer_size -= record_size;
						}
						else
						{
							record++;
							buffer_size--;
						}
					}
					if (!found)
					{
						memmove(buffer,record,buffer_size);
						record=buffer;
						buffer_size += fread(record+buffer_size,sizeof(char),BUFFER_SIZE,
							beekeeper_file);
					}
				}
				if (!found)
				{
					for (i=0;i<number_of_channels;i++)
					{
						gain[i]=32768;
					}
					display_message(WARNING_MESSAGE,
						"Missing calibration table, using 4mV");
				}
/*???debug */
for (i=0;i<16;i++)
{
	printf("%2d: %04X",i,gain[i]);
	printf(" %2d: %04X",i+16,gain[i+16]);
	printf(" %2d: %04X",i+32,gain[i+32]);
	printf(" %2d: %04X",i+48,gain[i+48]);
	if (128==number_of_channels)
	{
		printf(" %2d: %04X",i+64,gain[i+64]);
		printf(" %2d: %04X",i+80,gain[i+80]);
		printf(" %3d: %04X",i+96,gain[i+96]);
		printf(" %3d: %04X",i+112,gain[i+112]);
	}
	printf("\n");
}
				/* count the number of devices */
				number_of_devices=0;
				for (i=0;i<number_of_channels;i++)
				{
					if (0xFFFF!=gain[i])
					{
						number_of_devices++;
					}
				}
				/* create a standard rig */
				number_of_rows=((int)number_of_devices-1)/8+1;
				if (ALLOCATE(electrodes_in_row,int,number_of_rows))
				{
					index=number_of_rows-1;
					electrodes_in_row[index]=(int)number_of_devices-8*index;
					while (index>0)
					{
						index--;
						electrodes_in_row[index]=8;
					}
					if (rig=create_standard_Rig("Beekeeper EEG",PATCH,MONITORING_OFF,
						EXPERIMENT_OFF,number_of_rows,electrodes_in_row,1,0,(float)1
#if defined (UNEMAP_USE_3D)
						/*??JW perhaps we should pass this down from above*/
						,(struct Unemap_package *)NULL
#endif /* defined (UNEMAP_USE_3D) */
						))
					{
						rig->current_region=rig->region_list->region;
						/* set calibration */
						if (0x6000<gain[0])
						{
							/* full range of 12 bit D/A is 4000 microvolts */
							full_range=(float)(4000./4096.);
						}
						else
						{
							/* full range of 12 bit D/A is 2000 microvolts */
							full_range=(float)(2000./4096.);
						}
						device=rig->devices;
						for (i=number_of_devices;i>0;i--)
						{
							(*device)->channel->offset=0.;
							(*device)->channel->gain=full_range;
							device++;
						}
						/* create the signal buffer */
						if ((char)0xE0==sync_byte)
						{
							frequency=400.;
						}
						else
						{
							frequency=200.;
						}
						/*???debug */
						printf("number_of_devices=%d, number_of_samples=%d\n",
							number_of_devices,number_of_samples);
						if (signal_buffer=create_Signal_buffer(SHORT_INT_VALUE,
							number_of_devices,number_of_samples,frequency))
						{
							signal_time=signal_buffer->times;
							signal_value=(signal_buffer->signals).short_int_values;
							/* extract the signal values (in microvolts) */
							rewind(beekeeper_file);
							found=0;
							buffer_size=fread(buffer,sizeof(char),BUFFER_SIZE+record_size,
								beekeeper_file);
							record=buffer;
							end_time=start_time;
							while (record_size<=buffer_size)
							{
								while (record_size<=buffer_size)
								{
									if (((unsigned char)(record[1])<4)&&(sync_byte==record[0])&&
										check_sum((unsigned char *)record))
									{
										/* extract the time */
#if !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER)
										/* convert from big to little endian */
										temp_char=record[2];
										record[2]=record[5];
										record[5]=temp_char;
										temp_char=record[3];
										record[3]=record[4];
										record[4]=temp_char;
#endif /* !defined (__BYTE_ORDER) || (1234!=__BYTE_ORDER) */
										memcpy((char *)&time,record+2,sizeof(unsigned long));
										/* unpack the 12-bit signal data */
										packed=(unsigned char *)(record+7);
										unpacked=value;
										for (i=number_of_channels/2;i>0;i--)
										{
											value_1=(unsigned short)(*packed);
											packed++;
											value_2=(unsigned short)(*packed);
											packed++;
											value_1=(value_1<<4)+(unsigned short)((*packed)>>4);
											value_2=(value_2<<4)+(unsigned short)((*packed)&0xF);
											packed++;
											*unpacked=value_1;
											unpacked++;
											*unpacked=value_2;
											unpacked++;
										}
/*???debug */
if (!found)
{
	long temp_long;
	unsigned short temp_short;

	printf("time %ld\n",(long int)(time));
	for (i=0;i<number_of_channels;i++)
	{
		if (0xFFFF!=gain[i])
		{
			temp_long=((long)(value[i])*(long)(gain[i]))>>14;
			temp_short=(short)temp_long-2048;
			printf("%d  %8X %8lX %8d %g\n",i,value[i],temp_long,temp_short,
				(float)temp_short*full_range);
		}
	}
}
found++;
#if defined (FILL_IN_DATA_GAPS)
										if ((time>start_time)&&
											((data_gap_length=(int)(time-end_time))>1))
										{
											if (data_gap_length<=max_data_gap_length)
											{
												for (j=1;j<=data_gap_length;j++)
												{
													*signal_time=(*(signal_time-1))+1;
													signal_time++;
													for (i=0;i<number_of_channels;i++)
													{
														if (0xFFFF!=gain[i])
														{
															*signal_value=(short)(((long)(data_gap_length-j)*
																(long)(*(signal_value-j*number_of_devices))+
																(long)j*((((long)(value[i])*(long)(gain[i]))>>
																14)-(long)2048))/(long)data_gap_length);
															signal_value++;
														}
													}
												}
											}
											else
											{
												for (j=1;j<=max_data_gap_length;j++)
												{
													*signal_time=(*(signal_time-1))+1;
													signal_time++;
													for (i=0;i<number_of_channels;i++)
													{
														if (0xFFFF!=gain[i])
														{
															*signal_value=(short)8192;
															signal_value++;
														}
													}
												}
												*signal_time=(*(signal_time-1))+1;
												signal_time++;
												for (i=0;i<number_of_channels;i++)
												{
													if (0xFFFF!=gain[i])
													{
														*signal_value=
															(short)(((long)(value[i])*(long)(gain[i]))>>14)-
															2048;
														signal_value++;
													}
												}
											}
										}
										else
										{
											*signal_time=(*(signal_time-1))+1;
											signal_time++;
											for (i=0;i<number_of_channels;i++)
											{
												if (0xFFFF!=gain[i])
												{
													*signal_value=
														(short)(((long)(value[i])*(long)(gain[i]))>>14)-
														2048;
													signal_value++;
												}
											}
										}
										end_time=time;
#else /* defined (FILL_IN_DATA_GAPS) */
										*signal_time=(int)(time-start_time);
										signal_time++;
										for (i=0;i<number_of_channels;i++)
										{
											if (0xFFFF!=gain[i])
											{
												*signal_value=
													(short)(((long)(value[i])*(long)(gain[i]))>>14)-
													2048;
												signal_value++;
											}
										}
#endif /* defined (FILL_IN_DATA_GAPS) */
										record += record_size;
										buffer_size -= record_size;
									}
									else
									{
										record++;
										buffer_size--;
									}
								}
								memmove(buffer,record,buffer_size);
								record=buffer;
								buffer_size += fread(record+buffer_size,sizeof(char),
									BUFFER_SIZE,beekeeper_file);
							}
/*???debug */
printf("read signals\n");
							/* link the devices to the signals */
							number_of_devices=rig->number_of_devices;
							device=rig->devices;
							return_code=1;
							while ((number_of_devices>0)&&return_code)
							{
								if (ELECTRODE==(*device)->description->type)
								{
									if (!((*device)->signal=create_Signal(
										(*device)->channel->number,signal_buffer,UNDECIDED,0)))
									{
										return_code=0;
									}
								}
								else
								{
									if (!((*device)->signal=create_Signal(
										(*device)->channel->number,signal_buffer,REJECTED,0)))
									{
										return_code=0;
									}
								}
								device++;
								number_of_devices--;
							}
							if (!return_code)
							{
								destroy_Signal_buffer(&signal_buffer);
								destroy_Rig(&rig);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_beekeeper_eeg_file.  Could not create signal buffer");
							destroy_Rig(&rig);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_beekeeper_eeg_file.  Error creating standard rig");
					}
					DEALLOCATE(electrodes_in_row);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_beekeeper_eeg_file.  Could not allocate electrodes_in_row");
					rig=(struct Rig *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_beekeeper_eeg_file.  No data in file: %s",file_name);
				rig=(struct Rig *)NULL;
			}
			fclose(beekeeper_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_beekeeper_eeg_file.  Could not open file: %s",file_name);
			rig=(struct Rig *)NULL;
		}
		if (rig)
		{
			*((struct Rig **)rig_pointer)=rig;
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_beekeeper_eeg_file.  Missing argument(s)");
		return_code=0;
	}
/*???debug */
printf("leave read_beekeeper_eeg_file\n");
	LEAVE;

	return (return_code);
} /* read_beekeeper_eeg_file */
