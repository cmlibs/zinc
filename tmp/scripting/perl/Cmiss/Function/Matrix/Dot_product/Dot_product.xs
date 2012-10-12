#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_function_matrix_dot_product.h"
#include "typemap.h"

MODULE = Cmiss::Function::Matrix::Dot_product		PACKAGE = Cmiss::Function::Matrix::Dot_product		PREFIX = Cmiss_function_matrix_dot_product_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(AV *variables_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			Cmiss_function_variable_id variable_1,variable_2;

			if (variables_array&&(2==av_len(variables_array)+1))
			{
				if (sv_derived_from(AvARRAY(variables_array)[0],
					"Cmiss::Function_variable")&&(variable_1=INT2PTR(
					Cmiss__Function_variable,SvIV((SV*)(SvRV(AvARRAY(variables_array)[
					0]))))))
				{
					if (sv_derived_from(AvARRAY(variables_array)[1],
						"Cmiss::Function_variable")&&(variable_2=INT2PTR(
						Cmiss__Function_variable,SvIV((SV*)(SvRV(AvARRAY(variables_array)[
						1]))))))
					{
						RETVAL=Cmiss_function_matrix_dot_product_create(variable_1,
							variable_2);
					}
					else
					{
						Perl_croak(aTHX_ "Cmiss::Function::Matrix::Dot_product::new.  "
							"Invalid variable_2");
					}
				}
				else
				{
					Perl_croak(aTHX_ "Cmiss::Function::Matrix::Dot_product::new.  "
						"Invalid variable_1");
				}
			}
			else
			{
				Perl_croak(aTHX_ "Cmiss::Function::Matrix::Dot_product::new.  Binary only");
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
