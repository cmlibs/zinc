/*******************************************************************************
FILE : database.h

LAST MODIFIED : 21 April 1995

DESCRIPTION :
The structures for the objects in the hierarchial database.
???DB.  It is not clear what needs to be done here.  I'll start with FE instead.
==============================================================================*/
#if !defined (DATABASE_H)
#define DATABASE_H

/*
Global types
------------
*/
struct Database_object
/*******************************************************************************
LAST MODIFIED : 21 April 1995

DESCRIPTION :
An object in the database hierarchy.
???DB.  select/highlight is the basic function ?
==============================================================================*/
{
	char *name;
	struct Database_object_list_item *parents;
	int access_count;
}; /* struct Database_object */

/*
Global functions
----------------
*/
#endif
