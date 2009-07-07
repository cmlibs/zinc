/* strftime example */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string.h>

int main ( int argc, char *argv[] )
{
	time_t rawtime;
	struct tm * timeinfo;
	char buffer [256];

	if ( argc != 5 )
		return 1;

	FILE * pFile;
	pFile = fopen( argv[1], "w" );
	if (pFile!=NULL)
	{
	/*
		#define VERSION "CMISS(cmgui) version @CMGUI_MAJOR_VERSION@.@CMGUI_MINOR_VERSION@.@CMGUI_PATCH_VERSION@  [DATE]\nCopyright 1996-2009, Auckland UniServices Ltd."
	*/
		fputs ("\n#if !defined GENERATED_VERSION_H\n#define GENERATED_VERSION_H\n\n#define VERSION ",pFile);
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );

		fprintf( pFile, "\"CMISS(cmgui) version %d.%d.%d ", atoi(argv[2]),atoi(argv[3]),atoi(argv[4]) );
		strftime (buffer,256,"%c %Z\\nCopyright 1996-%Y, Auckland UniServices Ltd.\"\n",timeinfo);
		fputs (buffer,pFile);

		fputs( "\n#endif\n\n", pFile );
		fclose (pFile);
	}
						  
	return 0;
}
