/*******************************************************************************
FILE : any_object_private.h

LAST MODIFIED : 2 October 2002

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
typedef int (*Any_object_cleanup_function)(void *subobject);
/*******************************************************************************
LAST MODIFIED : 2 October 2002

DESCRIPTION :
Optional clean-up function for the subobject in an any_object. Called only by
destroy function; void pointer to subobject is returned, NOT address of pointer.
==============================================================================*/

struct Any_object
/*******************************************************************************
LAST MODIFIED : 2 October 2002

DESCRIPTION :
==============================================================================*/
{
	char *type_string;
	void *subobject;
	Any_object_cleanup_function any_object_cleanup_function;
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
LAST MODIFIED : 2 October 2002

DESCRIPTION :
Creates an Any_object which uses the static <type_string> to identify the type
of the <subobject>.
This function is private to objects using Any_object interface.
==============================================================================*/

int Any_object_set_cleanup_function(struct Any_object *any_object,
	Any_object_cleanup_function any_object_cleanup_function);
/*******************************************************************************
LAST MODIFIED : 2 October 2002

DESCRIPTION :
Adds function to <any_object> which will be called when it is destroyed. The
single argument to this function is a void pointer to the subobject in the
Any_object, NOT the address of a pointer to it.
This function is private to objects using Any_object interface.
==============================================================================*/

#endif /* !defined (ANY_OBJECT_PRIVATE_H) */
