//******************************************************************************
// FILE : function_inverse.cpp
//
// LAST MODIFIED : 28 April 2005
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_derivative_matrix.hpp"
#include "computed_variable/function_function_size_type.hpp"
#include "computed_variable/function_identity.hpp"
#include "computed_variable/function_inverse.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_wrapper.hpp"
#include "computed_variable/function_variable_composite.hpp"
#include "computed_variable/function_variable_intersection.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_value_function_size_type.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

// module typedefs
// ===============

typedef boost::intrusive_ptr< Function_matrix<Scalar> >
	Function_matrix_scalar_handle;
typedef boost::intrusive_ptr< Function_variable_matrix<Scalar> >
	Function_variable_matrix_scalar_handle;
typedef boost::intrusive_ptr< Function_variable_matrix<Function_size_type> >
	Function_variable_matrix_function_size_type_handle;

// module classes
// ==============

// class Function_variable_independent
// -----------------------------------

// forward declaration so that can use _handle
class Function_variable_independent;
typedef boost::intrusive_ptr<Function_variable_independent>
	Function_variable_independent_handle;

class Function_variable_independent : public Function_variable_wrapper
//******************************************************************************
// LAST MODIFIED : 28 April 2005
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
			Function_variable_independent_handle result;

			result=new Function_variable_independent(*this);
			if (working_variable)
			{
				result->working_variable=working_variable->clone();
			}
			else
			{
				result->working_variable=0;
			}

			return (result);
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

				out << "independent(";
				if (working_variable)
				{
					out << *(working_variable->get_string_representation());
				}
				out << ")";
				*return_string=out.str();
			}

			return (return_string);
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

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_inverse
// ---------------------------------

class Function_derivatnew_inverse : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 23 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_inverse(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_inverse();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
	private:
		Function_derivatnew_handle extended_derivative_inverse;
};
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

// class Function_variable_dependent
// ---------------------------------

// forward declaration so that can use _handle
class Function_variable_dependent;
typedef boost::intrusive_ptr<Function_variable_dependent>
	Function_variable_dependent_handle;

class Function_variable_dependent : public Function_variable_wrapper
//******************************************************************************
// LAST MODIFIED : 18 April 2005
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
			Function_variable_dependent_handle result;

			result=new Function_variable_dependent(*this);
			if (working_variable)
			{
				result->working_variable=working_variable->clone();
			}
			else
			{
				result->working_variable=0;
			}

			return (result);
		};
		// overload Function_variable::evaluate
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate()
		{
			Function_handle result;
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function());
			Function_variable_handle specified_dependent_variable=get_wrapped();

			result=0;
			if (function_inverse&&specified_dependent_variable)
			{
#if defined (BEFORE_CACHING)
				Function_variable_handle dependent_variable,
					independent_value_variable,independent_variable;
				Function_handle dependent_value_estimate,independent_value;

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
						Function_matrix_scalar_handle derivative_matrix=
							boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
							independent_variable->evaluate_derivative(
							independent_variables));

						if (derivative_matrix)
						{
							Function_matrix_scalar_handle increment_function=
								derivative_matrix->solve(boost::dynamic_pointer_cast<
								Function_matrix<Scalar>,Function>(error->function()));
							Function_variable_handle increment;

							if (valid=(increment_function&&
								(increment=increment_function->output())&&
								((*dependent_variable) += (*increment))))
							{
								++iteration_number;
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
						if (equivalent(specified_dependent_variable,dependent_variable))
						{
							result=dependent_variable->get_value();
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
								!equivalent(*dependent_variable_iterator,
								specified_dependent_variable))
							{
								++dependent_variable_iterator;
								++dependent_value_iterator;
							}
							if ((dependent_variable_iterator!=
								dependent_variable_iterator_end)&&
								(dependent_value_iterator!=dependent_value_iterator_end))
							{
								result=(*dependent_value_iterator)->get_value();
							}
						}
					}
					// reset required value
					independent_variable->set_value(independent_value);
				}
#else // defined (BEFORE_CACHING)
				if (!(function_inverse->evaluated()))
				{
					Function_variable_handle dependent_variable,
						independent_value_variable,independent_variable;
					Function_handle independent_value;

					if ((dependent_variable=function_inverse->dependent_variable)&&
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
							Function_matrix_scalar_handle derivative_matrix;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
							Function_derivatnew_handle temp_function;
							Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

							if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
								derivative_matrix=boost::dynamic_pointer_cast<Function_matrix<
								Scalar>,Function>(independent_variable->evaluate_derivative(
								independent_variables))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
								(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
								Function>(independent_variable->derivative(
								independent_variables)))&&
								(temp_variable=temp_function->output())&&
								(temp_variable->evaluate())&&
								(temp_variable=temp_function->matrix(independent_variables))&&
								(derivative_matrix=boost::dynamic_pointer_cast<Function_matrix<
								Scalar>,Function>(temp_variable->get_value()))
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
								)
							{
								Function_matrix_scalar_handle increment_function=
									derivative_matrix->solve(boost::dynamic_pointer_cast<
									Function_matrix<Scalar>,Function>(error->function()));
								Function_variable_handle increment;

								if (valid=(increment_function&&
									(increment=increment_function->output())&&
									((*dependent_variable) += (*increment))))
								{
									++iteration_number;
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
						// reset required value
						independent_variable->set_value(independent_value);
						if (valid)
						{
							function_inverse->set_evaluated();
						}
					}
				}
				if (function_inverse->evaluated())
				{
					Function_variable_handle dependent_variable;

					if (dependent_variable=function_inverse->dependent_variable)
					{
						if (equivalent(specified_dependent_variable,dependent_variable))
						{
							result=dependent_variable->get_value();
						}
						else
						{
							Function_variable_iterator dependent_variable_iterator,
								dependent_variable_iterator_end;
							
							dependent_variable_iterator=dependent_variable->begin_atomic();
							dependent_variable_iterator_end=
								dependent_variable->end_atomic();
							while ((dependent_variable_iterator!=
								dependent_variable_iterator_end)&&
								!equivalent(*dependent_variable_iterator,
								specified_dependent_variable))
							{
								++dependent_variable_iterator;
							}
							if (dependent_variable_iterator!=
								dependent_variable_iterator_end)
							{
								result=(*dependent_variable_iterator)->get_value();
							}
						}
					}
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		};
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate()
		{
			bool result(true);
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function());
			Function_variable_handle specified_dependent_variable=get_wrapped();

			if (function_inverse&&specified_dependent_variable)
			{
#if defined (BEFORE_CACHING)
				Function_variable_handle dependent_variable,
					independent_value_variable,independent_variable;
				Function_handle dependent_value_estimate,independent_value;

				if ((dependent_variable=function_inverse->dependent_variable)&&
					(dependent_value_estimate=dependent_variable->get_value())&&
					(independent_variable=function_inverse->independent_variable)&&
					(independent_value=independent_variable->get_value())&&
					(independent_value_variable=independent_value->output()))
				{
					Scalar error_norm,increment_norm,step_tolerance,value_tolerance;
					Function_variable_handle error;
					Function_size_type iteration_number,maximum_iterations;
					std::list<Function_variable_handle> independent_variables(0);

					step_tolerance=function_inverse->step_tolerance_private;
					value_tolerance=function_inverse->value_tolerance_private;
					maximum_iterations=function_inverse->maximum_iterations_private;
					independent_variables.push_back(dependent_variable);
					result=((independent_variable->evaluate)()&&
						(independent_variable->set_value)(
						(independent_variable->get_value)())&&
						(error=(*independent_value_variable)-(*independent_variable))&&
						(0<=(error_norm=error->norm())));
					iteration_number=0;
					increment_norm=1+step_tolerance;
					while (result&&
						!(((0>=value_tolerance)&&(0>=step_tolerance))||
						(error_norm<value_tolerance)||(increment_norm<step_tolerance))&&
						(iteration_number<maximum_iterations))
					{
						Function_matrix_scalar_handle derivative_matrix=
							boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
							independent_variable->evaluate_derivative(
							independent_variables));

						if (derivative_matrix)
						{
							Function_matrix_scalar_handle increment_function=
								derivative_matrix->solve(boost::dynamic_pointer_cast<
								Function_matrix<Scalar>,Function>(error->function()));
							Function_variable_handle increment;

							if (result=(increment_function&&
								(increment=increment_function->output())&&
								((*dependent_variable) += (*increment))))
							{
								++iteration_number;
								result=((independent_variable->evaluate)()&&
									(independent_variable->set_value)(
									(independent_variable->get_value)())&&
									(error=(*independent_value_variable)-
									(*independent_variable))&&
									(0<=(error_norm=error->norm()))&&
									(0<=(increment_norm=increment->norm())));
							}
						}
						else
						{
							result=false;
						}
					}
					// reset required value
					independent_variable->set_value(independent_value);
				}
#else // defined (BEFORE_CACHING)
				if (!(function_inverse->evaluated()))
				{
					Function_variable_handle dependent_variable,
						independent_value_variable,independent_variable;
					Function_handle independent_value;

					if ((dependent_variable=function_inverse->dependent_variable)&&
						(independent_variable=function_inverse->independent_variable)&&
						(independent_value=independent_variable->get_value())&&
						(independent_value_variable=independent_value->output()))
					{
						Scalar error_norm,increment_norm,step_tolerance,value_tolerance;
						Function_variable_handle error;
						Function_size_type iteration_number,maximum_iterations;
						std::list<Function_variable_handle> independent_variables(0);

						step_tolerance=function_inverse->step_tolerance_private;
						value_tolerance=function_inverse->value_tolerance_private;
						maximum_iterations=function_inverse->maximum_iterations_private;
						independent_variables.push_back(dependent_variable);
						result=((independent_variable->evaluate)()&&
							(independent_variable->set_value)(
							(independent_variable->get_value)())&&
							(error=(*independent_value_variable)-(*independent_variable))&&
							(0<=(error_norm=error->norm())));
						iteration_number=0;
						increment_norm=1+step_tolerance;
						while (result&&
							!(((0>=value_tolerance)&&(0>=step_tolerance))||
							(error_norm<value_tolerance)||(increment_norm<step_tolerance))&&
							(iteration_number<maximum_iterations))
						{
							Function_matrix_scalar_handle derivative_matrix;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
							Function_derivatnew_handle temp_function;
							Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

							if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
								derivative_matrix=boost::dynamic_pointer_cast<Function_matrix<
								Scalar>,Function>(independent_variable->evaluate_derivative(
								independent_variables))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
								(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
								Function>(independent_variable->derivative(
								independent_variables)))&&
								(temp_variable=temp_function->output())&&
								(temp_variable->evaluate())&&
								(temp_variable=temp_function->matrix(independent_variables))&&
								(derivative_matrix=boost::dynamic_pointer_cast<Function_matrix<
								Scalar>,Function>(temp_variable->get_value()))
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
								)
							{
								Function_matrix_scalar_handle increment_function=
									derivative_matrix->solve(boost::dynamic_pointer_cast<
									Function_matrix<Scalar>,Function>(error->function()));
								Function_variable_handle increment;

								if (result=(increment_function&&
									(increment=increment_function->output())&&
									((*dependent_variable) += (*increment))))
								{
									++iteration_number;
									result=((independent_variable->evaluate)()&&
										(independent_variable->set_value)(
										(independent_variable->get_value)())&&
										(error=(*independent_value_variable)-
										(*independent_variable))&&
										(0<=(error_norm=error->norm()))&&
										(0<=(increment_norm=increment->norm())));
								}
							}
							else
							{
								result=false;
							}
						}
						// reset required value
						independent_variable->set_value(independent_value);
						if (result)
						{
							function_inverse->set_evaluated();
						}
					}
				}
#endif // defined (BEFORE_CACHING)
			}

			return (result);
		};
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		// overload Function_variable::evaluate_derivative
		Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables)
		{
			Function_handle result(0);
			Function_inverse_handle function_inverse=boost::dynamic_pointer_cast<
				Function_inverse,Function>(function());
			Function_variable_handle specified_dependent_variable=get_wrapped();

			if (function_inverse&&specified_dependent_variable)
			{
				const Function_size_type number_of_independent_variables=
					independent_variables.size();
				Function_variable_handle dependent_variable,independent_variable;

				dependent_variable=function_inverse->dependent_variable;
				independent_variable=function_inverse->independent_variable;
				if (dependent_variable&&independent_variable&&
					(0<number_of_independent_variables))
				{
//#define OLD_CODE2
#if defined (OLD_CODE2)
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
					for (i=0;i<number_of_independent_variables;++i)
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
						++independent_variables_iterator;
					}
					if (0<maximum_number_of_independent_values)
					{
						Function_handle dependent_value;

						// make sure that dependent is consistent with independent
						dependent_value=dependent_variable->get_value();
						if (dependent_variable->set_value((function_inverse->output())->
							evaluate()))
						{
							try
							{
								Function_variable_handle extended_dependent_variable(
									new Function_variable_union(extended_dependent_list));
								Function_variable_handle extended_independent_variable(
									new Function_variable_union(extended_independent_list));
								std::list<Function_variable_handle>
									extended_dependent_variables(number_of_independent_variables,
									extended_dependent_variable);
								Function_derivative_matrix_handle derivative_inverse(
									new Function_derivative_matrix(extended_independent_variable,
									extended_dependent_variables));

								//???debug
								std::cout << "extended_dependent_variable=" << *(extended_dependent_variable->get_string_representation()) << std::endl;
								//???debug
								std::cout << "extended_independent_variable=" << *(extended_independent_variable->get_string_representation()) << std::endl;
								//???debug
								std::cout << "before inverse matrix=" << (derivative_inverse->matrices).back() << std::endl;
								if (derivative_inverse&&(derivative_inverse=
									boost::dynamic_pointer_cast<Function_derivative_matrix,
									Function>((derivative_inverse->inverse)())))
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

									for (i=0;i<number_of_matrices;++i)
									{
										for (j=0;j<number_of_independent_variables;++j)
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
														++j;
													}
													++extended_variable_iterator;
												}
												dependent_value_mapping[i]=j;
												++i;
											}
											++variable_iterator;
										}
										matrix_iterator=(derivative_inverse->matrices).begin();
										number_of_matrices=0;
										independent_variables_iterator=
											independent_variables.begin();
										for (i=0;i<number_of_independent_variables;++i)
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
														(**extended_variable_iterator!=
														**variable_iterator))
													{
														if (1==(*extended_variable_iterator)->
															number_differentiable())
														{
															++k;
														}
														++extended_variable_iterator;
													}
													independent_value_mapping[i,j]=k;
													++j;
												}
												++variable_iterator;
											}
											for (column=0;column<number_of_columns;++column)
											{
												column_1=independent_value_mapping[column];
												for (row=0;row<number_of_dependent_values;++row)
												{
													matrix_1(row,column)=(*matrix_iterator)(
														dependent_value_mapping[row],column_1);
												}
											}
											matrix_variables(number_of_matrices,i)=true;
											matrices.push_back(matrix_1);
											++matrix_iterator;
											for (j=0;j<number_of_matrices;++j)
											{
												number_of_columns=numbers_of_independent_values[i];
												for (k=0;k<i;++k)
												{
													if (matrix_variables(j,k))
													{
														matrix_variables(j+number_of_matrices+1,k)=true;
														number_of_columns *=
															numbers_of_independent_values[k];
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
													for (k=0;k<=i;++k)
													{
														if (matrix_variables(j+number_of_matrices+1,k))
														{
															column_2 *= number_of_extended_independent_values;
															column_2 += independent_value_mapping(k,
																derivative_index[k]);
														}
													}
													for (row=0;row<number_of_dependent_values;++row)
													{
														matrix_2(row,column)=(*matrix_iterator)(
															dependent_value_mapping[row],column_2);
													}
													++column;
													// increment derivative_index
													finished=true;
													k=i;
													while (finished&&(k>0))
													{
														if (matrix_variables(j+number_of_matrices+1,k))
														{
															++derivative_index[k];
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
														--k;
													}
													if (finished&&
														matrix_variables(j+number_of_matrices+1,0))
													{
														++derivative_index[0];
														finished=(derivative_index[0]==
															numbers_of_independent_values[0]);
													}
												}
												matrices.push_back(matrix_2);
												++matrix_iterator;
											}
											number_of_matrices += number_of_matrices+1;
											++independent_variables_iterator;
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

										variable_iterator=
											specified_dependent_variable->begin_atomic();
										i=0;
										while (i<number_of_dependent_values)
										{
											if (1==(*variable_iterator)->number_differentiable())
											{
												j=0;
												extended_variable_iterator=extended_dependent_variable->
													begin_atomic();
												while ((j<number_of_extended_dependent_values)&&
													!equivalent(*extended_variable_iterator,
													*variable_iterator))
												{
													if (1==(*extended_variable_iterator)->
														number_differentiable())
													{
														++j;
													}
													++extended_variable_iterator;
												}
												dependent_value_mapping[i]=j;
												++i;
											}
											++variable_iterator;
										}
										number_of_columns=1;
										independent_variables_iterator=
											independent_variables.begin();
										for (i=0;i<number_of_independent_variables;++i)
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
#if !defined (OLD_CODE)
												bool found;
#endif // !defined (OLD_CODE)
												Function_variable_handle
#if !defined (OLD_CODE)
													temp_extended_independent_variable,
#endif // !defined (OLD_CODE)
													temp_independent_variable;
												Function_variable_independent_handle temp_variable;
#if !defined (OLD_CODE)
												Function_variable_wrapper_handle temp_wrapper_variable;
#endif // !defined (OLD_CODE)

												temp_independent_variable= *variable_iterator;
												if ((temp_variable=
													boost::dynamic_pointer_cast<
													Function_variable_independent,Function_variable>(
													temp_independent_variable))&&(equivalent(
													temp_variable->function(),
													function_inverse)))
												{
													temp_independent_variable=
														temp_variable->get_wrapped();
												}
												if (1==temp_independent_variable->
													number_differentiable())
												{
													k=0;
													extended_variable_iterator=
														extended_independent_variable->begin_atomic();
#if defined (OLD_CODE)
													while ((k<number_of_extended_independent_values)&&
														!equivalent(*extended_variable_iterator,
														temp_independent_variable))
													{
														if (1==(*extended_variable_iterator)->
															number_differentiable())
														{
															++k;
														}
														++extended_variable_iterator;
													}
#else // defined (OLD_CODE)
													found=false;
													while ((k<number_of_extended_independent_values)&&
														!found)
													{
														temp_extended_independent_variable=
															*extended_variable_iterator;
														if ((boost::dynamic_pointer_cast<Function_identity,
															Function>(temp_extended_independent_variable->
															function()))&&(temp_wrapper_variable=
															boost::dynamic_pointer_cast<
															Function_variable_wrapper,Function_variable>(
															temp_extended_independent_variable)))
														{
															temp_extended_independent_variable=
																temp_wrapper_variable->get_wrapped();
														}
														if (equivalent(temp_extended_independent_variable,
															temp_independent_variable))
														{
															found=true;
														}
														else
														{
															if (1==temp_extended_independent_variable->
																number_differentiable())
															{
																++k;
															}
															++extended_variable_iterator;
														}
													}
#endif // defined (OLD_CODE)
													independent_value_mapping(i,j)=k;
													++j;
												}
												++variable_iterator;
											}
											++independent_variables_iterator;
										}
										if (0<number_of_columns)
										{
											bool finished;
											Matrix matrix(number_of_dependent_values,
												number_of_columns);
											Matrix
												&matrix_extended=(derivative_inverse->matrices).back();
											Function_size_type column,column_extended,row;

											//???debug
											std::cout << "dependent_value_mapping=";
											for (i=0;i<number_of_dependent_values;++i)
											{
												std::cout << " " << dependent_value_mapping[i];
											}
											std::cout << std::endl;
											//???debug
											std::cout << "independent_value_mapping=" << independent_value_mapping << std::endl;
											column=0;
											finished=false;
											while (!finished)
											{
												column_extended=0;
												for (i=0;i<number_of_independent_variables;++i)
												{
													column_extended *=
														number_of_extended_independent_values;
													column_extended +=
														independent_value_mapping(i,derivative_index[i]);
												}
												for (row=0;row<number_of_dependent_values;++row)
												{
													matrix(row,column)=matrix_extended(
														dependent_value_mapping[row],column_extended);
												}
												++column;
												// increment derivative_index
												finished=true;
												i=number_of_independent_variables-1;
												while (finished&&(i>0))
												{
													++derivative_index[i];
													if (derivative_index[i]==
														numbers_of_independent_values[i])
													{
														derivative_index[i]=0;
													}
													else
													{
														finished=false;
													}
													--i;
												}
												if (finished)
												{
													++derivative_index[0];
													finished=(derivative_index[0]==
														numbers_of_independent_values[0]);
												}
											}
											result=Function_matrix_scalar_handle(
												new Function_matrix<Scalar>(matrix));
											//???debug
											std::cout << "matrix=" << matrix << std::endl;
											//???debug
											std::cout << "matrix_extended=" << matrix_extended << std::endl;
										}
									}
								}
							}
							catch (Function_derivative_matrix::Construction_exception)
							{
								// do nothing
								//???debug
								std::cout << "Function_variable_dependent::evaluate_derivative.  Failed" << std::endl;
							}
							// reset dependent variable value
							dependent_variable->set_value(dependent_value);
						}
					}
#else // defined (OLD_CODE2)
					Function_size_type i,maximum_number_of_independent_values,
						number_of_columns,number_of_independent_values,number_of_rows;
					std::list<Function_variable_handle>::iterator
						independent_variables_iterator;
					std::vector<Function_size_type>
						numbers_of_independent_values(number_of_independent_variables);

					number_of_rows=number_differentiable();
					independent_variables_iterator=independent_variables.begin();
					maximum_number_of_independent_values=0;
					number_of_columns=1;
					for (i=0;i<number_of_independent_variables;++i)
					{
						number_of_independent_values=
							(*independent_variables_iterator)->number_differentiable();
						numbers_of_independent_values[i]=number_of_independent_values;
						if (maximum_number_of_independent_values<
							number_of_independent_values)
						{
							maximum_number_of_independent_values=number_of_independent_values;
						}
						number_of_columns *= number_of_independent_values;
						++independent_variables_iterator;
					}
					if ((0<number_of_columns)&&(0<number_of_rows)&&
						(0<maximum_number_of_independent_values))
					{
#if defined (OLD_CODE3)
						bool valid;
						Function_size_type j,k,number_of_extended_dependent_values,
							number_of_extended_independent_values;
						Function_variable_handle temp_extended_independent_variable,
							temp_independent_variable;
						Function_variable_iterator extended_variable_iterator,
							variable_iterator;
						Function_variable_wrapper_handle temp_wrapper_variable;
						std::list<Function_variable_handle> extended_dependent_list(0),
							extended_independent_list(0);

						// set up extended dependent and independent variables
						extended_independent_list.push_back(independent_variable);
						extended_dependent_list.push_back(dependent_variable);
						number_of_extended_dependent_values=
							dependent_variable->number_differentiable();
						number_of_extended_independent_values=
							independent_variable->number_differentiable();
						independent_variables_iterator=independent_variables.begin();
						i=0;
						valid=true;
						while (valid&&(i<number_of_independent_variables))
						{
							Function_variable_handle extended_dependent_variable(
								new Function_variable_union(extended_dependent_list));
							Function_variable_intersection_handle intersection(
								new Function_variable_intersection(
								extended_dependent_variable,temp_independent_variable=
								*independent_variables_iterator));

							number_of_independent_values=numbers_of_independent_values[i];
							if (intersection&&(0==intersection->number_differentiable()))
							{
								extended_dependent_list.push_back(
									temp_independent_variable);
								extended_independent_list.push_back(Function_handle(
									new Function_identity(temp_independent_variable))->
									output());
								number_of_extended_dependent_values +=
									number_of_independent_values;
								number_of_extended_independent_values +=
									number_of_independent_values;
							}
							else
							{
								variable_iterator=temp_independent_variable->begin_atomic();
								j=0;
								while (j<number_of_independent_values)
								{
									temp_independent_variable= *variable_iterator;
									if (1==temp_independent_variable->number_differentiable())
									{
										extended_dependent_variable=
											new Function_variable_union(extended_dependent_list);
										k=0;
										extended_variable_iterator=
											extended_dependent_variable->begin_atomic();
										while ((k<number_of_extended_dependent_values)&&
											!equivalent(*extended_variable_iterator,
											temp_independent_variable))
										{
											if (1==(*extended_variable_iterator)->
												number_differentiable())
											{
												++k;
											}
											++extended_variable_iterator;
										}
										if (k>=number_of_extended_dependent_values)
										{
											// have to clone otherwise changes when increment iterator
											temp_independent_variable=
												temp_independent_variable->clone();
											extended_dependent_list.push_back(
												temp_independent_variable);
											extended_independent_list.push_back(Function_handle(
												new Function_identity(temp_independent_variable))->
												output());
											++number_of_extended_dependent_values;
											++number_of_extended_independent_values;
										}
									}
									++j;
									++variable_iterator;
								}
							}
							++i;
							++independent_variables_iterator;
						}
						if (valid&&(0<number_of_extended_dependent_values)&&
							(0<number_of_extended_independent_values))
						{
							Function_handle dependent_value;

							// make sure that dependent is consistent with independent
							dependent_value=dependent_variable->get_value();
							if (dependent_variable->set_value((function_inverse->output())->
								evaluate()))
							{
								try
								{
									Function_variable_handle extended_dependent_variable(
										new Function_variable_composite(extended_dependent_list));
									Function_variable_handle extended_independent_variable(
										new Function_variable_composite(extended_independent_list));
									std::list<Function_variable_handle>
										extended_dependent_variables(
										number_of_independent_variables,
										extended_dependent_variable);
									Function_derivative_matrix_handle derivative_inverse(
										new Function_derivative_matrix(
										extended_independent_variable,
										extended_dependent_variables));

									//???debug
									std::cout << "extended_dependent_variable=" << *(extended_dependent_variable->get_string_representation()) << std::endl;
									//???debug
									std::cout << "extended_independent_variable=" << *(extended_independent_variable->get_string_representation()) << std::endl;
									//???debug
									std::cout << "before inverse matrix=" << (derivative_inverse->matrices).back() << std::endl;
									if (derivative_inverse&&(derivative_inverse=
										boost::dynamic_pointer_cast<Function_derivative_matrix,
										Function>((derivative_inverse->inverse)())))
									{
										// extract Matrix for d(this)/d(independent_variables)
										bool finished;
										Function_size_type column,column_extended,row;
										Function_variable_independent_handle temp_variable;
										Matrix matrix(number_of_rows,number_of_columns);
										Matrix
											&matrix_extended=(derivative_inverse->matrices).back();
										std::vector<Function_size_type> derivative_index(
											number_of_independent_variables,0);
										std::vector<Function_size_type> dependent_value_mapping(
											number_of_rows,0);
										ublas::matrix<Function_size_type> independent_value_mapping(
											number_of_independent_variables,
											maximum_number_of_independent_values);

										if (equivalent(specified_dependent_variable,
											dependent_variable))
										{
											for (i=0;i<number_of_rows;++i)
											{
												dependent_value_mapping[i]=i;
											}
										}
										else
										{
											k=dependent_variable->number_differentiable();
											variable_iterator=
												specified_dependent_variable->begin_atomic();
											i=0;
											while (i<number_of_rows)
											{
												if (1==(*variable_iterator)->number_differentiable())
												{
													j=0;
													extended_variable_iterator=dependent_variable->
														begin_atomic();
													while ((j<k)&&!equivalent(*extended_variable_iterator,
														*variable_iterator))
													{
														if (1==(*extended_variable_iterator)->
															number_differentiable())
														{
															++j;
														}
														++extended_variable_iterator;
													}
													dependent_value_mapping[i]=j;
													++i;
												}
												++variable_iterator;
											}
										}
										//???debug
										std::cout << "dependent_value_mapping=";
										for (i=0;i<number_of_rows;++i)
										{
											std::cout << " " << dependent_value_mapping[i];
										}
										std::cout << std::endl;
										independent_variables_iterator=
											independent_variables.begin();
										for (i=0;i<number_of_independent_variables;++i)
										{
											number_of_independent_values=
												numbers_of_independent_values[i];
											// initialize independent_value_mapping(i,*)
											variable_iterator=
												(*independent_variables_iterator)->begin_atomic();
											j=0;
											while (j<number_of_independent_values)
											{
												bool found;

												temp_independent_variable= *variable_iterator;
												if ((temp_variable=boost::dynamic_pointer_cast<
													Function_variable_independent,Function_variable>(
													temp_independent_variable))&&(equivalent(
													temp_variable->function(),
													function_inverse)))
												{
													temp_independent_variable=
														temp_variable->get_wrapped();
												}
												if (1==temp_independent_variable->
													number_differentiable())
												{
													k=0;
													extended_variable_iterator=
														extended_independent_variable->begin_atomic();
													found=false;
													while ((k<number_of_extended_independent_values)&&
														!found)
													{
														temp_extended_independent_variable=
															*extended_variable_iterator;
														if ((boost::dynamic_pointer_cast<Function_identity,
															Function>(temp_extended_independent_variable->
															function()))&&(temp_wrapper_variable=
															boost::dynamic_pointer_cast<
															Function_variable_wrapper,Function_variable>(
															temp_extended_independent_variable)))
														{
															temp_extended_independent_variable=
																temp_wrapper_variable->get_wrapped();
														}
														if (equivalent(temp_extended_independent_variable,
															temp_independent_variable))
														{
															found=true;
														}
														else
														{
															if (1==temp_extended_independent_variable->
																number_differentiable())
															{
																++k;
															}
															++extended_variable_iterator;
														}
													}
													independent_value_mapping(i,j)=k;
													++j;
												}
												++variable_iterator;
											}
											++independent_variables_iterator;
										}
										//???debug
										std::cout << "independent_value_mapping=" << independent_value_mapping << std::endl;
										column=0;
										finished=false;
										while (!finished)
										{
											column_extended=0;
											for (i=0;i<number_of_independent_variables;++i)
											{
												column_extended *=
													number_of_extended_independent_values;
												column_extended +=
													independent_value_mapping(i,derivative_index[i]);
											}
											for (row=0;row<number_of_rows;++row)
											{
												matrix(row,column)=matrix_extended(
													dependent_value_mapping[row],column_extended);
											}
											++column;
											// increment derivative_index
											finished=true;
											i=number_of_independent_variables-1;
											while (finished&&(i>0))
											{
												++derivative_index[i];
												if (derivative_index[i]==
													numbers_of_independent_values[i])
												{
													derivative_index[i]=0;
												}
												else
												{
													finished=false;
												}
												--i;
											}
											if (finished)
											{
												++derivative_index[0];
												finished=(derivative_index[0]==
													numbers_of_independent_values[0]);
											}
										}
										//???debug
										std::cout << "matrix=" << matrix << std::endl;
										//???debug
										std::cout << "matrix_extended=" << matrix_extended << std::endl;
										result=Function_matrix_scalar_handle(
											new Function_matrix<Scalar>(matrix));
									}
								}
								catch (Function_derivative_matrix::Construction_exception)
								{
									// do nothing
									//???debug
									std::cout << "Function_variable_dependent::evaluate_derivative.  Failed" << std::endl;
								}
								// reset dependent variable value
								dependent_variable->set_value(dependent_value);
							}
						}
#else // defined (OLD_CODE3)
						bool found,valid;
						Function_handle temp_function;
						Function_size_type j,j_max,k,l,l_max,number_of_dependent_values,
							number_of_extended_dependent_values,
							number_of_extended_independent_values,
							number_of_independent_values;
						Function_variable_handle temp_atomic_variable;
						Function_variable_iterator variable_atomic_iterator_1,
							variable_atomic_iterator_2;
						Function_variable_wrapper_handle temp_wrapper_variable;
						Matrix temp_matrix(1,1);
						std::list<Function_variable_handle> extended_dependent_list(0),
							extended_independent_list(0);
						std::list<Function_variable_handle>::iterator
							independent_variables_iterator_2;
						ublas::matrix<Function_size_type> independent_value_mapping(
							number_of_independent_variables,
							maximum_number_of_independent_values);

						number_of_dependent_values=
							dependent_variable->number_differentiable();
						number_of_independent_values=
							independent_variable->number_differentiable();
						// set up extended dependent and independent variables
						// set up independent value mapping
						extended_independent_list.push_back(independent_variable);
						extended_dependent_list.push_back(dependent_variable);
						number_of_extended_dependent_values=number_of_dependent_values;
						number_of_extended_independent_values=number_of_independent_values;
						independent_variables_iterator=independent_variables.begin();
						i=0;
						valid=true;
						while (valid&&(i<number_of_independent_variables))
						{
							variable_atomic_iterator_1=
								(*independent_variables_iterator)->begin_atomic();
							j=0;
							j_max=numbers_of_independent_values[i];
							while (j<j_max)
							{
								if (1==(*variable_atomic_iterator_1)->number_differentiable())
								{
									found=false;
									// check for repeat in independent variables
									if (0<i)
									{
										independent_variables_iterator_2=
											independent_variables.begin();
										k=0;
										while (!found&&(k<i))
										{
											variable_atomic_iterator_2=
												(*independent_variables_iterator_2)->begin_atomic();
											l=0;
											l_max=numbers_of_independent_values[k];
											while (!found&&(l<l_max))
											{
												if (1==(*variable_atomic_iterator_2)->
													number_differentiable())
												{
													if (equivalent(*variable_atomic_iterator_1,
														*variable_atomic_iterator_2))
													{
														found=true;
														independent_value_mapping(i,j)=
															independent_value_mapping(k,l);
													}
													++l;
												}
												++variable_atomic_iterator_2;
											}
											++k;
											++independent_variables_iterator_2;
										}
									}
									if (!found)
									{
										// check for repeat in dependent
										temp_atomic_variable=0;
										variable_atomic_iterator_2=
											dependent_variable->begin_atomic();
										k=0;
										while (!found&&(k<number_of_dependent_values))
										{
											if (1==(*variable_atomic_iterator_2)->
												number_differentiable())
											{
												if (equivalent(*variable_atomic_iterator_1,
													*variable_atomic_iterator_2))
												{
													found=true;
													// replace with a unique atomic variable (derivative
													//   wrt any other is zero)
													if (temp_function=Function_handle(
														new Function_matrix<Scalar>(temp_matrix)))
													{
														temp_atomic_variable=temp_function->output();
														independent_value_mapping(i,j)=
															number_of_extended_independent_values;
													}
												}
												++k;
											}
											++variable_atomic_iterator_2;
										}
										if (!found)
										{
											temp_atomic_variable= *variable_atomic_iterator_1;
											if (temp_wrapper_variable=boost::dynamic_pointer_cast<
												Function_variable_wrapper,Function_variable>(
												temp_atomic_variable))
											{
												temp_atomic_variable=
													temp_wrapper_variable->get_wrapped();
											}
											// check for repeat in independent
											variable_atomic_iterator_2=
												independent_variable->begin_atomic();
											k=0;
											while (!found&&(k<number_of_independent_values))
											{
												if (1==(*variable_atomic_iterator_2)->
													number_differentiable())
												{
													if (equivalent(temp_atomic_variable,
														*variable_atomic_iterator_2))
													{
														found=true;
														independent_value_mapping(i,j)=k;
													}
													++k;
												}
												++variable_atomic_iterator_2;
											}
											if (!found)
											{
												independent_value_mapping(i,j)=
													number_of_extended_independent_values;
											}
											// have to clone otherwise changes when increment iterator
											temp_atomic_variable=
												(*variable_atomic_iterator_1)->clone();
										}
										if (temp_atomic_variable)
										{
											extended_dependent_list.push_back(
												temp_atomic_variable);
											extended_independent_list.push_back(Function_handle(
												new Function_identity(temp_atomic_variable))->
												output());
											++number_of_extended_dependent_values;
											++number_of_extended_independent_values;
										}
										else
										{
											valid=false;
										}
									}
									++j;
								}
								++variable_atomic_iterator_1;
							}
							++i;
							++independent_variables_iterator;
						}
						if (valid&&(0<number_of_extended_dependent_values)&&
							(0<number_of_extended_independent_values))
						{
							Function_handle dependent_value;

							// make sure that dependent is consistent with independent
							dependent_value=dependent_variable->get_value();
							if (((function_inverse->output())->evaluate)()&&
								(dependent_variable->set_value)((function_inverse->output())->
								get_value()))
							{
								try
								{
									Function_variable_handle extended_dependent_variable(
										new Function_variable_composite(extended_dependent_list));
									Function_variable_handle extended_independent_variable(
										new Function_variable_composite(extended_independent_list));
									std::list<Function_variable_handle>
										extended_dependent_variables(
										number_of_independent_variables,
										extended_dependent_variable);
									Function_derivative_matrix_handle derivative_inverse(
										new Function_derivative_matrix(
										extended_independent_variable,
										extended_dependent_variables));

									if (derivative_inverse&&(derivative_inverse=
										boost::dynamic_pointer_cast<Function_derivative_matrix,
										Function>((derivative_inverse->inverse)())))
									{
										// extract Matrix for d(this)/d(independent_variables)
										bool finished;
										Function_size_type column,column_extended,row;
										Function_variable_independent_handle temp_variable;
										Matrix matrix(number_of_rows,number_of_columns);
										Matrix
											&matrix_extended=(derivative_inverse->matrices).back();
										std::vector<Function_size_type> derivative_index(
											number_of_independent_variables,0);
										std::vector<Function_size_type> dependent_value_mapping(
											number_of_rows,0);

										if (equivalent(specified_dependent_variable,
											dependent_variable))
										{
											for (i=0;i<number_of_rows;++i)
											{
												dependent_value_mapping[i]=i;
											}
										}
										else
										{
											k=dependent_variable->number_differentiable();
											variable_atomic_iterator_2=
												specified_dependent_variable->begin_atomic();
											i=0;
											while (i<number_of_rows)
											{
												if (1==(*variable_atomic_iterator_2)->
													number_differentiable())
												{
													j=0;
													variable_atomic_iterator_1=dependent_variable->
														begin_atomic();
													while ((j<k)&&!equivalent(*variable_atomic_iterator_1,
														*variable_atomic_iterator_2))
													{
														if (1==(*variable_atomic_iterator_1)->
															number_differentiable())
														{
															++j;
														}
														++variable_atomic_iterator_1;
													}
													dependent_value_mapping[i]=j;
													++i;
												}
												++variable_atomic_iterator_2;
											}
										}
										column=0;
										finished=false;
										while (!finished)
										{
											column_extended=0;
											for (i=0;i<number_of_independent_variables;++i)
											{
												column_extended *=
													number_of_extended_independent_values;
												column_extended +=
													independent_value_mapping(i,derivative_index[i]);
											}
											for (row=0;row<number_of_rows;++row)
											{
												matrix(row,column)=matrix_extended(
													dependent_value_mapping[row],column_extended);
											}
											++column;
											// increment derivative_index
											finished=true;
											i=number_of_independent_variables-1;
											while (finished&&(i>0))
											{
												++derivative_index[i];
												if (derivative_index[i]==
													numbers_of_independent_values[i])
												{
													derivative_index[i]=0;
												}
												else
												{
													finished=false;
												}
												--i;
											}
											if (finished)
											{
												++derivative_index[0];
												finished=(derivative_index[0]==
													numbers_of_independent_values[0]);
											}
										}
										result=Function_matrix_scalar_handle(
											new Function_matrix<Scalar>(matrix));
									}
								}
								catch (Function_derivative_matrix::Construction_exception)
								{
									// do nothing
									//???debug
									std::cout << "Function_variable_dependent::evaluate_derivative.  Failed" << std::endl;
								}
								// reset dependent variable value
								dependent_variable->set_value(dependent_value);
							}
						}
#endif // defined (OLD_CODE3)
					}
#endif // defined (OLD_CODE2)
				}
			}

			return (result);
		};
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables)
		{
			return (Function_handle(new Function_derivatnew_inverse(
				Function_variable_handle(this),independent_variables)));
		}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	private:
		// copy constructor
		Function_variable_dependent(
			const Function_variable_dependent& variable_dependent):
			Function_variable_wrapper(variable_dependent){};
		// assignment
		Function_variable_dependent& operator=(const Function_variable_dependent&);
};

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_inverse
// ---------------------------------

Function_derivatnew_inverse::Function_derivatnew_inverse(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables)
//******************************************************************************
// LAST MODIFIED : 23 January 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (!(boost::dynamic_pointer_cast<Function_variable_dependent,
		Function_variable>(dependent_variable)&&
		boost::dynamic_pointer_cast<Function_inverse,
		Function>(dependent_variable->function())))
	{
		throw Function_derivatnew_inverse::Construction_exception();
	}
}

Function_derivatnew_inverse::~Function_derivatnew_inverse(){}
//******************************************************************************
// LAST MODIFIED : 23 January 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_derivatnew_inverse::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 23 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);

	if (!evaluated())
	{
		Function_inverse_handle function_inverse;

		if (boost::dynamic_pointer_cast<Function_variable_dependent,
			Function_variable>(dependent_variable)&&(function_inverse=
			boost::dynamic_pointer_cast<Function_inverse,Function>(
			dependent_variable->function())))
		{
			bool valid;
			Function_variable_handle derivative_inverse_output;

			valid=false;
			if ((derivative_inverse_output=derivative_inverse->output())&&
				(derivative_inverse_output->evaluate()))
			{
				derivative_matrix=(derivative_inverse->derivative_matrix).inverse();
				valid=true;
			}
			if (valid)
			{
				set_evaluated();
			}
		}
	}
	if (evaluated())
	{
		result=get_value(atomic_variable);
	}

	return (result);
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_derivatnew_inverse::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result(true);

	if (equivalent(Function_handle(this),atomic_variable->function())&&
		!evaluated())
	{
		Function_inverse_handle function_inverse;
		Function_variable_dependent_handle variable_dependent;
		Function_variable_handle derivative_dependent_variable;

		result=false;
		if ((variable_dependent=boost::dynamic_pointer_cast<
			Function_variable_dependent,Function_variable>(dependent_variable))&&
			(function_inverse=boost::dynamic_pointer_cast<Function_inverse,Function>(
			dependent_variable->function()))&&(derivative_dependent_variable=
			variable_dependent->get_wrapped()))
		{
#if defined (OLD_CODE)
			Function_variable_handle derivative_inverse_output;

			if ((derivative_inverse_output=derivative_inverse->output())&&
				(derivative_inverse_output->evaluate()))
			{
				//???debug
				std::cout << "Function_derivatnew_inverse::evaluate.  " << (derivative_inverse->derivative_matrix).back() << std::endl;
				derivative_matrix=(derivative_inverse->derivative_matrix).inverse();
				result=true;
				set_evaluated();
			}
#endif // defined (OLD_CODE)
			const Function_size_type number_of_independent_variables=
				independent_variables.size();
			Function_variable_handle inverse_dependent_variable,
				inverse_independent_variable;

			inverse_dependent_variable=function_inverse->dependent_variable;
			inverse_independent_variable=function_inverse->independent_variable;
			if (inverse_dependent_variable&&inverse_independent_variable&&
				(0<number_of_independent_variables))
			{
				Function_size_type i,maximum_number_of_independent_values,
					number_of_columns,number_of_independent_values,number_of_rows;
				std::list<Function_variable_handle>::iterator
					independent_variables_iterator;
				std::vector<Function_size_type>
					numbers_of_independent_values(number_of_independent_variables);

				number_of_rows=derivative_dependent_variable->number_differentiable();
				independent_variables_iterator=independent_variables.begin();
				maximum_number_of_independent_values=0;
				number_of_columns=1;
				for (i=0;i<number_of_independent_variables;++i)
				{
					number_of_independent_values=
						(*independent_variables_iterator)->number_differentiable();
					numbers_of_independent_values[i]=number_of_independent_values;
					if (maximum_number_of_independent_values<
						number_of_independent_values)
					{
						maximum_number_of_independent_values=number_of_independent_values;
					}
					number_of_columns *= number_of_independent_values;
					++independent_variables_iterator;
				}
				if ((0<number_of_columns)&&(0<number_of_rows)&&
					(0<maximum_number_of_independent_values))
				{
					bool found;
					Function_handle temp_function;
					Function_size_type j,j_max,k,l,l_max,number_of_dependent_values,
						number_of_extended_dependent_values,
						number_of_extended_independent_values,
						number_of_independent_values;
					Function_variable_handle temp_atomic_variable;
					Function_variable_iterator variable_atomic_iterator_1,
						variable_atomic_iterator_2;
					Function_variable_wrapper_handle temp_wrapper_variable;
					Matrix temp_matrix(1,1);
					std::list<Function_variable_handle> extended_dependent_list(0),
						extended_independent_list(0);
					std::list<Function_variable_handle>::iterator
						independent_variables_iterator_2;
					ublas::matrix<Function_size_type> independent_value_mapping(
						number_of_independent_variables,
						maximum_number_of_independent_values);

					number_of_dependent_values=
						inverse_dependent_variable->number_differentiable();
					number_of_independent_values=
						inverse_independent_variable->number_differentiable();
					// set up extended dependent and independent variables
					// set up independent value mapping
					extended_independent_list.push_back(inverse_independent_variable);
					extended_dependent_list.push_back(inverse_dependent_variable);
					number_of_extended_dependent_values=number_of_dependent_values;
					number_of_extended_independent_values=number_of_independent_values;
					independent_variables_iterator=independent_variables.begin();
					i=0;
					result=true;
					while (result&&(i<number_of_independent_variables))
					{
						variable_atomic_iterator_1=
							(*independent_variables_iterator)->begin_atomic();
						j=0;
						j_max=numbers_of_independent_values[i];
						while (j<j_max)
						{
							if (1==(*variable_atomic_iterator_1)->number_differentiable())
							{
								found=false;
								// check for repeat in independent variables
								if (0<i)
								{
									independent_variables_iterator_2=
										independent_variables.begin();
									k=0;
									while (!found&&(k<i))
									{
										variable_atomic_iterator_2=
											(*independent_variables_iterator_2)->begin_atomic();
										l=0;
										l_max=numbers_of_independent_values[k];
										while (!found&&(l<l_max))
										{
											if (1==(*variable_atomic_iterator_2)->
												number_differentiable())
											{
												if (equivalent(*variable_atomic_iterator_1,
													*variable_atomic_iterator_2))
												{
													found=true;
													independent_value_mapping(i,j)=
														independent_value_mapping(k,l);
												}
												++l;
											}
											++variable_atomic_iterator_2;
										}
										++k;
										++independent_variables_iterator_2;
									}
								}
								if (!found)
								{
									// check for repeat in dependent
									temp_atomic_variable=0;
									variable_atomic_iterator_2=
										inverse_dependent_variable->begin_atomic();
									k=0;
									while (!found&&(k<number_of_dependent_values))
									{
										if (1==(*variable_atomic_iterator_2)->
											number_differentiable())
										{
											if (equivalent(*variable_atomic_iterator_1,
												*variable_atomic_iterator_2))
											{
												found=true;
												// replace with a unique atomic variable (derivative
												//   wrt any other is zero)
												if (temp_function=Function_handle(
													new Function_matrix<Scalar>(temp_matrix)))
												{
													temp_atomic_variable=temp_function->output();
													independent_value_mapping(i,j)=
														number_of_extended_independent_values;
												}
											}
											++k;
										}
										++variable_atomic_iterator_2;
									}
									if (!found)
									{
										temp_atomic_variable= *variable_atomic_iterator_1;
										if (temp_wrapper_variable=boost::dynamic_pointer_cast<
											Function_variable_wrapper,Function_variable>(
											temp_atomic_variable))
										{
											temp_atomic_variable=
												temp_wrapper_variable->get_wrapped();
										}
										// check for repeat in independent
										variable_atomic_iterator_2=
											inverse_independent_variable->begin_atomic();
										k=0;
										while (!found&&(k<number_of_independent_values))
										{
											if (1==(*variable_atomic_iterator_2)->
												number_differentiable())
											{
												if (equivalent(temp_atomic_variable,
													*variable_atomic_iterator_2))
												{
													found=true;
													independent_value_mapping(i,j)=k;
												}
												++k;
											}
											++variable_atomic_iterator_2;
										}
										if (!found)
										{
											independent_value_mapping(i,j)=
												number_of_extended_independent_values;
										}
										// have to clone otherwise changes when increment iterator
										temp_atomic_variable=
											(*variable_atomic_iterator_1)->clone();
									}
									if (temp_atomic_variable)
									{
										extended_dependent_list.push_back(
											temp_atomic_variable);
										extended_independent_list.push_back(Function_handle(
											new Function_identity(temp_atomic_variable))->
											output());
										++number_of_extended_dependent_values;
										++number_of_extended_independent_values;
									}
									else
									{
										result=false;
									}
								}
								++j;
							}
							++variable_atomic_iterator_1;
						}
						++i;
						++independent_variables_iterator;
					}
					if (result&&(0<number_of_extended_dependent_values)&&
						(0<number_of_extended_independent_values))
					{
						Function_handle dependent_value;

						// make sure that dependent is consistent with independent
						dependent_value=inverse_dependent_variable->get_value();
						if (((function_inverse->output())->evaluate)()&&
							(inverse_dependent_variable->set_value)(
							(function_inverse->output())->get_value()))
						{
							Function_variable_handle extended_dependent_variable(
								new Function_variable_composite(extended_dependent_list));
							Function_variable_handle extended_independent_variable(
								new Function_variable_composite(extended_independent_list));
							std::list<Function_variable_handle>
								extended_dependent_variables(
								number_of_independent_variables,
								extended_dependent_variable);
							Function_derivatnew_handle extended_derivative=
								boost::dynamic_pointer_cast<Function_derivatnew,Function>(
								extended_independent_variable->derivative(
								extended_dependent_variables));
							Function_variable_handle extended_derivative_output;

							if (extended_derivative&&
								(extended_derivative_output=extended_derivative->output())&&
								(extended_derivative_output->evaluate()))
							{
								Derivative_matrix extended_derivative_inverse;
								std::list<Matrix> matrices;
								std::list<Matrix>::iterator extended_matrix_iterator;
								std::vector<bool> independent_variable_used(
									number_of_independent_variables+1,false);
								std::vector<Function_size_type> dependent_value_mapping(
									number_of_rows,0);
								std::vector<Function_size_type> derivative_index(
									number_of_independent_variables,0);

								extended_derivative_inverse=
									(extended_derivative->derivative_matrix).inverse();
								// extract derivative_matrix
								if (equivalent(derivative_dependent_variable,
									inverse_dependent_variable))
								{
									for (i=0;i<number_of_rows;++i)
									{
										dependent_value_mapping[i]=i;
									}
								}
								else
								{
									k=inverse_dependent_variable->number_differentiable();
									variable_atomic_iterator_2=
										derivative_dependent_variable->begin_atomic();
									i=0;
									while (i<number_of_rows)
									{
										if (1==(*variable_atomic_iterator_2)->
											number_differentiable())
										{
											j=0;
											variable_atomic_iterator_1=inverse_dependent_variable->
												begin_atomic();
											while ((j<k)&&!equivalent(*variable_atomic_iterator_1,
												*variable_atomic_iterator_2))
											{
												if (1==(*variable_atomic_iterator_1)->
													number_differentiable())
												{
													++j;
												}
												++variable_atomic_iterator_1;
											}
											dependent_value_mapping[i]=j;
											++i;
										}
										++variable_atomic_iterator_2;
									}
								}
								independent_variable_used[0]=true;
								extended_matrix_iterator=extended_derivative_inverse.begin();
								while (!independent_variable_used[
									number_of_independent_variables])
								{
									Matrix &matrix_extended= *extended_matrix_iterator;

									// calculate the number_of_columns and zero derivative_index
									number_of_columns=1;
									for (i=0;i<number_of_independent_variables;++i)
									{
										if (independent_variable_used[i])
										{
											number_of_columns *= numbers_of_independent_values[i];
											derivative_index[i]=0;
										}
									}
									// fill in matrix
									{
										bool finished;
										Function_size_type column,column_extended,row;
										Function_variable_independent_handle temp_variable;
										Matrix matrix(number_of_rows,number_of_columns);

										column=0;
										finished=false;
										while (!finished)
										{
											column_extended=0;
											for (i=0;i<number_of_independent_variables;++i)
											{
												if (independent_variable_used[i])
												{
													column_extended *=
														number_of_extended_independent_values;
													column_extended +=
														independent_value_mapping(i,derivative_index[i]);
												}
											}
											for (row=0;row<number_of_rows;++row)
											{
												matrix(row,column)=matrix_extended(
													dependent_value_mapping[row],column_extended);
											}
											++column;
											// increment derivative_index
											finished=true;
											i=number_of_independent_variables-1;
											while (finished&&(i>0))
											{
												if (independent_variable_used[i])
												{
													++derivative_index[i];
													if (derivative_index[i]==
														numbers_of_independent_values[i])
													{
														derivative_index[i]=0;
													}
													else
													{
														finished=false;
													}
												}
												--i;
											}
											if (finished&&independent_variable_used[0])
											{
												++derivative_index[0];
												finished=(derivative_index[0]==
													numbers_of_independent_values[0]);
											}
										}
										matrices.push_back(matrix);
									}
									// move to next matrix
									++extended_matrix_iterator;
									i=0;
									while ((i<number_of_independent_variables)&&
										independent_variable_used[i])
									{
										independent_variable_used[i]=false;
										++i;
									}
									independent_variable_used[i]=true;
								}
								derivative_matrix=Derivative_matrix(matrices);
							}
							else
							{
								result=false;
							}
							// reset dependent variable value
							inverse_dependent_variable->set_value(dependent_value);
						}
						else
						{
							result=false;
						}
						if (result)
						{
							set_evaluated();
						}
					}
				}
			}
		}
	}

	return (result);
}
#endif // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)


// class Function_variable_dependent_estimate
// ------------------------------------------

// forward declaration so that can use _handle
class Function_variable_dependent_estimate;
typedef boost::intrusive_ptr<Function_variable_dependent_estimate>
	Function_variable_dependent_estimate_handle;

class Function_variable_dependent_estimate : public Function_variable_wrapper
//******************************************************************************
// LAST MODIFIED : 28 April 2005
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
			Function_variable_dependent_estimate_handle result;

			result=new Function_variable_dependent_estimate(*this);
			if (working_variable)
			{
				result->working_variable=working_variable->clone();
			}
			else
			{
				result->working_variable=0;
			}

			return (result);
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


// class Function_variable_value_tolerance
// ---------------------------------------

// forward declaration so that can use _handle
class Function_variable_value_tolerance;
typedef boost::intrusive_ptr<Function_variable_value_tolerance>
	Function_variable_value_tolerance_handle;

class Function_variable_value_tolerance :
	public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 28 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_value_tolerance(
			const Function_inverse_handle& function_inverse):
			Function_variable_matrix<Scalar>(function_inverse,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			true,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			1,1){};
		// destructor
		~Function_variable_value_tolerance(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_value_tolerance(
				*this)));
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
			string_handle return_string;
			std::ostringstream out;

			if (return_string=new std::string)
			{
				out << "value_tolerance";
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
				Function_inverse_handle function_inverse=
					boost::dynamic_pointer_cast<Function_inverse,Function>(function());

				result=Function_variable_matrix_scalar_handle(
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
			if ((1==row_private)&&(1==column_private)&&(function_inverse=
				boost::dynamic_pointer_cast<Function_inverse,Function>(function())))
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


// class Function_variable_step_tolerance
// --------------------------------------

// forward declaration so that can use _handle
class Function_variable_step_tolerance;
typedef boost::intrusive_ptr<Function_variable_step_tolerance>
	Function_variable_step_tolerance_handle;

class Function_variable_step_tolerance : public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 28 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_step_tolerance(
			const Function_inverse_handle& function_inverse):
			Function_variable_matrix<Scalar>(function_inverse,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			true,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			1,1){};
		// destructor
		~Function_variable_step_tolerance(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_step_tolerance(
				*this)));
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
			string_handle return_string;
			std::ostringstream out;

			if (return_string=new std::string)
			{
				out << "step_tolerance";
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
				Function_inverse_handle function_inverse=
					boost::dynamic_pointer_cast<Function_inverse,Function>(function());

				result=Function_variable_matrix_scalar_handle(
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
			if ((1==row_private)&&(1==column_private)&&(function_inverse=
				boost::dynamic_pointer_cast<Function_inverse,Function>(function())))
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


// class Function_variable_maximum_iterations
// ------------------------------------------

// forward declaration so that can use _handle
class Function_variable_maximum_iterations;
typedef boost::intrusive_ptr<Function_variable_maximum_iterations>
	Function_variable_maximum_iterations_handle;

class Function_variable_maximum_iterations :
	public Function_variable_matrix<Function_size_type>
//******************************************************************************
// LAST MODIFIED : 28 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_maximum_iterations(
			const Function_inverse_handle& function_inverse):
			Function_variable_matrix<Function_size_type>(function_inverse,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			true,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			1,1){};
		// destructor
		~Function_variable_maximum_iterations(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_maximum_iterations(
				*this)));
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
		Function_variable_matrix_function_size_type_handle
			operator()(Function_size_type row,Function_size_type column) const
		{
			Function_variable_matrix_function_size_type_handle result(0);

			if ((1==row)&&(1==column))
			{
				Function_inverse_handle function_inverse=
					boost::dynamic_pointer_cast<Function_inverse,Function>(function());

				result=Function_variable_matrix_function_size_type_handle(
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
			if ((1==row_private)&&(1==column_private)&&(function_inverse=
				boost::dynamic_pointer_cast<Function_inverse,Function>(function())))
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
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (dependent_variable)
	{
		dependent_variable->add_dependent_function(this);
	}
}

Function_inverse::~Function_inverse()
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
	if (dependent_variable)
	{
		dependent_variable->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
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
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_dependent(this)));
}

bool Function_inverse::operator==(const Function& function) const
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
			const Function_inverse& function_inverse=
				dynamic_cast<const Function_inverse&>(function);

			result=(equivalent(dependent_variable,
				function_inverse.dependent_variable)&&
				equivalent(independent_variable,
				function_inverse.independent_variable));
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
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
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_dependent_estimate(
		this)));
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

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_inverse::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_dependent_handle atomic_variable_dependent;
	Function_variable_dependent_estimate_handle
		atomic_variable_dependent_estimate;

	if ((atomic_variable_dependent=boost::dynamic_pointer_cast<
		Function_variable_dependent,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_dependent->function()))
	{
		result=(atomic_variable_dependent->evaluate)();
	}
	else if ((atomic_variable_dependent_estimate=boost::dynamic_pointer_cast<
		Function_variable_dependent_estimate,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_dependent_estimate->function()))
	{
		result=get_value(atomic_variable);
	}

	return (result);
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_inverse::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result(true);
	Function_variable_dependent_handle atomic_variable_dependent;

	if ((atomic_variable_dependent=boost::dynamic_pointer_cast<
		Function_variable_dependent,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_dependent->function()))
	{
		result=(atomic_variable_dependent->evaluate)();
	}

	return (result);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

bool Function_inverse::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_independent_handle atomic_variable_independent;
	Function_variable_dependent_handle atomic_variable_dependent;
	Function_variable_dependent_estimate_handle
		atomic_variable_dependent_estimate;
	Function_variable_step_tolerance_handle atomic_variable_step_tolerance;
	Function_variable_value_tolerance_handle atomic_variable_value_tolerance;

	result=false;
	if ((atomic_variable_independent=boost::dynamic_pointer_cast<
		Function_variable_independent,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_independent->function()))
	{
		Function_variable_independent_handle atomic_independent_variable;

		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_independent,Function_variable>(
			atomic_independent_variables.front()))&&
			equivalent(atomic_variable_independent,atomic_independent_variable))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}
	else if ((atomic_variable_dependent_estimate=boost::dynamic_pointer_cast<
		Function_variable_dependent_estimate,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_dependent_estimate->function()))
	{
		Function_variable_dependent_handle atomic_independent_variable;

		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_dependent,Function_variable>(
			atomic_independent_variables.front()))&&
			equivalent(atomic_variable_dependent,atomic_independent_variable))
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
		equivalent(Function_handle(this),atomic_variable_dependent->function()))
	{
		Function_matrix_scalar_handle derivative_value;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle derivative_function;
		Function_variable_handle derivative_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_value=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>((atomic_variable_dependent->
			evaluate_derivative)(atomic_independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(atomic_variable_dependent->derivative(
			atomic_independent_variables)))&&(derivative_variable=
			derivative_function->output())&&(derivative_variable->evaluate())&&
			(derivative_variable=derivative_function->matrix(
			atomic_independent_variables))&&
			(derivative_value=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(derivative_variable->get_value()))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(1==derivative_value->number_of_rows())&&
			(1==derivative_value->number_of_columns()))
		{
			derivative=(*derivative_value)(1,1);
			result=true;
		}
	}
	else if ((atomic_variable_step_tolerance=boost::dynamic_pointer_cast<
		Function_variable_step_tolerance,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_step_tolerance->function()))
	{
		Function_variable_step_tolerance_handle atomic_independent_variable;

		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_step_tolerance,Function_variable>(
			atomic_independent_variables.front()))&&
			equivalent(atomic_variable_step_tolerance,atomic_independent_variable))
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
		equivalent(Function_handle(this),
		atomic_variable_value_tolerance->function()))
	{
		Function_variable_value_tolerance_handle atomic_independent_variable;

		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_value_tolerance,Function_variable>(
			atomic_independent_variables.front()))&&
			equivalent(atomic_variable_value_tolerance,atomic_independent_variable))
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
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_independent_handle atomic_variable_independent;
	Function_variable_dependent_handle atomic_variable_dependent;
	Function_variable_dependent_estimate_handle
		atomic_variable_dependent_estimate;
	Function_variable_maximum_iterations_handle
		atomic_variable_maximum_iterations;
	Function_variable_step_tolerance_handle atomic_variable_step_tolerance;
	Function_variable_value_tolerance_handle atomic_variable_value_tolerance;

	result=false;
	if ((atomic_variable_independent=boost::dynamic_pointer_cast<
		Function_variable_independent,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_independent->function()))
	{
		if (atomic_variable_independent->get_wrapped())
		{
			result=((atomic_variable_independent->get_wrapped())->set_value)(
				atomic_value->get_value());
		}
	}
	else if ((atomic_variable_dependent=boost::dynamic_pointer_cast<
		Function_variable_dependent,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_dependent->function()))
	{
		Function_variable_handle wrapped_variable;

		if (wrapped_variable=atomic_variable_dependent->get_wrapped())
		{
			result=(wrapped_variable->set_value)(atomic_value->get_value());
		}
	}
	else if ((atomic_variable_dependent_estimate=boost::dynamic_pointer_cast<
		Function_variable_dependent_estimate,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_dependent_estimate->function()))
	{
		Function_variable_handle wrapped_variable;

		if (wrapped_variable=atomic_variable_dependent_estimate->get_wrapped())
		{
			result=(wrapped_variable->set_value)(atomic_value->get_value());
		}
	}
	else if ((atomic_variable_maximum_iterations=boost::dynamic_pointer_cast<
		Function_variable_maximum_iterations,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_maximum_iterations->function()))
	{
		Function_variable_value_function_size_type_handle value_function_size_type;
		Function_variable_value_scalar_handle value_scalar;

		if ((std::string("Function_size_type")==(atomic_value->value())->type())&&
			(value_function_size_type=boost::dynamic_pointer_cast<
			Function_variable_value_function_size_type,Function_variable_value>(
			atomic_value->value())))
		{
			result=value_function_size_type->set(maximum_iterations_private,
				atomic_value);
		}
		else if ((std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			Scalar maximum_iterations_scalar;

			if (result=value_scalar->set(maximum_iterations_scalar,atomic_value))
			{
				maximum_iterations_private=
					(Function_size_type)maximum_iterations_scalar;
			}
		}
		if (result)
		{
			set_not_evaluated();
		}
	}
	else if ((atomic_variable_step_tolerance=boost::dynamic_pointer_cast<
		Function_variable_step_tolerance,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_step_tolerance->function()))
	{
		Function_variable_value_scalar_handle value_scalar;

		if ((std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			result=value_scalar->set(step_tolerance_private,atomic_value);
		}
		if (result)
		{
			set_not_evaluated();
		}
	}
	else if ((atomic_variable_value_tolerance=boost::dynamic_pointer_cast<
		Function_variable_value_tolerance,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
			atomic_variable_value_tolerance->function()))
	{
		Function_variable_value_scalar_handle value_scalar;

		if ((std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			result=value_scalar->set(value_tolerance_private,atomic_value);
		}
		if (result)
		{
			set_not_evaluated();
		}
	}

	return (result);
}

Function_handle Function_inverse::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_independent_handle atomic_variable_independent;
	Function_variable_dependent_handle atomic_variable_dependent;
	Function_variable_dependent_estimate_handle
		atomic_variable_dependent_estimate;
	Function_variable_maximum_iterations_handle
		atomic_variable_maximum_iterations;
	Function_variable_step_tolerance_handle atomic_variable_step_tolerance;
	Function_variable_value_tolerance_handle atomic_variable_value_tolerance;

	if ((atomic_variable_independent=boost::dynamic_pointer_cast<
		Function_variable_independent,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_independent->function()))
	{
		if (atomic_variable_independent->get_wrapped())
		{
			result=((atomic_variable_independent->get_wrapped())->get_value)();
		}
	}
	else if ((atomic_variable_dependent=boost::dynamic_pointer_cast<
		Function_variable_dependent,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_dependent->function()))
	{
		Function_variable_handle wrapped_variable;

		if (wrapped_variable=atomic_variable_dependent->get_wrapped())
		{
			result=(wrapped_variable->get_value)();
		}
	}
	else if ((atomic_variable_dependent_estimate=boost::dynamic_pointer_cast<
		Function_variable_dependent_estimate,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_dependent_estimate->function()))
	{
		Function_variable_handle wrapped_variable;

		if (wrapped_variable=atomic_variable_dependent_estimate->get_wrapped())
		{
			result=(wrapped_variable->get_value)();
		}
	}
	else if ((atomic_variable_maximum_iterations=boost::dynamic_pointer_cast<
		Function_variable_maximum_iterations,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_maximum_iterations->function()))
	{
		result=Function_handle(new Function_function_size_type(
			maximum_iterations_private));
	}
	else if ((atomic_variable_step_tolerance=boost::dynamic_pointer_cast<
		Function_variable_step_tolerance,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_step_tolerance->function()))
	{
		Matrix result_matrix(1,1);

		result_matrix(0,0)=step_tolerance_private;
		result=Function_handle(new Function_matrix<Scalar>(result_matrix));
	}
	else if ((atomic_variable_value_tolerance=boost::dynamic_pointer_cast<
		Function_variable_value_tolerance,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),
		atomic_variable_value_tolerance->function()))
	{
		Matrix result_matrix(1,1);

		result_matrix(0,0)=value_tolerance_private;
		result=Function_handle(new Function_matrix<Scalar>(result_matrix));
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
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (dependent_variable)
	{
		dependent_variable->add_dependent_function(this);
	}
}

Function_inverse& Function_inverse::operator=(
	const Function_inverse& function_inverse)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	step_tolerance_private=function_inverse.step_tolerance_private;
	value_tolerance_private=function_inverse.value_tolerance_private;
	if (function_inverse.dependent_variable)
	{
		function_inverse.dependent_variable->add_dependent_function(this);
	}
	if (dependent_variable)
	{
		dependent_variable->remove_dependent_function(this);
	}
	dependent_variable=function_inverse.dependent_variable;
	independent_variable=function_inverse.independent_variable;
	maximum_iterations_private=function_inverse.maximum_iterations_private;

	return (*this);
}

//???DB.  Where I'm up to
//???DB.  Set up value_private for inputs/outputs
//???DB.  Check evaluate
//???DB.  Check constructors
