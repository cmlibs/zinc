//******************************************************************************
// FILE : function_inverse.cpp
//
// LAST MODIFIED : 18 July 2004
//
// DESCRIPTION :
//???DB
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
// Function_variable_independent is very similar to Function_variable_inverse
//   (and Function_variable_rectangular_cartesian and
//   Function_variable_finite_element and ...)
// Function_variable_iterator_representation_atomic_independent is very similar
//   to Function_variable_iterator_representation_atomic_inverse (and
//   Function_variable_iterator_representation_atomic_rectangular_cartesian and
//   Function_variable_iterator_representation_atomic_finite_element and ...)
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
#if defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)
// Function_variable_value_tolerance is very similar to
//   Function_variable_step_tolerance
#else // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)
#endif // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)
// Templates?
//==============================================================================

#include <sstream>

#include "computed_variable/function_derivative_matrix.hpp"
#include "computed_variable/function_function_size_type.hpp"
#include "computed_variable/function_identity.hpp"
#include "computed_variable/function_inverse.hpp"
#include "computed_variable/function_matrix.hpp"
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
#include "computed_variable/function_variable.hpp"
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
#include "computed_variable/function_variable_wrapper.hpp"
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
#if defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)
#include "computed_variable/function_variable.hpp"
#else // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)
#include "computed_variable/function_variable_matrix.hpp"
#endif // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_value_function_size_type.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"

// module typedefs
// ===============

#if defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)
#else // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)
typedef boost::intrusive_ptr< Function_variable_matrix<Scalar> >
	Function_variable_matrix_handle;
#endif // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)

// module classes
// ==============

#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
// forward declaration so that can use _handle
class Function_variable_inverse;
typedef boost::intrusive_ptr<Function_variable_inverse>
	Function_variable_inverse_handle;

// class Function_variable_iterator_representation_atomic_inverse
// --------------------------------------------------------------

class Function_variable_iterator_representation_atomic_inverse:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 17 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_inverse(
			const bool begin,Function_variable_inverse_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_inverse();
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
		Function_variable_iterator_representation_atomic_inverse(const
			Function_variable_iterator_representation_atomic_inverse&);
	private:
		Function_variable_inverse_handle atomic_variable,variable;
		Function_variable_iterator atomic_dependent_iterator,
			atomic_dependent_iterator_begin,atomic_dependent_iterator_end;
};


// class Function_variable_inverse
// -------------------------------

class Function_variable_inverse : public Function_variable
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_inverse;
	friend class Function_variable_iterator_representation_atomic_inverse;
	public:
		// constructor
		Function_variable_inverse(
			const Function_inverse_handle& function_inverse,
			bool dependent_estimate=false):Function_variable(function_inverse),
			atomic_dependent_variable(0),dependent_estimate(dependent_estimate){};
		// constructor.  A zero <atomic_dependent_variable> indicates the whole
		//   dependent variable
		Function_variable_inverse(
			const Function_inverse_handle& function_inverse,
			Function_variable_handle& atomic_dependent_variable,
			bool dependent_estimate=false):Function_variable(function_inverse),
			atomic_dependent_variable(0),dependent_estimate(dependent_estimate)
		{
			if (function_inverse)
			{
				if (atomic_dependent_variable&&(function_inverse->dependent_variable))
				{
					Function_variable_iterator atomic_variable_iterator,
						end_atomic_variable_iterator;

					atomic_variable_iterator=
						function_inverse->dependent_variable->begin_atomic();
					end_atomic_variable_iterator=
						function_inverse->dependent_variable->end_atomic();
					while ((atomic_variable_iterator!=end_atomic_variable_iterator)&&
						(*atomic_variable_iterator!=atomic_dependent_variable))
					{
						atomic_variable_iterator++;
					}
					if (atomic_variable_iterator!=end_atomic_variable_iterator)
					{
						this->atomic_dependent_variable=atomic_dependent_variable;
					}
				}
			}
		};
		// destructor
		~Function_variable_inverse(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_inverse_handle(
				new Function_variable_inverse(*this)));
		};
		// overload Function_variable::evaluate
		Function_handle evaluate()
		{
			Function_handle result(0);
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function());

			if (dependent_estimate)
			{
				result=Function_variable::evaluate();
			}
			else
			{
				if (function_inverse)
				{
					Function_variable_handle dependent_variable,
						independent_value_variable,independent_variable;
					Function_handle dependent_value_estimate,independent_value,result;

					result=0;
					if ((dependent_variable=function_inverse->dependent_variable)&&
						(dependent_value_estimate=dependent_variable->get_value())&&
						(independent_variable=function_inverse->independent_variable)&&
						(independent_value=independent_variable->get_value())&&
						(independent_value_variable=independent_value->output()))
					{
						bool valid;
						Scalar error_norm,increment_norm,step_tolerance,value_tolerance;
						Function_variable_handle error;
						Function_size_type iteration_number,maximum_iterations;
						std::list<Function_variable_handle> independent_variables(0);

						step_tolerance=function_inverse->step_tolerance_private;
						value_tolerance=function_inverse->value_tolerance_private;
						maximum_iterations=function_inverse->maximum_iterations_private;
						independent_variables.push_back(dependent_variable);
						valid=((independent_variable->set_value)(
							(independent_variable->evaluate)())&&
							(error=(*independent_value_variable)-(*independent_variable))&&
							(0<=(error_norm=error->norm())));
						iteration_number=0;
						increment_norm=1+step_tolerance;
						while (valid&&
							!(((0>=value_tolerance)&&(0>=step_tolerance))||
							(error_norm<value_tolerance)||(increment_norm<step_tolerance))&&
							(iteration_number<maximum_iterations))
						{
							Function_matrix_handle derivative_matrix=
								boost::dynamic_pointer_cast<Function_matrix,Function>(
								independent_variable->evaluate_derivative(
								independent_variables));

							if (derivative_matrix)
							{
								Function_matrix_handle increment_function=derivative_matrix->
									solve(boost::dynamic_pointer_cast<Function_matrix,Function>(
									error->function()));
								Function_variable_handle increment;

								if (valid=(increment_function&&
									(increment=increment_function->output())&&
									((*dependent_variable) -= (*increment))))
								{
									iteration_number++;
									valid=((independent_variable->set_value)(
										(independent_variable->evaluate)())&&
										(error=(*independent_value_variable)-
										(*independent_variable))&&
										(0<=(error_norm=error->norm()))&&
										(0<=(increment_norm=increment->norm())));
								}
							}
							else
							{
								valid=false;
							}
						}
						if (valid)
						{
							if (atomic_dependent_variable)
							{
								Function_variable_handle dependent_value_output;
								Function_variable_iterator dependent_variable_iterator,
									dependent_variable_iterator_end,dependent_value_iterator,
									dependent_value_iterator_end;
								
								dependent_variable_iterator=dependent_variable->begin_atomic();
								dependent_variable_iterator_end=
									dependent_variable->end_atomic();
								dependent_value_output=dependent_value_estimate->output();
								dependent_value_iterator=dependent_value_output->begin_atomic();
								dependent_value_iterator_end=
									dependent_value_output->end_atomic();
								while ((dependent_variable_iterator!=
									dependent_variable_iterator_end)&&
									(dependent_value_iterator!=dependent_value_iterator_end)&&
									(*dependent_variable_iterator!=atomic_dependent_variable))
								{
									dependent_variable_iterator++;
									dependent_value_iterator++;
								}
								if ((dependent_variable_iterator!=
									dependent_variable_iterator_end)&&
									(dependent_value_iterator!=dependent_value_iterator_end))
								{
									result=(*dependent_value_iterator)->get_value();
								}
							}
							else
							{
								result=dependent_value_estimate;
							}
						}
					}
				}
			}

			return (result);
		};
		// overload Function_variable::evaluate_derivative
		Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables)
		{
			Function_handle result(0);
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function());

			if (dependent_estimate)
			{
				result=Function_variable::evaluate_derivative(independent_variables);
			}
			else
			{
				if (function_inverse)
				{
					const Function_size_type number_of_independent_variables=
						independent_variables.size();
					Function_variable_handle dependent_variable,independent_variable;

					dependent_variable=function_inverse->dependent_variable;
					independent_variable=function_inverse->independent_variable;
					if (dependent_variable&&independent_variable&&
						(0<number_of_independent_variables))
					{
						Function_size_type i,maximum_number_of_independent_values;
						std::list<Function_variable_handle> extended_dependent_list(0),
							extended_independent_list(0);
						std::list<Function_variable_handle>::iterator
							independent_variables_iterator;
						std::vector<Function_size_type>
							numbers_of_independent_values(number_of_independent_variables);

						// set up extended dependent and independent variables
						extended_independent_list.push_back(independent_variable);
						extended_dependent_list.push_back(dependent_variable);
						independent_variables_iterator=independent_variables.begin();
						maximum_number_of_independent_values=0;
						for (i=0;i<number_of_independent_variables;i++)
						{
							extended_independent_list.push_back(Function_handle(
								new Function_identity(*independent_variables_iterator))->
								output());
							extended_dependent_list.push_back(
								*independent_variables_iterator);
							numbers_of_independent_values[i]=
								(*independent_variables_iterator)->number_differentiable();
							if (maximum_number_of_independent_values<
								numbers_of_independent_values[i])
							{
								maximum_number_of_independent_values=
									numbers_of_independent_values[i];
							}
							independent_variables_iterator++;
						}
						if (0<maximum_number_of_independent_values)
						{
							Function_variable_handle extended_dependent_variable(
								new Function_variable_union(extended_dependent_list));
							Function_variable_handle extended_independent_variable(
								new Function_variable_union(extended_independent_list));
							std::list<Function_variable_handle> extended_dependent_variables(
								number_of_independent_variables,extended_dependent_variable);
							Function_derivative_matrix_handle derivative_inverse(
								new Function_derivative_matrix(extended_independent_variable,
								extended_dependent_variables));

							if (derivative_inverse&&(derivative_inverse=
								boost::dynamic_pointer_cast<Function_derivative_matrix,Function>
								((derivative_inverse->inverse)())))
							{
#if defined (OLD_CODE)
								// extract Derivative_matrix for
								//   d(this)/d(independent_variables)
								Function_size_type number_of_matrices=
									(derivative_inverse->matrices).size();
								ublas::matrix<bool> matrix_variables(number_of_matrices,
									number_of_independent_variables);
								Function_size_type number_of_dependent_values,
									number_of_extended_dependent_values,
									number_of_extended_independent_values;

								for (i=0;i<number_of_matrices;i++)
								{
									for (j=0;j<number_of_independent_variables;j++)
									{
										matrix_variables(i,j)=false;
									}
								}
								if (atomic_dependent_variable)
								{
									number_of_dependent_values=
										atomic_dependent_variable->number_differentiable();
								}
								else
								{
									number_of_dependent_values=
										dependent_variable->number_differentiable();
								}
								number_of_extended_dependent_values=
									extended_dependent_variable->number_differentiable();
								number_of_extended_independent_values=
									extended_independent_variable->number_differentiable();
								if ((0<number_of_dependent_values)&&
									(0<number_of_extended_dependent_values)%%
									(0<number_of_extended_independent_values))
								{
									Function_size_type column_1,j,k;
									Function_variable_iterator variable_iterator,
										extended_variable_iterator;
									std::list<Matrix> matrices(0);
									std::list<Matrix>::iterator matrix_iterator;
									std::vector<Function_size_type> dependent_value_mapping(
										number_of_dependent_values);
									std::vector<Function_size_type> derivative_index(
										number_of_independent_variables);
									ublas::matrix<Function_size_type> independent_value_mapping(
										number_of_independent_variables,
										maximum_number_of_independent_values);

									if (atomic_dependent_variable)
									{
										variable_iterator=atomic_dependent_variable->begin_atomic();
									}
									else
									{
										variable_iterator=dependent_variable->begin_atomic();
									}
									i=0;
									while (i<number_of_dependent_values)
									{
										if (1==(*variable_iterator)->number_differentiable())
										{
											j=0;
											extended_variable_iterator=extended_dependent_variable->
												begin_atomic();
											while ((j<number_of_extended_dependent_values)&&
												(**extended_variable_iterator!= **variable_iterator))
											{
												if (1==(*extended_variable_iterator)->
													number_differentiable())
												{
													j++;
												}
												extended_variable_iterator++;
											}
											dependent_value_mapping[i]=j;
											i++;
										}
										variable_iterator++;
									}
									matrix_iterator=(derivative_inverse->matrices).begin();
									number_of_matrices=0;
									independent_variables_iterator=independent_variables.begin();
									for (i=0;i<number_of_independent_variables;i++)
									{
										Function_size_type number_of_columns=
											numbers_of_independent_values[i];
										Function_size_type column,row;
										Matrix matrix_1(number_of_dependent_values,
											number_of_columns);

										variable_iterator=
											(*independent_variables_iterator)->begin_atomic();
										j=0;
										while (j<number_of_columns)
										{
											if (1==(*variable_iterator)->number_differentiable())
											{
												k=0;
												extended_variable_iterator=
													extended_independent_variable->begin_atomic();
												while ((k<number_of_extended_independent_values)&&
													(**extended_variable_iterator!= **variable_iterator))
												{
													if (1==(*extended_variable_iterator)->
														number_differentiable())
													{
														k++;
													}
													extended_variable_iterator++;
												}
												independent_value_mapping[i,j]=k;
												j++;
											}
											variable_iterator++;
										}
										for (column=0;column<number_of_columns;column++)
										{
											column_1=independent_value_mapping[column];
											for (row=0;row<number_of_dependent_values;row++)
											{
												matrix_1(row,column)=(*matrix_iterator)(
													dependent_value_mapping[row],column_1);
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

											bool finished;
											Matrix matrix_2(number_of_dependent_values,
												number_of_columns);
											Function_size_type column_2;

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
														column_2 *= number_of_extended_independent_values;
														column_2 += independent_value_mapping(k,
															derivative_index[k]);
													}
												}
												for (row=0;row<number_of_dependent_values;row++)
												{
													matrix_2(row,column)=(*matrix_iterator)(
														dependent_value_mapping[row],column_2);
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
														if (derivative_index[k]==
															numbers_of_independent_values[k])
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
												if (finished&&
													matrix_variables(j+number_of_matrices+1,0))
												{
													derivative_index[0]++;
													finished=(derivative_index[0]==
														numbers_of_independent_values[0]);
												}
											}
											matrices.push_back(matrix_2);
											matrix_iterator++;
										}
										number_of_matrices += number_of_matrices+1;
										independent_variables_iterator++;
									}
									result=Function_derivative_matrix_handle(
										new Function_derivative_matrix(
										Function_inverse_handle(this),independent_variables,
										matrices));
								}
#endif // defined (OLD_CODE)
								// extract Matrix for d(this)/d(independent_variables)
								Function_size_type number_of_dependent_values,
									number_of_extended_dependent_values,
									number_of_extended_independent_values;

								if (atomic_dependent_variable)
								{
									number_of_dependent_values=
										atomic_dependent_variable->number_differentiable();
								}
								else
								{
									number_of_dependent_values=
										dependent_variable->number_differentiable();
								}
								number_of_extended_dependent_values=
									extended_dependent_variable->number_differentiable();
								number_of_extended_independent_values=
									extended_independent_variable->number_differentiable();
								if ((0<number_of_dependent_values)&&
									(0<number_of_extended_dependent_values)&&
									(0<number_of_extended_independent_values))
								{
									Function_size_type j,k,number_of_columns;
									Function_variable_iterator variable_iterator,
										extended_variable_iterator;
									std::vector<Function_size_type> dependent_value_mapping(
										number_of_dependent_values);
									std::vector<Function_size_type> derivative_index(
										number_of_independent_variables);
									ublas::matrix<Function_size_type> independent_value_mapping(
										number_of_independent_variables,
										maximum_number_of_independent_values);

									if (atomic_dependent_variable)
									{
										variable_iterator=atomic_dependent_variable->begin_atomic();
									}
									else
									{
										variable_iterator=dependent_variable->begin_atomic();
									}
									i=0;
									while (i<number_of_dependent_values)
									{
										if (1==(*variable_iterator)->number_differentiable())
										{
											j=0;
											extended_variable_iterator=extended_dependent_variable->
												begin_atomic();
											while ((j<number_of_extended_dependent_values)&&
												!(**extended_variable_iterator== **variable_iterator))
											{
												if (1==(*extended_variable_iterator)->
													number_differentiable())
												{
													j++;
												}
												extended_variable_iterator++;
											}
											dependent_value_mapping[i]=j;
											i++;
										}
										variable_iterator++;
									}
									number_of_columns=1;
									independent_variables_iterator=independent_variables.begin();
									for (i=0;i<number_of_independent_variables;i++)
									{
										Function_size_type number_of_independent_values=
											numbers_of_independent_values[i];

										// initialize derivative_index
										derivative_index[i]=0;
										// update number_of_columns
										number_of_columns *= number_of_independent_values;
										// initialize independent_value_mapping(i,*)
										variable_iterator=
											(*independent_variables_iterator)->begin_atomic();
										j=0;
										while (j<number_of_independent_values)
										{
											if (1==(*variable_iterator)->number_differentiable())
											{
												k=0;
												extended_variable_iterator=
													extended_independent_variable->begin_atomic();
												while ((k<number_of_extended_independent_values)&&
													!(**extended_variable_iterator== **variable_iterator))
												{
													if (1==(*extended_variable_iterator)->
														number_differentiable())
													{
														k++;
													}
													extended_variable_iterator++;
												}
												independent_value_mapping(i,j)=k;
												j++;
											}
											variable_iterator++;
										}
										independent_variables_iterator++;
									}
									if (0<number_of_columns)
									{
										bool finished;
										Matrix matrix(number_of_dependent_values,number_of_columns);
										Matrix
											&matrix_extended=(derivative_inverse->matrices).back();
										Function_size_type column,column_extended,row;

										column=0;
										finished=false;
										while (!finished)
										{
											column_extended=0;
											for (i=0;i<number_of_independent_variables;i++)
											{
												column_extended *=
													number_of_extended_independent_values;
												column_extended +=
													independent_value_mapping(i,derivative_index[i]);
											}
											for (row=0;row<number_of_dependent_values;row++)
											{
												matrix(row,column)=matrix_extended(
													dependent_value_mapping[row],column_extended);
											}
											column++;
											// increment derivative_index
											finished=true;
											i=number_of_independent_variables;
											while (finished&&(i>0))
											{
												derivative_index[i]++;
												if (derivative_index[i]==
													numbers_of_independent_values[i])
												{
													derivative_index[i]=0;
												}
												else
												{
													finished=false;
												}
												i--;
											}
											if (finished)
											{
												derivative_index[0]++;
												finished=(derivative_index[0]==
													numbers_of_independent_values[0]);
											}
										}
										result=Function_matrix_handle(new Function_matrix(matrix));
									}
								}
							}
						}
					}
				}
			}

			return (result);
		};
		string_handle get_string_representation()
		{
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function());
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				if (atomic_dependent_variable)
				{
					out << *(atomic_dependent_variable->get_string_representation());
				}
				else
				{
					if (function_inverse&&(function_inverse->dependent_variable))
					{
						out << *(function_inverse->dependent_variable->
							get_string_representation());
					}
				}
				if (dependent_estimate)
				{
					out << " estimate";
				}
				*return_string=out.str();
			}

			return (return_string);
		};
		virtual Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_inverse(
				true,Function_variable_inverse_handle(
				const_cast<Function_variable_inverse*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_inverse(
				false,Function_variable_inverse_handle(
				const_cast<Function_variable_inverse*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_inverse(
				false,Function_variable_inverse_handle(
				const_cast<Function_variable_inverse*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_inverse(
				true,Function_variable_inverse_handle(
				const_cast<Function_variable_inverse*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			Function_size_type result;

			result=0;
			if (this)
			{
				if (atomic_dependent_variable)
				{
					result=atomic_dependent_variable->number_differentiable();
				}
				else
				{
					Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
						Function_inverse,Function>(function());

					if (function_inverse&&(function_inverse->dependent_variable))
					{
						result=function_inverse->dependent_variable->
							number_differentiable();
					}
				}
			}

			return (result);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_inverse_handle variable_inverse;

			result=false;
			if (variable_inverse=boost::dynamic_pointer_cast<
				Function_variable_inverse,Function_variable>(variable))
			{
				if ((variable_inverse->function()==function())&&
					(variable_inverse->dependent_estimate==dependent_estimate)&&
					(((0==variable_inverse->atomic_dependent_variable)&&
					(0==atomic_dependent_variable))||(atomic_dependent_variable&&
					(variable_inverse->atomic_dependent_variable)&&
					(*(variable_inverse->atomic_dependent_variable)==
					*atomic_dependent_variable))))
				{
					result=true;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_inverse(
			const Function_variable_inverse& variable_inverse):
			Function_variable(variable_inverse),
			atomic_dependent_variable(variable_inverse.atomic_dependent_variable){}
		// assignment
		Function_variable_inverse& operator=(const Function_variable_inverse&);
	private:
		// if zero then all
		Function_variable_handle atomic_dependent_variable;
		// if true then the initial estimate for the result otherwise the result
		bool dependent_estimate;
};


// class Function_variable_iterator_representation_atomic_inverse
// --------------------------------------------------------------

Function_variable_iterator_representation_atomic_inverse::
	Function_variable_iterator_representation_atomic_inverse(
	const bool begin,Function_variable_inverse_handle variable):
	Function_variable_iterator_representation(),atomic_variable(0),
	variable(variable),atomic_dependent_iterator(0),
	atomic_dependent_iterator_begin(0),atomic_dependent_iterator_end(0)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable)
	{
		Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
			Function_inverse,Function>(variable->function());

		if (function_inverse&&(atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_inverse,Function_variable>(variable->clone())))
		{
			atomic_dependent_iterator=0;
			atomic_dependent_iterator_begin=0;
			atomic_dependent_iterator_end=0;
			if (variable->atomic_dependent_variable)
			{
				atomic_dependent_iterator_begin=
					variable->atomic_dependent_variable->begin_atomic();
				atomic_dependent_iterator_end=
					variable->atomic_dependent_variable->end_atomic();
			}
			else
			{
				Function_variable_handle dependent_variable_local;

				if (dependent_variable_local=function_inverse->dependent_variable)
				{
					atomic_dependent_iterator_begin=
						dependent_variable_local->begin_atomic();
					atomic_dependent_iterator_end=
						dependent_variable_local->end_atomic();
				}
				else
				{
					// end
					atomic_variable=0;
				}
			}
			if (atomic_variable&&
				(atomic_dependent_iterator_begin!=atomic_dependent_iterator_end))
			{
				atomic_dependent_iterator=atomic_dependent_iterator_begin;
				atomic_variable->atomic_dependent_variable= *atomic_dependent_iterator;
			}
			else
			{
				// end
				atomic_variable=0;
			}
		}
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_inverse::clone()
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new Function_variable_iterator_representation_atomic_inverse(*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_inverse::
	~Function_variable_iterator_representation_atomic_inverse()
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_inverse::increment()
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		bool finished;

		finished=false;
		atomic_dependent_iterator++;
		if (atomic_dependent_iterator!=atomic_dependent_iterator_end)
		{
			atomic_variable->atomic_dependent_variable= *atomic_dependent_iterator;
			finished=true;
		}
		if (!finished)
		{
			atomic_variable=0;
		}
	}
}

void Function_variable_iterator_representation_atomic_inverse::decrement()
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		bool finished;

		finished=false;
		if (atomic_dependent_iterator!=atomic_dependent_iterator_begin)
		{
			atomic_dependent_iterator--;
			atomic_variable->atomic_dependent_variable=
				*atomic_dependent_iterator;
			finished=true;
		}
		if (!finished)
		{
			atomic_variable=0;
		}
	}
	else
	{
		if (variable)
		{
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(variable->function());

			if (function_inverse&&(atomic_variable=boost::dynamic_pointer_cast<
				Function_variable_inverse,Function_variable>(variable->clone())))
			{
				atomic_dependent_iterator=0;
				atomic_dependent_iterator_begin=0;
				atomic_dependent_iterator_end=0;
				if (variable->atomic_dependent_variable)
				{
					atomic_dependent_iterator_begin=
						variable->atomic_dependent_variable->begin_atomic();
					atomic_dependent_iterator_end=
						variable->atomic_dependent_variable->end_atomic();
				}
				else
				{
					Function_variable_handle dependent_variable_local;

					if (dependent_variable_local=function_inverse->dependent_variable)
					{
						atomic_dependent_iterator_begin=
							dependent_variable_local->begin_atomic();
						atomic_dependent_iterator_end=
							dependent_variable_local->end_atomic();
					}
					else
					{
						// end
						atomic_variable=0;
					}
				}
				if (atomic_variable&&
					(atomic_dependent_iterator_begin!=atomic_dependent_iterator_end))
				{
					atomic_dependent_iterator=atomic_dependent_iterator_end;
					atomic_dependent_iterator--;
					atomic_variable->atomic_dependent_variable=
						*atomic_dependent_iterator;
				}
			}
		}
	}
}

bool Function_variable_iterator_representation_atomic_inverse::equality(
	const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_inverse
		*representation_inverse=dynamic_cast<const
		Function_variable_iterator_representation_atomic_inverse *>(
		representation);

	result=false;
	if (representation_inverse)
	{
		if (((0==atomic_variable)&&(0==representation_inverse->atomic_variable))||
			(atomic_variable&&(representation_inverse->atomic_variable)&&
			(*atomic_variable== *(representation_inverse->atomic_variable))))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_inverse::dereference() const
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_inverse::
	Function_variable_iterator_representation_atomic_inverse(const
	Function_variable_iterator_representation_atomic_inverse&
	representation):Function_variable_iterator_representation(),
	atomic_variable(0),variable(representation.variable),
	atomic_dependent_iterator(representation.atomic_dependent_iterator),
	atomic_dependent_iterator_begin(
	representation.atomic_dependent_iterator_begin),
	atomic_dependent_iterator_end(representation.atomic_dependent_iterator_end)
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_inverse,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}


// class Function_variable_dependent_estimate
// ------------------------------------------

//???DB.  Included in Function_variable_inverse

#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)

// class Function_variable_dependent
// ---------------------------------

// forward declaration so that can use _handle
class Function_variable_dependent;
typedef boost::intrusive_ptr<Function_variable_dependent>
	Function_variable_dependent_handle;

class Function_variable_dependent : public Function_variable_wrapper
//******************************************************************************
// LAST MODIFIED : 9 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_dependent(
			const Function_inverse_handle& function_inverse):
			Function_variable_wrapper(function_inverse,
			function_inverse->dependent_variable){};
		// destructor
		~Function_variable_dependent(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_dependent_handle(
				new Function_variable_dependent(*this)));
		};
		// overload Function_variable::evaluate
		Function_handle evaluate()
		{
			Function_handle result(0);
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function());
			Function_variable_handle specified_dependent_variable=get_wrapped();

			if (function_inverse&&specified_dependent_variable)
			{
				Function_variable_handle dependent_variable,
					independent_value_variable,independent_variable;
				Function_handle dependent_value_estimate,independent_value,result;

				result=0;
				if ((dependent_variable=function_inverse->dependent_variable)&&
					(dependent_value_estimate=dependent_variable->get_value())&&
					(independent_variable=function_inverse->independent_variable)&&
					(independent_value=independent_variable->get_value())&&
					(independent_value_variable=independent_value->output()))
				{
					bool valid;
					Scalar error_norm,increment_norm,step_tolerance,value_tolerance;
					Function_variable_handle error;
					Function_size_type iteration_number,maximum_iterations;
					std::list<Function_variable_handle> independent_variables(0);

					step_tolerance=function_inverse->step_tolerance_private;
					value_tolerance=function_inverse->value_tolerance_private;
					maximum_iterations=function_inverse->maximum_iterations_private;
					independent_variables.push_back(dependent_variable);
					valid=((independent_variable->set_value)(
						(independent_variable->evaluate)())&&
						(error=(*independent_value_variable)-(*independent_variable))&&
						(0<=(error_norm=error->norm())));
					iteration_number=0;
					increment_norm=1+step_tolerance;
					while (valid&&
						!(((0>=value_tolerance)&&(0>=step_tolerance))||
						(error_norm<value_tolerance)||(increment_norm<step_tolerance))&&
						(iteration_number<maximum_iterations))
					{
						Function_matrix_handle derivative_matrix=
							boost::dynamic_pointer_cast<Function_matrix,Function>(
							independent_variable->evaluate_derivative(
							independent_variables));

						if (derivative_matrix)
						{
							Function_matrix_handle increment_function=derivative_matrix->
								solve(boost::dynamic_pointer_cast<Function_matrix,Function>(
								error->function()));
							Function_variable_handle increment;

							if (valid=(increment_function&&
								(increment=increment_function->output())&&
								((*dependent_variable) -= (*increment))))
							{
								iteration_number++;
								valid=((independent_variable->set_value)(
									(independent_variable->evaluate)())&&
									(error=(*independent_value_variable)-
									(*independent_variable))&&
									(0<=(error_norm=error->norm()))&&
									(0<=(increment_norm=increment->norm())));
							}
						}
						else
						{
							valid=false;
						}
					}
					if (valid)
					{
						if (*specified_dependent_variable== *dependent_variable)
						{
							result=dependent_value_estimate;
						}
						else
						{
							Function_variable_handle dependent_value_output;
							Function_variable_iterator dependent_variable_iterator,
								dependent_variable_iterator_end,dependent_value_iterator,
								dependent_value_iterator_end;
							
							dependent_variable_iterator=dependent_variable->begin_atomic();
							dependent_variable_iterator_end=
								dependent_variable->end_atomic();
							dependent_value_output=dependent_value_estimate->output();
							dependent_value_iterator=dependent_value_output->begin_atomic();
							dependent_value_iterator_end=
								dependent_value_output->end_atomic();
							while ((dependent_variable_iterator!=
								dependent_variable_iterator_end)&&
								(dependent_value_iterator!=dependent_value_iterator_end)&&
								(*dependent_variable_iterator!=specified_dependent_variable))
							{
								dependent_variable_iterator++;
								dependent_value_iterator++;
							}
							if ((dependent_variable_iterator!=
								dependent_variable_iterator_end)&&
								(dependent_value_iterator!=dependent_value_iterator_end))
							{
								result=(*dependent_value_iterator)->get_value();
							}
						}
					}
				}
			}

			return (result);
		};
		// overload Function_variable::evaluate_derivative
		Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables)
		{
			Function_handle result(0);
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function());

			if (function_inverse)
			{
				const Function_size_type number_of_independent_variables=
					independent_variables.size();
				Function_variable_handle dependent_variable,independent_variable;

				dependent_variable=function_inverse->dependent_variable;
				independent_variable=function_inverse->independent_variable;
				if (dependent_variable&&independent_variable&&
					(0<number_of_independent_variables))
				{
					Function_size_type i,maximum_number_of_independent_values;
					std::list<Function_variable_handle> extended_dependent_list(0),
						extended_independent_list(0);
					std::list<Function_variable_handle>::iterator
						independent_variables_iterator;
					std::vector<Function_size_type>
						numbers_of_independent_values(number_of_independent_variables);

					// set up extended dependent and independent variables
					extended_independent_list.push_back(independent_variable);
					extended_dependent_list.push_back(dependent_variable);
					independent_variables_iterator=independent_variables.begin();
					maximum_number_of_independent_values=0;
					for (i=0;i<number_of_independent_variables;i++)
					{
						extended_independent_list.push_back(Function_handle(
							new Function_identity(*independent_variables_iterator))->
							output());
						extended_dependent_list.push_back(
							*independent_variables_iterator);
						numbers_of_independent_values[i]=
							(*independent_variables_iterator)->number_differentiable();
						if (maximum_number_of_independent_values<
							numbers_of_independent_values[i])
						{
							maximum_number_of_independent_values=
								numbers_of_independent_values[i];
						}
						independent_variables_iterator++;
					}
					if (0<maximum_number_of_independent_values)
					{
						Function_variable_handle extended_dependent_variable(
							new Function_variable_union(extended_dependent_list));
						Function_variable_handle extended_independent_variable(
							new Function_variable_union(extended_independent_list));
						std::list<Function_variable_handle> extended_dependent_variables(
							number_of_independent_variables,extended_dependent_variable);
						Function_derivative_matrix_handle derivative_inverse(
							new Function_derivative_matrix(extended_independent_variable,
							extended_dependent_variables));

						if (derivative_inverse&&(derivative_inverse=
							boost::dynamic_pointer_cast<Function_derivative_matrix,Function>
							((derivative_inverse->inverse)())))
						{
#if defined (OLD_CODE)
							// extract Derivative_matrix for
							//   d(this)/d(independent_variables)
							Function_size_type number_of_matrices=
								(derivative_inverse->matrices).size();
							ublas::matrix<bool> matrix_variables(number_of_matrices,
								number_of_independent_variables);
							Function_size_type number_of_dependent_values,
								number_of_extended_dependent_values,
								number_of_extended_independent_values;

							for (i=0;i<number_of_matrices;i++)
							{
								for (j=0;j<number_of_independent_variables;j++)
								{
									matrix_variables(i,j)=false;
								}
							}
							number_of_dependent_values=number_differentiable();
							number_of_extended_dependent_values=
								extended_dependent_variable->number_differentiable();
							number_of_extended_independent_values=
								extended_independent_variable->number_differentiable();
							if ((0<number_of_dependent_values)&&
								(0<number_of_extended_dependent_values)%%
								(0<number_of_extended_independent_values))
							{
								Function_size_type column_1,j,k;
								Function_variable_iterator variable_iterator,
									extended_variable_iterator;
								std::list<Matrix> matrices(0);
								std::list<Matrix>::iterator matrix_iterator;
								std::vector<Function_size_type> dependent_value_mapping(
									number_of_dependent_values);
								std::vector<Function_size_type> derivative_index(
									number_of_independent_variables);
								ublas::matrix<Function_size_type> independent_value_mapping(
									number_of_independent_variables,
									maximum_number_of_independent_values);

								variable_iterator=begin_atomic();
								i=0;
								while (i<number_of_dependent_values)
								{
									if (1==(*variable_iterator)->number_differentiable())
									{
										j=0;
										extended_variable_iterator=extended_dependent_variable->
											begin_atomic();
										while ((j<number_of_extended_dependent_values)&&
											(**extended_variable_iterator!= **variable_iterator))
										{
											if (1==(*extended_variable_iterator)->
												number_differentiable())
											{
												j++;
											}
											extended_variable_iterator++;
										}
										dependent_value_mapping[i]=j;
										i++;
									}
									variable_iterator++;
								}
								matrix_iterator=(derivative_inverse->matrices).begin();
								number_of_matrices=0;
								independent_variables_iterator=independent_variables.begin();
								for (i=0;i<number_of_independent_variables;i++)
								{
									Function_size_type number_of_columns=
										numbers_of_independent_values[i];
									Function_size_type column,row;
									Matrix matrix_1(number_of_dependent_values,
										number_of_columns);

									variable_iterator=
										(*independent_variables_iterator)->begin_atomic();
									j=0;
									while (j<number_of_columns)
									{
										if (1==(*variable_iterator)->number_differentiable())
										{
											k=0;
											extended_variable_iterator=
												extended_independent_variable->begin_atomic();
											while ((k<number_of_extended_independent_values)&&
												(**extended_variable_iterator!= **variable_iterator))
											{
												if (1==(*extended_variable_iterator)->
													number_differentiable())
												{
													k++;
												}
												extended_variable_iterator++;
											}
											independent_value_mapping[i,j]=k;
											j++;
										}
										variable_iterator++;
									}
									for (column=0;column<number_of_columns;column++)
									{
										column_1=independent_value_mapping[column];
										for (row=0;row<number_of_dependent_values;row++)
										{
											matrix_1(row,column)=(*matrix_iterator)(
												dependent_value_mapping[row],column_1);
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

										bool finished;
										Matrix matrix_2(number_of_dependent_values,
											number_of_columns);
										Function_size_type column_2;

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
													column_2 *= number_of_extended_independent_values;
													column_2 += independent_value_mapping(k,
														derivative_index[k]);
												}
											}
											for (row=0;row<number_of_dependent_values;row++)
											{
												matrix_2(row,column)=(*matrix_iterator)(
													dependent_value_mapping[row],column_2);
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
													if (derivative_index[k]==
														numbers_of_independent_values[k])
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
											if (finished&&
												matrix_variables(j+number_of_matrices+1,0))
											{
												derivative_index[0]++;
												finished=(derivative_index[0]==
													numbers_of_independent_values[0]);
											}
										}
										matrices.push_back(matrix_2);
										matrix_iterator++;
									}
									number_of_matrices += number_of_matrices+1;
									independent_variables_iterator++;
								}
								result=Function_derivative_matrix_handle(
									new Function_derivative_matrix(
									Function_inverse_handle(this),independent_variables,
									matrices));
							}
#endif // defined (OLD_CODE)
							// extract Matrix for d(this)/d(independent_variables)
							Function_size_type number_of_dependent_values,
								number_of_extended_dependent_values,
								number_of_extended_independent_values;

							number_of_dependent_values=number_differentiable();
							number_of_extended_dependent_values=
								extended_dependent_variable->number_differentiable();
							number_of_extended_independent_values=
								extended_independent_variable->number_differentiable();
							if ((0<number_of_dependent_values)&&
								(0<number_of_extended_dependent_values)&&
								(0<number_of_extended_independent_values))
							{
								Function_size_type j,k,number_of_columns;
								Function_variable_iterator variable_iterator,
									extended_variable_iterator;
								std::vector<Function_size_type> dependent_value_mapping(
									number_of_dependent_values);
								std::vector<Function_size_type> derivative_index(
									number_of_independent_variables);
								ublas::matrix<Function_size_type> independent_value_mapping(
									number_of_independent_variables,
									maximum_number_of_independent_values);

								variable_iterator=begin_atomic();
								i=0;
								while (i<number_of_dependent_values)
								{
									if (1==(*variable_iterator)->number_differentiable())
									{
										j=0;
										extended_variable_iterator=extended_dependent_variable->
											begin_atomic();
										while ((j<number_of_extended_dependent_values)&&
											!(**extended_variable_iterator== **variable_iterator))
										{
											if (1==(*extended_variable_iterator)->
												number_differentiable())
											{
												j++;
											}
											extended_variable_iterator++;
										}
										dependent_value_mapping[i]=j;
										i++;
									}
									variable_iterator++;
								}
								number_of_columns=1;
								independent_variables_iterator=independent_variables.begin();
								for (i=0;i<number_of_independent_variables;i++)
								{
									Function_size_type number_of_independent_values=
										numbers_of_independent_values[i];

									// initialize derivative_index
									derivative_index[i]=0;
									// update number_of_columns
									number_of_columns *= number_of_independent_values;
									// initialize independent_value_mapping(i,*)
									variable_iterator=
										(*independent_variables_iterator)->begin_atomic();
									j=0;
									while (j<number_of_independent_values)
									{
										if (1==(*variable_iterator)->number_differentiable())
										{
											k=0;
											extended_variable_iterator=
												extended_independent_variable->begin_atomic();
											while ((k<number_of_extended_independent_values)&&
												!(**extended_variable_iterator== **variable_iterator))
											{
												if (1==(*extended_variable_iterator)->
													number_differentiable())
												{
													k++;
												}
												extended_variable_iterator++;
											}
											independent_value_mapping(i,j)=k;
											j++;
										}
										variable_iterator++;
									}
									independent_variables_iterator++;
								}
								if (0<number_of_columns)
								{
									bool finished;
									Matrix matrix(number_of_dependent_values,number_of_columns);
									Matrix
										&matrix_extended=(derivative_inverse->matrices).back();
									Function_size_type column,column_extended,row;

									column=0;
									finished=false;
									while (!finished)
									{
										column_extended=0;
										for (i=0;i<number_of_independent_variables;i++)
										{
											column_extended *=
												number_of_extended_independent_values;
											column_extended +=
												independent_value_mapping(i,derivative_index[i]);
										}
										for (row=0;row<number_of_dependent_values;row++)
										{
											matrix(row,column)=matrix_extended(
												dependent_value_mapping[row],column_extended);
										}
										column++;
										// increment derivative_index
										finished=true;
										i=number_of_independent_variables;
										while (finished&&(i>0))
										{
											derivative_index[i]++;
											if (derivative_index[i]==
												numbers_of_independent_values[i])
											{
												derivative_index[i]=0;
											}
											else
											{
												finished=false;
											}
											i--;
										}
										if (finished)
										{
											derivative_index[0]++;
											finished=(derivative_index[0]==
												numbers_of_independent_values[0]);
										}
									}
									result=Function_matrix_handle(new Function_matrix(matrix));
								}
							}
						}
					}
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_dependent(
			const Function_variable_dependent& variable_dependent):
			Function_variable_wrapper(variable_dependent){};
		// assignment
		Function_variable_dependent& operator=(const Function_variable_dependent&);
};


// class Function_variable_dependent_estimate
// ------------------------------------------

// forward declaration so that can use _handle
class Function_variable_dependent_estimate;
typedef boost::intrusive_ptr<Function_variable_dependent_estimate>
	Function_variable_dependent_estimate_handle;

class Function_variable_dependent_estimate : public Function_variable_wrapper
//******************************************************************************
// LAST MODIFIED : 9 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_dependent_estimate(
			const Function_inverse_handle& function_inverse):
			Function_variable_wrapper(function_inverse,
			function_inverse->dependent_variable){};
		// destructor
		~Function_variable_dependent_estimate(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_dependent_estimate_handle(
				new Function_variable_dependent_estimate(*this)));
		};
		string_handle get_string_representation()
		{
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function());
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				out << *(Function_variable_wrapper::get_string_representation())
					<< " estimate";
				*return_string=out.str();
			}

			return (return_string);
		};
	private:
		// copy constructor
		Function_variable_dependent_estimate(
			const Function_variable_dependent_estimate& variable_dependent_estimate):
			Function_variable_wrapper(variable_dependent_estimate){};
		// assignment
		Function_variable_dependent_estimate& operator=(
			const Function_variable_dependent_estimate&);
};
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)


#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
// forward declaration so that can use _handle
class Function_variable_independent;
typedef boost::intrusive_ptr<Function_variable_independent>
	Function_variable_independent_handle;

// class Function_variable_iterator_representation_atomic_independent
// ------------------------------------------------------------------

class Function_variable_iterator_representation_atomic_independent:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_independent(
			const bool begin,Function_variable_independent_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_independent();
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
		Function_variable_iterator_representation_atomic_independent(const
			Function_variable_iterator_representation_atomic_independent&);
	private:
		Function_variable_independent_handle atomic_variable,variable;
		Function_variable_iterator atomic_independent_iterator,
			atomic_independent_iterator_begin,atomic_independent_iterator_end;
};


// class Function_variable_independent
// -----------------------------------

class Function_variable_independent : public Function_variable
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_inverse;
	friend class Function_variable_iterator_representation_atomic_independent;
	public:
		// constructor
		Function_variable_independent(
			const Function_inverse_handle& function_inverse):
			Function_variable(function_inverse),atomic_independent_variable(0){};
		// constructor.  A zero <atomic_independent_variable> indicates the whole
		//   independent variable
		Function_variable_independent(
			const Function_inverse_handle& function_inverse,
			Function_variable_handle& atomic_independent_variable):
			Function_variable(function_inverse),atomic_independent_variable(0)
		{
			if (function_inverse)
			{
				if (atomic_independent_variable&&
					(function_inverse->independent_variable))
				{
					Function_variable_iterator atomic_variable_iterator,
						end_atomic_variable_iterator;

					atomic_variable_iterator=
						function_inverse->independent_variable->begin_atomic();
					end_atomic_variable_iterator=
						function_inverse->independent_variable->end_atomic();
					while ((atomic_variable_iterator!=end_atomic_variable_iterator)&&
						(*atomic_variable_iterator!=atomic_independent_variable))
					{
						atomic_variable_iterator++;
					}
					if (atomic_variable_iterator!=end_atomic_variable_iterator)
					{
						this->atomic_independent_variable=atomic_independent_variable;
					}
				}
			}
		};
		// destructor
		~Function_variable_independent(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_independent_handle(
				new Function_variable_independent(*this)));
		};
		string_handle get_string_representation()
		{
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function());
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				if (atomic_independent_variable)
				{
					out << *(atomic_independent_variable->get_string_representation());
				}
				else
				{
					if (function_inverse&&(function_inverse->independent_variable))
					{
						out << *(function_inverse->independent_variable->
							get_string_representation());
					}
				}
				*return_string=out.str();
			}

			return (return_string);
		};
		virtual Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_independent(
				true,Function_variable_independent_handle(
				const_cast<Function_variable_independent*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_independent(
				false,Function_variable_independent_handle(
				const_cast<Function_variable_independent*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_independent(
				false,Function_variable_independent_handle(
				const_cast<Function_variable_independent*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_independent(
				true,Function_variable_independent_handle(
				const_cast<Function_variable_independent*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			Function_size_type result;

			result=0;
			if (this)
			{
				Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
					Function_inverse,Function>(function());

				if (atomic_independent_variable)
				{
					result=atomic_independent_variable->number_differentiable();
				}
				else
				{
					if (function_inverse&&(function_inverse->independent_variable))
					{
						result=function_inverse->independent_variable->
							number_differentiable();
					}
				}
			}

			return (result);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_independent_handle variable_independent;

			result=false;
			if (variable_independent=boost::dynamic_pointer_cast<
				Function_variable_independent,Function_variable>(variable))
			{
				if ((variable_independent->function()==function())&&
					(((0==variable_independent->atomic_independent_variable)&&
					(0==atomic_independent_variable))||(atomic_independent_variable&&
					(variable_independent->atomic_independent_variable)&&
					(*(variable_independent->atomic_independent_variable)==
					*atomic_independent_variable))))
				{
					result=true;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_independent(
			const Function_variable_independent& variable_independent):
			Function_variable(variable_independent),atomic_independent_variable(
			variable_independent.atomic_independent_variable){}
		// assignment
		Function_variable_independent& operator=(
			const Function_variable_independent&);
	private:
		// if zero then all
		Function_variable_handle atomic_independent_variable;
};


// class Function_variable_iterator_representation_atomic_independent
// ------------------------------------------------------------------

Function_variable_iterator_representation_atomic_independent::
	Function_variable_iterator_representation_atomic_independent(
	const bool begin,Function_variable_independent_handle variable):
	Function_variable_iterator_representation(),atomic_variable(0),
	variable(variable),atomic_independent_iterator(0),
	atomic_independent_iterator_begin(0),atomic_independent_iterator_end(0)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable)
	{
		Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
			Function_inverse,Function>(variable->function());

		if (function_inverse&&(atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_independent,Function_variable>(variable->clone())))
		{
			atomic_independent_iterator=0;
			atomic_independent_iterator_begin=0;
			atomic_independent_iterator_end=0;
			if (variable->atomic_independent_variable)
			{
				atomic_independent_iterator_begin=
					variable->atomic_independent_variable->begin_atomic();
				atomic_independent_iterator_end=
					variable->atomic_independent_variable->end_atomic();
			}
			else
			{
				Function_variable_handle independent_variable_local;

				if (independent_variable_local=function_inverse->independent_variable)
				{
					atomic_independent_iterator_begin=
						independent_variable_local->begin_atomic();
					atomic_independent_iterator_end=
						independent_variable_local->end_atomic();
				}
				else
				{
					// end
					atomic_variable=0;
				}
			}
			if (atomic_variable&&
				(atomic_independent_iterator_begin!=atomic_independent_iterator_end))
			{
				atomic_independent_iterator=atomic_independent_iterator_begin;
				atomic_variable->atomic_independent_variable=
					*atomic_independent_iterator;
			}
			else
			{
				// end
				atomic_variable=0;
			}
		}
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_independent::clone()
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new Function_variable_iterator_representation_atomic_independent(
			*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_independent::
	~Function_variable_iterator_representation_atomic_independent()
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_independent::increment()
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		bool finished;

		finished=false;
		atomic_independent_iterator++;
		if (atomic_independent_iterator!=atomic_independent_iterator_end)
		{
			atomic_variable->atomic_independent_variable=
				*atomic_independent_iterator;
			finished=true;
		}
		if (!finished)
		{
			atomic_variable=0;
		}
	}
}

void Function_variable_iterator_representation_atomic_independent::decrement()
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		bool finished;

		finished=false;
		if (atomic_independent_iterator!=atomic_independent_iterator_begin)
		{
			atomic_independent_iterator--;
			atomic_variable->atomic_independent_variable=
				*atomic_independent_iterator;
			finished=true;
		}
		if (!finished)
		{
			atomic_variable=0;
		}
	}
	else
	{
		if (variable)
		{
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(variable->function());

			if (function_inverse&&(atomic_variable=boost::dynamic_pointer_cast<
				Function_variable_independent,Function_variable>(variable->clone())))
			{
				atomic_independent_iterator=0;
				atomic_independent_iterator_begin=0;
				atomic_independent_iterator_end=0;
				if (variable->atomic_independent_variable)
				{
					atomic_independent_iterator_begin=
						variable->atomic_independent_variable->begin_atomic();
					atomic_independent_iterator_end=
						variable->atomic_independent_variable->end_atomic();
				}
				else
				{
					Function_variable_handle independent_variable_local;

					if (independent_variable_local=function_inverse->independent_variable)
					{
						atomic_independent_iterator_begin=
							independent_variable_local->begin_atomic();
						atomic_independent_iterator_end=
							independent_variable_local->end_atomic();
					}
					else
					{
						// end
						atomic_variable=0;
					}
				}
				if (atomic_variable&&
					(atomic_independent_iterator_begin!=atomic_independent_iterator_end))
				{
					atomic_independent_iterator=atomic_independent_iterator_end;
					atomic_independent_iterator--;
					atomic_variable->atomic_independent_variable=
						*atomic_independent_iterator;
				}
				else
				{
					// end
					atomic_variable=0;
				}
			}
		}
	}
}

bool Function_variable_iterator_representation_atomic_independent::equality(
	const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_independent
		*representation_independent=dynamic_cast<const
		Function_variable_iterator_representation_atomic_independent *>(
		representation);

	result=false;
	if (representation_independent)
	{
		if (
			((0==atomic_variable)&&(0==representation_independent->atomic_variable))||
			(atomic_variable&&(representation_independent->atomic_variable)&&
			(*atomic_variable== *(representation_independent->atomic_variable))))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_independent::
	dereference() const
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_independent::
	Function_variable_iterator_representation_atomic_independent(const
	Function_variable_iterator_representation_atomic_independent&
	representation):Function_variable_iterator_representation(),
	atomic_variable(0),variable(representation.variable),
	atomic_independent_iterator(representation.atomic_independent_iterator),
	atomic_independent_iterator_begin(
	representation.atomic_independent_iterator_begin),
	atomic_independent_iterator_end(
	representation.atomic_independent_iterator_end)
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_independent,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)

// class Function_variable_independent
// -----------------------------------

// forward declaration so that can use _handle
class Function_variable_independent;
typedef boost::intrusive_ptr<Function_variable_independent>
	Function_variable_independent_handle;

class Function_variable_independent : public Function_variable_wrapper
//******************************************************************************
// LAST MODIFIED : 12 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_independent(
			const Function_inverse_handle& function_inverse):
			Function_variable_wrapper(function_inverse,
			function_inverse->independent_variable){};
		// destructor
		~Function_variable_independent(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_independent_handle(
				new Function_variable_independent(*this)));
		};
	private:
		// copy constructor
		Function_variable_independent(
			const Function_variable_independent& variable_independent):
			Function_variable_wrapper(variable_independent){};
		// assignment
		Function_variable_independent& operator=(
			const Function_variable_independent&);
};
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)


#if defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)
// forward declaration so that can use _handle
class Function_variable_value_tolerance;
typedef boost::intrusive_ptr<Function_variable_value_tolerance>
	Function_variable_value_tolerance_handle;

// class Function_variable_iterator_representation_atomic_value_tolerance
// ----------------------------------------------------------------------

class Function_variable_iterator_representation_atomic_value_tolerance:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_value_tolerance(
			const bool begin,Function_variable_value_tolerance_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_value_tolerance();
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
		Function_variable_iterator_representation_atomic_value_tolerance(const
			Function_variable_iterator_representation_atomic_value_tolerance&);
	private:
		Function_variable_value_tolerance_handle atomic_variable,variable;
};


// class Function_variable_value_tolerance
// ---------------------------------------

static bool Function_variable_value_tolerance_set_scalar_function(Scalar& value,
	const Function_variable_handle variable);

class Function_variable_value_tolerance : public Function_variable
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_inverse;
	friend class Function_variable_iterator_representation_atomic_value_tolerance;
	friend bool Function_variable_value_tolerance_set_scalar_function(
		Scalar& value,const Function_variable_handle variable);
	public:
		// constructor
		Function_variable_value_tolerance(
			const Function_inverse_handle& function_inverse):
			Function_variable(function_inverse)
		{
			if (function_inverse)
			{
				value_private=Function_variable_value_handle(
					new Function_variable_value_scalar(
					Function_variable_value_tolerance_set_scalar_function));
			}
		};
		// destructor
		~Function_variable_value_tolerance(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_value_tolerance_handle(
				new Function_variable_value_tolerance(*this)));
		};
		string_handle get_string_representation()
		{
			string_handle return_string;
			std::ostringstream out;

			if (return_string=new std::string)
			{
				out << "value_tolerance";
				*return_string=out.str();
			}

			return (return_string);
		};
		virtual Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_value_tolerance(
				true,Function_variable_value_tolerance_handle(
				const_cast<Function_variable_value_tolerance*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_value_tolerance(
				false,Function_variable_value_tolerance_handle(
				const_cast<Function_variable_value_tolerance*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_value_tolerance(
				false,Function_variable_value_tolerance_handle(
				const_cast<Function_variable_value_tolerance*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_value_tolerance(
				true,Function_variable_value_tolerance_handle(
				const_cast<Function_variable_value_tolerance*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			return (1);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_value_tolerance_handle variable_value_tolerance;

			result=false;
			if (variable_value_tolerance=boost::dynamic_pointer_cast<
				Function_variable_value_tolerance,Function_variable>(variable))
			{
				if (variable_value_tolerance->function()==function())
				{
					result=true;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_value_tolerance(
			const Function_variable_value_tolerance& variable_value_tolerance):
			Function_variable(variable_value_tolerance){}
		// assignment
		Function_variable_value_tolerance& operator=(
			const Function_variable_value_tolerance&);
};

static bool Function_variable_value_tolerance_set_scalar_function(Scalar& value,
	const Function_variable_handle variable)
{
	bool result;
	Function_inverse_handle function_inverse;
	Function_variable_value_tolerance_handle value_tolerance_variable;

	result=false;
	if ((value_tolerance_variable=boost::dynamic_pointer_cast<
		Function_variable_value_tolerance,Function_variable>(variable))&&
		(function_inverse=boost::dynamic_pointer_cast<Function_inverse,Function>(
		value_tolerance_variable->function())))
	{
		value=function_inverse->value_tolerance_value();
		result=true;
	}

	return (result);
}


// class Function_variable_iterator_representation_atomic_value_tolerance
// ----------------------------------------------------------------------

Function_variable_iterator_representation_atomic_value_tolerance::
	Function_variable_iterator_representation_atomic_value_tolerance(
	const bool begin,Function_variable_value_tolerance_handle variable):
	Function_variable_iterator_representation(),atomic_variable(0),
	variable(variable)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable&&(variable->function()))
	{
		//???DB.  OK not to clone because not changing
		atomic_variable=variable;
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_value_tolerance::clone()
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new Function_variable_iterator_representation_atomic_value_tolerance(
			*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_value_tolerance::
	~Function_variable_iterator_representation_atomic_value_tolerance()
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_value_tolerance::
	increment()
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		atomic_variable=0;
	}
}

void Function_variable_iterator_representation_atomic_value_tolerance::
	decrement()
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		atomic_variable=0;
	}
	else
	{
		if (variable&&(variable->function()))
		{
			//???DB.  OK not to clone because not changing
			atomic_variable=variable;
		}
	}
}

bool Function_variable_iterator_representation_atomic_value_tolerance::equality(
	const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_value_tolerance
		*representation_value_tolerance=dynamic_cast<const
		Function_variable_iterator_representation_atomic_value_tolerance *>(
		representation);

	result=false;
	if (representation_value_tolerance)
	{
		if (((0==atomic_variable)&&
			(0==representation_value_tolerance->atomic_variable))||
			(atomic_variable&&(representation_value_tolerance->atomic_variable)&&
			(*atomic_variable== *(representation_value_tolerance->atomic_variable))))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_value_tolerance::
	dereference() const
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_value_tolerance::
	Function_variable_iterator_representation_atomic_value_tolerance(const
	Function_variable_iterator_representation_atomic_value_tolerance&
	representation):Function_variable_iterator_representation(),
	atomic_variable(representation.atomic_variable),
	variable(representation.variable)
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_value_tolerance,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}
#else // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)

// class Function_variable_value_tolerance
// ---------------------------------------

// forward declaration so that can use _handle
class Function_variable_value_tolerance;
typedef boost::intrusive_ptr<Function_variable_value_tolerance>
	Function_variable_value_tolerance_handle;

class Function_variable_value_tolerance :
	public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 15 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_value_tolerance(
			const Function_inverse_handle& function_inverse):
			Function_variable_matrix<Scalar>(function_inverse,1,1){};
		// destructor
		~Function_variable_value_tolerance(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_value_tolerance_handle(
				new Function_variable_value_tolerance(*this)));
		};
		string_handle get_string_representation()
		{
			string_handle return_string;
			std::ostringstream out;

			if (return_string=new std::string)
			{
				out << "value_tolerance";
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_variable_matrix_handle operator()(
			Function_size_type row,Function_size_type column)
		{
			Function_variable_matrix_handle result(0);

			if ((1==row)&&(1==column))
			{
				Function_inverse_handle function_inverse=
					boost::dynamic_pointer_cast<Function_inverse,Function>(function());

				result=Function_variable_matrix_handle(
					new Function_variable_value_tolerance(function_inverse));
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
			Function_inverse_handle function_inverse;

			result=false;
			if ((1==row)&&(1==column)&&(function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function())))
			{
				value=function_inverse->value_tolerance_value();
				result=true;
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_value_tolerance(
			const Function_variable_value_tolerance& variable_value_tolerance):
			Function_variable_matrix<Scalar>(variable_value_tolerance){}
};
#endif // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)


#if defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)
// forward declaration so that can use _handle
class Function_variable_step_tolerance;
typedef boost::intrusive_ptr<Function_variable_step_tolerance>
	Function_variable_step_tolerance_handle;

// class Function_variable_iterator_representation_atomic_step_tolerance
// ---------------------------------------------------------------------

class Function_variable_iterator_representation_atomic_step_tolerance:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 18 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_step_tolerance(
			const bool begin,Function_variable_step_tolerance_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_step_tolerance();
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
		Function_variable_iterator_representation_atomic_step_tolerance(const
			Function_variable_iterator_representation_atomic_step_tolerance&);
	private:
		Function_variable_step_tolerance_handle atomic_variable,variable;
};


// class Function_variable_step_tolerance
// --------------------------------------

static bool Function_variable_step_tolerance_set_scalar_function(Scalar& value,
	const Function_variable_handle variable);

class Function_variable_step_tolerance : public Function_variable
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_inverse;
	friend class Function_variable_iterator_representation_atomic_step_tolerance;
	friend bool Function_variable_step_tolerance_set_scalar_function(
		Scalar& value,const Function_variable_handle variable);
	public:
		// constructor
		Function_variable_step_tolerance(
			const Function_inverse_handle& function_inverse):
			Function_variable(function_inverse)
		{
			if (function_inverse)
			{
				value_private=Function_variable_value_handle(
					new Function_variable_value_scalar(
					Function_variable_step_tolerance_set_scalar_function));
			}
		};
		// destructor
		~Function_variable_step_tolerance(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_step_tolerance_handle(
				new Function_variable_step_tolerance(*this)));
		};
		string_handle get_string_representation()
		{
			string_handle return_string;
			std::ostringstream out;

			if (return_string=new std::string)
			{
				out << "step_tolerance";
				*return_string=out.str();
			}

			return (return_string);
		};
		virtual Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_step_tolerance(
				true,Function_variable_step_tolerance_handle(
				const_cast<Function_variable_step_tolerance*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_step_tolerance(
				false,Function_variable_step_tolerance_handle(
				const_cast<Function_variable_step_tolerance*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_step_tolerance(
				false,Function_variable_step_tolerance_handle(
				const_cast<Function_variable_step_tolerance*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_step_tolerance(
				true,Function_variable_step_tolerance_handle(
				const_cast<Function_variable_step_tolerance*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			return (1);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_step_tolerance_handle variable_step_tolerance;

			result=false;
			if (variable_step_tolerance=boost::dynamic_pointer_cast<
				Function_variable_step_tolerance,Function_variable>(variable))
			{
				if (variable_step_tolerance->function()==function())
				{
					result=true;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_step_tolerance(
			const Function_variable_step_tolerance& variable_step_tolerance):
			Function_variable(variable_step_tolerance){}
		// assignment
		Function_variable_step_tolerance& operator=(
			const Function_variable_step_tolerance&);
};

static bool Function_variable_step_tolerance_set_scalar_function(Scalar& value,
	const Function_variable_handle variable)
{
	bool result;
	Function_inverse_handle function_inverse;
	Function_variable_step_tolerance_handle step_tolerance_variable;

	result=false;
	if ((step_tolerance_variable=boost::dynamic_pointer_cast<
		Function_variable_step_tolerance,Function_variable>(variable))&&
		(function_inverse=boost::dynamic_pointer_cast<Function_inverse,Function>(
		step_tolerance_variable->function())))
	{
		value=function_inverse->step_tolerance_value();
		result=true;
	}

	return (result);
}


// class Function_variable_iterator_representation_atomic_step_tolerance
// ---------------------------------------------------------------------

Function_variable_iterator_representation_atomic_step_tolerance::
	Function_variable_iterator_representation_atomic_step_tolerance(
	const bool begin,Function_variable_step_tolerance_handle variable):
	Function_variable_iterator_representation(),atomic_variable(0),
	variable(variable)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable&&(variable->function()))
	{
		//???DB.  OK not to clone because not changing
		atomic_variable=variable;
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_step_tolerance::clone()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new Function_variable_iterator_representation_atomic_step_tolerance(
			*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_step_tolerance::
	~Function_variable_iterator_representation_atomic_step_tolerance()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_step_tolerance::
	increment()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		atomic_variable=0;
	}
}

void Function_variable_iterator_representation_atomic_step_tolerance::
	decrement()
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		atomic_variable=0;
	}
	else
	{
		if (variable&&(variable->function()))
		{
			//???DB.  OK not to clone because not changing
			atomic_variable=variable;
		}
	}
}

bool Function_variable_iterator_representation_atomic_step_tolerance::equality(
	const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_step_tolerance
		*representation_step_tolerance=dynamic_cast<const
		Function_variable_iterator_representation_atomic_step_tolerance *>(
		representation);

	result=false;
	if (representation_step_tolerance)
	{
		if (((0==atomic_variable)&&
			(0==representation_step_tolerance->atomic_variable))||
			(atomic_variable&&(representation_step_tolerance->atomic_variable)&&
			(*atomic_variable== *(representation_step_tolerance->atomic_variable))))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_step_tolerance::
	dereference() const
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_step_tolerance::
	Function_variable_iterator_representation_atomic_step_tolerance(const
	Function_variable_iterator_representation_atomic_step_tolerance&
	representation):Function_variable_iterator_representation(),
	atomic_variable(representation.atomic_variable),
	variable(representation.variable)
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_step_tolerance,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}
#else // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)

// class Function_variable_step_tolerance
// --------------------------------------

// forward declaration so that can use _handle
class Function_variable_step_tolerance;
typedef boost::intrusive_ptr<Function_variable_step_tolerance>
	Function_variable_step_tolerance_handle;

class Function_variable_step_tolerance : public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 15 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_step_tolerance(
			const Function_inverse_handle& function_inverse):
			Function_variable_matrix<Scalar>(function_inverse,1,1){};
		// destructor
		~Function_variable_step_tolerance(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_step_tolerance_handle(
				new Function_variable_step_tolerance(*this)));
		};
		string_handle get_string_representation()
		{
			string_handle return_string;
			std::ostringstream out;

			if (return_string=new std::string)
			{
				out << "step_tolerance";
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_variable_matrix_handle operator()(
			Function_size_type row,Function_size_type column)
		{
			Function_variable_matrix_handle result(0);

			if ((1==row)&&(1==column))
			{
				Function_inverse_handle function_inverse=
					boost::dynamic_pointer_cast<Function_inverse,Function>(function());

				result=Function_variable_matrix_handle(
					new Function_variable_step_tolerance(function_inverse));
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
			Function_inverse_handle function_inverse;

			result=false;
			if ((1==row)&&(1==column)&&(function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function())))
			{
				value=function_inverse->step_tolerance_value();
				result=true;
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_step_tolerance(
			const Function_variable_step_tolerance& variable_step_tolerance):
			Function_variable_matrix<Scalar>(variable_step_tolerance){}
};
#endif // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_ABSTRACT)


#if defined (BEFORE_FUNCTION_VARIABLE_MATRIX_TEMPLATE)
//???DB.  Waiting for Function_variable_matrix to be made into a template
// forward declaration so that can use _handle
class Function_variable_maximum_iterations;
typedef boost::intrusive_ptr<Function_variable_maximum_iterations>
	Function_variable_maximum_iterations_handle;

// class Function_variable_iterator_representation_atomic_maximum_iterations
// -------------------------------------------------------------------------

class Function_variable_iterator_representation_atomic_maximum_iterations:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_maximum_iterations(
			const bool begin,Function_variable_maximum_iterations_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_maximum_iterations();
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
		Function_variable_iterator_representation_atomic_maximum_iterations(const
			Function_variable_iterator_representation_atomic_maximum_iterations&);
	private:
		Function_variable_maximum_iterations_handle atomic_variable,variable;
};


// class Function_variable_maximum_iterations
// ------------------------------------------

static bool
	Function_variable_maximum_iterations_set_function_size_type_function(
	Function_size_type& value,const Function_variable_handle variable);

class Function_variable_maximum_iterations : public Function_variable
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_inverse;
	friend class
		Function_variable_iterator_representation_atomic_maximum_iterations;
	friend bool
		Function_variable_maximum_iterations_set_function_size_type_function(
		Function_size_type& value,const Function_variable_handle variable);
	public:
		// constructor
		Function_variable_maximum_iterations(
			const Function_inverse_handle& function_inverse):
			Function_variable(function_inverse)
		{
			if (function_inverse)
			{
				value_private=Function_variable_value_handle(
					new Function_variable_value_function_size_type(
					Function_variable_maximum_iterations_set_function_size_type_function
					));
			}
		};
		// destructor
		~Function_variable_maximum_iterations(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_maximum_iterations_handle(
				new Function_variable_maximum_iterations(*this)));
		};
		string_handle get_string_representation()
		{
			string_handle return_string;
			std::ostringstream out;

			if (return_string=new std::string)
			{
				out << "maximum_iterations";
				*return_string=out.str();
			}

			return (return_string);
		};
		virtual Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_maximum_iterations(
				true,Function_variable_maximum_iterations_handle(
				const_cast<Function_variable_maximum_iterations*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_maximum_iterations(
				false,Function_variable_maximum_iterations_handle(
				const_cast<Function_variable_maximum_iterations*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_maximum_iterations(
				false,Function_variable_maximum_iterations_handle(
				const_cast<Function_variable_maximum_iterations*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_maximum_iterations(
				true,Function_variable_maximum_iterations_handle(
				const_cast<Function_variable_maximum_iterations*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			return (0);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_maximum_iterations_handle variable_maximum_iterations;

			result=false;
			if (variable_maximum_iterations=boost::dynamic_pointer_cast<
				Function_variable_maximum_iterations,Function_variable>(variable))
			{
				if (variable_maximum_iterations->function()==function())
				{
					result=true;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_maximum_iterations(
			const Function_variable_maximum_iterations& variable_maximum_iterations):
			Function_variable(variable_maximum_iterations){}
		// assignment
		Function_variable_maximum_iterations& operator=(
			const Function_variable_maximum_iterations&);
};

static bool
	Function_variable_maximum_iterations_set_function_size_type_function(
	Function_size_type& value,const Function_variable_handle variable)
{
	bool result;
	Function_inverse_handle function_inverse;
	Function_variable_maximum_iterations_handle maximum_iterations_variable;

	result=false;
	if ((maximum_iterations_variable=boost::dynamic_pointer_cast<
		Function_variable_maximum_iterations,Function_variable>(variable))&&
		(function_inverse=boost::dynamic_pointer_cast<Function_inverse,Function>(
		maximum_iterations_variable->function())))
	{
		value=function_inverse->maximum_iterations_value();
		result=true;
	}

	return (result);
}


// class Function_variable_iterator_representation_atomic_maximum_iterations
// -------------------------------------------------------------------------

Function_variable_iterator_representation_atomic_maximum_iterations::
	Function_variable_iterator_representation_atomic_maximum_iterations(
	const bool begin,Function_variable_maximum_iterations_handle variable):
	Function_variable_iterator_representation(),atomic_variable(0),
	variable(variable)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable&&(variable->function()))
	{
		//???DB.  OK not to clone because not changing
		atomic_variable=variable;
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_maximum_iterations::clone()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=
			new Function_variable_iterator_representation_atomic_maximum_iterations(
			*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_maximum_iterations::
	~Function_variable_iterator_representation_atomic_maximum_iterations()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_maximum_iterations::
	increment()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		atomic_variable=0;
	}
}

void Function_variable_iterator_representation_atomic_maximum_iterations::
	decrement()
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		atomic_variable=0;
	}
	else
	{
		if (variable&&(variable->function()))
		{
			//???DB.  OK not to clone because not changing
			atomic_variable=variable;
		}
	}
}

bool
	Function_variable_iterator_representation_atomic_maximum_iterations::equality(
	const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_maximum_iterations
		*representation_maximum_iterations=dynamic_cast<const
		Function_variable_iterator_representation_atomic_maximum_iterations *>(
		representation);

	result=false;
	if (representation_maximum_iterations)
	{
		if (((0==atomic_variable)&&
			(0==representation_maximum_iterations->atomic_variable))||
			(atomic_variable&&(representation_maximum_iterations->atomic_variable)&&
			(*atomic_variable==
			*(representation_maximum_iterations->atomic_variable))))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_maximum_iterations::
	dereference() const
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_maximum_iterations::
	Function_variable_iterator_representation_atomic_maximum_iterations(const
	Function_variable_iterator_representation_atomic_maximum_iterations&
	representation):Function_variable_iterator_representation(),
	atomic_variable(representation.atomic_variable),
	variable(representation.variable)
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_maximum_iterations,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}
#else // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_TEMPLATE)

// class Function_variable_maximum_iterations
// ------------------------------------------

// forward declaration so that can use _handle
class Function_variable_maximum_iterations;
typedef boost::intrusive_ptr<Function_variable_maximum_iterations>
	Function_variable_maximum_iterations_handle;

class Function_variable_maximum_iterations :
	public Function_variable_matrix<Function_size_type>
//******************************************************************************
// LAST MODIFIED : 18 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_maximum_iterations(
			const Function_inverse_handle& function_inverse):
			Function_variable_matrix<Function_size_type>(function_inverse,1,1){};
		// destructor
		~Function_variable_maximum_iterations(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_maximum_iterations_handle(
				new Function_variable_maximum_iterations(*this)));
		};
		string_handle get_string_representation()
		{
			string_handle return_string;
			std::ostringstream out;

			if (return_string=new std::string)
			{
				out << "maximum_iterations";
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_size_type number_differentiable()
		{
			return (0);
		}
		boost::intrusive_ptr< Function_variable_matrix<Function_size_type> >
			operator()(Function_size_type row,Function_size_type column)
		{
			boost::intrusive_ptr< Function_variable_matrix<Function_size_type> >
				result(0);

			if ((1==row)&&(1==column))
			{
				Function_inverse_handle function_inverse=
					boost::dynamic_pointer_cast<Function_inverse,Function>(function());

				result=
					boost::intrusive_ptr< Function_variable_matrix<Function_size_type> >(
					new Function_variable_maximum_iterations(function_inverse));
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
		bool get_entry(Function_size_type& value) const
		{
			bool result;
			Function_inverse_handle function_inverse;

			result=false;
			if ((1==row)&&(1==column)&&(function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function())))
			{
				value=function_inverse->maximum_iterations_value();
				result=true;
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_maximum_iterations(
			const Function_variable_maximum_iterations& variable_maximum_iterations):
			Function_variable_matrix<Function_size_type>(variable_maximum_iterations)
		{};
};
#endif // defined (BEFORE_FUNCTION_VARIABLE_MATRIX_TEMPLATE)


// global classes
// ==============

// class Function_inverse
// ----------------------

Function_inverse::Function_inverse(
	const Function_variable_handle & dependent_variable,
	Function_variable_handle& independent_variable):Function(),
	maximum_iterations_private(0),dependent_variable(dependent_variable),
	independent_variable(independent_variable),step_tolerance_private(0),
	value_tolerance_private(0)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
}

Function_inverse::~Function_inverse()
//******************************************************************************
// LAST MODIFIED : 17 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

string_handle Function_inverse::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 17 June 2004
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//???DB.  Need get_string_representation for variables to do properly?
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

Function_variable_handle Function_inverse::input()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_independent(this)));
}

Function_variable_handle Function_inverse::output()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_inverse(this,false)
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_dependent(this)
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		));
}

Function_variable_handle Function_inverse::independent()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_independent(this)));
}

Function_variable_handle Function_inverse::value_tolerance()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_value_tolerance(
		this)));
}

Function_variable_handle Function_inverse::step_tolerance()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_step_tolerance(this)));
}

Function_variable_handle Function_inverse::maximum_iterations()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_maximum_iterations(this)));
}

Function_variable_handle Function_inverse::dependent_estimate()
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_inverse(this,true)
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_dependent_estimate(this)
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		));
}

Scalar Function_inverse::step_tolerance_value()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (step_tolerance_private);
}

Scalar Function_inverse::value_tolerance_value()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (value_tolerance_private);
}

Function_size_type Function_inverse::maximum_iterations_value()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (maximum_iterations_private);
}

Function_handle Function_inverse::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_inverse_handle atomic_variable_inverse;

	if ((atomic_variable_inverse=boost::dynamic_pointer_cast<
		Function_variable_inverse,Function_variable>(atomic_variable))&&
		!(atomic_variable_inverse->dependent_estimate)&&
		(Function_handle(this)==atomic_variable_inverse->function()))
	{
		result=(atomic_variable_inverse->evaluate)();
	}
	else
	{
		result=get_value(atomic_variable);
	}
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_dependent_handle atomic_variable_dependent;
	Function_variable_dependent_estimate_handle
		atomic_variable_dependent_estimate;

	if ((atomic_variable_dependent=boost::dynamic_pointer_cast<
		Function_variable_dependent,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_dependent->function()))
	{
		result=(atomic_variable_dependent->evaluate)();
	}
	if ((atomic_variable_dependent_estimate=boost::dynamic_pointer_cast<
		Function_variable_dependent_estimate,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_dependent_estimate->function()))
	{
		result=get_value(atomic_variable);
	}
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)

	return (result);
}

bool Function_inverse::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_independent_handle atomic_variable_independent;
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_inverse_handle atomic_variable_inverse;
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_dependent_handle atomic_variable_dependent;
	Function_variable_dependent_estimate_handle
		atomic_variable_dependent_estimate;
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_step_tolerance_handle atomic_variable_step_tolerance;
	Function_variable_value_tolerance_handle atomic_variable_value_tolerance;

	result=false;
	if ((atomic_variable_independent=boost::dynamic_pointer_cast<
		Function_variable_independent,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_independent->function()))
	{
		Function_variable_independent_handle atomic_independent_variable;

		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_independent,Function_variable>(
			atomic_independent_variables.front()))&&
			(*atomic_variable_independent== *atomic_independent_variable))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	else if ((atomic_variable_inverse=boost::dynamic_pointer_cast<
		Function_variable_inverse,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_inverse->function()))
	{
		if (atomic_variable_inverse->dependent_estimate)
		{
			Function_variable_inverse_handle atomic_independent_variable;

			result=true;
			if ((1==atomic_independent_variables.size())&&
				(atomic_independent_variable=boost::dynamic_pointer_cast<
				Function_variable_inverse,Function_variable>(
				atomic_independent_variables.front()))&&
				(*atomic_variable_inverse== *atomic_independent_variable))
			{
				derivative=1;
			}
			else
			{
				derivative=0;
			}
		}
		else
		{
			Function_matrix_handle derivative_matrix=boost::dynamic_pointer_cast<
				Function_matrix,Function>((atomic_variable_inverse->
				evaluate_derivative)(atomic_independent_variables));

			if (derivative_matrix)
			{
				derivative=(*derivative_matrix)(0,0);
				result=true;
			}
		}
	}
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	else if ((atomic_variable_dependent_estimate=boost::dynamic_pointer_cast<
		Function_variable_dependent_estimate,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_dependent_estimate->function()))
	{
		Function_variable_dependent_handle atomic_independent_variable;

		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_dependent,Function_variable>(
			atomic_independent_variables.front()))&&
			(*atomic_variable_dependent== *atomic_independent_variable))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}
	else if ((atomic_variable_dependent=boost::dynamic_pointer_cast<
		Function_variable_dependent,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_dependent->function()))
	{
		Function_matrix_handle derivative_matrix=boost::dynamic_pointer_cast<
			Function_matrix,Function>((atomic_variable_dependent->
			evaluate_derivative)(atomic_independent_variables));

		if (derivative_matrix)
		{
			derivative=(*derivative_matrix)(0,0);
			result=true;
		}
	}
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	else if ((atomic_variable_step_tolerance=boost::dynamic_pointer_cast<
		Function_variable_step_tolerance,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_step_tolerance->function()))
	{
		Function_variable_step_tolerance_handle atomic_independent_variable;

		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_step_tolerance,Function_variable>(
			atomic_independent_variables.front()))&&
			(*atomic_variable_step_tolerance== *atomic_independent_variable))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}
	else if ((atomic_variable_value_tolerance=boost::dynamic_pointer_cast<
		Function_variable_value_tolerance,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_value_tolerance->function()))
	{
		Function_variable_value_tolerance_handle atomic_independent_variable;

		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_value_tolerance,Function_variable>(
			atomic_independent_variables.front()))&&
			(*atomic_variable_value_tolerance== *atomic_independent_variable))
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

bool Function_inverse::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_independent_handle atomic_variable_independent;
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_inverse_handle atomic_variable_inverse;
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_dependent_handle atomic_variable_dependent;
	Function_variable_dependent_estimate_handle
		atomic_variable_dependent_estimate;
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_maximum_iterations_handle
		atomic_variable_maximum_iterations;
	Function_variable_step_tolerance_handle atomic_variable_step_tolerance;
	Function_variable_value_tolerance_handle atomic_variable_value_tolerance;

	result=false;
	if ((atomic_variable_independent=boost::dynamic_pointer_cast<
		Function_variable_independent,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_independent->function()))
	{
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		if (atomic_variable_independent->atomic_independent_variable)
		{
			result=(atomic_variable_independent->atomic_independent_variable->
				set_value)(atomic_value->get_value());
		}
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		if (atomic_variable_independent->get_wrapped())
		{
			result=((atomic_variable_independent->get_wrapped())->set_value)(
				atomic_value->get_value());
		}
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	}
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	else if ((atomic_variable_inverse=boost::dynamic_pointer_cast<
		Function_variable_inverse,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_inverse->function()))
	{
		if (atomic_variable_inverse->atomic_dependent_variable)
		{
			result=(atomic_variable_inverse->atomic_dependent_variable->set_value)(
				atomic_value->get_value());
		}
	}
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	else if ((atomic_variable_dependent=boost::dynamic_pointer_cast<
		Function_variable_dependent,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_dependent->function()))
	{
		Function_variable_handle wrapped_variable;

		if (wrapped_variable=atomic_variable_dependent->get_wrapped())
		{
			result=(wrapped_variable->set_value)(atomic_value->get_value());
		}
	}
	else if ((atomic_variable_dependent_estimate=boost::dynamic_pointer_cast<
		Function_variable_dependent_estimate,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_dependent_estimate->function()))
	{
		Function_variable_handle wrapped_variable;

		if (wrapped_variable=atomic_variable_dependent_estimate->get_wrapped())
		{
			result=(wrapped_variable->set_value)(atomic_value->get_value());
		}
	}
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	else if ((atomic_variable_maximum_iterations=boost::dynamic_pointer_cast<
		Function_variable_maximum_iterations,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_maximum_iterations->function()))
	{
		Function_variable_value_function_size_type_handle value_function_size_type;

		if ((std::string("Function_size_type")==(atomic_value->value())->type())&&
			(value_function_size_type=boost::dynamic_pointer_cast<
			Function_variable_value_function_size_type,Function_variable_value>(
			atomic_value->value())))
		{
			result=value_function_size_type->set(maximum_iterations_private,
				atomic_value);
		}
	}
	else if ((atomic_variable_step_tolerance=boost::dynamic_pointer_cast<
		Function_variable_step_tolerance,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_step_tolerance->function()))
	{
		Function_variable_value_scalar_handle value_scalar;

		if ((std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			result=value_scalar->set(step_tolerance_private,atomic_value);
		}
	}
	else if ((atomic_variable_value_tolerance=boost::dynamic_pointer_cast<
		Function_variable_value_tolerance,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_value_tolerance->function()))
	{
		Function_variable_value_scalar_handle value_scalar;

		if ((std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			result=value_scalar->set(value_tolerance_private,atomic_value);
		}
	}

	return (result);
}

Function_handle Function_inverse::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 1 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_independent_handle atomic_variable_independent;
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_inverse_handle atomic_variable_inverse;
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_dependent_handle atomic_variable_dependent;
	Function_variable_dependent_estimate_handle
		atomic_variable_dependent_estimate;
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_maximum_iterations_handle
		atomic_variable_maximum_iterations;
	Function_variable_step_tolerance_handle atomic_variable_step_tolerance;
	Function_variable_value_tolerance_handle atomic_variable_value_tolerance;

	if ((atomic_variable_independent=boost::dynamic_pointer_cast<
		Function_variable_independent,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_independent->function()))
	{
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		if (atomic_variable_independent->atomic_independent_variable)
		{
			result=(atomic_variable_independent->atomic_independent_variable->
				get_value)();
		}
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		if (atomic_variable_independent->get_wrapped())
		{
			result=((atomic_variable_independent->get_wrapped())->get_value)();
		}
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	}
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	else if ((atomic_variable_inverse=boost::dynamic_pointer_cast<
		Function_variable_inverse,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_inverse->function()))
	{
		if (atomic_variable_inverse->atomic_dependent_variable)
		{
			result=(atomic_variable_inverse->atomic_dependent_variable->get_value)();
		}
	}
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	else if ((atomic_variable_dependent=boost::dynamic_pointer_cast<
		Function_variable_dependent,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_dependent->function()))
	{
		Function_variable_handle wrapped_variable;

		if (wrapped_variable=atomic_variable_dependent->get_wrapped())
		{
			result=(wrapped_variable->get_value)();
		}
	}
	else if ((atomic_variable_dependent_estimate=boost::dynamic_pointer_cast<
		Function_variable_dependent_estimate,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_dependent_estimate->function()))
	{
		Function_variable_handle wrapped_variable;

		if (wrapped_variable=atomic_variable_dependent_estimate->get_wrapped())
		{
			result=(wrapped_variable->get_value)();
		}
	}
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	else if ((atomic_variable_maximum_iterations=boost::dynamic_pointer_cast<
		Function_variable_maximum_iterations,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_maximum_iterations->function()))
	{
		result=Function_handle(new Function_function_size_type(
			maximum_iterations_private));
	}
	else if ((atomic_variable_step_tolerance=boost::dynamic_pointer_cast<
		Function_variable_step_tolerance,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_step_tolerance->function()))
	{
		Matrix result_matrix(1,1);

		result_matrix(0,0)=step_tolerance_private;
		result=Function_handle(new Function_matrix(result_matrix));
	}
	else if ((atomic_variable_value_tolerance=boost::dynamic_pointer_cast<
		Function_variable_value_tolerance,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_value_tolerance->function()))
	{
		Matrix result_matrix(1,1);

		result_matrix(0,0)=value_tolerance_private;
		result=Function_handle(new Function_matrix(result_matrix));
	}

	return (result);
}

Function_inverse::Function_inverse(const Function_inverse& function_inverse):
	Function(function_inverse),
	maximum_iterations_private(function_inverse.maximum_iterations_private),
	dependent_variable(function_inverse.dependent_variable),
	independent_variable(function_inverse.independent_variable),
	step_tolerance_private(function_inverse.step_tolerance_private),
	value_tolerance_private(function_inverse.value_tolerance_private)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_inverse& Function_inverse::operator=(
	const Function_inverse& function_inverse)
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	step_tolerance_private=function_inverse.step_tolerance_private;
	value_tolerance_private=function_inverse.value_tolerance_private;
	dependent_variable=function_inverse.dependent_variable;
	independent_variable=function_inverse.independent_variable;
	maximum_iterations_private=function_inverse.maximum_iterations_private;

	return (*this);
}

//???DB.  Where I'm up to
//???DB.  Set up value_private for inputs/outputs
//???DB.  Check evaluate
//???DB.  Check constructors
