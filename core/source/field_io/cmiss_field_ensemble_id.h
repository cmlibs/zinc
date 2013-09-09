/***************************************************************************//**
 * FILE : cmiss_field_ensemble_id.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_FIELD_ENSEMBLE_ID_H)
#define CMZN_FIELD_ENSEMBLE_ID_H

/***************************************************************************//**
 * The ensemble type specific handle to an ensemble cmzn_field.
 * This is a domain comprising a set of entries with unique identifiers.
 */
struct cmzn_field_ensemble;
typedef struct cmzn_field_ensemble *cmzn_field_ensemble_id;

/***************************************************************************//**
 * The ensemble type specific handle to a ensemble group cmzn_field.
 * This field stores a subset of entries from an ensemble.
 * When evaluated, returns 1 for entries in the group, 0 otherwise.
 */
struct cmzn_field_ensemble_group;
typedef struct cmzn_field_ensemble_group *cmzn_field_ensemble_group_id;

/***************************************************************************//**
 * The numeric type of the unique identifier for each entry in an ensemble.
 */
typedef int cmzn_ensemble_identifier;

/***************************************************************************//**
 * Iterator-handle for referencing an entry in a cmzn_field_ensemble.
 */
struct cmzn_ensemble_iterator;
typedef struct cmzn_ensemble_iterator *cmzn_ensemble_iterator_id;

/***************************************************************************//**
 * Index to a selection of entries from N ensembles.
 * Used for getting/setting parameters.
 */
struct cmzn_ensemble_index;
typedef struct cmzn_ensemble_index *cmzn_ensemble_index_id;

#endif
