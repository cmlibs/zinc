/*******************************************************************************
FILE : group_private.h

LAST MODIFIED : 18 January 1997

DESCRIPTION :
Macros for declaring standard group types and declaring standard group
functions.  This file contains the details of the internal workings of the group
and should be included in the module that declares the group for a particular
object type, but should not be visible to modules that use groups for a
particular object type.  This allows changes to the internal structure to be
made without affecting other parts of the program.
???DB.  Access counts and destroying ?
==============================================================================*/
#if !defined (GROUP_PRIVATE_H)
#define GROUP_PRIVATE_H
#include "general/group.h"
#include "general/object.h"
#include "general/list.h"

/*
Macros
======
*/

/*
Local types
-----------
*/
#define FULL_DECLARE_GROUP_TYPE( object_type ) \
DECLARE_GROUP_TYPE(object_type) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
A named list of object_type . \
============================================================================*/ \
{ \
	/* the name of the group */ \
	char *name; \
	/* a list of the object_type in the group */ \
	struct LIST(object_type) *object_list; \
	/* the number of structures that point to this group.  The group cannot be \
		destroyed while this is greater than 0 */ \
	int access_count; \
} /* struct GROUP(object_type) */

/*
Global functions
----------------
*/
#define DECLARE_CREATE_GROUP_FUNCTION( object_type ) \
PROTOTYPE_CREATE_GROUP_FUNCTION(object_type) \
{ \
	struct GROUP(object_type) *group; \
\
	ENTER(CREATE_GROUP(object_type)); \
	/* allocate memory for the group */ \
	if (ALLOCATE(group,struct GROUP(object_type),1)) \
	{ \
		/* allocate memory for and assign the group name */ \
		if (name) \
		{ \
			if (ALLOCATE(group->name,char,strlen(name)+1)) \
			{ \
				strcpy(group->name,name); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
"CREATE_GROUP(" #object_type ").  Could not allocate memory for group name"); \
				DEALLOCATE(group); \
			} \
		} \
		else \
		{ \
			if (ALLOCATE(group->name,char,1)) \
			{ \
				*(group->name)='\0'; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
"CREATE_GROUP(" #object_type ").  Could not allocate memory for null group name"); \
				DEALLOCATE(group); \
			} \
		} \
		if (group) \
		{ \
			if (group->object_list=CREATE_LIST(object_type)()) \
			{ \
				group->access_count=0; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE,"CREATE_GROUP(" #object_type \
					").  Could not create list"); \
				DEALLOCATE(group->name); \
				DEALLOCATE(group); \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"CREATE_GROUP(" #object_type \
			").  Could not allocate memory for " #object_type " group"); \
	} \
	LEAVE; \
\
	return (group); \
} /* CREATE_GROUP(object_type) */

#define DECLARE_DESTROY_GROUP_FUNCTION( object_type ) \
PROTOTYPE_DESTROY_GROUP_FUNCTION(object_type) \
{ \
	int return_code; \
	struct GROUP(object_type) *group; \
\
	ENTER(DESTROY_GROUP(object_type)); \
	/* check the argument */ \
	if (group_address&&(group= *group_address)&&(group->access_count<=0)) \
	{ \
		/* destroy the list */ \
		DESTROY_LIST(object_type)(&(group->object_list)); \
		/* free the memory for the group */ \
		DEALLOCATE(group->name); \
		DEALLOCATE(*group_address); \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"DESTROY_GROUP(" #object_type  \
			").  Invalid argument"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DESTROY_GROUP(object_type) */

#define DECLARE_COPY_GROUP_FUNCTION( object_type ) \
PROTOTYPE_COPY_GROUP_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(COPY_GROUP(object_type)); \
	/* check the argument */ \
	if (source_group&&target_group) \
	{ \
		/* destroy the list */ \
		COPY_LIST(object_type)(target_group->object_list, \
			source_group->object_list); \
		/* free the memory for the group */ \
		DEALLOCATE(target_group->name); \
		if (ALLOCATE(target_group->name,char,strlen(source_group->name)+1)) \
		{ \
			strcpy(target_group->name,source_group->name); \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"COPY_GROUP(" #object_type  \
				").  Could not allocate memory for name"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"COPY_GROUP(" #object_type  \
			").  Invalid argument"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* COPY_GROUP(object_type) */

#define DECLARE_REMOVE_OBJECT_FROM_GROUP( object_type ) \
PROTOTYPE_REMOVE_OBJECT_FROM_GROUP(object_type) \
{ \
	int return_code; \
\
	ENTER(REMOVE_OBJECT_FROM_GROUP(object_type)); \
	/* check the arguments */ \
	if (object&&group) \
	{ \
		return_code=REMOVE_OBJECT_FROM_LIST(object_type)(object, \
			group->object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_OBJECT_FROM_GROUP(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_OBJECT_FROM_GROUP(object_type) */

#define DECLARE_REMOVE_OBJECTS_FROM_GROUP_THAT( object_type ) \
PROTOTYPE_REMOVE_OBJECTS_FROM_GROUP_THAT(object_type) \
{ \
	int return_code; \
\
	ENTER(REMOVE_OBJECTS_FROM_GROUP_THAT(object_type)); \
	/* check the arguments */ \
	if (conditional&&group) \
	{ \
		return_code=REMOVE_OBJECTS_FROM_LIST_THAT(object_type)(conditional, \
			user_data,group->object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
		"REMOVE_OBJECT_FROM_GROUP_THAT(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_OBJECTS_FROM_GROUP_THAT(object_type) */

#define DECLARE_REMOVE_ALL_OBJECTS_FROM_GROUP( object_type ) \
PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_GROUP(object_type) \
{ \
	int return_code; \
\
	ENTER(REMOVE_ALL_OBJECTS_FROM_GROUP(object_type)); \
	/* check the arguments */ \
	if (group) \
	{ \
		return_code=REMOVE_ALL_OBJECTS_FROM_LIST(object_type)( \
			group->object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
		"REMOVE_ALL_OBJECTS_FROM_GROUP(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_ALL_OBJECTS_FROM_GROUP(object_type) */

#define DECLARE_ADD_OBJECT_TO_GROUP( object_type ) \
PROTOTYPE_ADD_OBJECT_TO_GROUP(object_type) \
{ \
	int return_code; \
\
	ENTER(ADD_OBJECT_TO_GROUP(object_type)); \
	/* check the arguments */ \
	if (object&&group) \
	{ \
		return_code=ADD_OBJECT_TO_LIST(object_type)(object,group->object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"ADD_OBJECT_TO_GROUP(" #object_type  \
			").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* ADD_OBJECT_TO_GROUP(object_type) */

#define DECLARE_NUMBER_IN_GROUP( object_type ) \
PROTOTYPE_NUMBER_IN_GROUP(object_type) \
{ \
	int number; \
\
	ENTER(NUMBER_IN_GROUP(object_type)); \
	if (group) \
	{ \
		number=NUMBER_IN_LIST(object_type)(group->object_list); \
	} \
	else \
	{ \
		number=0; \
	} \
	LEAVE; \
\
	return (number); \
} /* NUMBER_IN_GROUP(object_type) */

#if defined (OLD_CODE)
#define DECLARE_GET_GROUP_NAME( object_type ) \
PROTOTYPE_GET_GROUP_NAME(object_type) \
{ \
	char *name; \
\
	ENTER(GET_GROUP_NAME(object_type)); \
	if (group) \
	{ \
		/*???DB.  Allocate ? */ \
		name=group->name; \
	} \
	else \
	{ \
		name=(char *)NULL; \
	} \
	LEAVE; \
\
	return (name); \
} /* GET_GROUP_NAME(object_type) */
#endif /* defined (OLD_CODE) */

#define DECLARE_FIRST_OBJECT_IN_GROUP_THAT_FUNCTION( object_type ) \
PROTOTYPE_FIRST_OBJECT_IN_GROUP_THAT_FUNCTION(object_type) \
{ \
	struct object_type *object; \
\
	ENTER(FIRST_OBJECT_IN_GROUP_THAT(object_type)); \
	/* check the arguments */ \
	if (group) \
	{ \
		object=FIRST_OBJECT_IN_LIST_THAT(object_type)(conditional,user_data, \
			group->object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FIRST_OBJECT_IN_GROUP_THAT(" #object_type ").  Invalid argument(s)"); \
		object=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (object); \
} /* FIRST_OBJECT_IN_GROUP_THAT(object_type) */

#define DECLARE_FOR_EACH_OBJECT_IN_GROUP_FUNCTION( object_type ) \
PROTOTYPE_FOR_EACH_OBJECT_IN_GROUP_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(FOR_EACH_OBJECT_IN_GROUP(object_type)); \
	/* check the arguments */ \
	if (group&&iterator) \
	{ \
		return_code=FOR_EACH_OBJECT_IN_LIST(object_type)(iterator,user_data, \
			group->object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FOR_EACH_OBJECT_IN_GROUP(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* FOR_EACH_OBJECT_IN_GROUP(object_type) */

#define DECLARE_FIND_BY_IDENTIFIER_IN_GROUP_FUNCTION( object_type , \
	identifier, identifier_type , compare_function ) \
PROTOTYPE_FIND_BY_IDENTIFIER_IN_GROUP_FUNCTION(object_type,identifier, \
	identifier_type) \
{ \
	struct object_type *object; \
\
	ENTER(FIND_BY_IDENTIFIER_IN_GROUP(object_type,identifier)); \
	if (group) \
	{ \
		object=FIND_BY_IDENTIFIER_IN_LIST(object_type,identifier)( \
			identifier,group->object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"FIND_BY_IDENTIFIER_IN_GROUP(" \
			#object_type "," #identifier ").  Invalid argument"); \
		object=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (object); \
} /* FIND_BY_IDENTIFIER_IN_GROUP(object_type,identifier) */

#define DECLARE_IS_OBJECT_IN_GROUP_FUNCTION( object_type ) \
PROTOTYPE_IS_OBJECT_IN_GROUP_FUNCTION(object_type) \
{ \
	struct object_type *return_object; \
\
	ENTER(IS_OBJECT_IN_GROUP(object_type)); \
	return_object = (struct object_type *)NULL; \
	if (group) \
	{ \
		if (IS_OBJECT_IN_LIST(object_type)(object,group->object_list)) \
		{ \
			return_object = object; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"IS_OBJECT_IN_GROUP(" \
			#object_type ").  Invalid argument"); \
	} \
	LEAVE; \
\
	return (return_object); \
} /* IS_OBJECT_IN_GROUP(object_type) */

#define DECLARE_GROUP_FUNCTIONS( object_type ) \
DECLARE_OBJECT_FUNCTIONS(GROUP(object_type)) \
DECLARE_CREATE_GROUP_FUNCTION(object_type) \
DECLARE_DESTROY_GROUP_FUNCTION(object_type) \
DECLARE_COPY_GROUP_FUNCTION(object_type) \
DECLARE_REMOVE_OBJECT_FROM_GROUP(object_type) \
DECLARE_REMOVE_OBJECTS_FROM_GROUP_THAT(object_type) \
DECLARE_REMOVE_ALL_OBJECTS_FROM_GROUP(object_type) \
DECLARE_ADD_OBJECT_TO_GROUP(object_type) \
DECLARE_NUMBER_IN_GROUP(object_type) \
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(GROUP(object_type)) \
DECLARE_FIRST_OBJECT_IN_GROUP_THAT_FUNCTION(object_type) \
DECLARE_FOR_EACH_OBJECT_IN_GROUP_FUNCTION(object_type) \
DECLARE_IS_OBJECT_IN_GROUP_FUNCTION(object_type)

#endif /* !defined (GROUP_PRIVATE_H) */
