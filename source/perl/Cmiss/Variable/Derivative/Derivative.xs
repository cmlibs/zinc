#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "computed_variable/computed_variable.h"
#include "typemap.h"

MODULE = Cmiss::Variable::Derivative  PACKAGE = Cmiss::Variable::Derivative  PREFIX = Cmiss_variable_derivative_

PROTOTYPES: DISABLE

Cmiss::Variable
create(char *name,Cmiss::Variable dependent_variable,AV *independent_variables_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			int i,order;
			IV tmp_IV;
			Cmiss_variable_id *independent_variables;

			ACCESS(Cmiss_variable)(RETVAL);
			if (dependent_variable&&independent_variables_array&&
				(0<(order=av_len(independent_variables_array)+1)))
			{
				if (independent_variables=(Cmiss_variable_id *)malloc(order*
					sizeof(Cmiss_variable_id)))
				{
					i=0;
					while ((i<order)&&sv_derived_from(
						AvARRAY(independent_variables_array)[i],"Cmiss::Variable"))
					{
						tmp_IV=SvIV((SV*)(SvRV(AvARRAY(independent_variables_array)[i])));
						independent_variables[i]=INT2PTR(Cmiss__Variable,tmp_IV);
						i++;
					}
					if (!((i>=order)&&Cmiss_variable_derivative_set_type(RETVAL,
						dependent_variable,order,independent_variables)))
					{
						free(independent_variables);
						DEACCESS(Cmiss_variable)(&RETVAL);
					}
				}
				else
				{
					DEACCESS(Cmiss_variable)(&RETVAL);
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
