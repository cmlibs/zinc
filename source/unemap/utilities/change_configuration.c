/*******************************************************************************
FILE : change_configuration.c

LAST MODIFIED : 4 August 1999

DESCRIPTION :
Allow the user to change the configuration for a signal file.  Do not use
X-windows.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "unemap/rig.h"
#include "user_interface/user_interface.h"

/*
Global functions
----------------
*/
#if defined (OLD_CODE)
/* is in message.c, but not using X-windows */
int print_message(int number_of_strings, ...)
/*******************************************************************************
LAST MODIFIED : 26 January 1996

DESCRIPTION :
Prints the <number_of_strings> to a message box.
==============================================================================*/
{
	int arg_count,return_code;
	va_list ap;

	return_code=1;
	va_start(ap,number_of_strings);
	for (arg_count=0;arg_count<number_of_strings;arg_count++)
	{
		printf("%s",va_arg(ap,char *));
	}
	printf("\n");
	va_end(ap);

	return(return_code);
} /* print_message */
#endif /* defined (OLD_CODE) */

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 4 August 1999

DESCRIPTION :
==============================================================================*/
{
	char file_name[120];
	FILE *signal_file;
	int i,j,return_code;
	struct Device **configuration_device,**signal_device;
	struct Rig *configuration_rig,*signal_rig;

	return_code=0;
#if defined (OLD_CODE)
	assign_empty_string(&(user_settings.calibration_directory));
#endif /* defined (OLD_CODE) */
	configuration_rig=(struct Rig *)NULL;
	signal_rig=(struct Rig *)NULL;
	/* read the configuration file */
	printf("Configuration file name ? ");
	scanf("%s",file_name);
	if (read_configuration_file(file_name,(void *)(&configuration_rig)))
	{
		/* read the signal file */
		printf("Signal file name ? ");
		scanf("%s",file_name);
		if ((signal_file=fopen(file_name,"rb"))&&read_signal_file(signal_file,
			&signal_rig))
		{
			fclose(signal_file);
			/* check that the configurations are consistent */
			configuration_device=configuration_rig->devices;
			i=configuration_rig->number_of_devices;
			return_code=1;
			while (i>0)
			{
				/* linear combinations will be checked through the electrode devices
					they point to */
				if ((*configuration_device)->channel)
				{
					signal_device=signal_rig->devices;
					j=signal_rig->number_of_devices;
					while ((j>0)&&!(((*signal_device)->channel)&&
						((*signal_device)->channel->number==
						(*configuration_device)->channel->number)))
					{
						j--;
						signal_device++;
					}
					if (j>0)
					{
						(*configuration_device)->channel=(*signal_device)->channel;
						(*configuration_device)->signal=(*signal_device)->signal;
					}
					else
					{
						return_code=0;
						printf(
					"channel %d for configuration device %s is not in the signal file\n",
							(*configuration_device)->channel->number,
							(*configuration_device)->description->name);
					}
				}
				i--;
				configuration_device++;
			}
			if (return_code)
			{
				/* write out the signal file with the new rig */
				printf("New signal file name ? ");
				scanf("%s",file_name);
				if ((signal_file=fopen(file_name,"wb"))&&write_signal_file(signal_file,
					configuration_rig))
				{
					printf("New signal file created\n");
				}
				else
				{
					printf("ERROR.  Writing new signal file\n");
				}
			}
			else
			{
				printf(
					"ERROR.  Configuration file is not consistent with signal file\n");
			}
		}
		else
		{
			printf("ERROR.  Could not read configuration file\n");
		}
	}
	else
	{
		printf("ERROR.  Could not read configuration file\n");
	}

	return (return_code);
} /* main */
