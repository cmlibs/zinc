/**
 * @file elementtemplate.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_ELEMENTTEMPLATE_HPP__
#define CMZN_ELEMENTTEMPLATE_HPP__

#include "cmlibs/zinc/elementtemplate.h"
#include "cmlibs/zinc/element.hpp"
#include "cmlibs/zinc/elementfieldtemplate.hpp"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/node.hpp"

namespace CMLibs
{
namespace Zinc
{

class Elementtemplate
{
private:

	cmzn_elementtemplate_id id;

public:

	Elementtemplate() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Elementtemplate(cmzn_elementtemplate_id element_template_id) :
		id(element_template_id)
	{ }

	Elementtemplate(const Elementtemplate& elementTemplate) :
		id(cmzn_elementtemplate_access(elementTemplate.id))
	{ }

	Elementtemplate& operator=(const Elementtemplate& elementTemplate)
	{
		cmzn_elementtemplate_id temp_id = cmzn_elementtemplate_access(elementTemplate.id);
		if (0 != id)
		{
			cmzn_elementtemplate_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Elementtemplate()
	{
		if (0 != id)
		{
			cmzn_elementtemplate_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_elementtemplate_id getId() const
	{
		return id;
	}

	Element::ShapeType getElementShapeType() const
	{
		return static_cast<Element::ShapeType>(cmzn_elementtemplate_get_element_shape_type(id));
	}

	int setElementShapeType(Element::ShapeType shapeType)
	{
		return cmzn_elementtemplate_set_element_shape_type(id,
			static_cast<cmzn_element_shape_type>(shapeType));
	}

	int defineField(const Field& field, int componentNumber, const Elementfieldtemplate& eft)
	{
		return cmzn_elementtemplate_define_field(this->id, field.getId(),
			componentNumber, eft.getId());
	}

	int removeField(const Field& field)
	{
		return cmzn_elementtemplate_remove_field(this->id, field.getId());
	}

	int undefineField(const Field& field)
	{
		return cmzn_elementtemplate_undefine_field(this->id, field.getId());
	}
};

inline int Element::merge(const Elementtemplate& elementTemplate)
{
	return cmzn_element_merge(id, elementTemplate.getId());
}

}  // namespace Zinc
}

#endif /* CMZN_ELEMENTTEMPLATE_HPP__ */
