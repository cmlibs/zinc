/*******************************************************************************
FILE : unique_channels.c

LAST MODIFIED : 30 January 2000

DESCRIPTION :
Makes the channel numbers unique.  Does not use X-windows.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "unemap/rig.h"
#include "user_interface/user_interface.h"

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 30 January 2000

DESCRIPTION :
==============================================================================*/
{
	char file_name[120];
	FILE *signal_file;
	int j,return_code;
	struct Device **signal_device;
	struct Rig *signal_rig;

	return_code=0;
	/* read the signal file */
	printf("Signal file name ? ");
	scanf("%s",file_name);
	if ((signal_file=fopen(file_name,"rb"))&&read_signal_file(signal_file,
		&signal_rig))
	{
		fclose(signal_file);
		signal_device=signal_rig->devices;
		j=signal_rig->number_of_devices;
		signal_device += j;
		while (j>0)
		{
			signal_device--;
			if (signal_device&&(*signal_device)&&((*signal_device)->channel))
			{
				(*signal_device)->channel->number=j;
			}
			j--;
		}
		/* write out the signal file with the new rig */
		printf("New signal file name ? ");
		scanf("%s",file_name);
		if ((signal_file=fopen(file_name,"wb"))&&write_signal_file(signal_file,
			signal_rig))
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
		printf("ERROR.  Could not read signal file\n");
	}

	return (return_code);
} /* main */
