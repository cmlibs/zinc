//******************************************************************************
// FILE : function_base.hpp
//
// LAST MODIFIED : 12 January 2005
//
// DESCRIPTION :
// Basic declarations and #defines for functions.
//
// ???DB.  17Jan04
//   _handle is a bit of a misnomer for smart pointers because the thing about
//   handles is that they don't provide operator-> or operator*
//   (Alexandrescu p.161)
//==============================================================================
#if !defined (__FUNCTION_BASE_HPP__)
#define __FUNCTION_BASE_HPP__

#include <iostream>
#include <stdexcept>

#define EVALUATE_RETURNS_VALUE

//#define BEFORE_CACHING

//#define EXPORT_IMPLEMENTED
//#define ONE_TEMPLATE_DEFINITION_IMPLEMENTED

#if defined (EXPORT_IMPLEMENTED)
#define EXPORT export
#define ONE_TEMPLATE_DEFINITION_IMPLEMENTED
#else // defined (EXPORT_IMPLEMENTED)
#define EXPORT 
#endif // defined (EXPORT_IMPLEMENTED)

const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception)
{
	if (Assert_on&&!(assertion))
	{
		std::cerr << exception.what() << std::endl;
		throw exception;
	}
}

#include <string>

typedef std::string * string_handle;
	//???DB.  Replace with smart pointer or return reference?
	//???DB.  At present big probability of memory leak or segmentation fault

typedef unsigned int Function_size_type;

typedef double Scalar;

#include "boost/numeric/ublas/matrix.hpp"
#include "boost/numeric/ublas/vector.hpp"
#include "boost/numeric/ublas/io.hpp"

namespace ublas = boost::numeric::ublas;

// use column_major so that can use lapack=boost::numeric::bindings::lapack
typedef ublas::matrix<Scalar,ublas::column_major> Matrix;
typedef ublas::vector<Scalar> Vector;

#include "boost/intrusive_ptr.hpp"

// intentional ambiguity to prevent use of boost::intrusive_ptr::operator==
//   Use equivalent instead
template<class Value_type_1,class Value_type_2>
bool operator==(boost::intrusive_ptr<Value_type_1> const & first,
	boost::intrusive_ptr<Value_type_2> const & second);
template<class Value_type>
bool operator==(boost::intrusive_ptr<Value_type> const & first,
	Value_type * second);
template<class Value_type>
bool operator==(Value_type * first,
	boost::intrusive_ptr<Value_type> const & second);

// intentional ambiguity to prevent use of boost::intrusive_ptr::operator!=
//   Use !equivalent instead
template<class Value_type_1,class Value_type_2>
bool operator!=(boost::intrusive_ptr<Value_type_1> const & first,
	boost::intrusive_ptr<Value_type_2> const & second);
template<class Value_type>
bool operator!=(boost::intrusive_ptr<Value_type> const & first,
	Value_type * second);
template<class Value_type>
bool operator!=(Value_type * first,
	boost::intrusive_ptr<Value_type> const & second);

template<class Value_type_1,class Value_type_2>
bool equivalent(boost::intrusive_ptr<Value_type_1> const & first,
	boost::intrusive_ptr<Value_type_2> const & second)
{
	Value_type_1 *first_ptr=first.get();
	Value_type_2 *second_ptr=second.get();

	return (((first_ptr==second_ptr))||
		(first_ptr&&second_ptr&&(*first_ptr== *second_ptr)));
}

// forward declaration so that _handle can be used in definitions
class Function;

typedef boost::intrusive_ptr<Function> Function_handle;
void intrusive_ptr_add_ref(Function *);
void intrusive_ptr_release(Function *);

// forward declaration so that _handle can be used in definitions
class Function_variable;

typedef boost::intrusive_ptr<Function_variable> Function_variable_handle;
void intrusive_ptr_add_ref(Function_variable *);
void intrusive_ptr_release(Function_variable *);

#endif /* !defined (__FUNCTION_BASE_HPP__) */
