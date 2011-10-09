#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_function_derivative.h"
#include "typemap.h"

MODULE = Cmiss::Function::Derivative  PACKAGE = Cmiss::Function::Derivative  PREFIX = Cmiss_function_derivative_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(Cmiss::Function_variable dependent_variable, \
	AV *independent_variables_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_function structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_function_2=$cmiss_function_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			int i,order;
			IV tmp_IV;
			Cmiss_function_variable_list_id independent_variables;

			if (dependent_variable&&independent_variables_array&&
				(0<(order=av_len(independent_variables_array)+1)))
			{
				if (independent_variables=Cmiss_function_variable_list_create())
				{
					i=0;
					while ((i<order)&&independent_variables)
					{
						if (sv_derived_from(AvARRAY(independent_variables_array)[i],
							"Cmiss::Function_variable"))
						{
							tmp_IV=SvIV((SV*)(SvRV(AvARRAY(independent_variables_array)[i])));
							if (Cmiss_function_variable_list_add(independent_variables,
								INT2PTR(Cmiss__Function_variable,tmp_IV)))
							{
								i++;
							}
							else
							{
								Cmiss_function_variable_list_destroy(&independent_variables);
							}
						}
						else
						{
							Cmiss_function_variable_list_destroy(&independent_variables);
							Perl_croak(aTHX_ "Cmiss::Function::Derivative::new.  "
								"Independent variable is not an variable");
						}
					}
					if (independent_variables)
					{
						RETVAL=Cmiss_function_derivative_create(dependent_variable,
							independent_variables);
					}
					Cmiss_function_variable_list_destroy(&independent_variables);
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
