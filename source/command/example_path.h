/*******************************************************************************
FILE : example_path.h

LAST MODIFIED : 17 April 2000

DESCRIPTION :
==============================================================================*/
#if !defined (EXAMPLE_PATH_H)
#define EXAMPLE_PATH_H


/*
Global functions
----------------
*/

char *resolve_example_path(char *example_path, char *directory_name);
/*******************************************************************************
LAST MODIFIED : 17 April 2000

DESCRIPTION :
Uses the executable $example_path/common/resolve_example_path to demangle
a short example name into a full path.  The returned string is ALLOCATED.
==============================================================================*/

#endif /* !defined (EXAMPLE_PATH_H) */
