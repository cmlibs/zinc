/*******************************************************************************
FILE : spaces_to_tabs.c

LAST MODIFIED : 1 March 2002

DESCRIPTION :
Program to replace spaces/tabs at the beginning of lines by tabs and remove
spaces/tabs from the end of lines.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{
	char c,in_file_name[51],line[131],*line_end,*line_start,out_file_name[51];
	FILE *in_file,*out_file;
	int scanf_result,space_count,spaces_per_tab;

	/* get the number of spaces for each tab */
	do
	{
		printf("Number of spaces for each tab ? ");
		scanf_result=scanf("%d",&spaces_per_tab);
	}
	while ((scanf_result!=1)||(spaces_per_tab<=0));
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
	/* move through the input file one line at a time, converting spaces to tabs
		at the beginning of the line, removing spaces/tabs from the end of the line
		and writing it to the output file */
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
			/* remove spaces/tabs from the end of the line */
			line_end=line+strlen(line);
			while ((line_end-- !=line)&&((*line_end==' ')||(*line_end=='\t')));
			line_end[1]='\0';
			/* convert spaces to tabs at the beginning of the line */
			line_start=line;
			space_count=0;
			while (((c= *line_start)==' ')||(c=='\t'))
			{
				if (c=='\t')
				{
					space_count=0;
					fprintf(out_file,"\t");
				}
				else
				{
					space_count++;
					if (space_count==spaces_per_tab)
					{
						space_count=0;
						fprintf(out_file,"\t");
					}
				}
				line_start++;
			}
			/* output the converted line */
			fprintf(out_file,"%s\n",line_start);
		}
	}
	fclose(in_file);
	fclose(out_file);

	return (1);
} /* main */
