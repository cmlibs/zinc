#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include "computed_variable/computed_value_finite_element.h"
#include "typemap.h"

MODULE = Cmiss::Value::Element_xi		PACKAGE = Cmiss::Value::Element_xi		PREFIX = Cmiss_value_element_xi_

PROTOTYPES: DISABLE

Cmiss::Value
create(Cmiss::FE_element element=(struct FE_element *)NULL,AV *xi_array=(AV *)NULL)
	CODE:
		if (element&&(RETVAL=CREATE(Cmiss_value)()))
		{
			FE_value *xi;
			int i,number_of_xi;

			ACCESS(Cmiss_value)(RETVAL);
			if (xi_array&&(0<(number_of_xi=av_len(xi_array)+1)))
			{
				if ((get_FE_element_dimension(element)==number_of_xi)&&
					(xi=(FE_value *)malloc(number_of_xi*sizeof(FE_value))))
				{
					for (i=0;i<number_of_xi;i++)
					{
						xi[i]=(FE_value)SvNV(AvARRAY(xi_array)[i]);
					}
					if (!Cmiss_value_element_xi_set_type(RETVAL,element,xi))
					{
						free(xi);
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
				if (!Cmiss_value_element_xi_set_type(RETVAL,element,(FE_value *)NULL))
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
			char *element_name;
			int i,number_of_xi;
			FE_value *xi;
			struct FE_element *element;

			element_name=(char *)NULL;
			if (Cmiss_value_element_xi_get_type(cmiss_value,&element,&xi)&&element
				&&FE_element_to_any_element_string(element,&element_name)&&
				(RETVAL=newSVpv(element_name,0)))
			{
				if (0<(number_of_xi=get_FE_element_dimension(element)))
				{
					sv_catpvf(RETVAL,", xi=[%g",xi[0]);
					if (number_of_xi<8)
					{
						for (i=1;i<number_of_xi;i++)
						{
							sv_catpvf(RETVAL,",%g",xi[i]);
						}
					}
					else
					{
						for (i=1;i<3;i++)
						{
							sv_catpvf(RETVAL,",%g",xi[i]);
						}
						sv_catpv(RETVAL,",...");
						for (i=number_of_xi-3;i<number_of_xi;i++)
						{
							sv_catpvf(RETVAL,",%g",xi[i]);
						}
					}
					sv_catpv(RETVAL,"]");
				}
			}
			if (element_name)
			{
				free(element_name);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
