#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_function_coordinates.h"
#include "typemap.h"

MODULE = Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian  PACKAGE = Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian  PREFIX = Cmiss_function_prolate_spheroidal_to_rectangular_cartesian_

PROTOTYPES: DISABLE

Cmiss::Function
new_xs(AV *values_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_function structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_function_2=$cmiss_function_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		{
			int number_of_values;
			Scalar focus,lambda,mu,theta;

			lambda=0;
			mu=0;
			theta=0;
			focus=1;
			number_of_values=0;
			if (values_array)
			{
				number_of_values=av_len(values_array)+1;
			}
			if (0<number_of_values)
			{
				lambda=(Scalar)SvNV(AvARRAY(values_array)[0]);
				if (1<number_of_values)
				{
					mu=(Scalar)SvNV(AvARRAY(values_array)[1]);
					if (2<number_of_values)
					{
						theta=(Scalar)SvNV(AvARRAY(values_array)[2]);
						if (3<number_of_values)
						{
							focus=(Scalar)SvNV(AvARRAY(values_array)[3]);
						}
					}
				}
			}
			RETVAL=Cmiss_function_prolate_spheroidal_to_rectangular_cartesian_create(
				lambda,mu,theta,focus);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
component_name_xs( \
	Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian function, \
	char *name)
	CODE:
		RETVAL=Cmiss_function_prolate_spheroidal_to_rectangular_cartesian_component(
			function,name,0);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
component_number_xs( \
	Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian function, \
	unsigned int number)
	CODE:
		RETVAL=Cmiss_function_prolate_spheroidal_to_rectangular_cartesian_component(
			function,(char *)NULL,number);
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
focus_xs( \
	Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian function)
	CODE:
		if (function)
		{
			RETVAL=Cmiss_function_variable_prolate_spheroidal_to_rectangular_cartesian_focus(
				function);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
lambda_xs( \
	Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian function)
	CODE:
		if (function)
		{
			RETVAL=
				Cmiss_function_variable_prolate_spheroidal_to_rectangular_cartesian_lambda(
				function);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
mu_xs( \
	Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian function)
	CODE:
		if (function)
		{
			RETVAL=
				Cmiss_function_variable_prolate_spheroidal_to_rectangular_cartesian_mu(
				function);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
prolate_xs( \
	Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian function)
	CODE:
		if (function)
		{
			RETVAL=
				Cmiss_function_variable_prolate_spheroidal_to_rectangular_cartesian_prolate(
				function);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Function_variable
theta_xs( \
	Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian function)
	CODE:
		if (function)
		{
			RETVAL=
				Cmiss_function_variable_prolate_spheroidal_to_rectangular_cartesian_theta(
				function);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
