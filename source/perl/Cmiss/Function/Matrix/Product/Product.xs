#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include <string.h>
#include "api/cmiss_function_matrix_product.h"
#include "typemap.h"

MODULE = Cmiss::Function::Matrix::Product		PACKAGE = Cmiss::Function::Matrix::Product		PREFIX = Cmiss_function_matrix_product_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(AV *prodands_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			Cmiss_function_variable_id multiplicand,multiplier;

			if (prodands_array&&(2==av_len(prodands_array)+1))
			{
				if (sv_derived_from(AvARRAY(prodands_array)[0],
					"Cmiss::Function_variable")&&(multiplier=INT2PTR(
					Cmiss__Function_variable,SvIV((SV*)(SvRV(AvARRAY(prodands_array)[
					0]))))))
				{
					if (sv_derived_from(AvARRAY(prodands_array)[1],
						"Cmiss::Function_variable")&&(multiplicand=INT2PTR(
						Cmiss__Function_variable,SvIV((SV*)(SvRV(AvARRAY(prodands_array)[
						1]))))))
					{
						RETVAL=Cmiss_function_matrix_product_create(multiplier,
							multiplicand);
					}
					else
					{
						Perl_croak(aTHX_ "Cmiss::Function::Matrix::Product::new.  "
							"Invalid multiplicand");
					}
				}
				else
				{
					Perl_croak(aTHX_ "Cmiss::Function::Matrix::Product::new.  "
						"Invalid first multiplier");
				}
			}
			else
			{
				Perl_croak(aTHX_ "Cmiss::Function::Matrix::Product::new.  Binary only");
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
