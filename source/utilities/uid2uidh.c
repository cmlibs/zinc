/*******************************************************************************
FILE : uid2uidh.c

LAST MODIFIED : 12 June 2003

DESCRIPTION :
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

int main(int argc, char *argv[])	  
{
	char buffer, *char_data, out_data[6], string_name[100], *dot_ptr,
		*string_ptr;
   FILE *infile, *outfile;
	long byte_data, data;
	int byte_count, i, index;
#if (defined (BYTE_ORDER)) && (1234==BYTE_ORDER)
	int j;
#endif /* (defined (BYTE_ORDER)) && (1234==BYTE_ORDER) */

	if(argc != 3 && argc != 4)
	{
		printf("Usage: uid2uidh infile outfile [string_prefix_name]\n");
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

				if (argc == 4)
				{
					string_ptr = argv[3];
				}
				else
				{
					strcpy(string_name, argv[1]);
					i=strlen(string_name);
					while ((i>0)&&(' '==string_name[i-1]))
					{
						i--;
					}
					while ((i>0)&&('/'==string_name[i-1]))
					{
						i--;
					}
					string_name[i]='\0';
					while ((i>0)&&('/'!=string_name[i-1]))
					{
						i--;
					}
					string_ptr = string_name+i;
					if(dot_ptr = strchr(string_ptr, '.'))
					{
						*dot_ptr = 0;
					}
				}

				fprintf(outfile, "static char %s_uidh[] = \"", string_ptr);

				while(!feof(infile))
				{
					if(1 == fread(&buffer, sizeof(char), 1, infile))
					{
#if defined (DEBUG)
						printf("%d\n", buffer);
#endif /* defined (DEBUG) */
						byte_data = buffer & 255;
						data += byte_data << (8 * byte_count);

						if(byte_count == 3)
						{
							if(!data)
							{
								out_data[0] = '.';
								out_data[1] = '.';
								out_data[2] = '.';
								out_data[3] = '.';
								out_data[4] = '.';
								out_data[5] = '.';
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
									for(i = 0 ; i < 6 ; i++)
									{
										if(char_data[i])
										{
											out_data[5 - i] = char_data[i];
										}
										else
										{
											for (j = 0 ; j < i ; j++)
											{
												out_data[j] = out_data[6 - i + j];
											}
											for ( ; i < 6 ; i++)
											{
												out_data[i] = '.';
											}
										}
									}
								}
								else
								{
#endif /* (defined (BYTE_ORDER)) && (1234==BYTE_ORDER) */
									for( i = 0 ; i < 6 ; i++)
									{
										if(char_data[i])
										{
											out_data[i] = char_data[i];
										}
										else
										{
											for ( ; i < 6 ; i++)
											{
												out_data[i] = '.';
											}
										}
									}
#if (defined (BYTE_ORDER)) && (1234==BYTE_ORDER)
								}
#endif /* (defined (BYTE_ORDER)) && (1234==BYTE_ORDER) */
							}
					
							fwrite(out_data, sizeof(char), 6, outfile);
#if defined (DEBUG)
							printf("%c%c%c%c%c%c\n", out_data[0], 
								out_data[1], out_data[2], out_data[3],
								out_data[4], out_data[5]);
#endif /* defined (DEBUG) */

							data = 0;
							byte_count = 0;
						}
						else
						{
							byte_count++;
						}
					}
				}
				fprintf(outfile, "\";\n");
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
