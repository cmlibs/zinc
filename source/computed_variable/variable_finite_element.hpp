//******************************************************************************
// FILE : variable_finite_element.hpp
//
// LAST MODIFIED : 12 November 2003
//
// DESCRIPTION :
// Finite element types - element/xi and finite element field.
//==============================================================================
#if !defined (__VARIABLE_FINITE_ELEMENT_HPP__)
#define __VARIABLE_FINITE_ELEMENT_HPP__

#include <list>
#include "computed_variable/variable.hpp"
extern "C"
{
#include "finite_element/finite_element.h"
}

class Variable_element_xi : public Variable
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// An identity variable whose input/output is element/xi.
//==============================================================================
{
	friend class Variable_finite_element;
	public:
		// constructor
		Variable_element_xi(struct FE_element *element,const Vector& xi);
		// copy constructor
		Variable_element_xi(const Variable_element_xi&);
		// assignment
		Variable_element_xi& operator=(const Variable_element_xi&);
		// destructor
		~Variable_element_xi();
		// get the number of scalars in the result
		Variable_size_type size();
		// get the scalars in the result
		Vector *scalars();
		// input specifiers
		Variable_input_handle input_element_xi();
		Variable_input_handle input_element();
		Variable_input_handle input_xi();
		Variable_input_handle input_xi(Variable_size_type);
		Variable_input_handle input_xi(
			const boost::numeric::ublas::vector<Variable_size_type>);
	private:
		Variable_handle evaluate_local();
		void evaluate_derivative_local(Matrix& matrix,
			std::list<Variable_input_handle>& independent_variables);
		Variable_handle get_input_value_local(const Variable_input_handle& input);
		int set_input_value_local(const Variable_input_handle& input,
			const Variable_handle& value);
		string_handle get_string_representation_local();
	private:
		Vector xi;
		struct FE_element *element;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_element_xi> Variable_element_xi_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_element_xi> Variable_element_xi_handle;
#else
typedef Variable_element_xi * Variable_element_xi_handle;
#endif

class Variable_finite_element : public Variable
//******************************************************************************
// LAST MODIFIED : 12 November 2003
//
// DESCRIPTION :
// A variable for a finite element interpolation field.
//==============================================================================
{
	friend class Variable_input_nodal_values;
	public:
		// constructor
		Variable_finite_element(struct FE_field *field);
		Variable_finite_element(struct FE_field *field,std::string component_name);
		Variable_finite_element(struct FE_field *field,int component_number);
			// component_number 1 is the first component
		// copy constructor
		Variable_finite_element(const Variable_finite_element&);
		// assignment
		Variable_finite_element& operator=(const Variable_finite_element&);
		// destructor
		~Variable_finite_element();
		// get the number of scalars in the result
		Variable_size_type size();
		// get the scalars in the result
		Vector *scalars();
		// input specifiers
		Variable_input_handle input_element_xi();
		Variable_input_handle input_element();
		Variable_input_handle input_xi();
		Variable_input_handle input_xi(Variable_size_type);
		Variable_input_handle input_xi(
			const boost::numeric::ublas::vector<Variable_size_type>);
		Variable_input_handle input_nodal_values();
		Variable_input_handle input_nodal_values(struct FE_node *node,
			enum FE_nodal_value_type value_type,int version);
		Variable_input_handle input_nodal_values(struct FE_node *node);
		Variable_input_handle input_nodal_values(
			enum FE_nodal_value_type value_type);
		Variable_input_handle input_nodal_values(int version);
	private:
		Variable_handle evaluate_local();
		void evaluate_derivative_local(Matrix& matrix,
			std::list<Variable_input_handle>& independent_variables);
		Variable_handle get_input_value_local(const Variable_input_handle& input);
		int set_input_value_local(const Variable_input_handle& input,
			const Variable_handle& value);
		string_handle get_string_representation_local();
	private:
		int component_number;
		Scalar time;
		struct FE_element *element;
		struct FE_field *field;
		struct FE_node *node;
		Vector xi;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_finite_element>
	Variable_finite_element_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_finite_element>
	Variable_finite_element_handle;
#else
typedef Variable_finite_element * Variable_finite_element_handle;
#endif

#endif /* !defined (__VARIABLE_FINITE_ELEMENT_HPP__) */
