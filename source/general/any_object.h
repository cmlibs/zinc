/*******************************************************************************
FILE : any_object.h

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
==============================================================================*/
#if !defined (ANY_OBJECT_H)
#define ANY_OBJECT_H

#include "general/list.h"
#include "general/object.h"

/*
Global types
------------
*/

struct Any_object;
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Any_object);

/*
Global functions
----------------
*/

PROTOTYPE_OBJECT_FUNCTIONS(Any_object);
PROTOTYPE_LIST_FUNCTIONS(Any_object);

int DESTROY(Any_object)(struct Any_object **any_object_address);
/*******************************************************************************
LAST MODIFIED : 22 August 2000

DESCRIPTION :
Destroys the Any_object.
==============================================================================*/

int ensure_Any_object_is_in_list(struct Any_object *any_object,
	void *any_object_list_void);
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Iterator function for adding <any_object> to <any_object_list> if not currently
in it.
==============================================================================*/

int ensure_Any_object_is_not_in_list(struct Any_object *any_object,
	void *any_object_list_void);
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Iterator function for removing <any_object> from <any_object_list> if currently
in it.
==============================================================================*/

#endif /* !defined (ANY_OBJECT_H) */
