/*******************************************************************************
FILE : material_editor.c

LAST MODIFIED : 18 April 2000

DESCRIPTION :
==============================================================================*/
#include <math.h>
#define PI 3.1415927
#define PI_180 (PI/180.0)
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "choose/choose_texture.h"
#include "colour/colour_editor.h"
#include "colour/edit_var.h"
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "material/material_editor.h"
#include "material/material_editor.uidh"
#include "three_d_drawing/ThreeDDraw.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
/* UIL Identifiers */
#define material_editor_name_ID            1
#define material_editor_ambient_form_ID    2
#define material_editor_diffuse_form_ID    3
#define material_editor_emission_form_ID   4
#define material_editor_specular_form_ID   5
#define material_editor_alpha_form_ID      6
#define material_editor_shininess_form_ID  7
#define material_editor_texture_btn_ID     8
#define material_editor_texture_form_ID    9
#define material_editor_a3d_form_ID       10

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
	/* void *manager_callback_id; */
	int background;
	/* edit_material is always a local copy of what is passed to the editor */
	struct Graphical_material *edit_material;
	struct Callback_data update_callback;
	struct MANAGER(Texture) *texture_manager;
	Widget alpha_form,alpha_widget,ambient_form,ambient_widget,diffuse_form,
		diffuse_widget,emission_form,emission_widget,shininess_form,
		shininess_widget,specular_form,specular_widget,texture_button,texture_form,
		texture_widget,widget_parent,widget,a3d_form,a3d_widget;
#if defined (MATERIAL_EDITOR_NAME)
	Widget name;
#endif
	Widget *widget_address;
}; /* Material_editor */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int material_editor_hierarchy_open=0;
static MrmHierarchy material_editor_hierarchy;
#endif /* defined (MOTIF) */
#if defined (GL_API)
static Matrix Identity=
{
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};
#endif /* defined (GL_API) */

/*
Module functions
----------------
*/
static void material_editor_update(struct Material_editor *material_editor)
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the material.
==============================================================================*/
{
	ENTER(material_editor_update);
	/* checking arguments */
	if (material_editor)
	{
		if (material_editor->update_callback.procedure)
		{
			/* Inform the client that edit_material has been modified */
			(material_editor->update_callback.procedure)
				(material_editor->widget,
					material_editor->update_callback.data,
					material_editor->edit_material);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update */

static void material_editor_draw_sphere(Widget w,XtPointer tag,XtPointer reason)
/*******************************************************************************
LAST MODIFIED : 5 April 2000

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
	int i,j;
	struct Material_editor *material_editor=
		(struct Material_editor *)tag;
#if defined (OPENGL_API)
	float texture_height,texture_width;
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
	USE_PARAMETER(w);
	USE_PARAMETER(reason);
	/* make sure the Graphical material display list is up-to-date, which
		 in turn, requires any textures it uses to be compiled */
	compile_Graphical_material(material_editor->edit_material,NULL);
#if defined (OPENGL_API)
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
		/* no textures on the RGB background */
		execute_Texture((struct Texture *)NULL);
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
		Texture_get_physical_size(texture,&texture_width,&texture_height);
		horiz_factor=2.0*texture_width/sphere_horiz;
		horiz_offset=-0.5*texture_width;
		vert_factor=2.0*texture_height/sphere_vert;
		vert_offset=-0.5*texture_height;
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
#endif /* defined (OPENGL_API) */
	LEAVE;
} /* material_editor_draw_sphere */

static void material_editor_update_picture(
	struct Material_editor *material_editor)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Updates the picture with the changed material.
==============================================================================*/
{
	ENTER(material_editor_update_picture);
	/* checking arguments */
	if (material_editor)
	{
		X3dThreeDDrawingMakeCurrent(material_editor->a3d_widget);
		material_editor_draw_sphere(material_editor->a3d_widget,
			(XtPointer)material_editor,(XtPointer)NULL);
		X3dThreeDDrawingSwapBuffers();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update_picture.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update_picture */

static void material_editor_change_background(Widget w,XtPointer tag,
	XtPointer reason)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Increments the background pattern.
==============================================================================*/
{
	struct Material_editor
		*material_editor=(struct Material_editor *)tag;
	X3dThreeDDrawCallbackStruct *callback;

	ENTER(material_editor_change_background);
	USE_PARAMETER(w);
	if (callback=(X3dThreeDDrawCallbackStruct *)reason)
	{
		if (X3dCR_INPUT==callback->reason)
		{
			if ((callback->event)&&(ButtonPress==callback->event->type))
			{
				(material_editor->background)++;
				if (material_editor->background>2)
				{
					material_editor->background=0;
				}
				material_editor_update_picture(material_editor);
			}
		}
	}
	LEAVE;
} /* material_editor_change_background */

static void material_editor_update_ambient(Widget colour_widget,
	void *material_editor_void,void *colour_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Gets the current value of ambient colour.
==============================================================================*/
{
	struct Colour *colour;
	struct Material_editor *material_editor;

	ENTER(material_editor_update_ambient);
	USE_PARAMETER(colour_widget);
	if ((material_editor=(struct Material_editor *)material_editor_void)&&
		(colour=(struct Colour *)colour_void))
	{
		Graphical_material_set_ambient(material_editor->edit_material,colour);
		material_editor_update_picture(material_editor);
		material_editor_update(material_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update_ambient.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update_ambient */

static void material_editor_update_diffuse(Widget colour_widget,
	void *material_editor_void,void *colour_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Gets the current value of diffuse colour.
==============================================================================*/
{
	struct Colour *colour;
	struct Material_editor *material_editor;

	ENTER(material_editor_update_diffuse);
	USE_PARAMETER(colour_widget);
	if ((material_editor=(struct Material_editor *)material_editor_void)&&
		(colour=(struct Colour *)colour_void))
	{
		Graphical_material_set_diffuse(material_editor->edit_material,colour);
		material_editor_update_picture(material_editor);
		material_editor_update(material_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update_diffuse.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update_diffuse */

static void material_editor_update_emission(Widget colour_widget,
	void *material_editor_void,void *colour_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Gets the current value of emitted colour.
==============================================================================*/
{
	struct Colour *colour;
	struct Material_editor *material_editor;

	ENTER(material_editor_update_emission);
	USE_PARAMETER(colour_widget);
	if ((material_editor=(struct Material_editor *)material_editor_void)&&
		(colour=(struct Colour *)colour_void))
	{
		Graphical_material_set_emission(material_editor->edit_material,colour);
		material_editor_update_picture(material_editor);
		material_editor_update(material_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update_emission.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update_emission */

static void material_editor_update_specular(Widget colour_widget,
	void *material_editor_void,void *colour_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Gets the current value of specular colour.
==============================================================================*/
{
	struct Colour *colour;
	struct Material_editor *material_editor;

	ENTER(material_editor_update_specular);
	USE_PARAMETER(colour_widget);
	if ((material_editor=(struct Material_editor *)material_editor_void)&&
		(colour=(struct Colour *)colour_void))
	{
		Graphical_material_set_specular(material_editor->edit_material,colour);
		material_editor_update_picture(material_editor);
		material_editor_update(material_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update_specular.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update_specular */

static void material_editor_update_alpha(Widget colour_widget,
	void *material_editor_void,void *alpha_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Gets the current value of alpha.
==============================================================================*/
{
	double *alpha;
	struct Material_editor *material_editor;

	ENTER(material_editor_update_alpha);
	USE_PARAMETER(colour_widget);
	if ((material_editor=(struct Material_editor *)material_editor_void)&&
		(alpha=(double *)alpha_void))
	{
		Graphical_material_set_alpha(material_editor->edit_material,
			(MATERIAL_PRECISION)*alpha);
		material_editor_update_picture(material_editor);
		material_editor_update(material_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update_alpha. Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update_alpha */

static void material_editor_update_shininess(Widget colour_widget,
	void *material_editor_void,void *shininess_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Gets the current value of shininess.
==============================================================================*/
{
	double *shininess;
	struct Material_editor *material_editor;

	ENTER(material_editor_update_shininess);
	USE_PARAMETER(colour_widget);
	if ((material_editor=(struct Material_editor *)material_editor_void)&&
		(shininess=(double *)shininess_void))
	{
		Graphical_material_set_shininess(material_editor->edit_material,
			(MATERIAL_PRECISION)*shininess);
		material_editor_update_picture(material_editor);
		material_editor_update(material_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update_shininess.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update_shininess */

static void material_editor_texture_button_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Called when the texture field toggle button value changes.
==============================================================================*/
{
	int texture_set;
	struct Graphical_material *edit_material;
	struct Material_editor *material_editor;
	struct Texture *texture;

	ENTER(material_editor_texture_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if ((material_editor=(struct Material_editor *)client_data)
		&&(edit_material=material_editor->edit_material))
	{
		if (Graphical_material_get_texture(edit_material))
		{
			texture=(struct Texture *)NULL;
		}
		else
		{
			/* get texture field from the widget */
			texture=CHOOSE_OBJECT_GET_OBJECT(Texture)(
				material_editor->texture_widget);
		}
		Graphical_material_set_texture(edit_material,texture);
		texture_set=((struct Texture *)NULL != texture);
		/* (un)gray texture widget */
		XtVaSetValues(material_editor->texture_button,
			XmNset,(XtPointer)texture_set,NULL);
		XtSetSensitive(material_editor->texture_widget,texture_set);
		material_editor_update_picture(material_editor);
		material_editor_update(material_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_texture_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_texture_button_CB */

static void material_editor_update_texture(Widget widget,
	void *material_editor_void,void *texture_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Callback for change of texture.
==============================================================================*/
{
	struct Material_editor *material_editor;

	ENTER(material_editor_update_texture);
	USE_PARAMETER(widget);
	if (material_editor=(struct Material_editor *)material_editor_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(material_editor->texture_widget))
		{
			Graphical_material_set_texture(material_editor->edit_material,
				(struct Texture *)texture_void);
			material_editor_update_picture(material_editor);
			material_editor_update(material_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update_texture.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update_texture */

static void material_editor_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the material_editor widget.
==============================================================================*/
{
	struct Material_editor *material_editor;

	ENTER(material_editor_identify_button);
	USE_PARAMETER(reason);
	/* find out which material_editor widget we are in */
	XtVaGetValues(w,XmNuserData,&material_editor,NULL);
	switch (button_num)
	{
#if defined (MATERIAL_EDITOR_NAME)
		case material_editor_name_ID:
		{
			material_editor->name=w;
		} break;
#endif /* defined (MATERIAL_EDITOR_NAME) */
		case material_editor_alpha_form_ID:
		{
			material_editor->alpha_form=w;
		} break;
		case material_editor_ambient_form_ID:
		{
			material_editor->ambient_form=w;
		} break;
		case material_editor_diffuse_form_ID:
		{
			material_editor->diffuse_form=w;
		} break;
		case material_editor_emission_form_ID:
		{
			material_editor->emission_form=w;
		} break;
		case material_editor_shininess_form_ID:
		{
			material_editor->shininess_form=w;
		} break;
		case material_editor_specular_form_ID:
		{
			material_editor->specular_form=w;
		} break;
		case material_editor_texture_btn_ID:
		{
			material_editor->texture_button=w;
		} break;
		case material_editor_texture_form_ID:
		{
			material_editor->texture_form=w;
		} break;
		case material_editor_a3d_form_ID:
		{
			material_editor->a3d_form=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"material_editor_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* material_editor_identify_button */

static void material_editor_destroy_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 23 September 1996

DESCRIPTION :
Callback for the material_editor dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Material_editor *material_editor;

	ENTER(material_editor_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the data for the material_editor widget */
		XtVaGetValues(widget,XmNuserData,&material_editor,NULL);
		if (material_editor)
		{
			if (material_editor->edit_material)
			{
				DESTROY(Graphical_material)(&(material_editor->edit_material));
			}
			*(material_editor->widget_address)=(Widget)NULL;
			/* MANAGER_DEREGISTER(Graphical_material)(
				material_editor->manager_callback_id,
				material_editor->graphical_material_manager); */
			DEALLOCATE(material_editor);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"material_editor_destroy_CB.  Missing material_editor");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* material_editor_destroy_CB */

#if defined (MATERIAL_EDITOR_NAME)
static void material_editor_set_name(
	struct Material_editor *material_editor)
/*******************************************************************************
LAST MODIFIED : 1 May 1994

DESCRIPTION :
Writes the correct name on the label.
==============================================================================*/
{
	XmString temp_label;

	ENTER(material_editor_set_name);
	/* checking arguments */
	if (material_editor)
	{
		temp_label=
			XmStringCreateSimple((material_editor->edit_material)->name);
		XtVaSetValues(material_editor->name,XmNlabelString,temp_label,NULL);
		XmStringFree(temp_label);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_set_name.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_set_name */
#endif /* defined (MATERIAL_EDITOR_NAME) */

#if defined (OLD_CODE)
static void material_editor_global_object_change(
	struct MANAGER_MESSAGE(Graphical_material) *message,void *data)
/*******************************************************************************
LAST MODIFIED : 16 February 1997

DESCRIPTION :
Something has changed globally about the objects this widget uses, so refresh.
==============================================================================*/
{
	struct Material_editor *material_editor=data;

	ENTER(material_editor_global_object_change);
	/* checking arguments */
	if (message)
	{
#if defined (OLD_CODE)
/*???DB.  Is type needed */
		if (message->type==MANAGER_Graphical_material)
		{
#endif /* defined (OLD_CODE) */
			switch (message->change)
			{
				case MANAGER_CHANGE_ALL(Graphical_material):
				{
					if (IS_MANAGED(Graphical_material)(
						material_editor->global_value,
						material_editor->graphical_material_manager))
					{
						material_editor_set_data(material_editor->widget,
							MATERIAL_EDITOR_DATA,material_editor->global_value);
					}
					else
					{
						material_editor_set_data(material_editor->widget,
							MATERIAL_EDITOR_DATA,FIRST_OBJECT_IN_MANAGER_THAT(
							Graphical_material)(
							(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,
							(void *)NULL,material_editor->graphical_material_manager));
					}
				} break;
				case MANAGER_CHANGE_DELETE(Graphical_material):
				{
					if (message->object_changed==material_editor->global_value)
					{
						material_editor_set_data(material_editor->widget,
							MATERIAL_EDITOR_DATA,FIRST_OBJECT_IN_MANAGER_THAT(
							Graphical_material)(
							(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,
							(void *)NULL,material_editor->graphical_material_manager));
					}
				} break;
				case MANAGER_CHANGE_ADD(Graphical_material):
				{
					if (NULL==material_editor->global_value)
					{
						material_editor_set_data(material_editor->widget,
							MATERIAL_EDITOR_DATA,message->object_changed);
					}
				}; break;
				case MANAGER_CHANGE_IDENTIFIER(Graphical_material):
				case MANAGER_CHANGE_OBJECT(Graphical_material):
				case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Graphical_material):
				{
					if (message->object_changed==material_editor->global_value)
					{
						material_editor_set_data(material_editor->widget,
							MATERIAL_EDITOR_DATA,message->object_changed);
					}
				} break;
			}
#if defined (OLD_CODE)
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_global_object_change.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_global_object_change */
#endif /* defined (OLD_CODE) */

/*
Global functions
----------------
*/
Widget create_material_editor_widget(Widget *material_editor_widget,
	Widget parent,struct MANAGER(Texture) *texture_manager,
	struct Graphical_material *material,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 18 April 2000

DESCRIPTION :
Creates a material_editor widget.
==============================================================================*/
{
	int init_widgets;
	MrmType material_editor_dialog_class;
	struct Callback_data callback;
	struct Material_editor *material_editor=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"mat_editor_identify_button",(XtPointer)material_editor_identify_button},
		{"mat_editor_destroy_CB",(XtPointer)material_editor_destroy_CB},
		{"mat_editor_texture_btn_CB",(XtPointer)
			material_editor_texture_button_CB},
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"mat_editor_structure",(XtPointer)NULL},
#if defined (MATERIAL_EDITOR_NAME)
		{"material_editor_name_ID",(XtPointer)material_editor_name_ID},
#endif /* defined (MATERIAL_EDITOR_NAME) */
		{"mat_editor_ambient_form_ID",
		 (XtPointer)material_editor_ambient_form_ID},
		{"mat_editor_diffuse_form_ID",
		 (XtPointer)material_editor_diffuse_form_ID},
		{"mat_editor_emission_form_ID",
		 (XtPointer)material_editor_emission_form_ID},
		{"mat_editor_specular_form_ID",
		 (XtPointer)material_editor_specular_form_ID},
		{"mat_editor_alpha_form_ID",
		 (XtPointer)material_editor_alpha_form_ID},
		{"mat_editor_shininess_form_ID",
		 (XtPointer)material_editor_shininess_form_ID},
		{"mat_editor_texture_btn_ID",
		 (XtPointer)material_editor_texture_btn_ID},
		{"mat_editor_texture_form_ID",
		 (XtPointer)material_editor_texture_form_ID},
		{"mat_editor_a3d_form_ID",
		 (XtPointer)material_editor_a3d_form_ID}
	};
	Widget return_widget;

	ENTER(create_material_editor_widget);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (material_editor_widget&&parent&&texture_manager&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(material_editor_uidh,
			&material_editor_hierarchy,&material_editor_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(material_editor,struct Material_editor,1))
			{
				/* initialise the structure */
				/* material_editor->manager_callback_id=(void *)NULL; */
				material_editor->widget_parent=parent;
				material_editor->widget_address=material_editor_widget;
				material_editor->widget=(Widget)NULL;
				material_editor->background=0; /* tri-colour */
				material_editor->texture_manager=texture_manager;
#if defined (MATERIAL_EDITOR_NAME)
				material_editor->name=(Widget)NULL;
#endif /* defined (MATERIAL_EDITOR_NAME) */
				material_editor->alpha_form=(Widget)NULL;
				material_editor->alpha_widget=(Widget)NULL;
				material_editor->ambient_form=(Widget)NULL;
				material_editor->ambient_widget=(Widget)NULL;
				material_editor->diffuse_form=(Widget)NULL;
				material_editor->diffuse_widget=(Widget)NULL;
				material_editor->emission_form=(Widget)NULL;
				material_editor->emission_widget=(Widget)NULL;
				material_editor->shininess_form=(Widget)NULL;
				material_editor->shininess_widget=(Widget)NULL;
				material_editor->specular_form=(Widget)NULL;
				material_editor->specular_widget=(Widget)NULL;
				material_editor->texture_button=(Widget)NULL;
				material_editor->texture_form=(Widget)NULL;
				material_editor->texture_widget=(Widget)NULL;
				material_editor->a3d_form=(Widget)NULL;
				material_editor->a3d_widget=(Widget)NULL;
				material_editor->edit_material=(struct Graphical_material *)NULL;
				material_editor->update_callback.procedure=
					(Callback_procedure *)NULL;
				material_editor->update_callback.data=NULL;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(material_editor_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)material_editor;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(material_editor_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch position window widget */
						if (MrmSUCCESS==MrmFetchWidget(material_editor_hierarchy,
							"mat_editor_widget",material_editor->widget_parent,
							&(material_editor->widget),&material_editor_dialog_class))
						{
							/* leave managing up to material_editor_set_material */
							/* ie. as soon as we have a valid material */
							/* XtManageChild(material_editor->widget); */
							/* set the mode toggle to the correct position */
							init_widgets=1;
							if (!(material_editor->ambient_widget=
								create_colour_editor_widget(material_editor->ambient_form,
								COLOUR_EDITOR_RGB,(struct Colour *)NULL,user_interface)))
							{
								display_message(ERROR_MESSAGE,
			"create_material_editor_widget.  Could not create ambient colour widget");
								init_widgets=0;
							}
							if (!(material_editor->diffuse_widget=
								create_colour_editor_widget(material_editor->diffuse_form,
								COLOUR_EDITOR_RGB,(struct Colour *)NULL,user_interface)))
							{
								display_message(ERROR_MESSAGE,
			"create_material_editor_widget.  Could not create diffuse colour widget");
								init_widgets=0;
							}
							if (!(material_editor->emission_widget=
								create_colour_editor_widget(material_editor->emission_form,
								COLOUR_EDITOR_RGB,(struct Colour *)NULL,user_interface)))
							{
								display_message(ERROR_MESSAGE,
		"create_material_editor_widget.  Could not create emission colour widget");
								init_widgets=0;
							}
							if (!(material_editor->specular_widget=
								create_colour_editor_widget(material_editor->specular_form,
								COLOUR_EDITOR_RGB,(struct Colour *)NULL,user_interface)))
							{
								display_message(ERROR_MESSAGE,
		"create_material_editor_widget.  Could not create specular colour widget");
								init_widgets=0;
							}
							if (!(material_editor->alpha_widget=
								create_edit_var_widget(material_editor->alpha_form,
								"Alpha",0.0,0.0,1.0)))
							{
								display_message(ERROR_MESSAGE,
							"create_material_editor_widget.  Could not create alpha widget");
								init_widgets=0;
							}
							if (!(material_editor->shininess_widget=
								create_edit_var_widget(material_editor->shininess_form,
								"Shininess",0.0,0.0,1.0)))
							{
								display_message(ERROR_MESSAGE,
					"create_material_editor_widget.  Could not create shininess widget");
								init_widgets=0;
							}
							if (!(material_editor->texture_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Texture)(
								material_editor->texture_form,
								(struct Texture *)NULL,texture_manager,
								(MANAGER_CONDITIONAL_FUNCTION(Texture) *)NULL,(void *)NULL)))
							{
								init_widgets=0;
							}
							/* now bring up a 3d drawing widget */
							if (material_editor->a3d_widget=XtVaCreateWidget(
								"a3d_widget",threeDDrawingWidgetClass,
								material_editor->a3d_form,
								XmNwidth,100,
								XmNheight,100,
								XmNbottomAttachment,XmATTACH_FORM,
								XmNleftAttachment,XmATTACH_FORM,
								XmNrightAttachment,XmATTACH_FORM,
								XmNtopAttachment,XmATTACH_FORM,
								X3dNbufferingMode,X3dDOUBLE_BUFFERING,
								X3dNbufferColourMode,X3dCOLOUR_RGB_MODE,
								NULL))
							{
								XtManageChild(material_editor->a3d_widget);
							}
							else
							{
								display_message(ERROR_MESSAGE,
								"create_material_editor_widget.  Could not create 3d widget.");
								init_widgets=0;
							}
							if (init_widgets)
							{
								material_editor_set_material(material_editor->widget,
									material);
								/*???RC should do following in ~set_material */
								/* add a callback to the 3d widget */
								XtAddCallback(material_editor->a3d_widget,
									X3dNexposeCallback,material_editor_draw_sphere,
									material_editor);
								XtAddCallback(material_editor->a3d_widget,
									X3dNinputCallback,material_editor_change_background,
									material_editor);
								/* set callbacks for colour and edit_var editors: */
								callback.data=material_editor;
								callback.procedure=material_editor_update_ambient;
								colour_editor_set_callback(
									material_editor->ambient_widget,&callback);
								callback.procedure=material_editor_update_diffuse;
								colour_editor_set_callback(
									material_editor->diffuse_widget,&callback);
								callback.procedure=material_editor_update_emission;
								colour_editor_set_callback(
									material_editor->emission_widget,&callback);
								callback.procedure=material_editor_update_specular;
								colour_editor_set_callback(
									material_editor->specular_widget,&callback);
								callback.procedure=material_editor_update_alpha;
								edit_var_set_callback(
									material_editor->alpha_widget,&callback);
								callback.procedure=material_editor_update_shininess;
								edit_var_set_callback(
									material_editor->shininess_widget,&callback);
								callback.procedure=material_editor_update_texture;
								CHOOSE_OBJECT_SET_CALLBACK(Texture)(
									material_editor->texture_widget,&callback);
#if defined (MATERIAL_EDITOR_SET_NAME)
								/* set the name of the material_editor */
								material_editor_set_name(material_editor);
#endif /* defined (MATERIAL_EDITOR_SET_NAME) */
								/* register for any changes */
								/* material_editor->manager_callback_id=
									MANAGER_REGISTER(Graphical_material)(
									material_editor_global_object_change,material_editor,
									graphical_material_manager); */
								/* if (material_editor->global_value==
									material_editor->default_value)
								{
									XtUnmanageChild(material_editor->widget);
								} */
								return_widget=material_editor->widget;
							}
							else
							{
								DEALLOCATE(material_editor);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
			"create_material_editor_widget.  Could not fetch material_editor dialog");
							DEALLOCATE(material_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_material_editor_widget.  Could not register identifiers");
						DEALLOCATE(material_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_material_editor_widget.  Could not register callbacks");
					DEALLOCATE(material_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"create_material_editor_widget.  "
					"Could not allocate material_editor widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_material_editor_widget.  Could not open hierarchy");
		}
		*material_editor_widget=return_widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_material_editor_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_material_editor_widget */

int material_editor_get_callback(Widget material_editor_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Returns the update_callback for the material editor widget.
==============================================================================*/
{
	int return_code;
	struct Material_editor *material_editor;

	ENTER(material_editor_get_callback);
	/* check arguments */
	if (material_editor_widget&&callback)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(material_editor_widget,
			XmNuserData,&material_editor,NULL);
		if (material_editor)
		{
			callback->procedure=material_editor->update_callback.procedure;
			callback->data=material_editor->update_callback.data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"material_editor_get_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_get_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_get_callback */

int material_editor_set_callback(Widget material_editor_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Changes the update_callback for the material editor widget.
==============================================================================*/
{
	int return_code;
	struct Material_editor *material_editor=NULL;

	ENTER(material_editor_set_callback);
	if (material_editor_widget&&callback)
	{
		/* Get the pointer to the data for the material_editor dialog */
		XtVaGetValues(material_editor_widget,
			XmNuserData,&material_editor,NULL);
		if (material_editor)
		{
			material_editor->update_callback.procedure=callback->procedure;
			material_editor->update_callback.data=callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"material_editor_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_set_callback */

struct Graphical_material *material_editor_get_material(
	Widget material_editor_widget)
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Returns the address of the material being edited in the material_editor widget.
Do not modify or DEALLOCATE the returned material; copy it to another material.
==============================================================================*/
{
	struct Graphical_material *return_material;
	struct Material_editor *material_editor;

	ENTER(material_editor_set_material);
	if (material_editor_widget)
	{
		/* Get the pointer to the data for the material_editor dialog */
		XtVaGetValues(material_editor_widget,
			XmNuserData,&material_editor,NULL);
		if (material_editor)
		{
			return_material=material_editor->edit_material;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"material_editor_get_material.  Missing widget data.");
			return_material=(struct Graphical_material *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_get_material.  Missing widget.");
		return_material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (return_material);
} /* material_editor_get_material */

int material_editor_set_material(Widget material_editor_widget,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Changes the material in the material_editor widget.
==============================================================================*/
{
	int return_code,texture_set;
	MATERIAL_PRECISION alpha,shininess;
	struct Colour temp_colour;
	struct Material_editor *material_editor;
	struct Texture *texture;

	ENTER(material_editor_set_material);
	if (material_editor_widget)
	{
		/* Get the pointer to the data for the material_editor dialog */
		XtVaGetValues(material_editor_widget,
			XmNuserData,&material_editor,NULL);
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
#if defined (MATERIAL_EDITOR_NAME)
					/* set the name of the material_editor */
					material_editor_set_name(material_editor);
#endif /* defined (MATERIAL_EDITOR_NAME) */
					/* now make all the sub widgets reflect the new data */
					if (Graphical_material_get_ambient(
						material_editor->edit_material,&temp_colour))
					{
						colour_editor_set_colour(material_editor->ambient_widget,
							&temp_colour);
					}
					if (Graphical_material_get_diffuse(
						material_editor->edit_material,&temp_colour))
					{
						colour_editor_set_colour(material_editor->diffuse_widget,
							&temp_colour);
					}
					if (Graphical_material_get_emission(
						material_editor->edit_material,&temp_colour))
					{
						colour_editor_set_colour(material_editor->emission_widget,
							&temp_colour);
					}
					if (Graphical_material_get_specular(
						material_editor->edit_material,&temp_colour))
					{
						colour_editor_set_colour(material_editor->specular_widget,
							&temp_colour);
					}
					if (Graphical_material_get_alpha(material_editor->edit_material,
						&alpha))
					{
						edit_var_set_data(material_editor->alpha_widget,
							EDIT_VAR_VALUE,(EDIT_VAR_PRECISION)alpha);
					}
					if (Graphical_material_get_shininess(
						material_editor->edit_material,&shininess))
					{
						edit_var_set_data(material_editor->shininess_widget,
							EDIT_VAR_VALUE,(EDIT_VAR_PRECISION)shininess);
					}
					if (texture=
						Graphical_material_get_texture(material_editor->edit_material))
					{
						CHOOSE_OBJECT_SET_OBJECT(Texture)(
							material_editor->texture_widget,texture);
						texture_set=True;
					}
					else
					{
						texture_set=False;
					}
					XtVaSetValues(material_editor->texture_button,
						XmNset,(XtPointer)texture_set,NULL);
					XtSetSensitive(material_editor->texture_widget,texture_set);
					/* need to check window is there the first time else error occurs */
					if (XtWindow(material_editor->a3d_widget))
					{
						material_editor_update_picture(material_editor);
					}
					XtManageChild(material_editor->widget);
				}
				else
				{
					if (material_editor->edit_material)
					{
						DESTROY(Graphical_material)(&(material_editor->edit_material));
					}
					display_message(ERROR_MESSAGE,
						"material_editor_set_material.  Could not make copy of material");
					material=(struct Graphical_material *)NULL;
					return_code=0;
				}
			}
			if (!material)
			{
				XtUnmanageChild(material_editor->widget);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"material_editor_set_material.  Missing widget data.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_set_material.  Invalid argument(s).");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* material_editor_set_material */
