/***************************************************************************//**
 * FILE : scene.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
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
