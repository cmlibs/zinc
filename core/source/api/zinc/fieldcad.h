/***************************************************************************//**
 * FILE : fieldcad.h
 * 
 * The public interface to zinc fields which wrap cad entities.
 * Warning: prototype!
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
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
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

#ifndef CMZN_FIELDCAD_H__
#define CMZN_FIELDCAD_H__

#include "types/field_id.h"
#include "types/field_module_id.h"
#include "types/region_id.h"
#include "types/field_cad_id.h"
#include "types/field_group_id.h"

int cmzn_region_import_cad_file(cmzn_region_id region, const char *file_name);

struct cmzn_cad_identifier;
typedef struct cmzn_cad_identifier *cmzn_cad_identifier_id;

struct cmzn_field_cad_primitive_group_template;
typedef struct cmzn_field_cad_primitive_group_template *cmzn_field_cad_primitive_group_template_id;

cmzn_field_id cmzn_field_module_create_cad_primitive_group_template(cmzn_field_module_id field_module);

int cmzn_field_cad_primitive_group_template_destroy(cmzn_field_cad_primitive_group_template_id *cad_primitive_group_address);

cmzn_field_cad_primitive_group_template_id cmzn_field_cast_cad_primitive_group_template(cmzn_field_id field);

int cmzn_field_cad_primitive_group_template_add_cad_primitive(cmzn_field_cad_primitive_group_template_id cad_primitive_group,
	cmzn_cad_identifier_id cad_primitive);

int cmzn_field_cad_primitive_group_template_remove_cad_primitive(cmzn_field_cad_primitive_group_template_id cad_primitive_group,
	cmzn_cad_identifier_id cad_primitive);

int cmzn_field_cad_primitive_group_template_clear(cmzn_field_cad_primitive_group_template_id cad_primitive_group);

int cmzn_field_cad_primitive_group_template_is_cad_primitive_selected(
	cmzn_field_cad_primitive_group_template_id cad_primitive_group, cmzn_cad_identifier_id cad_primitive);

cmzn_field_cad_primitive_group_template_id cmzn_field_group_create_cad_primitive_group(cmzn_field_group_id group, cmzn_field_cad_topology_id cad_topology_domain);

cmzn_field_cad_primitive_group_template_id cmzn_field_group_get_cad_primitive_group(cmzn_field_group_id group, cmzn_field_cad_topology_id cad_topology_domain);

int cmzn_field_group_clear_region_tree_cad_primitive(cmzn_field_group_id group);

cmzn_cad_identifier_id cmzn_field_cad_primitive_group_template_get_first_cad_primitive(
	cmzn_field_cad_primitive_group_template_id cad_primitive_group);

cmzn_cad_identifier_id cmzn_field_cad_primitive_group_template_get_next_cad_primitive(
	cmzn_field_cad_primitive_group_template_id cad_primitive_group);


#endif /* #ifndef CMZN_FIELDCAD_H */

