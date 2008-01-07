/*******************************************************************************
FILE : material_editor_wx.cpp

LAST MODIFIED : 6 Nov 2007

DESCRIPTION :
Widgets for editing a graphical material.
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
extern "C" {
#include <math.h>
#define PI 3.1415927
#define PI_180 (PI/180.0)
#include <stdio.h>
#include "three_d_drawing/graphics_buffer.h"
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/font.h"
#include "graphics/texture.h"
#include "material/material_editor_wx.h"
#include "material/material_editor_dialog_wx.h"
#include "user_interface/message.h"
}
#include "wx/wx.h"
#include "wx/xrc/xmlres.h"
#include "choose/choose_manager_class.hpp"
#include "choose/choose_manager_listbox_class.hpp"
#include "colour/colour_editor_wx.hpp"
#include "icon/cmiss_icon.xpm"
#include "material/material_editor_wx.xrch"

class wxMaterialEditor;

/*
Module Types
------------
*/

struct Material_editor
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Contains all the information carried by the material_editor widget.
Note that we just hold a pointer to the material_editor, and must access and
deaccess it.
==============================================================================*/
{
	 int background;
	 /* edit_material is always a local copy of what is passed to the editor */
	 struct Graphical_material *edit_material, *current_material;
	 struct Graphics_buffer *graphics_buffer;
	 struct MANAGER(Graphical_material) *graphical_material_manager;
	 struct MANAGER(Texture) *texture_manager;
	 struct Material_editor_dialog **material_editor_dialog_address;
	 struct User_interface *user_interface;
	 wxMaterialEditor *wx_material_editor;
	 //wxListBox *material_editor_list_box;
	 wxCheckBox *material_editor_texture_check_box, *material_editor_second_texture_check_box,
			*material_editor_third_texture_check_box, *material_editor_fourth_texture_check_box,
			*material_editor_per_pixel_checkbox, *material_editor_bump_mapping_checkbox;
	 wxPanel *material_editor_ambient_colour_panel, *material_editor_diffuse_colour_panel,
			*material_editor_emitted_colour_panel, *material_editor_specular_colour_panel,
			*material_editor_sample_panel, *material_editor_texture_chooser_panel,
			*material_editor_second_texture_chooser_panel,
			*material_editor_third_texture_chooser_panel,
			*material_editor_fourth_texture_chooser_panel, *test_list_panel;
	 wxSlider *material_editor_alpha_slider, *material_editor_shininess_slider;
	 wxTextCtrl *material_editor_alpha_text_ctrl, *material_editor_shininess_text_ctrl;
	 Colour_editor *ambient_colour_editor, *diffuse_colour_editor, *emitted_colour_editor,
			*specular_colour_editor;
 	 void *material_manager_callback_id;
}; /* Material_editor */

/*
Global functions
----------------
*/

/******************************************************************************
 prototype
==============================================================================*/
static int make_current_material(struct Material_editor *material_editor,
	 struct Graphical_material *material);

static int material_editor_draw_sphere(struct Material_editor *material_editor)
/*******************************************************************************
LAST MODIFIED : 13 March 2002

DESCRIPTION :
Uses gl to draw a sphere with a lighting source.
==============================================================================*/
{
#define sphere_horiz 40
#define sphere_vert 40
#define sphere_view_dist 1.5
#define sphere_panel_size 1000
#define sphere_panel_dist 5
#define sphere_view_spacing 1.2
	int height,i,j,return_code,width;
#if defined (OPENGL_API)
	float texture_depth, texture_height, texture_width;
	GLdouble angle,aspect,coordinates[3],cos_angle,horiz_factor,horiz_offset,
		lower_coordinate,lower_radius,sin_angle,texture_coordinates[2],
		upper_coordinate,upper_radius,vert_factor,vert_offset,viewport_size[4];
	GLfloat light_position[]=
	{
		0.0, 5.0, 4.0, 0.0
	};
	GLfloat light_model_twoside=1.0;
	struct Texture *texture;
#endif /* defined (OPENGL_API) */

	ENTER(material_editor_draw_sphere);
	if (material_editor)
	{
		return_code=1;
		/* make sure the Graphical material display list is up-to-date, which
			 in turn, requires any textures it uses to be compiled */
		compile_Graphical_material(material_editor->edit_material,NULL, (struct Texture_tiling **)NULL);
#if defined (OPENGL_API)
		width = Graphics_buffer_get_width(material_editor->graphics_buffer);
		height = Graphics_buffer_get_height(material_editor->graphics_buffer);
		glViewport(0, 0, width, height);
		glGetDoublev(GL_VIEWPORT,viewport_size);
		glClearColor(0.0,0.0,0.0,0.0);
		glClearDepth(1.0);
		glEnable(GL_BLEND);
		glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,light_model_twoside);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		/* set up the view trans */
		if (viewport_size[3] > viewport_size[2])
		{
			if (0 != viewport_size[2])
			{
				aspect=viewport_size[3]/viewport_size[2];
			}
			else
			{
				aspect=1.0;
			}
			glOrtho(-sphere_view_spacing,sphere_view_spacing,
				-aspect*sphere_view_spacing,aspect*sphere_view_spacing,0.1,20.0);
		}
		else
		{
			if (0 != viewport_size[3])
			{
				aspect=viewport_size[2]/viewport_size[3];
			}
			else
			{
				aspect=1.0;
			}
			glOrtho(-aspect*sphere_view_spacing,aspect*sphere_view_spacing,
				-sphere_view_spacing,sphere_view_spacing,0.1,20.0);
		}
		/* set up the material and lights etc */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glLightfv(GL_LIGHT0,GL_POSITION,light_position);
		glEnable(GL_LIGHT0);
		gluLookAt(0.0,0.0,sphere_view_dist,0.0,0.0,0.0,0.0,1.0,0.0);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);
		if (2==material_editor->background)
		{
			glClearColor(1.0,1.0,1.0,1.0);
		}
		else
		{
			glClearColor(0.0,0.0,0.0,1.0);
		}
		/* clear the window */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (material_editor->background==0)
		{
			/* no material on the RGB background */
			execute_Graphical_material((struct Graphical_material *)NULL);
			glDisable(GL_LIGHTING);
			glBegin(GL_TRIANGLES);
			/* red */
			glColor3d(1.0,0.0,0.0);
			glVertex3d(0.0,0.0,-sphere_panel_dist);
			glVertex3d(sphere_panel_size*0.866,-sphere_panel_size*0.5,
				-sphere_panel_dist);
			glVertex3d(0.0,sphere_panel_size,-sphere_panel_dist);
			/* green */
			glColor3d(0.0,1.0,0.0);
			glVertex3d(0.0,0.0,-sphere_panel_dist);
			glVertex3d(0.0,sphere_panel_size,-sphere_panel_dist);
			glVertex3d(-sphere_panel_size*0.866,-sphere_panel_size*0.5,
				-sphere_panel_dist);
			/* blue */
			glColor3d(0.0,0.0,1.0);
			glVertex3d(0.0,0.0,-sphere_panel_dist);
			glVertex3d(-sphere_panel_size*0.866,-sphere_panel_size*0.5,
				-sphere_panel_dist);
			glVertex3d(sphere_panel_size*0.866,-sphere_panel_size*0.5,
				-sphere_panel_dist);
			glEnd();
		}
		execute_Graphical_material(material_editor->edit_material);
		/* draw the sphere */
		glEnable(GL_LIGHTING);
		if (texture=Graphical_material_get_texture(material_editor->edit_material))
		{
			Texture_get_physical_size(texture,&texture_width, &texture_height,
				&texture_depth);
			horiz_factor = 2.0*texture_width/sphere_horiz;
			horiz_offset = -0.5*texture_width;
			vert_factor = 2.0*texture_height/sphere_vert;
			vert_offset = -0.5*texture_height;
		}
		/* loop from bottom to top */
		for(j=0;j<sphere_vert;j++)
		{
			angle = (double)j * (PI/(double)sphere_vert);
			lower_coordinate=-cos(angle);
			lower_radius=sin(angle);
			angle = ((double)j+1.0) * (PI/(double)sphere_vert);
			upper_coordinate=-cos(angle);
			upper_radius=sin(angle);
			glBegin(GL_QUAD_STRIP);
			for(i=0;i<=sphere_horiz;i++)
			{
				angle = (double)i * (PI/(double)sphere_horiz);
				cos_angle=cos(angle);
				sin_angle=sin(angle);
				coordinates[0] = -cos_angle*upper_radius;
				coordinates[1] = upper_coordinate;
				coordinates[2] = sin_angle*upper_radius;
				if (texture)
				{
					texture_coordinates[0]=horiz_offset+(double)i*horiz_factor;
					texture_coordinates[1]=vert_offset+((double)j+1.0)*vert_factor;
					glTexCoord2dv(texture_coordinates);
				}
				glNormal3dv(coordinates);
				glVertex3dv(coordinates);
				coordinates[0] = -cos_angle*lower_radius;
				coordinates[1] = lower_coordinate;
				coordinates[2] = sin_angle*lower_radius;
				if (texture)
				{
					texture_coordinates[1]=vert_offset+(double)j*vert_factor;
					glTexCoord2dv(texture_coordinates);
				}
				glNormal3dv(coordinates);
				glVertex3dv(coordinates);
			}
			glEnd();
		}
		/* turn off material */
		execute_Graphical_material((struct Graphical_material *)NULL);
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_draw_sphere.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_draw_sphere */

static int material_editor_update_picture(
	struct Material_editor *material_editor)
/*******************************************************************************
LAST MODIFIED : 14 Novemeber 2007

DESCRIPTION :
Updates the picture with the changed material.
==============================================================================*/
{
	int return_code;

	ENTER(material_editor_update_picture);
	if (material_editor)
	{
		 Graphics_buffer_make_current(material_editor->graphics_buffer);
		 return_code=material_editor_draw_sphere(material_editor);
		 Graphics_buffer_swap_buffers(material_editor->graphics_buffer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update_picture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_update_picture */

static void material_editor_expose_picture_callback(struct Graphics_buffer *graphics_buffer,
	void *dummy_void, void *material_editor_void)
/*******************************************************************************
LAST MODIFIED : 27 Nov 2007

DESCRIPTION :
Forces a redraw of the picture representing the material.
==============================================================================*/
{
	struct Material_editor *material_editor;

	ENTER(material_editor_expose_picture_callback);
	USE_PARAMETER(graphics_buffer);
	USE_PARAMETER(dummy_void);
	if (material_editor=(struct Material_editor *)material_editor_void)
	{
		material_editor_update_picture(material_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_expose_picture_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_expose_picture_callback */

static void material_editor_change_background(struct Graphics_buffer *graphics_buffer,
	struct Graphics_buffer_input *input, void *material_editor_void)
/*******************************************************************************
LAST MODIFIED : 27 Nov 2007

DESCRIPTION :
Increments the background pattern.
==============================================================================*/
{
	struct Material_editor *material_editor;

	ENTER(material_editor_change_background);
	USE_PARAMETER(graphics_buffer);
	if (material_editor=(struct Material_editor *)material_editor_void)
	{
		if (GRAPHICS_BUFFER_BUTTON_PRESS == input->type)
		{
			(material_editor->background)++;
			if (material_editor->background>2)
			{
				material_editor->background=0;
			}
			material_editor_update_picture(material_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_change_background.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_change_background */

void material_editor_wx_set_textctrl_and_slider(wxTextCtrl *temp_textctrl, wxSlider *temp_slider,
	 MATERIAL_PRECISION temp_value)
{
	 char temp_str[20];
	 int slider_value;
	 MATERIAL_PRECISION temp;

	 sprintf(temp_str,MATERIAL_NUM_FORMAT,temp_value);
	 temp_textctrl->SetValue(temp_str);
	 temp = temp_value * 100;
	 slider_value = (int)(temp+0.5);
	 temp_slider->SetValue(slider_value);
}

void material_editor_wx_update_alpha(Material_editor *material_editor, float alpha)
{
	 Graphical_material_set_alpha(material_editor->edit_material,
			(MATERIAL_PRECISION) alpha);
	 material_editor_update_picture(material_editor);
	 material_editor_wx_set_textctrl_and_slider(material_editor->material_editor_alpha_text_ctrl, 
			material_editor->material_editor_alpha_slider,
			(MATERIAL_PRECISION) alpha);
}

void material_editor_wx_update_shininess(Material_editor *material_editor, float shininess)
{
	 Graphical_material_set_shininess(material_editor->edit_material,
			(MATERIAL_PRECISION) shininess);
	 material_editor_update_picture(material_editor);
	 material_editor_wx_set_textctrl_and_slider(material_editor->material_editor_shininess_text_ctrl, 
			material_editor->material_editor_shininess_slider,
			(MATERIAL_PRECISION) shininess);
}

void material_editor_wx_update_texture(Material_editor *material_editor, Texture *texture)
{
	 Graphical_material_set_texture(material_editor->edit_material,
			texture);
	 material_editor_update_picture(material_editor);
}

void material_editor_wx_update_second_texture(Material_editor *material_editor, Texture *texture)
{
	 Graphical_material_set_second_texture(material_editor->edit_material,
			texture);
	 material_editor_update_picture(material_editor);
}

void material_editor_wx_update_third_texture(Material_editor *material_editor, Texture *texture)
{
	 Graphical_material_set_third_texture(material_editor->edit_material,
			texture);
	 material_editor_update_picture(material_editor);
}

void material_editor_wx_update_fourth_texture(Material_editor *material_editor, Texture *texture)
{
	 Graphical_material_set_fourth_texture(material_editor->edit_material,
			texture);
	 material_editor_update_picture(material_editor);
}

class wxMaterialEditor : public wxFrame
{
	 Material_editor *material_editor;
	 DEFINE_MANAGER_CLASS(Texture);
	 Managed_object_chooser<Texture, MANAGER_CLASS(Cmiss_texture)>
	 *material_editor_texture_chooser;
	 Managed_object_chooser<Texture, MANAGER_CLASS(Cmiss_texture)>
	 *material_editor_second_texture_chooser;
	 Managed_object_chooser<Texture, MANAGER_CLASS(Cmiss_texture)>
	 *material_editor_third_texture_chooser;
	 Managed_object_chooser<Texture, MANAGER_CLASS(Cmiss_texture)>
	 *material_editor_fourth_texture_chooser;
	 DEFINE_MANAGER_CLASS(Graphical_material);
	 Managed_object_listbox<Graphical_material, MANAGER_CLASS(Graphical_material)>
	 *graphical_material_object_listbox;

public:

	 wxMaterialEditor(Material_editor *material_editor):
			material_editor(material_editor)
	 {
			wxXmlInit_material_editor_wx();
			wxXmlResource::Get()->LoadFrame(this,
				 (wxWindow *)NULL, _T("CmguiMaterialEditor"));
			this->SetIcon(cmiss_icon_xpm);


			material_editor->test_list_panel = XRCCTRL(*this,
				 "TestListPanel", wxPanel);
			material_editor->test_list_panel->SetSize(wxDefaultCoord,wxDefaultCoord,
				 400, 200);
			material_editor->test_list_panel->SetMinSize(wxSize(-1,100));

			graphical_material_object_listbox =
				 new Managed_object_listbox<Graphical_material, MANAGER_CLASS(Graphical_material)>
				 (material_editor->test_list_panel, (struct Graphical_material*)NULL, material_editor->graphical_material_manager,
						(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL, (void *)NULL, material_editor->user_interface);
			Callback_base<Graphical_material* > *material_editor_graphical_material_list_callback = 
				 new Callback_member_callback< Graphical_material*,
				 wxMaterialEditor, int (wxMaterialEditor::*)(Graphical_material *) >
				 (this, &wxMaterialEditor::material_editor_graphical_material_list_callback);
			graphical_material_object_listbox->set_callback(material_editor_graphical_material_list_callback);

			material_editor->material_editor_texture_chooser_panel = 
				 XRCCTRL(*this, "MaterialEditorTextureChooserPanel", wxPanel);
			material_editor_texture_chooser =
				 new Managed_object_chooser<Texture, MANAGER_CLASS(Cmiss_texture)>
				 (material_editor->material_editor_texture_chooser_panel, (struct Texture*)NULL, material_editor->texture_manager,
						(MANAGER_CONDITIONAL_FUNCTION(Texture) *)NULL, (void *)NULL, material_editor->user_interface);
			Callback_base<Texture* > *material_editor_texture_callback = 
				 new Callback_member_callback< Texture*,
				 wxMaterialEditor, int (wxMaterialEditor::*)(Texture *) >
				 (this, &wxMaterialEditor::material_editor_texture_callback);
			material_editor_texture_chooser->set_callback(material_editor_texture_callback);

			material_editor->material_editor_second_texture_chooser_panel = 
				 XRCCTRL(*this, "MaterialEditorSecondTextureChooserPanel", wxPanel);
			material_editor_second_texture_chooser =
				 new Managed_object_chooser<Texture, MANAGER_CLASS(Cmiss_texture)>
				 (material_editor->material_editor_second_texture_chooser_panel, (struct Texture*)NULL, 
						material_editor->texture_manager, (MANAGER_CONDITIONAL_FUNCTION(Texture) *)NULL, 
						(void *)NULL, material_editor->user_interface);
			Callback_base<Texture* > *material_editor_second_texture_callback = 
				 new Callback_member_callback< Texture*,
				 wxMaterialEditor, int (wxMaterialEditor::*)(Texture *) >
				 (this, &wxMaterialEditor::material_editor_second_texture_callback);
			material_editor_second_texture_chooser->set_callback(material_editor_second_texture_callback);

			material_editor->material_editor_third_texture_chooser_panel = 
				 XRCCTRL(*this, "MaterialEditorThirdTextureChooserPanel", wxPanel);
			material_editor_third_texture_chooser =
				 new Managed_object_chooser<Texture, MANAGER_CLASS(Cmiss_texture)>
				 (material_editor->material_editor_third_texture_chooser_panel, (struct Texture*)NULL, 
						material_editor->texture_manager, (MANAGER_CONDITIONAL_FUNCTION(Texture) *)NULL, 
						(void *)NULL, material_editor->user_interface);
			Callback_base<Texture* > *material_editor_third_texture_callback = 
				 new Callback_member_callback< Texture*,
				 wxMaterialEditor, int (wxMaterialEditor::*)(Texture *) >
				 (this, &wxMaterialEditor::material_editor_third_texture_callback);
			material_editor_third_texture_chooser->set_callback(material_editor_third_texture_callback);

			material_editor->material_editor_fourth_texture_chooser_panel = 
				 XRCCTRL(*this, "MaterialEditorFourthTextureChooserPanel", wxPanel);
			material_editor_fourth_texture_chooser =
				 new Managed_object_chooser<Texture, MANAGER_CLASS(Cmiss_texture)>
				 (material_editor->material_editor_fourth_texture_chooser_panel, (struct Texture*)NULL, 
						material_editor->texture_manager, (MANAGER_CONDITIONAL_FUNCTION(Texture) *)NULL, 
						(void *)NULL, material_editor->user_interface);
			Callback_base<Texture* > *material_editor_fourth_texture_callback = 
				 new Callback_member_callback< Texture*,
				 wxMaterialEditor, int (wxMaterialEditor::*)(Texture *) >
				 (this, &wxMaterialEditor::material_editor_fourth_texture_callback);
			material_editor_fourth_texture_chooser->set_callback(material_editor_fourth_texture_callback);
	 };

	 wxMaterialEditor()
	 {
	 };

	 ~wxMaterialEditor()
	 {
			delete graphical_material_object_listbox;
			delete material_editor_texture_chooser;
			delete material_editor_second_texture_chooser;
			delete material_editor_third_texture_chooser;
			delete material_editor_fourth_texture_chooser;
	 };

int material_editor_graphical_material_list_callback(Graphical_material *material)
{
	 ENTER(material_editor_graphical_material_callback);
	 if (!((graphical_material_object_listbox->get_number_of_object() > 0) && (material == NULL)))
	 {
			material_editor->current_material = material;
			material_editor_wx_set_material(material_editor,material);
	 }
	 LEAVE;
	 
	 return 1;
}

void material_editor_graphical_material_list_set_selected(Graphical_material *material)
{
	 ENTER(material_editor_graphical_material_list_set_selected);
	 if (graphical_material_object_listbox)
	 {
			graphical_material_object_listbox->set_object(material);
	 }
	 LEAVE;
}

struct Graphical_material *material_editor_graphical_material_list_get_selected()
{
	 if (graphical_material_object_listbox)
	 {
			return(graphical_material_object_listbox->get_object());
	 }
	 else
	 {
			return ((Graphical_material *)NULL);
	 }
}

int material_editor_texture_callback(Texture *texture)
/*******************************************************************************
LAST MODIFIED : 29 November  2007

DESCRIPTION :
Callback for the texture chooser.
==============================================================================*/
{
	 if (texture != NULL)
			material_editor_wx_update_texture(material_editor, texture);
	 return 1;
}

int material_editor_second_texture_callback(Texture *texture)
/*******************************************************************************
LAST MODIFIED : 5 Dec  2007

DESCRIPTION :
Callback for the second texture chooser.
==============================================================================*/
{
	 if (texture != NULL)
			material_editor_wx_update_second_texture(material_editor, texture);
	 return 1;
}

int material_editor_third_texture_callback(Texture *texture)
/*******************************************************************************
LAST MODIFIED : 5 Dec  2007

DESCRIPTION :
Callback for the third texture chooser.
==============================================================================*/
{
	 if (texture != NULL)
			material_editor_wx_update_third_texture(material_editor, texture);
	 return 1;
}

int material_editor_fourth_texture_callback(Texture *texture)
/*******************************************************************************
LAST MODIFIED : 5 Dec  2007

DESCRIPTION :
Callback for the fourth texture chooser.
==============================================================================*/
{
	 if (texture != NULL)
			material_editor_wx_update_fourth_texture(material_editor, texture);
	 return 1;
}

	 void material_editor_set_texture_object(Texture *texture)
/*******************************************************************************
LAST MODIFIED : 29 November 2007

DESCRIPTION :
Set the selected option in the testyre chooser.
==============================================================================*/
	 {
			material_editor_texture_chooser->set_object(texture);
	 }

	 void material_editor_set_second_texture_object(Texture *texture)
/*******************************************************************************
LAST MODIFIED : 29 November 2007

DESCRIPTION :
Set the selected option in the testyre chooser.
==============================================================================*/
	 {
			material_editor_second_texture_chooser->set_object(texture);
	 }

	 void material_editor_set_third_texture_object(Texture *texture)
/*******************************************************************************
LAST MODIFIED : 29 November 2007

DESCRIPTION :
Set the selected option in the testyre chooser.
==============================================================================*/
	 {
			material_editor_third_texture_chooser->set_object(texture);
	 }

	 void material_editor_set_fourth_texture_object(Texture *texture)
/*******************************************************************************
LAST MODIFIED : 29 November 2007

DESCRIPTION :
Set the selected option in the testyre chooser.
==============================================================================*/
	 {
			material_editor_fourth_texture_chooser->set_object(texture);
	 }

void OnMaterialEditorAlphaTextEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 28 November 2007

DESCRIPTION :
Callback for the alpha text widgets.
==============================================================================*/
 {
		char *text;
		float alpha;
		
		ENTER(OnMaterialEditorAlphaTextEntered);
		if (material_editor->material_editor_alpha_text_ctrl)
		{
			 text = (char*)material_editor->material_editor_alpha_text_ctrl->GetValue().mb_str(wxConvUTF8);
			 if (text)
			 {
					sscanf(text,"%f",&alpha);
			 }
			 else
			 {
					display_message(ERROR_MESSAGE,
						 "OnMaterialEditorAlphaTextEnteredValueEntered.  Missing alpha text");
			 }
			 material_editor_wx_update_alpha(material_editor, alpha);
		}

		LEAVE;
 }
	 
void OnMaterialEditorShininessTextEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 28 November 2007

DESCRIPTION :
Callback for the shininess text widgets.
==============================================================================*/
	 {
			char *text;
			float shininess;

			ENTER(OnMaterialEditorShininessTextEntered);
			if (material_editor->material_editor_alpha_text_ctrl)
			{
				 text = (char*)material_editor->material_editor_shininess_text_ctrl->GetValue().mb_str(wxConvUTF8);
				 if (text)
				 {
						sscanf(text,"%f",&shininess);
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "OnMaterialEditorShininessTextEnteredValueEntered.  Missing shininess text");
				 }
				 material_editor_wx_update_shininess(material_editor, shininess);
			}

			LEAVE;
	 }

void OnMaterialEditorAlphaSliderChanged(wxScrollEvent& event)	 
/*******************************************************************************
LAST MODIFIED : 28 November 2007

DESCRIPTION :
Callback for the alpha slider.
==============================================================================*/
 {
		int value;
		float alpha;
		
		ENTER(OnMaterialEditorAlphaSliderChanged);
		if (material_editor->material_editor_alpha_slider)
		{
			 value = material_editor->material_editor_alpha_slider->GetValue();
			 alpha = (float)value/(float)100.0;
		}
		material_editor_wx_update_alpha(material_editor, alpha);

		LEAVE;
 }

void OnMaterialEditorShininessSliderChanged(wxScrollEvent& event)	 
/*******************************************************************************
LAST MODIFIED : 28 November 2007

DESCRIPTION :
Callback for the shininess slider.
==============================================================================*/
 {
		int value;
		float shininess;
		
		ENTER(OnMaterialEditorShininessSliderChanged);
		if (material_editor->material_editor_shininess_slider)
		{
			 value = material_editor->material_editor_shininess_slider->GetValue();
			 shininess = (float)value/(float)100.0;
		}
		material_editor_wx_update_shininess(material_editor, shininess);

		LEAVE;
 }

void OnMaterialEditorTextureCheckBoxChecked(wxCommandEvent& event)
/*******************************************************************************
LAST MODIFIED : 28 November 2007

DESCRIPTION :
Callback for the texture check box
==============================================================================*/
{
	 Texture *texture;
	 int return_code;

	 ENTER(OnMaterialEditorTextureCheckBoxChecked);
	 texture = (struct Texture *)NULL;
	 if (material_editor->material_editor_texture_check_box)
	 {
			return_code = 0;
			if (material_editor->material_editor_texture_check_box->IsChecked())
			{
				 return_code = material_editor_texture_chooser->get_number_of_object();				 
			}
			if (return_code  == 0 )
			{
				 material_editor->material_editor_texture_check_box->SetValue(0);
				 material_editor->material_editor_texture_chooser_panel->Enable(false);
			}
			else
			{
				 material_editor->material_editor_texture_chooser_panel->Enable();
				 texture = material_editor_texture_chooser->get_object();
			}
			material_editor_wx_update_texture(material_editor, texture);
	 }

	 LEAVE;
}

void OnMaterialEditorSecondTextureCheckBoxChecked(wxCommandEvent& event)
/*******************************************************************************
LAST MODIFIED : 28 November 2007

DESCRIPTION :
Callback for the texture check box
==============================================================================*/
{
	 Texture *texture;
	 int return_code;

	 ENTER(OnMaterialEditorSecondTextureCheckBoxChecked);
	 texture = (struct Texture *)NULL;
	 if (material_editor->material_editor_second_texture_check_box)
	 {
			return_code = 0;
			if (material_editor->material_editor_second_texture_check_box->IsChecked())
			{
				 return_code = material_editor_second_texture_chooser->get_number_of_object();
				 if (material_editor->material_editor_per_pixel_checkbox->IsChecked())
				 {
						material_editor->material_editor_bump_mapping_checkbox->Enable(true);
				 }
			}
			else
			{
				 material_editor->material_editor_bump_mapping_checkbox->SetValue(false);
				 material_editor->material_editor_bump_mapping_checkbox->Enable(false);
				 if (material_editor->material_editor_per_pixel_checkbox->IsChecked())
				 {
						set_material_program_type(material_editor->edit_material,
							 /*bump_mapping_flag */ 0, 
							 0, 0, 0, 0, 0, 0, 0, 1);
				 }
				 else
				 {
						material_deaccess_material_program(material_editor->edit_material);
				 }
			}
			if (return_code  == 0 )
			{
				 material_editor->material_editor_second_texture_check_box->SetValue(0);
				 material_editor->material_editor_second_texture_chooser_panel->Enable(false);
			}
			else
			{
				 material_editor->material_editor_second_texture_chooser_panel->Enable();
				 texture = material_editor_second_texture_chooser->get_object();
			}
			material_editor_wx_update_second_texture(material_editor, texture);
	 }

	 LEAVE;
}

void OnMaterialEditorThirdTextureCheckBoxChecked(wxCommandEvent& event)
/*******************************************************************************
LAST MODIFIED : 28 November 2007

DESCRIPTION :
Callback for the texture check box
==============================================================================*/
{
	 Texture *texture;
	 int return_code;

	 ENTER(OnMaterialEditorThirdTextureCheckBoxChecked);
	 texture = (struct Texture *)NULL;
	 if (material_editor->material_editor_third_texture_check_box)
	 {
			return_code = 0;
			if (material_editor->material_editor_third_texture_check_box->IsChecked())
			{
				 return_code = material_editor_third_texture_chooser->get_number_of_object();				 
			}
			if (return_code  == 0 )
			{
				 material_editor->material_editor_third_texture_check_box->SetValue(0);
				 material_editor->material_editor_third_texture_chooser_panel->Enable(false);
			}
			else
			{
				 material_editor->material_editor_third_texture_chooser_panel->Enable();
				 texture = material_editor_third_texture_chooser->get_object();
			}
			material_editor_wx_update_third_texture(material_editor, texture);
	 }

	 LEAVE;
}

void OnMaterialEditorFourthTextureCheckBoxChecked(wxCommandEvent& event)
/*******************************************************************************
LAST MODIFIED : 28 November 2007

DESCRIPTION :
Callback for the texture check box
==============================================================================*/
{
	 Texture *texture;
	 int return_code;

	 ENTER(OnMaterialEditorFourthTextureCheckBoxChecked);
	 texture = (struct Texture *)NULL;
	 if (material_editor->material_editor_fourth_texture_check_box)
	 {
			return_code = 0;
			if (material_editor->material_editor_fourth_texture_check_box->IsChecked())
			{
				 return_code = material_editor_fourth_texture_chooser->get_number_of_object();				 
			}
			if (return_code  == 0 )
			{
				 material_editor->material_editor_fourth_texture_check_box->SetValue(0);
				 material_editor->material_editor_fourth_texture_chooser_panel->Enable(false);
			}
			else
			{
				 material_editor->material_editor_fourth_texture_chooser_panel->Enable();
				 texture = material_editor_fourth_texture_chooser->get_object();
			}
			material_editor_wx_update_fourth_texture(material_editor, texture);
	 }

	 LEAVE;
}

void OnMaterialEditorApplyButtonPressed(wxCommandEvent& event)
{
	 ENTER(OnMateruialEditorApplyButtonPressed)
	 if (material_editor->current_material && material_editor->edit_material)
	 {
			MANAGER_MODIFY_NOT_IDENTIFIER(Graphical_material,name)(
				 material_editor->current_material,material_editor->edit_material,
				 material_editor->graphical_material_manager);
			material_copy_bump_mapping_and_per_pixel_lighting_flag(material_editor->edit_material,
				 material_editor->current_material);
	 }
	 LEAVE;
}
void OnMaterialEditorOKButtonPressed(wxCommandEvent& event)
{
	 ENTER(OnMateruialEditorOKButtonPressed)
	 if (material_editor->current_material && material_editor->edit_material)
	 {
			MANAGER_MODIFY_NOT_IDENTIFIER(Graphical_material,name)(
				 material_editor->current_material,material_editor->edit_material,
				 material_editor->graphical_material_manager);
			material_copy_bump_mapping_and_per_pixel_lighting_flag(material_editor->edit_material,
				 material_editor->current_material);
	 }
	 DESTROY(Material_editor_dialog)(material_editor->material_editor_dialog_address);
	 LEAVE;
}
void OnMaterialEditorRevertButtonPressed(wxCommandEvent& event)
{
	 ENTER(MateruialEditorRevertButtonPressed)
	 material_editor_wx_set_material(material_editor, material_editor->current_material);
	 LEAVE;
}
void OnMaterialEditorCancelButtonPressed(wxCommandEvent& event)
{
	 ENTER(OnMateruialEditorCancelButtonPressed)
	 material_editor_wx_set_material(material_editor, material_editor->current_material);
	 DESTROY(Material_editor_dialog)(material_editor->material_editor_dialog_address);
	 LEAVE;
}

void OnMaterialEditorCreateNewMaterial(wxCommandEvent& event)
{
	 ENTER(OnMaterialEditorCreateNewMaterial);
	 Graphical_material *material;
	 char *text;
	 wxTextEntryDialog *NewMaterialDialog = new wxTextEntryDialog(this, "Enter name", 
			"Please Enter Name", "TEMP", wxOK|wxCANCEL|wxCENTRE, wxDefaultPosition);
	 if (NewMaterialDialog->ShowModal() == wxID_OK)
	 {
			text = (char*)NewMaterialDialog->GetValue().mb_str(wxConvUTF8);

			if (material = CREATE(Graphical_material)(text))
			{
				 if(MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)
						(material,material_editor->edit_material))
				 {
						material_copy_bump_mapping_and_per_pixel_lighting_flag(material_editor->edit_material,
							 material);
						ADD_OBJECT_TO_MANAGER(Graphical_material)(
							 material, material_editor->graphical_material_manager);
						make_current_material(material_editor, material);
						material_editor_wx_set_material(material_editor,material);
				 }
			}
	 }
	 delete NewMaterialDialog;
	 LEAVE;
}

void OnMaterialEditorDeleteMaterial(wxCommandEvent& event)
{
	 ENTER(OnMaterialEditorDeleteMaterial);

	 REMOVE_OBJECT_FROM_MANAGER(Graphical_material)(
			material_editor->current_material,material_editor->graphical_material_manager);
	 material_editor->current_material = NULL;
	 make_current_material(material_editor, NULL);
	 material_editor_wx_set_material(material_editor,material_editor->current_material);

	 LEAVE;
}

void OnMaterialEditorRenameMaterial(wxCommandEvent& event)
{
	 ENTER(OnMaterialEditorRenameMaterial);
	 //int selection;
	 char *text;
	 wxTextEntryDialog *NewMaterialDialog = new wxTextEntryDialog(this, "Enter name", 
			"Please Enter Name", graphical_material_object_listbox->get_string_selection(),
			wxOK|wxCANCEL|wxCENTRE, wxDefaultPosition);
	 if (NewMaterialDialog->ShowModal() == wxID_OK)
	 {
			text = (char*)NewMaterialDialog->GetValue().mb_str(wxConvUTF8);
			MANAGER_MODIFY_IDENTIFIER(Graphical_material, name)
				 (material_editor->current_material, text,
				 material_editor->graphical_material_manager);
	 }
	 delete NewMaterialDialog;

	 LEAVE;
}

void OnMaterialEditorAdvancedSettingsChanged(wxCommandEvent& event)
{
	 ENTER(OnMaterialEditorAdvancedSettingsChanged);
	 int return_code, bump_mapping_flag;
	 bump_mapping_flag = 0;
	 if (material_editor->material_editor_per_pixel_checkbox->IsChecked())
	 {
			if (material_editor->material_editor_second_texture_check_box->GetValue())
			{
				 material_editor->material_editor_bump_mapping_checkbox->Enable(true);
				 bump_mapping_flag =
						material_editor->material_editor_bump_mapping_checkbox->GetValue();
			}
			return_code = set_material_program_type(material_editor->edit_material,
				 /*bump_mapping_flag */ bump_mapping_flag, 
				 0, 0, 0, 0, 0, 0, 0, 1);
	 }
	 else
	 {
			material_editor->material_editor_bump_mapping_checkbox->SetValue(false);
			material_editor->material_editor_bump_mapping_checkbox->Enable(false);
			return_code = material_deaccess_material_program(material_editor->edit_material);
	 }
	 if (return_code)
			material_editor_update_picture(material_editor);

	 LEAVE;
}

void CloseMaterialEditor(wxCloseEvent &event)
{
	 ENTER(CloseMaterialEditor);
	 material_editor_wx_set_material(material_editor, material_editor->current_material);
	 DESTROY(Material_editor_dialog)(material_editor->material_editor_dialog_address);
	 LEAVE;
}

	 DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(wxMaterialEditor, wxFrame)
	 EVT_BUTTON(XRCID("MaterialCreateButton"), wxMaterialEditor::OnMaterialEditorCreateNewMaterial)
	 EVT_BUTTON(XRCID("MaterialDeleteButton"), wxMaterialEditor::OnMaterialEditorDeleteMaterial)
	 EVT_BUTTON(XRCID("MaterialRenameButton"), wxMaterialEditor::OnMaterialEditorRenameMaterial)
	 EVT_BUTTON(XRCID("MaterialCreateButton"), wxMaterialEditor::OnMaterialEditorCreateNewMaterial)
	 EVT_TEXT_ENTER(XRCID("MaterialEditorAlphaTextCtrl"),wxMaterialEditor::OnMaterialEditorAlphaTextEntered)
	 EVT_TEXT_ENTER(XRCID("MaterialEditorShininessTextCtrl"),wxMaterialEditor::OnMaterialEditorShininessTextEntered)
	 EVT_COMMAND_SCROLL(XRCID("MaterialEditorAlphaSlider"),wxMaterialEditor::OnMaterialEditorAlphaSliderChanged)
	 EVT_COMMAND_SCROLL(XRCID("MaterialEditorShininessSlider"),wxMaterialEditor::OnMaterialEditorShininessSliderChanged)
	 EVT_CHECKBOX(XRCID("MaterialEditorTextureCheckBox"), wxMaterialEditor::OnMaterialEditorTextureCheckBoxChecked)
	 EVT_CHECKBOX(XRCID("MaterialEditorSecondTextureCheckBox"), wxMaterialEditor::OnMaterialEditorSecondTextureCheckBoxChecked)
	 EVT_CHECKBOX(XRCID("MaterialEditorThirdTextureCheckBox"), wxMaterialEditor::OnMaterialEditorThirdTextureCheckBoxChecked)
	 EVT_CHECKBOX(XRCID("MaterialEditorFourthTextureCheckBox"), wxMaterialEditor::OnMaterialEditorFourthTextureCheckBoxChecked)
	 EVT_CHECKBOX(XRCID("MaterialEditorPerPixelLightingCheckBox"), wxMaterialEditor::OnMaterialEditorAdvancedSettingsChanged)
	 EVT_CHECKBOX(XRCID("MaterialEditorBumpMappingCheckBox"), wxMaterialEditor::OnMaterialEditorAdvancedSettingsChanged)
	 EVT_BUTTON(XRCID("wxMaterialApplyButton"),wxMaterialEditor::OnMaterialEditorApplyButtonPressed)
	 EVT_BUTTON(XRCID("wxMaterialOKButton"),wxMaterialEditor::OnMaterialEditorOKButtonPressed)
	 EVT_BUTTON(XRCID("wxMaterialRevertButton"),wxMaterialEditor::OnMaterialEditorRevertButtonPressed)
	 EVT_BUTTON(XRCID("wxMaterialCancelButton"),wxMaterialEditor::OnMaterialEditorCancelButtonPressed)
	 EVT_CLOSE(wxMaterialEditor::CloseMaterialEditor)
END_EVENT_TABLE()

static int make_current_material(
	struct Material_editor *material_editor,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Destroys the current material member of <material_editor> and rebuilds it as
a complete copy of <Material>.
==============================================================================*/
{
	 int return_code;
	ENTER(make_current_material);
	if (material_editor)
	{
		return_code=1;
		if (material)
		{
			if (!IS_MANAGED(Graphical_material)(material,
				material_editor->graphical_material_manager))
			{
#if defined (TEST_CODE)
				display_message(ERROR_MESSAGE,
					"make_current_material.  Material not managed");
#endif /* defined (TEST_CODE) */
				material=(struct Graphical_material *)NULL;
				return_code=0;
			}
		}
		if (!material)
		{
			material=FIRST_OBJECT_IN_MANAGER_THAT(Graphical_material)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,
				(void *)NULL,
				material_editor->graphical_material_manager);
		}
		material_editor->wx_material_editor->material_editor_graphical_material_list_set_selected(
			 material);
		material_editor->current_material=material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_dialog_set_material.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

void Material_editor_material_change(
	 struct MANAGER_MESSAGE(Graphical_material) *message, void *material_editor_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2008

DESCRIPTION :
Something has changed globally in the material manager. Update the
current material.
==============================================================================*/
{
	struct Material_editor *material_editor;

	ENTER(Materia_editor_material_change);
	if (message && (material_editor = (struct Material_editor *)material_editor_void))
	{
		switch (message->change)
		{
			 case MANAGER_CHANGE_REMOVE(Graphical_material):
			 {
					material_editor->current_material = 
						 FIRST_OBJECT_IN_MANAGER_THAT(Graphical_material)(
								NULL,(void *)NULL,	material_editor->graphical_material_manager);
					material_editor_wx_set_material(material_editor,material_editor->current_material);
			 } break;
		} 
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Materia_editor_materia_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Materia_editor_materia_change */

struct Material_editor *CREATE(Material_editor)(
	 struct Material_editor_dialog **material_editor_dialog_address,
	 struct MANAGER(Graphical_material) *graphical_material_manager,
	 struct MANAGER(Texture) *texture_manager, 
	 struct Graphical_material *material,
	 struct Graphics_buffer_package *graphics_buffer_package,
	 struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Creates a Material_editor.
==============================================================================*/
{
	 int init_widgets;
	struct Material_editor *material_editor;
	struct Graphical_material *temp_material;

	ENTER(CREATE(Material_editor));
	material_editor = (struct Material_editor *)NULL;
	if (texture_manager && user_interface)
	{
			/* allocate memory */
			if (ALLOCATE(material_editor, struct Material_editor, 1))
			{
				/* initialise the structure */
				material_editor->material_manager_callback_id=(void *)NULL;
				material_editor->background=0; /* tri-colour */
				material_editor->material_editor_dialog_address = material_editor_dialog_address;
				material_editor->texture_manager=texture_manager;
				material_editor->graphical_material_manager = graphical_material_manager;
				material_editor->graphics_buffer = (struct Graphics_buffer *)NULL;
				material_editor->current_material=(struct Graphical_material *)NULL;
				material_editor->edit_material=(struct Graphical_material *)NULL;
				material_editor->user_interface = user_interface;
				material_editor->wx_material_editor = (wxMaterialEditor *)NULL;
				material_editor->wx_material_editor = new wxMaterialEditor(material_editor);
				material_editor->material_editor_ambient_colour_panel = XRCCTRL(
					 *material_editor->wx_material_editor, "MaterialEditorPanel1", wxPanel);
				material_editor->ambient_colour_editor = new Colour_editor(
					 material_editor->material_editor_ambient_colour_panel, "Ambient Colour:", 
					 COLOUR_EDITOR_RGB, (struct Colour *)NULL, (void*)material_editor);
				material_editor->material_editor_diffuse_colour_panel = XRCCTRL(
					 *material_editor->wx_material_editor, "MaterialEditorPanel2", wxPanel);
				material_editor->diffuse_colour_editor = new Colour_editor(
					 material_editor->material_editor_diffuse_colour_panel, "Diffuse Colour:",
					 COLOUR_EDITOR_RGB, (struct Colour *)NULL, (void*)material_editor);
				material_editor->material_editor_emitted_colour_panel = XRCCTRL(
					 *material_editor->wx_material_editor, "MaterialEditorPanel3", wxPanel);
				material_editor->emitted_colour_editor = new Colour_editor(
					 material_editor->material_editor_emitted_colour_panel, "Emitted Colour:",
					 COLOUR_EDITOR_RGB, (struct Colour *)NULL, (void*)material_editor);
				material_editor->material_editor_specular_colour_panel = XRCCTRL(
					 *material_editor->wx_material_editor, "MaterialEditorPanel4", wxPanel);
				material_editor->specular_colour_editor = new Colour_editor(
					 material_editor->material_editor_specular_colour_panel, "Specular Colour:",
					 COLOUR_EDITOR_RGB, (struct Colour *)NULL, (void*)material_editor);
				material_editor->material_editor_sample_panel = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorSamplePanel", wxPanel);
				material_editor->material_editor_alpha_slider = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorAlphaSlider", wxSlider);
					 material_editor->material_editor_shininess_slider = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorShininessSlider", wxSlider);
				material_editor->material_editor_alpha_text_ctrl = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorAlphaTextCtrl", wxTextCtrl);
				material_editor->material_editor_shininess_text_ctrl = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorShininessTextCtrl", wxTextCtrl);
				material_editor->material_editor_texture_check_box = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorTextureCheckBox", wxCheckBox);
				material_editor->material_editor_second_texture_check_box = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorSecondTextureCheckBox", wxCheckBox);
				material_editor->material_editor_third_texture_check_box = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorThirdTextureCheckBox", wxCheckBox);
				material_editor->material_editor_fourth_texture_check_box = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorFourthTextureCheckBox", wxCheckBox);
				material_editor->material_editor_per_pixel_checkbox = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorPerPixelLightingCheckBox", wxCheckBox);
				material_editor->material_editor_bump_mapping_checkbox = XRCCTRL(*material_editor->wx_material_editor
					 , "MaterialEditorBumpMappingCheckBox", wxCheckBox);
				temp_material = material;
				if (!temp_material)
				{
					 temp_material=FIRST_OBJECT_IN_MANAGER_THAT(Graphical_material)(
							(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,(void *)NULL,
							material_editor->graphical_material_manager);
				}
				make_current_material(material_editor, temp_material);
				material_editor->material_manager_callback_id = 
					 MANAGER_REGISTER(Graphical_material)(
					 Material_editor_material_change, (void *)material_editor, 
					 material_editor->graphical_material_manager);
				/* now bring up a 3d drawing widget */
				init_widgets = 1;
				if (!(material_editor->graphics_buffer=
							create_Graphics_buffer_wx(graphics_buffer_package,
								 material_editor->material_editor_sample_panel,
								 GRAPHICS_BUFFER_ANY_BUFFERING_MODE, 
								 GRAPHICS_BUFFER_ANY_STEREO_MODE,
								 /*minimum_colour_buffer_depth*/8, /*minimum_depth_buffer_depth*/8,
								 /*minimum_accumulation_buffer_depth*/0, (struct Graphics_buffer *)NULL)))
				{
					 display_message(ERROR_MESSAGE,
							"CREATE(Material_editor).  Could not create 3d widget.");
					 init_widgets=0;
				}
				material_editor->wx_material_editor->Show();
				material_editor->wx_material_editor->Fit();
				material_editor->material_editor_texture_chooser_panel->SetSize(-1,100);
				if (init_widgets)
				{
					 Graphics_buffer_awaken(material_editor->graphics_buffer);
					 material_editor_wx_set_material(material_editor, temp_material);
					 Graphics_buffer_add_expose_callback(material_editor->graphics_buffer,
							material_editor_expose_picture_callback, (void *)material_editor);
					 Graphics_buffer_add_input_callback(material_editor->graphics_buffer,
							material_editor_change_background, (void *)material_editor);
// #if defined (MATERIAL_EDITOR_SET_NAME)
// 					 /* set the name of the material_editor */
// 					 material_editor_set_name(material_editor);
// #endif /* defined (MATERIAL_EDITOR_SET_NAME) */
				}

			}
			else
			{
				 display_message(ERROR_MESSAGE,"CREATE(Material_editor).  "
						"Could not allocate material_editor widget structure");
			}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Material_editor).  Invalid argument(s)");
	}
	LEAVE;

	return (material_editor);
} /* CREATE(Material_editor) */

int DESTROY(Material_editor)(struct Material_editor **material_editor_address)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*material_editor_address> and sets
<*material_editor_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Material_editor *material_editor;

	ENTER(DESTROY(Material_editor));
	if (material_editor_address &&
		(material_editor = *material_editor_address))
	{
		delete material_editor->wx_material_editor;
		if (material_editor->edit_material)
		{
			DESTROY(Graphical_material)(&(material_editor->edit_material));
		}
		if (material_editor->graphics_buffer)
		{
			DESTROY(Graphics_buffer)(&(material_editor->graphics_buffer));
		}
		if (material_editor->material_manager_callback_id)
		{
			 MANAGER_DEREGISTER(Graphical_material)(
					material_editor->material_manager_callback_id,
					material_editor->graphical_material_manager);
			 material_editor->material_manager_callback_id = (void *)NULL;

		}
		DEALLOCATE(*material_editor_address);
		*material_editor_address = (struct Material_editor *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Material_editor).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Material_editor) */

int material_editor_wx_set_material(
	struct Material_editor *material_editor, struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 6 November 2007

DESCRIPTION :
Sets the <material> to be edited by the <material_editor>.
==============================================================================*/
{
	 Colour temp_colour;
	 int return_code, texture_set, per_pixel_set;
	 MATERIAL_PRECISION alpha,shininess;
	 struct Texture *texture;

	ENTER(material_editor_wx_set_material);
	if (material_editor)
	{
		return_code=1;
		if (material_editor->edit_material)
		{
			DESTROY(Graphical_material)(&(material_editor->edit_material));
		}
		if (material)
		{
			/* create a copy for editing */
			if ((material_editor->edit_material=
				CREATE(Graphical_material)("copy"))&&
				MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)
				(material_editor->edit_material,material))
			{
				 material_copy_bump_mapping_and_per_pixel_lighting_flag(material,
						material_editor->edit_material);
#if defined (MATERIAL_EDITOR_NAME)
				/* set the name of the material_editor */
				material_editor_set_name(material_editor);
#endif /* defined (MATERIAL_EDITOR_NAME) */
				/* now make all the sub widgets reflect the new data */
				if (Graphical_material_get_ambient(
					material_editor->edit_material,&temp_colour))
				{
					 material_editor->ambient_colour_editor->colour_editor_wx_set_colour(
							&temp_colour);
				}
				if (Graphical_material_get_diffuse(
					material_editor->edit_material,&temp_colour))
				{
					 material_editor->diffuse_colour_editor->colour_editor_wx_set_colour(
							&temp_colour);
				}
				if (Graphical_material_get_emission(
					material_editor->edit_material,&temp_colour))
				{
					 material_editor->emitted_colour_editor->colour_editor_wx_set_colour(
							&temp_colour);
				}
				if (Graphical_material_get_specular(
					material_editor->edit_material,&temp_colour))
				{
					 material_editor->specular_colour_editor->colour_editor_wx_set_colour(
							&temp_colour);
				}
				if (Graphical_material_get_alpha(material_editor->edit_material,
					&alpha))
				{
					 material_editor_wx_set_textctrl_and_slider(material_editor->material_editor_alpha_text_ctrl,
							material_editor->material_editor_alpha_slider,
							alpha);
				}
				if (Graphical_material_get_shininess(
					material_editor->edit_material,&shininess))
				{
					 material_editor_wx_set_textctrl_and_slider(material_editor->material_editor_shininess_text_ctrl,
							material_editor->material_editor_shininess_slider,
							shininess);	 
				}
				if (texture=Graphical_material_get_texture(material_editor->edit_material))
				{
					 material_editor->wx_material_editor->material_editor_set_texture_object(texture);
					 texture_set=1;
				}
				else
				{
					 texture_set=0;
				}
				material_editor->material_editor_texture_check_box->SetValue(texture_set);
				material_editor->material_editor_texture_chooser_panel->Enable(texture_set);

				if (texture=Graphical_material_get_second_texture(material_editor->edit_material))
				{
					 material_editor->wx_material_editor->material_editor_set_second_texture_object(texture);
					 texture_set=1;
				}
				else
				{
					 texture_set=0;
				}
				material_editor->material_editor_second_texture_check_box->SetValue(texture_set);
				material_editor->material_editor_second_texture_chooser_panel->Enable(texture_set);

				if (texture=Graphical_material_get_third_texture(material_editor->edit_material))
				{
					 material_editor->wx_material_editor->material_editor_set_third_texture_object(texture);
					 texture_set=1;
				}
				else
				{
					 texture_set=0;
				}
				material_editor->material_editor_third_texture_check_box->SetValue(texture_set);
				material_editor->material_editor_third_texture_chooser_panel->Enable(texture_set);

				if (texture=Graphical_material_get_fourth_texture(material_editor->edit_material))
				{
					 material_editor->wx_material_editor->material_editor_set_fourth_texture_object(texture);
					 texture_set=1;
				}
				else
				{
					 texture_set=0;
				}
				material_editor->material_editor_fourth_texture_check_box->SetValue(texture_set);
				material_editor->material_editor_fourth_texture_chooser_panel->Enable(texture_set);

				per_pixel_set = Graphical_material_get_per_pixel_lighting_flag(material_editor->edit_material);
				if (per_pixel_set)
				{
					 material_editor->material_editor_per_pixel_checkbox->SetValue(true);
					 if (material_editor->material_editor_second_texture_check_box->GetValue())
					 {
							material_editor->material_editor_bump_mapping_checkbox->Enable(true);
							material_editor->material_editor_bump_mapping_checkbox->SetValue(
								 Graphical_material_get_bump_mapping_flag(material_editor->edit_material));
					 }
					 else
					 {
							material_editor->material_editor_bump_mapping_checkbox->Enable(false);
							material_editor->material_editor_bump_mapping_checkbox->SetValue(false);
					 }
				}
				else
				{
					 material_editor->material_editor_per_pixel_checkbox->SetValue(false);
					 material_editor->material_editor_bump_mapping_checkbox->SetValue(false);
					 material_editor->material_editor_bump_mapping_checkbox->Enable(false);
				}

 				/* need to check window is there the first time else error occurs */
				if (Graphics_buffer_is_visible(material_editor->graphics_buffer))
				{
					material_editor_update_picture(material_editor);
				}
			}
			else
			{
				if (material_editor->edit_material)
				{
					DESTROY(Graphical_material)(&(material_editor->edit_material));
				}
				display_message(ERROR_MESSAGE,
					"material_editor_wx_set_material.  Could not make copy of material");
				material=(struct Graphical_material *)NULL;
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_wx_set_material.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_wx_set_material */

void material_editor_update_colour(void *material_editor_void)
/*******************************************************************************
LAST MODIFIED : 30 November 2007

DESCRIPTION :
Update the material colour and settings in the material editor .
==============================================================================*/
{
	 ENTER(material_editor_update_colour);

	 Material_editor *material_editor;
	 Colour temp_colour;

	 if (material_editor=(struct Material_editor *)material_editor_void)
	 {
			temp_colour = material_editor->ambient_colour_editor->colour_editor_wx_get_colour();
			Graphical_material_set_ambient(material_editor->edit_material, &temp_colour);
			temp_colour = material_editor->diffuse_colour_editor->colour_editor_wx_get_colour();
			Graphical_material_set_diffuse(material_editor->edit_material, &temp_colour);
			temp_colour = material_editor->emitted_colour_editor->colour_editor_wx_get_colour();
			Graphical_material_set_emission(material_editor->edit_material, &temp_colour);
			temp_colour = material_editor->specular_colour_editor->colour_editor_wx_get_colour();
			Graphical_material_set_specular(material_editor->edit_material, &temp_colour);
			material_editor_update_picture(material_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "material_editor_update_colour.  Invalid argument(s)");
	 }
	 LEAVE;
}
