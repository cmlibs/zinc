/***************************************************************************//**
 * FILE : graphicsmaterial.hpp
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
#ifndef __ZN_CMISS_GRAPHICS_MATERIAL_HPP__
#define __ZN_CMISS_GRAPHICS_MATERIAL_HPP__

#include "zinc/graphicsmaterial.h"
#include "zinc/fieldimage.h"
#include "zinc/field.hpp"

namespace zinc
{

class GraphicsMaterial
{

protected:
	Cmiss_graphics_material_id id;

public:

	GraphicsMaterial() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsMaterial(Cmiss_graphics_material_id graphics_material_id) :
		id(graphics_material_id)
	{ }

	GraphicsMaterial(const GraphicsMaterial& graphicsMaterial) :
		id(Cmiss_graphics_material_access(graphicsMaterial.id))
	{ }

	GraphicsMaterial& operator=(const GraphicsMaterial& graphicsMaterial)
	{
		Cmiss_graphics_material_id temp_id = Cmiss_graphics_material_access(graphicsMaterial.id);
		if (0 != id)
		{
			Cmiss_graphics_material_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~GraphicsMaterial()
	{
		if (0 != id)
		{
			Cmiss_graphics_material_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_graphics_material_id getId()
	{
		return id;
	}

	enum Attribute
	{
		ATTRIBUTE_INVALID = CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_INVALID,
		ATTRIBUTE_IS_MANAGED = CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_IS_MANAGED,
		ATTRIBUTE_ALPHA = CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_ALPHA,
		ATTRIBUTE_AMBIENT = CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_AMBIENT,
		ATTRIBUTE_DIFFUSE = CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_DIFFUSE,
		ATTRIBUTE_EMISSION = CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_EMISSION,
		ATTRIBUTE_SHININESS = CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_SHININESS,
		ATTRIBUTE_SPECULAR = CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_SPECULAR
	};

	int getAttributeInteger(Attribute attribute)
	{
		return Cmiss_graphics_material_get_attribute_integer(id,
			static_cast<Cmiss_graphics_material_attribute>(attribute));
	}

	int setAttributeInteger(Attribute attribute, int value)
	{
		return Cmiss_graphics_material_set_attribute_integer(id,
			static_cast<Cmiss_graphics_material_attribute>(attribute), value);
	}

	int getAttributeReal(Attribute attribute)
	{
		return Cmiss_graphics_material_get_attribute_real(id,
			static_cast<Cmiss_graphics_material_attribute>(attribute));
	}

	int setAttributeReal(Attribute attribute, double value)
	{
		return Cmiss_graphics_material_set_attribute_real(id,
			static_cast<Cmiss_graphics_material_attribute>(attribute), value);
	}

	int getAttributeReal3(Attribute attribute, double *outValues)
	{
		return Cmiss_graphics_material_get_attribute_real3(id,
			static_cast<Cmiss_graphics_material_attribute>(attribute), outValues);
	}

	int setAttributeReal3(Attribute attribute, const double *values)
	{
		return Cmiss_graphics_material_set_attribute_real3(id,
			static_cast<Cmiss_graphics_material_attribute>(attribute), values);
	}

	char *getName()
	{
		return Cmiss_graphics_material_get_name(id);
	}

	int setName(const char *name)
	{
		return Cmiss_graphics_material_set_name(id, name);
	}

	int setImageField(int imageNumber, Field &imageField)
	{
		Cmiss_field_image_id field_image = Cmiss_field_cast_image(imageField.getId());
		int result = Cmiss_graphics_material_set_image_field(id, imageNumber, field_image);
		Cmiss_field_image_destroy(&field_image);

		return result;
	}

};

} // namespace Cmiss

#endif /* __ZN_CMISS_GRAPHICS_MATERIAL_HPP__ */
