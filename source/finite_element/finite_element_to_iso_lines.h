/*******************************************************************************
FILE : finite_element_to_iso_lines.h

LAST MODIFIED : 28 January 2000

DESCRIPTION :
Functions for computing, sorting and storing polylines of constant scalar field
value over 2-D elements.
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
#if !defined (FINITE_ELEMENT_TO_ISO_LINES_H)
#define FINITE_ELEMENT_TO_ISO_LINES_H

#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_object.h"

int create_iso_lines_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,
	struct Computed_field *scalar_field, FE_value iso_value, FE_value time,
	struct Computed_field *data_field,int number_of_segments_in_xi1_requested,
	int number_of_segments_in_xi2_requested,struct FE_element *top_level_element,
	struct GT_object *graphics_object,FE_value graphics_object_time);
/*******************************************************************************
LAST MODIFIED : 7 February 2002

DESCRIPTION :
Fills <graphics_object> (of type g_POLYLINE) with polyline contours of
<iso_scalar_field> at <iso_value>.
==============================================================================*/

#endif /* !defined (FINITE_ELEMENT_TO_ISO_LINES_H) */
