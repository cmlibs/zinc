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

char *resolve_example_path(char *example_path, char *directory_name,
  char **comfile_name);
/*******************************************************************************
LAST MODIFIED : 17 April 2000

DESCRIPTION :
Uses the executable $example_path/common/resolve_example_path to demangle
a short example name into a full path.  The returned string is ALLOCATED.
<*comfile_name> is allocated and returned as well if the resolve function
returns a string for it.  This too must be DEALLOCATED by the calling function.
==============================================================================*/

#endif /* !defined (EXAMPLE_PATH_H) */
