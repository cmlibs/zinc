/*******************************************************************************
FILE : computed_field_fibres.h

LAST MODIFIED : 18 October 2000

DESCRIPTION :
Computed fields for extracting fibre axes from fibre angles in elements.
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
#if !defined (COMPUTED_FIELD_FIBRES_H)
#define COMPUTED_FIELD_FIBRES_H

int Computed_field_set_type_fibre_axes(struct Computed_field *field,
	struct Computed_field *fibre_field,struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FIBRE_AXES, combining a fibre and
coordinate field to return the 3, 3-component fibre axis vectors:
fibre  = fibre direction,
sheet  = fibre normal in the plane of the sheet,
normal = normal to the fibre sheet.
Sets the number of components to 9.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.

Both the fibre and coordinate fields must have no more than 3 components. The
fibre field is expected to have a FIBRE coordinate_system, although this is not
enforced.
???RC To enforce the fibre field to have a FIBRE coordinate_system, must make
the MANAGER_COPY_NOT_IDENTIFIER fail if it would change the coordinate_system
while the field is in use. Not sure if we want that restriction.
==============================================================================*/

int Computed_field_register_types_fibres(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_FIBRES_H) */
