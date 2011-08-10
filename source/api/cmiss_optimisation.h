/**
 * @file cmiss_optimisation.h
 *
 * The public interface to Cmiss_optimisation.
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
 * Portions created by the Initial Developer are Copyright (C) 2005-2010
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
#ifndef __CMISS_OPTIMISATION_H__
#define __CMISS_OPTIMISATION_H__

/*
 Global types
 ------------
 */
#include "api/types/cmiss_element_id.h"
#include "api/types/cmiss_field_id.h"
#include "api/types/cmiss_field_module_id.h"
#include "api/types/cmiss_node_id.h"
#include "api/types/cmiss_optimisation_id.h"
#include "api/types/cmiss_region_id.h"

/**
 * The optimisation methods available via the Cmiss_optimisation object.
 *
 * @todo Might be worth separating the non-linear problem setup from the optimisation algorithm to mirror
 * the underlying Opt++ structure?
 */
enum Cmiss_optimisation_method {
	CMISS_OPTIMISATION_METHOD_INVALID = 0, /**< unspecified optimisation method. */
	CMISS_OPTIMISATION_METHOD_QUASI_NEWTON = 1,
	/*!< The default optimisation method. Suitable for most problems with a
	 * small set of independent parameters.
	 * Given a scalar valued objective field, finds the set of component values for the specified
	 * independent field(s) which minimises the objective field value.
	 */
	CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON = 2,
	/*!< A least squares method better suited to larger problems.
	 * Finds the set of independent field(s) component values which minimises the error. The error
	 * is defined in a least-squares sense as the difference in the DATA_FIELD value a each of the
	 * data points in the <data_nodeset> and the value of the MESH_FIELD at the corresponding projection
	 * location in the <mesh>.
	 */
	CMISS_OPTIMISATION_METHOD_NSDGSL = 3,
	/*!< Internal normalised steepest decent with a Golden section
	 * line search.
	 */
};

/**
 * Labels of optimisation attributes which may be set or obtained using generic
 * get/set_attribute functions.
 */
enum Cmiss_optimisation_attribute {
	CMISS_OPTIMISATION_ATTRIBUTE_OBJECTIVE_FIELD = 2,
	/*!< In Quasi-Newton and NSDGSL optimisation, this is the field which defines the current value
	 * of the objective function. It is expected, but not checked, to be a scalar valued constant
	 * field and as such, the objective function value is defined as the value of this field.
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_DATA_FIELD = 3,
	/*!< Only used with LSQ methods.
	 * What is it used for?
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MESH_FIELD = 4,
	/*!< Only used with LSQ methods.
	 * What is it used for?
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_FUNCTION_TOLERANCE = 5,
	/*!< (Opt++ stopping tolerance) Assigns a stopping tolerance for an optimisation algorithm. Please
	 * assign tolerances that make sense given the accuracy of your function. For example, setting
	 * TOLERANCE to 1.e-4 in your problem means the optimisation algorithm converges when the function
	 * value from one iteration to the next changes by 1.e-4 or less.
	 *
	 * Default value: 1.49012e-8
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_GRADIENT_TOLERANCE = 6,
	/*!< (Opt++ stopping tolerance) Assigns a stopping tolerance for an optimisation algorithm. Please
	 * assign tolerances that make sense given your function accuracy. For example, setting
	 * GRADIENT_TOLERANCE to 1.e-6 in your problem means the optimisation algorithm converges when the
	 * absolute or relative norm of the gradient is 1.e-6 or less.
	 *
	 * Default value: 6.05545e-6
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_STEP_TOLERANCE = 7,
	/*!< (Opt++ stopping tolerance) Assigns a stopping tolerance for the optimisation algorithm. Please
	 * set tolerances that make sense, given the accuracy of your function. For example, setting
	 * STEP_TOLERANCE to 1.e-2 in your problem means the optimisation algorithm converges when the relative
	 * steplength is 1.e-2 or less.
	 *
	 * Default value: 1.49012e-8
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS = 8,
	/*!< (Opt++ stopping tolerance) Places a limit on the number of iterations of the optimisation algorithm.
	 * It is useful when your
	 * function is computationally expensive or you are debugging the optimisation algorithm. When
	 * MAXIMUM_ITERATIONS iterations evaluations have been completed, the optimisation algorithm will stop
	 * and report the solution it has reached at that point. It may not be the optimal solution, but it will
	 * be the best it could provide given the limit on the number of iterations.
	 *
	 * Default value: 100.
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_FUNCTION_EVALUATIONS = 9,
	/*!< (Opt++ stopping tolerance) Places an upper bound on the number of function evaluations. The method
	 * is useful when your function
	 * is computationally expensive and you only have time to perform a limited number of evaluations. When
	 * MAXIMUM_NUMBER_FUNCTION_EVALUATIONS function evaluations have been completed, the optimisation
	 * algorithm will stop and report the solution it has reached at that point. It may not be the optimal
	 * solution, but it will be the best it could provide given the limit on the number of function evaluations.
	 *
	 * Default value: 1000
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_STEP = 10,
	/*<! (Opt++ steplength control) Places an upper bound on the length of the step that can be taken at each
	 * iteration of the optimisation
	 * algorithm. If the scale of your optimisation parameters exceeds the bound, adjust accordingly. If you want
	 * to be conservative in your search, you may want to set MAXIMUM_STEP to a smaller value than the default. In
	 * our (Opt++) experience, the default value is generally fine.
	 *
	 * Default value: 1.0e3
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MINIMUM_STEP = 11,
	/*<! (Opt++ steplength control) Places a lower bound on the length of the step that can be taken at each
	 * iteration of the optimisation
	 * algorithm. If the scale of your optimisation parameters exceeds the bound, adjust accordingly. If you
	 * expect the optimisation algorithm to navigate some tricky areas, set MINIMUM_STEP to a smaller value than
	 * the default. In our (Opt++) experience, the default value is generally fine.
	 *
	 * Default value: 1.49012e-8
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_LINESEARCH_TOLERANCE = 12,
	/*!< (Opt++ globalisation strategy parameter) In practice, the linesearch tolerance is set to a small value,
	 * so that almost any decrease in the function value results in an acceptable step. Suggested values are
	 * 1.e-4 for Newton methods and 1.e-1 for more exact line searches.
	 *
	 * Default value: 1.e-4
	 */
	CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_BACKTRACK_ITERATIONS = 13,
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
	CMISS_OPTIMISATION_ATTRIBUTE_TRUST_REGION_SIZE = 14,
	/*<! (Opt++ globalisation strategy parameter) Only relevant when you are using an algorithm with a trust-region
	 * or a trustpds search strategy. The value initialises the size of the trust region.
	 *
	 * Default value: 0.1?? (@todo Need to better initialise the default value, see https://software.sandia.gov/opt++/opt++2.4_doc/html/ControlParameters.html)
	 *
	 * If your problem is quadratic or close to it, you may want to initialise the size of the trust region to a
	 * larger value.
	 *
	 */
};

/*
 Global functions
 ----------------
 */
/**
 * Create a Cmiss_optimisation object. The factory object to be used in creating a
 * Cmiss_optimisation is up in the air. Perhaps belongs to the context since it potentially
 * applies across multiple regions and uses multiple fields. Will go with field module for
 * now. Need to define the method outside computed_filed.cpp to avoid adding a dependency
 * to the field library.
 *
 * @param field_module The field module to use in creating the optimisation object.
 * @return The newly created optimisation object, or NULL on failure. Cmiss_optimisation_destroy
 * should be used to free the object once it is no longer needed.
 */
Cmiss_optimisation_id Cmiss_field_module_create_optimisation(
		Cmiss_field_module_id field_module);

/**
 * Destroys reference to the optimisation object and sets pointer/handle to NULL.
 *
 * @param optimisation_address  Address of optimisation object reference.
 * @return  1 on success, 0 if invalid arguments.
 */
int Cmiss_optimisation_destroy(Cmiss_optimisation_id *optimisation_address);

/**
 * Sets the mesh to be used for this optimisation object.
 *
 * @param optimisation Handle to the optimisation object.
 * @param mesh The mesh to assign to this optimisation object.
 * @return 1 on success; 0 otherwise.
 */
int Cmiss_optimisation_set_mesh(Cmiss_optimisation_id optimisation,
		Cmiss_mesh_id mesh);

/**
 * Sets the data nodeset to use for this optimisation object. This is only required when
 * performing a least-squares optimisation problem.
 *
 * @param optimisation Handle to the optimisation object.
 * @param nodeset The nodeset containing the data points to be used in a least-squares optimisation problem.
 * @return 1 on success; 0 otherwise.
 */
int Cmiss_optimisation_set_data_nodeset(Cmiss_optimisation_id optimisation,
		Cmiss_nodeset_id nodeset);

/**
 * Get the current optimisation method for the given optimisation object.
 *
 * @param optimisation Handle to the cmiss optimisation object.
 * @return The current optimisation method.
 */
enum Cmiss_optimisation_method Cmiss_optimisation_get_method(
		Cmiss_optimisation_id optimisation);

/**
 * Set the optimisation method for the given optimisation object.
 *
 * @param optimisation Handle to the cmiss optimisation object.
 * @param method The optimisation method to use.
 * @return 1 if method successfully set, 0 if failed.
 */
int Cmiss_optimisation_set_method(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_method method);

/**
 * Get an integer or Boolean attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
int Cmiss_optimisation_get_attribute_integer(
		Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute);

/**
 * Set an integer or Boolean attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  1 if attribute successfully set, 0 if failed or attribute not valid
 * or able to be set for this optimisation object.
 */
int Cmiss_optimisation_set_attribute_integer(
		Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute, int value);

/**
 * Get a real attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
double Cmiss_optimisation_get_attribute_real(
		Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute);

/**
 * Set a real attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute  The identifier of the real attribute to set.
 * @param value  The new value for the attribute.
 * @return  1 if attribute successfully set, 0 if failed or attribute not valid
 * or able to be set for this optimisation object.
 */
int Cmiss_optimisation_set_attribute_real(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute, double value);

/**
 * Get a field attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute  The identifier of the field attribute to get.
 * @return  Value of the attribute (should be destroyed with Cmiss_field_destroy()).
 */
Cmiss_field_id Cmiss_optimisation_get_attribute_field(
		Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute attribute);

/**
 * Set a field attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute  The identifier of the field attribute to set.
 * @param value  The new value for the attribute (accessed internally so safe for caller to destroy locally).
 * @return  1 if attribute successfully set, 0 if failed or attribute not valid
 * or able to be set for this optimisation object.
 */
int
		Cmiss_optimisation_set_attribute_field(
				Cmiss_optimisation_id optimisation,
				enum Cmiss_optimisation_attribute attribute,
				Cmiss_field_id value);

/**
 * Add an independent field to the given optimisation problem description.
 *
 * @todo Need to also be able to get/remove independent fields.
 *
 * @param optimisation Handle to the Cmiss optimisation object.
 * @param field The independent field to add to the optimisation object (accessed
 * internally so safe for caller to destroy locally).
 * @return 1 if field successfully added, 0 if failed.
 */
int Cmiss_optimisation_add_independent_field(
		Cmiss_optimisation_id optimisation, Cmiss_field_id field);

/**
 * Perform the optimisation described by the provided optimisation object.
 *
 * @param optimisation Handle to the Cmiss optimisation object.
 * @return 1 if optimisation completed sucessfully; 0 otherwise.
 */
int Cmiss_optimisation_optimise(Cmiss_optimisation_id optimisation);

#endif /* __CMISS_OPTIMISATION_H__ */
