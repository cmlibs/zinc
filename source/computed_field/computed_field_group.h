/***************************************************************************//**
 *FILE : computed_field_group.h
 *
 * Implements a "group" computed_field which group regions, 
 * node and data point component.
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

#if !defined (COMPUTED_FIELD_GROUP_H)
#define COMPUTED_FIELD_GROUP_H

#include "api/cmiss_field_module.h"
#include "api/cmiss_field_group.h"

/*****************************************************************************//**
 * Sets up command data for group field.
 * 
 * @param computed_field_package  Container for command data.
 * @param root_region  Root region for getting paths to fields in other regions.
 * @return 1 on success, 0 on failure.
 */
int Computed_field_register_type_group(
	struct Computed_field_package *computed_field_package);

struct Computed_field *Cmiss_field_module_create_group(Cmiss_field_module_id field_module,
	Cmiss_region *region);

/*****************************************************************************//**
 * A helper function to conveniently calls the same function for each child in
 * selection group.
 *
 * @param group  group field.
 * @param *function  pointer to the function to be called by each child.
 * @param recursive  if 1 the function will be called by all children down the tree.
 * @return 1 on success, 0 on failure.
 */
int Cmiss_field_group_for_each_child(Cmiss_field_group_id group,
		int (*function)(Cmiss_field_id child_field), int recursive);


#endif /* !defined (COMPUTED_FIELD_GROUP_H) */
