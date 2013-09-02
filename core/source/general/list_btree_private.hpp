/***************************************************************************//**
 * FILE : indexed_list_btree_private.hpp
 *
 * Implementation of indexed lists using cmzn_btree template.
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#if !defined (INDEXED_LIST_BTREE_PRIVATE_HPP)
#define INDEXED_LIST_BTREE_PRIVATE_HPP

#include "general/cmiss_btree.hpp"
extern "C" {
#include "general/list.h"
}

#define CMZN_BTREE( object_type )  cmzn_set_ ## object_type

#define FULL_DECLARE_INDEXED_LIST_BTREE_TYPE( object_type )

/*
Global functions
----------------
*/

#define DECLARE_CREATE_INDEXED_LIST_BTREE_FUNCTION( object_type ) \
PROTOTYPE_CREATE_LIST_FUNCTION(object_type) \
{ \
	return reinterpret_cast<struct LIST(object_type) *>(CMZN_BTREE(object_type)::create_independent()); \
}

#define DECLARE_CREATE_RELATED_INDEXED_LIST_BTREE_FUNCTION( object_type ) \
PROTOTYPE_CREATE_RELATED_LIST_FUNCTION(object_type) \
{ \
	CMZN_BTREE(object_type) *other = reinterpret_cast<CMZN_BTREE(object_type) *>(other_list); \
	return reinterpret_cast<struct LIST(object_type) *>(other->create_related()); \
}

#define DECLARE_DESTROY_INDEXED_LIST_BTREE_FUNCTION( object_type ) \
PROTOTYPE_DESTROY_LIST_FUNCTION(object_type) \
{ \
	if (list_address) \
	{ \
		CMZN_BTREE(object_type) **cmiss_btree_address = \
			reinterpret_cast<CMZN_BTREE(object_type) **>(list_address); \
		return CMZN_BTREE(object_type)::deaccess(cmiss_btree_address); \
	} \
	return 0; \
}

#define DECLARE_COPY_INDEXED_LIST_BTREE_FUNCTION( object_type ) \
PROTOTYPE_COPY_LIST_FUNCTION(object_type) \
{ \
	int return_code; \
	if (target_list && source_list) \
	{ \
		CMZN_BTREE(object_type) *target = reinterpret_cast<CMZN_BTREE(object_type) *>(target_list); \
		CMZN_BTREE(object_type) *source = reinterpret_cast<CMZN_BTREE(object_type) *>(source_list); \
		*target = *source; \
		return_code = 1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"COPY_LIST(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	return (return_code); \
}

#define DECLARE_REMOVE_OBJECT_FROM_INDEXED_LIST_BTREE_FUNCTION( object_type ) \
PROTOTYPE_REMOVE_OBJECT_FROM_LIST_FUNCTION(object_type) \
{ \
	int return_code; \
	if (object && list) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		return_code = (1 == cmiss_btree->erase(object)); \
		if (!return_code) \
		{ \
			display_message(ERROR_MESSAGE, "REMOVE_OBJECT_FROM_LIST(" #object_type \
				").  Object is not in list"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_OBJECT_FROM_LIST(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	return (return_code); \
}

#define DECLARE_REMOVE_OBJECTS_FROM_INDEXED_LIST_BTREE_THAT_FUNCTION( object_type ) \
PROTOTYPE_REMOVE_OBJECTS_FROM_LIST_THAT_FUNCTION(object_type) \
{ \
	int return_code; \
	if (conditional && list) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		return_code = cmiss_btree->erase_conditional(conditional, user_data); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_OBJECTS_FROM_LIST_THAT" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	return (return_code); \
}

#define DECLARE_REMOVE_ALL_OBJECTS_FROM_INDEXED_LIST_BTREE_FUNCTION( object_type ) \
PROTOTYPE_REMOVE_ALL_OBJECTS_FROM_LIST_FUNCTION(object_type) \
{ \
	int return_code; \
	if (list) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		cmiss_btree->clear(); \
		return_code = 1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"REMOVE_ALL_OBJECTS_FROM_LIST" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	return (return_code); \
}

#define DECLARE_ADD_OBJECT_TO_INDEXED_LIST_BTREE_FUNCTION( object_type ) \
PROTOTYPE_ADD_OBJECT_TO_LIST_FUNCTION( object_type ) \
{ \
	int return_code; \
	if (object && list) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		return_code = cmiss_btree->insert(object); \
		if (!return_code) \
		{ \
			display_message(ERROR_MESSAGE, "ADD_OBJECT_TO_LIST(" #object_type \
				").  Object is already at that index"); \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"ADD_OBJECT_TO_LIST(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	return (return_code); \
}

#define DECLARE_NUMBER_IN_INDEXED_LIST_BTREE_FUNCTION( object_type ) \
PROTOTYPE_NUMBER_IN_LIST_FUNCTION(object_type) \
{ \
	if (list) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		return cmiss_btree->size(); \
	} \
	display_message(ERROR_MESSAGE, \
		"NUMBER_IN_LIST(" #object_type ").  Invalid argument"); \
	return 0; \
}

#define DECLARE_IS_OBJECT_IN_INDEXED_LIST_BTREE_FUNCTION( object_type ) \
PROTOTYPE_IS_OBJECT_IN_LIST_FUNCTION(object_type) \
{ \
	int return_code = 0; \
	if (list && object) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		return_code = cmiss_btree->contains(object); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"IS_OBJECT_IN_LIST(" #object_type ").  Invalid argument"); \
	} \
	return (return_code); \
}

#define DECLARE_FIRST_OBJECT_IN_INDEXED_LIST_BTREE_THAT_FUNCTION( object_type ) \
PROTOTYPE_FIRST_OBJECT_IN_LIST_THAT_FUNCTION(object_type) \
{ \
	if (list) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		return cmiss_btree->find_first_object_that(conditional, user_data); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FIRST_OBJECT_IN_LIST_THAT(" #object_type ").  Invalid argument(s)"); \
	} \
	return 0; \
}

#define DECLARE_FOR_EACH_OBJECT_IN_INDEXED_LIST_BTREE_FUNCTION( object_type ) \
PROTOTYPE_FOR_EACH_OBJECT_IN_LIST_FUNCTION(object_type) \
{ \
	int return_code = 1; \
	if (list && iterator) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		return_code = cmiss_btree->for_each_object(iterator, user_data); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FOR_EACH_OBJECT_IN_LIST(" #object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	return (return_code); \
}

#define DECLARE_CREATE_INDEXED_LIST_BTREE_ITERATOR_FUNCTION( object_type , iterator_type ) \
PROTOTYPE_CREATE_LIST_ITERATOR_FUNCTION(object_type,iterator_type) \
{ \
	CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
	if (cmiss_btree) \
		return new iterator_type(cmiss_btree); \
	return 0; \
}

#define DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_BTREE_FUNCTION( object_type , \
	identifier , identifier_type ) \
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(object_type,identifier, \
	identifier_type) \
{ \
	struct object_type *object = 0; \
	if (list) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		object = cmiss_btree->find_object_by_identifier(identifier); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, "FIND_BY_IDENTIFIER_IN_LIST(" #object_type \
			"," #identifier ").  Invalid argument"); \
	} \
	return (object); \
}

#define PROTOTYPE_INDEXED_LIST_BTREE_BEGIN_IDENTIFIER_CHANGE_FUNCTION( object_type , \
	identifier ) \
int LIST_BEGIN_IDENTIFIER_CHANGE(object_type,identifier)( \
	struct LIST(object_type) *list, struct object_type *object) \
/***************************************************************************** \
MANAGER functions using indexed object lists must call this before modifying \
the identifier of any object, and afterwards call the companion function \
LIST_END_IDENTIFIER_CHANGE with the returned \
identifier_change_data. These functions temporarily remove the object from \
any list it is in, then re-add it later so it is in the correct indexed \
position. <object> is ACCESSed between these two functions. \
Should only be declared with manager functions. \
============================================================================*/

#define DECLARE_INDEXED_LIST_BTREE_BEGIN_IDENTIFIER_CHANGE_FUNCTION( object_type , \
	identifier ) \
PROTOTYPE_INDEXED_LIST_BTREE_BEGIN_IDENTIFIER_CHANGE_FUNCTION(object_type, \
	identifier) \
{ \
	if (list && object) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		return cmiss_btree->begin_identifier_change(object); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"LIST_BEGIN_IDENTIFIER_CHANGE(" #object_type "," #identifier \
			").  Invalid argument(s)"); \
	} \
	return (0); \
}

#define PROTOTYPE_INDEXED_LIST_BTREE_END_IDENTIFIER_CHANGE_FUNCTION( \
	object_type , identifier ) \
void LIST_END_IDENTIFIER_CHANGE(object_type,identifier)( \
	struct LIST(object_type) *list) \
/***************************************************************************** \
Companion function to LIST_BEGIN_IDENTIFIER_CHANGE function. \
Re-adds the changed object to all the lists it was in. \
Should only be declared with manager functions. \
============================================================================*/

#define DECLARE_INDEXED_LIST_BTREE_END_IDENTIFIER_CHANGE_FUNCTION(object_type ,  \
	identifier ) \
PROTOTYPE_INDEXED_LIST_BTREE_END_IDENTIFIER_CHANGE_FUNCTION(object_type, \
	identifier) \
{ \
	if (list) \
	{ \
		CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
		cmiss_btree->end_identifier_change(); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"LIST_BEGIN_IDENTIFIER_CHANGE(" #object_type "," #identifier \
			").  Invalid argument(s)"); \
	} \
}

#define LIST_BTREE_STATISTICS(object_type,list) \
{ \
	CMZN_BTREE(object_type) *cmiss_btree = reinterpret_cast<CMZN_BTREE(object_type) *>(list); \
	int stem_count = 0; \
	int leaf_count = 0; \
	int min_leaf_depth = 0; \
	int max_leaf_depth = 0; \
	double mean_leaf_depth = 0.0; \
	double mean_stem_occupancy = 0.0; \
	double mean_leaf_occupancy = 0.0; \
	cmiss_btree->get_statistics(stem_count, leaf_count, \
		min_leaf_depth, max_leaf_depth, mean_leaf_depth, \
		mean_stem_occupancy, mean_leaf_occupancy); \
	display_message(INFORMATION_MESSAGE, "  Size = %d\n", cmiss_btree->size()); \
	display_message(INFORMATION_MESSAGE, "  Stem count = %d\n", stem_count); \
	display_message(INFORMATION_MESSAGE, "  Leaf count = %d\n", leaf_count); \
	display_message(INFORMATION_MESSAGE, "  Min leaf depth = %d\n", min_leaf_depth); \
	display_message(INFORMATION_MESSAGE, "  Max leaf depth = %d\n", max_leaf_depth); \
	display_message(INFORMATION_MESSAGE, "  Mean leaf depth = %g\n", mean_leaf_depth); \
	display_message(INFORMATION_MESSAGE, "  Mean stem occupancy = %g\n", mean_stem_occupancy); \
	display_message(INFORMATION_MESSAGE, "  Mean leaf occupancy = %g\n", mean_leaf_occupancy); \
}

/* prototypes for private headers, eg. finite_element_private.h */
#define PROTOTYPE_INDEXED_LIST_BTREE_IDENTIFIER_CHANGE_FUNCTIONS( object_type , identifier ) \
PROTOTYPE_INDEXED_LIST_BTREE_BEGIN_IDENTIFIER_CHANGE_FUNCTION(object_type, identifier); \
PROTOTYPE_INDEXED_LIST_BTREE_END_IDENTIFIER_CHANGE_FUNCTION(object_type,identifier)

#define DECLARE_INDEXED_LIST_BTREE_IDENTIFIER_CHANGE_FUNCTIONS( object_type , identifier ) \
DECLARE_INDEXED_LIST_BTREE_BEGIN_IDENTIFIER_CHANGE_FUNCTION(object_type,identifier) \
DECLARE_INDEXED_LIST_BTREE_END_IDENTIFIER_CHANGE_FUNCTION(object_type,identifier)

#define DECLARE_INDEXED_LIST_BTREE_FUNCTIONS( object_type ) \
DECLARE_CREATE_INDEXED_LIST_BTREE_FUNCTION(object_type) \
DECLARE_CREATE_RELATED_INDEXED_LIST_BTREE_FUNCTION(object_type) \
DECLARE_DESTROY_INDEXED_LIST_BTREE_FUNCTION(object_type) \
DECLARE_COPY_INDEXED_LIST_BTREE_FUNCTION(object_type) \
DECLARE_REMOVE_OBJECT_FROM_INDEXED_LIST_BTREE_FUNCTION(object_type) \
DECLARE_REMOVE_OBJECTS_FROM_INDEXED_LIST_BTREE_THAT_FUNCTION(object_type) \
DECLARE_REMOVE_ALL_OBJECTS_FROM_INDEXED_LIST_BTREE_FUNCTION(object_type) \
DECLARE_ADD_OBJECT_TO_INDEXED_LIST_BTREE_FUNCTION(object_type) \
DECLARE_NUMBER_IN_INDEXED_LIST_BTREE_FUNCTION(object_type) \
DECLARE_IS_OBJECT_IN_INDEXED_LIST_BTREE_FUNCTION(object_type) \
DECLARE_FIRST_OBJECT_IN_INDEXED_LIST_BTREE_THAT_FUNCTION(object_type) \
DECLARE_FOR_EACH_OBJECT_IN_INDEXED_LIST_BTREE_FUNCTION(object_type)

#endif /* !defined (INDEXED_LIST_BTREE_PRIVATE_HPP) */
