//******************************************************************************
// FILE : variable_io_specifier.cpp
//
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// An abstract class for specifying inputs/independents and outputs/dependents
// of a variable.
//
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
//
// ???DB.  29Jan04
//   Started from variable_input
//==============================================================================

#include "computed_variable/variable_base.hpp"

#include <typeinfo>

#include "computed_variable/variable_io_specifier.hpp"

#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
// class Variable_io_specifier_iterator_reference
// ----------------------------------------------

Variable_io_specifier_iterator_representation::
	Variable_io_specifier_iterator_representation():reference_count(0)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable_io_specifier_iterator_representation::
	~Variable_io_specifier_iterator_representation()
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{}

// class Variable_io_specifier_iterator
// ------------------------------------

Variable_io_specifier_iterator::Variable_io_specifier_iterator(
	Variable_io_specifier_iterator_representation *representation):
	representation(representation)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable_io_specifier_iterator::Variable_io_specifier_iterator(
	const Variable_io_specifier_iterator& iterator):
	representation(iterator.representation)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
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

Variable_io_specifier_iterator& Variable_io_specifier_iterator::operator=(
	const Variable_io_specifier_iterator& iterator)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// Assignment.
//==============================================================================
{
	// having a local Variable_io_specifier_iterator like this does the
	//   reference_count incrementing and decrementing (and destruction if need)
	Variable_io_specifier_iterator temp_iterator(iterator);
	Variable_io_specifier_iterator_representation *temp_representation;

	temp_representation=representation;
	representation=temp_iterator.representation;
	temp_iterator.representation=temp_representation;

	return (*this);
};

Variable_io_specifier_iterator::~Variable_io_specifier_iterator()
//******************************************************************************
// LAST MODIFIED : 29 January 2004
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

Variable_io_specifier_iterator& Variable_io_specifier_iterator::operator++()
//******************************************************************************
// LAST MODIFIED : 29 January 2004
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

Variable_io_specifier_iterator Variable_io_specifier_iterator::operator++(int)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// Increment (postfix).
//==============================================================================
{
	Variable_io_specifier_iterator tmp= *this;

	if (representation)
	{
		representation->increment();
	}

	return (tmp);
}

bool Variable_io_specifier_iterator::operator==(
	const Variable_io_specifier_iterator& iterator)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
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

bool Variable_io_specifier_iterator::operator!=(
	const Variable_io_specifier_iterator& iterator)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
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

Variable_io_specifier_handle Variable_io_specifier_iterator::operator*() const
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// Dereference.
//==============================================================================
{
	Variable_io_specifier_handle result(0);

	if (this&&(this->representation))
	{
		result=(this->representation->dereference)();
	}

	return (result);
};
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)

// class Variable_io_specifier
// ---------------------------

Variable_io_specifier::Variable_io_specifier(
	const enum Variable_io_specifier_type type):type(type)
#if defined (USE_INTRUSIVE_SMART_POINTER)
	,reference_count(0)
#endif
//******************************************************************************
// LAST MODIFIED : 30 January 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable_io_specifier::~Variable_io_specifier()
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

#if defined (USE_INTRUSIVE_SMART_POINTER)
void intrusive_ptr_add_ref(Variable_io_specifier *io_specifier)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (io_specifier)
	{
		(io_specifier->reference_count)++;
	}
}
#endif // defined (USE_INTRUSIVE_SMART_POINTER)

#if defined (USE_INTRUSIVE_SMART_POINTER)
void intrusive_ptr_release(Variable_io_specifier *io_specifier)
//******************************************************************************
// LAST MODIFIED : 29 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (io_specifier)
	{
		(io_specifier->reference_count)--;
		if (io_specifier->reference_count<=0)
		{
			delete io_specifier;
		}
	}
}
#endif // defined (USE_INTRUSIVE_SMART_POINTER)
