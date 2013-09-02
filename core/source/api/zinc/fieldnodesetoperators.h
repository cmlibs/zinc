/***************************************************************************//**
 * FILE : fieldnodesetoperators.h
 *
 * Implements field operators that sum or process fields over a nodeset.
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#ifndef CMZN_FIELDNODESETOPERATORS_H__
#define CMZN_FIELDNODESETOPERATORS_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/nodeid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a field which computes the sum of each source field component over
 * all nodes in the nodeset for which it is defined. Returned field has same
 * number of components as the source field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field to sum.
 * @param nodeset  The set of nodes to sum field over.
 * @return  Handle to newly created field.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_nodeset_sum(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset);

/**
 * Creates a field which computes the mean of each source field component over
 * all nodes in the nodeset for which it is defined. Returned field has same
 * number of components as the source field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field to obtain mean component values for.
 * @param nodeset  The set of nodes to obtain mean over.
 * @return  Handle to newly created field.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_nodeset_mean(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset);

/**
 * Creates a field which computes the sum of the squares of each source field
 * component over all nodes in the nodeset for which it is defined. Returned
 * field has same number of components as the source field.
 * This field type supports least-squares optimisation by giving individual
 * terms being squared and summed.
 * @see cmzn_optimisation_add_objective_field
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field to sum squared component values of.
 * @param nodeset  The set of nodes to sum field over.
 * @return  Handle to newly created field.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_nodeset_sum_squares(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset);

/**
 * Creates a field which computes the mean of the squares of each source field
 * component over all nodes in the nodeset for which it is defined. Returned
 * field has same number of components as the source field.
 * This field type supports least-squares optimisation by giving individual
 * terms being squared and summed, each divided by the square root of the number
 * of terms.
 * @see cmzn_optimisation_add_objective_field
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field to obtain mean squared component values for.
 * @param nodeset  The set of nodes to obtain mean over.
 * @return  Handle to newly created field.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_nodeset_mean_squares(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset);

/**
 * Creates a field which computes the minimum of each source field
 * component over all nodes in the nodeset for which it is defined. Returned
 * field has same number of components as the source field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field to obtain minimum values for.
 * @param nodeset  The set of nodes to obtain minimum over.
 * @return  Handle to newly created field.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_nodeset_minimum(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset);

/**
 * Creates a field which computes the maximum of each source field
 * component over all nodes in the nodeset for which it is defined. Returned
 * field has same number of components as the source field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field to obtain maximum values for.
 * @param nodeset  The set of nodes to obtain maximum over.
 * @return  Handle to newly created field.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_nodeset_maximum(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_nodeset_id nodeset);

#ifdef __cplusplus
}
#endif

#endif
