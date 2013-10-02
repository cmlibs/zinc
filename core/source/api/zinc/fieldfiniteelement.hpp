/***************************************************************************//**
 * FILE : fieldfiniteelement.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDTYPESFINITEELEMENT_HPP__
#define CMZN_FIELDTYPESFINITEELEMENT_HPP__

#include "zinc/fieldfiniteelement.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"
#include "zinc/element.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldFiniteElement : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldFiniteElement(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldFiniteElement Fieldmodule::createFieldFiniteElement(
		int numberOfComponents);

public:

	FieldFiniteElement() : Field(0)
	{ }

	FieldFiniteElement(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(cmzn_field_cast_finite_element(field.getId())))
	{	}

};

class FieldEmbedded : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldEmbedded(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldEmbedded Fieldmodule::createFieldEmbedded(Field& sourceField,
		Field& embeddedLocationField);

public:

	FieldEmbedded() : Field(0)
	{ }

};

class FieldFindMeshLocation : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldFindMeshLocation(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldFindMeshLocation Fieldmodule::createFieldFindMeshLocation(
		Field& sourceField, Field& meshField, Mesh& mesh);

public:

	FieldFindMeshLocation() : Field(0)
	{ }

	FieldFindMeshLocation(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(cmzn_field_cast_find_mesh_location(field.getId())))
	{	}

	enum SearchMode
	{
		SEARCH_MODE_FIND_EXACT = CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT,
		SEARCH_MODE_FIND_NEAREST = CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_NEAREST,
	};

	Mesh getMesh()
	{
		return Mesh(cmzn_field_find_mesh_location_get_mesh(
			reinterpret_cast<cmzn_field_find_mesh_location_id>(id)));
	}

	SearchMode getSearchMode()
	{
		return static_cast<SearchMode>(cmzn_field_find_mesh_location_get_search_mode(
			reinterpret_cast<cmzn_field_find_mesh_location_id>(id)));
	}

	int setSearchMode(SearchMode searchMode)
	{
		return cmzn_field_find_mesh_location_set_search_mode(
			reinterpret_cast<cmzn_field_find_mesh_location_id>(id),
			static_cast<cmzn_field_find_mesh_location_search_mode>(searchMode));
	}
};

class FieldNodeValue : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodeValue(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldNodeValue Fieldmodule::createFieldNodeValue(Field& sourceField,
		Node::ValueType valueType, int versionNumber);

public:

	FieldNodeValue() : Field(0)
	{ }

};

class FieldStoredMeshLocation : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldStoredMeshLocation(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldStoredMeshLocation Fieldmodule::createFieldStoredMeshLocation(Mesh& mesh);

public:

	FieldStoredMeshLocation() : Field(0)
	{ }

	FieldStoredMeshLocation(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(
			cmzn_field_cast_stored_mesh_location(field.getId())))
	{	}
};

class FieldStoredString : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldStoredString(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldStoredString Fieldmodule::createFieldStoredString();

public:

	FieldStoredString() : Field(0)
	{ }

	FieldStoredString(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(
			cmzn_field_cast_stored_string(field.getId())))
	{	}
};

inline FieldFiniteElement Fieldmodule::createFieldFiniteElement(int numberOfComponents)
{
	return FieldFiniteElement(cmzn_fieldmodule_create_field_finite_element(id,
		numberOfComponents));
}

inline FieldEmbedded Fieldmodule::createFieldEmbedded(Field& sourceField, Field& embeddedLocationField)
{
	return FieldEmbedded(cmzn_fieldmodule_create_field_embedded(id,
		sourceField.getId(), embeddedLocationField.getId()));
}

inline FieldFindMeshLocation Fieldmodule::createFieldFindMeshLocation(
	Field& sourceField, Field& meshField, Mesh& mesh)
{
	return FieldFindMeshLocation(cmzn_fieldmodule_create_field_find_mesh_location(id,
		sourceField.getId(), meshField.getId(), mesh.getId()));
}

inline FieldNodeValue Fieldmodule::createFieldNodeValue(Field& sourceField,
	Node::ValueType valueType, int versionNumber)
{
	return FieldNodeValue(cmzn_fieldmodule_create_field_node_value(id,
		sourceField.getId(), static_cast<cmzn_node_value_type>(valueType),
		versionNumber));
}

inline FieldStoredMeshLocation Fieldmodule::createFieldStoredMeshLocation(Mesh& mesh)
{
	return FieldStoredMeshLocation(cmzn_fieldmodule_create_field_stored_mesh_location(id,
		mesh.getId()));
}

inline FieldStoredString Fieldmodule::createFieldStoredString()
{
	return FieldStoredString(cmzn_fieldmodule_create_field_stored_string(id));
}

}  // namespace Zinc
}
#endif /* CMZN_FIELD_TYPES_FINITE_ELEMENT_HPP__ */
