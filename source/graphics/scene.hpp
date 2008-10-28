/*******************************************************************************
FILE : scene.h

LAST MODIFIED : 4 December 2003

DESCRIPTION :
Structure for storing the collections of objects that make up a 3-D graphical
model - lights, materials, primitives, etc.
Also contains interface routines for having these converted to display lists,
and for these to be assembled into a single display list of the whole scene.

HISTORY :
November 1997. Created from Scene description part of Drawing.
December 1997. Created MANAGER(Scene).
==============================================================================*/
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
#if !defined (SCENE_HPP)
#define SCENE_HPP

#include "general/callback_class.hpp"

struct Scene;
struct Scene_object;
struct Graphics_buffer;
class Render_graphics;
class Render_graphics_compile_members;
class Render_graphics_opengl;

/***************************************************************************//**
 * Render scene object.
 */
int execute_Scene_object(Scene_object *scene_object,
	Render_graphics_opengl *renderer);

/***************************************************************************//**
 * Compile members of a scene_object.
 */
int Scene_object_compile_members(Scene_object *scene_object,
	Render_graphics_compile_members *renderer);

/***************************************************************************//**
 * To speed up messaging response, graphical_elements put off building
 * graphics objects for their settings until requested. This function should be
 * called to request builds for all objects used by <scene>. It should be called
 * before the scene is output to OpenGL, VRML and wavefront objs.
 * This building is now incorporated into opengl scene compilation so does not need
 * to be called explicitly before rendering.
 */
int build_Scene(struct Scene *scene);

int compile_Scene(struct Scene *scene, struct Graphics_buffer *graphics_buffer);
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
Assembles the display list containing the whole scene. Before that, however, it
compiles the display lists of objects that will be executed in the scene.
The <graphics_buffer> is used to provide rendering contexts.
Note that lights are not included in the scene and must be handled separately!
Must also call build_Scene before this functions.
==============================================================================*/

/***************************************************************************//**
 * Actually render a scene by executing the scene objects.
 */
int Scene_render_opengl(Scene *scene, Render_graphics_opengl *renderer);

/***************************************************************************//**
 * Call the renderer to compile all the member objects which this Scene depends
 * on.
 */
int Scene_compile_members(struct Scene *scene,
	Render_graphics *renderer);

/***************************************************************************//**
 * Compile the display list for this object.
 */
int Scene_compile_opengl_display_list(struct Scene *scene,
	Callback_base< Scene * > *execute_function,
	Render_graphics_opengl *renderer);

/***************************************************************************//**
 * Execute the display list for this object.
 */
int Scene_execute_opengl_display_list(struct Scene *scene,
	Render_graphics_opengl *renderer);


#endif /* !defined (SCENE_HPP) */
