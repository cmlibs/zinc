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
#ifndef CMZN_GRAPHICSMATERIAL_HPP__
#define CMZN_GRAPHICSMATERIAL_HPP__

#include "zinc/graphicsmaterial.h"
#include "zinc/fieldimage.h"
#include "zinc/field.hpp"

namespace zinc
{

class GraphicsMaterial
{

protected:
	cmzn_graphics_material_id id;

public:

	GraphicsMaterial() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsMaterial(cmzn_graphics_material_id graphics_material_id) :
		id(graphics_material_id)
	{ }

	GraphicsMaterial(const GraphicsMaterial& graphicsMaterial) :
		id(cmzn_graphics_material_access(graphicsMaterial.id))
	{ }

	GraphicsMaterial& operator=(const GraphicsMaterial& graphicsMaterial)
	{
		cmzn_graphics_material_id temp_id = cmzn_graphics_material_access(graphicsMaterial.id);
		if (0 != id)
		{
			cmzn_graphics_material_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~GraphicsMaterial()
	{
		if (0 != id)
		{
			cmzn_graphics_material_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_graphics_material_id getId()
	{
		return id;
	}

	enum Attribute
	{
		ATTRIBUTE_INVALID = CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_INVALID,
		ATTRIBUTE_ALPHA = CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_ALPHA,
		ATTRIBUTE_AMBIENT = CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_AMBIENT,
		ATTRIBUTE_DIFFUSE = CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_DIFFUSE,
		ATTRIBUTE_EMISSION = CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_EMISSION,
		ATTRIBUTE_SHININESS = CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_SHININESS,
		ATTRIBUTE_SPECULAR = CMZN_GRAPHICS_MATERIAL_ATTRIBUTE_SPECULAR
	};

	bool isManaged()
	{
		return cmzn_graphics_material_is_managed(id);
	}

	int setManaged(bool value)
	{
		return cmzn_graphics_material_set_managed(id, value);
	}

	double getAttributeReal(Attribute attribute)
	{
		return cmzn_graphics_material_get_attribute_real(id,
			static_cast<cmzn_graphics_material_attribute>(attribute));
	}

	int setAttributeReal(Attribute attribute, double value)
	{
		return cmzn_graphics_material_set_attribute_real(id,
			static_cast<cmzn_graphics_material_attribute>(attribute), value);
	}

	int getAttributeReal3(Attribute attribute, double *valuesOut3)
	{
		return cmzn_graphics_material_get_attribute_real3(id,
			static_cast<cmzn_graphics_material_attribute>(attribute), valuesOut3);
	}

	int setAttributeReal3(Attribute attribute, const double *valuesIn3)
	{
		return cmzn_graphics_material_set_attribute_real3(id,
			static_cast<cmzn_graphics_material_attribute>(attribute), valuesIn3);
	}

	char *getName()
	{
		return cmzn_graphics_material_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_graphics_material_set_name(id, name);
	}

	int setImageField(int imageNumber, Field &imageField)
	{
		cmzn_field_image_id field_image = cmzn_field_cast_image(imageField.getId());
		int result = cmzn_graphics_material_set_image_field(id, imageNumber, field_image);
		cmzn_field_image_destroy(&field_image);

		return result;
	}

};

class GraphicsMaterialModule
{
protected:
	cmzn_graphics_material_module_id id;

public:

	GraphicsMaterialModule() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsMaterialModule(cmzn_graphics_material_module_id in_graphics_material_module_id) :
		id(in_graphics_material_module_id)
	{  }

	GraphicsMaterialModule(const GraphicsMaterialModule& graphicsMaterialModule) :
		id(cmzn_graphics_material_module_access(graphicsMaterialModule.id))
	{  }

	GraphicsMaterialModule& operator=(const GraphicsMaterialModule& graphicsMaterialModule)
	{
		cmzn_graphics_material_module_id temp_id = cmzn_graphics_material_module_access(
			graphicsMaterialModule.id);
		if (0 != id)
		{
			cmzn_graphics_material_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~GraphicsMaterialModule()
	{
		if (0 != id)
		{
			cmzn_graphics_material_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_graphics_material_module_id getId()
	{
		return id;
	}

	GraphicsMaterial createMaterial()
	{
		return GraphicsMaterial(cmzn_graphics_material_module_create_material(id));
	}

	GraphicsMaterial findMaterialByName(const char *name)
	{
		return GraphicsMaterial(cmzn_graphics_material_module_find_material_by_name(id, name));
	}

	int beginChange()
	{
		return cmzn_graphics_material_module_begin_change(id);
	}

	int endChange()
	{
		return cmzn_graphics_material_module_end_change(id);
	}

	int defineStandardMaterials()
	{
		return cmzn_graphics_material_module_define_standard_materials(id);
	}

	GraphicsMaterial getDefaultMaterial()
	{
		return GraphicsMaterial(cmzn_graphics_material_module_get_default_material(id));
	}

	int setDefaultMaterial(GraphicsMaterial &graphicsMaterial)
	{
		return cmzn_graphics_material_module_set_default_material(id, graphicsMaterial.getId());
	}

	GraphicsMaterial getDefaultSelectedMaterial()
	{
		return GraphicsMaterial(cmzn_graphics_material_module_get_default_selected_material(id));
	}

	int setDefaultSelectedMaterial(GraphicsMaterial &graphicsMaterial)
	{
		return cmzn_graphics_material_module_set_default_selected_material(id, graphicsMaterial.getId());
	}
};

} // namespace cmzn

#endif
