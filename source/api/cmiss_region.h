/*******************************************************************************
FILE : cmiss_region.h

LAST MODIFIED : 03 March 2005

DESCRIPTION :
The public interface to the Cmiss_regions.
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
#ifndef __CMISS_REGION_H__
#define __CMISS_REGION_H__

#include "general/object.h"
#include "api/cmiss_node.h"
#include "api/cmiss_element.h"
#include "api/cmiss_field.h"

/*
Global types
------------
*/

struct Cmiss_region;
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
==============================================================================*/

#ifndef CMISS_REGION_ID_DEFINED
   typedef struct Cmiss_region * Cmiss_region_id;
   #define CMISS_REGION_ID_DEFINED
#endif /* CMISS_REGION_ID_DEFINED */

/*
Global functions
----------------
*/

Cmiss_region_id Cmiss_region_create(void);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Creates a blank Cmiss_region.
==============================================================================*/

int Cmiss_region_destroy(Cmiss_region_id *region);
/*******************************************************************************
LAST MODIFIED : 3 January 2008

DESCRIPTION :
Destroys the <region> and sets the pointer to NULL.
==============================================================================*/

int Cmiss_region_begin_change(Cmiss_region_id region);
/*******************************************************************************
LAST MODIFIED : 6 June 2008

DESCRIPTION :
Changes made to the <region> between Cmiss_region_begin_change and
Cmiss_region_end_change do not generate events in the rest of cmgui until
the change count returns to zero.  This allows many changes to be made 
efficiently, resulting in only one update of the dependent objects.
==============================================================================*/

int Cmiss_region_end_change(Cmiss_region_id region);
/*******************************************************************************
LAST MODIFIED : 6 June 2008

DESCRIPTION :
Changes made to the <region> between Cmiss_region_begin_change and
Cmiss_region_end_change do not generate events in the rest of cmgui until
the change count returns to zero.  This allows many changes to be made 
efficiently, resulting in only one update of the dependent objects.
==============================================================================*/

int Cmiss_region_read_file(struct Cmiss_region *region, char *file_name);
/*******************************************************************************
LAST MODIFIED : 19 August 2002

DESCRIPTION :
==============================================================================*/

struct Cmiss_region *Cmiss_region_get_sub_region(struct Cmiss_region *region,
	const char *path);
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Returns the sub_region specified by the string <path> in <region>.
==============================================================================*/

struct Cmiss_element *Cmiss_region_get_element(struct Cmiss_region *region,
	const char *name);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Returns element with <name> in <region> if it exists.
==============================================================================*/

struct Cmiss_node *Cmiss_region_get_node(struct Cmiss_region *region,
	const char *name);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Returns node with <name> in <region> if it exists.
==============================================================================*/

Cmiss_node_id Cmiss_region_merge_Cmiss_node(Cmiss_region_id region,
	Cmiss_node_id node);
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Checks <node> is compatible with <region> and any existing Cmiss_node
using the same identifier, then merges it into <region>.
If no Cmiss_node of the same identifier exists in FE_region, <node> is added
to <region> and returned by this function, otherwise changes are merged into
the existing Cmiss_node and it is returned.
==============================================================================*/

int Cmiss_region_get_number_of_nodes_in_region(struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Returns the number of nodes in <region> if it exists.
==============================================================================*/

int Cmiss_region_get_number_of_elements_in_region(struct Cmiss_region *region);
/*******************************************************************************
LAST MODIFIED : 02 March 2005

DESCRIPTION :
Returns the number of elements in <region> if it exists.
==============================================================================*/

int Cmiss_region_for_each_node_in_region(struct Cmiss_region *region,
	Cmiss_node_iterator_function iterator_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Iterates over each node in <region>.
==============================================================================*/

int Cmiss_region_for_each_element_in_region(struct Cmiss_region *region,
	Cmiss_element_iterator_function iterator_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 02 March 2005

DESCRIPTION :
Iterates over each element in <region>.
==============================================================================*/

Cmiss_element_id Cmiss_region_merge_Cmiss_element(Cmiss_region_id region,
	Cmiss_element_id element);
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Checks <element> is compatible with <region> and any existing Cmiss_element
using the same identifier, then merges it into <region>.
If no Cmiss_element of the same identifier exists in Cmiss_region, <element> is added
to <region> and returned by this function, otherwise changes are merged into
the existing Cmiss_element and it is returned.
==============================================================================*/

int Cmiss_region_add_child_region(struct Cmiss_region *region,
	struct Cmiss_region *child_region, const char *child_name, int child_position);
/*******************************************************************************
LAST MODIFIED : 22 October 2002

DESCRIPTION :
Adds <child_region> to <region> at <child_position> under the <child_name>.
<child_position> must be from -1 to the regions number_of_child_regions; if it
is -1 it is added at the end of the list.
Ensures:
- <child_name> passes is_standard_identifier_name;
- <child_name> is not already used in <region>;
- <child_region> is not already in <region>;
- <child_region> is not the same as or contains <region>;
==============================================================================*/

Cmiss_field_id Cmiss_region_add_field(Cmiss_region_id region, 
	Cmiss_field_id field);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Adds <field> to <region>, and as a convenience returns the field id if
successful.
==============================================================================*/

Cmiss_field_id Cmiss_region_find_field_by_name(Cmiss_region_id region, 
	const char *field_name);
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Returns the field of <field_name> from the <region> if it is defined,
NULL otherwise.
==============================================================================*/

#endif /* __CMISS_REGION_H__ */
