/*******************************************************************************
FILE : computed_field_find_xi.h

LAST MODIFIED : 18 April 2005

DESCRIPTION :
Implements a special version of find_xi that uses OpenGL to accelerate the
lookup of the element.
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
#if !defined (COMPUTED_FIELD_FIND_XI_H)
#define COMPUTED_FIELD_FIND_XI_H

#include "region/cmiss_region.h"

struct Graphics_buffer_package;

struct Computed_field_find_element_xi_cache;
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
struct Computed_field_find_element_xi_cache is private.
==============================================================================*/

int Computed_field_perform_find_element_xi(struct Computed_field *field,
	FE_value *values, int number_of_values, struct FE_element **element, 
	FE_value *xi, int element_dimension, struct Cmiss_region *search_region,
	int find_nearest_location);
/*******************************************************************************
LAST MODIFIED : 18 April 2005

DESCRIPTION :
This function actually seacrches through the elements in the 
<search_region> trying to find an <xi> location which returns the correct
<values>.  This routine is either called directly by Computed_field_find_element_xi
or if that field is propogating it's values backwards, it is called by the 
ultimate parent finite_element field.
An <element_dimension> of 0 searches in elements of all dimension, any other
value searches just elements of that dimension.
==============================================================================*/

int Computed_field_find_element_xi_special(struct Computed_field *field, 
	struct Computed_field_find_element_xi_cache **cache_ptr,
	FE_value *values, int number_of_values, struct FE_element **element, 
	FE_value *xi, struct Cmiss_region *search_region,
	int element_dimension,
	struct Graphics_buffer_package *graphics_buffer_package,
	float *hint_minimums, float *hint_maximums, float *hint_resolution);
/*******************************************************************************
LAST MODIFIED : 12 May 2004

DESCRIPTION :
This function implements the reverse of some certain computed_fields
(Computed_field_is_find_element_xi_capable) so that it tries to find an element
and xi which would evaluate to the given values.
This implementation of find_element_xi has been separated out as it uses OpenGL
to accelerate the element xi lookup.
The <graphics_buffer_package> is required to connect to the OpenGL implementation.
The <find_element_xi_data> is passed in just to avoid reimplementing the code
from Computed_field_find_element_xi.
<hint_minimums> and <hint_maximums> are used to indicate the range over which
the values supplied will vary and <hint_resolution> indicates the resolution
at which values will be sampled for element_xi, as this algorithm will generate
an element lookup image using these parameters.
An <element_dimension> of 0 searches in elements of all dimension, any other
value searches just elements of that dimension.
The return code indicates if the algorithm should be relied on or whether a
sequential element_xi lookup should now be performed.
==============================================================================*/

struct Computed_field_find_element_xi_cache 
*CREATE(Computed_field_find_element_xi_cache)
	  (void);
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Stores cache data for the Computed_field_find_element_xi routine.
==============================================================================*/

int DESTROY(Computed_field_find_element_xi_cache)
	  (struct Computed_field_find_element_xi_cache **cache_address);
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Frees memory/deaccess cache at <*cache_address>.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_FIND_XI_H) */
