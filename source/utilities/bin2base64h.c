/*******************************************************************************
FILE : bin2baseh.c

LAST MODIFIED : 20 June 2003

DESCRIPTION :
Used to be uid2uidh.c but changed to support files that are not a multiple of
four in length.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined (UNIX) && defined (GENERIC_PC)
#if defined (CYGWIN)
#include <sys/param.h>
#else /* defined (CYGWIN) */
#include <endian.h>
#endif /* defined (CYGWIN) */
#endif /* defined (UNIX) && defined (GENERIC_PC) */
#if defined (SGI)
#include <sys/endian.h>
#endif /* defined (SGI) */
#if defined (AIX)
#include <sys/machine.h>
#endif /* defined (AIX) */
#if defined (WIN32_SYSTEM)
#define BYTE_ORDER 1234
#endif /* defined (WIN32_SYSTEM) */
#if defined (CYGWIN)
#include <sys/param.h>
#endif /* defined (CYGWIN) */

#if __GLIBC__ >= 2
#include <gnu/libc-version.h>
#endif

/* These functions are not ANSI so don't get included in stdlib.h */
extern long a64l(const char *);
extern char *l64a(long);

#if defined (BYTE_ORDER)
#if (1234==BYTE_ORDER)
static int glibc_version_greater_than_2_2_4(void)
/*******************************************************************************
LAST MODIFIED : 26 November 2002

DESCRIPTION :
Need to read the glibc version so that we can determine if we need to 
swap the endianness of values going into a64l
==============================================================================*/
{
#if __GLIBC__ >= 2
	char *version_string;
	int major_version, minor_version, minor_sub_version;
#endif /* __GLIBC__ >= 2 */
	static int return_code = -1;

	/* ENTER(glibc_version_greater_than_2_2_4); */

	/* This gets called a lot so lets make it fast */
	if (return_code == -1)
	{
#if __GLIBC__ >= 2
		version_string = (char *)gnu_get_libc_version();
		if (sscanf(version_string, "%d.%d.%d", &major_version, &minor_version, 
			&minor_sub_version))
		{
			
			if ((major_version > 2) ||
				((major_version == 2) && (minor_version > 2)) ||
				((major_version == 2) && (minor_version == 2) && (minor_sub_version > 4)))
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			return_code = 0;
		}
#else /* __GLIBC__ >= 2 */
		return_code = 0;
#endif/* __GLIBC__ >= 2 */
	}
	/* LEAVE; */
	
	return (return_code);
} /* glibc_version_greater_than_2_2_4 */
#endif /* (1234==BYTE_ORDER) */
#endif /* defined (BYTE_ORDER) */

static int write_base64_data(FILE *outfile, int byte_count, long data)
/*******************************************************************************
LAST MODIFIED : 20 June 2003

DESCRIPTION :
==============================================================================*/
{
	char *char_data, bytes_out, out_data[6];
	int i, j;

	switch(byte_count)
	{
		case 1:
		{
			bytes_out = 2;
		} break;
		case 2:
		{
			bytes_out = 3;
		} break;
		case 3:
		{
			bytes_out = 5;
		} break;
		case 4:
		{
			bytes_out = 6;
		} break;
		default:
		{
			printf("write_base64_data:  Unsupported byte_count %d\n", byte_count);
			exit(1);
		} break;
	}

	if(!data)
	{
		for (i = 0 ; i < bytes_out ; i++)
		{
			out_data[i] = '.';
		}
	}
	else
	{
		char_data = l64a(data);
#if defined (DEBUG)
		printf("%c%c%c%c%c%c\n", char_data[0], 
			char_data[1], char_data[2], char_data[3],
			char_data[4], char_data[5]);
#endif /* defined (DEBUG) */
#if (defined (BYTE_ORDER)) && (1234==BYTE_ORDER)
		if (!glibc_version_greater_than_2_2_4())
		{
			for(i = 0 ; i < bytes_out ; i++)
			{
				if(char_data[i])
				{
					out_data[bytes_out - 1 - i] = char_data[i];
				}
				else
				{
					for (j = 0 ; j < i ; j++)
					{
						out_data[j] = out_data[bytes_out - i + j];
					}
					for ( ; i < bytes_out ; i++)
					{
						out_data[i] = '.';
					}
				}
			}
		}
		else
		{
#endif /* (defined (BYTE_ORDER)) && (1234==BYTE_ORDER) */
			for( i = 0 ; i < bytes_out ; i++)
			{
				if(char_data[i])
				{
					out_data[i] = char_data[i];
				}
				else
				{
					for ( ; i < bytes_out ; i++)
					{
						out_data[i] = '.';
					}
				}
			}
#if (defined (BYTE_ORDER)) && (1234==BYTE_ORDER)
		}
#endif /* (defined (BYTE_ORDER)) && (1234==BYTE_ORDER) */
	}
					
	fwrite(out_data, sizeof(char), bytes_out, outfile);
#if defined (DEBUG)
	printf("%c%c%c%c%c%c\n", out_data[0], 
		out_data[1], out_data[2], out_data[3],
		out_data[4], out_data[5]);
#endif /* defined (DEBUG) */
}

int main(int argc, char *argv[])	  
{
	char buffer;
   FILE *infile, *outfile;
	long byte_data, data;
	int byte_count, i, index;
#if (defined (BYTE_ORDER)) && (1234==BYTE_ORDER)
	int j;
#endif /* (defined (BYTE_ORDER)) && (1234==BYTE_ORDER) */

	if(argc != 3)
	{
		printf("Usage: bin2base64h infile outfile\n");
	}
	else
	{
		index = 1;
		if(infile = fopen( argv[index], "r"))
		{
			index++;
			if(outfile = fopen( argv[index], "w"))
			{
				index++;
				byte_count = 0;
				data = 0;

				fprintf(outfile, "\"");

				while(!feof(infile))
				{
					if(1 == fread(&buffer, sizeof(char), 1, infile))
					{
#if defined (DEBUG)
						printf("%d\n", buffer);
#endif /* defined (DEBUG) */
						byte_data = buffer & 255;
						data += byte_data << (8 * byte_count);
						byte_count++;

						if(byte_count == 4)
						{
							write_base64_data(outfile, byte_count, data);

							data = 0;
							byte_count = 0;
						}
					}
				}
				if (byte_count > 0)
				{
					write_base64_data(outfile, byte_count, data);
				}

				fprintf(outfile, "\"\n");
				fclose (outfile);
			}
			else
			{
				fprintf(stderr,"uid2uidh.  Unable to open output file %s\n",
					argv[index]);
			}
			fclose (infile);
		}
		else
		{
			fprintf(stderr,"uid2uidh.  Unable to open input file %s\n",
				argv[index]);
		}
   }
	return(0);
}
