/*******************************************************************************
FILE : tuff2sig.c

LAST MODIFIED : 9 February 2000

DESCRIPTION :
Converts TUFF (Telefactor Universal File Format, Beehive 7) files to a unemap
signal file.

NOTES from TelefactoR Corporation file REFDATA.H :
The referential and differential files will have the same block diagram.

*********************************
*   TelefactorFileHeaderBlock   *  indicates whether file is ref or dif.
*         4096 bytes            *  
*********************************
*      DataBaseRecord           *
*         1024 bytes            *
*********************************
*    DATA_BLOCK_HEADER          *  IPT and ODT offsets are relative to start of this block
*           44 bytes            *
*********************************
*   InputDescriptorTable        *  include all AC and DC channels
*   (inputs * 18 bytes)         *
*********************************
*      Montage Name             *  
*    MONTAGE_NAME bytes         *
*********************************
*   OutputDescriptorTable       *  for ref file, this is the detection montage
*   (inputs * 16 bytes)         *  for dif file, this is how the data was written
*********************************
*      EndSync Padding          *  
*  (enough to make next data    *
*   block land on a 16 byte     *
*         boundary)             *
*********************************
*          DATA                 *
*********************************

  :    :    :    :    :    :   :

*********************************
*    DATA_BLOCK_HEADER          *  new segment time in this header
*           44 bytes            *
*********************************
*   InputDescriptorTable        *
*   (inputs * 18 bytes)         *
*********************************
*      Montage Name             *  
*    MONTAGE_NAME bytes         *
*********************************
*   OutputDescriptorTable       *
*   (inputs * 16 bytes)         *
*********************************
*      EndSync Padding          *  
*  (enough to make next data    *
*   block land on a 16 byte     *
*         boundary)             *
*********************************
*          DATA                 *
*********************************

  :    :    :    :    :    :   :
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "general/debug.h"
#include "general/myio.h"
#include "unemap/rig.h"

/*
Constants
---------
*/
#define BUFFER_LENGTH 4096

/* format for TUFF binary (assume PC) */
#define BIG_ENDIAN (unsigned char)1

/*
Constants (from TelefactoR Corporation file REFDATA.H)
---------
*/
/************   Referential/Differential File Header Block  *********/
/************************  4K HEADER  *******************************/

#define VERSION_4_15 4150 /* version number X 1000 */
/* Version 4.15 is the traditional TUFF file, where Selector uses the scheme
listed below */

#define VERSION_4_20 4200 /* version number X 1000 */
/* Version 4.20 supports VSR file format.  Note that Selector now has a 
different meaning */

#define CURRENT_VERSION  VERSION_4_20

/* USE THESE DEFINES. WE WANT TO AVOID SPELING ERRERS. */

#define COMPANYSIGNATURE  "Telefactor Corp"
#define DIFFERENTIALDATA  "Differential"
#define REFERENTIALDATA   "Referential"

#ifndef REFDATA_MAX_INPUTS
#define REFDATA_MAX_INPUTS    128 /* 128 channels max.   */
#endif

#ifndef REFDATA_MAX_OUTPUTS
#define REFDATA_MAX_OUTPUTS    32 /* Maximum Out Output Channels */
#endif

/* m_FileHeader.InputAmplifierDevice amplifier types */
#define DTM_AMPS         0x0000
#define CTE_AMPS         0x0001
#define DGEEG_HDBX       0x0002
#define BEELOGGER_CARD   0x0004
#define AD100_CARD         5000

/* m_FileHeader.InputAmplifierDevice encoder types */
#define ISOCODER     0x0000 
#define PANISOCODER  0x4000 /* for DEEGs */
#define MINISOCODER  0x8000 /* for DEEGs */
/* add these values to the amplifier type, store in m_FileHeader.InputAmplifierDevice */

/* Packing Schemes for m_FileHeader.PackedData */
#define NOT_PACKED      0
#define DDVCR_PACKED    1  /* BEEHIVE 12 bit packing scheme (2X12bit->3X8bit) */
#define AD100_PACKED    2  /* 10 bit data packed (6X10bit->5X16bit) */

/* Other variables */
#define SYNC_LENGTH                    2
#define ALARM_LENGTH                   2
#define DC_NOTE_OFFSET              1024
#define TELEFACTOR_FILE_HEADER_SIZE 4096L
#define DATA_BASE_HEADER_SIZE       1024L
#define MONTAGE_NAME                  32
#define COMP_SIGNATURE                16
#define FILE_SIGNATURE                14

/* m_FileHeader.SamplingMethod */
#define SMP_METHOD_NONE       " None At All."
#define SMP_METHOD_10HZ       "All at 10 Hz."
#define SMP_METHOD_20HZ       "All at 20 Hz."
#define SMP_METHOD_VAR_RATE   "Variable Rate"

#define SMP_METHOD_FREQ_BASED  "HZ"   /* if other than 10 or 20 (defined above) */
#define SMP_METHOD_MS_PERIOD   "MS"

/* event type constants for m_FileHeader.EventType */
#define SPK_DET_DTFL    0  /* Spike Detection File...*/
#define SEZ_DET_DTFL    1  /* Seizure Detection File.*/
#define SEZ_WTV_DTFL    2  /* Seizure WTV File.......*/
#define SPW_WTV_DTFL    3  /* Spike/Wave WTV File....*/
#define SPK_WTV_DTFL    4  /* Spike WTV File.........*/
#define PSHBTN_WTV_DTFL 5  /* Push Button WTV File...*/
#define TMSP_WTV_DTFL   6  /* Time Sample WTV file....*/
#define SPW_DET_DTFL    7  /* Burst Detection file....*/
#define EEG_DTFL        8  /* 10 bit EEG File.........*/
#define EKG_DTFL        9  /* EKG data................*/

#define MAXIMUM_AC_INPUTS   128
#define MAXIMUM_DC_INPUTS    16

/* Version 4.15 Selector offsets */
#define AVG_MARKER_CHAN   10000
#define DC_CHAN_ONE       60000
#define SPECIAL_CHAN_ONE  64000

#define ELECTRODE_L           5  /* electrode name length */

/* for DataBlockHeader.SyncWord */
#define REF_BEG_SYNC 0XAEAEAEAEL
#define REF_END_SYNC 0XBE
#define DIF_BEG_SYNC 0XDEDEDEDEL
#define DIF_END_SYNC 0XCE

/*******************  DATABASE HEADER BLOCK *************************/
/************************  1K HEADER  *******************************/

/***************
This first structure is designed to identify data files using minimal
storage space.  By incorporation this header into a referential file,
the user has immediate access to the patient information used at the
time of recording.  The information may not be correct, due to
editing during and after the test.

The FOXPRO_XXXX_ID_Number is a 10-byte unique identifier which provides
direct connections into the database files, giving the user quasi-
instantaneous connections to more information.
*************/

#define DB_ENTRIES         25
#define LARGEST_DB_ENTRY   60
#define DB_LABEL_LEN       20

#define FOX_PRO_ID_LEN  10
#define LAST_NAME_LEN   50
#define FIRST_NAME_LEN  50
#define DOB_LEN         12
#define ID_TYPE_LEN     12
#define ID_CONTENT_LEN  30
#define HOSPTL_NUM_LEN  30
#define PHONE_NUM_LEN   30
#define ROOM_NUM_LEN     8
#define SEX_LEN          2
#define HAND_LEN         2
#define CLASSES_LEN     10
#define STREET_ADDR_LEN 40
#define CITY_LEN        30
#define STATE_LEN       10
#define ZIP_CODE_LEN    10
#define COUNTRY_LEN     20
#define DIAGNOSIS_LEN   30
#define TEST_NUM_LEN    12
#define TEST_TYPE_LEN    6
#define TEST_DATE_LEN   12
#define TEST_TIME_LEN    8
#define TECH_NAME_LEN   20
#define LAB_ID_LEN      10
#define TEST_CLASS_LEN  10
#define PAT_COMPLNT_LEN 20
#define PAT_STATE_LEN   40
#define PAT_MEDS_LEN    60
#define DB_ELECT_LEN    15
#define ISO_SERIAL_LEN   5
#define ISO_HDBOX_LEN    5
#define FRONT_END_LEN   15
#define DISK_NAME_LEN   15

/*
Types (from TelefactoR Corporation file REFDATA.H)
-----
*/
typedef struct 
{
  unsigned char  CompanySignature[COMP_SIGNATURE];  /* use defines above */
  unsigned char  FileTypeSignature[FILE_SIGNATURE]; /* use defines above */
  unsigned short RefHeaderSize;                /* sizeof this header in bytes (4096 bytes) */
  unsigned char  RecordingSystemSignature[16]; /* Software name, eg. "SZAC" or "BH7" */
  unsigned char  SoftwareVersionSignature[8];  /* Version of software named above */
  unsigned long  SoftwareSerialNumber;         /* Version of TUFF file format used * 1000, eg. 4150 or 4200 */
  unsigned short InputAmplifierDevice;         /* see list above (OR amplifier with encoder type) eg.  (DGEEG_HDBX | PANISOCODER) */ 
  unsigned char  SamplingMethod[16];           /* see list above, eg. SMP_METHOD_10HZ */
  unsigned short DCNotesOffset;                /* use define above */
  unsigned short AmplifierSensitivity;         /* 1000, 2000, or 4000 */
  unsigned short MainsFrequency;               /* A. C. Power Freq.  50 or 60 */
  unsigned short InputAmplifiers;  /* set this to 0xFFFF if number inputs varies between */
                                   /* data block headers. AC and DCs */
  unsigned short MaxSamplingRate;  /* in Hz */
  unsigned short NumberOfBits;     /* 8,10,12, or 16 - for the AC data */
  unsigned short PackedData;       /* see list above  */
  unsigned long  DataBlockSize;    /* constant data block size (in seconds) */
  unsigned long  StartTime;        /* C-time, GMT no DST  */
  unsigned long  EndTime;          /* 0 - for still being written */
  unsigned long  EventTime;        /* time of first significant event in this file, C-time, GMT no DST */
  unsigned short EventType;        /* type of first significant event in this file, see list above */
  unsigned short SyncLength;       /* Length of the Sync Byte */
  unsigned short AlarmLength;      /* Length of the Alarm Byte */
  unsigned long  TOD_Start_Time;   /* actual TOD at start of writing */
  unsigned long  TOD_End_Time;     /* actual TOD at end of writing */
  unsigned long  DescriptorLength; /* 0 = default of 5 characters */
                                   /* all other values, use InputDescriptorTablEx structure*/
  unsigned long  SyncAlarmRate;    /* samples rate at which the Sync/Alaram combination exists */
                                   /* default = 0, which means the alarm occurs at the highest */
                                   /* sample rate.  Sleep Walker should put 10, to indicate that */
                                   /* the SyncAlarm occurs at the slowest sample rate. */
  
  /*** SPACE RESERVED FOR FUTURE EXPANSION  THROUGH 1024 ***/

  /* DC notes usually start at 1024, if notes are not used, make sure */
  /* TelefactorFileHeaderBlock[1024] is 0 */

  unsigned char  Reserved[3964];  /* so total is always 4096 */
  unsigned short EndHeaderSignature;
} TelefactorFileHeaderBlock;

typedef struct 
{
  unsigned char FOXPRO_Test_ID_Number[FOX_PRO_ID_LEN];
  unsigned char FOXPRO_Patient_ID_Number[FOX_PRO_ID_LEN];
  unsigned char FOXPRO_Referral_ID_Number[FOX_PRO_ID_LEN];
/********************************/
/* Patient specific information */
/********************************/
  char LastName[LAST_NAME_LEN];
  char FirstName[FIRST_NAME_LEN];
  char DOB[DOB_LEN];       /* MUST BE IN THIS FORMAT 'MM <symbol> DD <symbol> CCYY */
  char IDType[ID_TYPE_LEN];
  char IDContents[ID_CONTENT_LEN];
  char HospitalNumber[HOSPTL_NUM_LEN];
  char PhoneNumber[PHONE_NUM_LEN];
  char RoomNumber[ROOM_NUM_LEN];
  char Sex[SEX_LEN];
  char Handedness[HAND_LEN];
  char Classes[CLASSES_LEN];
  char StreetAddress1[STREET_ADDR_LEN];
  char StreetAddress2[STREET_ADDR_LEN];
  char City[CITY_LEN];
  char State[STATE_LEN];
  char ZipCode[ZIP_CODE_LEN];
  char Country[COUNTRY_LEN];
  char Diagnosis[DIAGNOSIS_LEN];
/********************************/
/* Testing specific information */
/********************************/
  char TestNumber[TEST_NUM_LEN];
  char TestType[TEST_TYPE_LEN];
  char TestDate[TEST_DATE_LEN]; /* MUST BE IN THIS FORMAT 'MM <symbol> DD <symbol> CCYY */
  char TestTime[TEST_TIME_LEN]; /* MUST BE IN THIS FORMAT 'HH <symbol> MM - 24 hour */
  char TechName[TECH_NAME_LEN];
  char LabID[LAB_ID_LEN];
  char TestClasses[TEST_CLASS_LEN];
  char TestDateRequested[TEST_DATE_LEN]; /* MUST BE IN THIS FORMAT 'MM <symbol> DD <symbol> CCYY */
  char PatientComplaint[PAT_COMPLNT_LEN];
  char PatientState[PAT_STATE_LEN];
  char PatientMedication[PAT_MEDS_LEN];
  char Electrodes[DB_ELECT_LEN];
  char IsocoderSerialNumber[ISO_SERIAL_LEN];
  char HeadBoxSerialNumber[ISO_HDBOX_LEN];
  char FrontEndDescription[FRONT_END_LEN];
  char DiskName[DISK_NAME_LEN];
/*********************************************/
/* Referral specific information = 140 bytes */
/*********************************************/
  char RefLastName[50];
  char RefFirstName[50];
  char RefTitle[10];
  char RefPhoneNumber[30];
} FoxProDBRecord;
/*****************************/
/*****************************/
typedef struct 
{
  unsigned char  CompanySignature[COMP_SIGNATURE];
  unsigned char  FileTypeSignature[FILE_SIGNATURE];
  unsigned short DBHeaderSize;
  FoxProDBRecord PatientInformation;
  unsigned char  Reserved[141];         /* 141 bytes */
} DataBaseRecord;

/************** Data Block Header **************/

typedef struct 
{
   unsigned long  SyncWord; /* use defines above */
   unsigned short ChangeFlag;  /* non-zero if significant changes between this header and */
                               /* previous header, eg. number of inputs changed */

   unsigned short BlockHeaderLength; /* total length of this header in bytes */
   /*
     BlockHeaderLength = sizeof DATA_BLOCK_HEADER 
                       + sizeof I.D.T. 
                       + sizeof Montage Name 
                       + sizeof O.D.T. 
                       + number of 0xBE End Syncs 
     BlockHeaderLength SHOULD BE ENOUGH TO MAKE THE BEGINNING OF THE
     NEXT DATA BLOCK START ON A 16-BYTE BOUNDARY 
   */
   unsigned long  BlockLength;    /* data following this header, in bytes */
   unsigned long  BlockNumber;    /* zero-based from start of file */
   unsigned long  SystemTime;     /* C-time, GMT no DST  */
   unsigned short FractionTime;   /* 0 - (max_sampling_rate - 1) */
   unsigned long  VCRTime;        /* C-time, GMT no DST  */ 
   unsigned long  VCRFrameCount;  /* 0 - (max_frames_rate - 1), */
                                  /* based on Mains, 30 for 60 Hz, 25 for 50 Hz */
   unsigned short NumberOfInputs;  /* if InputAmplifiers is -1, use this value for number */
                                   /* of input channels in this data block */
   unsigned short NumberOfBits;    /* AC data bits, DC data is stored at same number of bits */
   unsigned short BipolarData;     /* Unsigned(FALSE) or Signed(TRUE) Data */
   unsigned short PackedData;      /* use defines above */
   unsigned short InputDescriptorOffset;  /* relative to file address of this header */
   unsigned short Outputs;                /* outputs in OutputDescriptorBlock to follow */
   unsigned short OutputDescriptorOffset; /* actually points to montage name (32 bytes long) */
} DATA_BLOCK_HEADER;

typedef struct 
{
   unsigned short Selector;              /* 2 bytes  */
   /* for Version 4.2 and higher, Selector will mean channel type (values in TUFFDESC.H) */
   unsigned short SampleRate;            /* 2 bytes  */
   float          CalFactor;             /* 4 bytes  */
   /* if this is a DC variable input, CalFactor is the Slope  */
   float          CalOffset;             /* 4 bytes  */
   /* if this is a DC variable input, CalOffset is the Intercept */
   unsigned char  Label[ELECTRODE_L+1];  /* 6 bytes  */
} InputDescriptorTable;                  /* TOTAL = 18 bytes */

typedef struct 
{
   unsigned short InputType;              /* 2 bytes  */
   /* for Version 4.2 and higher, Selector will mean channel type (values in TUFFDESC.H) */
   unsigned short SampleRate;            /* 2 bytes  */
   float          CalFactor;             /* 4 bytes  */
   /* if this is a DC variable input, CalFactor is the Slope  */
   float          CalOffset;             /* 4 bytes  */
   /* if this is a DC variable input, CalOffset is the Intercept */
   char           Label[1];              /* variable */
} InputDescriptorTableEx;                /* TOTAL = variable */

typedef struct 
{
  unsigned short  G1;                    /* 2 bytes */
  /* for Version 4.2 and higher, G1 will be an index into InputDescriptorTable */
  unsigned short  G2;                    /* 2 bytes */
  /* for Version 4.2 and higher, G2 will be an index into InputDescriptorTable 
   Special channel type codes are also in TUFFDESC.H.
   for less than Version 4.2, the following still applies
       0 - 59999 - AC Channel
   60000 - 63999 - DC Channel
   64000 - 65535 - Special Channel
   64000         - Zero Ref
   64001         - Avg Ref
   64002         - Skip Channel
   64003         - End Montage
   65535         - Time Code      */

  unsigned short  SampleRate;            /* 2 bytes */
  unsigned short  LowFilter;             /* 2 bytes - low filter value * 10  */
  unsigned short  HighFilter;            /* 2 bytes - high filter value * 10, negative = trap on */
  unsigned short  Sensitivity;           /* 2 bytes - nominal value */
  unsigned char   SD;                    /* 1 byte - bit encoded */
                                         /* bit 0 = 0 - surface, 1 - depth */ 
                                         /* bit 1 = 0 - not-locked, 1 - locked */
  unsigned char   Eye;                   /* 1 byte */
  unsigned char   Dir;                   /* 1 byte */
  unsigned char   Status;                /* 1 byte */

  /* Variable length end sync 0xbe */
} OutputDescriptorTable;                 /* TOTAL = 16 bytes */

/************** The Data Block  **************************

    Each packet of samples will consist of :
    Sync       :     Alarm     : THE DATA
  Sync_L bytes : Alarm_L bytes : bytes ? bytes,

  where Sync_L and Alarm_L are usually both 2 bytes.

  Sync_L is the intra-second sample number starting at 0 up 
  to SampleRate-1.

**********************************************************/

/******** Optional Trailer (Time Catalog) ****************/
typedef struct {
    unsigned char  CompanySignature[COMP_SIGNATURE];
    unsigned char  TailerSignature[FILE_SIGNATURE];
    unsigned char  TimeRefRate;     /* every 2,4,5,10, or 30 min. */
} TIME_CATALOG;

/**********************************************************
   Following the Header is a structure
   struct  TimeRefTable {long time - (4 bytes)
                         long file_offset (4 bytes)                   
                        }[number of entries determined by rate]
***********************************************************/

typedef struct 
{
  unsigned char Name[MONTAGE_NAME];
  OutputDescriptorTable Outputs[REFDATA_MAX_OUTPUTS];
} SZ_MONT_STRUCT;

/*
Functions
---------
*/
static int copy_byte_swapped(unsigned char *destination,int number_of_bytes,
	unsigned char *source,unsigned char big_endian)
/*******************************************************************************
LAST MODIFIED : 1 March 1999

DESCRIPTION :
Copies the bytes from <source> into <destination>, swapping the byte order if
required.
==============================================================================*/
{
	unsigned char *destination_byte,*source_byte;
	int i,return_code;

	return_code=0;
	if (destination&&(0<number_of_bytes)&&source)
	{
#if defined (__BYTE_ORDER) && (1234==__BYTE_ORDER)
		if (big_endian)
		{
			source_byte=source;
			destination_byte=destination;
			for (i=number_of_bytes;i>0;i--)
			{
				*destination_byte= *source_byte;
				source_byte++;
				destination_byte++;
			}
		}
		else
		{
			source_byte=source;
			destination_byte=destination+number_of_bytes;
			for (i=number_of_bytes;i>0;i--)
			{
				destination_byte--;
				*destination_byte= *source_byte;
				source_byte++;
			}
		}
#else /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
		if (big_endian)
		{
			source_byte=source;
			destination_byte=destination+number_of_bytes;
			for (i=number_of_bytes;i>0;i--)
			{
				destination_byte--;
				*destination_byte= *source_byte;
				source_byte++;
			}
		}
		else
		{
			source_byte=source;
			destination_byte=destination;
			for (i=number_of_bytes;i>0;i--)
			{
				*destination_byte= *source_byte;
				source_byte++;
				destination_byte++;
			}
		}
#endif /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
	}

	return (return_code);
} /* copy_byte_swapped */

int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
==============================================================================*/
{
	DataBaseRecord database_record;
	DATA_BLOCK_HEADER data_block_header;
	FILE *signal_file,*tuff_file;
	float sampling_frequency;
	InputDescriptorTable input_descriptor;
	int ac_sampling_rate,bytes_per_sample,dc_sampling_rate,*electrodes_in_row,i,index,j,number_of_ac_channels,number_of_dc_channels,number_of_rows,number_of_samples,number_of_signals,offset,return_code=0,*time;
	long start_of_data_block;
	OutputDescriptorTable output_descriptor;
	short sample_offset,*value;
	struct Device **device;
	struct Rig *rig;
	struct Signal_buffer *signal_buffer;
	TelefactorFileHeaderBlock header_block;
	unsigned char buffer[BUFFER_LENGTH+1];
	unsigned short value_1,value_2;

	/* check arguments */
	if (3==argc)
	{
		if (tuff_file=fopen(argv[1],"rb"))
		{
			if (signal_file=fopen(argv[2],"wb"))
			{
				/* read the header block */
				/*???DB.  For SGI the sizeof(TelefactorFileHeaderBlock) is actually 4100
					because it doesn't allow words to start on a half word */
				if (4096==fread(buffer,1,4096,tuff_file))
				{
					/* fill in structure */
					offset=0;
					strncpy((char *)(header_block.CompanySignature),(char *)buffer+offset,
						COMP_SIGNATURE);
					offset += COMP_SIGNATURE;
					strncpy((char *)(header_block.FileTypeSignature),
						(char *)buffer+offset,FILE_SIGNATURE);
					offset += FILE_SIGNATURE;
					copy_byte_swapped((unsigned char *)&(header_block.RefHeaderSize),
						sizeof(header_block.RefHeaderSize),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.RefHeaderSize);
					strncpy((char *)(header_block.RecordingSystemSignature),
						(char *)buffer+offset,16);
					offset += 16;
					strncpy((char *)(header_block.SoftwareVersionSignature),
						(char *)buffer+offset,8);
					offset += 8;
					copy_byte_swapped(
						(unsigned char *)&(header_block.SoftwareSerialNumber),
						sizeof(header_block.SoftwareSerialNumber),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.SoftwareSerialNumber);
					copy_byte_swapped(
						(unsigned char *)&(header_block.InputAmplifierDevice),
						sizeof(header_block.InputAmplifierDevice),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.InputAmplifierDevice);
					strncpy((char *)(header_block.SamplingMethod),(char *)buffer+offset,
						16);
					offset += 16;
					copy_byte_swapped((unsigned char *)&(header_block.DCNotesOffset),
						sizeof(header_block.DCNotesOffset),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.DCNotesOffset);
					copy_byte_swapped(
						(unsigned char *)&(header_block.AmplifierSensitivity),
						sizeof(header_block.AmplifierSensitivity),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.AmplifierSensitivity);
					copy_byte_swapped((unsigned char *)&(header_block.MainsFrequency),
						sizeof(header_block.MainsFrequency),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.MainsFrequency);
					copy_byte_swapped((unsigned char *)&(header_block.InputAmplifiers),
						sizeof(header_block.InputAmplifiers),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.InputAmplifiers);
					copy_byte_swapped((unsigned char *)&(header_block.MaxSamplingRate),
						sizeof(header_block.MaxSamplingRate),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.MaxSamplingRate);
					copy_byte_swapped((unsigned char *)&(header_block.NumberOfBits),
						sizeof(header_block.NumberOfBits),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.NumberOfBits);
					copy_byte_swapped((unsigned char *)&(header_block.PackedData),
						sizeof(header_block.PackedData),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.PackedData);
					copy_byte_swapped((unsigned char *)&(header_block.DataBlockSize),
						sizeof(header_block.DataBlockSize),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.DataBlockSize);
					copy_byte_swapped((unsigned char *)&(header_block.StartTime),
						sizeof(header_block.StartTime),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.StartTime);
					copy_byte_swapped((unsigned char *)&(header_block.EndTime),
						sizeof(header_block.EndTime),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.EndTime);
					copy_byte_swapped((unsigned char *)&(header_block.EventTime),
						sizeof(header_block.EventTime),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.EventTime);
					copy_byte_swapped((unsigned char *)&(header_block.EventType),
						sizeof(header_block.EventType),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.EventType);
					copy_byte_swapped((unsigned char *)&(header_block.SyncLength),
						sizeof(header_block.SyncLength),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.SyncLength);
					copy_byte_swapped((unsigned char *)&(header_block.AlarmLength),
						sizeof(header_block.AlarmLength),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.AlarmLength);
					copy_byte_swapped((unsigned char *)&(header_block.TOD_Start_Time),
						sizeof(header_block.TOD_Start_Time),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.TOD_Start_Time);
					copy_byte_swapped((unsigned char *)&(header_block.TOD_End_Time),
						sizeof(header_block.TOD_End_Time),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.TOD_End_Time);
					copy_byte_swapped((unsigned char *)&(header_block.DescriptorLength),
						sizeof(header_block.DescriptorLength),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.DescriptorLength);
					copy_byte_swapped((unsigned char *)&(header_block.SyncAlarmRate),
						sizeof(header_block.SyncAlarmRate),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.SyncAlarmRate);
					strncpy((char *)(header_block.Reserved),(char *)buffer+offset,3964);
					offset += 3964;
					copy_byte_swapped((unsigned char *)&(header_block.EndHeaderSignature),
						sizeof(header_block.EndHeaderSignature),buffer+offset,BIG_ENDIAN);
					offset += sizeof(header_block.EndHeaderSignature);
					/*???debug */
					printf("header block\n");
					printf("offset=%d\n",offset);
					strncpy(buffer,header_block.CompanySignature,COMP_SIGNATURE);
					buffer[COMP_SIGNATURE]='\0';
					printf("CompanySignature=%s\n",buffer);
					strncpy(buffer,header_block.FileTypeSignature,FILE_SIGNATURE);
					buffer[FILE_SIGNATURE]='\0';
					printf("FileTypeSignature=%s\n",buffer);
					printf("RefHeaderSize=%d\n",header_block.RefHeaderSize);
					strncpy(buffer,header_block.RecordingSystemSignature,16);
					buffer[16]='\0';
					printf("RecordingSystemSignature=%s\n",buffer);
					strncpy(buffer,header_block.SoftwareVersionSignature,8);
					buffer[8]='\0';
					printf("SoftwareVersionSignature=%s\n",buffer);
					printf("SoftwareSerialNumber=%d\n",header_block.SoftwareSerialNumber);
					printf("InputAmplifierDevice=%d\n",header_block.InputAmplifierDevice);
					strncpy(buffer,header_block.SamplingMethod,16);
					buffer[16]='\0';
					printf("SamplingMethod=%s\n",buffer);
					printf("DCNotesOffset=%d\n",header_block.DCNotesOffset);
					printf("AmplifierSensitivity=%d\n",header_block.AmplifierSensitivity);
					printf("MainsFrequency=%d\n",header_block.MainsFrequency);
					printf("InputAmplifiers=%d\n",header_block.InputAmplifiers);
					printf("MaxSamplingRate=%d\n",header_block.MaxSamplingRate);
					printf("NumberOfBits=%d\n",header_block.NumberOfBits);
					printf("PackedData=%d\n",header_block.PackedData);
					printf("DataBlockSize=%d\n",header_block.DataBlockSize);
					printf("StartTime=%d\n",header_block.StartTime);
					printf("EndTime=%d\n",header_block.EndTime);
					printf("EventTime=%d\n",header_block.EventTime);
					printf("EventType=%d\n",header_block.EventType);
					printf("SyncLength=%d\n",header_block.SyncLength);
					printf("AlarmLength=%d\n",header_block.AlarmLength);
					printf("TOD_Start_Time=%d\n",header_block.TOD_Start_Time);
					printf("TOD_End_Time=%d\n",header_block.TOD_End_Time);
					printf("DescriptorLength=%d\n",header_block.DescriptorLength);
					printf("SyncAlarmRate=%d\n",header_block.SyncAlarmRate);
					printf("EndHeaderSignature=%d\n",header_block.EndHeaderSignature);
					printf("\n");
					if (1024==fread(buffer,1,1024,tuff_file))
					{
						/* fill in structure */
						offset=0;
						strncpy((char *)(database_record.CompanySignature),
							(char *)buffer+offset,COMP_SIGNATURE);
						offset += COMP_SIGNATURE;
						strncpy((char *)(database_record.FileTypeSignature),
							(char *)buffer+offset,FILE_SIGNATURE);
						offset += FILE_SIGNATURE;
						copy_byte_swapped((unsigned char *)&(database_record.DBHeaderSize),
							sizeof(database_record.DBHeaderSize),buffer+offset,BIG_ENDIAN);
						offset += sizeof(database_record.DBHeaderSize);
						/*???debug */
						printf("database record\n");
						printf("offset=%d\n",offset);
						strncpy(buffer,database_record.CompanySignature,COMP_SIGNATURE);
						buffer[COMP_SIGNATURE]='\0';
						printf("CompanySignature=%s\n",buffer);
						strncpy(buffer,database_record.FileTypeSignature,FILE_SIGNATURE);
						buffer[FILE_SIGNATURE]='\0';
						printf("FileTypeSignature=%s\n",buffer);
						printf("DBHeaderSize=%d\n",database_record.DBHeaderSize);
						printf("\n");
						/*???DB.  patient information is not needed */
						/* read the data block header */
						start_of_data_block=ftell(tuff_file);
						if (44==fread(buffer,1,44,tuff_file))
						{
							/* fill in structure */
							offset=0;
							copy_byte_swapped((unsigned char *)&(data_block_header.SyncWord),
								sizeof(data_block_header.SyncWord),buffer+offset,BIG_ENDIAN);
							offset += sizeof(data_block_header.SyncWord);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.ChangeFlag),
								sizeof(data_block_header.ChangeFlag),buffer+offset,BIG_ENDIAN);
							offset += sizeof(data_block_header.ChangeFlag);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.BlockHeaderLength),
								sizeof(data_block_header.BlockHeaderLength),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.BlockHeaderLength);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.BlockLength),
								sizeof(data_block_header.BlockLength),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.BlockLength);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.BlockNumber),
								sizeof(data_block_header.BlockNumber),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.BlockNumber);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.SystemTime),
								sizeof(data_block_header.SystemTime),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.SystemTime);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.FractionTime),
								sizeof(data_block_header.FractionTime),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.FractionTime);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.VCRTime),
								sizeof(data_block_header.VCRTime),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.VCRTime);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.VCRFrameCount),
								sizeof(data_block_header.VCRFrameCount),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.VCRFrameCount);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.NumberOfInputs),
								sizeof(data_block_header.NumberOfInputs),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.NumberOfInputs);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.NumberOfBits),
								sizeof(data_block_header.NumberOfBits),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.NumberOfBits);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.BipolarData),
								sizeof(data_block_header.BipolarData),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.BipolarData);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.PackedData),
								sizeof(data_block_header.PackedData),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.PackedData);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.InputDescriptorOffset),
								sizeof(data_block_header.InputDescriptorOffset),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.InputDescriptorOffset);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.Outputs),
								sizeof(data_block_header.Outputs),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.Outputs);
							copy_byte_swapped(
								(unsigned char *)&(data_block_header.OutputDescriptorOffset),
								sizeof(data_block_header.OutputDescriptorOffset),buffer+offset,
								BIG_ENDIAN);
							offset += sizeof(data_block_header.OutputDescriptorOffset);
							/*???debug */
							printf("data block header\n");
							printf("offset=%d\n",offset);
							printf("SyncWord=%X (ref=%X, dif=%X)\n",
								data_block_header.SyncWord,REF_BEG_SYNC,DIF_BEG_SYNC);
							printf("ChangeFlag=%d\n",data_block_header.ChangeFlag);
							printf("BlockHeaderLength=%d\n",
								data_block_header.BlockHeaderLength);
							printf("BlockLength=%d\n",
								data_block_header.BlockLength);
							printf("BlockNumber=%d\n",
								data_block_header.BlockNumber);
							printf("SystemTime=%d\n",
								data_block_header.SystemTime);
							printf("FractionTime=%d\n",
								data_block_header.FractionTime);
							printf("VCRTime=%d\n",
								data_block_header.VCRTime);
							printf("VCRFrameCount=%d\n",
								data_block_header.VCRFrameCount);
							printf("NumberOfInputs=%d\n",
								data_block_header.NumberOfInputs);
							printf("NumberOfBits=%d\n",
								data_block_header.NumberOfBits);
							printf("BipolarData=%d\n",
								data_block_header.BipolarData);
							printf("PackedData=%d\n",
								data_block_header.PackedData);
							printf("InputDescriptorOffset=%d\n",
								data_block_header.InputDescriptorOffset);
							printf("Outputs=%d\n",
								data_block_header.Outputs);
							printf("OutputDescriptorOffset=%d\n",
								data_block_header.OutputDescriptorOffset);
							printf("\n");
							number_of_signals=data_block_header.NumberOfInputs;
							/* create a standard rig */
							number_of_rows=((int)number_of_signals-1)/8+1;
							if (ALLOCATE(electrodes_in_row,int,number_of_rows))
							{
								index=number_of_rows-1;
								electrodes_in_row[index]=(int)number_of_signals-8*index;
								while (index>0)
								{
									index--;
									electrodes_in_row[index]=8;
								}
								if (rig=create_standard_Rig("TUFF",PATCH,MONITORING_OFF,
									EXPERIMENT_OFF,number_of_rows,electrodes_in_row,1,0,(float)1))
								{
									rig->current_region=rig->region_list->region;
									device=rig->devices;
									return_code=1;
									i=0;
									/*???debug */
									printf("input descriptor table\n");
									fseek(tuff_file,
										start_of_data_block+data_block_header.InputDescriptorOffset,
										SEEK_SET);
									number_of_dc_channels=0;
									number_of_ac_channels=0;
									dc_sampling_rate=0;
									ac_sampling_rate=0;
									while (return_code&&(i<number_of_signals))
									{
										if (18==fread(buffer,1,18,tuff_file))
										{
											/* fill in structure */
											offset=0;
											copy_byte_swapped(
												(unsigned char *)&(input_descriptor.Selector),
												sizeof(input_descriptor.Selector),buffer+offset,
												BIG_ENDIAN);
											offset += sizeof(input_descriptor.Selector);
											copy_byte_swapped(
												(unsigned char *)&(input_descriptor.SampleRate),
												sizeof(input_descriptor.SampleRate),buffer+offset,
												BIG_ENDIAN);
											offset += sizeof(input_descriptor.SampleRate);
											copy_byte_swapped(
												(unsigned char *)&(input_descriptor.CalFactor),
												sizeof(input_descriptor.CalFactor),buffer+offset,
												BIG_ENDIAN);
											offset += sizeof(input_descriptor.CalFactor);
											copy_byte_swapped(
												(unsigned char *)&(input_descriptor.CalOffset),
												sizeof(input_descriptor.CalOffset),buffer+offset,
												BIG_ENDIAN);
											offset += sizeof(input_descriptor.CalOffset);
											strncpy((char *)(input_descriptor.Label),
												(char *)buffer+offset,6);
											offset += 6;
											/*???debug */
											buffer[offset]='\0';
											printf("%d.  /%s/ %d %d %g %g",i+1,
												(char *)buffer+offset-6,
												input_descriptor.Selector,input_descriptor.SampleRate,
												input_descriptor.CalFactor,input_descriptor.CalOffset);
											if ((0<input_descriptor.SampleRate)&&
												(input_descriptor.SampleRate<2000))
											{
												if (header_block.SoftwareSerialNumber<VERSION_4_20)
												{
													if (input_descriptor.Selector<DC_CHAN_ONE)
													{
														if (ac_sampling_rate)
														{
															if (ac_sampling_rate!=input_descriptor.SampleRate)
															{
																printf(
													"Different sampling rates for different channels\n");
																return_code=0;
															}
														}
														else
														{
															ac_sampling_rate=input_descriptor.SampleRate;
														}
														number_of_ac_channels++;
													}
													else
													{
														printf("Currently only AC inputs\n");
														return_code=0;
														if (dc_sampling_rate)
														{
															if (dc_sampling_rate!=input_descriptor.SampleRate)
															{
																printf(
													"Different sampling rates for different channels\n");
																return_code=0;
															}
														}
														else
														{
															dc_sampling_rate=input_descriptor.SampleRate;
														}
														number_of_dc_channels++;
													}
												}
												else
												{
													printf("Currently only less than version 4.20\n");
													return_code=0;
												}
											}
											(*device)->channel->gain=(input_descriptor.CalFactor)*
												2000./(header_block.AmplifierSensitivity);
											/*???DB.  From D_TUFF.CPP (line 1077), but Lan says that
												range is too small for channels 33-64 */
/*											if (((*device)->channel->gain<0.8)||
												(1.2<(*device)->channel->gain))
											{
												(*device)->channel->gain=1.;
											}*/
											(*device)->channel->offset=input_descriptor.CalOffset;
											/*???debug */
											printf("  %s.  %g %g\n",(*device)->description->name,
												(*device)->channel->gain,(*device)->channel->offset);
											device++;
										}
										else
										{
											printf("Error reading input descriptor table\n");
											return_code=0;
										}
										i++;
									}
									/*???debug */
									printf("\n");
									printf("number_of_dc_channels=%d\n",number_of_dc_channels);
									printf("number_of_ac_channels=%d\n",number_of_ac_channels);
									printf("dc_sampling_rate=%d\n",dc_sampling_rate);
									printf("ac_sampling_rate=%d\n",ac_sampling_rate);
									printf("\n");
									if (return_code)
									{
										/*???debug */
										printf("montage\n");
										if (MONTAGE_NAME==fread(buffer,1,MONTAGE_NAME,tuff_file))
										{
											/*???debug */
											buffer[MONTAGE_NAME]='\0';
											printf("%s\n",(char *)buffer);
											printf("\n");
											/*???debug */
											printf("output descriptor table\n");
											i=0;
											while (return_code&&(i<data_block_header.Outputs))
											{
												if (16==fread(buffer,1,16,tuff_file))
												{
													/* fill in structure */
													offset=0;
													copy_byte_swapped(
														(unsigned char *)&(output_descriptor.G1),
														sizeof(output_descriptor.G1),buffer+offset,
														BIG_ENDIAN);
													offset += sizeof(output_descriptor.G1);
													copy_byte_swapped(
														(unsigned char *)&(output_descriptor.G2),
														sizeof(output_descriptor.G2),buffer+offset,
														BIG_ENDIAN);
													offset += sizeof(output_descriptor.G2);
													copy_byte_swapped(
														(unsigned char *)&(output_descriptor.SampleRate),
														sizeof(output_descriptor.SampleRate),buffer+offset,
														BIG_ENDIAN);
													offset += sizeof(output_descriptor.SampleRate);
													copy_byte_swapped(
														(unsigned char *)&(output_descriptor.LowFilter),
														sizeof(output_descriptor.LowFilter),buffer+offset,
														BIG_ENDIAN);
													offset += sizeof(output_descriptor.LowFilter);
													copy_byte_swapped(
														(unsigned char *)&(output_descriptor.HighFilter),
														sizeof(output_descriptor.HighFilter),
														buffer+offset,BIG_ENDIAN);
													offset += sizeof(output_descriptor.HighFilter);
													copy_byte_swapped(
														(unsigned char *)&(output_descriptor.Sensitivity),
														sizeof(output_descriptor.Sensitivity),
														buffer+offset,BIG_ENDIAN);
													offset += sizeof(output_descriptor.Sensitivity);
													output_descriptor.SD=buffer[offset];
													offset++;
													output_descriptor.Eye=buffer[offset];
													offset++;
													output_descriptor.Dir=buffer[offset];
													offset++;
													output_descriptor.Status=buffer[offset];
													offset++;
													/*???debug */
													printf("%d.  %d %d %d %d %d %d\n",i+1,
														output_descriptor.G1,output_descriptor.G2,
														output_descriptor.SampleRate,output_descriptor.LowFilter,
														output_descriptor.HighFilter,output_descriptor.Sensitivity);
												}
												else
												{
													printf("Error reading output descriptor table\n");
													return_code=0;
												}
												i++;
											}
											/*???debug */
											printf("\n");
											if (return_code)
											{
												/* goto start of data */
												fseek(tuff_file,start_of_data_block+
													data_block_header.BlockHeaderLength,SEEK_SET);
												if (0==strncmp(REFERENTIALDATA,
													header_block.FileTypeSignature,
													strlen(REFERENTIALDATA)))
												{
													bytes_per_sample=
														number_of_ac_channels*(header_block.NumberOfBits)/8+
														(header_block.SyncLength)+
														(header_block.AlarmLength);
													number_of_samples=(header_block.DataBlockSize)*
														ac_sampling_rate;
													sampling_frequency=(float)ac_sampling_rate;
													if ((0<number_of_samples)&&(signal_buffer=
														create_Signal_buffer(SHORT_INT_VALUE,
														number_of_signals,number_of_samples,
														sampling_frequency)))
													{
														/* initialize times */
														time=signal_buffer->times;
														for (i=0;i<number_of_samples;i++)
														{
															*time=i;
															time++;
														}
														/* link the devices to the signals */
														device=rig->devices;
														i=0;
														while (return_code&&(i<number_of_signals))
														{
															if (!((*device)->signal=create_Signal(
																(*device)->channel->number,signal_buffer,
																UNDECIDED,0)))
															{
																printf("Could not create signal\n");
																return_code=0;
															}
															i++;
															device++;
														}
														if (return_code)
														{
															/* read values */
															value=(signal_buffer->signals).short_int_values;
															return_code=1;
															i=number_of_samples;
															if (data_block_header.BipolarData)
															{
																sample_offset=0;
															}
															else
															{
																sample_offset=2048;
															}
															/*???debug */
															printf("sample_offset=%d\n",sample_offset);
															while (return_code&&(i>0))
															{
																if (bytes_per_sample==fread(buffer,1,
																	bytes_per_sample,tuff_file))
																{
																	/*???debug */
																	{
																		static int count=0;
																		short temp_short_1,temp_short_2;

																		if (count<20)
																		{
																			count++;
																			copy_byte_swapped(
																				(unsigned char *)&temp_short_1,2,
																				buffer,BIG_ENDIAN);
																			copy_byte_swapped(
																				(unsigned char *)&temp_short_2,2,
																				buffer+2,BIG_ENDIAN);
																			printf(
																				"%d.  sync=%d (%X).  alarm=%d (%X)\n",
																				count,temp_short_1,temp_short_1,
																				temp_short_2,temp_short_2);
																		}
																	}
																	if (data_block_header.PackedData)
																	{
																		offset=(header_block.SyncLength)+
																			(header_block.AlarmLength);
																		for (j=number_of_signals/2;j>0;j--)
																		{
																			value_1=(unsigned short)(buffer[offset]);
																			offset++;
																			value_2=(unsigned short)(buffer[offset]);
																			offset++;
																			value_1=(value_1<<4)+
																				(unsigned short)((buffer[offset])>>4);
																			value_2=(value_2<<4)+
																				(unsigned short)((buffer[offset])&0xF);
																			offset++;
																			*value=sample_offset-value_1;
																			value++;
																			*value=sample_offset-value_2;
																			value++;
																		}
																	}
																	else
																	{
																		offset=(header_block.SyncLength)+
																			(header_block.AlarmLength);
																		for (j=number_of_signals;j>0;j--)
																		{
																			copy_byte_swapped((unsigned char *)value,
																				sizeof(short),buffer+offset,BIG_ENDIAN);
																			*value=sample_offset-(*value);
																			value++;
																			offset += 2;
																		}
																	}
																	i--;
																}
																else
																{
																	printf("Error reading data\n");
																	return_code=0;
																}
															}
															if (return_code)
															{
																/*???debug */
																printf("end of file? %d\n",ftell(tuff_file));
																if (write_signal_file(signal_file,rig))
																{
																	printf("Created signal file: %s\n",argv[2]);
																}
																else
																{
																	printf("ERROR.  Writing signal file\n");
																	return_code=0;
																}
															}
														}
													}
													else
													{
														printf("Could not create signal buffer %d %d\n",
															number_of_samples,number_of_signals);
														return_code=0;
													}
												}
												else
												{
													printf("Currently only referential data\n");
													return_code=0;
												}
											}
										}
										else
										{
											printf("Error reading montage\n");
											return_code=0;
										}
									}
								}
								else
								{
									printf("Error creating standard rig\n");
									return_code=0;
								}
							}
							else
							{
								printf("Could not allocate electrodes_in_row\n");
								return_code=0;
							}
						}
						else
						{
							printf("Could not read data block header\n");
							return_code=0;
						}
					}
					else
					{
						printf("Could not read database record\n");
						return_code=0;
					}
				}
				else
				{
					printf("Could not read header block\n");
					return_code=0;
				}
				fclose(signal_file);
			}
			else
			{
				printf("Could not open signal file: %s\n",argv[2]);
				return_code=0;
			}
			fclose(tuff_file);
		}
		else
		{
			printf("Could not open header file: %s\n",argv[1]);
			return_code=0;
		}
	}
	else
	{
		printf( "usage: tuff2sig tuff_file signal_file\n");
		printf("  tuff_file is the name of the TUFF file (provided)\n");
		printf("  signal_file is the name of the signal file (created)\n");
		return_code=0;
	}
#if defined (OLD_CODE)
	char *channel_names,*delta8,*delta8s,*device_name;
	FILE *eeg_file,*signal_file,*tag_file;
	float gain=1,sampling_frequency=1/0.00375;
	int device_number,i,j,number_of_signals,number_of_samples,return_code=0,step,
		*time;
	long int event_addr,eeg_file_size,event_time,minimum_event_addr,start_time;
	short adc_range,bits_per_sample,data_format,eeg_gain,event_type,
		number_of_channels,offset,sample_rate,tag_type,*value;
	struct Channel *channel;
	struct Device **device,**devices;
	struct Device_description *description;
	struct Region *region;
	struct Region_list_item *region_list;
	struct Rig *rig;
	struct Signal *signal;
	struct Signal_buffer *signal_buffer;
	unsigned char buffer[BUFFER_LENGTH+1];
	unsigned short tag_length;

	/* check arguments */
	if (4==argc)
	{
		if (tag_file=fopen(argv[1],"rb"))
		{
			/* read the tag file */
			minimum_event_addr=0;
			bits_per_sample=12;
			sample_rate=200;
			number_of_channels=128;
			channel_names=(char *)NULL;
			while (sizeof(TAG)==fread(buffer,1,sizeof(TAG),tag_file))
			{
				copy_byte_swapped((unsigned char *)&tag_type,sizeof(tag_type),buffer,
					BIG_ENDIAN);
				copy_byte_swapped((unsigned char *)&tag_length,sizeof(tag_length),
					buffer+sizeof(tag_length),BIG_ENDIAN);
				switch (tag_type)
				{
					case TAG_COMMENT:
					case TAG_DATE:
					case TAG_PAT_BD:
					case TAG_PAT_ID:
					case TAG_PAT_NAME:
					case TAG_PAT_SEX:
					case TAG_TIME:
					{
						switch (tag_type)
						{
							case TAG_COMMENT:
							{
								printf("Comment: ");
							} break;
							case TAG_DATE:
							{
								printf("Date: ");
							} break;
							case TAG_PAT_BD:
							{
								printf("Patient birthday: ");
							} break;
							case TAG_PAT_ID:
							{
								printf("Patient id: ");
							} break;
							case TAG_PAT_NAME:
							{
								printf("Patient name: ");
							} break;
							case TAG_PAT_SEX:
							{
								printf("Patient sex: ");
							} break;
							case TAG_TIME:
							{
								printf("Time: ");
							} break;
						}
						while (tag_length>0)
						{
							if (tag_length>BUFFER_LENGTH)
							{
								fread(buffer,1,BUFFER_LENGTH,tag_file);
								buffer[BUFFER_LENGTH]='\0';
								tag_length -= BUFFER_LENGTH;
							}
							else
							{
								fread(buffer,1,tag_length,tag_file);
								buffer[tag_length]='\0';
								tag_length=0;
							}
							printf("%s",(char *)buffer);
						}
						printf("\n");
					} break;
					case TAG_START_TIME:
					{
						fread(buffer,1,sizeof(start_time),tag_file);
						copy_byte_swapped((unsigned char *)&start_time,sizeof(start_time),
							buffer,BIG_ENDIAN);
						printf(
"Start time (seconds from 1-1-1970 to midnight on the first day of recording): %ld\n",
							start_time);
					} break;
					case TAG_DATA_FORMAT:
					{
						fread(buffer,1,sizeof(data_format),tag_file);
						copy_byte_swapped((unsigned char *)&data_format,sizeof(data_format),
							buffer,BIG_ENDIAN);
						printf("Data format");
						if (15!=data_format)
						{
							printf(".  Incorrect: %d (15)",data_format);
						}
						printf("\n");
					} break;
					case TAG_NCHANNELS:
					{
						fread(buffer,1,sizeof(number_of_channels),tag_file);
						copy_byte_swapped((unsigned char *)&number_of_channels,
							sizeof(number_of_channels),buffer,BIG_ENDIAN);
						printf("Number of channels: %d\n",number_of_channels);
					} break;
					case TAG_SAMPLE_RATE:
					{
						fread(buffer,1,sizeof(sample_rate),tag_file);
						copy_byte_swapped((unsigned char *)&sample_rate,
							sizeof(sample_rate),buffer,BIG_ENDIAN);
						printf("Sampling rate: %d\n",sample_rate);
					} break;
					case TAG_BITS_PER_SAMPLE:
					{
						fread(buffer,1,sizeof(bits_per_sample),tag_file);
						copy_byte_swapped((unsigned char *)&bits_per_sample,
							sizeof(bits_per_sample),buffer,BIG_ENDIAN);
						printf("Bits per sample: %d\n",bits_per_sample);
					} break;
					case TAG_EEG_GAIN:
					{
						fread(buffer,1,sizeof(eeg_gain),tag_file);
						copy_byte_swapped((unsigned char *)&eeg_gain,
							sizeof(eeg_gain),buffer,BIG_ENDIAN);
						printf("EEG gain: %d\n",eeg_gain);
						/*???DB.  Assume that eeg_gain is microV per 1000 bits */
						gain=(float)eeg_gain/(float)1000;
					} break;
					case TAG_ADC_RANGE:
					{
						fread(buffer,1,sizeof(adc_range),tag_file);
						copy_byte_swapped((unsigned char *)&adc_range,
							sizeof(adc_range),buffer,BIG_ENDIAN);
						printf("ADC range: %d\n",adc_range);
					} break;
					case TAG_PROTOCOL:
					{
						printf("Protocol");
						if (sizeof(PROTOCOL)==tag_length)
						{
							if (ALLOCATE(channel_names,char,MAXNCHANNELS*CHAN_NAME_LEN))
							{
								fseek(tag_file,tag_length-MAXNCHANNELS*CHAN_NAME_LEN,SEEK_CUR);
								fread(channel_names,1,MAXNCHANNELS*CHAN_NAME_LEN,tag_file);
#if defined (DEBUG)
								/*???debug */
								for (i=0;i<MAXNCHANNELS;i++)
								{
									printf("\nchannel %d.  %.*s",i+1,CHAN_NAME_LEN,
										channel_names+(i*CHAN_NAME_LEN));
								}
#endif /* defined (DEBUG) */
							}
							else
							{
								printf(".  Could not allocate channel_names");
							}
						}
						else
						{
							printf(".  Incorrect length: %d (%d)",tag_length,
								(int)sizeof(PROTOCOL));
							fseek(tag_file,tag_length,SEEK_CUR);
						}
						printf("\n");
					} break;
					case TAG_MONTAGE:
					{
						printf("Montage");
						if (sizeof(MONTAGE)!=tag_length)
						{
							printf(".  Incorrect length: %d (%d)",tag_length,
								(int)sizeof(MONTAGE));
						}
						printf("\n");
						fseek(tag_file,tag_length,SEEK_CUR);
					} break;
					case TAG_SDMONTAGE:
					{
						printf("SD-Montage");
						if (sizeof(MONTAGE)!=tag_length)
						{
							printf(".  Incorrect length: %d (%d)",tag_length,
								(int)sizeof(MONTAGE));
						}
						printf("\n");
						fseek(tag_file,tag_length,SEEK_CUR);
					} break;
					case TAG_SDPARAMETERS:
					{
						printf("SD-Parameters\n");
						fseek(tag_file,tag_length,SEEK_CUR);
					} break;
					case TAG_ARCHIVE:
					{
						printf("Archive");
						if (0!=tag_length)
						{
							printf(".  Incorrect length: %d (0)",tag_length);
							fseek(tag_file,tag_length,SEEK_CUR);
						}
						printf("\n");
					} break;
					case TAG_END_OF_HEADER_DATA:
					{
						printf("End of header data");
						if (0!=tag_length)
						{
							printf(".  Incorrect length: %d (0)",tag_length);
							fseek(tag_file,tag_length,SEEK_CUR);
						}
						printf("\n");
					} break;
					case TAG_EVENT:
					{
						fread(buffer,1,10,tag_file);
						copy_byte_swapped((unsigned char *)&event_type,sizeof(event_type),
							buffer,BIG_ENDIAN);
						copy_byte_swapped((unsigned char *)&event_time,sizeof(event_time),
							buffer+sizeof(event_type),BIG_ENDIAN);
						copy_byte_swapped((unsigned char *)&event_addr,sizeof(event_addr),
							buffer+(sizeof(event_type)+sizeof(event_time)),BIG_ENDIAN);
						if (event_addr>0)
						{
							if ((0==minimum_event_addr)||(event_addr<minimum_event_addr))
							{
								minimum_event_addr=event_addr;
							}
						}
/*						printf("Event.  type=%d.  time=%ld.  address=%ld\n",event_type,
							event_time,event_addr);*/
						fseek(tag_file,tag_length-10,SEEK_CUR);
					} break;
					default:
					{
						/*???debug */
						printf("tag_type=%d, tag_length=%d\n",tag_type,tag_length);
						fseek(tag_file,tag_length,SEEK_CUR);
					} break;
				}
			}
			if (feof(tag_file))
			{
				/*???debug */
				printf("Finished reading tag file\n");
				if (eeg_file=fopen(argv[2],"rb"))
				{
					if (signal_file=fopen(argv[3],"wb"))
					{
						number_of_signals=number_of_channels;
						/* determine the number of samples */
						fseek(eeg_file,0,SEEK_END);
						eeg_file_size=ftell(eeg_file);
						rewind(eeg_file);
						step=sample_rate/5;
						number_of_samples=
							(eeg_file_size/(number_of_channels*(step+1)))*step;
#if defined (DEBUG)
						/*???debug */
						printf("number_of_samples=%d\n",number_of_samples);
						printf("number_of_signals=%d\n",number_of_signals);
#endif /* defined (DEBUG) */
						sampling_frequency=(float)sample_rate;
						if ((0<number_of_samples)&&(signal_buffer=
							create_Signal_buffer(SHORT_INT_VALUE,number_of_signals,
							number_of_samples,sampling_frequency))&&
							ALLOCATE(delta8s,char,number_of_signals*(step-1)))
						{
							/* initialize times */
							time=signal_buffer->times;
							for (i=0;i<number_of_samples;i++)
							{
								*time=i;
								time++;
							}
							/* read values */
							offset=1;
							for (i=bits_per_sample-1;i>0;i--)
							{
								offset *= 2;
							}
							value=(signal_buffer->signals).short_int_values;
							return_code=1;
							i=number_of_samples/step;
							while (return_code&&(i>0))
							{
								if (number_of_signals==fread((char *)(value+number_of_signals),
									sizeof(short),number_of_signals,eeg_file))
								{
									/* byte swap and adjust zero level */
#if defined (__BYTE_ORDER) && (1234==__BYTE_ORDER)
									for (j=number_of_signals;j>0;j--)
									{
										*value=value[number_of_signals]-offset;
#if defined (DEBUG)
										/*???debug */
										printf("%d  %04x  %04x  %d  %d\n",number_of_signals-j+1,
											*(value+number_of_signals),*value,*value,offset);
#endif /* defined (DEBUG) */
										value++;
									}
#else /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
									for (j=number_of_signals;j>0;j--)
									{
										((char *)value)[1]=((char *)(value+number_of_signals))[0];
										((char *)value)[0]=((char *)(value+number_of_signals))[1];
#if defined (DEBUG)
										/*???debug */
										printf("%d  %04x  %04x  %d  %d\n",number_of_signals-j+1,
											*(value+number_of_signals),*value,*value,offset);
#endif /* defined (DEBUG) */
										*value -= offset;
										value++;
									}
#endif /* defined (__BYTE_ORDER) && (1234==__BYTE_ORDER) */
									if (number_of_signals*(step-1)==fread(delta8s,sizeof(char),
										number_of_signals*(step-1),eeg_file))
									{
										delta8=delta8s;
										for (j=number_of_signals*(step-1);j>0;j--)
										{
											*value=
												*(value-number_of_signals)+(signed char)(*delta8);
#if defined (DEBUG)
											/*???debug */
											if (0==j%number_of_signals)
											{
												printf("%d  %02x  %hd  %d %d\n",
													step-(j/number_of_signals),*delta8,(short)(*delta8),
													*(value-number_of_signals),*value);
											}
#endif /* defined (DEBUG) */
											value++;
											delta8++;
										}
									}
									else
									{
										printf("ERROR.  Error reading delta8s for step.  %d %d\n",
											number_of_signals,step);
										return_code=0;
									}
								}
								else
								{
									printf("ERROR.  Error reading base values for step\n");
									return_code=0;
								}
								i--;
							}
							if (return_code)
							{
								ALLOCATE(devices,struct Device *,number_of_signals);
								if (channel_names)
								{
									ALLOCATE(device_name,char,CHAN_NAME_LEN+1);
								}
								else
								{
									ALLOCATE(device_name,char,
										2+(int)log10((double)number_of_signals));
								}
								if (devices&&device_name)
								{
									/* create the region */
									if ((region=create_Region("region",PATCH,0,
										number_of_signals))&&(region_list=
										create_Region_list_item(region,
										(struct Region_list_item *)NULL)))
									{
										/* create the devices */
										device=devices;
										device_number=0;
										while ((device_number<number_of_signals)&&return_code)
										{
											if (channel_names)
											{
												sprintf(device_name,"%.*s",CHAN_NAME_LEN,
													channel_names+(device_number*CHAN_NAME_LEN));
											}
											else
											{
												sprintf(device_name,"%d",device_number+1);
											}
											if ((description=create_Device_description(
												device_name,ELECTRODE,region))&&(channel=
												create_Channel(device_number+1,(float)0,gain))&&
												(signal=create_Signal(device_number,signal_buffer,
												UNDECIDED,0))&&(*device=create_Device(device_number,
												description,channel,signal)))
											{
												description->properties.electrode.position.x=
													(float)(device_number%8);
												description->properties.electrode.position.y=
													(float)(device_number/8);
												device_number++;
												device++;
											}
											else
											{
												printf(
													"ERROR.  Could not allocate memory for device\n");
												return_code=0;
											}
										}
										if (return_code)
										{
											/* create the rig */
											if (rig=create_Rig("BMSI",MONITORING_OFF,
												EXPERIMENT_OFF,number_of_signals,devices,
												(struct Page_list_item *)NULL,1,region_list,
												(struct Region *)NULL))
											{
												if (write_signal_file(signal_file,rig))
												{
													printf("Created signal file: %s\n",argv[3]);
												}
												else
												{
													printf("ERROR.  Writing signal file\n");
													return_code=0;
												}
											}
											else
											{
												printf("ERROR.  Could not create rig\n");
												return_code=0;
											}
										}
									}
									else
									{
										printf(
											"ERROR.  Could not allocate memory for region\n");
										return_code=0;
									}
								}
								else
								{
									printf(
									"ERROR.  Could not allocate memory for device list\n");
									return_code=0;
								}
							}
							DEALLOCATE(delta8s);
						}
						else
						{
							printf(
								"ERROR.  Could not create combined signal buffer/delta8s\n");
							printf("  number_of_signals=%d\n",number_of_signals);
							printf("  number_of_samples=%d\n",number_of_samples);
							printf("  step=%d\n",step);
							if (0<number_of_samples)
							{
								destroy_Signal_buffer(&signal_buffer);
							}
							return_code=0;
						}
						fclose(signal_file);
					}
					else
					{
						printf("Could not open signal file: %s\n",argv[3]);
						return_code=0;
					}
					fclose(eeg_file);
				}
				else
				{
					printf("Could not open eeg file: %s\n",argv[2]);
					return_code=0;
				}
			}
			else
			{
				printf("Error reading tag file\n");
			}
			fclose(tag_file);
		}
		else
		{
			printf("Could not open tag file: %s\n",argv[1]);
			return_code=0;
		}
	}
	else
	{
		printf( "usage: bmsi2sig tag_file eeg_file signal_file\n");
		printf("  tag_file is the name of the TAG file (provided)\n");
		printf("  eeg_file is the name of the EEG file (provided)\n");
		printf("  signal_file is the name of the signal file (created)\n");
		return_code=0;
	}
#endif /* defined (OLD_CODE) */

	return (return_code);
} /* main */
