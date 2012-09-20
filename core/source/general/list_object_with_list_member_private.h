/*******************************************************************************
FILE : list_object_with_list_member_private.h

LAST MODIFIED : 16 October 2002

DESCRIPTION :
Variant on list_private.h with special handling for add object/remove object
and destroy. When objects are added to the list their list_member is set to the
list, and it is cleared when removed. This makes it possible to identify whose
list the objects belong to. Special care is made to allow additional lists of
the same objects to be made; this involves not setting the list_member if it is
already set, and not clearing it if removing from a different list.

These lists are designed for objects which are automatically removed from their
list once the object is no longer in use. In such cases the macro
DECLARE_OBJECT_WITH_LIST_MEMBER_FUNCTIONS should also be invoked for the
object_type prior to DECLARE_LIST_OBJECT_WITH_LIST_MEMBER_FUNCTIONS.

Note only a subset of list functions are currently implemented for this list
type. More standard list functions should be added as required. Note also that
only functions adding and removing objects from the list need to changed from
those in list_private.h -- which is included by this header.
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
#if !defined (LIST_OBJECT_WITH_LIST_MEMBER_PRIVATE_H)
#define LIST_OBJECT_WITH_LIST_MEMBER_PRIVATE_H

#include "general/list_private.h"

/*
Macros
======
*/

/*
Global functions
----------------
*/

#define DECLARE_OBJECT_WITH_LIST_MEMBER_DESTROY_LIST_FUNCTION( object_type , list_member ) \
PROTOTYPE_DESTROY_LIST_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 14 October 2002 \
\
DESCRIPTION : \
Special LIST DESTROY function for objects that have pointers to their lists. \
Clears this pointer if it matches the list. \
============================================================================*/ \
{ \
	int return_code; \
	struct LIST_ITEM(object_type) *item,*remove; \
\
	ENTER(DESTROY_LIST(object_type)); \
	if (list_address) \
	{ \
		if (*list_address) \
		{ \
			item=(*list_address)->head; \
			while (item) \
			{ \
				remove=item; \
				item=item->next; \
				if (remove->object->list_member == *list_address) \
				{ \
					remove->object->list_member = (struct LIST(object_type) *)NULL; \
				} \
				DEACCESS(object_type)(&(remove->object)); \
				DEALLOCATE(remove); \
			} \
			DEALLOCATE(*list_address); \
		} \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"DESTROY_LIST(" #object_type ").  Invalid argument"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DESTROY_LIST(object_type) */

#define DECLARE_OBJECT_WITH_LIST_MEMBER_ADD_OBJECT_TO_LIST_FUNCTION( object_type , list_member ) \
PROTOTYPE_ADD_OBJECT_TO_LIST_FUNCTION( object_type ) \
/***************************************************************************** \
LAST MODIFIED : 14 October 2002 \
\
DESCRIPTION : \
Special ADD_OBJECT_TO_LIST function for objects that have pointers to \
their lists. Sets the list_member to this list, if not already set. \
============================================================================*/ \
{ \
	int return_code; \
	struct LIST_ITEM(object_type) *add_item; \
\
	ENTER(ADD_OBJECT_TO_LIST(object_type)); \
	if (object && list) \
	{ \
		/* allocate memory for list item */ \
		if (ALLOCATE(add_item, struct LIST_ITEM(object_type), 1)) \
		{ \
			if (!add_item->list_member) \
			{ \
				add_item->list_member = list; \
			} \
			add_item->object = ACCESS(object_type)(object); \
			add_item->next = (struct LIST_ITEM(object_type) *)NULL; \
			if (list->tail) \
			{ \
				(list->tail)->next = add_item; \
				list->tail = add_item; \
			} \
			else \
			{ \
				list->head = add_item; \
				list->tail = add_item; \
			} \
			(list->count)++; \
			return_code = 1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, "ADD_OBJECT_TO_LIST(" #object_type \
				").  Could not allocate memory for list item"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"ADD_OBJECT_TO_LIST(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* ADD_OBJECT_TO_LIST(object_type) */

#define DECLARE_OBJECT_WITH_LIST_MEMBER_REMOVE_OBJECT_FROM_LIST_FUNCTION( object_type , list_member ) \
PROTOTYPE_REMOVE_OBJECT_FROM_LIST_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 14 October 2002 \
\
DESCRIPTION : \
Special REMOVE_OBJECT_FROM_LIST function for objects that have pointers to \
their lists. Clears this pointer for removed objects if it matches the list. \
============================================================================*/ \
{ \
	int return_code; \
	struct LIST_ITEM(object_type) *item, *previous_item; \
\
	ENTER(REMOVE_OBJECT_FROM_LIST(object_type)); \
	if (object && list) \
	{ \
		/* search for the object in the list */ \
		item = list->head; \
		previous_item = (struct LIST_ITEM(object_type) *)NULL; \
		while (item && (object != item->object)) \
		{ \
			previous_item = item; \
			item = item->next; \
		} \
		if (item) \
		{ \
			/* remove the object from the list */ \
			if (item == list->head) \
			{ \
				list->head = item->next; \
			} \
			else \
			{ \
				previous_item->next = item->next; \
			} \
			if (item == list->tail) \
			{ \
				list->tail = previous_item; \
			} \
			if (item->object->list_member == list) \
			{ \
				item->object->list_member = (struct LIST(object_type) *)NULL; \
			} \
			return_code = DEACCESS(object_type)(&(item->object)); \
			DEALLOCATE(item); \
			(list->count)--; \
			return_code = 1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"REMOVE_OBJECT_FROM_LIST(" #object_type ").  Could not find object"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_OBJECT_FROM_LIST(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* REMOVE_OBJECT_FROM_LIST(object_type) */

#define DECLARE_LIST_OBJECT_WITH_LIST_MEMBER_FUNCTIONS( \
 object_type , list_member ) \
DECLARE_CREATE_LIST_FUNCTION(object_type) \
DECLARE_OBJECT_WITH_LIST_MEMBER_DESTROY_LIST_FUNCTION( \
	object_type, list_member) \
DECLARE_OBJECT_WITH_LIST_MEMBER_ADD_OBJECT_TO_LIST_FUNCTION( \
	object_type, list_member) \
DECLARE_OBJECT_WITH_LIST_MEMBER_REMOVE_OBJECT_FROM_LIST_FUNCTION( \
	object_type, list_member) \
DECLARE_FIRST_OBJECT_IN_LIST_THAT_FUNCTION(object_type)

#endif /* !defined (LIST_OBJECT_WITH_LIST_MEMBER_PRIVATE_H) */
