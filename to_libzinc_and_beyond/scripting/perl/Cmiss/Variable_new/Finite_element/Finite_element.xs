#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_new_finite_element.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Finite_element  PACKAGE = Cmiss::Variable_new::Finite_element  PREFIX = Cmiss_variable_new_finite_element_

PROTOTYPES: DISABLE

Cmiss::Variable_new
new_xs(Cmiss::Region region,char *path,char *name,char *component_name=(char *)NULL)
	CODE:
		{
			struct Cmiss_region *sub_region;

			if (path)
			{
				sub_region=Cmiss_region_get_sub_region(region,path);
			}
			else
			{
				sub_region=region;
			}
			RETVAL=Cmiss_variable_new_finite_element_create(sub_region,name,
				component_name);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_element_xi_xs(Cmiss::Variable_new::Finite_element variable_finite_element)
	CODE:
		if (variable_finite_element)
		{
			RETVAL=Cmiss_variable_new_input_finite_element_element_xi(
				variable_finite_element);
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_xi_xs(Cmiss::Variable_new::Finite_element variable_finite_element, \
	AV *indices_array)
	CODE:
		if (variable_finite_element)
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
					RETVAL=Cmiss_variable_new_input_finite_element_xi(variable_finite_element,
						number_of_indices,indices);
					free(indices);
				}
			}
			else
			{
				RETVAL=Cmiss_variable_new_input_finite_element_xi(
					variable_finite_element,0,(unsigned int *)NULL);
			}
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_nodal_values_xs( \
	Cmiss::Variable_new::Finite_element variable_finite_element, \
	Cmiss::Node node,char *type,int version)
	CODE:
		if (variable_finite_element)
		{
			enum FE_nodal_value_type value_type;
			int value_type_valid;

			value_type_valid=0;
			if (!strcmp("all",type))
			{
				value_type=FE_NODAL_UNKNOWN;
				value_type_valid=1;
			}
			else if (!strcmp("value",type))
			{
				value_type=FE_NODAL_VALUE;
				value_type_valid=1;
			}
			else if (!strcmp("d/ds1",type))
			{
				value_type=FE_NODAL_D_DS1;
				value_type_valid=1;
			}
			else if (!strcmp("d/ds2",type))
			{
				value_type=FE_NODAL_D_DS2;
				value_type_valid=1;
			}
			else if (!strcmp("d/ds3",type))
			{
				value_type=FE_NODAL_D_DS3;
				value_type_valid=1;
			}
			else if (!strcmp("d2/ds1ds2",type))
			{
				value_type=FE_NODAL_D2_DS1DS2;
				value_type_valid=1;
			}
			else if (!strcmp("d2/ds1ds3",type))
			{
				value_type=FE_NODAL_D2_DS1DS3;
				value_type_valid=1;
			}
			else if (!strcmp("d2/ds2ds3",type))
			{
				value_type=FE_NODAL_D2_DS2DS3;
				value_type_valid=1;
			}
			else if (!strcmp("d3/ds1ds2ds3",type))
			{
				value_type=FE_NODAL_D3_DS1DS2DS3;
				value_type_valid=1;
			}
			if (value_type_valid)
			{
				RETVAL=Cmiss_variable_new_input_finite_element_nodal_values(
					variable_finite_element,node,value_type,version);
			}
		}
	OUTPUT:
		RETVAL
