#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_function_matrix_sum.h"
#include "typemap.h"

MODULE = Cmiss::Function::Matrix::Sum		PACKAGE = Cmiss::Function::Matrix::Sum		PREFIX = Cmiss_function_matrix_sum_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(AV *summands_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			Cmiss_function_variable_id summand_1,summand_2;

			if (summands_array&&(2==av_len(summands_array)+1))
			{
				if (sv_derived_from(AvARRAY(summands_array)[0],
					"Cmiss::Function_variable")&&(summand_1=INT2PTR(
					Cmiss__Function_variable,SvIV((SV*)(SvRV(AvARRAY(summands_array)[
					0]))))))
				{
					if (sv_derived_from(AvARRAY(summands_array)[1],
						"Cmiss::Function_variable")&&(summand_2=INT2PTR(
						Cmiss__Function_variable,SvIV((SV*)(SvRV(AvARRAY(summands_array)[
						1]))))))
					{
						RETVAL=Cmiss_function_matrix_sum_create(summand_1,summand_2);
					}
					else
					{
						Perl_croak(aTHX_ "Cmiss::Function::Matrix::Sum::new.  "
							"Invalid second summand");
					}
				}
				else
				{
					Perl_croak(aTHX_ "Cmiss::Function::Matrix::Sum::new.  "
						"Invalid first summand");
				}
			}
			else
			{
				Perl_croak(aTHX_ "Cmiss::Function::Matrix::Sum::new.  Binary only");
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
