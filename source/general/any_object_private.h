/*******************************************************************************
FILE : any_object_private.h

LAST MODIFIED : 23 August 2000

DESCRIPTION :
Definition of Any_object structure for storing any object which can be uniquely
identified by a pointer with strong typing. LIST(Any_object) is declared.

Internally, type is represented by a static type_string pointer, constructed
from the type name itself. A void pointer is used for the object reference.

Macro prototypes in any_object_prototype.h and definitions in
any_object_definition.h allow Any_objects to be established for a particular
structure and through that interface remain strongly typed. Interfaces allow
a LIST(Any_object) to be treated as if it is a list of just the object_type for
which the macros are declared.

Private declatation of Any_object structure for inclusion in any_object.c and
any sub-classes to be defined.
==============================================================================*/
#if !defined (ANY_OBJECT_PRIVATE_H)
#define ANY_OBJECT_PRIVATE_H

#include "general/any_object.h"
#include "general/object.h"

/*
Private global types
--------------------
*/

struct Any_object
/*******************************************************************************
LAST MODIFIED : 22 August 2000

DESCRIPTION :
==============================================================================*/
{
	char *type_string;
	void *subobject;
	int access_count;
}; /* struct Any_object */

/*
Private global functions
------------------------
*/

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Any_object, \
	subobject,void *);

struct Any_object *CREATE(Any_object)(char *type_string,void *subobject);
/*******************************************************************************
LAST MODIFIED : 22 August 2000

DESCRIPTION :
Creates an Any_object which uses the static <type_string> to identify the type
of the <subobject>.
==============================================================================*/

#endif /* !defined (ANY_OBJECT_PRIVATE_H) */
