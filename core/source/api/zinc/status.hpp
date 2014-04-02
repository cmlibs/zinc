/**
 * @file status.hpp
 *
 * C++ function status/error codes.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_STATUS_HPP
#define CMZN_STATUS_HPP

#include "zinc/status.h"

namespace OpenCMISS
{
namespace Zinc
{

/**
 * Generic status codes returned by API functions to indicate success or error.
 *
 * WARNING: Planned future binary compatibility break.
 * At a future date we will change the value of OK to 0, and
 * ERROR_GENERAL to some other value, to bring the Zinc API in to line with
 * common API conventions. To maintain your source compatibility through this
 * break please ensure all code checking integer status codes returned by
 * functions compare against enumerated symbol names, not their current values.
 */
enum Status
{
	ERROR_MEMORY = CMZN_ERROR_MEMORY,
		/*!< Failed to allocate memory. */
	ERROR_ARGUMENT = CMZN_ERROR_ARGUMENT,
		/*!< Invalid argument(s) passed to API function. */
	ERROR_GENERAL = CMZN_ERROR_GENERAL,
		/*!< Unspecified error occurred. Can include invalid argument(s) for old APIs. */
	OK = CMZN_OK
		/*!< Successful return */
};

} // namespace Zinc
}

#endif
