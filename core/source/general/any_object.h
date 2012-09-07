/*******************************************************************************
FILE : any_object.h

LAST MODIFIED : 23 August 2000

DESCRIPTION :
Definition of Any_object structure for storing any object which can be uniquely
identified by a pointer with strong typing. LIST(Any_object) is declared.

Internally, type is represented by a static type_string pointer, constructed
from the type name itself. A void pointer is used for the object reference.

Macro prototypes in any_object_prototype.h and definitions in
any_object_definition.h allow Any_objects to be established for a particular
structure and through that interface remain strongly typed. Interfaces allow
a LIST(Any_object) to be treated as if it is a list of just the object_type for
which the macros are declared.
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
#if !defined (ANY_OBJECT_H)
#define ANY_OBJECT_H

#include "general/list.h"
#include "general/object.h"

/*
Global types
------------
*/

struct Any_object;
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Any_object);

/*
Global functions
----------------
*/

PROTOTYPE_OBJECT_FUNCTIONS(Any_object);
PROTOTYPE_LIST_FUNCTIONS(Any_object);

int DESTROY(Any_object)(struct Any_object **any_object_address);
/*******************************************************************************
LAST MODIFIED : 22 August 2000

DESCRIPTION :
Destroys the Any_object.
==============================================================================*/

int ensure_Any_object_is_in_list(struct Any_object *any_object,
	void *any_object_list_void);
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Iterator function for adding <any_object> to <any_object_list> if not currently
in it.
==============================================================================*/

int ensure_Any_object_is_not_in_list(struct Any_object *any_object,
	void *any_object_list_void);
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Iterator function for removing <any_object> from <any_object_list> if currently
in it.
==============================================================================*/

#endif /* !defined (ANY_OBJECT_H) */
