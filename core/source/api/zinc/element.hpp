/***************************************************************************//**
 * FILE : element.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_ELEMENT_HPP__
#define CMZN_ELEMENT_HPP__

#include "zinc/element.h"
#include "zinc/field.hpp"
#include "zinc/differentialoperator.hpp"
#include "zinc/node.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class ElementBasis
{
private:

	cmzn_element_basis_id id;

public:

	ElementBasis() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit ElementBasis(cmzn_element_basis_id element_basis_id) :
		id(element_basis_id)
	{ }

	ElementBasis(const ElementBasis& elementBasis) :
		id(cmzn_element_basis_access(elementBasis.id))
	{ }

	ElementBasis& operator=(const ElementBasis& elementBasis)
	{
		cmzn_element_basis_id temp_id = cmzn_element_basis_access(elementBasis.id);
		if (0 != id)
		{
			cmzn_element_basis_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~ElementBasis()
	{
		if (0 != id)
		{
			cmzn_element_basis_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum FunctionType
	{
		FUNCTION_TYPE_INVALID = CMZN_BASIS_FUNCTION_TYPE_INVALID,
		FUNCTION_CONSTANT = CMZN_BASIS_FUNCTION_CONSTANT,
		FUNCTION_LINEAR_LAGRANGE = CMZN_BASIS_FUNCTION_LINEAR_LAGRANGE,
		FUNCTION_QUADRATIC_LAGRANGE = CMZN_BASIS_FUNCTION_QUADRATIC_LAGRANGE,
		FUNCTION_CUBIC_LAGRANGE = CMZN_BASIS_FUNCTION_CUBIC_LAGRANGE,
		FUNCTION_LINEAR_SIMPLEX = CMZN_BASIS_FUNCTION_LINEAR_SIMPLEX,   /**< linked on 2 or more dimensions */
		FUNCTION_QUADRATIC_SIMPLEX = CMZN_BASIS_FUNCTION_QUADRATIC_SIMPLEX, /**< linked on 2 or more dimensions */
		FUNCTION_CUBIC_HERMITE = CMZN_BASIS_FUNCTION_CUBIC_HERMITE
	};

	cmzn_element_basis_id getId()
	{
		return id;
	}

	int getDimension()
	{
		return cmzn_element_basis_get_dimension(id);
	}

	enum FunctionType getFunctionType(int chartComponent)
	{
		return static_cast<FunctionType>(cmzn_element_basis_get_function_type(id, chartComponent));
	}

	int setFunctionType(int chartComponent, FunctionType functionType)
	{
		return cmzn_element_basis_set_function_type(id, chartComponent,
			static_cast<cmzn_basis_function_type>(functionType));
	}

	int getNumberOfNodes()
	{
		return cmzn_element_basis_get_number_of_nodes(id);
	}

	int getNumberOfFunctions()
	{
		return cmzn_element_basis_get_number_of_functions(id);
	}

};

class ElementTemplate;
class MeshScaleFactorSet;

class Element
{
private:

	cmzn_element_id id;

public:

	Element() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Element(cmzn_element_id element_id) :
		id(element_id)
	{ }

	Element(const Element& element) :
		id(cmzn_element_access(element.id))
	{ }

	~Element()
	{
		if (0 != id)
		{
			cmzn_element_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum FaceType
	{
		FACE_INVALID = CMZN_ELEMENT_FACE_INVALID,
		FACE_ALL = CMZN_ELEMENT_FACE_ALL,
		FACE_XI1_0 = CMZN_ELEMENT_FACE_XI1_0,
		FACE_XI1_1 = CMZN_ELEMENT_FACE_XI1_1,
		FACE_XI2_0 = CMZN_ELEMENT_FACE_XI2_0,
		FACE_XI2_1 = CMZN_ELEMENT_FACE_XI2_1,
		FACE_XI3_0 = CMZN_ELEMENT_FACE_XI3_0,
		FACE_XI3_1 = CMZN_ELEMENT_FACE_XI3_1
	};

	Element& operator=(const Element& element)
	{
		cmzn_element_id temp_id = cmzn_element_access(element.id);
		if (0 != id)
		{
			cmzn_element_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	enum ShapeType
	{
		SHAPE_TYPE_INVALID = CMZN_ELEMENT_SHAPE_TYPE_INVALID,/**< unspecified shape of known dimension */
		SHAPE_LINE = CMZN_ELEMENT_SHAPE_LINE ,        /**< 1-D: 0 <= xi1 <= 1 */
		SHAPE_SQUARE = CMZN_ELEMENT_SHAPE_SQUARE,      /**< 2-D: 0 <= xi1,xi2 <= 1 */
		SHAPE_TRIANGLE = CMZN_ELEMENT_SHAPE_TRIANGLE,    /**< 3-D: 0 <= xi1,xi2; xi1+xi2 <= 1 */
		SHAPE_CUBE = CMZN_ELEMENT_SHAPE_CUBE,        /**< 3-D: 0 <= xi1,xi2,xi3 <= 1 */
		SHAPE_TETRAHEDRON = CMZN_ELEMENT_SHAPE_TETRAHEDRON, /**< 3-D: 0 <= xi1,xi2,xi3; xi1+xi2+xi3 <= 1 */
		SHAPE_WEDGE12 = CMZN_ELEMENT_SHAPE_WEDGE12,     /**< 3-D: 0 <= xi1,xi2; xi1+xi2 <= 1; 0 <= xi3 <= 1 */
		SHAPE_WEDGE13 = CMZN_ELEMENT_SHAPE_WEDGE13,     /**< 3-D: 0 <= xi1,xi3; xi1+xi3 <= 1; 0 <= xi2 <= 1 */
		SHAPE_WEDGE23 = CMZN_ELEMENT_SHAPE_WEDGE23     /**< 3-D: 0 <= xi2,xi3; xi2+xi3 <= 1; 0 <= xi1 <= 1 */
	};

	enum PointSampleMode
	{
		POINT_SAMPLE_MODE_INVALID = CMZN_ELEMENT_POINT_SAMPLE_MODE_INVALID,
		POINT_SAMPLE_CELL_CENTRES = CMZN_ELEMENT_POINT_SAMPLE_CELL_CENTRES,
		POINT_SAMPLE_CELL_CORNERS = CMZN_ELEMENT_POINT_SAMPLE_CELL_CORNERS,
		POINT_SAMPLE_CELL_POISSON = CMZN_ELEMENT_POINT_SAMPLE_CELL_POISSON,
		POINT_SAMPLE_SET_LOCATION = CMZN_ELEMENT_POINT_SAMPLE_SET_LOCATION
	};

	cmzn_element_id getId()
	{
		return id;
	}

	inline int getScaleFactors(MeshScaleFactorSet& scaleFactorSet, int valuesCount, double *values);

	inline int setScaleFactors(MeshScaleFactorSet& scaleFactorSet, int valuesCount, const double *values);

	int getDimension()
	{
		return cmzn_element_get_dimension(id);
	}

	int getIdentifier()
	{
		return cmzn_element_get_identifier(id);
	}

	int setIdentifier(int identifier)
	{
		return cmzn_element_set_identifier(id, identifier);
	}

	enum ShapeType getShapeType()
	{
		return static_cast<ShapeType>(cmzn_element_get_shape_type(id));
	}

	int merge(ElementTemplate& elementTemplate);

};

class ElementTemplate
{
private:

	cmzn_element_template_id id;

public:

	ElementTemplate() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit ElementTemplate(cmzn_element_template_id element_template_id) :
		id(element_template_id)
	{ }

	ElementTemplate(const ElementTemplate& elementTemplate) :
		id(cmzn_element_template_access(elementTemplate.id))
	{ }

	ElementTemplate& operator=(const ElementTemplate& elementTemplate)
	{
		cmzn_element_template_id temp_id = cmzn_element_template_access(elementTemplate.id);
		if (0 != id)
		{
			cmzn_element_template_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~ElementTemplate()
	{
		if (0 != id)
		{
			cmzn_element_template_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_element_template_id getId()
	{
		return id;
	}

	enum Element::ShapeType getShapeType()
	{
		return static_cast<Element::ShapeType>(cmzn_element_template_get_shape_type(id));
	}

	int setShapeType(enum Element::ShapeType shapeType)
	{
		return cmzn_element_template_set_shape_type(id,
			static_cast<cmzn_element_shape_type>(shapeType));
	}

	inline int setNumberOfScaleFactors(MeshScaleFactorSet &scaleFactorSet, int numberOfScaleFactors);

	int getNumberOfNodes()
	{
		return cmzn_element_template_get_number_of_nodes(id);
	}

	int setNumberOfNodes(int numberOfNodes)
	{
		return cmzn_element_template_set_number_of_nodes(id, numberOfNodes);
	}

	int defineFieldSimpleNodal(Field& field, int componentNumber,
		ElementBasis& basis, int nodeIndexesCount, const int *nodeIndexesIn)
	{
		return cmzn_element_template_define_field_simple_nodal(
			id, field.getId(),  componentNumber, basis.getId(),
			nodeIndexesCount, nodeIndexesIn);
	}

	Node getNode(int localNodeIndex)
	{
		return Node(cmzn_element_template_get_node(id, localNodeIndex));
	}

	int setNode(int localNodeIndex, Node& node)
	{
		return cmzn_element_template_set_node(id, localNodeIndex, node.getId());
	}
};

class ElementIterator
{
private:

	cmzn_element_iterator_id id;

public:

	ElementIterator() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit ElementIterator(cmzn_element_iterator_id element_iterator_id) :
		id(element_iterator_id)
	{ }

	ElementIterator(const ElementIterator& elementIterator) :
		id(cmzn_element_iterator_access(elementIterator.id))
	{ }

	ElementIterator& operator=(const ElementIterator& elementIterator)
	{
		cmzn_element_iterator_id temp_id = cmzn_element_iterator_access(elementIterator.id);
		if (0 != id)
		{
			cmzn_element_iterator_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~ElementIterator()
	{
		if (0 != id)
		{
			cmzn_element_iterator_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Element next()
	{
		return Element(cmzn_element_iterator_next(id));
	}
};

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

	bool isValid()
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

	cmzn_mesh_id getId()
	{
		return id;
	}

	bool containsElement(Element& element)
	{
		return (0 != cmzn_mesh_contains_element(id, element.getId()));
	}

	ElementTemplate createElementTemplate()
	{
		return ElementTemplate(cmzn_mesh_create_element_template(id));
	}

	Element createElement(int identifier, ElementTemplate& elementTemplate)
	{
		return Element(cmzn_mesh_create_element(id, identifier, elementTemplate.getId()));
	}

	ElementIterator createElementIterator()
	{
		return ElementIterator(cmzn_mesh_create_element_iterator(id));
	}

	int defineElement(int identifier, ElementTemplate& elementTemplate)
	{
		return cmzn_mesh_define_element(id, identifier, elementTemplate.getId());
	}

	int destroyAllElements()
	{
		return cmzn_mesh_destroy_all_elements(id);
	}

	int destroyElement(Element& element)
	{
		 return cmzn_mesh_destroy_element(id, element.getId());
	}

	int destroyElementsConditional(Field& conditionalField)
	{
		return cmzn_mesh_destroy_elements_conditional(id,
			conditionalField.getId());
	}

	Element findElementByIdentifier(int identifier)
	{
		return Element(cmzn_mesh_find_element_by_identifier(id, identifier));
	}

	DifferentialOperator getChartDifferentialOperator(int order, int term)
	{
		return DifferentialOperator(cmzn_mesh_get_chart_differential_operator(
			id, order, term));
	}

	int getDimension()
	{
		return cmzn_mesh_get_dimension(id);
	}

	inline MeshScaleFactorSet findMeshScaleFactorSetByName(const char *name);

	inline MeshScaleFactorSet createMeshScaleFactorSet();

	Mesh getMaster()
	{
		return Mesh(cmzn_mesh_get_master(id));
	}

	char *getName()
	{
		return cmzn_mesh_get_name(id);
	}

	int getSize()
	{
		return cmzn_mesh_get_size(id);
	}

	int match(Mesh& mesh)
	{
		return cmzn_mesh_match(id, mesh.id);
	}

};

class MeshGroup  : public Mesh
{

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit MeshGroup(cmzn_mesh_group_id mesh_id) : Mesh(reinterpret_cast<cmzn_mesh_id>(mesh_id))
	{ }

	cmzn_mesh_group_id getId()
	{
		return (cmzn_mesh_group_id)(id);
	}

	int addElement(Element& element)
	{
		return cmzn_mesh_group_add_element(
			reinterpret_cast<cmzn_mesh_group_id>(id), element.getId());
	}

	int removeAllElements()
	{
		return cmzn_mesh_group_remove_all_elements(reinterpret_cast<cmzn_mesh_group_id>(id));
	}

	int removeElement(Element& element)
	{
		return cmzn_mesh_group_remove_element(reinterpret_cast<cmzn_mesh_group_id>(id),
			element.getId());
	}

	int removeElementsConditional(Field& conditionalField)
	{
		return cmzn_mesh_group_remove_elements_conditional(
			reinterpret_cast<cmzn_mesh_group_id>(id), conditionalField.getId());
	}

};

class MeshScaleFactorSet
{
protected:
	cmzn_mesh_scale_factor_set_id id;

public:

	MeshScaleFactorSet() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit MeshScaleFactorSet(cmzn_mesh_scale_factor_set_id scale_factor_set_id) :
		id(scale_factor_set_id)
	{ }

	MeshScaleFactorSet(const MeshScaleFactorSet& scaleFactorSet) :
		id(cmzn_mesh_scale_factor_set_access(scaleFactorSet.id))
	{	}

	~MeshScaleFactorSet()
	{
		if (0 != id)
		{
			cmzn_mesh_scale_factor_set_destroy(&id);
		}
	}

	MeshScaleFactorSet& operator=(const MeshScaleFactorSet& scaleFactorSet)
	{
		cmzn_mesh_scale_factor_set_id temp_id = cmzn_mesh_scale_factor_set_access(scaleFactorSet.id);
		if (0 != id)
		{
			cmzn_mesh_scale_factor_set_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_mesh_scale_factor_set_id getId()
	{
		return id;
	}

	char *getName()
	{
		return cmzn_mesh_scale_factor_set_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_mesh_scale_factor_set_set_name(id, name);
	}
};

inline int Element::merge(ElementTemplate& elementTemplate)
{
	return cmzn_element_merge(id, elementTemplate.getId());
}

MeshScaleFactorSet Mesh::findMeshScaleFactorSetByName(const char *name)
{
	return MeshScaleFactorSet(cmzn_mesh_find_mesh_scale_factor_set_by_name(id, name));
}

MeshScaleFactorSet Mesh::createMeshScaleFactorSet()
{
	return MeshScaleFactorSet(cmzn_mesh_create_mesh_scale_factor_set(id));
}

int ElementTemplate::setNumberOfScaleFactors(MeshScaleFactorSet &scaleFactorSet, int numberOfScaleFactors)
{
	return cmzn_element_template_set_number_of_scale_factors(id, scaleFactorSet.getId(),
		numberOfScaleFactors);
}

int Element::getScaleFactors(MeshScaleFactorSet& scaleFactorSet, int valuesCount, double *values)
{
	return cmzn_element_get_scale_factors(id, scaleFactorSet.getId(), valuesCount, values);
}

int Element::setScaleFactors(MeshScaleFactorSet& scaleFactorSet, int valuesCount, const double *values)
{
	return cmzn_element_set_scale_factors(id, scaleFactorSet.getId(), valuesCount, values);
}

}  // namespace Zinc
}

#endif /* CMZN_ELEMENT_HPP__ */
