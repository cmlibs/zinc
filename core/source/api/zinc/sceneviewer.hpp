/**
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
#include "zinc/scenefilter.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class Sceneviewerinput
{
protected:
	cmzn_sceneviewerinput_id id;

public:
	enum EventType
	{
		EVENT_INVALID = CMZN_SCENEVIEWERINPUT_EVENT_INVALID,
		EVENT_MOTION_NOTIFY = CMZN_SCENEVIEWERINPUT_EVENT_MOTION_NOTIFY,
		EVENT_BUTTON_PRESS = CMZN_SCENEVIEWERINPUT_EVENT_BUTTON_PRESS,
		EVENT_BUTTON_RELEASE = CMZN_SCENEVIEWERINPUT_EVENT_BUTTON_RELEASE,
		EVENT_KEY_PRESS = CMZN_SCENEVIEWERINPUT_EVENT_KEY_PRESS,
		EVENT_KEY_RELEASE = CMZN_SCENEVIEWERINPUT_EVENT_KEY_RELEASE
	};

	enum ButtonType
	{
		BUTTON_INVALID = CMZN_SCENEVIEWERINPUT_BUTTON_INVALID,
		BUTTON_LEFT = CMZN_SCENEVIEWERINPUT_BUTTON_LEFT,
		BUTTON_MIDDLE = CMZN_SCENEVIEWERINPUT_BUTTON_MIDDLE,
		BUTTON_RIGHT = CMZN_SCENEVIEWERINPUT_BUTTON_RIGHT,
		BUTTON_SCROLL_DOWN = CMZN_SCENEVIEWERINPUT_BUTTON_SCROLL_DOWN,
		BUTTON_SCROLL_UP = CMZN_SCENEVIEWERINPUT_BUTTON_SCROLL_UP
	};

	enum ModifierFlags
	{
		MODIFIER_NONE = CMZN_SCENEVIEWERINPUT_MODIFIER_NONE,
		MODIFIER_SHIFT = CMZN_SCENEVIEWERINPUT_MODIFIER_SHIFT,
		MODIFIER_CONTROL = CMZN_SCENEVIEWERINPUT_MODIFIER_CONTROL,
		MODIFIER_ALT = CMZN_SCENEVIEWERINPUT_MODIFIER_ALT,
		MODIFIER_BUTTON1 = CMZN_SCENEVIEWERINPUT_MODIFIER_BUTTON1
	};

	typedef int ModifiersType;

	Sceneviewerinput() : id(0)
	{  }

	// takes ownership of C-style region reference
	explicit Sceneviewerinput(cmzn_sceneviewerinput_id in_sceneviewerinput_id) :
		id(in_sceneviewerinput_id)
	{  }

	Sceneviewerinput(const Sceneviewerinput& sceneviewerinput) :
		id(cmzn_sceneviewerinput_access(sceneviewerinput.id))
	{  }

	Sceneviewerinput& operator=(const Sceneviewerinput& sceneviewerinput)
	{
		cmzn_sceneviewerinput_id temp_id = cmzn_sceneviewerinput_access(sceneviewerinput.id);
		if (0 != id)
		{
			cmzn_sceneviewerinput_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Sceneviewerinput()
	{
		if (0 != id)
		{
			cmzn_sceneviewerinput_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_sceneviewerinput_id getId()
	{
		return id;
	}

	int setPosition(int x, int y)
	{
		return cmzn_sceneviewerinput_set_position(id, x, y);
	}

	int setButtonNumber(int number)
	{
		return cmzn_sceneviewerinput_set_button_number(id, number);
	}

	int setButton(ButtonType button)
	{
		return cmzn_sceneviewerinput_set_button(id, static_cast<cmzn_sceneviewerinput_button_type>(button));
	}

	int setEventType(EventType eventType)
	{
		return cmzn_sceneviewerinput_set_event_type(id, static_cast<cmzn_sceneviewerinput_event_type>(eventType));
	}

	int setModifiers(ModifiersType modifiers)
	{
		return cmzn_sceneviewerinput_set_modifiers(id, static_cast<cmzn_sceneviewerinput_modifiers_type>(modifiers));
	}

};

class Sceneviewer
{
protected:
	cmzn_sceneviewer_id id;

public:

	enum BufferingMode
	{
		BUFFERING_INVALID = CMZN_SCENEVIEWER_BUFFERING_INVALID,
		BUFFERING_ANY_MODE = CMZN_SCENEVIEWER_BUFFERING_ANY_MODE,
		BUFFERING_SINGLE = CMZN_SCENEVIEWER_BUFFERING_SINGLE,
		BUFFERING_DOUBLE = CMZN_SCENEVIEWER_BUFFERING_DOUBLE,
		BUFFERING_RENDER_OFFSCREEN_AND_COPY = CMZN_SCENEVIEWER_BUFFERING_RENDER_OFFSCREEN_AND_COPY,
		BUFFERING_RENDER_OFFSCREEN_AND_BLEND = CMZN_SCENEVIEWER_BUFFERING_RENDER_OFFSCREEN_AND_BLEND
	};

	enum ProjectionMode
	{
		PROJECTION_INVALID = CMZN_SCENEVIEWER_PROJECTION_INVALID,
		PROJECTION_PARALLEL = CMZN_SCENEVIEWER_PROJECTION_PARALLEL,
		PROJECTION_PERSPECTIVE = CMZN_SCENEVIEWER_PROJECTION_PERSPECTIVE
	};

	enum StereoMode
	{
		STEREO_INVALID = CMZN_SCENEVIEWER_STEREO_INVALID,
		STEREO_ANY_MODE = CMZN_SCENEVIEWER_STEREO_ANY_MODE,
		STEREO_MONO = CMZN_SCENEVIEWER_STEREO_MONO,
		STEREO_STEREO = CMZN_SCENEVIEWER_STEREO_STEREO
	};

	enum TransparencyMode
	{
		TRANSPARENCY_INVALID = CMZN_SCENEVIEWER_TRANSPARENCY_INVALID,
		TRANSPARENCY_FAST = CMZN_SCENEVIEWER_TRANSPARENCY_FAST,
		TRANSPARENCY_SLOW = CMZN_SCENEVIEWER_TRANSPARENCY_SLOW,
		TRANSPARENCY_ORDER_INDEPENDENT = CMZN_SCENEVIEWER_TRANSPARENCY_ORDER_INDEPENDENT
	};

	Sceneviewer() : id(0)
	{  }

	// takes ownership of C-style region reference
	explicit Sceneviewer(cmzn_sceneviewer_id in_sceneviewer_id) :
		id(in_sceneviewer_id)
	{  }

	Sceneviewer(const Sceneviewer& sceneviewermodule) :
		id(cmzn_sceneviewer_access(sceneviewermodule.id))
	{  }

	Sceneviewer& operator=(const Sceneviewer& sceneviewer)
	{
		cmzn_sceneviewer_id temp_id = cmzn_sceneviewer_access(sceneviewer.id);
		if (0 != id)
		{
			cmzn_sceneviewer_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Sceneviewer()
	{
		if (0 != id)
		{
			cmzn_sceneviewer_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_sceneviewer_id getId()
	{
		return id;
	}

	int renderScene()
	{
		return cmzn_sceneviewer_render_scene(id);
	}

	int setScene(Scene scene)
	{
		return cmzn_sceneviewer_set_scene(id, scene.getId());
	}

	Scene getScene()
	{
		return Scene(cmzn_sceneviewer_get_scene(id));
	}

	int setScenefilter(Scenefilter scenefilter)
	{
		return cmzn_sceneviewer_set_scenefilter(id, scenefilter.getId());
	}

	Scenefilter getScenefilter()
	{
		return Scenefilter(cmzn_sceneviewer_get_scenefilter(id));
	}

	int setGraphicsBufferWidth(int width)
	{
		return cmzn_sceneviewer_set_graphics_buffer_width(id, width);
	}

	int setGraphicsBufferHeight(int height)
	{
		return cmzn_sceneviewer_set_graphics_buffer_height(id, height);
	}

	int setViewportSize(int width, int height)
	{
		return cmzn_sceneviewer_set_viewport_size(id, width, height);
	}

	Sceneviewerinput createSceneviewerinput()
	{
		return Sceneviewerinput(cmzn_sceneviewer_create_sceneviewerinput(id));
	}

	int processSceneviewerinput(Sceneviewerinput input)
	{
		return cmzn_sceneviewer_process_sceneviewerinput(id, input.getId());
	}

	int getAntialiasSampling()
	{
		return cmzn_sceneviewer_get_antialias_sampling(id);
	}

	int setAntialiasSampling(int numberOfSamples)
	{
		return cmzn_sceneviewer_set_antialias_sampling(id, numberOfSamples);
	}

	int getEyePosition(double *eyeValuesOut3)
	{
		return cmzn_sceneviewer_get_eye_position(id, eyeValuesOut3);
	}

	int setEyePosition(double const *eyeValuesIn3)
	{
		return cmzn_sceneviewer_set_eye_position(id, eyeValuesIn3);
	}

	int getLookatPosition(double *lookatValuesOut3)
	{
		return cmzn_sceneviewer_get_lookat_position(id, lookatValuesOut3);
	}

	int setLookatPosition(double const *lookatValuesIn3)
	{
		return cmzn_sceneviewer_set_lookat_position(id, lookatValuesIn3);
	}

	bool getPerturbLinesFlag()
	{
		return cmzn_sceneviewer_get_perturb_lines_flag(id);
	}

	int setPerturbLinesFlag(bool value)
	{
		return cmzn_sceneviewer_set_perturb_lines_flag(id, value);
	}

	ProjectionMode getProjectionMode()
	{
		return static_cast<ProjectionMode>(cmzn_sceneviewer_get_projection_mode(id));
	}

	int setProjectionMode(ProjectionMode projectionMode)
	{
		return cmzn_sceneviewer_set_projection_mode(id,
			static_cast<cmzn_sceneviewer_projection_mode>(projectionMode));
	}

	double getTranslationRate()
	{
		return cmzn_sceneviewer_get_translation_rate(id);
	}

	int setTranslationRate(double translationRate)
	{
		return cmzn_sceneviewer_set_translation_rate(id, translationRate);
	}

	double getTumbleRate()
	{
		return cmzn_sceneviewer_get_tumble_rate(id);
	}

	int setTumbleRate(double tumbleRate)
	{
		return cmzn_sceneviewer_set_tumble_rate(id, tumbleRate);
	}

	double getZoomRate()
	{
		return cmzn_sceneviewer_get_zoom_rate(id);
	}

	int setZoomRate(double zoomRate)
	{
		return cmzn_sceneviewer_set_zoom_rate(id, zoomRate);
	}

	int getUpVector(double *upVectorValuesOut3)
	{
		return cmzn_sceneviewer_get_up_vector(id, upVectorValuesOut3);
	}

	int setUpVector(double const *upVectorValuesIn3)
	{
		return cmzn_sceneviewer_set_up_vector(id, upVectorValuesIn3);
	}

	int getLookatParameters(double *eyeValuesOut3,double *lookatValuesOut3,double *upVectorValuesOut3)
	{
		return cmzn_sceneviewer_get_lookat_parameters(id, &eyeValuesOut3[0], &eyeValuesOut3[1], &eyeValuesOut3[2],
			&lookatValuesOut3[0], &lookatValuesOut3[1], &lookatValuesOut3[2],
			&upVectorValuesOut3[0], &upVectorValuesOut3[1], &upVectorValuesOut3[2]);
	}

	int setLookatParametersNonSkew(double const *eyeValuesIn3, double const *lookatValuesIn3, double const *upVectorValuesIn3)
	{
		return cmzn_sceneviewer_set_lookat_parameters_non_skew(id, eyeValuesIn3[0], eyeValuesIn3[1], eyeValuesIn3[2],
			lookatValuesIn3[0], lookatValuesIn3[1], lookatValuesIn3[2],
			upVectorValuesIn3[0], upVectorValuesIn3[1], upVectorValuesIn3[2]);
	}

	int getViewingVolume(double *left,double *right,double *bottom,double *top,
		double *near_plane, double *far_plane)
	{
		return cmzn_sceneviewer_get_viewing_volume(id, left, right, bottom, top,
			near_plane, far_plane);
	}

	int setViewingVolume(double left, double right, double bottom, double top,
		double near_plane, double far_plane)
	{
		return cmzn_sceneviewer_set_viewing_volume(id, left, right, bottom, top,
			near_plane, far_plane);
	}

	int setBackgroundColourComponentRGB(double red, double green, double blue)
	{
		return cmzn_sceneviewer_set_background_colour_component_rgb(id, red, green, blue);
	}

	int setBackgroundColourRGB(const double *valuesIn3)
	{
		return cmzn_sceneviewer_set_background_colour_rgb(id, valuesIn3);
	}

	int getBackgroundColourRGB(double *valuesOut3)
	{
		return cmzn_sceneviewer_get_background_colour_rgb(id, valuesOut3);
	}

	int viewAll()
	{
		return cmzn_sceneviewer_view_all(id);
	}

	TransparencyMode getTransparencyMode()
	{
		return static_cast<TransparencyMode>(cmzn_sceneviewer_get_transparency_mode(id));
	}

	int setTransparencyMode(TransparencyMode transparencyMode)
	{
		return cmzn_sceneviewer_set_transparency_mode(id,
			static_cast<cmzn_sceneviewer_transparency_mode>(transparencyMode));
	}

	int getTransparencyLayers()
	{
		return cmzn_sceneviewer_get_transparency_layers(id);
	}

	int setTransparencyLayers(int layers)
	{
		return cmzn_sceneviewer_set_transparency_layers(id, layers);
	}

	double getViewAngle()
	{
		return cmzn_sceneviewer_get_view_angle(id);
	}

	int setViewAngle(double viewAngle)
	{
		return cmzn_sceneviewer_set_view_angle(id, viewAngle);
	}

};

class Sceneviewermodule
{
protected:
	cmzn_sceneviewermodule_id id;

public:

	Sceneviewermodule() : id(0)
	{  }

	// takes ownership of C-style region reference
	Sceneviewermodule(cmzn_sceneviewermodule_id in_sceneviewermodule_id) :
		id(in_sceneviewermodule_id)
	{  }

	Sceneviewermodule(const Sceneviewermodule& sceneviewermodule) :
		id(cmzn_sceneviewermodule_access(sceneviewermodule.id))
	{  }

	Sceneviewermodule& operator=(const Sceneviewermodule& sceneviewermodule)
	{
		cmzn_sceneviewermodule_id temp_id = cmzn_sceneviewermodule_access(sceneviewermodule.id);
		if (0 != id)
		{
			cmzn_sceneviewermodule_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Sceneviewermodule()
	{
		if (0 != id)
		{
			cmzn_sceneviewermodule_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_sceneviewermodule_id getId()
	{
		return id;
	}

	Sceneviewer createSceneviewer(Sceneviewer::BufferingMode buffering_mode, Sceneviewer::StereoMode stereo_mode)
	{
		return Sceneviewer(cmzn_sceneviewermodule_create_sceneviewer(id,
			static_cast<cmzn_sceneviewer_buffering_mode>(buffering_mode),
			static_cast<cmzn_sceneviewer_stereo_mode>(stereo_mode)));
	}

};

}  // namespace Zinc
}

#endif
