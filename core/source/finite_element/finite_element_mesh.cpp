/**
 * FILE : finite_element_mesh.cpp
 *
 * Class defining a domain consisting of a set of finite elements.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_private.h"
#include "finite_element/finite_element_region_private.h"
#include "general/object.h"
#include "general/debug.h"
#include "general/message.h"
#include "general/mystring.h"

/*
Module types
------------
*/

cmzn_mesh_scale_factor_set::~cmzn_mesh_scale_factor_set()
{
	DEALLOCATE(name);
}

cmzn_mesh_scale_factor_set::cmzn_mesh_scale_factor_set(FE_mesh *fe_meshIn, const char *nameIn) :
	fe_mesh(fe_meshIn),
	name(duplicate_string(nameIn)),
	access_count(1)
{
}

int cmzn_mesh_scale_factor_set::setName(const char *nameIn)
{
	if (nameIn)
	{
		cmzn_mesh_scale_factor_set *existingSet = this->fe_mesh->find_scale_factor_set_by_name(nameIn);
		if (existingSet)
		{
			bool noChange = (existingSet == this);
			cmzn_mesh_scale_factor_set::deaccess(existingSet);
			if (noChange)
			{
				return CMZN_OK;
			}
		}
		else
		{
			// Note: assumes FE_mesh does not store sets in a map
			// Hence can change name in object
			DEALLOCATE(this->name);
			this->name = duplicate_string(nameIn);
			return CMZN_OK;
		}
	}
	return CMZN_ERROR_ARGUMENT;
}

FE_element_template::FE_element_template(FE_mesh *mesh_in, struct FE_element_field_info *element_field_info, FE_element_shape *element_shape_in) :
	cmzn::RefCounted(),
	mesh(mesh_in->access()),
	element_shape(ACCESS(FE_element_shape)(element_shape_in)),
	template_element(create_template_FE_element(element_field_info))
{
}

FE_element_template::FE_element_template(FE_mesh *mesh_in, struct FE_element *element) :
	cmzn::RefCounted(),
	mesh(mesh_in->access()),
	element_shape(ACCESS(FE_element_shape)(get_FE_element_shape(element))),
	template_element(create_FE_element_from_template(DS_LABEL_INDEX_INVALID, element))
{
}

FE_element_template::~FE_element_template()
{
	FE_mesh::deaccess(this->mesh);
	DEACCESS(FE_element)(&(this->template_element));
	if (this->element_shape)
		DEACCESS(FE_element_shape)(&(this->element_shape));
}

FE_mesh::FE_mesh(FE_region *fe_regionIn, int dimensionIn) :
	fe_region(fe_regionIn),
	dimension(dimensionIn),
	elementShapeFacesCount(0),
	elementShapeFacesArray(0),
	elementShapeMap(/*default*/),
	elementList(CREATE(LIST(FE_element)())),
	element_field_info_list(CREATE(LIST(FE_element_field_info))()),
	parentMesh(0),
	faceMesh(0),
	changeLog(0),
	last_fe_element_field_info(0),
	element_type_node_sequence_list(0),
	definingFaces(false),
	access_count(1)
{
}

FE_mesh::~FE_mesh()
{
	cmzn::Deaccess(this->changeLog);
	this->last_fe_element_field_info = 0;
	// must invalidate elements since client or nodal element:xi fields may still hold them
	// BUT! Need to leave elements that have been merged into another region
	cmzn_elementiterator *iter = this->createElementiterator();
	cmzn_element *element = 0;
	while ((0 != (element = cmzn_elementiterator_next_non_access(iter))))
	{
		if (FE_element_get_FE_mesh(element) == this)
			FE_element_invalidate(element);
	}
	cmzn_elementiterator_destroy(&iter);
	DESTROY(LIST(FE_element))(&(this->elementList));
	// remove pointers to this FE_mesh as destroying
	FOR_EACH_OBJECT_IN_LIST(FE_element_field_info)(
		FE_element_field_info_clear_FE_mesh, (void *)NULL,
		this->element_field_info_list);
	DESTROY(LIST(FE_element_field_info))(&(this->element_field_info_list));

	const size_t size = this->scale_factor_sets.size();
	for (size_t i = 0; i < size; ++i)
	{
		cmzn_mesh_scale_factor_set *scale_factor_set = this->scale_factor_sets[i];
		cmzn_mesh_scale_factor_set::deaccess(scale_factor_set);
	}
	for (unsigned int i = 0; i < this->elementShapeFacesCount; ++i)
		delete this->elementShapeFacesArray[i];
	delete[] this->elementShapeFacesArray;
}

/**
 * Call this to mark element with the supplied change.
 * Notifies change to clients of FE_region.
 */
void FE_mesh::elementChange(DsLabelIndex index, int change)
{
	if (this->fe_region && this->changeLog)
	{
		this->changeLog->setIndexChange(index, change);
		this->fe_region->update();
	}
}

/**
 * Call this to mark element with the supplied change, logging field changes
 * from the field_info_element in the fe_region.
 * Notifies change to clients of FE_region.
 * When an element is added or removed, the same element is used for <element> and
 * <field_info_element>. For changes to the contents of <element>, <field_info_element>
 * should contain the changed fields, consistent with merging it into <element>.
 */
void FE_mesh::elementChange(FE_element *element, int change, FE_element *field_info_element)
{
	if (this->fe_region && this->changeLog)
	{
		this->changeLog->setIndexChange(get_FE_element_index(element), change);
		// for efficiency, the following marks field changes only if field info changes
		FE_element_log_FE_field_changes(field_info_element, fe_region->fe_field_changes, /*recurseParents*/true);
		this->fe_region->update();
	}
}

/**
 * Records change to element affecting the supplied fields.
 */
void FE_mesh::elementFieldListChange(FE_element *element, int change,
	LIST(FE_field) *changed_fe_field_list)
{
	if (this->fe_region && this->changeLog)
	{
		this->changeLog->setIndexChange(get_FE_element_index(element), change);
		FOR_EACH_OBJECT_IN_LIST(FE_field)(FE_field_log_FE_field_change,
			(void *)this->fe_region->fe_field_changes, changed_fe_field_list);
		this->fe_region->update();
	}
}

/**
 * Call this instead of elementChange when only the identifier has changed.
 */
void FE_mesh::elementIdentifierChange(FE_element *element)
{
	if (this->fe_region && this->changeLog)
	{
		this->changeLog->setIndexChange(get_FE_element_index(element), DS_LABEL_CHANGE_TYPE_IDENTIFIER);
		this->fe_region->update();
	}
}

/**
 * Call this instead of elementChange when exactly one field, <fe_field> of
 * <element> has changed.
 */
void FE_mesh::elementFieldChange(FE_element *element, FE_field *fe_field)
{
	if (this->fe_region && this->changeLog)
	{
		this->changeLog->setIndexChange(get_FE_element_index(element), DS_LABEL_CHANGE_TYPE_RELATED);
		CHANGE_LOG_OBJECT_CHANGE(FE_field)(this->fe_region->fe_field_changes,
			fe_field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
		this->fe_region->update();
	}
}

void FE_mesh::elementAddedChange(FE_element *element)
{
	if (this->fe_region && this->changeLog)
	{
		this->changeLog->setIndexChange(get_FE_element_index(element), DS_LABEL_CHANGE_TYPE_ADD);
		// for efficiency, the following marks field changes only if field info changes
		FE_element_log_FE_field_changes(element, fe_region->fe_field_changes, /*recurseParents*/true);
		this->fe_region->update();
	}
}

void FE_mesh::elementRemovedChange(FE_element *element)
{
	if (this->fe_region)
	{
		this->changeLog->setIndexChange(get_FE_element_index(element), DS_LABEL_CHANGE_TYPE_REMOVE);
		// for efficiency, the following marks field changes only if field info changes
		FE_element_log_FE_field_changes(element, fe_region->fe_field_changes, /*recurseParents*/true);
		this->fe_region->update();
	}
}

// Only to be called by FE_region_clear, or when all elements removed to reset data structures
void FE_mesh::clear()
{
	FE_region_begin_change(this->fe_region);
	cmzn_elementiterator *iter = this->createElementiterator();
	cmzn_element *element = 0;
	while ((0 != (element = cmzn_elementiterator_next_non_access(iter))))
	{
		this->elementRemovedChange(element);
		FE_element_invalidate(element);
	}
	cmzn_elementiterator_destroy(&iter);
	REMOVE_ALL_OBJECTS_FROM_LIST(FE_element)(this->elementList);

	for (unsigned int i = 0; i < this->elementShapeFacesCount; ++i)
		delete this->elementShapeFacesArray[i];
	delete[] this->elementShapeFacesArray;
	this->elementShapeFacesCount = 0;
	this->elementShapeFacesArray = 0;
	this->elementShapeMap.clear();

	this->labels.clear();
	FE_region_end_change(this->fe_region);
}

void FE_mesh::createChangeLog()
{
	cmzn::Deaccess(this->changeLog);
	this->changeLog = DsLabelsChangeLog::create(&this->labels);
	if (!this->changeLog)
		display_message(ERROR_MESSAGE, "FE_mesh::createChangeLog.  Failed to create changes object");
	this->last_fe_element_field_info = 0;
}

DsLabelsChangeLog *FE_mesh::extractChangeLog()
{
	DsLabelsChangeLog *returnChangeLog = cmzn::Access(this->changeLog);
	this->createChangeLog();
	return returnChangeLog;
}

/**
 * Set the element shape for the element at index.
 * @param index  The index of the element in the mesh
 * @param element_shape  The shape to set; must be of same dimension as mesh.
 * @return  ElementShapeFaces for element with index, or 0 if failed.
 */
FE_mesh::ElementShapeFaces *FE_mesh::setElementShape(DsLabelIndex index, FE_element_shape *element_shape)
{
	if ((index < 0) || (get_FE_element_shape_dimension(element_shape) != this->dimension))
		return 0;
	ElementShapeFaces *currentElementShapeFaces = this->getElementShapeFaces(index);
	if (currentElementShapeFaces)
	{
		if (currentElementShapeFaces->getShape() == element_shape)
			return currentElementShapeFaces;
		FE_element *element = this->getElement(index); // temporary that element is needed
		if (element)
			clear_FE_element_faces(element, FE_element_shape_get_number_of_faces(currentElementShapeFaces->getShape()));
	}
	unsigned int shapeIndex = 0;
	while ((shapeIndex < this->elementShapeFacesCount) &&
			(this->elementShapeFacesArray[shapeIndex]->getShape() != element_shape))
		++shapeIndex;
	if (shapeIndex == this->elementShapeFacesCount)
	{
		if (1 == this->elementShapeFacesCount)
		{
			// must now store per-element shape
			if (!this->elementShapeMap.setValues(0, this->labels.getIndexSize() - 1, 0))
			{
				display_message(ERROR_MESSAGE, "FE_mesh::setElementShape  Failed to make per-element shape map");
				return 0;
			}
		}
		ElementShapeFaces *newElementShapeFaces = new ElementShapeFaces(&this->labels, element_shape);
		if (!newElementShapeFaces)
			return 0;
		ElementShapeFaces **tempElementShapeFaces = new ElementShapeFaces*[this->elementShapeFacesCount + 1];
		if (!tempElementShapeFaces)
		{
			delete newElementShapeFaces;
			return 0;
		}
		for (unsigned int i = 0; i < this->elementShapeFacesCount; ++i)
			tempElementShapeFaces[i] = this->elementShapeFacesArray[i];
		tempElementShapeFaces[shapeIndex] = newElementShapeFaces;
		delete[] this->elementShapeFacesArray;
		this->elementShapeFacesArray = tempElementShapeFaces;
		++this->elementShapeFacesCount;
	}
	if ((this->elementShapeFacesCount > (char)1) &&
			(!this->elementShapeMap.setValue(index, shapeIndex)))
		return 0;
	// No change message here, assume done by callers
	//this->elementChange(index, DS_LABEL_CHANGE_TYPE_DEFINITION);
	return this->elementShapeFacesArray[shapeIndex];
}

bool FE_mesh::setElementShapeFromTemplate(DsLabelIndex index, FE_element_template &element_template)
{
	// GRC make more efficient by caching shapeIndex
	return (0 != this->setElementShape(index, element_template.get_element_shape()));
}

/**
 * Returns a struct FE_element_field_info for the supplied <fe_element_field_list>.
 * The mesh maintains an internal list of these structures so they can be
 * shared between elements.
 * If <element_field_list> is omitted, an empty list is assumed.
 */
struct FE_element_field_info *FE_mesh::get_FE_element_field_info(
	struct LIST(FE_element_field) *fe_element_field_list)
{
	struct FE_element_field_info *fe_element_field_info = 0;
	struct FE_element_field_info *existing_fe_element_field_info;
	if (fe_element_field_list)
	{
		existing_fe_element_field_info =
			FIRST_OBJECT_IN_LIST_THAT(FE_element_field_info)(
				FE_element_field_info_has_matching_FE_element_field_list,
				(void *)fe_element_field_list,
				this->element_field_info_list);
	}
	else
	{
		existing_fe_element_field_info =
			FIRST_OBJECT_IN_LIST_THAT(FE_element_field_info)(
				FE_element_field_info_has_empty_FE_element_field_list, (void *)NULL,
				this->element_field_info_list);
	}
	if (existing_fe_element_field_info)
	{
		fe_element_field_info = existing_fe_element_field_info;
	}
	else
	{
		fe_element_field_info =
			CREATE(FE_element_field_info)(this, fe_element_field_list);
		if (fe_element_field_info)
		{
			if (!ADD_OBJECT_TO_LIST(FE_element_field_info)(fe_element_field_info,
				this->element_field_info_list))
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::get_FE_element_field_info.  Could not add to FE_region");
				DESTROY(FE_element_field_info)(&fe_element_field_info);
				fe_element_field_info = (struct FE_element_field_info *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_mesh::get_FE_element_field_info.  "
				"Could not create element field information");
		}
	}
	return (fe_element_field_info);
}

/**
 * Returns a clone of <fe_element_field_info> that belongs to this mesh and
 * uses equivalent FE_fields, FE_time_sequences and scale factor sets from it.
 * Used to merge elements from other FE_regions into.
 * It is an error if an equivalent/same name FE_field is not found.
 */
struct FE_element_field_info *FE_mesh::clone_FE_element_field_info(
	struct FE_element_field_info *fe_element_field_info)
{
	FE_element_field_info *clone_fe_element_field_info = 0;
	if (fe_element_field_info)
	{
		struct LIST(FE_element_field) *fe_element_field_list =
			FE_element_field_list_clone_for_FE_region(
				FE_element_field_info_get_element_field_list(fe_element_field_info), this);
		if (fe_element_field_list)
		{
			clone_fe_element_field_info = this->get_FE_element_field_info(fe_element_field_list);
			DESTROY(LIST(FE_element_field))(&fe_element_field_list);
		}
		if (!clone_fe_element_field_info)
		{
			display_message(ERROR_MESSAGE,
				"FE_mesh::clone_FE_element_field_info.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_mesh::clone_FE_element_field_info.  Invalid argument(s)");
	}
	return (clone_fe_element_field_info);
}

/**
 * Provided EXCLUSIVELY for the use of DEACCESS and REACCESS functions.
 * Called when the access_count of <fe_element_field_info> drops to 1 so that
 * the mesh can destroy FE_element_field_info not in use.
 */
int FE_mesh::remove_FE_element_field_info(
	struct FE_element_field_info *fe_element_field_info)
{
	if (fe_element_field_info == this->last_fe_element_field_info)
		this->last_fe_element_field_info = 0;
	return REMOVE_OBJECT_FROM_LIST(FE_element_field_info)(
		fe_element_field_info, this->element_field_info_list);
}

struct FE_element_field_info_check_field_node_value_labels_data
{
	struct FE_field *field;
	struct FE_region *target_fe_region;
};

/** @param data_void  Pointer to FE_element_field_info_check_field_node_value_labels_data */
int FE_element_field_info_check_field_node_value_labels_iterator(
	struct FE_element_field_info *element_field_info, void *data_void)
{
	FE_element_field_info_check_field_node_value_labels_data *data =
		static_cast<FE_element_field_info_check_field_node_value_labels_data *>(data_void);
	return FE_element_field_info_check_field_node_value_labels(
		element_field_info, data->field, data->target_fe_region);
}

/**
 * Checks element fields to ensure parameters are mapped by value/derivative
 * type and version, adding if necessary. Fails it not possible to add.
 * @return  Status code.
 */
int FE_mesh::check_field_element_node_value_labels(FE_field *field,
	FE_region *target_fe_region)
{
	if (!field)
		return CMZN_ERROR_ARGUMENT;
	FE_element_field_info_check_field_node_value_labels_data data = { field, target_fe_region };
	if (0 == FOR_EACH_OBJECT_IN_LIST(FE_element_field_info)(
		FE_element_field_info_check_field_node_value_labels_iterator, (void *)&data, this->element_field_info_list))
	{
		char *name;
		GET_NAME(FE_field)(field, &name);
		display_message(ERROR_MESSAGE, "FE_mesh::check_field_element_node_value_labels.  "
			"Field %s element maps cannot be converted to use node value labels", name);
		DEALLOCATE(name);
		return CMZN_ERROR_GENERAL;
	}
	return CMZN_OK;
}

/**
 * Find handle to the mesh scale factor set of the given name, if any.
 * Scale factors are stored in elements under a scale factor set.
 *
 * @param name  The name of the scale factor set. 
 * @return  Handle to the scale factor set, or 0 if none.
 * Up to caller to destroy returned handle.
 */
cmzn_mesh_scale_factor_set *FE_mesh::find_scale_factor_set_by_name(
	const char *name)
{
	if (name)
	{
		const size_t size = this->scale_factor_sets.size();
		for (size_t i = 0; i < size; ++i)
		{
			if (0 == strcmp(this->scale_factor_sets[i]->getName(), name))
			{
				return this->scale_factor_sets[i]->access();
			}
		}
	}
	return 0;
}

/**
 * Create a mesh scale factor set. The new set is given a unique name in the
 * mesh, which can be changed.
 * Scale factors are stored in elements under a scale factor set.
 *
 * @return  Handle to the new scale factor set, or 0 on failure. Up to caller
 * to destroy the returned handle.
 */
cmzn_mesh_scale_factor_set *FE_mesh::create_scale_factor_set()
{
	char tempName[10];
	for (int i = static_cast<int>(this->scale_factor_sets.size()) + 1; ; ++i)
	{
		sprintf(tempName, "temp%d", i);
		cmzn_mesh_scale_factor_set *existingSet = this->find_scale_factor_set_by_name(tempName);
		if (existingSet)
		{
			cmzn_mesh_scale_factor_set::deaccess(existingSet);
		}
		else
		{
			cmzn_mesh_scale_factor_set *scale_factor_set = cmzn_mesh_scale_factor_set::create(this, tempName);
			this->scale_factor_sets.push_back(scale_factor_set);
			return scale_factor_set->access();
		}
	}
	return 0;
}

bool FE_mesh::is_FE_field_in_use(struct FE_field *fe_field)
{
	if (FIRST_OBJECT_IN_LIST_THAT(FE_element_field_info)(
		FE_element_field_info_has_FE_field, (void *)fe_field,
		this->element_field_info_list))
	{
		/* since elements may still exist in the change_log,
		   must now check that no remaining elements use fe_field */
		/* for now, if there are elements then fe_field is in use */
		if (0 < NUMBER_IN_LIST(FE_element)(this->elementList))
			return true;
	}
	return false;
}

/**
 * Returns the number of elements in the mesh
 */
int FE_mesh::get_number_of_FE_elements()
{
	return NUMBER_IN_LIST(FE_element)(this->elementList);
}

void FE_mesh::list_btree_statistics()
{
	if (NUMBER_IN_LIST(FE_element)(this->elementList))
	{
		display_message(INFORMATION_MESSAGE, "%d-D elements:\n",dimension);
		FE_element_list_write_btree_statistics(this->elementList);
	}
}

bool FE_mesh::containsElement(FE_element *element)
{
	return (0 != IS_OBJECT_IN_LIST(FE_element)(element, this->elementList));
}

/**
 * Create an element iterator object for iterating through the elements of the
 * mesh. The iterator initially points at the position before the first element.
 * @return  Handle to element_iterator at position before first, or NULL if error.
 */
cmzn_elementiterator_id FE_mesh::createElementiterator()
{
	return CREATE_LIST_ITERATOR(FE_element)(this->elementList);
}

struct FE_element *FE_mesh::get_first_FE_element_that(
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function, void *user_data_void)
{
	return FIRST_OBJECT_IN_LIST_THAT(FE_element)(conditional_function,
		user_data_void, this->elementList);
}

int FE_mesh::for_each_FE_element(
	LIST_ITERATOR_FUNCTION(FE_element) iterator_function, void *user_data_void)
{
	return FOR_EACH_OBJECT_IN_LIST(FE_element)(iterator_function,
		user_data_void, this->elementList);
}

struct LIST(FE_element) *FE_mesh::createRelatedElementList()
{
	return CREATE_RELATED_LIST(FE_element)(this->elementList);
}

int FE_mesh::change_FE_element_identifier(struct FE_element *element, int new_identifier)
{
	if ((FE_element_get_FE_mesh(element) == this) && (new_identifier >= 0))
	{
		if (IS_OBJECT_IN_LIST(FE_element)(element, this->elementList))
		{
			// this temporarily removes the object from all indexed lists
			if (LIST_BEGIN_IDENTIFIER_CHANGE(FE_element, identifier)(
				this->elementList, element))
			{
				const DsLabelIndex index = get_FE_element_index(element);
				int return_code = this->labels.setIdentifier(index, new_identifier);
				LIST_END_IDENTIFIER_CHANGE(FE_element, identifier)(this->elementList);
				if (return_code == CMZN_OK)
					this->elementIdentifierChange(element);
				else if (return_code == CMZN_ERROR_ALREADY_EXISTS)
					display_message(ERROR_MESSAGE, "FE_mesh::change_FE_element_identifier.  Identifier %d is already used in %d-D mesh",
						new_identifier, this->dimension);
				else
					display_message(ERROR_MESSAGE, "FE_mesh::change_FE_element_identifier.  Failed to set label identifier");
				return return_code;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::change_FE_element_identifier.   Could not safely change identifier in indexed lists");
			}
			return CMZN_ERROR_GENERAL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_mesh::change_FE_element_identifier.  element is not in this mesh");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_mesh::change_FE_element_identifier.  Invalid argument(s)");
	}
	return CMZN_ERROR_ARGUMENT;
}

/** Creates a template that is a copy of the existing element */
FE_element_template *FE_mesh::create_FE_element_template(FE_element *element)
{
	if (FE_element_get_FE_mesh(element) != this)
		return 0;
	FE_element_template *element_template = new FE_element_template(this, element);
	return element_template;
}

/** @param element_shape  Element shape, must match mesh dimension */
FE_element_template *FE_mesh::create_FE_element_template(FE_element_shape *element_shape)
{
	if (get_FE_element_shape_dimension(element_shape) != this->dimension)
		return 0;
	FE_element_template *element_template = new FE_element_template(this,
		this->get_FE_element_field_info((struct LIST(FE_element_field) *)NULL), element_shape);
	return element_template;
}

/**
 * Convenience function returning an existing element with the identifier
 * from the mesh, or if none found or if identifier is -1, a new element with
 * with the identifier (or the first available identifier if -1), and with the
 * supplied shape or if none, unspecified shape of the same dimension as the
 * mesh.
 * It is expected that the calling function has wrapped calls to this function
 * with FE_region_begin/end_change.
 * @return  Accessed element, or 0 on error.
 */
struct FE_element *FE_mesh::get_or_create_FE_element_with_identifier(int identifier,
	struct FE_element_shape *element_shape)
{
	struct FE_element *element = 0;
	if ((-1 <= identifier) && ((!element_shape) ||
		(get_FE_element_shape_dimension(element_shape) == this->dimension)))
	{
		if (identifier >= 0)
			element = this->findElementByIdentifier(identifier);
		if (element)
		{
			ACCESS(FE_element)(element);
		}
		else
		{
			FE_element_template *element_template = this->create_FE_element_template(
				element_shape ? element_shape : CREATE(FE_element_shape)(dimension, /*type*/(int *)0, fe_region));
			element = this->create_FE_element(identifier, element_template);
			cmzn::Deaccess(element_template);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_mesh::get_or_create_FE_element_with_identifier.  "
			"Invalid argument(s)");
	}
	return (element);
}

/**
 * Checks the element_template is compatible with mesh & that there is no
 * existing element of supplied identifier, then creates element of that
 * identifier as a copy of element_template and adds it to the mesh.
 *
 * @param identifier  Non-negative integer identifier of new element, or -1 to
 * automatically generate (starting at 1). Fails if supplied identifier already
 * used by an existing element.
 * @return  Accessed element, or 0 on error.
 */
FE_element *FE_mesh::create_FE_element(int identifier, FE_element_template *element_template)
{
	struct FE_element *new_element = 0;
	if ((-1 <= identifier) && element_template)
	{
		if (element_template->mesh == this)
		{
			DsLabelIndex index = (identifier < 0) ? this->labels.createLabel() : this->labels.createLabel(identifier);
			if (index >= 0)
			{
				new_element = ::create_FE_element_from_template(index, element_template->get_template_element());
				if (this->setElementShapeFromTemplate(index, *element_template) &&
					ADD_OBJECT_TO_LIST(FE_element)(new_element, this->elementList))
				{
					this->elementAddedChange(new_element);
				}
				else
				{
					display_message(ERROR_MESSAGE, "FE_mesh::create_FE_element.  Failed to add element to list.");
					DEACCESS(FE_element)(&new_element);
					this->labels.removeLabel(index);
				}
			}
			else
			{
				if (this->labels.findLabelByIdentifier(identifier) >= 0)
					display_message(ERROR_MESSAGE, "FE_mesh::create_FE_element.  Identifier %d is already used in %d-D mesh.",
						identifier, this->dimension);
				else
					display_message(ERROR_MESSAGE, "FE_mesh::create_FE_element.  Could not create label");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_mesh::create_FE_element.  Element template is incompatible with mesh");
		}
	}
	return (new_element);
}

/**
 * Merge fields and other data from source element into destination.
 * Both elements must be of this mesh.
 * @return  Status code.
 */
int FE_mesh::merge_FE_element_existing(struct FE_element *destination, struct FE_element *source)
{
	if (destination && source)
	{
		if (destination == source)
			return CMZN_OK; // nothing to do; happens when adding faces
		if ((FE_element_get_FE_mesh(destination) == this) &&
			(FE_element_get_FE_mesh(source) == this))
		{
			int return_code = CMZN_OK;
			struct LIST(FE_field) *changed_fe_field_list = CREATE(LIST(FE_field))();
			if (changed_fe_field_list)
			{
				if (::merge_FE_element(destination, source, changed_fe_field_list))
				{
					this->elementFieldListChange(destination,
						DS_LABEL_CHANGE_TYPE_DEFINITION | DS_LABEL_CHANGE_TYPE_RELATED, changed_fe_field_list);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_mesh::merge_FE_element_existing.  Could not merge into %d-D element %d",
						this->dimension, cmzn_element_get_identifier(destination));
					return_code = CMZN_ERROR_GENERAL;
				}
				DESTROY(LIST(FE_field))(&changed_fe_field_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::merge_FE_element_existing.  Could not create field list");
				return_code = CMZN_ERROR_GENERAL;
			}
			return return_code;
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element_existing.  "
				"Source and/or destination elements are not from mesh");
		}
	}
	return CMZN_ERROR_ARGUMENT;
}

int FE_mesh::merge_FE_element_template(struct FE_element *destination, FE_element_template *fe_element_template)
{
	if (fe_element_template)
	{
		const DsLabelIndex index = get_FE_element_index(destination);
		const bool shapeChange = (!FE_element_shape_is_unspecified(fe_element_template->get_element_shape())) &&
			(this->getElementShape(index) != fe_element_template->get_element_shape());
		int return_code = CMZN_OK;
		if (shapeChange)
		{
			FE_region_begin_change(this->fe_region);
			// GRC make more efficient by caching shapeIndex for shape:
			if (0 == this->setElementShape(index, fe_element_template->get_element_shape()))
				return_code = CMZN_ERROR_GENERAL;
		}
		if (CMZN_OK == return_code)
			return_code = this->merge_FE_element_existing(destination, fe_element_template->get_template_element());
		if (shapeChange)
			FE_region_end_change(this->fe_region);
		return return_code;
	}
	return CMZN_ERROR_ARGUMENT;
}

/**
 * Find or create an element in this mesh that can be used on face number of
 * the parent element. The face is added to the parent.
 * The new face element is merged into this mesh, but without adding faces.
 * Must be between calls to begin_define_faces/end_define_faces.
 * Can only match faces correctly for coordinate fields with standard node
 * to element maps and no versions.
 * The element type node sequence is updated with any new face.
 *
 * @param parent_element  The parent element to find or create a face for.
 * @param face_number  Face number on parent, starting at 0.
 * @return  CMZN_OK on success, any other value on failure.
 */
int FE_mesh::find_or_create_face(struct FE_element *parent_element, int face_number)
{
	FE_element_type_node_sequence *element_type_node_sequence =
		CREATE(FE_element_type_node_sequence)(parent_element, face_number);
	if (!element_type_node_sequence)
		return CMZN_ERROR_GENERAL;

	int return_code = CMZN_OK;
	ACCESS(FE_element_type_node_sequence)(element_type_node_sequence);
	if (!FE_element_type_node_sequence_is_collapsed(element_type_node_sequence))
	{
		FE_element_type_node_sequence *existing_element_type_node_sequence =
			FE_element_type_node_sequence_list_find_match(
			this->element_type_node_sequence_list, element_type_node_sequence);
		if (existing_element_type_node_sequence)
		{
			set_FE_element_face(parent_element, face_number,
				FE_element_type_node_sequence_get_FE_element(existing_element_type_node_sequence));
		}
		else
		{
			FE_element_shape *parent_shape = get_FE_element_shape(parent_element);
			FE_element_shape *face_shape = get_FE_element_shape_of_face(parent_shape, face_number, fe_region);
			if (!face_shape)
				return_code = CMZN_ERROR_GENERAL;
			else
			{
				FE_element *face = this->get_or_create_FE_element_with_identifier(/*identifier*/-1, face_shape);
				if (!face)
					return_code = CMZN_ERROR_GENERAL;
				else
				{
					FE_element_type_node_sequence_set_FE_element(element_type_node_sequence, face);
					if (!(set_FE_element_face(parent_element, face_number, face) &&
						ADD_OBJECT_TO_LIST(FE_element_type_node_sequence)(
							element_type_node_sequence, this->element_type_node_sequence_list)))
						return_code = CMZN_ERROR_GENERAL;
					DEACCESS(FE_element)(&face);
				}
			}
		}
	}
	DEACCESS(FE_element_type_node_sequence)(&element_type_node_sequence);
	return return_code;
}

/**
 * Recursively define faces for element, creating and adding them to face
 * mesh if they don't already exist.
 * Always call between FE_region_begin/end_define_faces.
 * Always call between FE_region_begin/end_changes.
 * Function ensures that elements share existing faces and lines in preference to
 * creating new ones if they have matching dimension and nodes.
 */
int FE_mesh::define_FE_element_faces(struct FE_element *element)
{
	int return_code = 1;
	if (FE_element_get_FE_mesh(element) == this)
	{
		if (this->faceMesh && this->definingFaces)
		{
			int newFaceCount = 0;
			ElementShapeFaces *elementShapeFaces = this->getElementShapeFaces(get_FE_element_index(element));
			if (elementShapeFaces)
			{
				const int faceCount = elementShapeFaces->getFaceCount();
				for (int face_number = 0; (face_number < faceCount) && return_code; face_number++)
				{
					FE_element *face = 0;
					if ((return_code = get_FE_element_face(element, face_number, &face)) &&
						(!face) && this->definingFaces)
					{
						int result = this->faceMesh->find_or_create_face(element, face_number);
						if (result == CMZN_OK)
						{
							get_FE_element_face(element, face_number, &face);
							if (face)
								++newFaceCount;
						}
						else
							return_code = 0;
					}
					if (face)
					{
						// ensure the face is in the face mesh, recurse with its faces
						return_code = this->faceMesh->define_FE_element_faces(face);
					}
				}
				if (newFaceCount)
					this->elementChange(element, DS_LABEL_CHANGE_TYPE_DEFINITION, element);
			}
			else
				return_code = 0;
			if (!return_code)
				display_message(ERROR_MESSAGE, "FE_mesh::define_FE_element_faces.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "FE_mesh::define_FE_element_faces.  Element is not from this mesh");
		return_code = 0;
	}
	return (return_code);
}

/**
 * Creates a list of FE_element_type_node_sequence, and
 * if mesh dimension < MAXIMUM_ELEMENT_XI_DIMENSIONS fills it with sequences
 * for this element. Fails if any two faces have the same shape and nodes.
 */
int FE_mesh::begin_define_faces()
{
	if (this->element_type_node_sequence_list)
	{
		display_message(ERROR_MESSAGE, "FE_mesh::begin_define_faces.  Already defining faces");
		return CMZN_ERROR_ALREADY_EXISTS;
	}
	this->element_type_node_sequence_list = CREATE(LIST(FE_element_type_node_sequence))();
	if (!this->element_type_node_sequence_list)
	{
		display_message(ERROR_MESSAGE, "FE_mesh::begin_define_faces.  Could not create node sequence list");
		return CMZN_ERROR_MEMORY;
	}
	this->definingFaces = true;
	int return_code = CMZN_OK;
	if (this->dimension < MAXIMUM_ELEMENT_XI_DIMENSIONS)
	{
		cmzn_elementiterator_id iter = this->createElementiterator();
		cmzn_element_id element = 0;
		FE_element_type_node_sequence *element_type_node_sequence;
		while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
		{
			element_type_node_sequence = CREATE(FE_element_type_node_sequence)(element);
			if (!element_type_node_sequence)
			{
				display_message(ERROR_MESSAGE, "FE_mesh::begin_define_faces.  "
					"Could not create FE_element_type_node_sequence for %d-D element %d",
					this->dimension, get_FE_element_identifier(element));
				return_code = CMZN_ERROR_GENERAL;
				break;
			}
			if (!ADD_OBJECT_TO_LIST(FE_element_type_node_sequence)(
				element_type_node_sequence, this->element_type_node_sequence_list))
			{
				display_message(WARNING_MESSAGE, "FE_mesh::begin_define_faces.  "
					"Could not add FE_element_type_node_sequence for %d-D element %d.",
					this->dimension, get_FE_element_identifier(element));
				FE_element_type_node_sequence *existing_element_type_node_sequence =
					FE_element_type_node_sequence_list_find_match(
						this->element_type_node_sequence_list, element_type_node_sequence);
				if (existing_element_type_node_sequence)
				{
					display_message(WARNING_MESSAGE,
						"Reason: Existing %d-D element %d uses same node list, and will be used for face matching.",
						this->dimension, get_FE_element_identifier(
							FE_element_type_node_sequence_get_FE_element(existing_element_type_node_sequence)));
				}
				DESTROY(FE_element_type_node_sequence)(&element_type_node_sequence);
			}
		}
		cmzn_elementiterator_destroy(&iter);
	}
	return return_code;
}

void FE_mesh::end_define_faces()
{
	if (this->element_type_node_sequence_list)
		DESTROY(LIST(FE_element_type_node_sequence))(&this->element_type_node_sequence_list);
	else
		display_message(ERROR_MESSAGE, "FE_mesh::end_define_faces.  Wasn't defining faces");
	this->definingFaces = false;
}

/**
 * Ensures faces of elements in mesh exist in face mesh.
 * Recursively does same for faces in face mesh.
 * Call between begin/end_define_faces and begin/end_change.
 */
int FE_mesh::define_faces()
{
	int return_code = CMZN_OK;
	cmzn_elementiterator_id iter = this->createElementiterator();
	cmzn_element_id element = 0;
	while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
	{
		if (!this->define_FE_element_faces(element))
		{
			return_code = CMZN_ERROR_GENERAL;
			break;
		}
	}
	cmzn_elementiterator_destroy(&iter);
	return return_code;
}

/**
 * Removes <element> and all its faces that are not shared with other elements
 * from fe_region. Should enclose call between FE_region_begin_change and
 * FE_region_end_change to minimise messages.
 * This function is recursive.
 */
int FE_mesh::remove_FE_element_private(struct FE_element *element)
{
	int return_code = 1;
	if (IS_OBJECT_IN_LIST(FE_element)(element, this->elementList))
	{
		const DsLabelIndex index = get_FE_element_index(element);
		/* access element in case it is only accessed here */
		ACCESS(FE_element)(element);
		// must notify of change before invalidating element otherwise has no fields
		// assumes within begin/end change
		this->elementRemovedChange(element);
		REMOVE_OBJECT_FROM_LIST(FE_element)(element, this->elementList);
		if (this->parentMesh)
		{
			/* remove face elements from all their parents;
				 mark parent elements as DEFINITION_CHANGED */
			int face_number;
			struct FE_element *parent_element;
			while (return_code && (return_code =
				FE_element_get_first_parent(element, &parent_element, &face_number))
				&& parent_element)
			{
				return_code = set_FE_element_face(parent_element, face_number,
					(struct FE_element *)NULL);
				this->parentMesh->elementChange(parent_element,
					DS_LABEL_CHANGE_TYPE_DEFINITION, parent_element);
			}
		}
		if (this->faceMesh)
		{
			ElementShapeFaces *elementShapeFaces = this->getElementShapeFaces(get_FE_element_index(element));
			/* remove faces used by no other parent elements */
			if (elementShapeFaces)
			{
				const int faceCount = elementShapeFaces->getFaceCount();
				struct FE_element *face;
				for (int i = 0; (i < faceCount) && return_code; i++)
					if (get_FE_element_face(element, i, &face) && face)
					{
						// must remove face since element group or client may be accessing this element
						// so destructor won't be called to clean up in time.
						// also stops above remove parents code from needlessly being run
						set_FE_element_face(element, i, 0);
						if (FE_element_has_no_parents(face))
							return_code = this->faceMesh->remove_FE_element_private(face);
					}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::remove_FE_element_private.  Missing ElementShapeFaces");
				return_code = 0;
			}
		}
		FE_element_invalidate(element);
		this->labels.removeLabel(index);
		DEACCESS(FE_element)(&element);
		if (0 == this->labels.getSize())
			this->clear();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_mesh::remove_FE_element_private.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/**
 * Removes <element> and all its faces that are not shared with other elements
 * from <fe_region>.
 * FE_region_begin/end_change are called internally to reduce change messages to
 * one per call. User should place calls to the begin/end_change functions
 * around multiple calls to this function.
 * This function is recursive.
 */
int FE_mesh::remove_FE_element(struct FE_element *element)
{
	FE_region_begin_change(this->fe_region);
	int return_code = this->remove_FE_element_private(element);
	FE_region_end_change(this->fe_region);
	return return_code;
}

/**
 * Removes all the elements in <element_list>, and all their faces
 * that are not shared with other elements from <fe_region>.
 * FE_region_begin/end_change are called internally to reduce change messages to
 * one per call.
 */
int FE_mesh::remove_FE_element_list(struct LIST(FE_element) *remove_element_list)
{
	int return_code = 1;
	if (remove_element_list && (remove_element_list != this->elementList))
	{
		FE_region_begin_change(fe_region);
		cmzn_elementiterator *iter = CREATE_LIST_ITERATOR(cmzn_element)(remove_element_list);
		cmzn_element *element = 0;
		while ((0 != (element = cmzn_elementiterator_next_non_access(iter))))
		{
			if (!this->remove_FE_element_private(element))
			{
				return_code = 0;
				break;
			}
		}
		cmzn_elementiterator_destroy(&iter);
		FE_region_end_change(fe_region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "FE_mesh::remove_FE_element_list.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

bool FE_mesh::canMerge(FE_mesh &source)
{
	if (source.dimension != this->dimension)
	{
		display_message(ERROR_MESSAGE, "FE_mesh::canMerge.  Source mesh has wrong dimension");
		return false;
	}
	bool result = true;
	cmzn_elementiterator *iter = source.createElementiterator();
	cmzn_element *sourceElement;
	while (0 != (sourceElement = cmzn_elementiterator_next_non_access(iter)))
	{
		const DsLabelIndex sourceIndex = get_FE_element_index(sourceElement);
		const DsLabelIdentifier identifier = source.getElementIdentifier(sourceIndex);
		FE_element *targetElement = this->findElementByIdentifier(identifier);
		ElementShapeFaces *sourceElementShapeFaces = source.getElementShapeFaces(sourceIndex);
		if (!sourceElementShapeFaces)
		{
			display_message(ERROR_MESSAGE, "FE_mesh::canMerge.  Source %d-D element %d missing ElementShapeFaces",
				source.dimension, identifier);
			result = false;
			break;
		}
		if (FE_element_shape_is_unspecified(sourceElementShapeFaces->getShape()))
		{
			// unspecified shape is used for nodal element:xi values when element is
			// not read in from the same file, but could in future be used for
			// reading field definitions without shape information.
			// Must find a matching global element.
			if (!targetElement)
			{
				display_message(ERROR_MESSAGE, "%d-D element %d is not found in global mesh",
					this->dimension, identifier);
				result = false;
				break;
			}
		}
		else if (targetElement)
		{
			const DsLabelIndex targetIndex = get_FE_element_index(targetElement);
			ElementShapeFaces *targetElementShapeFaces = this->getElementShapeFaces(targetIndex);
			if (!sourceElementShapeFaces)
			{
				display_message(ERROR_MESSAGE, "FE_mesh::canMerge.  Target %d-D element %d missing ElementShapeFaces",
					source.dimension, identifier);
				result = false;
				break;
			}
			if (sourceElementShapeFaces->getShape() != targetElementShapeFaces->getShape())
			{
				display_message(ERROR_MESSAGE, "FE_mesh::canMerge.  Cannot merge %d-D element %d with different shape",
					source.dimension, identifier);
				result = false;
				break;
			}
			const int faceCount = sourceElementShapeFaces->getFaceCount();
			if (faceCount > 0)
			{
				if (!(source.faceMesh && this->faceMesh))
				{
					display_message(ERROR_MESSAGE, "FE_mesh::canMerge.  %d-D element %d missing face meshes",
						source.dimension, identifier);
					result = false;
					break;
				}
				if (!FE_elements_can_merge_faces(faceCount, *(this->faceMesh), targetElement, *(source.faceMesh), sourceElement))
				{
					display_message(ERROR_MESSAGE, "FE_mesh::canMerge.  Source %d-D element %d has different faces",
						source.dimension, identifier);
					result = false;
					break;
				}
			}
		}
	}
	cmzn_elementiterator_destroy(&iter);
	return result;
}

/**
 * Data for passing to FE_mesh::merge_FE_element_external.
 */
struct FE_mesh::Merge_FE_element_external_data
{
	FE_mesh &source;
	FE_nodeset &fe_nodeset;
	/* use following array and number to build up matching pairs of old element
		 field info what they become in the global_fe_region.
		 Note these are ACCESSed */
	FE_element_field_info **matching_element_field_info;
	int number_of_matching_element_field_info;

	Merge_FE_element_external_data(FE_mesh &sourceIn, FE_nodeset &fe_nodeset_in) :
		source(sourceIn),
		fe_nodeset(fe_nodeset_in),
		matching_element_field_info(0),
		number_of_matching_element_field_info(0)
	{
	}

	~Merge_FE_element_external_data()
	{
		if (this->matching_element_field_info)
		{
			for (int i = 2*this->number_of_matching_element_field_info - 1; 0 <= i; --i)
				DEACCESS(FE_element_field_info)(&(this->matching_element_field_info[i]));
			DEALLOCATE(this->matching_element_field_info);
		}
	}

	FE_element_field_info *get_matching_FE_element_field_info(FE_element_field_info *source_element_field_info)
	{
		// 1. Convert element to use a new FE_element_field_info from this mesh
		// fast path: check if the element_field_info has already been matched
		FE_element_field_info **matching_element_field_info = this->matching_element_field_info;
		for (int i = 0; i < this->number_of_matching_element_field_info; ++i)
		{
			if (*matching_element_field_info == source_element_field_info)
				return *(matching_element_field_info + 1);
			matching_element_field_info += 2;
		}
		return 0;
	}

	/**
	 * Record match between source_element_field_info and target_element_field_info.
	 * @return  True on success.
	 */
	bool add_matching_FE_element_field_info(
		FE_element_field_info *source_element_field_info, FE_element_field_info *target_element_field_info)
	{
		FE_element_field_info **matching_element_field_info;
		if (REALLOCATE(matching_element_field_info,
			this->matching_element_field_info, struct FE_element_field_info *,
			2*(this->number_of_matching_element_field_info + 1)))
		{
			matching_element_field_info[this->number_of_matching_element_field_info*2] =
				ACCESS(FE_element_field_info)(source_element_field_info);
			matching_element_field_info[this->number_of_matching_element_field_info*2 + 1] =
				ACCESS(FE_element_field_info)(target_element_field_info);
			this->matching_element_field_info = matching_element_field_info;
			++(this->number_of_matching_element_field_info);
			return true;
		}
		display_message(ERROR_MESSAGE,
			"FE_mesh::Merge_FE_element_external_data::add_matching_FE_element_field_info.  Failed");
		return false;
	}

};

/**
 * Merge element from another mesh, used when reading models from files into
 * temporary regions.
 * Before merging, substitutes into element an appropriate element field info
 * from this mesh, plus nodes from the corresponding FE_nodeset which have the
 * same identifiers as those currently used. Scale factors and nodes are
 * similarly converted.
 * Since this changes information in the element the caller is required to
 * destroy the source mesh immediately after calling this function on any
 * elements from it. Operations such as findElementByIdentifier will no longer
 * work as the element is given a new index for this mesh.
 */
int FE_mesh::merge_FE_element_external(struct FE_element *element,
	Merge_FE_element_external_data &data)
{
	int return_code = 1;
	struct FE_element_field_info *old_element_field_info;

	FE_element_shape *element_shape = get_FE_element_shape(element);
	if (element_shape && (old_element_field_info = FE_element_get_FE_element_field_info(element)))
	{
		const DsLabelIdentifier identifier = get_FE_element_identifier(element);
		FE_element *global_element = this->findElementByIdentifier(identifier);
		if (FE_element_shape_is_unspecified(element_shape))
		{
			if (!global_element)
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::merge_FE_element_external.  No matching embedding element");
				return 0;
			}
			return 1;
		}
		const DsLabelIndex newIndex = (global_element) ? get_FE_element_index(global_element) :
			this->labels.createLabel(identifier);
		if (newIndex < 0)
		{
			display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element_external.  Failed to get element label.");
			return 0;
		}
		ElementShapeFaces *elementShapeFaces = (global_element) ? this->getElementShapeFaces(newIndex) :
			this->setElementShape(newIndex, element_shape);
		if (!elementShapeFaces)
		{
			display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element_external.  Failed to get ElementShapeFaces");
			return 0;
		}

		return_code = 1;
		// 1. Convert element to use a new FE_element_field_info from this mesh
		FE_element_field_info *element_field_info = data.get_matching_FE_element_field_info(old_element_field_info);
		if (!element_field_info)
		{
			element_field_info = this->clone_FE_element_field_info(old_element_field_info);
			if (element_field_info)
			{
				if (!data.add_matching_FE_element_field_info(old_element_field_info, element_field_info))
				{
					DESTROY(FE_element_field_info)(&element_field_info);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::merge_FE_element_external.  Could not clone element_field_info");
			}
		}
		if (element_field_info)
		{
			/* substitute global nodes */
			int number_of_nodes;
			if (get_FE_element_number_of_nodes(element, &number_of_nodes))
			{
				struct FE_node *new_node, *old_node;
				for (int i = 0; i < number_of_nodes; ++i)
				{
					if (get_FE_element_node(element, i, &old_node))
					{
						if (old_node)
						{
							new_node = data.fe_nodeset.findNodeByIdentifier(get_FE_node_identifier(old_node));
							if (!((new_node) && set_FE_element_node(element, i, new_node)))
							{
								return_code = 0;
								break;
							}
						}
					}
					else
					{
						return_code = 0;
						break;
					}
				}
			}
			else
			{
				return_code = 0;
			}
			/* substitute global scale factor set identifiers */
			int number_of_scale_factor_sets = 0;
			if (get_FE_element_number_of_scale_factor_sets(element, &number_of_scale_factor_sets))
			{
				for (int i = 0; (i < number_of_scale_factor_sets) && return_code; ++i)
				{
					cmzn_mesh_scale_factor_set *source_scale_factor_set =
						get_FE_element_scale_factor_set_identifier_at_index(element, i);
					cmzn_mesh_scale_factor_set *global_scale_factor_set = this->find_scale_factor_set_by_name(source_scale_factor_set->getName());
					if (!global_scale_factor_set)
					{
						global_scale_factor_set = this->create_scale_factor_set();
						global_scale_factor_set->setName(source_scale_factor_set->getName());
					}
					set_FE_element_scale_factor_set_identifier_at_index(element, i, global_scale_factor_set);
					cmzn_mesh_scale_factor_set::deaccess(global_scale_factor_set);
				}
			}
			else
			{
				return_code = 0;
			}
			// merge equivalent-identifier faces into global or soon-to-be global target element
			// as no longer done by ::merge_FE_element
			FE_element *targetElement = global_element ? global_element : element;
			const int faceCount = elementShapeFaces->getFaceCount();
			if (faceCount > 0)
			{
				if ((!this->faceMesh) || (!data.source.faceMesh))
				{
					display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element_external.  Missing face mesh(es)");
					return_code = 0;
				}
				else
				{
					struct FE_element *targetFace, *sourceFace;
					for (int i = 0; i < faceCount; ++i)
					{
						if (get_FE_element_face(element, i, &sourceFace) && sourceFace)
						{
							// the face may have been transferred to target mesh
							if (FE_element_get_FE_mesh(sourceFace) == this->faceMesh)
								targetFace = sourceFace;
							else
							{
								const DsLabelIdentifier identifier = data.source.faceMesh->getElementIdentifier(get_FE_element_index(sourceFace));
								targetFace = this->faceMesh->findElementByIdentifier(identifier);
								if (!targetFace)
								{
									display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element_external.  Missing target face");
									return_code = 0;
									break;
								}
							}
							if (!set_FE_element_face(targetElement, i, targetFace))
								return_code = 0;
						}
					}
					// prevent faces from having temporary element as parent
					if (global_element)
						clear_FE_element_faces(element, faceCount);
				}
			}
			if (return_code)
			{
				DsLabelIndex oldIndex = get_FE_element_index(element);
				if (global_element)
					ACCESS(FE_element_field_info)(old_element_field_info);
				/* substitute the new element field info */
				FE_element_set_FE_element_field_info(element, element_field_info);
				set_FE_element_index(element, newIndex);
				if (global_element)
				{
					if (this->merge_FE_element_existing(global_element, element) != CMZN_OK)
						return_code = 0;
					// must restore the previous information for subsequent
					FE_element_set_FE_element_field_info(element, old_element_field_info);
					DEACCESS(FE_element_field_info)(&old_element_field_info);
					set_FE_element_index(element, oldIndex);
				}
				else
				{
					if (ADD_OBJECT_TO_LIST(FE_element)(element, this->elementList))
						this->elementAddedChange(element);
					else
					{
						display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element_external.  Failed to add element to list.");
						this->labels.removeLabel(newIndex);
						return_code = 0;
					}
				}
			}
		}
		else
		{
			return_code = 0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element_external.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_mesh::merge_FE_element_external.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int FE_mesh::merge(FE_mesh &source)
{
	int return_code = 1;
	if (source.dimension == this->dimension)
	{
		Merge_FE_element_external_data data(source,
			*FE_region_find_FE_nodeset_by_field_domain_type(this->fe_region, CMZN_FIELD_DOMAIN_TYPE_NODES));
		cmzn_elementiterator *iter = source.createElementiterator();
		cmzn_element *element;
		while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
		{
			if (!this->merge_FE_element_external(element, data))
			{
				display_message(ERROR_MESSAGE, "FE_mesh::merge.  Could not merge element");
				return_code = 0;
				break;
			}
		}
		cmzn_elementiterator_destroy(&iter);
	}
	else
	{
		return_code = 0;
	}
	return return_code;
}
