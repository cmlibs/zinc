/**
 * FILE : mesh.cpp
 *
 * Mesh implementation.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "general/mystring.h"
#include "computed_field/computed_field.h"
#include "computed_field/differential_operator.hpp"
#include "computed_field/field_cache.hpp"
#include "computed_field/field_module.hpp"
#include "element/elementtemplate.hpp"
#include "finite_element/finite_element_region_private.h"
#include "mesh/mesh.hpp"
#include "region/cmiss_region.hpp"


void cmzn_mesh::deaccess(cmzn_mesh*& mesh)
{
	if (mesh)
	{
		--(mesh->access_count);
		if (mesh->access_count <= 0)
		{
			delete mesh;
		}
		mesh = nullptr;
	}
}

cmzn_element* cmzn_mesh::createElement(int identifier, cmzn_elementtemplate* elementtemplate)
{
	if (!elementtemplate)
	{
		return nullptr;
	}
	return elementtemplate->createElement(identifier);
}

cmzn_elementtemplate* cmzn_mesh::createElementtemplate() const
{
	return cmzn_elementtemplate::create(this->feMesh);
}

int cmzn_mesh::destroyElementsConditional(cmzn_field* conditional_field)
{
	if (!conditional_field)
	{
		return CMZN_ERROR_ARGUMENT;
	}
	DsLabelsGroup* tmpLabelsGroup = this->feMesh->createLabelsGroup();
	if (!tmpLabelsGroup)
	{
		return CMZN_ERROR_GENERAL;
	}
	cmzn_region* region = FE_region_get_cmzn_region(this->feMesh->get_FE_region()); // not accessed
	cmzn_fieldcache* fieldcache = cmzn_fieldcache::create(region);
	cmzn_elementiterator* iterator = this->createElementiterator();
	cmzn_element* element = nullptr;
	while ((element = cmzn_elementiterator_next_non_access(iterator)))
	{
		fieldcache->setElement(element);
		if (cmzn_field_evaluate_boolean(conditional_field, fieldcache))
		{
			tmpLabelsGroup->setIndex(element->getIndex(), true);
		}
	}
	cmzn::Deaccess(iterator);
	cmzn_fieldcache::deaccess(fieldcache);
	int return_code = this->feMesh->destroyElementsInGroup(*tmpLabelsGroup);
	cmzn::Deaccess(tmpLabelsGroup);
	return return_code;
}

char* cmzn_mesh::getName() const
{
	return duplicate_string(this->feMesh->getName());
}

cmzn_mesh* cmzn_mesh::getMasterMesh() const
{
	cmzn_region* region = FE_region_get_cmzn_region(this->feMesh->get_FE_region()); // not accessed
	return region->findMeshByDimension(this->feMesh->getDimension());
}

cmzn_region* cmzn_mesh::getRegion() const
{
	// gracefully handle FE_mesh being orphaned at cleanup time
	FE_region* feRegion = this->feMesh->get_FE_region();
	if (feRegion)
	{
		return feRegion->getRegion();
	}
	return nullptr;
}

bool cmzn_mesh::hasMembershipChanges() const
{
	return this->feMesh->hasMembershipChanges();
}

/*
Global functions
----------------
*/

cmzn_mesh_id cmzn_fieldmodule_find_mesh_by_dimension(
	cmzn_fieldmodule_id fieldmodule, int dimension)
{
	if (fieldmodule)
	{
		cmzn_mesh* mesh = cmzn_fieldmodule_get_region_internal(fieldmodule)->findMeshByDimension(dimension);
		if (mesh)
		{
			return mesh->access();
		}
	}
	return nullptr;
}

cmzn_mesh_id cmzn_fieldmodule_find_mesh_by_name(
	cmzn_fieldmodule_id fieldmodule, const char* mesh_name)
{
	if ((fieldmodule) && (mesh_name))
	{
		cmzn_mesh* mesh = cmzn_fieldmodule_get_region_internal(fieldmodule)->findMeshByName(mesh_name);
		if (mesh)
		{
			return mesh->access();
		}
	}
	return nullptr;
}

cmzn_mesh_id cmzn_mesh_access(cmzn_mesh_id mesh)
{
	if (mesh)
		return mesh->access();
	return 0;
}

int cmzn_mesh_destroy(cmzn_mesh_id* mesh_address)
{
	if (mesh_address)
	{
		cmzn_mesh::deaccess(*mesh_address);
		return CMZN_OK;
	}
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
	if (mesh)
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
	{
		cmzn_element* element = mesh->findElementByIdentifier(identifier);
		if (element)
		{
			return element->access();
		}
	}
	return nullptr;
}

cmzn_differentialoperator_id cmzn_mesh_get_chart_differentialoperator(
	cmzn_mesh_id mesh, int order, int term)
{
	if ((mesh) && ((-1 == order) || ((1 <= order) && (order <= MAXIMUM_MESH_DERIVATIVE_ORDER))))
	{
		FieldDerivative* fieldDerivative = mesh->getFeMesh()->getFieldDerivative(order);
		if (fieldDerivative)
		{
			return cmzn_differentialoperator::create(fieldDerivative, term - 1);
		}
	}
	display_message(ERROR_MESSAGE, "Mesh getChartDifferentialoperator.  Invalid argument(s)");
	return nullptr;
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
		cmzn_region* region = FE_region_get_cmzn_region(mesh->getFeMesh()->get_FE_region());
		return cmzn_fieldmodule_create(region);
	}
	return 0;
}

cmzn_mesh_id cmzn_mesh_get_master_mesh(cmzn_mesh_id mesh)
{
	if (mesh)
	{
		return mesh->getMasterMesh()->access();
	}
	return nullptr;
}

char* cmzn_mesh_get_name(cmzn_mesh_id mesh)
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


