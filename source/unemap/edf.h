/*******************************************************************************
FILE : edf.h

LAST MODIFIED : 11 December 2001

DESCRIPTION :
Functions for reading EDF (European Data Format) data files, as output by the
Biosemi rig.
==============================================================================*/
#if !defined (EDF_H)
#define EDF_H
#include <stddef.h>
#include <string.h>
#include <math.h>
/*ieeefp.h doesn't exist for Linux. Needed for finite() for Irix*/
/*finite() in math.h in Linux */
#if defined (NOT_ANSI)
#include <ieeefp.h>
#endif /* defined (NOT_ANSI) */
#if defined (WIN32)
#include <float.h>
#endif /* defined (WIN32) */
#include "general/debug.h"
#include "general/geometry.h"
#include "general/myio.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "user_interface/confirmation.h"
#include "user_interface/user_interface.h"
#include "unemap/unemap_package.h"
/*
Global functions
----------------
*/
int read_bdf_or_edf_file(char *file_name,struct Rig **rig_pointer,
	struct User_interface *user_interface
#if defined (UNEMAP_USE_3D)
			,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_NODES)*/
				,int bdf);
/*******************************************************************************
LAST MODIFIED : 11 December 2001

DESCRIPTION :
Reads the signal data from a BDF or EDF file specified by <file_name> and 
creates a rig with it in <rig_pointer>.
Optionally reads in a config file, or create a defalut one.
<bdf> is a flag, 0=edf, 1=bdf.
==============================================================================*/

#endif /* #if !defined (EDF_H) */
