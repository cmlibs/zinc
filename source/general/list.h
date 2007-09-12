/*******************************************************************************
FILE : list.h

LAST MODIFIED : 25 April 2003

DESCRIPTION :
Macros for forward declaring standard list types and prototyping standard list
functions.  Full declarations are not given, so as to hide the internal workings
and allow them to be changed without affecting other parts of the program.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (LIST_H)
#define LIST_H

#include "general/object.h"

/*
Macros
======
*/

/*
Global types
------------
*/
#if ! defined (SHORT_NAMES)
#define LIST_( object_type )  list_ ## object_type
#else
#define LIST_( object_type )  l ## object_type
#endif
#define LIST( object_type )  LIST_(object_type)

/*???DB.  Should this be typedef'd and be a pointer ? */
#define DECLARE_LIST_TYPE( object_type ) \
struct LIST(object_type)

#if ! defined (SHORT_NAMES)
#define LIST_CONDITIONAL_FUNCTION_( object_type ) \
	list_conditional_function_ ## object_type
#else
#define LIST_CONDITIONAL_FUNCTION_( object_type )  lc ## object_type
#endif
#define LIST_CONDITIONAL_FUNCTION( object_type ) \
	LIST_CONDITIONAL_FUNCTION_(object_type)

#define DECLARE_LIST_CONDITIONAL_FUNCTION( object_type ) \
typedef int (LIST_CONDITIONAL_FUNCTION(object_type)) \
	(struct object_type *object,void *user_data)

#if ! defined (SHORT_NAMES)
#define LIST_ITERATOR_FUNCTION_( object_type ) \
	list_iterator_function_ ## object_type
#else
#define LIST_ITERATOR_FUNCTION_( object_type )  li ## object_type
#endif
#define LIST_ITERATOR_FUNCTION( object_type ) \
	LIST_ITERATOR_FUNCTION_(object_type)

#define DECLARE_LIST_ITERATOR_FUNCTION( object_type ) \
typedef int (LIST_ITERATOR_FUNCTION(object_type)) \
	(struct object_type *object,void *user_data)

/*
Global functions
----------------
*/
/*???DB.  What about ordering ? */
#define CREATE_LIST( object_type )  CREATE(LIST(object_type))

#define PROTOTYPE_CREATE_LIST_FUNCTION( object_type ) \
struct LIST(object_type) *CREATE_LIST(object_type)(void) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Creates a object_type list. \
==============================================================================*/

#define DESTROY_LIST( object_type )  DESTROY(LIST(object_type))

#define PROTOTYPE_DESTROY_LIST_FUNCTION( object_type ) \
int DESTROY_LIST(object_type)(struct LIST(object_type) **list_address) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Destroys the object_type list pointed to by <**list_address> and sets \
<*list_address> to NULL. \
==============================================================================*/

#define COPY_LIST( object_type )  COPY(LIST(object_type))

#define PROTOTYPE_COPY_LIST_FUNCTION( object_type ) \
int COPY_LIST(object_type)(struct LIST(object_type) *target_list, \
	struct LIST(object_type) *source_list) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Replaces the contents of the <target_list> with those of the <source_list>. \
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define REMOVE_OBJECT_FROM_LIST_( object_type ) \
	list_remove_object_ ## object_type
#else
#define REMOVE_OBJECT_FROM_LIST_( object_type )  rl ## object_type
#endif
#define REMOVE_OBJECT_FROM_LIST( object_type ) \
	REMOVE_OBJECT_FROM_LIST_(object_type)

#define PROTOTYPE_REMOVE_OBJECT_FROM_LIST_FUNCTION( object_type ) \
int REMOVE_OBJECT_FROM_LIST(object_type)(struct object_type *object, \
	struct LIST(object_type) *list) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Removes the <object> from the <list>. \
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define REMOVE_ALL_OBJECTS_FROM_LIST_( object_type )  \
	list_remove_all_objects_ ## object_type
#else
#define REMOVE_ALL_OBJECTS_FROM_LIST_( object_type )  ql ## object_type
#endif
#define REMOVE_ALL_OBJECTS_FROM_LIST( object_type ) \
	REMOVE_ALL_OBJECTS_FROM_LIST_(object_type)

#define PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_LIST_FUNCTION( object_type ) \
int REMOVE_ALL_OBJECTS_FROM_LIST(object_type)(struct LIST(object_type) *list) \
/***************************************************************************** \
LAST MODIFIED : 19 February 1996 \
\
DESCRIPTION : \
Removes all objects from the list \
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define REMOVE_OBJECTS_FROM_LIST_THAT_( object_type )  \
	list_remove_objects_that_ ## object_type
#else
#define REMOVE_OBJECTS_FROM_LIST_THAT_( object_type )  sl ## object_type
#endif
#define REMOVE_OBJECTS_FROM_LIST_THAT( object_type ) \
	REMOVE_OBJECTS_FROM_LIST_THAT_(object_type)

#define PROTOTYPE_REMOVE_OBJECTS_FROM_LIST_THAT_FUNCTION( object_type ) \
int REMOVE_OBJECTS_FROM_LIST_THAT(object_type)( \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional,void *user_data, \
	struct LIST(object_type) *list) \
/***************************************************************************** \
LAST MODIFIED : 14 November 1996 \
\
DESCRIPTION : \
Removes all objects from the list that <conditional> returns true for \
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define ADD_OBJECT_TO_LIST_( object_type )  list_add_object_ ## object_type
#else
#define ADD_OBJECT_TO_LIST_( object_type )  pl ## object_type
#endif
#define ADD_OBJECT_TO_LIST( object_type )  ADD_OBJECT_TO_LIST_(object_type)

#define PROTOTYPE_ADD_OBJECT_TO_LIST_FUNCTION( object_type ) \
int ADD_OBJECT_TO_LIST(object_type)(struct object_type *object, \
	struct LIST(object_type) *list) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Add the <object> to the <list>. \
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define ADD_OBJECT_TO_FRONT_OF_LIST_( object_type )  list_add_object_to_front ## object_type
#else
#define ADD_OBJECT_TO_FRONT_OF_LIST_( object_type )  pl ## object_type
#endif
#define ADD_OBJECT_TO_FRONT_OF_LIST( object_type )  ADD_OBJECT_TO_FRONT_OF_LIST_(object_type)

#define PROTOTYPE_ADD_OBJECT_TO_FRONT_OF_LIST_FUNCTION( object_type ) \
int ADD_OBJECT_TO_FRONT_OF_LIST(object_type)(struct object_type *object, \
	struct LIST(object_type) *list) \
/***************************************************************************** \
LAST MODIFIED : 11 September 2007 \
\
DESCRIPTION : \
Add the <object> to the front of <list>. \
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define NUMBER_IN_LIST_( object_type )  list_number_ ## object_type
#else
#define NUMBER_IN_LIST_( object_type )  nl ## object_type
#endif
#define NUMBER_IN_LIST( object_type )  NUMBER_IN_LIST_(object_type)

#define PROTOTYPE_NUMBER_IN_LIST_FUNCTION( object_type ) \
int NUMBER_IN_LIST(object_type)(struct LIST(object_type) *list) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Returns the number of items in the <list>. \
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define IS_OBJECT_IN_LIST_( object_type )  list_in_list_ ## object_type
#else
#define IS_OBJECT_IN_LIST_( object_type )  il ## object_type
#endif
#define IS_OBJECT_IN_LIST( object_type )  IS_OBJECT_IN_LIST_(object_type)

#define PROTOTYPE_IS_OBJECT_IN_LIST_FUNCTION( object_type ) \
int IS_OBJECT_IN_LIST(object_type)(struct object_type *object, \
	struct LIST(object_type) *list) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Returns true if the <object> is in the <list>. \
???DB.  Is this needed ? \
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define FIND_BY_IDENTIFIER_IN_LIST_( object_type , identifier ) \
	list_find_by_identifier_ ## object_type ## identifier
#else
#define FIND_BY_IDENTIFIER_IN_LIST_( object_type , identifier ) \
	fl ## object_type ## identifier
#endif
#define FIND_BY_IDENTIFIER_IN_LIST( object_type , identifier ) \
	FIND_BY_IDENTIFIER_IN_LIST_(object_type,identifier)

#define PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION( object_type , \
	identifier, identifier_type ) \
struct object_type *FIND_BY_IDENTIFIER_IN_LIST(object_type,identifier)( \
	identifier_type identifier,struct LIST(object_type) *list) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Searchs the <list> for the object with the specified <identifier> and returns \
the address of the object. \
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define FIRST_OBJECT_IN_LIST_THAT_( object_type ) \
	list_first_that_ ## object_type
#else
#define FIRST_OBJECT_IN_LIST_THAT_( object_type )  tl ## object_type
#endif
#define FIRST_OBJECT_IN_LIST_THAT( object_type ) \
	FIRST_OBJECT_IN_LIST_THAT_(object_type)

#define PROTOTYPE_FIRST_OBJECT_IN_LIST_THAT_FUNCTION( object_type ) \
struct object_type *FIRST_OBJECT_IN_LIST_THAT(object_type)( \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional,void *user_data, \
	struct LIST(object_type) *list) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
If <conditional> is not NULL, the "first" object in the <list> that \
<conditional> returns true is returned, otherwise the "first" object in the \
<list> is returned. \
==============================================================================*/

#if ! defined (SHORT_NAMES)
#define FOR_EACH_OBJECT_IN_LIST_( object_type )  list_for_each_ ## object_type
#else
#define FOR_EACH_OBJECT_IN_LIST_( object_type )  el ## object_type
#endif
#define FOR_EACH_OBJECT_IN_LIST( object_type ) \
	FOR_EACH_OBJECT_IN_LIST_(object_type)

#define PROTOTYPE_FOR_EACH_OBJECT_IN_LIST_FUNCTION( object_type ) \
int FOR_EACH_OBJECT_IN_LIST(object_type)( \
	LIST_ITERATOR_FUNCTION(object_type) *iterator,void *user_data, \
	struct LIST(object_type) *list) \
/***************************************************************************** \
LAST MODIFIED : 23 September 1995 \
\
DESCRIPTION : \
Calls <iterator> for each object in the <list>. \
==============================================================================*/

#define LIST_CLASS(object_type) list_class_ ## object_type
/***************************************************************************** \
LAST MODIFIED : 9 February 2007 \
\
DESCRIPTION : \
Wraps the existing Manager functionality and types into a class. \
==============================================================================*/
#define DEFINE_LIST_CLASS(object_type) \
class LIST_CLASS(object_type) \
{\
public: \
	typedef LIST(object_type) List_type; \
	typedef LIST_CONDITIONAL_FUNCTION(object_type) List_conditional_function; \
\
	LIST(object_type) *list; \
\
   LIST_CLASS(object_type)(LIST(object_type) *list) : list(list) \
	{ \
	} \
\
	inline int number_in_list() \
	{ \
		return NUMBER_IN_LIST(object_type)(list); \
	} \
\
	inline int for_each_object_in_list(	\
		LIST_ITERATOR_FUNCTION(object_type) *iterator, void *user_data) \
	{ \
		return FOR_EACH_OBJECT_IN_LIST(object_type)( \
			iterator, user_data, list); \
	} \
\
	inline object_type *first_object_in_list_that(	\
	  LIST_CONDITIONAL_FUNCTION(object_type) *conditional, void *user_data, \
	  struct LIST(object_type) *list) \
	{ \
		return FIRST_OBJECT_IN_LIST_THAT(object_type)( \
			conditional, user_data, list); \
	} \
\
	inline int get_object_name(object_type *object, char **name) \
	{ \
		return GET_NAME(object_type)(object, name);		\
	} \
\
}; /* LIST_CLASS(object_type) */

#define DECLARE_LIST_TYPES( object_type ) \
DECLARE_LIST_TYPE(object_type); \
DECLARE_LIST_CONDITIONAL_FUNCTION(object_type); \
DECLARE_LIST_ITERATOR_FUNCTION(object_type)

/* PROTOTYPE_ADD_OBJECT_TO_FRONT_OF_LIST_FUNCTION(object_type); is not
	automatically included as it is unavailable to INDEXED lists so
	it is up to the definer of the list to PROTOTYPE and DECLARE it 
	if appropriate/required. */

#define PROTOTYPE_LIST_FUNCTIONS( object_type ) \
PROTOTYPE_CREATE_LIST_FUNCTION(object_type); \
PROTOTYPE_DESTROY_LIST_FUNCTION(object_type); \
PROTOTYPE_COPY_LIST_FUNCTION(object_type); \
PROTOTYPE_REMOVE_OBJECT_FROM_LIST_FUNCTION(object_type); \
PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_LIST_FUNCTION(object_type); \
PROTOTYPE_REMOVE_OBJECTS_FROM_LIST_THAT_FUNCTION(object_type); \
PROTOTYPE_ADD_OBJECT_TO_LIST_FUNCTION(object_type); \
PROTOTYPE_NUMBER_IN_LIST_FUNCTION(object_type); \
PROTOTYPE_IS_OBJECT_IN_LIST_FUNCTION(object_type); \
PROTOTYPE_FIRST_OBJECT_IN_LIST_THAT_FUNCTION(object_type); \
PROTOTYPE_FOR_EACH_OBJECT_IN_LIST_FUNCTION(object_type)
#endif
