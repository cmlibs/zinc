/***************************************************************************//**
 * FILE : cmiss_element_private.cpp
 *
 * Implementation of public interface to Cmiss_element, finite element meshes.
 *
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
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005-2010
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

extern "C" {
#include "api/cmiss_element.h"
#include "element/element_operations.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "computed_field/computed_field_finite_element.h"
#include "user_interface/message.h"
}
#include "mesh/cmiss_element_private.hpp"
#include <vector>
#include "computed_field/computed_field_private.hpp"

namespace {

inline int Cmiss_element_shape_type_get_dimension(
	Cmiss_element_shape_type shape_type)
{
	switch (shape_type)
	{
		case CMISS_ELEMENT_SHAPE_LINE:
			return 1;
			break;
		case CMISS_ELEMENT_SHAPE_SQUARE:
		case CMISS_ELEMENT_SHAPE_TRIANGLE:
			return 2;
			break;
		case CMISS_ELEMENT_SHAPE_CUBE:
		case CMISS_ELEMENT_SHAPE_TETRAHEDRON:
		case CMISS_ELEMENT_SHAPE_WEDGE12:
		case CMISS_ELEMENT_SHAPE_WEDGE13:
		case CMISS_ELEMENT_SHAPE_WEDGE23:
			return 3;
			break;
		default:
			// do nothing
			break;
	}
	return 0;
}

}

/*
Global types
------------
*/

/*============================================================================*/

struct Cmiss_element_basis
{
private:
	FE_region *fe_region; // needed to get basis manager
	int dimensions;
	Cmiss_basis_function_type *function_types;
	int access_count;

public:
	Cmiss_element_basis(FE_region *fe_region, int dimensions,
			Cmiss_basis_function_type function_type) :
		fe_region(ACCESS(FE_region)(fe_region)),
		dimensions(dimensions),
		function_types(new Cmiss_basis_function_type[dimensions]),
		access_count(1)
	{
		for (int i = 0; i < dimensions; i++)
		{
			function_types[i] = function_type;
		}
	}

	static int deaccess(Cmiss_element_basis_id &basis)
	{
		if (!basis)
			return 0;
		--(basis->access_count);
		if (basis->access_count <= 0)
			delete basis;
		basis = 0;
		return 1;
	}

	int getDimensions() const { return dimensions; }

	/** @return  number of dimensions using supplied function_type */
	int getDimensionsUsingFunction(Cmiss_basis_function_type function_type) const
	{
		int count = 0;
		for (int i = 0; i < dimensions; i++)
		{
			if (function_types[i] == function_type)
				count++;
		}
		return count;
	}

	/**
	 * @return  1 if all function types set & at least 2 dimensions for each
	 * simplex basis */
	int isValid() const
	{
		int return_code = 1;
		if (0 < getDimensionsUsingFunction(CMISS_BASIS_FUNCTION_TYPE_INVALID))
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_element_basis::isValid.  Function type not set");
			return_code = 0;
		}
		if ((1 == getDimensionsUsingFunction(CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX)) ||
			(1 == getDimensionsUsingFunction(CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX)))
		{
			display_message(ERROR_MESSAGE, "Cmiss_element_basis::isValid.  "
				"Must be at least 2 linked dimension for simplex basis");
			return_code = 0;
		}
		return return_code;
	}

	/** @return  Accessed FE_basis, or NULL on error */
	FE_basis *getFeBasis()
	{
		if (!isValid())
			return NULL;
		const int length = dimensions*(dimensions + 1)/2 + 1;
		int *int_basis_type_array;
		if (!ALLOCATE(int_basis_type_array, int, length))
			return NULL;
		*int_basis_type_array = dimensions;
		int *temp = int_basis_type_array + 1;
		for (int i = 0; i < dimensions; i++)
		{
			FE_basis_type fe_basis_type = FE_BASIS_TYPE_INVALID;
			switch (function_types[i])
			{
			case CMISS_BASIS_FUNCTION_CONSTANT:
				fe_basis_type = FE_BASIS_CONSTANT;
				break;
			case CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE:
				fe_basis_type = LINEAR_LAGRANGE;
				break;
			case CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE:
				fe_basis_type = QUADRATIC_LAGRANGE;
				break;
			case CMISS_BASIS_FUNCTION_CUBIC_LAGRANGE:
				fe_basis_type = CUBIC_LAGRANGE;
				break;
			case CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX:
				fe_basis_type = LINEAR_SIMPLEX;
				break;
			case CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX:
				fe_basis_type = QUADRATIC_SIMPLEX;
				break;
			default:
				fe_basis_type = FE_BASIS_TYPE_INVALID;
				break;
			}
			*temp = (int)fe_basis_type;
			++temp;
			for (int j = i + 1; j < dimensions; j++)
			{
				if (((fe_basis_type == LINEAR_SIMPLEX) ||
					(fe_basis_type == QUADRATIC_SIMPLEX)) &&
					(function_types[j] == function_types[i]))
				{
					*temp = 1;
				}
				else
				{
					*temp = 0; // NO_RELATION
				}
				++temp;
			}
		}
		struct FE_basis *fe_basis = FE_region_get_FE_basis_matching_basis_type(
			fe_region, int_basis_type_array);
		DEALLOCATE(int_basis_type_array);
		return ACCESS(FE_basis)(fe_basis);
	}

	enum Cmiss_basis_function_type getFunctionType(int dimension) const
	{
		if ((dimension < 1) || (dimension > dimensions))
			return CMISS_BASIS_FUNCTION_TYPE_INVALID;
		return function_types[dimension-1];
	}

	int setFunctionType(int dimension, Cmiss_basis_function_type function_type)
	{
		if ((dimension < 1) || (dimension > dimensions))
			return 0;
		function_types[dimension-1] = function_type;
		return 1;
	}

	int getNumberOfNodes() const
	{
		// GRC this should be handled by making an FE_basis and asking it
		int number_of_nodes = 1;
		int linearSimplexDimensions =
			getDimensionsUsingFunction(CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX);
		switch (linearSimplexDimensions)
		{
			case 0:
				// do nothing
				break;
			case 2:
				number_of_nodes *= 3;
				break;
			case 3:
				number_of_nodes *= 4;
				break;
			default:
				number_of_nodes = 0;
				break;
		}
		int quadraticSimplexDimensions =
			getDimensionsUsingFunction(CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX);
		switch (quadraticSimplexDimensions)
		{
			case 0:
				// do nothing
				break;
			case 2:
				number_of_nodes *= 6;
				break;
			case 3:
				number_of_nodes *= 10;
				break;
			default:
				number_of_nodes = 0;
				break;
		}
		if (0 == number_of_nodes)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_element_basis::getNumberOfNodes.  "
				"Invalid number of dimensions linked in simplex");
			return 0;
		}
		for (int i = 0; i < dimensions; i++)
		{
			switch (function_types[i])
			{
				case CMISS_BASIS_FUNCTION_CONSTANT:
					break;
				case CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE:
					number_of_nodes *= 2;
					break;
				case CMISS_BASIS_FUNCTION_QUADRATIC_LAGRANGE:
					number_of_nodes *= 3;
					break;
				case CMISS_BASIS_FUNCTION_CUBIC_LAGRANGE:
					number_of_nodes *= 4;
					break;
				case CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX:
				case CMISS_BASIS_FUNCTION_QUADRATIC_SIMPLEX:
					// handled above
					break;
				default:
					display_message(ERROR_MESSAGE,
						"Cmiss_element_basis::getNumberOfNodes.  "
						"Invalid or unsupported basis type: %d", function_types[i]);
					number_of_nodes = 0;
					break;
			}
		}
		return number_of_nodes;
	}

private:
	~Cmiss_element_basis()
	{
		DEACCESS(FE_region)(&fe_region);
		delete[] function_types;
	}
};

/*============================================================================*/

namespace {

class Cmiss_element_field
{
	FE_field *fe_field;
	const int number_of_components;
	FE_element_field_component **components;

public:

	Cmiss_element_field(FE_field *fe_field) :
		fe_field(ACCESS(FE_field)(fe_field)),
		number_of_components(get_FE_field_number_of_components(fe_field)),
		components(new FE_element_field_component*[number_of_components])
	{
		for (int i = 0; i < number_of_components; i++)
			components[i] = 0;
	}

	~Cmiss_element_field()
	{
		DEACCESS(FE_field)(&fe_field);
		for (int i = 0; i < number_of_components; i++)
		{
			clearComponent(i);
		}
		delete[] components;
	}

	/** @param component_number  -1 for all components, or positive for single component */
	int buildComponent(int component_number, FE_basis *fe_basis, int number_of_nodes,
		const int *local_node_indexes)
	{
		if ((component_number < -1) || (component_number == 0) || (component_number > number_of_components))
			return 0;
		FE_element_field_component *component =
			CREATE(FE_element_field_component)(STANDARD_NODE_TO_ELEMENT_MAP,
				number_of_nodes, fe_basis, (FE_element_field_component_modify)NULL);
		if (component)
		{
			for (int node_index = 0; node_index < number_of_nodes; node_index++)
			{
				Standard_node_to_element_map *standard_node_map =
					CREATE(Standard_node_to_element_map)(local_node_indexes[node_index] - 1, /*number_of_values*/1);
				if (!(Standard_node_to_element_map_set_nodal_value_index(
						standard_node_map, /*value_number*/0, /*nodal_value_index*/0) &&
					Standard_node_to_element_map_set_scale_factor_index(
						standard_node_map, /*value_number*/0, /*no_scale_factor*/-1) &&
					FE_element_field_component_set_standard_node_map(component,
						node_index, standard_node_map)))
				{
					DESTROY(FE_element_field_component)(&component);
					component = NULL;
					break;
				}
			}
		}
		if (component)
		{
			int first = 0;
			int limit = number_of_components;
			if (component_number > 0)
			{
				first = component_number - 1;
				limit = component_number;
			}
			for (int i = first; i < limit; i++)
			{
				clearComponent(i);
				components[i] = component;
			}
			return 1;
		}
		return 0;
	}

	int defineOnElement(FE_element *element)
	{
		return define_FE_field_at_element(element, fe_field, components);
	}

	FE_field *getFeField() const { return fe_field; }

	/** @return  1 if all components have been defined */
	int isValid() const
	{
		for (int i = 0; i < number_of_components; i++)
		{
			if (NULL == components[i])
				return 0;
		}
		return 1;
	}

private:
	/** clear element field component pointer, but only destroy if no longer in use */
	void clearComponent(int component_index)
	{
		FE_element_field_component *component = components[component_index];
		components[component_index] = NULL;
		for (int i = 0; i < number_of_components; i++)
		{
			if (components[i] == component)
				return;
		}
		DESTROY(FE_element_field_component)(&component);
		return;
	}
};

}

/*============================================================================*/

struct Cmiss_element_template
{
	friend class Cmiss_mesh; // to obtain template_element
private:
	FE_region *fe_region;
	int element_dimension;
	Cmiss_element_shape_type shape_type;
	bool shape_is_set;
	int element_number_of_nodes;
	FE_element *template_element;
	std::vector<Cmiss_element_field*> fields;
	int access_count;

public:
	Cmiss_element_template(FE_region *fe_region, int element_dimension) :
		fe_region(ACCESS(FE_region)(fe_region)),
		element_dimension(element_dimension),
		shape_type(CMISS_ELEMENT_SHAPE_TYPE_INVALID),
		shape_is_set(false),
		element_number_of_nodes(0),
		template_element(NULL),
		access_count(1)
	{
	}

	static int deaccess(Cmiss_element_template_id &element_template)
	{
		if (!element_template)
			return 0;
		--(element_template->access_count);
		if (element_template->access_count <= 0)
			delete element_template;
		element_template = 0;
		return 1;
	}

	Cmiss_element_shape_type getShapeType() const { return shape_type; }

	int setShapeType(Cmiss_element_shape_type in_shape_type)
	{
		if (in_shape_type != CMISS_ELEMENT_SHAPE_TYPE_INVALID)
		{
			int shape_dimension = Cmiss_element_shape_type_get_dimension(in_shape_type);
			if (shape_dimension != element_dimension)
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_element_template::setShapeType.  Shape dimension is different from mesh");
				return 0;
			}
		}
		shape_is_set = true;
		if (in_shape_type == shape_type)
			return 1;
		shape_type = in_shape_type;
		clearTemplateElement();
		return 1;
	}

	int getNumberOfNodes() const { return element_number_of_nodes; }

	int setNumberOfNodes(int in_element_number_of_nodes)
	{
		if (in_element_number_of_nodes < element_number_of_nodes)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_element_template_set_number_of_nodes.  Cannot reduce number of nodes");
			return 0;
		}
		element_number_of_nodes = in_element_number_of_nodes;
		return 1;
	}

	int defineFieldSimpleNodal(Cmiss_field_id field,
		int component_number, Cmiss_element_basis_id basis, int basis_number_of_nodes,
		const int *local_node_indexes)
	{
		int return_code = 1;
		if (basis->getDimensions() != element_dimension)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_element_template_define_field_simple_nodal.  "
				"Basis has different number of dimensions to mesh");
			return_code = 0;
		}
		FE_basis *fe_basis = basis->getFeBasis();
		if (!fe_basis)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_element_template_define_field_simple_nodal.  "
				"Basis is invalid or incomplete");
			return_code = 0;
		}
		else
		{
			int expected_basis_number_of_nodes = basis->getNumberOfNodes();
			if (basis_number_of_nodes != expected_basis_number_of_nodes)
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_element_template_define_field_simple_nodal.  "
					"%d nodes supplied but %d expected for basis",
					basis_number_of_nodes, expected_basis_number_of_nodes);
				return_code = 0;
			}
		}
		for (int i = 0; i < basis_number_of_nodes; i++)
		{
			if ((local_node_indexes[i] < 1) || (local_node_indexes[i] > element_number_of_nodes))
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_element_template_define_field_simple_nodal.  "
					"Local node index out of range 1 to %d", element_number_of_nodes);
				return_code = 0;
				break;
			}
		}
		FE_region *master_FE_region = fe_region;
		FE_region_get_ultimate_master_FE_region(fe_region, &master_FE_region);
		Cmiss_field_finite_element_id finite_element_field = Cmiss_field_cast_finite_element(field);
		FE_field *fe_field = NULL;
		if (finite_element_field)
		{
			Computed_field_get_type_finite_element(field, &fe_field);
			if (FE_field_get_FE_region(fe_field) != master_FE_region)
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_element_template_define_field_simple_nodal.  "
					"Field is from another region");
				return_code = 0;
			}
			Cmiss_field_finite_element_destroy(&finite_element_field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_element_template_define_field_simple_nodal.  "
				"Can only define real finite_element field type on elements");
			return_code = 0;
		}
		if (return_code)
		{
			Cmiss_element_field& element_field = getElementField(fe_field);
			if (element_field.buildComponent(component_number, fe_basis, basis_number_of_nodes, local_node_indexes))
			{
				clearTemplateElement();
			}
			else
			{
				return_code = 0;
			}
		}
		REACCESS(FE_basis)(&fe_basis, NULL);
		return return_code;
	}

	int finalise()
	{
		if (template_element)
			return 1;
		int return_code = 1;
		if (!shape_is_set)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_element_template_finalise.  Element shape has not been set");
			return_code = 0;
		}
		for (unsigned int i = 0; i < fields.size(); i++)
		{
			if (!fields[i]->isValid())
			{
				char *field_name = NULL;
				GET_NAME(FE_field)(fields[i]->getFeField(), &field_name);
				display_message(ERROR_MESSAGE, "Cmiss_element_template_finalise.  "
					"Field %s definition is invalid or incomplete", field_name);
				DEALLOCATE(field_name);
				return_code = 0;
			}
		}
		if (return_code)
		{
			FE_element_shape *fe_element_shape = NULL;
			if (shape_type == CMISS_ELEMENT_SHAPE_TYPE_INVALID)
			{
				fe_element_shape = FE_element_shape_create_unspecified(fe_region, element_dimension);
			}
			else
			{
				fe_element_shape = FE_element_shape_create_simple_type(fe_region, shape_type);
			}
			if (fe_element_shape)
			{
				CM_element_information cm = { CM_ELEMENT, 0 };
				template_element = ACCESS(FE_element)(CREATE(FE_element)(&cm,
					fe_element_shape, fe_region, /*template_element*/NULL));
				set_FE_element_number_of_nodes(template_element, element_number_of_nodes);
				for (unsigned int i = 0; i < fields.size(); i++)
				{
					if (!fields[i]->defineOnElement(template_element))
					{
						DEACCESS(FE_element)(&template_element);
						break;
					}
				}
				DEACCESS(FE_element_shape)(&fe_element_shape);
			}
			if (!template_element)
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_element_template_finalise.  Failed to create template element");
				return_code = 0;
			}
		}
		return return_code;
	}

	int isFinalised() const { return (NULL != template_element); }

	Cmiss_node_id getNode(int local_node_index)
	{
		if (isFinalised())
		{
			Cmiss_node_id node = NULL;
			if (get_FE_element_node(getTemplateElement(), local_node_index - 1, &node))
				return ACCESS(FE_node)(node);
		}
		return NULL;
	}

	int setNode(int local_node_index, Cmiss_node_id node)
	{
		if (isFinalised())
		{
			return set_FE_element_node(getTemplateElement(), local_node_index - 1, node);
		}
		return 0;
	}

	int mergeIntoElement(Cmiss_element_id element)
	{
		int return_code = 0;
		if (isFinalised())
		{
			if (shape_type == CMISS_ELEMENT_SHAPE_TYPE_INVALID)
			{
				// leave the element shape intact
				struct FE_element_shape *element_shape = 0;
				get_FE_element_shape(element, &element_shape);
				set_FE_element_shape(template_element, element_shape);
			}
			return_code = FE_region_merge_FE_element_existing(
				FE_element_get_FE_region(element), element, template_element);
		}
		return return_code;
	}

private:
	~Cmiss_element_template()
	{
		for (unsigned int i = 0; i < fields.size(); i++)
		{
			delete fields[i];
		}
		REACCESS(FE_element)(&template_element, NULL);
		DEACCESS(FE_region)(&fe_region);
	}

	FE_element *getTemplateElement() { return template_element; }

	Cmiss_element_field& getElementField(FE_field *fe_field)
	{
		Cmiss_element_field *element_field = NULL;
		for (unsigned int i = 0; i < fields.size(); i++)
		{
			if (fields[i]->getFeField() == fe_field)
			{
				element_field = fields[i];
				break;
			}
		}
		if (!element_field)
		{
			element_field = new Cmiss_element_field(fe_field);
			fields.push_back(element_field);
		}
		return *element_field;
	}

	int hasFields() { return fields.size(); }

	void clearTemplateElement()
	{
		REACCESS(FE_element)(&template_element, NULL);
	}
};

/*============================================================================*/

struct Cmiss_mesh
{
private:
	FE_region *fe_region;
	int mesh_dimension;
	int access_count;

public:
	Cmiss_mesh(Cmiss_region_id region, int mesh_dimension) :
		fe_region(ACCESS(FE_region)(Cmiss_region_get_FE_region(region))),
		mesh_dimension(mesh_dimension),
		access_count(1)
	{
	}

	Cmiss_mesh_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(Cmiss_mesh_id &mesh)
	{
		if (!mesh)
			return 0;
		--(mesh->access_count);
		if (mesh->access_count <= 0)
			delete mesh;
		mesh = 0;
		return 1;
	}

	int containsElement(Cmiss_element_id element)
	{
		CM_element_information cm;
		get_FE_element_identifier(element, &cm);
		int element_dimension = get_FE_element_dimension(element);
		if (element_dimension == mesh_dimension)
		{
			FE_region *element_fe_region = FE_element_get_FE_region(element);
			if ((element_fe_region == fe_region) ||
				// special 'group region' check:
				FE_region_contains_FE_element(fe_region, element))
			{
				return 1;
			}
		}
		return 0;
	}

	Cmiss_element_id createElement(int identifier,
		Cmiss_element_template_id element_template)
	{
		FE_element *element = NULL;
		if (element_template->isFinalised())
		{
			FE_element *template_element = element_template->getTemplateElement();
			element = ACCESS(FE_element)(FE_region_create_FE_element_copy(
				fe_region, identifier, template_element));
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_mesh_create_element.  Element template is not finalised");
		}
		return element;
	}

	Cmiss_element_iterator_id createIterator()
	{
		return FE_region_create_element_iterator(fe_region, mesh_dimension);
	}

	int getDimension() const { return mesh_dimension; }

	FE_region *getFeRegion() const { return fe_region; }

	int getSize() const { return FE_region_get_number_of_FE_elements_of_dimension(fe_region, mesh_dimension); }

private:
	~Cmiss_mesh()
	{
		DEACCESS(FE_region)(&fe_region);
	}
};

/*
Global functions
----------------
*/

Cmiss_mesh_id Cmiss_field_module_get_mesh_by_name(
	Cmiss_field_module_id field_module, const char *mesh_name)
{
	Cmiss_mesh_id mesh = NULL;
	if (field_module && mesh_name)
	{
		int mesh_dimension = 0;
		if      (0 == strcmp(mesh_name, "cmiss_mesh_3d"))
			mesh_dimension = 3;
		else if (0 == strcmp(mesh_name, "cmiss_mesh_2d"))
			mesh_dimension = 2;
		else if (0 == strcmp(mesh_name, "cmiss_mesh_1d"))
			mesh_dimension = 1;
		if (0 < mesh_dimension)
		{
			mesh = new Cmiss_mesh(Cmiss_field_module_get_region_internal(field_module), mesh_dimension);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_get_mesh_by_name.  Unknown mesh name '%s'", mesh_name);
		}
	}
	return (mesh);
}

Cmiss_mesh_id Cmiss_mesh_access(Cmiss_mesh_id mesh)
{
	return mesh->access();
}

int Cmiss_mesh_destroy(Cmiss_mesh_id *mesh_address)
{
	if (mesh_address)
		return Cmiss_mesh::deaccess(*mesh_address);
	return 0;
}

int Cmiss_mesh_contains_element(Cmiss_mesh_id mesh, Cmiss_element_id element)
{
	if (mesh)
		return mesh->containsElement(element);
	return 0;
}

Cmiss_element_basis_id Cmiss_mesh_create_element_basis(Cmiss_mesh_id mesh,
	enum Cmiss_basis_function_type function_type)
{
	Cmiss_element_basis_id basis = NULL;
	if (mesh)
	{
		FE_region *fe_region = mesh->getFeRegion();
		FE_region_get_ultimate_master_FE_region(fe_region, &fe_region);
		basis = new Cmiss_element_basis(fe_region, mesh->getDimension(), function_type);
	}
	return basis;
}

Cmiss_element_template_id Cmiss_mesh_create_element_template(
	Cmiss_mesh_id mesh)
{
	if (mesh)
	{
		FE_region *fe_region = mesh->getFeRegion();
		FE_region_get_ultimate_master_FE_region(fe_region, &fe_region);
		return new Cmiss_element_template(fe_region, mesh->getDimension());
	}
	return NULL;
}

Cmiss_element_id Cmiss_mesh_create_element(Cmiss_mesh_id mesh,
	int identifier, Cmiss_element_template_id element_template)
{
	if (mesh && element_template)
		return mesh->createElement(identifier, element_template);
	return NULL;
}

Cmiss_element_iterator_id Cmiss_mesh_create_element_iterator(
	Cmiss_mesh_id mesh)
{
	if (mesh)
		return mesh->createIterator();
	return NULL;
}

int Cmiss_mesh_define_element(Cmiss_mesh_id mesh, int identifier,
	Cmiss_element_template_id element_template)
{
	Cmiss_element_id element =
		Cmiss_mesh_create_element(mesh, identifier, element_template);
	if (element)
	{
		Cmiss_element_destroy(&element);
		return 1;
	}
	return 0;
}

Cmiss_element_id Cmiss_mesh_find_element_by_identifier(Cmiss_mesh_id mesh,
	int identifier)
{
	Cmiss_element_id element = NULL;
	if (mesh)
	{
		element = FE_region_get_FE_element_from_identifier(mesh->getFeRegion(),
			mesh->getDimension(), identifier);
		if (element)
			ACCESS(FE_element)(element);
	}
	return element;
}

int Cmiss_mesh_get_size(Cmiss_mesh_id mesh)
{
	if (mesh)
	{
		return mesh->getSize();
	}
	return 0;
}

int Cmiss_mesh_get_dimension(Cmiss_mesh_id mesh)
{
	if (mesh)
	{
		return mesh->getDimension();
	}
	return 0;
}

char *Cmiss_mesh_get_name(Cmiss_mesh_id mesh)
{
	char *name = 0;
	if (mesh)
	{
		int error = 0;
		Cmiss_region_id region = 0;
		FE_region_get_Cmiss_region(mesh->getFeRegion(), &region);
		if (Cmiss_region_is_group(region))
		{
			char *group_name = Cmiss_region_get_name(region);
			append_string(&name, group_name, &error);
			append_string(&name, ".", &error);
			DEALLOCATE(group_name);
		}
		switch (mesh->getDimension())
		{
		case 1:
			append_string(&name, "cmiss_mesh_1d", &error);
			break;
		case 2:
			append_string(&name, "cmiss_mesh_2d", &error);
			break;
		case 3:
			append_string(&name, "cmiss_mesh_3d", &error);
			break;
		default:
			DEALLOCATE(name);
			name = 0;
			break;
		}
	}
	return name;
}

int Cmiss_mesh_remove_element(Cmiss_mesh_id mesh, Cmiss_element_id element)
{
	int return_code = 0;
	if (mesh && element)
	{
		FE_region *fe_region = mesh->getFeRegion();
		CM_element_information cm;
		get_FE_element_identifier(element, &cm);
		int element_dimension = get_FE_element_dimension(element);
		if (element_dimension == mesh->getDimension())
		{
			FE_region *master_fe_region = fe_region;
			FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
			return_code = FE_region_remove_FE_element(master_fe_region, element);
		}
	}

	return return_code;
}

int Cmiss_mesh_remove_elements_conditional(Cmiss_mesh_id mesh,
	Cmiss_field_id conditional_field)
{
	int return_code = 0;
	if (mesh && conditional_field)
	{
		FE_region *fe_region = mesh->getFeRegion();
		struct LIST(FE_element) *element_list =	FE_element_list_from_conditional_field_of_dimension(
			fe_region, mesh->getDimension(), conditional_field, /*time*/0);
		FE_region *master_fe_region = fe_region;
		FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
		return_code = FE_region_remove_FE_element_list(master_fe_region, element_list);
		DESTROY(LIST(FE_element))(&element_list);
	}

	return return_code;
}

FE_region *Cmiss_mesh_get_FE_region(Cmiss_mesh_id mesh)
{
	if (mesh)
	{
		return mesh->getFeRegion();
	}
	return 0;
}

Cmiss_region_id Cmiss_mesh_get_region(Cmiss_mesh_id mesh)
{
	Cmiss_region_id region = 0;
	if (mesh)
	{
		FE_region_get_Cmiss_region(mesh->getFeRegion(), &region);
	}
	return region;
}

int Cmiss_element_basis_destroy(Cmiss_element_basis_id *element_basis_address)
{
	if (element_basis_address)
		return Cmiss_element_basis::deaccess(*element_basis_address);
	return 0;
}

int Cmiss_element_basis_get_dimension(Cmiss_element_basis_id element_basis)
{
	if (element_basis)
		return element_basis->getDimensions();
	return 0;
}

enum Cmiss_basis_function_type Cmiss_element_basis_get_function_type(
	Cmiss_element_basis_id element_basis, int dimension)
{
	if (element_basis)
		return element_basis->getFunctionType(dimension);
	return CMISS_BASIS_FUNCTION_TYPE_INVALID;
}

int Cmiss_element_basis_set_function_type(Cmiss_element_basis_id element_basis,
	int dimension, enum Cmiss_basis_function_type function_type)
{
	if (element_basis)
		return element_basis->setFunctionType(dimension, function_type);
	return 0;
}

int Cmiss_element_basis_get_number_of_nodes(
	Cmiss_element_basis_id element_basis)
{
	if (element_basis)
		return element_basis->getNumberOfNodes();
	return 0;
}

int Cmiss_element_template_destroy(
	Cmiss_element_template_id *element_template_address)
{
	if (element_template_address)
		return Cmiss_element_template::deaccess(*element_template_address);
	return 0;
}

enum Cmiss_element_shape_type Cmiss_element_template_get_shape_type(
	Cmiss_element_template_id element_template)
{
	if (element_template)
		return element_template->getShapeType();
	return CMISS_ELEMENT_SHAPE_TYPE_INVALID;
}

int Cmiss_element_template_set_shape_type(Cmiss_element_template_id element_template,
	enum Cmiss_element_shape_type shape_type)
{
	if (element_template)
		return element_template->setShapeType(shape_type);
	return 0;
}

int Cmiss_element_template_get_number_of_nodes(
	Cmiss_element_template_id element_template)
{
	if (element_template)
		return element_template->getNumberOfNodes();
	return 0;
}

int Cmiss_element_template_set_number_of_nodes(
	Cmiss_element_template_id element_template, int number_of_nodes)
{
	if (element_template)
		return element_template->setNumberOfNodes(number_of_nodes);
	return 0;
}

int Cmiss_element_template_define_field_simple_nodal(
	Cmiss_element_template_id element_template,
	Cmiss_field_id field,  int component_number,
	Cmiss_element_basis_id basis, int number_of_nodes,
	const int *local_node_indexes)
{
	if (element_template && field && basis &&
		((0 == number_of_nodes) || (local_node_indexes)))
	{
		return element_template->defineFieldSimpleNodal(
			field, component_number, basis, number_of_nodes, local_node_indexes);
	}
	return 0;
}

int Cmiss_element_template_finalise(
	Cmiss_element_template_id element_template)
{
	if (element_template)
		return element_template->finalise();
	return 0;
}

Cmiss_node_id Cmiss_element_template_get_node(
	Cmiss_element_template_id element_template, int local_node_index)
{
	if (element_template)
		return element_template->getNode(local_node_index);
	return NULL;
}

int Cmiss_element_template_set_node(Cmiss_element_template_id element_template,
	int local_node_index, Cmiss_node_id node)
{
	if (element_template)
		return element_template->setNode(local_node_index, node);
	return 0;
}

Cmiss_element_id Cmiss_element_access(Cmiss_element_id element)
{
	return ACCESS(FE_element)(element);
}

int Cmiss_element_destroy(Cmiss_element_id *element_address)
{
	return DEACCESS(FE_element)(element_address);
}

int Cmiss_element_get_dimension(Cmiss_element_id element)
{
	return get_FE_element_dimension(element);
}

int Cmiss_element_get_identifier(struct Cmiss_element *element)
{
	int return_code = -1;
	struct CM_element_information cm;
	if (get_FE_element_identifier(element, &cm))
	{
		/* CM_element_type is understood from mesh */
		return_code = cm.number;
	}
	return (return_code);
}

enum Cmiss_element_shape_type Cmiss_element_get_shape_type(
	Cmiss_element_id element)
{
	Cmiss_element_shape_type shape_type = CMISS_ELEMENT_SHAPE_TYPE_INVALID;
	if (element)
	{
		struct FE_element_shape *fe_element_shape = NULL;
		get_FE_element_shape(element, &fe_element_shape);
		shape_type = FE_element_shape_get_simple_type(fe_element_shape);
	}
	return shape_type;
}

int Cmiss_element_merge(Cmiss_element_id element,
	Cmiss_element_template_id element_template)
{
	if (element && element_template)
		return element_template->mergeIntoElement(element);
	return 0;
}

enum Cmiss_element_shape_type Cmiss_element_shape_type_enum_from_string(
	const char *string)
{
	enum Cmiss_element_shape_type type = (Cmiss_element_shape_type)0;
	if (string)
	{
		const char *str[] = {"LINE", "SQUARE", "TRIANGLE", "CUBE",
			"TETRAHEDRON", "WEDGE12", "WEDGE13", "WEDGE23"};
		for (unsigned int i = 0; i < 8; i ++)
		{
			if (!strcmp(str[i], string))
			{
				type = (Cmiss_element_shape_type)(i + 1);
				break;
			}
		}
	}
	return type;
}

char *Cmiss_element_shape_type_enum_to_string(enum Cmiss_element_shape_type type)
{
	char *string = NULL;
	if (0 < type && type <= 8)
	{
		const char *str[] = {"LINE", "SQUARE", "TRIANGLE", "CUBE",
			"TETRAHEDRON", "WEDGE12", "WEDGE13", "WEDGE23"};
		string = duplicate_string(str[type - 1]);
	}
	return string;
}

enum Cmiss_basis_function_type Cmiss_basis_function_type_enum_from_string(
	const char *string)
{
	enum Cmiss_basis_function_type type = (Cmiss_basis_function_type)0;
	if (string)
	{
		const char *str[] = {"CONSTAN", "LINEAR_LAGRANGE", "QUADRATIC_LAGRANGE",
			"CUBIC_LAGRANG", "LINEAR_SIMPLEX", "QUADRATIC_SIMPLEX"};
		for (unsigned int i = 0; i < 6; i ++)
		{
			if (!strcmp(str[i], string))
			{
				type = (Cmiss_basis_function_type)(i + 1);
				break;
			}
		}
	}
	return type;
}

char *Cmiss_basis_function_type_enum_to_string(enum Cmiss_basis_function_type type)
{
	char *string = NULL;
	if (0 < type && type <= 6)
	{
		const char *str[] = {"CONSTAN", "LINEAR_LAGRANGE", "QUADRATIC_LAGRANGE",
			"CUBIC_LAGRANG", "LINEAR_SIMPLEX", "QUADRATIC_SIMPLEX"};
		string = duplicate_string(str[type - 1]);
	}
	return string;
}
