#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include "computed_variable/computed_value.h"
#include "typemap.h"

MODULE = Cmiss::Value::FE_value_vector		PACKAGE = Cmiss::Value::FE_value_vector		PREFIX = Cmiss_value_FE_value_vector_

PROTOTYPES: DISABLE

Cmiss::Value
create(fe_values_array)
	AV * fe_values_array
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_value structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_value_2=$cmiss_value_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=CREATE(Cmiss_value)())
		{
			FE_value *fe_value_vector;
			int i,number_of_fe_values;

			ACCESS(Cmiss_value)(RETVAL);
			if (fe_values_array&&(0<(number_of_fe_values=av_len(fe_values_array)+1)))
			{
				if (fe_value_vector=(FE_value *)malloc(number_of_fe_values*
					sizeof(FE_value)))
				{
					for (i=0;i<number_of_fe_values;i++)
					{
						fe_value_vector[i]=(FE_value)SvNV(AvARRAY(fe_values_array)[i]);
					}
					if (!Cmiss_value_FE_value_vector_set_type(RETVAL,number_of_fe_values,
						fe_value_vector))
					{
						DEACCESS(Cmiss_value)(&RETVAL);
					}
				}
				else
				{
					DEACCESS(Cmiss_value)(&RETVAL);
				}
			}
			else
			{
				if (!Cmiss_value_FE_value_vector_set_type(RETVAL,0,(FE_value *)NULL))
				{
					DEACCESS(Cmiss_value)(&RETVAL);
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
	
SV *
get_type(Cmiss::Value cmiss_value)
	CODE:
		if (RETVAL=newSVpv("[",0))
		{
			int i,number_of_fe_values;
			FE_value *fe_value_vector;

			if (Cmiss_value_FE_value_vector_get_type(cmiss_value,
				&number_of_fe_values,&fe_value_vector))
			{
				if (0<number_of_fe_values)
				{
					sv_catpvf(RETVAL,"%g",fe_value_vector[0]);
					if (number_of_fe_values<8)
					{
						for (i=1;i<number_of_fe_values;i++)
						{
							sv_catpvf(RETVAL,",%g",fe_value_vector[i]);
						}
					}
					else
					{
						for (i=1;i<3;i++)
						{
							sv_catpvf(RETVAL,",%g",fe_value_vector[i]);
						}
						sv_catpv(RETVAL,",...");
						for (i=number_of_fe_values-3;i<number_of_fe_values;i++)
						{
							sv_catpvf(RETVAL,",%g",fe_value_vector[i]);
						}
					}
				}
				sv_catpv(RETVAL,"]");
			}
			else
			{
				sv_free(RETVAL);
				RETVAL=0;
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
