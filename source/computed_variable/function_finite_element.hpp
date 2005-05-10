//******************************************************************************
// FILE : function_finite_element.hpp
//
// LAST MODIFIED : 8 March 2005
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_FINITE_ELEMENT_HPP__)
#define __FUNCTION_FINITE_ELEMENT_HPP__

#include <list>
#include <utility>

extern "C"
{
#include "finite_element/finite_element.h"
#include "region/cmiss_region.h"
}
#include "computed_variable/function.hpp"
#include "computed_variable/function_variable.hpp"

//???DB.  Testing.
//???DB.  Function_derivatnew_finite_element doesn't currently handle
//  composite independent variables.  Would have to fall back to
//  Function_derivatnew.  End up slower?
#define USE_Function_derivatnew_finite_element

class Function_element : public Function
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
// An identity function whose input/output is and element.
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_element(struct FE_element *element);
		// destructor
		~Function_element();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	// additional
	public:
		// return the dimension of the element
		Function_size_type dimension();
		// return the element.  NB.  The calling program should use
		//   ACCESS(FE_element) and DEACCESS(FE_element) to manage the lifetime of
		//   the returned element
		struct FE_element* element_value();
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
		Function_element(const Function_element&);
		// assignment
		Function_element& operator=(const Function_element&);
		// equality
		bool operator==(const Function&) const;
	private:
		struct FE_element *element_private;
};

typedef boost::intrusive_ptr<Function_element> Function_element_handle;

class Function_element_xi : public Function
//******************************************************************************
// LAST MODIFIED : 8 March 2005
//
// DESCRIPTION :
// An identity function whose input/output is element/xi.
//==============================================================================
{
	friend class Function_finite_element;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// for construction exception
		class Invalid_element_xi {};
		// constructor
		Function_element_xi(struct FE_element *element,const Vector& xi);
		// destructor
		~Function_element_xi();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	// additional
	public:
		// variables
		Function_variable_handle
			element(),
			element_xi(),
			xi(),
			xi(Function_size_type);
		// return the dimension of the element and the number of xi
		Function_size_type number_of_xi();
		// return the element.  NB.  The calling program should use
		//   ACCESS(FE_element) and DEACCESS(FE_element) to manage the lifetime of
		//   the returned element
		struct FE_element* element_value();
		// return the xi value 1<=index<=number_of_xi
		Scalar xi_value(Function_size_type index);
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
		Function_element_xi(const Function_element_xi&);
		// assignment
		Function_element_xi& operator=(const Function_element_xi&);
		// equality
		bool operator==(const Function&) const;
	private:
		struct FE_element *element_private;
		Vector xi_private;
};

typedef boost::intrusive_ptr<Function_element_xi> Function_element_xi_handle;

class Function_finite_element;
typedef boost::intrusive_ptr<Function_finite_element>
	Function_finite_element_handle;

class Function_finite_element : public Function
//******************************************************************************
// LAST MODIFIED : 17 February 2005
//
// DESCRIPTION :
// A function for a finite element interpolation field.
//
//???DB.  Make element_private and xi_private a Function_element_xi for
//  evaluate, evaluate_derivative and set_value?
//==============================================================================
{
	friend class Function_variable_matrix_components;
	friend class Function_variable_matrix_nodal_values;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	friend class Function_derivatnew_finite_element;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	public:
		// constructor
		Function_finite_element(struct FE_field *field);
		// destructor
		~Function_finite_element();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	// additional
	public:
		// variables
		Function_variable_handle
			component(std::string component_name),
			component(Function_size_type component_number),
				// component_number 1 is the first component
			element(),
			element_xi(),
			nodal_values(),
			nodal_values(std::string component_name,struct FE_node *node,
				enum FE_nodal_value_type value_type,Function_size_type version,
				struct FE_time_sequence *time_sequence),
			nodal_values(Function_size_type component_number,struct FE_node *node,
				enum FE_nodal_value_type value_type,Function_size_type version,
				struct FE_time_sequence *time_sequence),
			// use nodal_values_component instead of overloading nodal_values to
			//   prevent same argument signature as for version
			nodal_values_component(std::string component_name),
			nodal_values_component(Function_size_type component_number),
			nodal_values(struct FE_node *node),
			nodal_values(enum FE_nodal_value_type value_type),
			nodal_values(Function_size_type version),
			time(),
			xi(),
			xi(Function_size_type);
		// return the number of components
		Function_size_type number_of_components() const;
		// return the region that the field is defined for
		// NB.  The calling program should use ACCESS(Cmiss_region) and
		//   DEACCESS(Cmiss_region) to manage the lifetime of the returned region
		struct Cmiss_region *region() const;
		// return the number of versions for the component at the node
		Function_size_type number_of_versions(
			Function_size_type component_number,struct FE_node *node) const;
		// return the number of derivatives for the component at the node
		Function_size_type number_of_derivatives(
			Function_size_type component_number,struct FE_node *node) const;
	   // Return the time sequence at the node.
	   struct FE_time_sequence *time_sequence(struct FE_node *node) const;
		// return the (1+number_of_derivatives) nodal value types for the component
		// at the node
		// NB.  The calling program should DEALLOCATE the returned array when it is
		//   no longer needed
		enum FE_nodal_value_type *nodal_value_types(
			Function_size_type component_number,struct FE_node *node) const;
		// return true and get the value if exactly one nodal value is specified,
		//   otherwise return false
		bool get_nodal_value(Function_size_type component_number,
			struct FE_node *node,enum FE_nodal_value_type value_type,
			Function_size_type version,Scalar time,Scalar& value);
		// return true and set the value if exactly one nodal value is specified,
		//   otherwise return false
		bool set_nodal_value(Function_size_type component_number,
			struct FE_node *node,enum FE_nodal_value_type value_type,
			Function_size_type version,Scalar time,Scalar value);
		// return the dimension of the element and the number of xi
		Function_size_type number_of_xi() const;
		// return the element.  NB.  The calling program should use
		//   ACCESS(FE_element) and DEACCESS(FE_element) to manage the lifetime of
		//   the returned element
		struct FE_element* element_value() const;
		// return the time value
		Scalar time_value() const;
		// return the xi value 1<=index<=number_of_xi
		Scalar xi_value(Function_size_type index) const;
		// get the component value 1<=number<=number_of_components
		bool component_value(Function_size_type number,Scalar& value) const;
	   // define the field represented by this function on the given node
	   int define_on_Cmiss_node(struct FE_node *node,
		   struct FE_time_sequence *fe_time_sequence, 
	      struct FE_node_field_creator *fe_node_field_creator);
 	   // defines a tensor product basis on element for the field represented by
	   // this function.
	   int define_tensor_product_basis_on_element(
			struct FE_element *element, int dimension, enum FE_basis_type basis_type);
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (USE_Function_derivatnew_finite_element)
		bool evaluate_derivative_matrix(Function_size_type component_number,
			std::list<Function_variable_handle>& independent_variables,
			Matrix& matrix);
#endif // defined (USE_Function_derivatnew_finite_element)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
		Function_handle get_value(Function_variable_handle atomic_variable);
	private:
		// copy constructor
		Function_finite_element(const Function_finite_element&);
		// assignment
		Function_finite_element& operator=(const Function_finite_element&);
		// equality
		bool operator==(const Function&) const;
	private:
		Scalar time_private;
		struct FE_element *element_private;
		struct FE_field *field_private;
		struct FE_node *node_private;
		Vector components_private,xi_private;
};

#endif /* !defined (__FUNCTION_FINITE_ELEMENT_HPP__) */
