//******************************************************************************
// FILE : function_base.hpp
//
// LAST MODIFIED : 15 July 2004
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

#include <stdexcept>

#if defined (EXPORT_IMPLEMENTED)
#define EXPORT export
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
