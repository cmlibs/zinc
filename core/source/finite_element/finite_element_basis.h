/***************************************************************************//**
 * FILE : finite_element_basis.h
 *
 * Declarations of types and methods for finite element basis functions.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_BASIS_H)
#define FINITE_ELEMENT_BASIS_H

#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "general/value.h"

/*
Global types
------------
*/

enum FE_basis_type
/*******************************************************************************
LAST MODIFIED : 20 October 1997

DESCRIPTION :
The different basis types available.
NOTE: Must keep this up to date with functions
FE_basis_type_string
NOTE: External API uses different enum cmzn_element_basis_type
==============================================================================*/
{
	FE_BASIS_TYPE_INVALID=-1,
	NO_RELATION=0,
		/*???DB.  Used on the off-diagonals of the type matrix */
	BSPLINE,
	CUBIC_HERMITE,
	CUBIC_LAGRANGE,
	FE_BASIS_CONSTANT,
	FOURIER,
	HERMITE_LAGRANGE,
	LAGRANGE_HERMITE,
	LINEAR_LAGRANGE,
	LINEAR_SIMPLEX,
	POLYGON,
	QUADRATIC_LAGRANGE,
	QUADRATIC_SIMPLEX,
	SERENDIPITY,
	SINGULAR,
	TRANSITION
}; /* enum FE_basis_type */

typedef int (Standard_basis_function)(/*type_arguments*/void *,
	/*xi_coordinates*/const FE_value *, /*function_values*/FE_value *);

struct FE_basis;
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating basis function values from xi
coordinates.  For each of basis there will be only one copy stored in a global
list.
==============================================================================*/

DECLARE_LIST_TYPES(FE_basis);

DECLARE_MANAGER_TYPES(FE_basis);

/*
Global functions
----------------
*/

struct FE_basis *CREATE(FE_basis)(int *type);
/*******************************************************************************
LAST MODIFIED : 1 October 1995

DESCRIPTION :
A basis is created with the specified <type> (duplicated).  The basis is
returned.
==============================================================================*/

int DESTROY(FE_basis)(struct FE_basis **basis_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Frees the memory for <**basis_address> and sets <*basis_address> to NULL.
==============================================================================*/

struct FE_basis *make_FE_basis(int *basis_type,
	struct MANAGER(FE_basis) *basis_manager);
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
Finds the specfied FE_basis in the basis managed. If it isn't there, creates it,
and adds it to the manager.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_basis);

PROTOTYPE_LIST_FUNCTIONS(FE_basis);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_basis,type,int *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(FE_basis,type,int *);

PROTOTYPE_MANAGER_FUNCTIONS(FE_basis);

PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(FE_basis,type,int *);

int *FE_basis_string_to_type_array(const char *basis_description_string);
/*******************************************************************************
LAST MODIFIED : 1 November 2004

DESCRIPTION :
Creates a type array from the <basis_description_string>.  Returns the type
array which can be used in CREATE(FE_basis) and make_FE_basis.
It is up to the calling function to DEALLOCATE the returned array.

Some examples of basis descriptions are:
1. c.Hermite*c.Hermite*l.Lagrange  This has cubic variation in xi1 and xi2 and
	linear variation in xi3.
2. c.Hermite*l.simplex(3)*l.simplex  This has cubic variation in xi1 and 2-D
	linear simplex variation for xi2 and xi3.
3. polygon(5,3)*l.Lagrange*polygon  This has linear variation in xi2 and a 2-D
	5-gon for xi1 and xi3.
==============================================================================*/

const char *FE_basis_type_string(enum FE_basis_type basis_type);
/*******************************************************************************
LAST MODIFIED : 1 April 1999

DESCRIPTION :
Returns a pointer to a static string token for the given <basis_type>.
The calling function must not deallocate the returned string.
#### Must ensure implemented correctly for new FE_basis_type. ####
==============================================================================*/

/***************************************************************************//**
 * Returns the string description of the basis type used in serialisation.
 * ???RC Currently limited to handling one polygon or one simplex. Will have to
 * be rewritten for 4-D and above elements.
 * 
 * @param basis  FE_basis type array - see struct FE_basis
 * @return  allocated basis description string
 */
char *FE_basis_get_description_string(struct FE_basis *basis);

int FE_basis_get_dimension(struct FE_basis *basis, int *dimension_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns the dimension of <basis>.
If fails, puts zero at <dimension_address>.
==============================================================================*/

int FE_basis_get_number_of_basis_functions(struct FE_basis *basis,
	int *number_of_basis_functions_address);
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
Returns the number_of_basis_functions of <basis>.
If fails, puts zero at <number_of_basis_functions_address>.
==============================================================================*/

int FE_basis_get_xi_basis_type(struct FE_basis *basis,
	int xi_number, enum FE_basis_type *basis_type_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns the basis type of <basis> on <xi_number> -- on main diagonal of
type array. The first xi_number is 0.
==============================================================================*/

int FE_basis_get_next_linked_xi_number(
	struct FE_basis *basis, int xi_number,
	int *next_xi_number_address, int *xi_link_number_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns in <next_xi_number_address> the next xi number higher than <xi_number>
which is linked in basis with it, plus in <xi_link_number_address> the number
denoting how it is linked; currently used only for polygon basiss to denote the
number of polygon sides.
If there is no remaining linked dimension, 0 is returned in both addresses.
<xi_number> is from 0 to one less than the basis dimension.
Also checks that the linked xi numbers have the same basis type.
==============================================================================*/

/***************************************************************************//**
 * Determine if the basis function is non-linear.
 * Note: Does not count cross-terms so bilinear and trilinear return 0.
 * @return  1 if basis function has non-linear terms, 0 if note.
 */
int FE_basis_is_non_linear(struct FE_basis *basis);

/* exposed only for comparing function pointers */
int monomial_basis_functions(void *type_arguments,
	const FE_value *xi_coordinates, FE_value *function_values);

/* exposed only for comparing function pointers */
int polygon_basis_functions(void *type_arguments,
	const FE_value *xi_coordinates, FE_value *function_values);

int standard_basis_function_is_monomial(Standard_basis_function *function,
	void *arguments_void);
/*******************************************************************************
LAST MODIFIED : 12 June 2002

DESCRIPTION :
Returns true if the standard basis function is a monomial.
==============================================================================*/

/***************************************************************************//**
 * Return the internal basis type array.
 * Not to be modified.
 * @param basis  The finite element basis object.
 * @return  const pointer to internal basis type array.
 */
const int *FE_basis_get_basis_type(struct FE_basis *basis);

/***************************************************************************//**
 * Return the number of basis functions in the basis. Note this is the original
 * number before blending i.e. to monomial basis is performed.
 * Example: biquadratic simplex has 6 functions, its blended monomial has 9.
 * @param basis  The finite element basis object.
 */
int FE_basis_get_number_of_functions(struct FE_basis *basis);

/***************************************************************************//**
 * Get the number of blended basis functions if a blend matrix is present.
 * Typically this number represents the number of full monomial terms needed to
 * represent the basis.
 * @param basis  The finite element basis object.
 * @return  Number of blended basis functions or 0 if no blending.
 */
int FE_basis_get_number_of_blended_functions(struct FE_basis *basis);

/***************************************************************************//**
 * Calculate blended element values by multiplying raw element values with the
 * basis' blending matrix.
 * @param basis  The finite element basis object.
 * @param raw_element_values  Array of element values for unblended basis
 * functions.
 * @return   Allocated array of blended element values appropriate for dot
 * product with standard blended [monomial] basis functions, or NULL on failure.
 */
FE_value *FE_basis_get_blended_element_values(struct FE_basis *basis,
	const FE_value *raw_element_values);

/***************************************************************************//**
 * Calculate combined blending matrix as product of inherited_blend_matrix
 * (number_of_blended_values rows * number_of_inherited_values columns)
 * with basis blending functions to give matrix with
 * number_of_functions rows * number_of_inherited_values columns.
 * @param basis  The finite element basis object.
 * @param inherited_blend_matrix  Blending matrix to multiply.
 * @return   Allocated array of combined blending matrix, or NULL on failure.
 */
FE_value *FE_basis_calculate_combined_blending_matrix(struct FE_basis *basis,
	int number_of_blended_values, int number_of_inherited_values,
	const FE_value *inherited_blend_matrix);

/***************************************************************************//**
 * @param basis  The finite element basis object.
 * @return  Pointer to standard basis function.
 */
Standard_basis_function *FE_basis_get_standard_basis_function(struct FE_basis *basis);

/***************************************************************************//**
 * Dangerous; do not modify or deallocate the returned array.
 * @param basis  The finite element basis object.
 * @return  Pointer to internal stanard basis function arguments.
 */
const int *FE_basis_get_standard_basis_function_arguments(struct FE_basis *basis);

/***************************************************************************//**
 */
int calculate_standard_basis_transformation(struct FE_basis *basis,
	FE_value *coordinate_transformation,int inherited_dimension,
	int **inherited_arguments_address,int *number_of_inherited_values_address,
	Standard_basis_function **inherited_standard_basis_function_address,
	FE_value **blending_matrix_address);

#endif /* !defined (FINITE_ELEMENT_BASIS_H) */
