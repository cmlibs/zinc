/*******************************************************************************
FILE : tabs_to_spaces.c

LAST MODIFIED : 1 March 2002

DESCRIPTION :
Program to replace tabs at the beginning of lines by spaces.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
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

	return (1);
} /* main */
