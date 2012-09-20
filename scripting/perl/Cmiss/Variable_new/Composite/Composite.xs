#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_new_composite.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Composite  PACKAGE = Cmiss::Variable_new::Composite  PREFIX = Cmiss_variable_new_composite_

PROTOTYPES: DISABLE

Cmiss::Variable_new
new_xs(AV *variables_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable_new structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			int i,number_of_variables;
			IV tmp_IV;
			Cmiss_variable_new_list_id variables;

			if (variables_array&&(0<(number_of_variables=av_len(variables_array)+1)))
			{
				if (variables=Cmiss_variable_new_list_create())
				{
					i=0;
					while ((i<number_of_variables)&&variables)
					{
						if (sv_derived_from(AvARRAY(variables_array)[i],
							"Cmiss::Variable_new"))
						{
							tmp_IV=SvIV((SV*)(SvRV(AvARRAY(variables_array)[i])));
							if (Cmiss_variable_new_list_add(variables,
								INT2PTR(Cmiss__Variable_new,tmp_IV)))
							{
								i++;
							}
						}
						else
						{
							Cmiss_variable_new_list_destroy(&variables);
							Perl_croak(aTHX_ "Cmiss::Variable_new::Composite::new.  "
								"Not a variable");
						}
					}
				}
				if (variables)
				{
					RETVAL=Cmiss_variable_new_composite_create(variables);
				}
				Cmiss_variable_new_list_destroy(&variables);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
