/*******************************************************************************
FILE : myio.h

LAST MODIFIED : 21 April 1997

DESCRIPTION :
Some additions/modifications to stdio.
==============================================================================*/
#if !defined (MYIO_H)
#define MYIO_H

#include <stdio.h>
/*???DB.  Contains definition of __BYTE_ORDER for Linux */
#include <ctype.h>
#if defined (WINDOWS)
#define __BYTE_ORDER 1234
#endif /* defined (WINDOWS) */

/*
Global functions
----------------
*/
#if defined (__BYTE_ORDER) && (1234==__BYTE_ORDER)
int fread_little_to_big_endian(char *char_ptr,unsigned sizeof_type,int count,
	FILE *binary_file);
/*******************************************************************************
LAST MODIFIED : 4 September 1995

DESCRIPTION :
Does an fread and then converts from little to big endian.
==============================================================================*/

int fwrite_big_to_little_endian(char *char_ptr,unsigned sizeof_type,int count,
	FILE *binary_file);
/*******************************************************************************
LAST MODIFIED : 4 September 1995

DESCRIPTION :
fwrites the little endian form of <char_ptr>.
==============================================================================*/
#endif

/*
Global macros
-------------
*/
#if defined (__BYTE_ORDER) && (1234==__BYTE_ORDER)
#define BINARY_FILE_READ( char_ptr,sizeof_type,count,binary_file ) \
fread_little_to_big_endian(char_ptr,sizeof_type,count,binary_file)

#define BINARY_FILE_WRITE( char_ptr,sizeof_type,count,binary_file ) \
fwrite_big_to_little_endian(char_ptr,sizeof_type,count,binary_file)
#else
#define BINARY_FILE_READ( char_ptr,sizeof_type,count,binary_file ) \
fread(char_ptr,sizeof_type,count,binary_file)

#define BINARY_FILE_WRITE( char_ptr,sizeof_type,count,binary_file ) \
fwrite(char_ptr,sizeof_type,count,binary_file)
#endif
#endif
