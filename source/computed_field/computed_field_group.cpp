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
#include "computed_field/computed_field_subobject_group.hpp"
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "api/cmiss_field_subobject_group.h"
#include "api/cmiss_rendition.h"
#if defined (USE_OPENCASCADE)
#include "graphics/rendition.h"
#include "api/cmiss_field_cad.h"
#endif /* defined (USE_OPENCASCADE) */
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
}
#if defined (USE_OPENCASCADE)
#include "cad/field_location.hpp"
#endif /* defined (USE_OPENCASCADE) */
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

typedef std::map<Cmiss_region_id, Cmiss_field_group_id> Region_field_map;
typedef std::map<Cmiss_region_id, Cmiss_field_group_id>::iterator Region_field_map_iterator;
typedef std::map<Cmiss_region_id, Cmiss_field_group_id>::const_iterator Region_field_map_const_iterator;

namespace {

char computed_field_group_type_string[] = "group";

class Computed_field_group : public Computed_field_group_base
{
private:
	Cmiss_field_hierarchical_group_change_detail change_detail;
	Cmiss_region *region;
	int contains_all;
	Computed_field *local_node_group, *local_data_group, *local_element_group[MAXIMUM_ELEMENT_XI_DIMENSIONS];

public:
	std::map<Computed_field *, Computed_field *> domain_selection_group;
	Region_field_map subregion_group_map;
	Region_field_map_iterator child_group_pos;

	Computed_field_group(Cmiss_region *region)
		: Computed_field_group_base()
		, region(region)
		, contains_all(0)
		, local_node_group(NULL)
		, local_data_group(NULL)
		, subregion_group_map()
	{		//ACCESS(Cmiss_region)(region);
		child_group_pos = subregion_group_map.begin();
		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
			local_element_group[i] = NULL;
	}

	~Computed_field_group()
	{
		clear();
		remove_empty_subregion_groups();
		std::map<Computed_field *, Computed_field *>::iterator it = domain_selection_group.begin();
		for (;it != domain_selection_group.end(); it++)
		{
			//Cmiss_field_destroy(&(it->first)); don't destroy this it is not a reference just a key
			Cmiss_field_destroy(&(it->second));
		}
	}

	Cmiss_field_node_group_id create_node_group(Cmiss_nodeset_id nodeset);

	Cmiss_field_node_group_id get_node_group(Cmiss_nodeset_id nodeset);

	Cmiss_field_element_group_id create_element_group(Cmiss_fe_mesh_id mesh);

	Cmiss_field_element_group_id get_element_group(Cmiss_fe_mesh_id mesh);

	Cmiss_field_id get_subobject_group_for_domain(Cmiss_field_id domain);

#if defined (USE_OPENCASCADE)
	Cmiss_field_id create_cad_primitive_group(Cmiss_field_cad_topology_id cad_topology_domain);

	int clear_region_tree_cad_primitive();
#endif /*defined (USE_OPENCASCADE) */

	Cmiss_field_group_id getSubRegionGroup(Cmiss_region_id subregion);

	Cmiss_field_group_id createSubRegionGroup(Cmiss_region_id subregion);

	int clear_region_tree_node(int use_data);

	int clear_region_tree_element();

	Cmiss_field_id getFirstChild();

	Cmiss_field_id getNextChild();

	int remove_empty_subregion_groups();

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

	int isEmptyLocal() const;

	virtual int isEmpty() const
	{
		return (isEmptyLocal() && isEmptyNonLocal());
	}

	virtual int clear();

	int clearLocal();

	int addLocalRegion();

	int containsLocalRegion();

	int addRegion(struct Cmiss_region *child_region);

	int removeRegion(struct Cmiss_region *region);

	int containsRegion(struct Cmiss_region *region);

private:

	Computed_field_core* copy()
	{
		Computed_field_group *core = new Computed_field_group(region);
		core->contains_all = this->contains_all;
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

	Cmiss_field_element_group_id get_element_group_private(int dimension)
	{
		if ((dimension > 0) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
			return Cmiss_field_cast_element_group(local_element_group[dimension - 1]);
		return 0;
	}

	int getSubgroupLocal();

	int add_region_tree(struct Cmiss_region *region_tree);

	int remove_region(struct Cmiss_region *child_region);

	int remove_region_tree(struct Cmiss_region *child_region);

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

	int isEmptyNonLocal() const;

	int check_subobject_group_dependency(Computed_field_core *source_core);

};

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
		if (contains_all)
		{
			field->values[0] = 1;
		}
#if defined (USE_OPENCASCADE)
		else if (dynamic_cast<Field_cad_geometry_location*>(location))
		{
			printf("=== Cad geometry field location\n");
		}
#endif /* defined (USE_OPENCASCADE) */
		else if (dynamic_cast<Field_node_location*>(location))
		{
			Field_node_location *node_location = 
				reinterpret_cast<Field_node_location *>(location);
			Cmiss_node_id node = node_location->get_node();
			FE_region *fe_region = FE_node_get_FE_region(node);
			Cmiss_region_id node_region = NULL;
			FE_region_get_Cmiss_region(fe_region, &node_region);
			if (local_node_group)
			{
				Cmiss_field_node_group_id node_group_id = 
					Cmiss_field_cast_node_group(local_node_group);
				field->values[0] = Cmiss_field_node_group_contains_node(
					node_group_id, node);
				Cmiss_field_node_group_destroy(&node_group_id);
			}
			if (!(field->values[0]) &&  local_data_group)
			{
				Cmiss_field_node_group_id data_group_id =
					Cmiss_field_cast_node_group(local_data_group);
				field->values[0] = Cmiss_field_node_group_contains_node(
					data_group_id, node);
				Cmiss_field_node_group_destroy(&data_group_id);
			}
			if (!field->values[0] && node_region != region)
			{
				Region_field_map_iterator pos = subregion_group_map.find(node_region);
				if (pos == subregion_group_map.end())
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
 			int dimension = Cmiss_element_get_dimension(element);
 			Cmiss_field_element_group_id element_group = get_element_group_private(dimension);
 			if (element_group)
 			{
 				field->values[0] = Cmiss_field_element_group_contains_element(element_group, element);
				Cmiss_field_element_group_destroy(&element_group);
 			}
 			else
 			{
 				field->values[0] = 0;
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
	child_group_pos = subregion_group_map.begin();
	if (child_group_pos !=subregion_group_map.end())
	{
		child_field = Cmiss_field_access(Cmiss_field_group_base_cast(child_group_pos->second));
	}
	return child_field;
}

Cmiss_field_id Computed_field_group::getNextChild()
{
	Cmiss_field_id child_field= NULL;
	if (child_group_pos !=subregion_group_map.end())
	{
		child_group_pos++;
		if (child_group_pos !=subregion_group_map.end())
		{
			child_field = Cmiss_field_access(Cmiss_field_group_base_cast(child_group_pos->second));
		}
	}
	return child_field;
}

int Computed_field_group::isEmptyLocal() const
{
	if (contains_all)
	{
		return 0;
	}
	if (local_node_group && !isSubGroupEmpty(local_node_group->core))
		return 0;
	if (local_data_group && !isSubGroupEmpty(local_data_group->core))
		return 0;
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		if (local_element_group[i] && !isSubGroupEmpty(local_element_group[i]->core))
			return 0;
	}
	std::map<Computed_field *, Computed_field *>::const_iterator it = domain_selection_group.begin();
	while (it != domain_selection_group.end())
	{
		Computed_field *subobject_group_field = it->second;
		if (!isSubGroupEmpty(subobject_group_field->core))
			return 0;
		++it;
	}
	return 1;
}

int Computed_field_group::isEmptyNonLocal() const
{
	for (Region_field_map_const_iterator iter = subregion_group_map.begin();
		iter != subregion_group_map.end(); iter++)
	{
		if (!Cmiss_region_is_group(iter->first))
		{
			Cmiss_field_group_id subregion_group_field = iter->second;
			if (!isSubGroupEmpty(Cmiss_field_group_base_cast(subregion_group_field)->core))
				return 0;
		}
	}
	return 1;
}

int Computed_field_group::clearLocal()
{
	int return_code = 1;
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_module_begin_change(field_module);
	contains_all = 0;
	if (local_node_group)
	{
		Computed_field_group_base *group_base = dynamic_cast<Computed_field_group_base *>(local_node_group->core);
		return_code = group_base->clear();
		Cmiss_field_destroy(&local_node_group);
	}
	if (local_data_group)
	{
		Computed_field_group_base *group_base = dynamic_cast<Computed_field_group_base *>(local_data_group->core);
		return_code = group_base->clear();
		Cmiss_field_destroy(&local_data_group);
	}
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		if (local_element_group[i])
		{
			Computed_field_group_base *group_base = dynamic_cast<Computed_field_group_base *>(local_element_group[i]->core);
			return_code = group_base->clear();
			Cmiss_field_destroy(&local_element_group[i]);
		}
	}
	Computed_field_changed(this->field);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	return return_code;
};

int Computed_field_group::clear()
{
	int return_code = 0;
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_module_begin_change(field_module);
	for (Region_field_map_iterator iter = subregion_group_map.begin();
		iter != subregion_group_map.end(); iter++)
	{
		Computed_field_group *group_core = Computed_field_group_core_cast(iter->second);
		group_core->clear();
	}
	return_code = clearLocal();
	Computed_field_changed(this->field);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	return return_code;
};

int Computed_field_group::check_subobject_group_dependency(Computed_field_core *source_core)
{
	Computed_field_subobject_group *subobject_group = dynamic_cast<Computed_field_subobject_group *>(source_core);
	/* check_dependency method is not sufficient to determine a subobject group has changed or not for a group */
	if (subobject_group->check_dependency_for_group_special())
	{
		Computed_field_dependency_change_private(field);
		const Cmiss_field_subobject_group_change_detail *subobject_group_change_detail =
			dynamic_cast<const Cmiss_field_subobject_group_change_detail *>(source_core->get_change_detail());
		if (subobject_group_change_detail)
		{
			if ((subobject_group_change_detail->getChange() == CMISS_FIELD_GROUP_CLEAR) &&
				isEmptyLocal())
			{
				change_detail.changeClearLocal();
			}
			else
			{
				change_detail.changeMergeLocal(subobject_group_change_detail);
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
			check_subobject_group_dependency(local_node_group->core);
		if (local_data_group)
			check_subobject_group_dependency(local_data_group->core);
		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
		{
			if (local_element_group[i])
			{
				check_subobject_group_dependency(local_element_group[i]->core);
			}
		}
		std::map<Computed_field *, Computed_field *>::const_iterator it = domain_selection_group.begin();
		while (it != domain_selection_group.end())
		{
			Computed_field *subobject_group_field = it->second;
			check_subobject_group_dependency(subobject_group_field->core);
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
		for (Region_field_map_iterator iter = subregion_group_map.begin();
			iter != subregion_group_map.end(); iter++)
		{
			Cmiss_field_group_id subregion_group = iter->second;
			// future optimisation: check subfield is from changed region
			const Cmiss_field_change_detail *source_change_detail = NULL;
			int change = Computed_field_manager_message_get_object_change_and_detail(
				message, Cmiss_field_group_base_cast(subregion_group), &source_change_detail);
			if (change != MANAGER_CHANGE_NONE(Computed_field))
			{
				if (source_change_detail)
				{
					const Cmiss_field_group_base_change_detail *subregion_group_change_detail =
						dynamic_cast<const Cmiss_field_group_base_change_detail *>(source_change_detail);
					if (subregion_group_change_detail)
					{
						Cmiss_field_group_change_type subregion_group_change =
							subregion_group_change_detail->getChange();
						if (subregion_group_change != CMISS_FIELD_GROUP_NO_CHANGE)
						{
							if (((subregion_group_change == CMISS_FIELD_GROUP_CLEAR) ||
								(subregion_group_change == CMISS_FIELD_GROUP_REMOVE)) &&
								isEmptyNonLocal())
							{
								change_detail.changeClearNonLocal();
							}
							else
							{
								change_detail.changeMergeNonLocal(subregion_group_change_detail);
							}
							Computed_field_dependency_changed(field);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE, "Sub-region group changes could not be propagated.");
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

int Computed_field_group::removeRegion(Cmiss_region_id region)
{
	int return_code = 0;
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_group_id subgroup = getSubRegionGroup(region);
	if (subgroup)
	{
		Computed_field_group *group_core = Computed_field_group_core_cast(subgroup);
		group_core->contains_all = 0;
		Cmiss_field_group_destroy(&subgroup);
	}
	Computed_field_changed(this->field);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	return return_code;
}

int Computed_field_group::containsRegion(Cmiss_region_id region)
{
	int return_code = 0;
	Cmiss_field_group_id subgroup = getSubRegionGroup(region);
	if (subgroup)
	{
		Computed_field_group *group_core = Computed_field_group_core_cast(subgroup);
		return_code = group_core->containsLocalRegion();
		Cmiss_field_group_destroy(&subgroup);
	}
	return return_code;
}

Cmiss_field_group_id Computed_field_group::getSubRegionGroup(Cmiss_region_id subregion)
{
	Cmiss_field_group_id subregion_group = NULL;
	if (Cmiss_region_is_group(subregion))
	{
		Cmiss_region_id parent_region = Cmiss_region_get_parent(subregion);
		subregion_group = getSubRegionGroup(parent_region);
		Cmiss_region_destroy(&parent_region);
		return subregion_group;
	}
	if (region == subregion)
	{
		subregion_group = Cmiss_field_cast_group(this->getField());
	}
	else
	{
		Region_field_map_iterator iter = subregion_group_map.find(subregion);
		if (iter != subregion_group_map.end())
		{
			subregion_group = iter->second;
			ACCESS(Computed_field)(Cmiss_field_group_base_cast(subregion_group));
		}
		if (!subregion_group && !subregion_group_map.empty())
		{
			for (iter = subregion_group_map.begin(); iter != subregion_group_map.end(); iter++)
			{
				Cmiss_field_group_id temp = iter->second;
				Computed_field_group *group_core = Computed_field_group_core_cast(temp);
				subregion_group = group_core->getSubRegionGroup(subregion);
				if (subregion_group)
					break;
			}
		}
	}
	return subregion_group;
}

Cmiss_field_group_id Computed_field_group::createSubRegionGroup(Cmiss_region_id subregion)
{
	Cmiss_field_group_id subregion_group = NULL;
	if (Cmiss_region_is_group(subregion))
	{
		Cmiss_region_id parent_region = Cmiss_region_get_parent(subregion);
		subregion_group = createSubRegionGroup(parent_region);
		Cmiss_region_destroy(&parent_region);
		return subregion_group;
	}
	if (Cmiss_region_contains_subregion(region, subregion) && region != subregion)
	{
		Cmiss_region_id parent_region = Cmiss_region_get_parent(subregion);
		if (parent_region != region)
		{
			Cmiss_field_group_id temp = getSubRegionGroup(subregion);
			if (!temp)
			{
				/* this will construct the hierarchy tree */
				temp = getSubRegionGroup(parent_region);
				if (!temp)
					temp = createSubRegionGroup(parent_region);
				if (temp)
				{
					Computed_field_group *group_core = Computed_field_group_core_cast(temp);
					subregion_group = group_core->createSubRegionGroup(subregion);
				}
			}
			if (temp)
				Cmiss_field_group_destroy(&temp);
		}
		else // (parent_region == region)
		{
			Region_field_map_iterator pos = subregion_group_map.find(subregion);
			if (pos == subregion_group_map.end())
			{
				Cmiss_field_module_id field_module =
					Cmiss_region_get_field_module(subregion);
				Cmiss_field_id generic_field =
					Cmiss_field_module_find_field_by_name(field_module, this->getField()->name);
				if (generic_field)
				{
					subregion_group = Cmiss_field_cast_group(generic_field);
					// Not calling Cmiss_field_set_attribute_integer(subregion_group, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 0);
					Cmiss_field_destroy(&generic_field);
				}
				if (!subregion_group)
				{
					Cmiss_field_module_set_field_name(field_module, this->getField()->name);
					subregion_group = reinterpret_cast<Cmiss_field_group_id>(Cmiss_field_module_create_group(field_module));
				}
				Cmiss_field_module_destroy(&field_module);
				ACCESS(Computed_field)(Cmiss_field_group_base_cast(subregion_group));
				subregion_group_map.insert(std::make_pair(subregion, subregion_group));
			}
			// else already exists: fail
		}
		if (parent_region)
			Cmiss_region_destroy(&parent_region);
	}
	return subregion_group;
}

Cmiss_field_node_group_id Computed_field_group::create_node_group(Cmiss_nodeset_id nodeset)
{
	int use_data = 0;
	Cmiss_field_node_group_id node_field = NULL;
	if (!contains_all)
	{
		struct FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
		if (!FE_region_get_data_FE_region(fe_region))
		{
			use_data = 1;
		}
		if ((!use_data && !local_node_group) || (use_data && !local_data_group))
		{
			Cmiss_field_module_id field_module =
				Cmiss_region_get_field_module(region);
			node_field = get_node_group(nodeset);
			if (node_field)
			{
				Cmiss_field_node_group_destroy(&node_field);
			}
			if (!use_data && !local_node_group)
			{
				char *temp_string = Cmiss_field_get_name(this->getField());
				int error = 0;
				append_string(&temp_string, ".nodes", &error);
				local_node_group = Cmiss_field_module_find_field_by_name(field_module, temp_string);
				if (!local_node_group)
				{
					Cmiss_field_module_set_field_name(field_module, temp_string);
					local_node_group = Cmiss_field_module_create_node_group(field_module, nodeset);
				}
				Cmiss_field_set_attribute_integer(local_node_group, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 0);
				node_field = Cmiss_field_cast_node_group(local_node_group);
				DEALLOCATE(temp_string);
			}
			if (use_data && !local_data_group)
			{
				char *temp_string = Cmiss_field_get_name(this->getField());
				int error = 0;
				append_string(&temp_string, ".data", &error);
				local_data_group = Cmiss_field_module_find_field_by_name(field_module, temp_string);
				if (!local_data_group)
				{
					Cmiss_field_module_set_field_name(field_module, temp_string);
					local_data_group = Cmiss_field_module_create_node_group(field_module, nodeset);
				}
				Cmiss_field_set_attribute_integer(local_data_group, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 0);
				node_field = Cmiss_field_cast_node_group(local_data_group);
				DEALLOCATE(temp_string);
			}
			Cmiss_field_module_destroy(&field_module);
		}
	}
	return (node_field);
}

Cmiss_field_node_group_id Computed_field_group::get_node_group(Cmiss_nodeset_id nodeset)
{
	int use_data = 0;
	Cmiss_field_node_group_id node_field = NULL;
	if (!contains_all)
	{
		struct FE_region *fe_region = reinterpret_cast<FE_region *>(nodeset);
		if (!FE_region_get_data_FE_region(fe_region))
		{
			use_data = 1;
		}
		if (!use_data && local_node_group)
		{
			node_field = Cmiss_field_cast_node_group(local_node_group);
		}
		else if (use_data && local_data_group)
		{
			node_field = Cmiss_field_cast_node_group(local_data_group);
		}
		if (!node_field)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			char *temp_string = Cmiss_field_get_name(this->getField());
			int error = 0;
			if (!use_data)
			{
				append_string(&temp_string, ".nodes", &error);
				local_node_group = Cmiss_field_module_find_field_by_name(
					field_module, temp_string);
				if (local_node_group)
					node_field= Cmiss_field_cast_node_group(local_node_group);
			}
			else
			{
				append_string(&temp_string, ".data", &error);
				local_data_group = Cmiss_field_module_find_field_by_name(
					field_module, temp_string);
				if (local_data_group)
					node_field= Cmiss_field_cast_node_group(local_data_group);
			}
			DEALLOCATE(temp_string);
			Cmiss_field_module_destroy(&field_module);
		}
	}
	return (node_field);
}

Cmiss_field_element_group_id Computed_field_group::create_element_group(Cmiss_fe_mesh_id mesh)
{
	Cmiss_field_element_group_id element_field = NULL;
	if (!contains_all && mesh)
	{
		int dimension = Cmiss_fe_mesh_get_dimension(mesh);
		if (!local_element_group[dimension - 1])
		{
			Cmiss_field_module_id field_module =
				Cmiss_region_get_field_module(region);
			element_field = get_element_group(mesh);
			if (element_field)
			{
				Cmiss_field_element_group_destroy(&element_field);
			}
			if (!local_element_group[dimension - 1])
			{
				char *temp_string = Cmiss_field_get_name(this->getField());
				int error = 0;
				char suffix[30];
				sprintf(suffix, ".mesh_%dd", dimension);
				append_string(&temp_string, suffix, &error);
				local_element_group[dimension - 1] = Cmiss_field_module_find_field_by_name(field_module, temp_string);
				if (!local_element_group[dimension - 1])
				{
					Cmiss_field_module_set_field_name(field_module, temp_string);
					local_element_group[dimension - 1] = Cmiss_field_module_create_element_group(field_module, mesh);
				}
				Cmiss_field_set_attribute_integer(local_element_group[dimension - 1], CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 0);
				element_field = Cmiss_field_cast_element_group(local_element_group[dimension - 1]);
				DEALLOCATE(temp_string);
			}
			Cmiss_field_module_destroy(&field_module);
		}
	}

	return (element_field);
}

Cmiss_field_element_group_id Computed_field_group::get_element_group(Cmiss_fe_mesh_id mesh)
{
	Cmiss_field_element_group_id element_field = NULL;
	if (!contains_all && mesh)
	{
		int dimension = Cmiss_fe_mesh_get_dimension(mesh);
		if (local_element_group[dimension - 1])
		{
			element_field = Cmiss_field_cast_element_group(local_element_group[dimension - 1]);
		}
		else
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			char *temp_string = Cmiss_field_get_name(this->getField());
			int error = 0;
			char suffix[30];
			sprintf(suffix, ".mesh_%dd", dimension);
			append_string(&temp_string, suffix, &error);
			local_element_group[dimension - 1] = Cmiss_field_module_find_field_by_name(
				field_module, temp_string);
			DEALLOCATE(temp_string);
			if (local_element_group[dimension - 1])
				element_field = Cmiss_field_cast_element_group(local_element_group[dimension - 1]);
			Cmiss_field_module_destroy(&field_module);
		}
	}
	return (element_field);
}

Cmiss_field_id Computed_field_group::get_subobject_group_for_domain(Cmiss_field_id domain)
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
	if (cad_topology_domain)
	{
		const char *base_name = "cad_primitive_selection";
		const char *domain_field_name = Cmiss_field_get_name(reinterpret_cast<Cmiss_field_id>(cad_topology_domain));
		char *field_name = NULL;
		int error = 0;
		ALLOCATE(field_name, char, strlen(base_name)+strlen(domain_field_name)+2);
		field_name[0] = '\0';
		append_string(&field_name, base_name, &error);
		append_string(&field_name, "_", &error);
		append_string(&field_name, domain_field_name, &error);

		Cmiss_field_module_id field_module =
			Cmiss_region_get_field_module(region);
		Cmiss_field_module_set_field_name(field_module, field_name);
		field = Cmiss_field_module_create_cad_primitive_group_template(field_module);
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

int Computed_field_group::clear_region_tree_cad_primitive()
{
	Region_field_map_iterator pos;
	int return_code = 1;
	Cmiss_field_group_id group_field = NULL;
	std::map<Computed_field *, Computed_field *>::iterator it = domain_selection_group.begin();
	while (it != domain_selection_group.end())
	{
		Cmiss_field_cad_primitive_group_template_id cad_primitive_group =
			Cmiss_field_cast_cad_primitive_group_template(it->second);
		return_code = Cmiss_field_cad_primitive_group_template_clear(cad_primitive_group);
		Computed_field_changed(this->field);
		//Cmiss_field_id cad_primitive_group_field = reinterpret_cast<Computed_field*>(cad_primitive_group);
		Cmiss_field_cad_primitive_group_template_destroy(&cad_primitive_group);
		Cmiss_field_destroy(&it->second);
		domain_selection_group.erase(it++);
	}
	if (!subregion_group_map.empty())
	{
		for (pos = subregion_group_map.begin(); pos != subregion_group_map.end(); pos++)
		{
			group_field = pos->second;
			Cmiss_field_group_clear_region_tree_cad_primitive(group_field);
		}
	}

	return (return_code);
}

#endif /* defined (USE_OPENCASCADE) */


int Computed_field_group::addRegion(struct Cmiss_region *child_region)
{
	int return_code = 0;
	if (Cmiss_region_contains_subregion(region, child_region))
	{
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(child_region);
		Cmiss_field_module_begin_change(field_module);
		Cmiss_field_group_id subregion_group = getSubRegionGroup(child_region);
		if (!subregion_group)
			subregion_group = createSubRegionGroup(child_region);
		Computed_field_group *group_core =
			Computed_field_group_core_cast(subregion_group);
		group_core->addLocalRegion();
		Cmiss_field_group_destroy(&subregion_group);
		Computed_field_changed(this->field);
		Cmiss_field_module_end_change(field_module);
		Cmiss_field_module_destroy(&field_module);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_group::addRegion.  Sub region is not a child region"
			"or part of the parent region");
		return_code = 0;
	}

	return (return_code);
}

void Computed_field_group::remove_child_group(struct Cmiss_region *child_region)
{
	if (Cmiss_region_contains_subregion(region, child_region))
	{
		Region_field_map_iterator pos = subregion_group_map.find(child_region);
		if (pos != subregion_group_map.end())
		{
			Cmiss_region_id region = pos->first;
			if (region)
			{
				Cmiss_field_group_id temp = pos->second;
				subregion_group_map.erase(child_region);
				Cmiss_field_group_destroy(&temp);
			}
		}
	}
}

int Computed_field_group::remove_empty_subregion_groups()
{
	/* remove_empty_subgroups */
	for (Region_field_map_iterator iter = subregion_group_map.begin();
		iter != subregion_group_map.end();)
	{
		Cmiss_field_group_id subregion_group_field = iter->second;
		Computed_field_group *group_core = Computed_field_group_core_cast(subregion_group_field);
		group_core->remove_empty_subregion_groups();
		if (group_core->isEmpty())
		{
			subregion_group_map.erase(iter++);
			Cmiss_field_group_destroy(&subregion_group_field);
		}
		else
		{
			++iter;
		}
	}
	return 1;
}

int Computed_field_group::clear_region_tree_node(int use_data)
{
	Region_field_map_iterator pos;
	int return_code = 1;
	Cmiss_field_group_id group_field = NULL;
	if (!use_data && local_node_group)
	{
		Cmiss_field_node_group_id node_group =
			Cmiss_field_cast_node_group(local_node_group);
		return_code = Cmiss_field_node_group_clear(node_group);
		Computed_field_changed(this->field);
		Cmiss_field_node_group_destroy(&node_group);
	}
	if (use_data && local_data_group)
	{
		Cmiss_field_node_group_id data_group =
			Cmiss_field_cast_node_group(local_data_group);
		return_code = Cmiss_field_node_group_clear(data_group);
		Computed_field_changed(this->field);
		Cmiss_field_node_group_destroy(&data_group);
	}
	if (!subregion_group_map.empty())
	{
		for (pos = subregion_group_map.begin(); pos != subregion_group_map.end(); pos++)
		{
			group_field = pos->second;
			if (!use_data)
				Cmiss_field_group_clear_region_tree_node(group_field);
			else
				Cmiss_field_group_clear_region_tree_data(group_field);
		}
	}
	return (return_code);
}

int Computed_field_group::addLocalRegion()
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_module_begin_change(field_module);
	clearLocal();
	contains_all = 1;
	Computed_field_changed(this->field);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	return 1;
}

int Computed_field_group::containsLocalRegion()
{
	return contains_all;
}

int Computed_field_group::clear_region_tree_element()
{
	Region_field_map_iterator pos;
	int return_code = 1;
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		if (local_element_group[i])
		{
			Cmiss_field_element_group_id element_group =
				Cmiss_field_cast_element_group(local_element_group[i]);
			return_code = Cmiss_field_element_group_clear(element_group);
			Computed_field_changed(this->field);
			Cmiss_field_element_group_destroy(&element_group);
		}
	}
	if (!subregion_group_map.empty())
	{
		for (pos = subregion_group_map.begin(); pos != subregion_group_map.end(); pos++)
		{
			Cmiss_field_group_clear_region_tree_element(pos->second);
		}
	}
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

Cmiss_field_node_group_id Cmiss_field_group_create_node_group(Cmiss_field_group_id group, Cmiss_nodeset_id nodeset)
{
	Cmiss_field_node_group_id field = NULL;
	if (group && nodeset)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->create_node_group(nodeset);
		}
	}

	return field;
}

Cmiss_field_node_group_id Cmiss_field_group_get_node_group(Cmiss_field_group_id group, Cmiss_nodeset_id nodeset)
{
	Cmiss_field_node_group_id field = NULL;
	if (group && nodeset)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_node_group(nodeset);
		}
	}

	return field;
}

Cmiss_field_element_group_id Cmiss_field_group_create_element_group(Cmiss_field_group_id group,
	Cmiss_fe_mesh_id mesh)
{
	Cmiss_field_element_group_id field = NULL;
	if (group && mesh)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->create_element_group(mesh);
		}
	}

	return field;
}

Cmiss_field_element_group_id Cmiss_field_group_get_element_group(Cmiss_field_group_id group,
	Cmiss_fe_mesh_id mesh)
{
	Cmiss_field_element_group_id field = NULL;
	if (group && mesh)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_element_group(mesh);
		}
	}

	return field;
}

#if defined (USE_OPENCASCADE)
Cmiss_field_cad_primitive_group_template_id Cmiss_field_group_create_cad_primitive_group(Cmiss_field_group_id group, Cmiss_field_cad_topology_id cad_topology_domain)
{
	Cmiss_field_cad_primitive_group_template_id cad_primitive_group = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			Cmiss_field_id field = group_core->create_cad_primitive_group(cad_topology_domain);
			if (field != NULL)
			{
				cad_primitive_group = Cmiss_field_cast_cad_primitive_group_template(field);
				Cmiss_field_destroy(&field);
			}
		}
	}

	return cad_primitive_group;
}

Cmiss_field_cad_primitive_group_template_id Cmiss_field_group_get_cad_primitive_group(Cmiss_field_group_id group, Cmiss_field_cad_topology_id cad_topology_domain)
{
	Cmiss_field_cad_primitive_group_template_id cad_primitive_group = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			Cmiss_field_id field = group_core->get_subobject_group_for_domain(reinterpret_cast< Computed_field * >(cad_topology_domain));//cad_primitive_group();
			if (field != NULL)
			{
				cad_primitive_group = Cmiss_field_cast_cad_primitive_group_template(field);
				Cmiss_field_destroy(&field);
			}
		}
	}

	return cad_primitive_group;
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

Cmiss_field_group_id Cmiss_field_group_get_subregion_group(
	Cmiss_field_group_id group, Cmiss_region_id subregion)
{
	Cmiss_field_group_id subgroup = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			subgroup = group_core->getSubRegionGroup(subregion);
		}
	}
	return subgroup;
}

Cmiss_field_group_id Cmiss_field_group_create_subregion_group(
	Cmiss_field_group_id group, Cmiss_region_id subregion)
{
	Cmiss_field_group_id subgroup = NULL;
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			subgroup = group_core->createSubRegionGroup(subregion);
		}
	}
	return subgroup;
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

int Cmiss_field_group_remove_empty_subregion_groups(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->remove_empty_subregion_groups();
		}
	}
	return 0;
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

Cmiss_field_id Cmiss_field_group_get_subobject_group_for_domain(Cmiss_field_group_id group, Cmiss_field_id domain)
{
	Computed_field *field = NULL;

	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			field = group_core->get_subobject_group_for_domain(domain);
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

int Cmiss_field_group_destroy(Cmiss_field_group_id *group_address)
{
	return Cmiss_field_destroy(reinterpret_cast<Cmiss_field_id *>(group_address));
}

Computed_field *Cmiss_field_module_create_group(Cmiss_field_module_id field_module)
{
	Computed_field *field;

	ENTER(Computed_field_create_group);
	field = (Computed_field *)NULL;
	// GRC original_field->manager check will soon be unnecessary
	if (field_module)
	{
		Cmiss_region_id region = Cmiss_field_module_get_region(field_module);
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_group(region));
		Cmiss_region_destroy(&region);
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
					Cmiss_field_module_create_group(field_modify->get_field_module()));
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

int Cmiss_field_group_clear(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->clear();
		}
	}
	return 0;
}

int Cmiss_field_group_clear_local(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->clearLocal();
		}
	}
	return 0;
}


int Cmiss_field_group_is_empty(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->isEmpty();
		}
	}
	return 0;
}

int Cmiss_field_group_is_empty_local(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->isEmptyLocal();
		}
	}
	return 0;
}

int Cmiss_field_group_add_local_region(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->addLocalRegion();
		}
	}
	return 0;
}

int Cmiss_field_group_contains_local_region(Cmiss_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->containsLocalRegion();
		}
	}
	return 0;
}

int Cmiss_field_group_contains_region(Cmiss_field_group_id group, Cmiss_region_id region)
{
	if (group && region)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->containsRegion(region);
		}
	}
	return 0;
}

int Cmiss_field_group_remove_region(Cmiss_field_group_id group, Cmiss_region_id region)
{
	if (group && region)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->removeRegion(region);
		}
	}
	return 0;
}

int Cmiss_field_group_add_region(Cmiss_field_group_id group, Cmiss_region_id region)
{
	if (group && region)
	{
		Computed_field_group *group_core =
			Computed_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->addRegion(region);
		}
	}
	return 0;
}
