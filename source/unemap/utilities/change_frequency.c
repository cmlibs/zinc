/*******************************************************************************
FILE : change_frequency.c

LAST MODIFIED : 27 November 2001

DESCRIPTION :
Allow the user to change the frequency for a signal file.  Does not use
X-windows.
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
LAST MODIFIED : 27 November 2001

DESCRIPTION :
==============================================================================*/
{
	char file_name[120];
	FILE *signal_file;
	float new_frequency;
	int return_code;
	struct Rig *signal_rig;

	/* zero is a successful return */
	return_code = 0;
	signal_rig=(struct Rig *)NULL;
	/* read the signal file */
	printf("Signal file name ? ");
	scanf("%s",file_name);
	if ((signal_file=fopen(file_name,"rb"))&&read_signal_file(signal_file,
		&signal_rig))
	{
		fclose(signal_file);
		/* get the new frequency */
		printf("Current frequency %g\n",(*(signal_rig->devices))->signal->buffer->
			frequency);
		printf("New frequency ? ");
		scanf("%f",&new_frequency);
	/*???debug */
	printf("new_frequency=%g\n",new_frequency);
		/* set the new frequency */
		(*(signal_rig->devices))->signal->buffer->frequency=new_frequency;
		/* write out the signal file with the new rig */
		printf("New signal file name ? ");
		scanf("%s",file_name);
		if ((signal_file=fopen(file_name,"wb"))&&write_signal_file(signal_file,
			signal_rig))
		{
			fclose(signal_file);
			printf("New signal file created\n");
		}
		else
		{
			printf("ERROR.  Writing new signal file\n");
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
