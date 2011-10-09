#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_new_derivative.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Derivative  PACKAGE = Cmiss::Variable_new::Derivative  PREFIX = Cmiss_variable_new_derivative_

PROTOTYPES: DISABLE

Cmiss::Variable_new
new_xs(Cmiss::Variable_new dependent_variable,AV *independent_variables_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			int i,order;
			IV tmp_IV;
			Cmiss_variable_new_input_list_id independent_variables;

			if (dependent_variable&&independent_variables_array&&
				(0<(order=av_len(independent_variables_array)+1)))
			{
				if (independent_variables=Cmiss_variable_new_input_list_create())
				{
					i=0;
					while ((i<order)&&independent_variables)
					{
						if (sv_derived_from(AvARRAY(independent_variables_array)[i],
							"Cmiss::Variable_new_input"))
						{
							tmp_IV=SvIV((SV*)(SvRV(AvARRAY(independent_variables_array)[i])));
							if (Cmiss_variable_new_input_list_add(independent_variables,
								INT2PTR(Cmiss__Variable_new_input,tmp_IV)))
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
							Perl_croak(aTHX_ "Cmiss::Variable_new::Derivative::new.  "
								"Independent variable is not an input");
						}
					}
					if (independent_variables)
					{
						RETVAL=Cmiss_variable_new_derivative_create(dependent_variable,
							independent_variables);
					}
					Cmiss_variable_new_input_list_destroy(&independent_variables);
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
