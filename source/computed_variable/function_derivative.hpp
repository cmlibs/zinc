//******************************************************************************
// FILE : function_derivative.hpp
//
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_DERIVATIVE_HPP__)
#define __FUNCTION_DERIVATIVE_HPP__

#include <list>
#include "computed_variable/derivative_matrix.hpp"
#include "computed_variable/function.hpp"
#if defined (BEFORE_CACHING)
#else // defined (BEFORE_CACHING)
#include "computed_variable/function_matrix.hpp"
#endif // defined (BEFORE_CACHING)

class Function_derivative;

typedef boost::intrusive_ptr<Function_derivative> Function_derivative_handle;

#if defined (BEFORE_CACHING)
class Function_derivative : public Function
#else // defined (BEFORE_CACHING)
class Function_derivative : public Function_matrix<Scalar>
#endif // defined (BEFORE_CACHING)
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
// A derivative of another function.
//
// No storage for result and so can't be inverted.
// ???DB.  Could compose with a Function_matrix and then would be invertable?
//==============================================================================
{
	friend class Function_variable_derivative;
	friend class Function_variable_iterator_representation_atomic_derivative;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivative(const Function_variable_handle& dependent_variable,
			std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivative();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
		Function_handle get_value(Function_variable_handle atomic_variable);
	private:
		// copy constructor
		Function_derivative(const Function_derivative&);
		// assignment
		Function_derivative& operator=(const Function_derivative&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle dependent_variable_private;
#if defined (BEFORE_CACHING)
#else // defined (BEFORE_CACHING)
		static ublas::matrix<Scalar,ublas::column_major> constructor_values;
#endif // defined (BEFORE_CACHING)
		std::list<Function_variable_handle> independent_variables_private;
};

class Function_derivatnew;

typedef boost::intrusive_ptr<Function_derivatnew> Function_derivatnew_handle;

class Function_derivatnew : public Function
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
// A derivative of another function.
//==============================================================================
{
	friend class Function_variable_derivatnew;
	friend class Function_variable_iterator_representation_atomic_derivatnew;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew(const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
		Function_handle get_value(Function_variable_handle atomic_variable);
	// additional
	public:
		// the matrix for the actual derivative
		Function_variable_handle matrix(
			std::list<Function_variable_handle>& partial_independent_variables);
	private:
		// copy constructor
		Function_derivatnew(const Function_derivatnew&);
		// assignment
		Function_derivatnew& operator=(const Function_derivatnew&);
		// equality
		bool operator==(const Function&) const;
	public:
		//???DB.  Don't want public, but alternative is a method that returns a
		//  reference?
		Derivative_matrix derivative_matrix;
	protected:
		Function_variable_handle dependent_variable;
		std::list<Function_variable_handle> independent_variables;
};

#endif /* !defined (__FUNCTION_DERIVATIVE_HPP__) */
