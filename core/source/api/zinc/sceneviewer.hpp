/***************************************************************************//**
 * FILE : sceneviewer.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_SCENEVIEWER_HPP__
#define CMZN_SCENEVIEWER_HPP__

#include "zinc/sceneviewer.h"
#include "zinc/sceneviewerinput.h"
#include "zinc/scene.hpp"
#include "zinc/graphicsfilter.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class SceneViewerInput
{
protected:
	cmzn_scene_viewer_input_id id;

public:
	enum InputEventType
	{
		INPUT_EVENT_TYPE_INVALID = CMZN_SCENE_VIEWER_INPUT_INVALID,
		INPUT_EVENT_TYPE_MOTION_NOTIFY = CMZN_SCENE_VIEWER_INPUT_MOTION_NOTIFY,
		INPUT_EVENT_TYPE_BUTTON_PRESS = CMZN_SCENE_VIEWER_INPUT_BUTTON_PRESS,
		INPUT_EVENT_TYPE_BUTTON_RELEASE = CMZN_SCENE_VIEWER_INPUT_BUTTON_RELEASE,
		INPUT_EVENT_TYPE_KEY_PRESS = CMZN_SCENE_VIEWER_INPUT_KEY_PRESS,
		INPUT_EVENT_TYPE_KEY_RELEASE = CMZN_SCENE_VIEWER_INPUT_KEY_RELEASE
	};

	enum InputButtonType
	{
		INPUT_BUTTON_INVALID = CMZN_SCENE_VIEWER_INPUT_BUTTON_INVALID,
		INPUT_BUTTON_LEFT = CMZN_SCENE_VIEWER_INPUT_BUTTON_LEFT,
		INPUT_BUTTON_MIDDLE = CMZN_SCENE_VIEWER_INPUT_BUTTON_MIDDLE,
		INPUT_BUTTON_RIGHT = CMZN_SCENE_VIEWER_INPUT_BUTTON_RIGHT,
		INPUT_BUTTON_SCROLL_DOWN = CMZN_SCENE_VIEWER_INPUT_BUTTON_SCROLL_DOWN,
		INPUT_BUTTON_SCROLL_UP = CMZN_SCENE_VIEWER_INPUT_BUTTON_SCROLL_UP
	};

	enum InputModifierFlags
	{
		INPUT_MODIFIER_NONE = CMZN_SCENE_VIEWER_INPUT_MODIFIER_NONE,
		INPUT_MODIFIER_SHIFT = CMZN_SCENE_VIEWER_INPUT_MODIFIER_SHIFT,
		INPUT_MODIFIER_CONTROL = CMZN_SCENE_VIEWER_INPUT_MODIFIER_CONTROL,
		INPUT_MODIFIER_ALT = CMZN_SCENE_VIEWER_INPUT_MODIFIER_ALT,
		INPUT_MODIFIER_BUTTON1 = CMZN_SCENE_VIEWER_INPUT_MODIFIER_BUTTON1
	};

	typedef int InputModifier;

	SceneViewerInput() : id(0)
	{  }

	// takes ownership of C-style region reference
	explicit SceneViewerInput(cmzn_scene_viewer_input_id in_scene_viewer_input_id) :
		id(in_scene_viewer_input_id)
	{  }

	SceneViewerInput(const SceneViewerInput& scene_viewer_input) :
		id(cmzn_scene_viewer_input_access(scene_viewer_input.id))
	{  }

	SceneViewerInput& operator=(const SceneViewerInput& scene_viewer_input)
	{
		cmzn_scene_viewer_input_id temp_id = cmzn_scene_viewer_input_access(scene_viewer_input.id);
		if (0 != id)
		{
			cmzn_scene_viewer_input_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SceneViewerInput()
	{
		if (0 != id)
		{
			cmzn_scene_viewer_input_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_scene_viewer_input_id getId()
	{
		return id;
	}

	int setPosition(int x, int y)
	{
		return cmzn_scene_viewer_input_set_position(id, x, y);
	}

	int setButtonNumber(int number)
	{
		return cmzn_scene_viewer_input_set_button_number(id, number);
	}

	int setButton(InputButtonType button)
	{
		return cmzn_scene_viewer_input_set_button(id, static_cast<cmzn_scene_viewer_input_button_type>(button));
	}

	int setType(InputEventType type)
	{
		return cmzn_scene_viewer_input_set_type(id, static_cast<cmzn_scene_viewer_input_event_type>(type));
	}

	int setModifier(InputModifier modifier)
	{
		return cmzn_scene_viewer_input_set_modifier(id, static_cast<cmzn_scene_viewer_input_modifier>(modifier));
	}

};

class SceneViewer
{
protected:
	cmzn_scene_viewer_id id;

public:

	enum BufferingMode
	{
		BUFFERING_MODE_ANY = CMZN_SCENE_VIEWER_BUFFERING_ANY_MODE,
		BUFFERING_MODE_SINGLE = CMZN_SCENE_VIEWER_BUFFERING_SINGLE,
		BUFFERING_MODE_DOUBLE = CMZN_SCENE_VIEWER_BUFFERING_DOUBLE,
		BUFFERING_MODE_RENDER_OFFSCREEN_AND_COPY = CMZN_SCENE_VIEWER_BUFFERING_RENDER_OFFSCREEN_AND_COPY,
		BUFFERING_MODE_RENDER_OFFSCREEN_AND_BLEND = CMZN_SCENE_VIEWER_BUFFERING_RENDER_OFFSCREEN_AND_BLEND
	};

	enum StereoMode
	{
		STEREO_MODE_ANY = CMZN_SCENE_VIEWER_STEREO_ANY_MODE,
		STEREO_MODE_MONO = CMZN_SCENE_VIEWER_STEREO_MONO,
		STEREO_MODE_STEREO = CMZN_SCENE_VIEWER_STEREO_STEREO
	};

	enum TransparencyMode
	{
		TRANSPARENCY_INVALID = CMZN_SCENE_VIEWER_TRANSPARENCY_INVALID,
		TRANSPARENCY_FAST = CMZN_SCENE_VIEWER_TRANSPARENCY_FAST,
		TRANSPARENCY_SLOW = CMZN_SCENE_VIEWER_TRANSPARENCY_SLOW,
		TRANSPARENCY_ORDER_INDEPENDENT = CMZN_SCENE_VIEWER_TRANSPARENCY_ORDER_INDEPENDENT
	};

	SceneViewer() : id(0)
	{  }

	// takes ownership of C-style region reference
	explicit SceneViewer(cmzn_scene_viewer_id in_scene_viewer_id) :
		id(in_scene_viewer_id)
	{  }

	SceneViewer(const SceneViewer& scene_viewer_module) :
		id(cmzn_scene_viewer_access(scene_viewer_module.id))
	{  }

	SceneViewer& operator=(const SceneViewer& scene_viewer)
	{
		cmzn_scene_viewer_id temp_id = cmzn_scene_viewer_access(scene_viewer.id);
		if (0 != id)
		{
			cmzn_scene_viewer_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SceneViewer()
	{
		if (0 != id)
		{
			cmzn_scene_viewer_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_scene_viewer_id getId()
	{
		return id;
	}

	int renderScene()
	{
		return cmzn_scene_viewer_render_scene(id);
	}

	int setScene(Scene scene)
	{
		return cmzn_scene_viewer_set_scene(id, scene.getId());
	}

	Scene getScene()
	{
		return Scene(cmzn_scene_viewer_get_scene(id));
	}

	int setFilter(GraphicsFilter graphicsFilter)
	{
		return cmzn_scene_viewer_set_filter(id, graphicsFilter.getId());
	}

	GraphicsFilter getFilter()
	{
		return GraphicsFilter(cmzn_scene_viewer_get_filter(id));
	}

	int setGraphicsBufferWidth(int width)
	{
		return cmzn_scene_viewer_set_graphics_buffer_width(id, width);
	}

	int setGraphicsBufferHeight(int height)
	{
		return cmzn_scene_viewer_set_graphics_buffer_height(id, height);
	}

	int setViewportSize(int width, int height)
	{
		return cmzn_scene_viewer_set_viewport_size(id, width, height);
	}

	SceneViewerInput getInput()
	{
		return SceneViewerInput(cmzn_scene_viewer_get_input(id));
	}

	int processInput(SceneViewerInput input)
	{
		return cmzn_scene_viewer_process_input(id, input.getId());
	}

	int getEyePosition(double *eyeValuesOut3)
	{
		return cmzn_scene_viewer_get_eye_position(id, eyeValuesOut3);
	}

	int setEyePosition(double const *eyeValuesIn3)
	{
		return cmzn_scene_viewer_set_eye_position(id, eyeValuesIn3);
	}

	int getLookatPosition(double *lookatValuesOut3)
	{
		return cmzn_scene_viewer_get_lookat_position(id, lookatValuesOut3);
	}

	int setLookatPosition(double const *lookatValuesIn3)
	{
		return cmzn_scene_viewer_set_lookat_position(id, lookatValuesIn3);
	}

	int getUpVector(double *upVectorValuesOut3)
	{
		return cmzn_scene_viewer_get_up_vector(id, upVectorValuesOut3);
	}

	int setUpVector(double const *upVectorValuesIn3)
	{
		return cmzn_scene_viewer_set_up_vector(id, upVectorValuesIn3);
	}

	int getLookatParameters(double *eyeValuesOut3,double *lookatValuesOut3,double *upVectorValuesOut3)
	{
		return cmzn_scene_viewer_get_lookat_parameters(id, &eyeValuesOut3[0], &eyeValuesOut3[1], &eyeValuesOut3[2],
			&lookatValuesOut3[0], &lookatValuesOut3[1], &lookatValuesOut3[2],
			&upVectorValuesOut3[0], &upVectorValuesOut3[1], &upVectorValuesOut3[2]);
	}

	int setLookatParametersNonSkew(double const *eyeValuesIn3, double const *lookatValuesIn3, double const *upVectorValuesIn3)
	{
		return cmzn_scene_viewer_set_lookat_parameters_non_skew(id, eyeValuesIn3[0], eyeValuesIn3[1], eyeValuesIn3[2],
			lookatValuesIn3[0], lookatValuesIn3[1], lookatValuesIn3[2],
			upVectorValuesIn3[0], upVectorValuesIn3[1], upVectorValuesIn3[2]);
	}

	int getViewingVolume(double *left,double *right,double *bottom,double *top,
		double *near_plane, double *far_plane)
	{
		return cmzn_scene_viewer_get_viewing_volume(id, left, right, bottom, top,
			near_plane, far_plane);
	}

	int setViewingVolume(double left, double right, double bottom, double top,
		double near_plane, double far_plane)
	{
		return cmzn_scene_viewer_set_viewing_volume(id, left, right, bottom, top,
			near_plane, far_plane);
	}

	int setBackgroundColourComponentRGB(double red, double green, double blue)
	{
		return cmzn_scene_viewer_set_background_colour_component_rgb(id, red, green, blue);
	}

	int setBackgroundColourRGB(const double *valuesIn3)
	{
		return cmzn_scene_viewer_set_background_colour_rgb(id, valuesIn3);
	}

	int getBackgroundColourRGB(double *valuesOut3)
	{
		return cmzn_scene_viewer_get_background_colour_rgb(id, valuesOut3);
	}

	int viewAll()
	{
		return cmzn_scene_viewer_view_all(id);
	}

	enum TransparencyMode getTransparencyMode()
	{
		return static_cast<TransparencyMode>(cmzn_scene_viewer_get_transparency_mode(id));
	}

	int setTransparencyMode(TransparencyMode transparencyMode)
	{
		return cmzn_scene_viewer_set_transparency_mode(id,
			static_cast<cmzn_scene_viewer_transparency_mode>(transparencyMode));
	}

	int getTransparencyLayers()
	{
		return cmzn_scene_viewer_get_transparency_layers(id);
	}

	int setTransparencyLayers(int layers)
	{
		return cmzn_scene_viewer_set_transparency_layers(id, layers);
	}

};

class SceneViewerModule
{
protected:
	cmzn_scene_viewer_module_id id;

public:

	SceneViewerModule() : id(0)
	{  }

	// takes ownership of C-style region reference
	SceneViewerModule(cmzn_scene_viewer_module_id in_scene_viewer_module_id) :
		id(in_scene_viewer_module_id)
	{  }

	SceneViewerModule(const SceneViewerModule& scene_viewer_module) :
		id(cmzn_scene_viewer_module_access(scene_viewer_module.id))
	{  }

	SceneViewerModule& operator=(const SceneViewerModule& scene_viewer_module)
	{
		cmzn_scene_viewer_module_id temp_id = cmzn_scene_viewer_module_access(scene_viewer_module.id);
		if (0 != id)
		{
			cmzn_scene_viewer_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SceneViewerModule()
	{
		if (0 != id)
		{
			cmzn_scene_viewer_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_scene_viewer_module_id getId()
	{
		return id;
	}

	SceneViewer createSceneViewer(SceneViewer::BufferingMode buffering_mode, SceneViewer::StereoMode stereo_mode)
	{
		return SceneViewer(cmzn_scene_viewer_module_create_scene_viewer(id,
			static_cast<cmzn_scene_viewer_buffering_mode>(buffering_mode),
			static_cast<cmzn_scene_viewer_stereo_mode>(stereo_mode)));
	}

};

}  // namespace Zinc
}

#endif
