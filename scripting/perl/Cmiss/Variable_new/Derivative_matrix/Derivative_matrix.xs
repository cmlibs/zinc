#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "api/cmiss_variable_new_derivative_matrix.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Derivative_matrix PACKAGE = Cmiss::Variable_new::Derivative_matrix PREFIX = Cmiss_variable_new_Derivative_matrix_

PROTOTYPES: DISABLE

Cmiss::Variable_new
matrix_xs(Cmiss::Variable_new::Derivative_matrix variable_matrix, \
	AV *independent_variables_array)
	CODE:
		{
			int i,number_of_independent_variables;
			Cmiss_variable_new_input_list_id independent_variables;

			if (variable_matrix&&(0<(number_of_independent_variables=av_len(
				independent_variables_array)+1)))
			{
				if (independent_variables=Cmiss_variable_new_input_list_create())
				{
					i=0;
					while ((i<number_of_independent_variables)&&independent_variables)
					{
						if (sv_derived_from(AvARRAY(independent_variables_array)[i],
							"Cmiss::Variable_new_input"))
						{
							if (Cmiss_variable_new_input_list_add(independent_variables,
								INT2PTR(Cmiss__Variable_new_input,
								SvIV((SV*)(SvRV(AvARRAY(independent_variables_array)[i]))))))
							{
								i++;
							}
							else
							{
								Cmiss_variable_new_input_list_destroy(&independent_variables);
							}
						}
						else
						{
							Cmiss_variable_new_input_list_destroy(&independent_variables);
						}
					}
					if (independent_variables)
					{
						RETVAL=Cmiss_variable_new_derivative_matrix_get_matrix(
							variable_matrix,independent_variables);
						Cmiss_variable_new_input_list_destroy(&independent_variables);
					}
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
