/*******************************************************************************
FILE : change_events.h

LAST MODIFIED : 27 November 2001

DESCRIPTION :
Reads in an events file and a table giving the new electrode name for each
electrode.  Writes out the new events file.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
Main program
------------
*/

int main()
{
	char *amplifier_table[256]=
		{
			"I","II","III","aVR","aVL","aVF","V1","V2","V3","R1","R2","R3","R4","R5",
			"R6",
			(char *)NULL,
			"A1","A2","A3","A4","A5","A6","A7","A8","A9","A10","A11","A12","A13",
			"A14","A15","A16",
			"B1","B2","B3","B4","B5","B6","B7","B8","B9","B10","B11","B12","B13",
			"B14","B15","B16",
			"C1","C2","C3","C4","C5",
			(char *)NULL,(char *)NULL,(char *)NULL,
			"C6","C7","C8","C9","C10","C11","C12","C13","C14","C15","C16",
			"D1","D2","D3","D4","D5","D6","D7","D8","D9","D10","D11","D12","D13",
			"D14","D15","D16",
			"E1","E2","E3","E4","E5","E6","E7","E8","E9","E10",
			(char *)NULL,(char *)NULL,(char *)NULL,
			"E11","E12","E13","E14","E15","E16",
			"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12","F13",
			"F14","F15","F16",
			"G1","G2","G3","G4","G5","G6","G7","G8","G9","G10","G11","G12","G13",
			"G14","G15","G16",
			(char *)NULL,(char *)NULL,
			"H1","H2","H3","H4","H5","H6","H7","H8","H9","H10","H11","H12","H13",
			"H14","H15","H16",
			"I1","I2","I3","I4","I5","I6","I7","I8","I9","I10","I11","I12","I13",
			"I14","I15","I16",
			"J1","J2","J3","J4","J5",
			(char *)NULL,(char *)NULL,(char *)NULL,
			"J6","J7","J8","J9","J10","J11","J12","J13","J14","J15","J16",
			"K1","K2","K3","K4","K5","K6","K7","K8","K9","K10","K11","K12","K13",
			"K14","K15","K16",
			"L1","L2","L3","L4","L5","L6","L7","L8","L9","L10",
			(char *)NULL,(char *)NULL,(char *)NULL,
			"L11","L12","L13","L14","L15","L16",
			"M1","M2","M3","M4","M5","M6","M7","M8","M9","M10","M11","M12","M13",
			"M14","M15","M16",
			"N1","N2","N3","N4","N5","N6","N7","N8","N9","N10","N11","N12","N13",
			"N14","N15","N16",
			(char *)NULL,(char *)NULL
		};
	char temp_string[10000];
	FILE *input_file,*output_file,*translation_table_file;
	int i,new_electrode,*new_electrodes,number_of_characters,number_of_electrodes,
		old_electrode,*old_electrodes, return_code;
	struct Device **device;
	struct Rig *rig;

	/* zero is a successful return */
	return_code = 0;
	printf("Events file ?\n");
	scanf("%s",temp_string);
	if (input_file=fopen(temp_string,"r"))
	{
		printf("Translation table file (new_electrode old_electrode) ?\n");
		scanf("%s",temp_string);
		if (translation_table_file=fopen(temp_string,"r"))
		{
			number_of_electrodes=0;
			while (2==fscanf(translation_table_file," %d %d ",&new_electrode,
				&old_electrode))
			{
				number_of_electrodes++;
			}
			if (feof(translation_table_file)&&(0<number_of_electrodes))
			{
				if ((old_electrodes=(int *)malloc(number_of_electrodes*sizeof(int)))&&
					(new_electrodes=(int *)malloc(number_of_electrodes*sizeof(int))))
				{
					rewind(translation_table_file);
					number_of_electrodes=0;
					while (2==fscanf(translation_table_file," %d %d ",
						new_electrodes+number_of_electrodes,
						old_electrodes+number_of_electrodes))
					{
						number_of_electrodes++;
					}
					printf("New events file ?\n");
					scanf("%s",temp_string);
					if (output_file=fopen(temp_string,"w"))
					{
						/* read the signal file name */
						fscanf(input_file,"%[^\n]",temp_string);
						fscanf(input_file,"\n");
						fprintf(output_file,"%s\n",temp_string);
						/* read the table format */
						fscanf(input_file,"%[^\n]",temp_string);
						if (!strncmp("table",temp_string,5))
						{
							fscanf(input_file,"\n");
							fprintf(output_file,"%s\n",temp_string);
							fscanf(input_file,"%[^\n]",temp_string);
						}
						/* read the detection method */
						if (!strncmp("threshold",temp_string+19,9))
						{
							fscanf(input_file,"\n");
							fprintf(output_file,"%s\n",temp_string);
							fscanf(input_file,"%[^\n]",temp_string);
						}
						fscanf(input_file,"\n");
						fprintf(output_file,"%s\n",temp_string);
						/* read the start search and end search sample numbers */
						fscanf(input_file,"%[^\n]",temp_string);
						fscanf(input_file,"\n");
						fprintf(output_file,"%s\n",temp_string);
						/* read the number of activations being input */
						fscanf(input_file,"%[^\n]",temp_string);
						fscanf(input_file,"\n");
						fprintf(output_file,"%s\n",temp_string);
						/* read the heading line */
						fscanf(input_file,"\n");
						fprintf(output_file,"\n");
						fscanf(input_file,"%[^\n]",temp_string);
						fscanf(input_file,"\n");
						fprintf(output_file,"%s\n",temp_string);
						/* read the electrodes */
						fscanf(input_file,"%[^\n]",temp_string);
						while (!feof(input_file)&&strncmp("Reference",temp_string,9))
						{
							fscanf(input_file,"\n");
							sscanf(temp_string,"%d%n",&old_electrode,&number_of_characters);
							i=0;
							while ((i<number_of_electrodes)&&
								(old_electrode!=old_electrodes[i]))
							{
								i++;
							}
							if (i<number_of_electrodes)
							{
								i=new_electrodes[i]-1;
								if ((i>15)&&(amplifier_table[i]))
								{
									fprintf(output_file,"%s%s\n",amplifier_table[i],
										temp_string+number_of_characters);
								}
							}
							fscanf(input_file,"%[^\n]",temp_string);
						}
						fprintf(output_file,"%s\n",temp_string);
						fclose(output_file);
					}
					else
					{
						printf("Could not open %s\n",temp_string);
						return_code = 1;
					}
				}
				else
				{
					printf("Insufficient memory\n");
					return_code = 1;
				}
			}
			else
			{
				printf("Invalid translation table\n");
				return_code = 1;
			}
			fclose(translation_table_file);
		}
		else
		{
			printf("Could not open %s\n",temp_string);
			return_code = 1;
		}
		fclose(input_file);
	}
	else
	{
		printf("Could not open %s\n",temp_string);
		return_code = 1;
	}

	return (return_code);
} /* main */
