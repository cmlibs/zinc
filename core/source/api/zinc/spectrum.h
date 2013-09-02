/***************************************************************************//**
 * FILE : spectrum.h
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

#ifndef CMZN_SPECTRUM_H__
#define CMZN_SPECTRUM_H__

#include "types/spectrumid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* Returns a new reference to the spectrum module with reference count
* incremented. Caller is responsible for destroying the new reference.
*
* @param spectrum_module  The spectrum module to obtain a new reference to.
* @return  spectrum module with incremented reference count.
*/
ZINC_API cmzn_spectrum_module_id cmzn_spectrum_module_access(
	cmzn_spectrum_module_id spectrum_module);

/**
* Destroys this reference to the spectrum module (and sets it to NULL).
* Internally this just decrements the reference count.
*
* @param spectrum_module_address  Address of handle to spectrum module
*   to destroy.
* @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
*/
ZINC_API int cmzn_spectrum_module_destroy(
	cmzn_spectrum_module_id *spectrum_module_address);

/**
 * Create and return a handle to a new spectrum.
 *
 * @param spectrum_module  The handle to the spectrum module the
 * spectrum will belong to.
 * @return  Handle to the newly created spectrum if successful, otherwise NULL.
 */
ZINC_API cmzn_spectrum_id cmzn_spectrum_module_create_spectrum(
	cmzn_spectrum_module_id spectrum_module);

/**
* Begin caching or increment cache level for this spectrum module. Call this
* function before making multiple changes to minimise number of change messages
* sent to clients. Must remember to end_change after completing changes.
* @see cmzn_spectrum_module_end_change
*
* @param spectrum_module  The spectrum_module to begin change cache on.
* @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
*/
ZINC_API int cmzn_spectrum_module_begin_change(cmzn_spectrum_module_id spectrum_module);

/***************************************************************************//**
* Decrement cache level or end caching of changes for the spectrum module.
* Call cmzn_spectrum_module_begin_change before making multiple changes
* and call this afterwards. When change level is restored to zero,
* cached change messages are sent out to clients.
*
* @param spectrum_module  The glyph_module to end change cache on.
* @return  Status CMZN_OK on success, any other value on failure.
*/
ZINC_API int cmzn_spectrum_module_end_change(cmzn_spectrum_module_id spectrum_module);

/**
* Find the spectrum with the specified name, if any.
*
* @param spectrum_module  spectrum module to search.
* @param name  The name of the spectrum.
* @return  Handle to the spectrum of that name, or 0 if not found.
* 	Up to caller to destroy returned handle.
*/
ZINC_API cmzn_spectrum_id cmzn_spectrum_module_find_spectrum_by_name(
	cmzn_spectrum_module_id spectrum_module, const char *name);

/**
* Get the default spectrum, if any. By default, a single component spectrum
* with CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_RAINBOW is returned.
* Call cmzn_spectrum_module_set_default_spectrum to change the default
* spectrum.
*
* @param spectrum_module  spectrum module to query.
* @return  Handle to the default spectrum, or 0 if none.
* 	Up to caller to destroy returned handle.
*/
ZINC_API cmzn_spectrum_id cmzn_spectrum_module_get_default_spectrum(
	cmzn_spectrum_module_id spectrum_module);

/**
* Set the default spectrum.
*
* @param spectrum_module  spectrum module to modify
* @param spectrum  The spectrum to set as default.
* @return  CMZN_OK on success otherwise CMZN_ERROR_ARGUMENT.
*/
ZINC_API int cmzn_spectrum_module_set_default_spectrum(
	cmzn_spectrum_module_id spectrum_module,
	cmzn_spectrum_id spectrum);

/**
 * Access the spectrum, increase the access count of the time keeper by one.
 *
 * @param spectrum  handle to the "to be access" zinc spectrum.
 * @return  handle to spectrum if successfully access spectrum.
 */
ZINC_API cmzn_spectrum_id cmzn_spectrum_access(cmzn_spectrum_id spectrum);

/**
 * Destroy the spectrum.
 *
 * @param spectrum  address to the handle to the "to be destroyed"
 *   zinc spectrum.
 * @return  status CMZN_OK if successfully destroy spectrum, any other value
 * on failure.
 */
ZINC_API int cmzn_spectrum_destroy(cmzn_spectrum_id *spectrum);

/**
 * Get if a spectrum is managed. See cmzn_spectrum_set_managed for
 * more information about managed.
 *
 *  @param spectrum  The spectrum to get the managed value from.
 *  @return 1 if spectrum is managed, otherwise false.
 */
ZINC_API bool cmzn_spectrum_is_managed(cmzn_spectrum_id spectrum);

/**
 * When the managed status is 0 (default) spectrum is destroyed when no longer
 * in use, i.e. when number of external references to it drops to
 * zero. Set to 1 to manage spectrum object indefinitely, or until this
 * attribute is reset to zero, effectively marking it as pending destruction.
 *
 * @param spectrum  Handle to the zinc spectrum.
 * @param value  The new value for the managed flag: true or false.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_spectrum_set_managed(cmzn_spectrum_id spectrum, bool value);

/**
 * Return an allocated string containing spectrum name.
 *
 * @param spectrum  handle to the zinc spectrum.
 * @return  allocated string containing spectrum name, otherwise NULL. Up to
 * caller to free using cmzn_deallocate().
 */
ZINC_API char *cmzn_spectrum_get_name(cmzn_spectrum_id spectrum);

/**
 * Set/change name for <spectrum>.
 *
 * @param spectrum  The handle to zinc graphical spectrum.
 * @param name  name to be set to the spectrum
 * @return  status CMZN_OK if successfully set/change name for spectrum,
 * any other value on failure.
 */
ZINC_API int cmzn_spectrum_set_name(cmzn_spectrum_id spectrum, const char *name);


/**
 * Get the overwrite material flag for spectrum. When overwrite flag is true,
 * the spectrum will clear any material rgba on primitive before applying
 * its own rgba. If the flag is false, rgba from the spectrum will be added
 * to the primitives on top of the one given by material.
 *
 * @param spectrum  Handle to a spectrum object.
 * @return  true if overwrite flag is set for spectrum, false if the flag
 * 	is not set or on failure.
 */
ZINC_API bool cmzn_spectrum_is_material_overwrite(cmzn_spectrum_id spectrum);

/**
 * Set the overwrite material flag for spectrum.
 *
 * @param spectrum  Handle to a spectrum object.
 *
 * @return  CMZN_OK if successfully set, any other value on failure.
 */
ZINC_API int cmzn_spectrum_set_material_overwrite(cmzn_spectrum_id spectrum, bool overwrite);

/***************************************************************************//**
 * Use this function with cmzn_spectrum_end_change.
 *
 * Use this function before making multiple changes on the spectrum, this
 * will stop spectrum from executing any immediate changes made.
 * After multiple changes have been made, use
 * cmzn_spectrum_end_change to execute all changes made previously in spectrum
 * at once.
 *
 * @param scene  The handle to the spectrum.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_spectrum_begin_change(cmzn_spectrum_id spectrum);

/***************************************************************************//**
 * Use this function with cmzn_spectrum_begin_change.
 *
 * Use this function before making multiple changes on the spectrum, this
 * will stop spectrum from executing any immediate changes made.
 * After multiple changes have been made, use
 * cmzn_spectrum_end_change to execute all changes made previously in spectrum
 * at once.
 *
 * @param spectrum  The handle to the spectrum.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_spectrum_end_change(cmzn_spectrum_id spectrum);

/**
 * Returns the number of components in spectrum.
 *
 * @param spectrum  The handle to the spectrum
 * @return  Returns the number of components in spectrum.
 */
ZINC_API int cmzn_spectrum_get_number_of_components(cmzn_spectrum_id spectrum);

/**
 * Create a component for spectrum. Used to colour graphics.
 *
 * @param spectrum  Handle to spectrum the spectrum_component is created in.
 * @return  Handle to the new spectrum_component on success, otherwise 0.
 */
ZINC_API cmzn_spectrum_component_id cmzn_spectrum_create_component(
	cmzn_spectrum_id spectrum);

/**
 * Get the first component on the spectrum list of <component>.

 * @param spectrum  Handle to a spectrum object.
 * @return  Handle to a spectrum_component object if successful,
 * otherwise NULL;
 */
ZINC_API cmzn_spectrum_component_id cmzn_spectrum_get_first_component(
	cmzn_spectrum_id spectrum);

/**
 * Get the next component after <ref_component> on the spectrum list of <spectrum>.

 * @param spectrum  Handle to a spectrum object.
 * @param ref_component  Handle to a spectrum_component object.
 * @return  Handle to a spectrum_component object if successful, otherwise NULL;
 */
ZINC_API cmzn_spectrum_component_id cmzn_spectrum_get_next_component(
	cmzn_spectrum_id spectrum, cmzn_spectrum_component_id ref_component);

/**
 * Get the component before <ref_component> on the components list of <spectrum>.

 * @param spectrum  Handle to a spectrum object.
 * @param ref_grpahic  Handle to a spectrum_component object.
 * @return  Handle to a spectrum_component object if successful, otherwise NULL;
 */
ZINC_API cmzn_spectrum_component_id cmzn_spectrum_get_previous_component(
	cmzn_spectrum_id spectrum, cmzn_spectrum_component_id ref_component);

/**
 * Move an existing component in spectrum before ref_component. Both <component> and
 * <ref_component> must be from the same spectrum.
 *
 * @param spectrum  The handle to the spectrum.
 * @param component  spectrum_component to be moved.
 * @param ref_component  <component> will be moved into the current position of this
 * 		spectrum_component
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_spectrum_move_component_before(
	cmzn_spectrum_id spectrum, cmzn_spectrum_component_id component,
	cmzn_spectrum_component_id ref_component);

/***************************************************************************//**
 * Removes <component> from <spectrum> and decrements the position
 * of all subsequent graphics.
 *
 * @param spectrum  The handle to the spectrum of which the component is removed
 *   from.
 * @param component  The handle to a spectrum component object which will be
 *   removed from the spectrum.
 * @return  Status CMZN_OK if successfully remove component from spectrum,
 * any other value on failure.
 */
ZINC_API int cmzn_spectrum_remove_component(cmzn_spectrum_id spectrum,
	cmzn_spectrum_component_id component);

/**
 * Removes all components from the spectrum.
 *
 * @param spectrum  The handle to the spectrum of which the component is removed
 *   from.
 * @return  Status CMZN_OK if successfully remove all components from spectrum,
 * any other value on failure.
 */
ZINC_API int cmzn_spectrum_remove_all_components(cmzn_spectrum_id spectrum);

/**
 * Returns a new reference to the spectrum_component with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param component  The spectrum_component to obtain a new reference to.
 * @return  New spectrum_component reference with incremented reference count.
 */
ZINC_API cmzn_spectrum_component_id cmzn_spectrum_component_access(
	cmzn_spectrum_component_id component);

/**
 * Destroys the spectrum_component and sets the pointer to NULL.
 *
 * @param component_address  The pointer to the handle of the spectrum_component.
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_spectrum_component_destroy(
	cmzn_spectrum_component_id *component_address);

/**
 * Get the minimum value of the range this spectrum component will
 * lookup to on targeted field. Primitives with this field value will
 * display the minimum colour value of the component colour type
 *
 * @param component  The handle of the spectrum_component.
 * @return  value of range minimum on success.
 */
ZINC_API double cmzn_spectrum_component_get_range_minimum(
	cmzn_spectrum_component_id component);

/**
 * Set the minimum value of the range this spectrum component will
 * lookup to on targeted field. Primitives with this field value will
 * display the minimum colour value of the component colour type
 *
 * @param component  The handle of the spectrum_component.
 * @value  the value to be set for range minimum
 *
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_spectrum_component_set_range_minimum(
	cmzn_spectrum_component_id component,	double value);

/**
 * Get the maximum value of the range this spectrum component will
 * lookup to on targeted field. Primitives with this field value will
 * display the maximum colour value of the component colour type
 *
 * @param component  The handle of the spectrum_component.
 * @return  value of range maximum on success.
 */
ZINC_API double cmzn_spectrum_component_get_range_maximum(
	cmzn_spectrum_component_id component);

/**
 * Set the maximum value of the range this spectrum component will
 * lookup to on targeted field. Primitives with this field value will
 * display the maximum colour value of the component colour type
 *
 * @param component  The handle of the spectrum_component.
 * @value  the value to be set for range maximum
 *
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_spectrum_component_set_range_maximum(
	cmzn_spectrum_component_id component,	double value);

/**
 * Get the normalised minimum value for the colour type of this spectrum
 * component. The range of colour displayed by this spectrum ranges from
 * minimum value of colour to the maximum value of colour
 *
 * @param component  The handle of the spectrum_component.
 * @return  minimum value of colour on success.
 */
ZINC_API double cmzn_spectrum_component_get_colour_minimum(
	cmzn_spectrum_component_id component);

/**
 * Set the normalised minimum value for the colour type of this spectrum
 * component. The range of colour displayed by this spectrum ranges from
 * minimum value of colour to the miximum value of colour
 *
 * @param component  The handle of the spectrum_component.
 * @value  the minimum colour value to be set
 *
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_spectrum_component_set_colour_minimum(
	cmzn_spectrum_component_id component, double value);

/**
 * Get the normalised maximum value for the colour type of this spectrum
 * component. The range of colour displayed by this spectrum ranges from
 * minimum value of colour to the maximum value of colour
 *
 * @param component  The handle of the spectrum_component.
 * @return  maximum value of colour on success.
 */
ZINC_API double cmzn_spectrum_component_get_colour_maximum(
	cmzn_spectrum_component_id component);

/**
 * Set the normalised maximum value for the colour type of this spectrum
 * component. The range of colour displayed by this spectrum ranges from
 * minimum value of colour to the miximum value of colour
 *
 * @param component  The handle of the spectrum_component.
 * @value  the maximum colour value to be set
 *
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_spectrum_component_set_colour_maximum(
	cmzn_spectrum_component_id component, double value);

/**
 * Get the step value of a spectrum component. The step spectrum
 * defines the boundary between the red and blue colour of the
 * COLOUR_MAPPING_STEP spectrum compomemt.
 *
 * @param component  The handle of the spectrum_component.
 *
 * @return  step value of the spectrum component on success.
 */
ZINC_API double cmzn_spectrum_component_get_step_value(
	cmzn_spectrum_component_id component);

/**
 * Set the step value of a spectrum component. The step spectrum
 * defines the boundary between the red and blue colour of the
 * COLOUR_MAPPING_STEP spectrum component.
 *
 * @param component  The handle of the spectrum_component.
 * @value  the step value to be set
 *
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_spectrum_component_set_step_value(
	cmzn_spectrum_component_id component,	double value);

/**
 * Get the value which alters the colour progression when scale type
 * is set to CMZN_SPECTRUM_COMPONENT_SCALE_LOG
 *
 * @param component  The handle of the spectrum_component.
 *
 * @return  exaggeration value of the spectrum component on success.
 */
ZINC_API double cmzn_spectrum_component_get_exaggeration(
	cmzn_spectrum_component_id component);

/**
 * Set the value which alters the colour progression when scale type
 * is set to CMZN_SPECTRUM_COMPONENT_SCALE_LOG
 *
 * @param component  The handle of the spectrum_component.
 * @value  the exaggeration value to be set
 *
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_spectrum_component_set_exaggeration(
	cmzn_spectrum_component_id component,	double value);

/**
 * Get the value determining the proportion of band present on each section, number of
 * sections in a spectrum is determined by number of bands, value must be larger
 * than 0.0 and must not exceed 1.0.
 *
 * @param component  The handle of the spectrum_component.
 *
 * @return  banded ratio of the spectrum component on success.
 */
ZINC_API double cmzn_spectrum_component_get_banded_ratio(
	cmzn_spectrum_component_id component);

/**
 * Set the value determining the proportion of band present on each section, number of
 * sections in a spectrum is determined by number of bands, value must be larger
 * than 0.0 and must not exceed 1.0.
 *
 * @param component  The handle of the spectrum_component.
 * @value  the banded ratio to be set
 *
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_spectrum_component_set_banded_ratio(
	cmzn_spectrum_component_id component,	double value);

/**
 * Get the active state of a spectrum component, only active spectrum component
 * will be rendered
 *
 * @param component  Handle to the zinc spectrum component.
 * @return  true if spectrum component is active, false if
 * failed or spectrum component is not active
 */
ZINC_API bool cmzn_spectrum_component_is_active(
	cmzn_spectrum_component_id component);

/**
 * Set the active state of a spectrum component, only active spectrum component
 * will be rendered
 *
 * @param component  Handle to the zinc spectrum component.
 * @param active  Value to be set to the zinc spectrum component.
 * @return  CMZN_OK if value is set successfully, any other value if
 * failed.
 */
ZINC_API int cmzn_spectrum_component_set_active(
	cmzn_spectrum_component_id component, bool active);

/**
 * Get the reverse flag of a spectrum component, reverse spectrum component will
 * have the colour rendered reversely
 *
 * @param component  Handle to the zinc spectrum component.
 * @return  true if spectrum component is reverse, false if
 * 	failed or spectrum component is not reverse
 */
ZINC_API bool cmzn_spectrum_component_is_colour_reverse(
	cmzn_spectrum_component_id component);

/**
 * Set the reverse flag of a spectrum component, reverse spectrum component will
 * have the colour rendered reversely
 *
 * @param component  Handle to the zinc spectrum component.
 * @param reverse  Value to be set to the zinc spectrum component.
 * @return  CMZN_OK if value is set successfully, any other value if
 * failed.
 */
ZINC_API int cmzn_spectrum_component_set_colour_reverse(
	cmzn_spectrum_component_id component, bool reverse);

/**
 * Get the extend_above flag of a spectrum component, an extend above spectrum component
 * will have the spectrum component colour rendered even when field value exceeds
 * spectrum maximum range.
 *
 * @param component  Handle to the zinc spectrum component.
 * @return  true if spectrum component extends above, false if
 * 	failed or spectrum component does not extend above
 */
ZINC_API bool cmzn_spectrum_component_is_extend_above(
	cmzn_spectrum_component_id component);

/**
 * Set the extend_above flag of a spectrum component, an extend_above
 * spectrum component will have the colour rendered even when field value exceeds
 * spectrum maximum range.
 *
 * @param component  Handle to the zinc spectrum component.
 * @param extend_above  Value to be set to the zinc spectrum component.
 * @return  CMZN_OK if value is set successfully, any other value if
 * failed.
 */
ZINC_API int cmzn_spectrum_component_set_extend_above(
	cmzn_spectrum_component_id component,	bool extend_above);

/**
 * Get the extend_below flag of a spectrum component, an extend below spectrum component
 * will have the spectrum component colour rendered even when field value is below
 * spectrum minimum range.
 *
 * @param component  Handle to the zinc spectrum component.
 * @return  true if spectrum component extends below, false if
 * 	failed or spectrum component does not extend below
 */
ZINC_API bool cmzn_spectrum_component_is_extend_below(
	cmzn_spectrum_component_id component);

/**
 * Set the extend_below flag of a spectrum component, an extend below spectrum component
 * will have the spectrum component colour rendered even when field value is below
 * spectrum minimum range.
 *
 * @param component  Handle to the zinc spectrum component.
 * @param extend_below  Value to be set to the zinc spectrum component.
 * @return  CMZN_OK if value is set successfully, any other value if
 * failed.
 */
ZINC_API int cmzn_spectrum_component_set_extend_below(
	cmzn_spectrum_component_id component,	bool extend_below);

/**
 * Get the field component lookup number of a spectrum component, this value
 * determines which of the field component this spectrum component will look up on.
 *
 * @See cmzn_graphic_set_data_field
 *
 * @param component  Handle to the zinc spectrum component.
 * @return  positive integer of the field component number to look up to.
 *   Any other value if failed or value is not set correctly.
 */
ZINC_API int cmzn_spectrum_component_get_field_component(
	cmzn_spectrum_component_id component);

/**
 * Set the field component lookup number of a spectrum component, this value
 * determines which of the field component this spectrum component will look up on.
 *
 * @See cmzn_graphic_set_data_field
 *
 * @param component  Handle to the zinc spectrum component.
 * @param component_number  field component number for this spectrum to lookup.
 *
 * @return  CMZN_OK if value is set successfully, any other value if
 * failed.
 */
ZINC_API int cmzn_spectrum_component_set_field_component(
	cmzn_spectrum_component_id component,	int component_number);

/**
 * Get the number of bands this component contains within its range in
 * CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_BANDED
 *
 * @param component  Handle to the zinc spectrum component.
 *
 * @return  positive integer of nuymber of bands set for this components.
 *   Any other value if failed or value is not set correctly
 */
ZINC_API int cmzn_spectrum_component_get_number_of_bands(cmzn_spectrum_component_id component);

/**
 * Set the number of bands this component contains within its range in
 * CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_BANDED
 *
 * @param component  Handle to the zinc spectrum component.
 * @param number of bands  number of bands for this component
 *
 * @return  CMZN_OK if value is set successfully, any other value if
 * failed.
 */
ZINC_API int cmzn_spectrum_component_set_number_of_bands(cmzn_spectrum_component_id component,
	int number_of_bands);

enum cmzn_spectrum_component_scale_type
{
	CMZN_SPECTRUM_COMPONENT_SCALE_INVALID = 0,
	CMZN_SPECTRUM_COMPONENT_SCALE_LINEAR = 1,
	/*!< The colour value on spectrum will be interpolated linearly in range when
	 * this mode is chosen.
	 */
	CMZN_SPECTRUM_COMPONENT_SCALE_LOG = 2
};

/**
 * Get the interpolation_mode of this component.
 *
 * @param component  Handle to the zinc spectrum component.
 *
 * @return  interpolation_mode set for this spectrum.
 *   CMZN_SPECTRUM_COMPONENT_SCALE_INVALID if failed or
 *   mode is not set correctly
 */
ZINC_API enum cmzn_spectrum_component_scale_type
	cmzn_spectrum_component_get_scale_type(cmzn_spectrum_component_id component);

/**
 * Set the interpolation_mode of this component.
 *
 * @param component  Handle to the zinc spectrum component.
 * @param interpolation_mode  Interpolation mode to be set for spectrum component
 *
 * @return  CMZN_OK if value is set successfully, any other value if
 * 	failed.
 */
ZINC_API int cmzn_spectrum_component_set_scale_type(
	cmzn_spectrum_component_id component,
	enum cmzn_spectrum_component_scale_type scale_type);

/**
 * Colour mapping mode for specctrum component. Appearances of these mappings
 * can ne alterd by the various APIs provided in spectrum and spectrum components
 * APIs.
 */
enum cmzn_spectrum_component_colour_mapping
{
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_INVALID = 0,
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_ALPHA = 1,
	/*!< This colour mapping alters the alpha (transparency value) for
	 * primitives.
	 * This mode does not alter the rgb value and
	 * should be used with other spectrum component or with
	 * overwrite_material set to 0 in spectrum.
	 */
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_BANDED = 2,
	/*!< This colour mapping create non-coloured strips/bands.
	 *  appearances can be altered by value of CMZN_SPECTRUM_COMPONENT_ATTRIBUTE_BANDED_RATIO
	 *  and value of number of bands.
	 *  This mode does not alter the rgb value except for the bands and
	 *  should be used with other spectrum component or with
	 *  overwrite_material set to 0 in spectrum.
	 */
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_BLUE = 3,
	/*!< This colour mapping create a colour spectrum from black to blue.
	 */
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_GREEN = 4,
	/*!< This colour mapping create a colour spectrum from black to green.
	 */
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_MONOCHROME = 5,
	/*!< This colour mapping create a monochrome (grey scale) spectrum.
	 */
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_RAINBOW = 6,
	/*!< This colour mapping create a spectrum from blue to red, similar
	 * to the colour of a rainbow.
	 */
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_RED = 7,
	/*!< This colour mapping create a colour spectrum from black to red.
	 */
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_STEP = 8,
	/*!< This colour mapping create a spectrum with only two colours, red and green.
	 * The boundary between red and green can be altered by
	 * CMZN_SPECTRUM_COMPONENT_ATTRIBUTE_STEP_VALUE.
	 */
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_BLUE = 9,
	/*!< This colour mapping create a colour spectrum from black to blue.
	 */
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_RED = 10,
	/*!< This colour mapping create a colour spectrum from black to red.
	 */
	CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_GREEN = 11
	/*!< This colour mapping create a colour spectrum from black to green.
	 */
};

/***************************************************************************//**
 * Convert a short attribute name into an enum if the attribute name matches
 * any of the members in the enum.
 *
 * @param attribute_name  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_spectrum_component_colour_mapping
	cmzn_spectrum_component_colour_mapping_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_spectrum_component_colour_mapping_enum_to_string(
	enum cmzn_spectrum_component_colour_mapping component_colour);

/**
 * Get the colour_mapping of this component.
 *
 * @param component  Handle to the zinc spectrum component.
 *
 * @return  colour_mapping of the spectrum component.
 *   CMZN_SPECTRUM_COMPONENT_COLOUR_MAPPING_INVALID if failed or
 *   mode is not set correctly.
 */
ZINC_API enum cmzn_spectrum_component_colour_mapping
cmzn_spectrum_component_get_colour_mapping(cmzn_spectrum_component_id component);

/**
 * Set the colour_mapping of this component.
 *
 * @param component  Handle to the zinc spectrum component.
 * @param colour_mapping  colour_mapping to be set for spectrum component
 *
 * @return  CMZN_OK if value is set successfully, any other value if
 * 	failed.
 */
ZINC_API int cmzn_spectrum_component_set_colour_mapping(
	cmzn_spectrum_component_id component,	enum cmzn_spectrum_component_colour_mapping type);


#ifdef __cplusplus
}
#endif

#endif
