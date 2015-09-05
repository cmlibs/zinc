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

FE_mesh::FE_mesh(FE_region *fe_regionIn, int dimensionIn) :
	fe_region(fe_regionIn),
	dimension(dimensionIn),
	elementList(CREATE(LIST(FE_element)())),
	element_field_info_list(CREATE(LIST(FE_element_field_info))()),
	parentMesh(0),
	faceMesh(0),
	fe_element_changes(0),
	last_fe_element_field_info(0),
	element_type_node_sequence_list(0),
	definingFaces(false),
	next_fe_element_identifier_cache(0),
	access_count(1)
{
}

FE_mesh::~FE_mesh()
{
	DESTROY(CHANGE_LOG(FE_element))(&this->fe_element_changes);
	this->last_fe_element_field_info = 0;
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
}

/**
 * Call this to mark element with the supplied change, logging field changes
 * from the field_info_element in the fe_region.
 * Notifies change to clients of FE_region.
 * When an element is added or removed, the same element is used for <element> and
 * <field_info_element>. For changes to the contents of <element>, <field_info_element>
 * should contain the changed fields, consistent with merging it into <element>.
 */
void FE_mesh::elementChange(FE_element *element, CHANGE_LOG_CHANGE(FE_element) change, FE_element *field_info_element)
{
	if (this->fe_region)
	{
		CHANGE_LOG_OBJECT_CHANGE(FE_element)(this->fe_element_changes, element, change);
		// for efficiency, the following marks field changes only if field info changes
		FE_element_log_FE_field_changes(field_info_element, fe_region->fe_field_changes, /*recurseParents*/false);
		this->fe_region->update();
	}
}

/**
 * Records change to element affecting the supplied fields.
 */
void FE_mesh::elementFieldListChange(FE_element *element, CHANGE_LOG_CHANGE(FE_element) change,
	LIST(FE_field) *changed_fe_field_list)
{
	if (this->fe_region)
	{
		CHANGE_LOG_OBJECT_CHANGE(FE_element)(this->fe_element_changes, element, change);
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
	this->next_fe_element_identifier_cache = 0;
	CHANGE_LOG_OBJECT_CHANGE(FE_element)(this->fe_element_changes, element,
		CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_element));
	this->fe_region->update();
}

/**
 * Call this instead of elementChange when exactly one field, <fe_field> of
 * <element> has changed.
 */
void FE_mesh::elementFieldChange(FE_element *element, FE_field *fe_field)
{
	if (this->fe_region)
	{
		CHANGE_LOG_OBJECT_CHANGE(FE_element)(this->fe_element_changes, element,
			CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_element));
		CHANGE_LOG_OBJECT_CHANGE(FE_field)(this->fe_region->fe_field_changes,
			fe_field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
		this->fe_region->update();
	}
}

void FE_mesh::elementAddedChange(FE_element *element)
{
	if (this->fe_region)
	{
		this->next_fe_element_identifier_cache = 0;
		CHANGE_LOG_OBJECT_CHANGE(FE_element)(this->fe_element_changes, element,
			CHANGE_LOG_OBJECT_ADDED(FE_element));
		// for efficiency, the following marks field changes only if field info changes
		FE_element_log_FE_field_changes(element, fe_region->fe_field_changes, /*recurseParents*/true);
		this->fe_region->update();
	}
}

void FE_mesh::elementRemovedChange(FE_element *element)
{
	if (this->fe_region)
	{
		this->next_fe_element_identifier_cache = 0;
		CHANGE_LOG_OBJECT_CHANGE(FE_element)(this->fe_element_changes, element,
			CHANGE_LOG_OBJECT_REMOVED(FE_element));
		// for efficiency, the following marks field changes only if field info changes
		FE_element_log_FE_field_changes(element, fe_region->fe_field_changes, /*recurseParents*/true);
		this->fe_region->update();
	}
}

// NOTE! Only to be called by FE_region_clear
// Expects change cache to be on, otherwise very inefficient
void FE_mesh::clear()
{
	cmzn_elementiterator *iter = this->createElementiterator();
	cmzn_element *element = 0;
	while ((0 != (element = cmzn_elementiterator_next_non_access(iter))))
	{
		this->elementRemovedChange(element);
	}
	cmzn_elementiterator_destroy(&iter);
	REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)((LIST_CONDITIONAL_FUNCTION(FE_element) *)NULL,
		(void *)NULL, this->elementList);
}

void FE_mesh::createChangeLog()
{
	if (this->fe_element_changes)
		DESTROY(CHANGE_LOG(FE_element))(&this->fe_element_changes);
	this->fe_element_changes = CREATE(CHANGE_LOG(FE_element))(this->elementList, /*max_changes*/2000);
	this->last_fe_element_field_info = 0;
}

struct CHANGE_LOG(FE_element) *FE_mesh::extractChangeLog()
{
	struct CHANGE_LOG(FE_element) *changes = this->fe_element_changes;
	this->fe_element_changes = 0;
	this->createChangeLog();
	return changes;
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

/**
 * Returns the next unused element number for elements in mesh, starting from
 * <start_identifier>.
 * @param start_identifier  Minimum number for new identifier. Pass 0 to give
 * the first available number >= 1.
 */
int FE_mesh::get_next_FE_element_identifier(int start_identifier)
{
	struct CM_element_information cm =
	{
		DIMENSION_TO_CM_ELEMENT_TYPE(this->dimension),
		(start_identifier <= 0) ? 1 : start_identifier
	};
	if (this->next_fe_element_identifier_cache)
	{
		if (this->next_fe_element_identifier_cache > cm.number)
		{
			cm.number = this->next_fe_element_identifier_cache;
		}
	}
	while (FIND_BY_IDENTIFIER_IN_LIST(FE_element, identifier)(&cm, this->elementList))
	{
		++cm.number;
	}
	if (start_identifier <= 1)
	{
		/* Don't cache the value if we didn't start at the beginning */
		this->next_fe_element_identifier_cache = cm.number;
	}
	return cm.number;
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

/** @return  Non-accessed element */
FE_element *FE_mesh::findElementByIdentifier(int identifier) const
{
	CM_element_information cm = { DIMENSION_TO_CM_ELEMENT_TYPE(this->dimension), identifier };
	return FIND_BY_IDENTIFIER_IN_LIST(FE_element, identifier)(&cm, this->elementList);
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
	if (element && (new_identifier >= 0))
	{
		if (IS_OBJECT_IN_LIST(FE_element)(element, this->elementList))
		{
			if (this->findElementByIdentifier(new_identifier))
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::change_FE_element_identifier.  "
					"Element with new identifier already exists");
				return CMZN_ERROR_ALREADY_EXISTS;
			}
			// this temporarily removes the object from all indexed lists
			if (LIST_BEGIN_IDENTIFIER_CHANGE(FE_element, identifier)(
				this->elementList, element))
			{
				CM_element_information cm = { DIMENSION_TO_CM_ELEMENT_TYPE(this->dimension), new_identifier };
				set_FE_element_identifier(element, &cm);
				LIST_END_IDENTIFIER_CHANGE(FE_element, identifier)(this->elementList);
				this->elementIdentifierChange(element);
				return CMZN_OK;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::change_FE_element_identifier.  "
					"Could not safely change identifier in indexed lists");
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

/**
	* Convenience function returning an existing element with the identifier
	* from the mesh. If no existing element is found, a new element is created
	* with the given identifier and the supplied shape, or unspecified shape of
	* the given dimension if no shape provided.
	* If the returned element is not already in fe_region it is merged.
	* It is expected that the calling function has wrapped calls to this function
	* with FE_region_begin/end_change.
	*/
struct FE_element *FE_mesh::get_or_create_FE_element_with_identifier(int identifier,
	struct FE_element_shape *element_shape)
{
	struct FE_element *element = 0;
	int element_shape_dimension = 0;
	if (((!element_shape) || (get_FE_element_shape_dimension(element_shape, &element_shape_dimension) &&
		(element_shape_dimension == this->dimension))))
	{
		element = this->findElementByIdentifier(identifier);
		if (!element)
		{
			struct FE_element_shape *temp_element_shape = 0;
			if (!element_shape)
			{
				element_shape = temp_element_shape = CREATE(FE_element_shape)(
					dimension, /*type*/(int *)NULL, fe_region);
				ACCESS(FE_element_shape)(temp_element_shape);
			}
			if (element_shape)
			{
				CM_element_information cm = { DIMENSION_TO_CM_ELEMENT_TYPE(dimension), identifier};
				element = CREATE(FE_element)(&cm, element_shape,
					this, /*template_element*/(struct FE_element *)NULL);
				if (temp_element_shape)
				{
					DEACCESS(FE_element_shape)(&temp_element_shape);
				}
			}
			if (!element)
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::get_or_create_FE_element_with_identifier.  Could not create element");
			}
			if (element)
			{
				if (!this->merge_FE_element(element))
				{
					display_message(ERROR_MESSAGE,
						"FE_mesh::get_or_create_FE_element_with_identifier.   Could not merge element");
					/* following cleans up element if newly created, and clears pointer */
					REACCESS(FE_element)(&element, (struct FE_element *)NULL);
				}
			}
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
 * Checks the source element is compatible with mesh & that there is no
 * existing element of supplied identifier, then creates element of that
 * identifier as a copy of source and adds it to the fe_region.
 *
 * @param identifier  Non-negative integer identifier of new element, or -1 to
 * automatically generate (starting at 1). Fails if supplied identifier already
 * used by an existing element.
 * @return  New element (non-accessed), or NULL if failed.
 */
struct FE_element *FE_mesh::create_FE_element_copy(int identifier, struct FE_element *source)
{
	struct FE_element *new_element = 0;
	if (source && (-1 <= identifier))
	{
		if (FE_element_get_FE_mesh(source) == this)
		{
			int number = (identifier < 0) ? this->get_next_FE_element_identifier(0) : identifier;
			struct CM_element_information cm = { DIMENSION_TO_CM_ELEMENT_TYPE(this->dimension), number };
			new_element = CREATE(FE_element)(&cm, (struct FE_element_shape *)NULL,
				(FE_mesh *)NULL, source);
			if (ADD_OBJECT_TO_LIST(FE_element)(new_element, this->elementList))
			{
				if (this->definingFaces)
					FE_region_begin_change(fe_region);
				this->elementAddedChange(new_element); 
				if (this->definingFaces)
				{
					this->merge_FE_element_and_faces(new_element);
					FE_region_end_change(fe_region);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::create_FE_element_copy.  element identifier in use.");
				DESTROY(FE_element)(&new_element);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_mesh::create_FE_element_copy.  "
				"Source element is incompatible with region");
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
					this->elementFieldListChange(destination, static_cast<CHANGE_LOG_CHANGE(FE_element)>(
						CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_element) | CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_element)),
						changed_fe_field_list);
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

/**
 * Checks <element> is compatible with this mesh and any existing FE_element
 * using the same identifier, then merges it in.
 * If no FE_element of the same identifier exists in mesh, <element> is
 * added to mesh and returned by this function, otherwise changes are
 * merged into the existing FE_element and it is returned.
 * During the merge, any new fields from <element> are added to the existing
 * element of the same identifier, and existing fields are overwritten.
 *
 * @return  On success, the element from the region which differs from the
 * element argument if modifying an existing element, or NULL on error.
 */
struct FE_element *FE_mesh::merge_FE_element(struct FE_element *element)
{
	struct FE_element *merged_element = 0;
	if (element)
	{
		if (FE_element_get_FE_mesh(element) == this)
		{
			struct CM_element_information cm;
			get_FE_element_identifier(element, &cm);
			merged_element = FIND_BY_IDENTIFIER_IN_LIST(FE_element, identifier)(&cm, this->elementList);
			if (merged_element)
			{
				int return_code = this->merge_FE_element_existing(merged_element, element);
				if (return_code != CMZN_OK)
					merged_element = (struct FE_element *)NULL;
			}
			else
			{
				if (ADD_OBJECT_TO_LIST(FE_element)(element, this->elementList))
				{
					merged_element = element;
					this->elementAddedChange(merged_element);
				}
				else
				{
					display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element.  Could not add %d-D element %d",
						this->dimension, cmzn_element_get_identifier(merged_element));
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element.  "
				"element %d is not of this mesh", cmzn_element_get_identifier(merged_element));
		}
	}
	return merged_element;
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
 * @return  CMZN_OK on success.
 */
int FE_mesh::find_or_create_face(struct FE_element *parent_element, int face_number)
{
	struct FE_element_shape *parent_shape;
	get_FE_element_shape(parent_element, &parent_shape);
	FE_element_shape *face_shape = get_FE_element_shape_of_face(parent_shape, face_number, fe_region);
	if (!face_shape)
		return CMZN_ERROR_GENERAL;

	int new_face_number = this->get_next_FE_element_identifier(0);
	CM_element_information face_identifier =
		{ DIMENSION_TO_CM_ELEMENT_TYPE(this->dimension), new_face_number };
	FE_element *face = CREATE(FE_element)(&face_identifier, face_shape,	this, (struct FE_element *)NULL);
	if (!face)
		return CMZN_ERROR_GENERAL;

	/* must put the face in the element to inherit fields */
	set_FE_element_face(parent_element, face_number, face);
	/* try to find existing face with same shape and nodes */
	FE_element_type_node_sequence *element_type_node_sequence = CREATE(FE_element_type_node_sequence)(face);
	if (!element_type_node_sequence)
	{
		set_FE_element_face(parent_element, face_number, (FE_element *)0);
		return CMZN_ERROR_GENERAL;
	}

	int return_code = CMZN_OK;
	ACCESS(FE_element_type_node_sequence)(element_type_node_sequence);
	if (FE_element_type_node_sequence_is_collapsed(element_type_node_sequence))
	{
		set_FE_element_face(parent_element, face_number, (FE_element *)0);
	}
	else
	{
		FE_element_type_node_sequence *existing_element_type_node_sequence =
			FIND_BY_IDENTIFIER_IN_LIST(FE_element_type_node_sequence, identifier)(
				FE_element_type_node_sequence_get_identifier(element_type_node_sequence),
					this->element_type_node_sequence_list);
		if (existing_element_type_node_sequence)
		{
			face = FE_element_type_node_sequence_get_FE_element(existing_element_type_node_sequence);
			set_FE_element_face(parent_element, face_number, face);
		}
		else
		{
			// merge face and remember this sequence
			if ((0 == this->merge_FE_element(face)) || (!ADD_OBJECT_TO_LIST(FE_element_type_node_sequence)(
					element_type_node_sequence, this->element_type_node_sequence_list)))
				return_code = CMZN_ERROR_GENERAL;
		}
	}
	DEACCESS(FE_element_type_node_sequence)(&element_type_node_sequence);
	return return_code;
}

/**
 * Merge element and faces recursively, creating and adding new faces as below.
 * Always call between FE_region_begin/end_changes.
 * 
 * If calls to this function are placed between FE_region_begin/end_define_faces,
 * then any missing faces are created and also merged into face mesh.
 * Function ensures that elements share existing faces and lines in preference to
 * creating new ones if they have matching shape and nodes.
 */
int FE_mesh::merge_FE_element_and_faces(struct FE_element *element)
{
	int return_code = 1;
	if (FE_element_get_FE_mesh(element) == this)
	{
		int newFaceCount = 0;
		if (this->faceMesh && this->definingFaces)
		{
			int number_of_faces;
			get_FE_element_number_of_faces(element, &number_of_faces);
			for (int face_number = 0; (face_number < number_of_faces) && return_code; face_number++)
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
					return_code = this->faceMesh->merge_FE_element_and_faces(face);
				}
			}
		}
		if (return_code)
		{
			if (this->containsElement(element))
			{
				if (newFaceCount)
					this->elementChange(element, CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_element), element);
			}
			else
			{
				if (!this->merge_FE_element(element))
					return_code = 0;
			}
		}
		if (!return_code)
			display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element_and_faces.  Failed");
	}
	else
	{
		display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element_and_faces.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

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
	if (!FOR_EACH_OBJECT_IN_LIST(FE_element)(
		FE_element_face_line_to_element_type_node_sequence_list,
		(void *)(this->element_type_node_sequence_list),
		this->elementList))
	{
		display_message(ERROR_MESSAGE, "FE_mesh::begin_define_faces.  "
			"May not be able to share faces properly - perhaps two existing faces have same shape and node list?");
		return CMZN_ERROR_GENERAL;
	}
	return CMZN_OK;
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
		if (!this->merge_FE_element_and_faces(element))
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
		/* access element in case it is only accessed here */
		ACCESS(FE_element)(element);
		// must notify of change before invalidating element otherwise has no fields
		// assumes within begin/end change
		this->elementRemovedChange(element);
		FE_element_invalidate(element);
		REMOVE_OBJECT_FROM_LIST(FE_element)(element, this->elementList);
		if (this->parentMesh)
		{
			/* remove face elements from all their parents;
				 mark parent elements as OBJECT_NOT_IDENTIFIER_CHANGED */
			int face_number;
			struct FE_element *parent_element;
			while (return_code && (return_code =
				FE_element_get_first_parent(element, &parent_element, &face_number))
				&& parent_element)
			{
				return_code = set_FE_element_face(parent_element, face_number,
					(struct FE_element *)NULL);
				this->parentMesh->elementChange(parent_element,
					CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_element),
					parent_element);
			}
		}
		if (this->faceMesh)
		{
			struct FE_element *face;
			/* remove faces used by no other parent elements */
			int number_of_faces = 0;
			get_FE_element_number_of_faces(element, &number_of_faces);
			for (int i = 0; (i < number_of_faces) && return_code; i++)
				if (get_FE_element_face(element, i, &face) && face)
				{
					if (!cmzn_element_has_parent_in_list(face, this->elementList))
						return_code = this->faceMesh->remove_FE_element_private(face);
				}
		}
		DEACCESS(FE_element)(&element);
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
	cmzn_elementiterator *iter = source.createElementiterator();
	cmzn_element *element;
	bool result = true;
	while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
	{
		if (!FE_element_can_be_merged(element, this))
		{
			display_message(ERROR_MESSAGE, "Cannot merge element into mesh due to incompatible shape or faces");
			result = false;
			break;
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
	FE_nodeset *fe_nodeset;
	/* use following array and number to build up matching pairs of old element
		 field info what they become in the global_fe_region.
		 Note these are ACCESSed */
	struct FE_element_field_info **matching_element_field_info;
	int number_of_matching_element_field_info;
};

/**
 * Specialised version of merge_FE_element for merging elements from other
 * regions, used when reading models from files.
 * Before merging, substitutes into element an appropriate element field info
 * from this mesh, plus nodes from the corresponding FE_nodeset which have the
 * same identifiers as those currently used.
 */
int FE_mesh::merge_FE_element_external(struct FE_element *element,
	Merge_FE_element_external_data &data)
{
	int return_code = 1;
	struct FE_element_field_info *current_element_field_info;
	struct FE_element_shape *element_shape;

	if (element && (current_element_field_info = FE_element_get_FE_element_field_info(element)) &&
		get_FE_element_shape(element, &element_shape))
	{
		struct CM_element_information cm;
		get_FE_element_identifier(element, &cm);
		FE_element *global_element = this->findElementByIdentifier(cm.number);
		if (FE_element_shape_is_unspecified(element_shape))
		{
			if (!global_element)
			{
				display_message(ERROR_MESSAGE,
					"FE_mesh::merge_FE_element_external.  No matching embedding element");
				return_code = 0;
			}
		}
		else
		{
			return_code = 1;
			// 1. Convert element to use a new FE_element_field_info from this mesh
			// fast path: check if the element_field_info has already been matched
			FE_element_field_info **matching_element_field_info = data.matching_element_field_info;
			FE_element_field_info *element_field_info = 0;
			for (int i = 0; i < data.number_of_matching_element_field_info; ++i)
			{
				if (*matching_element_field_info == current_element_field_info)
				{
					element_field_info = *(matching_element_field_info + 1);
					break;
				}
				matching_element_field_info += 2;
			}
			if (!element_field_info)
			{
				element_field_info = this->clone_FE_element_field_info(current_element_field_info);
				if (element_field_info)
				{
					/* store combination of element field info in matching list */
					if (REALLOCATE(matching_element_field_info,
						data.matching_element_field_info, struct FE_element_field_info *,
						2*(data.number_of_matching_element_field_info + 1)))
					{
						matching_element_field_info[data.number_of_matching_element_field_info*2] =
							ACCESS(FE_element_field_info)(current_element_field_info);
						matching_element_field_info[data.number_of_matching_element_field_info*2 + 1] =
							ACCESS(FE_element_field_info)(element_field_info);
						data.matching_element_field_info = matching_element_field_info;
						++(data.number_of_matching_element_field_info);
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
				/* substitute the new element field info */
				FE_element_set_FE_element_field_info(element, element_field_info);
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
								new_node = data.fe_nodeset->findNodeByIdentifier(get_FE_node_identifier(old_node));
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
				// substitute global faces, but add parent pointers only if this element will become global
				bool willBeGlobal = (!global_element) || (element == global_element);
				int number_of_faces;
				if (get_FE_element_number_of_faces(element, &number_of_faces))
				{
					if ((0 < number_of_faces) && (!this->faceMesh))
					{
						display_message(ERROR_MESSAGE, "FE_mesh::merge_FE_element_external.  Missing face mesh");
						return_code = 0;
					}
					else
					{
						struct FE_element *new_face, *old_face;
						for (int i = 0; i < number_of_faces; ++i)
						{
							if (get_FE_element_face(element, i, &old_face))
							{
								if (old_face)
								{
									new_face = this->faceMesh->findElementByIdentifier(cmzn_element_get_identifier(old_face));
									if (new_face)
									{
										if (willBeGlobal)
										{
											if (!set_FE_element_face(element, i, new_face))
												return_code = 0;
										}
										else
										{
											if (!set_FE_element_face_no_parents(element, i, new_face))
												return_code = 0;
										}
									}
									else
									{
										return_code = 0;
									}
								}
							}
							else
							{
								return_code = 0;
							}
						}
					}
				}
				else
				{
					return_code = 0;
				}
				if (!this->merge_FE_element_and_faces(element))
				{
					return_code = 0;
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
		Merge_FE_element_external_data data;
		data.fe_nodeset = FE_region_find_FE_nodeset_by_field_domain_type(
			this->fe_region, CMZN_FIELD_DOMAIN_TYPE_NODES);
		/* use following array and number to build up matching pairs of old
		   element field info what they become in the global_fe_region */
		data.matching_element_field_info =
			(struct FE_element_field_info **)NULL;
		data.number_of_matching_element_field_info = 0;
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
		if (data.matching_element_field_info)
		{
			for (int i = 2*data.number_of_matching_element_field_info - 1; 0 <= i; --i)
				DEACCESS(FE_element_field_info)(&(data.matching_element_field_info[i]));
			DEALLOCATE(data.matching_element_field_info);
		}
	}
	else
	{
		return_code = 0;
	}
	return return_code;
}

int FE_element_is_not_in_FE_mesh(struct FE_element *element,
	void *fe_mesh_void)
{
	if (fe_mesh_void && (!static_cast<FE_mesh *>(fe_mesh_void)->containsElement(element)))
		return 1;
	return 0;
}
