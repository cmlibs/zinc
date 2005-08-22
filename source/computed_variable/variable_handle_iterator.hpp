//******************************************************************************
// FILE : variable_handle_iterator.hpp
//
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Handle iterator templates.
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
#if !defined (__VARIABLE_HANDLE_ITERATOR_HPP__)
#define __VARIABLE_HANDLE_ITERATOR_HPP__

#include <iterator>

template<typename Handle> class Handle_iterator;

template<typename Handle> class Handle_iterator_representation
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// If B is the base class then for a derived class A, the iterator will be of
// type Handle_iterator<B> with a representation derived from
// Handle_iterator_representation<B>.
//==============================================================================
{
	friend class Handle_iterator<Handle>;
	public:
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Handle_iterator_representation();
		// increment.  Do not want to return an object derived from
		//   Handle_iterator_representation
		virtual void increment()=0;
		// equality.  Used by Handle_iterator::operator== and !=
		virtual bool equality(const Handle_iterator_representation*)=0;
		// dereference
		virtual Handle dereference() const=0;
	protected:
		// constructor.  Protected so that can't create "plain"
		//   Handle_iterator_representations
		Handle_iterator_representation();
	private:
		// copy operations are private and undefined to prevent copying
		Handle_iterator_representation(const Handle_iterator_representation&);
		Handle_iterator_representation& operator=(
			const Handle_iterator_representation&);
	private:
		int reference_count;
};

template<typename Handle> class Handle_iterator:
	public std::iterator<std::input_iterator_tag,Handle>
//******************************************************************************
// LAST MODIFIED : 27 January 2004
//
// DESCRIPTION :
// An iterator for Handles.
//
// Set up as a handle so that all inputs can have a begin_atomic_inputs and an
// end_atomic_inputs that return an iterator that is specialized for the derived
// input class.  Used instead of covariant returns because can't overload
// operators for a pointer.
//
// Didn't use Handle because Handle does not have ++ and because an iterator
// needs extra information such as the input that it is iterating over.
//==============================================================================
{
	public:
		// constructor
		Handle_iterator(Handle_iterator_representation<Handle> *);
		// copy constructor
		Handle_iterator(const Handle_iterator&);
		// assignment
		Handle_iterator& operator=(const Handle_iterator&);
		// destructor.  Virtual for proper destruction of derived classes
		~Handle_iterator();
		// increment (prefix)
		Handle_iterator& operator++();
		// increment (postfix)
		Handle_iterator operator++(int);
		// equality
		bool operator==(const Handle_iterator&);
		// inequality
		bool operator!=(const Handle_iterator&);
		// dereference
		Handle operator*() const;
		// don't have a operator-> because its not needed and it would return a
		//   Handle*
	private:
		Handle_iterator_representation<Handle> *representation;
};

// some compilers (g++ ?) need the template definitions in all source files
#include "computed_variable/variable_handle_iterator.cpp"

#endif /* !defined (__VARIABLE_HANDLE_ITERATOR_HPP__) */
