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

/*******************************************************************************
 * Creates a field which returns the sum of a source field's value over all
 * nodes in its region. Returned field has same number of components as source.
 * GRC: rename this field before exposing in external API
 * 
 * @param field_factory  Specifies owning region and other generic arguments.
 * @param source_field  Field to sum.
 * @param group  Optional sub-group of region to limit sum over.  
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_region_sum(
	struct Cmiss_field_factory *field_factory,
	struct Computed_field *source_field, struct Cmiss_region *group);

/*******************************************************************************
 * Creates a field which returns the mean of a source field's values over all
 * nodes in its region. Returned field has same number of components as source.
 * GRC: rename this field before exposing in external API
 * 
 * @param field_factory  Specifies owning region and other generic arguments.
 * @param source_field  Field to sum & average.
 * @param group  Optional sub-group of region to limit sum over.  
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_region_mean(
	struct Cmiss_field_factory *field_factory,
	struct Computed_field *source_field, struct Cmiss_region *group);

#endif /* !defined (COMPUTED_FIELD_REGION_OPERATIONS_H) */
