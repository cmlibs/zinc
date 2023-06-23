/**
 * @file fieldrangeid.h
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDRANGEID_H__
#define CMZN_FIELDRANGEID_H__

/**
 * @brief Stores range of field over a domain.
 *
 * Stores range of a field's values over a chosen domain, the locations at
 * which each component minimum or maximum occurs and the field values there.
 */
struct cmzn_fieldrange;
typedef struct cmzn_fieldrange *cmzn_fieldrange_id;

#endif
