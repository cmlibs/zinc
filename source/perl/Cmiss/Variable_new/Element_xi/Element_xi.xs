#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_new_finite_element.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Element_xi  PACKAGE = Cmiss::Variable_new::Element_xi  PREFIX = Cmiss_variable_new_element_xi_

PROTOTYPES: DISABLE

Cmiss::Variable_new
new_xs(Cmiss::Element element=(struct Cmiss_element *)NULL, \
	AV *xi_array=(AV *)NULL)
	CODE:
		if (element)
		{
			Scalar *xi;
			unsigned int i,number_of_xi;

			if (xi_array&&(0<(number_of_xi=av_len(xi_array)+1)))
			{
				if (xi=(Scalar *)malloc(number_of_xi*sizeof(Scalar)))
				{
					for (i=0;i<number_of_xi;i++)
					{
						xi[i]=(Scalar)SvNV(AvARRAY(xi_array)[i]);
					}
					RETVAL=Cmiss_variable_new_element_xi_create(element,number_of_xi,xi);
					free(xi);
				}
			}
			else
			{
				RETVAL=Cmiss_variable_new_element_xi_create(element,0,(Scalar *)NULL);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_element_xi_xs(Cmiss::Variable_new::Element_xi variable_element_xi)
	CODE:
		if (variable_element_xi)
		{
			RETVAL=Cmiss_variable_new_input_element_xi_element_xi(
				variable_element_xi);
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_xi_xs(Cmiss::Variable_new::Element_xi variable_element_xi, \
	AV *indices_array)
	CODE:
		if (variable_element_xi)
		{
			unsigned int i,*indices,number_of_indices;

			if (indices_array&&(number_of_indices=av_len(indices_array)+1))
			{
				if (indices=(unsigned int *)malloc(number_of_indices*
					sizeof(unsigned int)))
				{
					for (i=0;i<number_of_indices;i++)
					{
						indices[i]=(unsigned int)SvIV(AvARRAY(indices_array)[i]);
					}
					RETVAL=Cmiss_variable_new_input_element_xi_xi(variable_element_xi,
						number_of_indices,indices);
					free(indices);
				}
			}
			else
			{
				RETVAL=Cmiss_variable_new_input_element_xi_xi(variable_element_xi,0,
					(unsigned int *)NULL);
			}
		}
	OUTPUT:
		RETVAL
