/*******************************************************************************
FILE : myio.h

LAST MODIFIED : 12 June 2003

DESCRIPTION :
Some additions/modifications to stdio.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (MYIO_H)
#define MYIO_H

#include <stdio.h>

#if defined (UNIX) && defined (GENERIC_PC)
#if defined (CYGWIN)
#include <sys/param.h>
#else /* defined (CYGWIN) */
#include <endian.h>
#endif /* defined (CYGWIN) */
#endif /* defined (UNIX) && defined (GENERIC_PC) */
#if defined (SGI)
#include <sys/endian.h>
#endif /* defined (SGI) */
#if defined (AIX)
#include <sys/machine.h>
#endif /* defined (AIX) */
#if defined (WIN32_SYSTEM)
#define BYTE_ORDER 1234
#endif /* defined (WIN32_SYSTEM) */

#if ! defined BYTE_ORDER
Warning BYTE_ORDER not defined
#endif /* ! defined BYTE_ORDER */

/*
Global macros
-------------
*/
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
#define BINARY_FILE_READ( char_ptr,sizeof_type,count,binary_file ) \
fread_little_to_big_endian(char_ptr,sizeof_type,count,binary_file)

#define BINARY_FILE_WRITE( char_ptr,sizeof_type,count,binary_file ) \
fwrite_big_to_little_endian(char_ptr,sizeof_type,count,binary_file)
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
#define BINARY_FILE_READ( char_ptr,sizeof_type,count,binary_file ) \
fread(char_ptr,sizeof_type,count,binary_file)

#define BINARY_FILE_WRITE( char_ptr,sizeof_type,count,binary_file ) \
fwrite(char_ptr,sizeof_type,count,binary_file)
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */

/*
Global functions
----------------
*/
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
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
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */

int get_line_number(FILE *stream);
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
Function for calculating the current line in a file.
==============================================================================*/
#endif /* !defined (MYIO_H) */
