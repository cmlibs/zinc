/*******************************************************************************
FILE : plt2cnfg.c

LAST MODIFIED : 16 September 1998

DESCRIPTION :
Converts a plt file (stdin) to a cnfg file (stdout)
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
	float electrode_x,electrode_y;
	int electrode_number;

	printf("patch : patch\n");
	printf("region : all\n");
	while (3==scanf(" %d %f %f",&electrode_number,&electrode_x,&electrode_y))
	{
		printf("electrode : %d\n",electrode_number);
		printf("channel : %d\n",electrode_number);
		printf("position : x = %g , y = %g\n",electrode_x,-electrode_y);
	}

	return (1);
}
