#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_function_finite_element.h"
#include "typemap.h"

MODULE = Cmiss::Function::Element_xi  PACKAGE = Cmiss::Function::Element_xi  PREFIX = Cmiss_function_element_xi_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(Cmiss::element element,AV *xi_array)
	CODE:
		{
			unsigned int number_of_xi;

			if (element&&xi_array&&(0<(number_of_xi=av_len(xi_array)+1)))
			{
				Scalar *xi;
				unsigned int i;

				if (xi=(Scalar *)malloc(number_of_xi*sizeof(Scalar)))
				{
					for (i=0;i<number_of_xi;i++)
					{
						xi[i]=(Scalar)SvNV(AvARRAY(xi_array)[i]);
					}
					RETVAL=Cmiss_function_element_xi_create(element,number_of_xi,xi);
					free(xi);
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
element_xs(Cmiss::Function::Element_xi element_xi)
	CODE:
		RETVAL=Cmiss_function_element_xi_element(element_xi);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
xi_xs(Cmiss::Function::Element_xi element_xi)
	CODE:
		RETVAL=Cmiss_function_element_xi_xi(element_xi);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
xi_entry_xs(Cmiss::Function::Element_xi element_xi,unsigned int index)
	CODE:
		RETVAL=Cmiss_function_element_xi_xi_entry(element_xi,index);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
