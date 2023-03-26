/**
 * @file fieldderivativesid.h
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDDERIVATIVESID_H__
#define CMZN_FIELDDERIVATIVESID_H__

/**
 * @brief A field returning a derivative with respect to an element xi direction.
 *
 * A field returning a derivative of the source field with respect to a chosen
 * element xi coordinate index. The derivative can be evaluated on elements and
 * is with respect to the xi direction on the element being evaluated on, not
 * the top-level element. Hence can only evaluate xi index up to the element
 * dimension.
 */
struct cmzn_field_derivative;
typedef struct cmzn_field_derivative *cmzn_field_derivative_id;

#endif
