/*******************************************************************************
FILE : computed_field_find_xi_private.hpp

LAST MODIFIED : 13 June 2008

DESCRIPTION :
Data structures and prototype functions needed for all find xi implementations.
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
 * Portions created by the Initial Developer are Copyright (C) 2008
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
#if !defined (COMPUTED_FIELD_FIND_XI_PRIVATE_HPP)
#define COMPUTED_FIELD_FIND_XI_PRIVATE_HPP

class Computed_field_find_element_xi_base_cache
{
public:
	struct FE_element *element;
	int valid_values;
	int number_of_values;
	FE_value *values;
	FE_value *working_values;
	int in_perform_find_element_xi;
	struct Cmiss_region *search_region;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	/* Warn when trying to destroy this cache as it is being filled in */
	
	Computed_field_find_element_xi_base_cache() :
		element((struct FE_element *)NULL),
		valid_values(0),
		number_of_values(0),
		values((FE_value *)NULL),
		working_values((FE_value *)NULL),
		in_perform_find_element_xi(0)
	{
	}
	
	virtual ~Computed_field_find_element_xi_base_cache()
	{
		if (values)
		{
			DEALLOCATE(values);
		}
		if (working_values)
		{
			DEALLOCATE(working_values);
		}
	}

};

struct Computed_field_find_element_xi_cache
/* cache is wrapped in a struct for compatibility with C code */
{
	Computed_field_find_element_xi_base_cache* cache_data;
};

struct Computed_field_find_element_xi_cache
	*CREATE(Computed_field_find_element_xi_cache)(
		Computed_field_find_element_xi_base_cache *cache_data);
/*******************************************************************************
LAST MODIFIED : 13 June 2008

DESCRIPTION :
Stores cache data for find_element_xi routines.
The new object takes ownership of the <cache_data>.
==============================================================================*/

struct Computed_field_iterative_find_element_xi_data
/*******************************************************************************
LAST MODIFIED: 21 August 2002

DESCRIPTION:
Data for passing to Computed_field_iterative_element_conditional
Important note:
The <values> passed in this structure must not be a pointer to values
inside a field cache otherwise they may be overwritten if that field
matches the <field> in this structure or one of its source fields.
==============================================================================*/
{
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *field;
	int number_of_values;
	FE_value *values;
	int element_dimension;
	int found_number_of_xi;
	FE_value *found_values;
	FE_value *found_derivatives;
	float tolerance;
	int find_nearest_location;
	struct FE_element *nearest_element;
	FE_value nearest_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	double nearest_element_distance_squared;
	int start_with_data_xi;
}; /* Computed_field_iterative_find_element_xi_data */

int Computed_field_iterative_element_conditional(
	struct FE_element *element, void *data_void);
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if a valid element xi is found.
Important note:
The <values> passed in the <data> structure must not be a pointer to values
inside a field cache otherwise they may be overwritten if the field is the same
as the <data> field or any of its source fields.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_FIND_XI_PRIVATE_HPP) */
