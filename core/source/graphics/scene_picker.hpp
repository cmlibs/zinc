/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (SCENE_PICKER_HPP)
#define SCENE_PICKER_HPP

#include <map>
#include "zinc/scenepicker.h"
#include "zinc/types/graphicsid.h"
#include "zinc/types/scenefilterid.h"
#include "zinc/types/fieldgroupid.h"
#include "zinc/types/elementid.h"
#include "zinc/types/nodeid.h"
#include "zinc/types/sceneid.h"
#include "zinc/types/scenecoordinatesystem.h"
#include "zinc/types/scenepickerid.h"
#include "zinc/types/sceneviewerid.h"


typedef std::multimap<cmzn_region *, cmzn_element_id> Region_element_map;
typedef std::multimap<cmzn_region *, cmzn_node_id> Region_node_map;

enum cmzn_scenepicker_object_type
{
	CMZN_SCENEPICKER_OBJECT_ANY = 0,
	CMZN_SCENEPICKER_OBJECT_NODE = 1,
	CMZN_SCENEPICKER_OBJECT_ELEMENT = 2,
	CMZN_SCENEPICKER_OBJECT_DATA = 3
};

struct cmzn_scenepicker
{
private:
	struct Interaction_volume *interaction_volume;
	cmzn_scene_id top_scene;
	cmzn_sceneviewer_id scene_viewer;
	int centre_x, centre_y, size_x, size_y;
	enum cmzn_scene_coordinate_system coordinate_system;
	cmzn_scenefilter_id filter;
	GLuint *select_buffer;
	int select_buffer_size;
	int number_of_hits;
	cmzn_scenefiltermodule_id filter_module;

	void updateViewerRectangle();

	int pickObjects();

	Region_node_map getPickedRegionSortedNodes(enum cmzn_scenepicker_object_type type);

	Region_element_map getPickedRegionSortedElements();

	void reset();

	/*provide a select buffer pointer and return the scene and graphics */
	int getSceneAndGraphics(GLuint *select_buffer_ptr,
		cmzn_scene_id *scene, cmzn_graphics_id *graphics);

public:

	int access_count;

	cmzn_scenepicker(cmzn_scenefiltermodule_id filter_module_in);

	~cmzn_scenepicker();

	cmzn_scenefilter_id getScenefilter();

	int setScenefilter(cmzn_scenefilter_id filter_in);

	cmzn_scene_id getScene();

	int setScene(cmzn_scene_id scene_in);

	int setSceneviewerRectangle(cmzn_sceneviewer_id scene_viewer_in,
		enum cmzn_scene_coordinate_system coordinate_system_in, double x1,
		double y1, double x2, double y2);

	int setInteractionVolume(struct Interaction_volume *interaction_volume_in);

	cmzn_element_id getNearestElement();

	cmzn_node_id getNearestNode(enum cmzn_scenepicker_object_type type);

	cmzn_graphics_id getNearestGraphics(enum cmzn_scenepicker_object_type type);

	inline cmzn_scenepicker *access()
	{
		++access_count;
		return this;
	}

	int addPickedElementsToGroup(cmzn_field_group_id group);

	int addPickedNodesToGroup(cmzn_field_group_id group,
		enum cmzn_scenepicker_object_type type);

};

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_scenepicker);

cmzn_scenepicker_id cmzn_scenepicker_create(cmzn_scenefiltermodule_id filter_module);

int cmzn_scenepicker_set_interaction_volume(cmzn_scenepicker_id scenepicker,
	struct Interaction_volume *interaction_volume);

int cmzn_scenepicker_add_picked_data_to_group(cmzn_scenepicker_id scenepicker,
	cmzn_field_group_id group);

cmzn_graphics_id cmzn_scenepicker_get_nearest_data_graphics(cmzn_scenepicker_id scenepicker);

cmzn_node_id cmzn_scenepicker_get_nearest_data(cmzn_scenepicker_id scenepicker);

#endif /* (SCENE_PICKER_HPP) */
