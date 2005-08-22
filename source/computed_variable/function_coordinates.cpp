//******************************************************************************
// FILE : function_coordinates.cpp
//
// LAST MODIFIED : 11 May 2005
//
// DESCRIPTION :
//==============================================================================
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <sstream>

#include "computed_variable/function_coordinates.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_composite.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#include "computed_variable/function_identity.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

// module typedefs
// ===============

typedef boost::intrusive_ptr< Function_variable_matrix<Scalar> >
	Function_variable_matrix_scalar_handle;

// module classes
// ==============

// forward declaration so that can use _handle
class Function_variable_matrix_rectangular_cartesian;
typedef boost::intrusive_ptr<Function_variable_matrix_rectangular_cartesian>
	Function_variable_matrix_rectangular_cartesian_handle;

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_matrix_rectangular_cartesian
// --------------------------------------------

class Function_derivatnew_matrix_rectangular_cartesian :
	public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 27 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// for evaluation exception
		class Evaluation_exception {};
		// constructor
		Function_derivatnew_matrix_rectangular_cartesian(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_matrix_rectangular_cartesian();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
	private:
		// copy operations are private and undefined to prevent copying
		void operator=(const Function&);
};

Function_derivatnew_matrix_rectangular_cartesian::
	Function_derivatnew_matrix_rectangular_cartesian(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables)
//******************************************************************************
// LAST MODIFIED : 27 April 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (boost::dynamic_pointer_cast<
		Function_variable_matrix_rectangular_cartesian,Function_variable>(
		dependent_variable)&&boost::dynamic_pointer_cast<
		Function_prolate_spheroidal_to_rectangular_cartesian,Function>(
		dependent_variable->function()))
	{
		// do nothing
	}
	else
	{
		throw Function_derivatnew_matrix_rectangular_cartesian::
			Construction_exception();
	}
}

Function_derivatnew_matrix_rectangular_cartesian::
	~Function_derivatnew_matrix_rectangular_cartesian(){}
//******************************************************************************
// LAST MODIFIED : 27 April 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)


// class Function_variable_matrix_rectangular_cartesian
// ----------------------------------------------------

class Function_variable_matrix_rectangular_cartesian :
	public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 18 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_prolate_spheroidal_to_rectangular_cartesian;
	public:
		// constructors.  A zero component_number indicates all components
		Function_variable_matrix_rectangular_cartesian(
			const Function_prolate_spheroidal_to_rectangular_cartesian_handle&
			function_prolate_spheroidal_to_rectangular_cartesian,
			Function_size_type component_number=0):Function_variable_matrix<Scalar>(
			function_prolate_spheroidal_to_rectangular_cartesian,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			false,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			component_number,1){};
		Function_variable_matrix_rectangular_cartesian(
			const Function_prolate_spheroidal_to_rectangular_cartesian_handle&
			function_prolate_spheroidal_to_rectangular_cartesian,
			const std::string component_name):Function_variable_matrix<Scalar>(
			function_prolate_spheroidal_to_rectangular_cartesian,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			false,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			0,1)
		{
			if (function_prolate_spheroidal_to_rectangular_cartesian)
			{
				Function_size_type local_component_number;

				local_component_number=0;
				if ((std::string("x")==component_name)||
					(std::string("X")==component_name))
				{
					local_component_number=1;
				}
				else if ((std::string("y")==component_name)||
					(std::string("Y")==component_name))
				{
					local_component_number=2;
				}
				else if ((std::string("z")==component_name)||
					(std::string("Z")==component_name))
				{
					local_component_number=3;
				}
				if ((0<local_component_number)&&(local_component_number<=
					function_prolate_spheroidal_to_rectangular_cartesian->
					number_of_components()))
				{
					row_private=local_component_number;
					value_private=Function_variable_value_handle(
						new Function_variable_value_specific<Scalar>(
						Function_variable_matrix_set_value_function<Scalar>));
				}
			}
		};
		// destructor
		~Function_variable_matrix_rectangular_cartesian(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_rectangular_cartesian(*this)));
		};
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		virtual Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables)
		{
			return (Function_handle(
				new Function_derivatnew_matrix_rectangular_cartesian(
				Function_variable_handle(this),independent_variables)));
		};
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		string_handle get_string_representation()
		{
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				out << "rectangular cartesian";
				switch (row_private)
				{
					case 0:
					{
						// do nothing
					} break;
					case 1:
					{
						out << ".x";
					} break;
					case 2:
					{
						out << ".y";
					} break;
					case 3:
					{
						out << ".z";
					} break;
					default:
					{
						out << "[" << row_private << "]";
					} break;
				}
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_handle get_value() const
		{
			Function_handle result(0);
			Function_prolate_spheroidal_to_rectangular_cartesian_handle
				function_prolate_spheroidal_to_rectangular_cartesian;

			if (this&&(function_prolate_spheroidal_to_rectangular_cartesian=
				boost::dynamic_pointer_cast<
				Function_prolate_spheroidal_to_rectangular_cartesian,Function>(
				function())))
			{
				if (0==row_private)
				{
					Matrix result_matrix(3,1);

					result_matrix(0,0)=
						(function_prolate_spheroidal_to_rectangular_cartesian->x_value)();
					result_matrix(1,0)=
						(function_prolate_spheroidal_to_rectangular_cartesian->y_value)();
					result_matrix(2,0)=
						(function_prolate_spheroidal_to_rectangular_cartesian->z_value)();
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
				else
				{
					if (row_private<=3)
					{
						Matrix result_matrix(1,1);

						switch (row_private)
						{
							case 1:
							{
								result_matrix(0,0)=
									(function_prolate_spheroidal_to_rectangular_cartesian->
									x_value)();
							} break;
							case 2:
							{
								result_matrix(0,0)=
									(function_prolate_spheroidal_to_rectangular_cartesian->
									y_value)();
							} break;
							case 3:
							{
								result_matrix(0,0)=
									(function_prolate_spheroidal_to_rectangular_cartesian->
									z_value)();
							} break;
						}
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
					}
				}
			}

			return (result);
		};
		Function_variable_matrix_scalar_handle operator()(
			Function_size_type row,Function_size_type column) const
		{
			Function_variable_matrix_scalar_handle result(0);

			if ((row<=number_of_rows())&&(column<=number_of_columns()))
			{
				result=Function_variable_matrix_scalar_handle(
					new Function_variable_matrix_rectangular_cartesian(
					boost::dynamic_pointer_cast<
					Function_prolate_spheroidal_to_rectangular_cartesian,Function>(
					function_private),row));
			}

			return (result);
		};
		Function_size_type number_of_rows() const
		{
			Function_prolate_spheroidal_to_rectangular_cartesian_handle
				function_prolate_spheroidal_to_rectangular_cartesian;
			Function_size_type result;

			result=0;
			if (this&&(function_prolate_spheroidal_to_rectangular_cartesian=
				boost::dynamic_pointer_cast<
				Function_prolate_spheroidal_to_rectangular_cartesian,Function>(
				function())))
			{
				result=function_prolate_spheroidal_to_rectangular_cartesian->
					number_of_components();
			}

			return (result);
		};
		Function_size_type number_of_columns() const
		{
			return (1);
		};
		bool get_entry(Scalar& value) const
		{
			bool result;
			Function_prolate_spheroidal_to_rectangular_cartesian_handle
				function_prolate_spheroidal_to_rectangular_cartesian;

			result=false;
			if (this&&(1==column_private)&&
				(function_prolate_spheroidal_to_rectangular_cartesian=
				boost::dynamic_pointer_cast<
				Function_prolate_spheroidal_to_rectangular_cartesian,Function>(
				function())))
			{
				switch (row_private)
				{
					case 1:
					{
						value=
							(function_prolate_spheroidal_to_rectangular_cartesian->x_value)();
						result=true;
					} break;
					case 2:
					{
						value=
							(function_prolate_spheroidal_to_rectangular_cartesian->y_value)();
						result=true;
					} break;
					case 3:
					{
						value=
							(function_prolate_spheroidal_to_rectangular_cartesian->z_value)();
						result=true;
					} break;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix_rectangular_cartesian(const
			Function_variable_matrix_rectangular_cartesian&
			variable_rectangular_cartesian):
			Function_variable_matrix<Scalar>(variable_rectangular_cartesian){};
};


// class Function_variable_matrix_prolate_spheroidal
// -------------------------------------------------

// forward declaration so that can use _handle
class Function_variable_matrix_prolate_spheroidal;
typedef boost::intrusive_ptr<Function_variable_matrix_prolate_spheroidal>
	Function_variable_matrix_prolate_spheroidal_handle;

class Function_variable_matrix_prolate_spheroidal :
	public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 27 April 2005
//
// DESCRIPTION :
// component  number
// lambda     1
// mu         2
// theta      3
//==============================================================================
{
	friend class Function_prolate_spheroidal_to_rectangular_cartesian;
	public:
		// constructor
		Function_variable_matrix_prolate_spheroidal(
			const Function_prolate_spheroidal_to_rectangular_cartesian_handle&
			function_prolate_spheroidal_to_rectangular_cartesian,
			Function_size_type component_number):Function_variable_matrix<Scalar>(
			function_prolate_spheroidal_to_rectangular_cartesian,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			true,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			component_number,1)
		{};
		Function_variable_matrix_prolate_spheroidal(
			const Function_prolate_spheroidal_to_rectangular_cartesian_handle&
			function_prolate_spheroidal_to_rectangular_cartesian,
			const std::string component_name):Function_variable_matrix<Scalar>(
			function_prolate_spheroidal_to_rectangular_cartesian,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			true,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			0,1)
		{
			if (function_prolate_spheroidal_to_rectangular_cartesian)
			{
				Function_size_type local_component_number;

				local_component_number=0;
				if ((std::string("lambda")==component_name)||
					(std::string("LAMBDA")==component_name))
				{
					local_component_number=1;
				}
				else if ((std::string("mu")==component_name)||
					(std::string("MU")==component_name))
				{
					local_component_number=2;
				}
				else if ((std::string("theta")==component_name)||
					(std::string("THETA")==component_name))
				{
					local_component_number=3;
				}
				if (0<local_component_number)
				{
					row_private=local_component_number;
					value_private=Function_variable_value_handle(
						new Function_variable_value_specific<Scalar>(
						Function_variable_matrix_set_value_function<Scalar>));
				}
			}
		};
		~Function_variable_matrix_prolate_spheroidal(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_prolate_spheroidal(*this)));
		};
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables)
		{
			return (Function_handle(new Function_derivatnew_identity(
				Function_variable_handle(this),independent_variables)));
		};
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		string_handle get_string_representation()
		{
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				switch (row_private)
				{
					case 0:
					{
						out << "(lambda,mu,theta)";
					} break;
					case 1:
					{
						out << "lambda";
					} break;
					case 2:
					{
						out << "mu";
					} break;
					case 3:
					{
						out << "theta";
					} break;
				}
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_variable_matrix_scalar_handle operator()(
			Function_size_type row,Function_size_type column) const
		{
			Function_variable_matrix_scalar_handle result(0);

			if ((0<row)&&(1==column))
			{
				Function_prolate_spheroidal_to_rectangular_cartesian_handle
					function_prolate_spheroidal_to_rectangular_cartesian=
					boost::dynamic_pointer_cast<
					Function_prolate_spheroidal_to_rectangular_cartesian,Function>(
					function());

				result=Function_variable_matrix_scalar_handle(
					new Function_variable_matrix_prolate_spheroidal(
					function_prolate_spheroidal_to_rectangular_cartesian,row));
			}

			return (result);
		};
		Function_size_type number_of_rows() const
		{
			return (3);
		};
		Function_size_type number_of_columns() const
		{
			return (1);
		};
		bool get_entry(Scalar& value) const
		{
			bool result;
			Function_prolate_spheroidal_to_rectangular_cartesian_handle
				function_prolate_spheroidal_to_rectangular_cartesian;

			result=false;
			if ((0<row_private)&&(1==column_private)&&
				(function_prolate_spheroidal_to_rectangular_cartesian=
				boost::dynamic_pointer_cast<
				Function_prolate_spheroidal_to_rectangular_cartesian,Function>(
				function())))
			{
				switch (row_private)
				{
					case 1:
					{
						value=(function_prolate_spheroidal_to_rectangular_cartesian->
							lambda_value)();
						result=true;
					} break;
					case 2:
					{
						value=(function_prolate_spheroidal_to_rectangular_cartesian->
							mu_value)();
						result=true;
					} break;
					case 3:
					{
						value=(function_prolate_spheroidal_to_rectangular_cartesian->
							theta_value)();
						result=true;
					} break;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix_prolate_spheroidal(
			const Function_variable_matrix_prolate_spheroidal& variable):
			Function_variable_matrix<Scalar>(variable){};
};


// class Function_variable_matrix_focus
// ------------------------------------

// forward declaration so that can use _handle
class Function_variable_matrix_focus;
typedef boost::intrusive_ptr<Function_variable_matrix_focus>
	Function_variable_matrix_focus_handle;

class Function_variable_matrix_focus : public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 18 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_prolate_spheroidal_to_rectangular_cartesian;
	public:
		// constructor
		Function_variable_matrix_focus(
			const Function_prolate_spheroidal_to_rectangular_cartesian_handle&
			function_prolate_spheroidal_to_rectangular_cartesian):
			Function_variable_matrix<Scalar>(
			function_prolate_spheroidal_to_rectangular_cartesian,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			true,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			1,1){};
		~Function_variable_matrix_focus(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_focus(*this)));
		};
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables)
		{
			return (Function_handle(new Function_derivatnew_identity(
				Function_variable_handle(this),independent_variables)));
		}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		string_handle get_string_representation()
		{
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				out << "focus";
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_variable_matrix_scalar_handle operator()(
			Function_size_type row,Function_size_type column) const
		{
			Function_variable_matrix_scalar_handle result(0);

			if ((1==row)&&(1==column))
			{
				Function_prolate_spheroidal_to_rectangular_cartesian_handle
					function_prolate_spheroidal_to_rectangular_cartesian=
					boost::dynamic_pointer_cast<
					Function_prolate_spheroidal_to_rectangular_cartesian,Function>(
					function());

				result=Function_variable_matrix_scalar_handle(
					new Function_variable_matrix_focus(
					function_prolate_spheroidal_to_rectangular_cartesian));
			}

			return (result);
		};
		Function_size_type number_of_rows() const
		{
			return (1);
		};
		Function_size_type number_of_columns() const
		{
			return (1);
		};
		bool get_entry(Scalar& value) const
		{
			bool result;
			Function_prolate_spheroidal_to_rectangular_cartesian_handle
				function_prolate_spheroidal_to_rectangular_cartesian;

			result=false;
			if ((1==row_private)&&(1==column_private)&&
				(function_prolate_spheroidal_to_rectangular_cartesian=
				boost::dynamic_pointer_cast<
				Function_prolate_spheroidal_to_rectangular_cartesian,Function>(
				function())))
			{
				value=(function_prolate_spheroidal_to_rectangular_cartesian->
					focus_value)();
				result=true;
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix_focus(
			const Function_variable_matrix_focus& variable):
			Function_variable_matrix<Scalar>(variable){};
};

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
bool calculate_prolate_spheroidal_to_rectangular_cartesian_derivative(
	Scalar& derivative,Function_size_type rectangular_cartesian_component,
	Function_size_type lambda_derivative_order,
	Function_size_type mu_derivative_order,
	Function_size_type theta_derivative_order,
	Function_size_type focus_derivative_order,
	Scalar cosh_lambda,Scalar sinh_lambda,Scalar cos_mu,Scalar sin_mu,
	Scalar cos_theta,Scalar sin_theta,Scalar focus)
//******************************************************************************
// LAST MODIFIED : 28 April 2005
//
// DESCRIPTION :
// For transforming from prolate spheroidal to cartesian coordinates.
// x = focus*cosh(lambda)*cos(mu)
// y = focus*sinh(lambda)*sin(mu)*cos(theta)
// z = focus*sinh(lambda)*sin(mu)*sin(theta)
//
// ???DB.  in-line or make into a macro?
//==============================================================================
{
	bool result(false);

	if ((1<=rectangular_cartesian_component)&&
		(rectangular_cartesian_component<=3))
	{
		result=true;
		if (focus_derivative_order<2)
		{
			if (0==focus_derivative_order)
			{
				derivative=focus;
			}
			else
			{
				derivative=1;
			}
			switch (rectangular_cartesian_component)
			{
				case 1:
				// x = focus*cosh(lambda)*cos(mu)
				{
					if (0==theta_derivative_order)
					{
						if (0==lambda_derivative_order%2)
						{
							derivative *= cosh_lambda;
						}
						else
						{
							derivative *= sinh_lambda;
						}
						if (0==mu_derivative_order%2)
						{
							derivative *= cos_mu;
						}
						else
						{
							derivative *= sin_mu;
						}
						if (1==((mu_derivative_order+1)%4)/2)
						{
							derivative= -derivative;
						}
					}
					else
					{
						derivative=0;
					}
				} break;
				case 2:
				// y = focus*sinh(lambda)*sin(mu)*cos(theta)
				{
					if (0==lambda_derivative_order%2)
					{
						derivative *= sinh_lambda;
					}
					else
					{
						derivative *= cosh_lambda;
					}
					if (0==mu_derivative_order%2)
					{
						derivative *= sin_mu;
					}
					else
					{
						derivative *= cos_mu;
					}
					if (1==(mu_derivative_order%4)/2)
					{
						derivative= -derivative;
					}
					if (0==theta_derivative_order%2)
					{
						derivative *= cos_theta;
					}
					else
					{
						derivative *= sin_theta;
					}
					if (1==((theta_derivative_order+1)%4)/2)
					{
						derivative= -derivative;
					}
				} break;
				case 3:
				// z = focus*sinh(lambda)*sin(mu)*sin(theta)
				{
					if (0==lambda_derivative_order%2)
					{
						derivative *= sinh_lambda;
					}
					else
					{
						derivative *= cosh_lambda;
					}
					if (0==mu_derivative_order%2)
					{
						derivative *= sin_mu;
					}
					else
					{
						derivative *= cos_mu;
					}
					if (1==(mu_derivative_order%4)/2)
					{
						derivative= -derivative;
					}
					if (0==theta_derivative_order%2)
					{
						derivative *= sin_theta;
					}
					else
					{
						derivative *= cos_theta;
					}
					if (1==(theta_derivative_order%4)/2)
					{
						derivative= -derivative;
					}
				} break;
			}
		}
		else
		{
			derivative=0;
		}
	}

	return (result);
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_matrix_rectangular_cartesian::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
//******************************************************************************
// LAST MODIFIED : 9 May 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)

	if (!evaluated())
	{
		Function_prolate_spheroidal_to_rectangular_cartesian_handle
			function_prolate_spheroidal_to_rectangular_cartesian;
		Function_variable_matrix_rectangular_cartesian_handle
			dependent_variable_rectangular_cartesian;

#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		result=false;
#endif // defined (EVALUATE_RETURNS_VALUE)
		if ((dependent_variable_rectangular_cartesian=boost::dynamic_pointer_cast<
			Function_variable_matrix_rectangular_cartesian,Function_variable>(
			dependent_variable))&&
			(function_prolate_spheroidal_to_rectangular_cartesian=
			boost::dynamic_pointer_cast<
			Function_prolate_spheroidal_to_rectangular_cartesian,Function>(
			dependent_variable->function())))
		{
			bool zero_derivative;
			Function_size_type rectangular_cartesian_component=
				dependent_variable_rectangular_cartesian->row(),
				number_of_dependent_values=dependent_variable->
				number_differentiable();
			Function_size_type focus_derivative_order,lambda_derivative_order,
				mu_derivative_order,theta_derivative_order;
			Function_variable_matrix_focus_handle variable_focus;
			Function_variable_matrix_prolate_spheroidal_handle
				variable_prolate_spheroidal;
			Scalar focus=function_prolate_spheroidal_to_rectangular_cartesian->
				focus_value(),
				lambda=function_prolate_spheroidal_to_rectangular_cartesian->
				lambda_value(),
				mu=function_prolate_spheroidal_to_rectangular_cartesian->mu_value(),
				theta=function_prolate_spheroidal_to_rectangular_cartesian->
				theta_value();
			Scalar cosh_lambda=cosh((double)lambda),cos_mu=cos((double)mu),
				cos_theta=cos((double)theta),sinh_lambda=sinh((double)lambda),
				sin_mu=sin((double)mu),sin_theta=sin((double)theta);
			std::list<Function_variable_handle>::const_iterator
				independent_variable_iterator;
			std::list<Matrix> matrices;
			std::list< std::list<Function_variable_handle> >
				matrix_independent_variables;
			std::vector<Function_variable_iterator>
				atomic_independent_variable_iterators(independent_variables.size());

			for (independent_variable_iterator=independent_variables.begin();
				independent_variable_iterator!=independent_variables.end();
				++independent_variable_iterator)
			{
				Function_variable_handle independent_variable=
					*independent_variable_iterator;
				Function_size_type number_of_independent_values=independent_variable->
					number_differentiable();
				std::list<Matrix>::iterator matrix_iterator,last;
				std::list< std::list<Function_variable_handle> >::iterator
					matrix_independent_variables_iterator;

				// calculate the derivative of dependent variable with respect to
				//   independent variable and add to matrix list
				{
					Function_size_type column;
					Function_variable_iterator
						atomic_independent_variable_iterator(0);
					Matrix new_matrix(number_of_dependent_values,
						number_of_independent_values);
					std::list<Function_variable_handle> new_matrix_independent_variables;

					new_matrix_independent_variables.push_back(independent_variable);
					column=0;
					for (atomic_independent_variable_iterator=independent_variable->
						begin_atomic();atomic_independent_variable_iterator!=
						independent_variable->end_atomic();
						++atomic_independent_variable_iterator)
					{
						Function_variable_handle atomic_independent_variable=
							*atomic_independent_variable_iterator;

						if (1==atomic_independent_variable->number_differentiable())
						{
							zero_derivative=true;
							lambda_derivative_order=0;
							mu_derivative_order=0;
							theta_derivative_order=0;
							focus_derivative_order=0;
							if ((variable_prolate_spheroidal=boost::dynamic_pointer_cast<
								Function_variable_matrix_prolate_spheroidal,Function_variable>(
								*atomic_independent_variable_iterator))&&
								equivalent(function_prolate_spheroidal_to_rectangular_cartesian,
								variable_prolate_spheroidal->function()))
							{
								switch (variable_prolate_spheroidal->row())
								{
									case 1:
									{
										lambda_derivative_order=1;
										zero_derivative=false;
									} break;
									case 2:
									{
										mu_derivative_order=1;
										zero_derivative=false;
									} break;
									case 3:
									{
										theta_derivative_order=1;
										zero_derivative=false;
									} break;
								}
							}
							else if ((variable_focus=boost::dynamic_pointer_cast<
								Function_variable_matrix_focus,Function_variable>(
								*atomic_independent_variable_iterator))&&
								equivalent(function_prolate_spheroidal_to_rectangular_cartesian,
								variable_focus->function()))
							{
								focus_derivative_order=1;
								zero_derivative=false;
							}
							if (zero_derivative)
							{
								new_matrix(0,column)=0;
								if (0==rectangular_cartesian_component)
								{
									new_matrix(1,column)=0;
									new_matrix(2,column)=0;
								}
							}
							else
							{
								if (0==rectangular_cartesian_component)
								{
									calculate_prolate_spheroidal_to_rectangular_cartesian_derivative(
										new_matrix(0,column),1,
										lambda_derivative_order,mu_derivative_order,
										theta_derivative_order,focus_derivative_order,
										cosh_lambda,sinh_lambda,cos_mu,sin_mu,cos_theta,sin_theta,
										focus);
									calculate_prolate_spheroidal_to_rectangular_cartesian_derivative(
										new_matrix(1,column),2,
										lambda_derivative_order,mu_derivative_order,
										theta_derivative_order,focus_derivative_order,
										cosh_lambda,sinh_lambda,cos_mu,sin_mu,cos_theta,sin_theta,
										focus);
									calculate_prolate_spheroidal_to_rectangular_cartesian_derivative(
										new_matrix(2,column),3,
										lambda_derivative_order,mu_derivative_order,
										theta_derivative_order,focus_derivative_order,
										cosh_lambda,sinh_lambda,cos_mu,sin_mu,cos_theta,sin_theta,
										focus);
								}
								else
								{
									calculate_prolate_spheroidal_to_rectangular_cartesian_derivative(
										new_matrix(0,column),
										rectangular_cartesian_component,
										lambda_derivative_order,mu_derivative_order,
										theta_derivative_order,focus_derivative_order,
										cosh_lambda,sinh_lambda,cos_mu,sin_mu,cos_theta,sin_theta,
										focus);
								}
							}
							++column;
						}
					}
					matrices.push_back(new_matrix);
					matrix_independent_variables.push_back(
						new_matrix_independent_variables);
				}
				last=matrices.end();
				--last;
				matrix_independent_variables_iterator=
					matrix_independent_variables.begin();
				for (matrix_iterator=matrices.begin();matrix_iterator!=last;
					++matrix_iterator)
				{
					bool no_derivative;
					Function_size_type column,i,new_matrix_derivative_order;
					Matrix& matrix= *matrix_iterator;
					Matrix new_matrix((matrix.size1)(),
						number_of_independent_values*(matrix.size2)());
					std::list<Function_variable_handle> new_matrix_independent_variables;
					std::list<Function_variable_handle>::iterator
						new_matrix_independent_variables_iterator;

					new_matrix_independent_variables=
						*matrix_independent_variables_iterator;
					new_matrix_independent_variables.push_back(independent_variable);
					new_matrix_derivative_order=new_matrix_independent_variables.size();
					new_matrix_independent_variables_iterator=
						new_matrix_independent_variables.begin();
					i=0;
					no_derivative=false;
					lambda_derivative_order=0;
					mu_derivative_order=0;
					theta_derivative_order=0;
					focus_derivative_order=0;
					while ((new_matrix_independent_variables_iterator!=
						new_matrix_independent_variables.end())&&!no_derivative)
					{
						atomic_independent_variable_iterators[i]=
							(*new_matrix_independent_variables_iterator)->begin_atomic();
						while ((atomic_independent_variable_iterators[i]!=
							(*new_matrix_independent_variables_iterator)->end_atomic())&&
							(1!=(*(atomic_independent_variable_iterators[i]))->
							number_differentiable()))
						{
							++atomic_independent_variable_iterators[i];
						}
						if (atomic_independent_variable_iterators[i]==
							(*new_matrix_independent_variables_iterator)->end_atomic())
						{
							no_derivative=true;
						}
						else
						{
							if ((variable_prolate_spheroidal=boost::dynamic_pointer_cast<
								Function_variable_matrix_prolate_spheroidal,Function_variable>(
								*atomic_independent_variable_iterators[i]))&&
								equivalent(function_prolate_spheroidal_to_rectangular_cartesian,
								variable_prolate_spheroidal->function()))
							{
								switch (variable_prolate_spheroidal->row())
								{
									case 1:
									{
										++lambda_derivative_order;
									} break;
									case 2:
									{
										++mu_derivative_order;
									} break;
									case 3:
									{
										++theta_derivative_order;
									} break;
								}
							}
							else if ((variable_focus=boost::dynamic_pointer_cast<
								Function_variable_matrix_focus,Function_variable>(
								*atomic_independent_variable_iterators[i]))&&
								equivalent(function_prolate_spheroidal_to_rectangular_cartesian,
								variable_focus->function()))
							{
								++focus_derivative_order;
							}
							++new_matrix_independent_variables_iterator;
						}
						++i;
					}
					if (new_matrix_independent_variables_iterator==
						new_matrix_independent_variables.end())
					{
						column=0;
						do
						{
							if (new_matrix_derivative_order==lambda_derivative_order+
								mu_derivative_order+theta_derivative_order+
								focus_derivative_order)
							{
								if (0==rectangular_cartesian_component)
								{
									calculate_prolate_spheroidal_to_rectangular_cartesian_derivative(
										new_matrix(0,column),1,
										lambda_derivative_order,mu_derivative_order,
										theta_derivative_order,focus_derivative_order,
										cosh_lambda,sinh_lambda,cos_mu,sin_mu,cos_theta,sin_theta,
										focus);
									calculate_prolate_spheroidal_to_rectangular_cartesian_derivative(
										new_matrix(1,column),2,
										lambda_derivative_order,mu_derivative_order,
										theta_derivative_order,focus_derivative_order,
										cosh_lambda,sinh_lambda,cos_mu,sin_mu,cos_theta,sin_theta,
										focus);
									calculate_prolate_spheroidal_to_rectangular_cartesian_derivative(
										new_matrix(2,column),3,
										lambda_derivative_order,mu_derivative_order,
										theta_derivative_order,focus_derivative_order,
										cosh_lambda,sinh_lambda,cos_mu,sin_mu,cos_theta,sin_theta,
										focus);
								}
								else
								{
									calculate_prolate_spheroidal_to_rectangular_cartesian_derivative(
										new_matrix(0,column),
										rectangular_cartesian_component,
										lambda_derivative_order,mu_derivative_order,
										theta_derivative_order,focus_derivative_order,
										cosh_lambda,sinh_lambda,cos_mu,sin_mu,cos_theta,sin_theta,
										focus);
								}
							}
							else
							{
								new_matrix(0,column)=0;
								if (0==rectangular_cartesian_component)
								{
									new_matrix(1,column)=0;
									new_matrix(2,column)=0;
								}
							}
							// move to next column
							i=new_matrix_independent_variables.size();
							new_matrix_independent_variables_iterator=
								new_matrix_independent_variables.end();
							if (i>0)
							{
								--i;
								--new_matrix_independent_variables_iterator;
								if ((variable_prolate_spheroidal=boost::dynamic_pointer_cast<
									Function_variable_matrix_prolate_spheroidal,
									Function_variable>(
									*atomic_independent_variable_iterators[i]))&&equivalent(
									function_prolate_spheroidal_to_rectangular_cartesian,
									variable_prolate_spheroidal->function()))
								{
									switch (variable_prolate_spheroidal->row())
									{
										case 1:
										{
											--lambda_derivative_order;
										} break;
										case 2:
										{
											--mu_derivative_order;
										} break;
										case 3:
										{
											--theta_derivative_order;
										} break;
									}
								}
								else if ((variable_focus=boost::dynamic_pointer_cast<
									Function_variable_matrix_focus,Function_variable>(
									*atomic_independent_variable_iterators[i]))&&equivalent(
									function_prolate_spheroidal_to_rectangular_cartesian,
									variable_focus->function()))
								{
									--focus_derivative_order;
								}
								++atomic_independent_variable_iterators[i];
								while ((atomic_independent_variable_iterators[i]!=
									(*new_matrix_independent_variables_iterator)->end_atomic())&&
									(1!=(*(atomic_independent_variable_iterators[i]))->
									number_differentiable()))
								{
									++atomic_independent_variable_iterators[i];
								}
								while ((i>0)&&
									((*new_matrix_independent_variables_iterator)->end_atomic()==
									atomic_independent_variable_iterators[i]))
								{
									atomic_independent_variable_iterators[i]=
										(*new_matrix_independent_variables_iterator)->
										begin_atomic();
									while ((atomic_independent_variable_iterators[i]!=
										(*new_matrix_independent_variables_iterator)->
										end_atomic())&&
										(1!=(*(atomic_independent_variable_iterators[i]))->
										number_differentiable()))
									{
										++atomic_independent_variable_iterators[i];
									}
									if ((variable_prolate_spheroidal=boost::dynamic_pointer_cast<
										Function_variable_matrix_prolate_spheroidal,
										Function_variable>(
										*atomic_independent_variable_iterators[i]))&&equivalent(
										function_prolate_spheroidal_to_rectangular_cartesian,
										variable_prolate_spheroidal->function()))
									{
										switch (variable_prolate_spheroidal->row())
										{
											case 1:
											{
												++lambda_derivative_order;
											} break;
											case 2:
											{
												++mu_derivative_order;
											} break;
											case 3:
											{
												++theta_derivative_order;
											} break;
										}
									}
									else if ((variable_focus=boost::dynamic_pointer_cast<
										Function_variable_matrix_focus,Function_variable>(
										*atomic_independent_variable_iterators[i]))&&equivalent(
										function_prolate_spheroidal_to_rectangular_cartesian,
										variable_focus->function()))
									{
										++focus_derivative_order;
									}
									--i;
									if ((variable_prolate_spheroidal=boost::dynamic_pointer_cast<
										Function_variable_matrix_prolate_spheroidal,
										Function_variable>(
										*atomic_independent_variable_iterators[i]))&&equivalent(
										function_prolate_spheroidal_to_rectangular_cartesian,
										variable_prolate_spheroidal->function()))
									{
										switch (variable_prolate_spheroidal->row())
										{
											case 1:
											{
												--lambda_derivative_order;
											} break;
											case 2:
											{
												--mu_derivative_order;
											} break;
											case 3:
											{
												--theta_derivative_order;
											} break;
										}
									}
									else if ((variable_focus=boost::dynamic_pointer_cast<
										Function_variable_matrix_focus,Function_variable>(
										*atomic_independent_variable_iterators[i]))&&equivalent(
										function_prolate_spheroidal_to_rectangular_cartesian,
										variable_focus->function()))
									{
										--focus_derivative_order;
									}
									++atomic_independent_variable_iterators[i];
									--new_matrix_independent_variables_iterator;
									while ((atomic_independent_variable_iterators[i]!=
										(*new_matrix_independent_variables_iterator)->
										end_atomic())&&
										(1!=(*(atomic_independent_variable_iterators[i]))->
										number_differentiable()))
									{
										++atomic_independent_variable_iterators[i];
									}
								}
								if ((*new_matrix_independent_variables_iterator)->end_atomic()!=
									atomic_independent_variable_iterators[i])
								{
									if ((variable_prolate_spheroidal=boost::dynamic_pointer_cast<
										Function_variable_matrix_prolate_spheroidal,
										Function_variable>(
										*atomic_independent_variable_iterators[i]))&&equivalent(
										function_prolate_spheroidal_to_rectangular_cartesian,
										variable_prolate_spheroidal->function()))
									{
										switch (variable_prolate_spheroidal->row())
										{
											case 1:
											{
												++lambda_derivative_order;
											} break;
											case 2:
											{
												++mu_derivative_order;
											} break;
											case 3:
											{
												++theta_derivative_order;
											} break;
										}
									}
									else if ((variable_focus=boost::dynamic_pointer_cast<
										Function_variable_matrix_focus,Function_variable>(
										*atomic_independent_variable_iterators[i]))&&equivalent(
										function_prolate_spheroidal_to_rectangular_cartesian,
										variable_focus->function()))
									{
										++focus_derivative_order;
									}
								}
							}
							++column;
						} while ((new_matrix_independent_variables.front())->end_atomic()!=
							atomic_independent_variable_iterators[0]);
					}
					matrices.push_back(new_matrix);
					matrix_independent_variables.push_back(
						new_matrix_independent_variables);
					++matrix_independent_variables_iterator;
				}
			}
			derivative_matrix=Derivative_matrix(matrices);
			set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
			result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
		}
	}
#if defined (EVALUATE_RETURNS_VALUE)
	if (evaluated())
	{
		result=get_value(atomic_variable);
	}
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)

	return (result);
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)


// global classes
// ==============

// class Function_prolate_spheroidal_to_rectangular_cartesian
// ----------------------------------------------------------

Function_prolate_spheroidal_to_rectangular_cartesian::
	Function_prolate_spheroidal_to_rectangular_cartesian(const Scalar lambda,
	const Scalar mu,const Scalar theta,const Scalar focus):Function(),
	number_of_components_private(3),focus_private(focus),lambda_private(lambda),
	mu_private(mu),theta_private(theta),x_private(0),y_private(0),z_private(0){}
//******************************************************************************
// LAST MODIFIED : 22 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Function_prolate_spheroidal_to_rectangular_cartesian::
	~Function_prolate_spheroidal_to_rectangular_cartesian()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

string_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	get_string_representation()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << "prolate spheroidal to rectangular cartesian";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	input()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_composite(
		Function_variable_handle(new Function_variable_matrix_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),0)),
		Function_variable_handle(new Function_variable_matrix_focus(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this))))));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	output()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
		Function_variable_matrix_rectangular_cartesian(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this))));
}

bool Function_prolate_spheroidal_to_rectangular_cartesian::operator==(
	const Function& function) const
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// Equality operator.
//==============================================================================
{
	bool result;

	result=false;
	if (this)
	{
		result=(typeid(*this)==typeid(function));
	}

	return (result);
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	component(std::string component_name)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the component output.
//==============================================================================
{
	return (Function_variable_handle(new
		Function_variable_matrix_rectangular_cartesian(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),
		component_name)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	component(Function_size_type component_number)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the component output.
//==============================================================================
{
	return (Function_variable_handle(new
		Function_variable_matrix_rectangular_cartesian(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),
		component_number)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	focus()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_focus(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this))));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	lambda()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
		Function_variable_matrix_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),1)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	mu()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
		Function_variable_matrix_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),2)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	prolate()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
		Function_variable_matrix_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),0)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	theta()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
		Function_variable_matrix_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),3)));
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::focus_value()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (focus_private);
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::lambda_value()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (lambda_private);
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::mu_value()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (mu_private);
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::theta_value()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (theta_private);
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::x_value()
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (x_private);
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::y_value()
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (y_private);
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::z_value()
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (z_private);
}

Function_size_type Function_prolate_spheroidal_to_rectangular_cartesian::
	number_of_components()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (number_of_components_private);
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_prolate_spheroidal_to_rectangular_cartesian::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_variable_matrix_rectangular_cartesian_handle
		atomic_variable_rectangular_cartesian;

	if (this&&(atomic_variable_rectangular_cartesian=boost::dynamic_pointer_cast<
		Function_variable_matrix_rectangular_cartesian,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_rectangular_cartesian->function())&&
		(0<atomic_variable_rectangular_cartesian->row_private)&&
		(atomic_variable_rectangular_cartesian->row_private<=
		number_of_components()))
	{
#if defined (BEFORE_CACHING)
#if defined (EVALUATE_RETURNS_VALUE)
		Matrix result_matrix(1,1);
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)

#if defined (EVALUATE_RETURNS_VALUE)
		result_matrix(0,0)=0;
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
		switch (atomic_variable_rectangular_cartesian->row_private)
		{
			case 1:
			{
				x_private=(Scalar)((double)focus_private*
					cosh((double)lambda_private)*cos((double)mu_private));
#if defined (EVALUATE_RETURNS_VALUE)
				result_matrix(0,0)=x_private;
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
			} break;
			case 2:
			{
				y_private=(Scalar)((double)focus_private*
					sinh((double)lambda_private)*sin((double)mu_private)*
					cos((double)theta_private));
#if defined (EVALUATE_RETURNS_VALUE)
				result_matrix(0,0)=y_private;
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
			} break;
			case 3:
			{
				z_private=(Scalar)((double)focus_private*
					sinh((double)lambda_private)*sin((double)mu_private)*
					sin((double)theta_private));
#if defined (EVALUATE_RETURNS_VALUE)
				result_matrix(0,0)=z_private;
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
			} break;
		}
#if defined (EVALUATE_RETURNS_VALUE)
		result=Function_handle(new Function_matrix<Scalar>(result_matrix));
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
#else // defined (BEFORE_CACHING)
		if (!evaluated())
		{
			double temp_double;

			x_private=(Scalar)((double)focus_private*
				cosh((double)lambda_private)*cos((double)mu_private));
			temp_double=(double)focus_private*
				sinh((double)lambda_private)*sin((double)mu_private);
			y_private=(Scalar)(temp_double*cos((double)theta_private));
			z_private=(Scalar)(temp_double*sin((double)theta_private));
			set_evaluated();
		}
#if defined (EVALUATE_RETURNS_VALUE)
		if (evaluated())
		{
			Matrix result_matrix(1,1);

			result_matrix(0,0)=0;
			switch (atomic_variable_rectangular_cartesian->row_private)
			{
				case 1:
				{
					result_matrix(0,0)=x_private;
				} break;
				case 2:
				{
					result_matrix(0,0)=y_private;
				} break;
				case 3:
				{
					result_matrix(0,0)=z_private;
				} break;
			}
			result=Function_handle(new Function_matrix<Scalar>(result_matrix));
		}
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (BEFORE_CACHING)
	}
	else
	{
		result=get_value(atomic_variable);
	}

	return (result);
}

bool Function_prolate_spheroidal_to_rectangular_cartesian::evaluate_derivative(
	Scalar&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	derivative
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	,Function_variable_handle
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	atomic_variable
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	,std::list<Function_variable_handle>&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	atomic_independent_variables
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	)
//******************************************************************************
// LAST MODIFIED : 28 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result(false);
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	Function_variable_matrix_focus_handle atomic_variable_focus;
	Function_variable_matrix_prolate_spheroidal_handle
		atomic_variable_prolate_spheroidal;
	Function_variable_matrix_rectangular_cartesian_handle
		atomic_variable_rectangular_cartesian;

	if (this)
	{
		if ((atomic_variable_prolate_spheroidal=boost::dynamic_pointer_cast<
			Function_variable_matrix_prolate_spheroidal,Function_variable>(
			atomic_variable))&&equivalent(Function_handle(this),
			atomic_variable_prolate_spheroidal->function())&&
			(0<atomic_variable_prolate_spheroidal->row_private))
		{
			Function_variable_matrix_prolate_spheroidal_handle
				atomic_independent_variable;

			result=true;
			if ((1==atomic_independent_variables.size())&&
				(atomic_independent_variable=boost::dynamic_pointer_cast<
				Function_variable_matrix_prolate_spheroidal,Function_variable>(
				atomic_independent_variables.front()))&&
				equivalent(atomic_variable_prolate_spheroidal,
				atomic_independent_variable))
			{
				derivative=1;
			}
			else
			{
				derivative=0;
			}
		}
		else if ((atomic_variable_focus=boost::dynamic_pointer_cast<
			Function_variable_matrix_focus,Function_variable>(atomic_variable))&&
			equivalent(Function_handle(this),atomic_variable_focus->function()))
		{
			Function_variable_matrix_focus_handle atomic_independent_variable;

			result=true;
			if ((1==atomic_independent_variables.size())&&
				(atomic_independent_variable=boost::dynamic_pointer_cast<
				Function_variable_matrix_focus,Function_variable>(
				atomic_independent_variables.front()))&&
				equivalent(atomic_variable_focus,atomic_independent_variable))
			{
				derivative=1;
			}
			else
			{
				derivative=0;
			}
		}
		else if ((atomic_variable_rectangular_cartesian=boost::dynamic_pointer_cast<
			Function_variable_matrix_rectangular_cartesian,Function_variable>(
			atomic_variable))&&equivalent(Function_handle(this),
			atomic_variable_rectangular_cartesian->function())&&
			(0<atomic_variable_rectangular_cartesian->row_private))
		{
			bool zero_derivative;
			Function_variable_matrix_focus_handle variable_focus;
			Function_variable_matrix_prolate_spheroidal_handle
				variable_prolate_spheroidal;
			Function_size_type focus_derivative_order,i,lambda_derivative_order,
				mu_derivative_order,theta_derivative_order;
			std::list<Function_variable_handle>::iterator
				independent_variable_iterator;

			// check for zero derivative and calculate derivative orders
			zero_derivative=false;
			lambda_derivative_order=0;
			mu_derivative_order=0;
			theta_derivative_order=0;
			focus_derivative_order=0;
			independent_variable_iterator=atomic_independent_variables.begin();
			i=atomic_independent_variables.size();
			while (!zero_derivative&&(i>0))
			{
				if ((variable_prolate_spheroidal=boost::dynamic_pointer_cast<
					Function_variable_matrix_prolate_spheroidal,Function_variable>(
					*independent_variable_iterator))&&equivalent(Function_handle(this),
					variable_prolate_spheroidal->function()))
				{
					switch (variable_prolate_spheroidal->row_private)
					{
						case 1:
						{
							++lambda_derivative_order;
						} break;
						case 2:
						{
							++mu_derivative_order;
						} break;
						case 3:
						{
							++theta_derivative_order;
						} break;
						default:
						{
							zero_derivative=true;
						} break;
					}
				}
				else if ((variable_focus=boost::dynamic_pointer_cast<
					Function_variable_matrix_focus,Function_variable>(
					*independent_variable_iterator))&&
					equivalent(Function_handle(this),variable_focus->function()))
				{
					++focus_derivative_order;
					if (1<focus_derivative_order)
					{
						zero_derivative=true;
					}
				}
				else
				{
					zero_derivative=true;
				}
				++independent_variable_iterator;
				--i;
			}
			if (zero_derivative)
			{
				result=true;
				derivative=0;
			}
			else
			{
				Scalar focus_derivative;

				// focus_derivative_order<=1 from construction above
				if (0==focus_derivative_order)
				{
					focus_derivative=focus_private;
				}
				else
				{
					focus_derivative=1;
				}
				// assign derivatives
				switch (atomic_variable_rectangular_cartesian->row_private)
				{
					case 1:
					{
						switch (lambda_derivative_order)
						{
							case 0:
							{
								switch (mu_derivative_order)
								{
									case 0:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*
													cosh(lambda_private)*cos(mu_private);
											} break;
											case 1:
											{
												result=true;
												derivative=0;
											} break;
											case 2:
											{
												result=true;
												derivative=0;
											} break;
										}
									} break;
									case 1:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative= -focus_derivative*
													cosh(lambda_private)*sin(mu_private);
											} break;
											case 1:
											{
												result=true;
												derivative=0;
											} break;
										}
									} break;
									case 2:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative= -focus_derivative*
													cosh(lambda_private)*cos(mu_private);
											} break;
										}
									} break;
								}
							} break;
							case 1:
							{
								switch (mu_derivative_order)
								{
									case 0:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*
													sinh(lambda_private)*cos(mu_private);
											} break;
											case 1:
											{
												result=true;
												derivative=0;
											} break;
										}
									} break;
									case 1:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative= -focus_derivative*
													sinh(lambda_private)*sin(mu_private);
											} break;
										}
									} break;
								}
							} break;
							case 2:
							{
								// mu order
								switch (mu_derivative_order)
								{
									case 0:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*
													cosh(lambda_private)*cos(mu_private);
											} break;
										}
									} break;
								}
							} break;
						}
					} break;
					case 2:
					{
						switch (lambda_derivative_order)
						{
							case 0:
							{
								switch (mu_derivative_order)
								{
									case 0:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*sinh(lambda_private)*
													sin(mu_private)*cos(theta_private);
											} break;
											case 1:
											{
												result=true;
												derivative= -focus_derivative*sinh(lambda_private)*
													sin(mu_private)*sin(theta_private);
											} break;
											case 2:
											{
												result=true;
												derivative= -focus_derivative*sinh(lambda_private)*
													sin(mu_private)*cos(theta_private);
											} break;
										}
									} break;
									case 1:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*sinh(lambda_private)*
													cos(mu_private)*cos(theta_private);
											} break;
											case 1:
											{
												result=true;
												derivative= -focus_derivative*sinh(lambda_private)*
													cos(mu_private)*sin(theta_private);
											} break;
										}
									} break;
									case 2:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative= -focus_derivative*sinh(lambda_private)*
													sin(mu_private)*cos(theta_private);
											} break;
										}
									} break;
								}
							} break;
							case 1:
							{
								switch (mu_derivative_order)
								{
									case 0:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*cosh(lambda_private)*
													sin(mu_private)*cos(theta_private);
											} break;
											case 1:
											{
												result=true;
												derivative= -focus_derivative*cosh(lambda_private)*
													sin(mu_private)*sin(theta_private);
											} break;
										}
									} break;
									case 1:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*cosh(lambda_private)*
													cos(mu_private)*cos(theta_private);
											} break;
										}
									} break;
								}
							} break;
							case 2:
							{
								// mu order
								switch (mu_derivative_order)
								{
									case 0:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*sinh(lambda_private)*
													sin(mu_private)*cos(theta_private);
											} break;
										}
									} break;
								}
							} break;
						}
					} break;
					case 3:
					{
						switch (lambda_derivative_order)
						{
							case 0:
							{
								switch (mu_derivative_order)
								{
									case 0:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*sinh(lambda_private)*
													sin(mu_private)*sin(theta_private);
											} break;
											case 1:
											{
												result=true;
												derivative=focus_derivative*sinh(lambda_private)*
													sin(mu_private)*cos(theta_private);
											} break;
											case 2:
											{
												result=true;
												derivative= -focus_derivative*sinh(lambda_private)*
													sin(mu_private)*sin(theta_private);
											} break;
										}
									} break;
									case 1:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*sinh(lambda_private)*
													cos(mu_private)*sin(theta_private);
											} break;
											case 1:
											{
												result=true;
												derivative=focus_derivative*sinh(lambda_private)*
													cos(mu_private)*cos(theta_private);
											} break;
										}
									} break;
									case 2:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative= -focus_derivative*sinh(lambda_private)*
													sin(mu_private)*sin(theta_private);
											} break;
										}
									} break;
								}
							} break;
							case 1:
							{
								switch (mu_derivative_order)
								{
									case 0:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*cosh(lambda_private)*
													sin(mu_private)*sin(theta_private);
											} break;
											case 1:
											{
												result=true;
												derivative=focus_derivative*cosh(lambda_private)*
													sin(mu_private)*cos(theta_private);
											} break;
										}
									} break;
									case 1:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*cosh(lambda_private)*
													cos(mu_private)*sin(theta_private);
											} break;
										}
									} break;
								}
							} break;
							case 2:
							{
								// mu order
								switch (mu_derivative_order)
								{
									case 0:
									{
										switch (theta_derivative_order)
										{
											case 0:
											{
												result=true;
												derivative=focus_derivative*sinh(lambda_private)*
													sin(mu_private)*sin(theta_private);
											} break;
										}
									} break;
								}
							} break;
						}
					} break;
					default:
					{
						result=true;
						derivative=0;
					} break;
				}
			}
		}
	}
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	// should not come here - handled by
	//   Function_derivatnew_matrix_rectangular_cartesian::evaluate
	Assert(false,std::logic_error(
		"Function_prolate_spheroidal_to_rectangular_cartesian::evaluate_derivative.  Should not come here"));
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

	return (result);
}

bool Function_prolate_spheroidal_to_rectangular_cartesian::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_matrix_focus_handle atomic_focus_variable;
	Function_variable_matrix_prolate_spheroidal_handle
		atomic_prolate_spheroidal_variable;
	Function_variable_matrix_rectangular_cartesian_handle
		atomic_rectangular_cartesian_variable;
	Function_variable_value_scalar_handle value_scalar;

	result=false;
	if (this)
	{
		if ((atomic_prolate_spheroidal_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_prolate_spheroidal,Function_variable>(
			atomic_variable))&&equivalent(Function_handle(this),
			atomic_prolate_spheroidal_variable->function())&&
			(0<atomic_prolate_spheroidal_variable->row_private)&&atomic_value&&
			(atomic_value->value())&&
			(std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			switch (atomic_prolate_spheroidal_variable->row_private)
			{
				case 1:
				{
					result=value_scalar->set(lambda_private,atomic_value);
				} break;
				case 2:
				{
					result=value_scalar->set(mu_private,atomic_value);
				} break;
				case 3:
				{
					result=value_scalar->set(theta_private,atomic_value);
				} break;
			}
		}
		else if ((atomic_focus_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_focus,Function_variable>(atomic_variable))&&
			equivalent(Function_handle(this),atomic_focus_variable->function())&&
			atomic_value&&(atomic_value->value())&&
			(std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			result=value_scalar->set(focus_private,atomic_value);
		}
		else if ((atomic_rectangular_cartesian_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_rectangular_cartesian,Function_variable>(
			atomic_variable))&&equivalent(Function_handle(this),
			atomic_rectangular_cartesian_variable->function())&&
			(0<atomic_rectangular_cartesian_variable->row_private)&&atomic_value&&
			(atomic_value->value())&&
			(std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			switch (atomic_rectangular_cartesian_variable->row_private)
			{
				case 1:
				{
					result=value_scalar->set(x_private,atomic_value);
				} break;
				case 2:
				{
					result=value_scalar->set(y_private,atomic_value);
				} break;
				case 3:
				{
					result=value_scalar->set(z_private,atomic_value);
				} break;
			}
		}
		if (result)
		{
			set_not_evaluated();
		}
	}

	return (result);
}

Function_handle Function_prolate_spheroidal_to_rectangular_cartesian::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_matrix_focus_handle atomic_variable_focus;
	Function_variable_matrix_prolate_spheroidal_handle
		atomic_variable_prolate_spheroidal;
	Function_variable_matrix_rectangular_cartesian_handle
		atomic_variable_rectangular_cartesian;

	if (this)
	{
		if ((atomic_variable_prolate_spheroidal=boost::dynamic_pointer_cast<
			Function_variable_matrix_prolate_spheroidal,Function_variable>(
			atomic_variable))&&equivalent(Function_handle(this),
			atomic_variable_prolate_spheroidal->function()))
		{
			Matrix result_matrix(1,1);

			if (atomic_variable_prolate_spheroidal->get_entry(result_matrix(0,0)))
			{
				result=Function_handle(new Function_matrix<Scalar>(result_matrix));
			}
		}
		if ((atomic_variable_focus=boost::dynamic_pointer_cast<
			Function_variable_matrix_focus,Function_variable>(atomic_variable))&&
			equivalent(Function_handle(this),atomic_variable_focus->function()))
		{
			Matrix result_matrix(1,1);

			if (atomic_variable_focus->get_entry(result_matrix(0,0)))
			{
				result=Function_handle(new Function_matrix<Scalar>(result_matrix));
			}
		}
		else if ((atomic_variable_rectangular_cartesian=boost::dynamic_pointer_cast<
			Function_variable_matrix_rectangular_cartesian,Function_variable>(
			atomic_variable))&&equivalent(Function_handle(this),
			atomic_variable_rectangular_cartesian->function()))
		{
			Matrix result_matrix(1,1);

			if (atomic_variable_rectangular_cartesian->get_entry(result_matrix(0,0)))
			{
				result=Function_handle(new Function_matrix<Scalar>(result_matrix));
			}
		}
	}

	return (result);
}

Function_prolate_spheroidal_to_rectangular_cartesian::
	Function_prolate_spheroidal_to_rectangular_cartesian(
	const Function_prolate_spheroidal_to_rectangular_cartesian& function):
	Function(),
	number_of_components_private(function.number_of_components_private),
	focus_private(function.focus_private),lambda_private(function.lambda_private),
	mu_private(function.mu_private),theta_private(function.theta_private),
	x_private(function.x_private),y_private(function.y_private),
	z_private(function.z_private){}
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

Function_prolate_spheroidal_to_rectangular_cartesian&
	Function_prolate_spheroidal_to_rectangular_cartesian::operator=(
	const Function_prolate_spheroidal_to_rectangular_cartesian& function)
//******************************************************************************
// LAST MODIFIED : 22 June 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	lambda_private=function.lambda_private;
	mu_private=function.mu_private;
	theta_private=function.theta_private;
	focus_private=function.focus_private;
	x_private=function.x_private;
	y_private=function.y_private;
	z_private=function.z_private;

	return (*this);
}
