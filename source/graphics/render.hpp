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

struct Scene;
struct Scene_object;
struct GT_element_group;
struct Texture;
struct Graphical_material;
struct Light;
struct Light_model;

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
public:
	virtual ~Render_graphics()
	{
	}
	
	/***************************************************************************//**
	 * Compile the Scene.
	 */
	virtual int Scene_compile(Scene *scene) = 0;
	
	/***************************************************************************//**
	 * Execute the Scene.
	 */
	virtual int Scene_execute(Scene *scene) = 0;

	/***************************************************************************//**
	 * Execute just the slow changing parts of the scene.
	 */
	virtual int Scene_execute_non_fast_changing(Scene *scene)
	{
		return Scene_execute(scene);
	}

	/***************************************************************************//**
	 * Execute just the fast changing parts of the scene.
	 */
	virtual int Scene_execute_fast_changing(Scene *scene)
	{
		return 1;
	}

	/***************************************************************************//**
	 * Compile the Scene_object.
	 */
	virtual int Scene_object_compile(Scene_object *scene_object) = 0;
	
	/***************************************************************************//**
	 * Execute the Scene_object.
	 */
	virtual int Scene_object_execute(Scene_object *scene_object) = 0;

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
	 * Execute the Graphical element group.
	 */
	virtual int Graphical_element_group_execute(
		GT_element_group *graphical_element_group) = 0;
	
	/***************************************************************************//**
	 * Compile the Graphical element group.
	 */
	virtual int Graphical_element_group_compile(
		GT_element_group *graphical_element_group) = 0;

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

}; /* class Render_graphics */


class Render_graphics_compile_members : public Render_graphics
{
public:
	Render_graphics_compile_members() : time(0.0), name_prefix(NULL)
	{
	}
	
	FE_value time;
	/** Passed from scene_objects to graphical_elements for compilation */
	const char *name_prefix;
	
	/***************************************************************************//**
	 * Compile the Scene.
	 */
	virtual int Scene_compile(Scene *scene);

	/***************************************************************************//**
	 * Compile the Scene_object.
	 */
	virtual int Scene_object_compile(Scene_object *scene_object);
		
	/***************************************************************************//**
	 * Compile the Graphical element group.
	 */
	virtual int Graphical_element_group_compile(
		GT_element_group *graphical_element_group);

	/***************************************************************************//**
	 * @see Render_graphics::Texture_compile
	 */
	virtual int Texture_compile(Texture *texture)
	{
		/* No member objects */
		return 1;
	}

	/***************************************************************************//**
	 * @see Render_graphics::Light_compile
	 */
	virtual int Light_compile(Light *light)
	{
		/* No member objects */
		return 1;
	}
	
	/***************************************************************************//**
	 * @see Render_graphics::Light_model_compile
	 */
	virtual int Light_model_compile(Light_model *light_model)
	{
		/* No member objects */
		return 1;
	}
	
};


/***************************************************************************//**
 * This renderer ensures that all the sub-objects are up to date.  In particular
 * this triggers graphical_elements to generate their Graphics_object
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
	
	/***************************************************************************//**
	 * By default this renderer only builds.
	 */
	virtual int Scene_execute(Scene *scene)
	{
		return 1;
	}

	/***************************************************************************//**
	 * By default this renderer only builds.
	 */
	virtual int Scene_object_execute(Scene_object *scene_object)
	{
		return 1;
	}

	/***************************************************************************//**
	 * Graphics objects are the primitives we are building so don't need to propagate.
	 */
	virtual int Graphics_object_compile(GT_object *graphics_object)
	{
		return 1;
	}

	/***************************************************************************//**
	 * By default this renderer only builds.
	 */
	virtual int Graphics_object_execute(GT_object *graphics_object)
	{
		return 1;
	}
	
	virtual int Graphics_object_render_immediate(GT_object *graphics_object)
	{
		return 1;
	}
	
	virtual int Graphical_element_group_execute(GT_element_group *graphical_element_group)
	{
		return 1;
	}
	
	virtual int Material_execute(Graphical_material *material)
	{
		return 1;
	}

	/***************************************************************************//**
	 * Graphics objects are the primitives we are building so don't need to propagate.
	 * Could implement a Material_compile_members that isn't OpenGL but we don't need
	 * it at the moment.
	 */
	virtual int Material_compile(Graphical_material *material)
	{
		return 1;
	}

	virtual int Texture_execute(Texture *texture)
	{
		return 1;
	}

	virtual int Light_execute(Light *light)
	{
		return 1;
	}

	virtual int Light_model_execute(Light_model *light_model)
	{
		return 1;
	}
	
};

#endif /* RENDER_HPP */
