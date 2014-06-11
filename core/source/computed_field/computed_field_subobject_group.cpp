/***************************************************************************//**
 * FILE : computed_field_subobject_group.cpp
 *
 * Implements region sub object groups, e.g. node group, element group.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
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
#include "computed_field/field_cache.hpp"
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
	cmzn_fieldcache_id cache;
	cmzn_field_id field;
};

int cmzn_element_field_is_true_iterator(cmzn_element_id element, void *data_void)
{
	cmzn_element_field_is_true_iterator_data *data =
		static_cast<cmzn_element_field_is_true_iterator_data *>(data_void);
	cmzn_fieldcache_set_element(data->cache, element);
	return cmzn_field_evaluate_boolean(data->field, data->cache);
}

struct cmzn_node_field_is_true_iterator_data
{
	cmzn_fieldcache_id cache;
	cmzn_field_id field;
};

int cmzn_node_field_is_true_iterator(cmzn_node_id node, void *data_void)
{
	cmzn_node_field_is_true_iterator_data *data =
		reinterpret_cast<cmzn_node_field_is_true_iterator_data *>(data_void);
	cmzn_fieldcache_set_node(data->cache, node);
	return cmzn_field_evaluate_boolean(data->field, data->cache);
}

} // anonymous namespace

cmzn_mesh_id Computed_field_element_group::getConditionalFieldIterationMesh(
	cmzn_field_id conditional_field) const
{
	cmzn_mesh_id mesh = 0;
	Computed_field_element_group *other_element_group =
		dynamic_cast<Computed_field_element_group *>(conditional_field->core);
	cmzn_field_group_id group;
	if (other_element_group)
	{
		mesh = cmzn_mesh_group_base_cast(cmzn_field_element_group_get_mesh_group(
			reinterpret_cast<cmzn_field_element_group_id>(other_element_group->field)));
	}
	else if (0 != (group = cmzn_field_cast_group(conditional_field)))
	{
		cmzn_field_element_group_id element_group = cmzn_field_group_get_field_element_group(group, this->master_mesh);
		if (element_group)
		{
			mesh = cmzn_mesh_group_base_cast(cmzn_field_element_group_get_mesh_group(element_group));
			cmzn_field_element_group_destroy(&element_group);
		}
		cmzn_field_group_destroy(&group);
	}
	else
		mesh = cmzn_mesh_access(this->master_mesh);
	return mesh;
}

int Computed_field_element_group::addElementsConditional(cmzn_field_id conditional_field)
{
	if ((!conditional_field) || (conditional_field->manager != this->field->manager))
		return CMZN_ERROR_ARGUMENT;
	cmzn_mesh_id iterationMesh = this->getConditionalFieldIterationMesh(conditional_field);
	if (iterationMesh)
	{
		const int old_size = NUMBER_IN_LIST(FE_element)(this->object_list);
		cmzn_elementiterator_id iter = cmzn_mesh_create_elementiterator(iterationMesh);
		cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(this->field);
		cmzn_fieldcache *cache = 0;
		if (cmzn_mesh_match(this->master_mesh, iterationMesh))
			cache = cmzn_fieldmodule_create_fieldcache(field_module);
		cmzn_element_id element = 0;
		while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
		{
			if (cache)
			{
				cache->setElement(element);
				if (!cmzn_field_evaluate_boolean(conditional_field, cache))
					continue;
			}
			ADD_OBJECT_TO_LIST(FE_element)(element, this->object_list);
		}
		cmzn_elementiterator_destroy(&iter);
		cmzn_fieldcache_destroy(&cache);
		cmzn_fieldmodule_destroy(&field_module);
		const int new_size = NUMBER_IN_LIST(FE_element)(this->object_list);
		if (new_size != old_size)
		{
			change_detail.changeAdd();
			update();
		}
		cmzn_mesh_destroy(&iterationMesh);
	}
	return CMZN_OK;
}

int Computed_field_element_group::removeElementsConditional(cmzn_field_id conditional_field)
{
	if ((!conditional_field) || (conditional_field->manager != this->field->manager))
		return CMZN_ERROR_ARGUMENT;
	int return_code = CMZN_OK;
	cmzn_mesh_id iterationMesh = this->getConditionalFieldIterationMesh(conditional_field);
	if (iterationMesh)
	{
		LIST(FE_element) *elementList = this->object_list;
		if (conditional_field->dependsOnField(this->field))
		{
			// copy list, since can't query it within REMOVE_OBJECTS_FROM_LIST_THAT
			elementList = CREATE_RELATED_LIST(FE_element)(this->object_list);
			if (!COPY_LIST(FE_element)(elementList, this->object_list))
				return_code = CMZN_ERROR_MEMORY;
		}
		if (CMZN_OK == return_code)
		{
			const int old_size = NUMBER_IN_LIST(FE_element)(this->object_list);
			if (cmzn_mesh_get_size(iterationMesh) < this->getSize())
			{
				cmzn_elementiterator_id iter = cmzn_mesh_create_elementiterator(iterationMesh);
				cmzn_element_id element = 0;
				while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
					REMOVE_OBJECT_FROM_LIST(FE_element)(element, elementList);
				cmzn_elementiterator_destroy(&iter);
			}
			else
			{
				cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(this->field);
				cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(field_module);
				cmzn_element_field_is_true_iterator_data data = { cache, conditional_field };
				if (!REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(
					cmzn_element_field_is_true_iterator, (void *)&data, elementList))
				{
					return_code = CMZN_ERROR_GENERAL;
				}
				cmzn_fieldcache_destroy(&cache);
				cmzn_fieldmodule_destroy(&field_module);
			}
			if ((elementList != this->object_list) && (CMZN_OK == return_code))
			{
				DESTROY(LIST(FE_element))(&this->object_list);
				this->object_list = elementList;
			}
			const int new_size = NUMBER_IN_LIST(FE_element)(this->object_list);
			if (new_size != old_size)
			{
				change_detail.changeRemove();
				update();
			}
		}
		if (elementList != this->object_list)
			DESTROY(LIST(FE_element))(&elementList);
		cmzn_mesh_destroy(&iterationMesh);
	}
	return return_code;
}

int Computed_field_element_group::addElementFaces(cmzn_element_id parent)
{
	if (!isParentElementCompatible(parent))
		return CMZN_ERROR_ARGUMENT;
	int return_code = CMZN_OK;
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
					return_code = CMZN_ERROR_GENERAL;
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
		return CMZN_ERROR_ARGUMENT;
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
		change_detail.changeRemove();
		update();
	}
	return CMZN_OK;
};

int Computed_field_node_group::removeNodesConditional(cmzn_field_id conditional_field)
{
	if (!conditional_field)
		return CMZN_ERROR_ARGUMENT;
	Computed_field_node_group *other_node_group = 0;
	cmzn_field_group_id group = cmzn_field_cast_group(conditional_field);
	if (group)
	{
		cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(group, master_nodeset);
		cmzn_field_group_destroy(&group);
		if (!node_group)
			return CMZN_OK;
		other_node_group = Computed_field_node_group_core_cast(node_group);
		cmzn_field_node_group_destroy(&node_group);
	}
	else
	{
		other_node_group = dynamic_cast<Computed_field_node_group *>(conditional_field->core);
	}
	int return_code = CMZN_OK;
	const int old_size = NUMBER_IN_LIST(FE_node)(object_list);
	if (other_node_group)
	{
		if (other_node_group->field == field)
			return clear();
		if (other_node_group->getSize() < getSize())
		{
			cmzn_nodeiterator_id iter = CREATE_LIST_ITERATOR(FE_node)(other_node_group->object_list);
			cmzn_node_id node = 0;
			while (0 != (node = cmzn_nodeiterator_next_non_access(iter)))
			{
				if (IS_OBJECT_IN_LIST(FE_node)(node, object_list))
					REMOVE_OBJECT_FROM_LIST(FE_node)(node, object_list);
			}
			cmzn_nodeiterator_destroy(&iter);
		}
		else
		{
			if (!REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(
				FE_node_is_in_list, (void *)other_node_group->object_list, object_list))
			{
				return_code = CMZN_ERROR_GENERAL;
			}
		}
	}
	else
	{
		cmzn_region_id region = cmzn_nodeset_get_region_internal(master_nodeset);
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
		cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(field_module);
		cmzn_node_field_is_true_iterator_data data = { cache, conditional_field };
		if (!REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(
			cmzn_node_field_is_true_iterator, (void *)&data, object_list))
		{
			return_code = CMZN_ERROR_GENERAL;
		}
		cmzn_fieldcache_destroy(&cache);
		cmzn_fieldmodule_destroy(&field_module);
	}
	const int new_size = NUMBER_IN_LIST(FE_node)(object_list);
	if (new_size != old_size)
	{
		change_detail.changeRemove();
		update();
	}
	return return_code;
}

int Computed_field_node_group::addElementNodes(cmzn_element_id element)
{
	if (!isParentElementCompatible(element))
		return CMZN_ERROR_ARGUMENT;
	int return_code = CMZN_OK;
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
						return_code = CMZN_ERROR_GENERAL;
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
							return_code = CMZN_ERROR_GENERAL;
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
		return CMZN_ERROR_ARGUMENT;
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
		change_detail.changeRemove();
		update();
	}
	return CMZN_OK;
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

Computed_field *cmzn_fieldmodule_create_field_node_group(cmzn_fieldmodule_id field_module, cmzn_nodeset_id nodeset)
{
	Computed_field *field;

	ENTER(cmzn_fieldmodule_create_field_node_group);
	field = (Computed_field *)NULL;
	if (field_module && nodeset && (cmzn_nodeset_get_region_internal(nodeset) ==
		cmzn_fieldmodule_get_region_internal(field_module)))
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
			"cmzn_fieldmodule_create_field_group.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* cmzn_fieldmodule_create_field_group */

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

Computed_field *cmzn_fieldmodule_create_field_element_group(cmzn_fieldmodule_id field_module,
		cmzn_mesh_id mesh)
{
	Computed_field *field;

	ENTER(cmzn_fieldmodule_create_field_element_group);
	field = (Computed_field *)NULL;
	if (field_module && mesh && (cmzn_mesh_get_region_internal(mesh) ==
		cmzn_fieldmodule_get_region_internal(field_module)))
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
			"cmzn_fieldmodule_create_field_group.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* cmzn_fieldmodule_create_field_group */

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

cmzn_field_id cmzn_fieldmodule_create_field_cad_primitive_group_template(cmzn_fieldmodule_id field_module)
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
			"cmzn_fieldmodule_create_field_group.  Invalid argument(s)");
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

