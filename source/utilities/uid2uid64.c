#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h> /*???DB.  Contains definition of __BYTE_ORDER for Linux */
#include <libgen.h>

/* These functions are not ANSI so don't get included in stdlib.h */
extern long a64l(const char *);
extern char *l64a(long);

int main(int argc, char *argv[])	  
{
	char buffer, *char_data, out_data[6], string_name[100], *dot_ptr,
		*string_ptr;
   FILE *infile, *infile64bit, *outfile;
	long byte_data, data;
	int byte_count, i, index, j;

	if(argc != 3 && argc != 5)
	{
		printf("Usage: uid2uid64 infile [-64bit infile64bit] outfile\n");
	}
	else
	{
		index = 1;
		infile64bit = (FILE *)NULL;
		if(infile = fopen( argv[index], "r"))
		{
			index++;
			if(argv[index][0] == '-')
			{
				if(!strncasecmp(argv[index], "-64bit", 6))
				{
					index++;
					infile64bit = fopen( argv[index], "r");
					index++;
				}
			}
			if(outfile = fopen( argv[index], "w"))
			{
				index++;
				byte_count = 0;
				data = 0;

				strcpy(string_name, argv[1]);
				string_ptr = basename(string_name);
				if(dot_ptr = strchr(string_ptr, '.'))
				{
					*dot_ptr = 0;
				}

				fprintf(outfile, "#if !defined (O64)\n");
				fprintf(outfile, "static char %s_uid64[] = \"", string_ptr);

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
#if (defined (__BYTE_ORDER)) && (1234==__BYTE_ORDER)
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
#else /* (defined (__BYTE_ORDER)) && (1234==__BYTE_ORDER) */
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
#endif /* (defined (__BYTE_ORDER)) && (1234==__BYTE_ORDER) */
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
				if(infile64bit)
				{
					fprintf(outfile, "#else /* !defined (O64) */\n");
					fprintf(outfile, "static char %s_uid64[] = \"", string_ptr);

					byte_count = 0;
					data = 0;

					while(!feof(infile64bit))
					{
						if(1 == fread(&buffer, sizeof(char), 1, infile64bit))
						{
#if defined (DEBUG)
							printf("%d\n", buffer);
#endif /* defined (DEBUG) */
							byte_data = buffer & 255;
							data += byte_data << (8 * byte_count);

							if(byte_count == 3)
							{
								char_data = l64a(data);
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
					fclose(infile64bit);
				}
				fprintf(outfile, "#endif /* !defined (O64) */\n");
				fclose (outfile);
			}
			else
			{
				fprintf(stderr,"uid2uid64.  Unable to open output file %s\n",
					argv[index]);
			}
			fclose (infile);
		}
		else
		{
			fprintf(stderr,"uid2uid64.  Unable to open input file %s\n",
				argv[index]);
		}
   }
	return(0);
}
