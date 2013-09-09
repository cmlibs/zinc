/*******************************************************************************
FILE : computed_field_lookup.h

LAST MODIFIED : 10 October 2003

DESCRIPTION :
Implements computed fields for lookups.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_LOOKUP_H)
#define COMPUTED_FIELD_LOOKUP_H

#include "region/cmiss_region.h"

/*****************************************************************************//**
 * Creates a field whose value equals source field calculated at the lookup node
 * instead of the domain location requested.
 * Do not put in external API; should instead make 'embedded' field take a
 * any location-supplying field: mesh, node etc.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field to evaluate.
 * @param lookup_node  Node from same region as source field to evaluate value at.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_nodal_lookup(
	struct cmzn_field_module *field_module,
	struct Computed_field *source_field, struct FE_node *lookup_node);

/***************************************************************************//**
 * If the field is of type COMPUTED_FIELD_LOOKUP, the function returns the
 * source field and the  lookup node where it is evaluated.
 * Note that nothing returned has been ACCESSed.
 */
int Computed_field_get_type_nodal_lookup(struct Computed_field *field,
  struct Computed_field **source_field, struct FE_node **lookup_node);

struct Computed_field *Computed_field_create_quaternion_SLERP(
	cmzn_field_module_id field_module, cmzn_field_id source_field,
	cmzn_node_id quaternion_SLERP_node);

#endif /* !defined (COMPUTED_FIELD_LOOKUP_H) */
