#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "api/cmiss_function.h"
#include "api/cmiss_function_composite.h"
#include "typemap.h"

MODULE = Cmiss::Function::Composite PACKAGE = Cmiss::Function::Composite PREFIX = Cmiss_function_Composite_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(AV *functions_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_function structure).  This means that don't need to worry
			about ACCESSing for Perl assignment/copy,
			$cmiss_function_2=$cmiss_function_1, because this increments the reference
			count for the stash (DESTROY is called when the stash reference count gets
			to zero) */
		{
			int i,number_of_functions;
			IV tmp_IV;
			Cmiss_function_list_id functions;

			if (functions_array&&(0<(number_of_functions=av_len(functions_array)+1)))
			{
				if (functions=Cmiss_function_list_create())
				{
					i=0;
					while ((i<number_of_functions)&&functions)
					{
						if (sv_derived_from(AvARRAY(functions_array)[i],
							"Cmiss::Function"))
						{
							tmp_IV=SvIV((SV*)(SvRV(AvARRAY(functions_array)[i])));
							if (Cmiss_function_list_add(functions,
								INT2PTR(Cmiss__Function,tmp_IV)))
							{
								i++;
							}
						}
						else
						{
							Cmiss_function_list_destroy(&functions);
							Perl_croak(aTHX_ "Cmiss::Function::Composite::new.  "
								"Not a function");
						}
					}
				}
				if (functions)
				{
					RETVAL=Cmiss_function_composite_create(functions);
				}
				Cmiss_function_list_destroy(&functions);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
