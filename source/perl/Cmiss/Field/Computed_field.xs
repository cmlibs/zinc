#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdio.h>
#include "api/cmiss_computed_field.h"
#include "general/io_stream.h"
#include "typemap.h"

MODULE = Cmiss::Computed_field		PACKAGE = Cmiss::Computed_field		PREFIX = Cmiss_computed_field_

PROTOTYPES: DISABLE

int
get_number_of_components(Cmiss::Computed_field field)
	CODE:
		RETVAL=0;
		if (field)
		{
			RETVAL=Cmiss_computed_field_get_number_of_components(field);
		}
	OUTPUT:
		RETVAL

void
evaluate_at_node(Cmiss::Computed_field field, Cmiss::Node node, float time)
	PPCODE:
		int i, number_of_values;
		float *values;

		number_of_values = Cmiss_computed_field_get_number_of_components(field);
		values = malloc(sizeof(float) * number_of_values);
		if (Cmiss_computed_field_evaluate_at_node(field,
			node, time, number_of_values, values))
		{
			EXTEND(SP, number_of_values);
			for (i = 0 ; i < number_of_values ; i++)
			{
				PUSHs(sv_2mortal(newSVnv(values[i])));
			}
			free(values);
		}
