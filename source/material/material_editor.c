/*******************************************************************************
FILE : material_editor.c

LAST MODIFIED : 1 December 1997

DESCRIPTION :
==============================================================================*/
#include <math.h>
#define PI 3.1415927
#define PI_180 (PI/180.0)
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "colour/colour_editor.h"
#include "colour/edit_var.h"
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "material/material_editor.h"
#include "material/material_editor.uid64"
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
#define material_editor_a3d_form_ID        8

/*
Module Types
------------
*/

struct Material_editor_struct
/*******************************************************************************
LAST MODIFIED : 1 October 1997

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
	Widget alpha_form,alpha_widget,ambient_form,ambient_widget,diffuse_form,
		diffuse_widget,emission_form,emission_widget,shininess_form,
		shininess_widget,specular_form,specular_widget,widget_parent,widget,
		a3d_form,a3d_widget;
#if defined (MATERIAL_EDITOR_NAME)
	Widget name;
#endif
	Widget *widget_address;
}; /* Material_editor_struct */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int material_editor_hierarchy_open=0;
static MrmHierarchy material_editor_hierarchy;
#endif /* defined (MOTIF) */
#if defined (GL_API) || defined (OPENGL_API)
static int material_editor_display_list=0;
#endif /* defined (GL_API) || defined (OPENGL_API) */
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
static void material_editor_update(
	struct Material_editor_struct *temp_material_editor)
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the material.
==============================================================================*/
{
	ENTER(material_editor_update);
	/* checking arguments */
	if (temp_material_editor)
	{
		if (temp_material_editor->update_callback.procedure)
		{
			/* Inform the client that edit_material has been modified */
			(temp_material_editor->update_callback.procedure)
				(temp_material_editor->widget,
					temp_material_editor->update_callback.data,
					temp_material_editor->edit_material);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update */

static void material_editor_graphics_init_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Sets up any color entries if necessary.
???GH.  I dont think we need this if we are using RGB values in all the calls.
==============================================================================*/
{
#define sphere_horiz 40
#define sphere_vert 60
#define sphere_view_dist 1.5
#define sphere_panel_size 1000
#define sphere_panel_dist 5
#define sphere_fov 90
	int i,j;
#if defined (GL_API)
	float temp,temp_vector[2][3],actual_vector[3];
#if defined (IBM)
	float actual_vector_2[3];
#endif /* defined (IBM) */
#endif /* defined (GL_API) */
#if defined (OPENGL_API)
	GLdouble temp,temp_vector[2][3],actual_vector[3];
	GLUquadricObj *quadObj;
#endif /* defined (OPENGL_API) */

	ENTER(material_editor_graphics_init_CB);
#if defined (GL_API)
	if(!material_editor_display_list)
	{
		material_editor_display_list=genobj();
		makeobj(material_editor_display_list);
		temp_vector[1][0]=1.0;
		temp_vector[1][2]=0.0;
		for(i=0;i<sphere_horiz;i++) /* this is going around the y axis */
		{
			temp_vector[0][0]=temp_vector[1][0];
			temp_vector[0][2]=temp_vector[1][2];
			temp_vector[1][0]=(float)cos((double)360.0/((double)sphere_horiz/
				(double)(i+1))*(double)PI_180);
			temp_vector[1][2]=(float)sin((float)360.0/((double)sphere_horiz/
				(double)(i+1))*(double)PI_180);
			bgntmesh();
			actual_vector[0]=0.0;
			actual_vector[2]=0.0;
			actual_vector[1]=1.0;
			n3f(actual_vector);
			v3f(actual_vector);
			/* do the first triangle as it is a special case... */
			temp=(float)sin((double)180.0/(double)(sphere_vert+2)*(double)PI_180);
			actual_vector[0]=temp_vector[1][0]*temp;
			actual_vector[2]=temp_vector[1][2]*temp;
			actual_vector[1]=(float)cos((double)180.0/(double)(sphere_vert+2)*
				(double)PI_180);
			n3f(actual_vector);
			v3f(actual_vector);
#if defined (IBM)
			/*???DB.  qstrip not defined for IBM */
			actual_vector_2[0]=temp_vector[0][0]*temp;
			actual_vector_2[1]=actual_vector[1];
			actual_vector_2[2]=temp_vector[0][2]*temp;
			n3f(actual_vector_2);
			v3f(actual_vector_2);
			endtmesh();
			/* the rest of them can be done to formulae */
			for(j=1;j<=(sphere_vert/2)+1;j++)
			{
				bgnpolygon();
				/*???DB.  Is the vertex order correct ? */
				n3f(actual_vector);
				v3f(actual_vector);
				n3f(actual_vector_2);
				v3f(actual_vector_2);
				temp=(float)sin((double)180.0/((double)(sphere_vert+2)/(double)j)*
					(double)PI_180);
				actual_vector[0]=temp_vector[0][0]*temp;
				actual_vector[2]=temp_vector[0][2]*temp;
				actual_vector[1]=(float)cos((double)180.0/((double)(sphere_vert+2)/
					(double)j)*(double)PI_180);
				actual_vector_2[0]=temp_vector[1][0]*temp;
				actual_vector_2[1]=actual_vector[1];
				actual_vector_2[2]=temp_vector[1][2]*temp;
				n3f(actual_vector_2);
				v3f(actual_vector_2);
				n3f(actual_vector);
				v3f(actual_vector);
				endpolygon();
			}
#else
			actual_vector[0]=temp_vector[0][0]*temp;
			actual_vector[2]=temp_vector[0][2]*temp;
			n3f(actual_vector);
			v3f(actual_vector);
			endtmesh();
			bgnqstrip();
			/* the rest of them can be done to formulae */
			for(j=1;j<=(sphere_vert/2)+1;j++)
			{
				temp=(float)sin((double)180.0/((double)(sphere_vert+2)/(double)j)*
					(double)PI_180);
				actual_vector[0]=temp_vector[0][0]*temp;
				actual_vector[2]=temp_vector[0][2]*temp;
				actual_vector[1]=(float)cos((double)180.0/((double)(sphere_vert+2)/
					(double)j)*(double)PI_180);
				n3f(actual_vector);
				v3f(actual_vector);
				actual_vector[0]=temp_vector[1][0]*temp;
				actual_vector[2]=temp_vector[1][2]*temp;
				n3f(actual_vector);
				v3f(actual_vector);
			}
			endqstrip();
#endif /* defined (IBM) */
		}
		closeobj();
	}
#endif /* defined (GL_API) */
#if defined (OPENGL_API)
	if(!material_editor_display_list)
	{
		material_editor_display_list=glGenLists(1);
		if(material_editor_display_list)
		{
			glNewList(material_editor_display_list,GL_COMPILE);
			/* now do the sphere using the glu routines */
			glEnable(GL_LIGHTING);
			temp_vector[1][0]=1.0;
			temp_vector[1][2]=0.0;
			for(i=0;i<sphere_horiz;i++) /* this is going around the y axis */
			{
				temp_vector[0][0]=temp_vector[1][0];
				temp_vector[0][2]=temp_vector[1][2];
				temp_vector[1][0]=cos((double)360.0/((double)sphere_horiz/
					(double)(i+1))*(double)PI_180);
				temp_vector[1][2]=sin((double)360.0/((double)sphere_horiz/
					(double)(i+1))*(double)PI_180);
				glBegin(GL_POLYGON);
				actual_vector[0]=0.0;
				actual_vector[2]=0.0;
				actual_vector[1]=1.0;
				glNormal3dv(actual_vector);
				glVertex3dv(actual_vector);
				/* do the first triangle as it is a special case... */
				temp=sin((double)180.0/(double)(sphere_vert+2)*(double)PI_180);
				actual_vector[0]=temp_vector[1][0]*temp;
				actual_vector[2]=temp_vector[1][2]*temp;
				actual_vector[1]=cos((double)180.0/(double)(sphere_vert+2)*
					(double)PI_180);
				glNormal3dv(actual_vector);
				glVertex3dv(actual_vector);
				actual_vector[0]=temp_vector[0][0]*temp;
				actual_vector[2]=temp_vector[0][2]*temp;
				glNormal3dv(actual_vector);
				glVertex3dv(actual_vector);
				glEnd();
				glBegin(GL_QUAD_STRIP);
				/* the rest of them can be done to formulae */
				for(j=1;j<=(sphere_vert/2)+1;j++)
				{
					temp=sin((double)180.0/((double)(sphere_vert+2)/(double)j)*
						(double)PI_180);
					actual_vector[0]=temp_vector[0][0]*temp;
					actual_vector[2]=temp_vector[0][2]*temp;
					actual_vector[1]=cos((double)180.0/((double)(sphere_vert+2)/
						(double)j)*(double)PI_180);
					glNormal3dv(actual_vector);
					glVertex3dv(actual_vector);
					actual_vector[0]=temp_vector[1][0]*temp;
					actual_vector[2]=temp_vector[1][2]*temp;
					glNormal3dv(actual_vector);
					glVertex3dv(actual_vector);
				}
				glEnd();
			}
			glEndList();
		}
	}
#endif /* defined (OPENGL_API) */
	LEAVE;
} /* initialize_callback */

static void material_editor_draw_sphere(Widget w,XtPointer tag,XtPointer reason)
/*******************************************************************************
LAST MODIFIED : 1 May 1995

DESCRIPTION :
Uses gl to draw a sphere with a lighting source.
==============================================================================*/
{
	int i;
	struct Material_editor_struct *temp_material_editor=
		(struct Material_editor_struct *)tag;
#if defined (GL_API)
	float aspect,actual_vector[3];
	long xsize,ysize;
	static float light[]=
	{
		POSITION, 5.0, 4.0, 0.0, 0.0,
		LMNULL
	};
	static float light_model[]=
	{
#if defined (TWOSIDE)
		TWOSIDE, 0.0,
#endif /* defined (TWOSIDE) */
		LMNULL
	};
#endif /* defined (GL_API) */
#if defined (OPENGL_API)
	GLdouble aspect,viewport_size[4],actual_vector[3];
	GLfloat light_position[]=
	{
		5.0, 4.0, 0.0, 0.0
	};
	GLfloat light_model_twoside=1.0;
#endif /* defined (OPENGL_API) */

	ENTER(material_editor_draw_sphere);
	/* make sure the Graphical material display list is up-to-date */
	compile_Graphical_material(temp_material_editor->edit_material,NULL);
#if defined (GL_API)
	getsize(&xsize,&ysize);
	aspect=(float)xsize/(float)ysize;
	lsetdepth(getgdesc(GD_ZMIN),getgdesc(GD_ZMAX));
	zbuffer(1);
	mmode(MVIEWING);
	loadmatrix(Identity);
	/* set up the view trans */
	perspective(sphere_fov*10,aspect,0.1,20.0);
	lookat(0.0,sphere_view_dist,0.0,0.0,0.0,0.0,-900);
	/* clear the window */
	czclear(0x000000,getgdesc(GD_ZMAX));
	blendfunction(BF_SA,BF_MSA);
	/* set up the material and lights etc */
	lmdef(DEFLIGHT,1,0,light);
	lmdef(DEFLMODEL,1,0,NULL);
	lmbind(LMODEL,1);
	lmbind(LIGHT0,1);
	if (temp_material_editor->background==0)
	{
		/* draw the backing panels */
		/* yz plane */
		bgnpolygon();
		actual_vector[0]=0.0;
		actual_vector[1]=1.0;
		actual_vector[2]=0.0;
		n3f(actual_vector);
		actual_vector[0]=1.0;
		actual_vector[1]=0.0;
		actual_vector[2]=0.0;
		c3f(actual_vector);
		actual_vector[0]=0.0;
		actual_vector[1]=-sphere_panel_dist;
		actual_vector[2]=0.0;
		v3f(actual_vector);
		actual_vector[0]=sphere_panel_size;
		v3f(actual_vector);
		actual_vector[2]=sphere_panel_size*0.866;
		v3f(actual_vector);
		actual_vector[0]=-sphere_panel_size*0.5;
		v3f(actual_vector);
		endpolygon();
		/* yz plane */
		bgnpolygon();
		actual_vector[0]=0.0;
		actual_vector[1]=1.0;
		actual_vector[2]=0.0;
		c3f(actual_vector);
		actual_vector[0]=0.0;
		actual_vector[1]=-sphere_panel_dist;
		actual_vector[2]=0.0;
		v3f(actual_vector);
		actual_vector[0]=sphere_panel_size;
		v3f(actual_vector);
		actual_vector[2]=-sphere_panel_size*0.866;
		v3f(actual_vector);
		actual_vector[0]=-sphere_panel_size*0.5;
		v3f(actual_vector);
		endpolygon();
		/* yz plane */
		bgnpolygon();
		actual_vector[0]=0.0;
		actual_vector[1]=0.0;
		actual_vector[2]=1.0;
		c3f(actual_vector);
		actual_vector[0]=0.0;
		actual_vector[1]=-sphere_panel_dist;
		actual_vector[2]=0.0;
		v3f(actual_vector);
		actual_vector[0]=-sphere_panel_size*0.5;
		actual_vector[2]=-sphere_panel_size*0.866;
		v3f(actual_vector);
		actual_vector[0]=-sphere_panel_size;
		actual_vector[1]=-sphere_panel_dist;
		actual_vector[2]=0.0;
		v3f(actual_vector);
		actual_vector[0]=-sphere_panel_size*0.5;
		actual_vector[2]=sphere_panel_size*0.866;
		v3f(actual_vector);
		endpolygon();
	}
	else
	{
		switch(temp_material_editor->background)
		{
			case 1:
			{
				for (i=0;i<3;i++)
				{
					actual_vector[i]=0.0;
				}
			} break;
			case 2:
			{
				for (i=0;i<3;i++)
				{
					actual_vector[i]=1.0;
				}
			} break;
		}
		c3f(actual_vector);
		clear();
	}
	execute_Graphical_material(temp_material_editor->edit_material);
	callobj(material_editor_display_list);
#endif /* defined (GL_API) */
#if defined (OPENGL_API)
	glGetDoublev(GL_VIEWPORT,viewport_size);
	aspect=viewport_size[2]/viewport_size[3];
	glClearColor(0.0,0.0,0.0,0.0);
	glClearDepth(1.0);
	glEnable(GL_BLEND);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	/* set up the view trans */
	gluPerspective(sphere_fov,aspect,0.1,20.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0,sphere_view_dist,0.0,0.0,0.0,0.0,1.0,0.0,0.0);
	/* clear the window */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	/* set up the material and lights etc */
	glLightfv(GL_LIGHT0,GL_POSITION,light_position);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,light_model_twoside);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	if (temp_material_editor->background==0)
	{
		glDisable(GL_LIGHTING);
		/* draw the backing panels */
		/* yz plane */
		glBegin(GL_POLYGON);
		actual_vector[0]=0.0;
		actual_vector[1]=1.0;
		actual_vector[2]=0.0;
		glNormal3dv(actual_vector);
		actual_vector[0]=1.0;
		actual_vector[1]=0.0;
		actual_vector[2]=0.0;
		glColor3dv(actual_vector);
		actual_vector[0]=0.0;
		actual_vector[1]=-sphere_panel_dist;
		actual_vector[2]=0.0;
		glVertex3dv(actual_vector);
		actual_vector[0]=sphere_panel_size;
		glVertex3dv(actual_vector);
		actual_vector[2]=sphere_panel_size*0.866;
		glVertex3dv(actual_vector);
		actual_vector[0]=-sphere_panel_size*0.5;
		glVertex3dv(actual_vector);
		glEnd();
		/* yz plane */
		glBegin(GL_POLYGON);
		actual_vector[0]=0.0;
		actual_vector[1]=1.0;
		actual_vector[2]=0.0;
		glColor3dv(actual_vector);
		actual_vector[0]=0.0;
		actual_vector[1]=-sphere_panel_dist;
		actual_vector[2]=0.0;
		glVertex3dv(actual_vector);
		actual_vector[0]=sphere_panel_size;
		glVertex3dv(actual_vector);
		actual_vector[2]=-sphere_panel_size*0.866;
		glVertex3dv(actual_vector);
		actual_vector[0]=-sphere_panel_size*0.5;
		glVertex3dv(actual_vector);
		glEnd();
		/* yz plane */
		glBegin(GL_POLYGON);
		actual_vector[0]=0.0;
		actual_vector[1]=0.0;
		actual_vector[2]=1.0;
		glColor3dv(actual_vector);
		actual_vector[0]=0.0;
		actual_vector[1]=-sphere_panel_dist;
		actual_vector[2]=0.0;
		glVertex3dv(actual_vector);
		actual_vector[0]=-sphere_panel_size*0.5;
		actual_vector[2]=-sphere_panel_size*0.866;
		glVertex3dv(actual_vector);
		actual_vector[0]=-sphere_panel_size;
		actual_vector[1]=-sphere_panel_dist;
		actual_vector[2]=0.0;
		glVertex3dv(actual_vector);
		actual_vector[0]=-sphere_panel_size*0.5;
		actual_vector[2]=sphere_panel_size*0.866;
		glVertex3dv(actual_vector);
		glEnd();
	}
	else
	{
		switch(temp_material_editor->background)
		{
			case 1:
			{
				for (i=0;i<3;i++)
				{
					actual_vector[i]=0.0;
				}
			} break;
			case 2:
			{
				for (i=0;i<3;i++)
				{
					actual_vector[i]=1.0;
				}
			} break;
		}
		glClearColor(actual_vector[0],actual_vector[1],actual_vector[2],1.0);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	execute_Graphical_material(temp_material_editor->edit_material);
	glCallList(material_editor_display_list);
#endif /* defined (OPENGL_API) */
	LEAVE;
} /* material_editor_draw_sphere */

static void material_editor_update_picture(
	struct Material_editor_struct *temp_material_editor)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Updates the picture with the changed material.
==============================================================================*/
{
	ENTER(material_editor_update_picture);
	/* checking arguments */
	if (temp_material_editor)
	{
		X3dThreeDDrawingMakeCurrent(temp_material_editor->a3d_widget);
		material_editor_draw_sphere(temp_material_editor->a3d_widget,
			(XtPointer)temp_material_editor,(XtPointer)NULL);
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
	struct Material_editor_struct
		*temp_material_editor=(struct Material_editor_struct *)tag;
	X3dThreeDDrawCallbackStruct *callback;

	ENTER(material_editor_change_background);
	if (callback=(X3dThreeDDrawCallbackStruct *)reason)
	{
		if (X3dCR_INPUT==callback->reason)
		{
			if ((callback->event)&&(ButtonPress==callback->event->type))
			{
				(temp_material_editor->background)++;
				if (temp_material_editor->background>2)
				{
					temp_material_editor->background=0;
				}
				material_editor_update_picture(temp_material_editor);
			}
		}
	}
	LEAVE;
} /* material_editor_change_background */

static void material_editor_update_ambient(Widget colour_widget,
	void *material_editor_void,void *colour_void)
/*******************************************************************************
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Gets the current value of ambient colour.
==============================================================================*/
{
	struct Colour *colour;
	struct Material_editor_struct *material_editor;

	ENTER(material_editor_update_ambient);
	if ((material_editor=(struct Material_editor_struct *)material_editor_void)&&
		(colour=(struct Colour *)colour_void))
	{
		Graphical_material_set_ambient(material_editor->edit_material,colour);
		material_editor_update_picture(material_editor);
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
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Gets the current value of diffuse colour.
==============================================================================*/
{
	struct Colour *colour;
	struct Material_editor_struct *material_editor;

	ENTER(material_editor_update_diffuse);
	if ((material_editor=(struct Material_editor_struct *)material_editor_void)&&
		(colour=(struct Colour *)colour_void))
	{
		Graphical_material_set_diffuse(material_editor->edit_material,colour);
		material_editor_update_picture(material_editor);
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
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Gets the current value of emitted colour.
==============================================================================*/
{
	struct Colour *colour;
	struct Material_editor_struct *material_editor;

	ENTER(material_editor_update_emission);
	if ((material_editor=(struct Material_editor_struct *)material_editor_void)&&
		(colour=(struct Colour *)colour_void))
	{
		Graphical_material_set_emission(material_editor->edit_material,colour);
		material_editor_update_picture(material_editor);
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
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Gets the current value of specular colour.
==============================================================================*/
{
	struct Colour *colour;
	struct Material_editor_struct *material_editor;

	ENTER(material_editor_update_specular);
	if ((material_editor=(struct Material_editor_struct *)material_editor_void)&&
		(colour=(struct Colour *)colour_void))
	{
		Graphical_material_set_specular(material_editor->edit_material,colour);
		material_editor_update_picture(material_editor);
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
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Gets the current value of alpha.
==============================================================================*/
{
	double *alpha;
	struct Material_editor_struct *material_editor;

	ENTER(material_editor_update_alpha);
	if ((material_editor=(struct Material_editor_struct *)material_editor_void)&&
		(alpha=(double *)alpha_void))
	{
		Graphical_material_set_alpha(material_editor->edit_material,
			(MATERIAL_PRECISION)*alpha);
		material_editor_update_picture(material_editor);
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
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Gets the current value of shininess.
==============================================================================*/
{
	double *shininess;
	struct Material_editor_struct *material_editor;

	ENTER(material_editor_update_shininess);
	if ((material_editor=(struct Material_editor_struct *)material_editor_void)&&
		(shininess=(double *)shininess_void))
	{
		Graphical_material_set_shininess(material_editor->edit_material,
			(MATERIAL_PRECISION)*shininess);
		material_editor_update_picture(material_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"material_editor_update_shininess.  Invalid argument(s)");
	}
	LEAVE;
} /* material_editor_update_shininess */

static void material_editor_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the material_editor widget.
==============================================================================*/
{
	struct Material_editor_struct *temp_material_editor;

	ENTER(material_editor_identify_button);
	/* find out which material_editor widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_material_editor,NULL);
	switch (button_num)
	{
#if defined (MATERIAL_EDITOR_NAME)
		case material_editor_name_ID:
		{
			temp_material_editor->name=w;
		} break;
#endif /* defined (MATERIAL_EDITOR_NAME) */
		case material_editor_alpha_form_ID:
		{
			temp_material_editor->alpha_form=w;
		} break;
		case material_editor_ambient_form_ID:
		{
			temp_material_editor->ambient_form=w;
		} break;
		case material_editor_diffuse_form_ID:
		{
			temp_material_editor->diffuse_form=w;
		} break;
		case material_editor_emission_form_ID:
		{
			temp_material_editor->emission_form=w;
		} break;
		case material_editor_shininess_form_ID:
		{
			temp_material_editor->shininess_form=w;
		} break;
		case material_editor_specular_form_ID:
		{
			temp_material_editor->specular_form=w;
		} break;
		case material_editor_a3d_form_ID:
		{
			temp_material_editor->a3d_form=w;
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
	struct Material_editor_struct *material_editor;

	ENTER(material_editor_destroy_CB);
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
	struct Material_editor_struct *temp_material_editor)
/*******************************************************************************
LAST MODIFIED : 1 May 1994

DESCRIPTION :
Writes the correct name on the label.
==============================================================================*/
{
	XmString temp_label;

	ENTER(material_editor_set_name);
	/* checking arguments */
	if (temp_material_editor)
	{
		temp_label=
			XmStringCreateSimple((temp_material_editor->edit_material)->name);
		XtVaSetValues(temp_material_editor->name,XmNlabelString,temp_label,NULL);
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
	struct Material_editor_struct *temp_material_editor=data;

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
						temp_material_editor->global_value,
						temp_material_editor->graphical_material_manager))
					{
						material_editor_set_data(temp_material_editor->widget,
							MATERIAL_EDITOR_DATA,temp_material_editor->global_value);
					}
					else
					{
						material_editor_set_data(temp_material_editor->widget,
							MATERIAL_EDITOR_DATA,FIRST_OBJECT_IN_MANAGER_THAT(
							Graphical_material)(
							(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,
							(void *)NULL,temp_material_editor->graphical_material_manager));
					}
				} break;
				case MANAGER_CHANGE_DELETE(Graphical_material):
				{
					if (message->object_changed==temp_material_editor->global_value)
					{
						material_editor_set_data(temp_material_editor->widget,
							MATERIAL_EDITOR_DATA,FIRST_OBJECT_IN_MANAGER_THAT(
							Graphical_material)(
							(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,
							(void *)NULL,temp_material_editor->graphical_material_manager));
					}
				} break;
				case MANAGER_CHANGE_ADD(Graphical_material):
				{
					if (NULL==temp_material_editor->global_value)
					{
						material_editor_set_data(temp_material_editor->widget,
							MATERIAL_EDITOR_DATA,message->object_changed);
					}
				}; break;
				case MANAGER_CHANGE_IDENTIFIER(Graphical_material):
				case MANAGER_CHANGE_OBJECT(Graphical_material):
				case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Graphical_material):
				{
					if (message->object_changed==temp_material_editor->global_value)
					{
						material_editor_set_data(temp_material_editor->widget,
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
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Creates a material_editor widget.
???RC what is the texture_manager needed for?
==============================================================================*/
{
	int i,init_widgets;
	MATERIAL_PRECISION alpha,shininess;
	MrmType material_editor_dialog_class;
	struct Callback_data callback;
	struct Material_editor_struct *temp_material_editor=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"mat_editor_identify_button",(XtPointer)material_editor_identify_button},
		{"mat_editor_destroy_CB",(XtPointer)material_editor_destroy_CB},
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
		{"mat_editor_alpha_form_ID",(XtPointer)material_editor_alpha_form_ID},
		{"mat_editor_shininess_form_ID",
		(XtPointer)material_editor_shininess_form_ID},
		{"mat_editor_a3d_form_ID",(XtPointer)material_editor_a3d_form_ID},
	};
	Widget return_widget;

	ENTER(create_material_editor_widget);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (material_editor_widget&&parent&&texture_manager&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(material_editor_uid64,
			&material_editor_hierarchy,&material_editor_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(temp_material_editor,struct Material_editor_struct,1))
			{
				/* initialise the structure */
				/* temp_material_editor->manager_callback_id=(void *)NULL; */
				temp_material_editor->widget_parent=parent;
				temp_material_editor->widget_address=material_editor_widget;
				temp_material_editor->widget=(Widget)NULL;
				temp_material_editor->background=0; /* tri-colour */
#if defined (MATERIAL_EDITOR_NAME)
				temp_material_editor->name=(Widget)NULL;
#endif /* defined (MATERIAL_EDITOR_NAME) */
				temp_material_editor->alpha_form=(Widget)NULL;
				temp_material_editor->alpha_widget=(Widget)NULL;
				temp_material_editor->ambient_form=(Widget)NULL;
				temp_material_editor->ambient_widget=(Widget)NULL;
				temp_material_editor->diffuse_form=(Widget)NULL;
				temp_material_editor->diffuse_widget=(Widget)NULL;
				temp_material_editor->emission_form=(Widget)NULL;
				temp_material_editor->emission_widget=(Widget)NULL;
				temp_material_editor->shininess_form=(Widget)NULL;
				temp_material_editor->shininess_widget=(Widget)NULL;
				temp_material_editor->specular_form=(Widget)NULL;
				temp_material_editor->specular_widget=(Widget)NULL;
				temp_material_editor->a3d_form=(Widget)NULL;
				temp_material_editor->a3d_widget=(Widget)NULL;
				temp_material_editor->edit_material=(struct Graphical_material *)NULL;
				temp_material_editor->update_callback.procedure=
					(Callback_procedure *)NULL;
				temp_material_editor->update_callback.data=NULL;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(material_editor_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)temp_material_editor;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(material_editor_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch position window widget */
						if (MrmSUCCESS==MrmFetchWidget(material_editor_hierarchy,
							"mat_editor_widget",temp_material_editor->widget_parent,
							&(temp_material_editor->widget),&material_editor_dialog_class))
						{
							/* leave managing up to material_editor_set_material */
							/* ie. as soon as we have a valid material */
							/* XtManageChild(temp_material_editor->widget); */
							/* set the mode toggle to the correct position */
							init_widgets=1;
							if (!(temp_material_editor->ambient_widget=
								create_colour_editor_widget(temp_material_editor->ambient_form,
								COLOUR_EDITOR_RGB,(struct Colour *)NULL,user_interface)))
							{
								display_message(ERROR_MESSAGE,
			"create_material_editor_widget.  Could not create ambient colour widget");
								init_widgets=0;
							}
							if (!(temp_material_editor->diffuse_widget=
								create_colour_editor_widget(temp_material_editor->diffuse_form,
								COLOUR_EDITOR_RGB,(struct Colour *)NULL,user_interface)))
							{
								display_message(ERROR_MESSAGE,
			"create_material_editor_widget.  Could not create diffuse colour widget");
								init_widgets=0;
							}
							if (!(temp_material_editor->emission_widget=
								create_colour_editor_widget(temp_material_editor->emission_form,
								COLOUR_EDITOR_RGB,(struct Colour *)NULL,user_interface)))
							{
								display_message(ERROR_MESSAGE,
		"create_material_editor_widget.  Could not create emission colour widget");
								init_widgets=0;
							}
							if (!(temp_material_editor->specular_widget=
								create_colour_editor_widget(temp_material_editor->specular_form,
								COLOUR_EDITOR_RGB,(struct Colour *)NULL,user_interface)))
							{
								display_message(ERROR_MESSAGE,
		"create_material_editor_widget.  Could not create specular colour widget");
								init_widgets=0;
							}
							if (!(temp_material_editor->alpha_widget=
								create_edit_var_widget(temp_material_editor->alpha_form,
								"Alpha",0.0,0.0,1.0)))
							{
								display_message(ERROR_MESSAGE,
							"create_material_editor_widget.  Could not create alpha widget");
								init_widgets=0;
							}
							if (!(temp_material_editor->shininess_widget=
								create_edit_var_widget(temp_material_editor->shininess_form,
								"Shininess",0.0,0.0,1.0)))
							{
								display_message(ERROR_MESSAGE,
					"create_material_editor_widget.  Could not create shininess widget");
								init_widgets=0;
							}
							/* now bring up a 3d drawing widget */
							if (temp_material_editor->a3d_widget=XtVaCreateWidget(
								"a3d_widget",threeDDrawingWidgetClass,
								temp_material_editor->a3d_form,
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
								XtManageChild(temp_material_editor->a3d_widget);
							}
							else
							{
								display_message(ERROR_MESSAGE,
								"create_material_editor_widget.  Could not create 3d widget.");
								init_widgets=0;
							}
							if (init_widgets)
							{
								material_editor_set_material(temp_material_editor->widget,
									material);
								/*???RC should do following in ~set_material */
								/* add a callback to the 3d widget */
								XtAddCallback(temp_material_editor->a3d_widget,
									X3dNinitializeCallback,material_editor_graphics_init_CB,
									temp_material_editor);
								XtAddCallback(temp_material_editor->a3d_widget,
									X3dNexposeCallback,material_editor_draw_sphere,
									temp_material_editor);
								XtAddCallback(temp_material_editor->a3d_widget,
									X3dNinputCallback,material_editor_change_background,
									temp_material_editor);
								/* set callbacks for colour and edit_var editors: */
								callback.data=temp_material_editor;
								callback.procedure=material_editor_update_ambient;
								colour_editor_set_callback(
									temp_material_editor->ambient_widget,&callback);
								callback.procedure=material_editor_update_diffuse;
								colour_editor_set_callback(
									temp_material_editor->diffuse_widget,&callback);
								callback.procedure=material_editor_update_emission;
								colour_editor_set_callback(
									temp_material_editor->emission_widget,&callback);
								callback.procedure=material_editor_update_specular;
								colour_editor_set_callback(
									temp_material_editor->specular_widget,&callback);
								callback.procedure=material_editor_update_alpha;
								edit_var_set_callback(
									temp_material_editor->alpha_widget,&callback);
								callback.procedure=material_editor_update_shininess;
								edit_var_set_callback(
									temp_material_editor->shininess_widget,&callback);
#if defined (MATERIAL_EDITOR_SET_NAME)
								/* set the name of the material_editor */
								material_editor_set_name(temp_material_editor);
#endif /* defined (MATERIAL_EDITOR_SET_NAME) */
								/* register for any changes */
								/* temp_material_editor->manager_callback_id=
									MANAGER_REGISTER(Graphical_material)(
									material_editor_global_object_change,temp_material_editor,
									graphical_material_manager); */
								/* if (temp_material_editor->global_value==
									temp_material_editor->default_value)
								{
									XtUnmanageChild(temp_material_editor->widget);
								} */
								return_widget=temp_material_editor->widget;
							}
							else
							{
								DEALLOCATE(temp_material_editor);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
			"create_material_editor_widget.  Could not fetch material_editor dialog");
							DEALLOCATE(temp_material_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_material_editor_widget.  Could not register identifiers");
						DEALLOCATE(temp_material_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_material_editor_widget.  Could not register callbacks");
					DEALLOCATE(temp_material_editor);
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
	struct Material_editor_struct *temp_material_editor;

	ENTER(material_editor_get_callback);
	/* check arguments */
	if (material_editor_widget&&callback)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(material_editor_widget,
			XmNuserData,&temp_material_editor,NULL);
		if (temp_material_editor)
		{
			callback->procedure=temp_material_editor->update_callback.procedure;
			callback->data=temp_material_editor->update_callback.data;
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
	struct Material_editor_struct *temp_material_editor=NULL;

	ENTER(material_editor_set_callback);
	if (material_editor_widget&&callback)
	{
		/* Get the pointer to the data for the material_editor dialog */
		XtVaGetValues(material_editor_widget,
			XmNuserData,&temp_material_editor,NULL);
		if (temp_material_editor)
		{
			temp_material_editor->update_callback.procedure=callback->procedure;
			temp_material_editor->update_callback.data=callback->data;
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
	struct Material_editor_struct *temp_material_editor;

	ENTER(material_editor_set_material);
	if (material_editor_widget)
	{
		/* Get the pointer to the data for the material_editor dialog */
		XtVaGetValues(material_editor_widget,
			XmNuserData,&temp_material_editor,NULL);
		if (temp_material_editor)
		{
			return_material=temp_material_editor->edit_material;
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
LAST MODIFIED : 1 December 1997

DESCRIPTION :
Changes the material in the material_editor widget.
==============================================================================*/
{
	int return_code;
	MATERIAL_PRECISION alpha,shininess;
	struct Colour temp_colour;
	struct Material_editor_struct *temp_material_editor;

	ENTER(material_editor_set_material);
	if (material_editor_widget)
	{
		/* Get the pointer to the data for the material_editor dialog */
		XtVaGetValues(material_editor_widget,
			XmNuserData,&temp_material_editor,NULL);
		if (temp_material_editor)
		{
			return_code=1;
			if (temp_material_editor->edit_material)
			{
				DESTROY(Graphical_material)(&(temp_material_editor->edit_material));
			}
			if (material)
			{
				/* create a copy for editing */
				if ((temp_material_editor->edit_material=
					CREATE(Graphical_material)("copy"))&&
					MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)
					(temp_material_editor->edit_material,material))
				{
#if defined (MATERIAL_EDITOR_NAME)
					/* set the name of the material_editor */
					material_editor_set_name(temp_material_editor);
#endif /* defined (MATERIAL_EDITOR_NAME) */
					/* now make all the sub widgets reflect the new data */
					if (Graphical_material_get_ambient(
						temp_material_editor->edit_material,&temp_colour))
					{
						colour_editor_set_colour(temp_material_editor->ambient_widget,
							&temp_colour);
					}
					if (Graphical_material_get_diffuse(
						temp_material_editor->edit_material,&temp_colour))
					{
						colour_editor_set_colour(temp_material_editor->diffuse_widget,
							&temp_colour);
					}
					if (Graphical_material_get_emission(
						temp_material_editor->edit_material,&temp_colour))
					{
						colour_editor_set_colour(temp_material_editor->emission_widget,
							&temp_colour);
					}
					if (Graphical_material_get_specular(
						temp_material_editor->edit_material,&temp_colour))
					{
						colour_editor_set_colour(temp_material_editor->specular_widget,
							&temp_colour);
					}
					if (Graphical_material_get_alpha(temp_material_editor->edit_material,
						&alpha))
					{
						edit_var_set_data(temp_material_editor->alpha_widget,
							EDIT_VAR_VALUE,(EDIT_VAR_PRECISION)alpha);
					}
					if (Graphical_material_get_shininess(
						temp_material_editor->edit_material,&shininess))
					{
						edit_var_set_data(temp_material_editor->shininess_widget,
							EDIT_VAR_VALUE,(EDIT_VAR_PRECISION)shininess);
					}
					/* need to check window is there the first time else error occurs */
					if (XtWindow(temp_material_editor->a3d_widget))
					{
						material_editor_update_picture(temp_material_editor);
					}
					XtManageChild(temp_material_editor->widget);
				}
				else
				{
					if (temp_material_editor->edit_material)
					{
						DESTROY(Graphical_material)(&(temp_material_editor->edit_material));
					}
					display_message(ERROR_MESSAGE,
						"material_editor_set_material.  Could not make copy of material");
					material=(struct Graphical_material *)NULL;
					return_code=0;
				}
			}
			if (!material)
			{
				XtUnmanageChild(temp_material_editor->widget);
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
