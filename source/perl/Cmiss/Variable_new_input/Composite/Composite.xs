#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_new_input_composite.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new_input::Composite  PACKAGE = Cmiss::Variable_new_input::Composite  PREFIX = Cmiss_variable_new_input_composite_

PROTOTYPES: DISABLE

Cmiss::Variable_new_input
new_xs(AV *inputs_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			int i,number_of_inputs;
			Cmiss_variable_new_input_list_id inputs;

			if (inputs_array&&(0<(number_of_inputs=av_len(inputs_array)+1)))
			{
				if (inputs=Cmiss_variable_new_input_list_create())
				{
					i=0;
					while ((i<number_of_inputs)&&inputs)
					{
						if (sv_derived_from(AvARRAY(inputs_array)[i],
							"Cmiss::Variable_new_input"))
						{
							if (Cmiss_variable_new_input_list_add(inputs,
								INT2PTR(Cmiss__Variable_new_input,
								SvIV((SV*)(SvRV(AvARRAY(inputs_array)[i]))))))
							{
								i++;
							}
							else
							{
								Cmiss_variable_new_input_list_destroy(&inputs);
							}
						}
						else
						{
							Cmiss_variable_new_input_list_destroy(&inputs);
						}
					}
					if (inputs)
					{
						RETVAL=Cmiss_variable_new_input_composite_create(inputs);
						Cmiss_variable_new_input_list_destroy(&inputs);
					}
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
