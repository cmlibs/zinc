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

#ifndef __CMISS_SPECTRUM_H__
#define __CMISS_SPECTRUM_H__

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
ZINC_API Cmiss_spectrum_module_id Cmiss_spectrum_module_access(
	Cmiss_spectrum_module_id spectrum_module);

/**
* Destroys this reference to the spectrum module (and sets it to NULL).
* Internally this just decrements the reference count.
*
* @param spectrum_module_address  Address of handle to spectrum module
*   to destroy.
* @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_spectrum_module_destroy(
	Cmiss_spectrum_module_id *spectrum_module_address);

/**
 * Create and return a handle to a new spectrum.
 *
 * @param spectrum_module  The handle to the spectrum module the
 * spectrum will belong to.
 * @return  Handle to the newly created spectrum if successful, otherwise NULL.
 */
ZINC_API Cmiss_spectrum_id Cmiss_spectrum_module_create_spectrum(
	Cmiss_spectrum_module_id spectrum_module);

/**
* Begin caching or increment cache level for this spectrum module. Call this
* function before making multiple changes to minimise number of change messages
* sent to clients. Must remember to end_change after completing changes.
* @see Cmiss_spectrum_module_end_change
*
* @param spectrum_module  The spectrum_module to begin change cache on.
* @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_spectrum_module_begin_change(Cmiss_spectrum_module_id spectrum_module);

/***************************************************************************//**
* Decrement cache level or end caching of changes for the spectrum module.
* Call Cmiss_spectrum_module_begin_change before making multiple changes
* and call this afterwards. When change level is restored to zero,
* cached change messages are sent out to clients.
*
* @param spectrum_module  The glyph_module to end change cache on.
* @return  Status CMISS_OK on success, any other value on failure.
*/
ZINC_API int Cmiss_spectrum_module_end_change(Cmiss_spectrum_module_id spectrum_module);

/**
* Find the spectrum with the specified name, if any.
*
* @param spectrum_module  spectrum module to search.
* @param name  The name of the spectrum.
* @return  Handle to the spectrum of that name, or 0 if not found.
* 	Up to caller to destroy returned handle.
*/
ZINC_API Cmiss_spectrum_id Cmiss_spectrum_module_find_spectrum_by_name(
	Cmiss_spectrum_module_id spectrum_module, const char *name);

/**
* Get the default spectrum, if any.
*
* @param spectrum_module  spectrum module to query.
* @return  Handle to the default spectrum, or 0 if none.
* 	Up to caller to destroy returned handle.
*/
ZINC_API Cmiss_spectrum_id Cmiss_spectrum_module_get_default_spectrum(
	Cmiss_spectrum_module_id spectrum_module);

/**
* Set the default spectrum.
*
* @param spectrum_module  spectrum module to modify
* @param spectrum  The spectrum to set as default.
* @return  CMISS_OK on success otherwise CMISS_ERROR_ARGUMENT.
*/
ZINC_API int Cmiss_spectrum_module_set_default_spectrum(
	Cmiss_spectrum_module_id spectrum_module,
	Cmiss_spectrum_id spectrum);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_spectrum_attribute Cmiss_spectrum_attribute_enum_from_string(
	const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_spectrum_attribute_enum_to_string(
	enum Cmiss_spectrum_attribute attribute);

/**
 * Access the spectrum, increase the access count of the time keeper by one.
 *
 * @param spectrum  handle to the "to be access" cmiss spectrum.
 * @return  handle to spectrum if successfully access spectrum.
 */
ZINC_API Cmiss_spectrum_id Cmiss_spectrum_access(Cmiss_spectrum_id spectrum);

/**
 * Destroy the spectrum.
 *
 * @param spectrum  address to the handle to the "to be destroyed"
 *   cmiss spectrum.
 * @return  status CMISS_OK if successfully destroy spectrum, any other value
 * on failure.
 */
ZINC_API int Cmiss_spectrum_destroy(Cmiss_spectrum_id *spectrum);

/**
 * Get an integer or Boolean attribute of the graphics spectrum.
 *
 * @param spectrum  Handle to the cmiss spectrum.
 * @param attribute  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
ZINC_API int Cmiss_spectrum_get_attribute_integer(Cmiss_spectrum_id spectrum,
	enum Cmiss_spectrum_attribute attribute);

/**
 * Set an integer or Boolean attribute of the graphics spectrum.
 *
 * @param spectrum  Handle to the cmiss spectrum.
 * @param attribute  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true in case more options are added in future.
 * @return  status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid or able to be set for this spectrum object.
 */
ZINC_API int Cmiss_spectrum_set_attribute_integer(Cmiss_spectrum_id spectrum,
	enum Cmiss_spectrum_attribute attribute, int value);

/**
 * Return an allocated string containing spectrum name.
 *
 * @param spectrum  handle to the cmiss spectrum.
 * @return  allocated string containing spectrum name, otherwise NULL. Up to
 * caller to free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_spectrum_get_name(Cmiss_spectrum_id spectrum);

/**
 * Set/change name for <spectrum>.
 *
 * @param spectrum  The handle to cmiss grpahical spectrum.
 * @param name  name to be set to the spectrum
 * @return  status CMISS_OK if successfully set/change name for spectrum,
 * any other value on failure.
 */
ZINC_API int Cmiss_spectrum_set_name(Cmiss_spectrum_id spectrum, const char *name);

/**
 * Set spectrum maximum and minimum.
 *
 * @deprecated
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @param minimum  Minimum value of the spectrum.
 * @param maximum  Maximum value of the spectrum.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_spectrum_set_minimum_and_maximum(Cmiss_spectrum_id spectrum, double minimum, double maximum);

/**
 * Set the spectrum to use the rainbow.
 *
 * @deprecated
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_spectrum_set_rainbow(Cmiss_spectrum_id spectrum);

/**
 * Get the minimum value from the given spectrum.
 *
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @return  the minimum value, 0.0 on failure.
 */
ZINC_API double Cmiss_spectrum_get_minimum(Cmiss_spectrum_id spectrum);

/**
 * Get the maximum value from the given spectrum.
 *
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @return  the maximum value, 0.0 on failure.
 */
ZINC_API double Cmiss_spectrum_get_maximum(Cmiss_spectrum_id spectrum);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_SPECTRUM_H__ */
