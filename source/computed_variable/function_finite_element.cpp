//******************************************************************************
// FILE : function_finite_element.cpp
//
// LAST MODIFIED : 17 February 2005
//
// DESCRIPTION :
// Finite element types - element, element/xi and finite element field.
//
//???DB.  How to do element/xi for set_value and rset_value when the element
//  dimension changes?
//???DB.  What about setting independent variable for Function_inverse
// 1.  Always have element as part of an element/xi (if no element then no xi)
//   - currently an element/xi function has all xi
// 2.  Make element/xi atomic
// 3.  Don't have an element/xi function.  No, means that need an element input.
//   - if can set element by itself then can "lose" (won't be saved for rset) xi
//     values
// 4.  Don't have an element input
// 5.  Don't reset specified values as part of evaluate and evaluate_derivative
//   No, worried that if a function is reused within an evaluation then it will
//   have the wrong values
// 6.  When change element dimension, what happens to iterators?
// 7.  Specify dimension at creation and don't allow dimension to change?
// 8.  Change set, rset and atomic?  Only way I can think of doing union,
//   intersection, ... variables, without functions having to know about them,
//   is atomic variables.
// 9.  Make functions have storage for their outputs.  Would then need a
//   get_value to retrieve without evaluating (for inverse).  Does evaluate set
//   the output storage?
//==============================================================================

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

extern "C"
{
#include "finite_element/finite_element_region.h"
//???DB.  Get rid of debug.h (C->C++)
#include "general/debug.h"
}
#if defined (NEW_CODE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (NEW_CODE)
#include "computed_variable/function_finite_element.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_composite.hpp"
#include "computed_variable/function_variable_element_xi.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_value_element.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"

// module typedefs
// ===============

typedef boost::intrusive_ptr< Function_matrix<Scalar> >
	Function_matrix_scalar_handle;

typedef boost::intrusive_ptr< Function_variable_matrix<Scalar> >
	Function_variable_matrix_scalar_handle;

// module classes
// ==============

// class Function_variable_matrix_components
// -----------------------------------------

// forward declaration so that can use _handle
class Function_variable_matrix_components;
typedef boost::intrusive_ptr<Function_variable_matrix_components>
	Function_variable_matrix_components_handle;

#if defined (NEW_CODE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
bool evaluate_derivative_matrix(Matrix& matrix,
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 11 January 2005
//
// DESCRIPTION :
// ???DB.  Throw an exception for failure?
//==============================================================================
{
	bool result,zero_derivative;
	Variable_size_type derivative_order=independent_variables.size(),
		number_of_columns,number_of_components;
	Variable_input_nodal_values_handle nodal_values_input(0);
	ublas::vector<Variable_input_element_xi_handle>
		element_xi_inputs(derivative_order);

	result=true;
	// matrix is zero'd on entry
	// check independent variables for a zero matrix
	number_of_components=this->size();
	zero_derivative=false;
	number_of_columns=0;
	std::for_each(independent_variables.begin(),independent_variables.end(),
		Variable_finite_element_check_derivative_functor(
		Variable_finite_element_handle(this),zero_derivative,
		nodal_values_input,element_xi_inputs.begin(),number_of_columns));
	Assert((number_of_components==matrix.size1())&&
		(number_of_columns==matrix.size2()),
		std::logic_error("Variable_finite_element::evaluate_derivative_local.  "
		"Incorrect matrix size"));
	if (!zero_derivative)
	{
		FE_value **component_values=(FE_value **)NULL,
			*monomial_derivative_values=(FE_value *)NULL,
			***nodal_value_component_values=(FE_value ***)NULL;
		int **component_monomial_info=(int **)NULL,
			*numbers_of_component_values=(int *)NULL,number_of_element_field_nodes=0,
			number_of_nodal_values=0,return_code=0;
		struct FE_element_field_values *element_field_values;
		struct FE_node **element_field_nodes=(struct FE_node **)NULL;
		Variable_size_type number_of_xi=xi.size();

		// set up temporary storage
		element_field_values=CREATE(FE_element_field_values)();
		if (element_field_values)
		{
			if (nodal_values_input)
			{
				if (nodal_value_calculate_component_values(
					Variable_finite_element_handle(this),field,component_number,
					(FE_value)time,element,nodal_values_input->node,
					nodal_values_input->value_type,nodal_values_input->version,
					&number_of_element_field_nodes,&element_field_nodes,
					element_field_values,&number_of_nodal_values,
					&numbers_of_component_values,&nodal_value_component_values)&&
					extract_component_monomial_info(element_field_values,
					number_of_components,component_number,number_of_xi,
					&numbers_of_component_values,&component_monomial_info,
					&monomial_derivative_values))
				{
					return_code=1;
				}
			}
			else
			{
				if (clear_FE_element_field_values(element_field_values)&&
					calculate_FE_element_field_values(element,field,(FE_value)time,
					(char)0,element_field_values,(struct FE_element *)NULL)&&
					extract_component_values(element_field_values,number_of_components,
					component_number,&numbers_of_component_values,&component_values)&&
					extract_component_monomial_info(element_field_values,
					number_of_components,component_number,number_of_xi,
					&numbers_of_component_values,&component_monomial_info,
					&monomial_derivative_values))
				{
					return_code=1;
				}
			}
			if (return_code)
			{
				FE_value *xi_array;
				int *xi_derivative_orders;

				xi_array=new FE_value[number_of_xi];
				xi_derivative_orders=new int[number_of_xi];
				if (xi_array&&xi_derivative_orders)
				{
					bool carry;
					FE_value **derivative_component_values=(FE_value **)NULL,
						*value_address_1,*value_address_2;
					Variable_size_type column_number,i;
					ublas::vector<Variable_size_type>
						derivative_independent_values(derivative_order+1);
					int j;
					Scalar component_value;

					for (i=0;i<=derivative_order;i++)
					{
						derivative_independent_values[i]=0;
					}
					for (i=0;i<number_of_xi;i++)
					{
						xi_array[i]=(FE_value)(xi[i]);
					}
					/* chose the component values to use for calculating the
						values */
					if (nodal_values_input)
					{
						derivative_component_values=nodal_value_component_values[0];
					}
					else
					{
						derivative_component_values=component_values;
					}
					/* loop over the values within the derivative block */
					column_number=0;
					while (return_code&&
						(0==derivative_independent_values[derivative_order]))
					{
						if (derivative_component_values)
						{
							for (i=0;i<number_of_xi;i++)
							{
								xi_derivative_orders[i]=0;
							}
							for (i=0;i<derivative_order;i++)
							{
								if (element_xi_inputs[i])
								{
									// element/xi derivative
									if (0<element_xi_inputs[i]->indices.size())
									{
										xi_derivative_orders[(element_xi_inputs[i]->indices)[
											derivative_independent_values[i]]-1]++;
									}
									else
									{
										xi_derivative_orders[
											derivative_independent_values[i]]++;
									}
								}
							}
							/* loop over components */
							i=0;
							while (return_code&&(i<number_of_components))
							{
								if (return_code=calculate_monomial_derivative_values(
									number_of_xi,component_monomial_info[i]+1,
									xi_derivative_orders,xi_array,monomial_derivative_values))
								{
									/* calculate the derivative */
									component_value=(Scalar)0;
									value_address_1=monomial_derivative_values;
									value_address_2=derivative_component_values[i];
									for (j=numbers_of_component_values[i];j>0;j--)
									{
										component_value +=
											(double)(*value_address_1)*
											(double)(*value_address_2);
										value_address_1++;
										value_address_2++;
									}
									matrix(i,column_number)=component_value;
								}
								i++;
							}
						}
						/* step to next value within derivative block */
						column_number++;
						i=0;
						carry=true;
						do
						{
							derivative_independent_values[i]++;
							if (element_xi_inputs[i])
							{
								Variable_size_type
									number_of_values=(element_xi_inputs[i]->indices).size();

								if (0<number_of_values)
								{
									if (derivative_independent_values[i]>=number_of_values)
									{
										derivative_independent_values[i]=0;
									}
									else
									{
										carry=false;
									}
								}
								else
								{
									if (derivative_independent_values[i]>=number_of_xi)
									{
										derivative_independent_values[i]=0;
									}
									else
									{
										carry=false;
									}
								}
							}
							else
							{
								if (derivative_independent_values[i]>=
									(Variable_size_type)number_of_nodal_values)
								{
									derivative_independent_values[i]=0;
								}
								else
								{
									carry=false;
								}
								// update the component values to use for calculating the values
								derivative_component_values=
									nodal_value_component_values[
									derivative_independent_values[i]];
							}
							i++;
						} while ((i<derivative_order)&&carry);
						if (carry)
						{
							derivative_independent_values[derivative_order]++;
						}
					}
				}
				delete [] xi_array;
				delete [] xi_derivative_orders;
			}
		}
		// remove temporary storage
		if (element_field_values)
		{
			DESTROY(FE_element_field_values)(&element_field_values);
		}
	}

	return (result);
}

// class Function_derivatnew_finite_element
// ----------------------------------------

class Function_derivatnew_finite_element : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 11 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_finite_element(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables):
			Function_derivatnew(dependent_variable,independent_variables){};
		// destructor
		~Function_derivatnew_finite_element(){};
	// inherited
	private:
		virtual Function_handle evaluate(Function_variable_handle atomic_variable)
		{
			Function_handle result(0);

			if (!evaluated())
			{
				Function_finite_element_handle function_finite_element;

				if (boost::dynamic_pointer_cast<Function_variable_matrix_components,
					Function_variable>(dependent_variable)&&(function_finite_element=
					boost::dynamic_pointer_cast<Function_finite_element,Function>(
					dependent_variable->function())))
				{
					bool valid;
					Function_size_type number_of_dependent_values=dependent_variable->
						number_differentiable();
					std::list<Function_variable_handle>::const_iterator
						independent_variable_iterator;
					std::list< std::list<Function_variable_handle> >
						matrix_independent_variables;
					std::list<Matrix> matrices;

					valid=true;
					independent_variable_iterator=independent_variables.begin();
					while (valid&&
						(independent_variable_iterator!=independent_variables.end()))
					{
						Function_variable_handle independent_variable=
							*independent_variable_iterator;
						Function_size_type number_of_independent_values=
							independent_variable->number_differentiable();
						std::list<Matrix>::iterator matrix_iterator,last;
						std::list< std::list<Function_variable_handle> >::iterator
							matrix_independent_variables_iterator;

						// calculate the derivative of dependent variable with respect to
						//   independent variable and add to matrix list
						{
							Matrix new_matrix(number_of_dependent_values,
								number_of_independent_values);
							std::list<Function_variable_handle>
								new_matrix_independent_variables;

							new_matrix_independent_variables.push_back(independent_variable);
							if (valid=evaluate_derivative_matrix(new_matrix,
								new_matrix_independent_variables.push_back))
							{
								matrices.push_back(new_matrix);
								matrix_independent_variables.push_back(
									new_matrix_independent_variables);
							}
						}
						last=matrices.end();
						last--;
						matrix_independent_variables_iterator=
							matrix_independent_variables.begin();
						matrix_iterator=matrices.begin();
						while (valid&&(matrix_iterator!=last))
						{
							Matrix& matrix= *matrix_iterator;
							Matrix new_matrix((matrix.size1)(),
								number_of_independent_values*(matrix.size2)());
							std::list<Function_variable_handle>
								new_matrix_independent_variables;

							new_matrix_independent_variables=
								*matrix_independent_variables_iterator;
							new_matrix_independent_variables.push_back(independent_variable);
							if (valid=evaluate_derivative_matrix(new_matrix,
								new_matrix_independent_variables.push_back))
							{
								matrices.push_back(new_matrix);
								matrix_independent_variables.push_back(
									new_matrix_independent_variables);
							}
							matrix_independent_variables_iterator++;
							matrix_iterator++;
						}
						independent_variable_iterator++;
					}
					if (valid)
					{
						derivative_matrix=Derivative_matrix(matrices);
						set_evaluated();
					}
				}
			}
			if (evaluated())
			{
				result=get_value(atomic_variable);
			}

			return (result);
		};
};
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (NEW_CODE)

class Function_variable_matrix_components :
	public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 23 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_finite_element;
	public:
		// constructors.  A zero component_number indicates all components
		Function_variable_matrix_components(
			const Function_finite_element_handle& function_finite_element,
			Function_size_type component_number=0):Function_variable_matrix<Scalar>(
			function_finite_element,component_number,1){};
		Function_variable_matrix_components(
			const Function_finite_element_handle& function_finite_element,
			const std::string component_name):
			Function_variable_matrix<Scalar>(function_finite_element,0,1)
		{
			if (function_finite_element)
			{
				char *name;
				int i;

				i=get_FE_field_number_of_components(
					function_finite_element->field_private);
				if (0<i)
				{
					name=(char *)NULL;
					do
					{
						i--;
						if (name)
						{
							DEALLOCATE(name);
						}
						name=get_FE_field_component_name(
							function_finite_element->field_private,i);
					} while ((i>0)&&(std::string(name)!=component_name));
					if (std::string(name)==component_name)
					{
						row_private=i+1;
						value_private=Function_variable_value_handle(
							new Function_variable_value_specific<Scalar>(
							Function_variable_matrix_set_value_function<Scalar>));
					}
				}
			}
		};
		// destructor
		~Function_variable_matrix_components(){};
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_components(*this)));
		};
#if defined (NEW_CODE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables)
		{
			return (Function_handle(new Function_derivatnew_finite_element(
				Function_variable_handle(this),independent_variables)));
		}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (NEW_CODE)
		string_handle get_string_representation()
			//???DB.  Overloading Function_variable_matrix.  Make better?
		{
			Assert(this&&function(),std::logic_error(
				"Function_variable_matrix_components::get_string_representation.  "
				"Missing function()"));
			Function_finite_element_handle function_finite_element=
				boost::dynamic_pointer_cast<Function_finite_element,Function>(
				function());
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				out << *(function_finite_element->get_string_representation());
				out << "[";
				if (0==row_private)
				{
					out << "*";
				}
				else
				{
					out << row_private;
				}
				out << "]";
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_handle get_value() const
		{
			Function_handle result(0);
			Function_finite_element_handle function_finite_element;

			if (this&&(function_finite_element=
				boost::dynamic_pointer_cast<Function_finite_element,Function>(
				function())))
			{
				if (0==row_private)
				{
					Function_size_type i,number_of_components=
						function_finite_element->number_of_components();
					Matrix result_matrix(number_of_components,1);

					i=0;
					while ((i<number_of_components)&&
						(function_finite_element->component_value)(i+1,result_matrix(i,0)))
					{
						i++;
					}
					if (i==number_of_components)
					{
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
					}
				}
				else
				{
					Matrix result_matrix(1,1);

					if ((function_finite_element->component_value)(row_private,
						result_matrix(0,0)))
					{
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
					new Function_variable_matrix_components(
					boost::dynamic_pointer_cast<Function_finite_element,Function>(
					function_private),row));
			}

			return (result);
		};
		Function_size_type number_of_rows() const
		{
			Function_finite_element_handle function_finite_element;
			Function_size_type result;

			result=0;
			if (this&&(function_finite_element=boost::dynamic_pointer_cast<
				Function_finite_element,Function>(function())))
			{
				result=function_finite_element->number_of_components();
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
			Function_finite_element_handle function_finite_element;

			result=false;
			if (this&&(1==column_private)&&(function_finite_element=
				boost::dynamic_pointer_cast<Function_finite_element,Function>(
				function())))
			{
				result=(function_finite_element->component_value)(row_private,value);
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix_components(
			const Function_variable_matrix_components& variable_finite_element):
			Function_variable_matrix<Scalar>(variable_finite_element){};
};


// forward declaration so that can use _handle
class Function_variable_element;
typedef boost::intrusive_ptr<Function_variable_element>
	Function_variable_element_handle;


// class Function_variable_iterator_representation_atomic_element
// --------------------------------------------------------------

class Function_variable_iterator_representation_atomic_element:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_element(
			const bool begin,Function_variable_element_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_element();
	private:
		// increment
		void increment();
		// decrement
		void decrement();
		// equality
		bool equality(
			const Function_variable_iterator_representation* representation);
		// dereference
		Function_variable_handle dereference() const;
	private:
		// copy constructor
		Function_variable_iterator_representation_atomic_element(
			const Function_variable_iterator_representation_atomic_element&);
	private:
		Function_variable_element_handle atomic_variable,variable;
};


// class Function_variable_element
// -------------------------------

class Function_variable_element : public Function_variable
//******************************************************************************
// LAST MODIFIED : 22 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_element;
	friend class Function_variable_iterator_representation_atomic_element;
	friend bool Function_variable_element_set_element_function(
		struct FE_element*& element,const Function_variable_handle variable);
	public:
		// constructors
		Function_variable_element(
			const Function_element_handle& function_element):
			Function_variable(function_element){};
		~Function_variable_element(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_element(*this)));
		};
		string_handle get_string_representation()
		{
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				out << "element";
				*return_string=out.str();
			}

			return (return_string);
		};
		virtual Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(
				new Function_variable_iterator_representation_atomic_element(true,
				Function_variable_element_handle(
				const_cast<Function_variable_element*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(
				new Function_variable_iterator_representation_atomic_element(false,
				Function_variable_element_handle(
				const_cast<Function_variable_element*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(
				new Function_variable_iterator_representation_atomic_element(
				false,Function_variable_element_handle(
				const_cast<Function_variable_element*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(
				new Function_variable_iterator_representation_atomic_element(
				true,Function_variable_element_handle(
				const_cast<Function_variable_element*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			return (0);
		};
	private:
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_element_handle variable_element;

			result=false;
			if (variable_element=boost::dynamic_pointer_cast<
				Function_variable_element,Function_variable>(variable))
			{
				if (equivalent(variable_element->function(),function()))
				{
					result=true;
				}
			}

			return (result);
		};
	protected:
		// copy constructor
		Function_variable_element(
			const Function_variable_element& variable_element):
			Function_variable(variable_element){};
	private:
		// assignment
		Function_variable_element& operator=(const Function_variable_element&);
};

//???DB.  Should this ACCESS(FE_element)?
bool Function_variable_element_set_element_function(
	struct FE_element*& element,const Function_variable_handle variable)
{
	bool result;
	Function_element_handle function_element;
	Function_variable_element_handle element_variable;

	result=false;
	if ((element_variable=boost::dynamic_pointer_cast<
		Function_variable_element,Function_variable>(variable))&&(function_element=
		boost::dynamic_pointer_cast<Function_element,Function>(
		element_variable->function())))
	{
		element=(function_element->element_value)();
		result=true;
	}

	return (result);
}


// class Function_variable_iterator_representation_atomic_element
// --------------------------------------------------------------

Function_variable_iterator_representation_atomic_element::
	Function_variable_iterator_representation_atomic_element(const bool begin,
	Function_variable_element_handle variable):atomic_variable(0),
	variable(variable)
//******************************************************************************
// LAST MODIFIED : 11 April 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable)
	{
		if (atomic_variable=boost::dynamic_pointer_cast<Function_variable_element,
			Function_variable>(variable->clone()))
		{
			atomic_variable->value_private=Function_variable_value_handle(
				new Function_variable_value_element(
				Function_variable_element_set_element_function));
		}
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_element::clone()
//******************************************************************************
// LAST MODIFIED : 11 April 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new Function_variable_iterator_representation_atomic_element(*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_element::
	~Function_variable_iterator_representation_atomic_element()
//******************************************************************************
// LAST MODIFIED : 11 April 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_element::increment()
//******************************************************************************
// LAST MODIFIED : 11 April 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		// end
		atomic_variable=0;
	}
}

void Function_variable_iterator_representation_atomic_element::decrement()
//******************************************************************************
// LAST MODIFIED : 11 April 2004
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		// end
		atomic_variable=0;
	}
	else
	{
		if (atomic_variable=boost::dynamic_pointer_cast<Function_variable_element,
			Function_variable>(variable->clone()))
		{
			atomic_variable->value_private=Function_variable_value_handle(
				new Function_variable_value_element(
				Function_variable_element_set_element_function));
		}
	}
}

bool Function_variable_iterator_representation_atomic_element::equality(
	const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_element
		*representation_element=dynamic_cast<
		const Function_variable_iterator_representation_atomic_element *>(
		representation);
	
	result=false;
	if (representation_element)
	{
		result=equivalent(atomic_variable,
			representation_element->atomic_variable);
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_element::dereference() const
//******************************************************************************
// LAST MODIFIED : 11 April 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_element::
	Function_variable_iterator_representation_atomic_element(const
	Function_variable_iterator_representation_atomic_element& representation):
	Function_variable_iterator_representation(),atomic_variable(0),
	variable(representation.variable)
//******************************************************************************
// LAST MODIFIED : 22 March 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<Function_variable_element,
			Function_variable>((representation.atomic_variable)->clone());
	}
}


// class Function_variable_element_xi_element_xi
// ---------------------------------------------

// forward declaration so that can use _handle
class Function_variable_element_xi_element_xi;
typedef boost::intrusive_ptr<Function_variable_element_xi_element_xi>
	Function_variable_element_xi_element_xi_handle;

class Function_variable_element_xi_element_xi :
	public Function_variable_element_xi
//******************************************************************************
// LAST MODIFIED : 22 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_element_xi;
	public:
		// constructors
		Function_variable_element_xi_element_xi(
			const Function_element_xi_handle& function_element_xi,bool element=true,
			bool xi=true,const ublas::vector<Function_size_type>& indices=
			ublas::vector<Function_size_type>(0)):
			Function_variable_element_xi(function_element_xi,element,xi,indices){};
		Function_variable_element_xi_element_xi(
			const Function_element_xi_handle& function_element_xi,
			Function_size_type index):
			Function_variable_element_xi(function_element_xi,index){};
		Function_variable_element_xi_element_xi(
			const Function_element_xi_handle& function_element_xi,
			const ublas::vector<Function_size_type>& indices):
			Function_variable_element_xi(function_element_xi,indices){};
		~Function_variable_element_xi_element_xi(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_element_xi_element_xi(*this)));
		};
		Function_size_type number_of_xi() const
		{
			Function_size_type result;

			result=0;
			if (xi_private)
			{
				result=indices.size();
				if (0==result)
				{
					Function_element_xi_handle function_element_xi=
						boost::dynamic_pointer_cast<Function_element_xi,Function>(
						function());
					
					if (function_element_xi)
					{
						result=(function_element_xi->number_of_xi)();
					}
				}
			}

			return (result);
		};
		bool get_xi(Scalar& value) const
		{
			bool result;

			result=false;
			if (xi_private&&!element_private)
			{
				switch (indices.size())
				{
					case 0:
					{
						Function_element_xi_handle function_element_xi=
							boost::dynamic_pointer_cast<Function_element_xi,Function>(
							function());
						
						if (function_element_xi)
						{
							value=(function_element_xi->xi_value)(1);
							result=true;
						}
					} break;
					case 1:
					{
						Function_element_xi_handle function_element_xi=
							boost::dynamic_pointer_cast<Function_element_xi,Function>(
							function());
						
						if (function_element_xi)
						{
							value=(function_element_xi->xi_value)(indices[0]);
							result=true;
						}
					} break;
				}
			}

			return (result);
		};
		//???DB.  Should this ACCESS(FE_element)?
		bool get_element(struct FE_element*& element) const
		{
			bool result;

			result=false;
			if (!xi_private&&element_private)
			{
				Function_element_xi_handle function_element_xi=
					boost::dynamic_pointer_cast<Function_element_xi,Function>(
					function());
				
				if (function_element_xi)
				{
					element=(function_element_xi->element_value)();
					result=true;
				}
			}

			return (result);
		};
	protected:
		// copy constructor
		Function_variable_element_xi_element_xi(
			const Function_variable_element_xi_element_xi& variable_element_xi):
			Function_variable_element_xi(variable_element_xi){};
	private:
		// assignment
		Function_variable_element_xi_element_xi& operator=(
			const Function_variable_element_xi_element_xi&);
};


// class Function_variable_element_xi_finite_element
// -------------------------------------------------

// forward declaration so that can use _handle
class Function_variable_element_xi_finite_element;
typedef boost::intrusive_ptr<Function_variable_element_xi_finite_element>
	Function_variable_element_xi_finite_element_handle;

class Function_variable_element_xi_finite_element :
	public Function_variable_element_xi
//******************************************************************************
// LAST MODIFIED : 22 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_finite_element;
	friend class Function_finite_element_check_derivative_functor;
	public:
		// constructors
		Function_variable_element_xi_finite_element(
			const Function_finite_element_handle& function_finite_element,
			bool element=true,bool xi=true,
			const ublas::vector<Function_size_type>& indices=
			ublas::vector<Function_size_type>(0)):
			Function_variable_element_xi(function_finite_element,element,xi,indices)
			{};
		Function_variable_element_xi_finite_element(
			const Function_finite_element_handle& function_finite_element,
			Function_size_type index):
			Function_variable_element_xi(function_finite_element,index){};
		Function_variable_element_xi_finite_element(
			const Function_finite_element_handle& function_finite_element,
			const ublas::vector<Function_size_type>& indices):
			Function_variable_element_xi(function_finite_element,indices){};
		~Function_variable_element_xi_finite_element(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_element_xi_finite_element(*this)));
		};
		Function_size_type number_of_xi() const
		{
			Function_size_type result;

			result=0;
			if (xi_private)
			{
				result=indices.size();
				if (0==result)
				{
					Function_finite_element_handle function_finite_element=
						boost::dynamic_pointer_cast<Function_finite_element,Function>(
						function());
					
					if (function_finite_element)
					{
						result=(function_finite_element->number_of_xi)();
					}
				}
			}

			return (result);
		};
		bool get_xi(Scalar& value) const
		{
			bool result;

			result=false;
			if (xi_private&&!element_private)
			{
				switch (indices.size())
				{
					case 0:
					{
						Function_finite_element_handle function_finite_element=
							boost::dynamic_pointer_cast<Function_finite_element,Function>(
							function());
						
						if (function_finite_element)
						{
							value=(function_finite_element->xi_value)(1);
							result=true;
						}
					} break;
					case 1:
					{
						Function_finite_element_handle function_finite_element=
							boost::dynamic_pointer_cast<Function_finite_element,Function>(
							function());
						
						if (function_finite_element)
						{
							value=(function_finite_element->xi_value)(indices[0]);
							result=true;
						}
					} break;
				}
			}

			return (result);
		};
		//???DB.  Should this ACCESS(FE_element)?
		bool get_element(struct FE_element*& element) const
		{
			bool result;

			result=false;
			if (!xi_private&&element_private)
			{
				Function_finite_element_handle function_finite_element=
					boost::dynamic_pointer_cast<Function_finite_element,Function>(
					function());
				
				if (function_finite_element)
				{
					element=(function_finite_element->element_value)();
					result=true;
				}
			}

			return (result);
		};
	protected:
		// copy constructor
		Function_variable_element_xi_finite_element(
			const Function_variable_element_xi_finite_element& variable_element_xi):
			Function_variable_element_xi(variable_element_xi){};
	private:
		// assignment
		Function_variable_element_xi_finite_element& operator=(
			const Function_variable_element_xi_finite_element&);
};


// forward declaration so that can use _handle
class Function_variable_matrix_nodal_values;
typedef boost::intrusive_ptr<Function_variable_matrix_nodal_values>
	Function_variable_matrix_nodal_values_handle;

// class Function_variable_iterator_representation_atomic_nodal_values
// -------------------------------------------------------------------

class Function_variable_iterator_representation_atomic_nodal_values:public
	Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_nodal_values(
			const bool begin,Function_variable_matrix_nodal_values_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_nodal_values();
	private:
		// increment
		void increment();
		// decrement
		void decrement();
		// equality
		bool equality(
			const Function_variable_iterator_representation* representation);
		// dereference
		Function_variable_handle dereference() const;
	private:
		// copy constructor
		Function_variable_iterator_representation_atomic_nodal_values(
			const Function_variable_iterator_representation_atomic_nodal_values&);
	private:
		enum FE_nodal_value_type *value_types;
		int number_of_values,value_type_index;
		struct FE_region *fe_region;
		Function_size_type number_of_versions;
		Function_variable_matrix_nodal_values_handle atomic_variable,variable;
		int time_index;
		struct FE_time_sequence *time_sequence;
};

// class Function_variable_matrix_nodal_values
// -------------------------------------------

struct Count_nodal_values_data
{
	enum FE_nodal_value_type value_type;
	Function_size_type component_number,number_of_components,number_of_values,
		version;
	struct FE_field *fe_field;
	int *node_offsets,number_of_node_offsets,*number_of_node_values;
	struct FE_node **offset_nodes;
	struct FE_time_sequence *time_sequence;
}; /* struct Count_nodal_values_data */

static int component_count_nodal_values(struct FE_node *node,
	struct FE_field *fe_field,Function_size_type component_number,
	enum FE_nodal_value_type value_type,Function_size_type version)
/*******************************************************************************
LAST MODIFIED : 27 April 2004

DESCRIPTION :
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type,*nodal_value_types;
	Function_size_type number_of_versions;
	int i,local_component_number,number_of_values;

	ENTER(component_count_nodal_values);
	local_component_number=(int)component_number-1;
	number_of_versions=
		(Function_size_type)get_FE_node_field_component_number_of_versions(node,
		fe_field,local_component_number);
	if (version<=number_of_versions)
	{
		number_of_values=1+get_FE_node_field_component_number_of_derivatives(node,
			fe_field,local_component_number);
		if (FE_NODAL_UNKNOWN!=value_type)
		{
			/*???DB.  Could use FE_nodal_value_version_exists instead */
			i=number_of_values;
			number_of_values=0;
			if (nodal_value_types=get_FE_node_field_component_nodal_value_types(node,
					fe_field,local_component_number))
			{
				nodal_value_type=nodal_value_types;
				while ((i>0)&&(value_type!= *nodal_value_type))
				{
					nodal_value_type++;
					i--;
				}
				if (i>0)
				{
					number_of_values++;
				}
				DEALLOCATE(nodal_value_types);
			}
		}
		if (0==version)
		{
			number_of_values *= number_of_versions;
		}
	}
	else
	{
		number_of_values=0;
	}
	LEAVE;

	return (number_of_values);
} /* component_count_nodal_values */

static int count_nodal_values(struct FE_node *node,
	void *count_nodal_values_data_void)
/*******************************************************************************
LAST MODIFIED : 17 November 2004

DESCRIPTION :
Counts the number of nodal values specified by <count_nodal_values_data_void> at
the <node>.
==============================================================================*/
{
	Function_size_type component_number;
	int node_number,number_of_values,number_of_times,return_code;
	struct Count_nodal_values_data *count_nodal_values_data;
	struct FE_node **offset_nodes;
	struct FE_time_sequence *time_sequence;

	ENTER(count_nodal_values);
	return_code=0;
	/* check arguments */
	if (node&&(count_nodal_values_data=(struct Count_nodal_values_data *)
		count_nodal_values_data_void))
	{
		return_code=1;
		if (offset_nodes=count_nodal_values_data->offset_nodes)
		{
			node_number=0;
			while ((node_number<count_nodal_values_data->number_of_node_offsets)&&
				(node!=offset_nodes[node_number]))
			{
				node_number++;
			}
			if (node_number<count_nodal_values_data->number_of_node_offsets)
			{
				(count_nodal_values_data->node_offsets)[node_number]=
					count_nodal_values_data->number_of_values;
			}
		}
		if (FE_field_is_defined_at_node(count_nodal_values_data->fe_field, node))
		{
			if (time_sequence=get_FE_node_field_FE_time_sequence(node, 
				count_nodal_values_data->fe_field))
			{
				if (count_nodal_values_data->time_sequence)
				{
					// Check all the values are in the total sequence
					if (number_of_times=FE_time_sequence_get_number_of_times(
						count_nodal_values_data->time_sequence))
					{
						FE_value time;
						int i,index,valid_times;
						
						valid_times=1;
						for (i=0;valid_times&&(i<number_of_times);i++)
						{
							if ((!FE_time_sequence_get_time_for_index(
								count_nodal_values_data->time_sequence,/*time_index*/i,&time))||
								(!FE_time_sequence_get_index_for_time(time_sequence,time,
								&index)))
							{
								valid_times=0;
							}
						}
						// If so just use the filtered set
						if (valid_times)
						{
							number_of_times=FE_time_sequence_get_number_of_times(
								count_nodal_values_data->time_sequence);
						}
						else
						{
							return_code=0;
							number_of_times=0;
						}
					}										
				}
				else
				{
					number_of_times=FE_time_sequence_get_number_of_times(time_sequence);
				}
			}
			else
			{
				number_of_times=1;
			}
			if ((0<(component_number=count_nodal_values_data->component_number))&&
				(component_number<=count_nodal_values_data->number_of_components))
			{
				number_of_values=component_count_nodal_values(node,
					count_nodal_values_data->fe_field,component_number,
					count_nodal_values_data->value_type,
					count_nodal_values_data->version);
			}
			else
			{
				number_of_values=0;
				for (component_number=1;component_number<=
					count_nodal_values_data->number_of_components;component_number++)
				{
					number_of_values += component_count_nodal_values(node,
						count_nodal_values_data->fe_field,component_number,
						count_nodal_values_data->value_type,
						count_nodal_values_data->version);
				}
			}
			if (number_of_times>1)
			{
				number_of_values *= number_of_times;
			}
		}
		else
		{
			number_of_values=0;
		}
		if ((offset_nodes=count_nodal_values_data->offset_nodes)&&
			(node_number<count_nodal_values_data->number_of_node_offsets))
		{
			(count_nodal_values_data->number_of_node_values)[node_number]=
				number_of_values;
		}
		count_nodal_values_data->number_of_values += number_of_values;
	}
	LEAVE;

	return (return_code);
} /* count_nodal_values */

class Function_variable_matrix_nodal_values :
	public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 22 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_finite_element;
	friend class Function_finite_element_check_derivative_functor;
	friend class Function_variable_iterator_representation_atomic_nodal_values;
	public:
		// constructors don't set row and column because too expensive
		Function_variable_matrix_nodal_values(
			const Function_finite_element_handle& function_finite_element):
			Function_variable_matrix<Scalar>(function_finite_element),
			value_type(FE_NODAL_UNKNOWN),node((struct FE_node *)NULL),
			component_number(0),version(0),
			time_sequence((struct FE_time_sequence *)NULL){};
		Function_variable_matrix_nodal_values(
			const Function_finite_element_handle& function_finite_element,
			const std::string component_name,struct FE_node *node,
			enum FE_nodal_value_type value_type,Function_size_type version,
			struct FE_time_sequence *time_sequence):
			Function_variable_matrix<Scalar>(function_finite_element),
			value_type(value_type),node(node),component_number(0),version(version),
			time_sequence(time_sequence)
		{
			if (function_finite_element)
			{
				char *name;
				int i;

				i=get_FE_field_number_of_components(
					function_finite_element->field_private);
				if (0<i)
				{
					name=(char *)NULL;
					do
					{
						i--;
						if (name)
						{
							DEALLOCATE(name);
						}
						name=get_FE_field_component_name(
							function_finite_element->field_private,i);
					} while ((i>0)&&(std::string(name)!=component_name));
					if (std::string(name)==component_name)
					{
						component_number=i+1;
					}
					DEALLOCATE(name);
				}
			}
			if (node)
			{
				ACCESS(FE_node)(node);
			}
			if (time_sequence)
			{
				ACCESS(FE_time_sequence)(time_sequence);
			}
		};
		Function_variable_matrix_nodal_values(
			const Function_finite_element_handle& function_finite_element,
			Function_size_type component_number,struct FE_node *node,
			enum FE_nodal_value_type value_type,Function_size_type version,
			struct FE_time_sequence *time_sequence):
			Function_variable_matrix<Scalar>(function_finite_element),
			value_type(value_type),node(node),component_number(component_number),
			version(version), time_sequence(time_sequence)
		{
			if (node)
			{
				ACCESS(FE_node)(node);
			}
			if (time_sequence)
			{
				ACCESS(FE_time_sequence)(time_sequence);
			}
		};
		~Function_variable_matrix_nodal_values()
		{
			DEACCESS(FE_node)(&node);
			DEACCESS(FE_time_sequence)(&time_sequence);
		};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_nodal_values(*this)));
		};
		string_handle get_string_representation()
		{
			Function_finite_element_handle function_finite_element=
				boost::dynamic_pointer_cast<Function_finite_element,Function>(
				function());
			string_handle return_string(0);

			if (function_finite_element&&(return_string=new std::string))
			{
				std::ostringstream out;

				out << "nodal_values(";
				if (0<component_number)
				{
					char *name;

					out << "component=";
					name=get_FE_field_component_name(
						function_finite_element->field_private,component_number-1);
					if (name)
					{
						out << name;
						DEALLOCATE(name);
					}
					else
					{
						out << component_number;
					}
				}
				else
				{
					out << "all components";
				}
				out << ",";
				if (node)
				{
					out << "node=" << get_FE_node_identifier(node);
				}
				else
				{
					out << "all nodes";
				}
				out << ",";
				if (0<version)
				{
					out << "version=" << version;
				}
				else
				{
					out << "all versions";
				}
				out << ",";
				if (FE_NODAL_UNKNOWN==value_type)
				{
					out << "all values";
				}
				else
				{
					out << ENUMERATOR_STRING(FE_nodal_value_type)(value_type);
				}
				out << ",";
				if (time_sequence)
				{
					FE_value fe_value;
					int i,number_of_times=FE_time_sequence_get_number_of_times(
						time_sequence);
					Matrix matrix_temp(number_of_times,1);
					
					for (i=0;i<number_of_times;i++)
					{
						FE_time_sequence_get_time_for_index(time_sequence,i,&fe_value);
						matrix_temp(i,0) = (Scalar)fe_value;
					}
					out << matrix_temp;
				}
				else
				{
					out << "all times";
				}				
				out << ")";
				*return_string=out.str();
			}

			return (return_string);
		};
		virtual Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(
				new Function_variable_iterator_representation_atomic_nodal_values(true,
				Function_variable_matrix_nodal_values_handle(
				const_cast<Function_variable_matrix_nodal_values*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(
				new Function_variable_iterator_representation_atomic_nodal_values(false,
				Function_variable_matrix_nodal_values_handle(
				const_cast<Function_variable_matrix_nodal_values*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(
				new Function_variable_iterator_representation_atomic_nodal_values(false,
				Function_variable_matrix_nodal_values_handle(
				const_cast<Function_variable_matrix_nodal_values*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(
				new Function_variable_iterator_representation_atomic_nodal_values(true,
				Function_variable_matrix_nodal_values_handle(
				const_cast<Function_variable_matrix_nodal_values*>(this)))));
		};
		Function_variable_matrix_scalar_handle operator()(
			Function_size_type row,Function_size_type column) const
		{
			Function_variable_matrix_scalar_handle result(0);

			if ((0<row)&&(1==column))
			{
				Function_size_type i;
				Function_variable_iterator iterator,iterator_end;

				i=1;
				iterator=begin_atomic();
				iterator_end=end_atomic();
				while ((i<row)&&(iterator!=iterator_end))
				{
					iterator++;
					i++;
				}
				if (iterator!=iterator_end)
				{
					result=boost::dynamic_pointer_cast<Function_variable_matrix<Scalar>,
						Function_variable>(*iterator);
				}
			}

			return (result);
		};
		Function_size_type number_of_rows() const
		{
			int number_of_components,return_code;
			struct Cmiss_region *cmiss_region;
			struct Count_nodal_values_data count_nodal_values_data;
			struct FE_field *fe_field;
			struct FE_region *fe_region;
			Function_finite_element_handle function_finite_element=
				boost::dynamic_pointer_cast<Function_finite_element,Function>(
				function());
			Function_size_type result;

			result=0;
			if (function_finite_element&&
				(fe_field=function_finite_element->field_private)&&
				(cmiss_region=function_finite_element->region())&&
				(fe_region=Cmiss_region_get_FE_region(cmiss_region))&&
				(0<(number_of_components=get_FE_field_number_of_components(fe_field))))
			{
				count_nodal_values_data.number_of_values=0;
				count_nodal_values_data.value_type=value_type;
				count_nodal_values_data.version=version;
				count_nodal_values_data.time_sequence=time_sequence;
				count_nodal_values_data.fe_field=fe_field;
				count_nodal_values_data.component_number=component_number;
				count_nodal_values_data.number_of_components=
					(Function_size_type)number_of_components;
				count_nodal_values_data.number_of_node_offsets=0;
				count_nodal_values_data.offset_nodes=(struct FE_node **)NULL;
				count_nodal_values_data.node_offsets=(int *)NULL;
				count_nodal_values_data.number_of_node_values=(int *)NULL;
				if (node)
				{
					return_code=count_nodal_values(node,(void *)&count_nodal_values_data);
				}
				else
				{
					return_code=FE_region_for_each_FE_node(fe_region,count_nodal_values,
						(void *)&count_nodal_values_data);
				}
				if (return_code)
				{
					result=(Function_size_type)(count_nodal_values_data.number_of_values);
				}
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
			FE_value fe_time;
			Function_finite_element_handle function_finite_element;
			Scalar time=0.0;

			result=false;
			if (this&&(function_finite_element=boost::dynamic_pointer_cast<
				Function_finite_element,Function>(function())))
			{
				if (time_sequence&&
					(1==FE_time_sequence_get_number_of_times(time_sequence)))
				{
					if (FE_time_sequence_get_time_for_index(time_sequence,/*time_index*/0,
						&fe_time))
					{
						time=fe_time;
					}
				}
				result=(function_finite_element->get_nodal_value)(component_number,node,
					value_type,version,time,value);
			}

			return (result);
		};
	private:
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_matrix_nodal_values_handle variable_nodal_values;

			result=false;
			if (variable_nodal_values=boost::dynamic_pointer_cast<
				Function_variable_matrix_nodal_values,Function_variable>(variable))
			{
				result=
					equivalent(variable_nodal_values->function(),function())&&
					(variable_nodal_values->component_number==component_number)&&
					(variable_nodal_values->value_type==value_type)&&
					(variable_nodal_values->version==version)&&
					(((!variable_nodal_values->time_sequence) && !time_sequence) ||
					(!compare_FE_time_sequence(variable_nodal_values->time_sequence,
					time_sequence)))&&(variable_nodal_values->node==node);
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix_nodal_values(
			const Function_variable_matrix_nodal_values& variable_nodal_values):
			Function_variable_matrix<Scalar>(variable_nodal_values),
			value_type(variable_nodal_values.value_type),
			node(variable_nodal_values.node),
			component_number(variable_nodal_values.component_number),
			version(variable_nodal_values.version),
			time_sequence(variable_nodal_values.time_sequence)
		{
			if (node)
			{
				ACCESS(FE_node)(node);
			}
			if (time_sequence)
			{
				ACCESS(FE_time_sequence)(time_sequence);
			}
		};
	private:
		enum FE_nodal_value_type value_type;
		struct FE_node *node;
		// for Function_variable_matrix_nodal_values the first version is number 1
		//   and the first component is number 1
		Function_size_type component_number,version;
		struct FE_time_sequence *time_sequence;
};

struct Get_previous_node_data
{
	enum FE_nodal_value_type value_type;
	Function_finite_element_handle function_finite_element;
	Function_size_type component_number,version;
	struct FE_node *current_node,*previous_node;
	struct FE_time_sequence *time_sequence;
}; /* struct Get_previous_node_data */

int get_previous_node(struct FE_node *node,void *get_previous_node_data_void)
/*******************************************************************************
LAST MODIFIED : 17 November 2004

DESCRIPTION :
Iterator for nodes in a region to find the node before the <current_node> in
<get_previous_node_data>.
==============================================================================*/
{
	int return_code;
	struct Get_previous_node_data *get_previous_node_data;
	Function_finite_element_handle function_finite_element;

	ENTER(get_previous_node);
	return_code=0;
	/* check arguments */
	if (node&&(get_previous_node_data=
		(struct Get_previous_node_data *)get_previous_node_data_void)&&
		(function_finite_element=get_previous_node_data->function_finite_element))
	{
		if (node==get_previous_node_data->current_node)
		{
			return_code=1;
		}
		else
		{
			Function_variable_handle variable=(function_finite_element->nodal_values)(
				get_previous_node_data->component_number,node,
				get_previous_node_data->value_type,get_previous_node_data->version,
				get_previous_node_data->time_sequence);

			if (variable&&(0<(variable->number_differentiable)()))
			{
				get_previous_node_data->previous_node=node;
			}
		}
	}
	LEAVE;

	return (return_code);
} /* get_previous_node */

struct Get_next_node_data
{
	enum FE_nodal_value_type value_type;
	Function_finite_element_handle function_finite_element;
	Function_size_type component_number,version;
	int found;
	struct FE_node *current_node;
	struct FE_time_sequence *time_sequence;
}; /* struct Get_next_node_data */

int get_next_node(struct FE_node *node,void *get_next_node_data_void)
/*******************************************************************************
LAST MODIFIED : 17 November 2004

DESCRIPTION :
Iterator for nodes in a region to find the node after the <current_node> in
<get_next_node_data>.
==============================================================================*/
{
	int return_code;
	struct Get_next_node_data *get_next_node_data;
	Function_finite_element_handle function_finite_element;

	ENTER(get_next_node);
	return_code=0;
	/* check arguments */
	if (node&&
		(get_next_node_data=(struct Get_next_node_data *)get_next_node_data_void)&&
		(function_finite_element=get_next_node_data->function_finite_element))
	{
		if (get_next_node_data->found)
		{
			Function_variable_handle variable=(function_finite_element->nodal_values)(
				get_next_node_data->component_number,node,
				get_next_node_data->value_type,get_next_node_data->version,
				get_next_node_data->time_sequence);

			if (variable&&(0<(variable->number_differentiable)()))
			{
				return_code=1;
			}
		}
		else
		{
			if (node==get_next_node_data->current_node)
			{
				get_next_node_data->found=1;
			}
		}
	}
	LEAVE;

	return (return_code);
} /* get_next_node */


// class Function_variable_iterator_representation_atomic_nodal_values
// -------------------------------------------------------------------

Function_variable_iterator_representation_atomic_nodal_values::
	Function_variable_iterator_representation_atomic_nodal_values(
	const bool begin,Function_variable_matrix_nodal_values_handle variable):
	value_types(0),number_of_values(0),value_type_index(0),fe_region(0),
	number_of_versions(0),atomic_variable(0),variable(variable),
	time_index(0),time_sequence((struct FE_time_sequence *)NULL)
//******************************************************************************
// LAST MODIFIED : 18 November 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//
//???DB.  Have to clone function_finite_element as well as variable
//==============================================================================
{
	Function_finite_element_handle function_finite_element;
	struct Cmiss_region *cmiss_region;

	if (variable&&(function_finite_element=boost::dynamic_pointer_cast<
		Function_finite_element,Function>(variable->function()))&&
		(cmiss_region=function_finite_element->region())&&
		(fe_region=Cmiss_region_get_FE_region(cmiss_region)))
	{
		ACCESS(FE_region)(fe_region);
		if (begin)
		{
			struct FE_node *node;

			node=variable->node;
			if (!node)
			{
				struct Get_next_node_data get_next_node_data;

				// want first so set found to true
				get_next_node_data.found=1;
				get_next_node_data.current_node=(struct FE_node *)NULL;
				get_next_node_data.function_finite_element=function_finite_element;
				get_next_node_data.component_number=variable->component_number;
				get_next_node_data.version=variable->version;
				get_next_node_data.time_sequence=variable->time_sequence;
				get_next_node_data.value_type=variable->value_type;
				node=FE_region_get_first_FE_node_that(fe_region,get_next_node,
					&get_next_node_data);
			}
			if (node)
			{
				Function_size_type component_number;
				Function_variable_handle out_variable;
				Function_variable_matrix_components_handle variable_finite_element(0);
				Function_variable_iterator component_end,component_iterator;

				ACCESS(FE_node)(node);
				out_variable=function_finite_element->output();
				component_iterator=out_variable->begin_atomic();
				component_end=out_variable->end_atomic();
				component_number=1;
				while ((component_iterator!=component_end)&&
					(((component_number!=variable->component_number)&&
					(0!=variable->component_number))||
					(!(variable_finite_element=boost::dynamic_pointer_cast<
					Function_variable_matrix_components,Function_variable>(
					*component_iterator)))||
					(1!=variable_finite_element->number_differentiable())))
				{
					component_iterator++;
					component_number++;
				}
				if (component_iterator!=component_end)
				{
					Function_size_type version;

					version=variable->version;
					if (0==version)
					{
						version=1;
					}
					number_of_versions=(function_finite_element->number_of_versions)(
						component_number,node);
					if (version<=number_of_versions)
					{
						number_of_values=1+(function_finite_element->number_of_derivatives)(
							component_number,node);
						if ((0<number_of_values)&&
							(value_types=(function_finite_element->nodal_value_types)(
							component_number,node)))
						{
							enum FE_nodal_value_type value_type,*value_type_ptr;

							value_type_index=0;
							value_type=FE_NODAL_UNKNOWN;
							if (FE_NODAL_UNKNOWN==variable->value_type)
							{
								value_type=value_types[value_type_index];
							}
							else
							{
								value_type_ptr=value_types;
								while ((value_type_index<number_of_values)&&
									(*value_type_ptr!=variable->value_type))
								{
									value_type_index++;
									value_type_ptr++;
								}
								if (value_type_index<number_of_values)
								{
									value_type=value_types[value_type_index];
								}
							}
							if (FE_NODAL_UNKNOWN!=value_type)
							{
								int number_of_times,valid_times;
								// The atomic sequence should be a single time from the 
								// total time sequence
								struct FE_time_sequence *particular_time_sequence=
									(struct FE_time_sequence *)NULL;

								valid_times=1;
								if (time_sequence=(function_finite_element->time_sequence)(
									node))
								{
									FE_value time;

									if (variable->time_sequence)
									{
										// Check all the values are in the total sequence
										if (number_of_times=FE_time_sequence_get_number_of_times(
											variable->time_sequence))
										{
											int i,index;

											for (i=0;valid_times&&(i<number_of_times);i++)
											{
												if ((!FE_time_sequence_get_time_for_index(
													variable->time_sequence,/*time_index*/i,&time))||
													(!FE_time_sequence_get_index_for_time(time_sequence,
													time,&index)))
												{
													valid_times=0;
												}
											}
											// If so just use the filtered set
											if (valid_times)
											{
												time_sequence=variable->time_sequence;
											}
										}										
									}
									if (valid_times)
									{
										particular_time_sequence=ACCESS(FE_time_sequence)(
											CREATE(FE_time_sequence)());
										if (number_of_times=FE_time_sequence_get_number_of_times(
											time_sequence))
										{
											FE_time_sequence_get_time_for_index(time_sequence,
												/*time_index*/0,&time);
											FE_time_sequence_set_time_and_index(
												particular_time_sequence,/*time_index*/0,time);
											time_index=0;
										}
										ACCESS(FE_time_sequence)(time_sequence);
									}
									else
									{
										time_sequence=(struct FE_time_sequence *)NULL;
									}
								}
								if (valid_times)
								{
									//Either we have a specific time or are non time varying field
									if (atomic_variable=
										Function_variable_matrix_nodal_values_handle(
										new Function_variable_matrix_nodal_values(
										function_finite_element,component_number,node,value_type,
										version,particular_time_sequence)))
									{
										atomic_variable->value_private=
											Function_variable_value_handle(
											new Function_variable_value_specific<Scalar>(
											Function_variable_matrix_set_value_function<Scalar>));
									}
								}
								DEACCESS(FE_time_sequence)(&particular_time_sequence);
							}
						}
					}
					if (!atomic_variable)
					{
						number_of_versions=0;
						number_of_values=0;
						DEALLOCATE(value_types);
						value_type_index=0;
					}
				}
				DEACCESS(FE_node)(&node);
			}
		}
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_nodal_values::clone()
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=
			new Function_variable_iterator_representation_atomic_nodal_values(*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_nodal_values::
	~Function_variable_iterator_representation_atomic_nodal_values()
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	DEALLOCATE(value_types);
	DEACCESS(FE_region)(&fe_region);
	DEACCESS(FE_time_sequence)(&time_sequence);
}

void Function_variable_iterator_representation_atomic_nodal_values::increment()
//******************************************************************************
// LAST MODIFIED : 21 January 2005
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  The incrementing order
// from fastest to slowest is: value type, version, component, node.
//==============================================================================
{
	Function_finite_element_handle function_finite_element;

	if (atomic_variable&&variable&&(function_finite_element=
		boost::dynamic_pointer_cast<Function_finite_element,Function>(
		variable->function())))
	{
		bool finished;

		finished=false;
		if (time_sequence)
		{
			FE_value time;
			int number_of_times;

			if (number_of_times=FE_time_sequence_get_number_of_times(time_sequence))
			{
				// Find what time index are we up to
				time_index++;
				if (time_index<number_of_times)
				{
					finished=true;
				}
				else
				{
					time_index=0;
				}
				// Set the time in the atomic variable sequence to the next time
				FE_time_sequence_get_time_for_index(time_sequence,time_index,&time);
				FE_time_sequence_set_time_and_index(atomic_variable->time_sequence,
					/*time_index*/0,time);
			}
		}
		if (!finished)
		{
			if (FE_NODAL_UNKNOWN==variable->value_type)
			{
				value_type_index++;
				if (value_type_index<number_of_values)
				{
					finished=true;
				}
				else
				{
					value_type_index=0;
				}
				atomic_variable->value_type=value_types[value_type_index];
			}
			if (!finished)
			{
				if (0==variable->version)
				{
					(atomic_variable->version)++;
					if (atomic_variable->version<=number_of_versions)
					{
						finished=true;
					}
					else
					{
						atomic_variable->version=1;
					}
				}
				if (!finished)
				{
					struct Get_next_node_data get_next_node_data;

					if (0==variable->component_number)
					{
						Function_size_type component_number;
						Function_variable_matrix_components_handle
							variable_finite_element(0);
						Function_variable_handle out_variable;
						Function_variable_iterator component_end,component_iterator;

						out_variable=function_finite_element->output();
						component_iterator=out_variable->begin_atomic();
						component_end=out_variable->end_atomic();
						component_number=1;
						while ((component_iterator!=component_end)&&
							(component_number!=atomic_variable->component_number))
						{
							component_iterator++;
							component_number++;
						}
						if (component_iterator!=component_end)
						{
							component_iterator++;
							component_number++;
							while ((component_iterator!=component_end)&&
								((!(variable_finite_element=boost::dynamic_pointer_cast<
								Function_variable_matrix_components,Function_variable>(
								*component_iterator)))||!equivalent(atomic_variable->function(),
								variable_finite_element->function())))
							{
								component_iterator++;
								component_number++;
							}
							if (component_iterator!=component_end)
							{
								atomic_variable->component_number=component_number;
								finished=true;
								DEALLOCATE(value_types);
								if ((0<(number_of_versions=(function_finite_element->
									number_of_versions)(atomic_variable->component_number,
									atomic_variable->node)))&&(0<(number_of_values=1+
									(function_finite_element->number_of_derivatives)(
									atomic_variable->component_number,atomic_variable->node)))&&
									(value_types=(function_finite_element->nodal_value_types)(
									atomic_variable->component_number,atomic_variable->node)))
								{
									atomic_variable->value_type=value_types[value_type_index];
								}
								else
								{
									atomic_variable=0;
								}
							}
						}
					}
					if (!finished)
					{
						struct Get_next_node_data get_next_node_data;

						if ((struct FE_node *)NULL==variable->node)
						{
							get_next_node_data.found=0;
							get_next_node_data.current_node=atomic_variable->node;
							get_next_node_data.function_finite_element=
								function_finite_element;
							get_next_node_data.component_number=variable->component_number;
							get_next_node_data.version=variable->version;
							get_next_node_data.time_sequence=variable->time_sequence;
							get_next_node_data.value_type=variable->value_type;
							if (atomic_variable->node=FE_region_get_first_FE_node_that(
								fe_region,get_next_node,&get_next_node_data))
							{
								ACCESS(FE_node)(atomic_variable->node);
								finished=true;
								if (0==variable->component_number)
								{
									Function_size_type component_number;
									Function_variable_matrix_components_handle
										variable_finite_element(0);
									Function_variable_handle out_variable;
									Function_variable_iterator component_end,component_iterator;

									out_variable=function_finite_element->output();
									component_iterator=out_variable->begin_atomic();
									component_end=out_variable->end_atomic();
									component_number=1;
									while ((component_iterator!=component_end)&&
										((!(variable_finite_element=boost::dynamic_pointer_cast<
										Function_variable_matrix_components,Function_variable>(
										*component_iterator)))||
										(1!=variable_finite_element->number_differentiable())))
									{
										component_iterator++;
										component_number++;
									}
									if ((component_iterator!=component_end)&&
										(variable_finite_element=boost::dynamic_pointer_cast<
										Function_variable_matrix_components,Function_variable>(
										*component_iterator))&&
										(1==variable_finite_element->number_differentiable()))
									{
										atomic_variable->component_number=component_number;
									}
									else
									{
										atomic_variable=0;
									}
								}
								if (atomic_variable)
								{
									DEALLOCATE(value_types);
									if ((0<(number_of_versions=(function_finite_element->
										number_of_versions)(atomic_variable->component_number,
										atomic_variable->node)))&&(0<(number_of_values=1+
										(function_finite_element->number_of_derivatives)(
										atomic_variable->component_number,atomic_variable->node)))&&
										(value_types=(function_finite_element->nodal_value_types)(
										atomic_variable->component_number,atomic_variable->node)))
									{
										atomic_variable->value_type=value_types[value_type_index];
									}
								}
							}
							DEACCESS(FE_node)(&(get_next_node_data.current_node));
						}
						if (!finished)
						{
							// end
							atomic_variable=0;
						}
					}
				}
			}
		}
	}
}

void Function_variable_iterator_representation_atomic_nodal_values::decrement()
//******************************************************************************
// LAST MODIFIED : 21 January 2005
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  The decrementing order
// from fastest to slowest is: value type, version, component, node.
//==============================================================================
{
	Function_finite_element_handle function_finite_element;

	if (variable&&(function_finite_element=boost::dynamic_pointer_cast<
		Function_finite_element,Function>(variable->function())))
	{
		if (atomic_variable)
		{
			bool finished;

			finished=false;
			if (time_sequence)
			{
				FE_value time;
				int number_of_times;
			
				if (number_of_times=FE_time_sequence_get_number_of_times(time_sequence))
				{
					// Find what time index are we up to
					time_index--;
					if (0<=time_index)
					{
						finished=true;
					}
					else
					{
						time_index=number_of_times-1;
					}
					// Set the time in the atomic variable sequence to the next time
					FE_time_sequence_get_time_for_index(time_sequence,time_index,&time);
					FE_time_sequence_set_time_and_index(atomic_variable->time_sequence,
						/*time_index*/0,time);
				}
			}
			if (!finished)
			{
				if (FE_NODAL_UNKNOWN==variable->value_type)
				{
					if (0<value_type_index)
					{
						value_type_index--;
						finished=true;
					}
					else
					{
						value_type_index=number_of_values-1;
					}
					atomic_variable->value_type=value_types[value_type_index];
				}
				if (!finished)
				{
					if (0==variable->version)
					{
						if (1<atomic_variable->version)
						{
							(atomic_variable->version)--;
							finished=true;
						}
						else
						{
							atomic_variable->version=number_of_versions;
						}
					}
					if (!finished)
					{
						if (0==variable->component_number)
						{
							Function_size_type component_number;
							Function_variable_matrix_components_handle
								variable_finite_element;
							Function_variable_handle out_variable;
							Function_variable_iterator component_begin,component_end,
								component_iterator;

							out_variable=function_finite_element->output();
							component_begin=out_variable->begin_atomic();
							component_end=out_variable->end_atomic();
							component_iterator=component_begin;
							component_number=1;
							while ((component_iterator!=component_end)&&
								(component_number!=atomic_variable->component_number))
							{
								component_iterator++;
								component_number++;
							}
							if (component_iterator!=component_begin)
							{
								component_iterator--;
								component_number--;
								while ((component_iterator!=component_begin)&&
									((!(variable_finite_element=boost::dynamic_pointer_cast<
									Function_variable_matrix_components,Function_variable>(
									*component_iterator)))||!equivalent(atomic_variable->
									function(),variable_finite_element->function())))
								{
									component_iterator--;
									component_number--;
								}
								if ((variable_finite_element=boost::dynamic_pointer_cast<
									Function_variable_matrix_components,Function_variable>(
									*component_iterator))&&equivalent(atomic_variable->function(),
									variable_finite_element->function()))
								{
									atomic_variable->component_number=component_number;
									finished=true;
									DEALLOCATE(value_types);
									if ((0<(number_of_versions=(function_finite_element->
										number_of_versions)(atomic_variable->component_number,
										atomic_variable->node)))&&(0<(number_of_values=1+
										(function_finite_element->number_of_derivatives)(
										atomic_variable->component_number,atomic_variable->node)))&&
										(value_types=(function_finite_element->nodal_value_types)(
											atomic_variable->component_number,atomic_variable->node)))
									{
										atomic_variable->value_type=value_types[value_type_index];
										atomic_variable->version=number_of_versions;
									}
									else
									{
										atomic_variable=0;
									}
								}
							}
						}
						if (!finished)
						{
							if ((struct FE_node *)NULL==variable->node)
							{
								struct Get_previous_node_data get_previous_node_data;

								//???DB.  This can be made more efficient by adding iterators to
								//  LISTs or by adding a GET_PREVIOUS function to LISTs
								get_previous_node_data.previous_node=(struct FE_node *)NULL;
								get_previous_node_data.current_node=atomic_variable->node;
								get_previous_node_data.function_finite_element=
									function_finite_element;
								get_previous_node_data.component_number=
									variable->component_number;
								get_previous_node_data.version=variable->version;
								get_previous_node_data.time_sequence=variable->time_sequence;
								get_previous_node_data.value_type=variable->value_type;
								FE_region_get_first_FE_node_that(fe_region,get_previous_node,
									&get_previous_node_data);
								if (get_previous_node_data.previous_node)
								{
									DEACCESS(FE_node)(&(atomic_variable->node));
									atomic_variable->node=ACCESS(FE_node)(
										get_previous_node_data.previous_node);
									finished=true;
									//???DB.  This can be made more efficient by adding iterators
									//  to LISTs or by adding a GET_PREVIOUS function to LISTs
									if (0==variable->component_number)
									{
										Function_size_type component_number;
										Function_variable_matrix_components_handle
											variable_finite_element;
										Function_variable_handle out_variable;
										Function_variable_iterator component_begin,component_end,
											component_iterator;

										out_variable=function_finite_element->output();
										component_begin=out_variable->begin_atomic();
										component_end=out_variable->end_atomic();
										component_number=
											(function_finite_element->number_of_components)();
										component_iterator=component_end;
										component_iterator--;
										while ((component_iterator!=component_begin)&&
											((!(variable_finite_element=boost::dynamic_pointer_cast<
											Function_variable_matrix_components,Function_variable>(
											*component_iterator)))||
											(1!=variable_finite_element->number_differentiable())))
										{
											component_iterator--;
											component_number--;
										}
										if ((component_iterator!=component_end)&&
											(variable_finite_element=boost::dynamic_pointer_cast<
												Function_variable_matrix_components,Function_variable>(
													*component_iterator))&&
											(1==variable_finite_element->number_differentiable()))
										{
											atomic_variable->component_number=component_number;
										}
										else
										{
											atomic_variable=0;
										}
									}
									if (atomic_variable)
									{
										DEALLOCATE(value_types);
										if ((0<(number_of_versions=(function_finite_element->
											number_of_versions)(atomic_variable->component_number,
											atomic_variable->node)))&&(0<(number_of_values=1+
											(function_finite_element->number_of_derivatives)(
											atomic_variable->component_number,
											atomic_variable->node)))&&(value_types=
											(function_finite_element->nodal_value_types)(
											atomic_variable->component_number,atomic_variable->node)))
										{
											atomic_variable->value_type=value_types[value_type_index];
										}
									}
								}
							}
							if (!finished)
							{
								// end
								atomic_variable=0;
							}
						}
					}
				}
			}
		}
		else
		{
			struct FE_node *node;

			node=variable->node;
			if (!node)
			{
				struct Get_previous_node_data get_previous_node_data;

				//???DB.  This can be made more efficient by adding iterators to
				//  LISTs or by adding a GET_PREVIOUS function to LISTs
				// find last by setting current_node to NULL
				get_previous_node_data.previous_node=(struct FE_node *)NULL;
				get_previous_node_data.current_node=(struct FE_node *)NULL;
				get_previous_node_data.function_finite_element=
					function_finite_element;
				get_previous_node_data.component_number=
					variable->component_number;
				get_previous_node_data.version=variable->version;
				get_previous_node_data.time_sequence=variable->time_sequence;
				get_previous_node_data.value_type=variable->value_type;
				FE_region_get_first_FE_node_that(fe_region,get_previous_node,
					&get_previous_node_data);
				node=get_previous_node_data.previous_node;
			}
			if (node)
			{
				Function_size_type component_number;
				Function_variable_handle out_variable;
				Function_variable_matrix_components_handle variable_finite_element(0);
				Function_variable_iterator component_begin,component_end,
					component_iterator;

				ACCESS(FE_node)(node);
				out_variable=function_finite_element->output();
				component_begin=out_variable->begin_atomic();
				component_end=out_variable->end_atomic();
				if (component_begin!=component_end)
				{
					if (0==variable->component_number)
					{
						component_number=(function_finite_element->number_of_components)();
						component_iterator=component_end;
						component_iterator--;
						while ((component_iterator!=component_begin)&&
							((!(variable_finite_element=boost::dynamic_pointer_cast<
							Function_variable_matrix_components,Function_variable>(
							*component_iterator)))||
							(1!=variable_finite_element->number_differentiable())))
						{
							component_iterator--;
							component_number--;
						}
					}
					else
					{
						component_iterator=component_begin;
						component_number=1;
						while ((component_iterator!=component_end)&&
							(component_number!=variable->component_number))
						{
							component_iterator++;
							component_number++;
						}
					}
					if ((component_iterator!=component_end)&&
						(variable_finite_element=boost::dynamic_pointer_cast<
						Function_variable_matrix_components,Function_variable>(
						*component_iterator))&&
						(1==variable_finite_element->number_differentiable()))
					{
						Function_size_type version;

						number_of_versions=(function_finite_element->number_of_versions)(
							component_number,node);
						version=variable->version;
						if (0==version)
						{
							version=number_of_versions;
						}
						if (version<=number_of_versions)
						{
							number_of_values=1+(function_finite_element->
								number_of_derivatives)(component_number,node);
							if ((0<number_of_values)&&
								(value_types=(function_finite_element->nodal_value_types)(
								component_number,node)))
							{
								enum FE_nodal_value_type value_type,*value_type_ptr;

								value_type_index=number_of_values-1;
								value_type=FE_NODAL_UNKNOWN;
								if (FE_NODAL_UNKNOWN==variable->value_type)
								{
									value_type=value_types[value_type_index];
								}
								else
								{
									value_type_ptr=value_types+value_type_index;
									while ((value_type_index>=0)&&
										(*value_type_ptr!=variable->value_type))
									{
										value_type_index--;
										value_type_ptr--;
									}
									if (value_type_index>=0)
									{
										value_type=value_types[value_type_index];
									}
								}
								if (FE_NODAL_UNKNOWN!=value_type)
								{
									int number_of_times,valid_times;
									// The atomic sequence should be a single time from the 
									// total time sequence
									struct FE_time_sequence *particular_time_sequence=
										(struct FE_time_sequence *)NULL;

									valid_times=1;
									if (time_sequence=(function_finite_element->time_sequence)(
										node))
									{
										FE_value time;

										if (variable->time_sequence)
										{
											// Check all the values are in the total sequence
											if (number_of_times=FE_time_sequence_get_number_of_times(
												variable->time_sequence))
											{
												int i,index;

												for (i=0;valid_times&&(i<number_of_times);i++)
												{
													if ((!FE_time_sequence_get_time_for_index(
														variable->time_sequence,/*time_index*/i,&time))||
														(!FE_time_sequence_get_index_for_time(time_sequence,
														time,&index)))
													{
														valid_times=0;
													}
												}
												// If so just use the filtered set
												if (valid_times)
												{
													time_sequence=variable->time_sequence;
												}
											}										
										}
										if (valid_times)
										{
											particular_time_sequence=ACCESS(FE_time_sequence)(
												CREATE(FE_time_sequence)());
											if (number_of_times=FE_time_sequence_get_number_of_times(
												 time_sequence))
											{
												FE_time_sequence_get_time_for_index(time_sequence,
													/*time_index*/number_of_times-1,&time);
												FE_time_sequence_set_time_and_index(
													particular_time_sequence,/*time_index*/0,time);
												time_index=0;
											}
											ACCESS(FE_time_sequence)(time_sequence);
										}
										else
										{
											time_sequence=(struct FE_time_sequence *)NULL;
										}
									}
									if (valid_times)
									{
										// either we have a specific time or are non time varying
										//   field
										if (atomic_variable=
											Function_variable_matrix_nodal_values_handle(new
											Function_variable_matrix_nodal_values(
											function_finite_element,component_number,node,value_type,
											version,particular_time_sequence)))
										{
											atomic_variable->value_private=
												Function_variable_value_handle(
												new Function_variable_value_specific<Scalar>(
												Function_variable_matrix_set_value_function<Scalar>));
										}
									}
									DEACCESS(FE_time_sequence)(&particular_time_sequence);
								}
							}
						}
						if (!atomic_variable)
						{
							number_of_versions=0;
							number_of_values=0;
							DEALLOCATE(value_types);
							value_type_index=0;
						}
					}
				}
				DEACCESS(FE_node)(&node);
			}
		}
	}
}

bool Function_variable_iterator_representation_atomic_nodal_values::equality(
	const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_nodal_values
		*representation_nodal_values=dynamic_cast<
		const Function_variable_iterator_representation_atomic_nodal_values *>(
		representation);

	result=false;
	if (representation_nodal_values)
	{
		result=equivalent(atomic_variable,
			representation_nodal_values->atomic_variable);
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_nodal_values::dereference()
	const
//******************************************************************************
// LAST MODIFIED : 22 March 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_nodal_values::
	Function_variable_iterator_representation_atomic_nodal_values(
	const Function_variable_iterator_representation_atomic_nodal_values&
	representation):Function_variable_iterator_representation(),
	value_types((enum FE_nodal_value_type *)NULL),
	number_of_values(representation.number_of_values),
	value_type_index(representation.value_type_index),
	fe_region(representation.fe_region),
	number_of_versions(representation.number_of_versions),
	atomic_variable(0),variable(representation.variable),
	time_index(0),time_sequence(representation.time_sequence)
//******************************************************************************
// LAST MODIFIED : 18 November 2004
//
// DESCRIPTION :
// Copy constructor
//==============================================================================
{
	if (representation.value_types)
	{
		if (ALLOCATE(value_types,enum FE_nodal_value_type,number_of_values))
		{
			int i;

			for (i=0;i<number_of_values;i++)
			{
				value_types[i]=(representation.value_types)[i];
			}
		}
		else
		{
			//???DB.  Throw an exception ?
			number_of_values=0;
			value_type_index=0;
		}
	}
	if (fe_region)
	{
		ACCESS(FE_region)(fe_region);
	}
	if (time_sequence)
	{
		ACCESS(FE_time_sequence)(time_sequence);
	}
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_nodal_values,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}

// forward declaration so that can use _handle
class Function_variable_value_time;
typedef boost::intrusive_ptr<Function_variable_value_time>
	Function_variable_value_time_handle;

class Function_variable_value_time :
	public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 17 February 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_value_time(
			const Function_finite_element_handle& function_finite_element):
			Function_variable_matrix<Scalar>(function_finite_element,1,1){};
		// destructor
		~Function_variable_value_time(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_value_time(*this)));
		};
		string_handle get_string_representation()
		{
			string_handle return_string;
			std::ostringstream out;

			if (return_string=new std::string)
			{
				out << "value_time";
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
				Function_finite_element_handle function_finite_element=
					boost::dynamic_pointer_cast<Function_finite_element,Function>(function());

				result=Function_variable_matrix_scalar_handle(
					new Function_variable_value_time(function_finite_element));
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
			Function_finite_element_handle function_finite_element;

			result=false;
			if ((1==row_private)&&(1==column_private)&&(function_finite_element=
				boost::dynamic_pointer_cast<Function_finite_element,Function>(function())))
			{
				value=function_finite_element->time_value();
				result=true;
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_value_time(
			const Function_variable_value_time& variable_value_time):
			Function_variable_matrix<Scalar>(variable_value_time){}
};

// global classes
// ==============

// class Function_element
// ----------------------

Function_element::Function_element(struct FE_element *element):
	element_private(element)
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (element_private)
	{
		ACCESS(FE_element)(element_private);
	}
}

Function_element::~Function_element()
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	DEACCESS(FE_element)(&element_private);
}

string_handle Function_element::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 14 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (element_private)
		{
			out << "element=" << FE_element_get_cm_number(element_private);
		}
		else
		{
			out << "no element";
		}
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_element::input()
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_element(
		Function_element_handle(this))));
}

Function_variable_handle Function_element::output()
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_element(
		Function_element_handle(this))));
}

bool Function_element::operator==(const Function& function) const
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
		try
		{
			const Function_element& function_element=
				dynamic_cast<const Function_element&>(function);

			result=(element_private==function_element.element_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

Function_size_type Function_element::dimension()
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Returns the element's dimension.
//==============================================================================
{
	return ((Function_size_type)get_FE_element_dimension(element_private));
}

struct FE_element* Function_element::element_value()
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Returns the element.
//==============================================================================
{
	return (element_private);
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_element::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (get_value(atomic_variable));
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_element::evaluate(Function_variable_handle)
//******************************************************************************
// LAST MODIFIED : 14 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (true);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

bool Function_element::evaluate_derivative(Scalar&,Function_variable_handle,
	std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
//==============================================================================
{
	// can't differentiate an element
	return (false);
}

bool Function_element::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_element_handle atomic_element_variable;

	result=false;
	if ((atomic_element_variable=boost::dynamic_pointer_cast<
		Function_variable_element,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_element_variable->function())&&
		atomic_value&&(atomic_value->value()))
	{
		Function_variable_value_element_handle value_element;

		if ((std::string("Element")==(atomic_value->value())->type())&&
			(value_element=boost::dynamic_pointer_cast<
			Function_variable_value_element,Function_variable_value>(atomic_value->
			value())))
		{
			result=value_element->set(element_private,atomic_value);
			if (element_private)
			{
				ACCESS(FE_element)(element_private);
			}
		}
		if (result)
		{
			set_not_evaluated();
		}
	}

	return (result);
}

Function_handle Function_element::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_element_handle atomic_variable_element;

	if ((atomic_variable_element=boost::dynamic_pointer_cast<
		Function_variable_element,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_element->function()))
	{
		result=Function_handle(new Function_element(*this));
	}

	return (result);
}

Function_element::Function_element(
	const Function_element& function_element):Function(function_element),
	element_private(function_element.element_private)
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (element_private)
	{
		ACCESS(FE_element)(element_private);
	}
}

Function_element& Function_element::operator=(
	const Function_element& function_element)
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	if (function_element.element_private)
	{
		if (element_private)
		{
			DEACCESS(FE_element)(&element_private);
		}
		element_private=ACCESS(FE_element)(function_element.element_private);
	}
	else
	{
		if (element_private)
		{
			DEACCESS(FE_element)(&element_private);
		}
	}

	return (*this);
}

// class Function_element_xi
// -------------------------

Function_element_xi::Function_element_xi(struct FE_element *element,
	const Vector& xi):Function(),element_private(element),xi_private(xi)
//******************************************************************************
// LAST MODIFIED : 31 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (element_private)
	{
		if (xi_private.size()==
			(Function_size_type)get_FE_element_dimension(element_private))
		{
			ACCESS(FE_element)(element_private);
		}
		else
		{
			throw std::invalid_argument("Function_element_xi::Function_element_xi");
		}
	}
}

Function_element_xi::~Function_element_xi()
//******************************************************************************
// LAST MODIFIED : 31 March 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	DEACCESS(FE_element)(&element_private);
}

string_handle Function_element_xi::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 14 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (element_private)
		{
			out << "element=" << FE_element_get_cm_number(element_private) <<
				" xi=" << xi_private;
		}
		else
		{
			out << "no element_xi";
		}
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_element_xi::input()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_element_xi_element_xi(
		Function_element_xi_handle(this))));
}

Function_variable_handle Function_element_xi::output()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_element_xi_element_xi(
		Function_element_xi_handle(this))));
}

bool Function_element_xi::operator==(const Function& function) const
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
		try
		{
			const Function_element_xi& function_element_xi=
				dynamic_cast<const Function_element_xi&>(function);
			Function_size_type i;

			result=((element_private==function_element_xi.element_private)&&
				((i=xi_private.size())==function_element_xi.xi_private.size()));
			while (result&&(i>0))
			{
				i--;
				result=(xi_private[i]==function_element_xi.xi_private[i]);
			}
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

Function_variable_handle Function_element_xi::element()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the element input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_element_xi_element_xi(
		Function_element_xi_handle(this),true,false)));
}

Function_variable_handle Function_element_xi::element_xi()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the element/xi input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_element_xi_element_xi(
		Function_element_xi_handle(this))));
}

Function_variable_handle Function_element_xi::xi()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_element_xi_element_xi(
		Function_element_xi_handle(this),false)));
}

Function_variable_handle Function_element_xi::xi(Function_size_type index)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_element_xi_element_xi(
		Function_element_xi_handle(this),index)));
}

Function_size_type Function_element_xi::number_of_xi()
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Returns the number of xi dimensions.
//==============================================================================
{
	return (xi_private.size());
}

struct FE_element* Function_element_xi::element_value()
//******************************************************************************
// LAST MODIFIED : 1 April 2004
//
// DESCRIPTION :
// Returns the element.
//==============================================================================
{
	return (element_private);
}

Scalar Function_element_xi::xi_value(Function_size_type index)
//******************************************************************************
// LAST MODIFIED : 5 May 2004
//
// DESCRIPTION :
// Returns the xi value.
//
//???DB.  Should it return all xi as a vector?
//==============================================================================
{
	return (xi_private[index-1]);
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_element_xi::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (get_value(atomic_variable));
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_element_xi::evaluate(Function_variable_handle)
//******************************************************************************
// LAST MODIFIED : 14 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (true);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

bool Function_element_xi::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_element_xi_element_xi_handle atomic_dependent_variable,
		atomic_independent_variable;

	result=false;
	if ((atomic_dependent_variable=boost::dynamic_pointer_cast<
		Function_variable_element_xi_element_xi,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_dependent_variable->function())&&
		(1==atomic_dependent_variable->number_differentiable()))
	{
		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_element_xi_element_xi,Function_variable>(
			atomic_independent_variables.front()))&&
			equivalent(atomic_dependent_variable,atomic_independent_variable))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}

	return (result);
}

bool Function_element_xi::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_element_xi_element_xi_handle atomic_element_xi_variable;

	result=false;
	if ((atomic_element_xi_variable=boost::dynamic_pointer_cast<
		Function_variable_element_xi_element_xi,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_element_xi_variable->function())&&
		atomic_value&&(atomic_value->value()))
	{
		Function_variable_value_element_handle value_element;
		Function_variable_value_scalar_handle value_scalar;

		if ((atomic_element_xi_variable->element_private)&&
			(std::string("Element")==(atomic_value->value())->type())&&
			(value_element=boost::dynamic_pointer_cast<
			Function_variable_value_element,Function_variable_value>(atomic_value->
			value())))
		{
			result=value_element->set(element_private,atomic_value);
			if (element_private)
			{
				ACCESS(FE_element)(element_private);
				xi_private.resize(get_FE_element_dimension(element_private));
			}
			else
			{
				xi_private.resize(0);
			}
		}
		else if ((atomic_element_xi_variable->xi_private)&&
			(std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			result=value_scalar->set(
				xi_private[(atomic_element_xi_variable->indices)[0]-1],atomic_value);
		}
		if (result)
		{
			set_not_evaluated();
		}
	}

	return (result);
}

Function_handle Function_element_xi::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_element_xi_element_xi_handle atomic_variable_element_xi;

	if ((atomic_variable_element_xi=boost::dynamic_pointer_cast<
		Function_variable_element_xi_element_xi,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_element_xi->function()))
	{
		if (atomic_variable_element_xi->xi_private)
		{
			Matrix result_matrix(1,1);

			if (atomic_variable_element_xi->get_xi(result_matrix(0,0)))
			{
				result=Function_handle(new Function_matrix<Scalar>(result_matrix));
			}
		}
		else
		{
			struct FE_element* element;

			if (atomic_variable_element_xi->get_element(element))
			{
				result=Function_handle(new Function_element(element));
			}
		}
	}

	return (result);
}

Function_element_xi::Function_element_xi(
	const Function_element_xi& function_element_xi):Function(function_element_xi),
	element_private(function_element_xi.element_private),
	xi_private(function_element_xi.xi_private)
//******************************************************************************
// LAST MODIFIED : 1 April 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (element_private)
	{
		if (xi_private.size()==
			(Function_size_type)get_FE_element_dimension(element_private))
		{
			ACCESS(FE_element)(element_private);
		}
		else
		{
			throw std::invalid_argument("Function_element_xi::Function_element_xi");
		}
	}
}

Function_element_xi& Function_element_xi::operator=(
	const Function_element_xi& function_element_xi)
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	Function_size_type number_of_xi=function_element_xi.xi_private.size();

	if (function_element_xi.element_private)
	{
		if ((0==number_of_xi)||
			(number_of_xi==(Function_size_type)get_FE_element_dimension(
			function_element_xi.element_private)))
		{
			if (element_private)
			{
				DEACCESS(FE_element)(&element_private);
			}
			element_private=ACCESS(FE_element)(function_element_xi.element_private);
			xi_private.resize(number_of_xi);
			xi_private=function_element_xi.xi_private;
		}
		else
		{
			throw std::invalid_argument("Function_element_xi::operator=");
		}
	}
	else
	{
		if (element_private)
		{
			DEACCESS(FE_element)(&element_private);
		}
		xi_private.resize(number_of_xi);
		xi_private=function_element_xi.xi_private;
	}

	return (*this);
}

// class Function_finite_element
// -----------------------------

Function_finite_element::Function_finite_element(struct FE_field *field):
	Function(),time_private(0),element_private((struct FE_element *)NULL),
	field_private(field),node_private((struct FE_node *)NULL),
	components_private(),xi_private()
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (field)
	{
		ACCESS(FE_field)(field_private);
		components_private.resize(get_FE_field_number_of_components(field_private));
	}
}

Function_finite_element::~Function_finite_element()
//******************************************************************************
// LAST MODIFIED : 1 April 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	DEACCESS(FE_element)(&element_private);
	DEACCESS(FE_field)(&field_private);
	DEACCESS(FE_node)(&node_private);
}

string_handle Function_finite_element::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 2 April 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (field_private)
		{
			out << get_FE_field_name(field_private);
		}
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_finite_element::input()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_composite(
		element_xi(),nodal_values())));
}

Function_variable_handle Function_finite_element::output()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_components(
		Function_finite_element_handle(this))));
}

bool Function_finite_element::operator==(const Function& function) const
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
		try
		{
			const Function_finite_element& function_finite_element=
				dynamic_cast<const Function_finite_element&>(function);

			result=(field_private==function_finite_element.field_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

Function_variable_handle Function_finite_element::component(
	std::string component_name)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the component output.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_components(
		Function_finite_element_handle(this),component_name)));
}

Function_variable_handle Function_finite_element::component(
	Function_size_type component_number)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the component output.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_components(
		Function_finite_element_handle(this),component_number)));
}

Function_variable_handle Function_finite_element::element()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the element input.
//==============================================================================
{
	return (Function_variable_handle
		(new Function_variable_element_xi_finite_element(
		Function_finite_element_handle(this),true,false)));
}

Function_variable_handle Function_finite_element::element_xi()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the element/xi input.
//==============================================================================
{
	return (Function_variable_handle
		(new Function_variable_element_xi_finite_element(
		Function_finite_element_handle(this))));
}

Function_variable_handle Function_finite_element::nodal_values()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_nodal_values(
		Function_finite_element_handle(this))));
}

Function_variable_handle Function_finite_element::nodal_values(
	std::string component_name,struct FE_node *node,
	enum FE_nodal_value_type value_type,Function_size_type version,
	struct FE_time_sequence *time_sequence)
//******************************************************************************
// LAST MODIFIED : 18 November 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_nodal_values(
		Function_finite_element_handle(this),component_name,node,value_type,
		version,time_sequence)));
}

Function_variable_handle Function_finite_element::nodal_values(
	Function_size_type component_number,struct FE_node *node,
	enum FE_nodal_value_type value_type,Function_size_type version,
	struct FE_time_sequence *time_sequence)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_nodal_values(
		Function_finite_element_handle(this),component_number,node,value_type,
		version,time_sequence)));
}

Function_variable_handle Function_finite_element::nodal_values_component(
	std::string component_name)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_nodal_values(
		Function_finite_element_handle(this),component_name,(struct FE_node *)NULL,
		FE_NODAL_UNKNOWN,0,(struct FE_time_sequence *)NULL)));
}

Function_variable_handle Function_finite_element::nodal_values_component(
	Function_size_type component_number)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_nodal_values(
		Function_finite_element_handle(this),component_number,
		(struct FE_node *)NULL,FE_NODAL_UNKNOWN,0,(struct FE_time_sequence *)NULL)));
}

Function_variable_handle
	Function_finite_element::nodal_values(struct FE_node *node)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_nodal_values(
		Function_finite_element_handle(this),(Function_size_type)0,node,
		FE_NODAL_UNKNOWN,0,(struct FE_time_sequence *)NULL)));
}

Function_variable_handle Function_finite_element::nodal_values(
	enum FE_nodal_value_type value_type)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_nodal_values(
		Function_finite_element_handle(this),(Function_size_type)0,
		(struct FE_node *)NULL,value_type,0,(struct FE_time_sequence *)NULL)));
}

Function_variable_handle Function_finite_element::nodal_values(
	Function_size_type version)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_nodal_values(
		Function_finite_element_handle(this),(Function_size_type)0,
		(struct FE_node *)NULL,FE_NODAL_UNKNOWN,version,
		(struct FE_time_sequence *)NULL)));
}

Function_variable_handle Function_finite_element::time()
//******************************************************************************
// LAST MODIFIED : 16 February 2005
//
// DESCRIPTION :
// Returns the time input.
//==============================================================================
{
	return (Function_variable_handle
		(new Function_variable_value_time(
		Function_finite_element_handle(this))));
}

Scalar Function_finite_element::time_value()
//******************************************************************************
// LAST MODIFIED : 16 February 2005
//
// DESCRIPTION :
// Returns the time input.
//==============================================================================
{
	return (this->time_private);
}

Function_variable_handle Function_finite_element::xi()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (Function_variable_handle
		(new Function_variable_element_xi_finite_element(
		Function_finite_element_handle(this),false)));
}

Function_variable_handle Function_finite_element::xi(
	Function_size_type index)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (Function_variable_handle
		(new Function_variable_element_xi_finite_element(
		Function_finite_element_handle(this),index)));
}

Function_size_type Function_finite_element::number_of_components() const
//******************************************************************************
// LAST MODIFIED : 2 April 2004
//
// DESCRIPTION :
// Return the number of components.
//==============================================================================
{
	return ((Function_size_type)get_FE_field_number_of_components(field_private));
}

struct Cmiss_region *Function_finite_element::region() const
//******************************************************************************
// LAST MODIFIED : 4 November 2004
//
// DESCRIPTION :
// Return the region that the field is defined for.
//
// NB.  The calling program should use ACCESS(Cmiss_region) and
//   DEACCESS(Cmiss_region) to manage the lifetime of the returned region
//==============================================================================
{
	struct Cmiss_region *result;
	struct FE_region *fe_region;

	result=(struct Cmiss_region *)NULL;
	if (fe_region=FE_field_get_FE_region(field_private))
	{
		FE_region_get_Cmiss_region(fe_region,&result);
	}

	return (result);
}

Function_size_type Function_finite_element::number_of_versions(
	Function_size_type component_number,struct FE_node *node) const
//******************************************************************************
// LAST MODIFIED : 2 April 2004
//
// DESCRIPTION :
// Return the number of versions for the component at the node.
//==============================================================================
{
	int local_component_number;

	local_component_number=(int)component_number-1;

	return ((Function_size_type)get_FE_node_field_component_number_of_versions(
		node,field_private,local_component_number));
}

Function_size_type Function_finite_element::number_of_derivatives(
	Function_size_type component_number,struct FE_node *node) const
//******************************************************************************
// LAST MODIFIED : 2 April 2004
//
// DESCRIPTION :
// Return the number of derivatives for the component at the node.
//==============================================================================
{
	int local_component_number;

	local_component_number=(int)component_number-1;

	return ((Function_size_type)get_FE_node_field_component_number_of_derivatives(
		node,field_private,local_component_number));
}

enum FE_nodal_value_type *Function_finite_element::nodal_value_types(
	Function_size_type component_number,struct FE_node *node) const
//******************************************************************************
// LAST MODIFIED : 2 April 2004
//
// DESCRIPTION :
// Return the nodal value types for the component at the node.
//
// NB.  The calling program should DEALLOCATE the returned array when its no
//   longer needed
//==============================================================================
{
	int local_component_number;

	local_component_number=(int)component_number-1;

	return (get_FE_node_field_component_nodal_value_types(node,field_private,
		local_component_number));
}

struct FE_time_sequence *Function_finite_element::time_sequence(
	struct FE_node *node) const
//******************************************************************************
// LAST MODIFIED : 22 November 2004
//
// DESCRIPTION :
// Return the time sequence at the node.
//==============================================================================
{
	return (get_FE_node_field_FE_time_sequence(node,field_private));
}

bool Function_finite_element::get_nodal_value(
	Function_size_type component_number,struct FE_node *node,
	enum FE_nodal_value_type value_type,Function_size_type version,
	Scalar time,Scalar& value)
//******************************************************************************
// LAST MODIFIED : 24 May 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	FE_value fe_value;
	struct FE_field_component fe_field_component;

	result=false;
	if (this)
	{
		fe_field_component.field=field_private;
		fe_field_component.number=component_number-1;
		if (get_FE_nodal_FE_value_value(node,
			&fe_field_component,(version-1),value_type,time,&fe_value))
		{
			value=(Scalar)fe_value;
			result=true;
		}
	}

	return (result);
}

bool Function_finite_element::set_nodal_value(
	Function_size_type component_number,struct FE_node *node,
	enum FE_nodal_value_type value_type,Function_size_type version,Scalar time,
	Scalar value)
//******************************************************************************
// LAST MODIFIED : 24 May 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	FE_value fe_value;
	struct FE_field_component fe_field_component;

	result=false;
	if (this)
	{
		fe_field_component.field=field_private;
		fe_field_component.number=component_number-1;
		fe_value=(FE_value)value;
		if (set_FE_nodal_FE_value_value(node,
			&fe_field_component,(version-1),value_type,time,fe_value))
		{
			result=true;
		}
	}

	return (result);
}

Function_size_type Function_finite_element::number_of_xi() const
//******************************************************************************
// LAST MODIFIED : 2 April 2004
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (xi_private.size());
}

struct FE_element* Function_finite_element::element_value() const
//******************************************************************************
// LAST MODIFIED : 2 April 2004
//
// DESCRIPTION :
// Returns the element.
//==============================================================================
{
	return (element_private);
}

Scalar Function_finite_element::xi_value(Function_size_type index) const
//******************************************************************************
// LAST MODIFIED : 4 May 2004
//
// DESCRIPTION :
// Returns the xi value.
//
//???DB.  Should it return all xi as a vector?
//==============================================================================
{
	return (xi_private[index-1]);
}

bool Function_finite_element::component_value(Function_size_type number,
	Scalar& value) const
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
// Returns the component value.
//==============================================================================
{
	bool result;

	result=false;
	if ((0<number)&&(number<=components_private.size()))
	{
		result=true;
		value=components_private[number-1];
	}

	return (result);
}

int Function_finite_element::define_on_Cmiss_node(struct FE_node *node,
	struct FE_time_sequence *fe_time_sequence, 
	struct FE_node_field_creator *fe_node_field_creator)
//******************************************************************************
// LAST MODIFIED : 9 November 2004
//
// DESCRIPTION :
// .
//==============================================================================
{
	int result;

	result=0;
	if (node&&fe_node_field_creator)
	{
		result=define_FE_field_at_node(node,field_private,fe_time_sequence,
			fe_node_field_creator);
	}

	return (result);
}

int Function_finite_element::define_tensor_product_basis_on_element(
	struct FE_element *element, int dimension, enum FE_basis_type basis_type)
//******************************************************************************
// LAST MODIFIED : 10 November 2004
//
// DESCRIPTION :
// Defines a tensor product basis on <element> with the specified <dimension> 
// and <basis_type>.  This does not support mixed basis types in the tensor
// product.
//==============================================================================
{
	int result;

	result=0;
	if (element && dimension && basis_type)
	{
		result = FE_element_define_tensor_product_basis(element,
			dimension, basis_type, field_private);
	}

	return (result);
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_finite_element::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 3 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_matrix_components_handle atomic_variable_finite_element;

	if (atomic_variable_finite_element=boost::dynamic_pointer_cast<
		Function_variable_matrix_components,Function_variable>(atomic_variable))
	{
#if defined (BEFORE_CACHING)
		FE_value *xi_coordinates;
		int element_dimension;

		xi_coordinates=(FE_value *)NULL;
		if (equivalent(Function_handle(this),
			atomic_variable_finite_element->function())&&
			(0<atomic_variable_finite_element->row_private)&&field_private&&
			(atomic_variable_finite_element->row_private<=number_of_components())&&
			((node_private&&!element_private)||(!node_private&&element_private&&
			(0<(element_dimension=get_FE_element_dimension(element_private)))&&
			((Function_size_type)element_dimension==xi_private.size())&&
			ALLOCATE(xi_coordinates,FE_value,element_dimension))))
		{
			FE_value fe_value;
			int i,local_component_number;

			local_component_number=
				(int)(atomic_variable_finite_element->row_private)-1;
			if (xi_coordinates)
			{
				for (i=0;i<element_dimension;i++)
				{
					xi_coordinates[i]=(FE_value)(xi_private[i]);
				}
			}
			if (calculate_FE_field(field_private,local_component_number,node_private,
				element_private,xi_coordinates,(FE_value)time_private,&fe_value))
			{
				Matrix result_matrix(1,1);

				components_private[local_component_number]=(Scalar)fe_value;
				result_matrix(0,0)=(Scalar)fe_value;
				result=Function_handle(new Function_matrix<Scalar>(result_matrix));
			}
		}
		DEALLOCATE(xi_coordinates);
#else // defined (BEFORE_CACHING)
		if (equivalent(Function_handle(this),
			atomic_variable_finite_element->function()))
		{
			if (!evaluated())
			{
				FE_value *component_values,*xi_coordinates;
				Function_size_type local_number_of_components;
				int element_dimension;

				xi_coordinates=(FE_value *)NULL;
				component_values=(FE_value *)NULL;
				if ((0<atomic_variable_finite_element->row_private)&&field_private&&
					(atomic_variable_finite_element->row_private<=
					(local_number_of_components=number_of_components()))&&
					((node_private&&!element_private)||(!node_private&&element_private&&
					(0<(element_dimension=get_FE_element_dimension(element_private)))&&
					((Function_size_type)element_dimension==xi_private.size())&&
					ALLOCATE(xi_coordinates,FE_value,element_dimension)))&&
					ALLOCATE(component_values,FE_value,local_number_of_components))
				{
					Function_size_type j;
					int i;

					if (xi_coordinates)
					{
						for (i=0;i<element_dimension;i++)
						{
							xi_coordinates[i]=(FE_value)(xi_private[i]);
						}
					}
					if (calculate_FE_field(field_private,-1,node_private,element_private,
						xi_coordinates,(FE_value)time_private,component_values))
					{
						for (j=0;j<local_number_of_components;j++)
						{
							components_private[j]=(Scalar)(component_values[j]);
						}
						set_evaluated();
					}
				}
				DEALLOCATE(component_values);
				DEALLOCATE(xi_coordinates);
			}
			if (evaluated())
			{
				result=get_value(atomic_variable);
			}
		}
#endif // defined (BEFORE_CACHING)
	}
	else
	{
		result=get_value(atomic_variable);
	}

	return (result);
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_finite_element::evaluate(Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 14 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result(true);
	Function_variable_matrix_components_handle atomic_variable_finite_element;

	if (atomic_variable_finite_element=boost::dynamic_pointer_cast<
		Function_variable_matrix_components,Function_variable>(atomic_variable))
	{
#if defined (BEFORE_CACHING)
		FE_value *xi_coordinates;
		int element_dimension;

		xi_coordinates=(FE_value *)NULL;
		if (equivalent(Function_handle(this),
			atomic_variable_finite_element->function())&&
			(0<atomic_variable_finite_element->row_private)&&field_private&&
			(atomic_variable_finite_element->row_private<=number_of_components())&&
			((node_private&&!element_private)||(!node_private&&element_private&&
			(0<(element_dimension=get_FE_element_dimension(element_private)))&&
			((Function_size_type)element_dimension==xi_private.size())&&
			ALLOCATE(xi_coordinates,FE_value,element_dimension))))
		{
			FE_value fe_value;
			int i,local_component_number;

			result=false;
			local_component_number=
				(int)(atomic_variable_finite_element->row_private)-1;
			if (xi_coordinates)
			{
				for (i=0;i<element_dimension;i++)
				{
					xi_coordinates[i]=(FE_value)(xi_private[i]);
				}
			}
			if (calculate_FE_field(field_private,local_component_number,node_private,
				element_private,xi_coordinates,(FE_value)time_private,&fe_value))
			{
				components_private[local_component_number]=(Scalar)fe_value;
				result=true;
			}
		}
		DEALLOCATE(xi_coordinates);
#else // defined (BEFORE_CACHING)
		if (equivalent(Function_handle(this),
			atomic_variable_finite_element->function()))
		{
			if (!evaluated())
			{
				FE_value *component_values,*xi_coordinates;
				Function_size_type local_number_of_components;
				int element_dimension;

				result=false;
				xi_coordinates=(FE_value *)NULL;
				component_values=(FE_value *)NULL;
				if ((0<atomic_variable_finite_element->row_private)&&field_private&&
					(atomic_variable_finite_element->row_private<=
					(local_number_of_components=number_of_components()))&&
					((node_private&&!element_private)||(!node_private&&element_private&&
					(0<(element_dimension=get_FE_element_dimension(element_private)))&&
					((Function_size_type)element_dimension==xi_private.size())&&
					ALLOCATE(xi_coordinates,FE_value,element_dimension)))&&
					ALLOCATE(component_values,FE_value,local_number_of_components))
				{
					Function_size_type j;
					int i;

					if (xi_coordinates)
					{
						for (i=0;i<element_dimension;i++)
						{
							xi_coordinates[i]=(FE_value)(xi_private[i]);
						}
					}
					if (calculate_FE_field(field_private,-1,node_private,element_private,
						xi_coordinates,(FE_value)time_private,component_values))
					{
						for (j=0;j<local_number_of_components;j++)
						{
							components_private[j]=(Scalar)(component_values[j]);
						}
						set_evaluated();
						result=true;
					}
				}
				DEALLOCATE(component_values);
				DEALLOCATE(xi_coordinates);
			}
		}
#endif // defined (BEFORE_CACHING)
	}

	return (result);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

class Function_finite_element_check_derivative_functor
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//
// NOTES :
// 1. Composite variables are already handled by evaluate_derivative.
//
// ???DB.  Only valid for monomial standard basis and nodal value based.  This
//   is enforced by <FE_element_field_values_get_monomial_component_info> and
//   <FE_element_field_values_get_component_values>.
//==============================================================================
{
	public:
		Function_finite_element_check_derivative_functor(
			Function_finite_element_handle function_finite_element,
			Function_size_type component_number,bool& zero_derivative,
			Function_variable_matrix_nodal_values_handle& nodal_values_variable,
			ublas::vector<Function_variable_element_xi_finite_element_handle>::
			iterator element_xi_variable_iterator):zero_derivative(zero_derivative),
			function_finite_element(function_finite_element),
			component_number(component_number),
			nodal_values_variable(nodal_values_variable),
			element_xi_variable_iterator(element_xi_variable_iterator){}
		~Function_finite_element_check_derivative_functor(){};
		int operator() (Function_variable_handle & variable)
		{
			*element_xi_variable_iterator=
				Function_variable_element_xi_finite_element_handle(0);
			if (!zero_derivative)
			{
				Function_variable_element_xi_finite_element_handle
					variable_element_xi_handle;
				Function_variable_matrix_nodal_values_handle
					variable_nodal_values_handle;

				if ((variable_element_xi_handle=boost::dynamic_pointer_cast<
					Function_variable_element_xi_finite_element,Function_variable>(
					variable))&&equivalent(
					variable_element_xi_handle->function(),function_finite_element)&&
					(variable_element_xi_handle->xi_private))
				{
					// not checking order of derivative against order of polynomial
					*element_xi_variable_iterator=variable_element_xi_handle;
				}
				else if ((variable_nodal_values_handle=
					boost::dynamic_pointer_cast<Function_variable_matrix_nodal_values,
					Function_variable>(variable))&&equivalent(
					variable_nodal_values_handle->function(),function_finite_element)&&
					(component_number==variable_nodal_values_handle->component_number))
				{
					if (nodal_values_variable)
					{
						// because the nodal values are the coefficients for a linear
						//   combination
						zero_derivative=true;
					}
					else
					{
						nodal_values_variable=variable_nodal_values_handle;
					}
				}
				else
				{
					zero_derivative=true;
				}
			}
			element_xi_variable_iterator++;

			return (1);
		};
	private:
		bool& zero_derivative;
		Function_finite_element_handle function_finite_element;
		Function_size_type component_number;
		Function_variable_matrix_nodal_values_handle& nodal_values_variable;
		ublas::vector<Function_variable_element_xi_finite_element_handle>::iterator
			element_xi_variable_iterator;
};

#if defined (OLD_CODE)
static int extract_component_values(
	struct FE_element_field_values *element_field_values,
	Function_size_type number_of_components,Function_size_type component_number,
	int **numbers_of_component_values_address,
	FE_value ***component_values_address)
/*******************************************************************************
LAST MODIFIED : 19 May 2004

DESCRIPTION :
Extracts the values for the component(s) (<number_of_components> and
<component_number>) from <element_field_values> and puts in
<*component_values_address> (after allocating storage).

If <*numbers_of_component_values_address> is NULL then storage is allocated for
it and the number of values extracted for each component entered.  Otherwise,
checks that the number of values extracted for each component agrees with the
entries.
==============================================================================*/
{
	FE_value **component_values;
	Function_size_type i;
	int extract_numbers_of_component_values,local_component_number,
		number_of_component_values,*numbers_of_component_values,return_code;

	ENTER(extract_component_values);
	return_code=0;
	Assert(element_field_values&&
		(0<number_of_components)&&((0==component_number)||
		((0<component_number)&&(component_number<=number_of_components)))&&
		numbers_of_component_values_address&&component_values_address,
		std::logic_error("extract_component_values.  Invalid argument(s)"));
	local_component_number=(int)component_number-1;
	extract_numbers_of_component_values=
		!(*numbers_of_component_values_address);
	/* allocate storage for components */
	if (extract_numbers_of_component_values)
	{
		if (0==component_number)
		{
			ALLOCATE(numbers_of_component_values,int,number_of_components);
		}
		else
		{
			ALLOCATE(numbers_of_component_values,int,1);
		}
	}
	else
	{
		numbers_of_component_values= *numbers_of_component_values_address;
	}
	if (0==component_number)
	{
		ALLOCATE(component_values,FE_value *,number_of_components);
	}
	else
	{
		ALLOCATE(component_values,FE_value *,1);
	}
	if (component_values&&numbers_of_component_values)
	{
		if (0==component_number)
		{
			i=0;
			return_code=1;
			while (return_code&&(i<number_of_components))
			{
				if (return_code=FE_element_field_values_get_component_values(
					element_field_values,(int)i,&number_of_component_values,
					component_values+i))
				{
					if (extract_numbers_of_component_values)
					{
						numbers_of_component_values[i]=number_of_component_values;
					}
					else
					{
						if (numbers_of_component_values[i]!=number_of_component_values)
						{
							DEALLOCATE(component_values[i]);
							return_code=0;
						}
					}
					i++;
				}
			}
			if (!return_code)
			{
				while (i>0)
				{
					i--;
					DEALLOCATE(component_values[i]);
				}
			}
		}
		else
		{
			if (return_code=FE_element_field_values_get_component_values(
				element_field_values,local_component_number,&number_of_component_values,
				component_values))
			{
				if (extract_numbers_of_component_values)
				{
					*numbers_of_component_values=number_of_component_values;
				}
				else
				{
					if (*numbers_of_component_values!=number_of_component_values)
					{
						DEALLOCATE(*component_values);
						return_code=0;
					}
				}
			}
		}
		if (return_code)
		{
			*component_values_address=component_values;
			*numbers_of_component_values_address=numbers_of_component_values;
		}
	}
	if (!return_code)
	{
		DEALLOCATE(component_values);
		if (extract_numbers_of_component_values)
		{
			DEALLOCATE(numbers_of_component_values);
		}
	}
	LEAVE;

	return (return_code);
} /* extract_component_values */

static int nodal_value_calculate_component_values(
	Function_finite_element_handle function_finite_element,
	struct FE_field *fe_field,Function_size_type component_number,
	FE_value fe_time,struct FE_element *element,struct FE_node *node,
	enum FE_nodal_value_type nodal_value_type,Function_size_type version,
	int *number_of_element_field_nodes_address,
	struct FE_node ***element_field_nodes_address,
	struct FE_element_field_values *element_field_values,
	int *number_of_nodal_values_address,int **numbers_of_component_values_address,
	FE_value ****component_values_address)
/*******************************************************************************
LAST MODIFIED : 6 August 2004

DESCRIPTION :
Calculate the component values for the derivatives of the specified components
(<component_number>) of <fe_field> in the <element> with respect to the
specified nodal values (<node>, <nodal_value_type> and <version>.

<*number_of_nodal_values_address> is calculated and an array with an entry for
each of the specified nodal values (<*number_of_nodal_values_address> long> is
allocated, NULLed and assigned to <*component_values_address>.  For each nodal
value, if the derivative is not identically zero then a 2 dimensional array
(first index is component number and second index is component value number) is
allocated, filled-in and assigned to the corresponding entry in
<*component_values_address>.

The interpolation function for the <fe_field> component(s) is assumed to be a
linear combination of basis functions polynomial in the element coordinates.
The coefficients for the linear combination are the nodal values.  So, the
derivative with respect to a nodal value is the corresponding basis function.
The component values are for the monomials.  To calculate these, all the nodal
values for the <element> are set to zero, the nodal value of interest is set to
one and <calculate_FE_element_field_values> is used.

<*number_of_element_field_nodes_address> and <*element_field_nodes_address> are
for the <fe_field> and <element> and can be passed in or computed in here.
<element_field_values> is working storage that is passed in.

???DB.  Can get all from <function_finite_element>.  Do multiple times?
==============================================================================*/
{
	FE_value ***component_values,***nodal_value_component_values;
	Function_size_type j,k,number_of_components,number_of_versions;
	int *element_field_nodal_value_offsets,*element_field_number_of_nodal_values,
		*element_field_number_of_specified_nodal_values,i,l,local_component_number,
		number_of_element_field_nodes,number_of_nodal_values,
		number_of_saved_element_field_nodes,number_of_values,return_code;
	struct Count_nodal_values_data count_nodal_values_data;
	struct FE_region *fe_region;
	struct FE_node **element_field_nodes;

	ENTER(nodal_value_calculate_component_values);
	return_code=0;
	/* check arguments */
	fe_region=FE_field_get_FE_region(fe_field);
	number_of_components=
		(Function_size_type)get_FE_field_number_of_components(fe_field);
	Assert(function_finite_element&&fe_field&&fe_region&&
		(0<number_of_components)&&((0==component_number)||
		((0<component_number)&&(component_number<=number_of_components)))&&
		element&&element_field_values&&number_of_nodal_values_address&&
		component_values_address,std::logic_error(
		"nodal_value_calculate_component_values.  Invalid argument(s)"));
	local_component_number=(int)component_number-1;
	if (number_of_element_field_nodes_address&&element_field_nodes_address)
	{
		number_of_element_field_nodes= *number_of_element_field_nodes_address;
		element_field_nodes= *element_field_nodes_address;
	}
	else
	{
		number_of_element_field_nodes=0;
		element_field_nodes=(struct FE_node **)NULL;
	}
	if ((number_of_element_field_nodes>0)&&element_field_nodes)
	{
		return_code=0;
	}
	else
	{
		return_code=calculate_FE_element_field_nodes(element,fe_field,
			&number_of_element_field_nodes,&element_field_nodes,
			(struct FE_element *)NULL);
	}
	if (return_code)
	{
		//???DB.  To be done/Needs finishing
		/* set up working storage */
		Function_handle *element_field_saved_nodal_values;

		ALLOCATE(element_field_nodal_value_offsets,int,
			number_of_element_field_nodes);
		ALLOCATE(element_field_number_of_specified_nodal_values,int,
			number_of_element_field_nodes);
		ALLOCATE(element_field_number_of_nodal_values,int,
			number_of_element_field_nodes);
		element_field_saved_nodal_values=
			new Function_handle[number_of_element_field_nodes];
		if (element_field_nodal_value_offsets&&
			element_field_number_of_specified_nodal_values&&
			element_field_number_of_nodal_values&&element_field_saved_nodal_values)
		{
			/* NULL working storage and calculate total number of nodal values for
				each <element_field_node> */
			for (i=0;i<number_of_element_field_nodes;i++)
			{
				element_field_saved_nodal_values[i]=Function_handle(0);
				element_field_nodal_value_offsets[i]= -1;
				element_field_number_of_specified_nodal_values[i]=0;
				element_field_number_of_nodal_values[i]=0;
				if (0==component_number)
				{
					for (j=0;j<number_of_components;j++)
					{
						number_of_versions=(Function_size_type)
							get_FE_node_field_component_number_of_versions(
							element_field_nodes[i],fe_field,(int)j);
						if (version<=number_of_versions)
						{
							number_of_values=
								(1+get_FE_node_field_component_number_of_derivatives(
								element_field_nodes[i],fe_field,(int)j));
							if (0==version)
							{
								element_field_number_of_nodal_values[i] +=
									number_of_values*number_of_versions;
							}
							else
							{
								element_field_number_of_nodal_values[i] += number_of_values;
							}
						}
					}
				}
				else
				{
					number_of_versions=(Function_size_type)
						get_FE_node_field_component_number_of_versions(
						element_field_nodes[i],fe_field,local_component_number);
					if (version<=number_of_versions)
					{
						number_of_values=
							(1+get_FE_node_field_component_number_of_derivatives(
							element_field_nodes[i],fe_field,local_component_number));
						if (0==version)
						{
							element_field_number_of_nodal_values[i] +=
								number_of_values*number_of_versions;
						}
						else
						{
							element_field_number_of_nodal_values[i] += number_of_values;
						}
					}
				}
			}
			/* calculate total number of specified nodal values (<number_of_values>)
				and the number of specified nodal values for the
				<element_field_nodes> */
			count_nodal_values_data.number_of_values=0;
			count_nodal_values_data.value_type=nodal_value_type;
			count_nodal_values_data.version=version;
			count_nodal_values_data.fe_field=fe_field;
			count_nodal_values_data.component_number=component_number;
			count_nodal_values_data.number_of_components=number_of_components;
			count_nodal_values_data.number_of_node_offsets=
				number_of_element_field_nodes;
			count_nodal_values_data.offset_nodes=element_field_nodes;
			count_nodal_values_data.node_offsets=element_field_nodal_value_offsets;
			count_nodal_values_data.number_of_node_values=
				element_field_number_of_specified_nodal_values;
			if (node)
			{
				return_code=count_nodal_values(node,(void *)&count_nodal_values_data);
			}
			else
			{
				return_code=FE_region_for_each_FE_node(fe_region,
					count_nodal_values,(void *)&count_nodal_values_data);
			}
			number_of_nodal_values=count_nodal_values_data.number_of_values;
			if (return_code&&(0<number_of_nodal_values))
			{
				/* allocate storage for the component_values */
				if (return_code&&ALLOCATE(component_values,FE_value **,
					number_of_nodal_values))
				{
					/* NULL the component values array */
					nodal_value_component_values=component_values;
					for (i=number_of_nodal_values;i>0;i--)
					{
						*nodal_value_component_values=(FE_value **)NULL;
						nodal_value_component_values++;
					}
					/* save and zero all the nodal values for the <fe_field> on the
						element */
					number_of_saved_element_field_nodes=0;
					while (return_code&&(number_of_saved_element_field_nodes<
						number_of_element_field_nodes))
					{
						Function_variable_matrix_nodal_values_handle variable(new
							Function_variable_matrix_nodal_values(function_finite_element,
							(Function_size_type)0,
							element_field_nodes[number_of_saved_element_field_nodes],
							FE_NODAL_UNKNOWN,0));

						if (element_field_saved_nodal_values[
							number_of_saved_element_field_nodes]=variable->get_value())
						{
							Function_size_type i,number_of_values=
								((element_field_saved_nodal_values[
								number_of_saved_element_field_nodes])->output())->
								number_differentiable();
							Matrix zero_vector(number_of_values,1);

							number_of_saved_element_field_nodes++;
							for (i=0;i<number_of_values;i++)
							{
								zero_vector(i,0)=(Scalar)0;
							}
							if (!((variable->set_value)(Function_matrix_scalar_handle(
								new Function_matrix<Scalar>(zero_vector)))))
							{
								return_code=0;
							}
						}
						else
						{
							return_code=0;
						}
					}
					/* calculate component values for nodal value derivatives */
					i=0;
					while (return_code&&(i<number_of_element_field_nodes))
					{
						if ((element_field_nodal_value_offsets[i]>=0)&&
							(element_field_number_of_specified_nodal_values[i]>0))
						{
							Function_size_type number_of_values=(Function_size_type)
								(element_field_number_of_specified_nodal_values[i]);
							Matrix unit_vector_values(number_of_values,1);
							Function_matrix_scalar_handle unit_vector(
								new Function_matrix<Scalar>(unit_vector_values));
							Function_variable_matrix_nodal_values_handle variable(new
								Function_variable_matrix_nodal_values(
								function_finite_element,component_number,
								element_field_nodes[i],nodal_value_type,version));

							for (j=0;j<number_of_values;j++)
							{
								(*unit_vector)(j,0)=(Scalar)0;
							}
							nodal_value_component_values=component_values+
								element_field_nodal_value_offsets[i];
							Assert((int)number_of_values==
								element_field_number_of_specified_nodal_values[i],
								std::logic_error("nodal_value_calculate_component_values.  "
								"Incorrect number of nodal values"));
							j=0;
							while (return_code&&(j<number_of_values))
							{
								/* set nodal values */
								(*unit_vector)(j,0)=(Scalar)1;
								if ((variable->set_value)(unit_vector)&&
									clear_FE_element_field_values(element_field_values)&&
									FE_element_field_values_set_no_modify(
									element_field_values)&&
									calculate_FE_element_field_values(element,fe_field,
									fe_time,(char)0,element_field_values,
									(struct FE_element *)NULL))
								{
									return_code=extract_component_values(
										element_field_values,number_of_components,
										component_number,numbers_of_component_values_address,
										nodal_value_component_values);
								}
								else
								{
									return_code=0;
								}
								/* set nodal values */
								(*unit_vector)(j,0)=(Scalar)0;
								nodal_value_component_values++;
								j++;
							}
							if (return_code)
							{
								// zero the last 1 from the above loop
								return_code=(variable->set_value)(unit_vector);
							}
							if (!return_code)
							{
								while (j>0)
								{
									j--;
									nodal_value_component_values--;
									if (*nodal_value_component_values)
									{
										if (0==component_number)
										{
											for (k=0;k<number_of_components;k++)
											{
												DEALLOCATE((*nodal_value_component_values)[k]);
											}
										}
										else
										{
											DEALLOCATE((*nodal_value_component_values)[0]);
										}
										DEALLOCATE(*nodal_value_component_values);
									}
								}
							}
						}
						i++;
					}
					if (return_code)
					{
						*number_of_nodal_values_address=number_of_nodal_values;
						*component_values_address=component_values;
					}
					else
					{
						while (i>0)
						{
							i--;
							if ((element_field_nodal_value_offsets[i]>=0)&&
								(element_field_number_of_specified_nodal_values[i]>0))
							{
								nodal_value_component_values=component_values+
									element_field_nodal_value_offsets[i];
								for (l=0;
									l<element_field_number_of_specified_nodal_values[i];l++)
								{
									if (*nodal_value_component_values)
									{
										if (0==component_number)
										{
											for (k=0;k<number_of_components;k++)
											{
												DEALLOCATE((*nodal_value_component_values)[k]);
											}
										}
										else
										{
											DEALLOCATE((*nodal_value_component_values)[0]);
										}
									}
									DEALLOCATE(*nodal_value_component_values);
									nodal_value_component_values++;
								}
							}
						}
						DEALLOCATE(component_values);
					}
					/* reset all the nodal values for the <fe_field> on the element */
					i=0;
					while (return_code&&(i<number_of_saved_element_field_nodes))
					{
						Function_variable_matrix_nodal_values_handle variable(new
							Function_variable_matrix_nodal_values(function_finite_element,
							(Function_size_type)0,element_field_nodes[i],FE_NODAL_UNKNOWN,0));

						return_code=(variable->set_value)(
							element_field_saved_nodal_values[i]);
						i++;
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				return_code=0;
			}
			/* destroy working computed values for saving current values and
				setting temporary values */
			for (i=0;i<number_of_element_field_nodes;i++)
			{
				element_field_saved_nodal_values[i]=Function_matrix_scalar_handle(0);
			}
		}
		else
		{
			return_code=0;
		}
		/* get rid of working storage */
		delete [] element_field_saved_nodal_values;
		DEALLOCATE(element_field_number_of_nodal_values);
		DEALLOCATE(element_field_number_of_specified_nodal_values);
		DEALLOCATE(element_field_nodal_value_offsets);
	}
	if (return_code&&number_of_element_field_nodes_address&&
		element_field_nodes_address)
	{
		*number_of_element_field_nodes_address=number_of_element_field_nodes;
		*element_field_nodes_address=element_field_nodes;
	}
	LEAVE;

	return (return_code);
} /* nodal_value_calculate_component_values */

static int extract_component_monomial_info(
	struct FE_element_field_values *element_field_values,
	Function_size_type number_of_components,Function_size_type component_number,
	int number_of_xi,int **numbers_of_component_values_address,
	int ***component_monomial_info_address,FE_value **monomial_values_address)
/*******************************************************************************
LAST MODIFIED : 19 May 2004

DESCRIPTION :
Extracts the component monomial info (<number_of_components> and
<component_number>) from <element_field_values> and puts in
<*component_monomial_info_address> (after allocating storage).

If <*numbers_of_component_values_address> is NULL then storage is allocated for
it and the number of values for each component calculated and entered.
Otherwise, checks that the number of values extracted for each component agrees
with the entries.

If <monomial_values_address> is not NULL and <*monomial_values_address> is NULL,
then storage for the maximum number of component values is allocated and
assigned to <*monomial_values_address>.
==============================================================================*/
{
	FE_value *monomial_values;
	Function_size_type i;
	int **component_monomial_info,extract_numbers_of_component_values,j,
		local_component_number,maximum_number_of_component_values,
		number_of_component_values,*numbers_of_component_values,return_code;

	ENTER(extract_component_monomial_info);
	return_code=0;
	Assert(element_field_values&&
		(0<number_of_components)&&((0==component_number)||
		((0<component_number)&&(component_number<=number_of_components)))&&
		(0<number_of_xi)&&numbers_of_component_values_address&&
		component_monomial_info_address,
		std::logic_error("extract_component_monomial_info.  Invalid argument(s)"));
	local_component_number=(int)component_number-1;
	/* allocate storage for components */
	extract_numbers_of_component_values=
		!(*numbers_of_component_values_address);
	/* allocate storage for components */
	if (extract_numbers_of_component_values)
	{
		if (0==component_number)
		{
			ALLOCATE(numbers_of_component_values,int,number_of_components);
		}
		else
		{
			ALLOCATE(numbers_of_component_values,int,1);
		}
	}
	else
	{
		numbers_of_component_values= *numbers_of_component_values_address;
	}
	if (0==component_number)
	{
		ALLOCATE(component_monomial_info,int *,number_of_components);
	}
	else
	{
		ALLOCATE(component_monomial_info,int *,1);
	}
	if (numbers_of_component_values&&component_monomial_info)
	{
		if (0==component_number)
		{
			i=0;
			return_code=1;
			maximum_number_of_component_values=0;
			while (return_code&&(i<number_of_components))
			{
				if (ALLOCATE(component_monomial_info[i],int,number_of_xi+1))
				{
					return_code=FE_element_field_values_get_monomial_component_info(
						element_field_values,(int)i,component_monomial_info[i]);
					if (return_code)
					{
						number_of_component_values=1;
						for (j=1;j<=component_monomial_info[i][0];j++)
						{
							number_of_component_values *= 1+component_monomial_info[i][j];
						}
						if (number_of_component_values>maximum_number_of_component_values)
						{
							maximum_number_of_component_values=number_of_component_values;
						}
						if (extract_numbers_of_component_values)
						{
							numbers_of_component_values[i]=number_of_component_values;
						}
						else
						{
							if (numbers_of_component_values[i]!=number_of_component_values)
							{
								return_code=0;
							}
						}
					}
					i++;
				}
				else
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				if (monomial_values_address&&!(*monomial_values_address))
				{
					if ((0<maximum_number_of_component_values)&&ALLOCATE(monomial_values,
						FE_value,maximum_number_of_component_values))
					{
						*monomial_values_address=monomial_values;
					}
					else
					{
						return_code=0;
					}
				}
				if (return_code)
				{
					*numbers_of_component_values_address=numbers_of_component_values;
					*component_monomial_info_address=component_monomial_info;
				}
			}
			if (!return_code)
			{
				while (i>0)
				{
					i--;
					DEALLOCATE(component_monomial_info[i]);
				}
			}
		}
		else
		{
			i=0;
			return_code=1;
			maximum_number_of_component_values=0;
			if (ALLOCATE(component_monomial_info[i],int,number_of_xi+1))
			{
				return_code=FE_element_field_values_get_monomial_component_info(
					element_field_values,local_component_number,
					component_monomial_info[i]);
				if (return_code)
				{
					number_of_component_values=1;
					for (j=1;j<=component_monomial_info[i][0];j++)
					{
						number_of_component_values *= 1+component_monomial_info[i][j];
					}
					if (number_of_component_values>maximum_number_of_component_values)
					{
						maximum_number_of_component_values=number_of_component_values;
					}
					if (extract_numbers_of_component_values)
					{
						numbers_of_component_values[i]=number_of_component_values;
					}
					else
					{
						if (numbers_of_component_values[i]!=number_of_component_values)
						{
							return_code=0;
						}
					}
				}
				i++;
			}
			else
			{
				return_code=0;
			}
			if (return_code)
			{
				if (monomial_values_address&&!(*monomial_values_address))
				{
					if ((0<maximum_number_of_component_values)&&ALLOCATE(monomial_values,
						FE_value,maximum_number_of_component_values))
					{
						*monomial_values_address=monomial_values;
					}
					else
					{
						return_code=0;
					}
				}
				if (return_code)
				{
					*numbers_of_component_values_address=numbers_of_component_values;
					*component_monomial_info_address=component_monomial_info;
				}
			}
			if (!return_code)
			{
				while (i>0)
				{
					i--;
					DEALLOCATE(component_monomial_info[i]);
				}
			}
		}
	}
	if (!return_code)
	{
		DEALLOCATE(component_monomial_info);
		if (extract_numbers_of_component_values)
		{
			DEALLOCATE(numbers_of_component_values);
		}
	}
	LEAVE;

	return (return_code);
} /* extract_component_monomial_info */
#endif // defined (OLD_CODE)

static int calculate_monomial_derivative_values(int number_of_xi_coordinates,
	int *xi_orders,int *xi_derivative_orders,FE_value *xi_coordinates,
	FE_value *monomial_derivative_values)
/*******************************************************************************
LAST MODIFIED : 22 March 2004

DESCRIPTION :
For the specified monomials (<number_of_xi_coordinates> and order<=<xi_orders>
for each xi), calculates there derivatives (<xi_derivative_orders>) at
<xi_coordinates> and stores in <monomial_derivative_values>.  Expects the memory
to already be allocated for the <monomial_derivative_values>.
NB.  xi_1 is varying slowest (xi_n fastest)
==============================================================================*/
{
	FE_value *temp_value,*value,xi,*xi_coordinate,xi_power;
	int derivative_order,i,j,k,order,number_of_values,return_code,
		*xi_derivative_order,*xi_order,zero_derivative;

	ENTER(calculate_monomial_derivative_values);
	return_code=0;
	Assert((0<number_of_xi_coordinates)&&(xi_order=xi_orders)&&
		(xi_derivative_order=xi_derivative_orders)&&(xi_coordinate=xi_coordinates)&&
		monomial_derivative_values,std::logic_error(
		"calculate_monomial_derivative_values.  Invalid argument(s)"));
	{
		/* check for zero derivative */
		zero_derivative=0;
		number_of_values=1;
		for (i=number_of_xi_coordinates;i>0;i--)
		{
			order= *xi_order;
			if (*xi_derivative_order>order)
			{
				zero_derivative=1;
			}
			number_of_values *= order+1;
			xi_derivative_order++;
			xi_order++;
		}
		value=monomial_derivative_values;
		if (zero_derivative)
		{
			/* zero derivative */
			for (k=number_of_values;k>0;k--)
			{
				*value=(FE_value)0;
				value++;
			}
		}
		else
		{
			xi_order=xi_orders;
			xi_derivative_order=xi_derivative_orders;
			*value=1;
			number_of_values=1;
			for (i=number_of_xi_coordinates;i>0;i--)
			{
				xi= *xi_coordinate;
				xi_coordinate++;
				derivative_order= *xi_derivative_order;
				xi_derivative_order++;
				order= *xi_order;
				xi_order++;
				if (0<derivative_order)
				{
					xi_power=1;
					for (j=derivative_order;j>1;j--)
					{
						xi_power *= (FE_value)j;
					}
					value += number_of_values*(derivative_order-1);
					for (j=derivative_order;j<=order;j++)
					{
						temp_value=monomial_derivative_values;
						for (k=number_of_values;k>0;k--)
						{
							value++;
							*value=(*temp_value)*xi_power;
							temp_value++;
						}
						xi_power *= xi*(float)(j+1)/(float)(j-derivative_order+1);
					}
					temp_value=monomial_derivative_values;
					for (k=derivative_order*number_of_values;k>0;k--)
					{
						*temp_value=0;
						temp_value++;
					}
				}
				else
				{
					xi_power=xi;
					for (j=order;j>0;j--)
					{
						temp_value=monomial_derivative_values;
						for (k=number_of_values;k>0;k--)
						{
							value++;
							*value=(*temp_value)*xi_power;
							temp_value++;
						}
						xi_power *= xi;
					}
				}
				number_of_values *= (order+1);
			}
		}
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* calculate_monomial_derivative_values */

bool Function_finite_element::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 11 January 2005
//
// DESCRIPTION :
// ???DB.  Throw an exception for failure?
//==============================================================================
{
	bool result;
	Function_size_type local_number_of_xi;
	Function_variable_element_xi_finite_element_handle atomic_variable_element_xi;
	Function_variable_matrix_components_handle atomic_variable_finite_element;
	Function_variable_matrix_nodal_values_handle atomic_variable_nodal_values;

	result=false;
	if ((atomic_variable_element_xi=boost::dynamic_pointer_cast<
		Function_variable_element_xi_finite_element,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_element_xi->function()))
	{
		// fall back to Function_element_xi::evaluate_derivative
		Function_element_xi_handle local_function_element_xi(
			new Function_element_xi(element_private,xi_private));
		Function_variable_element_xi_element_xi_handle
			local_function_variable_element_xi(new
			Function_variable_element_xi_element_xi(local_function_element_xi,
			atomic_variable_element_xi->element_private,
			atomic_variable_element_xi->xi_private,
			atomic_variable_element_xi->indices));

		if (local_function_element_xi&&local_function_variable_element_xi)
		{
			result=(local_function_element_xi->evaluate_derivative)(derivative,
				local_function_variable_element_xi,atomic_independent_variables);
		}
	}
	else if ((atomic_variable_finite_element=boost::dynamic_pointer_cast<
		Function_variable_matrix_components,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_finite_element->function())&&
		(0<atomic_variable_finite_element->row_private)&&field_private&&
		(atomic_variable_finite_element->row_private<=number_of_components())&&
		!node_private&&element_private&&(0<(local_number_of_xi=number_of_xi()))&&
		((Function_size_type)local_number_of_xi==xi_private.size()))
	{
		bool zero_derivative;
		Function_size_type derivative_order=atomic_independent_variables.size();
		Function_variable_matrix_nodal_values_handle nodal_values_variable(0);
		ublas::vector<Function_variable_element_xi_finite_element_handle>
			element_xi_variables(derivative_order);

		result=true;
		// check independent variables for a zero derivative
		zero_derivative=false;
		std::for_each(atomic_independent_variables.begin(),
			atomic_independent_variables.end(),
			Function_finite_element_check_derivative_functor(
			Function_finite_element_handle(this),
			atomic_variable_finite_element->row_private,zero_derivative,
			nodal_values_variable,element_xi_variables.begin()));
		if (zero_derivative)
		{
			derivative=0;
		}
		else
		{
			struct FE_element_field_values *element_field_values;

			element_field_values=CREATE(FE_element_field_values)();
			if (element_field_values)
			{
				zero_derivative=false;
				if (nodal_values_variable)
				{
					int i,number_of_element_field_nodes,
					number_of_saved_element_field_nodes;
					struct FE_node **element_field_nodes;

					element_field_nodes=(struct FE_node **)NULL;
					number_of_element_field_nodes=0;
					if (calculate_FE_element_field_nodes(element_private,field_private,
						&number_of_element_field_nodes,&element_field_nodes,
						(struct FE_element *)NULL))
					{
						// check if nodal_values_variable->node is one of the
						//   element_field_nodes
						i=0;
						while ((i<number_of_element_field_nodes)&&
							(nodal_values_variable->node!=element_field_nodes[i]))
						{
							i++;
						}
						if (i<number_of_element_field_nodes)
						{
							Function_handle *element_field_saved_nodal_values;

							// set up working storage
							element_field_saved_nodal_values=
								new Function_handle[number_of_element_field_nodes];
							if (element_field_saved_nodal_values)
							{
								// NULL working storage
								for (i=0;i<number_of_element_field_nodes;i++)
								{
									element_field_saved_nodal_values[i]=Function_handle(0);
								}
								// save and zero all the nodal values for the <fe_field> on the
								//   element 
								number_of_saved_element_field_nodes=0;
								while (result&&(number_of_saved_element_field_nodes<
									number_of_element_field_nodes))
								{
									Function_variable_matrix_nodal_values_handle variable(new
										Function_variable_matrix_nodal_values(
										boost::dynamic_pointer_cast<Function_finite_element,
										Function>(nodal_values_variable->function()),
										nodal_values_variable->component_number,
										element_field_nodes[number_of_saved_element_field_nodes],
										FE_NODAL_UNKNOWN,0,(struct FE_time_sequence *)NULL));

									if (element_field_saved_nodal_values[
										number_of_saved_element_field_nodes]=variable->get_value())
									{
										Function_size_type j,number_of_values=
											((element_field_saved_nodal_values[
											number_of_saved_element_field_nodes])->output())->
											number_differentiable();
										Matrix zero_vector(number_of_values,1);

										number_of_saved_element_field_nodes++;
										for (j=0;j<number_of_values;j++)
										{
											zero_vector(j,0)=(Scalar)0;
										}
										if (!((variable->set_value)(Function_matrix_scalar_handle(
											new Function_matrix<Scalar>(zero_vector)))))
										{
											result=false;
										}
									}
									else
									{
										result=false;
									}
								}
								if (result)
								{
									Matrix one_vector(1,1);

									// set the nodal_values_variable to 1
									one_vector(0,0)=(Scalar)1;
									if ((nodal_values_variable->set_value)(
										Function_matrix_scalar_handle(
										new Function_matrix<Scalar>(one_vector))))
									{
										// need to use FE_element_field_values_set_no_modify because
										//   after changing the nodal values to all zeros except one
										//   one, the modification (eg increasing in xi) would be
										//   wrong
										if (!(FE_element_field_values_set_no_modify(
											element_field_values)&&calculate_FE_element_field_values(
											element_private,field_private,(FE_value)time_private,
											(char)0,element_field_values,(struct FE_element *)NULL)))
										{
											result=false;
										}
									}
								}
								// reset all the nodal values for the <fe_field> on the element 
								i=number_of_saved_element_field_nodes;
								while (i>0)
								{
									i--;
									if (element_field_saved_nodal_values[i])
									{
										Function_variable_matrix_nodal_values_handle variable(new
											Function_variable_matrix_nodal_values(
											boost::dynamic_pointer_cast<Function_finite_element,
											Function>(nodal_values_variable->function()),
											nodal_values_variable->component_number,
											element_field_nodes[i],FE_NODAL_UNKNOWN,0,
											(struct FE_time_sequence *)NULL));

										(variable->set_value)(
											element_field_saved_nodal_values[i]);
									}
								}
								/* destroy working computed values for saving current values and
									setting temporary values */
								for (i=0;i<number_of_element_field_nodes;i++)
								{
									element_field_saved_nodal_values[i]=
										Function_matrix_scalar_handle(0);
								}
							}
							else
							{
								result=false;
							}
							/* get rid of working storage */
							delete [] element_field_saved_nodal_values;
						}
						else
						{
							zero_derivative=true;
						}
					}
					else
					{
						result=false;
					}
					if (element_field_nodes)
					{
						for (i=0;i<number_of_element_field_nodes;i++)
						{
							DEACCESS(FE_node)(element_field_nodes+i);
						}
						DEALLOCATE(element_field_nodes);
					}
				}
				else
				{
					if (!calculate_FE_element_field_values(element_private,field_private,
						(FE_value)time_private,(char)0,element_field_values,
						(struct FE_element *)NULL))
					{
						result=false;
					}
				}
				if (result)
				{
					if (zero_derivative)
					{
						derivative=0;
					}
					else
					{
						FE_value *component_values,*monomial_derivative_values;
						int *component_monomial_info,local_component_number,
							number_of_component_values;

						local_component_number=
							(int)(atomic_variable_finite_element->row_private)-1;
						component_values=(FE_value *)NULL;
						if (FE_element_field_values_get_component_values(
							element_field_values,local_component_number,
							&number_of_component_values,&component_values))
						{
							ALLOCATE(component_monomial_info,int,local_number_of_xi+1);
							ALLOCATE(monomial_derivative_values,FE_value,
								number_of_component_values);
							if (component_monomial_info&&
								FE_element_field_values_get_monomial_component_info(
								element_field_values,local_component_number,
								component_monomial_info))
							{
								FE_value *xi_array;
								int *xi_derivative_orders;

								xi_array=new FE_value[local_number_of_xi];
								xi_derivative_orders=new int[local_number_of_xi];
								if (xi_array&&xi_derivative_orders)
								{
									FE_value *value_address_1,*value_address_2;
									Function_size_type i;
									int j;
									Scalar component_value;

									for (i=0;i<local_number_of_xi;i++)
									{
										xi_array[i]=(FE_value)(xi_private[i]);
									}
									for (i=0;i<local_number_of_xi;i++)
									{
										xi_derivative_orders[i]=0;
									}
									for (i=0;i<derivative_order;i++)
									{
										if (element_xi_variables[i])
										{
											// element/xi derivative
											if (0<element_xi_variables[i]->indices.size())
											{
												xi_derivative_orders[
													(element_xi_variables[i]->indices)[0]-1]++;
											}
										}
									}
									if (result=calculate_monomial_derivative_values(
										local_number_of_xi,component_monomial_info+1,
										xi_derivative_orders,xi_array,monomial_derivative_values))
									{
										/* calculate the derivative */
										component_value=(Scalar)0;
										value_address_1=monomial_derivative_values;
										value_address_2=component_values;
										for (j=number_of_component_values;j>0;j--)
										{
											component_value +=
												(double)(*value_address_1)*
												(double)(*value_address_2);
											value_address_1++;
											value_address_2++;
										}
										derivative=component_value;
									}
								}
								else
								{
									result=false;
								}
								delete [] xi_array;
								delete [] xi_derivative_orders;
							}
							else
							{
								result=false;
							}
							DEALLOCATE(monomial_derivative_values);
							DEALLOCATE(component_monomial_info);
						}
						else
						{
							result=false;
						}
						DEALLOCATE(component_values);
					}
				}
			}
			else
			{
				result=false;
			}
			// remove temporary storage
			if (element_field_values)
			{
				DESTROY(FE_element_field_values)(&element_field_values);
			}
#if defined (OLD_CODE)
			int local_component_number;

			local_component_number=
				(int)(atomic_variable_finite_element->component_number)-1;
			if (nodal_values_variable)
			{
				int i,number_of_element_field_nodes,number_of_saved_element_field_nodes;
				struct FE_node **element_field_nodes;

				element_field_nodes=(struct FE_node **)NULL;
				number_of_element_field_nodes=0;
				if (calculate_FE_element_field_nodes(element_private,field_private,
					&number_of_element_field_nodes,&element_field_nodes,
					(struct FE_element *)NULL))
				{
					// check if nodal_values_variable->node is one of the
					//   element_field_nodes
					i=0;
					while ((i<number_of_element_field_nodes)&&
						(nodal_values_variable->node!=element_field_nodes[i]))
					{
						i++;
					}
					if (i<number_of_element_field_nodes)
					{
						Function_handle *element_field_saved_nodal_values;

						// set up working storage
						element_field_saved_nodal_values=
							new Function_handle[number_of_element_field_nodes];
						if (element_field_saved_nodal_values)
						{
							// NULL working storage
							for (i=0;i<number_of_element_field_nodes;i++)
							{
								element_field_saved_nodal_values[i]=Function_handle(0);
							}
							// save and zero all the nodal values for the <fe_field> on the
							//   element 
							number_of_saved_element_field_nodes=0;
							while (result&&(number_of_saved_element_field_nodes<
								number_of_element_field_nodes))
							{
								Function_variable_matrix_nodal_values_handle variable(new
									Function_variable_matrix_nodal_values(
									nodal_values_variable->function(),
									nodal_values_variable->component_number,
									element_field_nodes[number_of_saved_element_field_nodes],
									FE_NODAL_UNKNOWN,0));

								if (element_field_saved_nodal_values[
									number_of_saved_element_field_nodes]=variable->get_value())
								{
									Function_size_type j,number_of_values=
										((element_field_saved_nodal_values[
										number_of_saved_element_field_nodes])->output())->
										number_differentiable();
									Matrix zero_vector(number_of_values,1);

									number_of_saved_element_field_nodes++;
									for (j=0;j<number_of_values;j++)
									{
										zero_vector(j,0)=(Scalar)0;
									}
									if (!((variable->set_value)(Function_matrix_scalar_handle(
										new Function_matrix<Scalar>(zero_vector)))))
									{
										result=false;
									}
								}
								else
								{
									result=false;
								}
							}
							if (result)
							{
								Matrix one_vector(1,1);

								// set the nodal_values_variable to 1
								one_vector(0,0)=(Scalar)1;
								if ((nodal_values_variable->set_value)(
									Function_matrix_scalar_handle(
									new Function_matrix<Scalar>(one_vector))))
								{
									FE_value fe_value,*xi_coordinates;
									Function_size_type j;

									if (ALLOCATE(xi_coordinates,FE_value,local_number_of_xi))
									{
										struct FE_element_field_values *element_field_values;

										for (j=0;j<local_number_of_xi;j++)
										{
											xi_coordinates[j]=(FE_value)(xi_private[j]);
										}
										element_field_values=CREATE(FE_element_field_values)();
										// need to use FE_element_field_values_set_no_modify because
										//   after changing the nodal values to all zeros except one
										//   one, the modification (eg increasing in xi) would be
										//   wrong
										if (element_field_values&&clear_FE_element_field_values(
											element_field_values)&&
											FE_element_field_values_set_no_modify(
											element_field_values)&&calculate_FE_element_field_values(
											element_private,field_private,(FE_value)time_private,
											(char)0,element_field_values,(struct FE_element *)NULL)&&
											calculate_FE_element_field(local_component_number,
											element_field_values,xi_coordinates,&fe_value,
											(FE_value *)NULL))
										{
											derivative=(Scalar)fe_value;
										}
										else
										{
											result=false;
										}
										DESTROY(FE_element_field_values)(&element_field_values);
										DEALLOCATE(xi_coordinates);
									}
								}
							}
							// reset all the nodal values for the <fe_field> on the element 
							i=number_of_saved_element_field_nodes;
							while (i>0)
							{
								i--;
								if (element_field_saved_nodal_values[i])
								{
									Function_variable_matrix_nodal_values_handle variable(new
										Function_variable_matrix_nodal_values(
										nodal_values_variable->function(),
										nodal_values_variable->component_number,
										element_field_nodes[i],FE_NODAL_UNKNOWN,0));

									(variable->set_value)(
										element_field_saved_nodal_values[i]);
								}
							}
							/* destroy working computed values for saving current values and
								setting temporary values */
							for (i=0;i<number_of_element_field_nodes;i++)
							{
								element_field_saved_nodal_values[i]=
									Function_matrix_scalar_handle(0);
							}
						}
						else
						{
							result=false;
						}
						/* get rid of working storage */
						delete [] element_field_saved_nodal_values;
					}
					else
					{
						derivative=0;
					}
				}
				else
				{
					result=false;
				}
				if (element_field_nodes)
				{
					for (i=0;i<number_of_element_field_nodes;i++)
					{
						DEACCESS(FE_node)(element_field_nodes+i);
					}
					DEALLOCATE(element_field_nodes);
				}
			}
			else
			{
				struct FE_element_field_values *element_field_values;

				element_field_values=CREATE(FE_element_field_values)();
				if (element_field_values&&
					clear_FE_element_field_values(element_field_values)&&
					calculate_FE_element_field_values(element_private,field_private,
					(FE_value)time_private,(char)0,element_field_values,
					(struct FE_element *)NULL))
				{
					FE_value *component_values,*monomial_derivative_values;
					int *component_monomial_info,number_of_component_values;

					component_values=(FE_value *)NULL;
					if (FE_element_field_values_get_component_values(element_field_values,
						local_component_number,&number_of_component_values,
						&component_values))
					{
						ALLOCATE(component_monomial_info,int,local_number_of_xi+1);
						ALLOCATE(monomial_derivative_values,FE_value,
							number_of_component_values);
						if (component_monomial_info&&
							FE_element_field_values_get_monomial_component_info(
							element_field_values,local_component_number,
							component_monomial_info))
						{
							FE_value *xi_array;
							int *xi_derivative_orders;

							xi_array=new FE_value[local_number_of_xi];
							xi_derivative_orders=new int[local_number_of_xi];
							if (xi_array&&xi_derivative_orders)
							{
								FE_value *value_address_1,*value_address_2;
								Function_size_type i;
								int j;
								Scalar component_value;

								for (i=0;i<local_number_of_xi;i++)
								{
									xi_array[i]=(FE_value)(xi_private[i]);
								}
								for (i=0;i<local_number_of_xi;i++)
								{
									xi_derivative_orders[i]=0;
								}
								for (i=0;i<derivative_order;i++)
								{
									if (element_xi_variables[i])
									{
										// element/xi derivative
										if (0<element_xi_variables[i]->indices.size())
										{
											xi_derivative_orders[
												(element_xi_variables[i]->indices)[0]-1]++;
										}
									}
								}
								if (result=calculate_monomial_derivative_values(
									local_number_of_xi,component_monomial_info+1,
									xi_derivative_orders,xi_array,monomial_derivative_values))
								{
									/* calculate the derivative */
									component_value=(Scalar)0;
									value_address_1=monomial_derivative_values;
									value_address_2=component_values;
									for (j=number_of_component_values;j>0;j--)
									{
										component_value +=
											(double)(*value_address_1)*
											(double)(*value_address_2);
										value_address_1++;
										value_address_2++;
									}
									derivative=component_value;
								}
							}
							else
							{
								result=false;
							}
							delete [] xi_array;
							delete [] xi_derivative_orders;
						}
						else
						{
							result=false;
						}
						DEALLOCATE(monomial_derivative_values);
						DEALLOCATE(component_monomial_info);
					}
					else
					{
						result=false;
					}
					DEALLOCATE(component_values);
				}
				else
				{
					result=false;
				}
				// remove temporary storage
				if (element_field_values)
				{
					DESTROY(FE_element_field_values)(&element_field_values);
				}
			}
#endif // defined (OLD_CODE)
		}
#if defined (NEW_CODE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		//???DB.  Should enclose code above
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle derivative_function;
		Function_variable_handle derivative_variable;

		if (
			(derivative_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(atomic_variable_finite_element->derivative(
			atomic_independent_variables)))&&(derivative_variable=
			derivative_function->output())&&(derivative_variable->evaluate())&&
			(derivative_variable=derivative_function->matrix(
			atomic_independent_variables))&&
			(derivative_value=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(derivative_variable->get_value()))&&
			(1==derivative_value->number_of_rows())&&
			(1==derivative_value->number_of_columns()))
		{
			derivative=(*derivative_value)(1,1);
			result=true;
		}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (NEW_CODE)
	}
	else if ((atomic_variable_nodal_values=boost::dynamic_pointer_cast<
		Function_variable_matrix_nodal_values,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_nodal_values->function()))
	{
		Function_variable_matrix_nodal_values_handle atomic_independent_variable;

		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_nodal_values,Function_variable>(
			atomic_independent_variables.front()))&&
			equivalent(atomic_variable_nodal_values,atomic_independent_variable))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}

	return (result);
}

bool Function_finite_element::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_element_xi_finite_element_handle atomic_variable_element_xi;
	Function_variable_matrix_components_handle atomic_variable_finite_element;
	Function_variable_matrix_nodal_values_handle atomic_variable_nodal_values;

	result=false;
	if ((atomic_variable_element_xi=boost::dynamic_pointer_cast<
		Function_variable_element_xi_finite_element,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_element_xi->function()))
	{
		Function_variable_value_element_handle value_element;
		Function_variable_value_scalar_handle value_scalar;

		if ((atomic_variable_element_xi->element_private)&&(atomic_value->value())&&
			(std::string("Element")==(atomic_value->value())->type())&&
			(value_element=boost::dynamic_pointer_cast<
			Function_variable_value_element,Function_variable_value>(atomic_value->
			value())))
		{
			result=value_element->set(element_private,atomic_value);
			if (element_private)
			{
				ACCESS(FE_element)(element_private);
				xi_private.resize(get_FE_element_dimension(element_private));
			}
			else
			{
				xi_private.resize(0);
			}
		}
		else if ((atomic_variable_element_xi->xi_private)&&
			(std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			result=value_scalar->set(
				xi_private[(atomic_variable_element_xi->indices)[0]-1],atomic_value);
		}
	}
	else if ((atomic_variable_finite_element=boost::dynamic_pointer_cast<
		Function_variable_matrix_components,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
			atomic_variable_finite_element->function()))
	{
		Function_variable_value_scalar_handle value_scalar;

		if ((std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			result=value_scalar->set(components_private[
				(atomic_variable_finite_element->row_private)-1],atomic_value);
		}
	}
	else if ((atomic_variable_nodal_values=boost::dynamic_pointer_cast<
		Function_variable_matrix_nodal_values,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_nodal_values->function()))
	{
		Function_variable_value_scalar_handle value_scalar;
		FE_value fe_time;
		Scalar time=0.0;

		if (atomic_variable_nodal_values->time_sequence&&
			(1==FE_time_sequence_get_number_of_times(
			atomic_variable_nodal_values->time_sequence)))
		{
			if (FE_time_sequence_get_time_for_index(
				atomic_variable_nodal_values->time_sequence,/*time_index*/0,&fe_time))
			{
				time=fe_time;
			}
		}
		if ((std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			Scalar temp_scalar;

			if (value_scalar->set(temp_scalar,atomic_value))
			{
				result=set_nodal_value(atomic_variable_nodal_values->component_number,
					atomic_variable_nodal_values->node,
					atomic_variable_nodal_values->value_type,
					atomic_variable_nodal_values->version,time,temp_scalar);
			}
		}
	}
	if (result)
	{
		set_not_evaluated();
	}

	return (result);
}

Function_handle Function_finite_element::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_element_xi_finite_element_handle atomic_variable_element_xi;
	Function_variable_matrix_components_handle atomic_variable_finite_element;
	Function_variable_matrix_nodal_values_handle atomic_variable_nodal_values;

	if ((atomic_variable_element_xi=boost::dynamic_pointer_cast<
		Function_variable_element_xi_finite_element,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_element_xi->function()))
	{
		// fall back to Function_element_xi::get_value
		Function_element_xi_handle local_function_element_xi(
			new Function_element_xi(element_private,xi_private));
		Function_variable_element_xi_element_xi_handle
			local_function_variable_element_xi(new
			Function_variable_element_xi_element_xi(local_function_element_xi,
			atomic_variable_element_xi->element_private,
			atomic_variable_element_xi->xi_private,
			atomic_variable_element_xi->indices));

		if (local_function_element_xi&&local_function_variable_element_xi)
		{
			result=(local_function_element_xi->get_value)(
				local_function_variable_element_xi);
		}
	}
	else if ((atomic_variable_finite_element=boost::dynamic_pointer_cast<
		Function_variable_matrix_components,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_finite_element->function()))
	{
		Matrix result_matrix(1,1);

		if ((atomic_variable_finite_element->get_entry)(result_matrix(0,0)))
		{
			result=Function_handle(new Function_matrix<Scalar>(result_matrix));
		}
	}
	else if ((atomic_variable_nodal_values=boost::dynamic_pointer_cast<
		Function_variable_matrix_nodal_values,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_nodal_values->function()))
	{
		FE_value fe_time;
		Scalar time=0.0,value;

		if (atomic_variable_nodal_values->time_sequence&&
			(1==FE_time_sequence_get_number_of_times(
			atomic_variable_nodal_values->time_sequence)))
		{
			if (FE_time_sequence_get_time_for_index(
				atomic_variable_nodal_values->time_sequence,/*time_index*/0,&fe_time))
			{
				time=fe_time;
			}
		}
		if (get_nodal_value(atomic_variable_nodal_values->component_number,
			atomic_variable_nodal_values->node,
			atomic_variable_nodal_values->value_type,
			atomic_variable_nodal_values->version,time,value))
		{
			Matrix result_matrix(1,1);

			result_matrix(0,0)=value;
			result=Function_handle(new Function_matrix<Scalar>(result_matrix));
		}
	}

	return (result);
}

Function_finite_element::Function_finite_element(
	const Function_finite_element& function_finite_element):
	Function(function_finite_element),
	time_private(function_finite_element.time_private),
	element_private(function_finite_element.element_private),
	field_private(function_finite_element.field_private),
	node_private(function_finite_element.node_private),
	xi_private(function_finite_element.xi_private)
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_finite_element& Function_finite_element::operator=(
	const Function_finite_element& function_finite_element)
//******************************************************************************
// LAST MODIFIED : 8 April 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	time_private=function_finite_element.time_private;
	DEACCESS(FE_element)(&element_private);
	if (function_finite_element.element_private)
	{
		element_private=ACCESS(FE_element)(function_finite_element.element_private);
	}
	DEACCESS(FE_field)(&field_private);
	if (function_finite_element.field_private)
	{
		field_private=ACCESS(FE_field)(function_finite_element.field_private);
	}
	DEACCESS(FE_node)(&node_private);
	if (function_finite_element.node_private)
	{
		node_private=ACCESS(FE_node)(function_finite_element.node_private);
	}
	xi_private.resize(function_finite_element.xi_private.size());
	xi_private=function_finite_element.xi_private;

	return (*this);
}
