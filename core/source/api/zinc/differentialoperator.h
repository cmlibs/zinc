/***************************************************************************//**
 * FILE : differentialoperator.h
 *
 * Public interface to differential operator objects used to specify which
 * field derivative to evaluate.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_DIFFERENTIALOPERATOR_H__
#define CMZN_DIFFERENTIALOPERATOR_H__

#include "types/differentialoperatorid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Returns a new reference to the differential operator with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param differential_operator  The differential operator to obtain a new
 * reference to.
 * @return  New differential operator reference with incremented reference
 * count.
 */
ZINC_API cmzn_differentialoperator_id cmzn_differentialoperator_access(
	cmzn_differentialoperator_id differential_operator);

/***************************************************************************//**
 * Destroys reference to the differential operator and sets pointer/handle to
 * NULL. Internally this just decrements the reference count.
 *
 * @param differential_operator_address  Address of differential operator
 * reference.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_differentialoperator_destroy(
	cmzn_differentialoperator_id *differential_operator_address);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_DIFFERENTIALOPERATOR_H__ */
