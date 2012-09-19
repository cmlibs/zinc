#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_new_composition.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Composition  PACKAGE = Cmiss::Variable_new::Composition  PREFIX = Cmiss_variable_new_composition_

PROTOTYPES: DISABLE

Cmiss::Variable_new
new_xs(Cmiss::Variable_new dependent_variable,AV *input_source_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable_new structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			int i,number_of_source_variables;
			IV tmp_IV;
			Cmiss_variable_new_input_value_list_id input_source_list;

			if (dependent_variable&&input_source_array&&
				(0<(number_of_source_variables=av_len(input_source_array)+1))&&
				(0==number_of_source_variables%2))
			{
				number_of_source_variables /= 2;
				if (input_source_list=Cmiss_variable_new_input_value_list_create())
				{
					i=0;
					while ((i<number_of_source_variables)&&input_source_list)
					{
						if (sv_derived_from(AvARRAY(input_source_array)[2*i],
							"Cmiss::Variable_new_input")&&sv_derived_from(
							AvARRAY(input_source_array)[2*i+1],"Cmiss::Variable_new"))
						{
							if (Cmiss_variable_new_input_value_list_add(input_source_list,
								INT2PTR(Cmiss__Variable_new_input,SvIV((SV *)SvRV(AvARRAY(
								input_source_array)[2*i]))),
								INT2PTR(Cmiss__Variable_new,SvIV((SV *)SvRV(AvARRAY(
								input_source_array)[2*i+1])))))
							{
								i++;
							}
							else
							{
								Cmiss_variable_new_input_value_list_destroy(&input_source_list);
							}
						}
						else
						{
							Cmiss_variable_new_input_value_list_destroy(&input_source_list);
						}
					}
					if (input_source_list)
					{
						RETVAL=Cmiss_variable_new_composition_create(dependent_variable,
							input_source_list);
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
