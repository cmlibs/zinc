/***************************************************************************//**
 * FILE : cmiss_element_private.cpp
 *
 * Implementation of public interface to cmzn_element, finite element meshes.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/element.h"
#include "opencmiss/zinc/fieldsubobjectgroup.h"
#include "opencmiss/zinc/fieldmodule.h"
#include "opencmiss/zinc/status.h"
#include "element/element_operations.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_region.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_subobject_group.hpp"
#include "computed_field/differential_operator.hpp"
#include "computed_field/field_module.hpp"
#include "general/enumerator_conversion.hpp"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/cmiss_node_private.hpp"
#include <map>
#include <vector>

namespace {

inline int cmzn_element_shape_type_get_dimension(
	cmzn_element_shape_type shape_type)
{
	switch (shape_type)
	{
		case CMZN_ELEMENT_SHAPE_TYPE_LINE:
			return 1;
			break;
		case CMZN_ELEMENT_SHAPE_TYPE_SQUARE:
		case CMZN_ELEMENT_SHAPE_TYPE_TRIANGLE:
			return 2;
			break;
		case CMZN_ELEMENT_SHAPE_TYPE_CUBE:
		case CMZN_ELEMENT_SHAPE_TYPE_TETRAHEDRON:
		case CMZN_ELEMENT_SHAPE_TYPE_WEDGE12:
		case CMZN_ELEMENT_SHAPE_TYPE_WEDGE13:
		case CMZN_ELEMENT_SHAPE_TYPE_WEDGE23:
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

struct cmzn_elementbasis
{
private:
	FE_region *fe_region; // needed to get basis manager
	int dimension;
	cmzn_elementbasis_function_type *function_types;
	int access_count;

public:
	cmzn_elementbasis(FE_region *fe_region, int mesh_dimension,
			cmzn_elementbasis_function_type function_type) :
		fe_region(ACCESS(FE_region)(fe_region)),
		dimension(mesh_dimension),
		function_types(new cmzn_elementbasis_function_type[mesh_dimension]),
		access_count(1)
	{
		for (int i = 0; i < dimension; i++)
		{
			function_types[i] = function_type;
		}
	}

	cmzn_elementbasis_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_elementbasis_id &basis)
	{
		if (!basis)
			return CMZN_ERROR_ARGUMENT;
		--(basis->access_count);
		if (basis->access_count <= 0)
			delete basis;
		basis = 0;
		return CMZN_OK;
	}

	int getDimension() const { return dimension; }

	/** @return  number of dimension using supplied function_type */
	int getDimensionsUsingFunction(cmzn_elementbasis_function_type function_type) const
	{
		int count = 0;
		for (int i = 0; i < dimension; i++)
		{
			if (function_types[i] == function_type)
				count++;
		}
		return count;
	}

	/**
	 * @return  1 if all function types set & at least 2 chart components linked
	 * for each simplex basis */
	int isValid() const
	{
		int return_code = 1;
		if (0 < getDimensionsUsingFunction(CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID))
		{
			display_message(ERROR_MESSAGE,
				"cmzn_elementbasis::isValid.  Function type not set");
			return_code = 0;
		}
		if ((1 == getDimensionsUsingFunction(CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_SIMPLEX)) ||
			(1 == getDimensionsUsingFunction(CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_SIMPLEX)))
		{
			display_message(ERROR_MESSAGE, "cmzn_elementbasis::isValid.  "
				"Must be at least 2 linked dimension for simplex basis");
			return_code = 0;
		}
		return return_code;
	}

	/** @return  Accessed FE_basis, or NULL on error */
	FE_basis *getFeBasis() const
	{
		if (!isValid())
			return 0;
		const int length = dimension*(dimension + 1)/2 + 1;
		int *int_basis_type_array;
		if (!ALLOCATE(int_basis_type_array, int, length))
			return 0;
		*int_basis_type_array = dimension;
		int *temp = int_basis_type_array + 1;
		for (int i = 0; i < dimension; i++)
		{
			FE_basis_type fe_basis_type = cmzn_elementbasis_function_type_to_FE_basis_type(function_types[i]);
			*temp = (int)fe_basis_type;
			++temp;
			for (int j = i + 1; j < dimension; j++)
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

	enum cmzn_elementbasis_function_type getFunctionType(int chart_component) const
	{
		if ((chart_component < 1) || (chart_component > dimension))
			return CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID;
		return function_types[chart_component - 1];
	}

	int setFunctionType(int chart_component, cmzn_elementbasis_function_type function_type)
	{
		if ((chart_component < 1) || (chart_component > dimension))
			return 0;
		function_types[chart_component - 1] = function_type;
		return 1;
	}

	int getNumberOfNodes() const
	{
		FE_basis *basis = this->getFeBasis();
		int numberOfNodes = FE_basis_get_number_of_nodes(basis);
		DEACCESS(FE_basis)(&basis);
		return numberOfNodes;
	}

	int getNumberOfFunctions() const
	{
		FE_basis *basis = this->getFeBasis();
		int numberOfFunctions = FE_basis_get_number_of_functions(basis);
		DEACCESS(FE_basis)(&basis);
		return numberOfFunctions;
	}

	int getNumberOfFunctionsPerNode(int basisNodeIndex) const
	{
		FE_basis *basis = this->getFeBasis();
		int numberOfFunctionsPerNode = FE_basis_get_number_of_functions_per_node(basis, basisNodeIndex - 1);
		DEACCESS(FE_basis)(&basis);
		return numberOfFunctionsPerNode;
	}

private:
	~cmzn_elementbasis()
	{
		DEACCESS(FE_region)(&fe_region);
		delete[] function_types;
	}
};

/*============================================================================*/

namespace {

class cmzn_element_field
{
	FE_field *fe_field;
	const int number_of_components;
	FE_element_field_component **components;

public:

	cmzn_element_field(FE_field *fe_field) :
		fe_field(ACCESS(FE_field)(fe_field)),
		number_of_components(get_FE_field_number_of_components(fe_field)),
		components(new FE_element_field_component*[number_of_components])
	{
		for (int i = 0; i < number_of_components; i++)
			components[i] = 0;
	}

	~cmzn_element_field()
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
		if ((component_number < -1) || (component_number == 0) || (component_number > number_of_components) ||
				(FE_basis_get_number_of_nodes(fe_basis) != number_of_nodes))
			return CMZN_ERROR_ARGUMENT;
		FE_element_field_component *component =
			CREATE(FE_element_field_component)(STANDARD_NODE_TO_ELEMENT_MAP,
				number_of_nodes, fe_basis, (FE_element_field_component_modify)NULL);
		for (int nodeIndex = 0; (nodeIndex < number_of_nodes) && component; nodeIndex++)
		{
			const int numberOfValues = FE_basis_get_number_of_functions_per_node(fe_basis, nodeIndex);
			Standard_node_to_element_map *standard_node_map =
				Standard_node_to_element_map_create(local_node_indexes[nodeIndex] - 1, numberOfValues);
			for (int valueNumber = 0; valueNumber < numberOfValues; ++valueNumber)
			{
				if (!(Standard_node_to_element_map_set_nodal_value_type(
						standard_node_map, valueNumber, static_cast<FE_nodal_value_type>(valueNumber + FE_NODAL_VALUE)) &&
					Standard_node_to_element_map_set_nodal_version(
						standard_node_map, valueNumber, static_cast<FE_nodal_value_type>(1)) &&
					Standard_node_to_element_map_set_scale_factor_index(
						standard_node_map, valueNumber, /*no_scale_factor*/-1)))
				{
					DESTROY(FE_element_field_component)(&component);
					break;
				}
			}
			if (!FE_element_field_component_set_standard_node_map(component,
				nodeIndex, standard_node_map))
			{
				DESTROY(FE_element_field_component)(&component);
				Standard_node_to_element_map_destroy(&standard_node_map);
				break;
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
				this->components[i] = component;
			}
			return CMZN_OK;
		}
		return CMZN_ERROR_GENERAL;
	}

	int buildElementConstantComponent(int component_number, FE_basis *fe_basis)
	{
		if ((component_number < -1) || (component_number == 0) || (component_number > number_of_components))
			return CMZN_ERROR_ARGUMENT;
		FE_element_field_component *component =
			CREATE(FE_element_field_component)(ELEMENT_GRID_MAP,
				1, fe_basis, (FE_element_field_component_modify)NULL);
		if (component)
		{
			int basis_dimension = 0;
			FE_basis_get_dimension(fe_basis, &basis_dimension);
			for (int i = 0; i < basis_dimension; i++)
			{
				FE_element_field_component_set_grid_map_number_in_xi(
					component, i, 0);
			}
		}
		if (component)
		{
			FE_element_field_component_set_grid_map_value_index(
				component, 0);
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
			return CMZN_OK;
		}
		return CMZN_ERROR_MEMORY;
	}

	/** @param component_number  -1 for all components, or positive for single component */
	int setMapNodeValueLabel(int componentNumber, int basisNodeIndex,
		int nodeFunctionIndex, cmzn_node_value_label nodeValueLabel)
	{
		if ((componentNumber < -1) || (componentNumber == 0) || (componentNumber > this->number_of_components) ||
				(basisNodeIndex < 1) || (nodeFunctionIndex < 1))
			return CMZN_ERROR_ARGUMENT;
		const int first = (componentNumber > 0) ? (componentNumber - 1) : 0;
		const int limit = (componentNumber > 0) ? componentNumber : this->number_of_components;
		for (int i = first; i < limit; i++)
		{
			Global_to_element_map_type mapType;
			if ((0 == this->components[i]) ||
					(!FE_element_field_component_get_type(this->components[i], &mapType)) ||
					(STANDARD_NODE_TO_ELEMENT_MAP != mapType))
				return CMZN_ERROR_ARGUMENT;
		}
		int result = componentCopyOnWrite(componentNumber);
		if (CMZN_OK != result)
			return result;
		for (int i = first; i < limit; i++)
		{
			FE_element_field_component *component = this->components[i];
			Standard_node_to_element_map *standard_node_map;
			if (!FE_element_field_component_get_standard_node_map(component, basisNodeIndex - 1, &standard_node_map))
				return CMZN_ERROR_ARGUMENT;
			FE_nodal_value_type fe_nodal_value_type = cmzn_node_value_label_to_FE_nodal_value_type(nodeValueLabel);
			if (!Standard_node_to_element_map_set_nodal_value_type(standard_node_map, nodeFunctionIndex - 1, fe_nodal_value_type))
				return CMZN_ERROR_GENERAL;
		}
		return CMZN_OK;
	}

	/** @param component_number  -1 for all components, or positive for single component */
	int setMapNodeVersion(int componentNumber, int basisNodeIndex,
		int nodeFunctionIndex, int versionNumber)
	{
		if ((componentNumber < -1) || (componentNumber == 0) || (componentNumber > this->number_of_components) ||
				(basisNodeIndex < 1) || (nodeFunctionIndex < 1) || (versionNumber < 1))
			return CMZN_ERROR_ARGUMENT;
		const int first = (componentNumber > 0) ? (componentNumber - 1) : 0;
		const int limit = (componentNumber > 0) ? componentNumber : this->number_of_components;
		for (int i = first; i < limit; i++)
		{
			Global_to_element_map_type mapType;
			if ((0 == this->components[i]) ||
					(!FE_element_field_component_get_type(this->components[i], &mapType)) ||
					(STANDARD_NODE_TO_ELEMENT_MAP != mapType))
				return CMZN_ERROR_ARGUMENT;
		}
		int result = componentCopyOnWrite(componentNumber);
		if (CMZN_OK != result)
			return result;
		for (int i = first; i < limit; i++)
		{
			FE_element_field_component *component = this->components[i];
			Standard_node_to_element_map *standard_node_map;
			if (!FE_element_field_component_get_standard_node_map(component, basisNodeIndex - 1, &standard_node_map))
				return CMZN_ERROR_ARGUMENT;
			if (!Standard_node_to_element_map_set_nodal_version(standard_node_map, nodeFunctionIndex - 1, versionNumber))
				return CMZN_ERROR_GENERAL;
		}
		return CMZN_OK;
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

	/** copy component map if shared by another component not being modified */
	int componentCopyOnWrite(int componentNumber)
	{
		if ((1 != this->number_of_components) && (componentNumber > 0))
		{
			const int index = (componentNumber > 0) ? (componentNumber - 1) : 0;
			FE_element_field_component *component = this->components[index];
			for (int i = 0; i < this->number_of_components; i++)
			{
				if ((i != index) && (this->components[i] == component))
				{
					component = copy_create_FE_element_field_component(component);
					if (!component)
						return CMZN_ERROR_MEMORY;
					break;
				}
			}
		}
		return CMZN_OK;		
	}

};

}

/*============================================================================*/

struct cmzn_elementtemplate
{
	friend struct cmzn_mesh; // to obtain internal FE_element_template
private:
	FE_mesh *fe_mesh;
	cmzn_element_shape_type shape_type;
	bool shape_is_set;
	int element_number_of_nodes;
	FE_element_template *fe_element_template;
	std::vector<cmzn_element_field*> fields;
	int access_count;
	typedef std::map<cmzn_mesh_scale_factor_set*,int> ScaleFactorSetIntMap;
	ScaleFactorSetIntMap scale_factor_set_sizes;

public:
	cmzn_elementtemplate(FE_mesh *fe_mesh_in) :
		fe_mesh(fe_mesh_in->access()),
		shape_type(CMZN_ELEMENT_SHAPE_TYPE_INVALID),
		shape_is_set(false),
		element_number_of_nodes(0),
		fe_element_template(0),
		access_count(1)
	{
	}

	cmzn_elementtemplate_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_elementtemplate_id &element_template)
	{
		if (!element_template)
			return CMZN_ERROR_ARGUMENT;
		--(element_template->access_count);
		if (element_template->access_count <= 0)
			delete element_template;
		element_template = 0;
		return CMZN_OK;
	}

	cmzn_element_shape_type getShapeType() const { return shape_type; }

	int setShapeType(cmzn_element_shape_type in_shape_type)
	{
		if (in_shape_type != CMZN_ELEMENT_SHAPE_TYPE_INVALID)
		{
			int shape_dimension = cmzn_element_shape_type_get_dimension(in_shape_type);
			if (shape_dimension != this->fe_mesh->getDimension())
			{
				display_message(ERROR_MESSAGE,
					"cmzn_elementtemplate::setShapeType.  Shape dimension is different from mesh");
				return 0;
			}
		}
		shape_is_set = true;
		if (in_shape_type == shape_type)
			return 1;
		shape_type = in_shape_type;
		this->invalidate();
		return 1;
	}

	int setNumberOfScaleFactors(cmzn_mesh_scale_factor_set_id scale_factor_set, int numberOfScaleFactors)
	{
		int return_code = CMZN_OK;
		if (scale_factor_set && (0 <= numberOfScaleFactors))
		{
			ScaleFactorSetIntMap::iterator iter = this->scale_factor_set_sizes.find(scale_factor_set);
			if (iter != scale_factor_set_sizes.end())
			{
				if (0 == numberOfScaleFactors)
				{
					cmzn_mesh_scale_factor_set *temp = scale_factor_set;
					cmzn_mesh_scale_factor_set::deaccess(temp);
					scale_factor_set_sizes.erase(iter);
					this->invalidate();
				}
				else if (numberOfScaleFactors != iter->second)
				{
					display_message(ERROR_MESSAGE, "cmzn_elementtemplate_set_number_of_scale_factors.  "
						"Can't change number size of a scale factor set in element template");
					return_code = CMZN_ERROR_ARGUMENT;
				}
			}
			else if (0 < numberOfScaleFactors)
			{
				scale_factor_set->access();
				scale_factor_set_sizes[scale_factor_set] = numberOfScaleFactors;
				this->invalidate();
			}
		}
		else
		{
			return_code = CMZN_ERROR_ARGUMENT;
		}
		return return_code;
	}

	int getNumberOfNodes() const { return element_number_of_nodes; }

	int setNumberOfNodes(int in_element_number_of_nodes)
	{
		if (in_element_number_of_nodes < element_number_of_nodes)
		{
			display_message(ERROR_MESSAGE,
				"cmzn_elementtemplate_set_number_of_nodes.  Cannot reduce number of nodes");
			return CMZN_ERROR_ARGUMENT;
		}
		element_number_of_nodes = in_element_number_of_nodes;
		this->invalidate();
		return CMZN_OK;
	}

	int defineFieldSimpleNodal(cmzn_field_id field,
		int component_number, cmzn_elementbasis_id basis, int basis_number_of_nodes,
		const int *local_node_indexes)
	{
		int return_code = CMZN_OK;
		if (basis->getDimension() != this->fe_mesh->getDimension())
		{
			display_message(ERROR_MESSAGE,
				"cmzn_elementtemplate_define_field_simple_nodal.  "
				"Basis has different dimension to mesh");
			return_code = CMZN_ERROR_ARGUMENT;
		}
		FE_basis *fe_basis = basis->getFeBasis();
		if (!fe_basis)
		{
			display_message(ERROR_MESSAGE,
				"cmzn_elementtemplate_define_field_simple_nodal.  "
				"Basis is invalid or incomplete");
			return_code = CMZN_ERROR_ARGUMENT;
		}
		else
		{
			int expected_basis_number_of_nodes = basis->getNumberOfNodes();
			if (basis_number_of_nodes != expected_basis_number_of_nodes)
			{
				display_message(ERROR_MESSAGE,
					"cmzn_elementtemplate_define_field_simple_nodal.  "
					"%d nodes supplied but %d expected for basis",
					basis_number_of_nodes, expected_basis_number_of_nodes);
				return_code = CMZN_ERROR_ARGUMENT;
			}
		}
		for (int i = 0; i < basis_number_of_nodes; i++)
		{
			if ((local_node_indexes[i] < 1) || (local_node_indexes[i] > element_number_of_nodes))
			{
				display_message(ERROR_MESSAGE,
					"cmzn_elementtemplate_define_field_simple_nodal.  "
					"Local node index out of range 1 to number in element(%d)", element_number_of_nodes);
				return_code = CMZN_ERROR_ARGUMENT;
				break;
			}
		}
		cmzn_field_finite_element_id finite_element_field = cmzn_field_cast_finite_element(field);
		FE_field *fe_field = NULL;
		if (finite_element_field)
		{
			Computed_field_get_type_finite_element(field, &fe_field);
			if (FE_field_get_FE_region(fe_field) != this->fe_mesh->get_FE_region())
			{
				display_message(ERROR_MESSAGE,
					"cmzn_elementtemplate_define_field_simple_nodal.  "
					"Field is from another region");
				return_code = CMZN_ERROR_ARGUMENT;
			}
			cmzn_field_finite_element_destroy(&finite_element_field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_elementtemplate_define_field_simple_nodal.  "
				"Can only define real finite_element field type on elements");
			return_code = CMZN_ERROR_ARGUMENT;
		}
		if (CMZN_OK == return_code)
		{
			cmzn_element_field& element_field = getOrCreateElementField(fe_field);
			return_code = element_field.buildComponent(component_number, fe_basis, basis_number_of_nodes, local_node_indexes);
			this->invalidate();
		}
		REACCESS(FE_basis)(&fe_basis, NULL);
		return return_code;
	}

	int defineFieldElementConstant(cmzn_field_id field, int component_number)
	{
		int return_code = CMZN_OK;

		cmzn_elementbasis_id basis = new cmzn_elementbasis(
			this->fe_mesh->get_FE_region(), this->fe_mesh->getDimension(), CMZN_ELEMENTBASIS_FUNCTION_TYPE_CONSTANT);
		FE_basis *fe_basis = basis->getFeBasis();
		if (!fe_basis)
		{
			display_message(ERROR_MESSAGE,
				"cmzn_elementtemplate_define_field_element_constant.  "
				"Basis is invalid or incomplete");
			return_code = CMZN_ERROR_ARGUMENT;
		}
		cmzn_field_finite_element_id finite_element_field = cmzn_field_cast_finite_element(field);
		FE_field *fe_field = NULL;
		if (finite_element_field)
		{
			Computed_field_get_type_finite_element(field, &fe_field);
			if (FE_field_get_FE_region(fe_field) != this->fe_mesh->get_FE_region())
			{
				display_message(ERROR_MESSAGE,
					"cmzn_elementtemplate_define_field_simple_nodal.  "
					"Field is from another region");
				return_code = CMZN_ERROR_ARGUMENT;
			}
			cmzn_field_finite_element_destroy(&finite_element_field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_elementtemplate_define_field_simple_nodal.  "
				"Can only define real finite_element field type on elements");
			return_code = CMZN_ERROR_ARGUMENT;
		}
		if (CMZN_OK == return_code)
		{
			cmzn_element_field& element_field = getOrCreateElementField(fe_field);
			return_code = element_field.buildElementConstantComponent(component_number, fe_basis);
			this->invalidate();
		}
		REACCESS(FE_basis)(&fe_basis, NULL);
		cmzn_elementbasis_destroy(&basis);
		return return_code;
	}

	int setMapNodeValueLabel(cmzn_field_id field, int componentNumber,
		int basisNodeIndex, int nodeFunctionIndex, cmzn_node_value_label nodeValueLabel)
	{
		FE_field *fe_field = 0;
		if (Computed_field_get_type_finite_element(field, &fe_field))
		{
			cmzn_element_field *element_field = this->getElementField(fe_field);
			if (element_field)
				return element_field->setMapNodeValueLabel(componentNumber, basisNodeIndex, nodeFunctionIndex, nodeValueLabel);
		}
		return CMZN_ERROR_ARGUMENT;
	}

	int setMapNodeVersion(cmzn_field_id field, int componentNumber,
		int basisNodeIndex, int nodeFunctionIndex, int versionNumber)
	{
		FE_field *fe_field = 0;
		if (Computed_field_get_type_finite_element(field, &fe_field))
		{
			cmzn_element_field *element_field = this->getElementField(fe_field);
			if (element_field)
				return element_field->setMapNodeVersion(componentNumber, basisNodeIndex, nodeFunctionIndex, versionNumber);
		}
		return CMZN_ERROR_ARGUMENT;
	}

	int validate()
	{
		if (this->fe_element_template)
			return 1;
		int return_code = 1;
		if (!shape_is_set)
		{
			display_message(ERROR_MESSAGE,
				"cmzn_elementtemplate_validate.  Element shape has not been set");
			return_code = 0;
		}
		for (unsigned int i = 0; i < fields.size(); i++)
		{
			if (!fields[i]->isValid())
			{
				char *field_name = NULL;
				GET_NAME(FE_field)(fields[i]->getFeField(), &field_name);
				display_message(ERROR_MESSAGE, "cmzn_elementtemplate_validate.  "
					"Field %s definition is invalid or incomplete", field_name);
				DEALLOCATE(field_name);
				return_code = 0;
			}
		}
		if (return_code)
		{
			FE_element_shape *fe_element_shape = NULL;
			if (shape_type == CMZN_ELEMENT_SHAPE_TYPE_INVALID)
			{
				fe_element_shape = FE_element_shape_create_unspecified(this->fe_mesh->get_FE_region(), this->fe_mesh->getDimension());
			}
			else
			{
				fe_element_shape = FE_element_shape_create_simple_type(this->fe_mesh->get_FE_region(), shape_type);
			}
			if (fe_element_shape)
			{
				this->fe_element_template = this->fe_mesh->create_FE_element_template(fe_element_shape);
				set_FE_element_number_of_nodes(this->fe_element_template->get_template_element(), element_number_of_nodes);
				const int number_of_scale_factor_sets = static_cast<int>(scale_factor_set_sizes.size());
				if (0 < number_of_scale_factor_sets)
				{
					cmzn_mesh_scale_factor_set **scale_factor_set_identifiers =
						new cmzn_mesh_scale_factor_set*[number_of_scale_factor_sets];
					int *numbers_in_scale_factor_sets = new int[number_of_scale_factor_sets];
					int i = 0;
					for (ScaleFactorSetIntMap::iterator iter = scale_factor_set_sizes.begin();
						iter != scale_factor_set_sizes.end(); ++iter)
					{
						scale_factor_set_identifiers[i] = iter->first;
						numbers_in_scale_factor_sets[i] = iter->second;
						++i;
					}
					set_FE_element_number_of_scale_factor_sets(this->fe_element_template->get_template_element(), number_of_scale_factor_sets,
						scale_factor_set_identifiers, numbers_in_scale_factor_sets);
					delete[] scale_factor_set_identifiers;
					delete[] numbers_in_scale_factor_sets;
				}
				for (unsigned int i = 0; i < fields.size(); i++)
				{
					if (!fields[i]->defineOnElement(this->fe_element_template->get_template_element()))
					{
						cmzn::Deaccess(this->fe_element_template);
						break;
					}
				}
				DEACCESS(FE_element_shape)(&fe_element_shape);
			}
			if (!this->fe_element_template)
			{
				display_message(ERROR_MESSAGE,
					"cmzn_elementtemplate_validate.  Failed to create fe_element_template");
				return_code = 0;
			}
		}
		return return_code;
	}

	cmzn_node_id getNode(int local_node_index)
	{
		if (validate())
		{
			cmzn_node_id node = NULL;
			if (get_FE_element_node(this->fe_element_template->get_template_element(), local_node_index - 1, &node))
				return ACCESS(FE_node)(node);
		}
		return 0;
	}

	int setNode(int local_node_index, cmzn_node_id node)
	{
		if (validate() && set_FE_element_node(this->fe_element_template->get_template_element(), local_node_index - 1, node))
			return CMZN_OK;
		return CMZN_ERROR_GENERAL;
	}

	int mergeIntoElement(cmzn_element_id element)
	{
		FE_mesh *target_fe_mesh = FE_element_get_FE_mesh(element);
		if (target_fe_mesh == this->fe_mesh)
		{
			if (this->validate())
				return this->fe_mesh->merge_FE_element_template(element, this->fe_element_template);
			else
				display_message(ERROR_MESSAGE, "cmzn_element_merge.  Element template is not valid");
		}
		else
			display_message(ERROR_MESSAGE, "cmzn_element_merge.  Incompatible template");
		return CMZN_ERROR_ARGUMENT;
	}

private:
	~cmzn_elementtemplate()
	{
		for (ScaleFactorSetIntMap::iterator iter = scale_factor_set_sizes.begin();
			iter != scale_factor_set_sizes.end(); ++iter)
		{
			cmzn_mesh_scale_factor_set *temp = iter->first;
			cmzn_mesh_scale_factor_set::deaccess(temp);
		}
		for (unsigned int i = 0; i < fields.size(); i++)
		{
			delete fields[i];
		}
		cmzn::Deaccess(this->fe_element_template);
		FE_mesh::deaccess(this->fe_mesh);
	}

	FE_element_template *get_FE_element_template() { return this->fe_element_template; }

	cmzn_element_field *getElementField(FE_field *fe_field)
	{
		for (unsigned int i = 0; i < fields.size(); i++)
		{
			if (fields[i]->getFeField() == fe_field)
				return fields[i];
		}
		return 0;
	}

	cmzn_element_field& getOrCreateElementField(FE_field *fe_field)
	{
		cmzn_element_field *element_field = this->getElementField(fe_field);
		if (!element_field)
		{
			element_field = new cmzn_element_field(fe_field);
			fields.push_back(element_field);
		}
		return *element_field;
	}

	bool hasFields() { return (0 < fields.size()); }

	void invalidate()
	{
		cmzn::Deaccess(this->fe_element_template);
	}
};

/*============================================================================*/

struct cmzn_mesh
{
protected:
	FE_mesh *fe_mesh;
	cmzn_field_element_group_id group;
	int access_count;

	cmzn_mesh(cmzn_field_element_group_id group) :
		fe_mesh(Computed_field_element_group_core_cast(group)->get_fe_mesh()->access()),
		group(group),
		access_count(1)
	{
		// cmzn_field_element_group_access missing:
		cmzn_field_access(cmzn_field_element_group_base_cast(group));
	}

	~cmzn_mesh()
	{
		if (group)
			cmzn_field_element_group_destroy(&group);
		FE_mesh::deaccess(this->fe_mesh);
	}

public:
	cmzn_mesh(FE_mesh *fe_mesh_in) :
		fe_mesh(fe_mesh_in->access()),
		group(0),
		access_count(1)
	{
	}

	cmzn_mesh_id access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_mesh_id &mesh)
	{
		if (!mesh)
			return CMZN_ERROR_ARGUMENT;
		--(mesh->access_count);
		if (mesh->access_count <= 0)
			delete mesh;
		mesh = 0;
		return CMZN_OK;
	}

	bool containsElement(cmzn_element_id element)
	{
		if (group)
			return Computed_field_element_group_core_cast(group)->containsObject(element);
		return FE_element_get_FE_mesh(element) == this->fe_mesh;
	}

	cmzn_element_id createElement(int identifier,
		cmzn_elementtemplate_id element_template)
	{
		FE_element *element = NULL;
		if (element_template->validate())
		{
			if (group)
				FE_region_begin_change(this->fe_mesh->get_FE_region());
			element = this->fe_mesh->create_FE_element(identifier, element_template->get_FE_element_template());
			if (group)
			{
				Computed_field_element_group_core_cast(group)->addObject(element);
				FE_region_end_change(this->fe_mesh->get_FE_region());
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_mesh_create_element.  Element template is not valid");
		}
		return element;
	}

	cmzn_elementtemplate_id createElementtemplate()
	{
		return new cmzn_elementtemplate(this->fe_mesh);
	}

	cmzn_elementiterator_id createElementiterator()
	{
		if (group)
			return Computed_field_element_group_core_cast(group)->createElementiterator();
		return this->fe_mesh->createElementiterator();
	}

	int destroyAllElements()
	{
		if (this->group)
		{
			Computed_field_element_group *element_group = Computed_field_element_group_core_cast(this->group);
			return this->fe_mesh->destroyElementsInGroup(element_group->getLabelsGroup());
		}
		return this->fe_mesh->destroyAllElements();
	}

	int destroyElement(cmzn_element_id element)
	{
		if (this->containsElement(element))
			return this->fe_mesh->destroyElement(element);
		return 0;
	}

	int destroyElementsConditional(cmzn_field_id conditional_field)
	{
		if (!conditional_field)
			return CMZN_ERROR_ARGUMENT;
		DsLabelsGroup *labelsGroup = this->fe_mesh->createLabelsGroup();
		if (labelsGroup)
		{
			cmzn_region_id region = FE_region_get_cmzn_region(this->fe_mesh->get_FE_region());
			cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
			cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(fieldmodule);
			cmzn_elementiterator_id iterator = this->createElementiterator();
			cmzn_element_id element = 0;
			while (0 != (element = cmzn_elementiterator_next_non_access(iterator)))
			{
				cmzn_fieldcache_set_element(cache, element);
				if (cmzn_field_evaluate_boolean(conditional_field, cache))
					labelsGroup->setIndex(get_FE_element_index(element), true);
			}
			cmzn::Deaccess(iterator);
			cmzn_fieldcache_destroy(&cache);
			cmzn_fieldmodule_destroy(&fieldmodule);
			int return_code = this->fe_mesh->destroyElementsInGroup(*labelsGroup);
			cmzn::Deaccess(labelsGroup);
			return return_code;
		}
		return CMZN_ERROR_GENERAL;
	}

	cmzn_element_id findElementByIdentifier(int identifier) const
	{
		cmzn_element_id element = 0;
		if (group)
		{
			element = Computed_field_element_group_core_cast(group)->findElementByIdentifier(identifier);
		}
		else
		{
			element = this->fe_mesh->findElementByIdentifier(identifier);
		}
		if (element)
			ACCESS(FE_element)(element);
		return element;
	}

	int getDimension() const { return this->fe_mesh->getDimension(); }

	FE_mesh *get_FE_mesh() const { return this->fe_mesh; }

	cmzn_mesh_scale_factor_set *findMeshScaleFactorSetByName(const char *name)
	{
		return this->fe_mesh->find_scale_factor_set_by_name(name);
	}

	cmzn_mesh_scale_factor_set *createMeshScaleFactorSet()
	{
		return this->fe_mesh->create_scale_factor_set();
	}

	char *getName()
	{
		char *name = 0;
		if (group)
		{
			name = cmzn_field_get_name(cmzn_field_element_group_base_cast(group));
		}
		else
		{
			switch (this->fe_mesh->getDimension())
			{
			case 1:
				name = duplicate_string("mesh1d");
				break;
			case 2:
				name = duplicate_string("mesh2d");
				break;
			case 3:
				name = duplicate_string("mesh3d");
				break;
			default:
				break;
			}
		}
		return name;
	}

	cmzn_mesh_id getMaster()
	{
		if (!isGroup())
			return access();
		return new cmzn_mesh(this->fe_mesh);
	}

	int getSize() const
	{
		if (group)
			return Computed_field_element_group_core_cast(group)->getSize();
		return this->fe_mesh->getSize();
	}

	int isGroup()
	{
		return (0 != group);
	}

	// if mesh is a group, return the element group field, otherwise return 0
	cmzn_field_element_group *getGroupField()
	{
		return this->group;
	}

	bool match(cmzn_mesh& other_mesh)
	{
		return ((this->fe_mesh == other_mesh.fe_mesh) &&
			(group == other_mesh.group));
	}

};

struct cmzn_mesh_group : public cmzn_mesh
{
public:

	cmzn_mesh_group(cmzn_field_element_group_id group) :
		cmzn_mesh(group)
	{
	}

	int addElement(cmzn_element_id element)
	{
		return Computed_field_element_group_core_cast(group)->addObject(element);
	}

	int addElementsConditional(cmzn_field_id conditional_field)
	{
		return Computed_field_element_group_core_cast(group)->addElementsConditional(conditional_field);
	}

	int removeAllElements()
	{
		return Computed_field_element_group_core_cast(group)->clear();
	}

	int removeElement(cmzn_element_id element)
	{
		return Computed_field_element_group_core_cast(group)->removeObject(element);
	}

	int removeElementsConditional(cmzn_field_id conditional_field)
	{
		return Computed_field_element_group_core_cast(group)->removeElementsConditional(conditional_field);
	}

	int addElementFaces(cmzn_element_id element)
	{
		return Computed_field_element_group_core_cast(group)->addElementFaces(element);
	}

	int removeElementFaces(cmzn_element_id element)
	{
		return Computed_field_element_group_core_cast(group)->removeElementFaces(element);
	}

};

/*
Global functions
----------------
*/

cmzn_elementbasis_id cmzn_fieldmodule_create_elementbasis(
	cmzn_fieldmodule_id fieldmodule, int dimension,
	enum cmzn_elementbasis_function_type function_type)
{
	if (fieldmodule && (0 < dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		cmzn_region *region = cmzn_fieldmodule_get_region_internal(fieldmodule);
		FE_region *fe_region = cmzn_region_get_FE_region(region);
		if (fe_region)
		{
			return new cmzn_elementbasis(fe_region, dimension, function_type);
		}
	}
	return 0;
}

cmzn_mesh_id cmzn_fieldmodule_find_mesh_by_dimension(
	cmzn_fieldmodule_id fieldmodule, int dimension)
{
	cmzn_mesh_id mesh = NULL;
	if (fieldmodule && (1 <= dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		FE_region *fe_region = cmzn_region_get_FE_region(cmzn_fieldmodule_get_region_internal(fieldmodule));
		mesh = new cmzn_mesh(FE_region_find_FE_mesh_by_dimension(fe_region, dimension));
	}
	return mesh;
}

cmzn_mesh_id cmzn_fieldmodule_find_mesh_by_name(
	cmzn_fieldmodule_id fieldmodule, const char *mesh_name)
{
	cmzn_mesh_id mesh = 0;
	if (fieldmodule && mesh_name)
	{
		cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(fieldmodule, mesh_name);
		if (field)
		{
			cmzn_field_element_group_id element_group_field = cmzn_field_cast_element_group(field);
			if (element_group_field)
			{
				mesh = cmzn_mesh_group_base_cast(cmzn_field_element_group_get_mesh_group(element_group_field));
				cmzn_field_element_group_destroy(&element_group_field);
			}
			cmzn_field_destroy(&field);
		}
		else
		{
			int mesh_dimension = 0;
			if      (0 == strcmp(mesh_name, "mesh3d"))
				mesh_dimension = 3;
			else if (0 == strcmp(mesh_name, "mesh2d"))
				mesh_dimension = 2;
			else if (0 == strcmp(mesh_name, "mesh1d"))
				mesh_dimension = 1;
			if (0 < mesh_dimension)
			{
				mesh = cmzn_fieldmodule_find_mesh_by_dimension(fieldmodule, mesh_dimension);
			}
		}
	}
	return (mesh);
}

cmzn_mesh_id cmzn_mesh_access(cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->access();
	return 0;
}

int cmzn_mesh_destroy(cmzn_mesh_id *mesh_address)
{
	if (mesh_address)
		return cmzn_mesh::deaccess(*mesh_address);
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_mesh_contains_element(cmzn_mesh_id mesh, cmzn_element_id element)
{
	if (mesh)
		return mesh->containsElement(element);
	return false;
}

cmzn_elementtemplate_id cmzn_mesh_create_elementtemplate(
	cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->createElementtemplate();
	return 0;
}

cmzn_element_id cmzn_mesh_create_element(cmzn_mesh_id mesh,
	int identifier, cmzn_elementtemplate_id element_template)
{
	if (mesh && element_template)
		return mesh->createElement(identifier, element_template);
	return 0;
}

cmzn_elementiterator_id cmzn_mesh_create_elementiterator(
	cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->createElementiterator();
	return 0;
}

int cmzn_mesh_define_element(cmzn_mesh_id mesh, int identifier,
	cmzn_elementtemplate_id element_template)
{
	cmzn_element_id element =
		cmzn_mesh_create_element(mesh, identifier, element_template);
	if (element)
	{
		cmzn_element_destroy(&element);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_destroy_all_elements(cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->destroyAllElements();
	return 0;
}

int cmzn_mesh_destroy_element(cmzn_mesh_id mesh, cmzn_element_id element)
{
	if (mesh && element)
		return mesh->destroyElement(element);
	return 0;
}

int cmzn_mesh_destroy_elements_conditional(cmzn_mesh_id mesh,
	cmzn_field_id conditional_field)
{
	if (mesh)
		return mesh->destroyElementsConditional(conditional_field);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_element_id cmzn_mesh_find_element_by_identifier(cmzn_mesh_id mesh,
	int identifier)
{
	if (mesh)
		return mesh->findElementByIdentifier(identifier);
	return 0;
}

/**
 * Find handle to the mesh scale factor set of the given name, if any.
 * Scale factors are stored in elements under a scale factor set.
 *
 * @param mesh  The mesh to query.
 * @param name  The name of the scale factor set. 
 * @return  Handle to the scale factor set, or 0 if none.
 * Up to caller to destroy handle.
 */
cmzn_mesh_scale_factor_set_id
cmzn_mesh_find_mesh_scale_factor_set_by_name(cmzn_mesh_id mesh,
	const char *name)
{
	if (mesh)
		return mesh->findMeshScaleFactorSetByName(name);
	return 0;
}

/**
 * Create a mesh scale factor set. The new set is given a unique name which
 * can be changed.
 * Scale factors are stored in elements under a scale factor set.
 *
 * @param mesh  The mesh to create the new set in.
 * @return  Handle to the new scale factor set, or 0 on failure. Up to caller
 * to destroy the returned handle.
 */
cmzn_mesh_scale_factor_set_id cmzn_mesh_create_mesh_scale_factor_set(
	cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->createMeshScaleFactorSet();
	return 0;
}

cmzn_differentialoperator_id cmzn_mesh_get_chart_differentialoperator(
	cmzn_mesh_id mesh, int order, int term)
{
	if (mesh && (1 == order) && (1 <= term) && (term <= mesh->getDimension()))
		return new cmzn_differentialoperator(mesh->get_FE_mesh()->get_FE_region(), mesh->getDimension(), term);
	return 0;
}

int cmzn_mesh_get_dimension(cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->getDimension();
	return 0;
}

cmzn_fieldmodule_id cmzn_mesh_get_fieldmodule(cmzn_mesh_id mesh)
{
	if (mesh)
	{
		cmzn_region *region = FE_region_get_cmzn_region(mesh->get_FE_mesh()->get_FE_region());
		return cmzn_fieldmodule_create(region);
	}
	return 0;
}

cmzn_mesh_id cmzn_mesh_get_master_mesh(cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->getMaster();
	return 0;
}

char *cmzn_mesh_get_name(cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->getName();
	return 0;
}

int cmzn_mesh_get_size(cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->getSize();
	return 0;
}

bool cmzn_mesh_match(cmzn_mesh_id mesh1, cmzn_mesh_id mesh2)
{
	return (mesh1 && mesh2 && mesh1->match(*mesh2));
}

cmzn_mesh_group_id cmzn_mesh_cast_group(cmzn_mesh_id mesh)
{
	if (mesh && mesh->isGroup())
		return static_cast<cmzn_mesh_group_id>(mesh->access());
	return 0;
}

int cmzn_mesh_group_destroy(cmzn_mesh_group_id *mesh_group_address)
{
	if (mesh_group_address)
		return cmzn_mesh::deaccess(*(reinterpret_cast<cmzn_mesh_id*>(mesh_group_address)));
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_add_element(cmzn_mesh_group_id mesh_group, cmzn_element_id element)
{
	if (mesh_group)
		return mesh_group->addElement(element);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_add_elements_conditional(cmzn_mesh_group_id mesh_group,
   cmzn_field_id conditional_field)
{
	if (mesh_group)
		return mesh_group->addElementsConditional(conditional_field);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_remove_all_elements(cmzn_mesh_group_id mesh_group)
{
	if (mesh_group)
		return mesh_group->removeAllElements();
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_remove_element(cmzn_mesh_group_id mesh_group, cmzn_element_id element)
{
	if (mesh_group)
		return mesh_group->removeElement(element);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_remove_elements_conditional(cmzn_mesh_group_id mesh_group,
	cmzn_field_id conditional_field)
{
	if (mesh_group)
		return mesh_group->removeElementsConditional(conditional_field);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_add_element_faces(cmzn_mesh_group_id mesh_group, cmzn_element_id element)
{
	if (mesh_group)
		return mesh_group->addElementFaces(element);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_mesh_group_remove_element_faces(cmzn_mesh_group_id mesh_group, cmzn_element_id element)
{
	if (mesh_group)
		return mesh_group->removeElementFaces(element);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_mesh_group_id cmzn_field_element_group_get_mesh_group(
	cmzn_field_element_group_id element_group)
{
	if (element_group)
		return new cmzn_mesh_group(element_group);
	return 0;
}

FE_mesh *cmzn_mesh_get_FE_mesh_internal(cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->get_FE_mesh();
	return 0;
}

FE_region *cmzn_mesh_get_FE_region_internal(cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->get_FE_mesh()->get_FE_region();
	return 0;
}

cmzn_region_id cmzn_mesh_get_region_internal(cmzn_mesh_id mesh)
{
	if (!mesh)
		return 0;
	return FE_region_get_cmzn_region(mesh->get_FE_mesh()->get_FE_region());
}

cmzn_field_element_group *cmzn_mesh_get_element_group_field_internal(cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->getGroupField();
	return 0;
}

cmzn_elementbasis_id cmzn_elementbasis_access(
	cmzn_elementbasis_id element_basis)
{
	if (element_basis)
		return element_basis->access();
	return 0;
}

int cmzn_elementbasis_destroy(cmzn_elementbasis_id *element_basis_address)
{
	if (element_basis_address)
		return cmzn_elementbasis::deaccess(*element_basis_address);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_elementbasis_get_dimension(cmzn_elementbasis_id element_basis)
{
	if (element_basis)
		return element_basis->getDimension();
	return 0;
}

enum cmzn_elementbasis_function_type cmzn_elementbasis_get_function_type(
	cmzn_elementbasis_id element_basis, int chart_component)
{
	if (element_basis)
		return element_basis->getFunctionType(chart_component);
	return CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID;
}

int cmzn_elementbasis_set_function_type(cmzn_elementbasis_id element_basis,
	int chart_component, enum cmzn_elementbasis_function_type function_type)
{
	if (element_basis)
		return element_basis->setFunctionType(chart_component, function_type);
	return 0;
}

int cmzn_elementbasis_get_number_of_nodes(
	cmzn_elementbasis_id element_basis)
{
	if (element_basis)
		return element_basis->getNumberOfNodes();
	return 0;
}

int cmzn_elementbasis_get_number_of_functions(
	cmzn_elementbasis_id element_basis)
{
	if (element_basis)
		return element_basis->getNumberOfFunctions();
	return 0;
}

int cmzn_elementbasis_get_number_of_functions_per_node(
	cmzn_elementbasis_id element_basis, int basis_node_index)
{
	if (element_basis)
		return element_basis->getNumberOfFunctionsPerNode(basis_node_index);
	return 0;
}

cmzn_elementtemplate_id cmzn_elementtemplate_access(
	cmzn_elementtemplate_id element_template)
{
	if (element_template)
		return element_template->access();
	return 0;
}

int cmzn_elementtemplate_destroy(
	cmzn_elementtemplate_id *element_template_address)
{
	if (element_template_address)
		return cmzn_elementtemplate::deaccess(*element_template_address);
	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_element_shape_type cmzn_elementtemplate_get_element_shape_type(
	cmzn_elementtemplate_id element_template)
{
	if (element_template)
		return element_template->getShapeType();
	return CMZN_ELEMENT_SHAPE_TYPE_INVALID;
}

int cmzn_elementtemplate_set_element_shape_type(cmzn_elementtemplate_id element_template,
	enum cmzn_element_shape_type shape_type)
{
	if (element_template)
		return element_template->setShapeType(shape_type);
	return 0;
}

/**
 * Sets the number of scale factors to be used with a given scale factor set in
 * elements defined from this template.
 * Note: The number of scale factors is arbitrary for a scale factor set and
 * an element, but cannot be changed once set.
 * Scale factor indices in element parameter maps are relative to scale factor
 * set for the element field component, and start at 1.
 *
 * @param element_template  Element template to modify.
 * @param scale_factor_set  The mesh scale factor set to assign numbers of scale
 * factors for.
 * @param number_of_scale_factors  The number of scale factors to set.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
int cmzn_elementtemplate_set_number_of_scale_factors(
	cmzn_elementtemplate_id element_template,
	cmzn_mesh_scale_factor_set_id scale_factor_set, int number_of_scale_factors)
{
	if (element_template)
		return element_template->setNumberOfScaleFactors(scale_factor_set, number_of_scale_factors);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_elementtemplate_get_number_of_nodes(
	cmzn_elementtemplate_id element_template)
{
	if (element_template)
		return element_template->getNumberOfNodes();
	return 0;
}

int cmzn_elementtemplate_set_number_of_nodes(
	cmzn_elementtemplate_id element_template, int number_of_nodes)
{
	if (element_template)
		return element_template->setNumberOfNodes(number_of_nodes);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_elementtemplate_define_field_element_constant(cmzn_elementtemplate_id element_template,
	cmzn_field_id field, int component_number)
{
	if (element_template && field)
	{
		return element_template->defineFieldElementConstant(field, component_number);
	}
	return CMZN_ERROR_ARGUMENT;

}

int cmzn_elementtemplate_define_field_simple_nodal(
	cmzn_elementtemplate_id elementtemplate,
	cmzn_field_id field,  int component_number,
	cmzn_elementbasis_id basis, int number_of_nodes,
	const int *local_node_indexes)
{
	if (elementtemplate && field && basis &&
		((0 == number_of_nodes) || (local_node_indexes)))
	{
		return elementtemplate->defineFieldSimpleNodal(
			field, component_number, basis, number_of_nodes, local_node_indexes);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_elementtemplate_set_map_node_value_label(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field,
	int component_number, int basis_node_index, int node_function_index,
	enum cmzn_node_value_label node_value_label)
{
	if (elementtemplate)
		return elementtemplate->setMapNodeValueLabel(field, component_number,
			basis_node_index, node_function_index, node_value_label);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_elementtemplate_set_map_node_version(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field,
	int component_number, int basis_node_index, int node_function_index,
	int version_number)
{
	if (elementtemplate)
		return elementtemplate->setMapNodeVersion(field, component_number,
			basis_node_index, node_function_index, version_number);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_node_id cmzn_elementtemplate_get_node(
	cmzn_elementtemplate_id element_template, int local_node_index)
{
	if (element_template)
		return element_template->getNode(local_node_index);
	return NULL;
}

int cmzn_elementtemplate_set_node(cmzn_elementtemplate_id element_template,
	int local_node_index, cmzn_node_id node)
{
	if (element_template)
		return element_template->setNode(local_node_index, node);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_element_id cmzn_element_access(cmzn_element_id element)
{
	if (element)
		return ACCESS(FE_element)(element);
	return 0;
}

int cmzn_element_destroy(cmzn_element_id *element_address)
{
	if (element_address && (*element_address))
	{
		DEACCESS(FE_element)(element_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

/**
 * Gets the scale factors for a scale factor set in an element.
 *
 * @param element  The element to query.
 * @param scale_factor_set  The mesh scale factor set to get values for.
 * @param valuesCount  The size of the values array to receive scale factors,
 * which is the maximum number requested.
 * @param values  The array to receive the scale factors.
 * @return  The number of scale factors stored for the scale factor set in
 * element. Can be more or less than the number requested. Returns 0 on any
 * other error including bad arguments.
 */
int cmzn_element_get_scale_factors(cmzn_element_id element,
	cmzn_mesh_scale_factor_set_id scale_factor_set, int valuesCount,
	double *values)
{
	if (element && scale_factor_set &&
		((0 == valuesCount) || ((0 < valuesCount) && values)))
	{
		FE_value *scaleFactors = 0;
		int numberOfScaleFactors = get_FE_element_scale_factors_address(element, scale_factor_set, &scaleFactors);
		if (0 < numberOfScaleFactors)
		{
			for (int i = 0; i < numberOfScaleFactors; ++i)
			{
				values[i] = static_cast<double>(scaleFactors[i]);
			}
		}
		return numberOfScaleFactors;
	}
	return 0;
}

/**
 * Sets the scale factors for a scale factor set in an element. The number of
 * scale factors is arbitrary for a scale factor set in each element, but once
 * set it cannot be changed; it is settable only from the element template.
 * Each element field component has a single scale factor set from which it
 * gets scale factors.
 *
 * @param element  The element to modify.
 * @param scale_factor_set  The mesh scale factor set to set values for.
 * @param valuesCount  The number of scale factors to set. This must equal the
 * number of scale factors stored for the scale factor set in element.
 * @param values  The array of scale factors to set.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
int cmzn_element_set_scale_factors(cmzn_element_id element,
	cmzn_mesh_scale_factor_set_id scale_factor_set, int valuesCount,
	const double *values)
{
	if (element && scale_factor_set &&
		((0 == valuesCount) || ((0 < valuesCount) && values)))
	{
		FE_value *scaleFactors = 0;
		int numberOfScaleFactors = get_FE_element_scale_factors_address(element, scale_factor_set, &scaleFactors);
		if (numberOfScaleFactors == valuesCount)
		{
			for (int i = 0; i < numberOfScaleFactors; ++i)
			{
				scaleFactors[i] = static_cast<FE_value>(values[i]);
			}
			return CMZN_OK;
		}
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_element_get_dimension(cmzn_element_id element)
{
	return get_FE_element_dimension(element);
}

int cmzn_element_get_identifier(struct cmzn_element *element)
{
	return get_FE_element_identifier(element);
}

cmzn_mesh_id cmzn_element_get_mesh(cmzn_element_id element)
{
	FE_mesh *fe_mesh = FE_element_get_FE_mesh(element);
	if (fe_mesh)
		return new cmzn_mesh(fe_mesh);
	return 0;
}

enum cmzn_element_shape_type cmzn_element_get_shape_type(
	cmzn_element_id element)
{
	cmzn_element_shape_type shape_type = CMZN_ELEMENT_SHAPE_TYPE_INVALID;
	if (element)
	{
		FE_mesh *fe_mesh = FE_element_get_FE_mesh(element);
		if (fe_mesh)
			shape_type = fe_mesh->getElementShapeType(get_FE_element_index(element));
	}
	return shape_type;
}

int cmzn_element_merge(cmzn_element_id element,
	cmzn_elementtemplate_id element_template)
{
	if (element && element_template)
		return element_template->mergeIntoElement(element);
	return CMZN_ERROR_ARGUMENT;
}

class cmzn_element_shape_type_conversion
{
public:
	static const char *to_string(enum cmzn_element_shape_type type)
	{
		const char *enum_string = 0;
		switch (type)
		{
			case CMZN_ELEMENT_SHAPE_TYPE_LINE:
				enum_string = "LINE";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_SQUARE:
				enum_string = "SQUARE";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_TRIANGLE:
				enum_string = "TRIANGLE";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_CUBE:
				enum_string = "CUBE";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_TETRAHEDRON:
				enum_string = "TETRAHEDRON";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_WEDGE12:
				enum_string = "WEDGE12";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_WEDGE13:
				enum_string = "WEDGE13";
				break;
			case CMZN_ELEMENT_SHAPE_TYPE_WEDGE23:
				enum_string = "_WEDGE23";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum cmzn_element_shape_type cmzn_element_shape_type_enum_from_string(
	const char *string)
{
	return string_to_enum<enum cmzn_element_shape_type,	cmzn_element_shape_type_conversion>(string);
}

char *cmzn_element_shape_type_enum_to_string(enum cmzn_element_shape_type type)
{
	const char *type_string = cmzn_element_shape_type_conversion::to_string(type);
	return (type_string ? duplicate_string(type_string) : 0);
}

class cmzn_elementbasis_function_type_conversion
{
public:
	static const char *to_string(enum cmzn_elementbasis_function_type type)
	{
		const char *enum_string = 0;
		switch (type)
		{
			case CMZN_ELEMENTBASIS_FUNCTION_TYPE_CONSTANT:
				enum_string = "CONSTANT";
				break;
			case CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_LAGRANGE:
				enum_string = "LINEAR_LAGRANGE";
				break;
			case CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_LAGRANGE:
				enum_string = "QUADRATIC_LAGRANGE";
				break;
			case CMZN_ELEMENTBASIS_FUNCTION_TYPE_CUBIC_LAGRANGE:
				enum_string = "CUBIC_LAGRANGE";
				break;
			case CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_SIMPLEX:
				enum_string = "LINEAR_SIMPLEX";
				break;
			case CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_SIMPLEX:
				enum_string = "QUADRATIC_SIMPLEX";
				break;
			case CMZN_ELEMENTBASIS_FUNCTION_TYPE_CUBIC_HERMITE:
				enum_string = "CUBIC_HERMITE";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum cmzn_elementbasis_function_type cmzn_elementbasis_function_type_enum_from_string(
	const char *string)
{
	return string_to_enum<enum cmzn_elementbasis_function_type,	cmzn_elementbasis_function_type_conversion>(string);
}

char *cmzn_elementbasis_function_type_enum_to_string(enum cmzn_elementbasis_function_type type)
{
	const char *type_string = cmzn_elementbasis_function_type_conversion::to_string(type);
	return (type_string ? duplicate_string(type_string) : 0);
}

/**
 * Returns a new handle to the scale factor set with reference count
 * incremented. Caller is responsible for destroying the new handle.
 *
 * @param scale_factor_set  The mesh scale factor set to obtain a new
 * reference to.
 * @return  New handle to the scale factor set.
 */
cmzn_mesh_scale_factor_set_id cmzn_mesh_scale_factor_set_access(
	cmzn_mesh_scale_factor_set_id scale_factor_set)
{
	if (scale_factor_set)
		scale_factor_set->access();
	return scale_factor_set;
}

/**
 * Destroys this handle to the finite element mesh and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param mesh_address  Address of handle to the mesh to destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
int cmzn_mesh_scale_factor_set_destroy(
	cmzn_mesh_scale_factor_set_id *scale_factor_set_address)
{
	if (scale_factor_set_address)
	{
		return cmzn_mesh_scale_factor_set::deaccess(*scale_factor_set_address);
	}
	return CMZN_ERROR_ARGUMENT;
}

/**
 * Get the name of the mesh scale factor set.
 * @see cmzn_deallocate()
 *
 * @param scale_factor_set  The mesh scale factor set to query.
 * @return  On success: allocated string containing mesh name. Up to caller to
 * free using cmzn_deallocate().
 */
char *cmzn_mesh_scale_factor_set_get_name(
	cmzn_mesh_scale_factor_set_id scale_factor_set)
{
	if (scale_factor_set)
		return duplicate_string(scale_factor_set->getName());
	return 0;
}

/**
 * Set the name of the mesh scale factor set.
 *
 * @param scale_factor_set  The mesh scale factor set to modify.
 * @param name  The new name of the scale factor set; must not be in use by any
 * other set in the mesh.
 * @return  CMZN_OK on success, otherwise any other error code.
 */
int cmzn_mesh_scale_factor_set_set_name(
	cmzn_mesh_scale_factor_set_id scale_factor_set, const char *name)
{
	if (scale_factor_set && name)
		return scale_factor_set->setName(name);
	return 0;
}

cmzn_meshchanges::cmzn_meshchanges(cmzn_fieldmoduleevent *eventIn, cmzn_mesh *meshIn) :
	event(eventIn->access()),
	changeLog(cmzn::Access(eventIn->getFeRegionChanges()->getElementChangeLog(meshIn->getDimension()))),
	access_count(1)
{
}

cmzn_meshchanges::~cmzn_meshchanges()
{
	cmzn::Deaccess(this->changeLog);
	cmzn_fieldmoduleevent::deaccess(this->event);
}

cmzn_meshchanges *cmzn_meshchanges::create(cmzn_fieldmoduleevent *eventIn, cmzn_mesh *meshIn)
{
	if (eventIn && (eventIn->getFeRegionChanges()) && meshIn && 
		(cmzn_region_get_FE_region(eventIn->getRegion()) == cmzn_mesh_get_FE_region_internal(meshIn)))
		return new cmzn_meshchanges(eventIn, meshIn);
	return 0;
}

int cmzn_meshchanges::deaccess(cmzn_meshchanges* &meshchanges)
{
	if (meshchanges)
	{
		--(meshchanges->access_count);
		if (meshchanges->access_count <= 0)
			delete meshchanges;
		meshchanges = 0;
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

// Note: internal and external flags have same numerical values
cmzn_element_change_flags cmzn_meshchanges::getElementChangeFlags(cmzn_element *element)
{
	cmzn_element_change_flags change = CMZN_ELEMENT_CHANGE_FLAG_NONE;
	if (element)
	{
		if (this->changeLog->isIndexChange(get_FE_element_index(element)))
			change = this->changeLog->getChangeSummary();
		else if (this->changeLog->getChangeSummary() & CMZN_ELEMENT_CHANGE_FLAG_FIELD)
		{
			if (this->event->getFeRegionChanges()->elementOrParentChanged(element))
				change |= CMZN_ELEMENT_CHANGE_FLAG_FIELD;
		}
	}
	return change;
}

cmzn_meshchanges_id cmzn_meshchanges_access(
	cmzn_meshchanges_id meshchanges)
{
	if (meshchanges)
		return meshchanges->access();
	return 0;
}

int cmzn_meshchanges_destroy(cmzn_meshchanges_id *meshchanges_address)
{
	if (meshchanges_address)
		return cmzn_meshchanges::deaccess(*meshchanges_address);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_element_change_flags cmzn_meshchanges_get_element_change_flags(
	cmzn_meshchanges_id meshchanges, cmzn_element_id element)
{
	if (meshchanges)
		return meshchanges->getElementChangeFlags(element);
	return CMZN_ELEMENT_CHANGE_FLAG_NONE;
}

int cmzn_meshchanges_get_number_of_changes(
	cmzn_meshchanges_id meshchanges)
{
	if (meshchanges)
		return meshchanges->getNumberOfChanges();
	return 0;
}

cmzn_element_change_flags cmzn_meshchanges_get_summary_element_change_flags(
	cmzn_meshchanges_id meshchanges)
{
	if (meshchanges)
		return meshchanges->getSummaryElementChangeFlags();
	return CMZN_ELEMENT_CHANGE_FLAG_NONE;
}

cmzn_mesh_group_id cmzn_fieldmodule_create_mesh_group_from_name_internal(
	cmzn_fieldmodule_id fieldmodule, const char *mesh_group_name)
{
	cmzn_mesh_group_id mesh_group = 0;
	if (fieldmodule && mesh_group_name)
	{
		cmzn_field_id existing_field = cmzn_fieldmodule_find_field_by_name(fieldmodule, mesh_group_name);
		if (existing_field)
		{
			cmzn_field_destroy(&existing_field);
		}
		else
		{
			char *group_name = duplicate_string(mesh_group_name);
			char *mesh_name = strrchr(group_name, '.');
			if (mesh_name)
			{
				*mesh_name = '\0';
				++mesh_name;
				cmzn_mesh_id master_mesh = cmzn_fieldmodule_find_mesh_by_name(fieldmodule, mesh_name);
				cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(fieldmodule, group_name);
				cmzn_field_group_id group = cmzn_field_cast_group(field);
				cmzn_field_element_group_id element_group = cmzn_field_group_create_field_element_group(group, master_mesh);
				mesh_group = cmzn_field_element_group_get_mesh_group(element_group);
				cmzn_field_element_group_destroy(&element_group);
				cmzn_field_group_destroy(&group);
				cmzn_field_destroy(&field);
				cmzn_mesh_destroy(&master_mesh);
			}
			DEALLOCATE(group_name);
		}
	}
	return mesh_group;
}
