struct Computed_field_find_element_xi_special_cache;

int Computed_field_find_element_xi_special(struct Computed_field *field, 
	struct Computed_field_find_element_xi_special_cache **cache_ptr, FE_value *values,
	int number_of_values, struct FE_element **element, FE_value *xi,
	struct GROUP(FE_element) *search_element_group,
	struct User_interface *user_inteface,
	struct Computed_field_iterative_find_element_xi_data *find_element_xi_data,
	float *hint_minimums, float *hint_maximums, float *hint_resolution);
/*******************************************************************************
LAST MODIFIED : 20 June 2000

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
	  (struct Computed_field_find_element_xi_special_cache **mapping_address);
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Frees memory/deaccess cache at <*cache_address>.
==============================================================================*/
