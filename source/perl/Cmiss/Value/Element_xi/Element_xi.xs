#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include "api/cmiss_value.h"
#include "api/cmiss_value_element_xi.h"
#include "typemap.h"

MODULE = Cmiss::Value::Element_xi		PACKAGE = Cmiss::Value::Element_xi		PREFIX = Cmiss_value_element_xi_

PROTOTYPES: DISABLE

Cmiss::Value
create(int dimension=0,Cmiss::element element=(struct Cmiss_element *)NULL, \
	AV *xi_array=(AV *)NULL)
	CODE:
		if (element||(0<dimension))
		{
			float *xi;
			int i,number_of_xi;

			if (xi_array&&(0<(number_of_xi=av_len(xi_array)+1)))
			{
				if (((dimension==0) || (dimension==number_of_xi))&&
					(xi=(float *)malloc(number_of_xi*sizeof(float))))
				{
					dimension = number_of_xi;
					for (i=0;i<number_of_xi;i++)
					{
						xi[i]=(float)SvNV(AvARRAY(xi_array)[i]);
					}
					if (!(RETVAL=CREATE(Cmiss_value_element_xi)(dimension,element,
						xi)))
					{
						free(xi);
					}
				}
			}
			else
			{
				RETVAL=CREATE(Cmiss_value_element_xi)(dimension,element,
					(float *)NULL);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
