/*******************************************************************************
FILE : managed_group.h

LAST MODIFIED : 1 September 1999

DESCRIPTION :
Special version of group.h for groups that keep their manager (and thus can
send manager modify messages).

Macros for forward declaring standard group types and prototyping standard group
functions.  Full declarations are not given, so as to hide the internal workings
and allow them to be changed without affecting other parts of the program.
==============================================================================*/
#if !defined (GROUP_H)
#define GROUP_H
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"

/*
Macros
======
*/

/*
Global types
------------
*/
#if defined (FULL_NAMES)
#define GROUP_( object_type )  group_ ## object_type
#else
#define GROUP_( object_type )  g ## object_type
#endif
#define GROUP( object_type )  GROUP_(object_type)

/*???DB.  Should this be typedef'd and be a pointer ? */
#define DECLARE_GROUP_TYPE( object_type ) \
struct GROUP(object_type)

#if defined (FULL_NAMES)
#define GROUP_CONDITIONAL_FUNCTION_( object_type ) \
	group_conditional_function_ ## object_type
#else
#define GROUP_CONDITIONAL_FUNCTION_( object_type )  gc ## object_type
#endif
#define GROUP_CONDITIONAL_FUNCTION( object_type ) \
	GROUP_CONDITIONAL_FUNCTION_(object_type)

#define DECLARE_GROUP_CONDITIONAL_FUNCTION( object_type ) \
typedef int (GROUP_CONDITIONAL_FUNCTION(object_type)) \
	(struct object_type *object,void *user_data)

#if defined (FULL_NAMES)
#define GROUP_ITERATOR_FUNCTION_( object_type ) \
	group_iterator_function_ ## object_type
#else
#define GROUP_ITERATOR_FUNCTION_( object_type )  gi ## object_type
#endif
#define GROUP_ITERATOR_FUNCTION( object_type ) \
	GROUP_ITERATOR_FUNCTION_(object_type)

#define DECLARE_GROUP_ITERATOR_FUNCTION( object_type ) \
typedef int (GROUP_ITERATOR_FUNCTION(object_type)) \
	(struct object_type *object,void *user_data)

/*
Global functions
----------------
*/
#define CREATE_GROUP( object_type )  CREATE(GROUP(object_type))

#define PROTOTYPE_CREATE_GROUP_FUNCTION( object_type ) \
struct GROUP(object_type) *CREATE_GROUP(object_type)(char *name) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Allocates memory and assigns fields for a object_type group and adds the group \
to the list of all object_type groups. \
???DB.  Don't add to list of all ? \
==============================================================================*/

#define DESTROY_GROUP( object_type )  DESTROY(GROUP(object_type))

#define PROTOTYPE_DESTROY_GROUP_FUNCTION( object_type ) \
int DESTROY_GROUP(object_type)(struct GROUP(object_type) **group_address) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Removes the group from the list of all object_type groups, destroys the group \
list, frees the memory for <**group_address> and sets <*group_address> to \
NULL. \
==============================================================================*/

#define COPY_GROUP( object_type )  COPY(GROUP(object_type))

#define PROTOTYPE_COPY_GROUP_FUNCTION( object_type ) \
int COPY_GROUP(object_type)(struct GROUP(object_type) *target_group, \
	struct GROUP(object_type) *source_group) \
/***************************************************************************** \
LAST MODIFIED : 18 January 1997 \
\
DESCRIPTION : \
Replaces the contents of the <target_group> with those of the <source_group>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define REMOVE_OBJECT_FROM_GROUP( object_type ) \
	group_remove_object_ ## object_type
#else
#define REMOVE_OBJECT_FROM_GROUP( object_type )  rag ## object_type
#endif

#define PROTOTYPE_REMOVE_OBJECT_FROM_GROUP( object_type ) \
int REMOVE_OBJECT_FROM_GROUP(object_type)(struct object_type *object, \
	struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Removes the <object> from the <group>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define REMOVE_OBJECTS_FROM_GROUP_THAT_( object_type )  \
	group_remove_objects_that_ ## object_type
#else
#define REMOVE_OBJECTS_FROM_GROUP_THAT_( object_type )  wg ## object_type
#endif
#define REMOVE_OBJECTS_FROM_GROUP_THAT( object_type ) \
	REMOVE_OBJECTS_FROM_GROUP_THAT_(object_type)

#define PROTOTYPE_REMOVE_OBJECTS_FROM_GROUP_THAT( object_type ) \
int REMOVE_OBJECTS_FROM_GROUP_THAT(object_type)( \
	GROUP_CONDITIONAL_FUNCTION(object_type) *conditional,void *user_data, \
	struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 18 January 1997 \
\
DESCRIPTION : \
Removes all objects from the group that <conditional> returns true for \
==============================================================================*/

#if defined (FULL_NAMES)
#define REMOVE_ALL_OBJECTS_FROM_GROUP( object_type ) \
	group_remove_all_objects_ ## object_type
#else
#define REMOVE_ALL_OBJECTS_FROM_GROUP( object_type )  vg ## object_type
#endif

#define PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_GROUP( object_type ) \
int REMOVE_ALL_OBJECTS_FROM_GROUP(object_type)( \
	struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 18 January 1997 \
\
DESCRIPTION : \
Removes all objects from the <group>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define ADD_OBJECT_TO_GROUP( object_type )  group_add_object_ ## object_type
#else
#define ADD_OBJECT_TO_GROUP( object_type )  pg ## object_type
#endif

#define PROTOTYPE_ADD_OBJECT_TO_GROUP( object_type ) \
int ADD_OBJECT_TO_GROUP(object_type)(struct object_type *object, \
	struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Add the <object> to the <group>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define NUMBER_IN_GROUP( object_type )  group_number_ ## object_type
#else
#define NUMBER_IN_GROUP( object_type )  ng ## object_type
#endif

#define PROTOTYPE_NUMBER_IN_GROUP( object_type ) \
int NUMBER_IN_GROUP(object_type)(struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1994 \
\
DESCRIPTION : \
Returns the number of objects in the <group>. \
==============================================================================*/

#if defined (OLD_CODE)
#if defined (FULL_NAMES)
#define GET_GROUP_NAME( object_type )  group_get_name_ ## object_type
#else
#define GET_GROUP_NAME( object_type )  mg ## object_type
#endif

#define PROTOTYPE_GET_GROUP_NAME( object_type ) \
char *GET_GROUP_NAME(object_type)(struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Returns the name of the <group>. Danger! Returns pointer to actual name, not \
a copy. Obsolete; use GET_NAME(GROUP(OBJECT))(object_group) instead. \
==============================================================================*/
#endif /* defined (OLD_CODE) */

#if defined (FULL_NAMES)
#define FIRST_OBJECT_IN_GROUP_THAT( object_type ) \
	group_first_that_ ## object_type
#else
#define FIRST_OBJECT_IN_GROUP_THAT( object_type )  tg ## object_type
#endif

#define PROTOTYPE_FIRST_OBJECT_IN_GROUP_THAT_FUNCTION( object_type ) \
struct object_type *FIRST_OBJECT_IN_GROUP_THAT(object_type)( \
	GROUP_CONDITIONAL_FUNCTION(object_type) *conditional,void *user_data, \
	struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
If <conditional> is not NULL, the "first" object in the <group> that \
<conditional> returns true is returned, otherwise the "first" object in the \
<group> is returned. \
==============================================================================*/

#if defined (FULL_NAMES)
#define FOR_EACH_OBJECT_IN_GROUP( object_type ) group_for_each_ ## object_type
#else
#define FOR_EACH_OBJECT_IN_GROUP( object_type ) eg ## object_type
#endif

#define PROTOTYPE_FOR_EACH_OBJECT_IN_GROUP_FUNCTION( object_type ) \
int FOR_EACH_OBJECT_IN_GROUP(object_type)( \
	GROUP_ITERATOR_FUNCTION(object_type) *iterator,void *user_data, \
	struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Calls <iterator> for each object in the <group>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define FIND_BY_IDENTIFIER_IN_GROUP( object_type , identifier ) \
	group_find_by_identifier_ ## object_type ## identifier
#else
#define FIND_BY_IDENTIFIER_IN_GROUP( object_type , identifier ) \
	fg ## object_type ## identifier
#endif

#define PROTOTYPE_FIND_BY_IDENTIFIER_IN_GROUP_FUNCTION( object_type , \
	identifier, identifier_type ) \
struct object_type *FIND_BY_IDENTIFIER_IN_GROUP(object_type, \
	identifier)(identifier_type identifier,struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Searchs the <group> for the object with the specified <identifier> and returns \
the address of the object. \
==============================================================================*/

#if defined (FULL_NAMES)
#define IS_OBJECT_IN_GROUP( object_type )  is_object_in_group_ ## object_type
#else
#define IS_OBJECT_IN_GROUP( object_type )  gg ## object_type
#endif

#define PROTOTYPE_IS_OBJECT_IN_GROUP_FUNCTION( object_type ) \
struct object_type *IS_OBJECT_IN_GROUP(object_type)( \
	struct object_type *object,struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 19 February 1996 \
\
DESCRIPTION : \
Searchs the <group> for the object and returns the address of the object. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGED_GROUP_BEGIN_CACHE( object_type ) \
	group_begin_cache_ ## object_type
#else
#define MANAGED_GROUP_BEGIN_CACHE( object_type ) mgb ## object_type
#endif

#define PROTOTYPE_MANAGED_GROUP_BEGIN_CACHE_FUNCTION( object_type ) \
int MANAGED_GROUP_BEGIN_CACHE(object_type)(struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 10 February 1998 \
\
DESCRIPTION : \
Supresses updates from this point. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGED_GROUP_END_CACHE( object_type )  group_end_cache_ ## object_type
#else
#define MANAGED_GROUP_END_CACHE( object_type )  mge ## object_type
#endif

#define PROTOTYPE_MANAGED_GROUP_END_CACHE_FUNCTION( object_type ) \
int MANAGED_GROUP_END_CACHE(object_type)(struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 10 February 1998 \
\
DESCRIPTION : \
Performs a global update. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGED_GROUP_CAN_BE_DESTROYED( object_type )  group_can_be_destroyed_ ## object_type
#else
#define MANAGED_GROUP_CAN_BE_DESTROYED( object_type )  gcbd ## object_type
#endif

#define PROTOTYPE_MANAGED_GROUP_CAN_BE_DESTROYED_FUNCTION( object_type ) \
int MANAGED_GROUP_CAN_BE_DESTROYED(object_type)(struct GROUP(object_type) *group) \
/***************************************************************************** \
LAST MODIFIED : 1 September 1999 \
\
DESCRIPTION : \
Returns true if the managed group can be destroyed (access_count==1). \
==============================================================================*/

#define DECLARE_MANAGED_GROUP_TYPES( object_type ) \
DECLARE_GROUP_TYPE(object_type); \
DECLARE_GROUP_CONDITIONAL_FUNCTION(object_type); \
DECLARE_GROUP_ITERATOR_FUNCTION(object_type); \
DECLARE_LIST_TYPES(GROUP(object_type)); \
DECLARE_MANAGER_TYPES(GROUP(object_type))

#define PROTOTYPE_MANAGED_GROUP_FUNCTIONS( object_type ) \
PROTOTYPE_OBJECT_FUNCTIONS(GROUP(object_type)); \
PROTOTYPE_CREATE_GROUP_FUNCTION(object_type); \
PROTOTYPE_DESTROY_GROUP_FUNCTION(object_type); \
PROTOTYPE_COPY_GROUP_FUNCTION(object_type); \
PROTOTYPE_REMOVE_OBJECT_FROM_GROUP(object_type); \
PROTOTYPE_REMOVE_OBJECTS_FROM_GROUP_THAT(object_type); \
PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_GROUP(object_type); \
PROTOTYPE_ADD_OBJECT_TO_GROUP(object_type); \
PROTOTYPE_NUMBER_IN_GROUP(object_type); \
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(GROUP(object_type)); \
PROTOTYPE_FIRST_OBJECT_IN_GROUP_THAT_FUNCTION(object_type); \
PROTOTYPE_FOR_EACH_OBJECT_IN_GROUP_FUNCTION(object_type); \
PROTOTYPE_IS_OBJECT_IN_GROUP_FUNCTION(object_type); \
PROTOTYPE_MANAGED_GROUP_BEGIN_CACHE_FUNCTION(object_type); \
PROTOTYPE_MANAGED_GROUP_END_CACHE_FUNCTION(object_type); \
PROTOTYPE_MANAGED_GROUP_CAN_BE_DESTROYED_FUNCTION(object_type); \
PROTOTYPE_LIST_FUNCTIONS(GROUP(object_type)); \
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(GROUP(object_type),name,char *); \
PROTOTYPE_MANAGER_FUNCTIONS(GROUP(object_type)); \
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(GROUP(object_type),name,char *); \
PROTOTYPE_MANAGER_COPY_FUNCTIONS(GROUP(object_type),name,char *)

#endif
