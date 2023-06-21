/**
 * FILE : elementbasis.cpp
 *
 * Implementation of elementbasis.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "computed_field/computed_field_finite_element.h"
#include "computed_field/field_module.hpp"
#include "element/elementbasis.hpp"
#include "finite_element/finite_element_region_private.h"
#include "finite_element/finite_element_shape.hpp"
#include "general/enumerator_conversion.hpp"
#include "general/mystring.h"


cmzn_elementbasis::cmzn_elementbasis(FE_region* fe_region, int mesh_dimension,
	cmzn_elementbasis_function_type function_type) :
	fe_region(fe_region->access()),
	dimension(mesh_dimension),
	function_types(new cmzn_elementbasis_function_type[mesh_dimension]),
	access_count(1)
{
	for (int i = 0; i < dimension; i++)
	{
		function_types[i] = function_type;
	}
}

cmzn_elementbasis::~cmzn_elementbasis()
{
	FE_region::deaccess(this->fe_region);
	delete[] function_types;
}

cmzn_elementbasis* cmzn_elementbasis::create(FE_region* fe_region, int mesh_dimension,
	cmzn_elementbasis_function_type function_type)
{
	if ((fe_region) && (0 < mesh_dimension) && (mesh_dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS) &&
		(function_type > CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID))
	{
		return new cmzn_elementbasis(fe_region, mesh_dimension, function_type);
	}
	display_message(ERROR_MESSAGE, "Fieldmodule createElementbasis.  Invalid argument(s)");
	return 0;
}

cmzn_elementbasis* cmzn_elementbasis::create(FE_region* fe_region, FE_basis* basis)
{
	if ((fe_region) && (basis))
	{
		int dimension;
		FE_basis_get_dimension(basis, &dimension);
		FE_basis_type feBasisType;
		FE_basis_get_xi_basis_type(basis, /*xi*/0, &feBasisType);
		cmzn_elementbasis_function_type functionType = FE_basis_type_to_cmzn_elementbasis_function_type(feBasisType);
		if (functionType == CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID)
		{
			display_message(ERROR_MESSAGE, "cmzn_elementbasis::create.  Internal basis function type not implemented externally");
			return 0;
		}
		cmzn_elementbasis* elementbasis = new cmzn_elementbasis(fe_region, dimension, functionType);
		if (elementbasis)
		{
			for (int d = 1; d < dimension; ++d)
			{
				FE_basis_type feBasisType2;
				FE_basis_get_xi_basis_type(basis, d, &feBasisType2);
				if (feBasisType2 != feBasisType)
				{
					functionType = FE_basis_type_to_cmzn_elementbasis_function_type(feBasisType2);
					if (functionType == CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID)
					{
						display_message(ERROR_MESSAGE, "cmzn_elementbasis::create.  Internal basis function type not implemented externally");
						cmzn_elementbasis::deaccess(elementbasis);
						return 0;
					}
					elementbasis->setFunctionType(d + 1, functionType);
				}
			}
			return elementbasis;
		}
	}
	return 0;
}

int cmzn_elementbasis::getDimensionsUsingFunction(cmzn_elementbasis_function_type function_type) const
{
	int count = 0;
	for (int i = 0; i < dimension; i++)
	{
		if (function_types[i] == function_type)
			count++;
	}
	return count;
}

int cmzn_elementbasis::isValid() const
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

FE_basis* cmzn_elementbasis::getFeBasis() const
{
	if (!this->isValid())
	{
		return nullptr;
	}
	const int length = dimension * (dimension + 1) / 2 + 1;
	int* int_basis_type_array;
	if (!ALLOCATE(int_basis_type_array, int, length))
		return 0;
	*int_basis_type_array = dimension;
	int* temp = int_basis_type_array + 1;
	for (int i = 0; i < dimension; i++)
	{
		FE_basis_type fe_basis_type = cmzn_elementbasis_function_type_to_FE_basis_type(function_types[i]);
		*temp = (int)fe_basis_type;
		++temp;
		for (int j = i + 1; j < dimension; j++)
		{
			if (((fe_basis_type == CUBIC_HERMITE_SERENDIPITY) ||
				(fe_basis_type == LINEAR_SIMPLEX) ||
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
	struct FE_basis* fe_basis = FE_region_get_FE_basis_matching_basis_type(
		fe_region, int_basis_type_array);
	DEALLOCATE(int_basis_type_array);
	return fe_basis;
}

enum cmzn_elementbasis_function_type cmzn_elementbasis::getFunctionType(int chart_component) const
{
	if (chart_component == -1) // homogeneous case
	{
		for (int d = 1; d < this->dimension; ++d)
		{
			if (function_types[d] != function_types[d - 1])
				return CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID;
		}
		return function_types[0];
	}
	if ((chart_component < 1) || (chart_component > this->dimension))
		return CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID;
	return function_types[chart_component - 1];
}

int cmzn_elementbasis::setFunctionType(int chart_component, cmzn_elementbasis_function_type function_type)
{
	if (FE_BASIS_TYPE_INVALID == cmzn_elementbasis_function_type_to_FE_basis_type(function_type))
		return CMZN_ERROR_ARGUMENT;
	if (chart_component == -1) // homogeneous case
	{
		for (int d = 0; d < this->dimension; ++d)
			function_types[d] = function_type;
	}
	else
	{
		if ((chart_component < 1) || (chart_component > dimension))
			return CMZN_ERROR_ARGUMENT;
		function_types[chart_component - 1] = function_type;
	}
	return CMZN_OK;
}

int cmzn_elementbasis::getNumberOfNodes() const
{
	FE_basis* basis = this->getFeBasis();
	int numberOfNodes = FE_basis_get_number_of_nodes(basis);
	return numberOfNodes;
}

int cmzn_elementbasis::getNumberOfFunctions() const
{
	FE_basis* basis = this->getFeBasis();
	int numberOfFunctions = FE_basis_get_number_of_functions(basis);
	return numberOfFunctions;
}

int cmzn_elementbasis::getNumberOfFunctionsPerNode(int basisNodeIndex) const
{
	FE_basis* basis = this->getFeBasis();
	int numberOfFunctionsPerNode = FE_basis_get_number_of_functions_per_node(basis, basisNodeIndex - 1);
	return numberOfFunctionsPerNode;
}

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
		FE_region *fe_region = region->get_FE_region();
		if (fe_region)
		{
			return cmzn_elementbasis::create(fe_region, dimension, function_type);
		}
	}
	return nullptr;
}

cmzn_elementbasis_id cmzn_elementbasis_access(
	cmzn_elementbasis_id element_basis)
{
	if (element_basis)
	{
		return element_basis->access();
	}
	return nullptr;
}

int cmzn_elementbasis_destroy(cmzn_elementbasis_id* element_basis_address)
{
	if (element_basis_address)
	{
		cmzn_elementbasis::deaccess(*element_basis_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

FE_basis* cmzn_elementbasis_get_FE_basis(cmzn_elementbasis_id elementbasis)
{
	if (elementbasis)
	{
		return elementbasis->getFeBasis();
	}
	return nullptr;
}

int cmzn_elementbasis_get_dimension(cmzn_elementbasis_id element_basis)
{
	if (element_basis)
	{
		return element_basis->getDimension();
	}
	return 0;
}

enum cmzn_elementbasis_function_type cmzn_elementbasis_get_function_type(
	cmzn_elementbasis_id element_basis, int chart_component)
{
	if (element_basis)
	{
		return element_basis->getFunctionType(chart_component);
	}
	return CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID;
}

int cmzn_elementbasis_set_function_type(cmzn_elementbasis_id element_basis,
	int chart_component, enum cmzn_elementbasis_function_type function_type)
{
	if (element_basis)
	{
		return element_basis->setFunctionType(chart_component, function_type);
	}
	return 0;
}

int cmzn_elementbasis_get_number_of_nodes(
	cmzn_elementbasis_id element_basis)
{
	if (element_basis)
	{
		return element_basis->getNumberOfNodes();
	}
	return 0;
}

int cmzn_elementbasis_get_number_of_functions(
	cmzn_elementbasis_id element_basis)
{
	if (element_basis)
	{
		return element_basis->getNumberOfFunctions();
	}
	return 0;
}

int cmzn_elementbasis_get_number_of_functions_per_node(
	cmzn_elementbasis_id element_basis, int basis_node_index)
{
	if (element_basis)
	{
		return element_basis->getNumberOfFunctionsPerNode(basis_node_index);
	}
	return 0;
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
			case CMZN_ELEMENTBASIS_FUNCTION_TYPE_CUBIC_HERMITE_SERENDIPITY:
				enum_string = "CUBIC_HERMITE_SERENDIPITY";
				break;
			case CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_HERMITE_LAGRANGE:
				enum_string = "QUADRATIC_HERMITE_LAGRANGE";
				break;
			case CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_LAGRANGE_HERMITE:
				enum_string = "QUADRATIC_LAGRANGE_HERMITE";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum cmzn_elementbasis_function_type cmzn_elementbasis_function_type_enum_from_string(
	const char *name)
{
	return string_to_enum<enum cmzn_elementbasis_function_type,	cmzn_elementbasis_function_type_conversion>(name);
}

char *cmzn_elementbasis_function_type_enum_to_string(enum cmzn_elementbasis_function_type type)
{
	const char *type_string = cmzn_elementbasis_function_type_conversion::to_string(type);
	return (type_string ? duplicate_string(type_string) : 0);
}
