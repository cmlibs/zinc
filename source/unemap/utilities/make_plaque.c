/*******************************************************************************
FILE : make_plaque.c

LAST MODIFIED : 26 August 2000

DESCRIPTION :
Make a unemap configuration file for multiple flexible pcb plaques in a row.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{
	char file_name[121];
	FILE *cnfg_file;
	int electrode_number,i,j,number_of_pcbs,region_number;

	printf("File name? (.cnfg assumed)\n");
	scanf("%s",file_name);
	strcat(file_name,".cnfg");
	if (cnfg_file=fopen(file_name,"wt"))
	{
		fprintf(cnfg_file,"mixed : patch\n");
		region_number=1;
		electrode_number=0;
		do
		{
			printf("Number of 64 channel modules for strip/region %d (0 to end)?\n",
				region_number);
			scanf("%d",&number_of_pcbs);
			if (0<number_of_pcbs)
			{
				fprintf(cnfg_file,"patch : region %d\n",region_number);
				for (i=0;i<number_of_pcbs;i++)
				{
					for (j=0;j<16;j++)
					{
						fprintf(cnfg_file,"electrode : %d\n",electrode_number+i*64+j+1);
						fprintf(cnfg_file,"channel : %d\n",electrode_number+i*64+32-2*j);
						fprintf(cnfg_file,"position : x = %d, y = %d\n",j,
							(number_of_pcbs-i-1)*4+3);
					}
					for (j=0;j<16;j++)
					{
						fprintf(cnfg_file,"electrode : %d\n",electrode_number+i*64+j+17);
						fprintf(cnfg_file,"channel : %d\n",electrode_number+i*64+64-2*j);
						fprintf(cnfg_file,"position : x = %d, y = %d\n",j,
							(number_of_pcbs-i-1)*4+2);
					}
					for (j=0;j<16;j++)
					{
						fprintf(cnfg_file,"electrode : %d\n",electrode_number+i*64+j+33);
						fprintf(cnfg_file,"channel : %d\n",electrode_number+i*64+31-2*j);
						fprintf(cnfg_file,"position : x = %d, y = %d\n",j,
							(number_of_pcbs-i-1)*4+1);
					}
					for (j=0;j<16;j++)
					{
						fprintf(cnfg_file,"electrode : %d\n",electrode_number+i*64+j+49);
						fprintf(cnfg_file,"channel : %d\n",electrode_number+i*64+63-2*j);
						fprintf(cnfg_file,"position : x = %d, y = %d\n",j,
							(number_of_pcbs-i-1)*4);
					}
				}
				region_number++;
				electrode_number += number_of_pcbs*64;
			}
		} while (number_of_pcbs>0);
		fclose(cnfg_file);
	}
	else
	{
		printf("Could not open %s\n",file_name);
	}

	return (1);
} /* main */
