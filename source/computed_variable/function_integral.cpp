//******************************************************************************
// FILE : function_integral.cpp
//
// LAST MODIFIED : 5 November 2004
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

// class Function_variable_integral
// --------------------------------

struct Integrate_over_element_data
{
	friend int integrate_over_element(struct FE_element *,void *);
	public:
		// constructor
		Integrate_over_element_data(Function_variable_handle integrand_output,
			Function_variable_handle integrand_input,
			Function_variable_handle independent_output,
			Function_variable_handle independent_input,
			Quadrature_scheme *scheme,Matrix& result_matrix):first_private(true),
			independent_input(independent_input),
			independent_output(independent_output),integrand_input(integrand_input),
			integrand_output(integrand_output),scheme(scheme),
			result_matrix_private(result_matrix){};
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
		bool first_private;
		Function_variable_handle independent_input,independent_output,
			integrand_input,integrand_output;
		Quadrature_scheme *scheme;
		Matrix& result_matrix_private;
};

int integrate_over_element(struct FE_element *element,
	void *integrate_over_element_data_void)
//******************************************************************************
// LAST MODIFIED : 3 November 2004
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
		Function_variable_handle element_xi_input(0),independent_input(0),
			independent_output(0),jacobian_determinant_output(0);
		Vector& weights=scheme->weights;

		if ((independent_input=data->independent_input)&&
			(independent_output=data->independent_output))
		{
			std::list<Function_variable_handle> independent_variables(1,
				independent_input);
			Function_handle integrand=Function_handle(new Function_composition(
				integrand_output,integrand_input,independent_input)),
				jacobian=Function_handle(new Function_derivative(independent_output,
				independent_variables));
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
			Function_element_xi_handle element_xi(0);
			boost::intrusive_ptr< Function_matrix<Scalar> > integrand;
			Function_size_type i,j,number_of_columns,number_of_rows,point_number;
			Scalar multiplier,temp_scalar;
			Matrix& result_matrix=data->result_matrix_private;

			number_of_rows=0;
			number_of_columns=0;
			point_number=0;
			if (data->first_private)
			{
				data->first_private=false;
				if ((element_xi=new Function_element_xi(element,points[point_number]))&&
					(integrand=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
					Function>(integrand_output->evaluate(element_xi_input,
					element_xi))))
				{
					multiplier=weights[point_number];
					number_of_rows=integrand->number_of_rows();
					number_of_columns=integrand->number_of_columns();
					if (jacobian_determinant_output)
					{
						boost::intrusive_ptr< Function_matrix<Scalar> >
							jacobian_determinant(0);

						if (jacobian_determinant=boost::dynamic_pointer_cast<
							Function_matrix<Scalar>,Function>(jacobian_determinant_output->
							evaluate(element_xi_input,element_xi)))
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
						result_matrix.resize(number_of_rows,number_of_columns);

						for (i=0;i<number_of_rows;i++)
						{
							for (j=0;j<number_of_columns;j++)
							{
								result_matrix(i,j)=multiplier*(*integrand)(i+1,j+1);
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
			else
			{
				number_of_rows=result_matrix.size1();
				number_of_columns=result_matrix.size2();
			}
			while (return_code&&(point_number<number_of_points))
			{
				if ((element_xi=new Function_element_xi(element,points[point_number]))&&
					(integrand=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
					Function>(integrand_output->evaluate(element_xi_input,
					element_xi)))&&(number_of_rows==integrand->number_of_rows())&&
					(number_of_columns==integrand->number_of_columns()))
				{
					multiplier=weights[point_number];
					if (jacobian_determinant_output)
					{
						boost::intrusive_ptr< Function_matrix<Scalar> >
							jacobian_determinant(0);

						if (jacobian_determinant=boost::dynamic_pointer_cast<
							Function_matrix<Scalar>,Function>(jacobian_determinant_output->
							evaluate(element_xi_input,element_xi)))
						{
							multiplier=(*jacobian_determinant)(1,1);
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
						for (i=0;i<number_of_rows;i++)
						{
							for (j=0;j<number_of_columns;j++)
							{
								result_matrix(i,j) += multiplier*(*integrand)(i+1,j+1);
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
	}

	return (return_code);
}

class Function_variable_integral : public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 3 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_integral;
	public:
		// constructor
		Function_variable_integral(
			const boost::intrusive_ptr<Function_integral> function_integral):
			Function_variable_matrix<Scalar>(function_integral){};
		Function_variable_integral(
			const boost::intrusive_ptr<Function_integral>
			function_integral,const Function_size_type row,
			const Function_size_type column):Function_variable_matrix<Scalar>(
			function_integral,row,column){};
		// destructor
		~Function_variable_integral(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_integral(*this)));
		};
		Function_handle evaluate()
		{
			boost::intrusive_ptr<Function_integral> function_integral;
			Function_handle result(0);

			if (function_integral=boost::dynamic_pointer_cast<
				Function_integral,Function>(function()))
			{
				struct Integrate_over_element_data integrate_over_element_data(
					function_integral->integrand_output_private,
					function_integral->integrand_input_private,
					function_integral->independent_output_private,
					function_integral->independent_input_private,
					function_integral->scheme_private,
					function_integral->values);

				if (FE_region_for_each_FE_element(function_integral->domain_private,
					integrate_over_element,&integrate_over_element_data)&&
					!(integrate_over_element_data.first()))
				{
					result=Function_handle(new Function_matrix<Scalar>(
						function_integral->values));
				}
			}

			return (result);
		};
		Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables)
		{
			boost::intrusive_ptr<Function_integral> function_integral;
			Function_handle integrand(0),result(0);

			if ((function_integral=boost::dynamic_pointer_cast<
				Function_integral,Function>(function()))&&
				(integrand=Function_handle(new Function_derivative(
				function_integral->integrand_output_private,independent_variables))))
			{
				Matrix result_matrix;
				struct Integrate_over_element_data integrate_over_element_data(
					function_integral->integrand_output_private,
					function_integral->integrand_input_private,
					function_integral->independent_output_private,
					function_integral->independent_input_private,
					function_integral->scheme_private,result_matrix);

				if (FE_region_for_each_FE_element(function_integral->domain_private,
					integrate_over_element,&integrate_over_element_data)&&
					!(integrate_over_element_data.first()))
				{
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
			}

			return (result);
		};
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
	private:
		// copy constructor
		Function_variable_integral(
			const Function_variable_integral& variable_integral):
			Function_variable_matrix<Scalar>(variable_integral){};
};


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
// LAST MODIFIED : 5 November 2004
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
// LAST MODIFIED : 3 November 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	delete scheme_private;
	DEACCESS(FE_region)(&domain_private);
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

Function_handle Function_integral::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 3 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr<Function_variable_integral> atomic_variable_integral;
	Function_handle result(0);

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
// LAST MODIFIED : 3 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr<Function_variable_integral> atomic_variable_integral;

	result=false;
	if (this&&(atomic_variable_integral=boost::dynamic_pointer_cast<
		Function_variable_integral,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_integral->function())&&
		(0<atomic_variable_integral->row())&&
		(0<atomic_variable_integral->column())&&
		(0<atomic_independent_variables.size()))
	{
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_matrix=
			boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			atomic_variable_integral->evaluate_derivative(
			atomic_independent_variables));

		if (derivative_matrix)
		{
			result=true;
			derivative=(*derivative_matrix)(1,1);
		}
	}

	return (result);
}

bool Function_integral::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 3 November 2004
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
	if (!result)
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
// LAST MODIFIED : 3 November 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_integral& Function_integral::operator=(
	const Function_integral& function_integral)
//******************************************************************************
// LAST MODIFIED : 3 November 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	integrand_output_private=function_integral.integrand_output_private;
	integrand_input_private=function_integral.integrand_input_private;
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
	//???DB.  Need more memory management for scheme_private
	delete scheme_private;
	if (function_integral.scheme_private)
	{
		scheme_private=new Quadrature_scheme(*(function_integral.scheme_private));
	}

	return (*this);
}
