//******************************************************************************
// FILE : variable_inverse.cpp
//
// LAST MODIFIED : 9 February 2004
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

#include "computed_variable/variable_base.hpp"

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

#include "computed_variable/variable_derivative_matrix.hpp"
#include "computed_variable/variable_composite.hpp"
#include "computed_variable/variable_identity.hpp"
#if defined (USE_VARIABLE_INPUT)
#include "computed_variable/variable_input_composite.hpp"
#if defined (GENERALIZE_COMPOSITE_INPUT)
#include "computed_variable/variable_input_composite_union.hpp"
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
#else // defined (USE_VARIABLE_INPUT)
#include "computed_variable/variable_io_specifier_composite.hpp"
#endif // defined (USE_VARIABLE_INPUT)
#include "computed_variable/variable_inverse.hpp"
#include "computed_variable/variable_scalar.hpp"
#include "computed_variable/variable_vector.hpp"

// module classes
// ==============

// class Variable_input_independent
// --------------------------------

class Variable_input_independent;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_independent>
	Variable_input_independent_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_independent>
	Variable_input_independent_handle;
#else
typedef Variable_input_independent * Variable_input_independent_handle;
#endif

class Variable_input_independent : public
#if defined (USE_VARIABLE_INPUT)
	Variable_input
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_inverse;
	public:
		Variable_input_independent(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& dependent_variable,
			const Variable_handle& independent_variable):
			independent_variable(independent_variable),
			dependent_variable(dependent_variable){};
		~Variable_input_independent(){};
#if defined (USE_ITERATORS)
		// copy constructor
		Variable_input_independent(
			const Variable_input_independent& input_independent):
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			(),
			independent_variable(input_independent.independent_variable),
			dependent_variable(input_independent.dependent_variable){};
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			clone() const
		{
			return (Variable_input_independent_handle(
				new Variable_input_independent(*this)));
		}
		//???DB.  To be done
		virtual bool is_atomic();
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs();
		virtual Iterator end_atomic_inputs();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs();
		virtual Variable_input_iterator end_atomic_inputs();
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#if defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs();
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs();
#else // defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic();
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic();
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
		Variable_size_type number_differentiable()
		{
			return (independent_variable->number_differentiable());
		};
#else // defined (USE_ITERATORS)
		Variable_size_type size()
		{
			return (independent_variable->size());
		};
#endif // defined (USE_ITERATORS)
		virtual bool operator==(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			& input)
		{
			try
			{
				const Variable_input_independent& input_independent=
					dynamic_cast<const Variable_input_independent&>(input);

				return (
					(input_independent.independent_variable==independent_variable)&&
					(input_independent.dependent_variable)&&dependent_variable&&
					(*(input_independent.dependent_variable)== *dependent_variable));
			}
			catch (std::bad_cast)
			{
				return (false);
			}
		};
	private:
		Variable_handle independent_variable;
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			dependent_variable;
};

// class Variable_input_value_tolerance
// ------------------------------------

class Variable_input_value_tolerance;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_value_tolerance>
	Variable_input_value_tolerance_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_value_tolerance>
	Variable_input_value_tolerance_handle;
#else
typedef Variable_input_value_tolerance *
	Variable_input_value_tolerance_handle;
#endif

class Variable_input_value_tolerance : public
#if defined (USE_VARIABLE_INPUT)
	Variable_input
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_inverse;
	public:
		Variable_input_value_tolerance(
			const Variable_inverse_handle& variable_inverse):
			variable_inverse(variable_inverse){};
		~Variable_input_value_tolerance(){};
#if defined (USE_ITERATORS)
		// copy constructor
		Variable_input_value_tolerance(
			const Variable_input_value_tolerance& input_value_tolerance):
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			(),
			variable_inverse(input_value_tolerance.variable_inverse){};
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			clone() const
		{
			return (Variable_input_value_tolerance_handle(
				new Variable_input_value_tolerance(*this)));
		}
		//???DB.  To be done
		virtual bool is_atomic();
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs();
		virtual Iterator end_atomic_inputs();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs();
		virtual Variable_input_iterator end_atomic_inputs();
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#if defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs();
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs();
#else // defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic();
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic();
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)
		virtual Variable_size_type
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			()
		{
			return (1);
		};
		virtual bool operator==(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			& input)
		{
			try
			{
				const Variable_input_value_tolerance& input_value_tolerance=
					dynamic_cast<const Variable_input_value_tolerance&>(input);

				return (input_value_tolerance.variable_inverse==variable_inverse);
			}
			catch (std::bad_cast)
			{
				return (false);
			}
		};
	private:
		Variable_inverse_handle variable_inverse;
};

// class Variable_input_step_tolerance
// -----------------------------------

class Variable_input_step_tolerance;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_step_tolerance>
	Variable_input_step_tolerance_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_step_tolerance>
	Variable_input_step_tolerance_handle;
#else
typedef Variable_input_step_tolerance *
	Variable_input_step_tolerance_handle;
#endif

class Variable_input_step_tolerance : public
#if defined (USE_VARIABLE_INPUT)
	Variable_input
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_inverse;
	public:
		Variable_input_step_tolerance(
			const Variable_inverse_handle& variable_inverse):
			variable_inverse(variable_inverse){};
		~Variable_input_step_tolerance(){};
#if defined (USE_ITERATORS)
		// copy constructor
		Variable_input_step_tolerance(
			const Variable_input_step_tolerance& input_step_tolerance):
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			(),
			variable_inverse(input_step_tolerance.variable_inverse){};
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			clone() const
		{
			return (Variable_input_step_tolerance_handle(
				new Variable_input_step_tolerance(*this)));
		}
		//???DB.  To be done
		virtual bool is_atomic();
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs();
		virtual Iterator end_atomic_inputs();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs();
		virtual Variable_input_iterator end_atomic_inputs();
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#if defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs();
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs();
#else // defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic();
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic();
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)
		virtual Variable_size_type
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			()
		{
			return (1);
		};
		virtual bool operator==(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			& input)
		{
			try
			{
				const Variable_input_step_tolerance& input_step_tolerance=
					dynamic_cast<const Variable_input_step_tolerance&>(input);

				return (input_step_tolerance.variable_inverse==variable_inverse);
			}
			catch (std::bad_cast)
			{
				return (false);
			}
		};
	private:
		Variable_inverse_handle variable_inverse;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_step_tolerance>
	Variable_input_step_tolerance_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_step_tolerance>
	Variable_input_step_tolerance_handle;
#else
typedef Variable_input_step_tolerance *
	Variable_input_step_tolerance_handle;
#endif

// class Variable_input_maximum_iterations
// ---------------------------------------

class Variable_input_maximum_iterations;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_maximum_iterations>
	Variable_input_maximum_iterations_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_maximum_iterations>
	Variable_input_maximum_iterations_handle;
#else
typedef Variable_input_maximum_iterations *
	Variable_input_maximum_iterations_handle;
#endif

class Variable_input_maximum_iterations : public
#if defined (USE_VARIABLE_INPUT)
	Variable_input
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_inverse;
	public:
		Variable_input_maximum_iterations(
			const Variable_inverse_handle& variable_inverse):
			variable_inverse(variable_inverse){};
		~Variable_input_maximum_iterations(){};
#if defined (USE_ITERATORS)
		// copy constructor
		Variable_input_maximum_iterations(
			const Variable_input_maximum_iterations& input_maximum_iterations):
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			(),
			variable_inverse(input_maximum_iterations.variable_inverse){};
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			clone() const
		{
			return (Variable_input_maximum_iterations_handle(
				new Variable_input_maximum_iterations(*this)));
		}
		//???DB.  To be done
		virtual bool is_atomic();
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs();
		virtual Iterator end_atomic_inputs();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs();
		virtual Variable_input_iterator end_atomic_inputs();
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#if defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs();
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs();
#else // defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic();
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic();
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
		virtual Variable_size_type number_differentiable()
		{
			return (0);
		};
#else // defined (USE_ITERATORS)
		virtual Variable_size_type size()
		{
			return (1);
		};
#endif // defined (USE_ITERATORS)
		virtual bool operator==(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			& input)
		{
			try
			{
				const Variable_input_maximum_iterations& input_maximum_iterations=
					dynamic_cast<const Variable_input_maximum_iterations&>(input);

				return (input_maximum_iterations.variable_inverse==variable_inverse);
			}
			catch (std::bad_cast)
			{
				return (false);
			}
		};
	private:
		Variable_inverse_handle variable_inverse;
};

// class Variable_input_dependent_estimate
// ---------------------------------------

class Variable_input_dependent_estimate;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_dependent_estimate>
	Variable_input_dependent_estimate_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_dependent_estimate>
	Variable_input_dependent_estimate_handle;
#else
typedef Variable_input_dependent_estimate *
	Variable_input_dependent_estimate_handle;
#endif

class Variable_input_dependent_estimate : public
#if defined (USE_VARIABLE_INPUT)
	Variable_input
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_inverse;
	public:
		Variable_input_dependent_estimate(
			const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& dependent_variable,
			const Variable_handle& independent_variable):
			independent_variable(independent_variable),
			dependent_variable(dependent_variable){};
		~Variable_input_dependent_estimate(){};
#if defined (USE_ITERATORS)
		// copy constructor
		Variable_input_dependent_estimate(
			const Variable_input_dependent_estimate& input_dependent_estimate):
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			(),
			independent_variable(input_dependent_estimate.independent_variable),
			dependent_variable(input_dependent_estimate.dependent_variable){};
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			clone() const
		{
			return (Variable_input_dependent_estimate_handle(
				new Variable_input_dependent_estimate(*this)));
		}
		//???DB.  To be done
		virtual bool is_atomic();
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs();
		virtual Iterator end_atomic_inputs();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs();
		virtual Variable_input_iterator end_atomic_inputs();
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#if defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs();
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs();
#else // defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic();
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic();
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
		Variable_size_type number_differentiable()
		{
			return (dependent_variable->number_differentiable());
		};
#else // defined (USE_ITERATORS)
		Variable_size_type size()
		{
			return (dependent_variable->size());
		};
#endif // defined (USE_ITERATORS)
		virtual bool operator==(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			& input)
		{
			try
			{
				const Variable_input_dependent_estimate& input_dependent_estimate=
					dynamic_cast<const Variable_input_dependent_estimate&>(input);

				return ((input_dependent_estimate.independent_variable==
					independent_variable)&&(input_dependent_estimate.dependent_variable)&&
					dependent_variable&&(*(input_dependent_estimate.dependent_variable)==
					*dependent_variable));
			}
			catch (std::bad_cast)
			{
				return (false);
			}
		};
	private:
		Variable_handle independent_variable;
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			dependent_variable;
};


// global classes
// ==============

// class Variable_inverse
// ----------------------

Variable_inverse::Variable_inverse(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& dependent_variable,
	Variable_handle& independent_variable):step_tolerance(0),
	value_tolerance(0),dependent_variable_estimate(0),independent_value(0),
	independent_variable(independent_variable),
	dependent_variable(dependent_variable),maximum_iterations(0)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
}

Variable_inverse::Variable_inverse(const Variable_inverse& variable_inverse):
	Variable(),step_tolerance(variable_inverse.value_tolerance),
	value_tolerance(variable_inverse.step_tolerance),
	dependent_variable_estimate(variable_inverse.dependent_variable_estimate),
	independent_value(variable_inverse.independent_value),
	independent_variable(variable_inverse.independent_variable),
	dependent_variable(variable_inverse.dependent_variable),
	maximum_iterations(variable_inverse.maximum_iterations)
//******************************************************************************
// LAST MODIFIED : 16 December 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_inverse& Variable_inverse::operator=(
	const Variable_inverse& variable_inverse)
//******************************************************************************
// LAST MODIFIED : 16 December 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	step_tolerance=variable_inverse.step_tolerance;
	value_tolerance=variable_inverse.value_tolerance;
	dependent_variable_estimate=variable_inverse.dependent_variable_estimate;
	dependent_variable=variable_inverse.dependent_variable;
	independent_value=variable_inverse.independent_value;
	independent_variable=variable_inverse.independent_variable;
	maximum_iterations=variable_inverse.maximum_iterations;

	return (*this);
}

Variable_inverse::~Variable_inverse()
//******************************************************************************
// LAST MODIFIED : 4 December 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

#if defined (OLD_CODE)
Variable_handle Variable_inverse::evaluate_derivative(
	std::list<Variable_input_handle>& independent_variables,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 17 December 2003
//
// DESCRIPTION :
// Overload Variable::evaluate_derivative
//==============================================================================
{
	Variable_handle result(0);
	Variable_size_type i;

	i=independent_variables.size();
	if (this&&(0<i))
	{
		bool valid_independent_variables;
		std::list<Variable_input_handle>::iterator independent_variables_iterator;
		std::list<Variable_input_handle> intermediate_independent_variables(0);
		std::list<Variable_input_handle> restricted_independent_variables(0);
		Variable_input_handle this_input_independent;
		Variable_size_type number_of_matrices;

		valid_independent_variables=true;
		independent_variables_iterator=independent_variables.begin();
		number_of_matrices=0;
		this_input_independent=input_independent();
		while (valid_independent_variables&&(i>0))
		{
			if (dependent_variable&&(*independent_variables_iterator)&&
				(*dependent_variable== **independent_variables_iterator))
			{
				valid_independent_variables=false;
			}
			else
			{
				if (!(this_input_independent&&(*independent_variables_iterator)&&
					(*this_input_independent== **independent_variables_iterator)))
				{
					restricted_independent_variables.push_back(
						*independent_variables_iterator);
				}
			}
			intermediate_independent_variables.push_back(dependent_variable);
			number_of_matrices=2*number_of_matrices+1;
			independent_variables_iterator++;
			i--;
		}
		if (valid_independent_variables)
		{
			std::list<Variable_input_value_handle> values_extended=values;
			Variable_handle dependent_value=evaluate(values);

			values_extended.push_back(Variable_input_value_handle(
				new Variable_input_value(dependent_variable,dependent_value)));
			Variable_derivative_matrix_handle derivative_inverse=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
				((independent_variable->evaluate_derivative)(
				intermediate_independent_variables,values_extended));
			
			if (derivative_inverse&&
				(derivative_inverse=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
				((derivative_inverse->inverse)(Variable_inverse_handle(this)))
				))
			{
				if (0<restricted_independent_variables.size())
				{
					Variable_derivative_matrix_handle derivative_restricted=
#if defined (USE_SMART_POINTER)
						boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
						dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
						((independent_variable->evaluate_derivative)(
						restricted_independent_variables,values_extended));

					if (derivative_restricted)
					{
						// extend derivative_restricted
						std::list<Matrix> matrices(0);
						std::list<Matrix>::iterator matrix_iterator,
							matrix_restricted_iterator;
						ublas::vector<Variable_size_type>
							matrix_input_independent_order(number_of_matrices);
						Variable_size_type column,j,this_input_independent_size,row;

						independent_variables_iterator=independent_variables.begin();
						matrix_restricted_iterator=
							(derivative_restricted->matrices).begin();
						number_of_matrices=0;
						this_input_independent_size=this_input_independent->size();
						for (i=independent_variables.size();i>0;i--)
						{
							if (this_input_independent&&(*independent_variables_iterator)&&
								(*this_input_independent== **independent_variables_iterator))
							{
								Matrix matrix_identity(this_input_independent_size,
									this_input_independent_size);

								for (row=0;row<this_input_independent_size;row++)
								{
									for (column=0;column<this_input_independent_size;column++)
									{
										if (row==column)
										{
											matrix_identity(row,column)=(Scalar)1;
										}
										else
										{
											matrix_identity(row,column)=(Scalar)0;
										}
									}
								}
								matrices.push_back(matrix_identity);
								matrix_input_independent_order[number_of_matrices]=1;
								matrix_iterator=matrices.begin();
								j=0;
								while (j<number_of_matrices)
								{
									Variable_size_type number_of_columns=
										this_input_independent_size*(matrix_iterator->size2());
									Matrix matrix_zero(this_input_independent_size,
										number_of_columns);

									for (row=0;row<this_input_independent_size;row++)
									{
										for (column=0;column<number_of_columns;column++)
										{
											matrix_zero(row,column)=(Scalar)0;
										}
									}
									matrices.push_back(matrix_zero);
									matrix_input_independent_order[number_of_matrices+j+1]=
										matrix_input_independent_order[j]+1;
									matrix_iterator++;
									j++;
								}
							}
							else
							{
								matrices.push_back(*matrix_restricted_iterator);
								matrix_restricted_iterator++;
								matrix_input_independent_order[number_of_matrices]=0;
								matrix_iterator=matrices.begin();
								j=0;
								while (j<number_of_matrices)
								{
									if (0==matrix_input_independent_order[j])
									{
										matrices.push_back(*matrix_restricted_iterator);
										matrix_restricted_iterator++;
									}
									else
									{
										Variable_size_type number_of_columns=
											((*independent_variables_iterator)->size())*
											(matrix_iterator->size2());
										Matrix matrix_zero(this_input_independent_size,
											number_of_columns);
										
										for (row=0;row<this_input_independent_size;row++)
										{
											for (column=0;column<number_of_columns;column++)
											{
												matrix_zero(row,column)=(Scalar)0;
											}
										}
										matrices.push_back(matrix_zero);
									}
									matrix_input_independent_order[number_of_matrices+j+1]=
										matrix_input_independent_order[j];
									matrix_iterator++;
									j++;
								}
							}
							number_of_matrices=2*number_of_matrices+1;
							independent_variables_iterator++;
						}
						//???DB.  Temporary, only works for 1st order derivative. begin
						matrix_iterator=matrices.begin();
						for (i=matrices.size();i>0;i--)
						{
							Matrix& matrix_temp= *matrix_iterator;
							Variable_size_type column,row;

							for (row=0;row<matrix_temp.size1();row++)
							{
								for (column=0;column<matrix_temp.size2();column++)
								{
									matrix_temp(row,column)= -matrix_temp(row,column);
								}
							}
							matrix_iterator++;
						}
						//???DB.  Temporary, only works for 1st order derivative. end
						result=Variable_derivative_matrix_compose(Variable_handle(this),
							derivative_inverse,Variable_derivative_matrix_handle(
							new Variable_derivative_matrix(independent_variable,
							independent_variables,matrices)));
					}
				}
				else
				{
					result=derivative_inverse;
				}
			}
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)

#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
Variable_handle Variable_inverse::evaluate_derivative(
	std::list<Variable_input_handle>& independent_variables,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 11 January 2004
//
// DESCRIPTION :
// Overload Variable::evaluate_derivative
//==============================================================================
{
	Variable_handle result(0);
	Variable_input_handle input_independent_cache;
	const Variable_size_type number_of_independent_variables=
		independent_variables.size();

#if defined (DEBUG)
	//???debug
	std::cout << "enter Variable_inverse::evaluate_derivative" << std::endl;
#endif // defined (DEBUG)
	if (this&&(input_independent_cache=input_independent())&&dependent_variable&&
		independent_variable&&(0<number_of_independent_variables))
	{
		bool valid_independent_variables;
		std::list<Variable_handle> composite_variables_list(0);
		std::list<Variable_input_handle>::iterator independent_variables_iterator;
		std::list<Variable_input_value_handle> current_values(0),no_values(0);
		std::vector<Variable_size_type>
			numbers_of_independent_values(number_of_independent_variables);
		Variable_input_handle composite_input(0),temp_input(0);
		Variable_size_type i,j;

		// set the values so that sizes are valid
		// get the specified values
		std::for_each(values.begin(),values.end(),Variable_get_input_values(
			Variable_handle(this),current_values));
		// override the specified values
		std::for_each(values.begin(),values.end(),Variable_set_input_values(
			Variable_handle(this)));
		// set up the extended input
		composite_input=dependent_variable;
		valid_independent_variables=true;
		independent_variables_iterator=independent_variables.begin();
		i=0;
		while (valid_independent_variables&&(i<number_of_independent_variables))
		{
			if (!(*independent_variables_iterator)||
				(dependent_variable->intersect)(*independent_variables_iterator))
			{
				valid_independent_variables=false;
			}
			else
			{
				numbers_of_independent_values[i]=
					(*independent_variables_iterator)->size();
				if (temp_input=(**independent_variables_iterator)-
					(*input_independent_cache))
				{
					composite_input=(*composite_input)+(*temp_input);
				}
			}
			independent_variables_iterator++;
			i++;
		}
		if (valid_independent_variables)
		{
			Variable_handle composite_variable(0);
			std::list<Variable_input_handle> composite_independent_variables(
				number_of_independent_variables,composite_input);

			// set up an extended variable
			if (temp_input=(*composite_input)-(*dependent_variable))
			{
				composite_variable=Variable_handle(new Variable_composite(
					independent_variable,Variable_handle(new Variable_identity(
					temp_input))));
			}
			else
			{
				composite_variable=independent_variable;
			}
			if (composite_variable)
			{
				Variable_derivative_matrix_handle derivative_inverse(0);
				Variable_handle current_dependent_value(0),
					dependent_value=evaluate(no_values);

				current_dependent_value=composite_variable->get_input_value(
					dependent_variable);
				composite_variable->set_input_value(dependent_variable,dependent_value);
			
				// evaluate derivative
				derivative_inverse=
#if defined (USE_SMART_POINTER)
					boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
					((composite_variable->evaluate_derivative)(
					composite_independent_variables,no_values));
#if defined (DEBUG)
				//???debug
				std::cout << "derivative=" << (derivative_inverse->matrices).front() <<
					std::endl;
#endif // defined (DEBUG)
				if (derivative_inverse&&
					(derivative_inverse=
#if defined (USE_SMART_POINTER)
					boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
					((derivative_inverse->inverse)(Variable_inverse_handle(new
					Variable_inverse(composite_input,composite_variable))))))
				{
#if defined (DEBUG)
					//???debug
					std::cout << "derivative->inverse=" <<
						(derivative_inverse->matrices).front() << std::endl;
#endif // defined (DEBUG)
					// extract parts of the derivative_inverse matrices
					std::list<Matrix> matrices(0);
					std::list<Matrix>::iterator matrix_iterator;
					std::vector< std::list< std::pair<Variable_size_type,
						Variable_size_type> > > independent_variable_mappings(
						number_of_independent_variables);
					std::vector<Variable_size_type> derivative_index(
						number_of_independent_variables);
					Variable_size_type number_of_matrices=
						(derivative_inverse->matrices).size();
					ublas::matrix<bool> matrix_variables(number_of_matrices,
						number_of_independent_variables);
					Variable_size_type column_1,k,number_of_composite_input_values,
						number_of_dependent_values;

					for (i=0;i<number_of_matrices;i++)
					{
						for (j=0;j<number_of_independent_variables;j++)
						{
							matrix_variables(i,j)=false;
						}
					}
					independent_variables_iterator=independent_variables.begin();
					number_of_dependent_values=dependent_variable->size();
					number_of_composite_input_values=composite_input->size();
					matrix_iterator=(derivative_inverse->matrices).begin();
					number_of_matrices=0;
					for (i=0;i<number_of_independent_variables;i++)
					{
						std::list< std::pair<Variable_size_type,Variable_size_type> >&
							independent_variable_mapping=independent_variable_mappings[i];
						std::list< std::pair<Variable_size_type,Variable_size_type> >::
							iterator independent_variable_mapping_iterator;
						Variable_size_type number_of_columns=
							numbers_of_independent_values[i];
						Matrix matrix_1(number_of_dependent_values,number_of_columns);
						Variable_size_type column,row;

						independent_variable_mapping=(*independent_variables_iterator)->
							scalar_mapping(composite_input);
						Assert((0<independent_variable_mapping.size())&&
							(0==independent_variable_mapping.front().first),
							std::logic_error(
							"Variable_inverse::evaluate_derivative.  "
							"Invalid independent_variable_mapping 1"));
						independent_variable_mapping_iterator=
							independent_variable_mapping.begin();
						column=0;
						for (j=independent_variable_mapping.size()-1;j>0;j--)
						{
							column_1=independent_variable_mapping_iterator->second;
							independent_variable_mapping_iterator++;
							Assert((column<independent_variable_mapping_iterator->first),
								(column_1+(independent_variable_mapping_iterator->first)-column<
								matrix_iterator->size2()),
								"Variable_inverse::evaluate_derivative.  "
								"Invalid independent_variable_mapping 2"));
							while (column<independent_variable_mapping_iterator->first)
							{
								for (row=0;row<number_of_dependent_values;row++)
								{
									matrix_1(row,column)=(*matrix_iterator)(row,column_1);
								}
								column_1++;
								column++;
							}
						}
						matrix_variables(number_of_matrices,i)=true;
						matrices.push_back(matrix_1);
						matrix_iterator++;
						for (j=0;j<number_of_matrices;j++)
						{
							number_of_columns=numbers_of_independent_values[i];
							for (k=0;k<i;k++)
							{
								if (matrix_variables(j,k))
								{
									matrix_variables(j+number_of_matrices+1,k)=true;
									number_of_columns *= numbers_of_independent_values[k];
								}
								derivative_index[k]=0;
							}
							derivative_index[i]=0;
							matrix_variables(j+number_of_matrices+1,i)=true;

							bool finished,found;
							Matrix matrix_2(number_of_dependent_values,number_of_columns);
							Variable_size_type column_2;

							column=0;
							finished=false;
							while (!finished)
							{
								// calculated column of *matrix_iterator
								column_2=0;
								for (k=0;k<=i;k++)
								{
									if (matrix_variables(j+number_of_matrices+1,k))
									{
										column_2 *= number_of_composite_input_values;
										l=independent_variable_mappings[k].size()-1;
										independent_variable_mapping_iterator=
											independent_variable_mapping.begin();
										independent_variable_mapping_iterator++;
										while ((l>0)&&(derivative_index[k]>=
											independent_variable_mapping_iterator->first))
										{
											offset=independent_variable_mapping_iterator->first;
											l--;
											independent_variable_mapping_iterator++;
										}
										independent_variable_mapping_iterator--;
										column_2 += (independent_variable_mapping_iterator->second)+
											(derivative_index[k]-
											(independent_variable_mapping_iterator->first));
									}
								}
								for (row=0;row<number_of_dependent_values;row++)
								{
									matrix_2(row,column)=(*matrix_iterator)(row,column_2);
								}
								column++;
								// increment derivative_index
								finished=true;
								k=i;
								while (finished&&(k>0))
								{
									if (matrix_variables(j+number_of_matrices+1,k))
									{
										derivative_index[k]++;
										if (derivative_index[k]==numbers_of_independent_values[k])
										{
											derivative_index[k]=0;
										}
										else
										{
											finished=false;
										}
									}
									k--;
								}
								if (finished&&matrix_variables(j+number_of_matrices+1,0))
								{
									derivative_index[0]++;
									finished=
										derivative_index[0]==numbers_of_independent_values[0];
								}
							}
							matrices.push_back(matrix_2);
							matrix_iterator++;
						}
						number_of_matrices += number_of_matrices+1;
						independent_variables_iterator++;
					}
					result=Variable_derivative_matrix_handle(
						new Variable_derivative_matrix(Variable_inverse_handle(this),
						independent_variables,matrices));
				}
				// reset the dependent value
				composite_variable->set_input_value(dependent_variable,
					current_dependent_value);
			}
		}
		// reset the current values
		std::for_each(current_values.rbegin(),current_values.rend(),
			Variable_set_input_values(this));
	}
#if defined (DEBUG)
	//???debug
	std::cout << "leave Variable_inverse::evaluate_derivative" << std::endl;
#endif // defined (DEBUG)

	return (result);
}
#else // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
Variable_handle Variable_inverse::evaluate_derivative(
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>& independent_variables,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Overload Variable::evaluate_derivative
//==============================================================================
{
	Variable_handle result(0);
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		input_independent_cache;
	const Variable_size_type number_of_independent_variables=
		independent_variables.size();

#if defined (DEBUG)
	//???debug
	std::cout << "enter Variable_inverse::evaluate_derivative" << std::endl;
#endif // defined (DEBUG)
	if (this&&(input_independent_cache=input_independent())&&dependent_variable&&
		independent_variable&&(0<number_of_independent_variables))
	{
		bool valid_independent_variables;
		std::list<Variable_handle> composite_variables_list(0);
		std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			> composite_inputs_list(0);
		std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>::iterator composite_inputs_iterator,
			independent_variables_iterator;
		std::list<Variable_input_value_handle> current_values(0),no_values(0);
		std::vector<Variable_size_type>
			numbers_of_independent_values(number_of_independent_variables),
			offsets_of_independent_values(number_of_independent_variables);
		Variable_size_type i,j,offset;

		// set the values so that sizes are valid
		// get the specified values
		std::for_each(values.begin(),values.end(),Variable_get_input_values(
			Variable_handle(this),current_values));
		// override the specified values
		std::for_each(values.begin(),values.end(),Variable_set_input_values(
			Variable_handle(this)));
		// set up an extended variable and an extended input
		composite_variables_list.push_back(independent_variable);
		composite_inputs_list.push_back(dependent_variable);
		valid_independent_variables=true;
		independent_variables_iterator=independent_variables.begin();
		i=0;
		offset=independent_variable->
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			();
		while (valid_independent_variables&&(i<number_of_independent_variables))
		{
			if (!(*independent_variables_iterator)||
				(*dependent_variable== **independent_variables_iterator))
			{
				valid_independent_variables=false;
			}
			else
			{
				numbers_of_independent_values[i]=
#if defined (USE_ITERATORS)
					//???DB.  To be done
					1
#else // defined (USE_ITERATORS)
					(*independent_variables_iterator)->size()
#endif // defined (USE_ITERATORS)
					;
				composite_inputs_iterator=composite_inputs_list.begin();
				j=composite_inputs_list.size();
				if (!(*input_independent_cache== **independent_variables_iterator))
				{
					j--;
					composite_inputs_iterator++;
					while ((j>0)&&
						!(**composite_inputs_iterator== **independent_variables_iterator))
					{
						j--;
						composite_inputs_iterator++;
					}
				}
				if (0==j)
				{
					composite_variables_list.push_back(Variable_handle(
						new Variable_identity(*independent_variables_iterator)));
					composite_inputs_list.push_back(*independent_variables_iterator);
					offsets_of_independent_values[i]=offset;
					offset += numbers_of_independent_values[i];
				}
				else
				{
					j -= composite_inputs_list.size();
					if (0==j)
					{
						offsets_of_independent_values[i]=0;
					}
					else
					{
						offsets_of_independent_values[i]=offsets_of_independent_values[j-1];
					}
				}
			}
			independent_variables_iterator++;
			i++;
		}
		if (valid_independent_variables)
		{
			Variable_handle current_dependent_value(0),
				dependent_value=evaluate(no_values);
			Variable_handle composite_variable(new
				Variable_composite
				(composite_variables_list));
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
				composite_input(new
#if defined (GENERALIZE_COMPOSITE_INPUT)
				Variable_input_composite_union
#else // defined (GENERALIZE_COMPOSITE_INPUT)
#if defined (USE_VARIABLE_INPUT)
				Variable_input_composite
#else // defined (USE_VARIABLE_INPUT)
				Variable_io_specifier_composite
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
				(composite_inputs_list));
			std::list<
#if defined (USE_VARIABLE_INPUT)
				Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
				Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
				> composite_independent_variables(
				number_of_independent_variables,composite_input);

			current_dependent_value=composite_variable->get_input_value(
				dependent_variable);
			composite_variable->set_input_value(dependent_variable,dependent_value);

			Variable_derivative_matrix_handle derivative_inverse=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
				((composite_variable->evaluate_derivative)(
				composite_independent_variables,no_values));

#if defined (DEBUG)
			//???debug
			std::cout << "derivative=" << (derivative_inverse->matrices).front() <<
				std::endl;
#endif // defined (DEBUG)
			if (derivative_inverse&&
				(derivative_inverse=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
				((derivative_inverse->inverse)(Variable_inverse_handle(new
				Variable_inverse(composite_input,composite_variable))))))
			{
#if defined (DEBUG)
				//???debug
				std::cout << "derivative->inverse=" <<
					(derivative_inverse->matrices).front() << std::endl;
#endif // defined (DEBUG)
				// extract parts of the derivative_inverse matrices
				std::list<Matrix> matrices(0);
				std::list<Matrix>::iterator matrix_iterator;
				std::vector<Variable_size_type> derivative_index(
					number_of_independent_variables);
				Variable_size_type number_of_matrices=
					(derivative_inverse->matrices).size();
				ublas::matrix<bool> matrix_variables(number_of_matrices,
					number_of_independent_variables);
				Variable_size_type column_1,k,number_of_composite_input_values,
					number_of_dependent_values;

				for (i=0;i<number_of_matrices;i++)
				{
					for (j=0;j<number_of_independent_variables;j++)
					{
						matrix_variables(i,j)=false;
					}
				}
				independent_variables_iterator=independent_variables.begin();
				number_of_dependent_values=
#if defined (USE_ITERATORS)
					//???DB.  To be done
					1
#else // defined (USE_ITERATORS)
					dependent_variable->size()
#endif // defined (USE_ITERATORS)
					;
				number_of_composite_input_values=
#if defined (USE_ITERATORS)
					//???DB.  To be done
					1
#else // defined (USE_ITERATORS)
					composite_input->size()
#endif // defined (USE_ITERATORS)
					;
				matrix_iterator=(derivative_inverse->matrices).begin();
				number_of_matrices=0;
				for (i=0;i<number_of_independent_variables;i++)
				{
					Variable_size_type number_of_columns=numbers_of_independent_values[i];
					Matrix matrix_1(number_of_dependent_values,number_of_columns);
					Variable_size_type column,row;

					column_1=offsets_of_independent_values[i];
					for (column=0;column<number_of_columns;column++)
					{
						for (row=0;row<number_of_dependent_values;row++)
						{
							matrix_1(row,column)=(*matrix_iterator)(row,column_1);
						}
						column_1++;
					}
					matrix_variables(number_of_matrices,i)=true;
					matrices.push_back(matrix_1);
					matrix_iterator++;
					for (j=0;j<number_of_matrices;j++)
					{
						number_of_columns=numbers_of_independent_values[i];
						for (k=0;k<i;k++)
						{
							if (matrix_variables(j,k))
							{
								matrix_variables(j+number_of_matrices+1,k)=true;
								number_of_columns *= numbers_of_independent_values[k];
							}
							derivative_index[k]=0;
						}
						derivative_index[i]=0;
						matrix_variables(j+number_of_matrices+1,i)=true;

						bool finished;
						Matrix matrix_2(number_of_dependent_values,number_of_columns);
						Variable_size_type column_2;

						column=0;
						finished=false;
						while (!finished)
						{
							// calculated column of *matrix_iterator
							column_2=0;
							for (k=0;k<=i;k++)
							{
								if (matrix_variables(j+number_of_matrices+1,k))
								{
									column_2 *= number_of_composite_input_values;
									column_2 +=
										offsets_of_independent_values[k]+derivative_index[k];
								}
							}
							for (row=0;row<number_of_dependent_values;row++)
							{
								matrix_2(row,column)=(*matrix_iterator)(row,column_2);
							}
							column++;
							// increment derivative_index
							finished=true;
							k=i;
							while (finished&&(k>0))
							{
								if (matrix_variables(j+number_of_matrices+1,k))
								{
									derivative_index[k]++;
									if (derivative_index[k]==numbers_of_independent_values[k])
									{
										derivative_index[k]=0;
									}
									else
									{
										finished=false;
									}
								}
								k--;
							}
							if (finished&&matrix_variables(j+number_of_matrices+1,0))
							{
								derivative_index[0]++;
								finished=derivative_index[0]==numbers_of_independent_values[0];
							}
						}
						matrices.push_back(matrix_2);
						matrix_iterator++;
					}
					number_of_matrices += number_of_matrices+1;
					independent_variables_iterator++;
				}
				result=Variable_derivative_matrix_handle(new Variable_derivative_matrix(
					Variable_inverse_handle(this),independent_variables,matrices));
			}
			// reset the dependent value
			composite_variable->set_input_value(dependent_variable,
				current_dependent_value);
		}
		// reset the current values
		std::for_each(current_values.rbegin(),current_values.rend(),
			Variable_set_input_values(this));
	}
#if defined (DEBUG)
	//???debug
	std::cout << "leave Variable_inverse::evaluate_derivative" << std::endl;
#endif // defined (DEBUG)

	return (result);
}
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)

Variable_size_type Variable_inverse::
#if defined (USE_ITERATORS)
	number_differentiable
#else // defined (USE_ITERATORS)
	size
#endif // defined (USE_ITERATORS)
	() const
//******************************************************************************
// LAST MODIFIED : 20 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (dependent_variable->
#if defined (USE_ITERATORS)
		number_differentiable
#else // defined (USE_ITERATORS)
		size
#endif // defined (USE_ITERATORS)
		());
}

#if defined (USE_ITERATORS)
#else // defined (USE_ITERATORS)
Vector *Variable_inverse::scalars()
//******************************************************************************
// LAST MODIFIED : 4 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (evaluate_local()->scalars());
}
#endif // defined (USE_ITERATORS)

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_inverse::input_independent()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_independent(dependent_variable,independent_variable)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_inverse::input_value_tolerance()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_value_tolerance(this)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_inverse::input_step_tolerance()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_step_tolerance(this)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_inverse::input_maximum_iterations()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_maximum_iterations(this)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_inverse::input_dependent_estimate()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_dependent_estimate(dependent_variable,
		independent_variable)));
}

Variable_handle Variable_inverse::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_inverse_handle(new Variable_inverse(*this)));
}

Variable_handle Variable_inverse::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool valid;
	Scalar error_norm,increment_norm;
	std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		> independent_variables(0);
	std::list<Variable_input_value_handle> values(0);
	Variable_handle current_estimate,error,result;
	Variable_size_type iteration_number;

	result=0;
	valid=true;
	independent_variables.push_back(dependent_variable);
	values.push_back(Variable_input_value_handle(new Variable_input_value(
		dependent_variable,dependent_variable_estimate)));
	current_estimate=independent_variable->evaluate(values);
	valid=((error=(*independent_value)-(*current_estimate))&&
		(0<=(error_norm=error->norm())));
	iteration_number=0;
	increment_norm=1+step_tolerance;
	while (valid&&
		!(((0>=value_tolerance)&&(0>=step_tolerance))||
		(error_norm<value_tolerance)||(increment_norm<step_tolerance))&&
		(iteration_number<maximum_iterations))
	{
		Variable_derivative_matrix_handle derivative=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
			(independent_variable->evaluate_derivative(independent_variables,
			values));
		Variable_matrix_handle derivative_matrix;

		if (valid=(derivative&&
			(derivative_matrix=derivative->matrix(independent_variables))))
		{
			Variable_handle increment=derivative_matrix->solve(error);

			if (valid=(increment&&((*dependent_variable_estimate) -= (*increment))))
			{
				iteration_number++;
				current_estimate=independent_variable->evaluate(values);
				valid=((error=(*independent_value)-(*current_estimate))&&
					(0<=(error_norm=error->norm()))&&
					(0<=(increment_norm=increment->norm())));
			}
		}
	}
	if (valid)
	{
		result=dependent_variable_estimate;
	}

	return (result);
}

#if defined (USE_ITERATORS)
//???DB.  To be done
#else // defined (USE_ITERATORS)
bool Variable_inverse::evaluate_derivative_local(Matrix&,
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>&)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	// should not come here - handled by overloading Variable::evaluate_derivative
	Assert(false,std::logic_error(
		"Variable_inverse::evaluate_derivative_local.  "
		"Should not come here"));
	
	return (false);
}
#endif // defined (USE_ITERATORS)

Variable_handle Variable_inverse::get_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_handle result(0);
	Variable_input_dependent_estimate_handle input_dependent_estimate_handle;
	Variable_input_independent_handle input_independent_handle;
	Variable_input_maximum_iterations_handle input_maximum_iterations_handle;
	Variable_input_step_tolerance_handle input_step_tolerance_handle;
	Variable_input_value_tolerance_handle input_value_tolerance_handle;

	if ((input_independent_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_independent,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_independent *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_independent_handle->independent_variable==independent_variable)&&
		(input_independent_handle->dependent_variable)&&dependent_variable&&
		(*(input_independent_handle->dependent_variable)== *dependent_variable))
	{
		if (independent_value)
		{
			result=independent_value->clone();
		}
	}
	else if ((input_step_tolerance_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_step_tolerance,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_step_tolerance *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_step_tolerance_handle->variable_inverse==
		Variable_inverse_handle(this)))
	{
		result=Variable_scalar_handle(new Variable_scalar(step_tolerance));
	}
	else if ((input_value_tolerance_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_value_tolerance,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_value_tolerance *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_value_tolerance_handle->variable_inverse==
		Variable_inverse_handle(this)))
	{
		result=Variable_scalar_handle(new Variable_scalar(value_tolerance));
	}
	else if ((input_maximum_iterations_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_maximum_iterations,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_maximum_iterations *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_maximum_iterations_handle->variable_inverse==
		Variable_inverse_handle(this)))
	{
		result=Variable_scalar_handle(new Variable_scalar(
			(Scalar)maximum_iterations));
	}
	else if ((input_dependent_estimate_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_dependent_estimate,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_dependent_estimate *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_dependent_estimate_handle->independent_variable==
		independent_variable)&&
		(input_dependent_estimate_handle->dependent_variable)&&dependent_variable&&
		(*(input_dependent_estimate_handle->dependent_variable)==
		*dependent_variable))
	{
		if (dependent_variable_estimate)
		{
			result=dependent_variable_estimate->clone();
		}
	}

	return (result);
}

bool Variable_inverse::set_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input,
	const
#if defined (USE_VARIABLES_AS_COMPONENTS)
	Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
	& values)
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Variable_input_dependent_estimate_handle input_dependent_estimate_handle;
	Variable_input_independent_handle input_independent_handle;
	Variable_input_maximum_iterations_handle input_maximum_iterations_handle;
	Variable_input_step_tolerance_handle input_step_tolerance_handle;
	Variable_input_value_tolerance_handle input_value_tolerance_handle;

	result=false;
	if ((input_independent_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_independent,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_independent *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_independent_handle->independent_variable==independent_variable)&&
		(input_independent_handle->dependent_variable)&&dependent_variable&&
		(*(input_independent_handle->dependent_variable)== *dependent_variable))
	{
		if (values)
		{
			independent_value=values->clone();
		}
		else
		{
			independent_value=0;
		}
		result=true;
	}
	else if ((input_step_tolerance_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_step_tolerance,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_step_tolerance *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_step_tolerance_handle->variable_inverse==
		Variable_inverse_handle(this)))
	{
		if (values&&(1==values->
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			()))
		{
			Vector *values_vector;

			if (values_vector=
#if defined (USE_ITERATORS)
				//???DB.  To be done
				0
#else // defined (USE_ITERATORS)
				values->scalars()
#endif // defined (USE_ITERATORS)
				)
			{
				step_tolerance=(*values_vector)[0];
				result=true;
			}
		}
	}
	else if ((input_value_tolerance_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_value_tolerance,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_value_tolerance *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_value_tolerance_handle->variable_inverse==
		Variable_inverse_handle(this)))
	{
		if (values&&(1==values->
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			()))
		{
			Vector *values_vector;

			if (values_vector=
#if defined (USE_ITERATORS)
				//???DB.  To be done
				0
#else // defined (USE_ITERATORS)
				values->scalars()
#endif // defined (USE_ITERATORS)
				)
			{
				value_tolerance=(*values_vector)[0];
				result=true;
			}
		}
	}
	else if ((input_maximum_iterations_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_maximum_iterations,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_maximum_iterations *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_maximum_iterations_handle->variable_inverse==
		Variable_inverse_handle(this)))
	{
		if (values&&(1==values->
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			()))
		{
			Vector *values_vector;

			if (values_vector=
#if defined (USE_ITERATORS)
				//???DB.  To be done
				0
#else // defined (USE_ITERATORS)
				values->scalars()
#endif // defined (USE_ITERATORS)
				)
			{
				maximum_iterations=(Variable_size_type)((*values_vector)[0]);
				result=true;
			}
		}
	}
	else if ((input_dependent_estimate_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_dependent_estimate,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_dependent_estimate *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_dependent_estimate_handle->independent_variable==
		independent_variable)&&
		(input_dependent_estimate_handle->dependent_variable)&&dependent_variable&&
		(*(input_dependent_estimate_handle->dependent_variable)==
		*dependent_variable))
	{
		if (values)
		{
			dependent_variable_estimate=values->clone();
		}
		else
		{
			dependent_variable_estimate=0;
		}
		result=true;
	}
	else
	{
		result=(independent_variable->set_input_value)(input,values);
	}

	return (result);
}

string_handle Variable_inverse::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//???DB.  Need get_string_representation for inputs to do properly?
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << "inverse";
		*return_string=out.str();
	}

	return (return_string);
}
