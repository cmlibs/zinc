/**
 * FILE : status.hpp
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

enum Status
{
	ERROR_MEMORY = CMZN_ERROR_MEMORY,
		/*!< failed to allocate memory. */
	ERROR_ARGUMENT = CMZN_ERROR_ARGUMENT,
		/*!< invalid argument(s) passed to API function. Only reported for new APIs. */
	ERROR_GENERAL = CMZN_ERROR_GENERAL,
		/*!< unspecified error occurred. Can include invalid argument(s) for old APIs. */
	OK = CMZN_OK
		/*!< value to be returned on success */
};

} // namespace Zinc
}

#endif
