/*******************************************************************************
FILE : tabs_to_spaces.c

LAST MODIFIED : 9 March 1993

DESCRIPTION :
Program to replace tabs at the beginning of lines by spaces.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

main()
{
	char in_file_name[51],line[131],*line_start,out_file_name[51],*tab;
	FILE *in_file,*out_file;
	int i,scanf_result,spaces_per_tab;

	/* get the number of spaces for each tab */
	do
	{
		printf("Number of spaces for each tab ? ");
		scanf_result=scanf("%d",&spaces_per_tab);
	}
	while ((scanf_result!=1)||(spaces_per_tab<=0));
	if (tab=(char *)malloc((spaces_per_tab+1)*sizeof(char)))
	{
		for (i=0;i<spaces_per_tab;i++)
		{
			tab[i]=' ';
		}
		tab[spaces_per_tab]='\0';
		/* get the input file name */
		do
		{
			printf("Input file ? ");
			scanf("%50s",in_file_name);
		}
		while (!(in_file=fopen(in_file_name,"r")));
		/* get the output file name */
		do
		{
			printf("Output file ? ");
			scanf("%50s",out_file_name);
		}
		while (!(out_file=fopen(out_file_name,"w")));
		/* move through the input file one line at a time, converting tabs to spaces
			at the beginning of the line and writing it to the output file */
		while (EOF!=(scanf_result=fscanf(in_file,"%[^\n]%*c",line)))
		{
			/*??? %[^\n] doesn't work on the RS6000 when there are no characters in the
				line.  This is a work around */
			if (0==scanf_result)
			{
				fscanf(in_file,"%*c");
				fprintf(out_file,"\n");
			}
			else
			{
				/* convert tabs to spaces at the beginning of the line */
				line_start=line;
				while ((*line_start=='\t')||(*line_start==' '))
				{
					if (*line_start=='\t')
					{
						fprintf(out_file,tab);
					}
					else
					{
						fprintf(out_file," ");
					}
					line_start++;
				}
				/* output the converted line */
				fprintf(out_file,"%s\n",line_start);
			}
		}
		fclose(in_file);
		fclose(out_file);
	}
	else
	{
		printf("Could not allocate memeory for tab\n");
	}
} /* main */
