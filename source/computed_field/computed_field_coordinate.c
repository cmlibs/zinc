/*******************************************************************************
FILE : computed_field_find_xi.c

LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
#include <stdio.h>
#include <math.h>

#include "general/debug.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_finite_element.h"
#include "user_interface/message.h"

int Computed_field_evaluate_rc_coordinate(struct Computed_field *field,
	int element_dimension,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 25 June 1999

DESCRIPTION :
Function called by Computed_field_evaluate_cache_in_element/at_node to compute
rectangular cartesian coordinates from the source_field values in an arbitrary
coordinate system.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives if
calculate_derivatives set) for the same element, with the given
<element_dimension> = number of Xi coords.
Note: both COMPUTED_FIELD_DEFAULT_COORDINATE and COMPUTED_FIELD_RC_COORDINATE
are computed with this function.
==============================================================================*/
{
	FE_value *destination,*dx_dxi,temp[9],x[3];
	int i,j,return_code;
	
	ENTER(Computed_field_evaluate_rc_coordinate);
	if (field&&(Computed_field_is_type_default_coordinate(field)||
		(COMPUTED_FIELD_RC_COORDINATE==field->type)))
	{
		if (calculate_derivatives)
		{
			dx_dxi=temp;
		}
		else
		{
			dx_dxi=(FE_value *)NULL;
		}
		if (return_code=Computed_field_extract_rc(field->source_fields[0],
			element_dimension,x,dx_dxi))
		{
			/*???RC works because number_of_components is always 3 */
			for (i=0;i<3;i++)
			{
				field->values[i]=x[i];
			}
			if (calculate_derivatives)
			{
				destination=field->derivatives;
				for (i=0;i<3;i++)
				{
					for (j=0;j<element_dimension;j++)
					{
						*destination=dx_dxi[3*i+j];
						destination++;
					}
				}
				field->derivatives_valid = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_rc_coordinate.  Could not convert to RC");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_rc_coordinate.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_rc_coordinate */




