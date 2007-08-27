/*******************************************************************************
FILE : spectrum_editor_wx.cpp

LAST MODIFIED : 22 Aug 2007

DESCRIPTION:
Provides the wxWidgets interface to manipulate spectrum settings.
==============================================================================*/

extern "C" {
#include <stdio.h>
#include <math.h>
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_settings.h"
#include "graphics/spectrum_editor_wx.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/* SAB Trying to hide the guts of GT_object and its primitives,
	however the spectrum editor is modifying it's primitives quite a bit . */
#include "graphics/graphics_object_private.h"
}
#include "wx/wx.h"
#include <wx/splitter.h>
#include "wx/xrc/xmlres.h"
#include "graphics/spectrum_editor_wx.xrch"
#include "icon/cmiss_icon.xpm"

class wxSpectrumEditor;

struct Spectrum_editor
/*******************************************************************************
LAST MODIFIED : 22 August 2007

DESCRIPTION :
Contains all the information carried by the graphical element editor widget.
==============================================================================*/
{
	/* This editor_material is used when displaying the spectrum in the 
		3d widget */
	 struct Graphical_material *editor_material, *tick_material;
	 struct Spectrum_settings *current_settings;
	 struct Spectrum *edit_spectrum;
	 struct MANAGER(Spectrum) *spectrum_manager;
	 struct User_interface *user_interface;
	 void *material_manager_callback_id;
	 void *spectrum_manager_callback_id;
// 	 struct Callback_data update_callback;
	 struct Scene *spectrum_editor_scene;
	 struct Scene_viewer *spectrum_editor_scene_viewer;
	 struct GT_object *graphics_object, *tick_lines_graphics_object,
			*tick_labels_graphics_object;
	 int viewer_type;
	 wxSpectrumEditor *wx_spectrum_editor;
	 wxPanel *panel;
	 wxCheckListBox *spectrum_editor_check_list;
	 wxFrame *spectrum_editor_frame;
}; /* spectrum_editor */

static int make_edit_spectrum(
	struct Spectrum_editor *spectrum_editor,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Destroys the edit_spectrum member of <spectrum_editor> and rebuilds it as
a complete copy of <Spectrum>.
==============================================================================*/
{
	int return_code;

	ENTER(make_edit_spectrum);
	/* check arguments */
	if (spectrum_editor&&spectrum)
	{
		/* destroy current edit_spectrum */
		if (spectrum_editor->edit_spectrum)
		{
			DEACCESS(Spectrum)(&(spectrum_editor->edit_spectrum));
		}
		/* make an empty spectrum */
		if (spectrum_editor->edit_spectrum=ACCESS(Spectrum)(CREATE(Spectrum)("copy")))
		{
			/* copy general settings into new object */
			MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)
				(spectrum_editor->edit_spectrum,spectrum);

// 			XmToggleButtonGadgetSetState(spectrum_editor->opaque_button,
// 				Spectrum_get_opaque_colour_flag(spectrum_editor->edit_spectrum),
// 				False);

			set_GT_object_Spectrum(spectrum_editor->graphics_object,
				(void *)spectrum_editor->edit_spectrum);

			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"make_edit_spectrum.  Could not make copy of spectrum");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_edit_spectrum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* make_edit_spectrum */

class wxSpectrumEditor : public wxFrame
{
	 Spectrum_editor *spectrum_editor;

public:

	 wxSpectrumEditor(Spectrum_editor *spectrum_editor):
			spectrum_editor(spectrum_editor)
	 {
	 };

	 wxSpectrumEditor()
	 {
	 };


	 DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(wxSpectrumEditor, wxFrame)
END_EVENT_TABLE()

static int spectrum_editor_update_scene_viewer(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
==============================================================================*/
{
	char **strings, *string_data;
	const float extend_ends = 0.04;
	struct Colour black ={0, 0, 0}, white = {1.0, 1.0, 1.0};
	int i, j, npts1, npts2, number_of_points,
		tick_label_count, tick_line_count, return_code;
	float bar_min, bar_max, min, max, value_xi1;
	GTDATA *data;
	Triple *line_points, *label_points;
	struct GT_surface *surface;
	struct GT_polyline *tick_lines;
	struct GT_pointset *tick_labels;

	ENTER(spectrum_editor_update_scene_viewer);
	/* check arguments */
	if (!spectrum_editor->edit_spectrum)
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_scene_viewer1.  Invalid argument(s)");
	}
	if (!spectrum_editor->graphics_object)
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_scene_viewer2.  Invalid argument(s)");
	}
	if (!spectrum_editor->tick_lines_graphics_object)
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_scene_viewer3.  Invalid argument(s)");
	}
	if (!spectrum_editor->tick_labels_graphics_object)
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_scene_viewer4.  Invalid argument(s)");
	}
	if (spectrum_editor && spectrum_editor->edit_spectrum 
		&& spectrum_editor->graphics_object
		&& spectrum_editor->tick_lines_graphics_object
		 && spectrum_editor->tick_labels_graphics_object)
	{
		surface = GT_OBJECT_GET(GT_surface)(spectrum_editor->graphics_object, 0);
		tick_lines = GT_OBJECT_GET(GT_polyline)(spectrum_editor->tick_lines_graphics_object, 0);
		tick_labels = GT_OBJECT_GET(GT_pointset)(spectrum_editor->tick_labels_graphics_object, 0);
		data = surface->data;
		npts1 = surface->n_pts1;
		npts2 = surface->n_pts2;
		strings = tick_labels->text;
		label_points = tick_labels->pointlist;
		tick_label_count = tick_labels->n_pts;
		line_points = tick_lines->pointlist;
		tick_line_count = tick_lines->n_pts;

		switch (spectrum_editor->viewer_type % 6)
		{
			case 0:
			{
				Graphical_material_set_ambient(spectrum_editor->editor_material, &black );
				Graphical_material_set_diffuse(spectrum_editor->editor_material, &black );
				number_of_points = 5;
			} break;
			case 1:
			{
				number_of_points = 11;
			} break;
			case 2:
			{
				number_of_points = 2;
			} break;
			case 3:
			{
				Graphical_material_set_ambient(spectrum_editor->editor_material, &white );
				Graphical_material_set_diffuse(spectrum_editor->editor_material, &white );
				number_of_points = 5;
			} break;
			case 4:
			{
				number_of_points = 11;
			} break;
			case 5:
			{
				number_of_points = 2;
			} break;
		}
		if ( tick_label_count != number_of_points 
			|| tick_line_count != number_of_points )
		{
			if ( strings )
			{
				string_data = strings[0];
			}
			else
			{
				string_data = (char *)NULL;
			}
			if (REALLOCATE(line_points, line_points, Triple, 2 * number_of_points)
				&& REALLOCATE(label_points, label_points, Triple, number_of_points)
				&& REALLOCATE(string_data, string_data, char, number_of_points * 15)
				&& REALLOCATE(strings, strings, char *, number_of_points))
			{
				tick_line_count = number_of_points;
				tick_label_count = number_of_points;
				for ( i = 0 ; i < number_of_points ; i++ )
				{
					value_xi1 = (-5.0 + 10.0 * (float) i / 
						(float)(number_of_points - 1))
						/ (1. + 2.0 * extend_ends);
					label_points[i][0] = value_xi1;
					label_points[i][1] = 0;
					label_points[i][2] = -1.0;
					strings[i] = string_data + 15 * i;
					/* the strings will be set below */
				}
				i = 0;
				while( i < 2 * number_of_points )
				{
					value_xi1 = (-5.0 + 10.0 * (float) i / 
						(float)(2 * number_of_points - 2))
						/ (1. + 2.0 * extend_ends);
					line_points[i][0] = value_xi1;
					line_points[i][1] = 0;
					line_points[i][2] = -0.5;
					i++;
					line_points[i][0] = value_xi1;
					line_points[i][1] = 0;
					line_points[i][2] = -1.0;
					i++;
				}
				tick_labels->text = strings;
				tick_labels->pointlist = label_points;
				tick_labels->n_pts = tick_label_count;
				tick_lines->pointlist = line_points;
				tick_lines->n_pts = tick_line_count;
			}

			GT_object_changed(spectrum_editor->tick_lines_graphics_object);
		}
		min = get_Spectrum_minimum(spectrum_editor->edit_spectrum);
		max = get_Spectrum_maximum(spectrum_editor->edit_spectrum);

		for ( i = 0 ; i < tick_label_count ; i++ )
		{
			sprintf(strings[i], "%6.2g", min + (max - min)
				* (float)i / (float)(tick_label_count - 1));
		}

		bar_min = min - extend_ends * (max - min);
		bar_max = max + extend_ends * (max - min);
		for ( i = 0 ; i < npts2 ; i++ )
		{
			for ( j = 0 ; j < npts1 ; j++ )
			{
				*data = bar_min  + (bar_max - bar_min)
					* (float)j / (float)(npts1 - 1);
				data++;
			}
		}
		GT_object_changed(spectrum_editor->graphics_object);
		GT_object_changed(spectrum_editor->tick_labels_graphics_object);
		Scene_viewer_redraw(spectrum_editor->spectrum_editor_scene_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_scene_viewer.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_update_scene_viewer */

static void spectrum_editor_viewer_input_callback(
	struct Scene_viewer *scene_viewer, struct Graphics_buffer_input *input,
	void *spectrum_editor_void)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Callback for when input is received by the scene_viewer.
==============================================================================*/
{
	struct Spectrum_editor *spectrum_editor;

	ENTER(spectrum_editor_viewer_input_CB);
	USE_PARAMETER(scene_viewer);
	if (spectrum_editor=(struct Spectrum_editor *)spectrum_editor_void)
	{
		if (GRAPHICS_BUFFER_BUTTON_PRESS==input->type)
		{
			/* Increment the type */
			spectrum_editor->viewer_type++;
			spectrum_editor_update_scene_viewer(spectrum_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_viewer_input_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_viewer_input_CB */

struct Spectrum_editor *CREATE(Spectrum_editor)(
   struct Spectrum *spectrum,
	 struct Graphics_font *font,
	 struct Graphics_buffer_package *graphics_buffer_package,
	 struct User_interface *user_interface,
	 struct LIST(GT_object) *glyph_list,
	 struct MANAGER(Graphical_material) *graphical_material_manager,
	 struct MANAGER(Light) *light_manager,
	 struct MANAGER(Spectrum) *spectrum_manager,
	 struct MANAGER(Texture) *texture_manager)
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
Creates a spectrum_editor widget.
==============================================================================*/
{
	 char *name;
	 int i,j,return_code,surface_discretise_xi1=24,surface_discretise_xi2=108;
	 GTDATA *data;
	 struct Spectrum_editor *spectrum_editor;
	 struct Colour background_colour = {0.1, 0.1, 0.1},
			ambient_colour = {0.2, 0.2, 0.2}, black ={0, 0, 0},
			off_white = {0.9, 0.8, 0.8};
	 struct Graphics_buffer *graphics_buffer;
	 struct Light *viewer_light;
	 struct Light_model *viewer_light_model;
	 Triple *points, *normalpoints;
	 float value_xi1, value_xi2, light_direction[3] = {0, -0.2, -1.0};
	 struct GT_surface *cylinder_surface;
	 struct GT_polyline *tick_lines;
	 struct GT_pointset *tick_labels;
	 struct Graphical_material *tick_material;
	 char **strings, *string_data;
	 struct Spectrum *default_scene_spectrum;

	ENTER(CREATE(Spectrum_editor));
	spectrum_editor = (struct Spectrum_editor *)NULL;
	if (user_interface)
	{
			/* allocate memory */
			if (ALLOCATE(spectrum_editor,struct Spectrum_editor,1))
			{
				/* initialise the structure */
				spectrum_editor->current_settings = (struct Spectrum_settings *)NULL;
				spectrum_editor->edit_spectrum=(struct Spectrum *)NULL;
				spectrum_editor->user_interface=user_interface;
				spectrum_editor->material_manager_callback_id=(void *)NULL;
				spectrum_editor->spectrum_manager_callback_id=(void *)NULL;
// 				spectrum_editor->update_callback.procedure=(Callback_procedure *)NULL;
// 				spectrum_editor->update_callback.data=(void *)NULL;
				spectrum_editor->editor_material = (struct Graphical_material *)NULL;
				spectrum_editor->tick_material = (struct Graphical_material *)NULL;
				spectrum_editor->graphics_object = (struct GT_object *)NULL;
				spectrum_editor->tick_lines_graphics_object = (struct GT_object *)NULL;
				spectrum_editor->tick_labels_graphics_object = (struct GT_object *)NULL;
				spectrum_editor->spectrum_editor_scene = (struct Scene *)NULL;
				spectrum_editor->spectrum_editor_scene_viewer = (struct Scene_viewer *)NULL;
				spectrum_editor->viewer_type = 0;
				// User_Interface
				spectrum_editor->wx_spectrum_editor = (wxSpectrumEditor *)NULL;
				spectrum_editor->wx_spectrum_editor = new wxSpectrumEditor(spectrum_editor);
				wxXmlInit_spectrum_editor_wx();
				wxXmlResource::Get()->LoadFrame(spectrum_editor->wx_spectrum_editor,
					 (wxWindow *)NULL, _T("CmguiSpectrumEditor"));
				spectrum_editor->wx_spectrum_editor->SetIcon(cmiss_icon_xpm);
				spectrum_editor->panel = XRCCTRL(*spectrum_editor->wx_spectrum_editor
					 , "Panel", wxPanel);
				spectrum_editor->panel->SetSize(wxDefaultCoord,wxDefaultCoord,
					 400, 150);
				spectrum_editor->panel->SetMinSize(wxSize(-1,150));

				spectrum_editor->spectrum_editor_check_list = XRCCTRL(*spectrum_editor->wx_spectrum_editor
					 , "SpectrumCheckList", wxCheckListBox);
				spectrum_editor->spectrum_editor_check_list->SetSize(wxDefaultCoord,wxDefaultCoord,
					 400, 150);
				spectrum_editor->spectrum_editor_check_list->SetMinSize(wxSize(-1,150));

				spectrum_editor->spectrum_editor_frame = XRCCTRL(*spectrum_editor->wx_spectrum_editor
					 , "CmguiSpectrumEditor", wxFrame);
				spectrum_editor->spectrum_editor_frame->SetSize(wxDefaultCoord,wxDefaultCoord,
					 400, 400);
				spectrum_editor->spectrum_editor_frame->SetMinSize(wxSize(1,1));

				if (spectrum_editor->wx_spectrum_editor != NULL)
				{
					 return_code = 1;
					 spectrum_editor->editor_material = ACCESS(Graphical_material)
							(CREATE(Graphical_material)("editor_material"));
					 Graphical_material_set_ambient(spectrum_editor->editor_material, &black );
					 Graphical_material_set_diffuse(spectrum_editor->editor_material, &black );
					 Graphical_material_set_shininess(spectrum_editor->editor_material, 0.8 );
					 tick_material = CREATE(Graphical_material)("editor_material");
					 spectrum_editor->tick_material = ACCESS(Graphical_material)(
							tick_material);
					 Graphical_material_set_ambient(tick_material, &off_white );
					 Graphical_material_set_diffuse(tick_material, &off_white );
					 Graphical_material_set_shininess(tick_material, 0.8 );
					 if ( ALLOCATE( points, Triple, surface_discretise_xi1 *
								 surface_discretise_xi2) &&
							ALLOCATE( normalpoints, Triple, surface_discretise_xi1 *
								 surface_discretise_xi2) &&
							ALLOCATE( data, GTDATA, surface_discretise_xi1 *
								 surface_discretise_xi2 ) )
					 {
							for ( i = 0 ; i < surface_discretise_xi1 ; i++ )
							{
								 value_xi1 = sin ( (float) i * 2.0 * PI / (float)(surface_discretise_xi1 - 1));
								 value_xi2 = cos ( (float) i * 2.0 * PI / (float)(surface_discretise_xi1 - 1));
								 for ( j = 0 ; j < surface_discretise_xi2 ; j++ )
								 {
										points[i * surface_discretise_xi2 + j][0] = -5.0 + 10.0 * (float) j / 
											 (float)(surface_discretise_xi2 - 1);
										points[i * surface_discretise_xi2 + j][1] = value_xi1;
										points[i * surface_discretise_xi2 + j][2] = 0.5 + value_xi2;
										/* Normals */
										normalpoints[i * surface_discretise_xi2 + j][0] = 0;
										normalpoints[i * surface_discretise_xi2 + j][1] = value_xi1;
										normalpoints[i * surface_discretise_xi2 + j][2] = value_xi2;
										/* Spectrum */
										data[i * surface_discretise_xi2 + j] = (float) j / 
											 (float)(surface_discretise_xi2 - 1);
								 }
							}
							if ((spectrum_editor->graphics_object =
										CREATE(GT_object)("spectrum_editor_surface",g_SURFACE,
											 spectrum_editor->editor_material)))
							{
								 ACCESS(GT_object)(spectrum_editor->graphics_object);
								 if (cylinder_surface=CREATE(GT_surface)(
												g_SHADED_TEXMAP, g_QUADRILATERAL,
												surface_discretise_xi2, surface_discretise_xi1,
												points, normalpoints, /*tangentpoints*/(Triple *)NULL,
												/*texturepoints*/(Triple *)NULL,
												/* n_data_components */1, data))
								 {
										GT_OBJECT_ADD(GT_surface)(
											 spectrum_editor->graphics_object, 0,
											 cylinder_surface);
								 }
								 else
								 {
										DEALLOCATE( points );
										DEALLOCATE( data );
										return_code = 0;
										display_message(ERROR_MESSAGE,
											 "CREATE(Spectrum_editor). Unable to create surface");
								 }
							}
							else
							{
								 DEALLOCATE( points );
								 DEALLOCATE( data );
								 return_code = 0;
								 display_message(ERROR_MESSAGE,
										"CREATE(Spectrum_editor). Unable to create graphics_object");
							}
					 }
					 if ( return_code )
					 {
							points = (Triple *)NULL;
							if ((spectrum_editor->tick_lines_graphics_object=
										CREATE(GT_object)("spectrum_editor_tick_lines",g_POLYLINE,
											 tick_material)))
							{
								 GT_object_set_next_object(spectrum_editor->graphics_object,
										spectrum_editor->tick_lines_graphics_object);
								 if (tick_lines = CREATE(GT_polyline)(
												g_PLAIN_DISCONTINUOUS, /*line_width=default*/0,
												0, points, /* normalpoints */(Triple *)NULL,
												g_NO_DATA, (GTDATA *)NULL))
								 {
										GT_OBJECT_ADD(GT_polyline)(
											 spectrum_editor->tick_lines_graphics_object, 0,
											 tick_lines);
								 }
								 else
								 {
										return_code = 0;
										display_message(ERROR_MESSAGE,
											 "CREATE(Spectrum_editor). Unable to create lines");
								 }
							}
							else
							{
								 return_code = 0;
								 display_message(ERROR_MESSAGE,
										"CREATE(Spectrum_editor). Unable to create tick line graphics_object");
							}
					 }
					 if ( return_code
							&& ALLOCATE( points, Triple, 1) &&
							ALLOCATE( strings, char *, 1) &&
							ALLOCATE( string_data, char, 1))
					 {
							points[0][0] = 0;
							points[0][1] = 0;
							points[0][2] = 0;
							strings[0] = string_data;
							string_data[0] = 0;
							if ((spectrum_editor->tick_labels_graphics_object =
										CREATE(GT_object)("spectrum_editor_tick_labels",
											 g_POINTSET,tick_material)))
							{
								 GT_object_set_next_object(spectrum_editor->tick_lines_graphics_object,
										spectrum_editor->tick_labels_graphics_object);
								 if (tick_labels = CREATE(GT_pointset)(1,
											 points, strings, g_NO_MARKER, 0.0,
											 g_NO_DATA, (GTDATA *)NULL, (int *)NULL, font))
								 {
										GT_OBJECT_ADD(GT_pointset)(
											 spectrum_editor->tick_labels_graphics_object, 0,
											 tick_labels);
								 }
								 else
								 {
										return_code = 0;
										display_message(ERROR_MESSAGE,
											 "CREATE(Spectrum_editor). Unable to create tick label pointset");
								 }
							}
							else
							{
								 return_code = 0;
								 display_message(ERROR_MESSAGE,
										"CREATE(Spectrum_editor). Unable to create tick label graphics_object");
							}
					 }
					 if ( return_code )
					 {
							default_scene_spectrum = CREATE(Spectrum)("default_scene_spectrum");
							spectrum_editor->spectrum_editor_scene = CREATE(Scene)("spectrum_editor_scene");
							Scene_enable_graphics( spectrum_editor->spectrum_editor_scene,
								 glyph_list, graphical_material_manager, 
								 spectrum_editor->editor_material, font, light_manager,
								 spectrum_manager, default_scene_spectrum, 
								 texture_manager);
							viewer_light = CREATE(Light)("spectrum_editor_light");
							set_Light_direction(viewer_light, light_direction);
							viewer_light_model = CREATE(Light_model)("spectrum_editor_light_model");
							Light_model_set_ambient(viewer_light_model, &ambient_colour);
							if (graphics_buffer = create_Graphics_buffer_wx(
										 graphics_buffer_package, spectrum_editor->panel,
										 GRAPHICS_BUFFER_ANY_BUFFERING_MODE, GRAPHICS_BUFFER_ANY_STEREO_MODE,
										 /*minimum_colour_buffer_depth*/8,
										 /*minimum_depth_buffer_depth*/8,
										 /*minimum_accumulation_buffer_depth*/0))
							{
								 spectrum_editor->spectrum_editor_scene_viewer = 
										CREATE(Scene_viewer)(graphics_buffer,
											 &background_colour,
											 (struct MANAGER(Light) *)NULL,viewer_light,
											 (struct MANAGER(Light_model) *)NULL,viewer_light_model,
											 (struct MANAGER(Scene) *)NULL,
											 spectrum_editor->spectrum_editor_scene,
											 (struct MANAGER(Texture) *)NULL,
											 user_interface );
								 GET_NAME(GT_object)(spectrum_editor->graphics_object,
										&name);
								 return_code=Scene_add_graphics_object(
										spectrum_editor->spectrum_editor_scene,
										spectrum_editor->graphics_object, 0,
										name, /*fast_changing*/0);
								 DEALLOCATE(name);
								 Scene_viewer_set_input_mode(
										spectrum_editor->spectrum_editor_scene_viewer,
										SCENE_VIEWER_NO_INPUT );
								 Scene_viewer_add_input_callback(
										spectrum_editor->spectrum_editor_scene_viewer,
										spectrum_editor_viewer_input_callback,
										(void *)spectrum_editor);
								 Scene_viewer_set_viewport_size(
										spectrum_editor->spectrum_editor_scene_viewer,400,150);
								 Scene_viewer_set_lookat_parameters(
										spectrum_editor->spectrum_editor_scene_viewer,0,-1,0,0,0,
										0,0,0,1);
								 Scene_viewer_set_view_simple(
										spectrum_editor->spectrum_editor_scene_viewer,0,0,0,2.3,
										46,10);
								 Scene_viewer_redraw(
										spectrum_editor->spectrum_editor_scene_viewer);
							}
					 }
					 if (spectrum)
					 {
							spectrum_editor_set_spectrum(spectrum_editor, spectrum);
					 }
				}
				spectrum_editor->wx_spectrum_editor->Show();
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"CREATE(Spectrum_editor).  "
						"Could not allocate spectrum_editor widget structure");
			}
	}
	else
	{
		 display_message(ERROR_MESSAGE,
			"CREATE(Spectrum_editor).  Invalid argument(s)");
	}
	LEAVE;

	return (spectrum_editor);
} /* CREATE(Spectrum_editor) */

int spectrum_editor_set_spectrum(
	struct Spectrum_editor *spectrum_editor, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the <spectrum> to be edited by the <spectrum_editor>.
==============================================================================*/
{
	int return_code;
// 	struct Callback_data callback;

	ENTER(spectrum_editor_set_spectrum);
	if (spectrum_editor)
	{
		if (spectrum)
		{
			if (make_edit_spectrum(spectrum_editor,spectrum))
			{
				/* continue with the current_settings_type */
				 //			spectrum_editor_make_settings_list(spectrum_editor);
				
// 				XtManageChild(spectrum_editor->widget);
				/* select the first settings item in the list (if any) */
				spectrum_editor->current_settings=(struct Spectrum_settings *)NULL;
				//				spectrum_editor_select_settings_item(spectrum_editor);
				/* turn on callbacks from settings editor */
// 				callback.procedure=spectrum_editor_update_settings;
// 				callback.data=(void *)spectrum_editor;
// 				spectrum_editor_settings_set_callback(
// 					spectrum_editor->settings_widget, &callback);
			}
			else
			{
				spectrum=(struct Spectrum *)NULL;
			}
		}
		if (!spectrum)
		{
			/* turn off settings editor by passing NULL settings */
			spectrum_editor->current_settings=(struct Spectrum_settings *)NULL;
// 			spectrum_editor_settings_set_settings(spectrum_editor->settings_widget,
// 				spectrum_editor->current_settings);
// 			XtUnmanageChild(spectrum_editor->widget);
			/* turn off callbacks from settings editors */
// 			callback.procedure=(Callback_procedure *)NULL;
// 			callback.data=(void *)NULL;
// 			spectrum_editor_settings_set_callback(spectrum_editor->settings_widget,&callback);
			if (spectrum_editor->edit_spectrum)
			{
				DESTROY(Spectrum)(&(spectrum_editor->edit_spectrum));
			}
		}
		spectrum_editor_update_scene_viewer(spectrum_editor);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_set_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_set_spectrum */

// int Spectrum_editor_wx_add_item_to_spectrum_editor_check_list(

int DESTROY(Spectrum_editor)(struct Spectrum_editor **spectrum_editor_address)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*spectrum_editor_address> and sets
<*spectrum_editor_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct GT_pointset *tick_labels;
	struct Spectrum_editor *spectrum_editor;

	ENTER(DESTROY(Spectrum_editor));
	if (spectrum_editor_address &&
		(spectrum_editor = *spectrum_editor_address))
	{
		return_code = 1;
		DEACCESS(Graphical_material)(&spectrum_editor->editor_material);
		DEACCESS(Graphical_material)(&spectrum_editor->tick_material);
		/* The strings in the labels graphics object are stored in two 
			 ALLOCATED blocks instead of ALLOCATING each string individually.
			 So I will manually DEALLOCATE the strings and set them to NULL */
		tick_labels = GT_OBJECT_GET(GT_pointset)(
			spectrum_editor->tick_labels_graphics_object, 0);
		if ( tick_labels->text )
		{
			DEALLOCATE(tick_labels->text[0]);
			DEALLOCATE(tick_labels->text);
			tick_labels->text = (char **)NULL;
		}
		/* The DEACCESS for the first graphics object automatically works
			 down the linked list chain */
		DEACCESS(GT_object)(&spectrum_editor->graphics_object);
		DESTROY(Scene_viewer)(&spectrum_editor->spectrum_editor_scene_viewer);
		delete spectrum_editor->wx_spectrum_editor;
		/* destroy edit_spectrum */
		if (spectrum_editor->edit_spectrum)
		{
			DEACCESS(Spectrum)(
				&(spectrum_editor->edit_spectrum));
		}
		DEALLOCATE(*spectrum_editor_address);
		*spectrum_editor_address = (struct Spectrum_editor *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Spectrum_editor).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Spectrum_editor) */


