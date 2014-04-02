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
 * At a future date we will change the value of CMZN_OK to 0, and
 * CMZN_ERROR_GENERAL to some other value, to bring the Zinc API in to line with
 * common C API conventions. To maintain your source compatibility through this
 * break please ensure all code checking integer status codes returned by
 * functions compare against enumerated symbol names, not their current values.
 */
enum cmzn_status
{
	CMZN_ERROR_MEMORY = -2,
		/*!< Failed to allocate memory. */
	CMZN_ERROR_ARGUMENT = -1,
		/*!< Invalid argument(s) passed to API function. */
	CMZN_ERROR_GENERAL = 0,
		/*!< Unspecified error occurred. Can include invalid argument(s) for old APIs. */
	CMZN_OK = 1
		/*!< Successful return */
};

#ifdef __cplusplus
}
#endif

#endif
