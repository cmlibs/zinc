/*******************************************************************************
FILE : finite_element_conversion.h

LAST MODIFIED : 4 March 2003

DESCRIPTION :
Functions for building IGES information from finite elements and exporting
to file.
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
#if !defined (FINITE_ELEMENT_CONVERSION_H)
#define FINITE_ELEMENT_CONVERSION_H

#include "general/enumerator.h"

/*
Global types
------------
*/

enum Convert_finite_elements_mode
/******************************************************************************
LAST MODIFIED : 7 December 2005

DESCRIPTION :
Renders the visible objects as finite elements into the specified <fe_region>.
==============================================================================*/
{
	/* Convert the surface elements to topologically square cubic hermite 2D elements. */
	CONVERT_TO_FINITE_ELEMENTS_HERMITE_2D_PRODUCT
};

PROTOTYPE_ENUMERATOR_FUNCTIONS(Convert_finite_elements_mode);

/*
Global functions
----------------
*/

int finite_element_conversion(struct FE_region *source_fe_region, 
	struct FE_region *destination_fe_region,
	enum Convert_finite_elements_mode mode, int number_of_fields, 
	struct Computed_field **field_array);
/******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Convert the finite_elements in <source_fe_region> to new finite_elements
in <destination_fe_region> according to the <mode> defining the fields
in <field_array>.
==============================================================================*/
#endif /* !defined (FINITE_ELEMENT_CONVERSION_H) */
