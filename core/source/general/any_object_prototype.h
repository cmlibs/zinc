/*******************************************************************************
FILE : any_object_prototype.h

LAST MODIFIED : 23 August 2000

DESCRIPTION :
Macro prototypes for subclasses of struct Any_object.
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
#if !defined (ANY_OBJECT_PROTOTYPE_H)
#define ANY_OBJECT_PROTOTYPE_H

#include "general/any_object.h"
#include "general/list.h"
#include "general/object.h"

#define ANY_OBJECT(object_type) any_object_ ## object_type

/*
Global types
------------
*/

#if ! defined (SHORT_NAMES)
#define ANY_OBJECT_CONDITIONAL_FUNCTION( object_type ) \
	any_object_conditional_function_ ## object_type
#else
#define ANY_OBJECT_CONDITIONAL_FUNCTION( object_type )  aoc ## object_type
#endif

#define DECLARE_ANY_OBJECT_CONDITIONAL_FUNCTION( object_type ) \
typedef int (ANY_OBJECT_CONDITIONAL_FUNCTION(object_type)) \
	(struct object_type *object,void *user_data)

#if ! defined (SHORT_NAMES)
#define ANY_OBJECT_ITERATOR_FUNCTION( object_type ) \
	any_object_iterator_function_ ## object_type
#else
#define ANY_OBJECT_ITERATOR_FUNCTION( object_type )  aoi ## object_type
#endif

#define DECLARE_ANY_OBJECT_ITERATOR_FUNCTION( object_type ) \
typedef int (ANY_OBJECT_ITERATOR_FUNCTION(object_type)) \
	(struct object_type *object,void *user_data)

/*
Global functions
----------------
*/

#define PROTOTYPE_CREATE_ANY_OBJECT_FUNCTION( object_type ) \
struct Any_object *CREATE(ANY_OBJECT(object_type))(struct object_type *object)
/*******************************************************************************
LAST MODIFIED : 22 August 2000

DESCRIPTION :
Creates an Any_object for <object>.
==============================================================================*/

#define PROTOTYPE_ADD_OBJECT_TO_ANY_OBJECT_LIST_FUNCTION( object_type ) \
int ADD_OBJECT_TO_LIST(ANY_OBJECT(object_type))(struct object_type *object, \
	struct LIST(Any_object) *any_object_list)
/*******************************************************************************
LAST MODIFIED : 22 August 2000

DESCRIPTION :
Add <object> to the <any_object_list>.
==============================================================================*/

#define PROTOTYPE_REMOVE_OBJECT_FROM_ANY_OBJECT_LIST_FUNCTION( object_type ) \
int REMOVE_OBJECT_FROM_LIST(ANY_OBJECT(object_type))( \
	struct object_type *object,struct LIST(Any_object) *any_object_list)
/*******************************************************************************
LAST MODIFIED : 22 August 2000

DESCRIPTION :
Removes <object> from the <any_object_list>.
==============================================================================*/

#define PROTOTYPE_IS_OBJECT_IN_ANY_OBJECT_LIST_FUNCTION( object_type ) \
struct Any_object *IS_OBJECT_IN_LIST(ANY_OBJECT(object_type))( \
	struct object_type *object,struct LIST(Any_object) *any_object_list)
/*******************************************************************************
LAST MODIFIED : 22 August 2000

DESCRIPTION :
If <object> is in <any_object_list>, return its Any_object structure.
==============================================================================*/

#define PROTOTYPE_FOR_EACH_OBJECT_IN_ANY_OBJECT_LIST_FUNCTION( object_type ) \
int FOR_EACH_OBJECT_IN_LIST(ANY_OBJECT(object_type))( \
	ANY_OBJECT_ITERATOR_FUNCTION(object_type) *iterator_function, \
	void *user_data,struct LIST(Any_object) *any_object_list)
/*******************************************************************************
LAST MODIFIED : 22 August 2000

DESCRIPTION :
Performs the <iterator_function> with <user_data> for each object in
<any_object_list> of type <object_type>.
==============================================================================*/

#define PROTOTYPE_FIRST_OBJECT_IN_ANY_OBJECT_LIST_THAT_FUNCTION( object_type ) \
struct object_type *FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(object_type))( \
	ANY_OBJECT_CONDITIONAL_FUNCTION(object_type) *conditional_function, \
	void *user_data,struct LIST(Any_object) *any_object_list)
/*******************************************************************************
LAST MODIFIED : 22 August 2000

DESCRIPTION :
Returns the first object of <object_type> in <any_object> list which
satisfies the <conditional_function> with <user_data>. Returns the first object
of <object_type> if <conditional_function> is NULL.
==============================================================================*/

#define PROTOTYPE_ANY_OBJECT( object_type ) \
DECLARE_ANY_OBJECT_CONDITIONAL_FUNCTION(object_type); \
DECLARE_ANY_OBJECT_ITERATOR_FUNCTION(object_type); \
PROTOTYPE_CREATE_ANY_OBJECT_FUNCTION(object_type); \
PROTOTYPE_ADD_OBJECT_TO_ANY_OBJECT_LIST_FUNCTION(object_type); \
PROTOTYPE_REMOVE_OBJECT_FROM_ANY_OBJECT_LIST_FUNCTION(object_type); \
PROTOTYPE_IS_OBJECT_IN_ANY_OBJECT_LIST_FUNCTION(object_type); \
PROTOTYPE_FOR_EACH_OBJECT_IN_ANY_OBJECT_LIST_FUNCTION(object_type); \
PROTOTYPE_FIRST_OBJECT_IN_ANY_OBJECT_LIST_THAT_FUNCTION(object_type)

#endif /* !defined (ANY_OBJECT_PROTOTYPE_H) */

