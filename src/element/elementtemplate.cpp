/**
 * FILE : elementtemplate.cpp
 *
 * Implementation of elementtemplate.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "computed_field/computed_field_finite_element.h"
#include "element/elementtemplate.hpp"
#include "finite_element/finite_element_shape.hpp"


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

cmzn_elementtemplate::cmzn_elementtemplate(FE_mesh *feMeshIn) :
	fe_element_template(feMeshIn->create_FE_element_template()),
	access_count(1)
{
}

cmzn_elementtemplate::~cmzn_elementtemplate()
{
	cmzn::Deaccess(this->fe_element_template);
}

int cmzn_elementtemplate::setElementShapeType(cmzn_element_shape_type shapeTypeIn)
{
	FE_element_shape *elementShape = 0;
	if (CMZN_ELEMENT_SHAPE_TYPE_INVALID != shapeTypeIn)
	{
		const int shapeDimension = cmzn_element_shape_type_get_dimension(shapeTypeIn);
		if (shapeDimension != this->getFeMesh()->getDimension())
		{
			display_message(ERROR_MESSAGE,
				"Elementtemplate  setElementShapeType.  Shape dimension is different from template mesh");
			return CMZN_ERROR_ARGUMENT;
		}
		elementShape = FE_element_shape_create_simple_type(this->getFeMesh()->get_FE_region(), shapeTypeIn);
		if (!elementShape)
		{
			display_message(ERROR_MESSAGE,
				"Elementtemplate  setElementShapeType.  Failed to create element shape");
			return CMZN_ERROR_GENERAL;
		}
	}
	int return_code = this->fe_element_template->setElementShape(elementShape);
	if (elementShape)
	{
		DEACCESS(FE_element_shape)(&elementShape);
	}
	return return_code;
}

int cmzn_elementtemplate::defineField(FE_field *field, int componentNumber, cmzn_elementfieldtemplate *eft)
{
	if (!((field) && ((-1 == componentNumber) || ((0 < componentNumber) && (componentNumber <= get_FE_field_number_of_components(field))))
		&& (eft)))
	{
		display_message(ERROR_MESSAGE, "Elementtemplate defineField.  Invalid arguments");
		return CMZN_ERROR_ARGUMENT;
	}
    int return_code = this->fe_element_template->defineField(field, componentNumber - 1, eft);
	if (CMZN_OK != return_code)
		display_message(ERROR_MESSAGE, "Elementtemplate defineField.  Failed");
	return return_code;
}

int cmzn_elementtemplate::defineField(cmzn_field_id field, int componentNumber, cmzn_elementfieldtemplate *eft)
{
	FE_field *fe_field = 0;
	Computed_field_get_type_finite_element(field, &fe_field);
	if (!fe_field)
	{
		display_message(ERROR_MESSAGE,
			"Elementtemplate defineField.  Can only define a finite element type field on elements");
		return CMZN_ERROR_ARGUMENT;
	}
	return this->defineField(fe_field, componentNumber, eft);
}

int cmzn_elementtemplate::removeField(cmzn_field_id field)
{
	if (!field)
		return CMZN_ERROR_ARGUMENT;
	FE_field *fe_field = 0;
	Computed_field_get_type_finite_element(field, &fe_field);
	if (!fe_field)
	{
		display_message(ERROR_MESSAGE, "Elementtemplate removeField.  Not a finite element field");
		return CMZN_ERROR_ARGUMENT;
	}
	int return_code = this->fe_element_template->removeField(fe_field);
	return return_code;
}

int cmzn_elementtemplate::undefineField(cmzn_field_id field)
{
	if (!field)
		return CMZN_ERROR_ARGUMENT;
	FE_field *fe_field = 0;
	Computed_field_get_type_finite_element(field, &fe_field);
	if (!fe_field)
	{
		display_message(ERROR_MESSAGE, "Elementtemplate undefineField.  Not a finite element field");
		return CMZN_ERROR_ARGUMENT;
	}
	int return_code = this->fe_element_template->undefineField(fe_field);
	return return_code;
}

cmzn_element *cmzn_elementtemplate::createElement(int identifier)
{
	if (!this->validate())
	{
		display_message(ERROR_MESSAGE, "Mesh createElement.  Element template is not valid");
		return 0;
	}
	if (!this->fe_element_template->getElementShape())
	{
		display_message(ERROR_MESSAGE, "Mesh createElement.  Element template does not have a shape set");
		return 0;
	}
	cmzn_element *element = this->getFeMesh()->create_FE_element(identifier, this->fe_element_template);
	return element;
}

int cmzn_elementtemplate::mergeIntoElement(cmzn_element *element)
{
	if (this->validate())
	{
		int return_code = this->getFeMesh()->merge_FE_element_template(element, this->fe_element_template);
		return return_code;
	}
	display_message(ERROR_MESSAGE, "Element merge.  Element template is not valid");
	return CMZN_ERROR_ARGUMENT;
}

/*
Global functions
----------------
*/

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
	{
		cmzn_elementtemplate::deaccess(*element_template_address);
		return CMZN_OK;
	}
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
		return element_template->setElementShapeType(shape_type);
	return 0;
}

int cmzn_elementtemplate_define_field(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field,
	int component_number, cmzn_elementfieldtemplate_id eft)
{
	if (elementtemplate)
		return elementtemplate->defineField(field, component_number, eft);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_elementtemplate_remove_field(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field)
{
	if (elementtemplate)
		return elementtemplate->removeField(field);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_elementtemplate_undefine_field(
	cmzn_elementtemplate_id elementtemplate, cmzn_field_id field)
{
	if (elementtemplate)
		return elementtemplate->undefineField(field);
	return CMZN_ERROR_ARGUMENT;
}
