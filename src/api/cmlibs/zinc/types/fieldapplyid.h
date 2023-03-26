/**
 * @file fieldapplyid.h
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDAPPLYID_H__
#define CMZN_FIELDAPPLYID_H__

/**
 * @brief Apply another field function with argument binding.
 *
 * A fields which evaluates another field, optionally from a different region,
 * with binding of Argument fields it depends on.
 * To evaluate the field, all argument fields it depends on must be bound to
 * source fields from this field's region.
 * Currently limited to applying and binding real-valued fields only.
 */
struct cmzn_field_apply;
typedef struct cmzn_field_apply *cmzn_field_apply_id;

/**
 * @brief Field providing a real argument for binding in an apply field.
 *
 * A fields which promises real values but must be connected to a source field
 * by binding in an apply field. Evaluation is delegated to the currently bound
 * source field in the field cache, otherwise it fails.
 */
struct cmzn_field_argument_real;
typedef struct cmzn_field_argument_real *cmzn_field_argument_real_id;

#endif /* CMZN_FIELDAPPLYID_H__ */
