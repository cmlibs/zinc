/***************************************************************************//**
 * rendergl.hpp
 * OpenGL rendering calls
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
#if !defined (RENDERGL_HPP)
#define RENDERGL_HPP
#include "graphics/graphics_object.h"
#include "graphics/render.hpp"
#include "graphics/graphics_object_highlight.hpp"

class Render_graphics_opengl : public Render_graphics_compile_members
{
public:
	Graphics_buffer *graphics_buffer;

	/** Specifies that we should be rendering only objects marked as fast
	 * changing.
	 */
	int fast_changing;
	/** Indicates that we are rendering specifically to pick objects.
	 * Requires the ndc transformation to be different as the viewport is the picking window.
	 */
	int picking;

	int allow_texture_tiling; /** Flag controls whether when compiling a large texture
								  it can be split into multiple texture tiles */
	Texture_tiling *texture_tiling;  /** If a given texture is compiled into tiles
													 then this field is filled in and expected to
													 be used when compiling graphics that use that material. */
	double viewport_width, viewport_height;

	int current_layer, number_of_layers;

	unsigned int buffer_width;
	unsigned int buffer_height;

	SubObjectGroupHighlightFunctor *highlight_functor;

public:
	Render_graphics_opengl(Graphics_buffer *graphics_buffer) :
		graphics_buffer(graphics_buffer),
		fast_changing(0),
		picking(0),
		allow_texture_tiling(0),
		texture_tiling(0),
		viewport_width(1.0),
		viewport_height(1.0),
		current_layer(0),
		number_of_layers(1),
		buffer_width(0),
		buffer_height(0),
		highlight_functor(NULL)
	{
	}

	virtual ~Render_graphics_opengl()
	{
		if (highlight_functor)
		{
			delete highlight_functor;
		}
	}

	/***************************************************************************//**
	 * @see Render_graphics::Graphics_object_compile
	 */
	virtual int Graphics_object_compile(GT_object *graphics_object);

	/***************************************************************************//**
	 * @see Render_graphics::Material_compile
	 */
	virtual int Material_compile(Graphical_material *material);

	virtual int rendering_layer(int layer)
	{
		if (layer == current_layer)
			return 1;
		if (layer >= number_of_layers)
			number_of_layers = layer + 1;
		return 0;
	}

	virtual int next_layer()
	{
		++current_layer;
		if (current_layer < number_of_layers)
			return 1;
		current_layer = 0; // reset
		return 0;
	}

	virtual int get_current_layer() const
	{
		return current_layer;
	}

	virtual int set_highlight_functor(SubObjectGroupHighlightFunctor *functor)
	{
		if (highlight_functor)
			delete highlight_functor;
		highlight_functor = functor;
		return 1;
	}
}; /* class Render_graphics_opengl */


/***************************************************************************//**
 * Factory function to create a renderer that uses immediate mode
 * glBegin/glEnd calls to render primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_glbeginend_renderer(
	Graphics_buffer *graphics_buffer);

/***************************************************************************//**
 * Factory function to create a renderer that compiles objects into display lists
 * for rendering and uses glBegin/glEnd calls when compiling primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_glbeginend_display_list_renderer(
	Graphics_buffer *graphics_buffer);

/***************************************************************************//**
 * Factory function to create a renderer that uses immediate mode
 * client vertex arrays to render primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_client_vertex_arrays_renderer(
	Graphics_buffer *graphics_buffer);

/***************************************************************************//**
 * Factory function to create a renderer that compiles objects into display lists
 * for rendering and uses client vertex arrays when compiling primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_client_vertex_arrays_display_list_renderer(
	Graphics_buffer *graphics_buffer);

/***************************************************************************//**
 * Factory function to create a renderer that uses immediate mode
 * vertex buffer objects to render primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_vertex_buffer_object_renderer(
	Graphics_buffer *graphics_buffer);

/***************************************************************************//**
 * Factory function to create a renderer that compiles objects into display lists
 * for rendering and uses vertex buffer objects when compiling primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_vertex_buffer_object_display_list_renderer(
	Graphics_buffer *graphics_buffer);

#endif /* !defined (RENDERGL_HPP) */
