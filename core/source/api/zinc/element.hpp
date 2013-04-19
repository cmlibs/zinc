/***************************************************************************//**
 * FILE : element.hpp
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __ZN_ELEMENT_HPP__
#define __ZN_ELEMENT_HPP__

#include "zinc/element.h"
#include "zinc/field.hpp"
#include "zinc/differentialoperator.hpp"
#include "zinc/node.hpp"

namespace zinc
{

class ElementTemplate;

class Element
{
private:

	Cmiss_element_id id;

public:

	Element() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Element(Cmiss_element_id element_id) :
		id(element_id)
	{ }

	Element(const Element& element) :
		id(Cmiss_element_access(element.id))
	{ }

	~Element()
	{
		if (0 != id)
		{
			Cmiss_element_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Element& operator=(const Element& element)
	{
		Cmiss_element_id temp_id = Cmiss_element_access(element.id);
		if (0 != id)
		{
			Cmiss_element_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	enum ShapeType
	{
		SHAPE_TYPE_INVALID = CMISS_ELEMENT_SHAPE_TYPE_INVALID,/**< unspecified shape of known dimension */
		SHAPE_LINE = CMISS_ELEMENT_SHAPE_LINE ,        /**< 1-D: 0 <= xi1 <= 1 */
		SHAPE_SQUARE = CMISS_ELEMENT_SHAPE_SQUARE,      /**< 2-D: 0 <= xi1,xi2 <= 1 */
		SHAPE_TRIANGLE = CMISS_ELEMENT_SHAPE_TRIANGLE,    /**< 3-D: 0 <= xi1,xi2; xi1+xi2 <= 1 */
		SHAPE_CUBE = CMISS_ELEMENT_SHAPE_CUBE,        /**< 3-D: 0 <= xi1,xi2,xi3 <= 1 */
		SHAPE_TETRAHEDRON = CMISS_ELEMENT_SHAPE_TETRAHEDRON, /**< 3-D: 0 <= xi1,xi2,xi3; xi1+xi2+xi3 <= 1 */
		SHAPE_WEDGE12 = CMISS_ELEMENT_SHAPE_WEDGE12,     /**< 3-D: 0 <= xi1,xi2; xi1+xi2 <= 1; 0 <= xi3 <= 1 */
		SHAPE_WEDGE13 = CMISS_ELEMENT_SHAPE_WEDGE13,     /**< 3-D: 0 <= xi1,xi3; xi1+xi3 <= 1; 0 <= xi2 <= 1 */
		SHAPE_WEDGE23 = CMISS_ELEMENT_SHAPE_WEDGE23     /**< 3-D: 0 <= xi2,xi3; xi2+xi3 <= 1; 0 <= xi1 <= 1 */
	};

	Cmiss_element_id getId()
	{
		return id;
	}

	int getDimension()
	{
		return Cmiss_element_get_dimension(id);
	}

	int getIdentifier()
	{
		return Cmiss_element_get_identifier(id);
	}

	enum ShapeType getShapeType()
	{
		return static_cast<ShapeType>(Cmiss_element_get_shape_type(id));
	}

	int merge(ElementTemplate& elementTemplate);

};

class ElementBasis
{
private:

	Cmiss_element_basis_id id;

public:

	ElementBasis() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit ElementBasis(Cmiss_element_basis_id element_basis_id) :
		id(element_basis_id)
	{ }

	ElementBasis(const ElementBasis& elementBasis) :
		id(Cmiss_element_basis_access(elementBasis.id))
	{ }

	ElementBasis& operator=(const ElementBasis& elementBasis)
	{
		Cmiss_element_basis_id temp_id = Cmiss_element_basis_access(elementBasis.id);
		if (0 != id)
		{
			Cmiss_element_basis_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~ElementBasis()
	{
		if (0 != id)
		{
			Cmiss_element_basis_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum FunctionType
	{
		FUNCTION_TYPE_INVALID = CMISS_BASIS_FUNCTION_TYPE_INVALID,
		FUNCTION_CONSTANT = CMISS_BASIS_FUNCTION_CONSTANT,
		FUNCTION_LINEAR_LAGRANGE = CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE,
		FUNCTION_QUADRATIC_LAGRANGE = CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE,
		FUNCTION_CUBIC_LAGRANGE = CMISS_BASIS_FUNCTION_CUBIC_LAGRANGE,
		FUNCTION_LINEAR_SIMPLEX = CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX,   /**< linked on 2 or more dimensions */
		FUNCTION_QUADRATIC_SIMPLEX = CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX /**< linked on 2 or more dimensions */
	};

	Cmiss_element_basis_id getId()
	{
		return id;
	}

	int getDimension()
	{
		return Cmiss_element_basis_get_dimension(id);
	}

	enum FunctionType getFunctionType(int chartComponent)
	{
		return static_cast<FunctionType>(Cmiss_element_basis_get_function_type(id, chartComponent));
	}

	int setFunctionType(int chartComponent, FunctionType functionType)
	{
		return Cmiss_element_basis_set_function_type(id, chartComponent,
			static_cast<Cmiss_basis_function_type>(functionType));
	}

	int getNumberOfNodes()
	{
		return Cmiss_element_basis_get_number_of_nodes(id);
	}

};

class ElementTemplate
{
private:

	Cmiss_element_template_id id;

public:

	ElementTemplate() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit ElementTemplate(Cmiss_element_template_id element_template_id) :
		id(element_template_id)
	{ }

	ElementTemplate(const ElementTemplate& elementTemplate) :
		id(Cmiss_element_template_access(elementTemplate.id))
	{ }

	ElementTemplate& operator=(const ElementTemplate& elementTemplate)
	{
		Cmiss_element_template_id temp_id = Cmiss_element_template_access(elementTemplate.id);
		if (0 != id)
		{
			Cmiss_element_template_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~ElementTemplate()
	{
		if (0 != id)
		{
			Cmiss_element_template_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_element_template_id getId()
	{
		return id;
	}

	enum Element::ShapeType getShapeType()
	{
		return static_cast<Element::ShapeType>(Cmiss_element_template_get_shape_type(id));
	}

	int setShapeType(enum Element::ShapeType shapeType)
	{
		return Cmiss_element_template_set_shape_type(id,
			static_cast<Cmiss_element_shape_type>(shapeType));
	}

	int getNumberOfNodes()
	{
		return Cmiss_element_template_get_number_of_nodes(id);
	}

	int setNumberOfNodes(int numberOfNodes)
	{
		return Cmiss_element_template_set_number_of_nodes(id, numberOfNodes);
	}

	int defineFieldSimpleNodal(Field& field, int componentNumber,
		ElementBasis& basis, int basisNumberOfNodes, const int *localNodeIndexes)
	{
		return Cmiss_element_template_define_field_simple_nodal(
			id, field.getId(),  componentNumber, basis.getId(),
			basisNumberOfNodes, localNodeIndexes);
	}

	Node getNode(int localNodeIndex)
	{
		return Node(Cmiss_element_template_get_node(id, localNodeIndex));
	}

	int setNode(int localNodeIndex, Node& node)
	{
		return Cmiss_element_template_set_node(id, localNodeIndex, node.getId());
	}
};

class ElementIterator
{
private:

	Cmiss_element_iterator_id id;

public:

	ElementIterator() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit ElementIterator(Cmiss_element_iterator_id element_iterator_id) :
		id(element_iterator_id)
	{ }

	ElementIterator(const ElementIterator& elementIterator) :
		id(Cmiss_element_iterator_access(elementIterator.id))
	{ }

	ElementIterator& operator=(const ElementIterator& elementIterator)
	{
		Cmiss_element_iterator_id temp_id = Cmiss_element_iterator_access(elementIterator.id);
		if (0 != id)
		{
			Cmiss_element_iterator_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~ElementIterator()
	{
		if (0 != id)
		{
			Cmiss_element_iterator_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Element next()
	{
		return Element(Cmiss_element_iterator_next(id));
	}
};

class Mesh
{

protected:
	Cmiss_mesh_id id;

public:

	Mesh() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Mesh(Cmiss_mesh_id mesh_id) :	id(mesh_id)
	{ }

	Mesh(const Mesh& mesh) :
		id(Cmiss_mesh_access(mesh.id))
	{ }

	~Mesh()
	{
		if (0 != id)
		{
			Cmiss_mesh_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Mesh& operator=(const Mesh& mesh)
	{
		Cmiss_mesh_id temp_id = Cmiss_mesh_access(mesh.id);
		if (0 != id)
		{
			Cmiss_mesh_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	Cmiss_mesh_id getId()
	{
		return id;
	}

	bool containsElement(Element& element)
	{
		return (0 != Cmiss_mesh_contains_element(id, element.getId()));
	}

	ElementTemplate createElementTemplate()
	{
		return ElementTemplate(Cmiss_mesh_create_element_template(id));
	}

	Element createElement(int identifier, ElementTemplate& elementTemplate)
	{
		return Element(Cmiss_mesh_create_element(id, identifier, elementTemplate.getId()));
	}

	ElementIterator createElementIterator()
	{
		return ElementIterator(Cmiss_mesh_create_element_iterator(id));
	}

	int defineElement(int identifier, ElementTemplate& elementTemplate)
	{
		return Cmiss_mesh_define_element(id, identifier, elementTemplate.getId());
	}

	int destroyAllElements()
	{
		return Cmiss_mesh_destroy_all_elements(id);
	}

	int destroyElement(Element& element)
	{
		 return Cmiss_mesh_destroy_element(id, element.getId());
	}

	int destroyElementsConditional(Field& conditionalField)
	{
		return Cmiss_mesh_destroy_elements_conditional(id,
			conditionalField.getId());
	}

	Element findElementByIdentifier(int identifier)
	{
		return Element(Cmiss_mesh_find_element_by_identifier(id, identifier));
	}

	DifferentialOperator getChartDifferentialOperator(int order, int term)
	{
		return DifferentialOperator(Cmiss_mesh_get_chart_differential_operator(
			id, order, term));
	}

	int getDimension()
	{
		return Cmiss_mesh_get_dimension(id);
	}

	Mesh getMaster()
	{
		return Mesh(Cmiss_mesh_get_master(id));
	}

	char *getName()
	{
		return Cmiss_mesh_get_name(id);
	}

	int getSize()
	{
		return Cmiss_mesh_get_size(id);
	}

	int match(Mesh& mesh)
	{
		return Cmiss_mesh_match(id, mesh.id);
	}

};

class MeshGroup  : public Mesh
{

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit MeshGroup(Cmiss_mesh_group_id mesh_id) : Mesh(reinterpret_cast<Cmiss_mesh_id>(mesh_id))
	{ }

	Cmiss_mesh_group_id getId()
	{
		return (Cmiss_mesh_group_id)(id);
	}

	int addElement(Element& element)
	{
		return Cmiss_mesh_group_add_element(
			reinterpret_cast<Cmiss_mesh_group_id>(id), element.getId());
	}

	int removeAllElements()
	{
		return Cmiss_mesh_group_remove_all_elements(reinterpret_cast<Cmiss_mesh_group_id>(id));
	}

	int removeElement(Element& element)
	{
		return Cmiss_mesh_group_remove_element(reinterpret_cast<Cmiss_mesh_group_id>(id),
			element.getId());
	}

	int removeElementsConditional(Field& conditionalField)
	{
		return Cmiss_mesh_group_remove_elements_conditional(
			reinterpret_cast<Cmiss_mesh_group_id>(id), conditionalField.getId());
	}

};

inline int Element::merge(ElementTemplate& elementTemplate)
{
	return Cmiss_element_merge(id, elementTemplate.getId());
}

}  // namespace zinc


#endif /* __ZN_ELEMENT_HPP__ */
