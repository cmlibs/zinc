/*******************************************************************************
FILE : change_calibration.c

LAST MODIFIED : 2 April 2003

DESCRIPTION :
Allow the user to change the calibration for a signal file.  Do not use
X-windows.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <Mrm/MrmPublic.h>
#include "general/debug.h"
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
LAST MODIFIED : 7 December 2001

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
LAST MODIFIED : 2 April 2003

DESCRIPTION :
==============================================================================*/
{
	char file_name[120];
	FILE *output_file,*signal_file;
	int number_of_devices,return_code;
	struct Channel *channel;
	struct Device **device;
	struct Rig *rig;

	USE_PARAMETER(argc);
	USE_PARAMETER(argv);
	/* zero is a successful return */
	return_code = 0;
	rig=(struct Rig *)NULL;
	/* read the signal file */
	printf("Signal file name ? ");
	scanf("%s",file_name);
	if ((signal_file=fopen(file_name,"rb"))&&read_signal_file(signal_file,
		&rig))
	{
		fclose(signal_file);
		/* read the calibration file */
		printf("Calibration file name ? ");
		scanf("%s",file_name);
		if (output_file=fopen(file_name,"wt"))
		{
			/* write out the device signal indicies and channel
				characteristics */
			device=rig->devices;
			number_of_devices=rig->number_of_devices;
			return_code=1;
			while (return_code&&(number_of_devices>0))
			{
				/* don't write anything for an auxiliary device that is a
					linear combination */
				if (channel=(*device)->channel)
				{
					fprintf(output_file,"%d %g %g \n",channel->number,channel->offset,
						channel->gain);
				}
				device++;
				number_of_devices--;
			}
		}
		else
		{
			printf("ERROR.  Could not open calibration file\n");
			return_code = 1;
		}
	}
	else
	{
		printf("ERROR.  Could not read signal file\n");
		return_code = 1;
	}

	return (return_code);
} /* main */
