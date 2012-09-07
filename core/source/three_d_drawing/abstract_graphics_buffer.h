/*******************************************************************************
FILE : graphics_buffer.h

LAST MODIFIED : 4 June 2004

DESCRIPTION :
This provides a Cmgui interface to the OpenGL contexts of many types.
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
#if !defined (ABSTRACT_GRAPHICS_BUFFER_H)
#define ABSTRACT_GRAPHICS_BUFFER_H
/*
Global types
------------
*/

enum Graphics_buffer_type
{
	GRAPHICS_BUFFER_INVALID_TYPE,
	GRAPHICS_BUFFER_GLX_X3D_TYPE,
	GRAPHICS_BUFFER_GLX_DM_PBUFFER_TYPE, /* Special type available only on O2's */
	GRAPHICS_BUFFER_GLX_PBUFFER_TYPE, /* Accelerated offscreen rendering */
	GRAPHICS_BUFFER_GLX_PIXMAP_TYPE, /* Non shared offscreen, no good for our display lists but
											  can be used for find_xi_special buffer */
	GRAPHICS_BUFFER_GTKGLAREA_TYPE,
	GRAPHICS_BUFFER_GTKGLEXT_TYPE,
	GRAPHICS_BUFFER_WIN32_TYPE,
	GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE, /* Accelerated offscreen rendering,
															  automatically copied on to screen. */
	GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE, /* Non accelerated offscreen rendering,
															 automatically copied on to screen. */
	GRAPHICS_BUFFER_WX_TYPE,
	GRAPHICS_BUFFER_WX_OFFSCREEN_TYPE,
	GRAPHICS_BUFFER_CARBON_TYPE,
	GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE
};

enum Graphics_buffer_buffering_mode
{
	GRAPHICS_BUFFER_ANY_BUFFERING_MODE,
	GRAPHICS_BUFFER_SINGLE_BUFFERING,
	GRAPHICS_BUFFER_DOUBLE_BUFFERING,
	GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_COPY,
	GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND
};

enum Graphics_buffer_stereo_mode
{
	GRAPHICS_BUFFER_ANY_STEREO_MODE,
	GRAPHICS_BUFFER_MONO,
	GRAPHICS_BUFFER_STEREO
};

enum Graphics_buffer_input_modifier
{
	GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT = 1,
	GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL = 2,
	GRAPHICS_BUFFER_INPUT_MODIFIER_ALT = 4,
	GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1 = 8
};

enum Graphics_buffer_input_event_type
{
	GRAPHICS_BUFFER_INVALID_INPUT,
	GRAPHICS_BUFFER_MOTION_NOTIFY,
	GRAPHICS_BUFFER_BUTTON_PRESS,
	GRAPHICS_BUFFER_BUTTON_RELEASE,
	GRAPHICS_BUFFER_KEY_PRESS,
	GRAPHICS_BUFFER_KEY_RELEASE
};

struct Graphics_buffer_input
{
	enum Graphics_buffer_input_event_type type;
	int button_number;
	int key_code;
	int position_x;
	int position_y;
	/* flags indicating the state of the shift, control and alt keys - use
		 logical OR with GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT etc. */
	enum Graphics_buffer_input_modifier input_modifier;
};

struct Graphics_buffer_expose_data
{
	int left;
	int bottom;
	int right;
	int top;
};

struct Graphics_buffer
{
	enum Graphics_buffer_type type;
	struct Graphics_buffer_package *package;
	int access_count;

	virtual ~Graphics_buffer() {}

	struct LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback))
		  *initialise_callback_list;
	struct LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback))
		  *resize_callback_list;
	struct LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback))
		  *expose_callback_list;
	struct LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_input_callback))
		  *input_callback_list;

	DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_callback, \
		struct Graphics_buffer *, void *, void);

	DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_input_callback, \
		struct Graphics_buffer *, struct Graphics_buffer_input *, void);

	Graphics_buffer *access() { access_count++; return this;}
	int deaccess() {access_count--; if (access_count == 0) delete this; return 1;}
	virtual double get_width() const { return 0.0; }
	virtual double get_height() const { return 0.0; }
	virtual int get_origin_x() const { return 0; }
	virtual int get_origin_y() const { return 0; }
	virtual int is_visible() const { return 0; }
	virtual int make_current() { return 0; }
	virtual int get_buffering_mode(enum Graphics_buffer_buffering_mode *graphics_buffer_buffering_mode) const { return 0; }
	virtual int get_stereo_mode(enum Graphics_buffer_stereo_mode *graphics_buffer_stereo_mode) const { return 0; }
	virtual int add_initialise_callback(CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) initialise_callback, void *user_data) { return 0; }
	virtual int add_resize_callback(CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) resize_callback, void *user_data) const { return 0; }
	virtual int add_expose_callback(CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) expose_callback, void *user_data) const { return 0; }
	virtual int add_input_callback(CMISS_CALLBACK_FUNCTION(Graphics_buffer_input_callback) input_callback, void *user_data) const { return 0; }
	virtual int awaken() const { return 0; }
	virtual int set_width(int width) { return 0; }
	virtual int set_height(int height) { return 0; }
	virtual int get_visual_id() const { return 0; }
	virtual int get_colour_buffer_depth() const { return 0; }
	virtual int get_depth_buffer_depth() const { return 0; }
	virtual int get_accumulation_buffer_depth() const { return 0; }
	virtual int swap_buffers() const { return 0; }
	virtual int buffer_awaken() const { return 0; }
};

#endif /* !defined (ABSTRACT_GRAPHICS_BUFFER_H) */
