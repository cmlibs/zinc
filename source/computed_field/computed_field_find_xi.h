/*******************************************************************************
FILE : computed_field_find_xi.h

LAST MODIFIED : 18 July 2000

DESCRIPTION :
Implements a special version of find_xi that uses OpenGL to accelerate the
lookup of the element.
==============================================================================*/
#if !defined (COMPUTED_FIELD_FIND_XI_H)
#define COMPUTED_FIELD_FIND_XI_H

struct Computed_field_iterative_find_element_xi_data
{
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *field;
	int number_of_values;
	FE_value *values;
	int found_number_of_xi;
	FE_value *found_values;
	FE_value *found_derivatives;
	float tolerance;
}; /* Computed_field_iterative_find_element_xi_data */

struct Computed_field_find_element_xi_special_cache;
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
struct Computed_field_find_element_xi_special_cache is private.
==============================================================================*/

int Computed_field_iterative_element_conditional(
	struct FE_element *element, void *data_void);
/*******************************************************************************
LAST MODIFIED: 16 June 2000

DESCRIPTION:
Returns true if a valid element xi is found.
==============================================================================*/

int Computed_field_find_element_xi_special(struct Computed_field *field, 
	struct Computed_field_find_element_xi_special_cache **cache_ptr, 
	FE_value *values,int number_of_values, struct FE_element **element, 
	FE_value *xi, struct GROUP(FE_element) *search_element_group,
	struct User_interface *user_inteface,
	float *hint_minimums, float *hint_maximums, float *hint_resolution);
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
This function implements the reverse of some certain computed_fields
(Computed_field_is_find_element_xi_capable) so that it tries to find an element
and xi which would evaluate to the given values.
This implementation of find_element_xi has been separated out as it uses OpenGL
to accelerate the element xi lookup.
==============================================================================*/

struct Computed_field_find_element_xi_special_cache 
*CREATE(Computed_field_find_element_xi_special_cache)
	  (void);
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Stores cache data for the Computed_field_find_element_xi_special routine.
==============================================================================*/

int DESTROY(Computed_field_find_element_xi_special_cache)
	  (struct Computed_field_find_element_xi_special_cache **cache_address);
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Frees memory/deaccess cache at <*cache_address>.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_FIND_XI_H) */
