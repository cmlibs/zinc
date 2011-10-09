#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_function_matrix_divide_by_scalar.h"
#include "typemap.h"

MODULE = Cmiss::Function::Matrix::Divide_by_scalar		PACKAGE = Cmiss::Function::Matrix::Divide_by_scalar		PREFIX = Cmiss_function_matrix_divide_by_scalar_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(AV *dividands_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			Cmiss_function_variable_id dividend,divisor;

			if (dividands_array&&(2==av_len(dividands_array)+1))
			{
				if (sv_derived_from(AvARRAY(dividands_array)[0],
					"Cmiss::Function_variable")&&(dividend=INT2PTR(
					Cmiss__Function_variable,SvIV((SV*)(SvRV(AvARRAY(dividands_array)[
					0]))))))
				{
					if (sv_derived_from(AvARRAY(dividands_array)[1],
						"Cmiss::Function_variable")&&(divisor=INT2PTR(
						Cmiss__Function_variable,SvIV((SV*)(SvRV(AvARRAY(dividands_array)[
						1]))))))
					{
						RETVAL=Cmiss_function_matrix_divide_by_scalar_create(dividend,
							divisor);
					}
					else
					{
						Perl_croak(aTHX_ "Cmiss::Function::Matrix::Divide_by_scalar::new.  "
							"Invalid divisor");
					}
				}
				else
				{
					Perl_croak(aTHX_ "Cmiss::Function::Matrix::Divide_by_scalar::new.  "
						"Invalid dividend");
				}
			}
			else
			{
				Perl_croak(aTHX_ "Cmiss::Function::Matrix::Divide_by_scalar::new.  Binary only");
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
