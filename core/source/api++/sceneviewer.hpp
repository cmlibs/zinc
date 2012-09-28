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

#include "api/cmiss_scene_viewer.h"
#include "api/new/cmiss_scene_viewer_input.h"
#include "api++/scene.hpp"

namespace Zn
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

	SceneViewer() : id(0)
	{  }

	// takes ownership of C-style region reference
	explicit SceneViewer(Cmiss_scene_viewer_id in_scene_viewer_id) :
		id(in_scene_viewer_id)
	{  }

	SceneViewer(const SceneViewer& scene_viewer_package) :
		id(Cmiss_scene_viewer_access(scene_viewer_package.id))
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

	int setScene(Scene scene)
	{
		return Cmiss_scene_viewer_set_scene(id, scene.getId());
	}

	int redrawNow()
	{
		return Cmiss_scene_viewer_redraw_now(id);
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

	int defaultInputCallback(SceneViewerInput input)
	{
		return Cmiss_scene_viewer_default_input_callback(id, input.getId());
	}

};

class SceneViewerPackage
{
protected:
	Cmiss_scene_viewer_package_id id;

public:

	SceneViewerPackage() : id(0)
	{  }

	// takes ownership of C-style region reference
	SceneViewerPackage(Cmiss_scene_viewer_package_id in_scene_viewer_package_id) :
		id(in_scene_viewer_package_id)
	{  }

	SceneViewerPackage(const SceneViewerPackage& scene_viewer_package) :
		id(Cmiss_scene_viewer_package_access(scene_viewer_package.id))
	{  }

	SceneViewerPackage& operator=(const SceneViewerPackage& scene_viewer_package)
	{
		Cmiss_scene_viewer_package_id temp_id = Cmiss_scene_viewer_package_access(scene_viewer_package.id);
		if (0 != id)
		{
			Cmiss_scene_viewer_package_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SceneViewerPackage()
	{
		if (0 != id)
		{
			Cmiss_scene_viewer_package_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_scene_viewer_package_id getId()
	{
		return id;
	}

	SceneViewer createSceneViewer(SceneViewer::BufferingMode buffering_mode, SceneViewer::StereoMode stereo_mode)
	{
		return SceneViewer(Cmiss_scene_viewer_create(id,
			static_cast<Cmiss_scene_viewer_buffering_mode>(buffering_mode),
			static_cast<Cmiss_scene_viewer_stereo_mode>(stereo_mode)));
	}

};

}  // namespace Zn

#endif
