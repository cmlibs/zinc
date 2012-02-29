/*******************************************************************************
FILE : computed_field_lookup.h

LAST MODIFIED : 10 October 2003

DESCRIPTION :
Implements computed fields for lookups.
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#if !defined (COMPUTED_FIELD_LOOKUP_H)
#define COMPUTED_FIELD_LOOKUP_H

#include "region/cmiss_region.h"

int Computed_field_register_types_lookup(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region);
/*******************************************************************************
LAST MODIFIED : 01 October 2003

DESCRIPTION :
==============================================================================*/

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
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field, struct FE_node *lookup_node); 

/***************************************************************************//**
 * If the field is of type COMPUTED_FIELD_LOOKUP, the function returns the
 * source field and the  lookup node where it is evaluated.
 * Note that nothing returned has been ACCESSed.
 */
int Computed_field_get_type_nodal_lookup(struct Computed_field *field,
  struct Computed_field **source_field, struct FE_node **lookup_node);

#endif /* !defined (COMPUTED_FIELD_LOOKUP_H) */
