/*******************************************************************************
FILE : volume_texture_editor.c

LAST MODIFIED : 26 November 2001

DISCRIPTION :
Creation & Callback code for Motif texture window
==============================================================================*/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <Xm/Xm.h>
#include <X11/Intrinsic.h>
#include <Mrm/MrmAppl.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/ScrollBar.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include "general/callback.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_window.h"
#include "graphics/texture_graphics.h"
#include "graphics/texture_line.h"
#include "graphics/mcubes.h"
#include "graphics/volume_texture.h"
#include "graphics/volume_texture_editor.h"
#include "graphics/volume_texture_editor.uidh"
#include "graphics/volume_texture_editor_dialog.h"
#include "material/material_editor_dialog.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "io_devices/input_module.h"
#include "three_d_drawing/ThreeDDraw.h"
#include "user_interface/filedir.h"
#include "user_interface/message.h"
#include "view/coord.h"
#include "view/camera.h"
#include "select/select_graphical_material.h"
#include "select/select_environment_map.h"

/*
Module constants
----------------
*/
#define MAX_MATERIALS 100
static struct Graphical_material *material_pick_list[MAX_MATERIALS];
static struct Environment_map *env_map_pick_list[MAX_MATERIALS];

/*
Module variables
----------------
*/
Widget texture_window_shell=NULL,texture_window_widget=NULL;
/*???debug */
extern int debug_i,debug_j,debug_k;
/* for material picking */
static int pick_index=0;
static int env_map_pick_index=0;
static int create_vt_node_group_flag=0;
#if defined (GL_API)
Matrix idmat=
{
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};
static float ortho_left,ortho_right,ortho_bottom,ortho_top;
#endif /* defined (GL_API) */
#if defined (OPENGL_API)
static GLdouble ortho_left,ortho_right,ortho_bottom,ortho_top;
#endif /* defined (OPENGL_API) */
static int volume_texture_editor_hierarchy_open=0;
static MrmHierarchy volume_texture_editor_hierarchy;

/*
Global variables
----------------
*/
struct Light *vt_ed_light=(struct Light *)NULL;
struct Light_model *vt_ed_light_model=(struct Light_model *)NULL;

/*
Module functions
----------------
*/
int create_vt_node_group(struct Texture_window *texture_window,char *name)
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
==============================================================================*/
{
	int i,return_code;
	struct VT_node_group *group, **node_groups;
	struct VT_volume_texture *texture;
	XmString new_item;

	ENTER(create_vt_node_group);
/*???debug */
printf("Creating group %s\n", name);
	return_code=1;
	/* first of all, check for existence */
	if (texture=texture_window->current_texture)
	{
/*???debug */
printf("texture->n_groups = %d\n", texture->n_groups);
		i=0;
		while (return_code&&(i<texture->n_groups))
		{
			if (group=texture->node_groups[i])
			{
				if (0==strcmp(group->name,name))
				{
					display_message(ERROR_MESSAGE,"Group already exists");
/*???debug */
printf("Group already exists\n");
					return_code=0;
				}
			}
			i++;
		}
		if (return_code)
		{
			if (ALLOCATE(group,struct VT_node_group,1))
			{
				if (ALLOCATE(group->name,char,strlen(name)+1))
				{
					strcpy(group->name,name);
					group->n_nodes=0;
					group->nodes=(int *)NULL;
					/* ALLOCATE new set of pointers */
					if (ALLOCATE(node_groups,struct VT_node_group *,texture->n_groups+1))
					{
						for (i=0;i<texture->n_groups;i++)
						{
							node_groups[i]=texture->node_groups[i];
						}
						node_groups[texture->n_groups]=group;
						texture->n_groups++;
						DEALLOCATE(texture->node_groups);
						texture->node_groups=node_groups;
						strcpy(texture_window->current_node_group, name);
						new_item=XmStringCreateSimple(name);
						XmListAddItem(texture_window->node_group_list,new_item,0);
						XmStringFree(new_item);
/*???debug */
printf("New Group List:\n");
for (i=0;i<texture->n_groups;i++)
{
	printf("%s\n",(texture->node_groups[i])->name);
}
					}
					else
					{
						display_message(ERROR_MESSAGE,
						"create_vt_node_group.  Could not allocate memory for node groups");
						DEALLOCATE(group->name);
						DEALLOCATE(group);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
				"create_vt_node_group.  Could not allocate memory for node group name");
					DEALLOCATE(group);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_vt_node_group.  Could not allocate memory for node group");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "create_vt_node_group: no texture");
		return_code = 0;
	}
	LEAVE;

	return(return_code);
} /* create_vt_node_group */

int remove_vt_node_group(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
==============================================================================*/
{
	int count,found,i,remove_index,return_code;
	struct VT_node_group **node_groups;
	struct VT_volume_texture *texture;

	ENTER(remove_vt_node_group);
	/* first of all, check for existence */
	remove_index=0;
	return_code=0;
	count=0;
	found=0;
	if (texture=texture_window->current_texture)
	{
		if (texture->n_groups>0)
		{
			for (i=0;i<texture->n_groups;i++)
			{
				if (0==strcmp((texture->node_groups[i])->name,
					texture_window->current_node_group))
				{
					remove_index=i;
/*???debug */
printf("Removing group %s\n", (texture->node_groups[i])->name);
					XmListDeleteItem(texture_window->node_group_list,
						XmStringCreateSimple((texture->node_groups[i])->name));
					found=1;
				}
			}
			/* ALLOCATE new set of pointers */
			if (found)
			{
				DEALLOCATE(texture->node_groups[remove_index]->name);
				DEALLOCATE(texture->node_groups[remove_index]->nodes);
				DEALLOCATE(texture->node_groups[remove_index]);
				if (1==texture->n_groups)
				{
					texture->n_groups=0;
					DEALLOCATE(texture->node_groups);
				}
				else
				{
					if (ALLOCATE(node_groups,struct VT_node_group *,texture->n_groups-1))
					{
						count=0;
						for (i=0;i<texture->n_groups;i++)
						{
							if (i!=remove_index)
							{
								node_groups[count]=texture->node_groups[i];
								count++;
							}
						}
						DEALLOCATE(texture->node_groups);
						texture->node_groups=node_groups;
						XmListDeselectAllItems(texture_window->node_group_list);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"remove_vt_node_group.  Could not allocate memory for groups");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"Group not found");
				return_code=0;
			}
/*???debug */
printf("New Group List:\n");
for (i=0;i<texture->n_groups;i++)
{
	printf("%s\n", (texture->node_groups[i])->name);
}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"remove_vt_node_group.  No texture");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* remove_vt_node_group */

void process_prompt(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
==============================================================================*/
{
	char *string_ptr;
	char word[100], string[512], prompt[100];
	int i, offset, length;
	int nx, ny, nz;
	static double division[100][3];
	static int start_input, n[3];

	ENTER(process_prompt);
	if (create_vt_node_group_flag)
	{
		offset=0;
		XtVaGetValues(texture_window->prompt_text_field,XmNvalue,&string_ptr,NULL);
		strcpy(string,string_ptr);
		length=strlen(string);
/*???debug */
printf("string = %s\n", string);
		while ((sscanf(&string[offset], "%s ", word) != EOF) && offset < length)
		{
			offset += (strlen(word)+1);
			printf("scanned %s\n", word);
			if (0 == strcmp(">", word))
			{
					start_input = 1;
			}
			if (start_input && (0 != strcmp(">", word)) )
			{
				create_vt_node_group(texture_window, word);
				start_input = 0;
			}
		}
		create_vt_node_group_flag=0;
	}
	else
	{
		if (texture_window->irregular_mesh_request>0||texture_window->soft_mode_on)
		{
			offset =0;
			n[texture_window->irregular_mesh_request-1]=0;
			nx = texture_window->current_texture->dimension[0]+1;
			ny = texture_window->current_texture->dimension[1]+1;
			nz = texture_window->current_texture->dimension[2]+1;
			XtVaGetValues(texture_window->prompt_text_field,XmNvalue,&string_ptr,
				NULL);
			strcpy(string, string_ptr);
			length = strlen(string);
			printf("string = %s\n", string);
			while ((sscanf(&string[offset], "%s ", word) != EOF) && offset < length)
			{
				offset += (strlen(word)+1);
				printf("scanned %s\n", word);
			if (0 == strcmp(">", word))
			{
					start_input = 1;
			}
			if (start_input && (0 != strcmp(">", word)) )
			{
					if (texture_window->soft_mode_on)
					{
				texture_window->select_value2 = atof(word);
					}
					else
					{
				switch(texture_window->irregular_mesh_request)
				{
				case 1:
						division[n[0]][0] = atof(word);
						n[0]++;
						break;
				case 2:
						division[n[1]][1] = atof(word);
						n[1]++;
						break;
				case 3:
						division[n[2]][2] = atof(word);
						n[2]++;
						break;
				}
					}
			}
				}
				start_input = 0;
				if (texture_window->soft_mode_on)
				{
					sprintf(prompt, "Enter cutoff radius [%f]: > ", texture_window->select_value2);
					XtVaSetValues(texture_window->prompt_text_field,XmNvalue,
			prompt,NULL);
			XtVaSetValues(texture_window->prompt_text_field,XmNcursorPosition,
						40,NULL);
				}
				else
				{
				switch(texture_window->irregular_mesh_request)
			{
					case 1:
						if (n[0] != nx && n[0] != 0)
						{
					XtVaSetValues(texture_window->prompt_text_field,XmNvalue,
							"ERROR: Re-enter values for direction 1: > ",NULL);
					XtVaSetValues(texture_window->prompt_text_field,XmNcursorPosition,
					40,NULL);
					texture_window->irregular_mesh_request = 1;
						}
						else
						{
						sprintf(prompt, "%d Grid divisions for direction 2: > ", texture_window->current_texture->dimension[1]+1);
					XtVaSetValues(texture_window->prompt_text_field,XmNvalue,
					prompt,NULL);
					XtVaSetValues(texture_window->prompt_text_field,XmNcursorPosition,
					40,NULL);
					texture_window->irregular_mesh_request = 2;
						}
						break;
					case 2:
						if (n[1] != ny && n[1] != 0)
						{
					XtVaSetValues(texture_window->prompt_text_field,XmNvalue,
							"ERROR: Re-enter values for direction 2: > ",NULL);
					XtVaSetValues(texture_window->prompt_text_field,XmNcursorPosition,
					40,NULL);
					texture_window->irregular_mesh_request = 2;
						}
						else
						{
					sprintf(prompt, "%d Grid divisions for direction 3: > ", texture_window->current_texture->dimension[2]+1);
					XtVaSetValues(texture_window->prompt_text_field,XmNvalue,
					prompt,NULL);
					XtVaSetValues(texture_window->prompt_text_field,XmNcursorPosition,
					40,NULL);
					texture_window->irregular_mesh_request = 3;
						}
						break;
					case 3:
								if (n[2] != nz  && n[2] != 0)
						{
					XtVaSetValues(texture_window->prompt_text_field,XmNvalue,
							"ERROR: Re-enter values for direction 3: > ",NULL);
					XtVaSetValues(texture_window->prompt_text_field,XmNcursorPosition,
					40,NULL);
					texture_window->irregular_mesh_request = 3;
						}
						else
						{
					XtVaSetValues(texture_window->prompt_text_field,XmNvalue,
					"> ",NULL);
					XtVaSetValues(texture_window->prompt_text_field,XmNcursorPosition,
					40,NULL);
					texture_window->irregular_mesh_request = 0;

					if (texture_window->current_texture->grid_spacing)
					{
							DEALLOCATE(texture_window->current_texture->grid_spacing);
					}
					texture_window->current_texture->grid_spacing = NULL;
					if (ALLOCATE(texture_window->current_texture->grid_spacing, double,
							nx+ny+nz))
					{
							printf("Divisions <1>: ");
							for (i=0;i<nx;i++)
							{
						if (n[0] == 0)
						{
								texture_window->current_texture->grid_spacing[i] = (double) i / (double)(nx-1);
						}
						else
						{
								texture_window->current_texture->grid_spacing[i] = division[i][0];
						}
						printf("%lf ", texture_window->current_texture->grid_spacing[i]);
							}
							printf("\n");
							printf("Divisions <2>: ");
							for (i=0;i<ny;i++)
							{
						if (n[1] == 0)
						{
								texture_window->current_texture->grid_spacing[i+nx] = (double) i / (double)(ny-1);
						}
						else
						{
								texture_window->current_texture->grid_spacing[i+nx] = division[i][1];
						}
						printf("%lf ", texture_window->current_texture->grid_spacing[i+nx]);
							}
							printf("\n");
							printf("Divisions <3>: ");
							for (i=0;i<nz;i++)
							{
						if (n[2] == 0)
						{
								texture_window->current_texture->grid_spacing[i+nx+ny] = (double) i / (double)(nz-1);
						}
						else
						{
								texture_window->current_texture->grid_spacing[i+nx+ny] = division[i][2];
						}
						printf("%lf ", texture_window->current_texture->grid_spacing[i+nx+ny]);
							}
							printf("\n");
							update_grid(texture_window);
					}
					else
					{
							display_message(ERROR_MESSAGE,"grid_spacing.  Alloc failed");
					}
						}
						break;
					}
			}

		}
	}
	LEAVE;
} /* process_prompt */

static void regular_mesh(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 20 February 1997
DESCRIPTION :
==============================================================================*/
{
	struct Texture_window *texture_window;

	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	printf("Regular mesh called\n");
	if (texture_window=(struct Texture_window *)client_data)
	{
			DEALLOCATE(texture_window->current_texture->grid_spacing);
			texture_window->current_texture->grid_spacing = (double *) NULL;
	}
}

static void irregular_mesh(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 20 February 1997
DESCRIPTION :
==============================================================================*/
{
	struct Texture_window *texture_window;
	char string[100];

	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	printf("Irregular mesh called\n");
	if (texture_window=(struct Texture_window *)client_data)
	{
	sprintf(string, "%d Grid divisions for direction 1: > ", texture_window->current_texture->dimension[0]+1);
		texture_window->irregular_mesh_request = 1;
		XtVaSetValues(texture_window->prompt_text_field,XmNvalue,
			string,NULL);
		XtVaSetValues(texture_window->prompt_text_field,XmNcursorPosition,
					40,NULL);

	}
}

static void group_cb(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 9 January 1998

DESCRIPTION :
==============================================================================*/
{
	int index;
	struct Texture_window *texture_window;
	char string[100];

	ENTER(group_cb);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		XtVaGetValues(w,XmNuserData,&index,NULL);
		switch(index)
		{
			case 0:
			{
				printf("Creating Group:\n");
				sprintf(string, "Enter Group Name: > ");
				XtVaSetValues(texture_window->prompt_text_field,XmNvalue,
					string,NULL);
				XtVaSetValues(texture_window->prompt_text_field,XmNcursorPosition,
					40,NULL);
				create_vt_node_group_flag = 1;
			} break;
			case 1:
			{
				printf("Removing Group:\n");
				remove_vt_node_group(texture_window);
				create_vt_node_group_flag = 0;
			} break;
			case 2:
			{
				texture_window->edit_group_mode = !(texture_window->edit_group_mode);
				if (texture_window->edit_group_mode)
				{
					printf("Editing Groups ON\n");
				}
				else
				{
					printf("Editing Groups OFF\n");
				}
			} break;
			default: break;
		}
	}
}

static void slit_cb(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 9 January 1998

DESCRIPTION :

==============================================================================*/
{
	int index;
	struct Texture_window *texture_window;

	ENTER(slit_cb);
	USE_PARAMETER(cbs);
	/* checking arguments */
	if (texture_window=(struct Texture_window *)client_data)
	{
		XtVaGetValues(w,XmNuserData,&index,NULL);
		switch(index)
		{
			case 0:
			{
				texture_window->slit_mode[0] = !texture_window->slit_mode[0];
				if (texture_window->slit_mode[0])
				{
					printf("Slit editing on\n");
				}
				else
				{
					printf("Slit editing off\n");
				}
			} break;
			case 1:
			{
				texture_window->slit_mode[1] = !texture_window->slit_mode[1];
			} break;
			case 2:
			{
				texture_window->slit_mode[2] = !texture_window->slit_mode[2];
			} break;
			case 3:
			{
				texture_window->slit_mode[3] = !texture_window->slit_mode[3];
			} break;
			default: break;
		}
		if (texture_window->slit_mode[0])
		{
			printf("Slit type = xi[%d][%d][%d]\n",
				texture_window->slit_mode[1],
				texture_window->slit_mode[2],
				texture_window->slit_mode[3]);
		}
	}
}

static int display_material_sphere_3d(struct Graphical_material *material,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1996

DESCRIPTION :
Draws a sphere with the specified <material>.
==============================================================================*/
{
	float xstep,ystep;
	int return_code;
	struct Texture_window *texture_window;

	ENTER(display_material_sphere_3d);
	xstep=(ortho_right-ortho_left)*.09;
	ystep=(ortho_top-ortho_bottom)*0.25;
	/* check arguments */
	if (material&&(texture_window=(struct Texture_window *)user_data))
	{
		material_pick_list[pick_index]=material;
		execute_Graphical_material(material);
#if defined (OPENGL_API)
		glPushMatrix();
		/*???DB.  What if more than 10 materials ? */
		glTranslatef(xstep+xstep*(pick_index%10),ystep+ystep*((int)pick_index/10),
			0);
		/* if current texture window material, draw larger & finer */
		if (material==texture_window->current_material)
		{
			glScalef(9.0,9.0,9.0);
			glCallList(texture_window->tw_sphere);
		}
		else
		{
			glScalef(7.0,7.0,7.0);
			glCallList(texture_window->tw_sphere);
		}
		glPopMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		pushmatrix();
		/*???DB.  What if more than 10 materials ? */
		translate(xstep+xstep*(pick_index % 10),ystep+ystep*((int)pick_index/10),
			0);
		/* if current texture window material, draw larger & finer */
		if (material==texture_window->current_material)
		{
			scale(9.0,9.0,9.0);
			callobj(texture_window->tw_sphere);
		}
		else
		{
			scale(7.0,7.0,7.0);
			callobj(texture_window->tw_sphere);
		}
		popmatrix();
#endif /* defined (GL_API) */
		pick_index++;
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* display_material_sphere */

static int display_env_map_3d(struct Environment_map *env_map,
	void *void_texture_window)
/*******************************************************************************
LAST MODIFIED : 25 September 1996

DESCRIPTION :
Draws a sphere with the specified <material>.
==============================================================================*/
{
	float xstep,ystep;
	int ediv,n_envmaps,return_code;
	struct Texture_window *texture_window;

	ENTER(display_env_map_3d);
	/* check arguments */
	if (env_map&&(texture_window=(struct Texture_window *)void_texture_window))
	{
		n_envmaps=NUMBER_IN_MANAGER(Environment_map)(
			texture_window->environment_map_manager);
/*???debug */
printf("Drawing env map %s\n",env_map->name);
printf("Number of environment maps defined = %d\n",n_envmaps);
		if (n_envmaps > 5)
		{
			ediv=10;
		}
		else
		{
			ediv=5;
		}
		xstep=(ortho_right-ortho_left)*0.18*5.0/ediv;
		ystep=(ortho_top-ortho_bottom)*0.5*5.0/ediv;
		env_map_pick_list[env_map_pick_index]=env_map;
#if defined (OPENGL_API)
		glPushMatrix();
		/*???DB.  What if more than 10 materials ? */
		glTranslatef((float)xstep+xstep*(env_map_pick_index % ediv),
			ystep+ystep*((int)env_map_pick_index/ediv),0);
		/* if current texture window material, draw larger & finer */
		if (env_map==texture_window->current_env_map)
		{
			glScalef(9.0*5.0/ediv,9.0*5.0/ediv,9.0*5.0/ediv);
		}
		else
		{
			glScalef(7.0*5.0/ediv,7.0*5.0/ediv,7.0*5.0/ediv);
		}
		execute_Graphical_material(env_map->face_material[4]);
		glCallList(texture_window->tw_envsquare);
		glTranslatef(2.0,0,0);
		execute_Graphical_material(env_map->face_material[0]);
		glCallList(texture_window->tw_envsquare);
		glTranslatef(2.0,0,0);
		execute_Graphical_material(env_map->face_material[5]);
		glCallList(texture_window->tw_envsquare);
		glTranslatef(2.0,0,0);
		execute_Graphical_material(env_map->face_material[1]);
		glCallList(texture_window->tw_envsquare);
		glTranslatef(-4.0,2.0,0);
		glRotatef(-90,0,0,1.0);
		execute_Graphical_material(env_map->face_material[3]);
		glCallList(texture_window->tw_envsquare);
		glRotatef(-90,0,0,1.0);
		glTranslatef(0.0,-4.0,0);
		glRotatef(-90,0,0,1.0);
		execute_Graphical_material(env_map->face_material[2]);
		glCallList(texture_window->tw_envsquare);
		glPopMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		pushmatrix();
		/*???DB.  What if more than 10 materials ? */
		translate((float)xstep+xstep*(env_map_pick_index % ediv),ystep+ystep*
			((int)env_map_pick_index/ediv),0);
		/* if current texture window material, draw larger & finer */
		if (env_map==texture_window->current_env_map)
		{
			scale(9.0*5.0/ediv,9.0*5.0/ediv,9.0*5.0/ediv);
		}
		else
		{
			scale(7.0*5.0/ediv,7.0*5.0/ediv,7.0*5.0/ediv);
		}
		execute_Graphical_material(env_map->face_material[4]);
		callobj(texture_window->tw_envsquare);
		translate(2.0,0,0);
		execute_Graphical_material(env_map->face_material[0]);
		callobj(texture_window->tw_envsquare);
		translate(2.0,0,0);
		execute_Graphical_material(env_map->face_material[5]);
		callobj(texture_window->tw_envsquare);
		translate(2.0,0,0);
		execute_Graphical_material(env_map->face_material[1]);
		callobj(texture_window->tw_envsquare);
		translate(-4.0,2.0,0);
		rot(90,'z');
		execute_Graphical_material(env_map->face_material[3]);
		callobj(texture_window->tw_envsquare);
		rot(-90,'z');
		translate(0.0,-4.0,0);
		rot(-90,'z');
		execute_Graphical_material(env_map->face_material[2]);
		callobj(texture_window->tw_envsquare);
		popmatrix();
#endif /* defined (GL_API) */
		env_map_pick_index++;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"display_env_map_3d.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* display_env_map_3d */

static void select_3d_draw(Widget w,XtPointer tag,XtPointer reason)
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
Uses gl to draw a sphere with a lighting source.
==============================================================================*/
{
	struct Texture_window *texture_window=
		(struct Texture_window *)tag;
#if defined(OPENGL_API)
	GLdouble params[4],left,right,bottom,top;
#endif /* defined(OPENGL_API) */
#if defined(GL_API)
	Screencoord left,right,bottom,top;
#endif /* defined (GL_API) */

	ENTER(select_3d_draw);
	USE_PARAMETER(w);
	USE_PARAMETER(reason);
	pick_index=0;
	env_map_pick_index=0;
#if defined (OPENGL_API)
	glGetDoublev(GL_VIEWPORT, params);
	left = -params[2]/2;
	right = params[2]/2;
	bottom = -params[3]/2;
	top = params[3]/2;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	ortho_left = left;
	ortho_right = right;
	ortho_top = top;
	ortho_bottom = bottom;
	glOrtho(0,ortho_right*2,0,ortho_top*2,-100,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	/* polarview port */
	glTranslatef(0,0,-10.0);
	/* glRotatef etc. if angles defined */
#if defined (USEMANAGER)
	reset_Lights();
	printf("volume_texture_editor.c: activate_Light_model() is obsolete!\n");
	/* activate_Light_model(); */
	execute_Light(vt_ed_light,NULL);
#endif /* defined (USEMANAGER) */
#if defined (OLD_CODE)
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, light_ambient);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
#endif /* defined (OLD_CODE) */
	glColor3b(75,50,100);
	glClearColor(0.29,0.19,0.39,0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
	reshapeviewport();
	getviewport(&left,&right,&bottom,&top);
	mmode(MPROJECTION);
	ortho_left = left;
	ortho_right = right;
	ortho_top = top;
	ortho_bottom = bottom;
	ortho(ortho_left,ortho_right,ortho_bottom,ortho_top,-100,100);
	mmode(MVIEWING);
	loadmatrix(idmat);
	polarview(10.0,0,0,0);
	reset_Lights();
	activate_Light_model();
	execute_Light(vt_ed_light,NULL);
	RGBcolor(75,50,100);
	clear();
	zclear();
#endif /* defined (GL_API) */
	if (texture_window->env_mode_on)
	{
/*??? debug */
printf("b4 for each\n");
			FOR_EACH_OBJECT_IN_MANAGER(Environment_map)(display_env_map_3d,
				(void *)texture_window,texture_window->environment_map_manager);
/*??? debug */
printf("after for each\n");
	}
	/* draw current material window edit sphere */
	else
	{
#if defined (DEBUG)
		{
			struct Graphical_material *current_material;

			if (current_material=
				(struct Graphical_material *)material_editor_dialog_get_data(
					(Widget)NULL,MATERIAL_EDITOR_DIALOG_DATA))
			{
				execute_Graphical_material(current_material);
			}
		}
#endif /* defined (DEBUG) */
		FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(display_material_sphere_3d,
			(void *)texture_window,texture_window->graphical_material_manager);
	}
	LEAVE;
} /* select_3d_draw */

static void select_3d_init_CB(Widget widget,XtPointer tag,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
==============================================================================*/
{
#if defined (USEMANAGER)
	static char *vt_ed_light_model_properties="infinite ambient 0.5 0.5 0.5";
	static char *vt_ed_light_properties=
		"colour 1.0 1.0 1.0 direction 0.0 -0.5 -1.0 on infinite";
	struct Modify_light_data modify_light_data;
	struct Modify_light_model_data modify_light_model_data;
	struct Parse_state *parse_state;
#endif /* defined (USEMANAGER) */
	struct Texture_window *texture_window=(struct Texture_window *)tag;

	ENTER(select_3d_init_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
/*???debug */
printf("***** create light ******\n");
	X3dThreeDDrawingMakeCurrent(texture_window->select_3d_widget);
#if defined (OPENGL_API)
	glDepthRange(0,1);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
	lsetdepth(getgdesc(GD_ZMIN),getgdesc(GD_ZMAX));
	zbuffer(1);
	mmode(MVIEWING);
	blendfunction(BF_SA,BF_MSA);
#endif /* defined (GL_API) */
	/* set up the material and lights etc */
#if defined (USEMANAGER)
	if (vt_ed_light=CREATE(Light)("vt_ed_light"))
	{
		if (parse_state=create_Parse_state(vt_ed_light_properties))
		{
			if ((modify_light_data.light_manager=CREATE_MANAGER(Light)())&&
				(modify_light_data.default_light=CREATE(Light)("default")))
			{
				modify_Light(parse_state,(void *)vt_ed_light,
					(void *)&modify_light_data);
				DESTROY(Light)(&(modify_light_data.default_light));
				DESTROY_MANAGER(Light)(&(modify_light_data.light_manager));
			}
			else
			{
				DESTROY_MANAGER(Light)(&(modify_light_data.light_manager));
			}
			destroy_Parse_state(&parse_state);
		}
	}
	/* light model manager */
	if (vt_ed_light_model=CREATE(Light_model)("vt_ed_light_model"))
	{
		if (parse_state=create_Parse_state(vt_ed_light_model_properties))
		{
			if ((modify_light_model_data.light_model_manager=
				CREATE_MANAGER(Light_model)())&&
				(modify_light_model_data.default_light_model=
				CREATE(Light_model)("default")))
			{
				modify_Light_model(parse_state,(void *)vt_ed_light_model,
					(void *)&modify_light_model_data);
				DESTROY(Light_model)(&(modify_light_model_data.default_light_model));
				DESTROY_MANAGER(Light_model)(&(modify_light_model_data.
					light_model_manager));
			}
			else
			{
				DESTROY_MANAGER(Light_model)(&(modify_light_model_data.
					light_model_manager));
			}
			destroy_Parse_state(&parse_state);
		}
		printf("volume_texture_editor.c: set_Light_model obsolete\n");
		/* set_Light_model(vt_ed_light_model); */
	}
#endif /* defined (USEMANAGER) */
	LEAVE;
} /* select_3d_init_CB */

static void select_material_3d(struct Texture_window *texture_window,double x,
	double y)
/*******************************************************************************
LAST MODIFIED : 20 June 1996

DESCRIPTION :
==============================================================================*/
{
	int index,ediv;
	double norm_x,norm_y,xstep,ystep,xdiv;

	ENTER(select_material);
	/* checking arguments */
	if (texture_window)
	{
		if (texture_window->env_mode_on)
		{
			if (NUMBER_IN_MANAGER(Environment_map)(
				texture_window->environment_map_manager)>5)
			{
				ediv=10.0;
				/* these numbers represent the number of env_maps displayed in one row*/
			}
			else
			{
				ediv=5.0;
			}
			xstep=(ortho_right-ortho_left)*0.18*5.0/ediv;
			ystep=(ortho_top-ortho_bottom)*.5*5.0/ediv;
			xdiv=ediv;
		}
		else
		{
			xstep=(ortho_right-ortho_left)*0.09;
			ystep=(ortho_top-ortho_bottom)*.25;
			xdiv=10.0;
		}
		x *= (ortho_right-ortho_left);
		y *= (ortho_top-ortho_bottom);
		norm_x=(x-xstep)/xstep+0.5;
		norm_y=(y-ystep)/ystep+0.5;
		index=(int)norm_x+xdiv*((int)norm_y);
		if (texture_window->env_mode_on)
		{
/*???debug */
printf("******* Env Index = %d *******\n",index);
			if ((index<env_map_pick_index)&&(index>=0)&&env_map_pick_list[index])
			{
				texture_window->current_env_map=env_map_pick_list[index];
			}
/*???debug */
if (texture_window->current_env_map)
{
	printf("current_environment_map = %s \n",texture_window->
		current_env_map->name);
}
		}
		else
		{
/*??? debug */
printf("******* Material Index = %d *******\n",index);
			if ((index<pick_index)&&(index >= 0)&&material_pick_list[index])
			{
				texture_window->current_material=material_pick_list[index];
				material_editor_dialog_set_material((Widget)NULL,
					texture_window->current_material);
			}
/*???debug */
printf("current_material = %s \n",Graphical_material_name(texture_window->current_material));
		}
		X3dThreeDDrawingMakeCurrent(texture_window->select_3d_widget);
		select_3d_draw(NULL,texture_window,NULL);
		X3dThreeDDrawingSwapBuffers();
		X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
		graphics_loop((XtPointer)texture_window);
		X3dThreeDDrawingSwapBuffers();
	}
	else
	{
		display_message(ERROR_MESSAGE,"select_material.  Invalid argument(s)");
	}
	LEAVE;
} /* select_material */

static void select_3d_input(Widget w,XtPointer user_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
==============================================================================*/
{
	double norm_x,norm_y;
	int x,y;
	struct Texture_window *texture_window;
	unsigned int border_width,depth,height,width;
	Window win;
	XButtonEvent *event;
	X3dThreeDDrawCallbackStruct *callback;

	ENTER(select_3d_input);
	USE_PARAMETER(w);
	if (texture_window=(struct Texture_window *)user_data)
	{
		if (callback=(X3dThreeDDrawCallbackStruct *)call_data)
		{
			if (X3dCR_INPUT==callback->reason)
			{
			/* find window coords */
			XGetGeometry(texture_window->user_interface->display,
				callback->window,&win,&x,&y,&width,&height,&border_width,&depth);
/*??? debug */
printf("window size = %u, %u\n",width,height);
				if ((callback->event)&&((ButtonPress==callback->event->type)||
					(ButtonRelease==callback->event->type)))
				{
					event= &(callback->event->xbutton);
					if (ButtonPress==callback->event->type)
					{
/*??? debug */
printf("button press at %d %d\n",event->x,event->y);
						norm_x=(double)event->x/(double)width;
						norm_y=1.0-(double)event->y/(double)height;
						select_material_3d(texture_window,norm_x,norm_y);
					}
/*??? debug */
else
{
	printf("button release at %d %d\n",event->x,event->y);
}
printf("normalized mouse coords = %lf,%lf\n",(double)event->x/(double)width,
	1.0-(double)event->y/(double)height);
				}
				else
				{
					display_message(ERROR_MESSAGE,"select_3d_input.  Invalid X event");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"select_3d_input.  Invalid reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"select_3d_input.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"select_3d_input.  Missing user_data");
	}
	LEAVE;
} /* select_3d_input */

static void load_2d_texture(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
temp for debug
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(load_2d_texture);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		printVT(texture_window->current_texture);
	}
	else
	{
		display_message(ERROR_MESSAGE,"load_2d_texture.  Invalid argument(s)");
	}
	LEAVE;
} /* load_2d_texture */

static void save_as_finite_elements(Widget widget,XtPointer texture_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1996

DESCRIPTION :
Brings up the dialog for creating .exnode and .exelem files from the volume
texture.
==============================================================================*/
{
	struct Texture_window *window;

	ENTER(save_as_finite_elements);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (window=(struct Texture_window *)texture_window)
	{
		open_create_finite_elements_dialog(&(window->create_finite_elements_dialog),
			window->current_texture,window->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"save_as_finite_elements.  Invalid argument(s)");
	}
	LEAVE;
} /* save_as_finite_elements */

static void vt_select_mat_create(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(vt_select_mat_create);
	USE_PARAMETER(cbs);
/*???debug */
printf("vt_select_mat_create called\n");
	/* checking arguments */
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->vt_3d_select_form=w;
	}
	else
	{
		display_message(ERROR_MESSAGE,"vt_select_mat_create.  Invalid argument(s)");
	}
	LEAVE;

} /* vt_select_mat_create */

#if defined (OLD_CODE)
static void transformation_editor_callback(Widget w,void *user_data,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Called when the transformation changes.
==============================================================================*/
{
	double temp_val;
	int i;
	struct Cmgui_coordinate *new_coord;
	struct Texture_window *texture_window;

	ENTER(transformation_editor_callback);
	USE_PARAMETER(w);
	if ((texture_window=(struct Texture_window *)user_data)&&
		(new_coord=(struct Cmgui_coordinate *)call_data))
	{
/*???debug */
printf("In callback ********************************************\n");
printf("(%p) %lf %lf %lf %lf %lf %lf\n", texture_window,
		(new_coord->origin).position.data[0],(new_coord->origin).position.data[1],
			(new_coord->origin).position.data[2],(new_coord->origin).direction.
			data[0],(new_coord->origin).direction.data[1],(new_coord->origin).
			direction.data[2]);
		/* if texture window in move_model mode then the model is manipulated,
			otherwise, the cursor is manipulated */
		if (texture_window->input_move_model)
		{
#if defined (OPENGL_API)
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glTranslatef(new_coord->origin.position.data[0],
				new_coord->origin.position.data[1],new_coord->origin.position.data[2]);
			glRotatef(-new_coord->origin.direction.data[2],0,0,1);
			glRotatef(new_coord->origin.direction.data[1],1,0,0);
			glRotatef(-new_coord->origin.direction.data[0],0,1,0);
			glGetFloatv(GL_MODELVIEW_MATRIX,texture_window->model_matrix);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
			loadmatrix(idmat);
			translate((new_coord->origin).position.data[0],(new_coord->origin).
				position.data[1],(new_coord->origin).position.data[2]);
			rot(-(new_coord->origin).direction.data[2],'z');
			rot((new_coord->origin).direction.data[1],'x');
			rot(-(new_coord->origin).direction.data[0],'y');
/*
			rot(-(new_coord->origin).direction.data[0],'y');
			rot((new_coord->origin).direction.data[1],'x');
			rot(-(new_coord->origin).direction.data[2],'z');
*/
/*
			rot((new_coord->origin).direction.data[0],'z');
			rot((new_coord->origin).direction.data[1],'y');
			rot(-(new_coord->origin).direction.data[2],'x');
*/
			getmatrix(texture_window->model_matrix);
#endif /* defined (GL_API) */
		}
		else
		{
			for (i=0;i<3;i++)
			{
				temp_val=texture_window->ximax[i]-1.0/texture_window->xires[i];
				if ((new_coord->origin).position.data[i]<temp_val)
				{
					temp_val=(new_coord->origin).position.data[i];
				}
				if (temp_val>texture_window->ximin[i])
				{
					texture_window->xival[i]=temp_val;
				}
				else
				{
					texture_window->xival[i]=texture_window->ximin[i];
				}
				if (texture_window->set_input_limits)
				{
					(new_coord->origin).position.data[i]=texture_window->xival[i];
				}
			}
			if (texture_window->set_input_limits)
			{
				transformation_editor_dialog_set_data((Widget)NULL,
					TRANSFORMATION_EDITOR_DIALOG_DATA,new_coord);
			}
		}
		X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
		graphics_loop((XtPointer)texture_window);
		X3dThreeDDrawingSwapBuffers();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* transformation_editor_callback */
#endif /* defined (OLD_CODE) */

static void open_transformation_editor(Widget widget,XtPointer texture_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
==============================================================================*/
{
#if defined (OLD_CODE)
	struct Callback_data callback_data;
	struct Texture_window *window;
#endif /* defined (OLD_CODE) */

	ENTER(open_transformation_editor);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	USE_PARAMETER(texture_window);
	display_message(WARNING_MESSAGE,
		"open_transformation_editor.  Transformation editor is not available");
#if defined (OLD_CODE)
	if (window=(struct Texture_window *)texture_window)
	{
		bring_up_transformation_editor_dialog(window->transformation_editor_address,
			(struct Cmgui_coordinate *)NULL, window->user_interface);
		/* set update callback */
		callback_data.procedure=transformation_editor_callback;
		callback_data.data=(void *)texture_window;
		transformation_editor_dialog_set_data(
			*(window->transformation_editor_address),
			TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB,&callback_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_transformation_editor.  Missing texture_window");
	}
#endif /* defined (OLD_CODE) */
	LEAVE;
} /* open_transformation_editor */

#if defined (OLD_CODE)
static void open_material_editor(Widget widget,XtPointer texture_window,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 5 September 1996

DESCRIPTION :
==============================================================================*/
{
	struct Texture_window *window;

	ENTER(open_material_editor);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (window=(struct Texture_window *)texture_window)
	{
		bring_up_material_editor_dialog(window->material_editor_address,
			window->user_interface->application_shell,
			window->graphical_material_manager,window->texture_manager,
			window->current_material,window->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_material_editor.  Missing texture_window");
	}
	LEAVE;
} /* open_material_editor */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
/*???DB.  all_graphical_materials no longer exists */
static void adjust_material(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 23 November 1994

DESCRIPTION :
???DB.  Is this needed ?
==============================================================================*/
{
	int index;
	struct Graphical_material *current_material;
	struct Graphical_material_list_item *material_item;

	ENTER(adjust_material)
	/* checking arguments */
	if (texture_window)
	{
		if (current_material=texture_window->current_material)
		{
			material_item=all_graphical_materials;
			index=0;
			while (material_item&&(material_item->object)&&(current_material==
				material_item->object))
			{
				index++;
				material_item=material_item->next;
			}
			if (material_item&&(material_item->object))
			{
				XtVaSetValues(texture_window->material_sb,XmNvalue,index,NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"adjust_material.  Missing texture_window");
	}
	LEAVE;
} /* adjust_material */
#endif /* defined (OLD_CODE) */

static void adjustxi_sb(struct Texture_window *texture_window,int index)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
==============================================================================*/
{
	char string_number[10];
	double newvalue,xistep[3];
	int i,int_xival[3],value;

	ENTER(adjustxi_sb);
	/* checking arguments */
	if (texture_window)
	{
		XtVaGetValues(texture_window->xi_sb[index],XmNvalue,&value,NULL);
		if (texture_window->cop_mode_on)
		{
			newvalue=(double)value*(texture_window->ximax[index]-texture_window->
				ximin[index])/texture_window->xires[index]+texture_window->ximin[index]
				+(texture_window->ximax[index]-texture_window->ximin[index])
				/(2.0*texture_window->xires[index]);
			texture_window->cop[index]=newvalue
				/* +0.5*texture_window->xires[index] */;
/*???debug */
printf("COP: %lf, %lf, %lf\n", texture_window->cop[0],texture_window->cop[1],
	texture_window->cop[2]);
			sprintf(string_number,"%.3lf",newvalue);
			XtVaSetValues(texture_window->displacement_val_tf[index],XmNvalue,
				string_number,NULL);
		}
		else
		{
			if (texture_window->cell_mode)
			{
				newvalue=(double)value*(texture_window->ximax[index]-texture_window->
					ximin[index])/texture_window->xires[index]+texture_window->
					ximin[index]
					+(texture_window->ximax[index]-texture_window->ximin[index])
					/(2.0*texture_window->xires[index]);
			}
			else
			{
				newvalue=(double)value*(texture_window->ximax[index]-texture_window->
					ximin[index])/texture_window->xires[index]+texture_window->
					ximin[index];
			}
			texture_window->xival[index]=newvalue;
		}
		sprintf(string_number,"%.3lf",newvalue);
		XtVaSetValues(texture_window->xival_tf[index],XmNvalue,string_number,NULL);
		for (i=0;i<3;i++)
		{
			if (texture_window->cell_mode)
			{
				xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
					texture_window->xires[i];
				int_xival[i]=(int)((double)(texture_window->xival[i]-texture_window->
					ximin[i])/xistep[i]);
			}
			else
			{
				xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
					texture_window->xires[i];
				int_xival[i]=(int)((double)(texture_window->xival[i]-(texture_window->
					ximin[i]-0.5*xistep[i]/texture_window->xires[i]))/xistep[i]);
			}
		}
/*???debug */
printf("current coords: (%d, %d, %d)\n",int_xival[0]+1,int_xival[1]+1,
	int_xival[2]+1);
		debug_i=int_xival[0]+1;debug_j=int_xival[1]+1;debug_k=int_xival[2]+1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"adjustxi_sb.  Invalid argument(s)");
	}
	LEAVE;
} /* adjustxi_sb */

static int read_settings_file(char *file_name,void *texture_window)
/*******************************************************************************
LAST MODIFIED : 15 January 1998

DESCRIPTION :
Reads the volume texture from a file and updates <texture_window> settings.
==============================================================================*/
{
	char string_number[10];
	FILE *in_file;
	int i,return_code;
	struct Texture_window *window;
	struct VT_volume_texture *current_texture;
	XmString new_item;

	ENTER(read_settings_file);
	/* check arguments */
	if (file_name&&(window=(struct Texture_window *)texture_window)&&
		(current_texture=window->current_texture))
	{
		if (in_file=fopen(file_name,"r"))
		{
			/* delete any existing node groups */
			if (current_texture->n_groups>0)
			{
				for (i=0;i<current_texture->n_groups;i++)
				{
					strcpy(window->current_node_group,
						(current_texture->node_groups[i])->name);
					remove_vt_node_group(texture_window);
				}
				display_message(WARNING_MESSAGE,
					"Destroying all VT node groups and Xi slits");
				current_texture->n_groups=0;
			}
			if (return_code=read_volume_texture_from_file(current_texture,in_file,
				window->graphical_material_manager,window->environment_map_manager))
			{
				/* set appropriate values on texture_window */
				XtVaSetValues(window->isovalue_sb,XmNvalue,
					(int)((double)(window->current_texture->isovalue*(double) SLIDERMAX)),
					NULL);
				for (i=0;i<3;i++)
				{
					window->ximax[i]=current_texture->ximax[i];
					window->ximin[i]=current_texture->ximin[i];
					window->xival[i]=current_texture->ximin[i];
					window->xires[i]=current_texture->dimension[i];
					sprintf(string_number,"%d",(int)(window->xires[i]));
					XtVaSetValues(window->xires_tf[i],XmNvalue,string_number,NULL);
					adjustxi(window,i);
				}
				/* create group list */
				for (i=0;i<current_texture->n_groups;i++)
				{
					if (0==i)
					{
						strcpy(window->current_node_group,
							(current_texture->node_groups[i])->name);
					}
					new_item=XmStringCreateSimple(
						(current_texture->node_groups[i])->name);
					XmListAddItem(window->node_group_list,new_item,0);
					XmStringFree(new_item);
				}
			}
			fclose(in_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,"read_settings_file.  Could not open file");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_settings_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_settings_file */

static int write_settings_file(char *file_name,void *texture_window)
/*******************************************************************************
LAST MODIFIED : 24 November 1994

DESCRIPTION :
Writes the the volume texture to a file.
???DB.  write_materials_to_com_file ?
==============================================================================*/
{
	FILE *out_file;
	int return_code;
	struct Texture_window *window;

	ENTER(write_settings_file);
	/* check arguments */
	if (file_name&&(window=(struct Texture_window *)texture_window))
	{
		if (out_file=fopen(file_name,"w"))
		{
			return_code=write_volume_texture_to_file(window->current_texture,
				out_file);
			fclose(out_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_settings_file.  Could not open file");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_settings_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_settings_file */

static void group_list_create_cb(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(group_list_create_cb);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->node_group_list=w;
	}
	else
	{
		display_message(ERROR_MESSAGE,"group_list_create_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* group_list_create_cb */

static void group_list_select_cb(Widget w,XtPointer client_data,
	XmListCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
==============================================================================*/
{
	char *name;
	struct Texture_window *texture_window;

	ENTER(group_list_select_cb);
	USE_PARAMETER(w);
/*???debug */
printf("Group list selected\n");
	if (texture_window=(struct Texture_window *)client_data)
	{
		if (cbs->selected_item_count != 1)
		{
			printf("Select only one item\n");
		}
		else
		{
			XmStringGetLtoR(cbs->item,XmSTRING_DEFAULT_CHARSET,&name);
			strcpy(texture_window->current_node_group, name);
			printf("Current node group = %s\n", texture_window->current_node_group);
			X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
			graphics_loop((XtPointer)texture_window);
			X3dThreeDDrawingSwapBuffers();
		}
	}
	LEAVE;
} /* group_list_select_cb */

static void identify_tf(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Identifies text fields
==============================================================================*/
{
	char *string_number;
	int index;
	struct Texture_window *texture_window;

	ENTER(identify_tf);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		XtVaGetValues(w,XmNuserData,&index,NULL);
		switch (index)
		{
			case 0:
			{
				texture_window->prompt_text_field=w;
				string_number="Welcome";
			} break;
			case 1:
			{
				texture_window->ximin_tf[0]=w;
				string_number="0";
			} break;
			case 5:
			{
				texture_window->ximin_tf[1]=w;
				string_number="0";
			} break;
			case 9:
			{
				texture_window->ximin_tf[2]=w;
				string_number="0";
			} break;
			case 3:
			{
				texture_window->ximax_tf[0]=w;
				string_number="1";
			} break;
			case 7:
			{
				texture_window->ximax_tf[1]=w;
				string_number="1";
			} break;
			case 11:
			{
				texture_window->ximax_tf[2]=w;
				string_number="1";
			} break;
			case 2:
			{
				texture_window->xival_tf[0]=w;
				string_number="0.5";
			} break;
			case 6:
			{
				texture_window->xival_tf[1]=w;
				string_number="0.5";
			} break;
			case 10:
			{
				texture_window->xival_tf[2]=w;
				string_number="0.5";
			} break;
			case 4:
			{
				texture_window->xires_tf[0]=w;
				string_number="1";
			} break;
			case 8:
			{
				texture_window->xires_tf[1]=w;
				string_number="1";
			} break;
			case 12:
			{
				texture_window->xires_tf[2]=w;
				string_number="1";
			} break;
			case 13:
			{
				texture_window->material_val_tf=w;
				string_number="1.0";
			} break;
			case 14:
			{
				texture_window->displacement_val_tf[0]=w;
				string_number="0";
			} break;
			case 15:
			{
				texture_window->displacement_val_tf[1]=w;
				string_number="0";
			} break;
			case 16:
			{
				texture_window->displacement_val_tf[2]=w;
				string_number="0";
			} break;
			case 17:
			{
				texture_window->select_val_tf=w;
				string_number="1.0";
			}break;
			case 18:
			{
				string_number="1.0";
			} break;
			case 19:
			{
			string_number="1.0";
			} break;
		}
		XtVaSetValues(w,XmNvalue,string_number,NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"identify_tf.  Invalid argument(s)");
	}
	LEAVE;
} /* identify_tf */

static void identify_sb(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Identifies scroll bars
==============================================================================*/
{
	int index;
	struct Texture_window *texture_window;

	ENTER(identify_sb);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		XtVaGetValues(w,XmNuserData,&index,NULL);
		switch (index)
		{
			case 1:
			{
				texture_window->xi_sb[0]=w;
			} break;
			case 2:
			{
				texture_window->xi_sb[1]=w;
			} break;
			case 3:
			{
				texture_window->xi_sb[2]=w;
			} break;
			case 4:
			{
				texture_window->material_sb=w;
			} break;
			case 5:
			{
				texture_window->cutting_plane_sb[0]=w;
			} break;
			case 6:
			{
				texture_window->cutting_plane_sb[1]=w;
			} break;
			case 7:
			{
				texture_window->cutting_plane_sb[2]=w;
			} break;
			case 8:
			{
				texture_window->cutting_plane_sb[3]=w;
			} break;
			case 9:
			{
				texture_window->rotate_sb[0]=w;
			} break;
			case 10:
			{
				texture_window->rotate_sb[1]=w;
			} break;
			case 11:
			{
				texture_window->rotate_sb[2]=w;
			} break;
			case 12:
			{
				texture_window->zoom_sb=w;
				XtVaSetValues(w,XmNvalue,(int) SLIDERMAX/2,NULL);
			} break;
			case 13:
			{
				texture_window->res_sb=w;
			} break;
			case 14:
			{
				texture_window->isovalue_sb=w;
			} break;
		}
		/* set so slider has full range */
		XtVaSetValues(w,XmNminimum,0,XmNmaximum,(int)(SLIDERMAX/0.9),NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"identify_sb.  Invalid argument(s)");
	}
	LEAVE;
} /* identify_sb */






static double volume_intersect(double xl,double xh,double yl,double yh,
	double zl,double zh,double XL,double XH,double YL,double YH,double ZL,
	double ZH)
/*******************************************************************************
LAST MODIFIED : 20 June 1996

DESCRIPTION :
Calculates ratio of volume of cuboid [xl,xh,yl,yh,zl,zh] contained within
cuboid [XL,XH,YL,YH,ZL,ZH]. Returns 0 if no intersection
==============================================================================*/
{
	double ratio;

	ENTER(volume_intersect);
	if ((xh<XL)||(xl>XH)||(yh<YL)||(yl>YH)||(zh<ZL)||(zl>ZH))
	{
		ratio=0;
	}
	else
	{
		if (xl<XL)
		{
			if (xh>XH)
			{
				ratio=XH-XL;
			}
			else
			{
				ratio=xh-XL;
			}
		}
		else
		{
			if (xh>XH)
			{
				ratio=XH-xl;
			}
			else
			{
				ratio=xh-xl;
			}
		}
		if (yl<YL)
		{
			if (yh>YH)
			{
				ratio *= YH-YL;
			}
			else
			{
				ratio *= yh-YL;
			}
		}
		else
		{
			if (yh>YH)
			{
				ratio *= YH-yl;
			}
			else
			{
				ratio *= yh-yl;
			}
		}
		if (zl<ZL)
		{
			if (zh>ZH)
			{
				ratio *= ZH-ZL;
			}
			else
			{
				ratio *= zh-ZL;
			}
		}
		else
		{
			if (zh>ZH)
			{
				ratio *= ZH-zl;
			}
			else
			{
				ratio *= zh-zl;
			}
		}
		ratio /= (xh-xl)*(yh-yl)*(zh-zl);
	}
#if defined (OLD_CODE)
	if (xh<XL||xl>XH||yh<YL||yl>YH||zh<ZL||zl>ZH)
	{
		return(0.0);
	}
	else
	{
		return((MIN(XH,xh)-MAX(XL,xl))*(MIN(YH,yh)-MAX(YL,yl))
			*(MIN(ZH,zh)-MAX(ZL,zl))/((xh-xl)*(yh-yl)*(zh-zl)));
	}
#endif /* defined (OLD_CODE) */
	LEAVE;

	return (ratio);
} /* volume_intersect */

static void remesh_texture(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 9 July 1998

DESCRIPTION :
Major routine. regenerates texture mesh dependent of previous case
If new mesh greater in size , the old texture fills the new cells
occupying its space. In effect the spatial distribution does not change.
If the new mesh is less in size, the old texture is data reduced to fit into the
new mesh - resulting in loss of detail
1D arrays are used where A[i][j][k] = A'[i + j*dimi + k*dimi*dimj]
==============================================================================*/
{
	double cops[MAX_MATERIALS][3],cop[3],is,ie,js,je,ks,ke;
	double env_map_weight[MAX_MATERIALS],material_weight[MAX_MATERIALS],
		total_scalar_value,total_detail,weight;
	int i,index,number_of_distinct_materials,number_of_distinct_env_maps;
	int int_is,int_ie,int_js,int_je,int_ks,int_ke,j,k,l,m,max,n;
	int n_new_cells,n_new_nodes,n_old_nodes,n_old_cells,new_dim[3],old_dim[3];
	int old_i,old_j,old_k,s;
	struct Graphical_material *material,*materials[MAX_MATERIALS];
	struct Environment_map *env_map,*env_maps[MAX_MATERIALS];
	struct VT_texture_cell *cell_ptr,*new_cell,*old_cell;
	struct VT_texture_node *node_ptr,*new_node,*old_node;
	struct VT_volume_texture *new_texture,*old_texture,*texture;

	ENTER(remesh_texture);
	/*???debug */
	printf("enter remesh_texture\n");
	/* checking arguments */
	if (texture_window)
	{
		/* delete all node groups */
		if (texture=texture_window->current_texture)
		{
			if (texture->n_groups>0)
			{
				for (i=0;i<texture->n_groups;i++)
				{
					strcpy(texture_window->current_node_group,
						(texture->node_groups[i])->name);
					remove_vt_node_group(texture_window);
				}
				texture_window->current_texture->n_groups=0;
			}
		}
		old_texture=texture_window->current_texture;
		/* step 1 - allocate memory for new texture structure */
		/*???DB.  Check allocation */
		ALLOCATE(new_texture,struct VT_volume_texture,1);
		new_texture->index=old_texture->index;
		new_texture->texture_curve_list=old_texture->texture_curve_list;

		new_texture->isovalue=old_texture->isovalue;
		new_texture->calculate_nodal_values=1;
		new_texture->recalculate=1;
		new_texture->n_groups=old_texture->n_groups;
#if defined (OLD_CODE)
		strcpy(new_texture->name,old_texture->name);
#endif /* defined (OLD_CODE) */
		for (i=0;i<3;i++)
		{
			new_texture->ximin[i]=texture_window->current_texture->ximin[i];
			new_texture->ximax[i]=texture_window->current_texture->ximax[i];
			/* get new resolution settings */
			new_dim[i]=new_texture->dimension[i]=(int)texture_window->xires[i];
			old_dim[i]=old_texture->dimension[i];
		}
		n_new_cells=(new_texture->dimension[0])*(new_texture->dimension[1])
			*(new_texture->dimension[2]);
		n_new_nodes=(new_texture->dimension[0]+1)*(new_texture->dimension[1]+1)
			*(new_texture->dimension[2]+1);
/*???debug */
printf("remesh: allocating new fields\n");
		/*???DB.  Check allocation */
		ALLOCATE(new_texture->scalar_field,struct VT_scalar_field,1);
		/*???DB.  Check allocation */
		ALLOCATE(new_texture->clip_field,struct VT_scalar_field,1);
		/*???DB.  Check allocation */
		ALLOCATE(new_texture->clip_field2,struct VT_scalar_field,1);
		/*???DB.  Check allocation */
		ALLOCATE(new_texture->coordinate_field,struct VT_vector_field,1);
		for (i=0;i<3;i++)
		{
			new_texture->scalar_field->dimension[i]=
			new_texture->clip_field->dimension[i]=
			new_texture->clip_field2->dimension[i]=
			new_texture->coordinate_field->dimension[i]=new_texture->dimension[i];
		}
		if (!ALLOCATE(new_texture->scalar_field->scalar,double,
			(new_texture->dimension[0]+1)*(new_texture->dimension[1]+1)*
			(new_texture->dimension[2]+1)))
		{
			display_message(ERROR_MESSAGE,
				"remesh_texture.  Error in malloc scalar field");
			/*???DB.  Remove */
			exit(1);
		}
		if (!ALLOCATE(new_texture->clip_field->scalar,double,
			(new_texture->dimension[0]+1)*(new_texture->dimension[1]+1)*
			(new_texture->dimension[2]+1)))
		{
			display_message(ERROR_MESSAGE,
				"remesh_texture.  Error in malloc clip field");
			/*???DB.  Remove */
			exit(1);
		}
		if (!ALLOCATE(new_texture->clip_field2->scalar,double,
			(new_texture->dimension[0]+1)*(new_texture->dimension[1]+1)*
			(new_texture->dimension[2]+1)))
		{
			display_message(ERROR_MESSAGE,
				"remesh_texture.  Error in malloc clip field2");
			/*???DB.  Remove */
			exit(1);
		}
		if (!ALLOCATE(new_texture->coordinate_field->vector,double,
			3*(new_texture->dimension[0]+1)*(new_texture->dimension[1]+1)*
			(new_texture->dimension[2]+1)))
		{
			display_message(ERROR_MESSAGE,
				"remesh_texture.  Error in malloc coordinate field");
			/*???DB.  Remove */
			exit(1);
		}
		/* do not have to re malloc isosurface at present */
		/*???debug */
		printf("old_texture->mc_iso_surface %p\n",old_texture->mc_iso_surface);


		if (old_texture->mc_iso_surface != NULL)
		{
				new_texture->mc_iso_surface=old_texture->mc_iso_surface;
		}
		else
		{
				ALLOCATE(new_texture->mc_iso_surface, struct MC_iso_surface, 1);
	/*** DODGY ***/    old_texture->mc_iso_surface = new_texture->mc_iso_surface;
		for (i=0;i<3;i++)
				{
				old_texture->mc_iso_surface->dimension[i] = old_dim[i];
				}

				new_texture->mc_iso_surface->mc_cells = NULL;
/*		    new_texture->mc_iso_surface->detail_map = NULL;*/
		}


		/* destroy old mc_cells */
		printf("destroying mc_cells and contents\n");
clean_mc_iso_surface(1+texture_window->cutting_plane_on+texture_window->hollow_mode_on,new_texture->mc_iso_surface);
/*
		if (new_texture->mc_iso_surface->mc_cells != NULL)
		{
				for (i=0;i<(old_texture->mc_iso_surface->dimension[0]+2)*
				(old_texture->mc_iso_surface->dimension[1]+2)*(old_texture->mc_iso_surface->dimension[2]+2);i++)
				{
			if (new_texture->mc_iso_surface->mc_cells[i] != NULL)
					{
					if (new_texture->mc_iso_surface->mc_cells[i]->triangle_list != NULL)
					{
				destroy_mc_triangle_list(new_texture->mc_iso_surface->mc_cells[i],
						new_texture->mc_iso_surface->mc_cells[i]->triangle_list,
						new_texture->mc_iso_surface->mc_cells[i]->n_triangles,
						new_texture->mc_iso_surface->n_scalar_fields,
						new_texture->mc_iso_surface);
					}
					new_texture->mc_iso_surface->mc_cells[i]->triangle_list = NULL;
					new_texture->mc_iso_surface->mc_cells[i]->n_triangles = NULL;
					DEALLOCATE(new_texture->mc_iso_surface->mc_cells[i]);
					new_texture->mc_iso_surface->mc_cells[i] = NULL;
					}
				}
				DEALLOCATE(new_texture->mc_iso_surface->mc_cells[i]);
				new_texture->mc_iso_surface->n_vertices = 0;
				new_texture->mc_iso_surface->n_triangles = 0;
				new_texture->mc_iso_surface->compiled_vertex_list=NULL;
				new_texture->mc_iso_surface->compiled_triangle_list=NULL;

				DEALLOCATE(new_texture->mc_iso_surface->mc_cells);
		}
*/
		if (ALLOCATE(new_texture->mc_iso_surface->mc_cells, struct MC_cell *, (new_dim[0]+3)*(new_dim[1]+3)*(new_dim[2]+3)))
		{
				for (i=0;i< (new_dim[0]+3)*(new_dim[1]+3)*(new_dim[2]+3);i++)
				{
			new_texture->mc_iso_surface->mc_cells[i] = NULL;
				}
				for (i=0;i<3;i++)
				{
			new_texture->mc_iso_surface->dimension[i]=new_dim[i];
				}
		}
		else
		{
				display_message(ERROR_MESSAGE,
				"remesh_texture.  Error in malloc mc_cells");
		}
		if (new_texture->mc_iso_surface->detail_map != NULL)
		{
			DEALLOCATE(new_texture->mc_iso_surface->detail_map)
		}
/*???debug */
printf("after reallocating mc_cells\n");
printf("alloc detail map %d+3 %d+3 %d+3 = %d\n",new_dim[0],new_dim[1],
	new_dim[2],(new_dim[0]+3)*(new_dim[1]+3)*(new_dim[2]+3));
		if (ALLOCATE(new_texture->mc_iso_surface->detail_map,int,
			(new_dim[0]+3)*(new_dim[1]+3)*(new_dim[2]+3)))
		{
			for (i=0;i<(new_dim[0]+3)*(new_dim[1]+3)*(new_dim[2]+3);i++)
			{
				(new_texture->mc_iso_surface->detail_map)[i]=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"remesh_texture.  Could not allocate detail_map");
		}
		ALLOCATE(new_texture->global_texture_node_list,struct VT_texture_node *,
			n_new_nodes+1);
		if (!new_texture->global_texture_node_list)
		{
			display_message(ERROR_MESSAGE,
				"remesh_texture.  Malloc_failed at glob text list");
			exit(1);
		}
		ALLOCATE(new_texture->texture_cell_list,struct VT_texture_cell *,
			n_new_cells+1);
		if (!new_texture->texture_cell_list)
		{
			display_message(ERROR_MESSAGE,
				"remesh_texture.  Malloc_failed at text cell list");
			/*???DB.  Remove */
			exit(1);
		}
		material=texture_window->default_graphical_material;
		env_map=NULL;
		for (i=0;i<n_new_cells;i++)
		{
			ALLOCATE(cell_ptr,struct VT_texture_cell,1);
			if (!cell_ptr)
			{
				display_message(ERROR_MESSAGE,
					"remesh_texture.  Malloc failed at cell ptr");
				/*???DB.  Remove */
				exit(1);
			}
			new_texture->texture_cell_list[i]=cell_ptr;
			/*???DB.  Change ACCESS ? */
			if (material)
			{
				cell_ptr->material=ACCESS(Graphical_material)(material);
			}
			else
			{
				cell_ptr->material=(struct Graphical_material *)NULL;
			}
			if (env_map)
			{
				cell_ptr->env_map=ACCESS(Environment_map)(env_map);
			}
			else
			{
				cell_ptr->env_map=(struct Environment_map *)NULL;
			}
			cell_ptr->cop[0]=cell_ptr->cop[1]=cell_ptr->cop[2]=0.5;
			cell_ptr->scalar_value=0.0;
			cell_ptr->detail=0;
		}
		for (i=0;i<n_new_nodes;i++)
		{
			ALLOCATE(node_ptr,struct VT_texture_node,1);
			if (!node_ptr)
			{
				display_message(ERROR_MESSAGE,
					"remesh_texture.  Malloc failed at node ptr");
				/*???DB.  Remove */
				exit(1);
			}
			new_texture->global_texture_node_list[i]=node_ptr;
			node_ptr->material=ACCESS(Graphical_material)(material);
			node_ptr->dominant_material=ACCESS(Graphical_material)(material);
			node_ptr->scalar_value=1.0;
			node_ptr->active=0.0;
			node_ptr->cop[0]=0.5;
			node_ptr->cop[1]=0.5;
			node_ptr->cop[2]=0.5;
			node_ptr->node_type=0;
			for (j=0;j<8;j++)
			{
				node_ptr->cm_node_identifier[j]=i;
			}
		}
		/* step 2 - decide if increasing or decreasing resolution */
		if ((texture_window->xires[0]>old_texture->dimension[0])||
		(texture_window->xires[1]>old_texture->dimension[1])||
		(texture_window->xires[2]>old_texture->dimension[2]))
		{
			/* resolution increased */
			/* step 3 - read existing data and use to fill new texture */
			for (i=0;i<new_dim[0];i++)
			{
				for (j=0;j<new_dim[1];j++)
				{
					for (k=0;k<new_dim[2];k++)
					{
						/* will need connectivity formula for cell/node */
						new_cell=new_texture->texture_cell_list[
							i+j*new_dim[0]+k*new_dim[0]*new_dim[1]];
						/* work out corresponding old cell */
						old_i=(int)((double)((double)old_dim[0]/(double)new_dim[0])*
							(double)i);
						old_j=(int)((double)((double)old_dim[1]/(double)new_dim[1])*
							(double)j);
						old_k=(int)((double)((double)old_dim[2]/(double)new_dim[2])*
							(double)k);
						if (old_i >= old_dim[0]||old_j >= old_dim[1]||old_k >= old_dim[2])
						{
							display_message(ERROR_MESSAGE,
								"remesh_texture.  Old index > dimension");
							exit(1);
						}
						old_cell=old_texture->texture_cell_list[
							old_i+old_j*old_dim[0]+old_k*old_dim[0]*old_dim[1]];
						/* assign new values from old values here - could be function */
							/*???DB.  Change ACCESS ? */
						if (old_cell->material)
						{
							new_cell->material=ACCESS(Graphical_material)(old_cell->material);
						}
						else
						{
							new_cell->material=(struct Graphical_material *)NULL;
						}
						if (old_cell->env_map)
						{
							new_cell->env_map=ACCESS(Environment_map)(old_cell->env_map);
						}
						else
						{
							new_cell->env_map=(struct Environment_map *)NULL;
						}
						new_cell->cop[0]=old_cell->cop[0];
						new_cell->cop[1]=old_cell->cop[1];
						new_cell->cop[2]=old_cell->cop[2];
						new_cell->scalar_value=old_cell->scalar_value;
						new_cell->detail=old_cell->detail;
					}
				}
			}
			/* same for nodes */
			for (i=0;i<new_dim[0]+1;i++)
			{
				for (j=0;j<new_dim[1]+1;j++)
				{
					for (k=0;k<new_dim[2]+1;k++)
					{
						/* will need connectivity formula for cell/node */
						new_node=new_texture->global_texture_node_list[
						i+j*(new_dim[0]+1)+k*(new_dim[0]+1)*(new_dim[1]+1)];
						/* work out corresponding old cell */
						old_i=(int)((double)((double)(old_dim[0]+1)/(double)(new_dim[0]+1))*
							(double)i);
						old_j=(int)((double)((double)(old_dim[1]+1)/(double)(new_dim[1]+1))*
							(double)j);
						old_k=(int)((double)((double)(old_dim[2]+1)/(double)(new_dim[2]+1))*
							(double)k);
						if (old_i >= old_dim[0]+1||old_j >= old_dim[1]+1||old_k >=
							old_dim[2]+1)
						{
							display_message(ERROR_MESSAGE,
								"remesh_texture.  Old index > dimension");
							exit(1);
						}
						old_node=old_texture->global_texture_node_list[
							old_i+old_j*(old_dim[0]+1)+old_k*(old_dim[0]+1)*(old_dim[1]+1)];
						/* assign new values from old values here - could be function */
						new_node->material=ACCESS(Graphical_material)(old_node->material);
						new_node->dominant_material=ACCESS(Graphical_material)(
							old_node->dominant_material);
						new_node->cop[0]=old_node->cop[0];
						new_node->cop[1]=old_node->cop[1];
						new_node->cop[2]=old_node->cop[2];
						new_node->active=old_node->active;
						new_node->node_type=0;
						for (m=0;m<8;m++)
						{
							new_node->cm_node_identifier[m]=
								i+j*(new_dim[0]+1)+k*(new_dim[0]+1)*(new_dim[1]+1);
						}
						new_node->scalar_value=old_node->scalar_value;
					}
				}
			}
		}
		else
		{
			if ((texture_window->xires[0]<old_texture->dimension[0])||
				(texture_window->xires[1]<old_texture->dimension[1])||
				(texture_window->xires[2]<old_texture->dimension[2]))
			{
				/* resolution decreased */
				/* step 3 - read existing data and use to fill new texture */
				for (i=0;i<new_dim[0];i++)
				{
					for (j=0;j<new_dim[1];j++)
					{
						for (k=0;k<new_dim[2];k++)
						{
							/* will need connectivity formula for cell/node */
							new_cell=new_texture->texture_cell_list[
								i+j*new_dim[0]+k*new_dim[0]*new_dim[1]];
							/* work out extent of new cell in terms of old cell indices */
							/* start point */
							is=((double)((double)old_dim[0]/(double)new_dim[0])*(double)i);
							js=((double)((double)old_dim[1]/(double)new_dim[1])*(double)j);
							ks=((double)((double)old_dim[2]/(double)new_dim[2])*(double)k);
							int_is=(int)is;
							int_js=(int)js;
							int_ks=(int)ks;
							/* end point */
							ie=((double)((double)old_dim[0]/(double)new_dim[0])*
								(double)(i+1));
							je=((double)((double)old_dim[1]/(double)new_dim[1])*
								(double)(j+1));
							ke=((double)((double)old_dim[2]/(double)new_dim[2])*
								(double)(k+1));
							int_ie=(int)ie;
							int_je=(int)je;
							int_ke=(int)ke;
							/* scan through old cells and add volume contribution to
								material */
							number_of_distinct_materials=0;
							for (l=int_is;l<=int_ie;l++)
							{
								for (m=int_js;m<=int_je;m++)
								{
									for (n=int_ks;n<=int_ke;n++)
									{
										/* calculate amount of old cell in new cell range */
										weight=volume_intersect(l,l+1,m,m+1,n,n+1,
											is,ie,js,je,ks,ke );
										if (weight>0)
										{
											index=number_of_distinct_materials-1;
											if ((old_texture->texture_cell_list[
												l+old_dim[0]*m+old_dim[0]*old_dim[1]*n])->material)
											{
												material=(old_texture->texture_cell_list[
													l+old_dim[0]*m+old_dim[0]*old_dim[1]*n])->material;
#if defined (OLD_CODE)
												env_map=(old_texture->texture_cell_list[
													l+old_dim[0]*m+old_dim[0]*old_dim[1]*n])->env_map;
												for (s=0;s<3;s++)
												{
													cop[s]=(old_texture->texture_cell_list[
													l+old_dim[0]*m+old_dim[0]*old_dim[1]*n])->cop[s];
												}
#endif /* defined (OLD_CODE) */
											}
											else
											{
												material=NULL;
#if defined (OLD_CODE)
												env_map=NULL;
#endif /* defined (OLD_CODE) */
											}
											while ((index>=0)&&(material!=materials[index]))
											{
												index--;
											}
											if (index>0)
											{
												material_weight[index] += weight;
											}
											else
											{
												materials[number_of_distinct_materials]=material;
												material_weight[number_of_distinct_materials]=weight;
												number_of_distinct_materials++;
											}
										}
									}
								}
							}
							/* assign values from old values here - could be function */
							/* run through array & find highest count  */
							if (number_of_distinct_materials>0)
							{
								max=0;
								for (l=1;l<number_of_distinct_materials;l++)
								{
									if (material_weight[l]>material_weight[max])
									{
										max=l;
									}
								}
								new_cell->material=ACCESS(Graphical_material)(materials[max]);
								/*???MS.  this cop stuff is just a hack and hasnt been given any
									intelligent consideration */
								for (s=0;s<3;s++)
								{
									new_cell->cop[s]=cop[s];
								}
							}
							else
							{
								new_cell->material=(struct Graphical_material *)NULL;
							}
							/* environment maps */
							number_of_distinct_env_maps=0;
							for (l=int_is;l<=int_ie;l++)
							{
								for (m=int_js;m<=int_je;m++)
								{
									for (n=int_ks;n<=int_ke;n++)
									{
										/* calculate amount of old cell in new cell range */
										weight=volume_intersect(l,l+1,m,m+1,n,n+1,is,ie,js,je,ks,
											ke);
										if (weight>0)
										{
											index=number_of_distinct_env_maps-1;
											if ((old_texture->texture_cell_list[l+old_dim[0]*m+
												old_dim[0]*old_dim[1]*n])->env_map)
											{
												env_map=(old_texture->texture_cell_list[l+old_dim[0]*m+
													old_dim[0]*old_dim[1]*n])->env_map;
												for (s=0;s<3;s++)
												{
													cop[s]=(old_texture->texture_cell_list[l+old_dim[0]*m+
														old_dim[0]*old_dim[1]*n])->cop[s];
												}
											}
											else
											{
												env_map=NULL;
												cop[0]=0.5;
												cop[1]=0.5;
												cop[2]=0.5;
											}
											while ((index>=0)&&(env_map!=env_maps[index]))
											{
												index--;
											}
											if (index>0)
											{
												env_map_weight[index] += weight;
											}
											else
											{
												env_maps[number_of_distinct_env_maps]=env_map;
												for (s=0;s<3;s++)
												{
													cops[number_of_distinct_env_maps][s]=cop[s];
												}
												env_map_weight[number_of_distinct_env_maps]=weight;
												number_of_distinct_env_maps++;
											}
										}
									}
								}
							}
							/* assign values from old values here - could be function */
							/* run through array & find highest count  */
							if (number_of_distinct_env_maps>0)
							{
								max=0;
								for (l=1;l<number_of_distinct_env_maps;l++)
								{
									if (env_map_weight[l]>env_map_weight[max])
									{
										max=l;
									}
								}
								new_cell->env_map=env_maps[max];
								for (s=0;s<3;s++)
								{
									new_cell->cop[s]=cops[max][s];
								}
								/*???MS.  this cop stuff is just a hack and hasnt been given any
									intelligent consideration */
							}
							else
							{
								new_cell->env_map=(struct Environment_map *)NULL;
								for (s=0;s<3;s++)
								{
									new_cell->cop[s]=0.5;
								}
							}
							/* add scalar contribution */
							total_scalar_value=0.0;
							for (l=int_is;l<=int_ie;l++)
							{
								for (m=int_js;m<=int_je;m++)
								{
									for (n=int_ks;n<=int_ke;n++)
									{
										/* calculate amount of old cell in new cell range */
										weight=volume_intersect(l,l+1,m,m+1,n,n+1,
											is,ie,js,je,ks,ke );
										if (weight>0)
										{
											total_scalar_value += (old_texture->texture_cell_list[
												l+old_dim[0]*m+old_dim[0]*old_dim[1]*n])->
												scalar_value*weight;
										}
									}
								}
							}
							if (total_scalar_value >= 0.5) /* set isovalues 0 or 1 */
							{
								new_cell->scalar_value=1.0;
							}
							else
							{
								new_cell->scalar_value=0.0;
							}
							total_detail=0.0;
							for (l=int_is;l<=int_ie;l++)
							{
								for (m=int_js;m<=int_je;m++)
								{
									for (n=int_ks;n<=int_ke;n++)
									{
										/* calculate amount of old cell in new cell range */
										weight=volume_intersect(l,l+1,m,m+1,n,n+1,
											is,ie,js,je,ks,ke );
										if (weight>0)
										{
											total_detail += ((old_texture->texture_cell_list[
												l+old_dim[0]*m+old_dim[0]*old_dim[1]*n])->
												detail*weight);
										}
									}
								}
							}
							new_cell->detail = (int) total_detail;
						} /* k */
					}
				}
				/* do same for nodes */
				for (i=0;i<new_dim[0]+1;i++)
				{
					for (j=0;j<new_dim[1]+1;j++)
					{
						for (k=0;k<new_dim[2]+1;k++)
						{
							/* will need connectivity formula for cell/node */
							new_node=new_texture->global_texture_node_list[
								i+j*(new_dim[0]+1)+k*(new_dim[0]+1)*(new_dim[1]+1)];
							/* work out extent of new cell in terms of old cell indices */
							/* start point */
							is=((double)((double)(old_dim[0]+1)/(double)(new_dim[0]+1))*
								(double)i);
							js=((double)((double)(old_dim[1]+1)/(double)(new_dim[1]+1))*
								(double)j);
							ks=((double)((double)(old_dim[2]+1)/(double)(new_dim[2]+1))*
								(double)k);
							int_is=(int)is;
							int_js=(int)js;
							int_ks=(int)ks;
							/* end point */
							ie=((double)((double)(old_dim[0]+1)/(double)(new_dim[0]+1))*
								(double)(i+1));
							je=((double)((double)(old_dim[1]+1)/(double)(new_dim[1]+1))*
								(double)(j+1));
							ke=((double)((double)(old_dim[2]+1)/(double)(new_dim[2]+1))*
								(double)(k+1));
							int_ie=(int)ie;
							int_je=(int)je;
							int_ke=(int)ke;
							/* scan through old nodes and add volume contribution to
								material */
							number_of_distinct_materials=0;
							for (l=int_is;l<=int_ie;l++)
							{
								for (m=int_js;m<=int_je;m++)
								{
									for (n=int_ks;n<=int_ke;n++)
									{
										/* calculate amount of old cell in new cell range */
										weight=volume_intersect(l,l+1,m,m+1,n,n+1,
											is,ie,js,je,ks,ke );
										if (weight>0)
										{
											index=number_of_distinct_materials-1;
											if ((old_texture->global_texture_node_list[
												l+old_dim[0]*m+old_dim[0]*old_dim[1]*n])->material)
											{
												material=(old_texture->global_texture_node_list[
												l+old_dim[0]*m+old_dim[0]*old_dim[1]*n])->material;
												for (s=0;s<3;s++)
												{
													cop[s]=(old_texture->global_texture_node_list[
													l+old_dim[0]*m+old_dim[0]*old_dim[1]*n])->cop[s];
												}
											}
											else
											{
												material=NULL;
											}
											while ((index>=0)&&(material!=materials[index]))
											{
												index--;
											}
											if (index>0)
											{
												material_weight[index] += weight;
											}
											else
											{
												materials[number_of_distinct_materials]=material;
												material_weight[number_of_distinct_materials]=weight;
												number_of_distinct_materials++;
											}
										}
									}
								}
							}
							/* assign values from old values here - could be function */
							/* run through array & find highest count  */
							if (number_of_distinct_materials>0)
							{
								max=0;
								for (l=1;l<number_of_distinct_materials;l++)
								{
									if (material_weight[l]>material_weight[max])
									{
										max=l;
									}
								}
								new_node->material=ACCESS(Graphical_material)(materials[max]);
								/*???MS.  this may be wrong below... */
								new_node->dominant_material=ACCESS(Graphical_material)(
									materials[max]);
								/* this cop stuff is just a hack and hasnt been given any */
								/*intelligent consideration */
								for (s=0;s<3;s++)
								{
									new_cell->cop[s]=cop[s];
								}
							}
							else
							{
								new_node->material=(struct Graphical_material *)NULL;
								new_node->dominant_material=(struct Graphical_material *)NULL;
							}
						}
					}
				}
			}
		}
		/* step 4 - assign new current texture, remove old texture */
		/* clear scalar fields */
		DEALLOCATE(old_texture->scalar_field->scalar);
		DEALLOCATE(old_texture->clip_field->scalar);
		DEALLOCATE(old_texture->clip_field2->scalar);
		DEALLOCATE(old_texture->coordinate_field->vector);
		DEALLOCATE(old_texture->scalar_field);
		DEALLOCATE(old_texture->clip_field);
		DEALLOCATE(old_texture->clip_field2);
		DEALLOCATE(old_texture->coordinate_field);
		texture_window->current_texture=new_texture;
		/* deallocate old texture contents */
		n_old_cells=old_dim[0]*old_dim[1]*old_dim[2];
		for (i=0;i<n_old_cells;i++)
		{
			DEACCESS(Graphical_material)(&(((old_texture->texture_cell_list)[i])->
				material));
			DEACCESS(Environment_map)(&(((old_texture->texture_cell_list)[i])->
				env_map));
			DEALLOCATE(old_texture->texture_cell_list[i]);
		}
		DEALLOCATE(old_texture->texture_cell_list);
		n_old_nodes=(old_dim[0]+1)*(old_dim[1]+1)*(old_dim[2]+1);
		for (i=0;i<n_old_nodes;i++)
		{
			DEACCESS(Graphical_material)(
				&(((old_texture->global_texture_node_list)[i])->material));
			DEACCESS(Graphical_material)(
				&(((old_texture->global_texture_node_list)[i])->dominant_material));
			DEALLOCATE(old_texture->global_texture_node_list[i]);
		}
		DEALLOCATE(old_texture->global_texture_node_list);
	/* do grid_spacing here */
		if (old_texture->grid_spacing)
		{
				ALLOCATE(new_texture->grid_spacing, double,new_dim[0]+new_dim[1]+new_dim[2]+3);
				for (i=0;i<new_dim[0]+1;i++)
				{
			if (i < old_dim[i]+1)
			{
					new_texture->grid_spacing[i] = old_texture->grid_spacing[i];
			}
			else
			{
					new_texture->grid_spacing[i] = 1;
			}
				}
				for (i=0;i<new_dim[1]+1;i++)
				{
			if (i < old_dim[1]+1)
			{
					new_texture->grid_spacing[i+new_dim[0]+1] = old_texture->grid_spacing[i+old_dim[0]+1];
			}
			else
			{
					new_texture->grid_spacing[i+new_dim[0]+1] = 1;
			}
				}
				for (i=0;i<new_dim[2]+1;i++)
				{
			if (i < old_dim[2]+1)
			{
					new_texture->grid_spacing[i+new_dim[0]+new_dim[1]+2] =
				old_texture->grid_spacing[i+old_dim[0]+old_dim[1]+2];
			}
			else
			{
					new_texture->grid_spacing[i+new_dim[0]+new_dim[1]+2] = 1;
			}
				}


				display_message(WARNING_MESSAGE,"Make sure you re-specify the grid spacings");

				irregular_mesh((Widget) NULL,(XtPointer) texture_window,NULL);


		}
		else
		{
				new_texture->grid_spacing = (double *) NULL;
		}
		DEALLOCATE(old_texture);
		draw_texture_isosurface(texture_window);
/*???debug */
printf("remesh1\n");
		draw_texture_cells(texture_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,"remesh_texture.  Invalid argument(s)");
	}
	/*???debug */
	printf("leave remesh_texture\n");
	LEAVE;
} /* remesh_texture */

static void identify_tb(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
Identifies toggle buttons
==============================================================================*/
{
	int index;
	struct Texture_window *texture_window;

	ENTER(identify_tb);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		XtVaGetValues(w,XmNuserData,&index,NULL);
		texture_window->tb[index]=w;
	}
	else
	{
		display_message(ERROR_MESSAGE,"identify_tb.  Invalid argument(s)");
	}
	LEAVE;
} /* identify_tb */

static void adjustdisplacement(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
==============================================================================*/
{
	ENTER(adjustdisplacement);
	USE_PARAMETER(texture_window);
	LEAVE;
} /* adjustdisplacement */

static void tf_activated(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 20 February 1997

DESCRIPTION :
Updates data when text field activated
==============================================================================*/
{
	char *string_ptr;
	int index;
	struct Texture_window *texture_window;

	ENTER(tf_activated);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		XtVaGetValues(w,XmNuserData,&index,NULL);
		XtVaGetValues(w,XmNvalue,&string_ptr,NULL);
/*???debug */
printf("field(%d) = %f\n",index, atof(string_ptr));
		switch (index)
		{
			case 0:
			{
				process_prompt(texture_window);

			} break;
			case 1:
			{
				texture_window->ximin[0]=(double)atof(string_ptr);
				texture_window->current_texture->ximin[0]=texture_window->ximin[0];
				adjustxi(texture_window,0);
			} break;
			case 5:
			{
				texture_window->ximin[1]=(double)atof(string_ptr);
				texture_window->current_texture->ximin[1]=texture_window->ximin[1];
				adjustxi(texture_window,1);
			} break;
			case 9:
			{
				texture_window->ximin[2]=(double)atof(string_ptr);
				texture_window->current_texture->ximin[2]=texture_window->ximin[2];
				adjustxi(texture_window,2);
			} break;
			case 3:
			{
				texture_window->ximax[0]=(double)atof(string_ptr);
				texture_window->current_texture->ximax[0]=texture_window->ximax[0];
				adjustxi(texture_window,0);
			} break;
			case 7:
			{
				texture_window->ximax[1]=(double)atof(string_ptr);
				texture_window->current_texture->ximax[1]=texture_window->ximax[1];
				adjustxi(texture_window,1);
			} break;
			case 11:
			{
				texture_window->ximax[2]=(double)atof(string_ptr);
				texture_window->current_texture->ximax[2]=texture_window->ximax[2];
				adjustxi(texture_window,2);
			} break;
			case 2:
			{
				texture_window->xival[0]=(double)atof(string_ptr);
				adjustxi(texture_window,0);
			} break;
			case 6:
			{
				texture_window->xival[1]=(double)atof(string_ptr);
				adjustxi(texture_window,1);
			} break;
			case 10:
			{
				texture_window->xival[2]=(double)atof(string_ptr);
				adjustxi(texture_window,2);
			} break;
			case 4:
			{
				texture_window->xires[0]=(double)atof(string_ptr);
				remesh_texture(texture_window);
				adjustxi(texture_window,0);
			} break;
			case 8:
			{
				texture_window->xires[1]=(double)atof(string_ptr);
				remesh_texture(texture_window);
				adjustxi(texture_window,1);
			} break;
			case 12:
			{
				texture_window->xires[2]=(double)atof(string_ptr);
				remesh_texture(texture_window);
				adjustxi(texture_window,2);
			} break;
			case 13:
			{
				texture_window->brushsize[2]=(float)atof(string_ptr);
/*???debug */
printf("Brushsize = %f, %f, %f\n",texture_window
					->brushsize[0],
				texture_window->brushsize[1],texture_window->brushsize[2]);
			} break;
			case 18:
			{
				texture_window->brushsize[0]=(float)atoi(string_ptr);
/*???debug */
printf("Brushsize = %f, %f, %f\n",texture_window
					->brushsize[0],
				texture_window->brushsize[1],texture_window->brushsize[2]);
			} break;
			case 19:
			{
				texture_window->brushsize[1]=(float)atoi(string_ptr);
/*???debug */
printf("Brushsize = %f, %f, %f\n",texture_window
					->brushsize[0],
				texture_window->brushsize[1],texture_window->brushsize[2]);
			} break;
			case 14:
			{
				if (texture_window->cop_mode_on)
				{
					texture_window->cop[0]=(double)atof(string_ptr);
					X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
					graphics_loop((XtPointer)texture_window);
					X3dThreeDDrawingSwapBuffers();
				}
				else
				{
					texture_window->displacement[0]=(double)atof(string_ptr);
					adjustdisplacement(texture_window);
				}
			} break;
			case 15:
			{
				if (texture_window->cop_mode_on)
				{
					texture_window->cop[1]=(double)atof(string_ptr);
					X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
					graphics_loop((XtPointer)texture_window);
					X3dThreeDDrawingSwapBuffers();
				}
				else
				{
					texture_window->displacement[1]=(double)atof(string_ptr);
					adjustdisplacement(texture_window);
				}
			} break;
			case 16:
			{
				if (texture_window->cop_mode_on)
				{
					texture_window->cop[2]=(double)atof(string_ptr);
					X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
					graphics_loop((XtPointer)texture_window);
					X3dThreeDDrawingSwapBuffers();
				}
				else
				{
					texture_window->displacement[2]=(double)atof(string_ptr);
					adjustdisplacement(texture_window);
				}
			} break;
			case 17:
			{
				texture_window->select_value=(double)atof(string_ptr);
/*???debug */
printf("select value = %lf\n",texture_window->select_value);
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"tf_activated.  Invalid argument(s)");
	}
	LEAVE;
} /* tf_activated */

static void adjust_zoom(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
==============================================================================*/
{
	int sb_value;

	ENTER(adjust_zoom);
	/* checking arguments */
	if (texture_window)
	{
		XtVaGetValues(texture_window->zoom_sb,XmNvalue,&sb_value,NULL);
		texture_window->fovy=PI/2.0*(double)sb_value/(double)SLIDERMAX;
	}
	else
	{
		display_message(ERROR_MESSAGE,"adjust_zoom.  Invalid argument(s)");
	}
	LEAVE;
} /* adjust_zoom */

static void adjust_rotation(struct Texture_window *texture_window,int index)
/*******************************************************************************
LAST MODIFIED : 25 September 1996

DESCRIPTION :
==============================================================================*/
{
	int sb_value;
	float newrotation;

	ENTER(adjust_rotation);
	/* checking arguments */
	if (texture_window)
	{
		XtVaGetValues(texture_window->rotate_sb[index],XmNvalue,&sb_value,NULL);
		newrotation=(2.0*PI*(double)sb_value/(double)SLIDERMAX)-texture_window->
			rot_angle[index];
		texture_window->rot_angle[index]=2.0*PI*(double)sb_value/(double)SLIDERMAX;
#if defined (OPENGL_API)
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(texture_window->model_matrix);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		loadmatrix(texture_window->model_matrix);
#endif /* defined (GL_API) */
		switch (index)
		{
			case 0:
			{
#if defined (OPENGL_API)
				glRotatef(180.0/PI*newrotation,1,0,0);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
				rot(180.0/PI*newrotation,'x');
#endif /* defined (GL_API) */
			} break;
			case 1:
			{
#if defined (OPENGL_API)
				glRotatef(180.0/PI*newrotation,0,1,0);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
				rot(180.0/PI*newrotation,'y');
#endif /* defined (GL_API) */
			} break;
			case 2:
			{
#if defined (OPENGL_API)
				glRotatef(180.0/PI*newrotation,0,0,1);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
				rot(180.0/PI*newrotation,'z');
#endif/* defined (GL_API) */
			} break;
		}
#if defined (OPENGL_API)
		glGetFloatv(GL_MODELVIEW_MATRIX,texture_window->model_matrix);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		getmatrix(texture_window->model_matrix);
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"adjust_rotation.  Invalid argument(s)");
	}
	LEAVE;
} /* adjust_rotation */

static void adjust_isovalue(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
==============================================================================*/
{
	int sb_value;
	struct MC_iso_surface *mc_iso_surface;

	ENTER(adjust_isovalue);
	/* checking arguments */
	if (texture_window)
	{
		XtVaGetValues(texture_window->isovalue_sb,XmNvalue,&sb_value,NULL);
		texture_window->isovalue=sb_value/(double)SLIDERMAX;
		texture_window->current_texture->isovalue=texture_window->isovalue;
		/* update iso surface */
		draw_texture_isosurface(texture_window);
		if (mc_iso_surface = texture_window->current_texture->mc_iso_surface)
		{
			mc_iso_surface->active_block[0] = 1-texture_window->closed_surface;
			mc_iso_surface->active_block[1] = mc_iso_surface->dimension[0]+texture_window->closed_surface;
			mc_iso_surface->active_block[2] = 1-texture_window->closed_surface;
			mc_iso_surface->active_block[3] = mc_iso_surface->dimension[1]+texture_window->closed_surface;
			mc_iso_surface->active_block[4] = 1-texture_window->closed_surface;
			mc_iso_surface->active_block[5] = mc_iso_surface->dimension[2]+texture_window->closed_surface;

		draw_texture_isosurface(texture_window);
		}
/*???debug */
printf("Isovalue = %lf\n",texture_window->isovalue);
	}
	else
	{
		display_message(ERROR_MESSAGE,"adjust_isovalue.  Invalid argument(s)");
	}
	LEAVE;
} /* adjust_isovalue */

static void adjust_cutting_plane_sb(struct Texture_window *texture_window,
	int index)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
==============================================================================*/
{
	int value;

	ENTER(adjust_cutting_plane_sb);
	/* checking arguments */
	if (texture_window)
	{
/*???debug */
		XtVaGetValues(texture_window->cutting_plane_sb[index],XmNvalue,&value,NULL);
		texture_window->cutting_plane[index]=((double)value)/(double) SLIDERMAX;
		texture_window->current_texture->cutting_plane[index]=texture_window->
			cutting_plane[index];
		if (texture_window->cutting_plane_on&&texture_window->hollow_mode_on)
		{
			texture_window->current_texture->cut_isovalue=texture_window->
				cutting_plane[3];
		}
		else
		{
			if (texture_window->hollow_mode_on)
			{
				texture_window->current_texture->hollow_isovalue=texture_window->
					cutting_plane[3];
			}
		}
		/* update iso surface */
		draw_texture_isosurface(texture_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"adjust_cutting_plane_sb.  Invalid argument(s)");
	}
	LEAVE;
} /* adjust_cutting_plane_sb */

static void sb_activated(Widget w,XtPointer client_data,
	XmScrollBarCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
Updates data when scrollbar activated
==============================================================================*/
{
	int index,sb_value;
	struct Texture_window *texture_window;

	ENTER(sb_activated);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		XtVaGetValues(w,XmNuserData,&index,NULL);
		XtVaGetValues(w,XmNvalue,&sb_value,NULL);
		switch(index)
		{
			case 1:
			{
				adjustxi_sb(texture_window,0);
			} break;
			case 2:
			{
				adjustxi_sb(texture_window,1);
			} break;
			case 3:
			{
				adjustxi_sb(texture_window,2);
			} break;
			case 12:
			{
				adjust_zoom(texture_window);
			} break;
			case 9:
			{
				adjust_rotation(texture_window,0);
			} break;
			case 10:
			{
				adjust_rotation(texture_window,1);
			} break;
			case 11:
			{
				adjust_rotation(texture_window,2);
			} break;
			case 4:
			{
				adjust_material_sb(texture_window);
			} break;
			case 14:
			{
				adjust_isovalue(texture_window);
			} break;
			case 5:
			{
				adjust_cutting_plane_sb(texture_window,0);
			} break;
			case 6:
			{
				adjust_cutting_plane_sb(texture_window,1);
			} break;
			case 7:
			{
				adjust_cutting_plane_sb(texture_window,2);
			} break;
			case 8:
			{
				adjust_cutting_plane_sb(texture_window,3);
			} break;
		}
		/* temp call expose */
		X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
		graphics_loop((XtPointer)texture_window);
		X3dThreeDDrawingSwapBuffers();
	}
	else
	{
		display_message(ERROR_MESSAGE,"sb_activated.  Invalid argument(s)");
	}
	LEAVE;
} /* sb_activated */

static void initialize(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 9 July 1998

DESCRIPTION :
==============================================================================*/
{
	int i,j,n_cells,n_nodes;
	struct Graphical_material *default_material;
	struct VT_texture_cell *cell_ptr;
	struct VT_texture_node *node_ptr;
	struct VT_volume_texture *texture;
	struct MC_iso_surface *mc_iso_surface;

	ENTER(initialize);
	/* checking arguments */
	if (texture_window)
	{
		/* initialize texture data structure */
/*???debug */
printf("initializing texture\n");
		/*???DB.  Check allocation */
		/*???DB.  This should be create_VT_volume_texture */
		ALLOCATE(texture,struct VT_volume_texture,1);
		texture_window->current_texture=texture;
		texture->grid_spacing = NULL;
		texture->hollow_mode_on=0;
		texture->closed_surface=0;
		texture->decimation=0;
		texture->calculate_nodal_values=1;
		texture->recalculate=1;
		texture->cutting_plane_on=0;
		texture->cut_isovalue=0;
		texture->hollow_isovalue=0.75;
		texture->n_groups=0;
		texture->node_groups=NULL;
		texture->cutting_plane[0]=texture->cutting_plane[1]=texture->
			cutting_plane[2]=texture->cutting_plane[3]=0;
		texture->index=1;
		texture->name="fred";
		for (i=0;i<3;i++)
		{
			texture->ximin[i]=0;
			texture->ximax[i]=1;
			texture->dimension[i]=1; /* same as res */
		}
		n_cells=(texture->dimension[0])*(texture->dimension[1])*(texture->
			dimension[2]);
		n_nodes=(texture->dimension[0]+1)*(texture->dimension[1]+1)
			*(texture->dimension[2]+1);
/*???debug */
printf("Initialize ncells = %d, nnodes = %d\n",n_cells,n_nodes);
		/*???DB.  Check allocation */
		ALLOCATE(texture->texture_curve_list,struct VT_texture_curve *,1);
		*(texture->texture_curve_list)=(struct VT_texture_curve *)NULL;
		/*???DB.  Check allocation */
		ALLOCATE(texture->scalar_field,struct VT_scalar_field,1);
		/*???DB.  Check allocation */
		ALLOCATE(texture->clip_field,struct VT_scalar_field,1);
		/*???DB.  Check allocation */
		ALLOCATE(texture->clip_field2,struct VT_scalar_field,1);
		/*???DB.  Check allocation */
		ALLOCATE(texture->coordinate_field,struct VT_vector_field,1);
		for (i=0;i<3;i++)
		{
			texture->scalar_field->dimension[i]=
			texture->clip_field->dimension[i]=
			texture->coordinate_field->dimension[i]=texture->dimension[i];
		}
		if (!ALLOCATE(texture->scalar_field->scalar,double,(texture->dimension[0]+1)
			*(texture->dimension[1]+1)*(texture->dimension[2]+1)))
		{
			display_message(ERROR_MESSAGE,
				"initialize.  Error in malloc scalar field");
			/*???DB.  Remove */
			exit(1);
		}
/*???debug */
printf("alloc detail map %d\n", (texture->dimension[0]+3)*
	(texture->dimension[1]+3)*(texture->dimension[2]+3));
		if (!ALLOCATE(texture->mc_iso_surface,struct MC_iso_surface,1)||
			!ALLOCATE(texture->mc_iso_surface->mc_cells,struct MC_cell *,
			(texture->dimension[0]+3)*(texture->dimension[1]+3)*
			(texture->dimension[2]+3))||!ALLOCATE(texture->mc_iso_surface->detail_map,
			int,(texture->dimension[0]+3)*(texture->dimension[1]+3)*
			(texture->dimension[2]+3)))
		{
			display_message(ERROR_MESSAGE,
				"initialize.  Error in malloc mc_cells field");
		}
		else
		{
		printf("allocated mc_iso_surface and cells\n");
				for (i=0;i<(texture->dimension[0]+3) *
			(texture->dimension[1]+3)*(texture->dimension[2]+3);i++)
				{
					texture->mc_iso_surface->mc_cells[i] = NULL;
					texture->mc_iso_surface->detail_map[i] = 0;
				}
				for (i=0;i<3;i++)
				{
			texture->mc_iso_surface->dimension[i] = texture->dimension[i];
			printf("mc_dimension[%d] = %d\n", i, texture->dimension[i]);
				}
				texture->mc_iso_surface->n_scalar_fields = 0;
				texture->mc_iso_surface->n_triangles = 0;
				texture->mc_iso_surface->n_vertices = 0;
				texture->mc_iso_surface->compiled_vertex_list = NULL;
				texture->mc_iso_surface->compiled_triangle_list = NULL;
				texture->mc_iso_surface->deform = NULL;
				mc_iso_surface = texture->mc_iso_surface;
				/* allocate active block */
			mc_iso_surface->active_block[0] = 1-texture_window->closed_surface;
			mc_iso_surface->active_block[1] = mc_iso_surface->dimension[0]+texture_window->closed_surface;
			mc_iso_surface->active_block[2] = 1-texture_window->closed_surface;
			mc_iso_surface->active_block[3] = mc_iso_surface->dimension[1]+texture_window->closed_surface;
			mc_iso_surface->active_block[4] = 1-texture_window->closed_surface;
			mc_iso_surface->active_block[5] = mc_iso_surface->dimension[2]+texture_window->closed_surface;

		}
		if (!ALLOCATE(texture->clip_field->scalar,double,(texture->dimension[0]+1)*
			(texture->dimension[1]+1)*(texture->dimension[2]+1)))
		{
			display_message(ERROR_MESSAGE,"initialize.  Error in malloc clip field");
			/*???DB.  Remove */
			exit(1);
		}
		if (!ALLOCATE(texture->clip_field2->scalar,double,(texture->dimension[0]+1)*
			(texture->dimension[1]+1)*(texture->dimension[2]+1)))
		{
			display_message(ERROR_MESSAGE,"initialize.  Error in malloc clip field1");
			/*???DB.  Remove */
			exit(1);
		}
		if (!ALLOCATE(texture->coordinate_field->vector,double,
			3*(texture->dimension[0]+1)*(texture->dimension[1]+1)*
			(texture->dimension[2]+1)))
		{
			display_message(ERROR_MESSAGE,
				"initialize.  Error in malloc coordinate field");
			/*???DB.  Remove */
			exit(1);
		}
		/*???DB.  Check allocation */

		/*???DB.  Check allocation */
		ALLOCATE(texture->global_texture_node_list,struct VT_texture_node *,
			n_nodes+1);
		/*???DB.  Check allocation */
		ALLOCATE(texture->texture_cell_list,struct VT_texture_cell *,n_cells+1);
		default_material=texture_window->default_graphical_material;
		texture_window->current_material=default_material;
		for (i=0;i<n_cells;i++)
		{
			/*???DB.  Check allocation */
			ALLOCATE(cell_ptr,struct VT_texture_cell,1);
			texture->texture_cell_list[i]=cell_ptr;
			cell_ptr->material=ACCESS(Graphical_material)(default_material);
			cell_ptr->env_map=NULL;
/*???debug */
printf("default_material = %p\n",cell_ptr->material);
			cell_ptr->cop[0]=cell_ptr->cop[1]=cell_ptr->cop[2]=0.5;
			/* isosurface values */
			cell_ptr->scalar_value=0 /*1.0*/;
			cell_ptr->detail=0;
		}
		for (i=0;i<n_nodes;i++)
		{
			/*???DB.  Check allocation */
			ALLOCATE(node_ptr,struct VT_texture_node,1);
			texture->global_texture_node_list[i]=node_ptr;
			node_ptr->material=ACCESS(Graphical_material)(default_material);
			node_ptr->dominant_material=ACCESS(Graphical_material)(default_material);
			node_ptr->cop[0]=node_ptr->cop[1]=node_ptr->cop[2]=0.5;
			/* isosurface values */
			node_ptr->scalar_value=1.0;
			node_ptr->active=0;
			node_ptr->node_type=0;
			for (j=0;j<8;j++)
			{
				node_ptr->cm_node_identifier[j]=i;
			}
			for (j=0;j<3;j++)
			{
				node_ptr->scalar_gradient[j]=0.0;
			}
		}
#if defined (OLD_CODE)
/* not used */
		/* !!!! this is probably unnecssary */
		/* establish connectivity info fn goes here (more complicated than this) */
		for (i=0;i<n_cells;i++)
		{
			for (j=0;j<n_nodes;j++)
			{
				cell_ptr=texture_window->current_texture->texture_cell_list[i];
				cell_ptr->texture_node_list=texture_window->current_texture->
					global_texture_node_list[j+n_nodes*i]; /* ?? */
			}
		}
#endif /* defined (OLD_CODE) */
		/* open graphics & set graphics workproc going */
		for (i=0;i<3;i++)
		{
			adjustxi(texture_window,i);
		}
/*???debug */
printf("b4 create_texture_graphics\n");
		create_texture_graphics(texture_window);
#if defined (OLD_CODE)
		texture_window->workproc_id=0;
		texture_window->workproc_id=XtAppAddWorkProc(application_context,
			/*???DB.  application_context should be passed ? */
			graphics_loop,texture_window);
		/*???DB.  all_graphical_materials no longer exists */
		XtVaSetValues(texture_window->material_sb,XmNvalue,
			number_in_Graphical_material_list(all_graphical_materials),
			XmNsliderSize,1,XmNminimum,1,NULL);
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"initialise.  Invalid argument(s)");
	}
	LEAVE;
} /* initialize */

static void exit_window(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 9 July 1998

DESCRIPTION :
pops down & destroys window , frees memory.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(exit_window);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		XtPopdown(texture_window->texture_window_shell);
		XtDestroyWidget(texture_window->texture_window_shell);
#if defined (OLD_CODE)
		XtRemoveWorkProc(texture_window->workproc_id);
#endif /* defined (OLD_CODE) */
		close_texture_graphics(texture_window);
		DEALLOCATE(texture_window);
	}
	else
	{
		display_message(ERROR_MESSAGE,"exit_window.  Invalid argument(s)");
	}
	LEAVE;
} /* exit_window */

static void toggle_node_cell(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
Toggles whether node or cell based editing applies if converting from cell to
node we must effectively increase the resolution by one and reset limits so that
full range values can be obtained.
==============================================================================*/
{
	int i;
	struct Texture_window *texture_window;

	ENTER(toggle_node_cell);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->cell_mode= !(texture_window->cell_mode);
/*???debug */
if (texture_window->cell_mode)
{
	printf("cell mode\n");
}
else
{
	printf("node mode\n");
}
		for (i=0;i<3;i++)
		{
			adjustxi(texture_window,i);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_node_cell.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_node_cell */

static void toggle_paint_mode(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether painting or moving applies. In paint mode the current cell &
each cell traversed are filled with the current texture window material
Choosing paint mode automatically turns delete mode off
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_paint_mode);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->paint_mode=!texture_window->paint_mode;
		if (texture_window->paint_mode)
		{
/*???debug */
printf("paint mode on\n");
			paint_cell(texture_window);
		}
/*???debug */
else
{
	printf("paint mode off\n");
}
		XtVaSetValues(texture_window->tb[3],XmNset,False,NULL);
		XtVaSetValues(texture_window->tb[13],XmNset,False,NULL);
		texture_window->delete_mode=0;
		texture_window->delete_paint_mode=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_paint_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_paint_mode */

static void toggle_fill_mode(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether filling or moving applies. In fill mode the current cell &
each cell traversed are filled with 'clay'
Choosing paint mode automatically turns delete mode off
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_fill_mode);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->fill_mode= !texture_window->fill_mode;
		if ((texture_window->fill_mode)&&(texture_window->cell_mode))
		{
/*???debug */
printf("fill mode on\n");
			fill_cell(texture_window);
		}
		else
		{
			if ((texture_window->fill_mode)&&!(texture_window->cell_mode))
			{
				fill_node(texture_window);
			}
/*???debug */
else
{
	printf("fill mode off\n");
}
		}
		XtVaSetValues(texture_window->tb[3],XmNset,False,NULL);
		XtVaSetValues(texture_window->tb[13],XmNset,False,NULL);
		texture_window->delete_mode=0;
		texture_window->delete_paint_mode=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_fill_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_fill_mode */

static void toggle_detail(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
toggles detail
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_fill_mode);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->detail_mode=!texture_window->detail_mode;
		if ((texture_window->detail_mode)&&(texture_window->cell_mode))
		{
/*???debug */
printf("detail mode on\n");
			detail_cell(texture_window);
		}
		texture_window->delete_mode=0;
		texture_window->delete_paint_mode=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_detail.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_detail */

static void toggle_delete_mode(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether delete mode on or off. In delete mode the current cell &
each cell traversed are deleted
Paint mode is automatically turned off
This is independent of delete_paint mode
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_delete_mode);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->delete_mode=!texture_window->delete_mode;
		if ((texture_window->delete_mode)&&(texture_window->cell_mode))
		{
/*???debug */
printf("delete mode on\n");
			delete_cell(texture_window);
		}
		else
		{
			if ((texture_window->delete_mode)&&!(texture_window->cell_mode))
			{
				delete_active_node(texture_window);
			}
/*???debug */
else
{
	printf("delete mode off\n");
}
		}
		XtVaSetValues(texture_window->tb[0],XmNset,False,NULL);
		XtVaSetValues(texture_window->tb[2],XmNset,False,NULL);
		texture_window->paint_mode=0;
		texture_window->fill_mode=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_delete_mode.  Invalid argument(s)");
	}
	LEAVE;

} /* toggle_delete_mode */

static void toggle_delete_paint_mode(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
Deletes paint but not fill
Fill mode is automaticaly turned off
This is indepenedent of delete [fill] mode
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_delete_paint_mode);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->delete_paint_mode=!texture_window->delete_paint_mode;
		if (texture_window->delete_mode)
		{
/*???debug */
printf("delete paint mode on\n");
			delete_paint(texture_window);
		}
/*???debug */
else
{
	printf("delete paint mode off\n");
}
		XtVaSetValues(texture_window->tb[2],XmNset,False,NULL);
		texture_window->paint_mode=0;
		XtVaSetValues(texture_window->tb[0],XmNset,False,NULL);
		texture_window->fill_mode=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"toggle_delete_paint_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_delete_paint_mode */

static void toggle_auto(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether automatic recalculation of isosurface/cubes is on.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_auto);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->auto_on= !texture_window->auto_on;
/*???debug */
if (texture_window->auto_on)
{
	printf("auto mode on\n");
}
else
{
	printf("auto mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_auto.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_auto */

static void toggle_pick(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
toggles picking mode
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_pick);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->pick_mode= !texture_window->pick_mode;
/*???debug */
if (texture_window->pick_mode)
{
	printf("pick mode on\n");
}
else
{
	printf("pick mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_pick.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_pick */

static void toggle_grid(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether grid mesh is on.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_grid);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->grid_on= !texture_window->grid_on;
/*???debug */
if (texture_window->grid_on)
{
	printf("grid mode on\n");
}
else
{
	printf("grid mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_grid.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_grid */

static void toggle_cubes(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether cubes are displayed.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_cubes);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->cubes_on= !texture_window->cubes_on;
/*???debug */
if (texture_window->cubes_on)
{
	printf("cubes mode on\n");
}
else
{
	printf("cubes mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_cubes.  Invalid argument(s)");
	}
	LEAVE;

} /* toggle_cubes */

static void toggle_isosurface(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether isosurface is displayed.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_isosurface);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->isosurface_on= !texture_window->isosurface_on;
/*???debug */
if (texture_window->isosurface_on)
{
	printf("isosurface mode on\n");
}
else
{
	printf("isosurface mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_isosurface.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_isosurface */

static void toggle_closed_surface(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether closed surface generated.
==============================================================================*/
{
	struct Texture_window *texture_window;
	struct MC_iso_surface *mc_iso_surface;

	ENTER(toggle_closed_surface);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->closed_surface= !texture_window->closed_surface;
		texture_window->current_texture->closed_surface=texture_window->
			closed_surface;

		clean_mc_iso_surface(1+texture_window->hollow_mode_on+texture_window->cutting_plane_on,
			texture_window->current_texture->mc_iso_surface);
		mc_iso_surface = texture_window->current_texture->mc_iso_surface;
		mc_iso_surface->active_block[0] = 1-texture_window->closed_surface;
		mc_iso_surface->active_block[1] = mc_iso_surface->dimension[0]+texture_window->closed_surface;
		mc_iso_surface->active_block[2] = 1-texture_window->closed_surface;
		mc_iso_surface->active_block[3] = mc_iso_surface->dimension[1]+texture_window->closed_surface;
		mc_iso_surface->active_block[4] = 1-texture_window->closed_surface;
		mc_iso_surface->active_block[5] = mc_iso_surface->dimension[2]+texture_window->closed_surface;

/*???debug */
if (texture_window->closed_surface)
{
	printf("closed_surface mode on\n");

}
else
{
	printf("closed_surface mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"toggle_closed_surface.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_closed_surface */

static void toggle_see_paint(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether all paint area cubes are displayed.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_see_paint);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->see_paint_on=!texture_window->see_paint_on;
/*???debug */
if (texture_window->see_paint_on)
{
	printf("see_paint mode on\n");
}
else
{
	printf("see_paint mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_see_paint.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_see_paint */

static void toggle_shaded_surfaces(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether shaded surface generated.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_shaded_surfaces);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->shaded_surfaces= !texture_window->shaded_surfaces;
/*???debug */
if (texture_window->shaded_surfaces)
{
	printf("shaded_surface mode on\n");
}
else
{
	printf("shaded_surface mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"toggle_shaded_surfaces.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_shaded_surfaces */

static void toggle_decimation(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether triangle decimation used
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_decimation);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->decimation= !texture_window->decimation;
		texture_window->current_texture->decimation=texture_window->decimation;
/*???debug */
if (texture_window->decimation)
{
	printf("decimation mode on\n");
}
else
{
	printf("decimation mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_decimation.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_decimation */

static void toggle_normals(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether normals drawn.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_normals);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->normals= !texture_window->normals;
/*???debug */
if (texture_window->normals)
{
	printf("normals mode on\n");
}
else
{
	printf("normals mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_normals.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_normals */

static void toggle_wireframe(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether wireframe surface generated.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_wireframe);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->wireframe= !texture_window->wireframe;
/*???debug */
if (texture_window->wireframe)
{
	printf("wireframe mode on\n");
}
else
{
	printf("wireframe mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_wireframe.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_wireframe */

static void toggle_cutting_plane(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
toggles whether clipping plane on.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_cutting_plane);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->cutting_plane_on= !texture_window->cutting_plane_on;
		texture_window->current_texture->cutting_plane_on=texture_window->
			cutting_plane_on;
/*???debug */
if (texture_window->cutting_plane_on)
{
	printf("cutting plane on\n");
clean_mc_iso_surface(1+texture_window->hollow_mode_on /* FIX THIS */,
		texture_window->current_texture->mc_iso_surface);


}
else
{
clean_mc_iso_surface(1+texture_window->hollow_mode_on+1 /* FIX THIS */,
		texture_window->current_texture->mc_iso_surface);
	printf("cutting plane off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_cutting_plane.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_cutting_plane */

static void toggle_line_mode(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
puts line segment editing mode on.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_line_mode);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->line_mode_on= !texture_window->line_mode_on;
/*???debug */
if (texture_window->line_mode_on)
{
	printf("line_mode on\n");
}
else
{
	printf("line_mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_line_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_line_mode */

static void toggle_curve_mode(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
puts curve segment editing mode on.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_curve_mode);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->curve_mode_on= !texture_window->curve_mode_on;
/*???debug */
if (texture_window->curve_mode_on)
{
	printf("curve_mode on\n");
}
else
{
	printf("curve_mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_curve_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_curve_mode */

static void toggle_blob_mode(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
puts blob segment editing mode on.
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_blob_mode);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->blob_mode_on= !texture_window->blob_mode_on;
/*???debug */
if (texture_window->blob_mode_on)
{
	printf("blob_mode on\n");
}
else
{
	printf("blob_mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_blob_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_blob_mode */

static void toggle_soft_mode(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 4 March 1997

DESCRIPTION :
puts blob segment editing mode on.
==============================================================================*/
{
	struct Texture_window *texture_window;
	char string[100];

	ENTER(toggle_soft_mode);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->soft_mode_on= !texture_window->soft_mode_on;
/*???debug */
if (texture_window->soft_mode_on)
{
	printf("soft_mode on\n");
	sprintf(string, "Enter cutoff radius [%f]: > ", texture_window->select_value2);
			XtVaSetValues(texture_window->prompt_text_field,XmNvalue,
		string,NULL);
		XtVaSetValues(texture_window->prompt_text_field,XmNcursorPosition,
					40,NULL);
}
else
{
	printf("soft_mode off\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_soft_mode.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_soft_mode */

static void toggle_hollow(Widget w,XtPointer client_data,
	XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_hollow);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->hollow_mode_on= !texture_window->hollow_mode_on;
		texture_window->current_texture->hollow_mode_on=texture_window->
			hollow_mode_on;
/*???debug */
if (texture_window->hollow_mode_on)
{
	printf("hollow_mode on\n");
clean_mc_iso_surface(1+texture_window->cutting_plane_on /* FIX THIS */,
		texture_window->current_texture->mc_iso_surface);

}
else
{
	printf("hollow_mode off\n");
clean_mc_iso_surface(1+texture_window->cutting_plane_on+1 /* FIX THIS */,
		texture_window->current_texture->mc_iso_surface);
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_hollow.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_hollow */

static void select_cb(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
General select operation
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(select_cb);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
/*???debug */
printf("Select Button pressed\n");
		/* check line edit case */
		if (!texture_window->cell_mode)
		{
			if (texture_window->line_mode_on)
			{
				select_line(texture_window);
			}
			else
			{
				if (texture_window->curve_mode_on)
				{
					select_curve(texture_window,1);
				}
				else
				{
					if (texture_window->blob_mode_on)
					{
						select_blob(texture_window);
					}
					else
					{
						if (texture_window->soft_mode_on)
						{
							select_soft(texture_window);
						}

/*???debug */
						else
						{
								printf("no action defined\n");
						}
					}
				}
			}
		}
/*???debug */
else
{
	printf("No action defined\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"select_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* select_cb */


void select_material_cb(Widget w,void *data,void *temp_mat)
/*******************************************************************************
LAST MODIFIED : 21 December 1995

DESCRIPTION :
==============================================================================*/
{
	struct Graphical_material *material;
	struct Texture_window *texture_window;

	ENTER(select_material);
	USE_PARAMETER(w);
	texture_window = (struct Texture_window *)data;
	material = (struct Graphical_material *)temp_mat;
	if (texture_window)
	{
		texture_window->current_material=material;
		material_editor_dialog_set_material((Widget)NULL,
			texture_window->current_material);
		printf("current_material = %s \n",
			Graphical_material_name(texture_window->current_material));
		X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
		graphics_loop((XtPointer)texture_window);
		X3dThreeDDrawingSwapBuffers();
	}

	LEAVE;
} /* select_material */

void select_environment_map_cb(Widget w,void *data,void *temp_envmap)
/*******************************************************************************
LAST MODIFIED : 21 December 1995

DESCRIPTION :
==============================================================================*/
{
	struct Environment_map *envmap;
	struct Texture_window *texture_window;

	ENTER(select_environment_map_cb);
	USE_PARAMETER(w);
	texture_window = (struct Texture_window *)data;
	envmap = (struct Environment_map *)temp_envmap;
	if (texture_window)
	{
	texture_window->current_env_map=envmap;
	/* no environment_map editor exists yet
	material_editor_dialog_set_data((Widget)NULL,
			MATERIAL_EDITOR_DIALOG_DATA,(void *)(texture_window->
			current_material));	*/
	printf("current environment map = %s \n",texture_window->current_env_map->name);

	X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
	graphics_loop((XtPointer)texture_window);
	X3dThreeDDrawingSwapBuffers();
	}

	LEAVE;
}

static void select_gm_cb(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 19 February 1997

DESCRIPTION :
General deselect operation
==============================================================================*/
{
	struct Texture_window *window;
	struct Callback_data callback;
	Widget select_widget;

	ENTER(select_gm_cb);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (window=(struct Texture_window *)client_data)
	{
		if (!(window->graphical_material_palette_shell))
		{
			/* make the dialog shell */
			if (window->graphical_material_palette_shell=XtVaCreatePopupShell(
				"Graphical Material Palette",topLevelShellWidgetClass,
				window->user_interface->application_shell,XmNallowShellResize,
				TRUE,NULL))
			{
				if (CREATE_SELECT_WIDGET(Graphical_material)(&select_widget,
					window->graphical_material_palette_shell,SELECT_LIST,
					NULL,window->graphical_material_manager))
				{
					/*???DB.  Assign callbacks ? */
					callback.procedure=select_material_cb;
					callback.data=window;
					SELECT_SET_UPDATE_CB(Graphical_material)(select_widget,&callback);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"select_gm_cb.  Could not create select widget.");
					XtDestroyWidget(window->graphical_material_palette_shell);
					window->graphical_material_palette_shell=(Widget)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"select_gm_cb.  Error creating shell");
			}
		}
		if (window->graphical_material_palette_shell)
		{
			XtPopup(window->graphical_material_palette_shell,XtGrabNone);
		}
	}

}

static void select_envmap_cb(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 19 February 1997

DESCRIPTION :
General deselect operation
==============================================================================*/
{
	struct Texture_window *window;
	struct Callback_data callback;
	Widget select_widget;

	ENTER(select_gm_cb);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
printf("Creating Environment map palette\n");
	/* checking arguments */
	if (window=(struct Texture_window *)client_data)
	{
		if (!(window->envmap_palette_shell))
		{
			/* make the dialog shell */
			if (window->envmap_palette_shell=XtVaCreatePopupShell(
				"Environment map Palette",topLevelShellWidgetClass,
				window->user_interface->application_shell,XmNallowShellResize,
				TRUE,NULL))
			{
				if (CREATE_SELECT_WIDGET(Environment_map)(&select_widget,
					window->envmap_palette_shell,SELECT_LIST,
					NULL,window->environment_map_manager))
				{
					/*???DB.  Assign callbacks ? */
					callback.procedure=select_environment_map_cb;
					callback.data=window;
					SELECT_SET_UPDATE_CB(Environment_map)(select_widget,&callback);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"select_envmap_cb.  Could not create select widget.");
					XtDestroyWidget(window->envmap_palette_shell);
					window->envmap_palette_shell=(Widget)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"select_envmap_cb.  Error creating shell");
			}
		}
		if (window->envmap_palette_shell)
		{
			XtPopup(window->envmap_palette_shell,XtGrabNone);
		}
	}

}

static void deselect_cb(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
General deselect operation
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(deselect_cb);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
/*???debug */
printf("deselect Button pressed\n");
		/* check line edit case */
		if (!texture_window->cell_mode)
		{
			if (texture_window->line_mode_on||texture_window->curve_mode_on||
				texture_window->blob_mode_on || texture_window->soft_mode_on )
			{
				deselect_curve(texture_window);
			}
/*???debug */
else
{
	printf("no action defined\n");
}
		}
/*???debug */
else
{
	printf("No action defined\n");
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"deselect_cb.  Invalid argument(s)");
	}
	LEAVE;
} /* deselect_cb */

static void toggle_cop(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
==============================================================================*/
{
	int i;
	struct Texture_window *texture_window;

	ENTER(toggle_cop);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->cop_mode_on= !texture_window->cop_mode_on;
/*???debug */
if (texture_window->cop_mode_on)
{
	printf("cop_mode on\n");
}
else
{
	printf("cop_mode off\n");
}
		for (i=0;i<3;i++)
		{
			adjustxi(texture_window,i);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_cop.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_cop */

static void toggle_env(Widget w,XtPointer client_data,XmAnyCallbackStruct *cbs)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
Toggles whether environments or materials selected
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(toggle_env);
	USE_PARAMETER(w);
	USE_PARAMETER(cbs);
	if (texture_window=(struct Texture_window *)client_data)
	{
		texture_window->env_mode_on=!texture_window->env_mode_on;
/*???debug */
if (texture_window->env_mode_on)
{
	printf("env_mode on\n");
}
else
{
	printf("env_mode off\n");
}
		X3dThreeDDrawingMakeCurrent(texture_window->select_3d_widget);
		select_3d_draw(texture_window->select_3d_widget,texture_window,NULL);
		X3dThreeDDrawingSwapBuffers();
		X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
		graphics_loop((XtPointer)texture_window);
		X3dThreeDDrawingSwapBuffers();
	}
	else
	{
		display_message(ERROR_MESSAGE,"toggle_env.  Invalid argument(s)");
	}
	LEAVE;
} /* toggle_env */

/*
Global functions
----------------
*/
void adjustxi(struct Texture_window *texture_window,int index)
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
==============================================================================*/
{
	double slider_maximum,value;

	ENTER(adjustxi);
	/* checking arguments */
	if (texture_window)
	{
		if (texture_window->cell_mode)
		{
			slider_maximum=texture_window->xires[index];
			value=slider_maximum*texture_window->xival[index]/
				(texture_window->ximax[index]-texture_window->ximin[index])
				-(texture_window->ximax[index]-texture_window->ximin[index])
				/(2.0*texture_window->xires[index]);
		}
		else
		{
			/* inc resolution by one to include end point */
			slider_maximum=texture_window->xires[index]+1;
			value=slider_maximum*texture_window->xival[index]/
				(texture_window->ximax[index]-texture_window->ximin[index]);
		}
		if (value<0)
		{
			value=0;
		}
		if (value>slider_maximum)
		{
			value=slider_maximum;
		}
		XtVaSetValues(texture_window->xi_sb[index],XmNsliderSize,1,
			XmNmaximum,(int)slider_maximum,XmNvalue,(int)value,NULL);
		adjustxi_sb(texture_window,index);
		if (texture_window->tw_grid)
		{
			update_grid(texture_window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"adjustxi.  Invalid argument(s)");
	}
	LEAVE;
} /* adjustxi */

void printVT(struct VT_volume_texture *texture)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Debug
==============================================================================*/
{
	int i,j,k,n_cells,n_nodes;

	ENTER(printVT);
/*??? debug */
printf("************ PrintVT *************\n");
	/* checking arguments */
	if (texture)
	{
		n_cells=(texture->dimension[0])*(texture->dimension[1])*
			(texture->dimension[2]);
		n_nodes=(texture->dimension[0]+1)*(texture->dimension[1]+1)*
			(texture->dimension[2]+1);
/*???debug */
printf("#cells=%d, #nodes=%d\n",n_cells,n_nodes);
printf("xi ranges\n");
		for (i=0;i<3;i++)
		{
			printf("%lf ",texture->ximin[i]);
		}
		for (i=0;i<3;i++)
		{
			printf("%lf ",texture->ximax[i]);
		}
		/* write the discretization */
		printf("dimensions\n");
		for (i=0;i<3;i++)
		{
			printf("%d ",texture->dimension[i]);
		}
		printf("\n");
		printf("Cell scalar values\n");
		for (i=0;i<texture->dimension[0];i++)
		{
			for (j=0;j<texture->dimension[1];j++)
			{
				for (k=0;k<texture->dimension[2];k++)
				{
					printf("%lf ",texture->texture_cell_list
						[i+j*(texture->dimension[0])+
						(texture->dimension[0])*(texture->dimension[1])*k]->scalar_value);
				}
				printf("\n");
			}
			printf("\n");
		}
		printf("Cell materials\n");
		for (i=0;i<texture->dimension[0];i++)
		{
			for (j=0;j<texture->dimension[1];j++)
			{
				for (k=0;k<texture->dimension[2];k++)
				{
					if (texture->texture_cell_list[i+j*(texture->dimension[0])+
						(texture->dimension[0])*(texture->dimension[1])*k]->material)
					{
						printf("%s ",Graphical_material_name(texture->texture_cell_list
							[i+j*(texture->dimension[0])+(texture->dimension[0])*
							(texture->dimension[1])*k]->material));
					}
					else
					{
						printf("null");
					}
				}
				printf("\n");
			}
			printf("\n");
		}
		printf("Node scalars\n");
		for (i=0;i<texture->dimension[0]+1;i++)
		{
			for (j=0;j<texture->dimension[1]+1;j++)
			{
				for (k=0;k<texture->dimension[2]+1;k++)
				{
					printf("%lf ",texture->global_texture_node_list
						[i+j*(texture->
						dimension[0]+1)+(texture->dimension[0]+1)*(texture->dimension[1]
						+1)*k]->scalar_value);
				}
				printf("\n");
			}
			printf("\n");
		}
		printf("Node materials\n");
		printf("%p\n",texture);
		printf("dim %d %d %d\n",texture->dimension[0]+1,
			texture->dimension[1]+1,texture->dimension[2]+1);
		printf("tgnl: %p\n",texture->global_texture_node_list);
		for (i=0;i<texture->dimension[0]+1;i++)
		{
			for (j=0;j<texture->dimension[1]+1;j++)
			{
				for (k=0;k<texture->dimension[2]+1;k++)
				{
					printf("%p\n",texture->global_texture_node_list
						[i+j*(texture->dimension[0]+1)+(texture->dimension[0]+1)*
						(texture->dimension[1]+1)*k]);
					if (texture->global_texture_node_list[i+j*(texture->dimension[0]+1)+
						(texture->dimension[0]+1)*(texture->dimension[1]+1)*k]->material)
					{
						printf("label\n");
						printf("%s ",Graphical_material_name(
							texture->global_texture_node_list
							[i+j*(texture->dimension[0]+1)+(texture->dimension[0]+1)*
							(texture->dimension[1]+1)*k]->material));
					}
					else
					{
						printf("Null\n");
					}
				}
				printf("\n");
			}
			printf("\n");
		}
		printf("Node dominant materials\n");
		for (i=0;i<texture->dimension[0]+1;i++)
		{
			for (j=0;j<texture->dimension[1]+1;j++)
			{
				for (k=0;k<texture->dimension[2]+1;k++)
				{
					if (texture->global_texture_node_list[i+j*(texture->dimension[0]+1)+
						(texture->dimension[0]+1)*(texture->dimension[1]+1)*k]->
						dominant_material)
					{
						printf("%s ",Graphical_material_name(
							texture->global_texture_node_list
							[i+j*(texture->dimension[0]+1)+(texture->dimension[0]+1)*
							(texture->dimension[1]+1)*k]->dominant_material));
					}
					else
					{
						printf("Null ");
					}
				}
				printf("\n");
			}
			printf("\n");
		}
	}
	LEAVE;
} /* printVT */




struct Texture_window *create_texture_edit_window(
	struct Graphical_material *default_graphical_material,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Environment_map) *environment_map_manager,
	struct MANAGER(Texture) *texture_manager,Widget *material_editor_address,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 27 July 1998

DESCRIPTION :
Create the structures and retrieve the texture window from the uil file.
==============================================================================*/
{
	int i;
	static struct Texture_window *texture_window;
	static MrmType dummy_class;
	static MrmRegisterArg callback_list[] =
	{
		{"identify_tf",(XtPointer)identify_tf },
		{"identify_sb",(XtPointer)identify_sb },
		{"identify_tb",(XtPointer)identify_tb },
		{"tf_activated",(XtPointer)tf_activated},
		{"sb_activated",(XtPointer)sb_activated},
		{"exit_window",(XtPointer)exit_window},
		{"toggle_node_cell",(XtPointer)toggle_node_cell},
		{"toggle_paint_mode",(XtPointer)toggle_paint_mode},
		{"toggle_delete_paint_mode",(XtPointer)toggle_delete_paint_mode},
		{"toggle_fill_mode",(XtPointer)toggle_fill_mode},
		{"toggle_delete_mode",(XtPointer)toggle_delete_mode},
		{"toggle_auto",(XtPointer)toggle_auto},
		{"toggle_pick",(XtPointer)toggle_pick},
		{"toggle_cubes",(XtPointer)toggle_cubes},
		{"toggle_see_paint",(XtPointer)toggle_see_paint},
		{"toggle_isosurface",(XtPointer)toggle_isosurface},
		{"toggle_grid",(XtPointer)toggle_grid},
		{"toggle_closed_surface",(XtPointer)toggle_closed_surface},
		{"toggle_shaded_surfaces",(XtPointer)toggle_shaded_surfaces},
		{"toggle_normals",(XtPointer)toggle_normals},
		{"toggle_wireframe",(XtPointer)toggle_wireframe},
		{"toggle_line_mode",(XtPointer)toggle_line_mode},
		{"toggle_curve_mode",(XtPointer)toggle_curve_mode},
		{"toggle_blob_mode",(XtPointer)toggle_blob_mode},
		{"toggle_soft_mode",(XtPointer)toggle_soft_mode},
		{"toggle_hollow",(XtPointer)toggle_hollow},
		{"toggle_cop",(XtPointer)toggle_cop},
		{"toggle_env",(XtPointer)toggle_env},
		{"toggle_decimation",(XtPointer)toggle_decimation},
		{"toggle_detail",(XtPointer)toggle_detail},
		{"toggle_cutting_plane",(XtPointer)toggle_cutting_plane},
/*    { "open_material_editor", (XtPointer)open_material_editor}, */
		{"open_transformation_editor",(XtPointer)open_transformation_editor},
		{"open_file_and_read",(XtPointer)open_file_and_read},
		{"open_file_and_write",(XtPointer)open_file_and_write},
		{"select_cb",(XtPointer)select_cb},
		{"load_2d_texture",(XtPointer)load_2d_texture},
		{"save_as_finite_elements",(XtPointer)save_as_finite_elements},
		{"vt_select_mat_create",(XtPointer)vt_select_mat_create},
		{"select_gm_cb",(XtPointer)select_gm_cb},
		{"select_envmap_cb",(XtPointer)select_envmap_cb},
		{"regular_mesh",(XtPointer)regular_mesh},
		{"irregular_mesh",(XtPointer)irregular_mesh},
		{"group_cb",(XtPointer)group_cb},
		{"slit_cb",(XtPointer)slit_cb},
		{"group_list_create_cb",(XtPointer)group_list_create_cb},
		{"group_list_select_cb",(XtPointer)group_list_select_cb},
		{"deselect_cb",(XtPointer)deselect_cb}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"texture_window_pointer",(XtPointer)NULL},
		{"read_settings_file_data",(XtPointer)NULL},
		{"write_settings_file_data",(XtPointer)NULL}
	};

	ENTER(create_texture_edit_window);
	/* check arguments */
	if (user_interface&&graphical_material_manager&&environment_map_manager&&
		texture_manager&&material_editor_address&&
		default_graphical_material)
	{
		if (MrmOpenHierarchy_base64_string(volume_texture_editor_uidh,
			&volume_texture_editor_hierarchy,&volume_texture_editor_hierarchy_open))
		{
			if (ALLOCATE(texture_window,struct Texture_window,1))
			{
				/* initialize fields to zero here */
				texture_window->user_interface=user_interface;
				texture_window->default_graphical_material=default_graphical_material;
				texture_window->graphical_material_manager=graphical_material_manager;
				texture_window->environment_map_manager=environment_map_manager;
				texture_window->texture_manager=texture_manager;
				texture_window->graphical_material_palette_shell=(Widget)NULL;
				texture_window->envmap_palette_shell=(Widget)NULL;
				texture_window->create_finite_elements_dialog=
					(struct Create_finite_elements_dialog *)NULL;
				texture_window->material_editor_address=material_editor_address;
				texture_window->texture_window_shell=(Widget)NULL;
				texture_window->texture_window_widget=(Widget)NULL;
				texture_window->prompt_text_field=(Widget)NULL;
				texture_window->select_3d_widget=NULL;
				texture_window->vt_3d_select_form=NULL;
				texture_window->node_group_list=NULL;
				for (i=0;i<4;i++)
				{
					texture_window->slit_mode[i]=0;
				}
				texture_window->edit_group_mode=0;
				strcpy(texture_window->current_node_group,"default");
/*???debug */
printf("texture_window=%p\n",texture_window);
				for (i=0;i<3;i++)
				{
					texture_window->select_val_tf=NULL;
					texture_window->ximin_tf[i]=NULL;
					texture_window->ximin[i]=0;
					texture_window->ximax_tf[i]=NULL;
					texture_window->ximax[i]=1;
					texture_window->xires_tf[i]=NULL;
					texture_window->xires[i]=1;
					texture_window->xival_tf[i]=NULL;
					texture_window->xival[i]=0;
					texture_window->xi_sb[i]=NULL;
					texture_window->displacement_val_tf[i]=NULL;
					texture_window->displacement[i]=0;
					texture_window->rotate_sb[i]=NULL;
					texture_window->rot_angle[i]=0;
					texture_window->tw_axes=0;
					texture_window->tw_grid=0;
					texture_window->tw_texture=0;
					texture_window->tw_cube=0;
					texture_window->tw_wire_cube=0;
					texture_window->tw_mouseplane=0;
					texture_window->tw_sphere=0;
					texture_window->tw_small_sphere=0;
					texture_window->tw_env_map_sphere=0;
					texture_window->tw_box=0;
					texture_window->tw_3dtexture=0;
					texture_window->tw_isosurface=0;
					texture_window->tw_nodes=0;
					texture_window->tw_lines=0;
					texture_window->tw_curves=0;
					texture_window->tw_blobs=0;
					texture_window->tw_softs=0;
					texture_window->irregular_mesh_request=0;
					texture_window->current_axis=0;
					texture_window->mouse_x=0;
					texture_window->mouse_y=0;
					texture_window->mouse_xi1=0;
					texture_window->mouse_xi2=0;
					texture_window->mouse_xi3=0;
					texture_window->mouse_x_rel=0;
					texture_window->mouse_y_rel=0;
					texture_window->mouse_scale=2;
					/* cell mode initiallly set */
					texture_window->cell_mode=1;
					texture_window->paint_mode=0;
					texture_window->delete_mode=0;
					texture_window->auto_on=0;
					texture_window->pick_mode=0;
					texture_window->cubes_on=0;
					texture_window->fill_mode=0;
					texture_window->delete_paint_mode=0;
					texture_window->isosurface_on=0;
					texture_window->grid_on=0;
					texture_window->see_paint_on=0;
					texture_window->isovalue = 0;
					texture_window->closed_surface=0;
					texture_window->shaded_surfaces=1;
					texture_window->decimation=0;
					texture_window->detail_mode=0;
					texture_window->normals=0;
					texture_window->cutting_plane_on=0;
					texture_window->wireframe=0;
					texture_window->line_mode_on=0;
					texture_window->curve_mode_on=0;
					texture_window->blob_mode_on=0;
					texture_window->soft_mode_on=0;
					texture_window->hollow_mode_on=0;
					texture_window->cop_mode_on=0;
					texture_window->env_mode_on=0;
					texture_window->edit_curve.index=0;
					texture_window->edit_curve.type=0;
					texture_window->current_texture=NULL;
					texture_window->current_env_map=NULL;
					texture_window->set_input_limits=1;
					texture_window->input_move_model=1;
					texture_window->cop[0]=0.0;
					texture_window->cop[1]=0.0;
					texture_window->cop[2]=0.0;
					texture_window->brushsize[0]=
					texture_window->brushsize[1]=
					texture_window->brushsize[2]=1.0;
				}
				for (i=0;i<4;i++)
				{
					texture_window->cutting_plane_sb[i]=NULL;
				}
				for (i=0;i<6;i++)
				{
					texture_window->tw_cube_tex[i]=0;
				}
				texture_window->select_value=1.0;
				texture_window->select_value2=0.5;
				texture_window->zoom_sb=NULL;
				texture_window->res_sb=NULL;
				texture_window->material_sb=NULL;
				texture_window->material_val_tf=NULL;
				/* create and clear voltex */
				texture_window->voltex=
					CREATE(GT_voltex)(0,0,NULL,NULL,NULL,NULL,NULL,NULL,0,0,NULL,
					  g_VOLTEX_SHADED_TEXMAP);
/*???debug */
printf("create_texture_edit_window  %p\n",texture_window->voltex);
				/* create window widget */
				if (texture_window->texture_window_shell=XtVaCreatePopupShell(
					"Volume editor",transientShellWidgetClass,
					user_interface->application_shell,
					XmNmwmDecorations, MWM_DECOR_ALL|MWM_DECOR_MAXIMIZE,
					XmNmwmFunctions, MWM_FUNC_ALL|MWM_FUNC_MAXIMIZE,
					XmNtransient, FALSE,
					NULL))
				{
					/* Add destroy callback */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						volume_texture_editor_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* pass the animation_window_pointer to the uid stuff */
						identifier_list[0].value=(XtPointer)texture_window;
						identifier_list[1].value=(XtPointer)create_File_open_data(
							".voltex",REGULAR,read_settings_file,(void *)texture_window,0,
							user_interface);
						identifier_list[2].value=(XtPointer)create_File_open_data(".voltex",
							REGULAR,write_settings_file,(void *)texture_window,0,
							user_interface);
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							volume_texture_editor_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							if (MrmSUCCESS==MrmFetchWidget(volume_texture_editor_hierarchy,
								"texture_window",texture_window->texture_window_shell,
								&(texture_window->texture_window_widget),&dummy_class))
							{
								XtManageChild(texture_window->texture_window_widget);
								XtRealizeWidget(texture_window->texture_window_shell);
								XtPopup(texture_window->texture_window_shell, XtGrabNone);
#if defined (EXT_INPUT)
								input_module_add_input_window(
									texture_window->texture_window_shell,
									texture_window->user_interface);
#endif /* defined (EXT_INPUT) */
								initialize(texture_window);
/*???debug */
printf("texture_window initialized\n");
								/* now bring up a 3d drawing widget */
								if (texture_window->select_3d_widget=XtVaCreateWidget(
									"a3d_widget",threeDDrawingWidgetClass,
									texture_window->vt_3d_select_form,
									/* XmNwidth,100,
									XmNheight,100, */
									XmNbottomAttachment,XmATTACH_FORM,
									XmNleftAttachment,XmATTACH_FORM,
									XmNrightAttachment,XmATTACH_FORM,
									XmNtopAttachment,XmATTACH_FORM,
									X3dNbufferingMode,X3dDOUBLE_BUFFERING,
									X3dNbufferColourMode,X3dCOLOUR_RGB_MODE,
									NULL))
								{
									XtAddCallback(texture_window->select_3d_widget,
										X3dNinitializeCallback,select_3d_init_CB,
										texture_window);
									XtAddCallback(texture_window->select_3d_widget,
										X3dNexposeCallback,select_3d_draw,
										texture_window);
									XtAddCallback(texture_window->select_3d_widget,
										X3dNinputCallback,select_3d_input,
										texture_window);
									XtManageChild(texture_window->select_3d_widget);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_texture_edit_window.  Could not create 3d widget.");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
					"create_texture_edit_window.  Could not fetch widget texture window");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_texture_edit_window.  Could not register identifiers");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_texture_edit_window.  Could not register names");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_texture_edit_window.  Could not do popup");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"create_texture_edit_window.  Memory allocate for texture window failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_texture_edit_window.  Could not open hierarchy");
			texture_window=(struct Texture_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_texture_edit_window.  Invalid argument(s)");
		texture_window=(struct Texture_window *)NULL;
	}
	LEAVE;

	return (texture_window);
} /* create_texture_edit_window */

void adjust_material_sb(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 28 December 1995

DESCRIPTION :
???DB.  Is this needed ?
==============================================================================*/
{
	ENTER(adjust_material_sb);
	/* checking arguments */
	if (texture_window)
	{
#if defined (OLD_CODE)
/*???DB.  all_graphical_materials no longer exists */
		XtVaGetValues(texture_window->material_sb,XmNvalue,&index,NULL);
		material_item=all_graphical_materials;
		i=index;
		while ((i>0)&&material_item)
		{
			i--;
			material_item=material_item->next;
		}
		if (material_item&&(material_item->object))
		{
			sprintf(string_number,"%d",index);
			XtVaSetValues(texture_window->material_val_tf,XmNvalue,string_number,
				NULL);
		}
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"adjust_material_sb.  Missing texture_window");
	}
	LEAVE;
} /* adjust_material_sb */
