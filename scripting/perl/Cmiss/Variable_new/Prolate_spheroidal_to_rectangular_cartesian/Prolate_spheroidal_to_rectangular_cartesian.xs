#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include "api/cmiss_variable_new_coordinates.h"
#include "typemap.h"

MODULE = Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian  PACKAGE = Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian  PREFIX = Cmiss_variable_new_prolate_spheroidal_to_rectangular_cartesian_

PROTOTYPES: DISABLE

Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian
new_xs(AV *values_array)
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_variable_new structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_variable_2=$cmiss_variable_1,
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
			RETVAL=
				Cmiss_variable_new_prolate_spheroidal_to_rectangular_cartesian_create(
				lambda,mu,theta,focus);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_prolate_xs( \
	Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian variable)
	CODE:
		if (variable)
		{
			RETVAL=
				Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_prolate(
				variable);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_lambda_xs( \
	Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian variable)
	CODE:
		if (variable)
		{
			RETVAL=
				Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_lambda(
				variable);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_mu_xs( \
	Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian variable)
	CODE:
		if (variable)
		{
			RETVAL=
				Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_mu(
				variable);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_theta_xs( \
	Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian variable)
	CODE:
		if (variable)
		{
			RETVAL=
				Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_theta(
				variable);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Variable_new_input
input_focus_xs( \
	Cmiss::Variable_new::Prolate_spheroidal_to_rectangular_cartesian variable)
	CODE:
		if (variable)
		{
			RETVAL=Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_focus(
				variable);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
