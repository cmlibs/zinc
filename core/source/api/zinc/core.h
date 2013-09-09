/**
 * @file core.h
 */
/*******************************************************************************
FILE : core.h

LAST MODIFIED : 12 August 2003

DESCRIPTION :
The public interface to the some of the internal functions of cmiss.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_CORE_H__
#define CMZN_CORE_H__

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

ZINC_API void *cmzn_allocate(int bytes);
/*******************************************************************************
LAST MODIFIED : 12 August 2003

DESCRIPTION :
Frees the memory associated with the pointer.  Used to clean up when functions
return buffers allocated internally to cmiss.
==============================================================================*/

ZINC_API int cmzn_deallocate(void *ptr);
/*******************************************************************************
LAST MODIFIED : 12 August 2003

DESCRIPTION :
Frees the memory associated with the pointer.  Used to clean up when functions
return buffers allocated internally to cmiss.
==============================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* CMZN_CORE_H__ */
