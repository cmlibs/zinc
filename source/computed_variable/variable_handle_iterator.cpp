//******************************************************************************
// FILE : variable_handle_iterator.cpp
//
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
// Handle iterator definitions.
//==============================================================================

#include "computed_variable/variable_base.hpp"

#include "computed_variable/variable_handle_iterator.hpp"

#if defined (USE_ITERATORS)
#if !defined (USE_ITERATORS_NESTED)
#if !defined (DO_NOT_USE_ITERATOR_TEMPLATES)
// class Handle_iterator_representation<Handle>
// --------------------------------------------
template<typename Handle>
	Handle_iterator_representation<Handle>::Handle_iterator_representation():
	reference_count(0)
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

template<typename Handle>
	Handle_iterator_representation<Handle>::~Handle_iterator_representation()
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{}

// class Variable_input_iterator
// -----------------------------

template<typename Handle>
	Handle_iterator<Handle>::Handle_iterator(
	Handle_iterator_representation<Handle> *representation):
	representation(representation)
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

template<typename Handle>
	Handle_iterator<Handle>::Handle_iterator(
	const Handle_iterator<Handle>& iterator):
	representation(iterator.representation)
//******************************************************************************
// LAST MODIFIED : 28 January 2004
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

template<typename Handle>
	Handle_iterator<Handle>& Handle_iterator<Handle>::operator=(
	const Handle_iterator<Handle>& iterator)
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Assignment.
//==============================================================================
{
	// having a local Handle_iterator<Handle> like this does the reference_count
	//   incrementing and decrementing (and destruction if need)
	Handle_iterator<Handle> temp_iterator(iterator);
	Handle_iterator_representation<Handle> *temp_representation;

	temp_representation=representation;
	representation=temp_iterator.representation;
	temp_iterator.representation=temp_representation;

	return (*this);
};

template<typename Handle>
	Handle_iterator<Handle>::~Handle_iterator()
//******************************************************************************
// LAST MODIFIED : 28 January 2004
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

template<typename Handle>
	Handle_iterator<Handle>& Handle_iterator<Handle>::operator++()
//******************************************************************************
// LAST MODIFIED : 28 January 2004
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

template<typename Handle>
	Handle_iterator<Handle> Handle_iterator<Handle>::operator++(int)
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Increment (postfix).
//==============================================================================
{
	Handle_iterator<Handle> tmp= *this;

	if (representation)
	{
		representation->increment();
	}

	return (tmp);
}

template<typename Handle>
	bool Handle_iterator<Handle>::operator==(
	const Handle_iterator<Handle>& iterator)
//******************************************************************************
// LAST MODIFIED : 28 January 2004
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

template<typename Handle>
	bool Handle_iterator<Handle>::operator!=(
	const Handle_iterator<Handle>& iterator)
//******************************************************************************
// LAST MODIFIED : 28 January 2004
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

template<typename Handle>
	Handle Handle_iterator<Handle>::operator*() const
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// Dereference.
//==============================================================================
{
	Handle result(0);

	if (this&&(this->representation))
	{
		result=(this->representation->dereference)();
	}

	return (result);
};
#endif // !defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // !defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)
