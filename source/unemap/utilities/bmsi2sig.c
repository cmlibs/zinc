/*******************************************************************************
FILE : bmsi2sig.c

LAST MODIFIED : 7 January 2000

DESCRIPTION :
Converts BMSI 5000 TAG and EEG files to a unemap signal file.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "general/debug.h"
#include "general/myio.h"
#include "unemap/rig.h"

/*
Constants (from BMSI file format descriptions document)
---------
*/
#define TAG_ADC_RANGE 273
#define TAG_ARCHIVE 274
#define TAG_BITS_PER_SAMPLE 271
#define TAG_COMMENT 260
#define TAG_DATA_FORMAT 268
#define TAG_DATE 262
#define TAG_DONT_CARE 0
#define TAG_EEG_GAIN 272
#define TAG_END_OF_HEADER_DATA 512
#define TAG_EVENT 513
#define TAG_MONTAGE 265
#define TAG_NCHANNELS 269
#define TAG_PAT_BD 258
#define TAG_PAT_ID 257
#define TAG_PAT_NAME 256
#define TAG_PAT_SEX 259
#define TAG_PROTOCOL 264
#define TAG_SAMPLE_RATE 270
#define TAG_SDMONTAGE 266
#define TAG_SDPARAMETERS 267
#define TAG_START_TIME 261
#define TAG_TIME 263

#define MAXNCHANNELS 128
#define MAXNTRACES 128
#define MAXNANALOG 31
#define CHAN_NAME_LEN 4

#define BUFFER_LENGTH 256

/* format for .TAG and .EEG */
#define BIG_ENDIAN (unsigned char)1

/*
Types (from BMSI file format descriptions document)
-----
*/
typedef struct
{
	short type;
	unsigned short len;
} TAG;

typedef enum
{
	EVENT_START_STORE=0,
	EVENT_SAMPLE=1,
	EVENT_SPIKE=2,
	EVENT_SEIZURE=3,
	EVENT_PB=4,
	EVENT_END_STORE=5
} EVENTTYPE;

typedef struct
{
	short type;
	long time;
	long addr;
} EVENT;

typedef struct
{
	EVENT event;
	short whatfor;
} START_STORE_EVENT;

typedef struct
{
	/* version number (currently 7) */
	short version;
	/* code name for this protocol */
	char code[8];
	/* previous name (used on pre-7 versions) */
	char abbr[2];
	/* name of protocol */
	char name[24];
	/* number of channels */
	unsigned char nchannels;
	char sample_rate_index;
	/* names of channels */
	char channelname[MAXNCHANNELS][CHAN_NAME_LEN];
} PROTOCOL;

typedef struct
{
	/* grid 1 and 2 */
	char g[2];
	/* sensitivity */
	unsigned char sens;
	/* color */
	unsigned char color;
	/* high filter (low pass) setting */
	unsigned char hff;
	/* low filter (high pass) setting */
	unsigned char lff;
	/* bit-mapped flags */
	unsigned char flags;
	/* pad to 8 bytes for easy access */
	unsigned char SD_flags;
} TRACE;

/*???DB.  Guessing */
typedef short TCFILTER;
typedef short LNFILTER;

typedef struct
{
	/* number of traces */
	unsigned char ntraces[2];
	/* paper speed (0=15,1=30,2=60) */
	unsigned char pspeed[2];
	/* 0=black, 1=white */
	short trace_color;
	/* a multiple of 60 */
	short split_location;
	/* 0=replace, 1=overlay */
	short overlayflag;
	/* always 4 */
	short points_per_packet;
	/*0=scroll, 1=wipe */
	short wipeflag;
	/* 0=single, 1=double */
	short tracewidth;
	/* top trace number */
	unsigned char toptrace[2];
	/* 0=3uv/mm. 1=5uv/mm, 10=150 */
	unsigned char sensitivity[2];
	/* 0=off, 1=on */
	short filter;
	/* 0=not split, 1=split */
	short splitflag;
	/* total number of traces defined */
	short totaltraces;
	/* average reference enabled */
	short avgref;
	/* grid color */
	short gridcolor;
	/* cursor color */
	short cursorcolor;
	/* marker color */
	short markercolor;
	/* tells top video line number to use */
	short startline;
	/* paper color */
	short papercolor;
	/* paper opaque/transparent */
	short papertype;
	/* time constant filter */
	TCFILTER tcfilter;
	/* line filter */
	LNFILTER lnfilter;
	/* event parameters used to be here */
	short szfiller[16];
	/* paging speed */
	char paging_speed;
	/* keep things even */
	char charfiler;
	/* room for expansion (64 word total) */
	short filler[64-39];
	/* name of the montage */
	char name[24];
	/* include in average flags (bit encoded) */
	char incflags[MAXNCHANNELS/8];
	/* analog output channel numbers (64 to allow for expansion) */
	unsigned char analog[64];
	/* define the traces */
	TRACE trace[MAXNTRACES];
} MONTAGE;

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
LAST MODIFIED : 19 October 1999

DESCRIPTION :
==============================================================================*/
{
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

	return (return_code);
} /* main */
