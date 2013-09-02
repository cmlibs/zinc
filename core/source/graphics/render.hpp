/***************************************************************************//**
 * render.hpp
 * Header file for general rendering interface.
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
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Shane Blackett <shane@blackett.co.nz>
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
#ifndef RENDER_HPP
#define RENDER_HPP

struct cmzn_scene;
struct cmzn_graphics_filter;
#define Scene cmzn_scene // GRC temp
struct cmzn_graphic;
struct cmzn_scene;
struct GT_element_group;
struct Texture;
struct Graphical_material;
struct Light;
struct Light_model;
class SubObjectGroupHighlightFunctor;

/***************************************************************************//**
 * An interface class that defines how to implement a cmgui renderer.
 * By default the compile stages don't do anything.
 * The render stages are abstract virtual, so must be implemented for a renderer.
 * All objects have a compile and an execute stage.  The compile stages will be
 * called all before any executes.    Each compile stage should call back to
 * this renderer to ensure that the objects it uses are compiled when necessary.
 */
class Render_graphics
{
private:
	cmzn_scene *current_scene;
	cmzn_graphics_filter *graphics_filter;

public:
	Render_graphics() :
		current_scene(NULL),
		graphics_filter(NULL)
	{
	}

	virtual ~Render_graphics()
	{
	}
	
	virtual int Scene_compile(cmzn_scene *scene, cmzn_graphics_filter *graphics_filter) = 0;


	virtual int Scene_tree_execute(cmzn_scene *scene) = 0;

	/***************************************************************************//**
	 * Compile the Graphics_object.
	 */
	virtual int Graphics_object_compile(GT_object *graphics_object) = 0;

	/***************************************************************************//**
	 * Execute the Graphics_object.
	 */
	virtual int Graphics_object_execute(GT_object *graphics_object) = 0;

	/***************************************************************************//**
	 * Render the Graphics_object.  Typically as the graphics_object is temporary
	 * this method suggests to renderers that compile and then render that this object
	 * should instead render now.
	 */
	virtual int Graphics_object_render_immediate(GT_object *graphics_object) = 0;

	/***************************************************************************//**
	 * Execute the cmzn scene.
	 */
	virtual int cmzn_scene_execute(
		cmzn_scene *cmiss_scene) = 0;

	/***************************************************************************//**
	 * Compile the cmzn scene.
	 */
	virtual int cmzn_scene_compile(
	  cmzn_scene *cmiss_scene) = 0;

	virtual int cmzn_scene_execute_graphics(
		cmzn_scene *cmiss_scene) = 0;

	virtual int cmzn_scene_execute_child_scene(
		cmzn_scene *cmiss_scene) = 0;

	virtual int cmzn_scene_compile_members(
		cmzn_scene *cmiss_scene) = 0;
	
	/***************************************************************************//**
	 * Compile the Material.
	 */
	virtual int Material_compile(Graphical_material *material) = 0;
	
	/***************************************************************************//**
	 * Execute the Material.
	 */
	virtual int Material_execute(Graphical_material *material) = 0;

	/***************************************************************************//**
	 * Compile the Texture.
	 */
	virtual int Texture_compile(Texture *texture) = 0;
	
	/***************************************************************************//**
	 * Execute the Texture.
	 */
	virtual int Texture_execute(Texture *texture) = 0;

	/***************************************************************************//**
	 * Compile the Light.
	 */
	virtual int Light_compile(Light *light) = 0;
	
	/***************************************************************************//**
	 * Execute the Light.
	 */
	virtual int Light_execute(Light *light) = 0;

	/***************************************************************************//**
	 * Compile the Light.
	 */
	virtual int Light_model_compile(Light_model *light_model) = 0;
	
	/***************************************************************************//**
	 * Execute the Light.
	 */
	virtual int Light_model_execute(Light_model *light_model) = 0;

	cmzn_scene *get_Scene()
	{
		return current_scene;
	}

	cmzn_graphics_filter *getGraphicsFilter()
	{
		return graphics_filter;
	}

	void set_Scene(cmzn_scene *new_scene)
	{
		current_scene = new_scene;
	}

	void setGraphicsFilter(cmzn_graphics_filter *new_graphics_filter)
	{
		graphics_filter = new_graphics_filter;
	}

	/** Prepare coordinate system for executing graphics object.
	 * @return 1 on success, 0 if renderer should not display objects in coordinate system */
	virtual int begin_coordinate_system(enum cmzn_scene_coordinate_system /*coordinate_system*/) = 0;

	/** Must be called after executing graphics object after successful call to begin_coordinate_system. */
	virtual void end_coordinate_system(enum cmzn_scene_coordinate_system /*coordinate_system*/) = 0;

	/** Ensure this returns true for each graphic's layer before rendering.
	 * Override if renderer renders in layers to return true only when the
	 * supplied layer matches the current layer being rendered graphics. */
	virtual int rendering_layer(int /*layer*/)
	{
		return 1;
	}

	/** Override if renderer renders in layers to increment current layer if any remaining */
	virtual int next_layer()
	{
		return 0;
	}

	/** Override to return the current layer for layered renderers, starting at 0 */
	virtual int get_current_layer() const
	{
		return 0;
	}

	/** functor that determines either a primitive is highlighted or not */
	virtual int set_highlight_functor(SubObjectGroupHighlightFunctor *)
	{
		return 0;
	}

}; /* class Render_graphics */

class Render_graphics_compile_members : public Render_graphics
{

public:
	Render_graphics_compile_members() :
		time(0.0),
		name_prefix(NULL)
	{
		for (int i = 0; i < 16; i++)
		{
			world_view_matrix[i] = (i % 5) ? 0.0 : 1.0;
		}
	}

	FE_value time;
	/** Passed from cmiss_scenes to graphic for compilation */
	const char *name_prefix;
	/** set to initial modelview_matrix from viewer to get world coordinates.
	 * Values ordered down columns first, OpenGL style. Initialised to identity */
	double world_view_matrix[16];
	
	virtual int Scene_compile(cmzn_scene *scene, cmzn_graphics_filter *graphics_filter);

	/***************************************************************************//**
	 * Compile the cmzn scene.
	 */
	virtual int cmzn_scene_compile(
	  cmzn_scene *cmiss_scene);

	virtual int cmzn_scene_compile_members(
		cmzn_scene *cmiss_scene);

	/***************************************************************************//**
	 * @see Render_graphics::Texture_compile
	 */
	virtual int Texture_compile(Texture * /*texture*/)
	{
		/* No member objects */
		return 1;
	}

	/***************************************************************************//**
	 * @see Render_graphics::Light_compile
	 */
	virtual int Light_compile(Light * /*light*/)
	{
		/* No member objects */
		return 1;
	}
	
	/***************************************************************************//**
	 * @see Render_graphics::Light_model_compile
	 */
	virtual int Light_model_compile(Light_model * /*light_model*/)
	{
		/* No member objects */
		return 1;
	}

	void set_world_view_matrix(double *matrix)
	{
		for (int i = 0; i < 16; i++)
		{
			world_view_matrix[i] = matrix[i];
		}
	}
};


/***************************************************************************//**
 * This renderer ensures that all the sub-objects are up to date.  In particular
 * this triggers cmiss_scenes to generate their Graphics_object
 * representations.  (Previously this behaviour was build_GT_element_group.)
 * Unless overridden this renderer only builds objects so the execute methods all
 * just return 1. 
 */
class Render_graphics_build_objects : public Render_graphics_compile_members
{
public:
	Render_graphics_build_objects() : Render_graphics_compile_members()
	{
	}
	
	virtual int Scene_tree_execute(cmzn_scene *)
	{
		return 1;
	}

	/***************************************************************************//**
	 * Graphics objects are the primitives we are building so don't need to propagate.
	 */
	virtual int Graphics_object_compile(GT_object * /*graphics_object*/)
	{
		return 1;
	}

	/***************************************************************************//**
	 * By default this renderer only builds.
	 */
	virtual int Graphics_object_execute(GT_object * /*graphics_object*/)
	{
		return 1;
	}

	virtual int Graphics_object_render_immediate(GT_object * /*graphics_object*/)
	{
		return 1;
	}
	
	virtual int cmzn_scene_execute(cmzn_scene * /*cmiss_scene*/)
	{
		return 1;
	}

	virtual int cmzn_scene_execute_graphics(cmzn_scene * /*cmiss_scene*/)
	{
		return 1;
	}
	
	virtual int cmzn_scene_execute_child_scene(cmzn_scene * /*cmiss_scene*/)
	{
		return 1;
	}

	virtual int Material_execute(Graphical_material * /*material*/)
	{
		return 1;
	}

	/***************************************************************************//**
	 * Graphics objects are the primitives we are building so don't need to propagate.
	 * Could implement a Material_compile_members that isn't OpenGL but we don't need
	 * it at the moment.
	 */
	virtual int Material_compile(Graphical_material * /*material*/)
	{
		return 1;
	}

	virtual int Texture_execute(Texture * /*texture*/)
	{
		return 1;
	}

	virtual int Light_execute(Light * /*light*/)
	{
		return 1;
	}

	virtual int Light_model_execute(Light_model * /*light_model*/)
	{
		return 1;
	}

	virtual int begin_coordinate_system(enum cmzn_scene_coordinate_system /*coordinate_system*/)
	{
		return 1;
	}

	virtual void end_coordinate_system(enum cmzn_scene_coordinate_system /*coordinate_system*/)
	{
	}

};

#endif /* RENDER_HPP */
