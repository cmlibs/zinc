/*******************************************************************************
FILE : simple_list.h

LAST MODIFIED : 17 June 1996

DESCRIPTION :
A simple_list is just like a list except that it is simple - ie it does not
contain pointers to things that need to be destroyed individually.
This file contains the types, prototypes and definitions required to use
simple_list's.
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
#if !defined (SIMPLE_LIST_H)
#define SIMPLE_LIST_H
#include "general/list.h"
/*
Macros
------
*/
#define PROTOTYPE_SIMPLE_LIST_CREATE_OBJECT_FUNCTION( object_type ) \
struct object_type *CREATE(object_type)(void) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Creates the object by allocating the structure. \
==============================================================================*/

#define DECLARE_SIMPLE_LIST_CREATE_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_SIMPLE_LIST_CREATE_OBJECT_FUNCTION(object_type) \
{ \
	struct object_type *object; \
\
	ENTER(CREATE(object_type)); \
	if (!ALLOCATE(object,struct object_type,1)) \
	{ \
		display_message(ERROR_MESSAGE, \
			"CREATE(" #object_type ").  Could not allocate structure"); \
	} \
	LEAVE; \
\
	return (object); \
} /* CREATE(object_type) */

#define PROTOTYPE_SIMPLE_LIST_DESTROY_OBJECT_FUNCTION( object_type ) \
int DESTROY(object_type)(struct object_type **object_address) \
/***************************************************************************** \
LAST MODIFIED : 2 November 1995 \
\
DESCRIPTION : \
Destroys the object. \
==============================================================================*/

#define DECLARE_SIMPLE_LIST_DESTROY_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_SIMPLE_LIST_DESTROY_OBJECT_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(DESTROY(object_type)); \
	if ((object_address)&&(*object_address)) \
	{ \
		/* free the memory */ \
		DEALLOCATE(*object_address); \
		return_code = 1; \
	} \
	else \
	{ \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DESTROY(object_type) */

#define PROTOTYPE_SIMPLE_LIST_ACCESS_OBJECT_FUNCTION( object_type ) \
struct object_type *ACCESS(object_type)(struct object_type *object) \
/***************************************************************************** \
LAST MODIFIED : 24 September 1995 \
\
DESCRIPTION : \
Returns the <object>. \
==============================================================================*/

#define DECLARE_SIMPLE_LIST_ACCESS_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_SIMPLE_LIST_ACCESS_OBJECT_FUNCTION(object_type) \
{ \
	ENTER(ACCESS(object_type)); \
	if (!object) \
	{ \
		display_message(ERROR_MESSAGE, \
			"ACCESS(" #object_type ").  Invalid argument"); \
	} \
	LEAVE; \
\
	return (object); \
} /* ACCESS(object_type) */

#define PROTOTYPE_SIMPLE_LIST_DEACCESS_OBJECT_FUNCTION( object_type ) \
int DEACCESS(object_type)(struct object_type **object_address) \
/***************************************************************************** \
LAST MODIFIED : 2 November 1995 \
\
DESCRIPTION : \
Destroys the object. \
==============================================================================*/

#define DECLARE_SIMPLE_LIST_DEACCESS_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_SIMPLE_LIST_DEACCESS_OBJECT_FUNCTION(object_type) \
{ \
	int return_code; \
\
	ENTER(DEACCESS(object_type)); \
	if (object_address&&(*object_address)) \
	{ \
		return_code=DESTROY(object_type)(object_address); \
	} \
	else \
	{ \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DEACCESS(object_type) */

#define PROTOTYPE_SIMPLE_LIST_OBJECT_FUNCTIONS( object_type ) \
PROTOTYPE_SIMPLE_LIST_CREATE_OBJECT_FUNCTION(object_type); \
PROTOTYPE_SIMPLE_LIST_DESTROY_OBJECT_FUNCTION(object_type); \
PROTOTYPE_SIMPLE_LIST_ACCESS_OBJECT_FUNCTION(object_type); \
PROTOTYPE_SIMPLE_LIST_DEACCESS_OBJECT_FUNCTION(object_type)

#define DECLARE_SIMPLE_LIST_OBJECT_FUNCTIONS( object_type ) \
DECLARE_SIMPLE_LIST_CREATE_OBJECT_FUNCTION(object_type) \
DECLARE_SIMPLE_LIST_DESTROY_OBJECT_FUNCTION(object_type) \
DECLARE_SIMPLE_LIST_ACCESS_OBJECT_FUNCTION(object_type) \
DECLARE_SIMPLE_LIST_DEACCESS_OBJECT_FUNCTION(object_type)
#endif

