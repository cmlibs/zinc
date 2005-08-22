/*******************************************************************************
FILE : computed_field_value_index_ranges.h

LAST MODIFIED : 12 October 2001

DESCRIPTION :
Stores ranges of indices of field values in a multi-range for a Computed_field.
Used, eg., to indicate which components have been modified in an editor.
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
#if !defined (FIELD_VALUE_INDEX_RANGES_H)
#define FIELD_VALUE_INDEX_RANGES_H

#include "computed_field/computed_field.h"
#include "general/list.h"
#include "general/multi_range.h"
#include "general/object.h"

struct Field_value_index_ranges;
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Stores a multi_range indexed by field.
Used, eg., to indicate which components have been modified in an editor.
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Field_value_index_ranges);

/*
Global functions
----------------
*/

struct Field_value_index_ranges *CREATE(Field_value_index_ranges)(
	struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Creates a Field_value_index_ranges object for storing ranges of value indices
stored in <field>.
==============================================================================*/

int DESTROY(Field_value_index_ranges)(
	struct Field_value_index_ranges **field_value_index_ranges_address);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Destroys the Field_value_index_ranges.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Field_value_index_ranges);
PROTOTYPE_LIST_FUNCTIONS(Field_value_index_ranges);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Field_value_index_ranges,field, \
	struct Computed_field *);

int Field_value_index_ranges_add_range(
	struct Field_value_index_ranges *field_value_index_ranges,int start,int stop);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Adds the range from <start> to <stop> to the ranges in
<field_value_index_ranges>.
==============================================================================*/

int Field_value_index_ranges_is_index_in_range(
	struct Field_value_index_ranges *field_value_index_ranges,int index);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Returns true if the <index> is in the ranges of <field_value_index_ranges>.
==============================================================================*/

int Field_value_index_ranges_add_to_list(
	struct Field_value_index_ranges *field_value_index_ranges,
	void *field_value_index_ranges_list_void);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Ensures the <field_value_index_ranges> are in <field_value_index_ranges_list>.
==============================================================================*/

int Field_value_index_ranges_list_add_field_value_index(
	struct LIST(Field_value_index_ranges) *field_value_index_ranges_list,
	struct Computed_field *field,int index);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Shortcut for ensuring <field><index> is in the <field_value_index_ranges_list>.
==============================================================================*/

int Field_value_index_ranges_remove_from_list(
	struct Field_value_index_ranges *field_value_index_ranges,
	void *field_value_index_ranges_list_void);
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Ensures the <field_value_index_ranges> is not in
<field_value_index_ranges_list>.
==============================================================================*/

struct Computed_field *Field_value_index_ranges_get_field(
	struct Field_value_index_ranges *field_value_index_ranges);
/*******************************************************************************
LAST MODIFIED : 19 June 2000

DESCRIPTION :
Returns the field from the <field_value_index_ranges.
==============================================================================*/

struct Multi_range *Field_value_index_ranges_get_ranges(
	struct Field_value_index_ranges *field_value_index_ranges);
/*******************************************************************************
LAST MODIFIED : 19 June 2000

DESCRIPTION :
Returns the ranges from the <field_value_index_ranges.
==============================================================================*/

#endif /* !defined (FIELD_VALUE_INDEX_RANGES_H) */
