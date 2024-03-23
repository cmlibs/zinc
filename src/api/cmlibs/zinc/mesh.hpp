/**
 * @file mesh.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_MESH_HPP__
#define CMZN_MESH_HPP__

#include "cmlibs/zinc/mesh.h"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/element.hpp"
#include "cmlibs/zinc/elementfieldtemplate.hpp"
#include "cmlibs/zinc/elementtemplate.hpp"
#include "cmlibs/zinc/differentialoperator.hpp"

namespace CMLibs
{
namespace Zinc
{

class MeshGroup;

class Mesh
{

protected:
	cmzn_mesh_id id;

public:

	Mesh() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Mesh(cmzn_mesh_id mesh_id) :	id(mesh_id)
	{ }

	Mesh(const Mesh& mesh) :
		id(cmzn_mesh_access(mesh.id))
	{ }

	~Mesh()
	{
		if (0 != id)
		{
			cmzn_mesh_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	Mesh& operator=(const Mesh& mesh)
	{
		cmzn_mesh_id temp_id = cmzn_mesh_access(mesh.id);
		if (0 != id)
		{
			cmzn_mesh_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	cmzn_mesh_id getId() const
	{
		return id;
	}

	inline MeshGroup castGroup();

	bool containsElement(const Element& element) const
	{
		return cmzn_mesh_contains_element(id, element.getId());
	}

	Elementfieldtemplate createElementfieldtemplate(const Elementbasis& basis)
	{
		return Elementfieldtemplate(cmzn_mesh_create_elementfieldtemplate(id, basis.getId()));
	}

	Elementtemplate createElementtemplate()
	{
		return Elementtemplate(cmzn_mesh_create_elementtemplate(id));
	}

	Element createElement(int identifier, const Elementtemplate& elementTemplate)
	{
		return Element(cmzn_mesh_create_element(id, identifier, elementTemplate.getId()));
	}

	Elementiterator createElementiterator()
	{
		return Elementiterator(cmzn_mesh_create_elementiterator(id));
	}

	int defineElement(int identifier, const Elementtemplate& elementTemplate)
	{
		return cmzn_mesh_define_element(id, identifier, elementTemplate.getId());
	}

	int destroyAllElements()
	{
		return cmzn_mesh_destroy_all_elements(id);
	}

	int destroyElement(const Element& element)
	{
		 return cmzn_mesh_destroy_element(id, element.getId());
	}

	int destroyElementsConditional(const Field& conditionalField)
	{
		return cmzn_mesh_destroy_elements_conditional(id,
			conditionalField.getId());
	}

	Element findElementByIdentifier(int identifier) const
	{
		return Element(cmzn_mesh_find_element_by_identifier(id, identifier));
	}

	Differentialoperator getChartDifferentialoperator(int order, int term) const
	{
		return Differentialoperator(cmzn_mesh_get_chart_differentialoperator(
			id, order, term));
	}

	int getDimension() const
	{
		return cmzn_mesh_get_dimension(id);
	}

	Mesh getFaceMesh() const
	{
		return Mesh(cmzn_mesh_get_face_mesh(this->id));
	}

	inline Fieldmodule getFieldmodule() const;

	Mesh getMasterMesh() const
	{
		return Mesh(cmzn_mesh_get_master_mesh(id));
	}

	char *getName() const
	{
		return cmzn_mesh_get_name(id);
	}

	Mesh getParentMesh() const
	{
		return Mesh(cmzn_mesh_get_parent_mesh(this->id));
	}

	int getSize() const
	{
		return cmzn_mesh_get_size(id);
	}

};

inline bool operator==(const Mesh& a, const Mesh& b)
{
	return a.getId() == b.getId();
}

inline Mesh Element::getMesh() const
{
	return Mesh(cmzn_element_get_mesh(id));
}

class MeshGroup : public Mesh
{
	inline cmzn_mesh_group_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_mesh_group_id>(this->id);
	}

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit MeshGroup(cmzn_mesh_group_id mesh_id) : Mesh(reinterpret_cast<cmzn_mesh_id>(mesh_id))
	{ }

	MeshGroup()
	{ }

	cmzn_mesh_group_id getId() const
	{
		return (cmzn_mesh_group_id)(id);
	}

	int addAdjacentElements(int sharedDimension)
	{
		return cmzn_mesh_group_add_adjacent_elements(this->getDerivedId(),
			sharedDimension);
	}

	int addElement(const Element& element)
	{
		return cmzn_mesh_group_add_element(this->getDerivedId(),
			element.getId());
	}

	int addElementsConditional(const Field& conditionalField)
	{
		return cmzn_mesh_group_add_elements_conditional(
			this->getDerivedId(), conditionalField.getId());
	}

	inline FieldGroup getFieldGroup() const;

	int removeAllElements()
	{
		return cmzn_mesh_group_remove_all_elements(this->getDerivedId());
	}

	int removeElement(const Element& element)
	{
		return cmzn_mesh_group_remove_element(this->getDerivedId(),
			element.getId());
	}

	int removeElementsConditional(const Field& conditionalField)
	{
		return cmzn_mesh_group_remove_elements_conditional(this->getDerivedId(),
			conditionalField.getId());
	}

};

inline MeshGroup Mesh::castGroup()
{
	return MeshGroup(cmzn_mesh_cast_group(id));
}

class Meshchanges
{
private:

	cmzn_meshchanges_id id;

public:

	Meshchanges() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Meshchanges(cmzn_meshchanges_id meshchanges_id) :
		id(meshchanges_id)
	{ }

	Meshchanges(const Meshchanges& meshchanges) :
		id(cmzn_meshchanges_access(meshchanges.id))
	{ }

	Meshchanges& operator=(const Meshchanges& meshchanges)
	{
		cmzn_meshchanges_id temp_id = cmzn_meshchanges_access(meshchanges.id);
		if (0 != id)
			cmzn_meshchanges_destroy(&id);
		id = temp_id;
		return *this;
	}

	~Meshchanges()
	{
		if (0 != id)
			cmzn_meshchanges_destroy(&id);
	}

	bool isValid() const
	{
		return (0 != id);
	}

	Element::ChangeFlags getElementChangeFlags(const Element& element) const
	{
		return cmzn_meshchanges_get_element_change_flags(id, element.getId());
	}

	int getNumberOfChanges() const
	{
		return cmzn_meshchanges_get_number_of_changes(id);
	}

	Element::ChangeFlags getSummaryElementChangeFlags() const
	{
		return cmzn_meshchanges_get_summary_element_change_flags(id);
	}
};

}  // namespace Zinc
}

#endif /* CMZN_MESH_HPP__ */
