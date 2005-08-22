//******************************************************************************
// FILE : variable_io_specifier_composite.cpp
//
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
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

#include "computed_variable/variable_base.hpp"

#include <algorithm>
#include <iterator>
#include <typeinfo>
#include <iostream>

#include "computed_variable/variable_io_specifier_composite.hpp"

// module classes
// ==============

// class Variable_io_specifier_iterator_representation_atomic_composite
// --------------------------------------------------------------------

class Variable_io_specifier_iterator_representation_atomic_composite: public
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Variable_io_specifier_iterator_representation
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Handle_iterator_representation<Variable_io_specifier_handle>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
//******************************************************************************
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Variable_io_specifier_iterator_representation_atomic_composite(
			const bool begin,Variable_io_specifier_composite_handle source):
			atomic_io_specifier_iterator(0),io_specifiers_list_iterator(0),
			source(source)
		{
			if (begin&&source)
			{
				io_specifiers_list_iterator=(source->io_specifiers_list).begin();
				while (
					(io_specifiers_list_iterator!=(source->io_specifiers_list).end())&&
					((atomic_io_specifier_iterator=
					((*io_specifiers_list_iterator)->begin_atomic)())!=
					((*io_specifiers_list_iterator)->end_atomic)()))
				{
					io_specifiers_list_iterator++;
				}
				if (io_specifiers_list_iterator!=(source->io_specifiers_list).end())
				{
					io_specifiers_list_iterator=0;
					atomic_io_specifier_iterator=0;
				}
			}
		};
		// destructor
		~Variable_io_specifier_iterator_representation_atomic_composite(){};
		// increment
		void increment()
		{
			atomic_io_specifier_iterator++;
			if (atomic_io_specifier_iterator==
				((*io_specifiers_list_iterator)->end_atomic)())
			{
				do
				{
					io_specifiers_list_iterator++;
				} while (
					(io_specifiers_list_iterator!=(source->io_specifiers_list).end())&&
					((atomic_io_specifier_iterator=
					((*io_specifiers_list_iterator)->begin_atomic)())!=
					((*io_specifiers_list_iterator)->end_atomic)()));
				if (io_specifiers_list_iterator!=(source->io_specifiers_list).end())
				{
					io_specifiers_list_iterator=0;
					atomic_io_specifier_iterator=0;
				}
			}
		};
		// equality
		bool equality(const
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
			Variable_io_specifier_iterator_representation
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
			Handle_iterator_representation<Variable_io_specifier_handle>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
			* representation)
		{
			const Variable_io_specifier_iterator_representation_atomic_composite
				*representation_composite=dynamic_cast<
				const Variable_io_specifier_iterator_representation_atomic_composite *>(
				representation);

			return (representation_composite&&
				(source==representation_composite->source)&&
				(io_specifiers_list_iterator==
				representation_composite->io_specifiers_list_iterator)&&
				(atomic_io_specifier_iterator==
				representation_composite->atomic_io_specifier_iterator));
		};
		// dereference
		Variable_io_specifier_handle dereference() const
		{
			return (*atomic_io_specifier_iterator);
		};
	private:
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Variable_io_specifier_iterator
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Handle_iterator<Variable_io_specifier_handle>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
			atomic_io_specifier_iterator;
		std::list<Variable_io_specifier_handle>::iterator
			io_specifiers_list_iterator;
		Variable_io_specifier_composite_handle source;
};

// global classes
// ==============

// class Variable_io_specifier_composite
// -------------------------------------
Variable_io_specifier_composite::Variable_io_specifier_composite(
	const Variable_io_specifier_handle& io_specifier_1,
	const Variable_io_specifier_handle& io_specifier_2):
	Variable_io_specifier(),io_specifiers_list(0)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// Constructor.  Needs to "flatten" the <io_specifiers_list> ie. expand any
// composites.
//==============================================================================
{
	Variable_io_specifier_composite_handle io_specifier_1_composite=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_io_specifier_composite,
		Variable_io_specifier>(io_specifier_1);
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_io_specifier_composite *>(io_specifier_1);
#endif /* defined (USE_SMART_POINTER) */
	Variable_io_specifier_composite_handle io_specifier_2_composite=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_io_specifier_composite,
		Variable_io_specifier>(io_specifier_2);
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_io_specifier_composite *>(io_specifier_2);
#endif /* defined (USE_SMART_POINTER) */

	if (io_specifier_1_composite)
	{
		io_specifiers_list.insert(io_specifiers_list.end(),
			(io_specifier_1_composite->io_specifiers_list).begin(),
			(io_specifier_1_composite->io_specifiers_list).end());
	}
	else
	{
		io_specifiers_list.push_back(io_specifier_1);
	}
	if (io_specifier_2_composite)
	{
		io_specifiers_list.insert(io_specifiers_list.end(),
			(io_specifier_2_composite->io_specifiers_list).begin(),
			(io_specifier_2_composite->io_specifiers_list).end());
	}
	else
	{
		io_specifiers_list.push_back(io_specifier_2);
	}
}

Variable_io_specifier_composite::Variable_io_specifier_composite(
	std::list<Variable_io_specifier_handle>& io_specifiers_list):
	Variable_io_specifier(),io_specifiers_list(0)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// Constructor.  Needs to "flatten" the <io_specifiers_list> ie. expand any
// composites.
//==============================================================================
{
	std::list<Variable_io_specifier_handle>::iterator io_specifier_iterator;
	Variable_size_type i;

	// "flatten" the <io_specifiers_list>.  Does not need to be recursive because
	//   composite io_specifiers have flat lists
	io_specifier_iterator=io_specifiers_list.begin();
	for (i=io_specifiers_list.size();i>0;i--)
	{
		Variable_io_specifier_composite_handle io_specifier_composite=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_io_specifier_composite,
			Variable_io_specifier>(*io_specifier_iterator);
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_io_specifier_composite *>(*io_specifier_iterator);
#endif /* defined (USE_SMART_POINTER) */

		if (io_specifier_composite)
		{
			(this->io_specifiers_list).insert((this->io_specifiers_list).end(),
				(io_specifier_composite->io_specifiers_list).begin(),
				(io_specifier_composite->io_specifiers_list).end());
		}
		else
		{
			(this->io_specifiers_list).push_back(*io_specifier_iterator);
		}
		io_specifier_iterator++;
	}
}

Variable_io_specifier_composite::Variable_io_specifier_composite(
	const Variable_io_specifier_composite& io_specifier_composite):
	Variable_io_specifier(),
	io_specifiers_list(io_specifier_composite.io_specifiers_list)
//******************************************************************************
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_io_specifier_handle Variable_io_specifier_composite::clone() const
//******************************************************************************
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	return
		(Variable_io_specifier_handle(new Variable_io_specifier_composite(*this)));
}

bool Variable_io_specifier_composite::is_atomic()
//******************************************************************************
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	return
		((1==io_specifiers_list.size())&&(io_specifiers_list.front())->is_atomic());
}

#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
Variable_io_specifier_iterator
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
Handle_iterator<Variable_io_specifier_handle>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Variable_io_specifier_composite::begin_atomic()
//******************************************************************************
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Variable_io_specifier_iterator
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Handle_iterator<Variable_io_specifier_handle>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		(new Variable_io_specifier_iterator_representation_atomic_composite(true,
		Variable_io_specifier_composite_handle(this))));
}

#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
Variable_io_specifier_iterator
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
Handle_iterator<Variable_io_specifier_handle>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
	Variable_io_specifier_composite::end_atomic()
//******************************************************************************
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Variable_io_specifier_iterator
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		Handle_iterator<Variable_io_specifier_handle>
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		(new Variable_io_specifier_iterator_representation_atomic_composite(false,
		Variable_io_specifier_composite_handle(this))));
}

class Variable_io_specifier_composite_sum_number_differentiable_functor
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// A unary function (functor) for summing the number_differentiable of the
// io_specifiers that make up a composite io_specifier.
//==============================================================================
{
	public:
		Variable_io_specifier_composite_sum_number_differentiable_functor(
			Variable_size_type& sum):sum(sum)
		{
			sum=0;
		};
		int operator() (const Variable_io_specifier_handle& io_specifier)
		{
			sum += io_specifier->number_differentiable();
			return (0);
		};
	private:
		Variable_size_type& sum;
};

Variable_size_type Variable_io_specifier_composite::number_differentiable()
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type sum;

	// get the specified values
	std::for_each(io_specifiers_list.begin(),io_specifiers_list.end(),
		Variable_io_specifier_composite_sum_number_differentiable_functor(sum));

	return (sum);
}

Variable_io_specifier_composite& Variable_io_specifier_composite::operator=(
	const Variable_io_specifier_composite& io_specifier_composite)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// Assignment operator.
//???DB.  Same as implicit?
//==============================================================================
{
	//???DB.  Does assignment for super class first?
	io_specifiers_list=io_specifier_composite.io_specifiers_list;

	return (*this);
}

bool Variable_io_specifier_composite::operator==(
	const Variable_io_specifier& io_specifier)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	try
	{
		const Variable_io_specifier_composite& io_specifier_composite=
			dynamic_cast<const Variable_io_specifier_composite&>(io_specifier);

		return (io_specifiers_list==io_specifier_composite.io_specifiers_list);
	}
	catch (std::bad_cast)
	{
		return (false);
	}
}
