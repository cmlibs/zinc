#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include "api/cmiss_value.h"
#include "api/cmiss_value_fe_value.h"
#include "typemap.h"

MODULE = Cmiss::Value::FE_value_vector		PACKAGE = Cmiss::Value::FE_value_vector		PREFIX = Cmiss_value_FE_value_vector_

PROTOTYPES: DISABLE

Cmiss::Value
create(fe_values_array)
	AV * fe_values_array
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
	{
		FE_value *fe_value_vector;
		int i,number_of_fe_values;
		if (fe_values_array&&(0<(number_of_fe_values=av_len(fe_values_array)+1)))
		{
			if (fe_value_vector=(FE_value *)malloc(number_of_fe_values*
				sizeof(FE_value)))
			{
				for (i=0;i<number_of_fe_values;i++)
				{
					fe_value_vector[i]=(FE_value)SvNV(AvARRAY(fe_values_array)[i]);
				}
				RETVAL = CREATE(Cmiss_value_FE_value_vector)(number_of_fe_values,
					fe_value_vector);
			}
		}
	}
	if (!RETVAL)
	{
		XSRETURN_UNDEF;
	}
	OUTPUT:
		RETVAL
	
