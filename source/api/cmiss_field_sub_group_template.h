/***************************************************************************//**
 * FILE : cmiss_field_sub_group.h
 * 
 * Implements a cmiss field which maintains a group or selection of objects
 * from the region including the region itself, other fields representing domain
 * object groups (e.g. node, element), and related groups from child regions.
 * The field evaluates to 1 (true) at domain locations in the group, and 0
 * elsewhere.  
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
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Shane Blackett (shane at blackett.co.nz)
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

#if !defined (CMISS_FIELD_SUB_GROUP_TEMPLATE_H)
#define CMISS_FIELD_SUB_GROUP_TEMPLATE_H

#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"

struct Cmiss_field_node_group_template;
typedef struct Cmiss_field_node_group_template *Cmiss_field_node_group_template_id;

Cmiss_field_id Cmiss_field_module_create_node_group_template(Cmiss_field_module_id field_module);

Cmiss_field_node_group_template_id Cmiss_field_cast_node_group_template(Cmiss_field_id field);

int Cmiss_field_node_group_template_add_node(Cmiss_field_node_group_template_id node_group,
	Cmiss_node_id node);

int Cmiss_field_node_group_template_clear(Cmiss_field_node_group_template_id node_group);

int Cmiss_field_node_group_template_is_node_selected(
	Cmiss_field_node_group_template_id node_group, Cmiss_node_id node);

Cmiss_node_id Cmiss_field_node_group_template_get_first_node(
	Cmiss_field_node_group_template_id node_group);

Cmiss_node_id Cmiss_field_node_group_template_get_next_node(
	Cmiss_field_node_group_template_id node_group);


struct Cmiss_field_element_group_template;
typedef struct Cmiss_field_element_group_template *Cmiss_field_element_group_template_id;

Cmiss_field_id Cmiss_field_module_create_element_group_template(Cmiss_field_module_id field_module);

Cmiss_field_element_group_template_id Cmiss_field_cast_element_group_template(Cmiss_field_id field);

int Cmiss_field_element_group_template_add_element(Cmiss_field_element_group_template_id element_group,
	Cmiss_element_id element);

int Cmiss_field_element_group_template_clear(Cmiss_field_element_group_template_id element_group);

int Cmiss_field_element_group_template_is_element_selected(
	Cmiss_field_element_group_template_id element_group, Cmiss_element_id element);


#endif /* !defined (CMISS_FIELD_GROUP_TEMPLATE_H) */
