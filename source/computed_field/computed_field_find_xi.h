/*******************************************************************************
FILE : computed_field_find_xi.h

LAST MODIFIED : 21 August 2002

DESCRIPTION :
Implements a special version of find_xi that uses OpenGL to accelerate the
lookup of the element.
==============================================================================*/
#if !defined (COMPUTED_FIELD_FIND_XI_H)
#define COMPUTED_FIELD_FIND_XI_H

#include "user_interface/user_interface.h"

struct Computed_field_find_element_xi_cache;
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
struct Computed_field_find_element_xi_cache is private.
==============================================================================*/

int Computed_field_perform_find_element_xi(struct Computed_field *field,
	FE_value *values, int number_of_values, struct FE_element **element, 
	FE_value *xi, struct GROUP(FE_element) *search_element_group);
/*******************************************************************************
LAST MODIFIED : 17 December 2002

DESCRIPTION :
This function actually seacrches through the elements in the 
<search_element_group> trying to find an <xi> location which returns the correct
<values>.  This routine is either called directly by Computed_field_find_element_xi
or if that field is propogating it's values backwards, it is called by the 
ultimate parent finite_element field.
==============================================================================*/

int Computed_field_find_element_xi_special(struct Computed_field *field, 
	struct Computed_field_find_element_xi_cache **cache_ptr, 
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
