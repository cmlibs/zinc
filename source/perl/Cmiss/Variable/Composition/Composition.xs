#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "computed_variable/computed_variable_composition.h"
#include "typemap.h"

MODULE = Cmiss::Variable::Composition  PACKAGE = Cmiss::Variable::Composition  PREFIX = Cmiss_variable_composition_

PROTOTYPES: DISABLE

Cmiss::Variable
create(char *name,Cmiss::Variable dependent_variable, \
	AV *independent_source_variables_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,
			name))
		{
			int i,j,number_of_source_variables;
			IV tmp_IV;
			Cmiss_variable_id *independent_variables,*source_variables;

			ACCESS(Cmiss_variable)(RETVAL);
			if (dependent_variable&&independent_source_variables_array&&
				(0<(number_of_source_variables=av_len(
				independent_source_variables_array)+1))&&
				(0==number_of_source_variables%2))
			{
				number_of_source_variables /= 2;
				independent_variables=(Cmiss_variable_id *)malloc(
					number_of_source_variables*sizeof(Cmiss_variable_id));
				source_variables=(Cmiss_variable_id *)malloc(
					number_of_source_variables*sizeof(Cmiss_variable_id));
				if (independent_variables&&source_variables)
				{
					i=0;
					while ((i<number_of_source_variables)&&sv_derived_from(
						AvARRAY(independent_source_variables_array)[2*i],
						"Cmiss::Variable")&&sv_derived_from(
						AvARRAY(independent_source_variables_array)[2*i+1],
						"Cmiss::Variable"))
					{
						tmp_IV=SvIV((SV*)(SvRV(AvARRAY(independent_source_variables_array)[
							2*i])));
						independent_variables[i]=INT2PTR(Cmiss__Variable,tmp_IV);
						tmp_IV=SvIV((SV*)(SvRV(AvARRAY(independent_source_variables_array)[
							2*i+1])));
						source_variables[i]=INT2PTR(Cmiss__Variable,tmp_IV);
						i++;
					}
					if (!((i>=number_of_source_variables)&&
						Cmiss_variable_composition_set_type(RETVAL,dependent_variable,
						number_of_source_variables,source_variables,independent_variables)))
					{
						free(independent_variables);
						free(source_variables);
						DEACCESS(Cmiss_variable)(&RETVAL);
					}
				}
				else
				{
					DEACCESS(Cmiss_variable)(&RETVAL);
					if (independent_variables)
					{
						free(independent_variables);
					}
					if (source_variables)
					{
						free(source_variables);
					}
				}
			}
			else
			{
				DEACCESS(Cmiss_variable)(&RETVAL);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
