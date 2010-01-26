#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdio.h>
#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_field_arithmetic_operators.h"
#include "api/cmiss_field_composite.h"
#include "api/cmiss_field_conditional.h"
#include "api/cmiss_field_logical_operators.h"
#include "general/io_stream.h"
#include "typemap.h"

MODULE = Cmiss::Field		PACKAGE = Cmiss::Field		PREFIX = Cmiss_field_

PROTOTYPES: DISABLE

int
DESTROY(Cmiss::Field field)
	CODE:
		{
			struct Cmiss_field *temp_field;

			temp_field=field;
			RETVAL=Cmiss_field_destroy(&temp_field);
		}
	OUTPUT:
		RETVAL

Cmiss::Field_module
Cmiss_field_get_field_module(Cmiss::Field field)

int
Cmiss_field_get_number_of_components(Cmiss::Field field)

void
Cmiss_field_evaluate_at_node(Cmiss::Field field, Cmiss::Node node, float time)
	PPCODE:
	{
		int i, number_of_values;
		double *values;

		number_of_values = Cmiss_field_get_number_of_components(field);
		values = malloc(sizeof(double) * number_of_values);
		if (Cmiss_field_evaluate_at_node(field,
			node, time, number_of_values, values))
		{
			EXTEND(SP, number_of_values);
			for (i = 0 ; i < number_of_values ; i++)
			{
				PUSHs(sv_2mortal(newSVnv(values[i])));
			}
			free(values);
		}
	}

NO_OUTPUT int
Cmiss_field_get_name(IN Cmiss::Field field, \
	OUTLIST char *name)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_field_set_name(IN Cmiss::Field field, \
	char *name)


MODULE = Cmiss::Field		PACKAGE = Cmiss::Field_module		PREFIX = Cmiss_field_module_

int
DESTROY(Cmiss::Field_module field_module)
	CODE:
		{
			struct Cmiss_field_module *temp_field_module;

			temp_field_module=field_module;
			RETVAL=Cmiss_field_module_destroy(&temp_field_module);
		}
	OUTPUT:
		RETVAL

Cmiss::Field
Cmiss_field_module_find_field_by_name(Cmiss::Field_module field_module, char *name)
   POSTCALL:
	if (!RETVAL)
		XSRETURN_UNDEF;

Cmiss::Field
Cmiss_field_module_create_multiply(Cmiss::Field_module field_module, \
   Cmiss::Field source_field_one, Cmiss::Field source_field_two)

Cmiss::Field
Cmiss_field_module_create_add(Cmiss::Field_module field_module, \
   Cmiss::Field source_field_one, Cmiss::Field source_field_two)

Cmiss::Field
Cmiss_field_module_create_subtract(Cmiss::Field_module field_module, \
   Cmiss::Field source_field_one, Cmiss::Field source_field_two)

Cmiss::Field
Cmiss_field_module_create_divide(Cmiss::Field_module field_module, \
   Cmiss::Field source_field_one, Cmiss::Field source_field_two)

Cmiss::Field
Cmiss_field_module_create_sqrt(Cmiss::Field_module field_module, \
	Cmiss::Field source_field)

Cmiss::Field
Cmiss_field_module_create_log(Cmiss::Field_module field_module, \
	Cmiss::Field source_field)

Cmiss::Field
Cmiss_field_module_create_exp(Cmiss::Field_module field_module, \
	Cmiss::Field source_field)

Cmiss::Field
Cmiss_field_module_create_less_than(Cmiss::Field_module field_module, \
   Cmiss::Field source_field_one, Cmiss::Field source_field_two)

Cmiss::Field
Cmiss_field_module_create_greater_than(Cmiss::Field_module field_module, \
   Cmiss::Field source_field_one, Cmiss::Field source_field_two)

Cmiss::Field
Cmiss_field_module_create_if(Cmiss::Field_module field_module, \
   Cmiss::Field source_field_one, Cmiss::Field source_field_two, \
	Cmiss::Field source_field_three)

Cmiss::Field
Cmiss_field_module_create_constant(Cmiss::Field_module field_module, \
	SV *source_value)
	CODE:
	{
		AV *values_array;
		double double_value, *value, *values;
		int j, number_of_values;

		RETVAL = (struct Cmiss_field *)NULL;
    if (SvROK(source_value) && SvTYPE(SvRV(source_value))==SVt_PVAV)
    {
			values_array = (AV*)SvRV(ST(1));
			number_of_values = 1 + av_len(values_array);
			if (values=(double *)malloc(number_of_values*sizeof(double)))
			{
				value=values;
				for (j=0;j<number_of_values;j++)
				{
					*value=(double)SvNV(AvARRAY(values_array)[j]);
					value++;
				}
				RETVAL = Cmiss_field_module_create_constant(field_module, number_of_values,
					values);
				free(values);
			}
    }
    else if (!SvROK(source_value))
    {
 			double_value = (double)SvNV(source_value);
			RETVAL = Cmiss_field_module_create_constant(field_module, /*number_of_values*/1,
				&double_value);
		}
		else
 			Perl_croak(aTHX_ "Cmiss::Field_module::create_constant source value must be an array reference or a scalar");

		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	}
	OUTPUT:
		RETVAL
