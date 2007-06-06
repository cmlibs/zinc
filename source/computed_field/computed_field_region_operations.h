/*******************************************************************************
FILE : computed_field_region_operations.h

LAST MODIFIED : 7 January 2003

DESCRIPTION :
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
#if !defined (COMPUTED_FIELD_REGION_OPERATIONS_H)
#define COMPUTED_FIELD_REGION_OPERATIONS_H

#include "finite_element/finite_element.h"
#include "region/cmiss_region.h"

int Computed_field_register_types_region_operations(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region);
/*******************************************************************************
LAST MODIFIED : 01 May 2006

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_region_sum(struct Computed_field *field,
	struct Computed_field *operate_field,
	struct Cmiss_region *operate_region, char *region_path);
/*******************************************************************************
LAST MODIFIED : 01 May 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_REGION_SUM, this field allows you to
evaluate one field to find "texture coordinates", use a find_element_xi field
to then calculate a corresponding element/xi and finally calculate values using
this element/xi and a third field.  You can then evaluate values on a "host"
mesh for any points "contained" inside.  The <search_element_group> is the group
from which any returned element_xi will belong.
If <use_point_five_when_out_of_bounds> is true then if the texture_coordinate_field
values cannot be found in the find_element_xi_field, then instead of returning
failure, the values will be set to 0.5 and returned as success.
Only elements that have dimension equals <element_dimension> will be searched.
==============================================================================*/

int Computed_field_get_type_region_sum(struct Computed_field *field,
	struct Computed_field *operate_field,
	struct Cmiss_region **operate_region, char **region_path);
/*******************************************************************************
LAST MODIFIED : 01 May 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_REGION_SUM, the function returns the three
fields which define the field.
Note that the fields are not ACCESSed and the <region_path> points to the
internally used path.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_REGION_OPERATIONS_H) */
