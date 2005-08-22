//******************************************************************************
// FILE : variable_finite_element.hpp
//
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
// Finite element types - element/xi and finite element field.
//==============================================================================
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (__VARIABLE_FINITE_ELEMENT_HPP__)
#define __VARIABLE_FINITE_ELEMENT_HPP__

#include <list>
#include "computed_variable/variable.hpp"
#include "computed_variable/variable_vector.hpp"
extern "C"
{
#include "finite_element/finite_element.h"
}

class Variable_element_xi : public Variable
//******************************************************************************
// LAST MODIFIED : 4 February 2004
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
		// components are indivisible
#if defined (USE_ITERATORS)
		// returns the number of components that are differentiable
		virtual Variable_size_type number_differentiable() const;
#if defined (USE_VARIABLES_AS_COMPONENTS)
		virtual bool is_component();
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
		// for stepping through the components that make up the Variable
#if defined (USE_VARIABLES_AS_COMPONENTS)
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_components();
		virtual Iterator end_components();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Handle_iterator<Variable_handle> begin_components();
		virtual Handle_iterator<Variable_handle> end_components();
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#else // defined (USE_VARIABLES_AS_COMPONENTS)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_components();
		virtual Handle_iterator<Variable_io_specifier_handle> end_components();
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
#else // defined (USE_ITERATORS)
		// get the number of scalars in the result
		Variable_size_type size() const;
		// get the scalars in the result
		Vector *scalars();
#endif // defined (USE_VARIABLE_ITERATORS)
		// input specifiers
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			input_element_xi(),
			input_element(),
			input_xi(),
			input_xi(Variable_size_type),
			input_xi(const ublas::vector<Variable_size_type>);
		virtual Variable_handle operator-(const Variable&) const;
		virtual Variable_handle operator-=(const Variable&);
		virtual Variable_handle clone() const;
	private:
		Variable_handle evaluate_local();
		bool evaluate_derivative_local(Matrix& matrix,
			std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables);
		Variable_handle get_input_value_local(
			const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input_atomic);
		bool set_input_value_local(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input_atomic,
			const
#if defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
			& value);
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
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
// A variable for a finite element interpolation field.
//
//???DB.  Change to only using io_specifiers for components?
//==============================================================================
{
	//???DB.  Should be able to remove these friends by using region,
	//  number_of_versions, number_of_derivatives and nodal_value_types methods
	friend class Variable_input_nodal_values;
	friend class Variable_input_element_xi;
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
		// components are indivisible
#if defined (USE_ITERATORS)
		// returns the number of components that are differentiable
		virtual Variable_size_type number_differentiable() const;
#if defined (USE_VARIABLES_AS_COMPONENTS)
		virtual bool is_component();
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
		// for stepping through the components that make up the Variable
#if defined (USE_VARIABLES_AS_COMPONENTS)
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_components();
		virtual Iterator end_components();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Handle_iterator<Variable_handle> begin_components();
		virtual Handle_iterator<Variable_handle> end_components();
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#else // defined (USE_VARIABLES_AS_COMPONENTS)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_components();
		virtual Handle_iterator<Variable_io_specifier_handle> end_components();
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
#else // defined (USE_ITERATORS)
		// get the number of scalars in the result
		Variable_size_type size() const;
		// get the scalars in the result
		Vector *scalars();
#endif // defined (USE_VARIABLE_ITERATORS)
		// input specifiers
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			input_element_xi(),
			input_element(),
			input_xi(),
			input_xi(Variable_size_type),
			input_xi(const ublas::vector<Variable_size_type>),
			input_nodal_values(),
			input_nodal_values(struct FE_node *node,
				enum FE_nodal_value_type value_type,int version),
			input_nodal_values(struct FE_node *node),
			input_nodal_values(
				enum FE_nodal_value_type value_type),
			input_nodal_values(int version);
		virtual Variable_handle clone() const;
		// return the region that the field is defined for
		// NB.  The calling program should use ACCESS(FE_region) and
		//   DEACCESS(FE_region) to manage the lifetime of the returned region
		struct FE_region *region() const;
		// return the number of versions for the component at the node
		Variable_size_type number_of_versions(struct FE_node *node,
			Variable_size_type component_number);
		// return the number of derivatives for the component at the node
		Variable_size_type number_of_derivatives(struct FE_node *node,
			Variable_size_type component_number);
		// return the nodal value types for the component at the node
		// NB.  The calling program should DEALLOCATE the returned array when its no
		//   longer needed
		enum FE_nodal_value_type *nodal_value_types(struct FE_node *node,
			Variable_size_type component_number);
	private:
		Variable_handle evaluate_local();
		bool evaluate_derivative_local(Matrix& matrix,
			std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables);
		Variable_handle get_input_value_local(
			const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input_atomic);
		bool set_input_value_local(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input_atomic,
			const
#if defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
			& value);
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
