/***************************************************************************//**
 * FILE : cmiss_field_ensemble_id.h
 *
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005-2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#if !defined (CMISS_FIELD_ENSEMBLE_ID_H)
#define CMISS_FIELD_ENSEMBLE_ID_H

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
