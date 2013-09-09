/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main ( int argc, char *argv[] )
{
	time_t rawtime;
	struct tm * timeinfo;
	char buffer [256];

	if ( argc == 1)
	{
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		strftime (buffer,256,"%d/%m/%y %X %Z",timeinfo);
		printf( "%s\n", buffer );
		return 0;
	}

	if ( argc != 10 )
		return 1;

	FILE * pFile;
	pFile = fopen( argv[1], "w" );
	if (pFile!=NULL)
	{
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		strftime (buffer,256,"%d/%m/%y %X %Z",timeinfo);
		//
		// The version string should be something like this:
		// "CMZN(cmgui) version @CMGUI_MAJOR_VERSION@.@CMGUI_MINOR_VERSION@.@CMGUI_PATCH_VERSION@  [DATE]\nCopyright 1996-[YEAR], Auckland UniServices Ltd."
		//
		fprintf( pFile, "\n#if !defined GENERATED_VERSION_H\n#define GENERATED_VERSION_H\n\n" );
		fprintf( pFile, "#define CMZN_NAME_STRING \"CMZN(cmgui)\"\n" );
		fprintf( pFile, "#define CMZN_VERSION_STRING \"%d.%d.%d\"\n", atoi(argv[2]),atoi(argv[3]),atoi(argv[4]) );
		fprintf( pFile, "#define CMZN_DATE_STRING \"%s\"\n", buffer );
		fprintf( pFile, "#define CMZN_COPYRIGHT_STRING \"Copyright 1996-2009, Auckland UniServices Ltd.\"\n" );
		fprintf( pFile, "#define CMZN_SVN_REVISION_STRING \"%d\"\n", atoi(argv[5]) );
		fprintf( pFile, "#define CMZN_BUILD_STRING \"%s %s %s %s\"\n", argv[6], argv[7], argv[8], argv[9] );
		fprintf( pFile, "\n#endif\n\n" );
		fclose (pFile);
	}

	return 0;
}

