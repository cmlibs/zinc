/*******************************************************************************
FILE : group.h

LAST MODIFIED : 18 January 1997

DESCRIPTION :
Macros for forward declaring standard group types and prototyping standard group
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
#if !defined (GROUP_H)
#define GROUP_H
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
#define GROUP_( object_type )  group_ ## object_type
#else
#define GROUP_( object_type )  g ## object_type
#endif
#define GROUP( object_type )  GROUP_(object_type)

/*???DB.  Should this be typedef'd and be a pointer ? */
#define DECLARE_GROUP_TYPE( object_type ) \
struct GROUP(object_type)

#if ! defined (SHORT_NAMES)
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

#if ! defined (SHORT_NAMES)
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

#if ! defined (SHORT_NAMES)
#define REMOVE_OBJECT_FROM_GROUP( object_type ) \
	group_remove_object_ ## object_type
#else
#define REMOVE_OBJECT_FROM_GROUP( object_type )  rg ## object_type
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

#if ! defined (SHORT_NAMES)
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

#if ! defined (SHORT_NAMES)
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

#if ! defined (SHORT_NAMES)
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

#if ! defined (SHORT_NAMES)
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
#if ! defined (SHORT_NAMES)
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

#if ! defined (SHORT_NAMES)
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

#if ! defined (SHORT_NAMES)
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

#if ! defined (SHORT_NAMES)
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

#if ! defined (SHORT_NAMES)
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

#define DECLARE_GROUP_TYPES( object_type ) \
DECLARE_GROUP_TYPE(object_type); \
DECLARE_GROUP_CONDITIONAL_FUNCTION(object_type); \
DECLARE_GROUP_ITERATOR_FUNCTION(object_type)

#define PROTOTYPE_GROUP_FUNCTIONS( object_type ) \
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
PROTOTYPE_IS_OBJECT_IN_GROUP_FUNCTION(object_type)

#endif
