/***************************************************************************//**
 * FILE : computed_field_subobject_group.cpp
 *
 * Implements region sub object groups, e.g. node group, element group.
 */
/* ***** BEGIN LICENSE BLOCK *****
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*element_group
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
#include <stdlib.h>
#include "zinc/element.h"
#include "zinc/node.h"
#include "zinc/fieldmodule.h"
#include "zinc/fieldsubobjectgroup.h"
#include "computed_field/computed_field.h"
#if defined (USE_OPENCASCADE)
#include "zinc/fieldcad.h"
#endif /* defined (USE_OPENCASCADE) */

#include "computed_field/computed_field_subobject_group_internal.hpp"
#include "computed_field/computed_field_subobject_group_private.hpp"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_module.hpp"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "general/message.h"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/cmiss_node_private.hpp"
#if defined (USE_OPENCASCADE)
#include "cad/computed_field_cad_topology.h"
#include "cad/element_identifier.h"
#endif /* defined (USE_OPENCASCADE) */

namespace {

class Computed_field_sub_group_object_package : public Computed_field_type_package
{
public:
	cmzn_region *root_region;

	Computed_field_sub_group_object_package(cmzn_region *root_region)
		: root_region(root_region)
	{
		ACCESS(cmzn_region)(root_region);
	}
	
	~Computed_field_sub_group_object_package()
	{
		DEACCESS(cmzn_region)(&root_region);
	}
};

struct cmzn_element_field_is_true_iterator_data
{
	cmzn_field_cache_id cache;
	cmzn_field_id field;
};

int cmzn_element_field_is_true_iterator(cmzn_element_id element, void *data_void)
{
	cmzn_element_field_is_true_iterator_data *data =
		reinterpret_cast<cmzn_element_field_is_true_iterator_data *>(data_void);
	cmzn_field_cache_set_element(data->cache, element);
	return cmzn_field_evaluate_boolean(data->field, data->cache);
}

struct cmzn_node_field_is_true_iterator_data
{
	cmzn_field_cache_id cache;
	cmzn_field_id field;
};

int cmzn_node_field_is_true_iterator(cmzn_node_id node, void *data_void)
{
	cmzn_node_field_is_true_iterator_data *data =
		reinterpret_cast<cmzn_node_field_is_true_iterator_data *>(data_void);
	cmzn_field_cache_set_node(data->cache, node);
	return cmzn_field_evaluate_boolean(data->field, data->cache);
}

}

/** remove objects from the group if they are in the supplied list */
int Computed_field_element_group::removeElementsConditional(cmzn_field_id conditional_field)
{
	if (!conditional_field)
		return 0;
	Computed_field_element_group *other_element_group = 0;
	cmzn_field_group_id group = cmzn_field_cast_group(conditional_field);
	if (group)
	{
		cmzn_field_element_group_id element_group = cmzn_field_group_get_element_group(group, master_mesh);
		cmzn_field_group_destroy(&group);
		if (!element_group)
			return 1;
		other_element_group = Computed_field_element_group_core_cast(element_group);
		cmzn_field_element_group_destroy(&element_group);
	}
	else
	{
		other_element_group = dynamic_cast<Computed_field_element_group *>(conditional_field->core);
	}
	int return_code = 0;
	const int old_size = NUMBER_IN_LIST(FE_element)(object_list);
	if (other_element_group)
	{
		if (other_element_group->field == field)
			return clear();
		if (other_element_group->getSize() < getSize())
		{
			cmzn_element_iterator_id iter = CREATE_LIST_ITERATOR(FE_element)(other_element_group->object_list);
			cmzn_element_id element = 0;
			while (0 != (element = cmzn_element_iterator_next_non_access(iter)))
			{
				if (IS_OBJECT_IN_LIST(FE_element)(element, object_list))
					REMOVE_OBJECT_FROM_LIST(FE_element)(element, object_list);
			}
			cmzn_element_iterator_destroy(&iter);
		}
		else
		{
			return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(
				FE_element_is_in_list, (void *)other_element_group->object_list, object_list);
		}
	}
	else
	{
		cmzn_region_id region = cmzn_mesh_get_master_region_internal(master_mesh);
		cmzn_field_module_id field_module = cmzn_region_get_field_module(region);
		cmzn_field_cache_id cache = cmzn_field_module_create_cache(field_module);
		cmzn_element_field_is_true_iterator_data data = { cache, conditional_field };
		return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(
			cmzn_element_field_is_true_iterator, (void *)&data, object_list);
		cmzn_field_cache_destroy(&cache);
		cmzn_field_module_destroy(&field_module);
	}
	const int new_size = NUMBER_IN_LIST(FE_element)(object_list);
	if (new_size != old_size)
	{
		if (new_size == 0)
			change_detail.changeClear();
		else
			change_detail.changeRemove();
		update();
	}
	return return_code;
}

int Computed_field_element_group::addElementFaces(cmzn_element_id parent)
{
	if (!isParentElementCompatible(parent))
		return 0;
	int return_code = 1;
	int number_of_faces = 0;
	get_FE_element_number_of_faces(parent, &number_of_faces);
	cmzn_element_id face = 0;
	int number_added = 0;
	for (int i = 0; i < number_of_faces; i++)
	{
		if (get_FE_element_face(parent, i, &face) && face)
		{
			if (!IS_OBJECT_IN_LIST(FE_element)(face, object_list))
			{
				if (ADD_OBJECT_TO_LIST(FE_element)(face, object_list))
				{
					++number_added;
				}
				else
				{
					return_code = 0;
					break;
				}
			}
		}
	}
	if (number_added)
	{
		change_detail.changeAdd();
		update();
	}
	return return_code;
};

int Computed_field_element_group::removeElementFaces(cmzn_element_id parent)
{
	if (!isParentElementCompatible(parent))
		return 0;
	int number_of_faces = 0;
	get_FE_element_number_of_faces(parent, &number_of_faces);
	cmzn_element_id face = 0;
	int number_removed = 0;
	for (int i = 0; i < number_of_faces; i++)
	{
		if (get_FE_element_face(parent, i, &face) && face)
		{
			if (IS_OBJECT_IN_LIST(FE_element)(face, object_list))
			{
				REMOVE_OBJECT_FROM_LIST(FE_element)(face, object_list);
				++number_removed;
			}
		}
	}
	if (number_removed)
	{
		if (NUMBER_IN_LIST(FE_element)(object_list) == 0)
			change_detail.changeClear();
		else
			change_detail.changeRemove();
		update();
	}
	return 1;
};

int Computed_field_node_group::removeNodesConditional(cmzn_field_id conditional_field)
{
	if (!conditional_field)
		return 0;
	Computed_field_node_group *other_node_group = 0;
	cmzn_field_group_id group = cmzn_field_cast_group(conditional_field);
	if (group)
	{
		cmzn_field_node_group_id node_group = cmzn_field_group_get_node_group(group, master_nodeset);
		cmzn_field_group_destroy(&group);
		if (!node_group)
			return 1;
		other_node_group = Computed_field_node_group_core_cast(node_group);
		cmzn_field_node_group_destroy(&node_group);
	}
	else
	{
		other_node_group = dynamic_cast<Computed_field_node_group *>(conditional_field->core);
	}
	int return_code = 0;
	const int old_size = NUMBER_IN_LIST(FE_node)(object_list);
	if (other_node_group)
	{
		if (other_node_group->field == field)
			return clear();
		if (other_node_group->getSize() < getSize())
		{
			cmzn_node_iterator_id iter = CREATE_LIST_ITERATOR(FE_node)(other_node_group->object_list);
			cmzn_node_id node = 0;
			while (0 != (node = cmzn_node_iterator_next_non_access(iter)))
			{
				if (IS_OBJECT_IN_LIST(FE_node)(node, object_list))
					REMOVE_OBJECT_FROM_LIST(FE_node)(node, object_list);
			}
			cmzn_node_iterator_destroy(&iter);
		}
		else
		{
			return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(
				FE_node_is_in_list, (void *)other_node_group->object_list, object_list);
		}
	}
	else
	{
		cmzn_region_id region = cmzn_nodeset_get_master_region_internal(master_nodeset);
		cmzn_field_module_id field_module = cmzn_region_get_field_module(region);
		cmzn_field_cache_id cache = cmzn_field_module_create_cache(field_module);
		cmzn_node_field_is_true_iterator_data data = { cache, conditional_field };
		return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(
			cmzn_node_field_is_true_iterator, (void *)&data, object_list);
		cmzn_field_cache_destroy(&cache);
		cmzn_field_module_destroy(&field_module);
	}
	const int new_size = NUMBER_IN_LIST(FE_node)(object_list);
	if (new_size != old_size)
	{
		if (new_size == 0)
			change_detail.changeClear();
		else
			change_detail.changeRemove();
		update();
	}
	return return_code;
}

int Computed_field_node_group::addElementNodes(cmzn_element_id element)
{
	if (!isParentElementCompatible(element))
		return 0;
	int return_code = 1;
	int number_of_nodes = 0;
	int number_of_parents = 0;
	get_FE_element_number_of_nodes(element, &number_of_nodes);
	cmzn_node_id node = 0;
	int number_added = 0;
	if (number_of_nodes)
	{
		for (int i = 0; i < number_of_nodes; i++)
		{
			if (get_FE_element_node(element, i, &node) && node)
			{
				if (!IS_OBJECT_IN_LIST(FE_node)(node, object_list))
				{
					if (ADD_OBJECT_TO_LIST(FE_node)(node, object_list))
					{
						++number_added;
					}
					else
					{
						return_code = 0;
						break;
					}
				}
			}
		}
	}
	else if (get_FE_element_number_of_parents(element, &number_of_parents) &&
		(0 < number_of_parents))
	{
		int number_of_element_field_nodes;
		cmzn_node_id *element_field_nodes_array;
		if (calculate_FE_element_field_nodes(element, (struct FE_field *)NULL,
			&number_of_element_field_nodes, &element_field_nodes_array,
			/*top_level_element*/(struct FE_element *)NULL))
		{
			for (int i = 0; i < number_of_element_field_nodes; i++)
			{
				node = element_field_nodes_array[i];
				if (node)
				{
					if (!IS_OBJECT_IN_LIST(FE_node)(node, object_list))
					{
						if (ADD_OBJECT_TO_LIST(FE_node)(node, object_list))
						{
							++number_added;
						}
						else
						{
							display_message(ERROR_MESSAGE, "Could not add node");
							return_code = 0;
						}
					}
					cmzn_node_destroy(&node);
				}
			}
			DEALLOCATE(element_field_nodes_array);
		}
	}
	if (number_added)
	{
		change_detail.changeAdd();
		update();
	}
	return return_code;
};

int Computed_field_node_group::removeElementNodes(cmzn_element_id element)
{
	if (!isParentElementCompatible(element))
		return 0;
	int return_code = 1;
	int number_of_nodes = 0;
	int number_of_parents = 0;
	get_FE_element_number_of_nodes(element, &number_of_nodes);
	cmzn_node_id node = 0;
	int number_removed = 0;
	if (number_of_nodes)
	{
		for (int i = 0; i < number_of_nodes; i++)
		{
			if (get_FE_element_node(element, i, &node) && node)
			{
				if (IS_OBJECT_IN_LIST(FE_node)(node, object_list))
				{
					REMOVE_OBJECT_FROM_LIST(FE_node)(node, object_list);
					++number_removed;
				}
			}
		}
	}
	else if (get_FE_element_number_of_parents(element, &number_of_parents) &&
		(0 < number_of_parents))
	{
		int number_of_element_field_nodes;
		cmzn_node_id *element_field_nodes_array;
		if (calculate_FE_element_field_nodes(element, (struct FE_field *)NULL,
			&number_of_element_field_nodes, &element_field_nodes_array,
			/*top_level_element*/(struct FE_element *)NULL))
		{
			for (int i = 0; i < number_of_element_field_nodes; i++)
			{
				node = element_field_nodes_array[i];
				if (node)
				{
					if (IS_OBJECT_IN_LIST(FE_node)(node, object_list))
					{
						REMOVE_OBJECT_FROM_LIST(FE_node)(node, object_list);
						++number_removed;
					}
					cmzn_node_destroy(&node);
				}
			}
			DEALLOCATE(element_field_nodes_array);
		}
	}
	if (number_removed)
	{
		if (NUMBER_IN_LIST(FE_node)(object_list) == 0)
			change_detail.changeClear();
		else
			change_detail.changeRemove();
		update();
	}
	return return_code;
};

cmzn_field_node_group *cmzn_field_cast_node_group(cmzn_field_id field)
{
	if (field && dynamic_cast<Computed_field_node_group *>(field->core))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_node_group_id>(field));
	}
	else
	{
		return (NULL);
	}
}

inline Computed_field *Computed_field_cast(
	cmzn_field_node_group *node_group_field)
{
	return (reinterpret_cast<Computed_field*>(node_group_field));
}

int cmzn_field_node_group_destroy(cmzn_field_node_group_id *node_group_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(node_group_address));
}

Computed_field *cmzn_field_module_create_node_group(cmzn_field_module_id field_module, cmzn_nodeset_id nodeset)
{
	Computed_field *field;

	ENTER(cmzn_field_module_create_node_group);
	field = (Computed_field *)NULL;
	if (field_module && nodeset && (cmzn_nodeset_get_master_region_internal(nodeset) ==
		cmzn_field_module_get_master_region_internal(field_module)))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_node_group(nodeset));

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_field_module_create_group.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* cmzn_field_module_create_group */

cmzn_field_element_group *cmzn_field_cast_element_group(cmzn_field_id field)
{
	if (field && dynamic_cast<Computed_field_element_group*>(field->core))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_element_group_id>(field));
	}
	else
	{
		return (NULL);
	}
}

inline Computed_field *Computed_field_cast(
	cmzn_field_element_group *element_group_field)
{
	return (reinterpret_cast<Computed_field*>(element_group_field));
}

Computed_field *cmzn_field_module_create_element_group(cmzn_field_module_id field_module,
		cmzn_mesh_id mesh)
{
	Computed_field *field;

	ENTER(cmzn_field_module_create_element_group);
	field = (Computed_field *)NULL;
	if (field_module && mesh && (cmzn_mesh_get_master_region_internal(mesh) ==
		cmzn_field_module_get_master_region_internal(field_module)))
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_element_group(mesh));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_field_module_create_group.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* cmzn_field_module_create_group */

int cmzn_field_element_group_destroy(cmzn_field_element_group_id *element_group_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(element_group_address));
}

void cmzn_field_node_group_list_btree_statistics(
	cmzn_field_node_group_id node_group)
{
	Computed_field_node_group *node_group_core = Computed_field_node_group_core_cast(node_group);
	if (node_group_core)
	{
		node_group_core->write_btree_statistics();
	}
}

void cmzn_field_element_group_list_btree_statistics(
	cmzn_field_element_group_id element_group)
{
	Computed_field_element_group *element_group_core = Computed_field_element_group_core_cast(element_group);
	if (element_group_core)
	{
		element_group_core->write_btree_statistics();
	}
}

#if defined (USE_OPENCASCADE)

cmzn_field_id cmzn_field_module_create_cad_primitive_group_template(cmzn_field_module_id field_module)
{
	Computed_field *field = (struct Computed_field *)NULL;

	if (field_module)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/false, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_sub_group_object<cmzn_cad_identifier_id>());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_field_module_create_group.  Invalid argument(s)");
	}

	return (field);
}

int cmzn_field_cad_primitive_group_template_destroy(cmzn_field_cad_primitive_group_template_id *cad_primitive_group_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(cad_primitive_group_address));
}

cmzn_field_cad_primitive_group_template_id cmzn_field_cast_cad_primitive_group_template(cmzn_field_id field)
{
	if (dynamic_cast<Computed_field_sub_group_object<cmzn_cad_identifier_id>*>(field->core))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_cad_primitive_group_template_id>(field));
	}
	else
	{
		return (NULL);
	}
}

int cmzn_field_cad_primitive_group_template_add_cad_primitive(cmzn_field_cad_primitive_group_template_id cad_primitive_group,
	cmzn_cad_identifier_id cad_primitive)
{
	int return_code = 0;
	if (cad_primitive_group && cad_primitive)
	{
		Computed_field_sub_group_object<cmzn_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<cmzn_cad_identifier_id,
			cmzn_field_cad_primitive_group_template_id>(cad_primitive_group);
		int identifier = cad_primitive->identifier.number;
		cmzn_cad_identifier_id cad_primitive_copy = new cmzn_cad_identifier(cad_primitive->cad_topology, cad_primitive->identifier);
		//DEBUG_PRINT("=== Adding cad primitive object %p %d %d\n", cad_primitive_copy, cad_primitive->identifier.type, cad_primitive->identifier.number);
		group_core->add_object(identifier, cad_primitive_copy);
		return_code = 1;
	}

	return return_code;
}

int cmzn_field_cad_primitive_group_template_remove_cad_primitive(cmzn_field_cad_primitive_group_template_id cad_primitive_group,
	cmzn_cad_identifier_id cad_primitive)
{
	int return_code = 0;
	if (cad_primitive_group && cad_primitive)
	{
		Computed_field_sub_group_object<cmzn_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<cmzn_cad_identifier_id,
				cmzn_field_cad_primitive_group_template_id>(cad_primitive_group);
		int identifier = cad_primitive->identifier.number;
		//DEBUG_PRINT("=== Removing cad primitive object %p %d %d\n", cad_primitive, cad_primitive->identifier.type, cad_primitive->identifier.number);
		cmzn_cad_identifier_id cad_identifier = group_core->get_object(identifier);
		if (cad_identifier)
		{
			//DEBUG_PRINT("Deleting %p\n", cad_identifier);
			delete cad_identifier;
			group_core->remove_object(identifier);
			return_code = 1;
		}
	}

	return return_code;

}

int cmzn_field_cad_primitive_group_template_clear(cmzn_field_cad_primitive_group_template_id cad_primitive_group)
{
	int return_code = 0;

	if (cad_primitive_group)
	{
		Computed_field_sub_group_object<cmzn_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<cmzn_cad_identifier_id,
				cmzn_field_cad_primitive_group_template_id>(cad_primitive_group);
		cmzn_cad_identifier_id cad_identifier = group_core->getFirstObject();
		while (cad_identifier != NULL)
		{
			delete cad_identifier;
			cad_identifier = group_core->getNextObject();
		}
		return_code = group_core->clear();
	}

	return return_code;
}

int cmzn_field_cad_primitive_group_template_is_cad_primitive_selected(
	cmzn_field_cad_primitive_group_template_id cad_primitive_group, cmzn_cad_identifier_id cad_primitive)
{
	int return_code = 0;
	//struct CM_element_information cm_identifier;
	if (cad_primitive_group && cad_primitive)
	{
		Computed_field_sub_group_object<cmzn_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<cmzn_cad_identifier_id,
			cmzn_field_cad_primitive_group_template_id>(cad_primitive_group);
		int identifier = cad_primitive->identifier.number;
		return_code = group_core->get_object_selected(identifier, cad_primitive);
	}

	return return_code;
}

cmzn_cad_identifier_id cmzn_field_cad_primitive_group_template_get_first_cad_primitive(
	cmzn_field_cad_primitive_group_template_id cad_primitive_group)
{
	cmzn_cad_identifier_id cad_identifier = NULL;
	if (cad_primitive_group)
	{
		Computed_field_sub_group_object<cmzn_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<cmzn_cad_identifier_id,
			cmzn_field_cad_primitive_group_template_id>(cad_primitive_group);
		cmzn_cad_identifier_id cad_identifier_from_group = group_core->getFirstObject();
		if (cad_identifier_from_group)
		{
			cad_identifier = new cmzn_cad_identifier(*cad_identifier_from_group);
			//cmzn_field_cad_topology_access(cad_identifier->cad_topology);
		}
	}

	return cad_identifier;
}

cmzn_cad_identifier_id cmzn_field_cad_primitive_group_template_get_next_cad_primitive(
	cmzn_field_cad_primitive_group_template_id cad_primitive_group)
{
	cmzn_cad_identifier_id cad_identifier = NULL;
	if (cad_primitive_group)
	{
		Computed_field_sub_group_object<cmzn_cad_identifier_id> *group_core =
			Computed_field_sub_group_object_core_cast<cmzn_cad_identifier_id,
			cmzn_field_cad_primitive_group_template_id>(cad_primitive_group);
		cmzn_cad_identifier_id cad_identifier_from_group = group_core->getNextObject();
		if (cad_identifier_from_group)
		{
			cad_identifier = new cmzn_cad_identifier(*cad_identifier_from_group);
			//cmzn_field_cad_topology_access(cad_identifier->cad_topology);
		}
	}

	return cad_identifier;
}

#endif /* defined (USE_OPENCASCADE) */

