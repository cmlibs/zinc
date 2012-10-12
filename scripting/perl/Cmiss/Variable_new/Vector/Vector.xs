#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_variable_new_vector.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Vector		PACKAGE = Cmiss::Variable_new::Vector		PREFIX = Cmiss_variable_new_vector_

PROTOTYPES: DISABLE

Cmiss::Variable_new
new_xs(AV *values_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable_new structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_1=$cmiss_variable_2,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			int i,number_of_values;
			Scalar *values;

			if (values_array&&(0<(number_of_values=av_len(values_array)+1)))
			{
				if (values=(Scalar *)malloc(number_of_values*sizeof(Scalar)))
				{
					for (i=0;i<number_of_values;i++)
					{
						values[i]=(Scalar)SvNV(AvARRAY(values_array)[i]);
					}
					RETVAL=Cmiss_variable_new_vector_create(number_of_values,values);
					free(values);
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_values_xs(Cmiss::Variable_new::Vector variable_vector,AV *indices_array)
	CODE:
		if (variable_vector)
		{
			unsigned int i,*indices,number_of_indices;

			if (indices_array&&(number_of_indices=av_len(indices_array)+1))
			{
				if (indices=(unsigned int *)malloc(number_of_indices*
					sizeof(unsigned int)))
				{
					for (i=0;i<number_of_indices;i++)
					{
						indices[i]=(unsigned int)SvIV(AvARRAY(indices_array)[i]);
					}
					RETVAL=Cmiss_variable_new_input_vector_values(variable_vector,
						number_of_indices,indices);
					free(indices);
				}
			}
			else
			{
				RETVAL=Cmiss_variable_new_input_vector_values(variable_vector,0,
					(unsigned int *)NULL);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
