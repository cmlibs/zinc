/**
 * @file status.h
 *
 * The public interface to cmzn_status.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_STATUS_H__
#define CMZN_STATUS_H__


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generic status codes returned by API functions to indicate success or error.
 *
 * WARNING: Planned future binary compatibility break.
 * At a future date we will change value of CMZN_OK to 0, and
 * introduce negative-valued error codes e.g. 'CMZN_ERROR_ARGUMENT',
 * to bring the CMGUI Zinc API in to line with common C API conventions.
 * To maintain your source compatibility through this break please ensure
 * all code checking integer status codes returned by functions compare
 * against enum CMZN_OK, NOT its current literal value
 *
 */
enum cmzn_status
{
	CMZN_ERROR_MEMORY = -2,
		/*!< failed to allocate memory. */
	CMZN_ERROR_ARGUMENT = -1,
		/*!< invalid argument(s) passed to API function. Only reported for new APIs. */
	CMZN_ERROR_GENERAL = 0,
		/*!< unspecified error occurred. Can include invalid argument(s) for old APIs. */
	CMZN_OK = 1
		/*!< value to be returned on success */
};

#ifdef __cplusplus
}
#endif

#endif
