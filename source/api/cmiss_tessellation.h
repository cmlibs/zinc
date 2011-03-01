/***************************************************************************//**
 * cmiss_tessellation.h
 *
 * Public interface to tessellation objects. These describe how elements or
 * other continuous field domains are subdivided into graphics.
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#if !defined (CMISS_TESSELLATION_H)
#define CMISS_TESSELLATION_H

#ifndef CMISS_TESSELLATION_ID_DEFINED
struct Cmiss_tessellation;
typedef struct Cmiss_tessellation * Cmiss_tessellation_id;
#define CMISS_TESSELLATION_ID_DEFINED
#endif

/***************************************************************************//**
 * Labels of tessellation attributes which may be set or obtained using generic
 * get/set_attribute functions.
 * Note: not all attributes can be set.
 */
enum Cmiss_tessellation_attribute_id
{
	CMISS_TESSELLATION_ATTRIBUTE_IS_MANAGED = 1,
	/*!< Boolean as integer, when 0 (default) tessellation is destroyed when no
	 * longer in use, i.e. when number of external references to it drops to
	 * zero. Set to 1 to manage tessellation object indefinitely, or until this
	 * attribute is reset to zero, effectively marking it as pending destruction.
	 */

	CMISS_TESSELLATION_ATTRIBUTE_MINIMUM_DIVISIONS_SIZE = 2,
	/*!< positive integer size of minimum divisions array, defaults to 1 or size
	 * set in last call to Cmiss_tessellation_set_minimum_divisions.
	 * @see Cmiss_tessellation_set_minimum_divisions.
	 */

	CMISS_TESSELLATION_ATTRIBUTE_REFINEMENT_FACTORS_SIZE = 3
	/*!< positive integer size of refinement factors array, defaults to 1 or size
	 * set in last call to Cmiss_tessellation_set_refinement_factors.
	 * @see Cmiss_tessellation_set_refinement_factors.
	 */
};

/*******************************************************************************
 * Returns a new reference to the tessellation with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param tessellation  The tessellation to obtain a new reference to.
 * @return  New tessellation reference with incremented reference count.
 */
Cmiss_tessellation_id Cmiss_tessellation_access(Cmiss_tessellation_id tessellation);

/*******************************************************************************
 * Destroys this reference to the tessellation (and sets it to NULL).
 * Internally this just decrements the reference count.
 */
int Cmiss_tessellation_destroy(Cmiss_tessellation_id *tessellation_address);

/***************************************************************************//**
 * Get an integer or Boolean attribute of the tessellation object.
 *
 * @param tessellation  Handle to the cmiss tessellation.
 * @param attribute_id  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
int Cmiss_tessellation_get_attribute_integer(Cmiss_tessellation_id tessellation,
	enum Cmiss_tessellation_attribute_id attribute_id);

/***************************************************************************//**
 * Set an integer or Boolean attribute of the tessellation object.
 *
 * @param tessellation  Handle to the cmiss tessellation.
 * @param attribute_id  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  1 if attribute successfully set, 0 if failed or attribute not valid
 * or able to be set for this tessellation object.
 */
int Cmiss_tessellation_set_attribute_integer(Cmiss_tessellation_id tessellation,
	enum Cmiss_tessellation_attribute_id attribute_id, int value);

/***************************************************************************//**
 * Return an allocated string containing tessellation name.
 *
 * @param tessellation  handle to the cmiss tessellation.
 * @return  allocated string containing tessellation name, or NULL on failure.
 */
char *Cmiss_tessellation_get_name(Cmiss_tessellation_id tessellation);

/***************************************************************************//**
 * Set/change name for <tessellation>.
 *
 * @param tessellation  The handle to cmiss tessellation.
 * @param name  name to be set to the tessellation
 * @return  1 if successfully set/change name for tessellation, otherwise 0.
 */
int Cmiss_tessellation_set_name(Cmiss_tessellation_id tessellation, const char *name);

/***************************************************************************//**
 * Gets the minimum number of line segments used to approximate curves in each
 * dimension for coarse tessellation.
 *
 * @see Cmiss_tessellation_set_minimum_divisions
 * @param tessellation  The tessellation object to query.
 * @param size  The size of the minimum_divisions array to fill. Values for
 * dimensions beyond the size set use the last divisions value.
 * @param minimum_divisions  Array to receive numbers of divisions.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_tessellation_get_minimum_divisions(Cmiss_tessellation_id tessellation,
	int size, int *minimum_divisions);

/***************************************************************************//**
 * Sets the minimum number of line segments used to approximate curves in each
 * dimension of tessellation. Intended to be used where coarse tessellation is
 * acceptable, e.g. where fields vary linearly over elements.
 * The default minimum_divisions value for new tessellations is 1, size 1.
 * Note: The value set for the last dimension applies to all higher dimensions.
 *
 * @param tessellation  The tessellation object to modify.
 * @param size  The size of the minimum_divisions array, >= 1.
 * @param minimum_divisions  Array of number of divisions (>=1) for each
 * dimension, with the last number in array applying to all higher dimensions.
 * @return  1 if successfully set, 0 on error.
 */
int Cmiss_tessellation_set_minimum_divisions(Cmiss_tessellation_id tessellation,
	int size, const int *minimum_divisions);

/***************************************************************************//**
 * Gets the refinements to be used in product with the minimum divisions
 * to approximate curves in each dimension for fine tessellation.
 *
 * @see Cmiss_tessellation_set_refinement_factors
 * @param tessellation  The tessellation object to query.
 * @param size  The size of the refinement_factors array to fill. Values for
 * dimensions beyond the size set use the last refinement value.
 * @param refinement_factors  Array to receive refinement factors.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_tessellation_get_refinement_factors(Cmiss_tessellation_id tessellation,
	int size, int *refinement_factors);

/***************************************************************************//**
 * Sets the refinements to be used in product with the minimum divisions
 * to approximate curves in each dimension for fine tessellation. To be used
 * e.g. where fields vary non-linearly over elements, or greater data sampling
 * is needed for rendering with a spectrum.
 * The default refinement_factors value for new tessellations is 1, size 1.
 * Note: The value set for the last dimension applies to all higher dimensions.
 *
 * @see Cmiss_tessellation_set_minimum_divisions
 * @param tessellation  The tessellation object to modify.
 * @param size  The size of the refinement_factors array, >= 1.
 * @param refinement_factors  Array of number of fine subdivisions (>=1) per
 * minimum_division for each dimension, with the last number in array
 * applying to all higher dimensions.
 * @return  1 if successfully set, 0 on error.
 */
int Cmiss_tessellation_set_refinement_factors(Cmiss_tessellation_id tessellation,
	int size, const int *refinement_factors);

#endif
