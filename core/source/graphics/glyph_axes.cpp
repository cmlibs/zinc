/**
 * glyph_axes.cpp
 *
 * Derived glyph type which creates 2 or 3 dimensional axes with optional
 * per axis labels and materials.
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
 * Portions created by the Initial Developer are Copyright (C) 2013
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

#include "zinc/status.h"
#include "general/debug.h"
#include "general/message.h"
#include "general/mystring.h"
#include "graphics/glyph_axes.hpp"

cmzn_glyph_axes::cmzn_glyph_axes(cmzn_glyph *axisGlyphIn, double axisWidthIn) :
	axisGlyph(axisGlyphIn->access()),
	axisWidth(axisWidthIn),
	graphicsObject(0)
{
	for (int i = 0; i < 3; ++i)
	{
		axisLabels[i] = 0;
		axisMaterials[i] = 0;
	}
}

cmzn_glyph_axes::~cmzn_glyph_axes()
{
	if (graphicsObject)
	{
		DEACCESS(GT_object)(&graphicsObject);
	}
	cmzn_glyph_destroy(&axisGlyph);
	for (int i = 0; i < 3; ++i)
	{
		if (axisLabels[i])
		{
			DEALLOCATE(axisLabels[i]);
		}
		cmzn_graphics_material_destroy(&axisMaterials[i]);
	}
}

void cmzn_glyph_axes::invalidate()
{
	if (this->graphicsObject)
	{
		DEACCESS(GT_object)(&this->graphicsObject);
	}
	this->changed();
}

namespace {
/**
 * @param primaryAxis  0, 1 or 2.
 */
GT_object *createAxisGraphicsObject(int primaryAxis, GT_object *axisObject,
	double axisWidth, const char *name, cmzn_graphics_material *material, cmzn_font *font,
	int axisLabelsCount, char **axisLabels, enum cmzn_glyph_repeat_mode repeat_mode)
{
	Triple *point_list, *axis1_list, *axis2_list, *axis3_list, *scale_list;
	ALLOCATE(point_list, Triple, 1);
	(*point_list)[0] =  (*point_list)[1] =  (*point_list)[2] = 0.0;

	ALLOCATE(axis1_list, Triple, 1);
	(*axis1_list)[0] = (*axis1_list)[1] = (*axis1_list)[2] = 0.0;
	(*axis1_list)[primaryAxis] = 1.0;

	ALLOCATE(axis2_list, Triple, 1);
	(*axis2_list)[0] = (*axis2_list)[1] = (*axis2_list)[2] = 0.0;
	(*axis2_list)[(primaryAxis + 1) % 3] = 1.0;

	ALLOCATE(axis3_list, Triple, 1);
	(*axis3_list)[0] = (*axis3_list)[1] = (*axis3_list)[2] = 0.0;
	(*axis3_list)[(primaryAxis + 2) % 3] = 1.0;

	ALLOCATE(scale_list, Triple, 1);
	(*scale_list)[0] = 0.0;
	(*scale_list)[1] = 0.0;
	(*scale_list)[2] = 0.0;

	char *staticLabels[3] = { 0, 0, 0 };
	for (int i = 0; i < axisLabelsCount; ++i)
	{
		staticLabels[i] = axisLabels[i];
	}
	Triple glyph_base_size = { 1.0f, axisWidth, axisWidth };
	Triple glyph_scale_factors = { 0.0f, 0.0f, 0.0f };
	Triple glyph_offset = { 0.0f, 0.0f, 0.0f };
	Triple glyph_label_offset = { 1.1f, 0.0f, 0.0f };
	GT_glyph_set *glyph_set = CREATE(GT_glyph_set)(1,
		point_list, axis1_list, axis2_list, axis3_list, scale_list,
		axisObject, repeat_mode,
		glyph_base_size, glyph_scale_factors, glyph_offset,
		font, (char **)0, glyph_label_offset, staticLabels, g_NO_DATA, 0,
		/*label_bounds_dimension*/0, /*label_bounds_components*/0, /*label_bounds*/(ZnReal *)NULL,
		/*label_density_list*/(Triple *)NULL, /*object_name*/-1, /*names*/(int *)NULL);

	GT_object *graphicsObject = CREATE(GT_object)(name, g_GLYPH_SET, material);
	GT_OBJECT_ADD(GT_glyph_set)(graphicsObject, /*time*/0.0, glyph_set);
	return graphicsObject;
}

} // namespace

GT_object *cmzn_glyph_axes::getGraphicsObject(cmzn_tessellation *tessellation,
	cmzn_graphics_material *material, cmzn_font *font)
{
	USE_PARAMETER(tessellation);
	GT_object *axis_gt_object = this->axisGlyph->getGraphicsObject(tessellation, material, font);
	const bool usingFont = this->usesFont();
	const bool usingMaterials = (0 != this->axisMaterials[0]) || (0 != this->axisMaterials[1]) || (0 != this->axisMaterials[2]);
	if (this->graphicsObject)
	{
		GT_object *current_axis_gt_object = get_GT_object_glyph(this->graphicsObject);
		if ((axis_gt_object != current_axis_gt_object) ||
			(usingFont && (font != get_GT_object_font(this->graphicsObject))))
		{
			DEACCESS(GT_object)(&this->graphicsObject);
		}
		else if (usingMaterials)
		{
			GT_object *thisObject = this->graphicsObject;
			for (int i = 0; i < 3; ++i)
			{
				cmzn_graphics_material *useMaterial = this->axisMaterials[i] ? this->axisMaterials[i] : material;
				if (get_GT_object_default_material(thisObject) != useMaterial)
				{
					DEACCESS(GT_object)(&this->graphicsObject);
					break;
				}
				thisObject = GT_object_get_next_object(thisObject);
			}
		}
	}
	if (!this->graphicsObject)
	{
		if (usingMaterials)
		{
			GT_object *lastObject = 0;
			for (int i = 0; i < 3; ++i)
			{
				GT_object *thisObject = createAxisGraphicsObject(/*primaryAxis*/i, axis_gt_object, this->axisWidth,
					this->name, this->axisMaterials[i] ? this->axisMaterials[i] : material,
					font, 1, this->axisLabels + i, CMISS_GLYPH_REPEAT_NONE);
				if (lastObject)
				{
					GT_object_set_next_object(lastObject, thisObject);
					lastObject = thisObject;
					DEACCESS(GT_object)(&thisObject);
				}
				else
				{
					this->graphicsObject = lastObject = thisObject;
				}
			}
		}
		else
		{
			this->graphicsObject = createAxisGraphicsObject(/*primaryAxis*/0, axis_gt_object, this->axisWidth,
				this->name, static_cast<cmzn_graphics_material_id>(0),
				font, 3, this->axisLabels, CMISS_GLYPH_REPEAT_AXES_3D);
		}
	}
	DEACCESS(GT_object)(&axis_gt_object);
	return ACCESS(GT_object)(this->graphicsObject);
}

int cmzn_glyph_axes::setAxisWidth(double axisWidthIn)
{
	if (axisWidthIn >= 0.0)
	{
		if (axisWidthIn != this->axisWidth)
		{
			this->axisWidth = axisWidthIn;
			this->invalidate();
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

char *cmzn_glyph_axes::getAxisLabel(int axisNumber)
{
	if ((0 < axisNumber) && (axisNumber <= 3))
	{
		return (this->axisLabels[axisNumber - 1]) ?
			duplicate_string(this->axisLabels[axisNumber - 1]) : 0;
	}
	return 0;
}

int cmzn_glyph_axes::setAxisLabel(int axisNumber, const char *label)
{
	if ((0 < axisNumber) && (axisNumber <= 3))
	{
		if (!labels_match(this->axisLabels[axisNumber - 1], label))
		{
			if (this->axisLabels[axisNumber - 1])
			{
				DEALLOCATE(this->axisLabels[axisNumber - 1]);
			}
			this->axisLabels[axisNumber - 1] = (label && (label[0] != '\0')) ?
				duplicate_string(label) : 0;
			this->invalidate();
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

cmzn_graphics_material *cmzn_glyph_axes::getAxisMaterial(int axisNumber)
{
	if ((0 < axisNumber) && (axisNumber <= 3) && (this->axisMaterials[axisNumber - 1]))
	{
		return cmzn_graphics_material_access(this->axisMaterials[axisNumber - 1]);
	}
	return 0;
}

int cmzn_glyph_axes::setAxisMaterial(int axisNumber, cmzn_graphics_material *material)
{
	if ((0 < axisNumber) && (axisNumber <= 3))
	{
		if (this->axisMaterials[axisNumber - 1] != material)
		{
			REACCESS(Graphical_material)(&(this->axisMaterials[axisNumber - 1]), material);
			this->invalidate();
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

void cmzn_glyph_axes::fontChange()
{
	if (this->usesFont())
	{
		this->invalidate();
	}
}

void cmzn_glyph_axes::materialChange(struct MANAGER_MESSAGE(Graphical_material) *message)
{
	for (int i = 0; i < 3; ++i)
	{
		if (this->axisMaterials[i])
		{
			int change_flags = MANAGER_MESSAGE_GET_OBJECT_CHANGE(Graphical_material)(message, this->axisMaterials[i]);
			if (change_flags & MANAGER_CHANGE_RESULT(Graphical_material))
			{
				this->invalidate();
				return;
			}
		}
	}
}

cmzn_glyph_axes_id cmzn_glyph_module_create_axes(
	cmzn_glyph_module_id glyph_module, cmzn_glyph_id axis_glyph,
	double axis_width)
{
	if (glyph_module)
	{
		cmzn_glyph_axes_id axes = cmzn_glyph_axes::create(axis_glyph, axis_width);
		if (axes)
		{
			glyph_module->addGlyph(axes);
			return axes;
		}
	}
	return 0;
}

cmzn_glyph_axes_id cmzn_glyph_cast_axes(cmzn_glyph_id glyph)
{
	if (glyph && (dynamic_cast<cmzn_glyph_axes*>(glyph)))
	{
		glyph->access();
		return (reinterpret_cast<cmzn_glyph_axes_id>(glyph));
	}
	return 0;
}

int cmzn_glyph_axes_destroy(cmzn_glyph_axes_id *axes_address)
{
	return cmzn_glyph_destroy(reinterpret_cast<cmzn_glyph_id *>(axes_address));
}

double cmzn_glyph_axes_get_axis_width(cmzn_glyph_axes_id axes)
{
	if (axes)
		return axes->getAxisWidth();
	return 0.0;
}

int cmzn_glyph_axes_set_axis_width(cmzn_glyph_axes_id axes,
	double axis_width)
{
	if (axes)
		return axes->setAxisWidth(axis_width);
	return CMISS_ERROR_ARGUMENT;
}

char *cmzn_glyph_axes_get_axis_label(cmzn_glyph_axes_id axes, int axis_number)
{
	if (axes)
		return axes->getAxisLabel(axis_number);
	return 0;
}

int cmzn_glyph_axes_set_axis_label(cmzn_glyph_axes_id axes,
	int axis_number, const char *label)
{
	if (axes)
		return axes->setAxisLabel(axis_number, label);
	return CMISS_ERROR_ARGUMENT;
}

cmzn_graphics_material_id cmzn_glyph_axes_get_axis_material(
	cmzn_glyph_axes_id axes, int axis_number)
{
	if (axes)
		return axes->getAxisMaterial(axis_number);
	return 0;
}

int cmzn_glyph_axes_set_axis_material(cmzn_glyph_axes_id axes,
	int axis_number, cmzn_graphics_material_id material)
{
	if (axes)
		return axes->setAxisMaterial(axis_number, material);
	return CMISS_ERROR_ARGUMENT;
}
