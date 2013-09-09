/*******************************************************************************
FILE : fieldtime.h

LAST MODIFIED : 16 Nov 2011

DESCRIPTION :
Implements zinc fields that is controlled by time.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDTIME_H__
#define CMZN_FIELDTIME_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/timekeeperid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************************//**
 * Creates a field whose value equals the source_field evaluated at the time
 * given by time_field, overriding any time prescribed for field evaluation.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field to evaluate.
 * @param time_field  Field providing time value to evaluate at.
 * @return  Handle to a new time lookup field on success, NULL on failure.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_time_lookup(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_field_id time_field);

/***************************************************************************//**
 * Creates a field which returns the current time from the supplied time keeper.
 *
 * @param field_module  Region field module which will own new field.
 * @param time_keeper  cmzn_time_keeper object.
 * @return  Handle to a new time value field on success, NULL on failure.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_time_value(
	cmzn_field_module_id field_module, cmzn_time_keeper_id time_keeper);

#ifdef __cplusplus
}
#endif

#endif
