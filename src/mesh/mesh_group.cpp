/**
 * FILE : mesh_group.cpp
 *
 * Mesh group implementation.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "computed_field/computed_field_group.hpp"
#include "general/message.h"
#include "general/mystring.h"
#include "mesh/mesh_group.hpp"
#include "mesh/nodeset_group.hpp"
#include "finite_element/finite_element_region.h"


cmzn_mesh_group::cmzn_mesh_group(FE_mesh* feMeshIn, cmzn_field_group* groupIn) :
	cmzn_mesh(feMeshIn),
	group(groupIn),
	labelsGroup(feMeshIn->createLabelsGroup())
{
	feMeshIn->addMapper(this);
}

cmzn_mesh_group::~cmzn_mesh_group()
{
	this->feMesh->removeMapper(this);
	cmzn::Deaccess(this->labelsGroup);
}

void cmzn_mesh_group::ownerAccess()
{
	cmzn_field_group_base_cast(this->group)->access();
}

void cmzn_mesh_group::ownerDeaccess()
{
	// not clearing owner
	cmzn_field* field = cmzn_field_group_base_cast(this->group);
	cmzn_field::deaccess(field);
}

void cmzn_mesh_group::changeAdd()
{
	this->invalidateIterators();
	this->getGroupCore()->changeAddLocal();
}

void cmzn_mesh_group::changeRemove()
{
	this->invalidateIterators();
	this->getGroupCore()->changeRemoveLocal();
}

void cmzn_mesh_group::changeRemoveNoNotify()
{
	this->invalidateIterators();
	this->getGroupCore()->changeRemoveLocalNoNotify();
}

const cmzn_mesh_group* cmzn_mesh_group::getConditionalMeshGroup(
	const cmzn_field* conditionalField, bool& isEmptyMeshGroup) const
{
	const Computed_field_group* groupCore = cmzn_field_get_group_core(conditionalField);
	if (!groupCore)
	{
		isEmptyMeshGroup = false;
		return nullptr;
	}
	cmzn_mesh_group* conditionalMeshGroup = groupCore->getLocalMeshGroup(this->feMesh);
	isEmptyMeshGroup = (!conditionalMeshGroup) || (conditionalMeshGroup->labelsGroup->getSize() == 0);
	return conditionalMeshGroup;
}

Computed_field_group* cmzn_mesh_group::getGroupCore() const
{
	return cmzn_field_group_core_cast(this->group);
}

cmzn_field_group_subelement_handling_mode cmzn_mesh_group::getSubelementHandlingMode() const
{
	return this->getGroupCore()->getSubelementHandlingMode();
}

int cmzn_mesh_group::addElementFacesRecursive(cmzn_mesh_group& parentMeshGroup, DsLabelIndex parentIndex)
{
	if (this->feMesh->getParentMesh() != parentMeshGroup.feMesh)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	cmzn_mesh_group* faceMeshGroup = nullptr; // for adding faces of faces
	Computed_field_group* groupCore = this->getGroupCore();
	FE_mesh* faceFeMesh = this->feMesh->getFaceMesh();
	if (faceFeMesh)
	{
		faceMeshGroup = groupCore->getOrCreateMeshGroup(faceFeMesh);
		if (!faceMeshGroup)
		{
			return CMZN_ERROR_GENERAL;
		}
	}
	const FE_mesh::ElementShapeFaces* elementShapeFaces =
		parentMeshGroup.feMesh->getElementShapeFacesConst(parentIndex);
	if (!elementShapeFaces)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const int faceCount = elementShapeFaces->getFaceCount();
	if (0 == faceCount)
	{
		return CMZN_OK;
	}
	const DsLabelIndex* faces = elementShapeFaces->getElementFaces(parentIndex);
	if (!faces)
	{
		return CMZN_OK;
	}
	int return_code = CMZN_OK;
	int numberAdded = 0;
	for (int i = 0; i < faceCount; ++i)
	{
		if (faces[i] >= 0)
		{
			const int subResult = this->labelsGroup->setIndex(faces[i], true);
			if (CMZN_OK == subResult)
			{
				++numberAdded;
			}
			else if (CMZN_ERROR_ALREADY_EXISTS != subResult)
			{
				return_code = subResult;
				break;
			}
			if (faceMeshGroup)
			{
				return_code = faceMeshGroup->addElementFacesRecursive(*this, faces[i]);
				if (CMZN_OK != return_code)
				{
					break;
				}
			}
		}
	}
	if (numberAdded)
	{
		this->changeAdd();
	}
	return return_code;
}

int cmzn_mesh_group::removeElementFacesRecursive(cmzn_mesh_group& parentMeshGroup, DsLabelIndex parentIndex)
{
	if (this->feMesh->getParentMesh() != parentMeshGroup.feMesh)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	cmzn_mesh_group* faceMeshGroup = nullptr; // for removing faces of faces
	Computed_field_group* groupCore = this->getGroupCore();
	FE_mesh* faceFeMesh = this->feMesh->getFaceMesh();
	if (faceFeMesh)
	{
		faceMeshGroup = groupCore->getLocalMeshGroup(faceFeMesh);
	}
	const FE_mesh::ElementShapeFaces* elementShapeFaces =
		parentMeshGroup.feMesh->getElementShapeFacesConst(parentIndex);
	if (!elementShapeFaces)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const int faceCount = elementShapeFaces->getFaceCount();
	if (0 == faceCount)
	{
		return CMZN_OK;
	}
	const DsLabelIndex* faces = elementShapeFaces->getElementFaces(parentIndex);
	if (!faces)
	{
		return CMZN_OK;
	}
	int return_code = CMZN_OK;
	int numberRemoved = 0;
	for (int i = 0; i < faceCount; ++i)
	{
		if (faces[i] >= 0)
		{
			const DsLabelIndex* parents;
			const int parentsCount = this->feMesh->getElementParents(faces[i], parents);
			bool keepFace = false;
			for (int p = 0; p < parentsCount; ++p)
			{
				if (parentMeshGroup.labelsGroup->hasIndex(parents[p]))
				{
					keepFace = true;
					break;
				}
			}
			if (keepFace)
			{
				continue;
			}
			const int result = this->labelsGroup->setIndex(faces[i], false);
			if (CMZN_OK == result)
			{
				++numberRemoved;
				if (faceMeshGroup)
				{
					return_code = faceMeshGroup->removeElementFacesRecursive(*this, faces[i]);
					if (CMZN_OK != return_code)
					{
						break;
					}
				}
			}
			else if (CMZN_ERROR_NOT_FOUND != result)
			{
				return_code = result;
				break;
			}
		}
	}
	if (numberRemoved)
	{
		this->changeRemove();
	}
	return return_code;
}

int cmzn_mesh_group::addSubelements(cmzn_element* element)
{
	int return_code = CMZN_OK;
	Computed_field_group* groupCore = this->getGroupCore();
	FE_mesh* faceFeMesh = this->feMesh->getFaceMesh();
	if (faceFeMesh)
	{
		cmzn_mesh_group* faceMeshGroup = groupCore->getOrCreateMeshGroup(faceFeMesh);
		if (faceMeshGroup)
		{
			return_code = faceMeshGroup->addElementFacesRecursive(*this, element->getIndex());
		}
		else
		{
			return_code = CMZN_ERROR_GENERAL;
		}
	}
	FE_nodeset* feNodeset = this->feMesh->getNodeset();
	if ((CMZN_OK == return_code) && (feNodeset))
	{
		cmzn_nodeset_group* nodesetGroup = groupCore->getOrCreateNodesetGroup(feNodeset);
		if (nodesetGroup)
		{
			return_code = nodesetGroup->addElementNodes(element);
		}
		else
		{
			return_code = CMZN_ERROR_GENERAL;
		}
	}
	return return_code;
}

int cmzn_mesh_group::removeSubelements(cmzn_element* element)
{
	int return_code = CMZN_OK;
	Computed_field_group* groupCore = this->getGroupCore();
	const DsLabelIndex elementIndex = element->getIndex();
	FE_mesh* faceFeMesh = this->feMesh->getFaceMesh();
	if (faceFeMesh)
	{
		cmzn_mesh_group* faceMeshGroup = groupCore->getLocalMeshGroup(faceFeMesh);
		if (faceMeshGroup)
		{
			return_code = faceMeshGroup->removeElementFacesRecursive(*this, element->getIndex());
		}
	}
	FE_nodeset* feNodeset = this->feMesh->getNodeset();
	if ((CMZN_OK == return_code) && (feNodeset))
	{
		cmzn_nodeset_group* nodesetGroup = groupCore->getLocalNodesetGroup(feNodeset);
		if (nodesetGroup)
		{
			DsLabelsGroup* removeNodeLabelsGroup = nodesetGroup->getFeNodeset()->createLabelsGroup();
			if (removeNodeLabelsGroup)
			{
				// add nodes from this element, but remove nodes from neighbours still in group
				return_code = cmzn_element_add_nodes_to_labels_group(element, *removeNodeLabelsGroup);
				if (CMZN_OK == return_code)
				{
					if (faceFeMesh)
					{
						const FE_mesh::ElementShapeFaces* elementShapeFaces = this->feMesh->getElementShapeFacesConst(elementIndex);
						const DsLabelIndex* faces;
						if ((elementShapeFaces) && (faces = elementShapeFaces->getElementFaces(elementIndex)))
						{
							const int faceCount = elementShapeFaces->getFaceCount();
							for (int f = 0; (f < faceCount) && (return_code == CMZN_OK); ++f)
							{
								const DsLabelIndex* parents;
								const int parentsCount = faceFeMesh->getElementParents(faces[f], parents);
								for (int p = 0; p < parentsCount; ++p)
								{
									if ((parents[p] != elementIndex) && this->labelsGroup->hasIndex(parents[p]))
									{
										return_code = cmzn_element_remove_nodes_from_labels_group(this->feMesh->getElement(parents[p]), *removeNodeLabelsGroup);
										if (CMZN_OK != return_code)
										{
											break;
										}
									}
								}
							}
						}
					}
					else if (1 == this->feMesh->getDimension())
					{
						// remove nodes used by remaining elements = expensive
						cmzn_elementiterator* iter = this->createElementiterator();
						if (!iter)
						{
							return_code = CMZN_ERROR_MEMORY;
						}
						cmzn_element* tmpElement;
						while ((tmpElement = iter->nextElement()) && (CMZN_OK == return_code))
						{
							return_code = cmzn_element_remove_nodes_from_labels_group(tmpElement, *removeNodeLabelsGroup);
						}
						cmzn::Deaccess(iter);
					}
					if (CMZN_OK == return_code)
					{
						return_code = nodesetGroup->removeNodesInLabelsGroup(*removeNodeLabelsGroup);
					}
				}
				cmzn::Deaccess(removeNodeLabelsGroup);
			}
			else
			{
				return_code = CMZN_ERROR_MEMORY;
			}
		}
	}
	return return_code;
}

int cmzn_mesh_group::addSubelementsList(const DsLabelsGroup& addlabelsGroup)
{
	int return_code = CMZN_OK;
	Computed_field_group* groupCore = this->getGroupCore();
	cmzn_element* element;
	FE_mesh* faceFeMesh = this->feMesh->getFaceMesh();
	cmzn_mesh_group* faceMeshGroup = nullptr;
	if (faceFeMesh)
	{
		faceMeshGroup = groupCore->getOrCreateMeshGroup(faceFeMesh);
		if (!faceMeshGroup)
		{
			return CMZN_ERROR_GENERAL;
		}
	}
	FE_nodeset* feNodeset = this->feMesh->getNodeset();
	cmzn_nodeset_group* nodesetGroup = groupCore->getOrCreateNodesetGroup(feNodeset);
	if (!nodesetGroup)
	{
		return CMZN_ERROR_GENERAL;
	}
	cmzn_elementiterator* iter = this->feMesh->createElementiterator(&addlabelsGroup);
	if (!iter)
	{
		return_code = CMZN_ERROR_MEMORY;
	}
	while ((CMZN_OK == return_code) && (element = iter->nextElement()))
	{
		if (faceMeshGroup)
		{
			return_code = faceMeshGroup->addElementFacesRecursive(*this, element->getIndex());
		}
		if (CMZN_OK == return_code)
		{
			return_code = nodesetGroup->addElementNodes(element);
		}
	}
	cmzn::Deaccess(iter);
	return return_code;
}

int cmzn_mesh_group::removeSubelementsList(const DsLabelsGroup& removedlabelsGroup)
{
	int return_code = CMZN_OK;
	Computed_field_group* groupCore = this->getGroupCore();
	cmzn_element* element;
	FE_mesh* faceFeMesh = this->feMesh->getFaceMesh();
	if (faceFeMesh)
	{
		cmzn_mesh_group* faceMeshGroup = groupCore->getLocalMeshGroup(faceFeMesh);
		if (faceMeshGroup)
		{
			cmzn_elementiterator* iter = this->feMesh->createElementiterator(&removedlabelsGroup);
			if (!iter)
			{
				return_code = CMZN_ERROR_MEMORY;
			}
			while ((CMZN_OK == return_code) && (element = iter->nextElement()))
			{
				return_code = faceMeshGroup->removeElementFacesRecursive(*this, element->getIndex());
			}
			cmzn::Deaccess(iter);
		}
	}
	FE_nodeset* feNodeset = this->feMesh->getNodeset();
	if ((CMZN_OK == return_code) && (feNodeset))
	{
		cmzn_nodeset_group* nodesetGroup = groupCore->getLocalNodesetGroup(feNodeset);
		if (nodesetGroup)
		{
			DsLabelsGroup* removeNodeLabelsGroup = nodesetGroup->getFeNodeset()->createLabelsGroup();
			if (removeNodeLabelsGroup)
			{
				// fill list with nodes from removed elements
				cmzn_elementiterator* iter = this->feMesh->createElementiterator(&removedlabelsGroup);
				if (!iter)
				{
					return_code = CMZN_ERROR_MEMORY;
				}
				while ((CMZN_OK == return_code) && (element = iter->nextElement()))
				{
					return_code = cmzn_element_add_nodes_to_labels_group(element, *removeNodeLabelsGroup);
				}
				cmzn::Deaccess(iter);

				// remove nodes used by remaining elements = expensive
				iter = this->createElementiterator();
				if (!iter)
				{
					return_code = CMZN_ERROR_MEMORY;
				}
				while ((CMZN_OK == return_code) && (element = iter->nextElement()))
				{
					return_code = cmzn_element_remove_nodes_from_labels_group(element, *removeNodeLabelsGroup);
				}
				cmzn::Deaccess(iter);

				if (CMZN_OK == return_code)
				{
					return_code = nodesetGroup->removeNodesInLabelsGroup(*removeNodeLabelsGroup);
				}
				cmzn::Deaccess(removeNodeLabelsGroup);
			}
			else
			{
				return_code = CMZN_ERROR_MEMORY;
			}
		}
	}
	return return_code;
}

cmzn_element* cmzn_mesh_group::createElement(int identifier, cmzn_elementtemplate* elementtemplate)
{
	cmzn_region* region = this->getRegion();
	region->beginChangeFields();
	cmzn_element* element = cmzn_mesh::createElement(identifier, elementtemplate);
	if (element)
	{
		this->addElement(element);
	}
	region->endChangeFields();
	return element;
}

cmzn_element* cmzn_mesh_group::findElementByIdentifier(int identifier) const
{
	const DsLabelIndex index = this->feMesh->findIndexByIdentifier(identifier);
	if (this->labelsGroup->hasIndex(index))
	{
		return this->feMesh->getElement(index);
	}
	return nullptr;
}

char* cmzn_mesh_group::getName() const
{
	char* name = nullptr;
	int error = 0;
	if (this->group)
	{
		append_string(&name, cmzn_field_group_base_cast(this->group)->getName(), &error);
	}
	append_string(&name, ".", &error);
	append_string(&name, this->feMesh->getName(), &error);
	return name;
}

bool cmzn_mesh_group::hasMembershipChanges() const
{
	return (this->group) ? (MANAGER_CHANGE_NONE(cmzn_field) !=
		cmzn_field_group_base_cast(this->group)->manager_change_status) : false;
}

int cmzn_mesh_group::addAdjacentElements(int sharedDimension)
{
	const int thisDimension = this->getDimension();
	if ((sharedDimension < -thisDimension) ||
		(sharedDimension > (thisDimension - 1)))
	{
		display_message(ERROR_MESSAGE, "MeshGroup addAdjacentElements.  Invalid shared dimension");
		return CMZN_ERROR_ARGUMENT;
	}
	if (thisDimension == 1)
	{
		display_message(ERROR_MESSAGE, "MeshGroup addAdjacentElements.  Not implemented for 1-D mesh group");
		return CMZN_ERROR_NOT_IMPLEMENTED;
	}
	const FE_mesh* faceMesh = this->feMesh->getFaceMesh();
	const FE_mesh* lineMesh = (faceMesh) ? faceMesh->getFaceMesh() : nullptr;
	const int absoluteSharedDimension = (sharedDimension < 0) ? thisDimension + sharedDimension : sharedDimension;
	const bool sharePoints = absoluteSharedDimension == 0;
	const bool addLineNeighbours = (lineMesh) && (absoluteSharedDimension < (thisDimension - 1));
	const bool addFaceNeighbours = !addLineNeighbours;
	int result = CMZN_OK;
	// make new labels group for adjacent elements, later added to this group
	DsLabelsGroup* newLabelsGroup = this->feMesh->createLabelsGroup();
	DsLabelIterator* labelIterator = this->labelsGroup->createLabelIterator();
	if ((!newLabelsGroup) || (!labelIterator))
	{
		result = CMZN_ERROR_MEMORY;
	}
	DsLabelsGroup* nodeLabelsGroup = nullptr;
	if (sharePoints)
	{
		// make a list of nodes used by elements in current group to find adjacent elements sharing them
		nodeLabelsGroup = this->feMesh->getNodeset()->createLabelsGroup();
		cmzn_elementiterator* elementIter = this->createElementiterator();
		if ((nodeLabelsGroup) && (elementIter))
		{
			cmzn_element* element = nullptr;
			while ((result == CMZN_OK) && (element = elementIter->nextElement()))
			{
				result = cmzn_element_add_nodes_to_labels_group(element, *nodeLabelsGroup);
			}
		}
		else
		{
			result = CMZN_ERROR_MEMORY;
		}
		cmzn::Deaccess(elementIter);
	}
	DsLabelIndex elementIndex;
	while ((result == CMZN_OK) && ((elementIndex = labelIterator->nextIndex()) != DS_LABEL_INDEX_INVALID))
	{
		const FE_mesh::ElementShapeFaces* elementShapeFaces =
			this->feMesh->getElementShapeFacesConst(elementIndex);
		if (!elementShapeFaces)
		{
			continue;
		}
		const int facesCount = elementShapeFaces->getFaceCount();
		const DsLabelIndex* faces = elementShapeFaces->getElementFaces(elementIndex);
		if (!faces)
		{
			continue;
		}
		for (int f = 0; (f < facesCount) && (result == CMZN_OK); ++f)
		{
			const DsLabelIndex faceIndex = faces[f];
			if (faceIndex >= 0)
			{
				if (addFaceNeighbours)
				{
					// add parents of all faces
					const DsLabelIndex* faceParents = nullptr;
					const int faceParentsCount = faceMesh->getElementParents(faceIndex, faceParents);
					for (int fp = 0; fp < faceParentsCount; ++fp)
					{
						const int addResult = newLabelsGroup->setIndex(faceParents[fp], true);
						if ((addResult != CMZN_OK) && (addResult != CMZN_ERROR_ALREADY_EXISTS))
						{
							result = addResult;
							break;
						}
					}
				}
				else
				{
					// add parents' parents of all faces' faces
					const FE_mesh::ElementShapeFaces* faceElementShapeFaces =
						faceMesh->getElementShapeFacesConst(faceIndex);
					if (!faceElementShapeFaces)
					{
						continue;
					}
					const int linesCount = faceElementShapeFaces->getFaceCount();
					const DsLabelIndex* lines = faceElementShapeFaces->getElementFaces(faceIndex);
					if (!lines)
					{
						continue;
					}
					for (int l = 0; (l < linesCount) && (result == CMZN_OK); ++l)
					{
						const DsLabelIndex lineIndex = lines[l];
						if (lineIndex >= 0)
						{
							const DsLabelIndex* lineParents = nullptr;
							const int lineParentsCount = lineMesh->getElementParents(lineIndex, lineParents);
							for (int lp = 0; (lp < lineParentsCount) && (result == CMZN_OK); ++lp)
							{
								const DsLabelIndex* faceParents = nullptr;
								const int faceParentsCount = faceMesh->getElementParents(lineParents[lp], faceParents);
								for (int fp = 0; fp < faceParentsCount; ++fp)
								{
									const int addResult = newLabelsGroup->setIndex(faceParents[fp], true);
									if ((addResult != CMZN_OK) && (addResult != CMZN_ERROR_ALREADY_EXISTS))
									{
										result = addResult;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	cmzn::Deaccess(labelIterator);
	if (result == CMZN_OK)
	{
		if (sharePoints)
		{
			// traverse faces graph to find any element using nodes from nodeLabelsGroup
			// but stop traversing at any element not sharing nodes
			std::vector<DsLabelIdentifier> checkElementIndexes;  // used like a LIFO stack
			checkElementIndexes.reserve(newLabelsGroup->getSize());
			labelIterator = newLabelsGroup->createLabelIterator();
			if (labelIterator)
			{
				while ((elementIndex = labelIterator->nextIndex()) != DS_LABEL_INDEX_INVALID)
				{
					checkElementIndexes.push_back(elementIndex);
				}
				cmzn::Deaccess(labelIterator);
			}
			else
			{
				result = CMZN_ERROR_MEMORY;
			}
			while ((result == CMZN_OK) && (checkElementIndexes.size() > 0))
			{
				elementIndex = checkElementIndexes.back();
				checkElementIndexes.pop_back();
				const FE_mesh::ElementShapeFaces* elementShapeFaces =
					this->feMesh->getElementShapeFacesConst(elementIndex);
				if (!elementShapeFaces)
				{
					continue;
				}
				const int facesCount = elementShapeFaces->getFaceCount();
				const DsLabelIndex* faces = elementShapeFaces->getElementFaces(elementIndex);
				if (!faces)
				{
					continue;
				}
				for (int f = 0; (f < facesCount) && (result == CMZN_OK); ++f)
				{
					const DsLabelIndex faceIndex = faces[f];
					if (faceIndex >= 0)
					{
						const DsLabelIndex* faceParents = nullptr;
						const int faceParentCount = faceMesh->getElementParents(faceIndex, faceParents);
						for (int fp = 0; fp < faceParentCount; ++fp)
						{
							DsLabelIndex nearbyElementIndex = faceParents[fp];
							if (newLabelsGroup->hasIndex(nearbyElementIndex))
							{
								continue;
							}
							// note following function does not inherit nodes from parent elements
							if (this->feMesh->elementHasNodeInGroup(nearbyElementIndex, *nodeLabelsGroup))
							{
								result = newLabelsGroup->setIndex(nearbyElementIndex, true);
								if (result != CMZN_OK)
								{
									break;
								}
								checkElementIndexes.push_back(nearbyElementIndex);
							}
						}
					}
				}
			}
		}
		if (result == CMZN_OK)
		{
			newLabelsGroup->removeGroup(*this->labelsGroup);
			result = this->addElementsInLabelsGroup(*newLabelsGroup);
		}
	}
	cmzn::Deaccess(nodeLabelsGroup);
	cmzn::Deaccess(newLabelsGroup);
	return result;
}

int cmzn_mesh_group::addElement(cmzn_element* element)
{
	cmzn_region* region = this->getRegion();
	if ((!region) || (!this->feMesh->containsElement(element)))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const bool handleSubelements =
		this->getSubelementHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL;
	if (handleSubelements)
	{
		region->beginChangeFields();
	}
	int return_code = this->labelsGroup->setIndex(element->getIndex(), true);
	if (CMZN_OK == return_code)
	{
		this->changeAdd();
	}
	if (handleSubelements)
	{
		if ((CMZN_OK == return_code) || (CMZN_ERROR_ALREADY_EXISTS == return_code))
		{
			const int subresult = this->addSubelements(element);
			if (CMZN_OK != subresult)
			{
				return_code = subresult;
			}
		}
		region->endChangeFields();
	}
	return return_code;
}

int cmzn_mesh_group::addElementsConditional(cmzn_field* conditionalField)
{
	cmzn_region* region = this->getRegion();
	if ((!region) || (!conditionalField) || (conditionalField->getRegion() != region))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	bool isEmptyMeshGroup = false;
	const cmzn_mesh_group* otherMeshGroup = this->getConditionalMeshGroup(conditionalField, isEmptyMeshGroup);
	if (isEmptyMeshGroup)
	{
		return CMZN_OK;
	}
	if (otherMeshGroup)
	{
		return this->addElementsInLabelsGroup(*(otherMeshGroup->labelsGroup));
	}
	const bool handleSubelements =
		this->getSubelementHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL;
	if (handleSubelements)
	{
		region->beginChangeFields();
	}
	cmzn_elementiterator* iter = this->feMesh->createElementiterator();
	cmzn_fieldcache* fieldcache = cmzn_fieldcache::create(region);
	int return_code = CMZN_OK;
	const int oldSize = this->labelsGroup->getSize();
	if ((iter) && (fieldcache))
	{
		cmzn_element* element = nullptr;
		while ((element = iter->nextElement()))
		{
			fieldcache->setElement(element);
			if (!cmzn_field_evaluate_boolean(conditionalField, fieldcache))
			{
				continue;
			}
			const int result = this->labelsGroup->setIndex(element->getIndex(), true);
			if ((result != CMZN_OK) && (result != CMZN_ERROR_ALREADY_EXISTS))
			{
				return_code = result;
				break;
			}
			if (handleSubelements)
			{
				int subresult = this->addSubelements(element);
				if (subresult != CMZN_OK)
				{
					return_code = subresult;
					break;
				}
			}
		}
	}
	else
	{
		return_code = CMZN_ERROR_MEMORY;
	}
	cmzn_fieldcache::deaccess(fieldcache);
	cmzn::Deaccess(iter);
	if (this->labelsGroup->getSize() > oldSize)
	{
		this->changeAdd();
	}
	if (handleSubelements)
	{
		region->endChangeFields();
	}
	return return_code;
}

int cmzn_mesh_group::addElementsInLabelsGroup(const DsLabelsGroup& addLabelsGroup)
{
	cmzn_region* region = this->getRegion();
	if (!region)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const bool handleSubelements =
		this->getSubelementHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL;
	if (handleSubelements)
	{
		region->beginChangeFields();
	}
	const int oldSize = this->labelsGroup->getSize();
	this->labelsGroup->addGroup(addLabelsGroup);
	if (handleSubelements)
	{
		this->addSubelementsList(addLabelsGroup);
	}
	if (this->labelsGroup->getSize() > oldSize)
	{
		this->changeAdd();
	}
	if (handleSubelements)
	{
		region->endChangeFields();
	}
	return CMZN_OK;
}

int cmzn_mesh_group::removeAllElements()
{
	cmzn_region* region = this->getRegion();
	if (!region)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	if (this->getSize() == 0)
	{
		return CMZN_OK;
	}
	int return_code = CMZN_OK;
	const bool handleSubelements =
		this->getSubelementHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL;
	if (handleSubelements)
	{
		region->beginChangeFields();
		DsLabelsGroup* tmpLabelsGroup = DsLabelsGroup::create(this->labelsGroup->getLabels());
		tmpLabelsGroup->swap(*this->labelsGroup);
		int subResult = this->removeSubelementsList(*tmpLabelsGroup);
		if (subResult != CMZN_OK)
		{
			return_code = subResult;
		}
		cmzn::Deaccess(tmpLabelsGroup);
	}
	else
	{
		this->labelsGroup->clear();
	}
	this->changeRemove();
	if (handleSubelements)
	{
		region->endChangeFields();
	}
	return return_code;
};

int cmzn_mesh_group::removeElement(cmzn_element* element)
{
	cmzn_region* region = this->getRegion();
	if ((!region) || (!this->feMesh->containsElement(element)))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const bool handleSubelements =
		this->getSubelementHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL;
	if (handleSubelements)
	{
		region->beginChangeFields();
	}
	int return_code = this->labelsGroup->setIndex(element->getIndex(), false);
	if (CMZN_OK == return_code)
	{
		this->changeRemove();
	}
	if (handleSubelements)
	{
		if ((CMZN_OK == return_code) || (CMZN_ERROR_NOT_FOUND == return_code))
		{
			int subresult = this->removeSubelements(element);
			if (CMZN_OK != subresult)
			{
				return_code = subresult;
			}
		}
		region->endChangeFields();
	}
	return return_code;
};

int cmzn_mesh_group::removeElementsConditional(cmzn_field* conditionalField)
{
	cmzn_region* region = this->getRegion();
	if ((!region) || (!conditionalField) || (conditionalField->getRegion() != region))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	bool isEmptyMeshGroup = false;
	const cmzn_mesh_group* otherMeshGroup = this->getConditionalMeshGroup(conditionalField, isEmptyMeshGroup);
	if (isEmptyMeshGroup)
	{
		return CMZN_OK;
	}
	if (otherMeshGroup)
	{
		return this->removeElementsInLabelsGroup(*(otherMeshGroup->labelsGroup));
	}
	int return_code = CMZN_OK;
	const bool handleSubelements =
		this->getSubelementHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL;
	if (handleSubelements)
	{
		region->beginChangeFields();
	}
	cmzn_elementiterator* iter = this->feMesh->createElementiterator();
	cmzn_fieldcache* fieldcache = cmzn_fieldcache::create(region);
	const int oldSize = this->labelsGroup->getSize();
	if ((iter) && (fieldcache))
	{
		cmzn_element* element = nullptr;
		while ((element = iter->nextElement()))
		{
			fieldcache->setElement(element);
			if (!cmzn_field_evaluate_boolean(conditionalField, fieldcache))
			{
				continue;
			}
			const int result = this->labelsGroup->setIndex(element->getIndex(), false);
			if ((result != CMZN_OK) && (result != CMZN_ERROR_NOT_FOUND))
			{
				return_code = result;
				break;
			}
			if (handleSubelements)
			{
				int subresult = this->removeSubelements(element);
				if (subresult != CMZN_OK)
				{
					return_code = subresult;
					break;
				}
			}
		}
	}
	else
	{
		return_code = CMZN_ERROR_MEMORY;
	}
	cmzn_fieldcache::deaccess(fieldcache);
	cmzn::Deaccess(iter);
	if (this->labelsGroup->getSize() < oldSize)
	{
		this->changeRemove();
	}
	if (handleSubelements)
	{
		region->endChangeFields();
	}
	return return_code;
}

int cmzn_mesh_group::removeElementsInLabelsGroup(const DsLabelsGroup& removeLabelsGroup)
{
	// handle subelements code below doesn't work if this label group is used
	if (&removeLabelsGroup == this->labelsGroup)
	{
		return this->removeAllElements();
	}
	cmzn_region* region = this->getRegion();
	if (!region)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const bool handleSubelements =
		this->getSubelementHandlingMode() == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL;
	if (handleSubelements)
	{
		region->beginChangeFields();
	}
	const int oldSize = this->labelsGroup->getSize();
	this->labelsGroup->removeGroup(removeLabelsGroup);
	if (handleSubelements)
	{
		this->removeSubelementsList(removeLabelsGroup);
	}
	if (this->labelsGroup->getSize() < oldSize)
	{
		this->changeRemove();
	}
	if (handleSubelements)
	{
		region->endChangeFields();
	}
	return CMZN_OK;
}

int cmzn_mesh_group::addElementFaces(cmzn_element* parentElement)
{
	if ((!parentElement->getMesh()) || (parentElement->getMesh() != this->feMesh->getParentMesh()))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const FE_mesh::ElementShapeFaces* elementShapeFaces =
		this->feMesh->getParentMesh()->getElementShapeFacesConst(parentElement->getIndex());
	if (!elementShapeFaces)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const int faceCount = elementShapeFaces->getFaceCount();
	if (0 == faceCount)
	{
		return CMZN_OK;
	}
	const DsLabelIndex* faces = elementShapeFaces->getElementFaces(parentElement->getIndex());
	if (!faces)
	{
		return CMZN_OK;
	}
	int return_code = CMZN_OK;
	cmzn_region* region = this->getRegion();
	region->beginChangeFields();
	for (int i = 0; i < faceCount; ++i)
	{
		if (faces[i] >= 0)
		{
			cmzn_element* face = this->feMesh->getElement(faces[i]);
			if (face)
			{
				const int subResult = this->addElement(face);
				if ((subResult != CMZN_OK) && (subResult != CMZN_ERROR_ALREADY_EXISTS))
				{
					return_code = subResult;
					break;
				}
			}
		}
	}
	region->endChangeFields();
	return return_code;
}

int cmzn_mesh_group::removeElementFaces(cmzn_element_id parentElement)
{
	if ((!parentElement->getMesh()) || (parentElement->getMesh() != this->feMesh->getParentMesh()))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const FE_mesh::ElementShapeFaces* elementShapeFaces =
		this->feMesh->getParentMesh()->getElementShapeFacesConst(parentElement->getIndex());
	if (!elementShapeFaces)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const int faceCount = elementShapeFaces->getFaceCount();
	if (0 == faceCount)
	{
		return CMZN_OK;
	}
	const DsLabelIndex* faces = elementShapeFaces->getElementFaces(parentElement->getIndex());
	if (!faces)
	{
		return CMZN_OK;
	}
	int return_code = CMZN_OK;
	cmzn_region* region = this->getRegion();
	region->beginChangeFields();
	for (int i = 0; i < faceCount; ++i)
	{
		if (faces[i] >= 0)
		{
			cmzn_element* face = this->feMesh->getElement(faces[i]);
			if (face)
			{
				const int result = this->removeElement(face);
				if ((result != CMZN_OK) && (result != CMZN_ERROR_NOT_FOUND))
				{
					return_code = result;
					break;
				}
			}
		}
	}
	region->endChangeFields();
	return return_code;
}

void cmzn_mesh_group::destroyedAllObjects()
{
	if (this->getSize() > 0)
	{
		this->labelsGroup->clear();
		this->changeRemoveNoNotify();
	}
}

void cmzn_mesh_group::destroyedObject(DsLabelIndex destroyedIndex)
{
	if (this->labelsGroup->hasIndex(destroyedIndex))
	{
		this->labelsGroup->setIndex(destroyedIndex, false);
		this->changeRemoveNoNotify();
	}
}

void cmzn_mesh_group::destroyedObjectGroup(const DsLabelsGroup& destroyedLabelsGroup)
{
	const int oldSize = this->labelsGroup->getSize();
	this->labelsGroup->removeGroup(destroyedLabelsGroup);
	if (this->labelsGroup->getSize() < oldSize)
	{
		this->changeRemoveNoNotify();
	}
}

/*
Global functions
----------------
*/

cmzn_mesh_group_id cmzn_mesh_cast_group(cmzn_mesh_id mesh)
{
	cmzn_mesh_group* meshGroup = dynamic_cast<cmzn_mesh_group*>(mesh);
	if (meshGroup)
	{
		mesh->access();
		return meshGroup;
	}
	return nullptr;
}

int cmzn_mesh_group_destroy(cmzn_mesh_group_id* mesh_group_address)
{
	if (mesh_group_address)
	{
		cmzn_mesh_group::deaccess(*mesh_group_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_add_adjacent_elements(
	cmzn_mesh_group_id mesh_group, int shared_dimension)
{
	if (mesh_group)
	{
		return mesh_group->addAdjacentElements(shared_dimension);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_add_element(cmzn_mesh_group_id mesh_group, cmzn_element_id element)
{
	if (mesh_group)
	{
		return mesh_group->addElement(element);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_add_elements_conditional(cmzn_mesh_group_id mesh_group,
	cmzn_field_id conditional_field)
{
	if (mesh_group)
	{
		return mesh_group->addElementsConditional(conditional_field);
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_field_group_id cmzn_mesh_group_get_field_group(
	cmzn_mesh_group_id mesh_group)
{
	if (mesh_group)
	{
		cmzn_field_group* fieldGroup = mesh_group->getFieldGroup();
		if (fieldGroup)
		{
			cmzn_field_group_base_cast(fieldGroup)->access();
			return fieldGroup;
		}
	}
	return nullptr;
}

int cmzn_mesh_group_remove_all_elements(cmzn_mesh_group_id mesh_group)
{
	if (mesh_group)
	{
		return mesh_group->removeAllElements();
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_remove_element(cmzn_mesh_group_id mesh_group, cmzn_element_id element)
{
	if (mesh_group)
	{
		return mesh_group->removeElement(element);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_remove_elements_conditional(cmzn_mesh_group_id mesh_group,
	cmzn_field_id conditional_field)
{
	if (mesh_group)
	{
		return mesh_group->removeElementsConditional(conditional_field);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_add_element_faces(cmzn_mesh_group_id mesh_group, cmzn_element_id element)
{
	if (mesh_group)
	{
		return mesh_group->addElementFaces(element);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_remove_element_faces(cmzn_mesh_group_id mesh_group, cmzn_element_id element)
{
	if (mesh_group)
	{
		return mesh_group->removeElementFaces(element);
	}
	return CMZN_ERROR_ARGUMENT;
}
