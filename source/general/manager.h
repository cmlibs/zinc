/*******************************************************************************
FILE : manager.h

LAST MODIFIED : 22 June 2000

DESCRIPTION :
Managers oversee the creation, deletion and modification of global objects -
such as object_types, lights and cameras.  All registered clients of the manager
are sent a message when an object has changed.

This file contains macros for (forward) declaring global manager types and
prototyping global manager functions.  Full declarations are not given, so as to
hide the internal workings and allow them to be changed without affecting other
parts of the program.
???DB.  What about accessing ?  Should take over all accessing ? (after
	everything is converted to managers)

TO DO :
16 May 1997 (After making select into a template class)
1 Add a MANAGER_CREATE which creates an object with a unique identifier and adds
	it to the manager (see SELECT_MANAGER_CREATE)
==============================================================================*/
#if !defined (MANAGER_H)
#define MANAGER_H

#include "general/list.h"

/*
Macros
======
*/

/*
Global types
------------
*/
#if defined (FULL_NAMES)
#define MANAGER_( object_type )  manager_ ## object_type
#else
#define MANAGER_( object_type )  m ## object_type
#endif
#define MANAGER( object_type )  MANAGER_(object_type)

/*???DB.  Should this be typedef'd and be a pointer ? */
#define DECLARE_MANAGER_TYPE( object_type ) \
struct MANAGER(object_type)

#if defined (FULL_NAMES)
#define MANAGER_CHANGE_( object_type )  manager_change_ ## object_type
#else
#define MANAGER_CHANGE_( object_type )  mc ## object_type
#endif
#define MANAGER_CHANGE( object_type )  MANAGER_CHANGE_(object_type)

#if defined (FULL_NAMES)
#define MANAGER_CHANGE_NONE_( object_type )  manager_change_none_ ## object_type
#else
#define MANAGER_CHANGE_NONE_( object_type )  mcn ## object_type
#endif
#define MANAGER_CHANGE_NONE( object_type )  MANAGER_CHANGE_NONE_(object_type)

#if defined (FULL_NAMES)
#define MANAGER_CHANGE_ALL_( object_type )  manager_change_all_ ## object_type
#else
#define MANAGER_CHANGE_ALL_( object_type )  mca ## object_type
#endif
#define MANAGER_CHANGE_ALL( object_type )  MANAGER_CHANGE_ALL_(object_type)

#if defined (FULL_NAMES)
#define MANAGER_CHANGE_DELETE_( object_type ) \
	manager_change_delete_ ## object_type
#else
#define MANAGER_CHANGE_DELETE_( object_type )  mcd ## object_type
#endif
#define MANAGER_CHANGE_DELETE( object_type ) \
	MANAGER_CHANGE_DELETE_(object_type)

#if defined (FULL_NAMES)
#define MANAGER_CHANGE_ADD_( object_type )  manager_change_add_ ## object_type
#else
#define MANAGER_CHANGE_ADD_( object_type )  mcp ## object_type
#endif
#define MANAGER_CHANGE_ADD( object_type )  MANAGER_CHANGE_ADD_(object_type)

#if defined (FULL_NAMES)
#define MANAGER_CHANGE_IDENTIFIER_( object_type ) \
	manager_change_identifier_ ## object_type
#else
#define MANAGER_CHANGE_IDENTIFIER_( object_type )  mci ## object_type
#endif
#define MANAGER_CHANGE_IDENTIFIER( object_type ) \
	MANAGER_CHANGE_IDENTIFIER_(object_type)

#if defined (FULL_NAMES)
#define MANAGER_CHANGE_OBJECT_( object_type ) \
	manager_change_object_ ## object_type
#else
#define MANAGER_CHANGE_OBJECT_( object_type )  mco ## object_type
#endif
#define MANAGER_CHANGE_OBJECT( object_type ) \
	MANAGER_CHANGE_OBJECT_(object_type)

#if defined (FULL_NAMES)
#define MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER_( object_type ) \
	manager_change_object_not_identifier ## object_type
#else
#define MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER_( object_type )  mcn ## object_type
#endif
#define MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER( object_type ) \
	MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER_(object_type)

#define DECLARE_MANAGER_CHANGE_TYPE( object_type ) \
enum MANAGER_CHANGE(object_type) \
/***************************************************************************** \
LAST MODIFIED : 22 June 2000 \
\
DESCRIPTION : \
The message type sent to clients. \
============================================================================*/ \
{ \
	/* indicates that no changes have been made; no message sent */ \
	MANAGER_CHANGE_NONE(object_type), \
	/* sent if a multitude of changes have been made */ \
	MANAGER_CHANGE_ALL(object_type), \
	/* sent if there was a single delete */ \
	MANAGER_CHANGE_DELETE(object_type), \
	/* sent if there was a single add */ \
	MANAGER_CHANGE_ADD(object_type), \
	/* sent if only the identifier was modified in one object */ \
	MANAGER_CHANGE_IDENTIFIER(object_type), \
	/* if just one whole object was modified (may include identifier) */ \
	MANAGER_CHANGE_OBJECT(object_type), \
	/* if just the data changed in one object */ \
	MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(object_type) \
} /* enum MANAGER_CHANGE(object_type) */

#if defined (FULL_NAMES)
#define MANAGER_MESSAGE_( object_type )  manager_message_ ## object_type
#else
#define MANAGER_MESSAGE_( object_type )  mm ## object_type
#endif
#define MANAGER_MESSAGE( object_type )  MANAGER_MESSAGE_(object_type)

#define DECLARE_MANAGER_MESSAGE_TYPE( object_type ) \
struct MANAGER_MESSAGE(object_type) \
/***************************************************************************** \
LAST MODIFIED : 27 September 1995 \
\
DESCRIPTION : \
A message that will be sent when one of more of the objects being managed has \
changed. \
============================================================================*/ \
{ \
	enum MANAGER_CHANGE(object_type) change; \
	struct object_type *object_changed; \
} /* MANAGER_MESSAGE(object_type) */

#if defined (FULL_NAMES)
#define MANAGER_CALLBACK_FUNCTION_( object_type ) \
	manager_callback_function_ ## object_type
#else
#define MANAGER_CALLBACK_FUNCTION_( object_type )  mb ## object_type
#endif
#define MANAGER_CALLBACK_FUNCTION( object_type ) \
	MANAGER_CALLBACK_FUNCTION_(object_type)

#define DECLARE_MANAGER_CALLBACK_FUNCTION( object_type ) \
typedef void (MANAGER_CALLBACK_FUNCTION(object_type)) \
	(struct MANAGER_MESSAGE(object_type) *message,void *user_data)

#if defined (FULL_NAMES)
#define MANAGER_CONDITIONAL_FUNCTION_( object_type ) \
	manager_conditional_function_ ## object_type
#else
#define MANAGER_CONDITIONAL_FUNCTION_( object_type )  mf ## object_type
#endif
#define MANAGER_CONDITIONAL_FUNCTION( object_type ) \
	MANAGER_CONDITIONAL_FUNCTION_(object_type)

#define DECLARE_MANAGER_CONDITIONAL_FUNCTION( object_type ) \
typedef int (MANAGER_CONDITIONAL_FUNCTION(object_type)) \
	(struct object_type *object,void *user_data)

#if defined (FULL_NAMES)
#define MANAGER_ITERATOR_FUNCTION_( object_type ) \
	manager_iterator_function_ ## object_type
#else
#define MANAGER_ITERATOR_FUNCTION_( object_type )  mi ## object_type
#endif
#define MANAGER_ITERATOR_FUNCTION( object_type ) \
	MANAGER_ITERATOR_FUNCTION_(object_type)

#define DECLARE_MANAGER_ITERATOR_FUNCTION( object_type ) \
typedef int (MANAGER_ITERATOR_FUNCTION(object_type)) \
	(struct object_type *object,void *user_data)

/*
Global functions
----------------
*/
#define CREATE_MANAGER( object_type )  CREATE(MANAGER(object_type))

#define PROTOTYPE_CREATE_MANAGER_FUNCTION( object_type ) \
struct MANAGER(object_type) *CREATE_MANAGER(object_type)(void) \
/***************************************************************************** \
LAST MODIFIED : 27 September 1995 \
\
DESCRIPTION : \
Creates and returns a new manager. \
==============================================================================*/

#define DESTROY_MANAGER( object_type )  DESTROY(MANAGER(object_type))

#define PROTOTYPE_DESTROY_MANAGER_FUNCTION( object_type ) \
int DESTROY_MANAGER(object_type)( \
	struct MANAGER(object_type) **manager_address) \
/***************************************************************************** \
LAST MODIFIED : 27 September 1995 \
\
DESCRIPTION : \
Destroys the <**manager> and sets <*manager> to NULL. \
==============================================================================*/

#if defined (FULL_NAMES)
#define REMOVE_OBJECT_FROM_MANAGER_( object_type ) \
	manager_remove_object_ ## object_type
#else
#define REMOVE_OBJECT_FROM_MANAGER_( object_type )  rm ## object_type
#endif
#define REMOVE_OBJECT_FROM_MANAGER( object_type ) \
	REMOVE_OBJECT_FROM_MANAGER_(object_type)

#define PROTOTYPE_REMOVE_OBJECT_FROM_MANAGER_FUNCTION( object_type ) \
int REMOVE_OBJECT_FROM_MANAGER(object_type)(struct object_type *object, \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Deletes an <object> from the <manager>. If the <manager> is locked or the \
operation cannot be performed, it returns zero otherwise a non-zero is \
returned. \
==============================================================================*/

#if defined (FULL_NAMES)
#define REMOVE_ALL_OBJECTS_FROM_MANAGER_( object_type ) \
	manager_remove_all_objects_ ## object_type
#else
#define REMOVE_ALL_OBJECTS_FROM_MANAGER_( object_type )  sm ## object_type
#endif
#define REMOVE_ALL_OBJECTS_FROM_MANAGER( object_type ) \
	REMOVE_ALL_OBJECTS_FROM_MANAGER_(object_type)

#define PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_MANAGER_FUNCTION( object_type ) \
int REMOVE_ALL_OBJECTS_FROM_MANAGER(object_type)( \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 16 September 1996 \
\
DESCRIPTION : \
Removes all the objects from the <manager>. If the <manager> is locked or the \
operation cannot be performed, it returns zero otherwise a non-zero is \
returned. \
==============================================================================*/

#if defined (FULL_NAMES)
#define ADD_OBJECT_TO_MANAGER_( object_type ) \
	manager_add_object_ ## object_type
#else
#define ADD_OBJECT_TO_MANAGER_( object_type )  pm ## object_type
#endif
#define ADD_OBJECT_TO_MANAGER( object_type ) \
	ADD_OBJECT_TO_MANAGER_(object_type)

#define PROTOTYPE_ADD_OBJECT_TO_MANAGER_FUNCTION( object_type ) \
int ADD_OBJECT_TO_MANAGER(object_type)(struct object_type *object, \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Adds a <object> to the <manager>. If the <manager> is locked or the \
operation cannot be performed, it returns zero, otherwise a non-zero is \
returned. \
==============================================================================*/

#if defined (FULL_NAMES)
#define NUMBER_IN_MANAGER_( object_type )  manager_number_ ## object_type
#else
#define NUMBER_IN_MANAGER_( object_type )  nm ## object_type
#endif
#define NUMBER_IN_MANAGER( object_type )  NUMBER_IN_MANAGER_(object_type)

#define PROTOTYPE_NUMBER_IN_MANAGER_FUNCTION( object_type ) \
int NUMBER_IN_MANAGER(object_type)(struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 21 March 1996 \
\
DESCRIPTION : \
Returns the number of items in the <manager>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGER_MODIFY_( object_type )  manager_modify_ ## object_type
#else
#define MANAGER_MODIFY_( object_type )  hm ## object_type
#endif
#define MANAGER_MODIFY( object_type )  MANAGER_MODIFY_(object_type)

#define PROTOTYPE_MANAGER_MODIFY_FUNCTION( object_type ) \
int MANAGER_MODIFY(object_type)(struct object_type *object, \
	struct object_type *new_data,struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Copies the <new_data> to the <object>, including the new identifier.  If the \
<manager> is locked or the operation cannot be performed, it returns zero \
otherwise it returns a non-zero. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGER_MODIFY_NOT_IDENTIFIER_( object_type , identifier_field_name ) \
	manager_modify_not_identifier_ ## object_type ## identifier_field_name
#else
#define MANAGER_MODIFY_NOT_IDENTIFIER_( object_type , identifier_field_name ) \
	jm ## object_type ## identifier_field_name
#endif
#define MANAGER_MODIFY_NOT_IDENTIFIER( object_type , identifier_field_name ) \
	MANAGER_MODIFY_NOT_IDENTIFIER_(object_type,identifier_field_name)

#define PROTOTYPE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION( object_type , \
	identifier_field_name ) \
int MANAGER_MODIFY_NOT_IDENTIFIER(object_type,identifier_field_name)( \
	struct object_type *object,struct object_type *new_data, \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Copies the data from <new_data> to <object>, but does not change the \
identifier.  If the <manager> is locked or the operation cannot be performed, \
it returns zero, otherwise a non-zero is returned. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGER_MODIFY_IDENTIFIER_( object_type , identifier_field_name ) \
	manager_modify_identifier_ ## object_type ## identifier_field_name
#else
#define MANAGER_MODIFY_IDENTIFIER_( object_type , identifier_field_name ) \
	km ## object_type ## identifier_field_name
#endif
#define MANAGER_MODIFY_IDENTIFIER( object_type , identifier_field_name ) \
	MANAGER_MODIFY_IDENTIFIER_(object_type,identifier_field_name)

#define PROTOTYPE_MANAGER_MODIFY_IDENTIFIER_FUNCTION( object_type , \
	identifier_field_name , identifier_type ) \
int MANAGER_MODIFY_IDENTIFIER(object_type,identifier_field_name)( \
	struct object_type *object,identifier_type new_identifier, \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Changes the identifier of the <object>  to the <new_identifier>.  If the \
<manager> is locked or the operation cannot be performed, it returns zero, \
otherwise a non-zero is returned. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGER_REGISTER_( object_type )  manager_register_ ## object_type
#else
#define MANAGER_REGISTER_( object_type )  qm ## object_type
#endif
#define MANAGER_REGISTER( object_type )  MANAGER_REGISTER_(object_type)

#define PROTOTYPE_MANAGER_REGISTER_FUNCTION( object_type ) \
void *MANAGER_REGISTER(object_type)( \
	MANAGER_CALLBACK_FUNCTION(object_type) *callback,void *user_data, \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Registers a <callback> and <user_data> to be called when objects in the \
<manager> are changed.  It returns the id to be used when the callback is \
deregistered. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGER_DEREGISTER_( object_type )  manager_deregister_ ## object_type
#else
#define MANAGER_DEREGISTER_( object_type )  wm ## object_type
#endif
#define MANAGER_DEREGISTER( object_type )  MANAGER_DEREGISTER_(object_type)

#define PROTOTYPE_MANAGER_DEREGISTER_FUNCTION( object_type ) \
int MANAGER_DEREGISTER(object_type)(void *callback_id, \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Deregisters the callback with the specified <callback_id> from the <manager>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define IS_MANAGED_( object_type )  is_managed_ ## object_type
#else
#define IS_MANAGED_( object_type )  im ## object_type
#endif
#define IS_MANAGED( object_type )  IS_MANAGED_(object_type)

#define PROTOTYPE_IS_MANAGED_FUNCTION( object_type ) \
int IS_MANAGED(object_type)(struct object_type *object, \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Returns a non-zero if the <object> is managed by the <manager>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define FIND_BY_IDENTIFIER_IN_MANAGER_( object_type , identifier ) \
	manager_find_by_identifier_ ## object_type ## identifier
#else
#define FIND_BY_IDENTIFIER_IN_MANAGER_( object_type , identifier ) \
	fm ## object_type ## identifier
#endif
#define FIND_BY_IDENTIFIER_IN_MANAGER( object_type , identifier ) \
	FIND_BY_IDENTIFIER_IN_MANAGER_(object_type,identifier)

#define PROTOTYPE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION( object_type , \
	identifier , identifier_type ) \
struct object_type *FIND_BY_IDENTIFIER_IN_MANAGER(object_type,identifier)( \
	identifier_type identifier,struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Returns the object with the specified <identifier> from the <manager>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define FIRST_OBJECT_IN_MANAGER_THAT_( object_type ) \
	manager_first_that_ ## object_type
#else
#define FIRST_OBJECT_IN_MANAGER_THAT_( object_type )  tm ## object_type
#endif
#define FIRST_OBJECT_IN_MANAGER_THAT( object_type ) \
	FIRST_OBJECT_IN_MANAGER_THAT_(object_type)

#define PROTOTYPE_FIRST_OBJECT_IN_MANAGER_THAT_FUNCTION( object_type ) \
struct object_type *FIRST_OBJECT_IN_MANAGER_THAT(object_type)( \
	MANAGER_CONDITIONAL_FUNCTION(object_type) *conditional,void *user_data, \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
If <conditional> is not NULL, the "first" object in the <manager> that \
<conditional> returns true is returned, otherwise the "first" object in the \
<list> is returned. \
==============================================================================*/

#if defined (FULL_NAMES)
#define FOR_EACH_OBJECT_IN_MANAGER_( object_type ) \
	manager_for_each_ ## object_type
#else
#define FOR_EACH_OBJECT_IN_MANAGER_( object_type )  em ## object_type
#endif
#define FOR_EACH_OBJECT_IN_MANAGER( object_type ) \
	FOR_EACH_OBJECT_IN_MANAGER_(object_type)

#define PROTOTYPE_FOR_EACH_OBJECT_IN_MANAGER_FUNCTION( object_type ) \
int FOR_EACH_OBJECT_IN_MANAGER(object_type)( \
	MANAGER_ITERATOR_FUNCTION(object_type) *iterator,void *user_data, \
	struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Calls <iterator> for each object  being managed by the <manager>. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGER_BEGIN_CACHE_( object_type )  manager_begin_cache_ ## object_type
	#else
#define MANAGER_BEGIN_CACHE_( object_type )  vm ## object_type
#endif
#define MANAGER_BEGIN_CACHE( object_type )  MANAGER_BEGIN_CACHE_(object_type)

#define PROTOTYPE_MANAGER_BEGIN_CACHE_FUNCTION( object_type ) \
int MANAGER_BEGIN_CACHE(object_type)(struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Supresses updates from this point. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGER_END_CACHE_( object_type )  manager_end_cache_ ## object_type
#else
#define MANAGER_END_CACHE_( object_type )  xm ## object_type
#endif
#define MANAGER_END_CACHE( object_type )  MANAGER_END_CACHE_(object_type)

#define PROTOTYPE_MANAGER_END_CACHE_FUNCTION( object_type ) \
int MANAGER_END_CACHE(object_type)(struct MANAGER(object_type) *manager) \
/***************************************************************************** \
LAST MODIFIED : 28 September 1995 \
\
DESCRIPTION : \
Performs a global update. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGER_COPY_WITH_IDENTIFIER_( object_type , identifier_field_name ) \
	manager_copy_with_identifier_ ## object_type ## identifier_field_name
#else
#define MANAGER_COPY_WITH_IDENTIFIER_( object_type , identifier_field_name ) \
	cm ## object_type ## identifier_field_name
#endif
#define MANAGER_COPY_WITH_IDENTIFIER( object_type , identifier_field_name ) \
	MANAGER_COPY_WITH_IDENTIFIER_(object_type,identifier_field_name)

#define PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION( object_type , \
	identifier_field_name ) \
int MANAGER_COPY_WITH_IDENTIFIER(object_type,identifier_field_name)( \
	struct object_type *destination,struct object_type *source) \
/***************************************************************************** \
LAST MODIFIED : 30 September 1995 \
\
DESCRIPTION : \
Copies all the contents of <source> to <destination>.  Used by the manager. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGER_COPY_WITHOUT_IDENTIFIER_( object_type , \
	identifier_field_name )  manager_copy_without_identifier_ ## object_type \
	## identifier_field_name
#else
#define MANAGER_COPY_WITHOUT_IDENTIFIER_( object_type , \
	identifier_field_name )  bm ## object_type ## identifier_field_name
#endif
#define MANAGER_COPY_WITHOUT_IDENTIFIER( object_type , identifier_field_name ) \
	MANAGER_COPY_WITHOUT_IDENTIFIER_(object_type,identifier_field_name)

#define PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION( object_type , \
	identifier_field_name ) \
int MANAGER_COPY_WITHOUT_IDENTIFIER(object_type,identifier_field_name)( \
	struct object_type *destination,struct object_type *source) \
/***************************************************************************** \
LAST MODIFIED : 30 September 1995 \
\
DESCRIPTION : \
Copies all the contents, except the identifier, of <source> to <destination>. \
Used by the manager. \
==============================================================================*/

#if defined (FULL_NAMES)
#define MANAGER_COPY_IDENTIFIER_( object_type , identifier_field_name ) \
	manager_copy_identifier_ ## object_type ## identifier_field_name
#else
#define MANAGER_COPY_IDENTIFIER_( object_type , identifier_field_name ) \
	am ## object_type ## identifier_field_name
#endif
#define MANAGER_COPY_IDENTIFIER( object_type , identifier_field_name ) \
	MANAGER_COPY_IDENTIFIER_(object_type,identifier_field_name)

#define PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION( object_type , \
	identifier_field_name , identifier_type ) \
int MANAGER_COPY_IDENTIFIER(object_type,identifier_field_name)( \
	struct object_type *destination,identifier_type identifier_field_name) \
/***************************************************************************** \
LAST MODIFIED : 30 September 1995 \
\
DESCRIPTION : \
Copies the <identifier_field_name> to <destination>.  Used by the manager. \
==============================================================================*/

#define DECLARE_MANAGER_TYPES( object_type ) \
DECLARE_MANAGER_CHANGE_TYPE(object_type); \
DECLARE_MANAGER_MESSAGE_TYPE(object_type); \
DECLARE_MANAGER_CALLBACK_FUNCTION(object_type); \
DECLARE_MANAGER_TYPE(object_type); \
DECLARE_MANAGER_CONDITIONAL_FUNCTION(object_type); \
DECLARE_MANAGER_ITERATOR_FUNCTION(object_type)

#define PROTOTYPE_MANAGER_FUNCTIONS( object_type ) \
PROTOTYPE_CREATE_MANAGER_FUNCTION(object_type); \
PROTOTYPE_DESTROY_MANAGER_FUNCTION(object_type); \
PROTOTYPE_REMOVE_OBJECT_FROM_MANAGER_FUNCTION(object_type); \
PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_MANAGER_FUNCTION(object_type); \
PROTOTYPE_NUMBER_IN_MANAGER_FUNCTION(object_type); \
PROTOTYPE_MANAGER_REGISTER_FUNCTION(object_type); \
PROTOTYPE_MANAGER_DEREGISTER_FUNCTION(object_type); \
PROTOTYPE_IS_MANAGED_FUNCTION(object_type); \
PROTOTYPE_FIRST_OBJECT_IN_MANAGER_THAT_FUNCTION(object_type); \
PROTOTYPE_FOR_EACH_OBJECT_IN_MANAGER_FUNCTION(object_type); \
PROTOTYPE_MANAGER_BEGIN_CACHE_FUNCTION(object_type); \
PROTOTYPE_MANAGER_END_CACHE_FUNCTION(object_type)

#define PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS( object_type , \
	identifier_field_name , identifier_type ) \
PROTOTYPE_ADD_OBJECT_TO_MANAGER_FUNCTION(object_type); \
PROTOTYPE_MANAGER_MODIFY_FUNCTION(object_type); \
PROTOTYPE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION(object_type, \
	identifier_field_name); \
PROTOTYPE_MANAGER_MODIFY_IDENTIFIER_FUNCTION(object_type, \
	identifier_field_name,identifier_type); \
PROTOTYPE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(object_type, \
	identifier_field_name,identifier_type)

#define PROTOTYPE_MANAGER_COPY_FUNCTIONS( object_type , \
	identifier_field_name , identifier_type ) \
PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(object_type, \
	identifier_field_name); \
PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(object_type, \
	identifier_field_name,identifier_type); \
PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(object_type, \
	identifier_field_name)

#define DECLARE_MANAGER_TYPES_AND_PROTOTYPE_FUNCTIONS( object_type ) \
DECLARE_MANAGER_TYPES(object_type); \
PROTOTYPE_MANAGER_FUNCTIONS(object_type)
#endif
