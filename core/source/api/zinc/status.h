/**
 * FILE : cmiss_status.h
 *
 * The public interface to cmzn_status.
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
 * The Original Code is OpenCMISS-Zinc Library.
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

#ifndef CMZN_STATUS_H__
#define CMZN_STATUS_H__


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generic status codes returned by API functions to indicate success or error.
 *
 * WARNING: Planned future binary compatibility break.
 * At a future date we will change value of CMISS_OK to 0, and
 * introduce negative-valued error codes e.g. 'CMISS_ERROR_ARGUMENT',
 * to bring the CMGUI Zinc API in to line with common C API conventions.
 * To maintain your source compatibility through this break please ensure
 * all code checking integer status codes returned by functions compare
 * against enum CMISS_OK, NOT its current literal value
 *
 */
enum cmzn_status
{
	CMISS_ERROR_MEMORY = -2,
		/*!< failed to allocate memory. */
	CMISS_ERROR_ARGUMENT = -1,
		/*!< invalid argument(s) passed to API function. Only reported for new APIs. */
	CMISS_ERROR_GENERAL = 0,
		/*!< unspecified error occurred. Can include invalid argument(s) for old APIs. */
	CMISS_OK = 1
		/*!< value to be returned on success */
};

#ifdef __cplusplus
}
#endif

#endif
