//******************************************************************************
// FILE : function_integral.hpp
//
// LAST MODIFIED : 4 November 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_INTEGRAL_HPP__)
#define __FUNCTION_INTEGRAL_HPP__

#include <list>
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_matrix.hpp"

class Quadrature_scheme;

class Function_integral : public Function_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 4 November 2004
//
// DESCRIPTION :
// An integral of another function.
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	friend class Function_variable_integral;
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_integral(const Function_variable_handle& integrand_output,
			const Function_variable_handle& integrand_input,
			const Function_variable_handle& independent_output,
			const Function_variable_handle& independent_input,
			struct Cmiss_region *domain,std::string quadrature_scheme);
		// destructor
		~Function_integral();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	private:
		Function_handle evaluate(Function_variable_handle atomic_variable);
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
		Function_handle get_value(Function_variable_handle atomic_variable);
	private:
		// copy constructor
		Function_integral(const Function_integral&);
		// assignment
		Function_integral& operator=(const Function_integral&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle independent_input_private,
			independent_output_private,integrand_input_private,
			integrand_output_private;
		static ublas::matrix<Scalar,ublas::column_major> constructor_values;
		struct FE_region *domain_private;
		Quadrature_scheme *scheme_private;
};

#endif /* !defined (__FUNCTION_INTEGRAL_HPP__) */
