//******************************************************************************
// FILE : variable_input.cpp
//
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
// ???DB.  24Dec03
//   Require that the atoms (can't be sub-divided) that make up an input are
//   independent?
// ???DB.  31Dec03
//   Size for an input also depends on variable?  variable an input refers to is
//   part of it?
// ???DB.  14Jan04
//   Change from scalars and scalar_mapping to Variable_input_iterator
//   - need concept of "atom".  Then the iterator will run through the atoms for
//     the input
//     - what about when already atomic?
//       - if assume "flattened" then can call atoms with "local".  Means that
//         have an extra function call if "atomic"
//   - need to derive from STL iterator classes
//   - needed for all inputs or just composites/aggregates?
//   - back to idea of intersect for inputs?
// ???DB.  15Jan04
//   - what is the analog of input iterators for variables?  Components?
// ???DB.  17Jan04
//   - _handle is a bit of a misnomer because the thing about handles is that
//     they don't provide operator-> or operator* (Alexandrescu p.161)
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

#include <typeinfo>

#include "computed_variable/variable_input.hpp"
#if defined (USE_SCALAR_MAPPING)
#include "computed_variable/variable_input_composite.hpp"
#endif // defined (USE_SCALAR_MAPPING)

#if !defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
// class Variable_input_iterator_reference
// ---------------------------------------
Variable_input_iterator_representation::
	Variable_input_iterator_representation():reference_count(0)
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable_input_iterator_representation::
	~Variable_input_iterator_representation()
//******************************************************************************
// LAST MODIFIED : 26 January 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{}

// class Variable_input_iterator
// -----------------------------

Variable_input_iterator::Variable_input_iterator(
	Variable_input_iterator_representation *representation):
	representation(representation)
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable_input_iterator::Variable_input_iterator(
	const Variable_input_iterator& iterator):
	representation(iterator.representation)
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation)
	{
		(representation->reference_count)++;
	}
};

Variable_input_iterator& Variable_input_iterator::operator=(
	const Variable_input_iterator& iterator)
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// Assignment.
//==============================================================================
{
	// having a local Variable_input_iterator like this does the reference_count
	//   incrementing and decrementing (and destruction if need)
	Variable_input_iterator temp_iterator(iterator);
	Variable_input_iterator_representation *temp_representation;

	temp_representation=representation;
	representation=temp_iterator.representation;
	temp_iterator.representation=temp_representation;

	return (*this);
};

Variable_input_iterator::~Variable_input_iterator()
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	if (representation)
	{
		(representation->reference_count)--;
		if (representation->reference_count<=0)
		{
			delete representation;
		}
	}
}

Variable_input_iterator& Variable_input_iterator::operator++()
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// Increment (prefix).
//==============================================================================
{
	if (representation)
	{
		representation->increment();
	}

	return (*this);
}

Variable_input_iterator Variable_input_iterator::operator++(int)
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// Increment (postfix).
//==============================================================================
{
	Variable_input_iterator tmp= *this;

	if (representation)
	{
		representation->increment();
	}

	return (tmp);
}

bool Variable_input_iterator::operator==(
	const Variable_input_iterator& iterator)
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// Equality.
//==============================================================================
{
	bool result;

	result=false;
	if (representation)
	{
		result=(representation->equality)(iterator.representation);
	}
	else
	{
		if (!(iterator.representation))
		{
			result=true;
		}
	}

	return (result);
};

bool Variable_input_iterator::operator!=(
	const Variable_input_iterator& iterator)
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// Inequality.
//==============================================================================
{
	bool result;

	result=false;
	if (this)
	{
		result= !((*this)==iterator);
	}

	return (result);
}

Variable_input_handle Variable_input_iterator::operator*() const
//******************************************************************************
// LAST MODIFIED : 23 January 2004
//
// DESCRIPTION :
// Dereference.
//==============================================================================
{
	Variable_input_handle result(0);

	if (this&&(this->representation))
	{
		result=(this->representation->dereference)();
	}

	return (result);
};
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // !defined (USE_ITERATORS_NESTED)

// class Variable_input
// --------------------

Variable_input::Variable_input()
#if defined (USE_INTRUSIVE_SMART_POINTER)
	:reference_count(0)
#endif
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable_input::~Variable_input()
//******************************************************************************
// LAST MODIFIED : 9 September 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

#if defined (USE_INTRUSIVE_SMART_POINTER)
void intrusive_ptr_add_ref(Variable_input *input)
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	if (input)
	{
		(input->reference_count)++;
	}
}
#endif // defined (USE_INTRUSIVE_SMART_POINTER)

#if defined (USE_ITERATORS)
#if defined (USE_ITERATORS_NESTED)
Variable_input::Iterator::Iterator()
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable_input::Iterator::~Iterator()
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

#if defined (DO_NOT_WANT_TO_USE)
//???DB.  Want a compile or link time way of making sure that the Iterator
//  methods are defined for the Variable_input sub-classes
Variable_input::Iterator& Variable_input::Iterator::operator=(
	const Variable_input::Iterator&)
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Assignment.
//==============================================================================
{
	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable_input::Iterator::operator=().  Should not come here"));
	
	return (*this);
}

Variable_input::Iterator Variable_input::Iterator::operator++()
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Increment (prefix).
//==============================================================================
{
	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable_input::Iterator::operator++().  Should not come here"));
	
	return (*this);
}

Variable_input::Iterator Variable_input::Iterator::operator++(int)
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Increment (postfix).
//==============================================================================
{
	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable_input::Iterator::operator++(int).  Should not come here"));
	
	return (*this);
}

bool Variable_input::Iterator::operator==(const Iterator&)
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Equality.
//==============================================================================
{
	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable_input::Iterator::operator==(const Iterator&).  "
		"Should not come here"));
	
	return (false);
}

bool Variable_input::Iterator::operator!=(const Iterator&)
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Inquality.
//==============================================================================
{
	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable_input::Iterator::operator!=(const Iterator&).  "
		"Should not come here"));
	
	return (true);
}

Variable_input_handle& Variable_input::Iterator::operator*() const
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
// Dereference.
//==============================================================================
{
	Variable_input_handle *result_address=new Variable_input_handle(0);

	// should not come here - should be overloaded, but can't make pure
	Assert(false,std::logic_error(
		"Variable_input::Iterator::operator*().  Should not come here"));
	
	return (*result_address);
}
#endif // defined (DO_NOT_WANT_TO_USE)
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)

#if defined (USE_SCALAR_MAPPING)
std::list< std::pair<Variable_size_type,Variable_size_type> >
	Variable_input::scalar_mapping(Variable_input_handle target)
//******************************************************************************
// LAST MODIFIED : 12 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_input_composite_handle target_composite=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_composite,Variable_input>
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_composite *>
#endif /* defined (USE_SMART_POINTER) */
		(target);
	std::list< std::pair<Variable_size_type,Variable_size_type> > result(0);

	if (target_composite)
	{
		result=target_composite->scalar_mapping_target(Variable_input_handle(this));
	}
	else
	{
		result=scalar_mapping_local(target);
	}

	return (result);
}
#endif // defined (USE_SCALAR_MAPPING)

#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
class Variable_input_composite_scalar_mapping
//******************************************************************************
// LAST MODIFIED : 26 December 2003
//
// DESCRIPTION :
// A unary function (Functor) for merging scalar mappings when <source> is
// composite.
//==============================================================================
{
	public:
		Variable_input_composite_scalar_mapping(const Variable_input_handle source,
			std::list< std::pair<Variable_size_type,Variable_size_type> >&
			scalar_mapping):scalar_mapping(scalar_mapping),source(source),
			target_composite_size(0) {};
		~Variable_input_composite_scalar_mapping() {};
		int operator() (Variable_input_handle target)
		{
			std::list< std::pair<Variable_size_type,Variable_size_type> >
				scalar_mapping_temp=source->scalar_mapping_local(target);
			std::list< std::pair<Variable_size_type,Variable_size_type> >::iterator
				iterator,iterator_last,iterator_temp;
			Variable_size_type i,i_temp,target_size;

			target_size=target->size();
			iterator=scalar_mapping.begin();
			for (i=scalar_mapping.size();i>0;i--)
			{
				if (target_composite_size<=iterator->second)
				{
					iterator->second += target_size;
				}
				iterator++;
			}
			iterator_temp=scalar_mapping_temp.begin();
			for (i_temp=scalar_mapping_temp.size();i_temp>0;i_temp--)
			{
				if (iterator_temp->second<target_size)
				{
					if (0<(i=scalar_mapping.size()))
					{
						iterator=scalar_mapping.begin();
						iterator_last=iterator;
						while ((i>0)&&(iterator->first<=iterator_temp->first))
						{
							iterator_last=iterator;
							iterator++;
							i--;
						}
						Assert(iterator!=iterator_last,std::logic_error(
							"Variable_input_composite_scalar_mapping::operator().  "
							"First value of first pair of scalar mapping is not zero"));
						if (iterator_last->first==iterator_temp->first)
						{
							Assert(target_composite_size<=iterator_last->second,
								std::logic_error(
								"Variable_input_composite_scalar_mapping::operator().  "
								"target_composite inputs are not independent"));
							iterator_last->second=
								(iterator_temp->second)+target_composite_size;
						}
						else
						{
							scalar_mapping.insert(iterator,
								std::pair<Variable_size_type,Variable_size_type>(
								iterator_temp->first,
								(iterator_temp->second)+target_composite_size));
						}
					}
					else
					{
						scalar_mapping.push_back(
							std::pair<Variable_size_type,Variable_size_type>(
							iterator_temp->first,
							(iterator_temp->second)+target_composite_size));
					}
				}
				iterator_temp++;
			}
			target_composite_size += target_size;

			return (0);
		}
	private:
		std::list< std::pair<Variable_size_type,Variable_size_type> >&
			scalar_mapping;
		Variable_input_handle source;
		Variable_size_type target_composite_size;
};

std::list< std::pair<Variable_size_type,Variable_size_type> >
	Variable_input::scalar_mapping(const Variable_input& target) const
//******************************************************************************
// LAST MODIFIED : 26 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	std::list< std::pair<Variable_size_type,Variable_size_type> > result(0);

	try
	{
		const Variable_input_composite& target_composite=dynamic_cast<
			const Variable_input_composite&>(target);
		
		std::for_each(target_composite.begin(),target_composite.end(),
			Variable_input_composite_scalar_mapping(Variable_input_handle(this),
			result));
	}
	catch (std::bad_cast)
	{
		result=scalar_mapping_local(target);
	}

	return (result);
}

class Variable_input_composite_operator_plus
//******************************************************************************
// LAST MODIFIED : 28 December 2003
//
// DESCRIPTION :
// A unary function (Functor) for operator+ when <source> is composite.
//==============================================================================
{
	public:
		Variable_input_composite_operator_plus(Variable_input_handle& result):
			result(result) {};
		~Variable_input_composite_operator_plus() {};
		int operator() (Variable_input_handle second)
		{
			Variable_input_handle result_local=result->operator_plus_local(second);

			if (result_local)
			{
				result=result_local;
			}
			else
			{
				result=Variable_input_handle(new Variable_composite(result,second));
			}

			return (0);
		}
	private:
		Variable_input_handle& result;
};

Variable_input_handle Variable_input::operator+(const Variable_input& second)
//******************************************************************************
// LAST MODIFIED : 28 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_input_handle result(0);

	try
	{
		const Variable_input_composite& second_composite=dynamic_cast<
			const Variable_input_composite&>(second);
		
		result=Variable_input_handle(this);
		std::for_each(second_composite.begin(),second_composite.end(),
			Variable_input_composite_operator_plus(result));
	}
	catch (std::bad_cast)
	{
		if (!(result=operator_plus_local(second)))
		{
			result=Variable_input_handle(new Variable_input_composite(
				Variable_input_handle(this),Variable_input_handle(second)));
		}
	}

	return (result);
}

class Variable_input_composite_operator_minus
//******************************************************************************
// LAST MODIFIED : 28 December 2003
//
// DESCRIPTION :
// A unary function (Functor) for operator- when <source> is composite.
//==============================================================================
{
	public:
		Variable_input_composite_operator_minus(Variable_input_handle& result):
			result(result) {};
		~Variable_input_composite_operator_minus() {};
		int operator() (Variable_input_handle second)
		{
			if (result)
			{
				result=result->operator_minus_local(second);
			}

			return (0);
		}
	private:
		Variable_input_handle& result;
};

Variable_input_handle Variable_input::operator-(const Variable_input& second)
//******************************************************************************
// LAST MODIFIED : 28 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_input_handle result(0);

	try
	{
		const Variable_input_composite& second_composite=dynamic_cast<
			const Variable_input_composite&>(second);
		
		result=Variable_input_handle(this);
		std::for_each(second_composite.begin(),second_composite.end(),
			Variable_input_composite_operator_minus(result));
	}
	catch (std::bad_cast)
	{
		result=operator_minus_local(second);
	}

	return (result);
}

class Variable_input_composite_intersect
//******************************************************************************
// LAST MODIFIED : 28 December 2003
//
// DESCRIPTION :
// A unary function (Functor) for intersection when <source> is composite.
//==============================================================================
{
	public:
		Variable_input_composite_intersect(Variable_input_handle first,
			Variable_input_handle& result):first(first),result(result) {};
		~Variable_input_composite_intersect() {};
		int operator() (Variable_input_handle second)
		{
			Variable_input_handle intersect_temp=(first->intersect_local)(second);

			if (intersect_temp)
			{
				if (result)
				{
					result=(*result)+(*intersect_temp);
				}
				else
				{
					result=intersect_temp;
				}
			}

			return (0);
		}
	private:
		Variable_input_handle first;
		Variable_input_handle& result;
};

Variable_input_handle Variable_input::intersect(const Variable_input& second)
//******************************************************************************
// LAST MODIFIED : 28 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_input_handle result(0);

	try
	{
		const Variable_input_composite& second_composite=dynamic_cast<
			const Variable_input_composite&>(second);
		std::list<Variable_input_handle>::iterator composite_iterator;
		
		composite_iterator=second_composite.begin();
		if (composite_iterator!=second_composite.end())
		{
			result=
				((Variable_input_handle(this))->intersect_local)(*composite_iterator);
			composite_iterator++;
			std::for_each(composite_iterator,second_composite.end(),
				Variable_input_composite_intersect(Variable_input_handle(this),result));
		}
	}
	catch (std::bad_cast)
	{
		result=intersect_local(second);
	}

	return (result);
}
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)

#if defined (USE_INTRUSIVE_SMART_POINTER)
void intrusive_ptr_release(Variable_input *input)
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	if (input)
	{
		(input->reference_count)--;
		if (input->reference_count<=0)
		{
			delete input;
		}
	}
}
#endif // defined (USE_INTRUSIVE_SMART_POINTER)

#if defined (OLD_CODE)
Variable_input::size_type Variable_input::size()
//******************************************************************************
// LAST MODIFIED : 11 October 2003
//
// DESCRIPTION :
// Needed for setting up virtual table by compiler.
//==============================================================================
{
	return (0);
}
#endif

#if defined (USE_SCALAR_MAPPING)
std::list< std::pair<Variable_size_type,Variable_size_type> >
	Variable_input::scalar_mapping_local(Variable_input_handle target)
//******************************************************************************
// LAST MODIFIED : 12 January 2004
//
// DESCRIPTION :
// Default scalar_mapping_local.
//==============================================================================
{
	std::list< std::pair<Variable_size_type,Variable_size_type> > result(0);

	if (this&&target)
	{
		if (0<size())
		{
			if (*this== *target)
			{
				result.push_back(std::pair<Variable_size_type,Variable_size_type>(0,0));
			}
			else
			{
				result.push_back(std::pair<Variable_size_type,Variable_size_type>(0,
					target->size()));
			}
		}
		result.push_back(std::pair<Variable_size_type,Variable_size_type>(size(),
			target->size()));
	}

	return (result);
}
#endif // defined (USE_SCALAR_MAPPING)
