//******************************************************************************
// FILE : function_composition.cpp
//
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_composition.hpp"
#include "computed_variable/function_derivative_matrix.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_intersection.hpp"
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_composite.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"
#include "computed_variable/function_variable_wrapper.hpp"

// module classes
// ==============

// class Function_variable_composition
// -----------------------------------

// forward declaration so that can use _handle
class Function_variable_composition;
typedef boost::intrusive_ptr<Function_variable_composition>
	Function_variable_composition_handle;

class Function_variable_composition : public Function_variable_wrapper
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
// If <working_variable> is NULL then this wraps the <output_variable> of
// <function_composition>, otherwise <working_variable> is an atomic variable
// in the <output_variable>.
//==============================================================================
{
	friend class Function_composition;
	friend class Function_variable_iterator_representation_atomic_composition;
	public:
		// constructor
		Function_variable_composition(
			const Function_composition_handle& function_composition):
			Function_variable_wrapper(function_composition,0){};
		// constructor.  A zero <atomic_output_variable> indicates the whole output
		//   variable
		Function_variable_composition(
			const Function_composition_handle& function_composition,
			Function_variable_handle& atomic_output_variable):
			Function_variable_wrapper(function_composition,0)
		{
			if (function_composition)
			{
				if (atomic_output_variable&&(function_composition->output_private))
				{
					Function_variable_iterator atomic_variable_iterator,
						end_atomic_variable_iterator;

					atomic_variable_iterator=
						function_composition->output_private->begin_atomic();
					end_atomic_variable_iterator=
						function_composition->output_private->end_atomic();
					while ((atomic_variable_iterator!=end_atomic_variable_iterator)&&
						!equivalent(*atomic_variable_iterator,atomic_output_variable))
					{
						atomic_variable_iterator++;
					}
					if (atomic_variable_iterator!=end_atomic_variable_iterator)
					{
						this->working_variable=atomic_output_variable;
					}
				}
			}
		};
		// destructor
		~Function_variable_composition(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			Function_composition_handle function_composition=
				boost::dynamic_pointer_cast<Function_composition,Function>(function());
			Function_variable_handle result(0);

			if (working_variable)
			{
				Function_variable_handle local_working_variable;

				if (local_working_variable=working_variable->clone())
				{
					result=Function_variable_handle(
						new Function_variable_composition(function_composition,
						local_working_variable));
				}
			}
			else
			{
				result=Function_variable_handle(
					new Function_variable_composition(function_composition));
			}

			return (result);
		};
		Function_handle evaluate()
		{
			Function_composition_handle function_composition=
				boost::dynamic_pointer_cast<Function_composition,Function>(function());
			Function_handle result(0);
			Function_variable_handle wrapped_variable=get_wrapped();

			if (function_composition&&wrapped_variable)
			{
#if defined (BEFORE_CACHING)
				Function_handle input_current(0);

				if ((function_composition->input_private)&&
					(function_composition->value_private))
				{
					input_current=(function_composition->input_private)->get_value();
					(function_composition->input_private)->set_value(
						function_composition->value_private->evaluate());
				}
				result=wrapped_variable->evaluate();
				if (input_current)
				{
					(function_composition->input_private)->rset_value(input_current);
				}
#else // defined (BEFORE_CACHING)
				if (!(function_composition->evaluated()))
				{
					(function_composition->input_private)->set_value(
						function_composition->value_private->evaluate());
					if (function_composition->output_private->evaluate())
					{
						function_composition->set_evaluated();
					}
				}
				if (function_composition->evaluated())
				{
					result=wrapped_variable->get_value();
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		}
		Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables)
		{
			Function_composition_handle function_composition=
				boost::dynamic_pointer_cast<Function_composition,Function>(function());
			Function_handle result(0);
			Function_variable_handle wrapped_variable=get_wrapped();

			if (function_composition&&wrapped_variable)
			{
				Function_handle input_current(0);
				Function_size_type i;
				std::list<Function_variable_handle>::iterator
					independent_variables_iterator;

#if defined (OLD_CODE)
				bool valid_independent_variables;
				std::list<Function_variable_handle> &local_independent_variables=independent_variables;

				// check for differentiating wrt input/source variables
				valid_independent_variables=true;
				i=independent_variables.size();
				independent_variables_iterator=independent_variables.begin();
				while (valid_independent_variables&&(i>0))
				{
					Function_variable_intersection_handle input_intersection(
						new Function_variable_intersection(
						function_composition->input_private,
						*independent_variables_iterator));

					if (input_intersection&&
						(0==input_intersection->number_differentiable()))
					{
						Function_variable_intersection_handle value_intersection(
							new Function_variable_intersection(
							function_composition->value_private,
							*independent_variables_iterator));

						if ((0==value_intersection)||
							(0!=value_intersection->number_differentiable()))
						{
							valid_independent_variables=false;
						}
					}
					else
					{
						valid_independent_variables=false;
					}
					independent_variables_iterator++;
					i--;
				}
				//???debug
//				valid_independent_variables=true;
				if (valid_independent_variables)
#else // defined (OLD_CODE)
				bool intersection,valid;
				Function_handle temp_function;
				Function_variable_handle local_independent_variable,
					temp_atomic_variable;
				Function_variable_iterator atomic_iterator,atomic_iterator_end;
				Matrix temp_matrix(1,1);
				std::list<Function_variable_handle> local_independent_variable_list,
					local_independent_variables;

				valid=true;
				i=independent_variables.size();
				independent_variables_iterator=independent_variables.begin();
				while (valid&&(i>0))
				{
					local_independent_variable_list.clear();
					intersection=false;
					atomic_iterator=(*independent_variables_iterator)->begin_atomic();
					atomic_iterator_end=(*independent_variables_iterator)->end_atomic();
					while (valid&&(atomic_iterator!=atomic_iterator_end))
					{
						if (1==(*atomic_iterator)->number_differentiable())
						{
							Function_variable_intersection_handle input_intersection(
								new Function_variable_intersection(
								function_composition->input_private,*atomic_iterator));
							Function_variable_intersection_handle value_intersection(
								new Function_variable_intersection(
								function_composition->value_private,*atomic_iterator));

							temp_atomic_variable=0;
							if (input_intersection&&value_intersection&&
								(0==input_intersection->number_differentiable())&&
								(0==value_intersection->number_differentiable()))
							{
								// have to clone otherwise changes when increment iterator
								if (*atomic_iterator)
								{
									temp_atomic_variable=(*atomic_iterator)->clone();
								}
							}
							else
							{
								intersection=true;
								// replace with a unique atomic variable (derivative wrt any
								//   other is zero)
								if (temp_function=Function_handle(new Function_matrix<Scalar>(
									temp_matrix)))
								{
									temp_atomic_variable=temp_function->output();
								}
							}
							if (temp_atomic_variable)
							{
								local_independent_variable_list.push_back(temp_atomic_variable);
							}
							else
							{
								valid=false;
							}
						}
						atomic_iterator++;
					}
					if (valid)
					{
						local_independent_variable=0;
						if (intersection)
						{
							local_independent_variable=Function_variable_handle(
								new Function_variable_composite(
								local_independent_variable_list));
						}
						else
						{
							local_independent_variable= *independent_variables_iterator;
						}
						if (local_independent_variable)
						{
							local_independent_variables.push_back(local_independent_variable);
						}
						else
						{
							valid=false;
						}
					}
					independent_variables_iterator++;
					i--;
				}
				if (valid)
#endif // defined (OLD_CODE)
				{
					try
					{
						Function_variable_union_handle independent_variables_union(
							new Function_variable_union(local_independent_variables));
						Function_variable_union_handle g_dependent_variable(
							new Function_variable_union(function_composition->value_private,
							independent_variables_union));
						Function_derivative_matrix_handle derivative_g(
							new Function_derivative_matrix(g_dependent_variable,
							local_independent_variables));

						if (derivative_g)
						{
							Function_derivative_matrix_handle derivative_f(0);
							Function_handle input_current(0);
							Function_variable_union_handle f_independent_variable(
								new Function_variable_union(function_composition->input_private,
								independent_variables_union));
							std::list<Function_variable_handle> f_independent_variables(
								local_independent_variables.size(),f_independent_variable);

							if ((function_composition->input_private)&&
								(function_composition->value_private))
							{
								input_current=(function_composition->input_private)->
									get_value();
								(function_composition->input_private)->set_value(
									function_composition->value_private->evaluate());
							}
					//???debug
					try
					{
							derivative_f=new Function_derivative_matrix(wrapped_variable,
								f_independent_variables);
							if (derivative_f)
							{
								Function_derivative_matrix_handle derivative_matrix=
									Function_derivative_matrix_compose(
									Function_variable_handle(this),derivative_f,derivative_g);

								if (derivative_matrix)
								{
									result=derivative_matrix->matrix(local_independent_variables);
								}
							}
					//???debug
					}
					catch (Function_derivative_matrix::Construction_exception)
					{
						std::cout << "Function_variable_composition::evaluate_derivative.  f Failed" << std::endl;
						std::cout << *(wrapped_variable->get_string_representation()) << std::endl;
						std::cout << f_independent_variables.size() << " " << *((f_independent_variables.front())->get_string_representation()) << std::endl;
					}
							if (input_current)
							{
								(function_composition->input_private)->
									rset_value(input_current);
							}
						}
					}
					catch (Function_derivative_matrix::Construction_exception)
					{
						// do nothing
						//???debug
						std::cout << "Function_variable_composition::evaluate_derivative.  g Failed" << std::endl;
					}
				}
				//???debug
				else
				{
					std::cout << "Function_variable_composition::evaluate_derivative.  !valid_independent_variables" << std::endl;
				}
			}

			return (result);
		}
		string_handle get_string_representation()
		{
			Function_composition_handle function_composition=
				boost::dynamic_pointer_cast<Function_composition,Function>(function());
			Function_variable_handle wrapped_variable=get_wrapped();
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				if (wrapped_variable)
				{
					out << *(wrapped_variable->get_string_representation());
				}
				out << "(";
				if (function_composition)
				{
					if (function_composition->input_private)
					{
						out << *(function_composition->input_private->
							get_string_representation());
					}
					out << "=";
					if (function_composition->value_private)
					{
						out << *(function_composition->value_private->
							get_string_representation());
					}
				}
				out << ")";
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_variable_handle get_wrapped() const
		{
			Function_composition_handle function_composition=
				boost::dynamic_pointer_cast<Function_composition,Function>(function());
			Function_variable_handle result(0);

			if (this)
			{
				if (!(result=working_variable))
				{
					if (function_composition)
					{
						result=function_composition->output_private;
					}
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_composition(
			const Function_variable_composition& variable_composition):
			Function_variable_wrapper(variable_composition){};
		// assignment
		Function_variable_composition& operator=(
			const Function_variable_composition&);
};


// global classes
// ==============

// class Function_composition
// --------------------------

Function_composition::Function_composition(
	const Function_variable_handle output,const Function_variable_handle input,
	const Function_variable_handle value):Function(),input_private(input),
	output_private(output),value_private(value)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (output_private)
	{
		output_private->add_dependent_function(this);
	}
	if (value_private)
	{
		value_private->add_dependent_function(this);
	}
}

Function_composition::~Function_composition()
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	if (output_private)
	{
		output_private->remove_dependent_function(this);
	}
	if (value_private)
	{
		value_private->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

string_handle Function_composition::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 14 June 2004
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	string_handle return_string;

	if (return_string=new std::string)
	{
		std::ostringstream out;

		out << *(output_private->get_string_representation()) << "(" <<
			*(input_private->get_string_representation()) << "=" <<
			*(value_private->get_string_representation()) << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_composition::input()
//******************************************************************************
// LAST MODIFIED : 10 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle result(0);

	if (value_private&&(value_private->function()))
	{
		result=(value_private->function())->input();
	}

	return (result);
}

Function_variable_handle Function_composition::output()
//******************************************************************************
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_composition(
		Function_composition_handle(this))));
}

bool Function_composition::operator==(const Function& function) const
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
			const Function_composition& function_composition=
				dynamic_cast<const Function_composition&>(function);

			result=(equivalent(input_private,(function_composition.input_private))&&
				equivalent(output_private,(function_composition.output_private))&&
				equivalent(value_private,(function_composition.value_private)));
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

Function_handle Function_composition::evaluate(Function_variable_handle)
//******************************************************************************
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	// should not come here - handled by Function_variable_composition over-riding
	//   Function_variable::evaluate
	Assert(false,std::logic_error(
		"Function_composition::evaluate.  Should not come here"));
	
	return (0);
}

bool Function_composition::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_dependent_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
// Can come here via Function_derivative_matrix::Function_derivative_matrix
// and the Function_variable_inverse::evaluate_derivative or
// Function_variable_dependent::evaluate_derivative or
// Function_variable::evaluate_derivative.
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_matrix<Scalar> > derivative_value;
	Function_variable_composition_handle atomic_variable_composition;

	result=false;
	if ((atomic_variable_composition=boost::dynamic_pointer_cast<
		Function_variable_composition,Function_variable>(
		atomic_dependent_variable))&&equivalent(Function_handle(this),
		atomic_variable_composition->function()))
	{
		if ((derivative_value=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(atomic_variable_composition->
			evaluate_derivative(atomic_independent_variables)))&&
			(1==derivative_value->number_of_rows())&&
			(1==derivative_value->number_of_columns()))
		{
			derivative=(*derivative_value)(1,1);
			result=true;
		}
	}

	return (result);
}

bool Function_composition::set_value(
	const Function_variable_handle atomic_variable,
	const Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_handle function;
	Function_variable_composition_handle atomic_variable_composition;
	Function_variable_handle local_atomic_variable;

	result=false;
	if ((atomic_variable_composition=boost::dynamic_pointer_cast<
		Function_variable_composition,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_composition->function()))
	{
		local_atomic_variable=atomic_variable_composition->get_wrapped();
	}
	else
	{
		local_atomic_variable=atomic_variable;
	}
	if (output_private&&(function=(output_private->function)())&&
		(function->set_value)(local_atomic_variable,atomic_value))
	{
		result=true;
	}
	if (value_private&&(function=(value_private->function)())&&
		(function->set_value)(local_atomic_variable,atomic_value))
	{
		result=true;
	}

	return (result);
}

Function_handle Function_composition::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result;
	Function_variable_composition_handle atomic_variable_composition;
	Function_variable_handle local_atomic_variable;

	result=0;
	if ((atomic_variable_composition=boost::dynamic_pointer_cast<
		Function_variable_composition,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_composition->function()))
	{
		local_atomic_variable=atomic_variable_composition->get_wrapped();
	}
	else
	{
		local_atomic_variable=atomic_variable;
	}
	if (output_private&&(function=(output_private->function)()))
	{
		result=(function->get_value)(local_atomic_variable);
	}

	return (result);
}

Function_composition::Function_composition(
	const Function_composition& function_composition):Function(),
	input_private(function_composition.input_private),
	output_private(function_composition.output_private),
	value_private(function_composition.value_private)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (output_private)
	{
		output_private->add_dependent_function(this);
	}
	if (value_private)
	{
		value_private->add_dependent_function(this);
	}
}

Function_composition& Function_composition::operator=(
	const Function_composition& function_composition)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	Function_handle function;

	input_private=function_composition.input_private;
	if (function_composition.output_private)
	{
		function_composition.output_private->add_dependent_function(this);
	}
	if (output_private)
	{
		output_private->remove_dependent_function(this);
	}
	output_private=function_composition.output_private;
	if (function_composition.value_private)
	{
		function_composition.value_private->add_dependent_function(this);
	}
	if (value_private)
	{
		value_private->remove_dependent_function(this);
	}
	value_private=function_composition.value_private;

	return (*this);
}
