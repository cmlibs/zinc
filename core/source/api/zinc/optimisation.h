/***************************************************************************//**
 * @file cmiss_optimisation.h
 *
 * The public interface to Cmiss_optimisation class which can minimise N
 * objective functions by modifying parameters of M independent fields.
 *
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#ifndef CMZN_OPTIMISATION_H__
#define CMZN_OPTIMISATION_H__

/*
 Global types
 ------------
 */

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/optimisationid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * The optimisation methods available via the Cmiss_optimisation object.
 *
 * @todo Might be worth separating the non-linear problem setup from the optimisation algorithm to mirror
 * the underlying Opt++ structure?
 */
enum Cmiss_optimisation_method
{
	CMISS_OPTIMISATION_METHOD_INVALID = 0,
	/*!< Invalid or unspecified optimisation method.
	 */
	CMISS_OPTIMISATION_METHOD_QUASI_NEWTON = 1,
	/*!< The default optimisation method. Suitable for most problems with a small
	 * set of independent parameters.
	 * Given a scalar valued objective function (scalar sum of all objective
	 * fields' components), finds the set of DOFs for the independent field(s)
	 * which minimises the objective function value.
	 */
	CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON = 2,
	/*!< A least squares method better suited to larger problems.
	 * Finds the set of independent field(s) DOF values which minimises the
	 * squares of the objective components supplied. Works specially with fields
	 * giving sum-of-squares e.g. nodeset_sum_squares, nodeset_mean_squares to
	 * supply individual terms before squaring to the optimiser.
	 */
};

/***************************************************************************//**
 * Labels of optimisation attributes which may be set or obtained using generic
 * get/set_attribute functions.
 */
enum Cmiss_optimisation_attribute
{
	CMISS_OPTIMISATION_ATTRIBUTE_FUNCTION_TOLERANCE = 1,
	/*!< (Opt++ stopping tolerance) Assigns a stopping tolerance for an optimisation algorithm. Please
	 * assign tolerances that make sense given the accuracy of your function. For example, setting
	 * TOLERANCE to 1.e-4 in your problem means the optimisation algorithm converges when the function
	 * value from one iteration to the next changes by 1.e-4 or less.
	 *
	 * Default value: 1.49012e-8
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_GRADIENT_TOLERANCE = 2,
	/*!< (Opt++ stopping tolerance) Assigns a stopping tolerance for an optimisation algorithm. Please
	 * assign tolerances that make sense given your function accuracy. For example, setting
	 * GRADIENT_TOLERANCE to 1.e-6 in your problem means the optimisation algorithm converges when the
	 * absolute or relative norm of the gradient is 1.e-6 or less.
	 *
	 * Default value: 6.05545e-6
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_STEP_TOLERANCE = 3,
	/*!< (Opt++ stopping tolerance) Assigns a stopping tolerance for the optimisation algorithm. Please
	 * set tolerances that make sense, given the accuracy of your function. For example, setting
	 * STEP_TOLERANCE to 1.e-2 in your problem means the optimisation algorithm converges when the relative
	 * steplength is 1.e-2 or less.
	 *
	 * Default value: 1.49012e-8
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS = 4,
	/*!< (Opt++ stopping tolerance) Places a limit on the number of iterations of the optimisation algorithm.
	 * It is useful when your
	 * function is computationally expensive or you are debugging the optimisation algorithm. When
	 * MAXIMUM_ITERATIONS iterations evaluations have been completed, the optimisation algorithm will stop
	 * and report the solution it has reached at that point. It may not be the optimal solution, but it will
	 * be the best it could provide given the limit on the number of iterations.
	 *
	 * Default value: 100.
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_FUNCTION_EVALUATIONS = 5,
	/*!< (Opt++ stopping tolerance) Places an upper bound on the number of function evaluations. The method
	 * is useful when your function
	 * is computationally expensive and you only have time to perform a limited number of evaluations. When
	 * MAXIMUM_NUMBER_FUNCTION_EVALUATIONS function evaluations have been completed, the optimisation
	 * algorithm will stop and report the solution it has reached at that point. It may not be the optimal
	 * solution, but it will be the best it could provide given the limit on the number of function evaluations.
	 *
	 * Default value: 1000
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_STEP = 6,
	/*<! (Opt++ steplength control) Places an upper bound on the length of the step that can be taken at each
	 * iteration of the optimisation
	 * algorithm. If the scale of your optimisation parameters exceeds the bound, adjust accordingly. If you want
	 * to be conservative in your search, you may want to set MAXIMUM_STEP to a smaller value than the default. In
	 * our (Opt++) experience, the default value is generally fine.
	 *
	 * Default value: 1.0e3
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MINIMUM_STEP = 7,
	/*<! (Opt++ steplength control) Places a lower bound on the length of the step that can be taken at each
	 * iteration of the optimisation
	 * algorithm. If the scale of your optimisation parameters exceeds the bound, adjust accordingly. If you
	 * expect the optimisation algorithm to navigate some tricky areas, set MINIMUM_STEP to a smaller value than
	 * the default. In our (Opt++) experience, the default value is generally fine.
	 *
	 * Default value: 1.49012e-8
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_LINESEARCH_TOLERANCE = 8,
	/*!< (Opt++ globalisation strategy parameter) In practice, the linesearch tolerance is set to a small value,
	 * so that almost any decrease in the function value results in an acceptable step. Suggested values are
	 * 1.e-4 for Newton methods and 1.e-1 for more exact line searches.
	 *
	 * Default value: 1.e-4
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_BACKTRACK_ITERATIONS = 9,
	/*<! (Opt++ globalisation strategy parameter) Only relevant when you use a algorithm with a linesearch
	 * search strategy. The value places a limit on the number of iterations in the linesearch routine of the
	 * optimisation algorithm. If the limit is reached before computing a step with acceptable decrease, the
	 * algorithm terminates with an error message. The reported solution is not optimal, but the best one
	 * given the number of linesearch iterations. Increasing the number of linesearch iterations may lead to
	 * an acceptable step, but it also results in more function evaluations and a shorter steplength.
	 *
	 * Default value: 5
	 */
	/*
	 * @todo Reserving this one for when trust region methods are available via the API. Currently everything
	 * uses linesearch methods only.
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_TRUST_REGION_SIZE = 10,
	/*<! (Opt++ globalisation strategy parameter) Only relevant when you are using an algorithm with a trust-region
	 * or a trustpds search strategy. The value initialises the size of the trust region.
	 *
	 * Default value: 0.1?? (@todo Need to better initialise the default value, see https://software.sandia.gov/opt++/opt++2.4_doc/html/ControlParameters.html)
	 *
	 * If your problem is quadratic or close to it, you may want to initialise the size of the trust region to a
	 * larger value.
	 */
};

/*
 Global functions
 ----------------
 */

/***************************************************************************//**
 * Create an optimisation object for optimising values and parameters of fields
 * from a field module.
 *
 * @param field_module  The field module to optimise fields from.
 * @return  Handle to the newly created optimisation object, or NULL on failure.
 * Cmiss_optimisation_destroy must be called to destroy the handle.
 */
ZINC_API Cmiss_optimisation_id Cmiss_field_module_create_optimisation(
	Cmiss_field_module_id field_module);

/*******************************************************************************
 * Returns a new handle to the optimisation with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param optimisation  The optimisation to obtain a new reference to.
 * @return  New optimisation handle with incremented reference count.
 */
ZINC_API Cmiss_optimisation_id Cmiss_optimisation_access(Cmiss_optimisation_id optimisation);

/***************************************************************************//**
 * Destroys reference to the optimisation object and sets pointer/handle to NULL.
 *
 * @param optimisation_address  Address of optimisation object reference.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_optimisation_destroy(Cmiss_optimisation_id *optimisation_address);

/***************************************************************************//**
 * Get the current optimisation method for the given optimisation object.
 *
 * @param optimisation  Handle to the optimisation object.
 * @return  The current optimisation method.
 */
ZINC_API enum Cmiss_optimisation_method Cmiss_optimisation_get_method(
	Cmiss_optimisation_id optimisation);

/***************************************************************************//**
 * Set the optimisation method for the given optimisation object.
 *
 * @param optimisation  Handle to the optimisation object.
 * @param method  The optimisation method to use.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_optimisation_set_method(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_method method);

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_optimisation_method Cmiss_optimisation_method_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param format  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_optimisation_method_enum_to_string(enum Cmiss_optimisation_method method);

/***************************************************************************//**
 * Get an integer or Boolean attribute of the optimisation object.
 *
 * @param optimisation  Handle to the optimisation object.
 * @param attribute  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
ZINC_API int Cmiss_optimisation_get_attribute_integer(Cmiss_optimisation_id optimisation,
	enum Cmiss_optimisation_attribute attribute);

/***************************************************************************//**
 * Set an integer or Boolean attribute of the optimisation object.
 *
 * @param optimisation  Handle to the optimisation object.
 * @param attribute  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  Status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid or able to be set for this optimisation object.
 */
ZINC_API int Cmiss_optimisation_set_attribute_integer(Cmiss_optimisation_id optimisation,
	enum Cmiss_optimisation_attribute attribute, int value);

/***************************************************************************//**
 * Get a real attribute of the optimisation object.
 *
 * @param optimisation  Handle to the optimisation object.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
ZINC_API double Cmiss_optimisation_get_attribute_real(Cmiss_optimisation_id optimisation,
	enum Cmiss_optimisation_attribute attribute);

/***************************************************************************//**
 * Set a real attribute of the optimisation object.
 *
 * @param optimisation  Handle to the optimisation object.
 * @param attribute  The identifier of the real attribute to set.
 * @param value  The new value for the attribute.
 * @return  Status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid or able to be set for this optimisation object.
 */
ZINC_API int Cmiss_optimisation_set_attribute_real(Cmiss_optimisation_id optimisation,
	enum Cmiss_optimisation_attribute attribute, double value);

/***************************************************************************//**
 * Convert a short attribute name into an enum if the attribute name matches
 * any of the members in the enum.
 *
 * @param attribute_name  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_optimisation_attribute Cmiss_optimisation_attribute_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_optimisation_attribute_enum_to_string(
	enum Cmiss_optimisation_attribute attribute);

/***************************************************************************//**
 * Get the first independent field from the optimisation problem description.
 * @see Cmiss_optimisation_get_next_independent_field
 *
 * @param optimisation  Handle to the optimisation object to query.
 * @return  Handle to first independent field or NULL if none. Up to caller to
 * destroy the returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_optimisation_get_first_independent_field(
	Cmiss_optimisation_id optimisation);

/***************************************************************************//**
 * Get the next independent field in the optimisation problem description after
 * the supplied ref_field. Use to iterate over the independent fields, taking
 * care to destroy all returned field handles exactly once:
 *
 * Cmiss_field_id field = Cmiss_optimisation_get_first_objective_field(optimisation);
 * while (field)
 * {
 *    Cmiss_field_id next_field = Cmiss_optimisation_get_next_objective_field(optimisation, field);
 *    Cmiss_field_destroy(&field);
 *    field = next_field;
 * }
 *
 * @param optimisation  Handle to the optimisation object to query.
 * @param ref_field  Handle to an independent field from the optimisation.
 * @return  Handle to next independent field after ref_field or NULL if none.
 * Up to caller to destroy the returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_optimisation_get_next_independent_field(
	Cmiss_optimisation_id optimisation, Cmiss_field_id ref_field);

/***************************************************************************//**
 * Add an independent field to the given optimisation problem description.
 * Valid independent fields are limited to constant and finite_element types.
 * The parameters of these fields are modified to minimise the objective fields.
 * NOTE: Beware that many existing cubic Hermite meshes in EX format do not
 * correctly share common value or derivative versions and thus will 'open up'
 * during fitting/optimisation.
 *
 * @param optimisation  Handle to the optimisation object.
 * @param field  Real-valued independent field to add to the optimisation object
 * (accessed internally so safe for caller to destroy locally).
 * @return  Status CMISS_OK if field successfully added, any other value if
 * failed or already added.
 */
ZINC_API int Cmiss_optimisation_add_independent_field(Cmiss_optimisation_id optimisation,
	Cmiss_field_id field);

/***************************************************************************//**
 * Remove an independent field from the optimisation problem.
 *
 * @param optimisation  Handle to the optimisation object.
 * @param field  The independent field to remove.
 * @return  Status CMISS_OK if field successfully removed, any other value if
 * failed or field not found.
 */
ZINC_API int Cmiss_optimisation_remove_independent_field(
	Cmiss_optimisation_id optimisation, Cmiss_field_id field);

/***************************************************************************//**
 * Get the first objective field from the optimisation problem description.
 * @see Cmiss_optimisation_get_next_objective_field
 *
 * @param optimisation  Handle to the optimisation object to query.
 * @return  Handle to first objective field or NULL if none. Up to caller to
 * destroy the returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_optimisation_get_first_objective_field(
	Cmiss_optimisation_id optimisation);

/***************************************************************************//**
 * Get the next objective field in the optimisation problem description after
 * the supplied ref_field. Use to iterate over the objective fields, taking
 * care to destroy all returned field handles exactly once:
 *
 * Cmiss_field_id field = Cmiss_optimisation_get_first_objective_field(optimisation);
 * while (field)
 * {
 *    Cmiss_field_id next_field = Cmiss_optimisation_get_next_objective_field(optimisation, field);
 *    Cmiss_field_destroy(&field);
 *    field = next_field;
 * }
 *
 * @param optimisation  Handle to the optimisation object to query.
 * @param ref_field  Handle to an objective field from the optimisation.
 * @return  Handle to next objective field after ref_field or NULL if none.
 * Up to caller to destroy the returned handle.
 */
ZINC_API Cmiss_field_id Cmiss_optimisation_get_next_objective_field(
	Cmiss_optimisation_id optimisation, Cmiss_field_id ref_field);

/***************************************************************************//**
 * Add an objective field to the optimisation problem description.
 * Valid objective fields must be spatially constant. The least squares solution
 * method treats fields performing a sum of squares (nodeset_sum_squares,
 * nodeset_mean_squares) specially, passing each term to the optimiser.
 * The overall objective function becomes the sum of all components of all
 * objective fields, or for the least-squares method, the sum of the squares of
 * all terms (or components if the objective field is not a sum of squares).
 *
 * @param optimisation  Handle to the optimisation object.
 * @param field  Real-valued objective field to add to the optimisation object
 * (accessed internally so safe for caller to destroy locally).
 * @return  Status CMISS_OK if field successfully added, any other value if
 * failed or already added.
 */
ZINC_API int Cmiss_optimisation_add_objective_field(Cmiss_optimisation_id optimisation,
	Cmiss_field_id field);

/***************************************************************************//**
 * Remove an objective field from the optimisation problem.
 *
 * @param optimisation  Handle to the optimisation object.
 * @param field  The objective field to remove.
 * @return   Status CMISS_OK if field successfully removed, any other value if
 * failed or field not found.
 */
ZINC_API int Cmiss_optimisation_remove_objective_field(
	Cmiss_optimisation_id optimisation, Cmiss_field_id field);

/***************************************************************************//**
 * Get a textual report on the last solution.
 *
 * @param optimisation  Handle to the optimisation object to query.
 * @return  Allocated string containing report which user must free using
 * Cmiss_deallocate, or NULL on error or if haven't performed optimisation yet.
 */
ZINC_API char *Cmiss_optimisation_get_solution_report(Cmiss_optimisation_id optimisation);

/***************************************************************************//**
 * Perform the optimisation described by the provided optimisation object.
 *
 * @param optimisation Handle to the Cmiss optimisation object.
 * @return Status CMISS_OK if optimisation completed successfully (stopping
 * criteria satisfied), and any other value on failure.
 */
ZINC_API int Cmiss_optimisation_optimise(Cmiss_optimisation_id optimisation);

#ifdef __cplusplus
}
#endif

#endif
