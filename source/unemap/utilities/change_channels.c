/*******************************************************************************
FILE : change_channels.h

LAST MODIFIED : 27 November 2001

DESCRIPTION :
Reads in a configuration file and a table giving the channel for each electrode.
Writes out the reordered configuration file.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unemap/rig.h"
#include "user_interface/message.h"

/*
Main program
------------
*/

int main()
{
	char temp_string[100];
	FILE *configuration_file,*translation_table_file;
	int channel_number, i, return_code;
	struct Device **device;
	struct Rig *rig;

	/* zero is a successful return */
	return_code = 0;
	printf("Configuration file ?\n");
	scanf("%s",temp_string);
	if (read_configuration_file(temp_string,(void *)&rig))
	{
		printf("Translation table file (electrode channel) ?\n");
		scanf("%s",temp_string);
		if (translation_table_file=fopen(temp_string,"r"))
		{
			while (2==fscanf(translation_table_file," %s %d ",temp_string,
				&channel_number))
			{
				i=rig->number_of_devices;
				device=rig->devices;
				while ((i>0)&&strcmp(temp_string,(*device)->description->name))
				{
					device++;
					i--;
				}
				if (i>0)
				{
					(*device)->channel->number=channel_number;
				}
			}
			if (feof(translation_table_file))
			{
				printf("New configuration file ?\n");
				scanf("%s",temp_string);
				if (configuration_file=fopen(temp_string,"w"))
				{
					write_configuration(rig,configuration_file,TEXT);
					fclose(configuration_file);
				}
				else
				{
					display_message(ERROR_MESSAGE,"Could not open %s",temp_string);
					return_code = 3;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Invalid translation table\n");
				return_code = 2;
			}
			fclose(translation_table_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not open %s",temp_string);
			return_code = 1;
		}
	}

	return (return_code);
}
