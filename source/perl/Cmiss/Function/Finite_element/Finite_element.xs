#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_function_finite_element.h"
#include "api/cmiss_region.h"
#include "typemap.h"

MODULE = Cmiss::Function::Finite_element  PACKAGE = Cmiss::Function::Finite_element  PREFIX = Cmiss_function_finite_element_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(Cmiss::Region region,char *path,char *name)
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
			RETVAL=Cmiss_function_finite_element_create(sub_region,name);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function
create_standard_interpolation_rc_constant_time_xs( \
	Cmiss::Region region,char *name,int number_of_components, \
	AV *component_names_array)
	CODE:
	{
		int i, length, number_of_component_names;

		if (component_names_array&&(0<(number_of_component_names=av_len(component_names_array)+1)))
		{
			char **component_names;
			if (number_of_components == number_of_component_names)
			{
				if (component_names=(char **)malloc(number_of_components*sizeof(char *)))
				{
					for (i=0;i<number_of_components;i++)
					{
						component_names[i]=(char *)SvPV(AvARRAY(component_names_array)[i], length);
					}
					RETVAL=Cmiss_function_finite_element_create_standard_interpolation_rc_constant_time(
						region,name,number_of_components,component_names);
				}
			}
			else
			{
				croak("Number of strings in component names array does not match number of components");
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
component_name_xs(Cmiss::Function::Finite_element finite_element,char *name)
	CODE:
		RETVAL=Cmiss_function_finite_element_component(finite_element,name,0);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
component_number_xs(Cmiss::Function::Finite_element finite_element, \
	unsigned int number)
	CODE:
		RETVAL=Cmiss_function_finite_element_component(finite_element,(char *)NULL,
			number);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
element_xi_xs(Cmiss::Function::Finite_element finite_element)
	CODE:
		RETVAL=Cmiss_function_finite_element_element_xi(finite_element);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
element_xs(Cmiss::Function::Finite_element finite_element)
	CODE:
		RETVAL=Cmiss_function_finite_element_element(finite_element);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
xi_xs(Cmiss::Function::Finite_element finite_element)
	CODE:
		RETVAL=Cmiss_function_finite_element_xi(finite_element);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
xi_entry_xs(Cmiss::Function::Finite_element finite_element,unsigned int index)
	CODE:
		RETVAL=Cmiss_function_finite_element_xi_entry(finite_element,index);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL


Cmiss::Function_variable
nodal_values_xs( \
	Cmiss::Function::Finite_element finite_element, \
	char *component_name,unsigned int component_number, \
	Cmiss::Node node,char *type,unsigned int version, \
	Cmiss::Time_sequence time_sequence)
	CODE:
		if (finite_element)
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
				RETVAL=Cmiss_function_finite_element_nodal_values(finite_element,
					component_name,component_number,node,value_type,version,
					time_sequence);
			}
		}
	OUTPUT:
		RETVAL

int
define_on_Cmiss_node_xs(Cmiss::Function::Finite_element finite_element,Cmiss::Node node, \
	Cmiss::Time_sequence time_sequence, Cmiss::Node_field_creator field_creator)
	CODE:
		RETVAL=Cmiss_function_finite_element_define_on_Cmiss_node(finite_element,node,
			time_sequence, field_creator);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

int
define_tensor_product_basis_on_element_xs(Cmiss::Function::Finite_element finite_element, \
	Cmiss::Element element, int dimension, int basis_type_enumerator)
	CODE:
		RETVAL=Cmiss_function_finite_element_define_tensor_product_basis_on_element(
			finite_element,element,dimension, basis_type_enumerator);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
