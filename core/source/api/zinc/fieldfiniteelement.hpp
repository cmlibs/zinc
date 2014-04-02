/**
 * @file fieldfiniteelement.hpp
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
public:

	FieldFiniteElement() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldFiniteElement(cmzn_field_finite_element_id field_finite_element_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_finite_element_id))
	{	}
};

class FieldEmbedded : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldEmbedded(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldEmbedded Fieldmodule::createFieldEmbedded(const Field& sourceField,
		const Field& embeddedLocationField);

public:

	FieldEmbedded() : Field(0)
	{ }

};

class FieldFindMeshLocation : public Field
{
public:

	FieldFindMeshLocation() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldFindMeshLocation(cmzn_field_find_mesh_location_id field_find_mesh_location_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_find_mesh_location_id))
	{	}

	enum SearchMode
	{
		SEARCH_MODE_INVALID = CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_INVALID,
		SEARCH_MODE_EXACT = CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_EXACT,
		SEARCH_MODE_NEAREST = CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_NEAREST
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

	friend FieldNodeValue Fieldmodule::createFieldNodeValue(const Field& sourceField,
		Node::ValueLabel nodeValueLabel, int versionNumber);

public:

	FieldNodeValue() : Field(0)
	{ }

};

class FieldStoredMeshLocation : public Field
{
public:

	FieldStoredMeshLocation() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldStoredMeshLocation(cmzn_field_stored_mesh_location_id field_stored_mesh_location_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_stored_mesh_location_id))
	{	}
};

class FieldStoredString : public Field
{
public:

	FieldStoredString() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldStoredString(cmzn_field_stored_string_id field_stored_string_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_stored_string_id))
	{	}
};

inline FieldFiniteElement Fieldmodule::createFieldFiniteElement(int numberOfComponents)
{
	return FieldFiniteElement(reinterpret_cast<cmzn_field_finite_element_id>(
		cmzn_fieldmodule_create_field_finite_element(id,numberOfComponents)));
}

inline FieldFiniteElement Field::castFiniteElement()
{
	return FieldFiniteElement(cmzn_field_cast_finite_element(id));
}

inline FieldEmbedded Fieldmodule::createFieldEmbedded(const Field& sourceField, const Field& embeddedLocationField)
{
	return FieldEmbedded(cmzn_fieldmodule_create_field_embedded(id,
		sourceField.getId(), embeddedLocationField.getId()));
}

inline FieldFindMeshLocation Fieldmodule::createFieldFindMeshLocation(
	const Field& sourceField, const Field& meshField, const Mesh& mesh)
{
	return FieldFindMeshLocation(reinterpret_cast<cmzn_field_find_mesh_location_id>(
		cmzn_fieldmodule_create_field_find_mesh_location(id, sourceField.getId(), meshField.getId(), mesh.getId())));
}

inline FieldFindMeshLocation Field::castFindMeshLocation()
{
	return FieldFindMeshLocation(cmzn_field_cast_find_mesh_location(id));
}

inline FieldNodeValue Fieldmodule::createFieldNodeValue(const Field& sourceField,
	Node::ValueLabel nodeValueLabel, int versionNumber)
{
	return FieldNodeValue(cmzn_fieldmodule_create_field_node_value(id,
		sourceField.getId(), static_cast<cmzn_node_value_label>(nodeValueLabel),
		versionNumber));
}

inline FieldStoredMeshLocation Fieldmodule::createFieldStoredMeshLocation(const Mesh& mesh)
{
	return FieldStoredMeshLocation(reinterpret_cast<cmzn_field_stored_mesh_location_id>(
		cmzn_fieldmodule_create_field_stored_mesh_location(id, mesh.getId())));
}

inline FieldStoredMeshLocation Field::castStoredMeshLocation()
{
	return FieldStoredMeshLocation(cmzn_field_cast_stored_mesh_location(id));
}

inline FieldStoredString Fieldmodule::createFieldStoredString()
{
	return FieldStoredString(reinterpret_cast<cmzn_field_stored_string_id>(
		cmzn_fieldmodule_create_field_stored_string(id)));
}

inline FieldStoredString Field::castStoredString()
{
	return FieldStoredString(cmzn_field_cast_stored_string(id));
}

}  // namespace Zinc
}
#endif /* CMZN_FIELD_TYPES_FINITE_ELEMENT_HPP__ */
