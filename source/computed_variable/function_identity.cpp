//******************************************************************************
// FILE : function_identity.cpp
//
// LAST MODIFIED : 13 May 2005
//
// DESCRIPTION :
//???DB.  Need to be able to get to the variable it wraps so that can do
//  operations like += for variables?
//???DB.  Are union, intersection, composite wrappers?
//==============================================================================

#include <sstream>

#include "computed_variable/function_identity.hpp"
#include "computed_variable/function_variable_wrapper.hpp"


// module classes
// ==============

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_variable_identity
// --------------------------------

// forward declaration so that can use _handle
class Function_variable_identity;
typedef boost::intrusive_ptr<Function_variable_identity>
	Function_variable_identity_handle;

class Function_variable_identity : public Function_variable_wrapper
//******************************************************************************
// LAST MODIFIED : 13 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_identity(const Function_handle& identity_function,
			const Function_variable_handle& identity_variable):
			Function_variable_wrapper(identity_function,identity_variable){};
		// destructor
		~Function_variable_identity(){};
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_identity(*this)));
		};
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables)
		{
			return (Function_handle(new Function_derivatnew_identity(get_wrapped(),
				independent_variables)));
		};
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	private:
		// copy constructor
		Function_variable_identity(
			const Function_variable_identity& variable_identity):
			Function_variable_wrapper(variable_identity){};
};
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)


// global classes
// ==============

// class Function_identity
// -----------------------

Function_identity::Function_identity(const Function_variable_handle& variable):
	Function(),variable_private(variable)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (variable_private)
	{
		variable_private->add_dependent_function(this);
	}
}

Function_identity::~Function_identity()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	if (variable_private)
	{
		variable_private->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)a
}

string_handle Function_identity::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	string_handle return_string(0);

	if (return_string=new std::string)
	{
		std::ostringstream out;

		out << "identity(";
		if (variable_private)
		{
			out << *(variable_private->get_string_representation());
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_identity::input()
//******************************************************************************
// LAST MODIFIED : 13 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_variable_wrapper
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_variable_identity
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		(Function_handle(this),variable_private)));
}

Function_variable_handle Function_identity::output()
//******************************************************************************
// LAST MODIFIED : 13 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_variable_wrapper
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_variable_identity
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		(Function_handle(this),variable_private)));
}

bool Function_identity::operator==(const Function& function) const
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
			const Function_identity& function_identity=
				dynamic_cast<const Function_identity&>(function);

			result=equivalent(variable_private,function_identity.variable_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_identity::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_wrapper_handle atomic_identity_variable;

	if ((atomic_identity_variable=boost::dynamic_pointer_cast<
		Function_variable_wrapper,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_identity_variable->function()))
	{
		result=(atomic_identity_variable->get_wrapped()->get_value)();
	}

	return (result);
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_identity::evaluate(Function_variable_handle)
//******************************************************************************
// LAST MODIFIED : 14 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (true);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

bool Function_identity::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 20 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_handle atomic_variable_local,atomic_independent_variable;
	Function_variable_wrapper_handle atomic_variable_identity;

	result=false;
	if ((atomic_variable_identity=boost::dynamic_pointer_cast<
		Function_variable_wrapper,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_identity->function())&&
		(1==atomic_variable_identity->number_differentiable())&&
		(atomic_variable_local=atomic_variable_identity->get_wrapped()))
	{
		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=atomic_independent_variables.front())&&
			equivalent(atomic_variable_local,atomic_independent_variable))
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

bool Function_identity::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_handle function;

	result=false;
	if (variable_private&&(function=(variable_private->function)()))
	{
		result=(function->set_value)(atomic_variable,atomic_value);
	}

	return (result);
}

Function_handle Function_identity::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result;

	result=0;
	if (variable_private&&(function=(variable_private->function)()))
	{
		result=(function->get_value)(atomic_variable);
	}

	return (result);
}

Function_identity::Function_identity(
	const Function_identity& function_identity):Function(),
	variable_private(function_identity.variable_private)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (variable_private)
	{
		variable_private->add_dependent_function(this);
	}
}

Function_identity& Function_identity::operator=(
	const Function_identity& function_identity)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	if (function_identity.variable_private)
	{
		function_identity.variable_private->add_dependent_function(this);
	}
	if (variable_private)
	{
		variable_private->remove_dependent_function(this);
	}
	variable_private=function_identity.variable_private;

	return (*this);
}


#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_identity
// ----------------------------------

Function_derivatnew_identity::Function_derivatnew_identity(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables){}
//******************************************************************************
// LAST MODIFIED : 14 April 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Function_derivatnew_identity::~Function_derivatnew_identity(){}
//******************************************************************************
// LAST MODIFIED : 14 April 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_identity::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
//******************************************************************************
// LAST MODIFIED : 13 May 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)

	if (!evaluated())
	{
		Function_size_type number_of_dependent_values=dependent_variable->
			number_differentiable();
		std::list<Function_variable_handle>::const_iterator
			independent_variable_iterator;
		std::list<Matrix> matrices;

#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		result=false;
#endif // defined (EVALUATE_RETURNS_VALUE)
		for (independent_variable_iterator=independent_variables.begin();
			independent_variable_iterator!=independent_variables.end();
			++independent_variable_iterator)
		{
			Function_variable_handle independent_variable=
				*independent_variable_iterator;
			Function_size_type number_of_independent_values=independent_variable->
				number_differentiable();
			std::list<Matrix>::iterator matrix_iterator,last;

			// calculate the derivative of dependent variable with respect to
			//   independent variable and add to matrix list
			{
				Function_size_type column,row;
				Function_variable_iterator atomic_dependent_variable_iterator,
					atomic_dependent_variable_iterator_end,
					atomic_independent_variable_iterator,
					atomic_independent_variable_iterator_begin,
					atomic_independent_variable_iterator_end;
				Matrix new_matrix(number_of_dependent_values,
					number_of_independent_values);

				atomic_dependent_variable_iterator_end=dependent_variable->
					end_atomic();
				atomic_independent_variable_iterator_begin=independent_variable->
					begin_atomic();
				atomic_independent_variable_iterator_end=independent_variable->
					end_atomic();
				row=0;
				for (atomic_dependent_variable_iterator=dependent_variable->
					begin_atomic();atomic_dependent_variable_iterator!=
					atomic_dependent_variable_iterator_end;
					++atomic_dependent_variable_iterator)
				{
					Function_variable_handle atomic_dependent_variable=
						*atomic_dependent_variable_iterator;

					if (1==atomic_dependent_variable->number_differentiable())
					{
						column=0;
						for (atomic_independent_variable_iterator=
							atomic_independent_variable_iterator_begin;
							atomic_independent_variable_iterator!=
							atomic_independent_variable_iterator_end;
							atomic_independent_variable_iterator++)
						{
							Function_variable_handle atomic_independent_variable=
								*atomic_independent_variable_iterator;

							if (1==atomic_independent_variable->number_differentiable())
							{
#if defined (DEBUG)
								//???debug
								std::cout << row << " " << column << " " << *atomic_dependent_variable->get_string_representation() << " " << *atomic_independent_variable->get_string_representation() << std::endl;
								//???debug
								std::cout << "  " << typeid(atomic_dependent_variable->function()).name() << " " << typeid(atomic_independent_variable->function()).name() << std::endl;
#endif // defined (DEBUG)
								if (equivalent(atomic_dependent_variable,
									atomic_independent_variable))
								{
									new_matrix(row,column)=1;
								}
								else
								{
									new_matrix(row,column)=0;
								}
								++column;
							}
						}
						++row;
					}
				}
				matrices.push_back(new_matrix);
			}
			last=matrices.end();
			--last;
			for (matrix_iterator=matrices.begin();matrix_iterator!=last;
				++matrix_iterator)
			{
				Matrix& matrix= *matrix_iterator;
				Matrix new_matrix((matrix.size1)(),
					number_of_independent_values*(matrix.size2)());

				new_matrix.clear();
				matrices.push_back(new_matrix);
			}
		}
		derivative_matrix=Derivative_matrix(matrices);
		set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (OLD_CODE)
		{
			//???DB.  Where I'm up to
			//???DB.  Don't worry about atomic_variable.  Evaluate the whole
			//  derivative?
			bool valid;
			Function_derivatnew_handle derivative_function;
			Function_variable_handle local_dependent_variable=dependent_variable;
			std::list<Function_variable_handle> local_independent_variables;

			valid=true;
			while (valid&&boost::dynamic_pointer_cast<Function_variable_derivatnew,
				Function_variable>(local_dependent_variable))
			{
				if (derivative_function=boost::dynamic_pointer_cast<
					Function_derivatnew,Function>(local_dependent_variable->function()))
				{
					local_dependent_variable=derivative_function->dependent_variable;
					local_independent_variables.insert(
						local_independent_variables.begin(),
						(derivative_function->independent_variables).begin(),
						(derivative_function->independent_variables).end());
				}
				else
				{
					valid=false;
				}
			}
			if (valid)
			{
				bool incremented_independent;
				Function_size_type i,local_order=local_independent_variables.size(),
					number_of_dependent_values=
					local_dependent_variable->number_differentiable();
				std::list<Function_variable_handle>::iterator
					independent_variable_iterator,local_independent_variable_iterator;
				std::list<Function_variable_iterator>
					atomic_local_independent_iterators_begin,
					atomic_local_independent_iterators_end;
				std::list< std::list<Function_variable_handle> >
					matrix_independent_variables;
				std::list<Matrix> matrices;

				local_independent_variable_iterator=
					local_independent_variables.begin();
				for (i=local_order;i>0;--i)
				{
					number_of_dependent_values *=
						(*local_independent_variable_iterator)->number_differentiable();
					atomic_local_independent_iterators_begin.push_back(
						(*local_independent_variable_iterator)->begin_atomic());
					atomic_local_independent_iterators_end.push_back(
						(*local_independent_variable_iterator)->end_atomic());
					++local_independent_variable_iterator;
				}
				valid=(number_of_dependent_values>0);
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
						Function_size_type row,column;
						Function_variable_iterator atomic_dependent_variable_iterator(0),
							atomic_independent_variable_iterator(0);
						Matrix new_matrix(number_of_dependent_values,
							number_of_independent_values);
						std::list<Function_variable_handle>
							new_matrix_independent_variables;
						std::list<Function_variable_iterator>
							atomic_local_independent_iterators;
						std::list<Function_variable_iterator>::iterator
							independent_iterators_begin_iterator,
							independent_iterators_end_iterator,
							independent_iterators_iterator;

						new_matrix_independent_variables.push_back(independent_variable);
						row=0;
						independent_iterators_begin_iterator=
							atomic_local_independent_iterators_begin.begin();
						independent_iterators_end_iterator=
							atomic_local_independent_iterators_end.begin();
						for (i=local_order;i>0;--i)
						{
							atomic_independent_variable_iterator=
								*independent_iterators_begin_iterator;
							while ((atomic_independent_variable_iterator!=
								*independent_iterators_end_iterator)&&
								(1!=(*atomic_independent_variable_iterator)->
								number_differentiable()))
							{
								++atomic_independent_variable_iterator;
							}
							atomic_local_independent_iterators.push_back(
								atomic_independent_variable_iterator);
							++independent_iterators_begin_iterator;
							++independent_iterators_end_iterator;
						}
						atomic_dependent_variable_iterator=
							local_dependent_variable->begin_atomic();
						while (valid&&(atomic_dependent_variable_iterator!=
							local_dependent_variable->end_atomic()))
						{
							Function_variable_handle atomic_dependent_variable=
								*atomic_dependent_variable_iterator;

							if (1==atomic_dependent_variable->number_differentiable())
							{
								std::list<Function_variable_handle>
									new_matrix_atomic_independent_variables;

								Assert(atomic_dependent_variable&&
									(atomic_dependent_variable->function()),std::logic_error(
									"Function_derivatnew::Function_derivatnew.  "
									"Atomic variable missing function()"));
								independent_iterators_iterator=
									atomic_local_independent_iterators.begin();
								for (i=local_order;i>0;--i)
								{
									new_matrix_atomic_independent_variables.push_back(
										*(*independent_iterators_iterator));
									++independent_iterators_iterator;
								}
								column=0;
								atomic_independent_variable_iterator=independent_variable->
									begin_atomic();
								while (valid&&(atomic_independent_variable_iterator!=
									independent_variable->end_atomic()))
								{
									Function_variable_handle atomic_independent_variable=
										*atomic_independent_variable_iterator;

									if (1==atomic_independent_variable->number_differentiable())
									{
#if defined (OLD_CODE)
										new_matrix_atomic_independent_variables.push_back(
											atomic_independent_variable);
										if (!((atomic_dependent_variable->function())->
											evaluate_derivative(new_matrix(row,column),
											atomic_dependent_variable,
											new_matrix_atomic_independent_variables)))
										{
											valid=false;
										}
										new_matrix_atomic_independent_variables.pop_back();
#else // defined (OLD_CODE)
										Function_derivatnew_handle function_derivative;
										Function_variable_handle variable_derivative;
										Function_matrix_scalar_handle matrix_derivative;

										new_matrix_atomic_independent_variables.push_back(
											atomic_independent_variable);
										if ((function_derivative=boost::dynamic_pointer_cast<
											Function_derivatnew,Function>(
											atomic_dependent_variable->
											derivative(new_matrix_atomic_independent_variables)))&&
											(variable_derivative=function_derivative->output())&&
											(variable_derivative->evaluate)()&&
											(matrix_derivative=boost::dynamic_pointer_cast<
											Function_matrix<Scalar>,Function>((function_derivative->
											matrix())->get_value())))
										{
											new_matrix(row,column)=(*matrix_derivative)(1,1);
										}
										else
										{
											valid=false;
										}
										new_matrix_atomic_independent_variables.pop_back();
#endif // defined (OLD_CODE)
										++column;
									}
									++atomic_independent_variable_iterator;
								}
								// move to next row
								i=local_order;
								incremented_independent=false;
								independent_iterators_begin_iterator=
									atomic_local_independent_iterators_begin.end();
								independent_iterators_iterator=
									atomic_local_independent_iterators.end();
								independent_iterators_end_iterator=
									atomic_local_independent_iterators_end.end();
								while (!incremented_independent&&(i>0))
								{
									--independent_iterators_begin_iterator;
									--independent_iterators_iterator;
									--independent_iterators_end_iterator;
									--i;
									atomic_independent_variable_iterator=
										*independent_iterators_iterator;
									do
									{
										++atomic_independent_variable_iterator;
									}
									while ((atomic_independent_variable_iterator!=
										*independent_iterators_end_iterator)&&
										(1!=(*atomic_independent_variable_iterator)->
										number_differentiable()));
									if (atomic_independent_variable_iterator==
										*independent_iterators_end_iterator)
									{
										atomic_independent_variable_iterator=
											*independent_iterators_begin_iterator;
										while ((atomic_independent_variable_iterator!=
											*independent_iterators_end_iterator)&&
											(1!=(*atomic_independent_variable_iterator)->
											number_differentiable()))
										{
											++atomic_independent_variable_iterator;
										}
									}
									else
									{
										incremented_independent=true;
									}
									*independent_iterators_iterator=
										atomic_independent_variable_iterator;
								}
								if (!incremented_independent)
								{
									++atomic_dependent_variable_iterator;
								}
								++row;
							}
							else
							{
								++atomic_dependent_variable_iterator;
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
					matrix_iterator=matrices.begin();
					while (valid&&(matrix_iterator!=last))
					{
						Function_size_type row,column;
						Function_variable_iterator atomic_dependent_variable_iterator(0),
							atomic_independent_variable_iterator(0);
						Matrix& matrix= *matrix_iterator;
						Matrix new_matrix((matrix.size1)(),
							number_of_independent_values*(matrix.size2)());
						std::list<Function_variable_handle>
							new_matrix_independent_variables;
						std::list<Function_variable_iterator>
							atomic_local_independent_iterators;
						std::list<Function_variable_iterator>::iterator
							independent_iterators_begin_iterator,
							independent_iterators_end_iterator,
							independent_iterators_iterator;

						new_matrix_independent_variables=
							*matrix_independent_variables_iterator;
						new_matrix_independent_variables.push_back(independent_variable);
						//???DB.  Generalize code above to have a set up matrix function?
						row=0;
						atomic_dependent_variable_iterator=
							local_dependent_variable->begin_atomic();
						independent_iterators_begin_iterator=
							atomic_local_independent_iterators_begin.begin();
						independent_iterators_end_iterator=
							atomic_local_independent_iterators_end.begin();
						for (i=local_order;i>0;--i)
						{
							atomic_independent_variable_iterator=
								*independent_iterators_begin_iterator;
							while ((atomic_independent_variable_iterator!=
								*independent_iterators_end_iterator)&&
								(1!=(*atomic_independent_variable_iterator)->
								number_differentiable()))
							{
								++atomic_independent_variable_iterator;
							}
							atomic_local_independent_iterators.push_back(
								*independent_iterators_begin_iterator);
							++independent_iterators_begin_iterator;
							++independent_iterators_end_iterator;
						}
						while (valid&&(atomic_dependent_variable_iterator!=
							local_dependent_variable->end_atomic()))
						{
							Function_variable_handle atomic_dependent_variable=
								*atomic_dependent_variable_iterator;

							if (1==atomic_dependent_variable->number_differentiable())
							{
								bool no_derivative;
								std::vector<Function_variable_iterator>
									atomic_independent_variable_iterators(
									new_matrix_independent_variables.size());
								std::list<Function_variable_handle>::iterator
									new_matrix_independent_variables_iterator;

								Assert(atomic_dependent_variable&&
									(atomic_dependent_variable->function()),std::logic_error(
									"Function_derivatnew::Function_derivatnew.  "
									"Atomic variable missing function()"));
								new_matrix_independent_variables_iterator=
									new_matrix_independent_variables.begin();
								i=0;
								no_derivative=false;
								while ((new_matrix_independent_variables_iterator!=
									new_matrix_independent_variables.end())&&!no_derivative)
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
									if (atomic_independent_variable_iterators[i]==
										(*new_matrix_independent_variables_iterator)->
										end_atomic())
									{
										no_derivative=true;
									}
									else
									{
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
										std::list<Function_variable_handle>
											new_matrix_atomic_independent_variables(0);

										i=new_matrix_independent_variables.size();
										while (i>0)
										{
											--i;
											new_matrix_atomic_independent_variables.push_front(
												*(atomic_independent_variable_iterators[i]));
										}
										independent_iterators_iterator=
											atomic_local_independent_iterators.end();
										for (i=local_order;i>0;--i)
										{
											--independent_iterators_iterator;
											new_matrix_atomic_independent_variables.push_front(
												*(*independent_iterators_iterator));
										}
										if ((atomic_dependent_variable->function())->
											evaluate_derivative(new_matrix(row,column),
											atomic_dependent_variable,
											new_matrix_atomic_independent_variables))
										{
											// move to next column
											i=new_matrix_independent_variables.size();
											new_matrix_independent_variables_iterator=
												new_matrix_independent_variables.end();
											if (i>0)
											{
												--i;
												--new_matrix_independent_variables_iterator;
												++atomic_independent_variable_iterators[i];
												while ((atomic_independent_variable_iterators[i]!=
													(*new_matrix_independent_variables_iterator)->
													end_atomic())&&
													(1!=(*(atomic_independent_variable_iterators[i]))->
													number_differentiable()))
												{
													++atomic_independent_variable_iterators[i];
												}
												while ((i>0)&&
													((*new_matrix_independent_variables_iterator)->
													end_atomic()==
													atomic_independent_variable_iterators[i]))
												{
													atomic_independent_variable_iterators[i]=
														(*new_matrix_independent_variables_iterator)->
														begin_atomic();
													while ((atomic_independent_variable_iterators[i]!=
														(*new_matrix_independent_variables_iterator)->
														end_atomic())&&(1!=
														(*(atomic_independent_variable_iterators[i]))->
														number_differentiable()))
													{
														++atomic_independent_variable_iterators[i];
													}
													--i;
													++atomic_independent_variable_iterators[i];
													--new_matrix_independent_variables_iterator;
													while ((atomic_independent_variable_iterators[i]!=
														(*new_matrix_independent_variables_iterator)->
														end_atomic())&&(1!=
														(*(atomic_independent_variable_iterators[i]))->
														number_differentiable()))
													{
														++atomic_independent_variable_iterators[i];
													}
												}
											}
										}
										else
										{
											valid=false;
										}
										++column;
									} while (valid&&
										((new_matrix_independent_variables.front())->
										end_atomic()!=atomic_independent_variable_iterators[0]));
								}
								// move to next row
								i=local_order;
								incremented_independent=false;
								independent_iterators_begin_iterator=
									atomic_local_independent_iterators_begin.end();
								independent_iterators_iterator=
									atomic_local_independent_iterators.end();
								independent_iterators_end_iterator=
									atomic_local_independent_iterators_end.end();
								while (!incremented_independent&&(i>0))
								{
									--independent_iterators_begin_iterator;
									--independent_iterators_iterator;
									--independent_iterators_end_iterator;
									--i;
									atomic_independent_variable_iterator=
										*independent_iterators_iterator;
									do
									{
										++atomic_independent_variable_iterator;
									}
									while ((atomic_independent_variable_iterator!=
										*independent_iterators_end_iterator)&&
										(1!=(*atomic_independent_variable_iterator)->
										number_differentiable()));
									if (atomic_independent_variable_iterator==
										*independent_iterators_end_iterator)
									{
										atomic_independent_variable_iterator=
											*independent_iterators_begin_iterator;
										while ((atomic_independent_variable_iterator!=
											*independent_iterators_end_iterator)&&
											(1!=(*atomic_independent_variable_iterator)->
											number_differentiable()))
										{
											++atomic_independent_variable_iterator;
										}
									}
									else
									{
										incremented_independent=true;
									}
									*independent_iterators_iterator=
										atomic_independent_variable_iterator;
								}
								if (!incremented_independent)
								{
									++atomic_dependent_variable_iterator;
								}
								++row;
							}
							else
							{
								++atomic_dependent_variable_iterator;
							}
						}
						matrices.push_back(new_matrix);
						matrix_independent_variables.push_back(
							new_matrix_independent_variables);
						++matrix_independent_variables_iterator;
						++matrix_iterator;
					}
					++independent_variable_iterator;
				}
				if (valid)
				{
					derivative_matrix=Derivative_matrix(matrices);
					set_evaluated();
				}
			}
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
			result=valid;
#endif // defined (EVALUATE_RETURNS_VALUE)
		}
#endif // defined (OLD_CODE)
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
