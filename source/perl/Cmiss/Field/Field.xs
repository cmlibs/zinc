#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdio.h>
#include "api/cmiss_field.h"
#include "general/io_stream.h"
#include "typemap.h"

MODULE = Cmiss::Field		PACKAGE = Cmiss::Field		PREFIX = Cmiss_field_

PROTOTYPES: DISABLE

Cmiss::Field
Cmiss_field_create(Cmiss::Region region, Cmiss::Field_type_object type)

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

int
Cmiss_field_get_number_of_components(Cmiss::Field field)

void
Cmiss_field_evaluate_at_node(Cmiss::Field field, Cmiss::Node node, float time)
	PPCODE:
		int i, number_of_values;
		float *values;

		number_of_values = Cmiss_field_get_number_of_components(field);
		values = malloc(sizeof(float) * number_of_values);
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

NO_OUTPUT int
Cmiss_field_get_name(IN Cmiss::Field field, \
	OUTLIST char *name)
   POSTCALL:
	if (RETVAL == 0)
			XSRETURN_UNDEF;

int
Cmiss_field_set_name(IN Cmiss::Field field, \
	char *name)

int
Cmiss_field_set_type(Cmiss::Field field, \
   Cmiss::Field_type_object field_type)

Cmiss::Field_type_object
Cmiss_field_type_create_add( \
   Cmiss::Field source_field_one, Cmiss::Field source_field_two)

