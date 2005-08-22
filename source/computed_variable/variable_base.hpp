//******************************************************************************
// FILE : variable_base.hpp
//
// LAST MODIFIED : 8 February 2004
//
// DESCRIPTION :
// Basic declarations and #defines for variables.
//
// ???DB.  17Jan04
//   _handle is a bit of a misnomer for smart pointers because the thing about
//   handles is that they don't provide operator-> or operator*
//   (Alexandrescu p.161)
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
#if !defined (__VARIABLE_BASE_HPP__)
#define __VARIABLE_BASE_HPP__

#include <stdexcept>

const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception) 
{
	if (Assert_on&&!(assertion)) throw exception;
}

#define USE_SMART_POINTER
#define USE_INTRUSIVE_SMART_POINTER

#if defined (USE_INTRUSIVE_SMART_POINTER) && !defined (USE_SMART_POINTER)
#define USE_SMART_POINTER
#endif

#if defined (USE_INTRUSIVE_SMART_POINTER)
#include "boost/intrusive_ptr.hpp"
#elif defined (USE_SMART_POINTER)
#include "boost/shared_ptr.hpp"
#endif

typedef unsigned int Variable_size_type;

// #defines some for ideas to get around problems with intersecting inputs.  The
//   ideas didn't quite work out, but led on to the Function/Function_variable
//   which uses the ideas and hopefully will work out
//
// Could add a method for getting the mapping between scalars.  This became very
//   unwieldy with too much shared knowledge needed
// #define USE_SCALAR_MAPPING
// One way to create unions and intersections of inputs is to add methods to
//   inputs.  This was part of the too much shared knowledge and unwieldiness
// #define VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS
// Add specialized helper methods (evaluate_derivative, get_input_value) for
//   composite inputs to allow finding overlaps and set operations
// #define GENERALIZE_COMPOSITE_INPUT
// Instead of using scalars() and size(), have iterators for stepping the
//   "atomic" inputs/components that make up a Variable_input/Variable.  Allows
//   overlap to be determined and Variable_input/Variable's that are unions or
//   intersections or ... (instead of just composite)
// #define USE_ITERATORS
// Started by having the iterator class nested within the Variable_input, but
//   this caused problems with return types for methods of derived classes
// #define USE_ITERATORS_NESTED
// Saw that the iterators for components are the same as for inputs and came up
//   with the Handle_iterator template
// #define DO_NOT_USE_ITERATOR_TEMPLATES
// Found difficulty in specifying components and saw that I'd need objects very
//   close to Variable_inputs for specifying components.  Came up with
//   Variable_io_specifier for replacing Variable_inputs and specifying
//   components
#define USE_VARIABLE_INPUT
// Decided that better to iterate through Variable_io_specifiers than Variables
//   for components (don't need all the computation methods or to be able to
//   refer to the storage for Variable that they are a component of)
#define USE_VARIABLES_AS_COMPONENTS

#endif /* !defined (__VARIABLE_BASE_HPP__) */
