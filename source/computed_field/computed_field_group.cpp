/*****************************************************************************//**
 * FILE : computed_field_group.cpp
 * 
 * Implements a cmiss field which is an group for another field, commonly from a
 * different region to make it available locally.
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
extern "C" {
#include <stdlib.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_group.h"
}
#include "computed_field/computed_field_group_base.hpp"
#include "computed_field/computed_field_sub_group_template.hpp"
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "api/cmiss_field_sub_group_template.h"
#include "api/cmiss_rendition.h"
#if defined (USE_OPENCASCADE)
#include "api/cmiss_field_cad.h"
#endif /* defined (USE_OPENCASCADE) */
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/rendition.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
}
#include <list>
#include <map>

/*
Module types
------------
*/

struct Cmiss_field_hierarchical_group_change_detail : public Cmiss_field_group_base_change_detail
{
public:
	Cmiss_field_hierarchical_group_change_detail() :
		local_change(CMISS_FIELD_GROUP_NO_CHANGE),
		non_local_change(CMISS_FIELD_GROUP_NO_CHANGE)
	{
	}

	void clear()
	{
		local_change = CMISS_FIELD_GROUP_NO_CHANGE;
		non_local_change = CMISS_FIELD_GROUP_NO_CHANGE;
	}

	/** note: if returns CLEAR or REMOVE, must check sub-group is empty */
	Cmiss_field_group_change_type getChange() const
	{
		return merge(local_change, non_local_change);
	}

	Cmiss_field_group_change_type getLocalChange() const
	{
		return local_change;
	}

	Cmiss_field_group_change_type getNonLocalChange() const
	{
		return non_local_change;
	}

	/** Inform group has been cleared, but wasn't before */
	void changeClear()
	{
		local_change = CMISS_FIELD_GROUP_CLEAR;
		non_local_change = CMISS_FIELD_GROUP_CLEAR;
	}

	/** Inform local group has been cleared, but wasn't before */
	void changeClearLocal()
	{
		local_change = CMISS_FIELD_GROUP_CLEAR;
	}

	/** Inform local group has been cleared, but wasn't before */
	void changeClearNonLocal()
	{
		non_local_change = CMISS_FIELD_GROUP_CLEAR;
	}

	void changeMergeLocal(const Cmiss_field_group_base_change_detail *in_change_detail)
	{
		local_change = merge(local_change, in_change_detail->getChange());
	}

	void changeMergeNonLocal(const Cmiss_field_group_base_change_detail *in_change_detail)
	{
		non_local_change = merge(non_local_change, in_change_detail->getChange());
	}

private:
	Cmiss_field_group_change_type local_change;
	Cmiss_field_group_change_type non_local_change;

	/** Warning: Assumes group is not empty. Caller must check empty -> clear */
	static Cmiss_field_group_change_type merge(
		Cmiss_field_group_change_type change1,
		Cmiss_field_group_change_type change2)
	{
		Cmiss_field_group_change_type result = change1;
		if ((change2 != CMISS_FIELD_GROUP_NO_CHANGE) && (change2 != change1))
		{
			if (change2 == CMISS_FIELD_GROUP_CLEAR)
			{
				if (change1 == CMISS_FIELD_GROUP_NO_CHANGE)
				{
					result = CMISS_FIELD_GROUP_REMOVE;
				}
				else if (change1 == CMISS_FIELD_GROUP_ADD)
				{
					result = CMISS_FIELD_GROUP_REPLACE;
				}
			}
			else if (change1 == CMISS_FIELD_GROUP_NO_CHANGE)
			{
				result = change2;
			}
			else
			{
				result = CMISS_FIELD_GROUP_REPLACE;
			}
		}
		return result;
	}
};

class Computed_field_group_package : public Computed_field_type_package
{
public:
	Cmiss_region *root_region;

	Computed_field_group_package(Cmiss_region *root_region)
	  : root_region(root_region)
	{
		ACCESS(Cmiss_region)(root_region);
	}
	
	~Computed_field_group_package()
	{
		DEACCESS(Cmiss_region)(&root_region);
	}
};

typedef std::map<Cmiss_region_id, Cmiss_field_id> Region_field_map;
typedef std::map<Cmiss_region_id, Cmiss_field_id>::iterator Region_field_map_iterator;
typedef std::map<Cmiss_region_id, Cmiss_field_id>::const_iterator Region_field_map_const_iterator;

namespace {

char computed_field_group_type_string[] = "group";

class Computed_field_group : public Computed_field_group_base
{
private:
	Cmiss_field_hierarchical_group_change_detail change_detail;

public:
	Cmiss_region *region;
	int selection_on;
	Computed_field *local_node_group, *local_data_group, *local_element_group;
	std::map<Computed_field *, Computed_field *> domain_selection_group;
#if defined (USE_OPENCASCADE)
	Computed_field *local_cad_primitive_group;
#endif /* defined (USE_OPENCASCADE) */
	Region_field_map sub_group_map;
	Region_field_map_iterator child_group_pos;

	Computed_field_group(Cmiss_region *region)
		: Computed_field_group_base()
		, region(region)
		, selection_on(0)
		, local_node_group(NULL)
		, local_data_group(NULL)
		, local_element_group(NULL)
#if defined (USE_OPENCASCADE)
		, local_cad_primitive_group(NULL)
#endif /* defined (USE_OPENCASCADE) */
		, sub_group_map()
	{		//ACCESS(Cmiss_region)(region);
		child_group_pos = sub_group_map.begin();
	}

	~Computed_field_group()
	{
		if (local_node_group)
		{
			Cmiss_field_destroy(&local_node_group);
		}
		if (local_data_group)
		{
			Cmiss_field_destroy(&local_data_group);
		}
		if (local_element_group)
		{
			Cmiss_field_destroy(&local_element_group);
		}
#if defined (USE_OPENCASCADE)
		if (local_cad_primitive_group)
		{
			Cmiss_field_destroy(&local_cad_primitive_group);
		}
#endif /* defined (USE_OPENCASCADE) */
		std::map<Computed_field *, Computed_field *>::iterator it = domain_selection_group.begin();
		for (;it != domain_selection_group.end(); it++)
		{
			//Cmiss_field_destroy(&(it->first)); don't destroy this it is not a reference just a key
			Cmiss_field_destroy(&(it->second));
		}
		/* the child selection is responsible for informing the parent selection that this group is now
		 * removed */
		Cmiss_region *parent_region = Cmiss_region_get_parent(region);
		if (parent_region)
		{
			Cmiss_rendition_id parent_rendition = Cmiss_region_get_rendition_internal(parent_region);
			if (parent_rendition)
			{
				if (Cmiss_rendition_has_selection_group(parent_rendition))
				{
					Cmiss_field *parent_group_field =
						Cmiss_rendition_get_or_create_selection_group(parent_rendition);
					Cmiss_field_group *parent_group = Cmiss_field_cast_group(parent_group_field);
					Cmiss_field_destroy(&parent_group_field);
					Computed_field_group *group_core = static_cast<Computed_field_group*>(
							reinterpret_cast<Computed_field*>(parent_group)->core);
					if (group_core != this)
						group_core->remove_child_group(region);
					parent_group_field = reinterpret_cast<Cmiss_field *>(parent_group);
					Cmiss_field_destroy(&parent_group_field);
				}
				Cmiss_rendition_destroy(&parent_rendition);
			}
			Cmiss_region_destroy(&parent_region);
		}
	}

	Cmiss_field_id create_node_group();

	Cmiss_field_id get_node_group();

	Cmiss_field_id create_data_group();

	Cmiss_field_id get_data_group();

	Cmiss_field_id create_element_group();

	Cmiss_field_id get_element_group();

	Cmiss_field_id get_subgroup_for_domain(Cmiss_field_id domain);

#if defined (USE_OPENCASCADE)
	Cmiss_field_id create_cad_primitive_group(Cmiss_field_cad_topology_id cad_topology_domain);

	Cmiss_field_id get_cad_primitive_group();

	int clear_region_tree_cad_primitive();
#endif /*defined (USE_OPENCASCADE) */

	Cmiss_field_id create_sub_group(Cmiss_region_id sub_region);

	int clear_region_tree_node(int use_data);

	int clear_region_tree_element();

	int add_region(struct Cmiss_region *child_region);

	int clear_group_if_empty();

	Cmiss_field_id getFirstChild();

	Cmiss_field_id getNextChild();

	virtual Cmiss_field_change_detail *extract_change_detail()
	{
		if (change_detail.getChange() == CMISS_FIELD_GROUP_NO_CHANGE)
			return NULL;
		Cmiss_field_hierarchical_group_change_detail *prior_change_detail =
			new Cmiss_field_hierarchical_group_change_detail();
		*prior_change_detail = change_detail;
#ifdef GRC
		{
			Cmiss_region *region = Computed_field_get_region(field);
			char *path = Cmiss_region_get_path(region);
			display_message(INFORMATION_MESSAGE, "Group %s%s change local %d non-local %d\n", path, field->name,
				prior_change_detail->getLocalChange(), prior_change_detail->getNonLocalChange());
			DEALLOCATE(path);
		}
#endif // GRC
		change_detail.clear();
		return prior_change_detail;
	}

	virtual int check_dependency();

	virtual void propagate_hierarchical_field_changes(MANAGER_MESSAGE(Computed_field) *message);

	virtual int isEmpty() const
	{
		return (isEmptyLocal() && isEmptyNonLocal());
	}

	virtual int clear();

private:

	Computed_field_core* copy()
	{
		Computed_field_group *core = new Computed_field_group(region);
		core->selection_on = this->selection_on;
		return (core);
	};

	const char* get_type_string()
	{
		return (computed_field_group_type_string);
	}

	void remove_child_group(struct Cmiss_region *child_region);

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int add_region_tree(struct Cmiss_region *region_tree);

	int remove_region(struct Cmiss_region *child_region);

	int remove_region_tree(struct Cmiss_region *child_region);

	int contain_region(struct Cmiss_region *child_region);

	int contain_region_tree(struct Cmiss_region *child_region);

	inline int isSubGroupEmpty(Computed_field_core *source_core) const
	{
		Computed_field_group_base *group_base = dynamic_cast<Computed_field_group_base *>(source_core);
		if (group_base)
		{
			return group_base->isEmpty();
		}
		display_message(ERROR_MESSAGE,
			"Computed_field_group::isSubGroupEmpty.  Subgroup not derived from Computed_field_group_base");
		return 0;
	}

	int isEmptyLocal() const;

	int isEmptyNonLocal() const;

	int check_sub_group_dependency(Computed_field_core *source_core);
};

/***************************************************************************//**
 * Compare the type specific data.
 */
int Computed_field_group::compare(Computed_field_core *other_core)
{
	int return_code;

	ENTER(Computed_field_group::compare);
	if (field && dynamic_cast<Computed_field_group*>(other_core))
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_group::compare */

/***************************************************************************//**
 * Evaluate the values of the field at the supplied location.
 */
int Computed_field_group::evaluate_cache_at_location(
	Field_location* location)
{
	int return_code = 1;

	ENTER(Computed_field_group::evaluate_cache_at_location);
	if (field && location)
	{
			field->values[0] = 0;
		if (selection_on)
		{
			field->values[0] = 1;
		}
		else if (dynamic_cast<Field_node_location*>(location))
		{
			Field_node_location *node_location = 
				reinterpret_cast<Field_node_location *>(location);
			Cmiss_node_id node = node_location->get_node();
			FE_region *fe_region = FE_node_get_FE_region(node);
			Cmiss_region_id node_region = NULL;
			FE_region_get_Cmiss_region(fe_region, &node_region);
			Cmiss_field_id temporary_handle = NULL;
			if (local_node_group)
			{
				Cmiss_field_node_group_template_id node_group_id = 
					Cmiss_field_cast_node_group_template(local_node_group);
				field->values[0] = Cmiss_field_node_group_template_is_node_selected(
					node_group_id, node);
				temporary_handle = reinterpret_cast<Computed_field *>(node_group_id);
				Cmiss_field_destroy(&temporary_handle);
			}
			if (!(field->values[0]) &&  local_data_group)
			{
				Cmiss_field_node_group_template_id data_group_id =
					Cmiss_field_cast_node_group_template(local_data_group);
				field->values[0] = Cmiss_field_node_group_template_is_node_selected(
					data_group_id, node);
				temporary_handle = reinterpret_cast<Computed_field *>(data_group_id);
				Cmiss_field_destroy(&temporary_handle);
			}
			if (!field->values[0] && node_region != region)
			{
				Region_field_map_iterator pos = sub_group_map.find(node_region);
				if (pos == sub_group_map.end())
				{
					field->values[0] = 0;
				}
			}
		}
 		else if (dynamic_cast<Field_element_xi_location*>(location))
 		{
 			Field_element_xi_location *element_xi_location =
 				reinterpret_cast<Field_element_xi_location *>(location);
 			Cmiss_element_id element = element_xi_location->get_element();
 			Cmiss_region_id element_region = Cmiss_element_get_region(element);
 			Cmiss_field_id temporary_handle = NULL;
 			if (element_region != region)
 			{
 				Region_field_map_iterator pos = sub_group_map.find(element_region);
 				if (pos == sub_group_map.end())
 				{
 					field->values[0] = 0;
 				}
 				else
 				{
 					Cmiss_field_id temporary_field_handle = pos->second;
 					Cmiss_field_group *temporary_group_field_handle =
 						Cmiss_field_cast_group(temporary_field_handle);
 					Computed_field_group *group_core = static_cast<Computed_field_group*>(
 						reinterpret_cast<Computed_field*>(temporary_group_field_handle)->core);
 					if (group_core)
 					{
 						temporary_handle = group_core->local_element_group;
 					}
 					Cmiss_field_destroy(&temporary_field_handle);
 				}
 			}
 			else
 			{
 				temporary_handle = local_element_group;
 			}
 			if (temporary_handle)
 			{
 				Cmiss_field_element_group_template_id element_group_id =
 					Cmiss_field_cast_element_group_template(temporary_handle);
 				field->values[0] = Cmiss_field_element_group_template_is_element_selected(
 					element_group_id, element);
 				Cmiss_field_destroy(&temporary_handle);
 			}
 		}
		else
		{
			field->values[0] = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_group::evaluate_cache_at_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_group::evaluate_cache_at_location */

Cmiss_field_id Computed_field_group::getFirstChild()
{
	Cmiss_field_id child_field= NULL;
	child_group_pos = sub_group_map.begin();
	if (child_group_pos !=sub_group_map.end())
	{
		child_field = Cmiss_field_access(child_group_pos->second);
	}
	return child_field;
}

Cmiss_field_id Computed_field_group::getNextChild()
{
	Cmiss_field_id child_field= NULL;
	if (child_group_pos !=sub_group_map.end())
	{
		child_group_pos++;
		if (child_group_pos !=sub_group_map.end())
		{
			child_field = Cmiss_field_access(child_group_pos->second);
		}
	}
	return child_field;
}

int Computed_field_group::isEmptyLocal() const
{
	if (selection_on)
	{
		return 0;
	}
	if (local_node_group && !isSubGroupEmpty(local_node_group->core))
		return 0;
	if (local_data_group && !isSubGroupEmpty(local_data_group->core))
		return 0;
	if (local_element_group && !isSubGroupEmpty(local_element_group->core))
		return 0;
	std::map<Computed_field *, Computed_field *>::const_iterator it = domain_selection_group.begin();
	while (it != domain_selection_group.end())
	{
		Computed_field *sub_group_field = it->second;
		if (!isSubGroupEmpty(sub_group_field->core))
			return 0;
		++it;
	}
	return 1;
}

int Computed_field_group::isEmptyNonLocal() const
{
	for (Region_field_map_const_iterator iter = sub_group_map.begin();
		iter != sub_group_map.end(); iter++)
	{
		Cmiss_field_id sub_group_field = iter->second;
		if (!isSubGroupEmpty(sub_group_field->core))
			return 0;
	}
	return 1;
}

int Computed_field_group::clear()
{
	// GRC not implemented yet
	selection_on = 0;
	return 0;
};

int Computed_field_group::check_sub_group_dependency(Computed_field_core *source_core)
{
	if (source_core->check_dependency())
	{
		Computed_field_dependency_change_private(field);
		const Cmiss_field_sub_group_change_detail *sub_group_change_detail =
			dynamic_cast<const Cmiss_field_sub_group_change_detail *>(source_core->get_change_detail());
		if (sub_group_change_detail)
		{
			if ((sub_group_change_detail->getChange() == CMISS_FIELD_GROUP_CLEAR) &&
				isEmptyLocal())
			{
				change_detail.changeClearLocal();
			}
			else
			{
				change_detail.changeMergeLocal(sub_group_change_detail);
			}
		}
	}
	return 1;
}

int Computed_field_group::check_dependency()
{
	if (field)
	{
		if (local_node_group)
			check_sub_group_dependency(local_node_group->core);
		if (local_data_group)
			check_sub_group_dependency(local_data_group->core);
		if (local_element_group)
			check_sub_group_dependency(local_element_group->core);
		std::map<Computed_field *, Computed_field *>::const_iterator it = domain_selection_group.begin();
		while (it != domain_selection_group.end())
		{
			Computed_field *sub_group_field = it->second;
			check_sub_group_dependency(sub_group_field->core);
			++it;
		}
		return (field->manager_change_status & MANAGER_CHANGE_RESULT(Computed_field));
	}
	return 0;
}

void Computed_field_group::propagate_hierarchical_field_changes(
	MANAGER_MESSAGE(Computed_field) *message)
{
	if (message)
	{
		for (Region_field_map_iterator iter = sub_group_map.begin();
			iter != sub_group_map.end(); iter++)
		{
			Cmiss_field_id sub_group = iter->second;
			// future optimisation: check subfield is from changed region
			const Cmiss_field_change_detail *source_change_detail = NULL;
			int change = Computed_field_manager_message_get_object_change_and_detail(message, sub_group, &source_change_detail);
			if (change != MANAGER_CHANGE_NONE(Computed_field))
			{
				if (source_change_detail)
				{
					const Cmiss_field_group_base_change_detail *sub_group_change_detail =
						dynamic_cast<const Cmiss_field_group_base_change_detail *>(source_change_detail);
					if (sub_group_change_detail)
					{
						Cmiss_field_group_change_type sub_group_change = sub_group_change_detail->getChange();
						if (sub_group_change != CMISS_FIELD_GROUP_NO_CHANGE)
						{
							if (((sub_group_change == CMISS_FIELD_GROUP_CLEAR) ||
								(sub_group_change == CMISS_FIELD_GROUP_REMOVE)) &&
								isEmptyNonLocal())
							{
								change_detail.changeClearNonLocal();
							}
							else
							{
								change_detail.changeMergeNonLocal(sub_group_change_detail);
							}
							Computed_field_dependency_changed(field);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE, "Sub-group changes could not be propagated.");
					}
				}
				// we have found only possible subgroup for sub-region:
				break;
			}
		}
	}
}

/***************************************************************************//**
 * Writes type-specific details of the field to the console. 
 */
int Computed_field_group::list()
{
	int return_code;
	
	ENTER(List_Computed_field_group);
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    Region : ");
		if (region)
		{
			char *path = Cmiss_region_get_path(region);
			display_message(INFORMATION_MESSAGE, "%s", path);
			DEALLOCATE(path);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_group */

/***************************************************************************//**
 * Returns allocated command string for reproducing this field. Includes type.
 */
char *Computed_field_group::get_command_string()
{
	char *command_string;
	int error;

	ENTER(Computed_field_group::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, computed_field_group_type_string, &error);
		append_string(&command_string, " region ", &error);
		if (region)
		{
			char *path = Cmiss_region_get_path(region);
			append_string(&command_string, path, &error);
			DEALLOCATE(path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_group::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_group::get_command_string */

Cmiss_field_id Computed_field_group::create_sub_group(Cmiss_region_id sub_region)
{
	Cmiss_field_id sub_group = NULL;
	if (Cmiss_region_contains_subregion(region, sub_region) && region != sub_region)
	{
		Region_field_map_iterator pos;

		pos = sub_group_map.find(sub_region);
		if (pos == sub_group_map.end())
		{
			Cmiss_field_module_id field_module =
				Cmiss_region_get_field_module(sub_region);
			sub_group = Cmiss_field_module_create_group(field_module, sub_region);
			Cmiss_field_set_name(sub_group, "cmiss_selection");
			Cmiss_field_module_destroy(&field_module);
			sub_group_map.insert(std::make_pair(sub_region,sub_group));
		}
		else
		{
			sub_group = Cmiss_field_access(pos->second);
		}
	}

	return sub_group;
}

Cmiss_field_id Computed_field_group::create_node_group()
{
	if (!local_node_group)
	{
		Cmiss_field_module_id field_module =
			Cmiss_region_get_field_module(region);
		local_node_group = get_node_group();
		if (!local_node_group)
		{
			char *temp_string = Cmiss_field_get_name(this->getField());
			int error = 0;
			append_string(&temp_string, ".nodes", &error);
			local_node_group = Cmiss_field_module_create_node_group_template(field_module);
			Cmiss_field_set_name(local_node_group, temp_string);
			Cmiss_field_access(local_node_group);
			DEALLOCATE(temp_string);
		}
		Cmiss_field_module_destroy(&field_module);
	}
	else
	{
		Cmiss_field_access(local_node_group);
	}

	return (local_node_group);
}

Cmiss_field_id Computed_field_group::get_node_group()
{
	if (local_node_group)
	{
		Cmiss_field_access(local_node_group);
	}
	else
	{
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
		char *temp_string = Cmiss_field_get_name(this->getField());
		int error = 0;
		append_string(&temp_string, ".nodes", &error);
		local_node_group = Cmiss_field_module_find_field_by_name(
			field_module, temp_string);
		DEALLOCATE(temp_string);
		if (local_node_group)
			Cmiss_field_access(local_node_group);
		Cmiss_field_module_destroy(&field_module);
	}

	return (local_node_group);
}

Cmiss_field_id Computed_field_group::create_data_group()
{
	if (!local_data_group)
	{
		Cmiss_field_module_id field_module =
			Cmiss_region_get_field_module(region);
		local_data_group = get_data_group();
		if (!local_data_group)
		{
			char *temp_string = Cmiss_field_get_name(this->getField());
			int error = 0;
			append_string(&temp_string, ".data", &error);
			local_data_group = Cmiss_field_module_create_node_group_template(field_module);
			Cmiss_field_set_name(local_data_group, temp_string);
			Cmiss_field_access(local_data_group);
			DEALLOCATE(temp_string);
		}
		Cmiss_field_module_destroy(&field_module);
	}
	else
	{
		Cmiss_field_access(local_data_group);
	}

	return (local_data_group);
}

Cmiss_field_id Computed_field_group::get_data_group()
{
	if (local_data_group)
	{
		Cmiss_field_access(local_data_group);
	}
	else
	{
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
		char *temp_string = Cmiss_field_get_name(this->getField());
		int error = 0;
		append_string(&temp_string, ".data", &error);
		local_data_group = Cmiss_field_module_find_field_by_name(
			field_module, temp_string);
		DEALLOCATE(temp_string);
		if (local_data_group)
			Cmiss_field_access(local_data_group);
		Cmiss_field_module_destroy(&field_module);
	}

	return (local_data_group);
}

Cmiss_field_id Computed_field_group::create_element_group()
{
	if (!local_element_group)
	{
		Cmiss_field_module_id field_module =
			Cmiss_region_get_field_module(region);
		local_element_group = get_element_group();
		if (!local_element_group)
		{
			char *temp_string = Cmiss_field_get_name(this->getField());
			int error = 0;
			append_string(&temp_string, ".elements", &error);
			local_element_group = Cmiss_field_module_create_element_group_template(field_module);
			Cmiss_field_set_name(local_element_group, temp_string);
			Cmiss_field_access(local_element_group);
			DEALLOCATE(temp_string);
		}
		Cmiss_field_module_destroy(&field_module);
	}
	else
	{
		Cmiss_field_access(local_element_group);
	}

	return (local_element_group);
}

Cmiss_field_id Computed_field_group::get_element_group()
{
	if (local_element_group)
	{
		Cmiss_field_access(local_element_group);
	}
	else
	{
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
		char *temp_string = Cmiss_field_get_name(this->getField());
		int error = 0;
		append_string(&temp_string, ".elements", &error);
		local_element_group = Cmiss_field_module_find_field_by_name(
			field_module, temp_string);
		DEALLOCATE(temp_string);
		if (local_element_group)
			Cmiss_field_access(local_element_group);
		Cmiss_field_module_destroy(&field_module);
	}
	return (local_element_group);
}

Cmiss_field_id Computed_field_group::get_subgroup_for_domain(Cmiss_field_id domain)
{
	Computed_field *field = NULL;
	std::map<Computed_field *, Computed_field *>::const_iterator it;
	it = domain_selection_group.find(domain);
	if (it != domain_selection_group.end())
	{
		field = it->second;
		Cmiss_field_access(field);
	}

	return field;
}

#if defined (USE_OPENCASCADE)
Cmiss_field_id Computed_field_group::create_cad_primitive_group(Cmiss_field_cad_topology_id cad_topology_domain)
{
	Computed_field *field = NULL;
	//field = get_subgroup_for_domain(cad_topology_domain)
	if (cad_topology_domain)
	{
		const char *base_name = "cad_primitive_selection";
		const char *domain_field_name = Cmiss_field_get_name(reinterpret_cast<Cmiss_field_id>(cad_topology_domain));
		char *field_name = NULL;
		int error = 0;
		//printf( "string lengths: %d, %d\n", strlen(base_name), strlen(domain_field_name));
		ALLOCATE(field_name, char, strlen(base_name)+strlen(domain_field_name)+2);
		field_name[0] = '\0';
		append_string(&field_name, base_name, &error);
		append_string(&field_name, "_", &error);
		append_string(&field_name, domain_field_name, &error);

		Cmiss_field_module_id field_module =
			Cmiss_region_get_field_module(region);
		field = Cmiss_field_module_create_cad_primitive_group_template(field_module);
		Cmiss_field_set_name(field, field_name);
		Computed_field *cad_topology_key = reinterpret_cast<Cmiss_field_id>(cad_topology_domain);
		domain_selection_group.insert(std::pair<Computed_field *, Computed_field *>(cad_topology_key, field));

		Cmiss_field_module_destroy(&field_module);
		Cmiss_field_access(field);
		DEALLOCATE(field_name);
	}
	else
	{
		display_message(ERROR_MESSAGE, "Computed_field_group::create_cad_primitive_group.  Invalid arguments\n");
	}

	return (field);
}

Cmiss_field_id Computed_field_group::get_cad_primitive_group()
{
	if (local_cad_primitive_group)
	{
		Cmiss_field_access(local_cad_primitive_group);
	}

	return (local_cad_primitive_group);
}

int Computed_field_group::clear_region_tree_cad_primitive()
{
	Region_field_map_iterator pos;
	int return_code = 1;
	Cmiss_field_group_id group_field = NULL;
	std::map<Computed_field *, Computed_field *>::iterator it = domain_selection_group.begin();
	for (;it != domain_selection_group.end();)
	{
		Cmiss_field_cad_primitive_group_template_id cad_primitive_group =
			Cmiss_field_cast_cad_primitive_group_template(it->second);
		return_code = Cmiss_field_cad_primitive_group_template_clear(cad_primitive_group);
		Computed_field_changed(this->field);
		Cmiss_field_id cad_primitive_group_field = reinterpret_cast<Computed_field*>(cad_primitive_group);
		Cmiss_field_destroy(&cad_primitive_group_field);
		Cmiss_field_destroy(&it->second);
		domain_selection_group.erase(it++);
	}
	//if (local_cad_primitive_group)
	//{
	//	Cmiss_field_cad_primitive_group_template_id cad_primitive_group =
	//		Cmiss_field_cast_cad_primitive_group_template(local_cad_primitive_group);
	//	return_code = Cmiss_field_cad_primitive_group_template_clear(cad_primitive_group);
	//	Computed_field_changed(this->field);
	//	Cmiss_field_id cad_primitive_group_field = reinterpret_cast<Computed_field*>(cad_primitive_group);
	//	Cmiss_field_destroy(&cad_primitive_group_field);
	//	Cmiss_field_destroy(&local_cad_primitive_group);
	//}
	if (!sub_group_map.empty())
	{
		for (pos = sub_group_map.begin(); pos != sub_group_map.end();)
		{
			group_field = Cmiss_field_cast_group(pos->second);
			Computed_field_group *temp_group = static_cast<Computed_field_group*>(
				reinterpret_cast<Computed_field*>(group_field)->core);
			Cmiss_field_id temporary_handle = pos->second;
			if (temp_group != this)
			{
				pos++;
				Cmiss_field_group_clear_region_tree_cad_primitive(group_field);
				Cmiss_field_group_clear_group_if_empty(group_field);
			}
			else
			{
				if (!local_node_group && selection_on == 0)
				{
					if (pos->first != region)
					{
						Cmiss_rendition_id rendition = Cmiss_rendition_get_from_region(pos->first);
						if (rendition)
						{
							Cmiss_rendition_remove_selection_group(rendition);
							Cmiss_rendition_destroy(&rendition);
						}
					}
					else
					{
						Cmiss_field_id temporary_handle2 = pos->second;
						Cmiss_field_destroy(&temporary_handle2);
					}
					sub_group_map.erase(pos++);
				}
				else
				{
					++pos;
				}
			}
			Cmiss_field_destroy(&temporary_handle);
		}
	}
	if (sub_group_map.empty())
		clear_group_if_empty();

	return (return_code);
}

#endif /* defined (USE_OPENCASCADE) */


int Computed_field_group::add_region(struct Cmiss_region *child_region)
{
	int return_code = 0;
	if (Cmiss_region_contains_subregion(region, child_region))
	{
		Cmiss_rendition *child_rendition = Cmiss_region_get_rendition_internal(
			child_region);
		if (child_rendition)
		{
			Computed_field *child_group_field = 
				Cmiss_rendition_get_or_create_selection_group(child_rendition);
			if (child_group_field)
			{
				sub_group_map.insert(std::make_pair(child_region,child_group_field));
				Cmiss_field_destroy(&child_group_field);
				return_code = 1;
			}
		}
		DEACCESS(Cmiss_rendition)(&child_rendition);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_group::add_region.  Sub region is not a child region"
			"or part of the parent region");
		return_code = 0;
	}

	return (return_code);
}

void Computed_field_group::remove_child_group(struct Cmiss_region *child_region)
{
	if (Cmiss_region_contains_subregion(region, child_region))
	{
		sub_group_map.erase(child_region);
	}
	this->clear_group_if_empty();
}

int Computed_field_group::clear_group_if_empty()
{
	int return_code = 0;
	if (local_node_group)
	{
		Cmiss_field_node_group_template_id node_group_id =
			Cmiss_field_cast_node_group_template(local_node_group);
		Cmiss_node_id node = Cmiss_field_node_group_template_get_first_node(node_group_id);
		if (node)
		{
			Cmiss_node_destroy(&node);
		}
		else
		{
			Cmiss_field_destroy(&local_node_group);
		}
		Cmiss_field_id node_group_field = reinterpret_cast<Computed_field*>(node_group_id);
		Cmiss_field_destroy(&node_group_field);
	}
	if (local_data_group)
	{
		Cmiss_field_node_group_template_id data_group_id =
			Cmiss_field_cast_node_group_template(local_data_group);
		Cmiss_node_id node = Cmiss_field_node_group_template_get_first_node(data_group_id);
		if (node)
		{
			Cmiss_node_destroy(&node);
		}
		else
		{
			Cmiss_field_destroy(&local_data_group);
		}
		Cmiss_field_id data_group_field = reinterpret_cast<Computed_field*>(data_group_id);
		Cmiss_field_destroy(&data_group_field);
	}
	if (local_element_group)
	{
		Cmiss_field_element_group_template_id element_group_id =
			Cmiss_field_cast_element_group_template(local_element_group);
		Cmiss_element_id element = Cmiss_field_element_group_template_get_first_element(element_group_id);
		if (element)
		{
			Cmiss_element_destroy(&element);
		}
		else
		{
			Cmiss_field_destroy(&local_element_group);
		}
		Cmiss_field_id element_group_field = reinterpret_cast<Computed_field*>(element_group_id);
		Cmiss_field_destroy(&element_group_field);
	}

	if (!local_node_group && !local_element_group && !local_data_group
#if defined (USE_OPENCASCADE)
		&& !local_cad_primitive_group
#endif /* defined (USE_OPENCASCADE) */
		&& selection_on == 0 &&
		sub_group_map.empty())
	{
		Cmiss_rendition_id rendition = Cmiss_region_get_rendition_internal(region);
		if (rendition)
		{
			Cmiss_rendition_remove_selection_group(rendition);
			Cmiss_rendition_destroy(&rendition);
		}
		return_code = 1;
	}
	return return_code;
}

int Computed_field_group::clear_region_tree_node(int use_data)
{
	Region_field_map_iterator pos;
	int return_code = 1;
	Cmiss_field_group_id group_field = NULL;
	if (!use_data && local_node_group)
	{
		Cmiss_field_node_group_template_id node_group =
			Cmiss_field_cast_node_group_template(local_node_group);
		return_code = Cmiss_field_node_group_template_clear(node_group);
		Computed_field_changed(this->field);
		Cmiss_field_id node_group_field = reinterpret_cast<Computed_field*>(node_group);
		Cmiss_field_destroy(&node_group_field);
		Cmiss_field_destroy(&local_node_group);
	}
	if (use_data && local_data_group)
	{
		Cmiss_field_node_group_template_id data_group =
			Cmiss_field_cast_node_group_template(local_data_group);
		return_code = Cmiss_field_node_group_template_clear(data_group);
		Computed_field_changed(this->field);
		Cmiss_field_id data_group_field = reinterpret_cast<Computed_field*>(data_group);
		Cmiss_field_destroy(&data_group_field);
		Cmiss_field_destroy(&local_data_group);
	}
	if (!sub_group_map.empty())
	{
		for (pos = sub_group_map.begin(); pos != sub_group_map.end();)
		{
			group_field = Cmiss_field_cast_group(pos->second);
			Computed_field_group *temp_group = static_cast<Computed_field_group*>(
				reinterpret_cast<Computed_field*>(group_field)->core);
			Cmiss_field_id temporary_handle = pos->second;
			if (temp_group != this) /* if it is a true region */
			{
				pos++;
				if (!use_data)
					Cmiss_field_group_clear_region_tree_node(group_field);
				else
					Cmiss_field_group_clear_region_tree_data(group_field);
				Cmiss_field_group_clear_group_if_empty(group_field);
			}
			else
			{
				if (!local_element_group && selection_on == 0)
				{
					if (pos->first != region)
					{
						Cmiss_rendition_id rendition = Cmiss_region_get_rendition_internal(pos->first);
						if (rendition)
						{
							Cmiss_rendition_remove_selection_group(rendition);
							Cmiss_rendition_destroy(&rendition);
						}
					}
					else
					{
						Cmiss_field_id temporary_handle2 = pos->second;
						Cmiss_field_destroy(&temporary_handle2);
					}
					sub_group_map.erase(pos++);
				}
				else
				{
					++pos;
				}
			}
			Cmiss_field_destroy(&temporary_handle);
		}
	}
	if (sub_group_map.empty())
		clear_group_if_empty();

	return (return_code);
}

int Computed_field_group::clear_region_tree_element()
{
  Region_field_map_iterator pos;
  int return_code = 1;
  Cmiss_field_group_id group_field = NULL;
	if (local_element_group)
	{
		Cmiss_field_element_group_template_id element_group =
			Cmiss_field_cast_element_group_template(local_element_group);
		return_code = Cmiss_field_element_group_template_clear(element_group);
		Computed_field_changed(this->field);
		Cmiss_field_id element_group_field = reinterpret_cast<Computed_field*>(element_group);
		Cmiss_field_destroy(&element_group_field);
		Cmiss_field_destroy(&local_element_group);
	}
	if (!sub_group_map.empty())
	{
		for (pos = sub_group_map.begin(); pos != sub_group_map.end();)
		{
			group_field = Cmiss_field_cast_group(pos->second);
			Computed_field_group *temp_group = static_cast<Computed_field_group*>(
				reinterpret_cast<Computed_field*>(group_field)->core);
			Cmiss_field_id temporary_handle = pos->second;
			if (temp_group != this)
			{
				pos++;
				Cmiss_field_group_clear_region_tree_element(group_field);
				Cmiss_field_group_clear_group_if_empty(group_field);
			}
			else
			{
				if (!local_node_group && selection_on == 0)
				{
					if (pos->first != region)
					{
						Cmiss_rendition_id rendition = Cmiss_region_get_rendition_internal(pos->first);
						if (rendition)
						{
							Cmiss_rendition_remove_selection_group(rendition);
							Cmiss_rendition_destroy(&rendition);
						}
					}
					else
					{
						Cmiss_field_id temporary_handle2 = pos->second;
						Cmiss_field_destroy(&temporary_handle2);
					}
					sub_group_map.erase(pos++);
				}
				else
				{
					++pos;
				}
			}
			Cmiss_field_destroy(&temporary_handle);
		}
	}
	if (sub_group_map.empty())
		clear_group_if_empty();

	return (return_code);
}

} //namespace

Cmiss_field_group *Cmiss_field_cast_group(Cmiss_field_id field)
{
	if (dynamic_cast<Computed_field_group*>(field->core))
	{
		Cmiss_field_access(field);
		return (reinterpret_cast<Cmiss_field_group_id>(field));
	}
	else
	{
		return (NULL);
	}
}

inline Computed_field *Computed_field_cast(
	Cmiss_field_group *group_field)
{
	return (reinterpret_cast<Computed_field*>(group_field));
}

inline Computed_field_group *Computed_field_group_core_cast(
	Cmiss_field_group *group_field)
{
	return (static_cast<Computed_field_group*>(
		reinterpret_cast<Computed_field*>(group_field)->core));
}

int Cmiss_field_group_add_region(
	Cmiss_field_group_id group, Cmiss_region_id sub_region)
{
	int return_code = 0;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return_code = group_core->add_region(sub_region);
		}
	}
	return return_code;
}

Cmiss_field_id Cmiss_field_group_create_node_group(Cmiss_field_group_id group)
{
	Cmiss_field_id field = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->create_node_group();
		}
	}

	return field;
}

Cmiss_field_id Cmiss_field_group_get_node_group(Cmiss_field_group_id group)
{
	Cmiss_field_id field = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_node_group();
		}
	}

	return field;
}

Cmiss_field_id Cmiss_field_group_create_data_group(Cmiss_field_group_id group)
{
	Cmiss_field_id field = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->create_data_group();
		}
	}

	return field;
}

Cmiss_field_id Cmiss_field_group_get_data_group(Cmiss_field_group_id group)
{
	Cmiss_field_id field = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_data_group();
		}
	}

	return field;
}

Cmiss_field_id Cmiss_field_group_create_element_group(Cmiss_field_group_id group)
{
	Cmiss_field_id field = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->create_element_group();
		}
	}

	return field;
}

Cmiss_field_id Cmiss_field_group_get_element_group(Cmiss_field_group_id group)
{
	Cmiss_field_id field = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_element_group();
		}
	}

	return field;
}

#if defined (USE_OPENCASCADE)
Cmiss_field_id Cmiss_field_group_create_cad_primitive_group(Cmiss_field_group_id group, Cmiss_field_cad_topology_id cad_topology_domain)
{
	Cmiss_field_id field = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->create_cad_primitive_group(cad_topology_domain);
		}
	}

	return field;
}

Cmiss_field_id Cmiss_field_group_get_cad_primitive_group(Cmiss_field_group_id group, Cmiss_field_cad_topology_id cad_topology_domain)
{
	Cmiss_field_id field = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_cad_primitive_group();
		}
	}

	return field;
}

int Cmiss_field_group_clear_region_tree_cad_primitive(Cmiss_field_group_id group)
{
	int return_code = 0;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return_code = group_core->clear_region_tree_cad_primitive();
		}
	}
	return return_code;
}


#endif /* defined (USE_OPENCASCADE) */


Cmiss_field_id Cmiss_field_group_create_sub_group(
	Cmiss_field_group_id group, Cmiss_region_id sub_region)
{
	Cmiss_field_id field = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->create_sub_group(sub_region);
		}
	}
	return field;
}

int Cmiss_field_group_clear_region_tree_node(Cmiss_field_group_id group)
{
	int return_code = 0;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return_code = group_core->clear_region_tree_node(/*use_data*/0);
		}
	}
	return return_code;
}

int Cmiss_field_group_clear_region_tree_data(Cmiss_field_group_id group)
{
	int return_code = 0;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return_code = group_core->clear_region_tree_node(/*use_data*/1);
		}
	}
	return return_code;
}

int Cmiss_field_group_clear_group_if_empty(Cmiss_field_group_id group)
{
	int return_code = 0;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return_code = group_core->clear_group_if_empty();
		}
	}
	return return_code;
}

int Cmiss_field_group_clear_region_tree_element(Cmiss_field_group_id group)
{
	int return_code = 0;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return_code = group_core->clear_region_tree_element();
		}
	}
	return return_code;
}

Cmiss_field_id Cmiss_field_group_get_subgroup_for_domain(Cmiss_field_group_id group, Cmiss_field_id domain)
{
	Computed_field *field = NULL;

	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_subgroup_for_domain(domain);
		}
	}

	return field;
}

int Cmiss_field_group_for_each_child(Cmiss_field_group_id group,
		int (*function)(Cmiss_field_id child_field), int recursive)
{
	int return_code = 0;
	Cmiss_field_id group_field = Computed_field_cast(group);
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			Cmiss_field_id child_field = group_core->getFirstChild();
			while (child_field)
			{
				if (child_field != group_field)
				{
					function(child_field);
					if (recursive)
					{
						Cmiss_field_group_id child_group = Cmiss_field_cast_group(child_field);
						Cmiss_field_destroy(&child_field);
						Cmiss_field_group_for_each_child(child_group, function, recursive);
						child_field = Computed_field_cast(child_group);
					}
				}
				Cmiss_field_destroy(&child_field);
				child_field = group_core->getNextChild();
			}
		}
		return_code = 1;
	}
	return return_code;
}

Computed_field *Cmiss_field_module_create_group(Cmiss_field_module_id field_module,
	Cmiss_region *region)
{
	Computed_field *field;

	ENTER(Computed_field_create_group);
	field = (Computed_field *)NULL;
	// GRC original_field->manager check will soon be unnecessary
	if (field_module && region)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_group(region));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_module_create_group.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Cmiss_field_module_create_group */

/*****************************************************************************//**
 * Command modifier function for defining a field as type computed_field_group
 * (if not already) and allowing its contents to be modified.
 *
 * @param state  The parse state containing the command tokens.
 * @param field_modify_void  Void pointer to Computed_field_modify_data containing
 *   the field and the region it will be added to.
 * @param computed_field_group_package_void  Void pointer to
 *   Computed_field_group_package.
 * @return  1 on success, 0 on failure.
 */
int define_Computed_field_type_group(Parse_state *state,
	void *field_modify_void, void *computed_field_group_package_void)
{
	int return_code;
	Computed_field_group_package *computed_field_group_package;
	Computed_field_modify_data *field_modify;

	ENTER(define_Computed_field_type_group);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void) &&
		(computed_field_group_package =
			(Computed_field_group_package *)computed_field_group_package_void))
	{
		return_code = 1;
		char *region_path = NULL;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Creates a field which hold information of grouping of the region,"
			"node point and data points.");
		Option_table_add_string_entry(option_table,"region",
			&region_path, " REGION_PATH");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		/* no errors,not asking for help */
		if (return_code)
		{
			Cmiss_region *root_region = NULL;
			Cmiss_region *region = NULL;
 			
			if (region_path)
			{
				if (region_path[0] == CMISS_REGION_PATH_SEPARATOR_CHAR)
				{
					// absolute path
					root_region = computed_field_group_package->root_region;
				}
				else
				{
					// relative path
					root_region = field_modify->get_region();
				}
				region = Cmiss_region_find_subregion_at_path(root_region,
						region_path);
				if (region)
				{
						display_message(ERROR_MESSAGE,
							"gfx define field group:  Could not find region %s", region_path);
						display_parse_state_location(state);
						return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx define field group:  Must specify region");
				display_parse_state_location(state);
				return_code = 0;
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_group(field_modify->get_field_module(),
						region));
				Cmiss_region_destroy(&region);
			}
		}
		if (!return_code)
		{
			if ((!state->current_token) ||
				(strcmp(PARSER_HELP_STRING, state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
			{
				display_message(ERROR_MESSAGE, "gfx define field group:  Failed");
			}
		}
		DEALLOCATE(region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_group */

int Computed_field_register_type_group(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region)
{
	int return_code;
	Computed_field_group_package *computed_field_group_package = 
		new Computed_field_group_package(root_region);

	ENTER(Computed_field_register_type_group);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_group_type_string,
			define_Computed_field_type_group,
			computed_field_group_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_group */

