/*******************************************************************************
FILE : any_object_definition.h

LAST MODIFIED : 23 August 2000

DESCRIPTION :
Macro definitions for subclasses of struct Any_object.
==============================================================================*/
#if !defined (ANY_OBJECT_DEFINITION_H)
#define ANY_OBJECT_DEFINITION_H

#include "general/any_object.h"
#include "general/any_object_private.h"
#include "general/any_object_prototype.h"
#include "general/object.h"

#define ANY_OBJECT_TYPE_STRING(object_type) \
any_object_type_string_ ## object_type

/*
Module variables
----------------
*/

#define DEFINE_ANY_OBJECT_TYPE_STRING(object_type) \
static char ANY_OBJECT_TYPE_STRING(object_type)[] = #object_type;

/*
Global functions
----------------
*/

#define DEFINE_CREATE_ANY_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_CREATE_ANY_OBJECT_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 22 August 2000 \
 \
DESCRIPTION : \
Creates an Any_object for <object>.
============================================================================*/ \
{ \
	struct Any_object *any_object; \
\
	ENTER(CREATE(ANY_OBJECT(object_type))); \
	if (!(any_object= \
		CREATE(Any_object)(ANY_OBJECT_TYPE_STRING(object_type),(void *)object))) \
	{ \
		display_message(ERROR_MESSAGE, \
			"CREATE(ANY_OBJECT(" #object_type ").  Failed"); \
	} \
	LEAVE; \
\
	return (any_object); \
} /* CREATE(ANY_OBJECT(object_type) */

#define DEFINE_ADD_OBJECT_TO_ANY_OBJECT_LIST_FUNCTION( object_type ) \
PROTOTYPE_ADD_OBJECT_TO_ANY_OBJECT_LIST_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 22 August 2000 \
\
DESCRIPTION : \
Add <object> to the <any_object_list>. \
============================================================================*/ \
{ \
	int return_code; \
	struct Any_object *any_object; \
\
	ENTER(ADD_OBJECT_TO_LIST(ANY_OBJECT(object_type))); \
	if (object&&any_object_list) \
	{ \
		if (any_object=CREATE(ANY_OBJECT(object_type))(object)) \
		{ \
			if (ADD_OBJECT_TO_LIST(Any_object)(any_object,any_object_list)) \
			{ \
				return_code=1; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"ADD_OBJECT_TO_LIST(ANY_OBJECT(" #object_type \
					")).  Object already in list"); \
				DESTROY(Any_object)(&any_object); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"ADD_OBJECT_TO_LIST(ANY_OBJECT(" #object_type \
				")).  Could not create Any_object"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"ADD_OBJECT_TO_LIST(ANY_OBJECT(" \
			#object_type ")).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* ADD_OBJECT_TO_LIST(ANY_OBJECT(object_type)) */

#define DEFINE_REMOVE_OBJECT_FROM_ANY_OBJECT_LIST_FUNCTION( object_type ) \
PROTOTYPE_REMOVE_OBJECT_FROM_ANY_OBJECT_LIST_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 23 August 2000 \
\
DESCRIPTION : \
Removes <object> from the <any_object_list>. \
============================================================================*/ \
{ \
	int return_code; \
	struct Any_object *any_object; \
\
	ENTER(REMOVE_OBJECT_FROM_LIST(ANY_OBJECT(object_type))); \
	if (object&&any_object_list) \
	{ \
		if (any_object= \
			IS_OBJECT_IN_LIST(ANY_OBJECT(object_type))(object,any_object_list)) \
		{ \
			if (REMOVE_OBJECT_FROM_LIST(Any_object)(any_object,any_object_list)) \
			{ \
				return_code=1; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"REMOVE_OBJECT_FROM_LIST(ANY_OBJECT(" #object_type \
					")).  Could not remove from list"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"REMOVE_OBJECT_FROM_LIST(ANY_OBJECT(" #object_type \
				")).  Object is not in list"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"REMOVE_OBJECT_FROM_LIST(ANY_OBJECT(" \
			#object_type ")).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_OBJECT_FROM_LIST(ANY_OBJECT(object_type)) */

#define DEFINE_IS_OBJECT_IN_ANY_OBJECT_LIST_FUNCTION( object_type ) \
PROTOTYPE_IS_OBJECT_IN_ANY_OBJECT_LIST_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 23 August 2000 \
\
DESCRIPTION : \
If <object> is in <any_object_list>, return its Any_object structure.
============================================================================*/ \
{ \
	struct Any_object *any_object; \
\
	ENTER(IS_OBJECT_IN_LIST(ANY_OBJECT(object_type))); \
	if (object&&any_object_list) \
	{ \
		if (any_object=FIND_BY_IDENTIFIER_IN_LIST(Any_object,subobject)( \
			(void *)object,any_object_list)) \
		{ \
			if (ANY_OBJECT_TYPE_STRING(object_type) != any_object->type_string) \
			{ \
				display_message(ERROR_MESSAGE, \
					"IS_OBJECT_IN_LIST(ANY_OBJECT(" #object_type \
					")).  Any_object of wrong type has same object pointer!"); \
				any_object=(struct Any_object *)NULL; \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"IS_OBJECT_IN_LIST(ANY_OBJECT(" \
			#object_type ")).  Invalid argument(s)"); \
		any_object=(struct Any_object *)NULL; \
	} \
	LEAVE; \
\
	return (any_object); \
} /* IS_OBJECT_IN_LIST(ANY_OBJECT(object_type)) */

#if defined (FULL_NAMES)
#define ANY_OBJECT_ITERATOR_DATA( object_type ) \
	any_object_iterator_data_ ## object_type
#else
#define ANY_OBJECT_ITERATOR_DATA( object_type ) \
	aoid ## object_type
#endif

#define DECLARE_ANY_OBJECT_ITERATOR_DATA_TYPE( object_type ) \
struct ANY_OBJECT_ITERATOR_DATA(object_type) \
/***************************************************************************** \
LAST MODIFIED : 22 August 2000 \
\
DESCRIPTION : \
User_data for ANY_OBJECT_ITERATOR(object_type).
============================================================================*/ \
{ \
	ANY_OBJECT_ITERATOR_FUNCTION(object_type) *iterator_function; \
	void *user_data; \
}

#if defined (FULL_NAMES)
#define ANY_OBJECT_ITERATOR( object_type ) \
	any_object_iterator_ ## object_type
#else
#define ANY_OBJECT_ITERATOR_FUNCTION( object_type ) \
	aoi ## object_type
#endif

#define DEFINE_ANY_OBJECT_ITERATOR_FUNCTION( object_type ) \
static int ANY_OBJECT_ITERATOR(object_type)( \
	struct Any_object *any_object,void *iterator_data_void) \
/***************************************************************************** \
LAST MODIFIED : 22 August 2000 \
\
DESCRIPTION : \
If the <any_object> is of object_type, call the iterator with the object and \
user_data. \
============================================================================*/ \
{ \
	int return_code; \
	struct ANY_OBJECT_ITERATOR_DATA(object_type) *iterator_data; \
\
	ENTER(ANY_OBJECT_ITERATOR(object_type)); \
	if (any_object&&(iterator_data= \
		(struct ANY_OBJECT_ITERATOR_DATA(object_type) *)iterator_data_void)&& \
		iterator_data->iterator_function) \
	{ \
		if (ANY_OBJECT_TYPE_STRING(object_type) == any_object->type_string) \
		{ \
			return_code=(iterator_data->iterator_function)( \
				(struct object_type *)any_object->subobject,iterator_data->user_data); \
		} \
		else \
		{ \
			return_code=1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"FOR_EACH_OBJECT_IN_LIST(ANY_OBJECT(" \
			#object_type ")).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* ANY_OBJECT_ITERATOR_FUNCTION(object_type) */

#define DEFINE_FOR_EACH_OBJECT_IN_ANY_OBJECT_LIST_FUNCTION( object_type ) \
PROTOTYPE_FOR_EACH_OBJECT_IN_ANY_OBJECT_LIST_FUNCTION( object_type ) \
/***************************************************************************** \
LAST MODIFIED : 23 August 2000 \
\
DESCRIPTION : \
Performs the <iterator_function> with <user_data> for each object in \
<any_object_list> of type <object_type>. \
============================================================================*/ \
{ \
	int return_code; \
	struct ANY_OBJECT_ITERATOR_DATA(object_type) iterator_data; \
 \
	ENTER(FOR_EACH_OBJECT_IN_LIST(ANY_OBJECT(object_type))); \
	if (iterator_function&&any_object_list) \
	{ \
		iterator_data.iterator_function = iterator_function; \
		iterator_data.user_data = user_data; \
		return_code=FOR_EACH_OBJECT_IN_LIST(Any_object)( \
			ANY_OBJECT_ITERATOR(object_type),(void *)&iterator_data, \
			any_object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"FOR_EACH_OBJECT_IN_LIST(ANY_OBJECT(" \
			#object_type ")).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* FOR_EACH_OBJECT_IN_LIST(ANY_OBJECT(object_type)) */

#if defined (FULL_NAMES)
#define ANY_OBJECT_CONDITIONAL_DATA( object_type ) \
	any_object_conditional_data_ ## object_type
#else
#define ANY_OBJECT_CONDITIONAL_DATA( object_type ) \
	aocd ## object_type
#endif

#define DECLARE_ANY_OBJECT_CONDITIONAL_DATA_TYPE( object_type ) \
struct ANY_OBJECT_CONDITIONAL_DATA(object_type) \
/***************************************************************************** \
LAST MODIFIED : 22 August 2000 \
\
DESCRIPTION : \
User_data for ANY_OBJECT_CONDITIONAL(object_type).
============================================================================*/ \
{ \
	ANY_OBJECT_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	void *user_data; \
}

#if defined (FULL_NAMES)
#define ANY_OBJECT_CONDITIONAL( object_type ) \
	any_object_conditional_ ## object_type
#else
#define ANY_OBJECT_CONDITIONAL_FUNCTION( object_type ) \
	aoc ## object_type
#endif

#define DEFINE_ANY_OBJECT_CONDITIONAL_FUNCTION( object_type ) \
static int ANY_OBJECT_CONDITIONAL(object_type)( \
	struct Any_object *any_object,void *conditional_data_void) \
/***************************************************************************** \
LAST MODIFIED : 22 August 2000 \
\
DESCRIPTION : \
Returns true if <any_object> is of object_type, and there is either no \
conditional function or it returns true when called for the object with the \
given user_data. \
============================================================================*/ \
{ \
	int return_code; \
	struct ANY_OBJECT_CONDITIONAL_DATA(object_type) *conditional_data; \
\
	ENTER(ANY_OBJECT_CONDITIONAL(object_type)); \
	if (any_object&&(conditional_data= \
		(struct ANY_OBJECT_CONDITIONAL_DATA(object_type) *)conditional_data_void)) \
	{ \
		if ((ANY_OBJECT_TYPE_STRING(object_type) == any_object->type_string) && \
			((!conditional_data->conditional_function) || \
				(conditional_data->conditional_function)( \
					(struct object_type *)any_object->subobject, \
					conditional_data->user_data))) \
		{ \
			return_code=1; \
		} \
		else \
		{ \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"FOR_EACH_OBJECT_IN_LIST(ANY_OBJECT(" \
			#object_type ")).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* ANY_OBJECT_CONDITIONAL_FUNCTION(object_type) */

#define DEFINE_FIRST_OBJECT_IN_ANY_OBJECT_LIST_THAT_FUNCTION( object_type ) \
PROTOTYPE_FIRST_OBJECT_IN_ANY_OBJECT_LIST_THAT_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 23 August 2000 \
\
DESCRIPTION : \
Returns the first object of <object_type> in <any_object> list which \
satisfies the <conditional_function> with <user_data>. Returns the first \
object of <object_type> if <conditional_function> is NULL. \
============================================================================*/ \
{ \
	struct Any_object *any_object; \
	struct ANY_OBJECT_CONDITIONAL_DATA(object_type) conditional_data; \
	struct object_type *object; \
\
	ENTER(FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(object_type))); \
	if (any_object_list) \
	{ \
		conditional_data.conditional_function = conditional_function; \
		conditional_data.user_data = user_data; \
		if (any_object=FIRST_OBJECT_IN_LIST_THAT(Any_object)( \
			ANY_OBJECT_CONDITIONAL(object_type),(void *)&conditional_data, \
			any_object_list)) \
		{ \
			object = (struct object_type *)any_object->subobject; \
		} \
		else \
		{ \
			object = (struct object_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(" \
			#object_type ")).  Invalid argument(s)"); \
		object=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (object); \
} /* FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(object_type)) */

#define DEFINE_ANY_OBJECT( object_type ) \
DEFINE_ANY_OBJECT_TYPE_STRING(object_type) \
DEFINE_CREATE_ANY_OBJECT_FUNCTION(object_type) \
DEFINE_ADD_OBJECT_TO_ANY_OBJECT_LIST_FUNCTION(object_type) \
DEFINE_REMOVE_OBJECT_FROM_ANY_OBJECT_LIST_FUNCTION(object_type) \
DEFINE_IS_OBJECT_IN_ANY_OBJECT_LIST_FUNCTION(object_type) \
DECLARE_ANY_OBJECT_ITERATOR_DATA_TYPE(object_type); \
DEFINE_ANY_OBJECT_ITERATOR_FUNCTION(object_type) \
DEFINE_FOR_EACH_OBJECT_IN_ANY_OBJECT_LIST_FUNCTION(object_type) \
DECLARE_ANY_OBJECT_CONDITIONAL_DATA_TYPE(object_type); \
DEFINE_ANY_OBJECT_CONDITIONAL_FUNCTION(object_type) \
DEFINE_FIRST_OBJECT_IN_ANY_OBJECT_LIST_THAT_FUNCTION(object_type)

#endif /* !defined (ANY_OBJECT_DEFINITION_H) */

