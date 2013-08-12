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
#ifndef __SCENE_VIEWER_HPP__
#define __SCENE_VIEWER_HPP__

#include "zinc/sceneviewer.h"
#include "zinc/sceneviewerinput.h"
#include "zinc/scene.hpp"
#include "zinc/graphicsfilter.hpp"

namespace zinc
{

class SceneViewerInput
{
protected:
	Cmiss_scene_viewer_input_id id;

public:
	enum InputEventType
	{
		INPUT_EVENT_TYPE_INVALID = CMISS_SCENE_VIEWER_INPUT_INVALID,
		INPUT_EVENT_TYPE_MOTION_NOTIFY = CMISS_SCENE_VIEWER_INPUT_MOTION_NOTIFY,
		INPUT_EVENT_TYPE_BUTTON_PRESS = CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS,
		INPUT_EVENT_TYPE_BUTTON_RELEASE = CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE,
		INPUT_EVENT_TYPE_KEY_PRESS = CMISS_SCENE_VIEWER_INPUT_KEY_PRESS,
		INPUT_EVENT_TYPE_KEY_RELEASE = CMISS_SCENE_VIEWER_INPUT_KEY_RELEASE
	};

	enum InputButtonType
	{
		INPUT_BUTTON_INVALID = CMISS_SCENE_VIEWER_INPUT_BUTTON_INVALID,
		INPUT_BUTTON_LEFT = CMISS_SCENE_VIEWER_INPUT_BUTTON_LEFT,
		INPUT_BUTTON_MIDDLE = CMISS_SCENE_VIEWER_INPUT_BUTTON_MIDDLE,
		INPUT_BUTTON_RIGHT = CMISS_SCENE_VIEWER_INPUT_BUTTON_RIGHT,
		INPUT_BUTTON_SCROLL_DOWN = CMISS_SCENE_VIEWER_INPUT_BUTTON_SCROLL_DOWN,
		INPUT_BUTTON_SCROLL_UP = CMISS_SCENE_VIEWER_INPUT_BUTTON_SCROLL_UP
	};

	SceneViewerInput() : id(0)
	{  }

	// takes ownership of C-style region reference
	explicit SceneViewerInput(Cmiss_scene_viewer_input_id in_scene_viewer_input_id) :
		id(in_scene_viewer_input_id)
	{  }

	SceneViewerInput(const SceneViewerInput& scene_viewer_input) :
		id(Cmiss_scene_viewer_input_access(scene_viewer_input.id))
	{  }

	SceneViewerInput& operator=(const SceneViewerInput& scene_viewer_input)
	{
		Cmiss_scene_viewer_input_id temp_id = Cmiss_scene_viewer_input_access(scene_viewer_input.id);
		if (0 != id)
		{
			Cmiss_scene_viewer_input_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SceneViewerInput()
	{
		if (0 != id)
		{
			Cmiss_scene_viewer_input_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_scene_viewer_input_id getId()
	{
		return id;
	}

	int setPosition(int x, int y)
	{
		return Cmiss_scene_viewer_input_set_position(id, x, y);
	}

	int setButtonNumber(int number)
	{
		return Cmiss_scene_viewer_input_set_button_number(id, number);
	}

	int setButton(InputButtonType button)
	{
		return Cmiss_scene_viewer_input_set_button(id, static_cast<Cmiss_scene_viewer_input_button_type>(button));
	}

	int setType(InputEventType type)
	{
		return Cmiss_scene_viewer_input_set_type(id, static_cast<Cmiss_scene_viewer_input_event_type>(type));
	}

};

class SceneViewer
{
protected:
	Cmiss_scene_viewer_id id;

public:

	enum BufferingMode
	{
		BUFFERING_MODE_ANY = CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE,
		BUFFERING_MODE_SINGLE = CMISS_SCENE_VIEWER_BUFFERING_SINGLE,
		BUFFERING_MODE_DOUBLE = CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
		BUFFERING_MODE_RENDER_OFFSCREEN_AND_COPY = CMISS_SCENE_VIEWER_BUFFERING_RENDER_OFFSCREEN_AND_COPY,
		BUFFERING_MODE_RENDER_OFFSCREEN_AND_BLEND = CMISS_SCENE_VIEWER_BUFFERING_RENDER_OFFSCREEN_AND_BLEND
	};

	enum StereoMode
	{
		STEREO_MODE_ANY = CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
		STEREO_MODE_MONO = CMISS_SCENE_VIEWER_STEREO_MONO,
		STEREO_MODE_STEREO = CMISS_SCENE_VIEWER_STEREO_STEREO
	};

	enum TransparencyMode
	{
		TRANSPARENCY_INVALID = CMISS_SCENE_VIEWER_TRANSPARENCY_INVALID,
		TRANSPARENCY_FAST = CMISS_SCENE_VIEWER_TRANSPARENCY_FAST,
		TRANSPARENCY_SLOW = CMISS_SCENE_VIEWER_TRANSPARENCY_SLOW,
		TRANSPARENCY_ORDER_INDEPENDENT = CMISS_SCENE_VIEWER_TRANSPARENCY_ORDER_INDEPENDENT
	};

	SceneViewer() : id(0)
	{  }

	// takes ownership of C-style region reference
	explicit SceneViewer(Cmiss_scene_viewer_id in_scene_viewer_id) :
		id(in_scene_viewer_id)
	{  }

	SceneViewer(const SceneViewer& scene_viewer_module) :
		id(Cmiss_scene_viewer_access(scene_viewer_module.id))
	{  }

	SceneViewer& operator=(const SceneViewer& scene_viewer)
	{
		Cmiss_scene_viewer_id temp_id = Cmiss_scene_viewer_access(scene_viewer.id);
		if (0 != id)
		{
			Cmiss_scene_viewer_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SceneViewer()
	{
		if (0 != id)
		{
			Cmiss_scene_viewer_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_scene_viewer_id getId()
	{
		return id;
	}

	int renderScene()
	{
		return Cmiss_scene_viewer_render_scene(id);
	}

	int setScene(Scene scene)
	{
		return Cmiss_scene_viewer_set_scene(id, scene.getId());
	}

	Scene getScene()
	{
		return Scene(Cmiss_scene_viewer_get_scene(id));
	}

	int setFilter(GraphicsFilter graphicsFilter)
	{
		return Cmiss_scene_viewer_set_filter(id, graphicsFilter.getId());
	}

	GraphicsFilter getFilter()
	{
		return GraphicsFilter(Cmiss_scene_viewer_get_filter(id));
	}

	int setGraphicsBufferWidth(int width)
	{
		return Cmiss_scene_viewer_set_graphics_buffer_width(id, width);
	}

	int setGraphicsBufferHeight(int height)
	{
		return Cmiss_scene_viewer_set_graphics_buffer_height(id, height);
	}

	int setViewportSize(int width, int height)
	{
		return Cmiss_scene_viewer_set_viewport_size(id, width, height);
	}

	SceneViewerInput getInput()
	{
		return SceneViewerInput(Cmiss_scene_viewer_get_input(id));
	}

	int setInput(SceneViewerInput input)
	{
		return Cmiss_scene_viewer_input_viewport_transform(id, input.getId());
	}

	int getLookatParameters(double *eyex,double *eyey,double *eyez,
		double *lookatx,double *lookaty,double *lookatz,
		double *upx,double *upy,double *upz)
	{
		return Cmiss_scene_viewer_get_lookat_parameters(id, eyex, eyey, eyez,
			lookatx, lookaty, lookatz,
			upx, upy, upz);
	}

	int getViewingVolume(double *left,double *right,double *bottom,double *top,
		double *near_plane, double *far_plane)
	{
		return Cmiss_scene_viewer_get_viewing_volume(id, left, right, bottom, top,
			near_plane, far_plane);
	}

	int setBackgroundColourComponentRGB(double red, double green, double blue)
	{
		return Cmiss_scene_viewer_set_background_colour_component_rgb(id, red, green, blue);
	}

	int setBackgroundColourRGB(const double *valuesIn3)
	{
		return Cmiss_scene_viewer_set_background_colour_rgb(id, valuesIn3);
	}

	int getBackgroundColourRGB(double *valuesOut3)
	{
		return Cmiss_scene_viewer_get_background_colour_rgb(id, valuesOut3);
	}

	int viewAll()
	{
		return Cmiss_scene_viewer_view_all(id);
	}

	enum TransparencyMode getTransparencyMode()
	{
		return static_cast<TransparencyMode>(Cmiss_scene_viewer_get_transparency_mode(id));
	}

	int setTransparencyMode(TransparencyMode transparencyMode)
	{
		return Cmiss_scene_viewer_set_transparency_mode(id,
			static_cast<Cmiss_scene_viewer_transparency_mode>(transparencyMode));
	}

	int getTransparencyLayers()
	{
		return Cmiss_scene_viewer_get_transparency_layers(id);
	}

	int setTransparencyLayers(int layers)
	{
		return Cmiss_scene_viewer_set_transparency_layers(id, layers);
	}

};

class SceneViewerModule
{
protected:
	Cmiss_scene_viewer_module_id id;

public:

	SceneViewerModule() : id(0)
	{  }

	// takes ownership of C-style region reference
	SceneViewerModule(Cmiss_scene_viewer_module_id in_scene_viewer_module_id) :
		id(in_scene_viewer_module_id)
	{  }

	SceneViewerModule(const SceneViewerModule& scene_viewer_module) :
		id(Cmiss_scene_viewer_module_access(scene_viewer_module.id))
	{  }

	SceneViewerModule& operator=(const SceneViewerModule& scene_viewer_module)
	{
		Cmiss_scene_viewer_module_id temp_id = Cmiss_scene_viewer_module_access(scene_viewer_module.id);
		if (0 != id)
		{
			Cmiss_scene_viewer_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SceneViewerModule()
	{
		if (0 != id)
		{
			Cmiss_scene_viewer_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_scene_viewer_module_id getId()
	{
		return id;
	}

	SceneViewer createSceneViewer(SceneViewer::BufferingMode buffering_mode, SceneViewer::StereoMode stereo_mode)
	{
		return SceneViewer(Cmiss_scene_viewer_module_create_scene_viewer(id,
			static_cast<Cmiss_scene_viewer_buffering_mode>(buffering_mode),
			static_cast<Cmiss_scene_viewer_stereo_mode>(stereo_mode)));
	}

};

}  // namespace zinc

#endif
