/*******************************************************************************
FILE : make_plaque.c

LAST MODIFIED : 14 October 2002

DESCRIPTION :
Make a unemap configuration file for multiple flexible pcb plaques in a row.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{
	char file_name[121],region_name[121];
	FILE *cnfg_file;
	int bipolar,channel,ecg_module,electrode_number,end1,end2,end3,end4,
		external_module,i,j,number_of_pcbs,number_in_row,old_4by16,
		old_4by16_numbering,start1,start2,start3,start4,strip_number,write_ecg,
		write_external,x,x_start;

	printf("File name? (.cnfg assumed)\n");
	scanf("%s%*c",file_name);
	strcat(file_name,".cnfg");
	if (cnfg_file=fopen(file_name,"wt"))
	{
		printf("Number of 64 channel module with ECG attached (0 for none)?\n");
		scanf("%d%*c",&ecg_module);
		write_ecg=0;
		do
		{
			printf("Number of 64 channel module with external inputs attached "
				"(0 for none)?\n");
			scanf("%d%*c",&external_module);
			if ((external_module==ecg_module)&&(0!=external_module))
			{
				printf(
					"ECG and external inputs can't be attached to the same module\n");
			}
		} while ((external_module==ecg_module)&&(0!=external_module));
		write_external=0;
		fprintf(cnfg_file,"mixed : patch\n");
		do
		{
			printf("Region name?\n");
			scanf("%[^\n]%*c",region_name);
			printf("4x16 (o) or 4x17 (n) ?\n");
			scanf("%s%*c",file_name);
			if (('o'==file_name[0])||('O'==file_name[0]))
			{
				old_4by16=1;
				old_4by16_numbering=1;
			}
			else
			{
				old_4by16=0;
				printf("Have same electrode number position mapping as 4x16 (y/n) ?\n");
				scanf("%s%*c",file_name);
				if (('y'==file_name[0])||('Y'==file_name[0]))
				{
					old_4by16_numbering=1;
				}
				else
				{
					old_4by16_numbering=0;
				}
			}
			printf("Bipolar (b) or unipolar (u) ?\n");
			scanf("%s%*c",file_name);
			if (('b'==file_name[0])||('B'==file_name[0]))
			{
				bipolar=1;
				if (old_4by16)
				{
					number_in_row=15;
				}
				else
				{
					number_in_row=16;
				}
			}
			else
			{
				bipolar=0;
				number_in_row=16;
			}
			fprintf(cnfg_file,"patch : %s\n",region_name);
			electrode_number=0;
			strip_number=1;
			x_start=0;
			do
			{
				printf("Number of 64 channel modules for strip %d of %s"
					" (0 to end, negative to skip) ?\n",strip_number,region_name);
				scanf("%d%*c",&number_of_pcbs);
				if (0!=number_of_pcbs)
				{
					if (0<number_of_pcbs)
					{
						for (i=0;i<number_of_pcbs;i++)
						{
							if ((ecg_module==(electrode_number/64)+i+1)&&bipolar)
							{
								/*???DB.  Currently only for bipolar */
								if (old_4by16)
								{
#if defined (OLD_CODE)
/* pre bipolar adapter ecg */
									start1=2;
									end1=number_in_row;
									start2=0;
									end2=number_in_row;
									start3=1;
									end3=number_in_row;
									start4=0;
									end4=number_in_row;
#endif /* defined (OLD_CODE) */
									start1=0;
									end1=number_in_row;
									start2=0;
									end2=number_in_row;
									start3=0;
									end3=number_in_row-1;
									start4=0;
									end4=number_in_row;
								}
								else
								{
									start1=0;
									end1=number_in_row;
									start2=1;
									end2=number_in_row;
									start3=1;
									end3=number_in_row;
									start4=1;
									end4=number_in_row;
								}
							}
							else
							{
								if ((external_module==(electrode_number/64)+i+1)&&bipolar)
								{
									/*???DB.  Currently only for bipolar */
									if (old_4by16)
									{
										start1=0;
										end1=number_in_row;
										start2=0;
										end2=number_in_row-4;
										start3=0;
										end3=number_in_row;
										start4=0;
										end4=number_in_row-4;
									}
									else
									{
										start1=4;
										end1=number_in_row;
										start2=0;
										end2=number_in_row;
										start3=4;
										end3=number_in_row;
										start4=0;
										end4=number_in_row;
									}
								}
								else
								{
									start1=0;
									end1=number_in_row;
									start2=0;
									end2=number_in_row;
									start3=0;
									end3=number_in_row;
									start4=0;
									end4=number_in_row;
								}
							}
							for (j=start1;j<end1;j++)
							{
								fprintf(cnfg_file,"electrode : %d\n",electrode_number+i*64+j+1);
								if (old_4by16)
								{
									x=j;
									if (bipolar)
									{
										channel=electrode_number+i*64+64-2*j;
									}
									else
									{
										channel=electrode_number+i*64+32-2*j;
									}
								}
								else
								{
									if (old_4by16_numbering)
									{
										x=j;
										if (bipolar)
										{
											channel=electrode_number+i*64+1+2*(start1+end1-1-j);
										}
										else
										{
											channel=electrode_number+i*64+33+2*(start1+end1-1-j);
										}
									}
									else
									{
										x=number_in_row-1-j;
										if (bipolar)
										{
											channel=electrode_number+i*64+1+2*j;
										}
										else
										{
											channel=electrode_number+i*64+33+2*j;
										}
									}
								}
								fprintf(cnfg_file,"channel : %d\n",channel);
								fprintf(cnfg_file,"position : x = %d, y = %d\n",x_start+x,
									(number_of_pcbs-i-1)*4+3);
							}
							for (j=start2;j<end2;j++)
							{
								fprintf(cnfg_file,"electrode : %d\n",
									electrode_number+i*64+j+17);
								if (old_4by16)
								{
									x=j;
									if (bipolar)
									{
										channel=electrode_number+i*64+32-2*j;
									}
									else
									{
										channel=electrode_number+i*64+64-2*j;
									}
								}
								else
								{
									if (old_4by16_numbering)
									{
										x=j;
										if (bipolar)
										{
											channel=electrode_number+i*64+33+2*(start2+end2-1-j);
										}
										else
										{
											channel=electrode_number+i*64+1+2*(start2+end2-1-j);
										}
									}
									else
									{
										x=number_in_row-1-j;
										if (bipolar)
										{
											channel=electrode_number+i*64+33+2*j;
										}
										else
										{
											channel=electrode_number+i*64+1+2*j;
										}
									}
								}
								fprintf(cnfg_file,"channel : %d\n",channel);
								fprintf(cnfg_file,"position : x = %d, y = %d\n",x_start+x,
									(number_of_pcbs-i-1)*4+2);
							}
							for (j=start3;j<end3;j++)
							{
								fprintf(cnfg_file,"electrode : %d\n",
									electrode_number+i*64+j+33);
								if (old_4by16)
								{
									x=j;
									if (bipolar)
									{
										channel=electrode_number+i*64+61-2*j;
									}
									else
									{
										channel=electrode_number+i*64+31-2*j;
									}
								}
								else
								{
									if (old_4by16_numbering)
									{
										x=j;
										if (bipolar)
										{
											channel=electrode_number+i*64+2+2*(start3+end3-1-j);
										}
										else
										{
											channel=electrode_number+i*64+34+2*(start3+end3-1-j);
										}
									}
									else
									{
										x=number_in_row-1-j;
										if (bipolar)
										{
											channel=electrode_number+i*64+2+2*j;
										}
										else
										{
											channel=electrode_number+i*64+34+2*j;
										}
									}
								}
								fprintf(cnfg_file,"channel : %d\n",channel);
								fprintf(cnfg_file,"position : x = %d, y = %d\n",x_start+x,
									(number_of_pcbs-i-1)*4+1);
							}
							for (j=start4;j<end4;j++)
							{
								fprintf(cnfg_file,"electrode : %d\n",
									electrode_number+i*64+j+49);
								if (old_4by16)
								{
									x=j;
									if (bipolar)
									{
										channel=electrode_number+i*64+29-2*j;
									}
									else
									{
										channel=electrode_number+i*64+63-2*j;
									}
								}
								else
								{
									if (old_4by16_numbering)
									{
										x=j;
										if (bipolar)
										{
											channel=electrode_number+i*64+34+2*(start4+end4-1-j);
										}
										else
										{
											channel=electrode_number+i*64+2+2*(start4+end4-1-j);
										}
									}
									else
									{
										x=number_in_row-1-j;
										if (bipolar)
										{
											channel=electrode_number+i*64+34+2*j;
										}
										else
										{
											channel=electrode_number+i*64+2+2*j;
										}
									}
								}
								fprintf(cnfg_file,"channel : %d\n",channel);
								fprintf(cnfg_file,"position : x = %d, y = %d\n",x_start+x,
									(number_of_pcbs-i-1)*4);
							}
						}
						x_start += number_in_row;
						strip_number++;
					}
					else
					{
						number_of_pcbs= -number_of_pcbs;
					}
					if ((electrode_number/64<ecg_module)&&
						(ecg_module<=(electrode_number/64)+number_of_pcbs)&&bipolar)
					{
						write_ecg=1;
					}
					if ((electrode_number/64<external_module)&&
						(external_module<=(electrode_number/64)+number_of_pcbs)&&bipolar)
					{
						write_external=1;
					}
					electrode_number += number_of_pcbs*64;
				}
			} while (number_of_pcbs!=0);
			printf("Another region (y/n) ?\n");
			scanf("%s%*c",file_name);
		} while (('y'==file_name[0])||('Y'==file_name[0]));
		if (write_ecg)
		{
			fprintf(cnfg_file,"patch : ecg\n");
#if defined (OLD_CODE)
/* pre bipolar adapter ecg */
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
#endif /* defined (OLD_CODE) */
			fprintf(cnfg_file,"auxiliary : ecg_%d\n",(ecg_module-1)*64+2);
			fprintf(cnfg_file,"channel : %d\n",(ecg_module-1)*64+2);
			fprintf(cnfg_file,"auxiliary : ecg_%d\n",(ecg_module-1)*64+33);
			fprintf(cnfg_file,"channel : %d\n",(ecg_module-1)*64+33);
			fprintf(cnfg_file,"auxiliary : ecg_%d\n",(ecg_module-1)*64+34);
			fprintf(cnfg_file,"channel : %d\n",(ecg_module-1)*64+34);
			fprintf(cnfg_file,"auxiliary : I\n");
			fprintf(cnfg_file,"sum : ecg_%d-ecg_%d\n",(ecg_module-1)*64+2,
				(ecg_module-1)*64+33);
			fprintf(cnfg_file,"auxiliary : II\n");
			fprintf(cnfg_file,"sum : ecg_%d-ecg_%d\n",(ecg_module-1)*64+34,
				(ecg_module-1)*64+33);
			fprintf(cnfg_file,"auxiliary : III\n");
			fprintf(cnfg_file,"sum : ecg_%d-ecg_%d\n",(ecg_module-1)*64+34,
				(ecg_module-1)*64+2);
			fprintf(cnfg_file,"auxiliary : aVR\n");
			fprintf(cnfg_file,"sum : ecg_%d-0.5*ecg_%d-0.5*ecg_%d\n",
				(ecg_module-1)*64+33,(ecg_module-1)*64+2,(ecg_module-1)*64+34);
			fprintf(cnfg_file,"auxiliary : aVL\n");
			fprintf(cnfg_file,"sum : ecg_%d-0.5*ecg_%d-0.5*ecg_%d\n",
				(ecg_module-1)*64+2,(ecg_module-1)*64+33,(ecg_module-1)*64+34);
			fprintf(cnfg_file,"auxiliary : aVF\n");
			fprintf(cnfg_file,"sum : ecg_%d-0.5*ecg_%d-0.5*ecg_%d\n",
				(ecg_module-1)*64+34,(ecg_module-1)*64+2,(ecg_module-1)*64+33);
		}
		if (write_external)
		{
			fprintf(cnfg_file,"patch : external\n");
			fprintf(cnfg_file,"auxiliary : A1\n");
			fprintf(cnfg_file,"channel : %d\n",(external_module-1)*64+1);
			fprintf(cnfg_file,"auxiliary : A2\n");
			fprintf(cnfg_file,"channel : %d\n",(external_module-1)*64+2);
			fprintf(cnfg_file,"auxiliary : A3\n");
			fprintf(cnfg_file,"channel : %d\n",(external_module-1)*64+3);
			fprintf(cnfg_file,"auxiliary : A4\n");
			fprintf(cnfg_file,"channel : %d\n",(external_module-1)*64+4);
			fprintf(cnfg_file,"auxiliary : A5\n");
			fprintf(cnfg_file,"channel : %d\n",(external_module-1)*64+5);
			fprintf(cnfg_file,"auxiliary : A6\n");
			fprintf(cnfg_file,"channel : %d\n",(external_module-1)*64+6);
			fprintf(cnfg_file,"auxiliary : A7\n");
			fprintf(cnfg_file,"channel : %d\n",(external_module-1)*64+7);
			fprintf(cnfg_file,"auxiliary : A8\n");
			fprintf(cnfg_file,"channel : %d\n",(external_module-1)*64+8);
		}
		fclose(cnfg_file);
	}
	else
	{
		printf("Could not open %s\n",file_name);
	}

	return (1);
} /* main */
