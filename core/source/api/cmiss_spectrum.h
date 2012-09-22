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

#include "types/cmiss_spectrum_id.h"

#include "cmiss_shared_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Labels of spectrum attributes which may be set or obtained using generic
 * get/set_attribute functions.
 */
enum Cmiss_spectrum_attribute
{
	CMISS_SPECTRUM_ATTRIBUTE_INVALID = 0,
	CMISS_SPECTRUM_ATTRIBUTE_IS_MANAGED = 1,
	/*!< Boolean as integer, when 0 (default) spectrum is destroyed when no
	 * longer in use, i.e. when number of external references to it drops to
	 * zero. Set to 1 to manage spectrum object indefinitely, or until this
	 * attribute is reset to zero, effectively marking it as pending destruction.
	 */
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_spectrum_attribute Cmiss_spectrum_attribute_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_spectrum_attribute_enum_to_string(
	enum Cmiss_spectrum_attribute attribute);

/***************************************************************************//**
 * Access the spectrum, increase the access count of the time keeper by one.
 *
 * @param spectrum  handle to the "to be access" cmiss spectrum.
 * @return  handle to spectrum if successfully access spectrum.
 */
ZINC_API Cmiss_spectrum_id Cmiss_spectrum_access(Cmiss_spectrum_id spectrum);

/***************************************************************************//**
 * Destroy the spectrum.
 *
 * @param spectrum  address to the handle to the "to be destroyed"
 *   cmiss spectrum.
 * @return  status CMISS_OK if successfully destroy spectrum, any other value
 * on failure.
 */
ZINC_API int Cmiss_spectrum_destroy(Cmiss_spectrum_id *spectrum);

/***************************************************************************//**
 * Get an integer or Boolean attribute of the graphics spectrum.
 *
 * @param spectrum  Handle to the cmiss spectrum.
 * @param attribute  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
ZINC_API int Cmiss_spectrum_get_attribute_integer(Cmiss_spectrum_id spectrum,
	enum Cmiss_spectrum_attribute attribute);

/***************************************************************************//**
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

/***************************************************************************//**
 * Return an allocated string containing spectrum name.
 *
 * @param spectrum  handle to the cmiss spectrum.
 * @return  allocated string containing spectrum name, otherwise NULL. Up to
 * caller to free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_spectrum_get_name(Cmiss_spectrum_id spectrum);

/***************************************************************************//**
 * Set/change name for <spectrum>.
 *
 * @param spectrum  The handle to cmiss grpahical spectrum.
 * @param name  name to be set to the spectrum
 * @return  status CMISS_OK if successfully set/change name for spectrum,
 * any other value on failure.
 */
ZINC_API int Cmiss_spectrum_set_name(Cmiss_spectrum_id spectrum, const char *name);

/***************************************************************************//**
 * Execute cmgui command as in standalone cmgui application however this execute
 * command function will apply to the spectrum being passed into this function
 * only. It takes a string of command as gfx modify spectrum <spectrum> does.
 *
 * NOTE: This function may be removed in the future once more API functions are
 * made available to the users.
 *
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @param command  Command to be executed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_spectrum_execute_command(Cmiss_spectrum_id spectrum,
	const char *command_string);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_SPECTRUM_H__ */
