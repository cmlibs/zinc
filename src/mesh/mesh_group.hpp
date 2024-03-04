/**
 * FILE : mesh_group.hpp
 *
 * Interface to mesh group implementation.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "cmlibs/zinc/fieldgroup.h"
#include "datastore/labelsgroup.hpp"
#include "mesh/mesh.hpp"


class Computed_field_group;

struct cmzn_mesh_group : public cmzn_mesh, public FE_domain_mapper
{
	friend cmzn_field_group;

	// group is accessed once per external access of this object, i.e.
	// not for first access which is held by group itself.
	// This ensures group always exists while external handles exist.
	cmzn_field_group* group;
	DsLabelsGroup* labelsGroup;

	cmzn_mesh_group(FE_mesh* feMeshIn, cmzn_field_group* groupIn);

	~cmzn_mesh_group();

	/** Increment access count of owner group to ensure it stays around */
	virtual void ownerAccess();

	/** Decrement access count of owner group */
	virtual void ownerDeaccess();

	/* Record that group has changed by object add, with client notification */
	void changeAdd();

	/* Record that group has changed by object remove, with client notification */
	void changeRemove();

	/* Record that group has changed by object remove, but do not notify clients.
	 * Only used when objects removed because destroyed in parent domain. */
	void changeRemoveNoNotify();

	inline void invalidateIterators()
	{
		this->labelsGroup->invalidateLabelIterators();
	}

	/** If the conditionalField is a group, get the nodeset group for this master nodeset.
	 * @param  isEmptyNodesetGroup  Set to true if field is a group containing no nodes
	 * for this master nodeset, otherwise false.
	 * @return  Non-accessed mesh group, or nullptr if none found or not a group */
	const cmzn_mesh_group* getConditionalMeshGroup(
		const cmzn_field* conditionalField, bool& isEmptyMeshGroup) const;

	/** get group core object, guaranteed to exist */
	Computed_field_group* getGroupCore() const;

	/** @return  Subelement handling mode of owning group, controlling whether faces/lines/nodes
	 * are added or removed from related mesh/nodeset groups */
	cmzn_field_group_subelement_handling_mode getSubelementHandlingMode() const;

	/** Adds faces of parent element to element group, and their faces to related face mesh groups
	 * recursively. Only call between begin/end change. */
	int addElementFacesRecursive(cmzn_mesh_group& parentMeshGroup, DsLabelIndex parentIndex);

	/** Removes faces of parent element from element group, and their faces from related mesh groups
	 * recursively. Only call between begin/end change. */
	int removeElementFacesRecursive(cmzn_mesh_group& parentMeshGroup, DsLabelIndex parentIndex);

	/** Adds faces and nodes of element to related subobject groups.
	 * Only call between begin/end change. */
	int addSubelements(cmzn_element* element);

	/** Removes faces and nodes of element from related subobject groups, but
	 * only if not used by peers. Only call between begin/end change. */
	int removeSubelements(cmzn_element* element);

	/** Add faces and nodes of elements with indexes in labels group from related subobject
	 * groups. Only call between begin/end change. */
	int addSubelementsList(const DsLabelsGroup& addlabelsGroup);

	/** Removes faces and nodes of elements with indexes in labels group from related subobject
	 * groups, but only if not used by peers. Only call between begin/end change. */
	int removeSubelementsList(const DsLabelsGroup& removedlabelsGroup);

public:

	static void deaccess(cmzn_mesh_group*& mesh_group)
	{
		cmzn_mesh::deaccess(reinterpret_cast<cmzn_mesh*&>(mesh_group));
	}

	/** Create element and ensure it's in this group.
	 * @return  Accessed new element or nullptr if failed. */
	virtual cmzn_element* createElement(int identifier, cmzn_elementtemplate* elementtemplate);

	virtual bool containsElement(cmzn_element* element) const
	{
		return cmzn_mesh::containsElement(element) && this->labelsGroup->hasIndex(element->getIndex());
	}

	bool containsIndex(DsLabelIndex elementIndex) const
	{
		return this->labelsGroup->hasIndex(elementIndex);
	}

	virtual cmzn_elementiterator* createElementiterator() const
	{
		return this->feMesh->createElementiterator(this->labelsGroup);
	}

	virtual int destroyAllElements()
	{
		return this->feMesh->destroyElementsInGroup(*(this->labelsGroup));
	}

	/** @return  Non-accessed element, or nullptr if not found */
	virtual cmzn_element* findElementByIdentifier(int identifier) const;

	const DsLabelsGroup* getLabelsGroup() const
	{
		return this->labelsGroup;
	}

	/** @return  Allocated name as GROUP_NAME.MESH_NAME, or nullptr if failed */
	virtual char* getName() const;

	virtual int getSize() const
	{
		return this->labelsGroup->getSize();
	}

	/** @return  Non-accessed group owning this mesh group, guaranteed to exist */
	cmzn_field_group* getFieldGroup() const
	{
		return this->group;
	}

	/** @return  True if mesh group has recorded changes in membership */
	virtual bool hasMembershipChanges() const;

	int addElement(cmzn_element* element);

	int addElementsConditional(cmzn_field* conditionalField);

	/** add objects with identifier ranges from first to last, inclusive
	 * NOTE: does not handle subelements.
	 */
	int addElementsInIdentifierRange(DsLabelIdentifier first, DsLabelIdentifier last)
	{
		return this->labelsGroup->addIndexesInIdentifierRange(first, last);
	}

	int addElementsInLabelsGroup(const DsLabelsGroup& addLabelsGroup);

	int removeAllElements();

	int removeElement(cmzn_element* element);

	int removeElementsConditional(cmzn_field* conditionalField);

	int removeElementsInLabelsGroup(const DsLabelsGroup& removeLabelsGroup);

	/** @param element  Element from parent master mesh */
	int addElementFaces(cmzn_element* parentElement);

	/** @param element  Element from parent master mesh */
	int removeElementFaces(cmzn_element* parentElement);

	void destroyedAllObjects();

	void destroyedObject(DsLabelIndex destroyedIndex);

	void destroyedObjectGroup(const DsLabelsGroup& destroyedLabelsGroup);

};

/**
 * Ensures all faces of the supplied element are in this mesh_group.
 * Candidate for external API.
 *
 * @param mesh_group  The mesh group to add faces to. Must be of dimension 1
 * less than that of element and be a subgroup for the master mesh expected to
 * own the element's faces.
 * @param element  The element whose faces are to be added.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
int cmzn_mesh_group_add_element_faces(cmzn_mesh_group_id mesh_group,
	cmzn_element_id element);

/**
 * Ensures all faces of the supplied element are not in this mesh_group.
 * Candidate for external API.
 *
 * @param mesh_group  The mesh group to remove faces from. Must be of dimension
 * 1 less than that of element and be a subgroup for the master mesh expected
 * to own the element's faces.
 * @param element  The element whose faces are to be removed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
int cmzn_mesh_group_remove_element_faces(cmzn_mesh_group_id mesh_group,
	cmzn_element_id element);
