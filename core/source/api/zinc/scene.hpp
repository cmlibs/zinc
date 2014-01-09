/**
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
#include "zinc/field.hpp"
#include "zinc/graphics.hpp"
#include "zinc/scenefilter.hpp"
#include "zinc/selection.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class Sceneviewermodule;
class Scenepicker;

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

	int convertToPointCloud(Scenefilter& filter, Nodeset& nodeset,
		Field& coordinateField, double lineDensity, double lineDensityScaleFactor,
		double surfaceDensity, double surfaceDensityScaleFactor)
	{
		return cmzn_scene_convert_to_point_cloud(id, filter.getId(),
			nodeset.getId(), coordinateField.getId(),
			lineDensity, lineDensityScaleFactor,
			surfaceDensity, surfaceDensityScaleFactor);
	}

	Graphics createGraphics(Graphics::Type graphicsType)
	{
		return Graphics(cmzn_scene_create_graphics(id,
			static_cast<cmzn_graphics_type>(graphicsType)));
	}

	GraphicsContours createGraphicsContours()
	{
		return GraphicsContours(cmzn_scene_create_graphics_contours(id));
	}

	GraphicsLines createGraphicsLines()
	{
		return GraphicsLines(cmzn_scene_create_graphics_lines(id));
	}

	GraphicsPoints createGraphicsPoints()
	{
		return GraphicsPoints(cmzn_scene_create_graphics_points(id));
	}

	GraphicsStreamlines createGraphicsStreamlines()
	{
		return GraphicsStreamlines(cmzn_scene_create_graphics_streamlines(id));
	}

	GraphicsSurfaces createGraphicsSurfaces()
	{
		return GraphicsSurfaces(cmzn_scene_create_graphics_surfaces(id));
	}

	Selectionnotifier createSelectionnotifier()
	{
		return Selectionnotifier(cmzn_scene_create_selectionnotifier(id));
	}

	/**
	 * Returns the graphics of the specified name from the scene. Beware that
	 * graphics in the same scene may have the same name and this function will
	 * only return the first graphics found with the specified name;
	 *
	 * @param scene  Scene in which to find the graphics.
	 * @param name  The name of the graphics to find.
	 * @return  New reference to graphics of specified name, or 0 if not found.
	 */
	Graphics findGraphicsByName(const char *name)
	{
		return Graphics(cmzn_scene_find_graphics_by_name(id, name));
	}

	Graphics getFirstGraphics()
	{
		return Graphics(cmzn_scene_get_first_graphics(id));
	}

	Graphics getNextGraphics(Graphics& refGraphics)
	{
		return Graphics(cmzn_scene_get_next_graphics(id, refGraphics.getId()));
	}

	Graphics getPreviousGraphics(Graphics& refGraphics)
	{
		return Graphics(cmzn_scene_get_previous_graphics(id, refGraphics.getId()));
	}

	int getNumberOfGraphics()
	{
		return cmzn_scene_get_number_of_graphics(id);
	}

	Scenefiltermodule getScenefiltermodule()
	{
		return Scenefiltermodule(cmzn_scene_get_scenefiltermodule(id));
	}

	Field getSelectionField()
	{
		return Field(cmzn_scene_get_selection_field(id));
	}

	int setSelectionField(Field& selectionField)
	{
		return cmzn_scene_set_selection_field(id, selectionField.getId());
	}

	int getSpectrumDataRange(Scenefilter& filter, Spectrum& spectrum,
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

	int moveGraphicsBefore(Graphics& graphics, Graphics& refGraphics)
	{
		return cmzn_scene_move_graphics_before(id, graphics.getId(), refGraphics.getId());
	}

	int removeAllGraphics()
	{
		return cmzn_scene_remove_all_graphics(id);
	}

	int removeGraphics(Graphics& graphics)
	{
		return cmzn_scene_remove_graphics(id, graphics.getId());
	}

	Scenepicker createScenepicker();

};

inline Scene Region::getScene()
{
	return Scene(cmzn_region_get_scene(id));
}

} // namespace Zinc
}

#endif
