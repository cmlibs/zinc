//******************************************************************************
// FILE : function_integral.cpp
//
// LAST MODIFIED : 30 March 2005
//
// DESCRIPTION :
//???DB.  Need more memory management for Integration_scheme
//==============================================================================

#include <sstream>

extern "C"
{
#include "finite_element/finite_element_region.h"
//???DB.  Get rid of debug.h (C->C++)
#include "general/debug.h"
}
#include "computed_variable/function_matrix_determinant.hpp"
#include "computed_variable/function_matrix_product.hpp"
#include "computed_variable/function_composition.hpp"
#include "computed_variable/function_derivative.hpp"
#include "computed_variable/function_finite_element.hpp"
#include "computed_variable/function_integral.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_exclusion.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_union.hpp"

// module classes
// ==============

class Quadrature_scheme
{
	friend class Function_integral;
	friend int integrate_over_element(struct FE_element *,void *);
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Quadrature_scheme(struct FE_region *domain,std::string quadrature_scheme):
			number_of_points(0),points(0),weights(0),exactly_integrated_basis(0)
		{
			bool success;

			success=false;
			if (domain)
			{
				int *basis_type_array;

				if (basis_type_array=FE_basis_string_to_type_array(
					quadrature_scheme.c_str()))
				{
					if (exactly_integrated_basis=
						FE_region_get_FE_basis_matching_basis_type(domain,basis_type_array))
					{
						int dimension;

						if (FE_basis_get_dimension(exactly_integrated_basis,&dimension)&&
							(0<dimension))
						{
							bool valid_basis;
							enum FE_basis_type basis_type;
							Function_size_type j;
							int i,k,link_type,next_xi_number,xi_number;

							// determine the number of integration points
							valid_basis=true;
							xi_number=0;
							number_of_points=1;
							while (valid_basis&&(xi_number<dimension)&&
								FE_basis_get_xi_basis_type(exactly_integrated_basis,xi_number,
								&basis_type)&&FE_basis_get_next_linked_xi_number(
								exactly_integrated_basis,xi_number,&next_xi_number,&link_type)
								//???DB.  Starting with tensor products
								&&(0==next_xi_number)&&(0==link_type)
								)
							{
								switch (basis_type)
								{
									case LINEAR_LAGRANGE:
									{
										/* 1 weight.  Do nothing */
									} break;
									case QUADRATIC_LAGRANGE:
									{
										number_of_points *= 2;
									} break;
									case CUBIC_HERMITE:
									case CUBIC_LAGRANGE:
									{
										number_of_points *= 3;
									} break;
									default:
									{
										valid_basis=false;
									} break;
								}
								xi_number++;
							}
							if (valid_basis&&(xi_number>=dimension)&&
								(points=new Vector[number_of_points]))
							{
								success=true;
								weights.resize(number_of_points);
								weights[0]=(Scalar)1;
								for (j=0;j<number_of_points;j++)
								{
									points[j].resize(dimension);
								}
								for (i=0;i<dimension;i++)
								{
									points[0][i]=(Scalar)0.5;
								}
								number_of_points=1;
								for (i=0;i<dimension;i++)
								{
									FE_basis_get_xi_basis_type(exactly_integrated_basis,i,
										&basis_type);
									switch (basis_type)
									{
										case LINEAR_LAGRANGE:
										{
											/* 1 weight.  Do nothing */
											for (j=0;j<number_of_points;j++)
											{
												points[j][i]=(Scalar)0.5;
											}
										} break;
										case QUADRATIC_LAGRANGE:
										{
											for (j=0;j<number_of_points;j++)
											{
												weights[j] *= (Scalar)0.5;
												weights[j+number_of_points]=
													weights[j];
												for (k=0;k<i;k++)
												{
													points[j+number_of_points][k]=points[j][k];
												}
												points[j][i]=(Scalar)(0.5-0.288675134594813);
												points[j+number_of_points][i]=
													(Scalar)(0.5+0.288675134594813);
											}
											number_of_points *= 2;
										} break;
										case CUBIC_HERMITE:
										case CUBIC_LAGRANGE:
										{
											for (j=0;j<number_of_points;j++)
											{
												weights[j+number_of_points]=
													weights[j]*(Scalar)0.444444444444444;
												weights[j] *= (Scalar)0.277777777777778;
												weights[j+2*number_of_points]=weights[j];
												for (k=0;k<i;k++)
												{
													points[j+number_of_points][k]=points[j][k];
													points[j+2*number_of_points][k]=points[j][k];
												}
												points[j][i]=(Scalar)(0.5-0.387298334620741);
												points[j+number_of_points][i]=(Scalar)0.5;
												points[j+2*number_of_points][i]=
													(Scalar)(0.5+0.387298334620741);
											}
											number_of_points *= 3;
										} break;
									}
								}
							}
						}
					}
					DEALLOCATE(basis_type_array);
				}
			}
			if (success)
			{
				// An object that is partially constructed will only have constructors
				//   called for its fully constructed sub-objects.  So the ACCESS
				//   shouldn't be done until all checks have been made, otherwise
				//   the DEACCESS (in destructor) won't be done
				ACCESS(FE_basis)(exactly_integrated_basis);
			}
			else
			{
				throw Quadrature_scheme::Construction_exception();
			}
		};
		// destructor
		~Quadrature_scheme()
		{
			DEACCESS(FE_basis)(&exactly_integrated_basis);
			delete[] points;
		};
		bool operator==(const Quadrature_scheme& scheme)
		{
			bool result;

			result=false;
			if (this&&(exactly_integrated_basis==scheme.exactly_integrated_basis))
			{
				result=true;
			}

			return (result);
		};
	private:
		Quadrature_scheme(const Quadrature_scheme& scheme):
			number_of_points(scheme.number_of_points),
			points(new Vector[scheme.number_of_points]),
			weights(scheme.weights),
			exactly_integrated_basis(scheme.exactly_integrated_basis)
		{
			Function_size_type i;

			for (i=0;i<number_of_points;i++)
			{
				points[i]=scheme.points[i];
			}
			if (exactly_integrated_basis)
			{
				ACCESS(FE_basis)(exactly_integrated_basis);
			}
		};
	private:
		Function_size_type number_of_points;
		Vector *points,weights;
		struct FE_basis *exactly_integrated_basis;
};

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_integral
// ----------------------------------

class Function_derivatnew_integral : public Function_derivatnew
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
		Function_derivatnew_integral(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_integral();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
};
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

// class Function_variable_integral
// --------------------------------

struct Integrate_over_element_data
//******************************************************************************
// LAST MODIFIED : 7 February 2005
//
// DESCRIPTION :
//==============================================================================
{
	friend int integrate_over_element(struct FE_element *,void *);
	public:
		// constructors
		Integrate_over_element_data(Function_variable_handle integrand_output,
			Function_size_type row,Function_size_type column,
			Function_variable_handle integrand_input,
			Function_variable_handle independent_output,
			Function_variable_handle independent_input,
			Quadrature_scheme *scheme,Matrix& result_matrix):
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			derivative_private(false),
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			first_private(true),
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			dummy_derivative_matrix(),
			result_derivative_matrix(dummy_derivative_matrix),derivative_function(0),
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			column(column),number_of_columns(0),number_of_rows(0),row(row),
			element_xi_input(0),independent_input(independent_input),
			independent_output(independent_output),integrand_input(integrand_input),
			integrand_output(integrand_output),jacobian_determinant_output(0),
			dummy_matrix(),
			result_matrix_private(result_matrix),
			scheme(scheme)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			,dummy_derivative_variables(0),
			derivative_variables(dummy_derivative_variables)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			{};
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Integrate_over_element_data(Function_variable_handle integrand_output,
			Function_size_type row,Function_size_type column,
			const std::list<Function_variable_handle>& derivative_variables,
			Function_variable_handle integrand_input,
			Function_variable_handle independent_output,
			Function_variable_handle independent_input,
			Quadrature_scheme *scheme,Derivative_matrix& result_derivative_matrix):
			derivative_private(true),first_private(true),dummy_derivative_matrix(),
			result_derivative_matrix(result_derivative_matrix),derivative_function(0),
			column(column),number_of_columns(0),number_of_rows(0),row(row),
			element_xi_input(0),independent_input(independent_input),
			independent_output(independent_output),integrand_input(integrand_input),
			integrand_output(integrand_output),jacobian_determinant_output(0),
			dummy_matrix(),
			result_matrix_private(dummy_matrix),
			scheme(scheme),
			dummy_derivative_variables(0),
			derivative_variables(derivative_variables)
			{};
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		// destructor
		~Integrate_over_element_data(){};
		bool first()
		{
			return (first_private);
		};
	private:
		// copy constructor
		Integrate_over_element_data(const Integrate_over_element_data&);
		// assignment
		Integrate_over_element_data& operator=(const Integrate_over_element_data&);
	private:
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		bool derivative_private;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		bool first_private;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Derivative_matrix dummy_derivative_matrix;
		Derivative_matrix& result_derivative_matrix;
		Function_derivatnew_handle derivative_function;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_size_type column,number_of_columns,number_of_rows,row;
		Function_variable_handle element_xi_input,independent_input,
			independent_output,integrand_input,integrand_output,
			jacobian_determinant_output;
		Matrix dummy_matrix;
		Matrix& result_matrix_private;
		Quadrature_scheme *scheme;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		const std::list<Function_variable_handle> dummy_derivative_variables;
		const std::list<Function_variable_handle>& derivative_variables;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
};

int integrate_over_element(struct FE_element *element,
	void *integrate_over_element_data_void)
//******************************************************************************
// LAST MODIFIED : 22 March 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_size_type number_of_points;
	Function_variable_handle integrand_input(0),integrand_output(0);
	int dimension,return_code;
	struct Integrate_over_element_data *data;
	Quadrature_scheme *scheme;
	Vector *points;

	return_code=0;
	if (element&&(data=(struct Integrate_over_element_data *)
		integrate_over_element_data_void)&&
		(integrand_input=data->integrand_input)&&
		(integrand_output=data->integrand_output)&&
		(scheme=data->scheme)&&FE_basis_get_dimension(
		scheme->exactly_integrated_basis,&dimension)&&(0<dimension)&&
		(dimension==get_FE_element_dimension(element))&&(0<(number_of_points=
		scheme->number_of_points))&&(points=scheme->points))
	{
		boost::intrusive_ptr< Function_matrix<Scalar> > integrand(0);
		Function_element_xi_handle element_xi(0);
		Function_size_type column,i,j,number_of_columns,number_of_rows,point_number,
			row;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_size_type k;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_variable_handle element_xi_input(0),independent_input(0),
			independent_output(0),jacobian_determinant_output(0);
		Scalar multiplier,temp_scalar;
		Vector& weights=scheme->weights;

		number_of_rows=0;
		number_of_columns=0;
		point_number=0;
		row=data->row;
		column=data->column;
		if (data->first_private)
		{
			data->first_private=false;
			if ((independent_input=data->independent_input)&&
				(independent_output=data->independent_output))
			{
				std::list<Function_variable_handle> independent_variables(1,
					independent_input);
				Function_handle integrand=Function_handle(new Function_composition(
					integrand_output,integrand_input,independent_output)),jacobian=
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					Function_handle(new Function_derivative(independent_output,
					independent_variables))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					independent_output->derivative(independent_variables)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					;
				Function_variable_handle jacobian_output;

				if (integrand&&(integrand_output=integrand->output())&&jacobian&&
					(jacobian_output=jacobian->output()))
				{
					Function_handle jacobian_determinant=Function_handle(
						new Function_matrix_determinant<Scalar>(jacobian_output));

					if (jacobian_determinant&&
						(jacobian_determinant_output=jacobian_determinant->output()))
					{
						element_xi_input=independent_input;
						return_code=1;
					}
				}
			}
			else
			{
				independent_input=0;
				element_xi_input=integrand_input;
				return_code=1;
			}
			if (return_code)
			{
				if ((element_xi=new Function_element_xi(element,points[point_number]))&&
#if defined (EVALUATE_RETURNS_VALUE)
					(integrand=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
					Function>(integrand_output->evaluate(element_xi_input,
					element_xi)))
#else // defined (EVALUATE_RETURNS_VALUE)
					(element_xi_input->set_value)(element_xi)&&
					(integrand_output->evaluate)()&&
					(integrand=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
					Function>(integrand_output->get_value()))
#endif // defined (EVALUATE_RETURNS_VALUE)
					)
				{
					multiplier=weights[point_number];
					number_of_rows=integrand->number_of_rows();
					number_of_columns=integrand->number_of_columns();
					if (jacobian_determinant_output)
					{
						boost::intrusive_ptr< Function_matrix<Scalar> >
							jacobian_determinant(0);

#if defined (EVALUATE_RETURNS_VALUE)
						if (jacobian_determinant=boost::dynamic_pointer_cast<
							Function_matrix<Scalar>,Function>(jacobian_determinant_output->
							evaluate(element_xi_input,element_xi)))
#else // defined (EVALUATE_RETURNS_VALUE)
						if ((jacobian_determinant_output->evaluate)()&&
							(jacobian_determinant=boost::dynamic_pointer_cast<Function_matrix<
							Scalar>,Function>(jacobian_determinant_output->get_value())))
#endif // defined (EVALUATE_RETURNS_VALUE)
						{
							temp_scalar=(*jacobian_determinant)(1,1);
							if (temp_scalar<0)
							{
								temp_scalar= -temp_scalar;
							}
							multiplier *= temp_scalar;
						}
						else
						{
							return_code=0;
						}
					}
					if (return_code)
					{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						if (data->derivative_private)
						{
							if ((data->derivative_function=
								boost::dynamic_pointer_cast<Function_derivatnew,Function>(
								integrand_output->derivative(data->derivative_variables)))&&
								(integrand_output=data->derivative_function->output())&&
								(integrand_output->evaluate()))
							{
								std::list<Matrix> matrices;
								std::list<Matrix>::iterator matrix_iterator,matrix_iterator_end;

								matrix_iterator=
									(data->derivative_function->derivative_matrix).begin();
								matrix_iterator_end=
									(data->derivative_function->derivative_matrix).end();
								if (0==row)
								{
									if (0==column)
									{
										while (matrix_iterator!=matrix_iterator_end)
										{
											Matrix &temp_matrix=(*matrix_iterator);
											Function_size_type
												temp_number_of_columns=temp_matrix.size2(),
												temp_number_of_rows=temp_matrix.size1();
											Matrix result_matrix(temp_number_of_rows,
												temp_number_of_columns);

											for (i=0;i<temp_number_of_rows;i++)
											{
												for (j=0;j<temp_number_of_columns;j++)
												{
													result_matrix(i,j)=multiplier*temp_matrix(i,j);
												}
											}
											matrices.push_back(result_matrix);
											++matrix_iterator;
										}
									}
									else
									{
										while (matrix_iterator!=matrix_iterator_end)
										{
											Matrix &temp_matrix=(*matrix_iterator);
											Function_size_type
												temp_number_of_columns=temp_matrix.size2(),
												temp_number_of_rows=number_of_rows;
											Matrix result_matrix(temp_number_of_rows,
												temp_number_of_columns);

											k=column-1;
											for (i=0;i<temp_number_of_rows;i++)
											{
												for (j=0;j<temp_number_of_columns;j++)
												{
													result_matrix(i,j)=multiplier*temp_matrix(k,j);
												}
												k += number_of_columns;
											}
											matrices.push_back(result_matrix);
											++matrix_iterator;
										}
									}
								}
								else
								{
									if (0==column)
									{
										while (matrix_iterator!=matrix_iterator_end)
										{
											Matrix &temp_matrix=(*matrix_iterator);
											Function_size_type
												temp_number_of_columns=temp_matrix.size2(),
												temp_number_of_rows=number_of_columns;
											Matrix result_matrix(temp_number_of_rows,
												temp_number_of_columns);

											k=(row-1)*number_of_columns;
											for (i=0;i<temp_number_of_rows;i++)
											{
												for (j=0;j<temp_number_of_columns;j++)
												{
													result_matrix(i,j)=multiplier*temp_matrix(k,j);
												}
												k++;
											}
											matrices.push_back(result_matrix);
											++matrix_iterator;
										}
									}
									else
									{
										while (matrix_iterator!=matrix_iterator_end)
										{
											Matrix &temp_matrix=(*matrix_iterator);
											Function_size_type
												temp_number_of_columns=temp_matrix.size2();
											Matrix result_matrix(1,temp_number_of_columns);

											k=(row-1)*number_of_columns+column-1;
											for (j=0;j<temp_number_of_columns;j++)
											{
												result_matrix(0,j)=multiplier*temp_matrix(k,j);
											}
											matrices.push_back(result_matrix);
											++matrix_iterator;
										}
									}
								}
								data->result_derivative_matrix=Derivative_matrix(matrices);
							}
							else
							{
								return_code=0;
							}
						}
						else
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
						{
							Matrix& result_matrix=data->result_matrix_private;

							if (0==row)
							{
								if (0==column)
								{
									result_matrix.resize(number_of_rows,number_of_columns);

									for (i=0;i<number_of_rows;i++)
									{
										for (j=0;j<number_of_columns;j++)
										{
											result_matrix(i,j)=multiplier*(*integrand)(i+1,j+1);
										}
									}
								}
								else
								{
									result_matrix.resize(number_of_rows,1);

									for (i=0;i<number_of_rows;i++)
									{
										result_matrix(i,0)=multiplier*(*integrand)(i+1,column);
									}
								}
							}
							else
							{
								if (0==column)
								{
									result_matrix.resize(1,number_of_columns);

									for (j=0;j<number_of_columns;j++)
									{
										result_matrix(0,j)=multiplier*(*integrand)(row,j+1);
									}
								}
								else
								{
									result_matrix.resize(1,1);

									result_matrix(0,0)=multiplier*(*integrand)(row,column);
								}
							}
						}
					}
				}
				else
				{
					return_code=0;
				}
				point_number++;
			}
			data->element_xi_input=element_xi_input;
			data->integrand_output=integrand_output;
			data->jacobian_determinant_output=jacobian_determinant_output;
			data->number_of_rows=number_of_rows;
			data->number_of_columns=number_of_columns;
		}
		else
		{
			element_xi_input=data->element_xi_input;
			jacobian_determinant_output=data->jacobian_determinant_output;
			number_of_rows=data->number_of_rows;
			number_of_columns=data->number_of_columns;
			return_code=1;
		}
		while (return_code&&(point_number<number_of_points))
		{
			if ((element_xi=new Function_element_xi(element,points[point_number]))&&
#if defined (EVALUATE_RETURNS_VALUE)
				(integrand_output->evaluate(element_xi_input,element_xi))
#else // defined (EVALUATE_RETURNS_VALUE)
				(element_xi_input->set_value)(element_xi)&&
				(integrand_output->evaluate)()
#endif // defined (EVALUATE_RETURNS_VALUE)
				)
			{
				multiplier=weights[point_number];
				if (jacobian_determinant_output)
				{
					boost::intrusive_ptr< Function_matrix<Scalar> >
						jacobian_determinant(0);

#if defined (EVALUATE_RETURNS_VALUE)
					if (jacobian_determinant=boost::dynamic_pointer_cast<
						Function_matrix<Scalar>,Function>(jacobian_determinant_output->
						evaluate(element_xi_input,element_xi)))
#else // defined (EVALUATE_RETURNS_VALUE)
					if ((jacobian_determinant_output->evaluate)()&&
						(jacobian_determinant=boost::dynamic_pointer_cast<
						Function_matrix<Scalar>,Function>(jacobian_determinant_output->
						get_value())))
#endif // defined (EVALUATE_RETURNS_VALUE)
					{
						temp_scalar=(*jacobian_determinant)(1,1);
						if (temp_scalar<0)
						{
							temp_scalar= -temp_scalar;
						}
						multiplier *= temp_scalar;
					}
					else
					{
						return_code=0;
					}
				}
				if (return_code)
				{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					if (data->derivative_private)
					{
						if ((data->result_derivative_matrix).size()==
							(data->derivative_function->derivative_matrix).size())
						{
							std::list<Matrix>::iterator matrix_iterator,matrix_iterator_end,
								result_matrix_iterator;

							matrix_iterator=
								(data->derivative_function->derivative_matrix).begin();
							matrix_iterator_end=
								(data->derivative_function->derivative_matrix).end();
							result_matrix_iterator=(data->result_derivative_matrix).begin();
							if (0==row)
							{
								if (0==column)
								{
									while (return_code&&(matrix_iterator!=matrix_iterator_end))
									{
										Matrix &temp_matrix=(*matrix_iterator);
										Matrix &result_matrix=(*result_matrix_iterator);
										Function_size_type
											temp_number_of_columns=result_matrix.size2(),
											temp_number_of_rows=result_matrix.size1();

										if ((temp_number_of_rows==temp_matrix.size1())&&
											(temp_number_of_columns==temp_matrix.size2()))
										{
											for (i=0;i<temp_number_of_rows;i++)
											{
												for (j=0;j<temp_number_of_columns;j++)
												{
													result_matrix(i,j) += multiplier*temp_matrix(i,j);
												}
											}
										}
										else
										{
											return_code=0;
										}
										++matrix_iterator;
										++result_matrix_iterator;
									}
								}
								else
								{
									while (return_code&&(matrix_iterator!=matrix_iterator_end))
									{
										Matrix &temp_matrix=(*matrix_iterator);
										Matrix &result_matrix=(*result_matrix_iterator);
										Function_size_type
											temp_number_of_columns=result_matrix.size2(),
											temp_number_of_rows=result_matrix.size1();

										if (
											(number_of_rows*number_of_columns==temp_matrix.size1())&&
											(temp_number_of_columns==temp_matrix.size2()))
										{
											k=column-1;
											for (i=0;i<temp_number_of_rows;i++)
											{
												for (j=0;j<temp_number_of_columns;j++)
												{
													result_matrix(i,j) += multiplier*temp_matrix(k,j);
												}
												k += number_of_columns;
											}
										}
										else
										{
											return_code=0;
										}
										++matrix_iterator;
										++result_matrix_iterator;
									}
								}
							}
							else
							{
								if (0==column)
								{
									while (return_code&&(matrix_iterator!=matrix_iterator_end))
									{
										Matrix &temp_matrix=(*matrix_iterator);
										Matrix &result_matrix=(*result_matrix_iterator);
										Function_size_type
											temp_number_of_columns=result_matrix.size2(),
											temp_number_of_rows=result_matrix.size1();

										if (
											(number_of_rows*number_of_columns==temp_matrix.size1())&&
											(temp_number_of_columns==temp_matrix.size2()))
										{
											k=(row-1)*number_of_columns;
											for (i=0;i<temp_number_of_rows;i++)
											{
												for (j=0;j<temp_number_of_columns;j++)
												{
													result_matrix(i,j) += multiplier*temp_matrix(k,j);
												}
												k++;
											}
										}
										else
										{
											return_code=0;
										}
										++matrix_iterator;
										++result_matrix_iterator;
									}
								}
								else
								{
									while (return_code&&(matrix_iterator!=matrix_iterator_end))
									{
										Matrix &temp_matrix=(*matrix_iterator);
										Matrix &result_matrix=(*result_matrix_iterator);
										Function_size_type
											temp_number_of_columns=result_matrix.size2();

										if (
											(number_of_rows*number_of_columns==temp_matrix.size1())&&
											(temp_number_of_columns==temp_matrix.size2()))
										{
											k=(row-1)*number_of_columns+column-1;
											for (j=0;j<temp_number_of_columns;j++)
											{
												result_matrix(0,j) += multiplier*temp_matrix(k,j);
											}
										}
										else
										{
											return_code=0;
										}
										++matrix_iterator;
										++result_matrix_iterator;
									}
								}
							}
						}
						else
						{
							return_code=0;
						}
					}
					else
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					{
						if ((integrand=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
							Function>(integrand_output->get_value()))&&
							(number_of_rows==integrand->number_of_rows())&&
							(number_of_columns==integrand->number_of_columns()))
						{
							Matrix& result_matrix=data->result_matrix_private;

							if (0==row)
							{
								if (0==column)
								{
									for (i=0;i<number_of_rows;i++)
									{
										for (j=0;j<number_of_columns;j++)
										{
											result_matrix(i,j) += multiplier*(*integrand)(i+1,j+1);
										}
									}
								}
								else
								{
									for (i=0;i<number_of_rows;i++)
									{
										result_matrix(i,0) += multiplier*(*integrand)(i+1,column);
									}
								}
							}
							else
							{
								if (0==column)
								{
									for (j=0;j<number_of_columns;j++)
									{
										result_matrix(0,j) += multiplier*(*integrand)(row,j+1);
									}
								}
								else
								{
									result_matrix(0,0) += multiplier*(*integrand)(row,column);
								}
							}
						}
					}
				}
			}
			else
			{
				return_code=0;
			}
			point_number++;
		}
	}

	return (return_code);
}

class Function_variable_integral : public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 21 March 2005
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_integral;
	public:
		// constructor
		Function_variable_integral(
			const boost::intrusive_ptr<Function_integral> function_integral):
			Function_variable_matrix<Scalar>(function_integral
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			,false
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			){};
		Function_variable_integral(
			const boost::intrusive_ptr<Function_integral>
			function_integral,const Function_size_type row,
			const Function_size_type column):Function_variable_matrix<Scalar>(
			function_integral,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			false,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			row,column){};
		// destructor
		~Function_variable_integral(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_integral(*this)));
		};
		// overload Function_variable::evaluate
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate()
		{
			boost::intrusive_ptr<Function_integral> function_integral;
			Function_handle result(0);

			if (function_integral=boost::dynamic_pointer_cast<
				Function_integral,Function>(function()))
			{
#if defined (BEFORE_CACHING)
				struct Integrate_over_element_data integrate_over_element_data(
					function_integral->integrand_output_private,row_private,
					column_private,function_integral->integrand_input_private,
					function_integral->independent_output_private,
					function_integral->independent_input_private,
					function_integral->scheme_private,
					function_integral->values);

				if (FE_region_for_each_FE_element(function_integral->domain_private,
					integrate_over_element,&integrate_over_element_data)&&
					!(integrate_over_element_data.first()))
				{
					if ((0==row_private)&&(0==column_private))
					{
						result=Function_handle(new Function_matrix<Scalar>(
							function_integral->values));
					}
					else
					{
						Function_size_type i,j,number_of_columns,number_of_rows;
						Matrix &result_matrix=function_integral->values;
						
						if ((0<(number_of_rows=result_matrix.size1()))&&
							(0<(number_of_columns=result_matrix.size2()))&&
							(row_private<=number_of_rows)&&
							(column_private<=number_of_columns))
						{
							if (0==row_private)
							{
								Matrix sub_matrix(number_of_rows,1);
								
								j=column_private-1;
								for (i=0;i<number_of_rows;i++)
								{
									sub_matrix(i,0)=result_matrix(i,j);
								}
								result=Function_handle(new Function_matrix<Scalar>(sub_matrix));
							}
							else
							{
								if (0==column_private)
								{
									Matrix sub_matrix(1,number_of_columns);
									
									i=row_private-1;
									for (j=0;j<number_of_columns;j++)
									{
										sub_matrix(0,j)=result_matrix(i,j);
									}
									result=Function_handle(new Function_matrix<Scalar>(
										sub_matrix));
								}
								else
								{
									Matrix sub_matrix(1,1);
									
									sub_matrix(0,0)=result_matrix(row_private-1,column_private-1);
									result=Function_handle(new Function_matrix<Scalar>(
										sub_matrix));
								}
							}
						}
					}
				}
#else // defined (BEFORE_CACHING)
				if (!(function_integral->evaluated()))
				{
					struct Integrate_over_element_data integrate_over_element_data(
						function_integral->integrand_output_private,row_private,
						column_private,function_integral->integrand_input_private,
						function_integral->independent_output_private,
						function_integral->independent_input_private,
						function_integral->scheme_private,
						function_integral->values);

					if (FE_region_for_each_FE_element(function_integral->domain_private,
						integrate_over_element,&integrate_over_element_data)&&
						!(integrate_over_element_data.first()))
					{
						function_integral->set_evaluated();
					}
				}
				if (function_integral->evaluated())
				{
					if ((0==row_private)&&(0==column_private))
					{
						result=Function_handle(new Function_matrix<Scalar>(
							function_integral->values));
					}
					else
					{
						Function_size_type i,j,number_of_columns,number_of_rows;
						Matrix &result_matrix=function_integral->values;
						
						if ((0<(number_of_rows=result_matrix.size1()))&&
							(0<(number_of_columns=result_matrix.size2()))&&
							(row_private<=number_of_rows)&&
							(column_private<=number_of_columns))
						{
							if (0==row_private)
							{
								Matrix sub_matrix(number_of_rows,1);
								
								j=column_private-1;
								for (i=0;i<number_of_rows;i++)
								{
									sub_matrix(i,0)=result_matrix(i,j);
								}
								result=Function_handle(new Function_matrix<Scalar>(sub_matrix));
							}
							else
							{
								if (0==column_private)
								{
									Matrix sub_matrix(1,number_of_columns);
									
									i=row_private-1;
									for (j=0;j<number_of_columns;j++)
									{
										sub_matrix(0,j)=result_matrix(i,j);
									}
									result=Function_handle(new Function_matrix<Scalar>(
										sub_matrix));
								}
								else
								{
									Matrix sub_matrix(1,1);
									
									sub_matrix(0,0)=result_matrix(row_private-1,column_private-1);
									result=Function_handle(new Function_matrix<Scalar>(
										sub_matrix));
								}
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
			boost::intrusive_ptr<Function_integral> function_integral;

			if (function_integral=boost::dynamic_pointer_cast<
				Function_integral,Function>(function()))
			{
#if defined (BEFORE_CACHING)
				struct Integrate_over_element_data integrate_over_element_data(
					function_integral->integrand_output_private,row_private,
					column_private,function_integral->integrand_input_private,
					function_integral->independent_output_private,
					function_integral->independent_input_private,
					function_integral->scheme_private,
					function_integral->values);

				result=false;
				if (FE_region_for_each_FE_element(function_integral->domain_private,
					integrate_over_element,&integrate_over_element_data)&&
					!(integrate_over_element_data.first()))
				{
					result=true;
				}
#else // defined (BEFORE_CACHING)
				if (!(function_integral->evaluated()))
				{
					struct Integrate_over_element_data integrate_over_element_data(
						function_integral->integrand_output_private,row_private,
						column_private,function_integral->integrand_input_private,
						function_integral->independent_output_private,
						function_integral->independent_input_private,
						function_integral->scheme_private,
						function_integral->values);

					result=false;
					if (FE_region_for_each_FE_element(function_integral->domain_private,
						integrate_over_element,&integrate_over_element_data)&&
						!(integrate_over_element_data.first()))
					{
						function_integral->set_evaluated();
						result=true;
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
			boost::intrusive_ptr<Function_integral> function_integral;
			Function_handle integrand(0),result(0);
			Function_variable_handle integrand_input(0);

			if (function_integral=boost::dynamic_pointer_cast<
				Function_integral,Function>(function()))
			{
				Function_variable_handle independent_input,independent_output;

				if ((independent_input=function_integral->independent_input_private)&&
					(independent_output=function_integral->independent_output_private))
				{
					std::list<Function_variable_handle> jacobian_independent_variables(1,
						independent_input);
					Function_handle jacobian=Function_handle(new Function_derivative(
						independent_output,jacobian_independent_variables)),
						jacobian_determinant;

					integrand=Function_handle(new Function_composition(
						function_integral->integrand_output_private,
						function_integral->integrand_input_private,independent_output));
					if (integrand&&jacobian&&(jacobian_determinant=Function_handle(
						new Function_matrix_determinant<Scalar>(jacobian->output())))&&
						(integrand=new Function_matrix_product<Scalar>(
						jacobian_determinant->output(),integrand->output())))
					{
						integrand=Function_handle(new Function_derivative(
							integrand->output(),independent_variables));
						integrand_input=independent_input;
					}
				}
				else
				{
					integrand=Function_handle(new Function_derivative(
						function_integral->integrand_output_private,independent_variables));
					integrand_input=function_integral->integrand_input_private;
				}
			}
			if (integrand&&integrand_input)
			{
				Matrix result_matrix;
				struct Integrate_over_element_data integrate_over_element_data(
					integrand->output(),0,0,
//					function_integral->integrand_output_private,0,0,
					integrand_input,Function_variable_handle(0),
					Function_variable_handle(0),function_integral->scheme_private,
					result_matrix);

				if (FE_region_for_each_FE_element(function_integral->domain_private,
					integrate_over_element,&integrate_over_element_data)&&
					!(integrate_over_element_data.first()))
				{
					if ((0==row_private)&&(0==column_private))
					{
						result=Function_handle(new Function_matrix<Scalar>(result_matrix));
					}
					else
					{
						boost::intrusive_ptr< Function_matrix<Scalar> > integrand_value=
							boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
							function_integral->integrand_output_private->get_value());
						Function_size_type number_of_columns,number_of_rows;

						if (integrand_value&&
							(0<(number_of_rows=integrand_value->number_of_rows()))&&
							(0<(number_of_columns=integrand_value->number_of_columns())))
						{
							Function_size_type i,j,k,number_of_derivatives;

							number_of_derivatives=result_matrix.size2();
							if (0==row_private)
							{
								Matrix sub_matrix(number_of_rows,number_of_derivatives);

								k=column_private-1;
								for (i=0;i<number_of_rows;i++)
								{
									for (j=0;j<number_of_derivatives;j++)
									{
										sub_matrix(i,j)=result_matrix(k,j);
									}
									k += number_of_columns;
								}
								result=Function_handle(new Function_matrix<Scalar>(sub_matrix));
							}
							else
							{
								if (0==column_private)
								{
									Matrix sub_matrix(number_of_columns,number_of_derivatives);

									k=(row_private-1)*number_of_columns;
									for (i=0;i<number_of_columns;i++)
									{
										for (j=0;j<number_of_derivatives;j++)
										{
											sub_matrix(i,j)=result_matrix(k,j);
										}
										k++;
									}
									result=Function_handle(
										new Function_matrix<Scalar>(sub_matrix));
								}
								else
								{
									Matrix sub_matrix(1,number_of_derivatives);

									k=(row_private-1)*number_of_columns+column_private-1;
									for (j=0;j<number_of_derivatives;j++)
									{
										sub_matrix(0,j)=result_matrix(k,j);
									}
									result=Function_handle(
										new Function_matrix<Scalar>(sub_matrix));
								}
							}
						}
					}
				}
			}

			return (result);
		};
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables)
		{
			return (Function_handle(new Function_derivatnew_integral(
				Function_variable_handle(this),independent_variables)));
		}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		string_handle get_string_representation()
		{
			boost::intrusive_ptr<Function_integral> function_integral=
				boost::dynamic_pointer_cast<Function_integral,Function>(function());
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				out << "integral(";
				if (function_integral->integrand_output_private)
				{
					out << *(function_integral->integrand_output_private->
						get_string_representation());
				}
				out << "[";
				if (0==row_private)
				{
					out << "1:" << number_of_rows();
				}
				else
				{
					out << row_private;
				}
				out << ",";
				if (0==column_private)
				{
					out << "1:" << number_of_columns();
				}
				else
				{
					out << column_private;
				}
				out << "]";
				if (function_integral->independent_output_private)
				{
					out << "," << *(function_integral->independent_output_private->
						get_string_representation());
				}
				out << ";";
				if (function_integral->domain_private)
				{
					//???DB.  Don't know how to get region name
					out << "domain";
				}
				out << ")";
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_size_type number_differentiable()
		{
			boost::intrusive_ptr<Function_integral> function_integral=
				boost::dynamic_pointer_cast<Function_integral,Function>(function());
			Function_size_type result;

			result=0;
			if (function_integral)
			{
				result=
					function_integral->integrand_output_private->number_differentiable();
			}

			return (result);
		}
	private:
		// copy constructor
		Function_variable_integral(
			const Function_variable_integral& variable_integral):
			Function_variable_matrix<Scalar>(variable_integral){};
};

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_integral
// ----------------------------------

Function_derivatnew_integral::Function_derivatnew_integral(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables)
//******************************************************************************
// LAST MODIFIED : 28 January 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (!(boost::dynamic_pointer_cast<Function_variable_integral,
		Function_variable>(dependent_variable)&&
		boost::dynamic_pointer_cast<Function_integral,
		Function>(dependent_variable->function())))
	{
		throw Function_derivatnew_integral::Construction_exception();
	}
}

Function_derivatnew_integral::~Function_derivatnew_integral(){}
//******************************************************************************
// LAST MODIFIED : 27 January 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_derivatnew_integral::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 27 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);

	if (!evaluated())
	{
		Function_integral_handle function_integral;

		if (boost::dynamic_pointer_cast<Function_variable_integral,
			Function_variable>(dependent_variable)&&(function_integral=
			boost::dynamic_pointer_cast<Function_integral,Function>(
			dependent_variable->function())))
		{
			//???DB.  To be done
		}
	}
	if (evaluated())
	{
		result=get_value(atomic_variable);
	}

	return (result);
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_derivatnew_integral::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 30 March 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result(true);

	if (equivalent(Function_handle(this),atomic_variable->function())&&
		!evaluated())
	{
		boost::intrusive_ptr<Function_integral> function_integral;
		boost::intrusive_ptr<Function_variable_integral> variable_integral;
		Function_handle integrand(0);
		Function_variable_handle integrand_input(0);

		result=false;
		if ((variable_integral=boost::dynamic_pointer_cast<
			Function_variable_integral,Function_variable>(dependent_variable))&&
			(function_integral=boost::dynamic_pointer_cast<Function_integral,
			Function>(dependent_variable->function())))
		{
			Function_variable_handle independent_input,independent_output;

			if ((independent_input=function_integral->independent_input_private)&&
				(independent_output=function_integral->independent_output_private))
			{
				std::list<Function_variable_handle> jacobian_independent_variables(1,
					independent_input);
				Function_handle jacobian=independent_output->derivative(
					jacobian_independent_variables),jacobian_determinant;

				integrand=Function_handle(new Function_composition(
					function_integral->integrand_output_private,
					function_integral->integrand_input_private,independent_output));
				if (integrand&&jacobian&&(jacobian_determinant=Function_handle(
					new Function_matrix_determinant<Scalar>(jacobian->output())))&&
					(integrand=new Function_matrix_product<Scalar>(
					jacobian_determinant->output(),integrand->output())))
				{
					integrand=integrand->output()->derivative(independent_variables);
					integrand_input=independent_input;
				}
			}
			else
			{
				integrand=function_integral->integrand_output_private->derivative(
					independent_variables);
				integrand_input=function_integral->integrand_input_private;
			}
		}
		if (integrand&&integrand_input)
		{
			struct Integrate_over_element_data integrate_over_element_data(
				function_integral->integrand_output_private,variable_integral->row(),
				variable_integral->column(),independent_variables,
				function_integral->integrand_input_private,
				function_integral->independent_output_private,
				function_integral->independent_input_private,
				function_integral->scheme_private,derivative_matrix);

			if (FE_region_for_each_FE_element(function_integral->domain_private,
				integrate_over_element,&integrate_over_element_data)&&
				!(integrate_over_element_data.first()))
			{
				set_evaluated();
				result=true;
			}
		}
	}

	return (result);
}
#endif // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)


// global classes
// ==============

// class Function_integral
// -----------------------

ublas::matrix<Scalar,ublas::column_major>
	Function_integral::constructor_values(0,0);

Function_integral::Function_integral(
	const Function_variable_handle& integrand_output,
	const Function_variable_handle& integrand_input,
	const Function_variable_handle& independent_output,
	const Function_variable_handle& independent_input,
	struct Cmiss_region *domain,std::string quadrature_scheme):
	Function_matrix<Scalar>(Function_integral::constructor_values),
	independent_input_private(independent_input),
	independent_output_private(independent_output),
	integrand_input_private(integrand_input),
	integrand_output_private(integrand_output),domain_private(0),scheme_private(0)
//******************************************************************************
// LAST MODIFIED : 3 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (domain&&(domain_private=Cmiss_region_get_FE_region(domain)))
	{
		try
		{
			scheme_private=new Quadrature_scheme(domain_private,quadrature_scheme);
			if (scheme_private)
			{
				if (independent_output_private)
				{
					independent_output_private->add_dependent_function(this);
				}
				if (integrand_output_private)
				{
					integrand_output_private->add_dependent_function(this);
				}
				// An object that is partially constructed will only have constructors
				//   called for its fully constructed sub-objects.  So the ACCESS
				//   shouldn't be done until all checks have been made, otherwise
				//   the DEACCESS (in destructor) won't be done
				ACCESS(FE_region)(domain_private);
			}
			else
			{
				throw Function_integral::Construction_exception();
			}
		}
		catch (Quadrature_scheme::Construction_exception)
		{
			throw Function_integral::Construction_exception();
		}
	}
	else
	{
		throw Function_integral::Construction_exception();
	}
}

Function_integral::~Function_integral()
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	delete scheme_private;
	DEACCESS(FE_region)(&domain_private);
#if defined (CIRCULAR_SMART_POINTERS)
#else // defined (CIRCULAR_SMART_POINTERS)
	if (independent_output_private)
	{
		independent_output_private->remove_dependent_function(this);
	}
	if (integrand_output_private)
	{
		integrand_output_private->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

string_handle Function_integral::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 28 October 2004
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	string_handle return_string(0);

	if (return_string=new std::string)
	{
		std::ostringstream out;

		out << "integral(";
		if (integrand_output_private)
		{
			out << *(integrand_output_private->get_string_representation());
		}
		if (independent_output_private)
		{
			out << "," << *(independent_output_private->get_string_representation());
		}
		out << ";";
		if (domain_private)
		{
			//???DB.  Don't know how to get region name
			out << "domain";
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_integral::input()
//******************************************************************************
// LAST MODIFIED : 3 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle result(0),temp_independent(0),temp_integrand(0);

	if (integrand_output_private)
	{
		if (integrand_input_private)
		{
			temp_integrand=Function_variable_handle(new Function_variable_exclusion(
				(integrand_output_private->function())->input(),
				integrand_input_private));
		}
		else
		{
			temp_integrand=(integrand_output_private->function)()->input();
		}
	}
	if (independent_output_private)
	{
		if (independent_input_private)
		{
			temp_independent=Function_variable_handle(new Function_variable_exclusion(
				(independent_output_private->function)()->input(),
				independent_input_private));
		}
		else
		{
			temp_independent=(independent_output_private->function)()->input();
		}
	}
	if (temp_integrand)
	{
		if (temp_independent)
		{
			result=Function_variable_handle(new Function_variable_union(
				temp_integrand,temp_independent));
		}
		else
		{
			result=temp_integrand;
		}
	}
	else
	{
		result=temp_independent;
	}

	return (result);
}

Function_variable_handle Function_integral::output()
//******************************************************************************
// LAST MODIFIED : 3 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_integral(
		boost::intrusive_ptr<Function_integral>(this))));
}

bool Function_integral::operator==(const Function& function) const
//******************************************************************************
// LAST MODIFIED : 28 October 2004
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
			const Function_integral& function_integral=
				dynamic_cast<const Function_integral&>(function);

			result=equivalent(integrand_output_private,
				function_integral.integrand_output_private)&&
				equivalent(integrand_input_private,
				function_integral.integrand_input_private)&&
				equivalent(independent_output_private,
				function_integral.independent_output_private)&&
				equivalent(independent_input_private,
				function_integral.independent_input_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_integral::evaluate(Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 14 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr<Function_variable_integral> atomic_variable_integral;
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)

	if (this&&(atomic_variable_integral=boost::dynamic_pointer_cast<
		Function_variable_integral,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_integral->function())&&
		(0<atomic_variable_integral->row())&&
		(0<atomic_variable_integral->column()))
	{
		result=(atomic_variable_integral->evaluate)();
	}

	return (result);
}

bool Function_integral::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 28 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_matrix<Scalar> > derivative_value;
	boost::intrusive_ptr<Function_variable_integral> atomic_variable_integral;

	result=false;
	if (this&&(atomic_variable_integral=boost::dynamic_pointer_cast<
		Function_variable_integral,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_integral->function())&&
		(0<atomic_variable_integral->row())&&
		(0<atomic_variable_integral->column())&&
		(0<atomic_independent_variables.size()))
	{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle derivative_function;
		Function_variable_handle derivative_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_value=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(atomic_variable_integral->
			evaluate_derivative(atomic_independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(atomic_variable_integral->derivative(
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

	return (result);
}

bool Function_integral::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr<Function_variable_integral> atomic_variable_integral;
	boost::intrusive_ptr< Function_variable_value_specific<Scalar> > value_type;
	Function_handle function;

	result=false;
	if ((atomic_variable_integral=boost::dynamic_pointer_cast<
		Function_variable_integral,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_integral->function())&&
		atomic_value&&(atomic_value->value())&&(value_type=
		boost::dynamic_pointer_cast<Function_variable_value_specific<Scalar>,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_type->set(values((atomic_variable_integral->row())-1,
			(atomic_variable_integral->column())-1),atomic_value);
	}
	if (result)
	{
		set_not_evaluated();
	}
	else
	{
		if (integrand_output_private&&
			(function=integrand_output_private->function()))
		{
			result=function->set_value(atomic_variable,atomic_value);
		}
		if (independent_output_private&&
			(function=independent_output_private->function()))
		{
			if (function->set_value(atomic_variable,atomic_value))
			{
				result=true;
			}
		}
	}

	return (result);
}

Function_handle Function_integral::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 3 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr<Function_variable_integral> atomic_variable_integral;
	Function_handle function,result;
	ublas::matrix<Scalar,ublas::column_major> result_matrix(1,1);

	result=0;
	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_integral=boost::dynamic_pointer_cast<
		Function_variable_integral,Function_variable>(atomic_variable))&&
		(atomic_variable_integral->get_entry)(result_matrix(0,0)))
	{
		result=Function_handle(new Function_matrix<Scalar>(result_matrix));
	}
	if (!result)
	{
		if (integrand_output_private&&
			(function=integrand_output_private->function()))
		{
			result=function->get_value(atomic_variable);
		}
		if (!result)
		{
			if (independent_output_private&&
				(function=independent_output_private->function()))
			{
				result=function->get_value(atomic_variable);
			}
		}
	}

	return (result);
}

Function_integral::Function_integral(
	const Function_integral& function_integral):
	Function_matrix<Scalar>(function_integral),
	independent_input_private(function_integral.independent_input_private),
	independent_output_private(function_integral.independent_output_private),
	integrand_input_private(function_integral.integrand_input_private),
	integrand_output_private(function_integral.integrand_output_private),
	domain_private(function_integral.domain_private),
	scheme_private(function_integral.scheme_private)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (independent_output_private)
	{
		independent_output_private->add_dependent_function(this);
	}
	if (integrand_output_private)
	{
		integrand_output_private->add_dependent_function(this);
	}
}

Function_integral& Function_integral::operator=(
	const Function_integral& function_integral)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	if (function_integral.integrand_output_private)
	{
		function_integral.integrand_output_private->add_dependent_function(this);
	}
	if (integrand_output_private)
	{
		integrand_output_private->remove_dependent_function(this);
	}
	integrand_output_private=function_integral.integrand_output_private;
	integrand_input_private=function_integral.integrand_input_private;
	if (function_integral.independent_output_private)
	{
		function_integral.independent_output_private->add_dependent_function(this);
	}
	if (independent_output_private)
	{
		independent_output_private->remove_dependent_function(this);
	}
	independent_output_private=function_integral.independent_output_private;
	independent_input_private=function_integral.independent_input_private;
	if (domain_private)
	{
		DEACCESS(FE_region)(&domain_private);
	}
	if (function_integral.domain_private)
	{
		domain_private=ACCESS(FE_region)(function_integral.domain_private);
	}
	delete scheme_private;
	if (function_integral.scheme_private)
	{
		scheme_private=new Quadrature_scheme(*(function_integral.scheme_private));
	}

	return (*this);
}
