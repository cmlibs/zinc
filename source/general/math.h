/*******************************************************************************
FILE : math.h

LAST MODIFIED : 26 June 2002

DESCRIPTION :
Defines the finite function for UNIX and WIN32_SYSTEM
==============================================================================*/
#if !defined (MATH_H)
#define MATH_H

#if defined (SGI)
/* For finite so that we can check for Nans which some compilers seem
	to accept as valid in an fscanf */
#include <ieeefp.h>
#endif /* defined (SGI) */
#if defined (WIN32_SYSTEM)
#define finite _finite
#endif /* defined (WIN32_SYSTEM) */

#endif /* !defined (MATH_H) */
