// FILE : derivative_matrix.hpp
//
// LAST MODIFIED : 23 December 2004
//
// DESCRIPTION :
// ???DB.  Maybe move into function_derivative?
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
#if !defined (__DERIVATIVE_MATRIX_HPP__)
#define __DERIVATIVE_MATRIX_HPP__

#include <list>
#include <vector>

#include "computed_variable/function_base.hpp"

class Derivative_matrix : public std::list<Matrix>
//******************************************************************************
// LAST MODIFIED : 23 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// for calculation exception
		class Calculation_exception {};
		// constructors
		Derivative_matrix();
		Derivative_matrix(const std::list<Matrix>& matrices);
		// copy constructor
		Derivative_matrix(const Derivative_matrix&);
#if defined (OLD_CODE)
		// assignment
		Derivative_matrix& operator=(const Derivative_matrix&);
#endif // defined (OLD_CODE)
		// destructor
		~Derivative_matrix();
		// implement the chain rule for differentiation
		Derivative_matrix operator*(const Derivative_matrix&) const;
		// calculate the chain rule inverse
		Derivative_matrix inverse();
	private:
		Function_size_type number_of_dependent_values,order;
		std::vector<Function_size_type> numbers_of_independent_values;
};

#endif /* !defined (__DERIVATIVE_MATRIX_HPP__) */
