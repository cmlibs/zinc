/*******************************************************************************
FILE : make_plaque.c

LAST MODIFIED : 1 October 2000

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
	int ecg_module,electrode_number,i,j,number_of_pcbs,region_number,start1,
		start2;

	printf("File name? (.cnfg assumed)\n");
	scanf("%s",file_name);
	strcat(file_name,".cnfg");
	if (cnfg_file=fopen(file_name,"wt"))
	{
		printf("Number of 64 channel module with ECG attached (0 for none)?\n");
		scanf("%d",&ecg_module);
		fprintf(cnfg_file,"mixed : patch\n");
		region_number=1;
		electrode_number=0;
		do
		{
			printf("Number of 64 channel modules for strip/region %d (0 to end, negative to skip)?\n",
				region_number);
			scanf("%d",&number_of_pcbs);
			if (0!=number_of_pcbs)
			{
				if (0<number_of_pcbs)
				{
					fprintf(cnfg_file,"patch : region %d\n",region_number);
					for (i=0;i<number_of_pcbs;i++)
					{
						if (ecg_module==(electrode_number/64)+i+1)
						{
							start1=2;
							start2=1;
						}
						else
						{
							start1=0;
							start2=0;
						}
						for (j=start1;j<16;j++)
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
						for (j=start2;j<16;j++)
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
				}
				else
				{
					number_of_pcbs= -number_of_pcbs;
				}
				if ((electrode_number/64<ecg_module)&&
					(ecg_module<=(electrode_number/64)+number_of_pcbs))
				{
					fprintf(cnfg_file,"patch : ecg\n");
					fprintf(cnfg_file,"auxiliary : ecg_%d\n",(ecg_module-1)*64+30);
					fprintf(cnfg_file,"channel : %d\n",(ecg_module-1)*64+30);
					fprintf(cnfg_file,"auxiliary : ecg_%d\n",(ecg_module-1)*64+31);
					fprintf(cnfg_file,"channel : %d\n",(ecg_module-1)*64+31);
					fprintf(cnfg_file,"auxiliary : ecg_%d\n",(ecg_module-1)*64+32);
					fprintf(cnfg_file,"channel : %d\n",(ecg_module-1)*64+32);
					fprintf(cnfg_file,"auxiliary : I\n");
					fprintf(cnfg_file,"sum : ecg_%d-ecg_%d\n",(ecg_module-1)*64+30,
						(ecg_module-1)*64+31);
					fprintf(cnfg_file,"auxiliary : II\n");
					fprintf(cnfg_file,"sum : ecg_%d-ecg_%d\n",(ecg_module-1)*64+32,
						(ecg_module-1)*64+31);
					fprintf(cnfg_file,"auxiliary : III\n");
					fprintf(cnfg_file,"sum : ecg_%d-ecg_%d\n",(ecg_module-1)*64+32,
						(ecg_module-1)*64+30);
					fprintf(cnfg_file,"auxiliary : aVR\n");
					fprintf(cnfg_file,"sum : ecg_%d-0.5*ecg_%d-0.5*ecg_%d\n",
						(ecg_module-1)*64+31,(ecg_module-1)*64+30,(ecg_module-1)*64+32);
					fprintf(cnfg_file,"auxiliary : aVL\n");
					fprintf(cnfg_file,"sum : ecg_%d-0.5*ecg_%d-0.5*ecg_%d\n",
						(ecg_module-1)*64+30,(ecg_module-1)*64+31,(ecg_module-1)*64+32);
					fprintf(cnfg_file,"auxiliary : aVF\n");
					fprintf(cnfg_file,"sum : ecg_%d-0.5*ecg_%d-0.5*ecg_%d\n",
						(ecg_module-1)*64+32,(ecg_module-1)*64+30,(ecg_module-1)*64+31);
				}
				electrode_number += number_of_pcbs*64;
			}
		} while (number_of_pcbs!=0);
		fclose(cnfg_file);
	}
	else
	{
		printf("Could not open %s\n",file_name);
	}

	return (1);
} /* main */
