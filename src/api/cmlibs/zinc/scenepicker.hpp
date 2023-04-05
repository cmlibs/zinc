/**
 * @file scenepicker.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_SCENEPICKER_HPP__
#define CMZN_SCENEPICKER_HPP__

#include "cmlibs/zinc/scenepicker.h"
#include "cmlibs/zinc/element.hpp"
#include "cmlibs/zinc/fieldgroup.hpp"
#include "cmlibs/zinc/graphics.hpp"
#include "cmlibs/zinc/node.hpp"
#include "cmlibs/zinc/scene.hpp"
#include "cmlibs/zinc/scenefilter.hpp"
#include "cmlibs/zinc/sceneviewer.hpp"

namespace CMLibs
{
namespace Zinc
{

class Scenepicker
{
protected:
	cmzn_scenepicker_id id;

public:

	Scenepicker() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Scenepicker(cmzn_scenepicker_id in_scenepicker_id) :
		id(in_scenepicker_id)
	{  }

	Scenepicker(const Scenepicker& scenepicker) :
		id(cmzn_scenepicker_access(scenepicker.id))
	{  }

	Scenepicker& operator=(const Scenepicker& scenepicker)
	{
		cmzn_scenepicker_id temp_id = cmzn_scenepicker_access(scenepicker.id);
		if (0 != id)
		{
			cmzn_scenepicker_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Scenepicker()
	{
		if (0 != id)
		{
			cmzn_scenepicker_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_scenepicker_id getId() const
	{
		return id;
	}

	int setSceneviewerRectangle(const Sceneviewer& sceneviewer, Scenecoordinatesystem scenecoordinatesystem, double x1,
		double y1, double x2, double y2)
	{
		return cmzn_scenepicker_set_sceneviewer_rectangle(
			id , sceneviewer.getId(),
			static_cast<cmzn_scenecoordinatesystem>(scenecoordinatesystem),
			x1, y1, x2, y2);
	}

	Element getNearestElement() const
	{
		return Element(cmzn_scenepicker_get_nearest_element(id));
	}

	Node getNearestNode() const
	{
		return Node(cmzn_scenepicker_get_nearest_node(id));
	}

	Graphics getNearestElementGraphics() const
	{
		return Graphics(cmzn_scenepicker_get_nearest_element_graphics(id));
	}

	Graphics getNearestNodeGraphics() const
	{
		return Graphics(cmzn_scenepicker_get_nearest_node_graphics(id));
	}

	Graphics getNearestGraphics() const
	{
		return Graphics(cmzn_scenepicker_get_nearest_graphics(id));
	}

	int addPickedElementsToFieldGroup(FieldGroup& fieldGroup) const
	{
		return cmzn_scenepicker_add_picked_elements_to_field_group(id,
			(reinterpret_cast<cmzn_field_group_id>(fieldGroup.getId())));
	}

	int addPickedNodesToFieldGroup(FieldGroup& fieldGroup) const
	{
		return cmzn_scenepicker_add_picked_nodes_to_field_group(id,
			(reinterpret_cast<cmzn_field_group_id>(fieldGroup.getId())));
	}

	Scene getScene() const
	{
		return Scene(cmzn_scenepicker_get_scene(id));
	}

	int setScene(const Scene& scene)
	{
		return cmzn_scenepicker_set_scene(id, scene.getId());
	}

	Scenefilter getScenefilter() const
	{
		return Scenefilter(cmzn_scenepicker_get_scenefilter(id));
	}

	int setScenefilter(const Scenefilter& filter)
	{
		return cmzn_scenepicker_set_scenefilter(id, filter.getId());
	}

	int getPickingVolumeCentre(double *coordinateValuesOut3) const
	{
		return cmzn_scenepicker_get_picking_volume_centre(id, coordinateValuesOut3);
	}

};

inline Scenepicker Scene::createScenepicker()
{
	return Scenepicker(cmzn_scene_create_scenepicker(id));
}

}  // namespace Zinc
}

#endif
