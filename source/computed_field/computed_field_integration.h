/*******************************************************************************
FILE : computed_field_integration.h

LAST MODIFIED : 26 October 2000

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
#if !defined (COMPUTED_FIELD_INTEGRATION_H)
#define COMPUTED_FIELD_INTEGRATION_H

int Computed_field_is_type_integration(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_integration(struct Computed_field *field,
	struct FE_element *seed_element, struct FE_region *fe_region,
	struct Computed_field *integrand, struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_INTEGRATION.
The seed element is set to the number given and the mapping calculated.
Sets the number of components to the dimension of the given element.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
=============================================================================*/

int Computed_field_get_type_integration(struct Computed_field *field,
	struct FE_element **seed_element, struct Computed_field **integrand,
	struct Computed_field **coordinate_field);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_INTEGRATION, 
the seed element used for the mapping is returned - otherwise an error is reported.
==============================================================================*/

int Computed_field_register_type_integration(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region);
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_INTEGRATION_H) */
