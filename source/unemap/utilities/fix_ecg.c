/*******************************************************************************
FILE : fix_ecg.c

LAST MODIFIED : 7 December 2001

DESCRIPTION :
Fixes signal files which were made by write_signal_file prior to fixing the
bug where signals for devices after auxiliary sums had the wrong indices into
the signal buffer.  Came to prominence in LA when acquiring with an ecg.

Does not use X.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "general/debug.h"
#include "unemap/rig.h"

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 7 December 2001

DESCRIPTION :
==============================================================================*/
{
	char file_name[120];
	FILE *signal_file;
	int i,j,return_code;
	struct Device **device;
	struct Rig *rig;
	struct Signal *signal;

	USE_PARAMETER(argc);
	USE_PARAMETER(argv);
	return_code=0;
	rig=(struct Rig *)NULL;
	/* read the signal file */
	printf("Signal file name ? ");
	scanf("%s",file_name);
	if ((signal_file=fopen(file_name,"rb"))&&read_signal_file(signal_file,&rig))
	{
		fclose(signal_file);
		/* fix the indices */
		device=rig->devices;
		i=rig->number_of_devices;
		j=0;
		while (i>0)
		{
			if ((*device)->channel)
			{
				signal=(*device)->signal;
				while (signal)
				{
					signal->index=j;
					j++;
					signal=signal->next;
				}
			}
			i--;
			device++;
		}
		/* write out the signal file */
		printf("New signal file name ? ");
		scanf("%s",file_name);
		if ((signal_file=fopen(file_name,"wb"))&&write_signal_file(signal_file,rig))
		{
			return_code=1;
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
