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
#include "api/types/cmiss_field_id.h"
#include "api/types/cmiss_field_module_id.h"
#include "api/types/cmiss_optimisation_id.h"
#include "api/types/cmiss_region_id.h"

/**
 * The optimisation methods available via the Cmiss_optimisation object.
 *
 * @todo Might be worth separating the non-linear problem setup from the optimisation algorithm to mirror
 * the underlying Opt++ structure?
 */
enum Cmiss_optimisation_method
{
	CMISS_OPTIMISATION_METHOD_QUASI_NEWTON, /*!< The default optimisation method. Suitable
		for most problems with a small set of independent parameters. */
	CMISS_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON, /*!< A least squares method better
		suited to larger problems. */
	CMISS_OPTIMISATION_METHOD_NSDGSL, /*!< Duane's original hard-coded normalised steepest decent
		with a Golden section line search. */
	CMISS_OPTIMISATION_METHOD_INVALID
};

enum Cmiss_optimisation_attribute_id
{
	CMISS_OPTIMISATION_ATTRIBUTE_DIMENSION,
	CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS,
	CMISS_OPTIMISATION_ATTRIBUTE_MAXIMUM_NUMBER_FUNCTION_EVALUATIONS,
	CMISS_OPTIMISATION_ATTRIBUTE_ABSOLUTE_TOLERANCE,
	CMISS_OPTIMISATION_ATTRIBUTE_RELATIVE_TOLERANCE,
	CMISS_OPTIMISATION_ATTRIBUTE_OBJECTIVE_FIELD,
	CMISS_OPTIMISATION_ATTRIBUTE_DATA_FIELD, /* !< Only used with LSQ methods. */
	CMISS_OPTIMISATION_ATTRIBUTE_MESH_FIELD /* !< Only used with LSQ methods. */
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
Cmiss_optimisation_id Cmiss_field_module_create_optimisation(Cmiss_field_module_id field_module);

/**
 * Destroys reference to the optimisation object and sets pointer/handle to NULL.
 *
 * @param optimisation_address  Address of optimisation object reference.
 * @return  1 on success, 0 if invalid arguments.
 */
int Cmiss_optimisation_destroy(Cmiss_optimisation_id *optimisation_address);

/**
 * Sets the mesh region (group?) for this optimisation object.
 *
 * @param optimisation Handle to the cmiss optimisation object.
 * @param region The mesh region.
 * @return 1 on success; 0 otherwise.
 */
int Cmiss_optimisation_set_mesh_region(Cmiss_optimisation_id optimisation, Cmiss_region_id region);

/**
 * Sets the data region (group?) for this optimisation object.
 *
 * @param optimisation Handle to the cmiss optimisation object.
 * @param region The data region.
 * @return 1 on success; 0 otherwise.
 */
int Cmiss_optimisation_set_data_region(Cmiss_optimisation_id optimisation, Cmiss_region_id region);

/**
 * Get the current optimisation method for the given optimisation object.
 *
 * @param optimisation Handle to the cmiss optimisation object.
 * @return The current optimisation method.
 */
enum Cmiss_optimisation_method Cmiss_optimisation_get_method(Cmiss_optimisation_id optimisation);

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
 * @param attribute_id  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
int Cmiss_optimisation_get_attribute_integer(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute_id attribute_id);

/**
 * Set an integer or Boolean attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute_id  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  1 if attribute successfully set, 0 if failed or attribute not valid
 * or able to be set for this optimisation object.
 */
int Cmiss_optimisation_set_attribute_integer(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute_id attribute_id, int value);

/**
 * Get a real attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute_id  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
double Cmiss_optimisation_get_attribute_real(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute_id attribute_id);

/**
 * Set a real attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute_id  The identifier of the real attribute to set.
 * @param value  The new value for the attribute.
 * @return  1 if attribute successfully set, 0 if failed or attribute not valid
 * or able to be set for this optimisation object.
 */
int Cmiss_optimisation_set_attribute_real(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute_id attribute_id, double value);

/**
 * Get a field attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute_id  The identifier of the field attribute to get.
 * @return  Value of the attribute (should be destroyed with Cmiss_field_destroy()).
 */
Cmiss_field_id Cmiss_optimisation_get_attribute_field(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute_id attribute_id);

/**
 * Set a field attribute of the optimisation object.
 *
 * @param optimisation  Handle to the cmiss optimisation object.
 * @param attribute_id  The identifier of the field attribute to set.
 * @param value  The new value for the attribute (accessed internally so safe for caller to destroy locally).
 * @return  1 if attribute successfully set, 0 if failed or attribute not valid
 * or able to be set for this optimisation object.
 */
int Cmiss_optimisation_set_attribute_field(Cmiss_optimisation_id optimisation,
		enum Cmiss_optimisation_attribute_id attribute_id, Cmiss_field_id value);

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
int Cmiss_optimisation_add_independent_field(Cmiss_optimisation_id optimisation,
		Cmiss_field_id field);

/**
 * Perform the optimisation described by the provided optimisation object.
 *
 * @param optimisation Handle to the Cmiss optimisation object.
 * @return 1 if optimisation completed sucessfully; 0 otherwise.
 */
int Cmiss_optimisation_optimise(Cmiss_optimisation_id optimisation);

#endif /* __CMISS_OPTIMISATION_H__ */
