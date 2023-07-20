/**
FILE : computed_field.cpp

Definition of generic field class defining a mapping from domain locations
to field values. Evaluation is given by Computed_field_core derived object
owned by field, of which Finite Element field is one type.
*/
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "cmlibs/zinc/result.h"
#include "cmlibs/zinc/status.h"
#include "cmlibs/zinc/fieldcomposite.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/differential_operator.hpp"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/field_cache.hpp"
#include "computed_field/field_module.hpp"
#include "computed_field/field_range.hpp"
#include "computed_field/fieldparametersprivate.hpp"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_field_evaluation.hpp"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_discretization.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/indexed_list_stl_private.hpp"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "general/value.h"
#include "general/message.h"
#include "general/enumerator_conversion.hpp"
#include <typeinfo>

/*
Module functions
----------------
*/

/** override to set change status of fields which depend on changed fields */
inline void MANAGER_UPDATE_DEPENDENCIES(cmzn_field)(
	struct MANAGER(cmzn_field) *manager)
{
	cmzn_set_cmzn_field *all_fields = reinterpret_cast<cmzn_set_cmzn_field *>(manager->object_list);
	for (cmzn_set_cmzn_field::iterator iter = all_fields->begin(); iter != all_fields->end(); iter++)
	{
		cmzn_field_id field = *iter;
		field->core->check_dependency();
	}
}

inline struct cmzn_field_change_detail *MANAGER_EXTRACT_CHANGE_DETAIL(cmzn_field)(
	struct cmzn_field *field)
{
	return field->core->extract_change_detail();
}

inline void MANAGER_CLEANUP_CHANGE_DETAIL(cmzn_field)(
	cmzn_field_change_detail **change_detail_address)
{
	delete *change_detail_address;
}

DECLARE_MANAGER_UPDATE_FUNCTION(cmzn_field)

DECLARE_MANAGER_FIND_CLIENT_FUNCTION(cmzn_field)

DECLARE_MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL_FUNCTION(cmzn_field)

/*
Global functions
----------------
*/

cmzn_field::cmzn_field() :
	name(nullptr),
	automaticName(false),
	cache_index(0),
	number_of_components(0),
	core(nullptr),
	number_of_source_fields(0),
	source_fields(nullptr),
	number_of_source_values(0),
	source_values(nullptr),
	fieldparameters(nullptr),
	manager(nullptr),
	manager_change_status(MANAGER_CHANGE_NONE(cmzn_field)),
	attribute_flags(0),
	access_count(1)
{
}

cmzn_field::~cmzn_field()
{
	// destroy core first as some e.g. apply need to remove callbacks
	delete this->core;
	this->core = nullptr;
	if (this->source_fields)
	{
		for (int i = 0; i < this->number_of_source_fields; ++i)
		{
			cmzn_field::deaccess(this->source_fields[i]);
		}
		DEALLOCATE(this->source_fields);
	}
	this->number_of_source_fields = 0;
	if (this->source_values)
	{
		DEALLOCATE(this->source_values);
	}
	this->number_of_source_values = 0;
    DEALLOCATE(this->name);
}

cmzn_field *cmzn_field::create(const char *nameIn)
{
	cmzn_field *field = new cmzn_field();
	if (nameIn)
	{
		field->name = duplicate_string(nameIn);
		if (!field->name)
		{
			display_message(ERROR_MESSAGE, "cmzn_field::create.  Could not copy name.");
			cmzn_field::deaccess(field);
		}
	}
	return field;
}

void cmzn_field::deaccess(cmzn_field*& field_ref)
{
	if (field_ref)
	{
		cmzn_field* field = field_ref;
		field_ref = nullptr; // clear client's pointer ASAP in case manager message sent below
		--(field->access_count);
		if (field->access_count <= 0)
		{
			delete field;
		}
		else if ((0 == (field->attribute_flags & COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT)) &&
			(field->manager) && ((1 == field->access_count) ||
			((2 == field->access_count) &&
				(MANAGER_CHANGE_NONE(cmzn_field) != field->manager_change_status))) &&
			field->core->not_in_use())
		{
			// removing a field from manager can release other fields it has reference to so cache changes:
			MANAGER(cmzn_field)* manager = field->manager;
			MANAGER_BEGIN_CACHE(cmzn_field)(manager);
			REMOVE_OBJECT_FROM_MANAGER(cmzn_field)(field, field->manager);
			MANAGER_END_CACHE(cmzn_field)(manager);
		}
	}
}

PROTOTYPE_ACCESS_OBJECT_FUNCTION(cmzn_field)
{
	if (object)
		return object->access();
	return nullptr;
}

PROTOTYPE_DEACCESS_OBJECT_FUNCTION(cmzn_field)
{
	if (object_address)
	{
		cmzn_field::deaccess(*object_address);
		return 1;
	}
	display_message(ERROR_MESSAGE, "DEACCESS(cmzn_field).  Invalid argument");
	return 0;
}

PROTOTYPE_REACCESS_OBJECT_FUNCTION(cmzn_field)
{
	if (object_address)
	{
		cmzn_field::reaccess(*object_address, new_object);
		return 1;
	}
	display_message(ERROR_MESSAGE, "REACCESS(cmzn_field).  Invalid argument");
	return 0;
}

cmzn_field_id cmzn_field_access(cmzn_field_id field)
{
	if (field)
		return field->access();
	return nullptr;
}

int cmzn_field_destroy(cmzn_field_id *field_address)
{
	if (field_address)
	{
		cmzn_field::deaccess(*field_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_fieldmodule_id cmzn_field_get_fieldmodule(cmzn_field_id field)
{
	struct cmzn_region *region = Computed_field_get_region(field);
	return cmzn_region_get_fieldmodule(region);
}

enum cmzn_field_value_type cmzn_field_get_value_type(cmzn_field_id field)
{
	if (field)
	{
		return field->getValueType();
	}
	return CMZN_FIELD_VALUE_TYPE_INVALID;
}

#if defined (DEBUG_CODE)
int cmzn_field_get_access_count(cmzn_field_id field)
{
	return field->access_count;
}
#endif /* defined (DEBUG_CODE) */

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(cmzn_field)
/*****************************************************************************
LAST MODIFIED : 6 September 2007

DESCRIPTION :
Forms a string out of the objects identifier.
If the name of the computed field is "constants" then this is a special field
and the values are listed in an array.  See set_Computed_field_conditional
in computed_field_set.cpp.
============================================================================*/
{
	int return_code = 1;
	if (object && name_ptr)
	{
		*name_ptr = duplicate_string(object->getName());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GET_NAME(cmzn_field).  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

DECLARE_INDEXED_LIST_STL_FUNCTIONS(cmzn_field)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL_FUNCTION(cmzn_field,name,const char *)

cmzn_fielditerator_id cmzn_fielditerator_access(cmzn_fielditerator_id iterator)
{
	if (iterator)
		return iterator->access();
	return 0;
}

int cmzn_fielditerator_destroy(cmzn_fielditerator_id *iterator_address)
{
	if (!iterator_address)
		return 0;
	return cmzn_fielditerator::deaccess(*iterator_address);
}

cmzn_field_id cmzn_fielditerator_next(cmzn_fielditerator_id iterator)
{
	if (iterator)
		return iterator->next();
	return 0;
}

cmzn_field_id cmzn_fielditerator_next_non_access(cmzn_fielditerator_id iterator)
{
	if (iterator)
		return iterator->next_non_access();
	return 0;
}

DECLARE_MANAGER_FUNCTIONS(cmzn_field, manager)

PROTOTYPE_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(cmzn_field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
cmzn_field requires a special version of this function mainly due to the
finite_element type which automatically wraps FE_fields. If the computed field
is not itself in use, it calls the field's optional computed_field_not_in_use
function and bases its result on that.
Note: assumes caller is accessing field once!
==============================================================================*/
{
	int return_code;

	ENTER(MANAGED_OBJECT_NOT_IN_USE(cmzn_field));
	return_code = 0;
	if (manager && object)
	{
		if (manager == object->manager)
		{
			if (((1 + extraAccesses) == object->access_count) ||
				((MANAGER_CHANGE_NONE(cmzn_field) != object->manager_change_status) &&
				 ((2 + extraAccesses) == object->access_count)))
			{
				return_code = object->core ? object->core->not_in_use() : 1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"MANAGED_OBJECT_NOT_IN_USE(cmzn_field).  Object is not in this manager");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGED_OBJECT_NOT_IN_USE(cmzn_field).  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* MANAGED_OBJECT_NOT_IN_USE(cmzn_field) */

DECLARE_ADD_OBJECT_TO_MANAGER_FUNCTION(cmzn_field,name,manager)

DECLARE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(cmzn_field, name, const char *)
DECLARE_CREATE_INDEXED_LIST_STL_ITERATOR_FUNCTION(cmzn_field,cmzn_fielditerator)

DECLARE_MANAGER_OWNER_FUNCTIONS(cmzn_field, struct cmzn_region)

char *Computed_field_manager_get_unique_field_name(
	struct MANAGER(cmzn_field) *manager, const char *part1,
	const char *part2, int startNumber)
{
	const size_t len = strlen(part1) + strlen(part2);
	char *fieldName = nullptr;
	ALLOCATE(fieldName, char, len + 20);
	if (!fieldName)
		return nullptr;
	sprintf(fieldName, "%s%s", part1, part2);
	int number = (startNumber < 0) ? NUMBER_IN_MANAGER(cmzn_field)(manager) + 1 : startNumber;
	do
	{
		if (number != 0)
		{
			sprintf(fieldName + len, "%d", number);
		}
		++number;
	}
	while (FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_field,name)(fieldName, manager));
	return fieldName;
}

cmzn_fielditerator_id Computed_field_manager_create_iterator(
	struct MANAGER(cmzn_field) *manager)
{
	if (manager)
		return CREATE_LIST_ITERATOR(cmzn_field)(manager->object_list);
	return 0;
}

cmzn_fielditerator_id Computed_field_list_create_iterator(
	struct LIST(cmzn_field) *list)
{
	return CREATE_LIST_ITERATOR(cmzn_field)(list);
}

int cmzn_field_get_cache_index_private(cmzn_field_id field)
{
	return field ? field->cache_index : 0;
}

int cmzn_field_set_cache_index_private(cmzn_field_id field, int cache_index)
{
	if (field && (cache_index >= 0))
	{
		field->cache_index = cache_index;
		return 1;
	}
	return 0;
}

int Computed_field_add_to_manager_private(struct cmzn_field *field,
	struct MANAGER(cmzn_field) *manager)
{
	int return_code;
	if (field && manager && (!field->manager))
	{
		if (field->name)
		{
			if (FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_field,name)(field->getName(), manager))
			{
				display_message(ERROR_MESSAGE, "Field create.  Field of name \"%s\" already exists", field->getName());
				return 0;
			}
		}
		else
		{
			field->setNameAutomatic(manager);
		}
        return_code = ADD_OBJECT_TO_MANAGER(cmzn_field)(field,manager);
		if (return_code)
		{
			// notify field types which need to perform extra tasks once added to region
			field->core->fieldAddedToRegion();
        }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_to_manager_private.  Invalid argument(s).");
		return_code = 0;
	}
	return return_code;
}

cmzn_field *Computed_field_create_generic(
	cmzn_fieldmodule *fieldmodule, bool check_source_field_regions,
	int number_of_components,
	int number_of_source_fields, cmzn_field **source_fields,
	int number_of_source_values, const double *source_values,
	Computed_field_core *field_core, const char *name)
{
	cmzn_field *field = nullptr;
	if ((NULL != fieldmodule) && (0 < number_of_components) &&
		((0 == number_of_source_fields) ||
			((0 < number_of_source_fields) && (NULL != source_fields))) &&
		((0 == number_of_source_values) ||
			((0 < number_of_source_values) && (NULL != source_values))) &&
		(NULL != field_core))
	{
		int return_code = 1;
		cmzn_region *region = cmzn_fieldmodule_get_region_internal(fieldmodule);
		for (int i = 0; i < number_of_source_fields; i++)
		{
			if (NULL != source_fields[i])
			{
				if (check_source_field_regions && (source_fields[i]->getRegion() != region))
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_create_generic.  Source field is from a different region");
					return_code = 0;
				}
            }
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_create_generic.  Missing source field");
				return_code = 0;
			}
		}
		if (return_code)
		{
            field = cmzn_field::create(name);
            if (field)
			{
				field->number_of_components = number_of_components;
				if (0 < number_of_source_fields)
				{
					ALLOCATE(field->source_fields, struct cmzn_field *, number_of_source_fields);
					if (NULL != field->source_fields)
					{
						field->number_of_source_fields = number_of_source_fields;
						for (int i = 0; i < number_of_source_fields; i++)
						{
							field->source_fields[i] = source_fields[i]->access();
						}
					}
					else
					{
						return_code = 0;
					}
				}
				if (0 < number_of_source_values)
				{
					ALLOCATE(field->source_values, FE_value, number_of_source_values);
					if (NULL != field->source_values)
					{
						field->number_of_source_values = number_of_source_values;
						for (int i = 0; i < number_of_source_values; i++)
						{
							field->source_values[i] = static_cast<FE_value>(source_values[i]);
						}
					}
					else
					{
						return_code = 0;
					}
				}
                if (return_code)
				{
					// 2nd stage of construction - can fail
					if (!field_core->attach_to_field(field))
					{
						return_code = 0;
					}
				}
                field->core = field_core;
				field_core = nullptr;  // now owned by field, clear so not destroyed below
				if (return_code)
				{
					// only some field types implement the following, e.g. set default
					// coordinate system of new field to that of a source field:
                    field->core->inherit_source_field_attributes();
                    if (!region->addField(field))
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_create_generic.  Unable to add field to region");
						return_code = 0;
					}
				}
                if (!return_code)
				{
					cmzn_field::deaccess(field);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_generic.  Invalid argument(s)");
	}
	delete field_core;
	return (field);
}

/** @return  True if field can be evaluated with supplied cache.
 * Must be called to check field evaluate/assign functions from external API.
 */
inline bool cmzn_fieldcache_check(cmzn_field_id field, cmzn_fieldcache_id cache)
{
	return (field && cache && (field->manager->owner == cache->getRegion()));
}

void cmzn_field::clearCaches()
{
	// some fields (integration, histogram) still have caches in field itself to clear:
	core->clear_cache();
	cmzn_region_id region = this->manager->owner;
	cmzn_set_cmzn_field *all_fields = reinterpret_cast<cmzn_set_cmzn_field *>(this->manager->object_list);
	for (cmzn_set_cmzn_field::iterator iter = all_fields->begin(); iter != all_fields->end(); iter++)
	{
		cmzn_field_id field = *iter;
		if (field->dependsOnField(this))
		{
			region->clearFieldValueCaches(field);
		}
	}
}

bool cmzn_field::compareFullDefinition(const cmzn_field& otherField) const
{
	if (this->compareBasicDefinition(otherField)
		&& (this->number_of_source_fields == otherField.number_of_source_fields)
		&& (this->number_of_source_values == otherField.number_of_source_values)
		&& (typeid(this->core) == typeid(otherField.core)))
	{
		for (int i = 0; i < this->number_of_source_fields; ++i)
		{
			if (this->source_fields[i] != otherField.source_fields[i])
			{
				return false;
			}
		}
		for (int i = 0; i < this->number_of_source_values; ++i)
		{
			if (this->source_values[i] != otherField.source_values[i])
			{
				return false;
			}
		}
		if (this->core->compare(otherField.core))
		{
			return true;
		}
	}
	return false;
}

int cmzn_field::copyDefinition(const cmzn_field& source)
{
	/* check <source> does not depend on <destination> else infinite loop */
	if (source.dependsOnField(this))
	{
		display_message(ERROR_MESSAGE, "cmzn_field::copyDefinition.  Cannot make field depend on itself");
		return CMZN_ERROR_ARGUMENT;
	}
	if ((source.manager) && (source.manager != this->manager))
	{
		display_message(ERROR_MESSAGE, "cmzn_field::copyDefinition.  Source is from another region");
		return CMZN_ERROR_ARGUMENT;
	}
	if (((source.number_of_components != this->number_of_components) ||
		(source.getValueType() != this->getValueType())) &&
		(!MANAGED_OBJECT_NOT_IN_USE(cmzn_field)(this, manager, 1)))
	{
		display_message(ERROR_MESSAGE, "cmzn_field::copyDefinition.  "
			"Cannot change number of components or value type while field is in use");
		return CMZN_ERROR_ARGUMENT;
	}
	if (!(this->core->not_in_use() || this->core->compareExact(source.core)))
	{
		display_message(ERROR_MESSAGE, "cmzn_field::copyDefinition.  "
			"Cannot replace definition of finite element field while defined on model");
		return CMZN_ERROR_IN_USE;
	}
	int return_code = CMZN_OK;
	cmzn_field **newSourceFields = nullptr;
	if (source.number_of_source_fields > 0)
	{
		if (!ALLOCATE(newSourceFields, cmzn_field *, source.number_of_source_fields))
		{
			return_code = CMZN_ERROR_MEMORY;
		}
	}
	FE_value *newSourceValues = nullptr;
	if (source.number_of_source_values > 0)
	{
		if (!ALLOCATE(newSourceValues, FE_value, source.number_of_source_values))
		{
			return_code = CMZN_ERROR_MEMORY;
		}
	}
	Computed_field_core *newCore = nullptr;
	if (source.core)
	{
		newCore = source.core->copy();
		if (!newCore)
		{
			return_code = CMZN_ERROR_MEMORY;
		}
	}
	else
	{
		return_code = CMZN_ERROR_ARGUMENT;
	}
	if (return_code == CMZN_OK)
	{
		//  cache changes as previous source fields may be removed by this modification
		MANAGER_BEGIN_CACHE(cmzn_field)(this->manager);
		// save and clear old source fields last as may cause intermediate fields to be cleaned up
		cmzn_field **oldSourceFields = this->source_fields;
		const int oldNumberOfSourceFields = this->number_of_source_fields;
		// make the copy
		this->coordinate_system = source.coordinate_system;
		this->number_of_components = source.number_of_components;
		this->number_of_source_fields = source.number_of_source_fields;
		for (int i = 0; i < source.number_of_source_fields; i++)
		{
			newSourceFields[i] = source.source_fields[i]->access();
		}
		this->source_fields = newSourceFields;
		this->number_of_source_values = source.number_of_source_values;
		for (int i = 0; i < source.number_of_source_values; i++)
		{
			newSourceValues[i] = source.source_values[i];
		}
		DEALLOCATE(this->source_values);
		this->source_values = newSourceValues;
		delete this->core;
		this->core = newCore;
		if (!this->core->attach_to_field(this))
		{
			display_message(ERROR_MESSAGE, "cmzn_field::copyDefinition.  Failed to attach core to field.");
			return_code = CMZN_ERROR_GENERAL;
		}
		// notify of change and clean up old source fields
		this->setChanged();
		if (oldSourceFields)
		{
			for (int i = 0; i < oldNumberOfSourceFields; ++i)
			{
				cmzn_field::deaccess(oldSourceFields[i]);
			}
			DEALLOCATE(oldSourceFields);
		}
		MANAGER_END_CACHE(cmzn_field)(this->manager);
	}
	else
	{
		display_message(ERROR_MESSAGE, "cmzn_field::copyDefinition.  Failed to copy source");
		if (newSourceFields)
		{
			DEALLOCATE(newSourceFields);
		}
		if (newSourceValues)
		{
			DEALLOCATE(newSourceValues);
		}
		delete newCore;
	}
	return return_code;
}

cmzn_fieldparameters *cmzn_field::getFieldparameters()
{
	if (this->fieldparameters)
		return this->fieldparameters->access();
	this->fieldparameters = cmzn_fieldparameters::create(this);
	return this->fieldparameters;
}

bool cmzn_field::isResultChanged()
{
	if ((this->manager_change_status & MANAGER_CHANGE_RESULT(Computed_field))
		|| this->core->isResultChanged())
	{
		return true;
	}
	for (int i = 0; i < this->number_of_source_fields; ++i)
	{
		if (this->source_fields[i]->isResultChanged())
		{
			return true;
		}
	}
	return false;
}

int cmzn_field::setCoordinateSystem(const Coordinate_system& coordinateSystemIn, bool notifyChange)
{
	if (coordinateSystemIn.type != NOT_APPLICABLE)
	{
		const cmzn_field_value_type valueType = this->getValueType();
		if ((valueType == CMZN_FIELD_VALUE_TYPE_STRING) ||
			(valueType == CMZN_FIELD_VALUE_TYPE_MESH_LOCATION))
		{
			display_message(ERROR_MESSAGE,
				"Field setCoordinateSystem.  Non-numeric fields may only have coordinate system type NOT_APPLICABLE");
			return CMZN_ERROR_ARGUMENT;
		}
	}
	if (!(this->coordinate_system == coordinateSystemIn))
	{
		this->coordinate_system = coordinateSystemIn;
		// propagate to wrapped FE_field or other object
		this->core->propagate_coordinate_system();
		if (notifyChange)
		{
			this->setChanged();
		}
	}
	return CMZN_OK;
}

FE_value cmzn_field::getCoordinateSystemFocus() const
{
	return this->coordinate_system.parameters.focus;
}

int cmzn_field::setCoordinateSystemFocus(FE_value focus)
{
	if (focus <= 0.0)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	Coordinate_system tmpCoordinateSystem(this->coordinate_system.type, focus);
	return this->setCoordinateSystem(tmpCoordinateSystem);
}

cmzn_field_coordinate_system_type cmzn_field::getCoordinateSystemType() const
{
	cmzn_field_coordinate_system_type coordinateSystemType = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_INVALID;
	switch (this->coordinate_system.type)
	{
	case RECTANGULAR_CARTESIAN:
		coordinateSystemType = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN;
		break;
	case CYLINDRICAL_POLAR:
		coordinateSystemType = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_CYLINDRICAL_POLAR;
		break;
	case SPHERICAL_POLAR:
		coordinateSystemType = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR;
		break;
	case PROLATE_SPHEROIDAL:
		coordinateSystemType = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL;
		break;
	case OBLATE_SPHEROIDAL:
		coordinateSystemType = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL;
		break;
	case FIBRE:
		coordinateSystemType = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_FIBRE;
		break;
	case NOT_APPLICABLE:
		coordinateSystemType = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE;
		break;
	default:
		break;
	}
	return coordinateSystemType;
}

int cmzn_field::setCoordinateSystemType(cmzn_field_coordinate_system_type coordinateSystemType)
{
	Coordinate_system_type type = UNKNOWN_COORDINATE_SYSTEM;
	switch (coordinateSystemType)
	{
	case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_INVALID:
		break;
	case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN:
		type = RECTANGULAR_CARTESIAN;
		break;
	case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_CYLINDRICAL_POLAR:
		type = CYLINDRICAL_POLAR;
		break;
	case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR:
		type = SPHERICAL_POLAR;
		break;
	case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL:
		type = PROLATE_SPHEROIDAL;
		break;
	case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL:
		type = OBLATE_SPHEROIDAL;
		break;
	case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_FIBRE:
		type = FIBRE;
		break;
	case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE:
		type = NOT_APPLICABLE;
		break;
	}
	if (type == UNKNOWN_COORDINATE_SYSTEM)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	Coordinate_system tmpCoordinateSystem(type, this->coordinate_system.parameters.focus);
	return this->setCoordinateSystem(tmpCoordinateSystem);
}

int cmzn_field::setName(const char *nameIn)
{
	if (!nameIn)
	{
		display_message(ERROR_MESSAGE, "Field setName.  Missing name");
		return CMZN_ERROR_ARGUMENT;
	}
	if ((this->name) && (0 == strcmp(this->name, nameIn)))
	{
		// no change, but clear automaticName as it has been explicitly set
		this->automaticName = false;
		return CMZN_OK;
	}
	int returnCode = CMZN_OK;
	cmzn_set_cmzn_field *managerFieldList = nullptr;
	bool restoreObjectToLists = false;
	if (this->manager)
	{
		cmzn_field *existingField = FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_field, name)(nameIn, this->manager);
		if (existingField)
		{
			if ((!existingField->hasAutomaticName()) || (CMZN_OK != existingField->setNameAutomatic()))
			{
				display_message(ERROR_MESSAGE,
					"Field setName.  Field named \"%s\" already exists in region.", nameIn);
				return CMZN_ERROR_ALREADY_EXISTS;
			}
		}
		// this temporarily removes the object from all related lists
		managerFieldList = reinterpret_cast<cmzn_set_cmzn_field *>(this->manager->object_list);
		restoreObjectToLists = managerFieldList->begin_identifier_change(this);
		if (!restoreObjectToLists)
		{
			display_message(ERROR_MESSAGE,
				"Field setName.  Could not safely change identifier in manager");
			returnCode = CMZN_ERROR_GENERAL;
		}
	}
	if (returnCode)
	{
		char *newName = duplicate_string(nameIn);
		if (newName)
		{
			DEALLOCATE(this->name);
			this->name = newName;
			this->automaticName = false;  // note set to true in setNameAutomatic()
		}
		else
		{
			returnCode = CMZN_ERROR_MEMORY;
		}
	}
	if (restoreObjectToLists)
	{
		managerFieldList->end_identifier_change();
	}
	if (CMZN_OK == returnCode)
	{
		if (this->manager)
		{
			// allow core type to change nameIn of wrapped objects e.g. FE_field
			// only do this once field is in manager otherwise complicates replacing dummy fields
			// begin/end cache to avoid two messages if core implements set_name
			MANAGER_BEGIN_CACHE(cmzn_field)(this->manager);
			MANAGED_OBJECT_CHANGE(cmzn_field)(this, MANAGER_CHANGE_IDENTIFIER(cmzn_field));
			this->core->set_name(nameIn);
			MANAGER_END_CACHE(cmzn_field)(this->manager);
		}
	}
	return returnCode;
}

int cmzn_field::setNameAutomatic(MANAGER(cmzn_field) *fieldManager)
{
	if (!((this->manager) || (fieldManager)))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const char *oldName = this->getName() ? duplicate_string(this->getName()) : nullptr;
	char *uniqueName = Computed_field_manager_get_unique_field_name((this->manager) ? this->manager : fieldManager);
	const int returnCode = this->setName(uniqueName);  // will set this->automaticName = false
	if (CMZN_OK == returnCode)
	{
		if (this->manager)
		{
			const char *regionPath = this->getRegion()->getPath();
			display_message(WARNING_MESSAGE, "Changing automatic name of field %s to %s in region %s%s", oldName, uniqueName, CMZN_REGION_PATH_SEPARATOR_STRING, regionPath);
			DEALLOCATE(regionPath);
		}
		this->automaticName = true;
	}
	DEALLOCATE(uniqueName);
	if (oldName)
		DEALLOCATE(oldName);
	return returnCode;
}

int cmzn_field::setNameUnique(const char *part1, const char *part2, int startNumber)
{
	char *uniqueName = Computed_field_manager_get_unique_field_name(this->manager, part1, part2, startNumber);
	const int returnCode = this->setName(uniqueName);
	DEALLOCATE(uniqueName);
	return returnCode;
}

int cmzn_field::setSourceField(int index, cmzn_field *sourceField, bool notifyChange)
{
	if ((index < 0) || (index > this->number_of_source_fields))
	{
		display_message(ERROR_MESSAGE, "cmzn_field::setSourceField  Invalid arguments");
		return CMZN_ERROR_ARGUMENT;
	}
	bool changed = false;
	if (sourceField)
	{
		if (index == this->number_of_source_fields)
		{
			cmzn_field **tmp;
			REALLOCATE(tmp, this->source_fields, cmzn_field *, index + 1);
			if (!tmp)
				return CMZN_ERROR_MEMORY;
			tmp[index] = sourceField->access();
			this->source_fields = tmp;
			++(this->number_of_source_fields);
			changed = true;
		}
		else if (sourceField != this->source_fields[index])
		{
			cmzn_field::reaccess(this->source_fields[index], sourceField);
			changed = true;
		}
	}
	else if (index != this->number_of_source_fields)
	{
		cmzn_field::deaccess(this->source_fields[index]);
		--(this->number_of_source_fields);
		for (int i = index; i < this->number_of_source_fields; ++i)
		{
			this->source_fields[i] = this->source_fields[i + 1];
		}
		changed = true;
	}
	if (changed && notifyChange)
	{
		this->setChanged();
	}
	return CMZN_OK;
}

int Computed_field_is_defined_in_element(struct cmzn_field *field,
	struct FE_element *element)
{
	int return_code = 0;
	if (field && element)
	{
		cmzn_fieldmodule_id fieldmodule = cmzn_field_get_fieldmodule(field);
		cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(fieldmodule);
		cmzn_fieldcache_set_element(field_cache, element);
		return_code = cmzn_field_is_defined_at_location(field, field_cache);
		cmzn_fieldcache_destroy(&field_cache);
		cmzn_fieldmodule_destroy(&fieldmodule);
	}
	return return_code;
}

int Computed_field_is_defined_in_element_conditional(
	struct cmzn_field *field, void *element_void)
{
	return Computed_field_is_defined_in_element(field, static_cast<FE_element*>(element_void));
}

int Computed_field_is_defined_at_node(struct cmzn_field *field,
	struct FE_node *node)
{
	int return_code = 0;
	if (field && node)
	{
		cmzn_fieldmodule_id fieldmodule = cmzn_field_get_fieldmodule(field);
		cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(fieldmodule);
		cmzn_fieldcache_set_node(field_cache, node);
		return_code = cmzn_field_is_defined_at_location(field, field_cache);
		cmzn_fieldcache_destroy(&field_cache);
		cmzn_fieldmodule_destroy(&fieldmodule);
	}
	return return_code;
}

int Computed_field_is_defined_at_node_conditional(struct cmzn_field *field,
	void *node_void)
{
	return Computed_field_is_defined_at_node(field, static_cast<FE_node*>(node_void));
}

char *Computed_field_core::getComponentName(int componentNumber) const
{
	char name[24];
	sprintf(name, "%d", componentNumber);
	return duplicate_string(name);
}

FieldValueCache *Computed_field_core::createValueCache(cmzn_fieldcache& /*fieldCache*/)
{
	return new RealFieldValueCache(field->number_of_components);
}

/** @return  true if all source fields are defined at cache location */
bool Computed_field_core::is_defined_at_location(cmzn_fieldcache& cache)
{
	for (int i = 0; i < field->number_of_source_fields; ++i)
	{
		if (!this->getSourceField(i)->core->is_defined_at_location(cache))
			return false;
	}
	return true;
}

int Computed_field_core::evaluateDerivativeFiniteDifference(cmzn_fieldcache& cache, RealFieldValueCache& valueCache, const FieldDerivative& fieldDerivative)
{
	const FE_mesh *mesh = fieldDerivative.getMesh();
	const Field_location_element_xi* element_xi_location = cache.get_location_element_xi();
	const Field_location_node* node_location = nullptr;
	cmzn_element *element = nullptr;
	const FE_value *xi = nullptr;
	cmzn_node *node = nullptr;
	if (element_xi_location)
	{
		element = element_xi_location->get_element();
		xi = element_xi_location->get_xi();
	}
	else if ((node_location = cache.get_location_node()))
	{
		node = node_location->get_node();
		element = node_location->get_host_element();
		if (mesh)
		{
			display_message(ERROR_MESSAGE,
				"Field evaluateDerivativeFiniteDifference:  Cannot evaluate mesh derivatives at embedded node locations");
			return 0;
		}
	}
	if (!element)
	{
		display_message(ERROR_MESSAGE,
			"Field evaluateDerivativeFiniteDifference:  Only implemented for element location or node location with host element");
		return 0;
	}
	if ((mesh) && (element->getMesh() != mesh))
	{
		display_message(ERROR_MESSAGE,
			"Field evaluateDerivativeFiniteDifference:  Only implemented for derivative and location on the same mesh");
		return 0;
	}
	// evaluate field at perturbed locations in extra working cache
	cmzn_fieldcache *workingCache = valueCache.getOrCreateSharedExtraCache(cache);
	RealFieldValueCache* lowerValueCache;
	if ((0 == workingCache) || (0 == (lowerValueCache = static_cast<RealFieldValueCache*>(this->field->getValueCache(*workingCache)))))
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_core::evaluateDerivativeFiniteDifference.  Could not get working value cache");
		return 0;
	}
	workingCache->setTime(cache.getTime());
	const FieldDerivative *lowerFieldDerivative = fieldDerivative.getLowerDerivative();
	const int componentCount = this->field->number_of_components;
	FE_value perturbedXi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	// values set differently for parameter or mesh derivative applied:
	FE_value perturbationDelta;
	int derivativeCount;
	FE_element_field_evaluation *parameterFieldEvaluation = nullptr;
	cmzn_fieldparameters *fieldParameters = fieldDerivative.getFieldparameters();
	if (fieldParameters)
	{
		// parameter derivatives are always applied first here so outside/after mesh derivatives
		perturbationDelta = fieldParameters->getPerturbationDelta();
		derivativeCount = fieldParameters->getNumberOfElementParameters(element);
		if (derivativeCount <= 0)  // GRC or is zero a success?
			return 0;
		// need to set element location first to obtain FE_element_field_evaluation for
		if (node_location)
		{
			const int elementDimension = element->getDimension();
			// need any valid xi coordinate
			for (int d = 0; d < elementDimension; ++d)
				perturbedXi[d] = 0.0;
			xi = perturbedXi;
		}
		workingCache->setMeshLocation(element, xi);
		parameterFieldEvaluation = cmzn_field_get_cache_FE_element_field_evaluation(fieldParameters->getField(), workingCache);
		if (!parameterFieldEvaluation)
		{
			display_message(ERROR_MESSAGE, "Field evaluateDerivativeFiniteDifference:  Failed to get parameter field evaluation");
			return 0;
		}
	}
	else
	{
		const int elementDimension = mesh->getDimension();
		for (int d = 0; d < elementDimension; ++d)
			perturbedXi[d] = xi[d];
		perturbationDelta = 1.0E-5;
		derivativeCount = elementDimension;
	}
	const FE_value plusDelta = +perturbationDelta;
	const FE_value minusDelta = -perturbationDelta;
	const FE_value weight = 0.5/perturbationDelta;
	DerivativeValueCache &derivativeValueCache = *valueCache.getDerivativeValueCache(fieldDerivative);
	const FE_value *lowerValues = nullptr;
	for (int d = 0; d < derivativeCount; ++d)
	{
		// -/+ perturbation of lower derivative
		for (int k = 0; k < 2; ++k)
		{
			if (fieldParameters)
			{
				parameterFieldEvaluation->addParameterPerturbation(/*parameterIndex*/d, (k == 0) ? minusDelta : plusDelta);
				// the following must call locationChanged:
				if (element_xi_location)
					workingCache->setMeshLocation(element, xi);
				else
					workingCache->setNodeWithHostElement(node, element);
			}
			else  // (mesh)
			{
				perturbedXi[d] += (k == 0) ? minusDelta : plusDelta;
				workingCache->setMeshLocation(element, perturbedXi);
			}
			int lowerDerivativeTermCount = 1;
			if (lowerFieldDerivative)
			{
				const DerivativeValueCache *derivativeCache = this->field->evaluateDerivative(*workingCache, *lowerFieldDerivative);
				if (derivativeCache)
				{
					lowerValues = derivativeCache->values;
					lowerDerivativeTermCount = derivativeCache->getTermCount();
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Field evaluateDerivativeFiniteDifference:  Could not evaluate field lower derivative values");
					lowerValues = nullptr;
				}
			}
			else
			{
				if (this->field->evaluate(*workingCache))
				{
					lowerValues = lowerValueCache->values;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Field evaluateDerivativeFiniteDifference:  Could not evaluate field values");
					lowerValues = nullptr;
				}
			}
			if (lowerValues)
			{
				// values cycle over components slowest, then lower derivatives, fastest over this finite difference derivative
				for (int c = 0; c < componentCount; ++c)
				{
					const FE_value *src = lowerValues + c*lowerDerivativeTermCount;
					FE_value *dst = derivativeValueCache.values + c*derivativeCount*lowerDerivativeTermCount + d;
					if (k == 0)
					{
						for (int v = 0; v < lowerDerivativeTermCount; ++v)
						{
							*dst = src[v];
							dst += derivativeCount;
						}
					}
					else
					{
						for (int v = 0; v < lowerDerivativeTermCount; ++v)
						{
							*dst = weight*(src[v] - *dst);
							dst += derivativeCount;
						}
					}
				}
			}
			if (fieldParameters)
				parameterFieldEvaluation->removeParameterPerturbation();
			else
				perturbedXi[d] = xi[d];
			if (!lowerValues)
				return 0;  // can only return here now parameter perturbation is removed
		}
	}
	return 1;
}

// default valid for most complicated or transcendental functions:
// use the maximum source field order, maximised up to the mesh order or total order
int Computed_field_core::getDerivativeTreeOrder(const FieldDerivative& fieldDerivative)
{
	int order = 0;
	for (int i = 0; i < this->field->number_of_source_fields; ++i)
	{
		const int sourceMaximumOrder = fieldDerivative.getMaximumTreeOrder(this->field->source_fields[i]->getDerivativeTreeOrder(fieldDerivative));
		if (sourceMaximumOrder > order)
			order = sourceMaximumOrder;
	}
	return order;
}

int Computed_field_has_string_value_type(struct cmzn_field *field,
	void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	return (cmzn_field_get_value_type(field) == CMZN_FIELD_VALUE_TYPE_STRING);
}

int Computed_field_core::has_multiple_times()
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns 1 if any of the source fields have multiple times.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_default_has_multiple_times);
	if (field)
	{
		return_code=0;
		for (i=0;(i<field->number_of_source_fields)&&(!return_code);i++)
		{
			if (Computed_field_has_multiple_times(field->source_fields[i]))
			{
				return_code=1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_has_multiple_times */

int Computed_field_for_each_ancestor_same_region(struct cmzn_field *field,
	LIST_ITERATOR_FUNCTION(cmzn_field) *iterator_function, void *user_data)
{
	int return_code = 0;
	if ((field) && (iterator_function))
	{
		return_code = (iterator_function)(field, user_data);
		for (int i = 0; (i < field->number_of_source_fields) && return_code; i++)
		{
			if (field->source_fields[i]->manager == field->manager)
			{
				return_code = Computed_field_for_each_ancestor_same_region(
					field->source_fields[i], iterator_function, user_data);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_for_each_ancestor_same_region.  Invalid argument(s)");
	}
	return (return_code);
}

// External API
int cmzn_field_assign_mesh_location(cmzn_field_id field,
	cmzn_fieldcache_id cache, cmzn_element_id element,
	int number_of_chart_coordinates, const double *chart_coordinates)
{
	if (cmzn_fieldcache_check(field, cache) && element && chart_coordinates &&
		(number_of_chart_coordinates >= get_FE_element_dimension(element)) &&
		(CMZN_FIELD_VALUE_TYPE_MESH_LOCATION == cmzn_field_get_value_type(field)))
	{
		MeshLocationFieldValueCache *valueCache = MeshLocationFieldValueCache::cast(field->getValueCache(*cache));
		valueCache->setMeshLocation(element, chart_coordinates);
		enum FieldAssignmentResult result = field->assign(*cache, *valueCache);
		if (result != FIELD_ASSIGNMENT_RESULT_FAIL)
			return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

// External API
int cmzn_field_assign_real(cmzn_field_id field, cmzn_fieldcache_id cache,
	int number_of_values, const double *values)
{
	if (cmzn_fieldcache_check(field, cache) && field->isNumerical() &&
		(number_of_values >= field->number_of_components) && values)
	{
		RealFieldValueCache *valueCache = RealFieldValueCache::cast(field->getValueCache(*cache));
		valueCache->setValues(values);
		enum FieldAssignmentResult result = field->assign(*cache, *valueCache);
		if (result != FIELD_ASSIGNMENT_RESULT_FAIL)
			return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

// External API
int cmzn_field_assign_string(cmzn_field_id field, cmzn_fieldcache_id cache,
	const char *string_value)
{
	if (cmzn_fieldcache_check(field, cache) && string_value &&
		(CMZN_FIELD_VALUE_TYPE_STRING == cmzn_field_get_value_type(field)))
	{
		StringFieldValueCache *valueCache = StringFieldValueCache::cast(field->getValueCache(*cache));
		valueCache->setString(string_value);
		enum FieldAssignmentResult result = field->assign(*cache, *valueCache);
		if (result != FIELD_ASSIGNMENT_RESULT_FAIL)
			return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_field_depends_on_field(cmzn_field_id field,
	cmzn_field_id other_field)
{
	if ((field) && (other_field))
	{
		return field->dependsOnField(other_field);
	}
	return false;
}

// Internal function
// Note this also returns false for undefined. Not ready to expose in
// external API until this is deemed reasonable.
bool cmzn_field_evaluate_boolean(cmzn_field_id field, cmzn_fieldcache_id cache)
{
	const FE_value zero_tolerance = 1e-6;
	if (cmzn_fieldcache_check(field, cache) && field->isNumerical())
	{
		const FieldValueCache *valueCache = field->evaluate(*cache);
		if (valueCache)
		{
			const FE_value *sourceValues = RealFieldValueCache::cast(valueCache)->values;
			for (int i = 0; i < field->number_of_components; ++i)
			{
				if ((sourceValues[i] < -zero_tolerance) ||
					(sourceValues[i] > zero_tolerance))
				{
					return true;
				}
			}
		}
	}
	return false;
}

// External API
cmzn_element_id cmzn_field_evaluate_mesh_location(cmzn_field_id field,
	cmzn_fieldcache_id cache, int number_of_chart_coordinates,
	double *chart_coordinates)
{
	cmzn_element_id element = 0;
	if (cmzn_fieldcache_check(field, cache) && chart_coordinates &&
		(CMZN_FIELD_VALUE_TYPE_MESH_LOCATION == cmzn_field_get_value_type(field)))
	{
		const FieldValueCache *valueCache = field->evaluate(*cache);
		if (valueCache)
		{
			const MeshLocationFieldValueCache& meshLocationValueCache =
				MeshLocationFieldValueCache::cast(*valueCache);
			int dimension = get_FE_element_dimension(meshLocationValueCache.element);
			if (number_of_chart_coordinates >= dimension)
			{
				for (int i = 0; i < dimension; i++)
				{
					chart_coordinates[i] = meshLocationValueCache.xi[i];
				}
				element = ACCESS(FE_element)(meshLocationValueCache.element);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_field_evaluate_mesh_location.  Invalid argument(s)");
	}
	return element;
}

// External API
// Note: no warnings if not evaluated so can be used for is_defined
int cmzn_field_evaluate_real(cmzn_field_id field, cmzn_fieldcache_id cache,
	int number_of_values, double *values)
{
	if (cmzn_fieldcache_check(field, cache) && (number_of_values >= field->number_of_components) && values &&
		field->core->has_numerical_components())
	{
		const FieldValueCache *valueCache = field->evaluate(*cache);
		if (valueCache)
		{
			const FE_value *sourceValues = RealFieldValueCache::cast(valueCache)->values;
			for (int i = 0; i < field->number_of_components; ++i)
			{
				values[i] = sourceValues[i];
			}
			return CMZN_OK;
		}
		return CMZN_ERROR_GENERAL;
	}
	return CMZN_ERROR_ARGUMENT;
}

/** Internal function only. Evaluate real field values with all first derivatives w.r.t. xi.
 * @deprecated
 * Try to remove its use as soon as possible.
 */
int cmzn_field_evaluate_real_with_derivatives(cmzn_field_id field,
	cmzn_fieldcache_id cache, int number_of_values, double *values,
	int number_of_derivatives, double *derivatives)
{
	const Field_location_element_xi* element_xi_location = cache->get_location_element_xi();
	if (!element_xi_location)
	{
		display_message(ERROR_MESSAGE, "cmzn_field_evaluate_real_with_derivatives.  Requires element_xi location");
		return CMZN_ERROR_ARGUMENT;
	}
	int result = cmzn_field_evaluate_real(field, cache, number_of_values, values);
	if (result != CMZN_OK)
		return result;
	const FieldDerivative *fieldDerivative = element_xi_location->get_element()->getMesh()->getFieldDerivative(/*order*/1);
	const int termCount = fieldDerivative->getMeshTermCount();
	if ((number_of_derivatives != termCount) || (!derivatives))
	{
		result = CMZN_ERROR_ARGUMENT;
	}
	else
	{
		const DerivativeValueCache *derivativeValueCache = field->evaluateDerivative(*cache, *fieldDerivative);
		if (!derivativeValueCache)
		{
			result = CMZN_ERROR_GENERAL;
		}
		else
		{
			const int valueCount = field->number_of_components*termCount;
			const FE_value *srcValues = derivativeValueCache->values;
			for (int v = 0; v < valueCount; ++v)
				derivatives[v] = srcValues[v];
			result = CMZN_OK;
		}
	}
	return result;
}

// External API
// Note: no warnings if not evaluated so can be used for is_defined
char *cmzn_field_evaluate_string(cmzn_field_id field,
	cmzn_fieldcache_id cache)
{
	if (cmzn_fieldcache_check(field, cache))
	{
		const FieldValueCache *valueCache = field->evaluate(*cache);
		if (valueCache)
			return valueCache->getAsString();
	}
	return 0;
}

// External API
int cmzn_field_evaluate_derivative(cmzn_field_id field,
	cmzn_differentialoperator_id differential_operator,
	cmzn_fieldcache_id cache, int number_of_values, double *values)
{
	if (cmzn_fieldcache_check(field, cache)
		&& (differential_operator)
		&& (number_of_values > 0) && (values)
		&& field->core->has_numerical_components())
	{
		FieldDerivative& fieldDerivative = differential_operator->getFieldDerivative();
		if (field->manager->owner != fieldDerivative.getRegion())
			return CMZN_ERROR_ARGUMENT;
		const DerivativeValueCache *derivativeValueCache = field->evaluateDerivative(*cache, fieldDerivative);
		if (derivativeValueCache)
		{
			// with some derivatives, don't know number of terms until evaluated, so check here:
			const int componentCount = field->number_of_components;
			const int termCount = derivativeValueCache->getTermCount();
			const int term = differential_operator->getTerm();
			if (term < 0)
			{
				const int valueCount = componentCount*termCount;
				if (number_of_values < valueCount)
				{
					display_message(ERROR_MESSAGE, "Field evaluateDerivative.  Too few values requested");
					return CMZN_ERROR_ARGUMENT;
				}
				const FE_value *derivatives = derivativeValueCache->values;
				for (int v = 0; v < valueCount; ++v)
					values[v] = derivatives[v];
			}
			else
			{
				if (number_of_values < componentCount)
				{
					display_message(ERROR_MESSAGE, "Field evaluateDerivative.  Too few values requested");
					return CMZN_ERROR_ARGUMENT;
				}
				const FE_value *derivatives = derivativeValueCache->values + term;
				for (int c = 0; c < componentCount; ++c)
					values[c] = derivatives[c*termCount];
			}
			return CMZN_OK;
		}
		return CMZN_ERROR_GENERAL;
	}
	display_message(ERROR_MESSAGE, "Field evaluateDerivative.  Invalid arguments");
	return CMZN_ERROR_ARGUMENT;
}

// External API
int cmzn_field_evaluate_fieldrange(cmzn_field_id field,
	cmzn_fieldcache_id fieldcache, cmzn_fieldrange_id fieldrange)
{
	if (fieldrange)
	{
		return fieldrange->evaluateRange(field, fieldcache);
	}
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_field_is_defined_at_location(cmzn_field_id field,
	cmzn_fieldcache_id cache)
{
	if (cmzn_fieldcache_check(field, cache))
		return field->core->is_defined_at_location(*cache);
	return false;
}

int Computed_field_get_native_discretization_in_element(
	struct cmzn_field *field,struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
If the <field> or its source field is grid-based in <element>, returns in
<number_in_xi> the numbers of finite difference cells in each xi-direction of
<element>. Note that this number is one less than the number of grid points in
each direction. <number_in_xi> should be allocated with at least as much space
as the number of dimensions in <element>, but is assumed to have no more than
MAXIMUM_ELEMENT_XI_DIMENSIONS so that
int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] can be passed to this function.
Returns 0 with no errors if the field is not grid-based.
In particular, make sure all the same field types are supported here and in
Computed_field_set_values_in_[managed_]element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_native_discretization_in_element);
	if (field && element && number_in_xi &&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS >= get_FE_element_dimension(element)))
	{
		return_code =
			field->core->get_native_discretization_in_element(element, number_in_xi);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_native_discretization_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_native_discretization_in_element */

int Computed_field_core::get_native_discretization_in_element(
	struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Inherits its result from the first source field -- if any.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_get_native_discretization_in_element);
	if (field&&element&&number_in_xi&&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS>=get_FE_element_dimension(element)))
	{
		if (field->source_fields && (0 < field->number_of_source_fields))
		{
			return_code=Computed_field_get_native_discretization_in_element(
				field->source_fields[0],element,number_in_xi);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_get_native_discretization_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_get_native_discretization_in_element */

int recursively_add_source_fields_to_list( struct cmzn_field *field, struct LIST(cmzn_field) *field_list )
{
	int return_code = 1;
	if ( field )
	{
		ADD_OBJECT_TO_LIST(cmzn_field)(field,field_list);
		for (int i=0;(i<field->number_of_source_fields)&&(!return_code);i++)
		{
			recursively_add_source_fields_to_list(field->source_fields[i],field_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"recursively_add_sourcefields_to_list.  Invalid argument(s)");
		return_code=0;
	}

	return return_code;
}

int Computed_field_is_coordinate_field(struct cmzn_field *field, void *not_in_use)
{
	USE_PARAMETER(not_in_use);
	return (cmzn_field_is_type_coordinate(field) &&
		Computed_field_has_up_to_3_numerical_components(field, 0));
}

int Computed_field_get_domain( struct cmzn_field *field, struct LIST(cmzn_field) *domain_field_list )
{
	int return_code = 0;
	if (field && domain_field_list)
	{
		return_code = field->core->get_domain( domain_field_list );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_domain.  Invalid argument(s)");
	}
	return return_code;
}

int Computed_field_is_non_linear(struct cmzn_field *field)
{
	int return_code = 0;
	if (field)
	{
		return_code = field->core->is_non_linear();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_non_linear.  Invalid argument(s)");
	}

	return return_code;
}

int cmzn_field_get_number_of_components(cmzn_field_id field)
{
	if (field)
		return field->number_of_components;
	display_message(ERROR_MESSAGE, "cmzn_field_get_number_of_components.  Missing field");
	return 0;
}

const char *Computed_field_get_type_string(struct cmzn_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns the string which identifies the type.
The calling function must not deallocate the returned string.
==============================================================================*/
{
	const char *return_string;

	ENTER(Computed_field_get_type_string);
	if (field)
	{
		if (field->core)
		{
			return_string = field->core->get_type_string();
		}
		else
		{
			return_string = (const char *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_get_type_string.  Missing field");
		return_string = (const char *)NULL;
	}
	LEAVE;

	return (return_string);
} /* Computed_field_get_type_string */

int Computed_field_get_native_resolution(struct cmzn_field *field,
	int *dimension, int **sizes,
	struct cmzn_field **texture_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Gets the <dimension>, <sizes>, <minimums>, <maximums> and <texture_coordinate_field> from
the <field>. These parameters will be used in image processing.

==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_native_resolution);
	if (field)
	{
		return_code = field->core->get_native_resolution(dimension, sizes,
			texture_coordinate_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_native_resolution.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_get_native_resolution */

int Computed_field_core::get_native_resolution(
	int *dimension, int **sizes,
	struct cmzn_field **texture_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Inherits its result from the first source field that returns it-- if any.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_default_get_native_resolution);
	if (field&&dimension&&sizes&&texture_coordinate_field)
	{
		if (field->source_fields && (0 < field->number_of_source_fields))
		{
			i = 0;
			do
			{
				return_code=Computed_field_get_native_resolution(
					field->source_fields[i],dimension, sizes,
					texture_coordinate_field);
				i++;
			}
			while ((!(*sizes))	&& i < field->number_of_source_fields);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_get_native_resolution.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_get_native_resolution */

int Computed_field_has_value_type_mesh_location(struct cmzn_field *field,
	void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	return (cmzn_field_get_value_type(field) == CMZN_FIELD_VALUE_TYPE_MESH_LOCATION);
}

int Computed_field_has_numerical_components(struct cmzn_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Conditional function returning true if <field> returns numerical components.
Note that whether the numbers were integer, FE_value or double, they may be
returned as FE_value when evaluated.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_numerical_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code = field->core->has_numerical_components();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_numerical_components */

int Computed_field_is_scalar(struct cmzn_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Conditional function returning true if <field> has 1 component and it is
numerical.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_scalar);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(1 == field->number_of_components)&&
			Computed_field_has_numerical_components(field,(void *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_is_scalar.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_scalar */

int Computed_field_has_up_to_3_numerical_components(
	struct cmzn_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Conditional function returning true if <field> has up to 3 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_up_to_3_numerical_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(3 >= field->number_of_components)&&
			Computed_field_has_numerical_components(field,(void *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_up_to_3_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_up_to_3_numerical_components */

int Computed_field_has_up_to_4_numerical_components(
	struct cmzn_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Conditional function returning true if <field> has up to 4 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_up_to_4_numerical_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(4 >= field->number_of_components)&&
			Computed_field_has_numerical_components(field,(void *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_up_to_4_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_up_to_4_numerical_components */

int Computed_field_has_at_least_2_components(struct cmzn_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> has at least 2 components.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_at_least_2_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(2 <= field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_at_least_2_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_at_least_2_components */

int Computed_field_has_3_components(struct cmzn_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> has exactly three
components - useful for selecting vector/coordinate fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_3_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(3 == field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_3_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_3_components */

int Computed_field_has_4_components(struct cmzn_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> has exactly four
components - useful for selecting vector/coordinate fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_4_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(4 == field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_4_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_4_components */

int Computed_field_has_16_components(struct cmzn_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 21 February 2008

DESCRIPTION :
Iterator/conditional function returning true if <field> has exactly sixteen
components - useful for selecting transformation matrix.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_16_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(16 == field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_16_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_16_components */

int Computed_field_has_n_components(struct cmzn_field *field,
	void *components_ptr_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> has the same number of
components as that specified by <components_ptr_void>.
==============================================================================*/
{
	int *components, return_code;

	ENTER(Computed_field_has_n_components);
	if (field && (components = (int *)components_ptr_void))
	{
		return_code=(*components == field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_n_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_n_components */

int Computed_field_has_multiple_times(struct cmzn_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Conditional function returning true if <field> depends on time.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_multiple_times);
	return_code=0;
	if (field)
	{
		return_code = field->core->has_multiple_times();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_multipletimes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_multiple_times */

int Computed_field_is_orientation_scale_capable(struct cmzn_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Conditional function returning true if the field can be used to orient or scale
glyphs. Generally, this means it has 1,2,3,4,6 or 9 components, where:
1 = scalar (no vector, isotropic scaling).
2 = 1 2-D vector (2nd axis is normal in plane, 3rd is out of 2-D plane);
3 = 1 3-D vector (orthogonal 2nd and 3rd axes are arbitrarily chosen);
4 = 2 2-D vectors (3rd axis taken as out of 2-D plane);
6 = 2 3-D vectors (3rd axis found from cross product);
9 = 3 3-D vectors = complete definition of 3 axes.
???RC.  Include coordinate system in test?
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_orientation_scale_capable);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(
			(1==field->number_of_components)||(2==field->number_of_components)||
			(3==field->number_of_components)||(4==field->number_of_components)||
			(6==field->number_of_components)||(9==field->number_of_components));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_orientation_scale_capable.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_orientation_scale_capable */

int Computed_field_is_stream_vector_capable(struct cmzn_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Conditional function returning true if the field is suitable for 3-D streamline
tracking. This means it has either 3, 6 or 9 components (with 3 components per
vector), or has a FIBRE coordinate_system, meaning it can be wrapped to produce
9-component fibre_axes.  Also now supports 2 components for use with a 2
component coordinate field.
The number of components controls how the field is interpreted:
3 = 1 3-D vector (lateral direction and normal worked out from curl of field);
6 = 2 3-D vectors (2nd vector is lateral direction. Stream ribbon normal found
	from cross product);
9 = 3 3-D vectors (2nd vector is lateral direction; 3rd vector is stream ribbon
	normal).
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_stream_vector_capable);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=((2==field->number_of_components)||(3==field->number_of_components)||
			(6==field->number_of_components)||(9==field->number_of_components)||
			((3>=field->number_of_components)&&
				(FIBRE == field->getCoordinateSystem().getType())));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_stream_vector_capable.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_stream_vector_capable */

bool equivalent_computed_fields_at_elements(struct FE_element *element_1,
	struct FE_element *element_2)
{
	return equivalent_FE_fields_in_elements(element_1,element_2);
}

int equivalent_computed_fields_at_nodes(struct FE_node *node_1,
	struct FE_node *node_2)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if all fields are defined in the same way at the two nodes.
==============================================================================*/
{
	int return_code;

	ENTER(equivalent_computed_fields_at_nodes);
	return_code=0;
	if (node_1&&node_2)
	{
		return_code=equivalent_FE_fields_at_nodes(node_1,node_2);
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* equivalent_computed_fields_at_nodes */

int list_Computed_field(struct cmzn_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Writes the properties of the <field> to the command window.
==============================================================================*/
{
	char *component_name,*temp_string;
	int return_code;

	ENTER(list_Computed_field);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=1;
		display_message(INFORMATION_MESSAGE,"field : %s\n",field->name);
		display_message(INFORMATION_MESSAGE,"  number_of_components = %d\n",
			field->number_of_components);
		if (NULL != (temp_string=Coordinate_system_string(&field->coordinate_system)))
		{
			display_message(INFORMATION_MESSAGE,"  coordinate_system = %s\n",
				temp_string);
			DEALLOCATE(temp_string);
		}
		display_message(INFORMATION_MESSAGE,"  field type = %s\n",
			Computed_field_get_type_string(field));
		field->core->list();
		/* write the names of the components */
		if (1<field->number_of_components)
		{
			display_message(INFORMATION_MESSAGE,"  component names:");
			for (int i = 1; i <= field->number_of_components; ++i)
			{
				if (NULL != (component_name = cmzn_field_get_component_name(field, i)))
				{
					if (1 < i)
					{
						display_message(INFORMATION_MESSAGE,",");
					}
					display_message(INFORMATION_MESSAGE," %s",component_name);
					DEALLOCATE(component_name);
				}
			}
			display_message(INFORMATION_MESSAGE,"\n");
		}
		display_message(INFORMATION_MESSAGE,"  (access count = %d)\n",
			field->access_count);
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field */

int list_Computed_field_name(struct cmzn_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Lists a single line about a computed field including just name, type, number of
components and coordinate system.
==============================================================================*/
{
	char *temp_string;
	int return_code;

	ENTER(list_Computed_field_name);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"%s",field->name);
		display_message(INFORMATION_MESSAGE," : %s",
			Computed_field_get_type_string(field));
		display_message(INFORMATION_MESSAGE,", %d component(s)",
			field->number_of_components);
		if (NULL != (temp_string=Coordinate_system_string(&field->coordinate_system)))
		{
			display_message(INFORMATION_MESSAGE,", %s",temp_string);
			DEALLOCATE(temp_string);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_name */

int Computed_field_core::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Default listing of source fields and source values.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_core::list);
	if (field)
	{
		if (0 < field->number_of_source_fields)
		{
			display_message(INFORMATION_MESSAGE,
				"    source fields :");
			for (i = 0 ; i < field->number_of_source_fields ; i++)
			{
				display_message(INFORMATION_MESSAGE,
					" %s", field->source_fields[i]->name);
			}
			display_message(INFORMATION_MESSAGE, "\n");
		}
		if (0 < field->number_of_source_values)
		{
			display_message(INFORMATION_MESSAGE,
				"    values :");
			for (i = 0 ; i < field->number_of_source_values ; i++)
			{
				display_message(INFORMATION_MESSAGE,
					" %g", field->source_values[i]);
			}
			display_message(INFORMATION_MESSAGE, "\n");
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_core::list.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_core::list */

char *Computed_field_core::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Default listing of source fields and source values.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_get_command_string(filter));
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, get_type_string(), &error);
		if (0 < field->number_of_source_fields)
		{
			if (1 == field->number_of_source_fields)
				append_string(&command_string, " field", &error);
			else
				append_string(&command_string, " fields", &error);
			for (i = 0 ; i < field->number_of_source_fields ; i++)
			{
				if (GET_NAME(cmzn_field)(field->source_fields[i], &field_name))
				{
					append_string(&command_string, " ", &error);
					make_valid_token(&field_name);
					append_string(&command_string, field_name, &error);
					DEALLOCATE(field_name);
				}
			}
		}
		if (0 < field->number_of_source_values)
		{
			append_string(&command_string, " values", &error);
			for (i = 0 ; i < field->number_of_source_values ; i++)
			{
				sprintf(temp_string, " %g", field->source_values[i]);
				append_string(&command_string, temp_string, &error);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_core::get_command_string.  Missing field");
	}

	return (command_string);
} /* Computed_field_core::get_command_string */

int Computed_field_core::get_domain( struct LIST(cmzn_field) *domain_field_list ) const
{
	int return_code = 0;

	if (field && domain_field_list)
	{
		return_code = 1;
		for (int i = 0; (i < field->number_of_source_fields) && return_code; i++)
		{
			return_code = field->source_fields[i]->core->get_domain( domain_field_list );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_core::get_domain.  Invalid argument(s)");
	}

	return return_code;
}

int Computed_field_core::check_dependency()
{
	if (field)
	{
		if (0 == (field->manager_change_status & MANAGER_CHANGE_FULL_RESULT(cmzn_field)))
		{
			for (int i = 0; i < field->number_of_source_fields; i++)
			{
				int source_change_status = field->source_fields[i]->core->check_dependency();
				if (source_change_status & MANAGER_CHANGE_FULL_RESULT(cmzn_field))
				{
					field->setChangedPrivate(MANAGER_CHANGE_FULL_RESULT(cmzn_field));
					break;
				}
				else if (source_change_status & MANAGER_CHANGE_PARTIAL_RESULT(cmzn_field))
					field->setChangedPrivate(MANAGER_CHANGE_PARTIAL_RESULT(cmzn_field));
			}
		}
		return field->manager_change_status;
	}
	return MANAGER_CHANGE_NONE(cmzn_field);
}

bool Computed_field_core::is_non_linear() const
{
	if (field)
	{
		for (int i = 0; i < field->number_of_source_fields; i++)
		{
			if (field->source_fields[i]->core->is_non_linear())
			{
				return true;
			}
		}
	}
	return false;
}

bool Computed_field_core::is_purely_function_of_field(cmzn_field *other_field)
{
	if (this->field == other_field)
		return true;
	for (int i = 0; i < this->field->number_of_source_fields; ++i)
		if (!this->field->source_fields[i]->core->is_purely_function_of_field(other_field))
			return false;
	return true;
}

bool cmzn_field::broadcastComponents(cmzn_field*& field_one, cmzn_field*& field_two)
{
	if ((field_one) && (field_two))
	{
		if (field_one->number_of_components ==
			field_two->number_of_components)
		{
			return true;
		}
		else
		{
			int number_of_components = 0;
			cmzn_field** field_to_wrap = nullptr;
			if (1 == field_one->number_of_components)
			{
				number_of_components = field_two->number_of_components;
				field_to_wrap = &field_one;
			}
			else if (1 == field_two->number_of_components)
			{
				number_of_components = field_one->number_of_components;
				field_to_wrap = &field_two;
			}
			if (field_to_wrap)
			{
				std::vector<int> source_component_indexes(number_of_components, 1);
				cmzn_fieldmodule* fieldmodule = cmzn_fieldmodule_create(field_one->getRegion());
				// broadcast wrapper field has same name stem as wrapped field
				cmzn_fieldmodule_begin_change(fieldmodule);
				cmzn_field* broadcast_wrapper = cmzn_fieldmodule_create_field_component_multiple(fieldmodule,
					*field_to_wrap, number_of_components, source_component_indexes.data());
				if (broadcast_wrapper)
				{
					broadcast_wrapper->setNameUnique((*field_to_wrap)->getName(), "_", 1);
				}
				cmzn_fieldmodule_end_change(fieldmodule);
				cmzn_fieldmodule_destroy(&fieldmodule);
				cmzn_field::deaccess(*field_to_wrap);
				// assign to take over access
				*field_to_wrap = broadcast_wrapper;
				return true;
			}
		}
	}
	display_message(ERROR_MESSAGE,
		"cmzn_field::broadcastComponents.  Invalid arguments");
	return false;
}

bool cmzn_field_is_managed(cmzn_field_id field)
{
	if (field)
	{
		return (0 != (field->attribute_flags & COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT));
	}
	return 0;
}

int cmzn_field_set_managed(cmzn_field_id field, bool value)
{
	if (field)
	{
		bool old_value = cmzn_field_is_managed(field);
		if (value)
		{
			field->attribute_flags |= COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT;
		}
		else
		{
			field->attribute_flags &= ~COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT;
		}
		if (value != old_value)
		{
			MANAGED_OBJECT_CHANGE(cmzn_field)(
				field, MANAGER_CHANGE_DEFINITION(cmzn_field));
		}
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

char *cmzn_field_get_component_name(cmzn_field_id field, int component_number)
{
	if (field && (0 < component_number) &&
		(component_number <= field->number_of_components))
	{
		return field->core->getComponentName(component_number);
	}
	return 0;
}

int cmzn_field_set_component_name(cmzn_field_id field,
	int component_number, const char *name)
{
	if (field && (0 < component_number) &&
		(component_number <= field->number_of_components) && name)
	{
		return field->core->setComponentName(component_number, name);
	}
	return CMZN_ERROR_ARGUMENT;
}

double cmzn_field_get_coordinate_system_focus(cmzn_field_id field)
{
	if (field)
	{
		return static_cast<double>(field->getCoordinateSystemFocus());
	}
	return 0.0;
}

int cmzn_field_set_coordinate_system_focus(cmzn_field_id field, double focus)
{
	if (field)
	{
		return field->setCoordinateSystemFocus(static_cast<FE_value>(focus));
	}
	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_field_coordinate_system_type cmzn_field_get_coordinate_system_type(
	cmzn_field_id field)
{
	if (field)
	{
		return field->getCoordinateSystemType();
	}
	return CMZN_FIELD_COORDINATE_SYSTEM_TYPE_INVALID;
}

int cmzn_field_set_coordinate_system_type(cmzn_field_id field,
	enum cmzn_field_coordinate_system_type coordinate_system_type)
{
	if (field)
	{
		return field->setCoordinateSystemType(coordinate_system_type);
	}
	return CMZN_ERROR_ARGUMENT;
}

char *cmzn_field_get_name(cmzn_field_id field)
{
	if (field)
	{
		return duplicate_string(field->getName());
	}
	return nullptr;
}

int cmzn_field_set_name(cmzn_field_id field, const char *name)
{
	if (field)
	{
		return field->setName(name);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_field_get_number_of_source_fields(cmzn_field_id field)
{
	if (field)
		return field->number_of_source_fields;
	return 0;
}

cmzn_field_id cmzn_field_get_source_field(cmzn_field_id field, int index)
{
	cmzn_field_id source_field = nullptr;
	if ((field) && (0 < index) && (index <= field->number_of_source_fields))
	{
		source_field = field->source_fields[index - 1]->access();
	}
	return source_field;
}

bool cmzn_field_is_type_coordinate(cmzn_field_id field)
{
	if (field)
		return field->core->isTypeCoordinate();
	return false;
}

int cmzn_field_set_type_coordinate(cmzn_field_id field, bool value)
{
	if (field)
	{
		const bool oldValue = field->core->isTypeCoordinate();
		if (value == oldValue)
			return CMZN_OK;
		int result = field->core->setTypeCoordinate(value);
		const bool newValue = field->core->isTypeCoordinate();
		if (newValue != oldValue)
		{
			MANAGED_OBJECT_CHANGE(cmzn_field)(field, MANAGER_CHANGE_DEFINITION(cmzn_field));
		}
		return result;
	}
	return CMZN_ERROR_ARGUMENT;
}

int Computed_field_manager_set_region(struct MANAGER(cmzn_field) *manager,
	struct cmzn_region *region)
{
	return MANAGER_SET_OWNER(cmzn_field)(manager, region);
}

struct cmzn_region *Computed_field_manager_get_region(
	struct MANAGER(cmzn_field) *manager)
{
	return MANAGER_GET_OWNER(cmzn_field)(manager);
}

const cmzn_set_cmzn_field &Computed_field_manager_get_fields(
	struct MANAGER(cmzn_field) *manager)
{
	return const_cast<const cmzn_set_cmzn_field&>(
		*(reinterpret_cast<cmzn_set_cmzn_field*>(manager->object_list)));
}

struct cmzn_region *Computed_field_get_region(struct cmzn_field *field)
{
	if (field)
		return field->getRegion();
	return 0;
}

int Computed_field_does_not_depend_on_field(cmzn_field *field, void *source_field_void)
{
	struct cmzn_field *source_field = (struct cmzn_field *)source_field_void;
	int return_code = 0;

	if (field && source_field && field != source_field)
	{
		return_code = field->dependsOnField(source_field);
	}

	return !return_code;
}

int Computed_field_is_not_source_field_of_others(struct cmzn_field *field)
{
	int return_code = 0;

	if (field->manager)
	{
		return_code = FOR_EACH_OBJECT_IN_MANAGER(cmzn_field)(
			Computed_field_does_not_depend_on_field,(void *)field,
			field->manager);
	}
	else
	{
		return_code = 1;
	}

	return return_code;
}

void Computed_field_manager_propagate_hierarchical_field_changes(
	MANAGER(cmzn_field) *manager, MANAGER_MESSAGE(cmzn_field) *message)
{
	if (manager && message)
	{
		MANAGER_BEGIN_CACHE(cmzn_field)(manager);
		cmzn_set_cmzn_field *all_fields = reinterpret_cast<cmzn_set_cmzn_field *>(manager->object_list);
		for (cmzn_set_cmzn_field::iterator iter = all_fields->begin(); iter != all_fields->end(); iter++)
			(*iter)->core->propagate_hierarchical_field_changes(message);
		MANAGER_END_CACHE(cmzn_field)(manager);
	}
}

void Computed_field_manager_subregion_removed(MANAGER(cmzn_field) *manager,
	cmzn_region *subregion)
{
	if (manager && subregion)
	{
		MANAGER_BEGIN_CACHE(cmzn_field)(manager);
		cmzn_set_cmzn_field *all_fields = reinterpret_cast<cmzn_set_cmzn_field *>(manager->object_list);
		for (cmzn_set_cmzn_field::iterator iter = all_fields->begin(); iter != all_fields->end(); iter++)
			(*iter)->core->subregionRemoved(subregion);
		MANAGER_END_CACHE(cmzn_field)(manager);
	}
}

int Computed_field_manager_message_get_object_change_and_detail(
	struct MANAGER_MESSAGE(cmzn_field) *message, struct cmzn_field *field,
	const struct cmzn_field_change_detail **change_detail_address)
{
	if (message)
		return message->getObjectChangeFlagsAndDetail(field, change_detail_address);
	if (change_detail_address)
		*change_detail_address = 0;
	return MANAGER_CHANGE_NONE(cmzn_field);
}

class cmzn_field_coordinate_system_type_conversion
{
public:
	static const char *to_string(enum cmzn_field_coordinate_system_type coordinate_system_type)
	{
		const char *enum_string = 0;
		switch (coordinate_system_type)
		{
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN:
				enum_string = "RECTANGULAR_CARTESIAN";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_CYLINDRICAL_POLAR:
				enum_string = "CYLINDRICAL_POLAR";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR:
				enum_string = "SPHERICAL_POLAR";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL:
				enum_string = "PROLATE_SPHEROIDAL";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL:
				enum_string = "OBLATE_SPHEROIDAL";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_FIBRE:
				enum_string = "FIBRE";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE:
				enum_string = "NOT_APPLICABLE";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum cmzn_field_coordinate_system_type cmzn_field_coordinate_system_type_enum_from_string(const char *name)
{
	return string_to_enum<enum cmzn_field_coordinate_system_type, cmzn_field_coordinate_system_type_conversion>(name);
}

char *cmzn_field_coordinate_system_type_enum_to_string(enum cmzn_field_coordinate_system_type coordinate_system_type)
{
	const char *coordinate_system_type_string = cmzn_field_coordinate_system_type_conversion::to_string(coordinate_system_type);
	return (coordinate_system_type_string ? duplicate_string(coordinate_system_type_string) : 0);
}


class cmzn_field_type_conversion
{
public:
	static const char *to_string(enum cmzn_field_type type)
	{
		const char *enum_string = 0;
		switch (type)
		{
			case CMZN_FIELD_TYPE_INVALID:
				// no enum string
				break;
			case CMZN_FIELD_TYPE_APPLY:
				enum_string = "APPLY";
				break;
			case CMZN_FIELD_TYPE_ARGUMENT_REAL:
				enum_string = "ARGUMENT_REAL";
				break;
			case CMZN_FIELD_TYPE_ADD:
				enum_string = "ADD";
				break;
			case CMZN_FIELD_TYPE_POWER:
				enum_string = "POWER";
				break;
			case CMZN_FIELD_TYPE_MULTIPLY:
				enum_string = "MULTIPLY";
				break;
			case CMZN_FIELD_TYPE_DIVIDE:
				enum_string = "DIVIDE";
				break;
			case CMZN_FIELD_TYPE_SUBTRACT:
				enum_string = "SUBTRACT";
				break;
			case CMZN_FIELD_TYPE_LOG:
				enum_string = "LOG";
				break;
			case CMZN_FIELD_TYPE_SQRT:
				enum_string = "SQRT";
				break;
			case CMZN_FIELD_TYPE_EXP:
				enum_string = "EXP";
				break;
			case CMZN_FIELD_TYPE_ABS:
				enum_string = "ABS";
				break;
			case CMZN_FIELD_TYPE_IDENTITY:
				enum_string = "IDENTITY";
				break;
			case CMZN_FIELD_TYPE_COMPONENT:
				enum_string = "COMPONENT";
				break;
			case CMZN_FIELD_TYPE_CONCATENATE:
				enum_string = "CONCATENATE";
				break;
			case CMZN_FIELD_TYPE_IF:
				enum_string = "IF";
				break;
			case CMZN_FIELD_TYPE_CONSTANT:
				enum_string = "CONSTANT";
				break;
			case CMZN_FIELD_TYPE_STRING_CONSTANT:
				enum_string = "STRING_CONSTANT";
				break;
			case CMZN_FIELD_TYPE_COORDINATE_TRANSFORMATION:
				enum_string = "COORDINATE_TRANSFORMATION";
				break;
			case CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANSFORMATION:
				enum_string = "VECTOR_COORDINATE_TRANSFORMATION";
				break;
			case CMZN_FIELD_TYPE_DERIVATIVE:
				enum_string = "DERIVATIVE";
				break;
			case CMZN_FIELD_TYPE_CURL:
				enum_string = "CURL";
				break;
			case CMZN_FIELD_TYPE_DIVERGENCE:
				enum_string = "DIVERGENCE";
				break;
			case CMZN_FIELD_TYPE_GRADIENT:
				enum_string = "GRADIENT";
				break;
			case CMZN_FIELD_TYPE_FIBRE_AXES:
				enum_string = "FIBRE_AXES";
				break;
			case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
				enum_string = "EDGE_DISCONTINUITY";
				break;
			case CMZN_FIELD_TYPE_EMBEDDED:
				enum_string = "EMBEDDED";
				break;
			case CMZN_FIELD_TYPE_NODE_VALUE:
				enum_string = "NODE_VALUE";
				break;
			case CMZN_FIELD_TYPE_STORED_STRING:
				enum_string = "STORED_STRING";
				break;
			case CMZN_FIELD_TYPE_IS_EXTERIOR:
				enum_string = "IS_EXTERIOR";
				break;
			case CMZN_FIELD_TYPE_IS_ON_FACE:
				enum_string = "IS_ON_FACE";
				break;
			case CMZN_FIELD_TYPE_AND:
				enum_string = "AND";
				break;
			case CMZN_FIELD_TYPE_EQUAL_TO:
				enum_string = "EQUAL_TO";
				break;
			case CMZN_FIELD_TYPE_GREATER_THAN:
				enum_string = "GREATER_THAN";
				break;
			case CMZN_FIELD_TYPE_IS_DEFINED:
				enum_string = "IS_DEFINED";
				break;
			case CMZN_FIELD_TYPE_LESS_THAN:
				enum_string = "LESS_THAN";
				break;
			case CMZN_FIELD_TYPE_OR:
				enum_string = "OR";
				break;
			case CMZN_FIELD_TYPE_NOT:
				enum_string = "NOT";
				break;
			case CMZN_FIELD_TYPE_XOR:
				enum_string = "XOR";
				break;
			case CMZN_FIELD_TYPE_DETERMINANT:
				enum_string = "DETERMINANT";
				break;
			case CMZN_FIELD_TYPE_EIGENVALUES:
				enum_string = "EIGENVALUES";
				break;
			case CMZN_FIELD_TYPE_EIGENVECTORS:
				enum_string = "EIGENVECTORS";
				break;
			case CMZN_FIELD_TYPE_MATRIX_INVERT:
				enum_string = "MATRIX_INVERT";
				break;
			case CMZN_FIELD_TYPE_MATRIX_MULTIPLY:
				enum_string = "MATRIX_MULTIPLY";
				break;
			case CMZN_FIELD_TYPE_PROJECTION:
				enum_string = "PROJECTION";
				break;
			case CMZN_FIELD_TYPE_TRANSPOSE:
				enum_string = "TRANSPOSE";
				break;
			case CMZN_FIELD_TYPE_TIME_LOOKUP:
				enum_string = "TIME_LOOKUP";
				break;
			case CMZN_FIELD_TYPE_SIN:
				enum_string = "SIN";
				break;
			case CMZN_FIELD_TYPE_COS:
				enum_string = "COS";
				break;
			case CMZN_FIELD_TYPE_TAN:
				enum_string = "TAN";
				break;
			case CMZN_FIELD_TYPE_ASIN:
				enum_string = "ASIN";
				break;
			case CMZN_FIELD_TYPE_ACOS:
				enum_string = "ACOS";
				break;
			case CMZN_FIELD_TYPE_ATAN:
				enum_string = "ATAN";
				break;
			case CMZN_FIELD_TYPE_ATAN2:
				enum_string = "ATAN2";
				break;
			case CMZN_FIELD_TYPE_CROSS_PRODUCT:
				enum_string = "CROSS_PRODUCT";
				break;
			case CMZN_FIELD_TYPE_DOT_PRODUCT:
				enum_string = "DOT_PRODUCT";
				break;
			case CMZN_FIELD_TYPE_MAGNITUDE:
				enum_string = "MAGNITUDE";
				break;
			case CMZN_FIELD_TYPE_NORMALISE:
				enum_string = "NORMALISE";
				break;
			case CMZN_FIELD_TYPE_SUM_COMPONENTS:
				enum_string = "SUM_COMPONENTS";
				break;
			case CMZN_FIELD_TYPE_FINITE_ELEMENT:
				enum_string = "FINITE_ELEMENT";
				break;
			case CMZN_FIELD_TYPE_TIME_VALUE:
				enum_string = "TIME_VALUE";
				break;
			case CMZN_FIELD_TYPE_STORED_MESH_LOCATION:
				enum_string = "STORED_MESH_LOCATION";
				break;
			case CMZN_FIELD_TYPE_FIND_MESH_LOCATION:
				enum_string = "FIND_MESH_LOCATION";
				break;
			case CMZN_FIELD_TYPE_MESH_INTEGRAL:
				enum_string = "MESH_INTEGRAL";
				break;
			case CMZN_FIELD_TYPE_MESH_INTEGRAL_SQUARES:
				enum_string = "MESH_INTEGRAL_SQUARES";
				break;
			case CMZN_FIELD_TYPE_NODESET_MAXIMUM:
				enum_string = "NODESET_MAXIMUM";
				break;
			case CMZN_FIELD_TYPE_NODESET_MEAN:
				enum_string = "NODESET_MEAN";
				break;
			case CMZN_FIELD_TYPE_NODESET_MEAN_SQUARES:
				enum_string = "NODESET_MEAN_SQUARES";
				break;
			case CMZN_FIELD_TYPE_NODESET_MINIMUM:
				enum_string = "NODESET_MINIMUM";
				break;
			case CMZN_FIELD_TYPE_NODESET_SUM:
				enum_string = "NODESET_SUM";
				break;
			case CMZN_FIELD_TYPE_NODESET_SUM_SQUARES:
				enum_string = "NODESET_SUM_SQUARES";
				break;
		}
		return enum_string;
	}
};

enum cmzn_field_type cmzn_field_type_enum_from_string(const char *name)
{
	return string_to_enum<enum cmzn_field_type, cmzn_field_type_conversion>(name);
}

char *cmzn_field_type_enum_to_string(enum cmzn_field_type type)
{
	const char *type_string = cmzn_field_type_conversion::to_string(type);
	return (type_string ? duplicate_string(type_string) : 0);
}

class cmzn_field_type_class_name_conversion
{
public:
	static const char *to_string(enum cmzn_field_type type)
	{
		const char *class_name = 0;
		switch (type)
		{
			case CMZN_FIELD_TYPE_INVALID:
				// no class name
				break;
			case CMZN_FIELD_TYPE_APPLY:
				class_name = "FieldApply";
				break;
			case CMZN_FIELD_TYPE_ARGUMENT_REAL:
				class_name = "FieldArgumentReal";
				break;
			case CMZN_FIELD_TYPE_ADD:
				class_name = "FieldAdd";
				break;
			case CMZN_FIELD_TYPE_POWER:
				class_name = "FieldPower";
				break;
			case CMZN_FIELD_TYPE_MULTIPLY:
				class_name = "FieldMultiply";
				break;
			case CMZN_FIELD_TYPE_DIVIDE:
				class_name = "FieldDivide";
				break;
			case CMZN_FIELD_TYPE_SUBTRACT:
				class_name = "FieldSubtract";
				break;
			case CMZN_FIELD_TYPE_LOG:
				class_name = "FieldLog";
				break;
			case CMZN_FIELD_TYPE_SQRT:
				class_name = "FieldSqrt";
				break;
			case CMZN_FIELD_TYPE_EXP:
				class_name = "FieldExp";
				break;
			case CMZN_FIELD_TYPE_ABS:
				class_name = "FieldAbs";
				break;
			case CMZN_FIELD_TYPE_IDENTITY:
				class_name = "FieldIdentity";
				break;
			case CMZN_FIELD_TYPE_COMPONENT:
				class_name = "FieldComponent";
				break;
			case CMZN_FIELD_TYPE_CONCATENATE:
				class_name = "FieldConcatenate";
				break;
			case CMZN_FIELD_TYPE_IF:
				class_name = "FieldIf";
				break;
			case CMZN_FIELD_TYPE_CONSTANT:
				class_name = "FieldConstant";
				break;
			case CMZN_FIELD_TYPE_STRING_CONSTANT:
				class_name = "FieldStringConstant";
				break;
			case CMZN_FIELD_TYPE_COORDINATE_TRANSFORMATION:
				class_name = "FieldCoordinateTransformation";
				break;
			case CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANSFORMATION:
				class_name = "FieldVectorCoordinateTransformation";
				break;
			case CMZN_FIELD_TYPE_DERIVATIVE:
				class_name = "FieldDerivative";
				break;
			case CMZN_FIELD_TYPE_CURL:
				class_name = "FieldCurl";
				break;
			case CMZN_FIELD_TYPE_DIVERGENCE:
				class_name = "FieldDivergence";
				break;
			case CMZN_FIELD_TYPE_GRADIENT:
				class_name = "FieldGradient";
				break;
			case CMZN_FIELD_TYPE_FIBRE_AXES:
				class_name = "FieldFibreAxes";
				break;
			case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
				class_name = "FieldEdgeDiscontinuity";
				break;
			case CMZN_FIELD_TYPE_EMBEDDED:
				class_name = "FieldEmbedded";
				break;
			case CMZN_FIELD_TYPE_NODE_VALUE:
				class_name = "FieldNodeValue";
				break;
			case CMZN_FIELD_TYPE_STORED_STRING:
				class_name = "FieldStoredString";
				break;
			case CMZN_FIELD_TYPE_IS_EXTERIOR:
				class_name = "FieldIsExterior";
				break;
			case CMZN_FIELD_TYPE_IS_ON_FACE:
				class_name = "FieldIsOnFace";
				break;
			case CMZN_FIELD_TYPE_AND:
				class_name = "FieldAnd";
				break;
			case CMZN_FIELD_TYPE_EQUAL_TO:
				class_name = "FieldEqualTo";
				break;
			case CMZN_FIELD_TYPE_GREATER_THAN:
				class_name = "FieldGreaterThan";
				break;
			case CMZN_FIELD_TYPE_IS_DEFINED:
				class_name = "FieldIsDefined";
				break;
			case CMZN_FIELD_TYPE_LESS_THAN:
				class_name = "FieldLessThan";
				break;
			case CMZN_FIELD_TYPE_OR:
				class_name = "FieldOr";
				break;
			case CMZN_FIELD_TYPE_NOT:
				class_name = "FieldNot";
				break;
			case CMZN_FIELD_TYPE_XOR:
				class_name = "FieldXor";
				break;
			case CMZN_FIELD_TYPE_DETERMINANT:
				class_name = "FieldDeterminant";
				break;
			case CMZN_FIELD_TYPE_EIGENVALUES:
				class_name = "FieldEigenvalues";
				break;
			case CMZN_FIELD_TYPE_EIGENVECTORS:
				class_name = "FieldEigenvectors";
				break;
			case CMZN_FIELD_TYPE_MATRIX_INVERT:
				class_name = "FieldMatrixInvert";
				break;
			case CMZN_FIELD_TYPE_MATRIX_MULTIPLY:
				class_name = "FieldMatrixMultiply";
				break;
			case CMZN_FIELD_TYPE_PROJECTION:
				class_name = "FieldProjection";
				break;
			case CMZN_FIELD_TYPE_TRANSPOSE:
				class_name = "FieldTranspose";
				break;
			case CMZN_FIELD_TYPE_TIME_LOOKUP:
				class_name = "FieldTimeLookup";
				break;
			case CMZN_FIELD_TYPE_SIN:
				class_name = "FieldSin";
				break;
			case CMZN_FIELD_TYPE_COS:
				class_name = "FieldCos";
				break;
			case CMZN_FIELD_TYPE_TAN:
				class_name = "FieldTan";
				break;
			case CMZN_FIELD_TYPE_ASIN:
				class_name = "FieldAsin";
				break;
			case CMZN_FIELD_TYPE_ACOS:
				class_name = "FieldAcos";
				break;
			case CMZN_FIELD_TYPE_ATAN:
				class_name = "FieldAtan";
				break;
			case CMZN_FIELD_TYPE_ATAN2:
				class_name = "FieldAtan2";
				break;
			case CMZN_FIELD_TYPE_CROSS_PRODUCT:
				class_name = "FieldCrossProduct";
				break;
			case CMZN_FIELD_TYPE_DOT_PRODUCT:
				class_name = "FieldDotProduct";
				break;
			case CMZN_FIELD_TYPE_MAGNITUDE:
				class_name = "FieldMagnitude";
				break;
			case CMZN_FIELD_TYPE_NORMALISE:
				class_name = "FieldNormalise";
				break;
			case CMZN_FIELD_TYPE_SUM_COMPONENTS:
				class_name = "FieldSumComponents";
				break;
			case CMZN_FIELD_TYPE_FINITE_ELEMENT:
				class_name = "FieldFiniteElement";
				break;
			case CMZN_FIELD_TYPE_TIME_VALUE:
				class_name = "FieldTimeValue";
				break;
			case CMZN_FIELD_TYPE_STORED_MESH_LOCATION:
				class_name = "FieldStoredMeshLocation";
				break;
			case CMZN_FIELD_TYPE_FIND_MESH_LOCATION:
				class_name = "FieldFindMeshLocation";
				break;
			case CMZN_FIELD_TYPE_MESH_INTEGRAL:
				class_name = "FieldMeshIntegral";
				break;
			case CMZN_FIELD_TYPE_MESH_INTEGRAL_SQUARES:
				class_name = "FieldMeshIntegralSquares";
				break;
			case CMZN_FIELD_TYPE_NODESET_MAXIMUM:
				class_name = "FieldNodesetMaximum";
				break;
			case CMZN_FIELD_TYPE_NODESET_MEAN:
				class_name = "FieldNodesetMean";
				break;
			case CMZN_FIELD_TYPE_NODESET_MEAN_SQUARES:
				class_name = "FieldNodesetMeanSquares";
				break;
			case CMZN_FIELD_TYPE_NODESET_MINIMUM:
				class_name = "FieldNodesetMinimum";
				break;
			case CMZN_FIELD_TYPE_NODESET_SUM:
				class_name = "FieldNodesetSum";
				break;
			case CMZN_FIELD_TYPE_NODESET_SUM_SQUARES:
				class_name = "FieldNodesetSumSquares";
				break;
		}
		return class_name;
	}
};

enum cmzn_field_type cmzn_field_type_enum_from_class_name(const char *name)
{
	return string_to_enum<enum cmzn_field_type, cmzn_field_type_class_name_conversion>(name);
}

char *cmzn_field_type_enum_to_class_name(enum cmzn_field_type type)
{
	const char *type_string = cmzn_field_type_class_name_conversion::to_string(type);
	return (type_string ? duplicate_string(type_string) : 0);
}

enum cmzn_field_type cmzn_field_get_type(cmzn_field_id field)
{
	return field->core->get_type();
}

char *cmzn_field_get_class_name(cmzn_field_id field)
{
	if (field)
	{
		cmzn_field_type type = cmzn_field_get_type(field);
		char *class_name = cmzn_field_type_enum_to_class_name(type);
		if (!class_name)
		{
			display_message(ERROR_MESSAGE, "Field getClassName.  Class name not available for this field");
		}
		return class_name;
	}
	return nullptr;
}

bool cmzn_field_has_class_name(cmzn_field_id field,
	const char *class_name)
{
	if ((field) && (class_name))
	{
		cmzn_field_type type = cmzn_field_get_type(field);
		const char *tmp_class_name = cmzn_field_type_class_name_conversion::to_string(type);
		return (nullptr != tmp_class_name) && (0 == strcmp(class_name, tmp_class_name));
	}
	return false;
}

cmzn_fieldparameters_id cmzn_field_get_fieldparameters(cmzn_field_id field)
{
	if (field)
		return field->getFieldparameters();
	display_message(ERROR_MESSAGE, "Field getFieldparameters:  Invalid field");
	return nullptr;
}
