/*******************************************************************************
FILE : computed_value_finite_element.c

LAST MODIFIED : 25 April 2003

DESCRIPTION :
Implements computed values which interface to finite elements:
- element_xi
==============================================================================*/
#include "computed_variable/computed_value_finite_element.h"
#include "computed_variable/computed_value_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module methods
--------------
*/
static char Cmiss_value_element_xi_type_string[]="Element_xi";
	/*???DB.  End up with same pointer as
		computed_variable_element_xi_type_string? */

struct Cmiss_value_element_xi_type_specific_data
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
???DB.  Is it necessary to add a dimension or insist on non-NULL element?
???DB.  Have an xi number?  Probably better on variable?
==============================================================================*/
{
	FE_value *xi;
	struct FE_element *element;
}; /* struct Cmiss_value_element_xi_type_specific_data */

static START_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	DEALLOCATE(data->xi);
	if (data->element)
	{
		DEACCESS(FE_element)(&(data->element));
	}
	return_code=1;
}
END_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	FE_value *destination_xi,*source_xi;
	int number_of_values;

	if (source->element)
	{
		if (source_xi=source->xi)
		{
			number_of_values=get_FE_element_dimension(source->element);
			if ((0<number_of_values)&&ALLOCATE(destination_xi,FE_value,
				number_of_values))
			{
				destination->xi=destination_xi;
				while (number_of_values>0)
				{
					*destination_xi= *source_xi;
					destination_xi++;
					source_xi++;
					number_of_values--;
				}
				destination->element=ACCESS(FE_element)(source->element);
			}
			else
			{
				DEALLOCATE(destination);
			}
		}
		else
		{
			DEALLOCATE(destination);
		}
	}
	else
	{
		if (source->xi)
		{
			DEALLOCATE(destination);
		}
		else
		{
			destination->xi=(FE_value *)NULL;
			destination->element=(struct FE_element *)NULL;
		}
	}
}
END_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	FE_value *xi_total,*xi_1,*xi_2;
	int number_of_xi;
	struct Cmiss_value_element_xi_type_specific_data *data_total,*data_1,
		*data_2;

	data_1=(struct Cmiss_value_element_xi_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_element_xi_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	data_total=(struct Cmiss_value_element_xi_type_specific_data *)
		Cmiss_value_get_type_specific_data(total);
	ASSERT_IF(data_1&&data_2&&data_total,return_code,0)
	{
		if ((data_1->element)&&(xi_1=data_1->xi)&&(0<(number_of_xi=
			get_FE_element_dimension(data_1->element)))&&(data_2->element)&&
			(xi_2=data_2->xi)&&(number_of_xi==
			get_FE_element_dimension(data_2->element))&&(data_total->element)&&
			(xi_total=data_total->xi)&&(number_of_xi==
			get_FE_element_dimension(data_total->element)))
		{
			while (number_of_xi>0)
			{
				*xi_total += (*xi_1)*(*xi_2);
				xi_total++;
				xi_1++;
				xi_2++;
				number_of_xi++;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_value_element_xi_multiply_and_accumulate_type_specific. "
				"Inconsistent element/xi");
		}
	}
}
END_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	struct Cmiss_value_element_xi_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_value_element_xi_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_element_xi_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	ASSERT_IF(data_1&&data_2,return_code,0)
	{
		if (data_1->element)
		{
			if (data_2->element)
			{
				if (get_FE_element_dimension(data_1->element)==
					get_FE_element_dimension(data_2->element))
				{
					return_code=1;
				}
			}
		}
		else
		{
			if (!(data_2->element))
			{
				return_code=1;
			}
		}
	}
}
END_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(element_xi)

/*
Global functions
----------------
*/
int Cmiss_value_element_xi_set_type(Cmiss_value_id value,
	struct FE_element *element,FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Makes <value> of type element_xi and sets its <element> and <xi).  After
success, the <value> is responsible for DEALLOCATEing <xi>.

???DB.  Assuming that the <element> knows its FE_region (can get manager)
???DB.  Is it necessary to add a dimension or insist on non-NULL element?
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_element_xi_type_specific_data *data;

	ENTER(Cmiss_value_element_xi_set_type);
	return_code=0;
	/* check arguments */
	if (value)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Cmiss_value_element_xi_type_specific_data,1))
		{
			/* 2.  Clear current type-specific data */
			Cmiss_value_clear_type(value);
			/* 3.  Establish the new type */
			Cmiss_value_set_type_specific_information(value,
				Cmiss_value_element_xi_type_string,(void *)data);
			if (element)
			{
				data->element=ACCESS(FE_element)(element);
			}
			else
			{
				data->element=(struct FE_element *)NULL;
			}
			data->xi=xi;
			/* set all the methods */
			return_code=CMISS_VALUE_ESTABLISH_METHODS(value,element_xi);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_element_xi_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_element_xi_set_type.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_element_xi_set_type */

DECLARE_CMISS_VALUE_IS_TYPE_FUNCTION(element_xi)

int Cmiss_value_element_xi_get_type(Cmiss_value_id value,
	struct FE_element **element_address,FE_value **xi_address)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
If <value> is of type element_xi, gets its <*element_address> and <*xi_address).

The calling program must not DEALLOCATE the returned <*xi_address>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_element_xi_type_specific_data *data;

	ENTER(Cmiss_value_element_xi_get_type);
	return_code=0;
	/* check arguments */
	if (value&&(element_address||xi_address))
	{
		data=(struct Cmiss_value_element_xi_type_specific_data *)
			Cmiss_value_get_type_specific_data(value);
		ASSERT_IF(data,return_code,0)
		{
			if (element_address)
			{
				*element_address=data->element;
			}
			if (xi_address)
			{
				*xi_address=data->xi;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_element_xi_get_type.  "
			"Invalid argument(s).  %p %p %p",value,element_address,xi_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_element_xi_get_type */
