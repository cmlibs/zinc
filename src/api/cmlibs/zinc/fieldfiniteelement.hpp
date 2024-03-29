/**
 * @file fieldfiniteelement.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDTYPESFINITEELEMENT_HPP__
#define CMZN_FIELDTYPESFINITEELEMENT_HPP__

#include "cmlibs/zinc/fieldfiniteelement.h"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldcache.hpp"
#include "cmlibs/zinc/fieldmodule.hpp"
#include "cmlibs/zinc/element.hpp"
#include "cmlibs/zinc/node.hpp"

namespace CMLibs
{
namespace Zinc
{

class FieldFiniteElement : public Field
{
	inline cmzn_field_finite_element_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_finite_element_id>(this->id);
	}

public:

	FieldFiniteElement() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldFiniteElement(cmzn_field_finite_element_id field_finite_element_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_finite_element_id))
	{	}

	int getNodeParameters(const Fieldcache& cache, int componentNumber,
		Node::ValueLabel nodeValueLabel, int versionNumber, int valuesCount, double *valuesOut) const
	{
		return cmzn_field_finite_element_get_node_parameters(this->getDerivedId(),
			cache.getId(), componentNumber, static_cast<cmzn_node_value_label>(nodeValueLabel),
			versionNumber, valuesCount, valuesOut);
	}

	int setNodeParameters(const Fieldcache& cache, int componentNumber,
		Node::ValueLabel nodeValueLabel, int versionNumber, int valuesCount, const double *valuesIn)
	{
		return cmzn_field_finite_element_set_node_parameters(this->getDerivedId(),
			cache.getId(), componentNumber, static_cast<cmzn_node_value_label>(nodeValueLabel),
			versionNumber, valuesCount, valuesIn);
	}

	bool hasParametersAtLocation(const Fieldcache& cache) const
	{
		return cmzn_field_finite_element_has_parameters_at_location(this->getDerivedId(), cache.getId());
	}
};

class FieldEdgeDiscontinuity : public Field
{
	friend FieldEdgeDiscontinuity Fieldmodule::createFieldEdgeDiscontinuity(
		const Field& sourceField);

	inline cmzn_field_edge_discontinuity_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_edge_discontinuity_id>(id);
	}

public:

	FieldEdgeDiscontinuity() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldEdgeDiscontinuity(cmzn_field_edge_discontinuity_id field_edge_discontinuity_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_edge_discontinuity_id))
	{	}

	enum Measure
	{
		MEASURE_INVALID = CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_INVALID,
		MEASURE_C1 = CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_C1,
		MEASURE_G1 = CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_G1,
		MEASURE_SURFACE_NORMAL = CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_SURFACE_NORMAL
	};

	Field getConditionalField() const
	{
		return Field(cmzn_field_edge_discontinuity_get_conditional_field(getDerivedId()));
	}

	int setConditionalField(const Field& conditionalField)
	{
		return cmzn_field_edge_discontinuity_set_conditional_field(getDerivedId(), conditionalField.getId());
	}

	Measure getMeasure() const
	{
		return static_cast<Measure>(cmzn_field_edge_discontinuity_get_measure(getDerivedId()));
	}

	int setMeasure(Measure measure)
	{
		return cmzn_field_edge_discontinuity_set_measure(getDerivedId(),
			static_cast<cmzn_field_edge_discontinuity_measure>(measure));
	}

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
	inline cmzn_field_find_mesh_location_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_find_mesh_location_id>(id);
	}

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

	Mesh getMesh() const
	{
		return Mesh(cmzn_field_find_mesh_location_get_mesh(this->getDerivedId()));
	}

	static SearchMode SearchModeEnumFromString(const char *name)
	{
		return static_cast<SearchMode>(cmzn_field_find_mesh_location_search_mode_enum_from_string(name));
	}

	static char *SearchModeEnumToString(SearchMode mode)
	{
		return cmzn_field_find_mesh_location_search_mode_enum_to_string(static_cast<cmzn_field_find_mesh_location_search_mode>(mode));
	}

	Mesh getSearchMesh() const
	{
		return Mesh(cmzn_field_find_mesh_location_get_search_mesh(this->getDerivedId()));
	}

	int setSearchMesh(const Mesh& mesh)
	{
		return cmzn_field_find_mesh_location_set_search_mesh(this->getDerivedId(), mesh.getId());
	}

	SearchMode getSearchMode() const
	{
		return static_cast<SearchMode>(cmzn_field_find_mesh_location_get_search_mode(
			this->getDerivedId()));
	}

	int setSearchMode(SearchMode searchMode)
	{
		return cmzn_field_find_mesh_location_set_search_mode(this->getDerivedId(),
			static_cast<cmzn_field_find_mesh_location_search_mode>(searchMode));
	}
};

class FieldNodeValue : public Field
{
private:
	inline cmzn_field_node_value_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_node_value_id>(this->id);
	}

public:

	FieldNodeValue() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodeValue(cmzn_field_node_value_id field_node_value_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_node_value_id))
	{	}

	Node::ValueLabel getNodeValueLabel() const
	{
		return static_cast<Node::ValueLabel>(
			cmzn_field_node_value_get_node_value_label(this->getDerivedId()));
	}

	int setNodeValueLabel(Node::ValueLabel nodeValueLabel)
	{
		return cmzn_field_node_value_set_node_value_label(this->getDerivedId(),
			static_cast<cmzn_node_value_label>(nodeValueLabel));
	}

	int getVersionNumber() const
	{
		return cmzn_field_node_value_get_version_number(this->getDerivedId());
	}

	int setVersionNumber(int versionNumber)
	{
		return cmzn_field_node_value_set_version_number(this->getDerivedId(), versionNumber);
	}

};

class FieldStoredMeshLocation : public Field
{
	inline cmzn_field_stored_mesh_location_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_stored_mesh_location_id>(id);
	}

public:

	FieldStoredMeshLocation() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldStoredMeshLocation(cmzn_field_stored_mesh_location_id field_stored_mesh_location_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_stored_mesh_location_id))
	{	}

	Mesh getMesh() const
	{
		return Mesh(cmzn_field_stored_mesh_location_get_mesh(this->getDerivedId()));
	}

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

class FieldIsExterior : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldIsExterior(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldIsExterior Fieldmodule::createFieldIsExterior();

public:

	FieldIsExterior() : Field(0)
	{ }
};

class FieldIsOnFace : public Field
{
private:
	inline cmzn_field_is_on_face_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_is_on_face_id>(this->id);
	}

public:

	FieldIsOnFace() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldIsOnFace(cmzn_field_is_on_face_id field_is_on_face_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_is_on_face_id))
	{ }

	Element::FaceType getElementFaceType() const
	{
		return static_cast<Element::FaceType>(
			cmzn_field_is_on_face_get_element_face_type(this->getDerivedId()));
	}

	int setElementFaceType(Element::FaceType face)
	{
		return cmzn_field_is_on_face_set_element_face_type(this->getDerivedId(),
			static_cast<cmzn_element_face_type>(face));
	}

};

class FieldNodeLookup : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodeLookup(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldNodeLookup Fieldmodule::createFieldNodeLookup(const Field& sourceField,
		const Node& lookupNode);

public:

	FieldNodeLookup() : Field(0)
	{ }

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

inline FieldEdgeDiscontinuity Fieldmodule::createFieldEdgeDiscontinuity(
	const Field& sourceField)
{
	return FieldEdgeDiscontinuity(reinterpret_cast<cmzn_field_edge_discontinuity_id>(
		cmzn_fieldmodule_create_field_edge_discontinuity(id, sourceField.getId())));
}

inline FieldEdgeDiscontinuity Field::castEdgeDiscontinuity()
{
	return FieldEdgeDiscontinuity(cmzn_field_cast_edge_discontinuity(id));
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
	return FieldNodeValue(reinterpret_cast<cmzn_field_node_value_id>(
		cmzn_fieldmodule_create_field_node_value(this->id, sourceField.getId(),
			static_cast<cmzn_node_value_label>(nodeValueLabel), versionNumber)));
}

inline FieldNodeValue Field::castNodeValue()
{
	return FieldNodeValue(cmzn_field_cast_node_value(this->id));
}

inline FieldStoredMeshLocation Fieldmodule::createFieldStoredMeshLocation(const Mesh& hostMesh)
{
	return FieldStoredMeshLocation(reinterpret_cast<cmzn_field_stored_mesh_location_id>(
		cmzn_fieldmodule_create_field_stored_mesh_location(id, hostMesh.getId())));
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

inline FieldIsExterior Fieldmodule::createFieldIsExterior()
{
	return FieldIsExterior(cmzn_fieldmodule_create_field_is_exterior(id));
}

inline FieldIsOnFace Fieldmodule::createFieldIsOnFace(Element::FaceType face)
{
	return FieldIsOnFace(reinterpret_cast<cmzn_field_is_on_face_id>(
		cmzn_fieldmodule_create_field_is_on_face(this->id,
			static_cast<cmzn_element_face_type>(face))));
}

inline FieldIsOnFace Field::castIsOnFace()
{
	return FieldIsOnFace(cmzn_field_cast_is_on_face(this->id));
}

inline FieldNodeLookup Fieldmodule::createFieldNodeLookup(const Field& sourceField,
	const Node& lookupNode)
{
	return FieldNodeLookup(cmzn_fieldmodule_create_field_node_lookup(
		id, sourceField.getId(), lookupNode.getId()));
}

}  // namespace Zinc
}
#endif /* CMZN_FIELD_TYPES_FINITE_ELEMENT_HPP__ */
