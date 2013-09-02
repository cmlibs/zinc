/***************************************************************************//**
 * FILE : scene.hpp
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
#ifndef CMZN_SCENE_HPP__
#define CMZN_SCENE_HPP__

#include "zinc/scene.h"
#include "zinc/graphic.hpp"
#include "zinc/graphicsfilter.hpp"
#include "zinc/fieldgroup.hpp"
#include "zinc/selection.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class ScenePicker;

class Scene
{

protected:
	cmzn_scene_id id;

public:

	Scene() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Scene(cmzn_scene_id scene_id) : id(scene_id)
	{  }

	Scene(const Scene& scene) : id(cmzn_scene_access(scene.id))
	{  }

	Scene& operator=(const Scene& scene)
	{
		cmzn_scene_id temp_id = cmzn_scene_access(scene.id);
		if (0 != id)
		{
			cmzn_scene_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Scene()
	{
		if (0 != id)
		{
			cmzn_scene_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	// needed for casting constructors: see SceneImage(Scene&)
	cmzn_scene_id getId()
	{
		return id;
	}

	int beginChange()
	{
		return cmzn_scene_begin_change(id);
	}

	int endChange()
	{
		return cmzn_scene_end_change(id);
	}

	int convertToPointCloud(GraphicsFilter& filter, Nodeset& nodeset,
		Field& coordinateField, double lineDensity, double lineDensityScaleFactor,
		double surfaceDensity, double surfaceDensityScaleFactor)
	{
		return cmzn_scene_convert_to_point_cloud(id, filter.getId(),
			nodeset.getId(), coordinateField.getId(),
			lineDensity, lineDensityScaleFactor,
			surfaceDensity, surfaceDensityScaleFactor);
	}

	Graphic createGraphic(Graphic::Type graphicType)
	{
		return Graphic(cmzn_scene_create_graphic(id,
			static_cast<cmzn_graphic_type>(graphicType)));
	}

	GraphicContours createGraphicContours()
	{
		return GraphicContours(cmzn_scene_create_graphic_contours(id));
	}

	GraphicLines createGraphicLines()
	{
		return GraphicLines(cmzn_scene_create_graphic_lines(id));
	}

	GraphicPoints createGraphicPoints()
	{
		return GraphicPoints(cmzn_scene_create_graphic_points(id));
	}

	GraphicStreamlines createGraphicStreamlines()
	{
		return GraphicStreamlines(cmzn_scene_create_graphic_streamlines(id));
	}

	GraphicSurfaces createGraphicSurfaces()
	{
		return GraphicSurfaces(cmzn_scene_create_graphic_surfaces(id));
	}

	SelectionHandler createSelectionHandler()
	{
		return SelectionHandler(cmzn_scene_create_selection_handler(id));
	}

	/**
	 * Returns the graphic of the specified name from the scene. Beware that
	 * graphics in the same scene may have the same name and this function will
	 * only return the first graphic found with the specified name;
	 *
	 * @param scene  Scene in which to find the graphic.
	 * @param graphic_name  The name of the graphic to find.
	 * @return  New reference to graphic of specified name, or 0 if not found.
	 */
	Graphic findGraphicByName(const char *graphicName)
	{
		return Graphic(cmzn_scene_find_graphic_by_name(id, graphicName));
	}

	Graphic getFirstGraphic()
	{
		return Graphic(cmzn_scene_get_first_graphic(id));
	}

	Graphic getNextGraphic(Graphic& refGraphic)
	{
		return Graphic(cmzn_scene_get_next_graphic(id, refGraphic.getId()));
	}

	Graphic getPreviousGraphic(Graphic& refGraphic)
	{
		return Graphic(cmzn_scene_get_previous_graphic(id, refGraphic.getId()));
	}

	int getNumberOfGraphics()
	{
		return cmzn_scene_get_number_of_graphics(id);
	}

	FieldGroup getSelectionGroup()
	{
		return FieldGroup(cmzn_scene_get_selection_group(id));
	}

	int setSelectionGroup(FieldGroup& fieldGroup)
	{
		return cmzn_scene_set_selection_group(id,
			(cmzn_field_group_id)(fieldGroup.getId()));
	}

	int getSpectrumDataRange(GraphicsFilter& filter, Spectrum& spectrum,
		int valuesCount, double *minimumValuesOut, double *maximumValuesOut)
	{
		return cmzn_scene_get_spectrum_data_range(id, filter.getId(),
			spectrum.getId(), valuesCount, minimumValuesOut, maximumValuesOut);
	}

	bool getVisibilityFlag()
	{
		return cmzn_scene_get_visibility_flag(id);
	}

	int setVisibilityFlag(bool visibilityFlag)
	{
		return cmzn_scene_set_visibility_flag(id, visibilityFlag);
	}

	int moveGraphicBefore(Graphic& graphic, Graphic& refGraphic)
	{
		return cmzn_scene_move_graphic_before(id, graphic.getId(), refGraphic.getId());
	}

	int removeAllGraphics()
	{
		return cmzn_scene_remove_all_graphics(id);
	}

	int removeGraphic(Graphic& graphic)
	{
		return cmzn_scene_remove_graphic(id, graphic.getId());
	}

	ScenePicker createPicker();

};

} // namespace Zinc
}

#endif
