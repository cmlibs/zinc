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

Computed_field_subobject_group::~Computed_field_subobject_group()
{
	if (this->ownerGroup)
		display_message(ERROR_MESSAGE, "Computed_field_subobject_group %s destroyed with non-zero ownerGroup",
			this->field ? this->field->name : "?");
}

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

int cmzn_element_field_is_true_conditional(cmzn_element_id element, void *data_void)
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
		static_cast<cmzn_node_field_is_true_iterator_data *>(data_void);
	cmzn_fieldcache_set_node(data->cache, node);
	return cmzn_field_evaluate_boolean(data->field, data->cache);
}

} // anonymous namespace

Computed_field_element_group *Computed_field_element_group::getConditionalElementGroup(
	cmzn_field *conditionalField, bool &isEmptyGroup) const
{
	isEmptyGroup = false;
	Computed_field_element_group *otherElementGroup =
		dynamic_cast<Computed_field_element_group *>(conditionalField->core);
	if (!otherElementGroup)
	{
		Computed_field_group *group = dynamic_cast<Computed_field_group *>(conditionalField->core);
		if (group)
		{
			otherElementGroup = group->getElementGroupPrivate(this->dimension);
			if (!otherElementGroup)
				isEmptyGroup = true;
		}
	}
	if (otherElementGroup && (otherElementGroup->getSize() == 0))
		isEmptyGroup = true;
	return otherElementGroup;
}

int Computed_field_element_group::addObject(cmzn_element *object)
{
	if (!isElementCompatible(object))
		return CMZN_ERROR_ARGUMENT;
	int return_code = CMZN_OK;
	const bool handleSubelements =
		(this->getSubobjectHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL);
	if (handleSubelements)
		this->beginChange();
	if (ADD_OBJECT_TO_LIST(cmzn_element)(object, object_list))
	{
		change_detail.changeAdd();
		update();
	}
	else if (IS_OBJECT_IN_LIST(cmzn_element)(object, object_list))
		return_code = CMZN_ERROR_ALREADY_EXISTS;
	else
		return_code = CMZN_ERROR_GENERAL;
	if (handleSubelements)
	{
		if ((CMZN_OK == return_code) || (CMZN_ERROR_ALREADY_EXISTS == return_code))
		{
			int subresult = this->addSubelements(object);
			if (CMZN_OK != subresult)
				return_code = subresult;
		}
		this->endChange();
	}
	return return_code;
}

int Computed_field_element_group::addElementsConditional(cmzn_field_id conditional_field)
{
	if ((!conditional_field) || (conditional_field->manager != this->field->manager))
		return CMZN_ERROR_ARGUMENT;
	bool isEmptyGroup;
	Computed_field_element_group *otherElementGroup =
		this->getConditionalElementGroup(conditional_field, isEmptyGroup);
	if (isEmptyGroup)
		return CMZN_OK;
	int return_code = CMZN_OK;
	this->beginChange();
	cmzn_fieldcache *cache = 0;
	const int oldSize = this->getSize();
	const bool handleSubelements =
		(this->getSubobjectHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL);
	cmzn_elementiterator *iter;
	if (otherElementGroup)
		iter = otherElementGroup->createIterator();
	else
	{
		iter = cmzn_mesh_create_elementiterator(this->master_mesh);
		cache = new cmzn_fieldcache(cmzn_mesh_get_region_internal(this->master_mesh));
		if (!cache)
			return_code = CMZN_ERROR_MEMORY;
	}
	if (!iter)
		return_code = CMZN_ERROR_MEMORY;
	cmzn_element_id element = 0;
	while ((CMZN_OK == return_code) && (0 != (element = cmzn_elementiterator_next_non_access(iter))))
	{
		if (cache)
		{
			cache->setElement(element);
			if (!cmzn_field_evaluate_boolean(conditional_field, cache))
				continue;
		}
		if (!(ADD_OBJECT_TO_LIST(cmzn_element)(element, this->object_list) ||
			IS_OBJECT_IN_LIST(cmzn_element)(element, this->object_list)))
		{
			return_code = CMZN_ERROR_GENERAL;
			break;
		}
		if (handleSubelements)
		{
			return_code = this->addSubelements(element);
			if (return_code != CMZN_OK)
				break;
		}
	}
	cmzn_elementiterator_destroy(&iter);
	const int newSize = this->getSize();
	if (newSize != oldSize)
	{
		change_detail.changeAdd();
		update();
	}
	cmzn_fieldcache_destroy(&cache);
	this->endChange();
	return return_code;
}

int Computed_field_element_group::removeObject(cmzn_element *object)
{
	if (!isElementCompatible(object))
		return CMZN_ERROR_ARGUMENT;
	int return_code = CMZN_OK;
	const bool handleSubelements =
		(this->getSubobjectHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL);
	if (handleSubelements)
		this->beginChange();
	if (!IS_OBJECT_IN_LIST(cmzn_element)(object, object_list))
		return_code = CMZN_ERROR_NOT_FOUND;
	else if (REMOVE_OBJECT_FROM_LIST(cmzn_element)(object, object_list))
	{
		change_detail.changeRemove();
		update();
	}
	if (handleSubelements)
	{
		if (CMZN_OK == return_code)
			return_code = this->removeSubelements(object);
		this->endChange();
	}
	return return_code;
};

int Computed_field_element_group::removeElementsConditional(cmzn_field_id conditional_field)
{
	if ((!conditional_field) || (conditional_field->manager != this->field->manager))
		return CMZN_ERROR_ARGUMENT;
	bool isEmptyGroup;
	Computed_field_element_group *otherElementGroup =
		this->getConditionalElementGroup(conditional_field, isEmptyGroup);
	if (isEmptyGroup)
		return CMZN_OK;
	const int oldSize = this->getSize();
	if (oldSize == 0)
		return CMZN_OK;
	int return_code = CMZN_OK;
	this->beginChange();
	cmzn_fieldcache *cache = 0;
	const bool handleSubelements =
		(this->getSubobjectHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL);
	if (handleSubelements || conditional_field->dependsOnField(this->field))
	{
		// create list of elements to remove, then process it
		LIST(cmzn_element) *removeElementList = CREATE_RELATED_LIST(cmzn_element)(this->object_list);
		cmzn_elementiterator *iter;
		if (otherElementGroup && (otherElementGroup->getSize() <= oldSize))
			iter = otherElementGroup->createIterator();
		else
		{
			iter = this->createIterator();
			cache = new cmzn_fieldcache(cmzn_mesh_get_region_internal(this->master_mesh));
			if (!cache)
				return_code = CMZN_ERROR_MEMORY;
		}
		if (!iter)
			return_code = CMZN_ERROR_MEMORY;
		cmzn_element_id element = 0;
		while ((CMZN_OK == return_code) && (0 != (element = cmzn_elementiterator_next_non_access(iter))))
		{
			if (cache)
			{
				cmzn_fieldcache_set_element(cache, element);
				if (!cmzn_field_evaluate_boolean(conditional_field, cache))
					continue;
			}
			else if (!this->containsObject(element))
				continue;
			return_code = ADD_OBJECT_TO_LIST(cmzn_element)(element, removeElementList);
		}
		cmzn_elementiterator_destroy(&iter);
		if (CMZN_OK == return_code)
		{
			if (!REMOVE_OBJECTS_FROM_LIST_THAT(cmzn_element)(
				FE_element_is_in_list, (void *)removeElementList, this->object_list))
			{
				return_code = CMZN_ERROR_GENERAL;
			}
			if (handleSubelements)
			{
				iter = CREATE_LIST_ITERATOR(cmzn_element)(removeElementList);
				if (!iter)
					return_code = CMZN_ERROR_MEMORY;
				while ((0 != (element = cmzn_elementiterator_next_non_access(iter)))
					&& (CMZN_OK == return_code))
				{
					return_code = this->removeSubelements(element);
				}
				cmzn_elementiterator_destroy(&iter);
			}
		}
		DESTROY(LIST(cmzn_element))(&removeElementList);
	}
	else
	{
		cache = new cmzn_fieldcache(cmzn_mesh_get_region_internal(this->master_mesh));
		cmzn_element_field_is_true_iterator_data data = { cache, conditional_field };
		if (!REMOVE_OBJECTS_FROM_LIST_THAT(cmzn_element)(
			cmzn_element_field_is_true_conditional, (void *)&data, this->object_list))
		{
			return_code = CMZN_ERROR_GENERAL;
		}
	}
	const int newSize = this->getSize();
	if (newSize != oldSize)
	{
		change_detail.changeRemove();
		update();
	}
	cmzn_fieldcache_destroy(&cache);
	this->endChange();
	return return_code;
}

int Computed_field_element_group::clear()
{
	int return_code = CMZN_OK;
	if (0 < this->getSize())
	{
		this->beginChange();
		if (this->getSubobjectHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL)
		{
			LIST(cmzn_element) *removeElementList = this->object_list;
			this->object_list = CREATE_RELATED_LIST(cmzn_element)(removeElementList);
			if (this->object_list)
			{
				cmzn_elementiterator *iter = CREATE_LIST_ITERATOR(cmzn_element)(removeElementList);
				if (!iter)
					return_code = CMZN_ERROR_MEMORY;
				cmzn_element *element;
				while ((0 != (element = cmzn_elementiterator_next_non_access(iter)))
					&& (CMZN_OK == return_code))
				{
					return_code = this->removeSubelements(element);
				}
				cmzn_elementiterator_destroy(&iter);
				DESTROY(LIST(cmzn_element))(&removeElementList);
			}
			else
			{
				this->object_list = removeElementList;
				return_code = CMZN_ERROR_MEMORY;
			}
		}
		else
			REMOVE_ALL_OBJECTS_FROM_LIST(cmzn_element)(object_list);
		change_detail.changeRemove();
		update();
		this->endChange();
	}
	return return_code;
};

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
			if (!IS_OBJECT_IN_LIST(cmzn_element)(face, object_list))
			{
				if (ADD_OBJECT_TO_LIST(cmzn_element)(face, object_list))
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
}

int Computed_field_element_group::addSubelements(cmzn_element_id element)
{
	int return_code = CMZN_OK;
	if (1 < this->dimension)
	{
		Computed_field_element_group *faceElementGroup =
			this->ownerGroup->getElementGroupPrivate(this->dimension - 1, /*create*/true);
		if (faceElementGroup)
			return_code = faceElementGroup->addElementFacesRecursive(element);
		else
			return_code = CMZN_ERROR_GENERAL;
	}
	if (CMZN_OK == return_code)
	{
		Computed_field_node_group *nodeGroup = this->ownerGroup->getNodeGroupPrivate(CMZN_FIELD_DOMAIN_TYPE_NODES, /*create*/true);
		if (nodeGroup)
			return_code = nodeGroup->addElementNodes(element);
		else
			return_code = CMZN_ERROR_GENERAL;
	}
	return return_code;
}

int Computed_field_element_group::removeSubelements(cmzn_element_id element)
{
	int return_code = CMZN_OK;
	if (1 < this->dimension)
	{
		Computed_field_element_group *faceElementGroup =
			this->ownerGroup->getElementGroupPrivate(this->dimension - 1);
		if (faceElementGroup)
			return_code = faceElementGroup->removeElementFacesRecursive(element, *this);
	}
	if (CMZN_OK == return_code)
	{
		Computed_field_node_group *nodeGroup = this->ownerGroup->getNodeGroupPrivate(CMZN_FIELD_DOMAIN_TYPE_NODES);
		if (nodeGroup)
		{
			LIST(cmzn_node) *removeNodeList = nodeGroup->createRelatedNodeList();
			if (removeNodeList)
			{
				// add nodes from this element, but remove nodes from neighbours still in group
				return_code = cmzn_element_add_nodes_to_list(element, removeNodeList);
				int numberOfFaces = 0;
				get_FE_element_number_of_faces(element, &numberOfFaces);
				for (int face = 0; (face < numberOfFaces) && (CMZN_OK == return_code); ++face)
				{
					int adjacentElementsCount = 0;
					cmzn_element **adjacentElements = 0;
					if (adjacent_FE_element(element, face, &adjacentElementsCount, &adjacentElements))
					{
						for (int i = 0; i < adjacentElementsCount; ++i)
							if (IS_OBJECT_IN_LIST(cmzn_element)(adjacentElements[i], this->object_list))
							{
								return_code = cmzn_element_remove_nodes_from_list(adjacentElements[i], removeNodeList);
								if (CMZN_OK != return_code)
									break;
							}
						if (adjacentElementsCount)
							DEALLOCATE(adjacentElements);
					}
					else
						return_code = CMZN_ERROR_GENERAL;
				}
				if (CMZN_OK == return_code)
					return_code = nodeGroup->removeNodesInList(removeNodeList);
				DESTROY(LIST(cmzn_node))(&removeNodeList);
			}
			else
			  return_code = CMZN_ERROR_GENERAL;
		}
	}
	return return_code;
}

int Computed_field_element_group::addElementFacesRecursive(cmzn_element_id element)
{
	Computed_field_element_group *faceElementGroup = 0;
	if (1 < this->dimension)
	{
		faceElementGroup = this->ownerGroup->getElementGroupPrivate(this->dimension - 1, /*create*/true);
		if (!faceElementGroup)
			return CMZN_ERROR_GENERAL;
	}
	int return_code = CMZN_OK;
	int number_of_faces = 0;
	get_FE_element_number_of_faces(element, &number_of_faces);
	cmzn_element_id face = 0;
	int numberAdded = 0;
	for (int i = 0; i < number_of_faces; ++i)
	{
		if (get_FE_element_face(element, i, &face) && face)
		{
			if (ADD_OBJECT_TO_LIST(cmzn_element)(face, this->object_list))
				++numberAdded;
			else if (!IS_OBJECT_IN_LIST(cmzn_element)(face, this->object_list))
			{
				return_code = CMZN_ERROR_GENERAL;
				break;
			}
			if (1 < this->dimension)
			{
				return_code = faceElementGroup->addElementFacesRecursive(face);
				if (CMZN_OK != return_code)
					break;
			}
		}
	}
	if (numberAdded)
	{
		this->change_detail.changeAdd();
		this->update();
	}
	return return_code;
}

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
			if (IS_OBJECT_IN_LIST(cmzn_element)(face, object_list))
			{
				REMOVE_OBJECT_FROM_LIST(cmzn_element)(face, object_list);
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
}

int Computed_field_element_group::removeElementFacesRecursive(cmzn_element_id element, Computed_field_element_group& parentElementGroup)
{
	Computed_field_element_group *faceElementGroup = 0;
	if (1 < this->dimension)
		faceElementGroup = this->ownerGroup->getElementGroupPrivate(this->dimension - 1, /*create*/false);
	int return_code = CMZN_OK;
	int number_of_faces = 0;
	get_FE_element_number_of_faces(element, &number_of_faces);
	cmzn_element_id face = 0;
	int numberRemoved = 0;
	for (int i = 0; i < number_of_faces; ++i)
	{
		if (get_FE_element_face(element, i, &face) && face)
		{
			if (cmzn_element_has_parent_in_list(face, parentElementGroup.object_list))
				continue;
			if (REMOVE_OBJECT_FROM_LIST(cmzn_element)(face, this->object_list))
			{
				++numberRemoved;
				if (faceElementGroup)
				{
					return_code = faceElementGroup->removeElementFacesRecursive(face, *this);
					if (CMZN_OK != return_code)
						break;
				}
			}
		}
	}
	if (numberRemoved)
	{
		this->change_detail.changeRemove();
		this->update();
	}
	return return_code;
}

Computed_field_node_group *Computed_field_node_group::getConditionalNodeGroup(
	cmzn_field *conditionalField, bool &isEmptyGroup) const
{
	isEmptyGroup = false;
	Computed_field_node_group *otherNodeGroup = dynamic_cast<Computed_field_node_group *>(conditionalField->core);
	if (!otherNodeGroup)
	{
		Computed_field_group *group = dynamic_cast<Computed_field_group *>(conditionalField->core);
		if (group)
		{
			FE_nodeset *fe_nodeset = cmzn_nodeset_get_FE_nodeset_internal(this->master_nodeset);
			otherNodeGroup = group->getNodeGroupPrivate(fe_nodeset->getFieldDomainType());
			if (!otherNodeGroup)
				isEmptyGroup = true;
		}
	}
	if (otherNodeGroup && (otherNodeGroup->getSize() == 0))
		isEmptyGroup = true;
	return otherNodeGroup;
}

int Computed_field_node_group::addObject(cmzn_node *object)
{
	if (!isNodeCompatible(object))
		return CMZN_ERROR_ARGUMENT;
	if (ADD_OBJECT_TO_LIST(cmzn_node)(object, this->object_list))
	{
		change_detail.changeAdd();
		update();
		return CMZN_OK;
	}
	else if (IS_OBJECT_IN_LIST(cmzn_node)(object, this->object_list))
		return CMZN_ERROR_ALREADY_EXISTS;
	return CMZN_ERROR_GENERAL;
}

int Computed_field_node_group::addNodesConditional(cmzn_field_id conditional_field)
{
	if ((!conditional_field) || (conditional_field->manager != this->field->manager))
		return CMZN_ERROR_ARGUMENT;
	bool isEmptyGroup;
	Computed_field_node_group *otherNodeGroup =
		this->getConditionalNodeGroup(conditional_field, isEmptyGroup);
	if (isEmptyGroup)
		return CMZN_OK;
	int return_code = CMZN_OK;
	cmzn_fieldcache *cache = 0;
	const int oldSize = this->getSize();
	cmzn_nodeiterator *iter;
	if (otherNodeGroup)
		iter = otherNodeGroup->createIterator();
	else
	{
		iter = cmzn_nodeset_create_nodeiterator(this->master_nodeset);
		cache = new cmzn_fieldcache(cmzn_nodeset_get_region_internal(this->master_nodeset));
		if (!cache)
			return_code = CMZN_ERROR_MEMORY;
	}
	if (!iter)
		return_code = CMZN_ERROR_MEMORY;
	cmzn_node_id node = 0;
	while ((CMZN_OK == return_code) && (0 != (node = cmzn_nodeiterator_next_non_access(iter))))
	{
		if (cache)
		{
			cache->setNode(node);
			if (!cmzn_field_evaluate_boolean(conditional_field, cache))
				continue;
		}
		if (!(ADD_OBJECT_TO_LIST(cmzn_node)(node, this->object_list) ||
			IS_OBJECT_IN_LIST(cmzn_node)(node, this->object_list)))
		{
			return_code = CMZN_ERROR_GENERAL;
		}
	}
	cmzn_nodeiterator_destroy(&iter);
	const int newSize = NUMBER_IN_LIST(cmzn_node)(this->object_list);
	if (newSize != oldSize)
	{
		change_detail.changeAdd();
		update();
	}
	cmzn_fieldcache_destroy(&cache);
	return CMZN_OK;
}

int Computed_field_node_group::removeObject(cmzn_node *object)
{
	if (!isNodeCompatible(object))
		return CMZN_ERROR_ARGUMENT;
	if (!IS_OBJECT_IN_LIST(cmzn_node)(object, object_list))
		return CMZN_ERROR_NOT_FOUND;
	if (REMOVE_OBJECT_FROM_LIST(cmzn_node)(object, this->object_list))
	{
		change_detail.changeRemove();
		update();
		return CMZN_OK;
	}
	return CMZN_ERROR_GENERAL;
}

int Computed_field_node_group::removeNodesConditional(cmzn_field_id conditional_field)
{
	if ((!conditional_field) || (conditional_field->manager != this->field->manager))
		return CMZN_ERROR_ARGUMENT;
	bool isEmptyGroup;
	Computed_field_node_group *otherNodeGroup =
		this->getConditionalNodeGroup(conditional_field, isEmptyGroup);
	if (isEmptyGroup)
		return CMZN_OK;
	const int oldSize = NUMBER_IN_LIST(cmzn_node)(this->object_list);
	if (oldSize == 0)
		return CMZN_OK;
	int return_code = CMZN_OK;
	LIST(cmzn_node) *nodeList = this->object_list;
	if (conditional_field->dependsOnField(this->field))
	{
		// copy list, since can't query it within REMOVE_OBJECTS_FROM_LIST_THAT
		nodeList = CREATE_RELATED_LIST(cmzn_node)(this->object_list);
		if (!COPY_LIST(cmzn_node)(nodeList, this->object_list))
			return_code = CMZN_ERROR_GENERAL;
	}
	if (CMZN_OK == return_code)
	{
		if (otherNodeGroup && (otherNodeGroup->getSize() < this->getSize()))
		{
			cmzn_nodeiterator_id iter = otherNodeGroup->createIterator();
			if (iter)
			{
				cmzn_node_id node = 0;
				while (0 != (node = cmzn_nodeiterator_next_non_access(iter)))
					REMOVE_OBJECT_FROM_LIST(cmzn_node)(node, nodeList);
				cmzn_nodeiterator_destroy(&iter);
			}
			else
				return_code = CMZN_ERROR_MEMORY;
		}
		else
		{
			cmzn_fieldcache *cache = new cmzn_fieldcache(cmzn_nodeset_get_region_internal(this->master_nodeset));
			if (cache)
			{
				cmzn_node_field_is_true_iterator_data data = { cache, conditional_field };
				if (!REMOVE_OBJECTS_FROM_LIST_THAT(cmzn_node)(
					cmzn_node_field_is_true_iterator, (void *)&data, nodeList))
				{
					return_code = CMZN_ERROR_GENERAL;
				}
				cmzn_fieldcache_destroy(&cache);
			}
			else
				return_code = CMZN_ERROR_MEMORY;
		}
		if ((nodeList != this->object_list) && (CMZN_OK == return_code))
		{
			DESTROY(LIST(cmzn_node))(&this->object_list);
			this->object_list = nodeList;
		}
		const int newSize = NUMBER_IN_LIST(cmzn_node)(this->object_list);
		if (newSize != oldSize)
		{
			change_detail.changeRemove();
			update();
		}
	}
	if (nodeList != this->object_list)
		DESTROY(LIST(cmzn_node))(&nodeList);
	return return_code;
}

int Computed_field_node_group::removeNodesInList(LIST(cmzn_node) *removeNodeList)
{
	if (!removeNodeList)
		return CMZN_ERROR_ARGUMENT;
	int return_code = CMZN_OK;
	const int oldSize = NUMBER_IN_LIST(cmzn_node)(this->object_list);
	if (!REMOVE_OBJECTS_FROM_LIST_THAT(cmzn_node)(FE_node_is_in_list, (void *)(removeNodeList), this->object_list))
		return_code = CMZN_ERROR_GENERAL;
	const int newSize = NUMBER_IN_LIST(cmzn_node)(this->object_list);
	if (newSize < oldSize)
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
	FE_nodeset *fe_nodeset = cmzn_nodeset_get_FE_nodeset_internal(this->master_nodeset);
	if (fe_nodeset->getFieldDomainType() != CMZN_FIELD_DOMAIN_TYPE_NODES)
		return CMZN_ERROR_ARGUMENT;
	const int oldSize = NUMBER_IN_LIST(cmzn_node)(this->object_list);
	int return_code = cmzn_element_add_nodes_to_list(element, this->object_list);
	const int newSize = NUMBER_IN_LIST(cmzn_node)(this->object_list);
	if (newSize > oldSize)
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
	FE_nodeset *fe_nodeset = cmzn_nodeset_get_FE_nodeset_internal(this->master_nodeset);
	if (fe_nodeset->getFieldDomainType() != CMZN_FIELD_DOMAIN_TYPE_NODES)
		return CMZN_ERROR_ARGUMENT;
	const int oldSize = NUMBER_IN_LIST(cmzn_node)(this->object_list);
	int return_code = cmzn_element_remove_nodes_from_list(element, this->object_list);
	const int newSize = NUMBER_IN_LIST(cmzn_node)(this->object_list);
	if (newSize < oldSize)
	{
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

