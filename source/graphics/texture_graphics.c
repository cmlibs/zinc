/*******************************************************************************
FILE : texture_graphics.c

LAST MODIFIED : 8 March 2002

DESCRIPTION :
Routines for GL texture display window
???DB.  Get rid of lmbind, will interfere with new lighting
==============================================================================*/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <X11/Intrinsic.h>
#include <Mrm/MrmAppl.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
/* graphics set up */
#if defined (OLD_CODE)
#if defined (GL_API)
#include <gl/device.h>
#endif /* defined (GL_API) */
#endif /* defined (OLD_CODE) */
/*???debug */
#if defined (UNIX)
#include <sys/times.h>
#endif /* defined (UNIX) */
#include "general/debug.h"
#include "general/geometry.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_object.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/mcubes.h"
#include "graphics/rendergl.h"
#include "graphics/texture_line.h"
#include "graphics/volume_texture.h"
#include "graphics/texture_graphics.h"
#include "graphics/volume_texture_editor.h"
#include "io_devices/input_module.h"
#include "material/material_editor_dialog.h"
#include "three_d_drawing/ThreeDDraw.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
/*???DB.  Not needed and should not be used */
#define MAX_MATERIALS 100

/* list of materials for selection */
static struct Graphical_material *material_pick_list[MAX_MATERIALS];
static struct Environment_map *env_map_pick_list[MAX_MATERIALS];

/*
Module variables
----------------
*/
/*???DB.  Temporary module variable to test */
int pick_index,env_map_pick_index;

/* size of material selection viewport */
static float /*ortho_left,*/ortho_right,/*ortho_bottom,*/ortho_top;

/* last node picked - for rubberbanding selection */
static int last_pick[3];


/* independent lighting from cmgui */
GLfloat light_ambient[] = {0.2, 0.2, 0.2, 1.0};
GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat light_position[] = {0, 1.0, 1.0, 0};

GLfloat lmodel_ambient[] = {0, 0, 0, 1.0};

#if defined (OLD_CODE)
#if defined (GL_API)
static float light1[]=
{
	LCOLOR,1.0,1.0,1.0,
	AMBIENT,0.2,0.2,0.2,
	POSITION,0,1,1,0.0,
	LMNULL
};
static float default_material[9]=
{
	AMBIENT,0.2,0.2,0.2,
	DIFFUSE,0.8,0.8,0.8,
	LMNULL
};
#endif /* defined (GL_API) */
#endif /* defined (OLD_CODE) */
/* graphics_objects */
#if defined (GL_API)
static Matrix idmat=
{
	1.0,0.0,0.0,0.0,
	0.0,1.0,0.0,0.0,
	0.0,0.0,1.0,0.0,
	0.0,0.0,0.0,1.0
};

#if defined (OLD_CODE)
float Idmat[4][4]=
{
	{1.0,0.0,0.0,0.0},
	{0.0,1.0,0.0,0.0},
	{0.0,0.0,1.0,0.0},
	{0.0,0.0,0.0,1.0}
};
#endif /* defined (OLD_CODE) */
#endif /* defined (GL_API) */
float cube_tex[4][2]=
{
	{0,0},{1,0},{1,1},{0,1}
};
float cube_vert[8][3]=
{
	{0,0,0},{1,0,0},{1,1,0},{0,1,0},
	{0,0,1},{1,0,1},{1,1,1},{0,1,1}
};
float cube_normals[6][3]=
{
	{0,0,-1},{0,0,1},{0,-1,0},{1,0,0},{0,1,0},{-1,0,0}
};
float box[4][3]=
{
	{-1,-1,0},{1,-1,0},{1,1,0},{-1,1,0}
};

/*
Module functions
----------------
*/
static void process_pick(struct Texture_window *texture_window,int button,
	int cx,int cy,int shift)
/*******************************************************************************
LAST MODIFIED : 15 January 1998

DESCRIPTION :
Processes picking
==============================================================================*/
{
	double m[3],M[3],r[3],xivali,xivalj,xivalk;
	double x, y, z, obj1, obj2, obj3, min_z;
	int i,j,k,R[3], ii, jj, kk;
	int min_index[3];
	double min_xival[3];
	double min_dist = 1280*1280+1024*1024;
	int dim[3], step, base_index;
	struct VT_texture_node *node;
	struct VT_node_group *node_group;
	int *nodes, node_index, found, count;
	int node_type;
	int iii, jjj, kkk, temp;

	ENTER(process_pick);
	printf("Process pick called: button = %d    x, y = %d, %d  shift = %d\n",
		button, cx, cy, shift);
	/* now project set of nodes and compare */

	for (i=0;i<3;i++)
	{
		m[i]=texture_window->ximin[i];
		M[i]=texture_window->ximax[i];
		r[i]=texture_window->xires[i];
		R[i]=texture_window->xires[i];
		dim[i]=texture_window->current_texture->dimension[i]+1;
	}
	/* step through grid points */
	for (i=0;i<=r[0];i++)
	{
		for (j=0;j<=r[1];j++)
		{
			for (k=0;k<=r[2];k++)
			{
				xivali=(double)i*(M[0]-m[0])/r[0]+m[0];
				xivalj=(double)j*(M[1]-m[1])/r[1]+m[1];
				xivalk=(double)k*(M[2]-m[2])/r[2]+m[2];
				if (texture_window->current_texture->grid_spacing)
				{
					xivali = texture_window->current_texture->grid_spacing[i]*(M[0]-m[0])+m[0];
					xivalj = texture_window->current_texture->grid_spacing[R[0]+1+j]*(M[1]-m[1])+m[1];
					xivalk = texture_window->current_texture->grid_spacing[R[0]+R[1]+2+k]*(M[2]-m[2])+m[2];
				}

				/* project point and compare with cursor */
				gluProject(	xivali, xivalj, xivalk,
					texture_window->modelview_matrix,
					texture_window->projection_matrix,
					texture_window->viewport, &x, &y, &z);
				/* printf("projecting (%lf, %lf, %lf) to (%lf %lf %lf)\n", xivali, xivalj, xivalk, x, y, z);*/
				if ( ((double) cx - x)*((double) cx - x) + ((double) cy - y)*((double) cy - y) < min_dist)
				{
					min_dist = ((double) cx - x)*((double) cx - x) + ((double) cy - y)*((double) cy - y);
					ii = min_index[0] = i;
					jj = min_index[1] = j;
					kk = min_index[2] = k;
					min_xival[0] = 	xivali;
					min_xival[1] = 	xivalj;
					min_xival[2] = 	xivalk;
					min_z = z;
				}
			}
		}
	}
	if (!shift)
	{
		last_pick[0] = ii;
		last_pick[1] = jj;
		last_pick[2] = kk;
	}
/*???debug */
printf("Node selected at %d, %d, %d  (%lf, %lf, %lf)\n",min_index[0],
	min_index[1],min_index[2],min_xival[0], min_xival[1],min_xival[2]);
	/* now check if the node is split - if so unproject and see if it is upper or
		lower in xi[i] */
	node=(texture_window->current_texture->global_texture_node_list)[
		ii+dim[0]*jj+dim[0]*dim[1]*kk];

	gluUnProject((double)cx, (double)cy, min_z,
		texture_window->modelview_matrix,
		texture_window->projection_matrix,
		texture_window->viewport, &obj1, &obj2, &obj3);
/*???debug */
/* printf("reprojecting back to grid: %lf %lf %lf\n", obj1, obj2, obj3);*/
	node_type = 0;
	if (obj1 > min_xival[0])
	{
		node_type +=1;
	}
	if (obj2 > min_xival[1])
	{
		node_type +=2;
	}
	if (obj3 > min_xival[2])
	{
		node_type +=4;
	}
/*???debug */
/*printf("Type %d: Selected node #%d\n",node_type, node->cm_node_identifier[node_type]);*/


#if defined (MOVECURSOR)
	texture_window->xival[0] = min_xival[0];
	texture_window->xival[1] = min_xival[1];
	texture_window->xival[2] = min_xival[2];

	printf("1######### xival = %lf %lf %lf\n", texture_window->xival[0], texture_window->xival[1], texture_window->xival[2]);

	/* must convert to analogous 'integer' xival (the grid table is used to convert to correct value */
	if (texture_window->current_texture->grid_spacing)
	{
		if (texture_window->cell_mode)
		{
			texture_window->xival[0] = min_index[0]/r[0];
			texture_window->xival[1] = min_index[1]/r[1];
			texture_window->xival[2] = min_index[2]/r[2];
		}
		else
		{
			texture_window->xival[0] = min_index[0]/(r[0]+1.0);
			texture_window->xival[1] = min_index[1]/(r[1]+1.0);
			texture_window->xival[2] = min_index[2]/(r[2]+1.0);
		}
	}
#endif
	/*------------------------ process according to pick mode ------------------------ */

	if (texture_window->edit_group_mode)
	{
		/* add cell index to group. If it already exists, delete it. */
		/* find current groups */
		found = 0;
		for (i=0;i<texture_window->current_texture->n_groups;i++)
		{
			if (0==strcmp((texture_window->current_texture->node_groups[i])->name,
				texture_window->current_node_group))
			{
				node_group = texture_window->current_texture->node_groups[i];
				found = 1;
			}
		}
		if (found)
		{
			if (!shift)
			{
				base_index = ii+jj*dim[0]+kk*dim[0]*dim[1];
				if (node->node_type > 0)
				{
					base_index = node->cm_node_identifier[node_type];
				}
				step = dim[0]*dim[1]*dim[2];
				/* scan node groups - if not found add, if found delete */
				node_index = -1;
				for (i=0;i<node_group->n_nodes;i++)
				{
					if (node_group->nodes[i] == base_index)
					{
						node_index = i;
					}
				}
				if (node_index>=0)
				{
					/* delete from list */
					printf("Deleting node #%d\n", node_group->nodes[node_index]);
					if (ALLOCATE(nodes, int, node_group->n_nodes-1))
					{
						count = 0;
						for (i=0;i<node_group->n_nodes;i++)
						{
							if (i != node_index)
							{
								nodes[count] = node_group->nodes[i];
								count++;
							}
						}
						DEALLOCATE(node_group->nodes);
						node_group->nodes = nodes;
						node_group->n_nodes--;
					}
					else
					{
						display_message(WARNING_MESSAGE, "process pick: alloc failed");
					}
				}
				else
				{
					/* add to list */
					printf("Adding node #%d\n", base_index);
					if (ALLOCATE(nodes, int, node_group->n_nodes+1))
					{
						count = 0;
						for (i=0;i<node_group->n_nodes; i++)
						{
							nodes[i] = node_group->nodes[i];
						}
						nodes[node_group->n_nodes] = base_index;
						DEALLOCATE(node_group->nodes);
						node_group->nodes = nodes;
						node_group->n_nodes++;
					}
					else
					{
						display_message(WARNING_MESSAGE, "process pick: alloc failed");
					}
				}
			}
			else
			{
			printf("Shift depressed: rubberbanding\n");
				if (last_pick[0] > ii)
				{
					temp = ii;
					ii = last_pick[0];
					last_pick[0] = temp;
				}
				if (last_pick[1] > jj)
				{
					temp = jj;
					jj = last_pick[1];
					last_pick[1] = temp;
				}
				if (last_pick[2] > kk)
				{
					temp = kk;
					kk = last_pick[2];
					last_pick[2] = temp;
				}
				for (iii = last_pick[0];iii<=ii;iii++)
				{
					for (jjj = last_pick[1];jjj<=jj;jjj++)
					{
						for (kkk = last_pick[2];kkk<=kk;kkk++)
						{
							if (!(iii == last_pick[0] && jjj == last_pick[1] && kkk == last_pick[2]))
							{
							node=(texture_window->current_texture->global_texture_node_list)[
								iii+dim[0]*jjj+dim[0]*dim[1]*kkk];
							base_index = iii+jjj*dim[0]+kkk*dim[0]*dim[1];
							/* for now only rubberband base nodes */
							/* if (node->node_type > 0)
							{
								base_index = node->cm_node_identifier[node_type];
							} */
							step = dim[0]*dim[1]*dim[2];
							/* scan node groups - if not found add, if found delete */
							node_index = -1;
							for (i=0;i<node_group->n_nodes;i++)
							{
								if (node_group->nodes[i] == base_index)
								{
									node_index = i;
								}
							}
							if (node_index>=0)
							{
								/* delete from list */
								printf("Deleting node #%d\n", node_group->nodes[node_index]);
								if (ALLOCATE(nodes, int, node_group->n_nodes-1))
								{
									count = 0;
									for (i=0;i<node_group->n_nodes;i++)
									{
										if (i != node_index)
										{
											nodes[count] = node_group->nodes[i];
											count++;
										}
									}
									DEALLOCATE(node_group->nodes);
									node_group->nodes = nodes;
									node_group->n_nodes--;
								}
								else
								{
									display_message(WARNING_MESSAGE, "process pick: alloc failed");
								}
							}
							else
							{
								/* add to list */
								printf("Adding node #%d\n", base_index);
								if (ALLOCATE(nodes, int, node_group->n_nodes+1))
								{
									count = 0;
									for (i=0;i<node_group->n_nodes; i++)
									{
										nodes[i] = node_group->nodes[i];
									}
									nodes[node_group->n_nodes] = base_index;
									DEALLOCATE(node_group->nodes);
									node_group->nodes = nodes;
									node_group->n_nodes++;
								}
								else
								{
									display_message(WARNING_MESSAGE, "process pick: alloc failed");
								}
							}
							}

						} /* kkk */
					}
				}
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "process pick: group not found");
		}
/*???debug */
/* list groups */
printf("List of current groups:\n");
for (i=0;i<texture_window->current_texture->n_groups;i++)
{
	node_group = texture_window->current_texture->node_groups[i];
	printf("Group name: %s\n", node_group->name);
	for (j=0;j<node_group->n_nodes;j++)
	{
		printf("%d\n", node_group->nodes[j]);
	}
}
	}
	else
	{
		if (texture_window->slit_mode[0])
		{
			/* write slit xi to node index */
			base_index = ii+jj*dim[0]+kk*dim[0]*dim[1];
			step = dim[0]*dim[1]*dim[2];
			node=texture_window->current_texture->global_texture_node_list[base_index];
			/* convert from binary xi representation to node_type */
			node->node_type=texture_window->slit_mode[1]+
				2*texture_window->slit_mode[2]+4*texture_window->slit_mode[3];
			/* when a node is slit, two or more nodes are created. In the order of
				increasing xi, the new nodes are numbered acoording to where they would
				fit on a cube numbered in increasing xi.  e.g.
				- a slit in xi1 produces a node_type of 1: this produces two sets of
					nodes on each side of the slit.  The lower set has the number
					base_index, the higher base_index + node_type*step;
				- a slit in xi2 produces a node_type of 2: this produces two sets of
					nodes on each side of the slit.  The lower set has the number
					base_index, the higher base_index + node_type*step;
				- a slit in xi3 produces a node_type of 4: this produces two sets of
					nodes on each side of the slit.  The lower set has the number
					base_index, the higher base_index + node_type*step;
				- a slit in xi1 and xi3 produces a node_type of 5: this produces 4 sets
					of nodes (xi2 same) around the original node which have numbers:
					base_index, base_index+1*step,base_index+4*step, base_index+5*step.
				2 = binary 010 = every combination of it components 0, 2 = 0, 2
				3 = binary 011 = every combination of it components 0, 1, 2 = 0, 1, 2, 3
				5 = binary 101 = every combination of it components 0, 1, 4 = 0, 1, 4, 5
				7 = binary 111 = every combination of it components 0, 1, 2, 4 = 0, 1,
					2, 3, 4, 5, 6, 7
				Because the node is shared by different surrounding elements, we need to
				repeat it in the faces so that it is consistently accessed depending on
				which element has it - so a split in xi1 makes two nodes which are
				shared by nodes which only differ in whether xi1 is lower or upper
				e.g. on a cube,  nodes 0, 2, 4, 6 all share xi1=0 and nodes 1, 3, 5, 7
				all share xi1=1
				For a split in xi1&xi2 = type 3, we split into groups of (0, 0, xi3),
				(1, 0, xi3), (0, 1, xi3), (1, 1, xi3) i.e. 0, 4    1, 5    2, 6    3, 7,
				and apply indices of
				base_index + (combinations of node_type 3 = 0, 1, 2, 3) * step */
			/* now set cmiss numbers according to node */
			switch (node->node_type)
			{
				case 0:
				{
					for (i=0;i<8;i++)
					{
						node->cm_node_identifier[i] = base_index;
					}
				} break;
				case 1:
				{
					node->cm_node_identifier[0] =
					node->cm_node_identifier[2] =
					node->cm_node_identifier[4] =
					node->cm_node_identifier[6] = base_index;
					node->cm_node_identifier[1] =
					node->cm_node_identifier[3] =
					node->cm_node_identifier[5] =
					node->cm_node_identifier[7] = base_index+step;
				} break;
				case 2:
				{
					node->cm_node_identifier[0] =
					node->cm_node_identifier[1] =
					node->cm_node_identifier[4] =
					node->cm_node_identifier[5] = base_index;
					node->cm_node_identifier[2] =
					node->cm_node_identifier[3] =
					node->cm_node_identifier[6] =
					node->cm_node_identifier[7] = base_index+2*step;
				} break;
				case 3:
				{
					node->cm_node_identifier[0] =
					node->cm_node_identifier[4] = base_index;
					node->cm_node_identifier[1] =
					node->cm_node_identifier[5] = base_index+step;
					node->cm_node_identifier[2] =
					node->cm_node_identifier[6] = base_index+2*step;
					node->cm_node_identifier[3] =
					node->cm_node_identifier[7] = base_index+3*step;
				} break;
				case 4:
				{
					node->cm_node_identifier[0] =
					node->cm_node_identifier[1] =
					node->cm_node_identifier[2] =
					node->cm_node_identifier[3] = base_index;
					node->cm_node_identifier[4] =
					node->cm_node_identifier[5] =
					node->cm_node_identifier[6] =
					node->cm_node_identifier[7] = base_index+4*step;
				} break;
				case 5:
				{
					node->cm_node_identifier[0] =
					node->cm_node_identifier[2] = base_index;
					node->cm_node_identifier[1] =
					node->cm_node_identifier[3] = base_index+step;
					node->cm_node_identifier[4] =
					node->cm_node_identifier[6] = base_index+4*step;
					node->cm_node_identifier[5] =
					node->cm_node_identifier[7] = base_index+5*step;
				} break;
				case 6:
				{
					node->cm_node_identifier[0] =
					node->cm_node_identifier[1] = base_index;
					node->cm_node_identifier[2] =
					node->cm_node_identifier[3] = base_index+2*step;
					node->cm_node_identifier[4] =
					node->cm_node_identifier[5] = base_index+4*step;
					node->cm_node_identifier[6] =
					node->cm_node_identifier[7] = base_index+6*step;
				} break;
				case 7:
				{
					for (i=0;i<8;i++)
					{
						node->cm_node_identifier[i]=base_index+i*step;
					}
				} break;
			}
/*???debug */
printf("Setting node (%d %d %d) Type = %d, Indices = %d:%d:%d:%d:%d:%d:%d:%d\n",
	ii, jj, kk, node->node_type,node->cm_node_identifier[0],node->cm_node_identifier[1],
	node->cm_node_identifier[2],node->cm_node_identifier[3],node->cm_node_identifier[4],
	node->cm_node_identifier[5],node->cm_node_identifier[6],node->cm_node_identifier[7]);
		}
	}
	adjustxi(texture_window,1);
	adjustxi(texture_window,2);
	update_grid(texture_window);
	/* draw_current_cell(texture_window);*/
	X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
	graphics_loop((XtPointer)texture_window);
	X3dThreeDDrawingSwapBuffers();
} /* process_pick */

static void draw_current_group(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
==============================================================================*/
{
	double xistep[3];
	int i, int_xival[3], nx, ny, nz;
	double *grid_spacing,sx,sy,sz,tx,ty,tz;
	struct VT_node_group *node_group;
	int found=0, node_type;
	int n, ii, jj, kk, node_index;

	ENTER(draw_current_group);

	/* checking arguments */
if (texture_window)
{
	for (i=0;i<texture_window->current_texture->n_groups;i++)
	{
			if (strcmp((texture_window->current_texture->node_groups[i])->name, texture_window->current_node_group) == 0)
			{
				node_group = texture_window->current_texture->node_groups[i];
				found = 1;
			}
			nx = texture_window->current_texture->dimension[0]+1;
			ny = texture_window->current_texture->dimension[1]+1;
			nz = texture_window->current_texture->dimension[2]+1;

	}
	if (found)
	{
		for (n=0;n<node_group->n_nodes;n++)
		{
			node_index = node_group->nodes[n];
			node_type = 0;

			if (node_index >= nx*ny*nz)
			{
				/* node_type = node_group->nodes[n] / (nx*ny*nz); */
				node_index = node_group->nodes[n] % (nx*ny*nz);
			}
			kk = node_index / (nx*ny);
			jj = (node_index - kk*nx*ny) / nx;
			ii = (node_index - kk*nx*ny - jj*nx);

			node_type = texture_window->current_texture->global_texture_node_list[ii+nx*jj+nx*ny*kk]->node_type;
/*???debug */
/*printf("Decomposing node #%d into node %d type %d\n", node_group->nodes[n],
	node_index, node_type);
printf("Drawing node %d (%d %d %d)\n", node_index, ii, jj, kk);*/

			if (texture_window->current_texture->grid_spacing)
			{
				grid_spacing = texture_window->current_texture->grid_spacing;
				sx = (texture_window->ximax[0]-texture_window->ximin[0]);
				sy = (texture_window->ximax[1]-texture_window->ximin[1]);
				sz = (texture_window->ximax[2]-texture_window->ximin[2]);
				tx = texture_window->ximin[0];
				ty = texture_window->ximin[1];
				tz = texture_window->ximin[2];
			}
			else
			{
				grid_spacing = (double *) NULL;
			}
			for (i=0;i<3;i++)
			{
				xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
				texture_window->xires[i];

				/* int_xival[i]=(int)((double)(texture_window->xival[i]-
					(texture_window->ximin[i]-0.5*xistep[i]/texture_window->xires[i]))/
					xistep[i]);*/

			}
			int_xival[0] = ii;
			int_xival[1] = jj;
			int_xival[2] = kk;

			glPushMatrix();
			if (grid_spacing)
			{
				glTranslatef(grid_spacing[int_xival[0]]*sx+tx,
					grid_spacing[nx+int_xival[1]]*sy+ty,
					grid_spacing[nx+ny+int_xival[2]]*sz+tz);
				glScalef(grid_spacing[int_xival[0]+(int)texture_window->brushsize[0]]
					-grid_spacing[int_xival[0]],grid_spacing[nx+int_xival[1]+
					(int)texture_window->brushsize[1]]-grid_spacing[nx+int_xival[1]],
					grid_spacing[nx+ny+int_xival[2]+(int)texture_window->brushsize[1]]-
					grid_spacing[nx+ny+int_xival[2]]);
				glScalef(sx,sy,sz);
			}
			else
			{
				glTranslatef(texture_window->ximin[0] + int_xival[0] * (xistep[0]),
					texture_window->ximin[1] + int_xival[1] * (xistep[1]),
					texture_window->ximin[2] + int_xival[2] * (xistep[2]));
				glScalef(xistep[0],xistep[1],xistep[2]);
			}
			if (node_type > 0)
			{
				switch (node_type)
				{
					case 1:
						if (node_group->nodes[n] > node_index)
						{
							glTranslatef(0.15, 0, 0);
						}
						else
						{
							glTranslatef(-0.15, 0, 0);
						}
						break;
					case 2:
						if (node_group->nodes[n] > node_index)
						{
							glTranslatef(0, 0.15, 0);
						}
						else
						{
							glTranslatef(0, -0.15, 0);
						}
						break;
					case 4:
						if (node_group->nodes[n] > node_index)
						{
							glTranslatef(0, 0, 0.15);
						}
						else
						{
							glTranslatef(0, 0, -0.15);
						}
						break;
				}
			}


			glDisable(GL_LIGHTING);
				glColor3f(0, 1.0, 0);
			/* glScalef(xistep[0]/3.9,xistep[1]/3.9,xistep[2]/3.9);*/
			/*
			glCallList(texture_window->tw_sphere);
			*/
			glPointSize(5);
			glBegin(GL_POINTS);
			glVertex3f(0, 0, 0);
			glEnd();
			glEnable(GL_LIGHTING);
			glPopMatrix();
		}
	}

}
else
{
	display_message(ERROR_MESSAGE,"draw_current_nodesl. Invalid argument");
}
	LEAVE;
} /* draw_current_cell */

#if defined (OLD_CODE)
static int display_material_sphere(struct Graphical_material *material,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
Draws a sphere with the specified <material>.
==============================================================================*/
{
	int return_code;
	struct Texture_window *texture_window;

	ENTER(display_material_sphere);
	/* check arguments */
	if (material&&(texture_window=(struct Texture_window *)user_data))
	{
		material_pick_list[pick_index]=material;
		execute_Graphical_material(material);
#if defined (OPENGL_API)
		glPushMatrix();
		/*???DB.  What if more than 10 materials ? */
		glTranslatef(ortho_right*.2 + ortho_right*((int) pick_index/10) *.2,
			ortho_top *.08 + ortho_top*(pick_index % 10)*.08,0);
		/* if current texture window material, draw larger & finer */
		if (material==texture_window->current_material)
		{
			glScalef(3.0,3.0,3.0);
			glCallList(texture_window->tw_sphere);
		}
		else
		{
			glCallList(texture_window->tw_small_sphere);
		}
		glPopMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		pushmatrix();
		/*???DB.  What if more than 10 materials ? */
		translate(ortho_right*.2+ortho_right*((int)pick_index/10)*.2,
			ortho_top*.08+ortho_top*(pick_index%10)*.08,0);
		/* if current texture window material, draw larger & finer */
		if (material==texture_window->current_material)
		{
			scale(3.0,3.0,3.0);
			callobj(texture_window->tw_sphere);
		}
		else
		{
			callobj(texture_window->tw_small_sphere);
		}
		popmatrix();
#endif /* defined (GL_API) */
		pick_index++;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"display_material_sphere.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* display_material_sphere */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static int display_env_map(struct Environment_map *env_map,
	void *void_texture_window)
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
Draws a sphere with the specified <material>.
==============================================================================*/
{
	int i,return_code;
	struct Texture_window *texture_window;

	ENTER(display_env_map);
	/* check arguments */
	if (env_map&&(texture_window=(struct Texture_window *)void_texture_window))
	{
		env_map_pick_list[env_map_pick_index]=env_map;
		for (i=0;i<6;i++)
		{
			execute_Graphical_material(env_map->face_material[i]);
#if defined (OPENGL_API)
			glPushMatrix();
			/*???DB.  What if more than 10 materials ? */
			glTranslatef(
				ortho_right*.2+ortho_right*((int)env_map_pick_index/10)*.2+(float)i*3.0,
				ortho_top *.08 + ortho_top*(env_map_pick_index % 10)*.08,0);
			/* if current texture window material, draw larger & finer */
			if (env_map==texture_window->current_env_map)
			{
				glScalef(1.75,1.75,1.75);
				glCallList(texture_window->tw_sphere);
			}
			else
			{
				glScalef(0.5,0.5,0.5);
				glCallList(texture_window->tw_small_sphere);
			}
			glPopMatrix();
#endif
#if defined (GL_API)
			pushmatrix();
			/*???DB.  What if more than 10 materials ? */
			translate(
				ortho_right*.2+ortho_right*((int)env_map_pick_index/10)*.2+(float)i*3.0,
				ortho_top*.08+ortho_top*(env_map_pick_index % 10)*.08,0);
			/* if current texture window material, draw larger & finer */
			if (env_map==texture_window->current_env_map)
			{
				scale(1.75,1.75,1.75);
				callobj(texture_window->tw_sphere);
			}
			else
			{
				scale(0.5,0.5,0.5);
				callobj(texture_window->tw_small_sphere);
			}
			popmatrix();
#endif /* defined (GL_API) */
		}
		env_map_pick_index++;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"display_env_map.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* display_env_map */
#endif /* defined (OLD_CODE) */

/*
Global functions
----------------
*/
void draw_texture_isosurface(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 8 March 2002

DESCRIPTION :
Calculates an isosurface from the volume texture, and creates an isosurface
object.
==============================================================================*/
{
	double *iso_poly_cop,isovalue_list[MAX_SCALAR_FIELDS];
	int face_index,i,j,n_iso_polys,n_scalar_fields,n_vertices,*texturemap_index,
		*triangle_list;
	float *texturemap_coord;
	struct Graphical_material *material;
	struct GT_voltex *voltex;
	struct Environment_map **iso_env_map;
	struct VT_iso_vertex *vertex_list;
	struct VT_scalar_field *scalar_field_list[MAX_SCALAR_FIELDS];
	struct VT_volume_texture *current_texture;
	struct MC_triangle *triangle;
/*???debug */
#if defined (UNIX)
	double cpu_time0, cpu_time1,cpu_time2,cpu_time3,cpu_time4,cpu_time5,cpu_time6;
	double real_time0, real_time1,real_time2,real_time3,real_time4,real_time5,real_time6;
	struct tms buffer;
	/* clock_ticks = 100 */
#endif /* defined (UNIX) */

	ENTER(draw_texture_isosurface);
	/* check argument */
	if (texture_window)
	{
#if defined (OLD_CODE)
		default_material=default_Graphical_material;
#endif /* defined (OLD_CODE) */
/*???debug */
#if defined (UNIX)
		real_time0=((double)times(&buffer))/100.0;
		cpu_time0=((double)buffer.tms_utime)/100.0;
#endif /* defined (UNIX) */
		current_texture=texture_window->current_texture;
#if defined (OLD_CODE)
		voltex=texture_window->voltex;
/*???debug */
printf("DEBUG:voltex = %p\n",voltex);
#endif /* defined (OLD_CODE) */

		/* calculate scalar field using filter */
		update_scalars(current_texture,texture_window->cutting_plane);
		scalar_field_list[0]=current_texture->scalar_field;
		isovalue_list[0]=texture_window->isovalue;
/*???debug */
printf("Ok\n");
		if (texture_window->hollow_mode_on)
		{
			/* should combine clip field and this field in some way */
			for (i=0;i<(current_texture->clip_field2->dimension[0]+1)*
				(current_texture->clip_field2->dimension[1]+1)*
				(current_texture->clip_field2->dimension[2]+1);i++)
			{
				current_texture->clip_field2->scalar[i]=1.0-
					current_texture->scalar_field->scalar[i];
			}
		}
		n_scalar_fields=1;
		if (texture_window->cutting_plane_on)
		{
			scalar_field_list[n_scalar_fields]=current_texture->clip_field;
			isovalue_list[n_scalar_fields]=texture_window->cutting_plane[3];
			n_scalar_fields++;
		}
		if (texture_window->hollow_mode_on)
		{
			scalar_field_list[n_scalar_fields]=current_texture->clip_field2;
			isovalue_list[n_scalar_fields]= /* 0.75 */
				current_texture->hollow_isovalue* texture_window->isovalue;
			n_scalar_fields++;
		}
		/* create isosurface */
#if defined (UNIX)
		real_time1=((double)times(&buffer))/100.0;
		cpu_time1=((double)buffer.tms_utime)/100.0;
#endif /* defined (UNIX) */
#if defined (OLD_CODE)
		/*???DB.  Why does the iso value have to be between 0 and 1 ? */
		if (texture_window->isovalue >= 0.0&&texture_window->isovalue <= 1.0)
#endif /* defined (OLD_CODE) */
		{
			if (current_texture->mc_iso_surface == NULL)
			{
				display_message(ERROR_MESSAGE,
						"generate isosurface: mc_iso_surface = NULL");
			}
			else
			{
				/* calculate detail map here */
/*???debug */
printf("ct->mc->detail = %p\n", current_texture->mc_iso_surface->detail_map);
printf("n_scalar_fields = %d\n", n_scalar_fields);
				calculate_detail_map(current_texture,current_texture->mc_iso_surface);
			}
/*???debug */
printf("2 n_scalar_fields = %d\n", n_scalar_fields);
			printf("Calling Marching cubes: ISOVALUE = %lf\n", texture_window->isovalue);
			marching_cubes(scalar_field_list,n_scalar_fields,
				current_texture->coordinate_field,
				current_texture->mc_iso_surface,
				isovalue_list,texture_window->closed_surface,
				(texture_window->cutting_plane_on)||(texture_window->hollow_mode_on),
				texture_window->decimation);
			/* set polygon materials from texture data */


			printf("Calculating materials....\n");
			calculate_mc_material(current_texture,current_texture->mc_iso_surface);
			printf("Done.\n");

/*???debug */
#if defined (UNIX)
			real_time2=((double)times(&buffer))/100.0;
			cpu_time2=((double)buffer.tms_utime)/100.0;
#endif /* defined (UNIX) */
/*???debug */
printf("Calculating texture projections...\n");
			for (i=0;i<current_texture->mc_iso_surface->n_triangles;i++)
			{
				triangle=current_texture->mc_iso_surface->compiled_triangle_list[i];
				face_index=triangle->triangle_list_index;
				if (face_index > 0 && face_index <= 6)
				{
					/* special case - texture projection on actual face */
					mc_face_cube_map_function(&(triangle->env_map_index[0]),
						triangle->texture_coord[0],
						triangle->vertices[0],
						triangle->iso_poly_cop[0],face_index,
						current_texture->ximin,current_texture->ximax);
					mc_face_cube_map_function(&(triangle->env_map_index[1]),
						triangle->texture_coord[1],
						triangle->vertices[1],
						triangle->iso_poly_cop[1],face_index,
						current_texture->ximin,current_texture->ximax);
					mc_face_cube_map_function(&(triangle->env_map_index[2]),
						triangle->texture_coord[2],
						triangle->vertices[2],
						triangle->iso_poly_cop[2],face_index,
						current_texture->ximin,current_texture->ximax);
				}
				else
				{
					/* general case */
					mc_cube_map_function(&(triangle->env_map_index[0]),
						triangle->texture_coord[0],
						triangle->vertices[0],
						triangle->iso_poly_cop[0],
						current_texture->ximin,current_texture->ximax);
					mc_cube_map_function(&(triangle->env_map_index[1]),
						triangle->texture_coord[1],
						triangle->vertices[1],
						triangle->iso_poly_cop[1],
						current_texture->ximin,current_texture->ximax);
					mc_cube_map_function(&(triangle->env_map_index[2]),
						triangle->texture_coord[2],
						triangle->vertices[2],
						triangle->iso_poly_cop[2],
						current_texture->ximin,current_texture->ximax);
				}
			}  /* for */
			printf("Done.\n");


			/*???MS. ( cutting plane[3] = clip isovalue ) */
		}
#if defined (OLD_CODE)
		else
		{
			printf("error : isovalue is %f\n",texture_window->isovalue);
		}
#endif /* defined (OLD_CODE) */
/*???debug */
#if defined (UNIX)
		real_time3=((double)times(&buffer))/100.0;
		cpu_time3=((double)buffer.tms_utime)/100.0;
#endif /* defined (UNIX) */
		/* make isosurface object */
#if defined (OPENGL_API)
		printf("Compiling isosurface display list #%u ....\n",texture_window->tw_isosurface);

		glNewList(texture_window->tw_isosurface,GL_COMPILE);
		/*printf("Done.\n");*/
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		delobj(texture_window->tw_isosurface);
		makeobj(texture_window->tw_isosurface);
#endif /* defined (GL_API) */
		/* the voltex structure displayed here uses the same routines and
			structures as the cmgui graphics module */
		/* destroy old gtvoltex if it exists */

/*------------------------MC ISOSURFACE---------------------------*/
/*???debug */
printf("Creating GTVOLTEX from MC_iso_surface....\n");
/*
printf("draw_texture_isosurface.  %p\n",texture_window->voltex);
*/
		if (texture_window->voltex)
		{
			DESTROY(GT_voltex)(&(texture_window->voltex));
		}
		n_iso_polys=current_texture->mc_iso_surface->n_triangles;
		n_vertices=current_texture->mc_iso_surface->n_vertices;

/*??? debug */
printf("DEBUG:Before allocate npolys = %d n_verts = %d\n",n_iso_polys,
	n_vertices);
		/* allocate new gtvoltex. Note this is not the most efficient way to go,
			but it ensures consistency of data structures and rendering */
		/* make sure ptrs are null  in case of failure */

		if ((n_iso_polys>0)&&(n_vertices>0))
		{
			if (ALLOCATE(triangle_list,int,3*n_iso_polys)&&ALLOCATE(iso_poly_cop,
				double,3*n_iso_polys*3)&&ALLOCATE(texturemap_coord,float,
				3*n_iso_polys*3)&&ALLOCATE(texturemap_index,int,3*n_iso_polys)&&
				ALLOCATE(vertex_list,struct VT_iso_vertex,n_vertices*1
				/*n_xi_rep[0]*n_xi_rep[1]*n_xi_rep[2]*/)&&ALLOCATE(iso_env_map,
				struct Environment_map *,3*n_iso_polys))
			{
				if (!(texture_window->voltex=CREATE(GT_voltex)(n_iso_polys,n_vertices,
					triangle_list,vertex_list,iso_env_map,iso_poly_cop,
					texturemap_coord,texturemap_index,1
					/* n_xi_rep[0]*n_xi_rep[1]*n_xi_rep[2]*/,
					0 /*n_data_components*/, (GTDATA *)NULL, g_VOLTEX_SHADED_TEXMAP )))
				{
					display_message(ERROR_MESSAGE,
						"create_GTVOLTEX_from_FE_element.   Could not create voltex");
				}
				else
				{
/*??? debug */
/*
printf("DEBUG:voltex successfully created: t_list = %p, v_list = %p\n",
					texture_window->voltex->triangle_list,
					texture_window->voltex->vertex_list);
*/
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
		"create_GTVOLTEX_from_FE_element.   Could not allocate memory for voltex");
				triangle_list=(int *)NULL;
				iso_poly_cop=(double *)NULL;
				texturemap_coord=(float *)NULL;
				texturemap_index=(int *)NULL;
				vertex_list=(struct VT_iso_vertex *)NULL;
				iso_env_map=(struct Environment_map **)NULL;
			}
/*???debug */
#if defined (UNIX)
real_time4=((double)times(&buffer))/100.0;
cpu_time4=((double)buffer.tms_utime)/100.0;
#endif /* defined (UNIX) */

			/* now memory structures are allocated fill them in */
			if (voltex=texture_window->voltex)
			{
				for (i=0;i<n_iso_polys;i++)
				{
					/* need this list for calculating normals from surrounds */
					for (j=0;j<3;j++)
					{

						triangle_list[3*i+j]=
						current_texture->mc_iso_surface->
							compiled_triangle_list[i]->vertex_index[j];
if (triangle_list[3*i+j] > n_vertices || triangle_list[3*i+j] < 0)
{
		printf("ERROR---- triangle_list[3*%d+%d] = %d > %d\n", i, j, triangle_list[3*i+j], n_vertices);

}
						if (material = current_texture->mc_iso_surface->
							compiled_triangle_list[i]->material[j])
						{
							GT_voltex_set_vertex_material(voltex, 3*i+j, material);
						}

						iso_env_map[3*i+j] =
						current_texture->mc_iso_surface->
							compiled_triangle_list[i]->env_map[j];

						texturemap_index[3*i+j]=
						current_texture->mc_iso_surface->
							compiled_triangle_list[i]->env_map_index[j];

						texturemap_coord[3*(3*i+0)+j]=
						current_texture->mc_iso_surface->
							compiled_triangle_list[i]->texture_coord[0][j];
						texturemap_coord[3*(3*i+1)+j]=
						current_texture->mc_iso_surface->
							compiled_triangle_list[i]->texture_coord[1][j];
						texturemap_coord[3*(3*i+2)+j]=
						current_texture->mc_iso_surface->
							compiled_triangle_list[i]->texture_coord[2][j];

						iso_poly_cop[3*(3*i+0)+j]=
						current_texture->mc_iso_surface->
							compiled_triangle_list[i]->iso_poly_cop[0][j];
						iso_poly_cop[3*(3*i+1)+j]=
						current_texture->mc_iso_surface->
							compiled_triangle_list[i]->iso_poly_cop[1][j];
						iso_poly_cop[3*(3*i+2)+j]=
						current_texture->mc_iso_surface->
							compiled_triangle_list[i]->iso_poly_cop[2][j];
					}
				}
				/* copy pointer based vertex/triangle lists into array based lists (for
					duplication and rendering) */
				for (i=0;i<n_vertices;i++)
				{
					for (j=0;j<3;j++)
					{
						vertex_list[i].coord[j]=current_texture->mc_iso_surface->
							compiled_vertex_list[i]->coord[j];
						vertex_list[i].normal[j]=current_texture->mc_iso_surface->
							compiled_vertex_list[i]->normal[j];
					}
					/* step through vertex list and allocate triangle indices */
					vertex_list[i].n_ptrs=
						current_texture->mc_iso_surface->compiled_vertex_list[i]->
							n_triangle_ptrs;

					for (j=0;j<vertex_list[i].n_ptrs;j++)
					{

						vertex_list[i].ptrs[j]=current_texture->mc_iso_surface->
							compiled_vertex_list[i]->triangle_ptrs[j]->triangle_index;
if (vertex_list[i].ptrs[j] > n_iso_polys || vertex_list[i].ptrs[j] < 0)
{
		printf("ERROR---- vertex_list[%d].ptrs[%d] = %d > %d\n", i,j,  vertex_list[i].ptrs[j], n_iso_polys);

}
					}
				}


/*----------------------------------------------------------------*/

#if defined (UNIX)
real_time5=((double)times(&buffer))/100.0;
cpu_time5=((double)buffer.tms_utime)/100.0;
#endif /* defined (UNIX) */
/*??? debug */
/*
printf("DEBUG:voltex->n_iso_polys = %d\n",voltex->n_iso_polys);
*/
				/* render using graphics window routines */
			if(texture_window->shaded_surfaces)
			{
				draw_voltexGL(voltex->n_iso_polys,voltex->triangle_list,
					voltex->vertex_list,voltex->n_vertices,voltex->n_rep,
					voltex->per_vertex_materials,
					voltex->iso_poly_material_index, voltex->iso_env_map,
					voltex->texturemap_coord, voltex->texturemap_index,
					/*n_data_components*/0,/*data*/(GTDATA *)NULL,
					(struct Graphical_material *)NULL,(struct Spectrum *)NULL);
			}
			printf("Done.\n");
			}
			else
			{
				DEALLOCATE(triangle_list);
				DEALLOCATE(vertex_list);
				DEALLOCATE(texturemap_coord);
				DEALLOCATE(texturemap_index);
				DEALLOCATE(iso_env_map);
				DEALLOCATE(iso_poly_cop);
			} /* if voltex */
/*???debug */
#if defined (UNIX)
real_time6=((double)times(&buffer))/100.0;
cpu_time6=((double)buffer.tms_utime)/100.0;
#endif /* defined (UNIX) */
			/* done */
			/* extra display features rendered by the editor only */
#if defined (OPENGL_API)
/*glNewList(texture_window->tw_isosurface,GL_COMPILE);*/
			if (texture_window->normals)
			{
					for (i=0;i<current_texture->mc_iso_surface->n_vertices;i++)
					{

					glColor3f(1, 0, 1);
					glBegin(GL_LINES);
					glVertex3fv(&(current_texture->mc_iso_surface->
							compiled_vertex_list[i]->coord[0]));
					glVertex3f(current_texture->mc_iso_surface->
							compiled_vertex_list[i]->coord[0]+
						current_texture->mc_iso_surface->
							compiled_vertex_list[i]->normal[0]*0.3,
						current_texture->mc_iso_surface->
							compiled_vertex_list[i]->coord[1]+
						current_texture->mc_iso_surface->
							compiled_vertex_list[i]->normal[1]*0.3,
						current_texture->mc_iso_surface->
							compiled_vertex_list[i]->coord[2]+
						current_texture->mc_iso_surface->
							compiled_vertex_list[i]->normal[2]*0.3);
					glEnd();
					}
			}
			if (texture_window->wireframe)
			{
				glPushAttrib(GL_ALL_ATTRIB_BITS);
				glDisable(GL_LIGHTING);
				glDisable(GL_TEXTURE_2D);
				glColor3f(0, 1, 0);
					for (i=0;i<current_texture->mc_iso_surface->n_triangles;i++)
					{

					glBegin(GL_LINE_LOOP);
					glVertex3fv(&(current_texture->mc_iso_surface->
							compiled_triangle_list[i]->vertices[0]->coord[0]));
					glVertex3fv(&(current_texture->mc_iso_surface->
							compiled_triangle_list[i]->vertices[1]->coord[0]));
					glVertex3fv(&(current_texture->mc_iso_surface->
							compiled_triangle_list[i]->vertices[2]->coord[0]));

					glEnd();

					}
				glPopAttrib();
			}
#endif /* defined (OPENGL_API) */

		}
#if defined (OLD_CODE)
		else
		{
			display_message(WARNING_MESSAGE,
				"draw_texture_isosurface.  n_iso_polys<=0 or n_vertices<=0");
		}
#endif /* defined (OLD_CODE) */
#if defined (GL_API)
		closeobj();
#endif /* defined (GL_API) */
#if defined (OPENGL_API)
		glEndList();
#endif /* defined (OPENGL_API) */
/*???debug */
#if defined (UNIX)
		printf("------------ Drawing Timing Diagnostics ----------\n");
		printf("Scalar calculation:        Real = %lf    CPU = %lf\n",
			real_time1-real_time0,cpu_time1-cpu_time0);
		printf("Isosurface calculation:        Real = %lf    CPU = %lf\n",
			real_time2-real_time1,cpu_time2-cpu_time1);
		printf("Texture projection: Real = %lf    CPU = %lf\n",
			real_time3-real_time2,cpu_time3-cpu_time2);
		printf("Voltex memory allocation:    Real = %lf    CPU = %lf\n",
			real_time4-real_time3,cpu_time4-cpu_time3);
		printf("Voltex Data fill in:    Real = %lf    CPU = %lf\n",
			real_time5-real_time4,cpu_time5-cpu_time4);
		printf("Voltex Drawing:    Real = %lf    CPU = %lf\n",real_time6-real_time5,
			cpu_time6-cpu_time5);
		printf("__________________________________________________\n");
		printf("Total time:        Real = %lf    CPU = %lf\n",real_time6-real_time1,
			cpu_time6-cpu_time1);
		printf("\n\n");
#endif /* defined (UNIX) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_texture_isosurface.  Invalid argument(s)");
	}
	LEAVE;
} /* draw_texture_isosurface */

void create_texture_graphics(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
==============================================================================*/
{
#if defined (OLD_CODE)
#if defined (OPENGL_API)
	Font raster_font_id;
	unsigned int raster_font_first_character,raster_font_last_character;
	XFontStruct *raster_font_info;
#endif /* defined (OPENGL_API) */
#endif /* defined (OLD_CODE) */

	ENTER(create_texture_graphics);
/*???debug */
printf("creating texture_graphics_window\n");
	if (texture_window&&
#if defined (DEBUG)
		(texture_window->graphics_window_shell=XtVaAppCreateShell(
		"texture_window_graphics_shell","Cmgui",topLevelShellWidgetClass,display,
#endif /* defined (DEBUG) */
		(texture_window->graphics_window_shell=XtVaCreatePopupShell(
		"texture_window_graphics_shell",transientShellWidgetClass,
		texture_window->user_interface->application_shell,XmNtransient,FALSE,
		/*???DB.  application_shell should be passed ? */
		XtNallowShellResize,True,NULL))&&
		(texture_window->graphics_window=XtVaCreateWidget(
		"texture_window_graphics_window",threeDDrawingWidgetClass,
		texture_window->graphics_window_shell,
		XtNwidth,500,XtNheight,500,
		X3dNbufferColourMode,X3dCOLOUR_RGB_MODE,
		X3dNbufferingMode,X3dDOUBLE_BUFFERING,NULL)))
	{
/*???debug */
printf("adding callbacks to texture_graphics_window %p\n",
	texture_window->graphics_window);
		XtAddCallback(texture_window->graphics_window,X3dNinitializeCallback,
			texture_graphics_initialize_callback,(XtPointer)texture_window);
		XtAddCallback(texture_window->graphics_window,X3dNexposeCallback,
			texture_graphics_expose_callback,(XtPointer)texture_window);
		XtAddCallback(texture_window->graphics_window,X3dNinputCallback,
			texture_graphics_input_callback,(XtPointer)texture_window);
		XtManageChild(texture_window->graphics_window);
		/*  printf("b4 realize widget\n"); */
		XtRealizeWidget(texture_window->graphics_window_shell);
		XtPopup(texture_window->graphics_window_shell,XtGrabNone);
#if defined (EXT_INPUT)
		input_module_add_input_window(texture_window->graphics_window_shell,
			texture_window->user_interface);
#endif /* defined (EXT_INPUT) */
#if defined (OLD_CODE)
/*???debug */
printf("loading mc tables\n");
		/* load marching cube tables - maybe move this? */
		load_mc_tables();
#endif /* defined (OLD_CODE) */
		/* default window settings */
		texture_window->front_plane=1;
		texture_window->back_plane=1000;
		texture_window->fovy=PI/4.0;
		texture_window->r=10.0;
#if defined (OPENGL_API)
#if defined (OLD_CODE)
		/* create the raster font */
		if (raster_font_info=XLoadQueryFont(texture_window->user_interface->display,
			"-adobe-helvetica-medium-r-normal--17-120-100-100-p-88-iso8859-1"))
		{
			raster_font_id=raster_font_info->fid;
			raster_font_first_character=raster_font_info->min_char_or_byte2;
			raster_font_last_character=raster_font_info->max_char_or_byte2;
			if (texture_window->raster_font_base=
				glGenLists(raster_font_last_character+1))
			{
				glXUseXFont(raster_font_id,raster_font_first_character,
					raster_font_last_character-raster_font_first_character+1,
					(texture_window->raster_font_base)+raster_font_first_character);
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"create_texture_graphics.  Could not allocate display list for raster font");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_texture_graphics.  Could not find raster font");
		}
#else
		/*???DB.  Should this be wrapper_init_text ? */
		initialize_graphics_library(texture_window->user_interface);
#endif /* defined (OLD_CODE) */
#endif /* defined (OPENGL_API) */
#if defined (CODE_FRAGMENTS)
		texture_window->theta=0; /* 3.0/4.0*PI */;
		texture_window->phi=0; /* PI/4.0 */;
#endif /* (CODE_FRAGMENTS) */
		/*
		texture_window->theta=3.0/4.0*PI;
		texture_window->phi=PI/4.0;
		texture_window->twist=0;
		texture_window->drawing_scale=1.0;
		*/
		texture_window->theta=0;
		texture_window->phi=0;
		texture_window->twist=0;
		texture_window->drawing_scale=1.0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_texture_graphics.  Could not create graphics window");
	}
	LEAVE;

} /* create_texture_graphics */

Boolean graphics_loop(XtPointer window)
/*******************************************************************************
LAST MODIFIED : 19 January 1998

DESCRIPTION :
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(graphics_loop);
	if (texture_window=(struct Texture_window *)window)
	{
		/* make sure that all materials are compiled */
		FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(compile_Graphical_material,
			(void *)NULL,texture_window->graphical_material_manager);
#if defined (OPENGL_API)
		/*???DB.  Should this be here ?  Is it necessary */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
#endif /* defined (OPENGL_API) */
		/* if paint mode, paint cell space (ie anything in this space is
			coloured accordingly) */
		if (texture_window->paint_mode)
		{
			paint_cell(texture_window);
		}
		if (!(texture_window->cell_mode))
		{
			value_node(texture_window);
		}
		/* if detail mode detail cell with current select value */
		if ((texture_window->detail_mode)&&(texture_window->cell_mode))
		{
			detail_cell(texture_window);
		}
		/* if fill mode fill cell */
		if ((texture_window->fill_mode)&&(texture_window->cell_mode))
		{
			fill_cell(texture_window);
		}
		if ((texture_window->fill_mode)&&!(texture_window->cell_mode))
		{
			fill_node(texture_window);
		}
		if ((texture_window->delete_mode)&&(texture_window->cell_mode))
		{
			delete_cell(texture_window);
		}
		if ((texture_window->delete_mode)&&!(texture_window->cell_mode))
		{
			delete_active_node(texture_window);
		}
		if (texture_window->delete_paint_mode)
		{
			delete_paint(texture_window);
		}
		if ((texture_window->auto_on)&&((texture_window->delete_paint_mode)||
			(texture_window->delete_mode)||(texture_window->fill_mode)||
			(texture_window->paint_mode)||(texture_window->detail_mode)))
		{
			draw_texture_isosurface(texture_window);
		}
#if defined (OPENGL_API)

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective((int) 10 * (texture_window->fovy * 180.0/PI),
			1.0,
			texture_window->front_plane,
			texture_window->back_plane);
		glGetDoublev(GL_PROJECTION_MATRIX, texture_window->projection_matrix);
		glGetIntegerv(GL_VIEWPORT, texture_window->viewport);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0.0, 0.0, -texture_window->r);
		glRotatef (-10 * (texture_window->twist * 180/PI),0.0,0.0,1.0);
		glRotatef (-10 * (texture_window->phi * 180/PI),1.0,0.0,0.0);
		glRotatef (-10 * (texture_window->theta * 180.0/PI),0.0,0.0,1.0);

#if defined (USEMANAGER)
		reset_Lights();
		execute_Light(vt_ed_light);
#else /* defined (USEMANAGER) */
		reset_Lights();
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
#endif /* defined (USEMANAGER) */
		glClearColor(0.39,0.19,0.29,0.0);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT);
		glMultMatrixf(texture_window->model_matrix);
		glGetDoublev(GL_MODELVIEW_MATRIX, texture_window->modelview_matrix);
		glColor3b(255,255,255);
		glCallList(texture_window->tw_axes);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
#if defined (OLD_CODE)
		winset(texture_window->graphics_window);
#endif /* defined (OLD_CODE) */
		loadmatrix(idmat);
		perspective((int)10*(texture_window->fovy*180.0/PI),1.0,
			texture_window->front_plane,texture_window->back_plane);
		mmode(MVIEWING);
		loadmatrix(idmat);
		polarview(texture_window->r,
			(int)10*(texture_window->theta*180.0/PI),
			(int)10*(texture_window->phi*180/PI),
			(int)10*(texture_window->twist*180/PI));
		reset_Lights();
		execute_Light(vt_ed_light);
		RGBcolor(75,50,100);
		clear();
		zclear();
		multmatrix(texture_window->model_matrix);
		RGBcolor(255,255,255);
		callobj(texture_window->tw_axes);
#endif /* defined (GL_API) */
		if (texture_window->grid_on)
		{
#if defined (OPENGL_API)
			glColor3b(200,100,100);
			glCallList(texture_window->tw_grid);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
			RGBcolor(200,200,200);
			callobj(texture_window->tw_grid);
#endif /* defined (GL_API) */
		}
		if (texture_window->line_mode_on)
		{
			draw_texture_lines(texture_window);
#if defined (OPENGL_API)
			glCallList(texture_window->tw_lines);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
			callobj(texture_window->tw_lines);
#endif /* defined (GL_API) */
		}
		if (texture_window->curve_mode_on)
		{
			draw_texture_curves(texture_window);
#if defined (OPENGL_API)
			glCallList(texture_window->tw_curves);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
			callobj(texture_window->tw_curves);
#endif /* defined (GL_API) */
		}
		if (texture_window->blob_mode_on)
		{
			draw_texture_blobs(texture_window);
#if defined (OPENGL_API)
			glCallList(texture_window->tw_blobs);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
			callobj(texture_window->tw_blobs);
#endif /* defined (GL_API) */
		}
		if (texture_window->soft_mode_on)
		{
			draw_texture_softs(texture_window);
#if defined (OPENGL_API)
			glCallList(texture_window->tw_softs);
#endif /* defined (OPENGL_API) */

		}
		if (texture_window->cubes_on && texture_window->see_paint_on)
		{
			draw_texture_nodes(texture_window);
#if defined (OPENGL_API)
			glCallList(texture_window->tw_nodes);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
			callobj(texture_window->tw_nodes);
#endif /* defined (GL_API) */
		}
		if (texture_window->cubes_on)
		{
			draw_texture_cells(texture_window);
#if defined (OPENGL_API)
			glCallList(texture_window->tw_3dtexture);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
			callobj(texture_window->tw_3dtexture);
#endif /* defined (GL_API) */
		}
		if (texture_window->isosurface_on)
		{
#if defined (OPENGL_API)
			glCallList(texture_window->tw_isosurface);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
			callobj(texture_window->tw_isosurface);
#endif /* defined (GL_API) */
		}
		execute_Graphical_material(texture_window->current_material);
		/* printf("In graphics loop9\n"); */
		/* draw the current cursor position */
		draw_current_cell(texture_window);
		/* printf("In graphics loop10\n"); */
		if (texture_window->edit_group_mode)
		{
			draw_current_group(texture_window);
		}

#if defined (DEBUG)
{
GLfloat params[4];

/* test variables to see what the hell is going on */
printf("1: Lighting Diagnostics\n");
printf("LightingEnabled = %d\n", glIsEnabled(GL_LIGHTING));
printf("Lights Enabled: %d %d %d %d %d %d %d %d\n", glIsEnabled(GL_LIGHT0),
	glIsEnabled(GL_LIGHT1), glIsEnabled(GL_LIGHT2), glIsEnabled(GL_LIGHT3),
	glIsEnabled(GL_LIGHT4), glIsEnabled(GL_LIGHT5), glIsEnabled(GL_LIGHT6),
	glIsEnabled(GL_LIGHT7));
glGetLightfv(GL_LIGHT0, GL_AMBIENT, params);
printf("GL_AMBIENT = %f %f %f %f\n", params[0], params[1], params[2], params[3]);
glGetLightfv(GL_LIGHT0,GL_DIFFUSE, params);
printf("GL_DIFFUSE = %f %f %f %f\n", params[0], params[1], params[2], params[3]);
glGetLightfv(GL_LIGHT0,GL_POSITION, params);
printf("GL_POSITION = %f %f %f %f\n", params[0], params[1], params[2], params[3]);

}
#endif

#if defined (GL_API)
#if defined (OLD_CODE)
		swapbuffers();
#endif /* defined (OLD_CODE) */
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"graphics_loop.  Missing window");
	}
	LEAVE;

	return (TRUE);
} /* graphics_loop */

void close_texture_graphics(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 30 July 1996

DESCRIPTION :
==============================================================================*/
{
	ENTER(close_texture_graphics);
	/* checking argument */
	if (texture_window)
	{
#if defined (OLD_CODE)
#if defined (GL_API)
		winclose(texture_window->graphics_window);
#endif /* defined (GL_API) */
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"close_texture_graphics.  Invalid argument");
	}
	LEAVE;
} /* close_texture_graphics */

void makewiresphere(void)
/*******************************************************************************
LAST MODIFIED : 25 September 1996

DESCRIPTION :
==============================================================================*/
{
	double a;

#if defined (OPENGL_API)
	glBegin(GL_LINE_LOOP);
	for (a=0;a<2*PI;a+=2*PI/18)
	{
			glVertex3f(cos(a), sin(a), 0);
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	for (a=0;a<2*PI;a+=2*PI/18)
	{
			glVertex3f(0, cos(a), sin(a));
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	for (a=0;a<2*PI;a+=2*PI/18)
	{
			glVertex3f(cos(a),0,sin(a));
	}
	glEnd();
#endif /* defined (OPENGL_API) */
} /* makesphere */

void make_objects(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 15 January 1998

DESCRIPTION :
==============================================================================*/
{
	int i;

	ENTER(make_objects);
	/* checking arguments */
	if (texture_window)
	{
#if defined (OPENGL_API)
		glNewList(texture_window->tw_envsquare,GL_COMPILE);
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[1]);
		glTexCoord2fv(cube_tex[0]);
		glVertex3fv(box[0]);
		glTexCoord2fv(cube_tex[1]);
		glVertex3fv(box[1]);
		glTexCoord2fv(cube_tex[2]);
		glVertex3fv(box[2]);
		glTexCoord2fv(cube_tex[3]);
		glVertex3fv(box[3]);
		glEnd();
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glColor3b(255,0,0);
		glBegin(GL_LINE_STRIP);
		glVertex3f(box[0][0],box[0][1],box[0][2]);
		glVertex3f(box[1][0],box[1][1],box[1][2]);
		glVertex3f(box[2][0],box[2][1],box[2][2]);
		glVertex3f(box[3][0],box[3][1],box[3][2]);
		glVertex3f(box[0][0],box[0][1],box[0][2]);
		glEnd();
		glPopAttrib();
		glEndList();
		glNewList(texture_window->tw_axes,GL_COMPILE);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3f(1.5,0,0);
		glVertex3f(0,0,0);
		glVertex3f(0,1.5,0);
		glVertex3f(0,0,0);
		glVertex3f(0,0,1.5);
		glEnd();
		glRasterPos3f(1.5,0,0);
#if defined (OLD_CODE)
		glPushAttrib(GL_LIST_BIT);
		glListBase(texture_window->raster_font_base+24);
		glCallLists(strlen("Xi1"),GL_UNSIGNED_BYTE,"Xi1");
		glPopAttrib();
#else
		wrapperPrintText("Xi1");
#endif /* defined (OLD_CODE) */
		glRasterPos3f(0,1.5,0);
#if defined (OLD_CODE)
		glPushAttrib(GL_LIST_BIT);
		glListBase(texture_window->raster_font_base+24);
		glCallLists(strlen("Xi2"),GL_UNSIGNED_BYTE,"Xi2");
		glPopAttrib();
#else
		wrapperPrintText("Xi2");
#endif /* defined (OLD_CODE) */
		glRasterPos3f(0,0,1.5);
#if defined (OLD_CODE)
		glPushAttrib(GL_LIST_BIT);
		glListBase(texture_window->raster_font_base+24);
		glCallLists(strlen("Xi3"),GL_UNSIGNED_BYTE,"Xi3");
		glPopAttrib();
#else
		wrapperPrintText("Xi3");
#endif /* defined (OLD_CODE) */
		glPopAttrib();
		glEndList();
		glNewList(texture_window->tw_cube_tex[4],GL_COMPILE);
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[0]);
		glTexCoord2fv(cube_tex[1]);
		glVertex3fv(cube_vert[0]);
		glTexCoord2fv(cube_tex[0]);
		glVertex3fv(cube_vert[1]);
		glTexCoord2fv(cube_tex[3]);
		glVertex3fv(cube_vert[2]);
		glTexCoord2fv(cube_tex[2]);
		glVertex3fv(cube_vert[3]);
		glEnd();
		glEndList();
		glNewList(texture_window->tw_cube_tex[5],GL_COMPILE);
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[1]);
		glTexCoord2fv(cube_tex[0]);
		glVertex3fv(cube_vert[4]);
		glTexCoord2fv(cube_tex[1]);
		glVertex3fv(cube_vert[5]);
		glTexCoord2fv(cube_tex[2]);
		glVertex3fv(cube_vert[6]);
		glTexCoord2fv(cube_tex[3]);
		glVertex3fv(cube_vert[7]);
		glEnd();
		glEndList();
		glNewList(texture_window->tw_cube_tex[2],GL_COMPILE);
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[2]);
		glTexCoord2fv(cube_tex[0]);
		glVertex3fv(cube_vert[0]);
		glTexCoord2fv(cube_tex[1]);
		glVertex3fv(cube_vert[1]);
		glTexCoord2fv(cube_tex[2]);
		glVertex3fv(cube_vert[5]);
		glTexCoord2fv(cube_tex[3]);
		glVertex3fv(cube_vert[4]);
		glEnd();
		glEndList();
		glNewList(texture_window->tw_cube_tex[1],GL_COMPILE);
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[3]);
		glTexCoord2fv(cube_tex[1]);
		glVertex3fv(cube_vert[1]);
		glTexCoord2fv(cube_tex[2]);
		glVertex3fv(cube_vert[2]);
		glTexCoord2fv(cube_tex[3]);
		glVertex3fv(cube_vert[6]);
		glTexCoord2fv(cube_tex[0]);
		glVertex3fv(cube_vert[5]);
		glEnd();
		glEndList();
		glNewList(texture_window->tw_cube_tex[3],GL_COMPILE);
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[4]);
		glTexCoord2fv(cube_tex[2]);
		glVertex3fv(cube_vert[2]);
		glTexCoord2fv(cube_tex[3]);
		glVertex3fv(cube_vert[3]);
		glTexCoord2fv(cube_tex[0]);
		glVertex3fv(cube_vert[7]);
		glTexCoord2fv(cube_tex[1]);
		glVertex3fv(cube_vert[6]);
		glEnd();
		glEndList();
		glNewList(texture_window->tw_cube_tex[0],GL_COMPILE);
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[5]);
		glTexCoord2fv(cube_tex[3]);
		glVertex3fv(cube_vert[3]);
		glTexCoord2fv(cube_tex[0]);
		glVertex3fv(cube_vert[0]);
		glTexCoord2fv(cube_tex[1]);
		glVertex3fv(cube_vert[4]);
		glTexCoord2fv(cube_tex[2]);
		glVertex3fv(cube_vert[7]);
		glEnd();
		glEndList();
		glNewList(texture_window->tw_cube,GL_COMPILE);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
#if defined (OLD_CODE)
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
#endif /* defined (OLD_CODE) */
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[0]);
		glVertex3fv(cube_vert[0]);
		glVertex3fv(cube_vert[1]);
		glVertex3fv(cube_vert[2]);
		glVertex3fv(cube_vert[3]);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[1]);
		glVertex3fv(cube_vert[4]);
		glVertex3fv(cube_vert[5]);
		glVertex3fv(cube_vert[6]);
		glVertex3fv(cube_vert[7]);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[2]);
		glVertex3fv(cube_vert[0]);
		glVertex3fv(cube_vert[1]);
		glVertex3fv(cube_vert[5]);
		glVertex3fv(cube_vert[4]);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[3]);
		glVertex3fv(cube_vert[1]);
		glVertex3fv(cube_vert[2]);
		glVertex3fv(cube_vert[6]);
		glVertex3fv(cube_vert[5]);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[4]);
		glVertex3fv(cube_vert[2]);
		glVertex3fv(cube_vert[3]);
		glVertex3fv(cube_vert[7]);
		glVertex3fv(cube_vert[6]);
		glEnd();
		glBegin(GL_POLYGON);
		glNormal3fv(cube_normals[5]);
		glVertex3fv(cube_vert[3]);
		glVertex3fv(cube_vert[0]);
		glVertex3fv(cube_vert[4]);
		glVertex3fv(cube_vert[7]);
		glEnd();
		glPopAttrib();
		glEndList();
		glNewList(texture_window->tw_wire_cube,GL_COMPILE);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glColor3b(0,255,0);
		glBegin(GL_LINE_STRIP);
		glVertex3f(0,0,0);
		glVertex3f(0,1,0);
		glVertex3f(1,1,0);
		glVertex3f(1,0,0);
		glVertex3f(0,0,0);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex3f(0,0,1);
		glVertex3f(0,1,1);
		glVertex3f(1,1,1);
		glVertex3f(1,0,1);
		glVertex3f(0,0,1);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(0,0,0);glVertex3f(0,0,1);
		glVertex3f(1,0,0);glVertex3f(1,0,1);
		glVertex3f(0,1,0);glVertex3f(0,1,1);
		glVertex3f(1,1,0);glVertex3f(1,1,1);
		glEnd();
		glPopAttrib();
		glEndList();
		glNewList(texture_window->tw_cop,GL_COMPILE);
		glColor3b(255,0,255);
		glBegin(GL_LINES);
		glVertex3f(0,0,0);glVertex3f(1,1,1);
		glVertex3f(0,0,1);glVertex3f(1,1,0);
		glVertex3f(0,1,0);glVertex3f(1,0,1);
		glVertex3f(1,0,0);glVertex3f(0,1,1);
		glEnd();
		glEndList();
		glNewList(texture_window->tw_box,GL_COMPILE);
		glBegin(GL_LINE_STRIP);
		glVertex3f(box[0][0],box[0][1],box[0][2]);
		for (i=1;i<4;i++)
		{
			glVertex3f(box[i][0],box[i][1],box[i][2]);
		}
		glVertex3f(box[0][0],box[0][1],box[0][2]);
		glEnd();
		glEndList();
		glNewList(texture_window->tw_mouseplane,GL_COMPILE);
#if defined (OLD_CODE)
		bgnpolygon();
		v3f(cube_vert[3]);
		v3f(cube_vert[2]);
		v3f(cube_vert[1]);
		v3f(cube_vert[0]);
		endpolygon();
		RGBcolor(255,0,0);
		linewidth(2);
		move(0,0,0);draw(1.5,0,0);move(1.5,.1,0);draw(1.6,0,0);draw(1.5,-.1,0);
		move(0,0,0);draw(0,1.5,0);move(.1,1.5,0);draw(0,1.6,0);draw(-.1,1.5,0);
		linewidth(1);
		cmov(1.7,0,0);
		charstr("x");
		cmov(0,1.7,0);
		charstr("y");
#endif /* defined (OLD_CODE) */
		glEndList();
		glNewList(texture_window->tw_wiresphere,GL_COMPILE);
		makewiresphere();
		glEndList();
		glNewList(texture_window->tw_sphere,GL_COMPILE);
		makesphere(1.0,0.0,PI,15,15);
		glEndList();
		glNewList(texture_window->tw_small_sphere,GL_COMPILE);
		makesphere(2.0,0.0,PI/2.0,3,6);
		glEndList();
		glNewList(texture_window->tw_env_map_sphere,GL_COMPILE);
		makesphere(0.5,0.0,PI,4,4);
		glEndList();
		/*glNewList(texture_window->tw_isosurface,GL_COMPILE);
		glEndList();*/
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		makeobj(texture_window->tw_envsquare);
		bgnpolygon();
		n3f(cube_normals[1]);
		t2f(cube_tex[0]);
		v3f(box[0]);
		t2f(cube_tex[1]);
		v3f(box[1]);
		t2f(cube_tex[2]);
		v3f(box[2]);
		t2f(cube_tex[3]);
		v3f(box[3]);
		endpolygon();
		RGBcolor(255,0,0);
		move(box[0][0],box[0][1],box[0][2]);
		draw(box[1][0],box[1][1],box[1][2]);
		draw(box[2][0],box[2][1],box[2][2]);
		draw(box[3][0],box[3][1],box[3][2]);
		draw(box[0][0],box[0][1],box[0][2]);
		closeobj();
		makeobj(texture_window->tw_axes);
		move(0,0,0);
		draw(1.5,0,0);
		move(0,0,0);
		draw(0,1.5,0);
		move(0,0,0);
		draw(0,0,1.5);
		cmov(1.5,0,0);
		charstr("1");
		cmov(0,1.5,0);
		charstr("2");
		cmov(0,0,1.5);
		charstr("3");
		closeobj();
		makeobj(texture_window->tw_cube_tex[4]);
		bgnpolygon();
		n3f(cube_normals[0]);
		t2f(cube_tex[1]);
		v3f(cube_vert[0]);
		t2f(cube_tex[0]);
		v3f(cube_vert[1]);
		t2f(cube_tex[3]);
		v3f(cube_vert[2]);
		t2f(cube_tex[2]);
		v3f(cube_vert[3]);
		endpolygon();
		closeobj();
		makeobj(texture_window->tw_cube_tex[5]);
		bgnpolygon();
		n3f(cube_normals[1]);
		t2f(cube_tex[0]);
		v3f(cube_vert[4]);
		t2f(cube_tex[1]);
		v3f(cube_vert[5]);
		t2f(cube_tex[2]);
		v3f(cube_vert[6]);
		t2f(cube_tex[3]);
		v3f(cube_vert[7]);
		endpolygon();
		closeobj();
		makeobj(texture_window->tw_cube_tex[2]);
		bgnpolygon();
		n3f(cube_normals[2]);
		t2f(cube_tex[0]);
		v3f(cube_vert[0]);
		t2f(cube_tex[1]);
		v3f(cube_vert[1]);
		t2f(cube_tex[2]);
		v3f(cube_vert[5]);
		t2f(cube_tex[3]);
		v3f(cube_vert[4]);
		endpolygon();
		closeobj();
		makeobj(texture_window->tw_cube_tex[1]);
		bgnpolygon();
		n3f(cube_normals[3]);
		t2f(cube_tex[1]);
		v3f(cube_vert[1]);
		t2f(cube_tex[2]);
		v3f(cube_vert[2]);
		t2f(cube_tex[3]);
		v3f(cube_vert[6]);
		t2f(cube_tex[0]);
		v3f(cube_vert[5]);
		endpolygon();
		closeobj();
		makeobj(texture_window->tw_cube_tex[3]);
		bgnpolygon();
		n3f(cube_normals[4]);
		t2f(cube_tex[2]);
		v3f(cube_vert[2]);
		t2f(cube_tex[3]);
		v3f(cube_vert[3]);
		t2f(cube_tex[0]);
		v3f(cube_vert[7]);
		t2f(cube_tex[1]);
		v3f(cube_vert[6]);
		endpolygon();
		closeobj();
		makeobj(texture_window->tw_cube_tex[0]);
		bgnpolygon();
		n3f(cube_normals[5]);
		t2f(cube_tex[3]);
		v3f(cube_vert[3]);
		t2f(cube_tex[0]);
		v3f(cube_vert[0]);
		t2f(cube_tex[1]);
		v3f(cube_vert[4]);
		t2f(cube_tex[2]);
		v3f(cube_vert[7]);
		endpolygon();
		closeobj();
		makeobj(texture_window->tw_cube);
		bgnpolygon();
		n3f(cube_normals[0]);
		v3f(cube_vert[0]);
		v3f(cube_vert[1]);
		v3f(cube_vert[2]);
		v3f(cube_vert[3]);
		endpolygon();
		bgnpolygon();
		n3f(cube_normals[1]);
		v3f(cube_vert[4]);
		v3f(cube_vert[5]);
		v3f(cube_vert[6]);
		v3f(cube_vert[7]);
		endpolygon();
		bgnpolygon();
		n3f(cube_normals[2]);
		v3f(cube_vert[0]);
		v3f(cube_vert[1]);
		v3f(cube_vert[5]);
		v3f(cube_vert[4]);
		endpolygon();
		bgnpolygon();
		n3f(cube_normals[3]);
		v3f(cube_vert[1]);
		v3f(cube_vert[2]);
		v3f(cube_vert[6]);
		v3f(cube_vert[5]);
		endpolygon();
		bgnpolygon();
		n3f(cube_normals[4]);
		v3f(cube_vert[2]);
		v3f(cube_vert[3]);
		v3f(cube_vert[7]);
		v3f(cube_vert[6]);
		endpolygon();
		bgnpolygon();
		n3f(cube_normals[5]);
		v3f(cube_vert[3]);
		v3f(cube_vert[0]);
		v3f(cube_vert[4]);
		v3f(cube_vert[7]);
		endpolygon();
		closeobj();
		makeobj(texture_window->tw_wire_cube);
		RGBcolor(0,255,0);
		linewidth(3);
		move(0,0,0);
		draw(0,1,0);
		draw(1,1,0);
		draw(1,0,0);
		draw(0,0,0);
		move(0,0,1);
		draw(0,1,1);
		draw(1,1,1);
		draw(1,0,1);
		draw(0,0,1);
		move(0,0,0);
		draw(0,0,1);
		move(1,0,0);
		draw(1,0,1);
		move(0,1,0);
		draw(0,1,1);
		move(1,1,0);
		draw(1,1,1);
		linewidth(1);
		closeobj();
		makeobj(texture_window->tw_cop);
		RGBcolor(255,0,255);
		linewidth(3);
		move(0,0,0);
		draw(1,1,1);
		move(0,0,1);
		draw(1,1,0);
		move(0,1,0);
		draw(1,0,1);
		move(1,0,0);
		draw(0,1,1);
		linewidth(1);
		closeobj();
		makeobj(texture_window->tw_box);
		move(box[0][0],box[0][1],box[0][2]);
		for (i=1;i<4;i++)
		{
			draw(box[i][0],box[i][1],box[i][2]);
		}
		draw(box[0][0],box[0][1],box[0][2]);
		closeobj();
		makeobj(texture_window->tw_mouseplane);
		cpack(0x800000ff);
		bgnpolygon();
		v3f(cube_vert[3]);
		v3f(cube_vert[2]);
		v3f(cube_vert[1]);
		v3f(cube_vert[0]);
		endpolygon();
		RGBcolor(255,0,0);
		linewidth(2);
		move(0,0,0);
		draw(1.5,0,0);
		move(1.5,.1,0);
		draw(1.6,0,0);
		draw(1.5,-.1,0);
		move(0,0,0);
		draw(0,1.5,0);
		move(.1,1.5,0);
		draw(0,1.6,0);
		draw(-.1,1.5,0);
		linewidth(1);
		cmov(1.7,0,0);
		charstr("x");
		cmov(0,1.7,0);
		charstr("y");
		closeobj();
		makeobj(texture_window->tw_sphere);
		makesphere(1.0,0.0,PI,15,15);
		closeobj();
		makeobj(texture_window->tw_small_sphere);
		makesphere(2.0,0.0,PI/2.0,3,6);
		closeobj();
		makeobj(texture_window->tw_env_map_sphere);
		makesphere(0.5,0.0,PI,4,4);
		closeobj();
		makeobj(texture_window->tw_isosurface);
		closeobj();
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_objects.  Invalid argument");
	}
	LEAVE;
} /* make_objects */

void update_grid(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
==============================================================================*/
{
	/* draw grid lines */
	double m[3],M[3],r[3],xivali, xivalj;
	int i,j,k, R[3], dim[3];
	struct VT_texture_node *node;
	double xistep[3];
	int nx,ny;
	double *grid_spacing,sx,sy,sz,tx,ty,tz;

	ENTER(update_grid);
	/* checking argument */
	if (texture_window)
	{
#if defined (OPENGL_API)
		glNewList(texture_window->tw_grid,GL_COMPILE);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
#endif
#if defined (GL_API)
		delobj(texture_window->tw_grid);
		makeobj(texture_window->tw_grid);
#endif /* defined (GL_API) */
		for (i=0;i<3;i++)
		{
			m[i]=texture_window->ximin[i];
			M[i]=texture_window->ximax[i];
			r[i]=texture_window->xires[i];
			R[i]=texture_window->xires[i];
			dim[i]=texture_window->current_texture->dimension[i]+1;
		}
		/* draw lines in xi 0 direction */
		for (i=0;i<=r[1];i++)
		{
			for (j=0;j<=r[2];j++)
			{

				xivali=(double)i*(M[1]-m[1])/r[1]+m[1];
				xivalj=(double)j*(M[2]-m[2])/r[2]+m[2];
				if (texture_window->current_texture->grid_spacing)
				{
				xivali = texture_window->current_texture->grid_spacing[R[0]+1+i]*(M[1]-m[1])+m[1];
				xivalj = texture_window->current_texture->grid_spacing[R[0]+R[1]+2+j]*(M[2]-m[2])+m[2];

				}
#if defined (OPENGL_API)
				glBegin(GL_LINES);
				glVertex3f(m[0],xivali,xivalj);
				glVertex3f(M[0],xivali,xivalj);
				glEnd();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
				move(m[0],xivali,xivalj);
				draw(M[0],xivali,xivalj);
#endif /* defined (GL_API) */
			}
		}
		/* xi 2 direction */
		for (i=0;i<=r[0];i++)
		{
			for (j=0;j<=r[1];j++)
			{

				xivali=(double)i*(M[0]-m[0])/r[0]+m[0];
				xivalj=(double)j*(M[1]-m[1])/r[1]+m[1];
				if (texture_window->current_texture->grid_spacing)
				{
					xivali = texture_window->current_texture->grid_spacing[i]*(M[0]-m[0])+m[0];
					xivalj = texture_window->current_texture->grid_spacing[R[0]+1+j]*(M[1]-m[1])+m[1];

				}
#if defined (OPENGL_API)
				glBegin(GL_LINES);
				glVertex3f(xivali,xivalj,m[2]);
				glVertex3f(xivali,xivalj,M[2]);
				glEnd();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
				move(xivali,xivalj,m[2]);
				draw(xivali,xivalj,M[2]);
#endif /* defined (GL_API) */
			}
		}
		/* xi 1 direction */
		for (i=0;i<=r[0];i++)
		{
			for (j=0;j<=r[2];j++)
			{
				xivali=(double)i*(M[0]-m[0])/r[0]+m[0];
				xivalj=(double)j*(M[2]-m[2])/r[2]+m[2];
				if (texture_window->current_texture->grid_spacing)
				{
					xivali = texture_window->current_texture->grid_spacing[i]*(M[0]-m[0])+m[0];
					xivalj = texture_window->current_texture->grid_spacing[R[0]+R[1]+2+j]*(M[2]-m[2])+m[2];
				}
#if defined (OPENGL_API)
				glBegin(GL_LINES);
				glVertex3f(xivali,m[1],xivalj);
				glVertex3f(xivali,M[1],xivalj);
				glEnd();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
				move(xivali,m[1],xivalj);
				draw(xivali,M[1],xivalj);
#endif /* defined (GL_API) */
			}
		}

/* search for split nodes and draw points on either side of slit */
		if (texture_window->current_texture->grid_spacing)
		{
			nx = texture_window->current_texture->dimension[0]+1;
			ny = texture_window->current_texture->dimension[1]+1;
#if defined (OLD_CODE)
			nz = texture_window->current_texture->dimension[2]+1;
#endif /* defined (OLD_CODE) */
			grid_spacing = texture_window->current_texture->grid_spacing;
			sx = (texture_window->ximax[0]-texture_window->ximin[0]);
			sy = (texture_window->ximax[1]-texture_window->ximin[1]);
			sz = (texture_window->ximax[2]-texture_window->ximin[2]);
			tx = texture_window->ximin[0];
			ty = texture_window->ximin[1];
			tz = texture_window->ximin[2];
		}
		else
		{
			grid_spacing = (double *) NULL;
		}
		for (i=0;i<3;i++)
		{
			xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
				texture_window->xires[i];
		}
	for (i=0;i<dim[0];i++)
	{
		for (j=0;j<dim[1];j++)
		{
			for (k=0;k<dim[2];k++)
			{
				node=texture_window->current_texture->global_texture_node_list[i+j*dim[0]+
						k*dim[0]*dim[1]];
				if (node->node_type > 0)
				{
					glPushMatrix();
					if (texture_window->current_texture->grid_spacing)
					{
						xivali = texture_window->current_texture->grid_spacing[i]*(M[0]-m[0])+m[0];
						xivalj = texture_window->current_texture->grid_spacing[R[0]+1+j]*(M[1]-m[1])+m[1];
#if defined (OLD_CODE)
						xivalk = texture_window->current_texture->grid_spacing[R[0]+R[1]+2+k]*(M[2]-m[2])+m[2];
#endif /* defined (OLD_CODE) */

						glTranslatef(grid_spacing[i]*sx+tx,
							grid_spacing[nx+j]*sy+ty,
							grid_spacing[nx+ny+k]*sz+tz);
						glScalef(grid_spacing[i+1]-grid_spacing[i],
							grid_spacing[nx+j+1]-grid_spacing[nx+j],
							grid_spacing[nx+ny+k+1]-grid_spacing[nx+ny+k]);
						glScalef(sx,sy,sz);
					}
					else
					{
						xivali = (double)i*(M[0]-m[0])/r[0]+m[0];
						xivalj = (double)j*(M[1]-m[1])/r[1]+m[1];
#if defined (OLD_CODE)
						xivalk = (double)k*(M[2]-m[2])/r[2]+m[2];
#endif /* defined (OLD_CODE) */
						glTranslatef(texture_window->ximin[0] + i * xistep[0],
							texture_window->ximin[1] + j * xistep[1],
							texture_window->ximin[2] + k * xistep[2]);
						glScalef(xistep[0],xistep[1],xistep[2]);
					}

					glBegin(GL_LINES);
					switch(node->node_type)
					{
						case 1:	/* slit in xi1 */
							/* draw a node either side of xivali */
							glColor3f(0, 0, 0);
							glVertex3f(-0.15, 0, 0);
							glVertex3f(0.15, 0, 0);
							break;
						case 2: /* slit in xi2 */
							glColor3f(0, 0, 0);
							glVertex3f(0, -0.15, 0);
							glVertex3f(0, 0.15, 0);
							break;
						case 4: /* slit in xi3 */
							glColor3f(0, 0, 0);
							glVertex3f(0, 0, -0.15);
							glVertex3f(0, 0, 0.15);
						default:
							break;
					}
					glEnd();
					glPointSize(4.0);
					glBegin(GL_POINTS);
					switch(node->node_type)
					{
						case 1:	/* slit in xi1 */
							/* draw a node either side of xivali */
							glColor3f(1.0, 0, 0);
							glVertex3f(-0.15, 0, 0);
							glVertex3f(0.15, 0, 0);
							break;
						case 2: /* slit in xi2 */
							glColor3f(1.0, 0, 0);
							glVertex3f(0, -0.15, 0);
							glVertex3f(0, 0.15, 0);
							break;
						case 4: /* slit in xi3 */
							glColor3f(1.0, 0, 0);
							glVertex3f(0, 0, -0.15);
							glVertex3f(0, 0, 0.15);
						default:
							break;
					}
					glEnd();

					glPopMatrix();
				}
			}
		}
	}



#if defined (OPENGL_API)
		glPopAttrib();
		glEndList();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		closeobj();
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"update_grid.  Invaild argument");
	}
	LEAVE;
} /* update_grid */

void draw_current_cell(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
==============================================================================*/
{
	double xistep[3];
	int i, int_xival[3], int_cop[3], nx, ny;
	struct Environment_map *env_map;
	double *grid_spacing,sx,sy,sz,tx,ty,tz;
	GLfloat no_mat[] = {0, 0, 0, 1.0};
	GLfloat mat_diffuse[] = {0, 1.0, 1.0, 1.0};
	GLfloat mat_specular[] = {1.0, 0, 1.0, 1.0};
	GLfloat mat_shininess[] = {50.0};

	ENTER(draw_current_cell);
	/* checking arguments */
	if (texture_window)
	{
		if (texture_window->current_texture->grid_spacing)
		{
			nx = texture_window->current_texture->dimension[0]+1;
			ny = texture_window->current_texture->dimension[1]+1;
#if defined (OLD_CODE)
			nz = texture_window->current_texture->dimension[2]+1;
#endif /* defined (OLD_CODE) */
			grid_spacing = texture_window->current_texture->grid_spacing;
			sx = (texture_window->ximax[0]-texture_window->ximin[0]);
			sy = (texture_window->ximax[1]-texture_window->ximin[1]);
			sz = (texture_window->ximax[2]-texture_window->ximin[2]);
			tx = texture_window->ximin[0];
			ty = texture_window->ximin[1];
			tz = texture_window->ximin[2];
		}
		else
		{
			grid_spacing = (double *) NULL;
		}
		for (i=0;i<3;i++)
		{
			xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
				texture_window->xires[i];

			if (texture_window->cell_mode)
			{
				int_xival[i]=(int)((double)(texture_window->xival[i]-
				texture_window->ximin[i])/xistep[i]);
			}
			else
			{
				int_xival[i]=(int)((double)(texture_window->xival[i]-
				(texture_window->ximin[i]-0.5*xistep[i]/texture_window->xires[i]))/
				xistep[i]);
			}
			int_cop[i]=(int)((double)(texture_window->cop[i]-texture_window->
				ximin[i])/xistep[i]);
		}
#if defined (OPENGL_API)
		glPushMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		pushmatrix();
#endif /* defined (GL_API) */
		if (texture_window->cell_mode)
		{
#if defined (OPENGL_API)
				if (grid_spacing)
				{
			glTranslatef(grid_spacing[int_xival[0]]*sx+tx,
				grid_spacing[nx+int_xival[1]]*sy+ty,
				grid_spacing[nx+ny+int_xival[2]]*sz+tz);
			glScalef(grid_spacing[int_xival[0]+(int)texture_window->brushsize[0]]-grid_spacing[int_xival[0]],
				grid_spacing[nx+int_xival[1]+(int)texture_window->brushsize[1]]-grid_spacing[nx+int_xival[1]],
				grid_spacing[nx+ny+int_xival[2]+(int)texture_window->brushsize[1]]-grid_spacing[nx+ny+int_xival[2]]);
			glScalef(sx,sy,sz);
			glColor3f(0,0,1.0);
			/*cpack(0x8000ff00);*/
			glPushMatrix();
				}
				else
				{
			glTranslatef(texture_window->ximin[0] + int_xival[0] * xistep[0],
				texture_window->ximin[1] + int_xival[1] * xistep[1],
				texture_window->ximin[2] + int_xival[2] * xistep[2]);
			glScalef(xistep[0],xistep[1],xistep[2]);
			glColor3f(0,0,1.0);
			/*cpack(0x8000ff00);*/
			glPushMatrix();
			glScalef((float)texture_window->brushsize[0],
				(float)texture_window->brushsize[1],
				(float)texture_window->brushsize[2]);
			}
			if (!texture_window->cop_mode_on)
			{
				if (texture_window->env_mode_on)
				{
					if (env_map=texture_window->current_env_map)
					{
						for (i=0;i<6;i++)
						{
							execute_Graphical_material(env_map->face_material[i]);
							glCallList(texture_window->tw_cube_tex[i]);
						}
					}
				}
				else
				{
					glCallList(texture_window->tw_cube);
				}
			}
			glCallList(texture_window->tw_wire_cube);
			glPopMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
			translate(texture_window->ximin[0]+int_xival[0]*xistep[0],
				texture_window->ximin[1]+int_xival[1]*xistep[1],
				texture_window->ximin[2]+int_xival[2]*xistep[2]);
			scale(xistep[0],xistep[1],xistep[2]);
			cpack(0x8000ff00);
			pushmatrix();
			scale((float)texture_window->brushsize[0],
				(float)texture_window->brushsize[1],
				(float)texture_window->brushsize[2]);
			if (!texture_window->cop_mode_on)
			{
				if (texture_window->env_mode_on)
				{
					if (env_map=texture_window->current_env_map)
					{
						for (i=0;i<6;i++)
						{
							execute_Graphical_material(env_map->face_material[i]);
							callobj(texture_window->tw_cube_tex[i]);
						}
					}
				}
				else
				{
					callobj(texture_window->tw_cube);
				}
			}
			callobj(texture_window->tw_wire_cube);
			popmatrix();
#endif /* defined (GL_API) */
		}
		else
		{
#if defined (OPENGL_API)
			if (grid_spacing)
			{
				glTranslatef(grid_spacing[int_xival[0]]*sx+tx,
				grid_spacing[nx+int_xival[1]]*sy+ty,
				grid_spacing[nx+ny+int_xival[2]]*sz+tz);
				if (!(texture_window->soft_mode_on))
				{
					glScalef(grid_spacing[int_xival[0]+(int)texture_window->brushsize[0]]
						-grid_spacing[int_xival[0]],grid_spacing[nx+int_xival[1]+
						(int)texture_window->brushsize[1]]-grid_spacing[nx+int_xival[1]],
						grid_spacing[nx+ny+int_xival[2]+(int)texture_window->brushsize[1]]-
						grid_spacing[nx+ny+int_xival[2]]);
					glScalef(sx,sy,sz);
				}
			}
			else
			{
				glTranslatef(texture_window->ximin[0] + int_xival[0] * (xistep[0]),
				texture_window->ximin[1] + int_xival[1] * (xistep[1]),
				texture_window->ximin[2] + int_xival[2] * (xistep[2]));
			}
			if (texture_window->soft_mode_on)
			{
				glPushMatrix();
				glScalef(texture_window->select_value2, texture_window->select_value2,
					texture_window->select_value2);
				glCallList(texture_window->tw_wiresphere);
				glPopMatrix();
			}
/* why dont this work propa */
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, no_mat);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, no_mat);

			glScalef(xistep[0]/4,xistep[1]/4,xistep[2]/4);
			glCallList(texture_window->tw_sphere);

#endif
#if defined (GL_API)
			translate(texture_window->ximin[0]+int_xival[0]*(xistep[0]),
				texture_window->ximin[1]+int_xival[1]*(xistep[1]),
				texture_window->ximin[2]+int_xival[2]*(xistep[2]));
			scale(xistep[0]/4,xistep[1]/4,xistep[2]/4);
			callobj(texture_window->tw_sphere);
#endif /* defined (GL_API) */
		}
#if defined (OPENGL_API)
		glPopMatrix();
		glPushMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		popmatrix();
		pushmatrix();
#endif /* defined (GL_API) */
		if (texture_window->cop_mode_on)
		{
#if defined (OPENGL_API)
			glTranslatef(texture_window->ximin[0] + int_cop[0] * xistep[0],
				texture_window->ximin[1] + int_cop[1] * xistep[1],
				texture_window->ximin[2] + int_cop[2] * xistep[2]);
			glScalef(xistep[0],xistep[1],xistep[2]);
			glColor3f(1.0,0.0,0.0);
			glCallList(texture_window->tw_cop);
#endif
#if defined (GL_API)
			translate(texture_window->ximin[0]+int_cop[0]*xistep[0],
				texture_window->ximin[1]+int_cop[1]*xistep[1],
				texture_window->ximin[2]+int_cop[2]*xistep[2]);
			scale(xistep[0],xistep[1],xistep[2]);
			cpack(0x8000ff00);
			callobj(texture_window->tw_cop);
#endif /* defined (GL_API) */
		}
#if defined (OPENGL_API)
		glPopMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		popmatrix();
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_current_cell. Invalid argument");
	}
	LEAVE;
} /* draw_current_cell */



void get_mouse_coords(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Work out mouse coords in % terms of window
==============================================================================*/
{
#if defined (OLD_CODE)
#if defined (GL_API)
	long mx,my,xlength,xmin,ylength,ymin;
#endif /* defined (GL_API) */
#endif /* defined (OLD_CODE) */

	ENTER(get_mouse_coords);
	/* checking arguments */
	if (texture_window)
	{
#if defined (OLD_CODE)
#if defined (GL_API)
		getorigin(&xmin,&ymin);
		getsize(&xlength,&ylength);
		mx=getvaluator(MOUSEX);
		my=getvaluator(MOUSEY);
		texture_window->mouse_x=texture_window->mouse_scale*((double)(mx-xmin))/
			((double)xlength);
		texture_window->mouse_y=texture_window->mouse_scale*((double)(my-ymin))/
			((double) ylength);
#endif /* defined (GL_API) */
#endif /* defined (OLD_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_mouse_coords. Invalid argument");
	}
	LEAVE;
} /* get_mouse_coords */

void draw_mouse_3d(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 25 September 1996

DESCRIPTION :
==============================================================================*/
{
	double xirange[3],xistep;
	int i;

	ENTER(draw_mouse_3d);
	/* checking arguments */
	if (texture_window)
	{
		for (i=0;i<3;i++)
		{
			xirange[i]=texture_window->ximax[i]-texture_window->ximin[i];
		}
#if defined (OPENGL_API)
		glPushMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		pushmatrix();
#endif /* defined (GL_API) */
		switch (texture_window->current_axis)
		{
			case 0:
			{
				texture_window->xival[0] += (texture_window->mouse_x-texture_window->
					mouse_x_rel)*xirange[0];
				texture_window->xival[1] += (texture_window->mouse_y-texture_window->
					mouse_y_rel)*xirange[1];
#if defined (OPENGL_API)
			glTranslatef(0,0,texture_window->xival[2]);
			glCallList(texture_window->tw_mouseplane);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
				translate(0,0,texture_window->xival[2]);
				callobj(texture_window->tw_mouseplane);
#endif /* defined (GL_API) */
			} break;
			case 1:
			{
				texture_window->xival[1] += (texture_window->mouse_y-texture_window->
					mouse_y_rel)*xirange[1];
				texture_window->xival[2] += (texture_window->mouse_x-texture_window->
					mouse_x_rel)*xirange[2];
#if defined (OPENGL_API)
				glTranslatef(texture_window->xival[0],0,0);
				glRotatef(-90,0,1.0,0);
				glCallList(texture_window->tw_mouseplane);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
				translate(texture_window->xival[0],0,0);
				rot(-90,'y');
				callobj(texture_window->tw_mouseplane);
#endif /* defined (GL_API) */
			} break;
			case 2:
			{
				texture_window->xival[0] += (texture_window->mouse_x-texture_window->
					mouse_x_rel)*xirange[0];
				texture_window->xival[2] += (texture_window->mouse_y-texture_window->
					mouse_y_rel)*xirange[2];
#if defined (OPENGL_API)
				glTranslatef(0,texture_window->xival[1],0);
				glRotatef(90,1.0,0,0);
				glCallList(texture_window->tw_mouseplane);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
				translate(0,texture_window->xival[1],0);
				rot(90,'x');
				callobj(texture_window->tw_mouseplane);
#endif /* defined (GL_API) */
			} break;
		}
		texture_window->mouse_x_rel=texture_window->mouse_x;
		texture_window->mouse_y_rel=texture_window->mouse_y;
		for (i=0;i<3;i++)
		{
			xistep=xirange[i]/texture_window->xires[i];
			if (texture_window->cell_mode)
			{
				/* max origin of cube back one in each dimension */
				if (texture_window->xival[i]>texture_window->ximax[i]-xistep)
				{
					texture_window->xival[i]=texture_window->ximax[i]-xistep;
				}
				if (texture_window->xival[i]<texture_window->ximin[i])
				{
					texture_window->xival[i] = texture_window->ximin[i];
				}
			}
			else
			{
				if (texture_window->xival[i]>texture_window->ximax[i])
				{
					texture_window->xival[i]=texture_window->ximax[i];
				}
				if (texture_window->xival[i]<texture_window->ximin[i])
				{
					texture_window->xival[i]=texture_window->ximin[i];
				}
			}
		}
#if defined (OPENGL_API)
		glPopMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		popmatrix();
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_mouse_3d.  Invalid argument");
	}
	LEAVE;
} /* draw_mouse_3d */

void makesphere(double r,double phi1,double phi2,int dis1,int dis2)
/*******************************************************************************
LAST MODIFIED : 25 September 1996

DESCRIPTION :
==============================================================================*/
{
	double a,b,ia,ib;
	GLfloat norm[4][3];
	GLfloat vert[4][3];
	int i,j,k,l;
	ENTER(makesphere);
	ia=(phi2-phi1)/dis1;
		/*???MS.  fix this */
	ib=2.0*PI/dis2;
	for (i=0,a=phi1*PI;i<dis1;i++,a+=ia)
	{
		for (j=0,b=0;j<dis2;j++,b+=ib)
		{
			vert[3][0]=r*sin(a)*cos(b);
			vert[3][1]=r*sin(a)*sin(b);
			vert[3][2]=r*cos(a);
			vert[2][0]=r*sin(a)*cos(b+ib);
			vert[2][1]=r*sin(a)*sin(b+ib);
			vert[2][2]=r*cos(a);
			vert[1][0]=r*sin(a+ia)*cos(b+ib);
			vert[1][1]=r*sin(a+ia)*sin(b+ib);
			vert[1][2]=r*cos(a+ia);
			vert[0][0]=r*sin(a+ia)*cos(b);
			vert[0][1]=r*sin(a+ia)*sin(b);
			vert[0][2]=r*cos(a+ia);
			for (k = 0;k<4;k++)
			{
				for (l=0;l<3;l++)
				{
					norm[k][l]=vert[k][l]/r;

				}
			}
#if defined (OPENGL_API)
			glBegin(GL_POLYGON);
			for (k=0;k<4;k++)
			{
				glNormal3fv(&(norm[k][0]));
				glVertex3fv(&(vert[k][0]));
			}
			glEnd();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
			bgnpolygon();
			for (k=0;k<4;k++)
			{
				normal(norm[k]);
				v3f(vert[k]);
			}
			endpolygon();
#endif /* defined (GL_API) */
		}
	}
	LEAVE;
} /* makesphere */

void select_material(struct Texture_window *texture_window,double x,double y)
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
==============================================================================*/
{
	float norm_x,norm_y;
	int index;

	ENTER(select_material);
	/* checking arguments */
	if (texture_window)
	{
		norm_x=x*4.0*ortho_right+ortho_right/8.0;
		norm_y=(y-0.25)*4.0/3.0*ortho_top+ortho_top/20.0*4.0/3.0;
		index=10*((int)(norm_x/(ortho_right*.2)-1.0))+
			(int)(norm_y/(ortho_top*.08))-1;
		if (texture_window->env_mode_on)
		{
/*???debug */
printf("******* Env Index = %d *******\n",index);
			if ((index<env_map_pick_index)&&(index>=0)&&env_map_pick_list[index])
			{
				texture_window->current_env_map=env_map_pick_list[index];
			}
/*???debug */
printf("current_environment_map = %s \n",texture_window->current_env_map->name);
		}
		else
		{
/*???debug */
printf("Material Index = %d",index);
			if ((index<pick_index)&&(index >= 0)&&material_pick_list[index])
			{
				texture_window->current_material=material_pick_list[index];
#if defined (OLD_CODE)
				XtVaSetValues(texture_window->material_sb,XmNvalue,index,NULL);
				adjust_material_sb(texture_window);
#endif /* defined (OLD_CODE) */
				/* also set current editing material */
				material_editor_dialog_set_material((Widget)NULL,
					texture_window->current_material);
			}
/*???debug */
printf("current_material = %s \n",Graphical_material_name(texture_window->
	current_material));
		}
		X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
		graphics_loop((XtPointer)texture_window);
		X3dThreeDDrawingSwapBuffers();
	}
	else
	{
		display_message(ERROR_MESSAGE,"select_material.  Invalid argument");
	}
	LEAVE;
} /* select_material */

void draw_texture_cells(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Scans through the volume texture and on finding non empty cells draws a cube of
the appropriate material.
1D arrays are used where A[i][j][k] = A'[i + j*dimi + k*dimi*dimj]
==============================================================================*/
{
	double xistep[3],ximin[3];
	int dim[3],i,j,k,nx,ny;
	struct VT_volume_texture *current_texture;
	struct VT_texture_cell *cell;
	double *grid_spacing,sx,sy,sz,tx,ty,tz;

	ENTER(draw_texture_cells);
	/* checking arguments */
	if (texture_window)
	{
		if (texture_window->current_texture->grid_spacing)
		{
			nx = texture_window->current_texture->dimension[0]+1;
			ny = texture_window->current_texture->dimension[0]+1;
#if defined (OLD_CODE)
			nz = texture_window->current_texture->dimension[0]+1;
#endif /* defined (OLD_CODE) */
			grid_spacing = texture_window->current_texture->grid_spacing;
			sx = (texture_window->ximax[0]-texture_window->ximin[0]);
			sy = (texture_window->ximax[1]-texture_window->ximin[1]);
			sz = (texture_window->ximax[2]-texture_window->ximin[2]);
			tx = texture_window->ximin[0];
			ty = texture_window->ximin[1];
			tz = texture_window->ximin[2];
		}
		else
		{
			grid_spacing = (double *) NULL;
		}
		/* printf("in draw texture cells\n"); */
		current_texture=texture_window->current_texture;
		for (i=0;i<3;i++)
		{
			dim[i]=current_texture->dimension[i];
			ximin[i]=texture_window->ximin[i];
			xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
				texture_window->xires[i];
		}
		/* printf("dim = %d %d %d\n",dim[0],dim[1],dim[2]); */
#if defined (OLD_CODE)
		default_material=default_Graphical_material;
		if (default_material == NULL)
		{
			display_message(ERROR_MESSAGE,
				"draw_texture_cells.  No default material defined");
		}
#endif /* defined (OLD_CODE) */
#if defined (OPENGL_API)
		glNewList(texture_window->tw_3dtexture,GL_COMPILE);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		delobj(texture_window->tw_3dtexture);
		makeobj(texture_window->tw_3dtexture);
#endif /* defined (GL_API) */
		for (i=0;i<dim[0];i++)
		{
			for (j=0;j<dim[1];j++)
			{
				for (k=0;k<dim[2];k++)
				{
					cell=current_texture->texture_cell_list[
						i+j*dim[0]+k*dim[0]*dim[1]];
					/* printf("[%d %d %d] %.2lf \n",i,j,k,cell->scalar_value); */
					/* check if cell exists in this position */
					if (cell)
					{
						/* if scalar_value = 0 then don't draw cell unless see paint mode*/
						if (cell->scalar_value)
						{
#if defined (OPENGL_API)
							glPushMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
							pushmatrix();
#endif /* defined (GL_API) */
							/* only draw if cell has material */
							if (cell->env_map)
							{
								execute_Graphical_material(cell->env_map->face_material[0]);
							}
							else
							{
								if (cell->material)
								{
									execute_Graphical_material(cell->material);
								}
								else
								{
#if defined (OLD_CODE)
									execute_Graphical_material(default_material);
#endif  /* defined (OLD_CODE) */
								}
							}
#if defined (OPENGL_API)
									if (grid_spacing)
									{
								glTranslatef(grid_spacing[i]*sx+tx,
								grid_spacing[nx+j]*sy+ty,
								grid_spacing[nx+ny+k]*sz+tz);
								glScalef(grid_spacing[i+1]-grid_spacing[i],
									grid_spacing[nx+j+1]-grid_spacing[nx+j],
									grid_spacing[nx+ny+k+1]-grid_spacing[nx+ny+k]);
								glScalef(sx,sy,sz);
									}
									else
									{

								glTranslatef( ximin[0] + xistep[0]*i,
								ximin[1] + xistep[1]*j,
								ximin[2] + xistep[2]*k );
								glScalef(xistep[0],xistep[1],xistep[2]);
							}
							if (cell->env_map)
							{
								glTranslatef(0.5,0.5,0.5);
								glCallList(texture_window->tw_env_map_sphere);
							}
							else
							{
								glCallList(texture_window->tw_cube);
							}
							glPopMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
							translate(ximin[0]+xistep[0]*i,ximin[1]+xistep[1]*j,
								ximin[2]+xistep[2]*k);
							scale(xistep[0],xistep[1],xistep[2]);
							if (cell->env_map)
							{
								translate(0.5,0.5,0.5);
								callobj(texture_window->tw_env_map_sphere);
							}
							else
							{
								callobj(texture_window->tw_cube);
							}
							popmatrix();
#endif /* defined (GL_API) */
						}
						else
						{
							if ((cell->material)&&(texture_window->see_paint_on))
							{
#if defined (OPENGL_API)
								glPushMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
								pushmatrix();
#endif /* defined (GL_API) */
								if (cell->env_map)
								{
									execute_Graphical_material(cell->env_map->face_material[0]);
								}
								else
								{
									if (cell->material)
									{
										execute_Graphical_material(cell->material);
									}
									else
									{
#if defined (OLD_CODE)
										execute_Graphical_material(default_material);
#endif /* defined (OLD_CODE) */
									}
								}
#if defined (OPENGL_API)
									if (grid_spacing)
									{
								glTranslatef(grid_spacing[i]*sx+tx,
								grid_spacing[nx+j]*sy+ty,
								grid_spacing[nx+ny+k]*sz+tz);
								glScalef(grid_spacing[i+1]-grid_spacing[i],
									grid_spacing[nx+j+1]-grid_spacing[nx+j],
									grid_spacing[nx+ny+k+1]-grid_spacing[nx+ny+k]);
								glScalef(sx,sy,sz);
								glScalef(0.6,0.6,0.6);

									}
									else
									{
								glTranslatef( ximin[0] + xistep[0]*i,
									ximin[1] + xistep[1]*j,
									ximin[2] + xistep[2]*k );
								glTranslatef(xistep[0]*0.2,xistep[1]*.2,xistep[2]*.2);
								glScalef(xistep[0]*0.6,xistep[1]*0.6,xistep[2]*0.6);
							}
								if (cell->env_map)
								{
									glTranslatef(0.5,0.5,0.5);
									glCallList(texture_window->tw_env_map_sphere);
								}
								else
								{
									glCallList(texture_window->tw_wire_cube);
								}
								glPopMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
								translate(ximin[0]+xistep[0]*i,ximin[1]+xistep[1]*j,
									ximin[2]+xistep[2]*k);
								translate(xistep[0]*0.2,xistep[1]*.2,xistep[2]*.2);
								scale(xistep[0]*0.6,xistep[1]*0.6,xistep[2]*0.6);
								if (cell->env_map)
								{
									translate(0.5,0.5,0.5);
									callobj(texture_window->tw_env_map_sphere);
								}
								else
								{
									callobj(texture_window->tw_cube);
								}
								popmatrix();
#endif /* defined (GL_API) */
							}
						}
					}
				}
			}
		}
#if defined (OPENGL_API)
		glEndList();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		closeobj();
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_texture_cells.  Invalid argument");
	}
/*???debug */
printf("leaving draw texture cells\n");
	LEAVE;
} /* draw_texture_cells */

void draw_texture_nodes(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Scans through the volume texture and on finding non empty cells draws a cube of
the appropriate material
1D arrays are used where A[i][j][k] = A'[i + j*dimi + k*dimi*dimj]
==============================================================================*/
{
	double ximin[3],xistep[3];
	int dim[3], i, j, k, nx, ny;
	struct VT_texture_node *node;
	struct VT_volume_texture *current_texture;
	double *grid_spacing,sx,sy,sz,tx,ty,tz;

	ENTER(draw_texture_nodes);
	/* checking arguments */
	if (texture_window)
	{
		if (texture_window->current_texture->grid_spacing)
		{
			nx = texture_window->current_texture->dimension[0] + 1;
			ny = texture_window->current_texture->dimension[0] + 1;
#if defined (OLD_CODE)
			nz = texture_window->current_texture->dimension[0] + 1;
#endif /* defined (OLD_CODE) */
			grid_spacing = texture_window->current_texture->grid_spacing;
			sx = (texture_window->ximax[0]-texture_window->ximin[0]);
			sy = (texture_window->ximax[1]-texture_window->ximin[1]);
			sz = (texture_window->ximax[2]-texture_window->ximin[2]);
			tx = texture_window->ximin[0];
			ty = texture_window->ximin[1];
			tz = texture_window->ximin[2];
		}
		else
		{
			grid_spacing = (double *) NULL;
		}
		current_texture=texture_window->current_texture;
		for (i=0;i<3;i++)
		{
			dim[i]=current_texture->dimension[i]+1;
			ximin[i]=texture_window->ximin[i];
			xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
				(texture_window->xires[i]);
		}
#if defined (OPENGL_API)
		glNewList(texture_window->tw_nodes,GL_COMPILE);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		delobj(texture_window->tw_nodes);
		makeobj(texture_window->tw_nodes);
#endif /* defined (GL_API) */
		for (i=0;i<dim[0];i++)
		{
			for (j=0;j<dim[1];j++)
			{
				for (k=0;k<dim[2];k++)
				{
					node=current_texture->global_texture_node_list[i+j*dim[0]+
						k*dim[0]*dim[1]];
					/* check if node exists in this position */
					if (node)
					{
						if ((node->material)&&(texture_window->see_paint_on))
						{
#if defined (OPENGL_API)
							glPushMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
							pushmatrix();
#endif /* defined (GL_API) */
							execute_Graphical_material(node->material);
#if defined (OPENGL_API)
									if (grid_spacing)
									{
								glTranslatef(grid_spacing[i]*sx+tx,
								grid_spacing[nx+j]*sy+ty,
								grid_spacing[nx+ny+k]*sz+tz);
							}
							else
							{
							glTranslatef( ximin[0] + xistep[0]*i,ximin[1] + xistep[1]*j,
								ximin[2] + xistep[2]*k );
							}
							glScalef(xistep[0]/5,xistep[1]/5,xistep[2]/5);
							glCallList(texture_window->tw_sphere);
							glPopMatrix();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
							translate(ximin[0]+xistep[0]*i,ximin[1]+xistep[1]*j,
								ximin[2]+xistep[2]*k );
							scale(xistep[0]/5,xistep[1]/5,xistep[2]/5);
							callobj(texture_window->tw_sphere);
							popmatrix();
#endif /* defined (GL_API) */
						}
					}
				}
			}
		}
#if defined (OPENGL_API)
		glEndList();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		closeobj();
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_texture_nodes.  Invalid argument");
	}
	LEAVE;
} /* draw_texture_nodes*/

void draw_texture_lines(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Traverses volume_textures line lists and draws the segments
==============================================================================*/
{
	double ximin[3];
	int i;
	struct VT_volume_texture *current_texture;
	struct VT_texture_curve *p;

	ENTER(draw_texture_lines);
	/* checking arguments */
	if (texture_window&&(current_texture=texture_window->current_texture))
	{
		for (i=0;i<3;i++)
		{
			ximin[i]=texture_window->ximin[i];
#if defined (OLD_CODE)
			dim[i]=current_texture->dimension[i]+1;
			xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
				(texture_window->xires[i]);
#endif /* defined (OLD_CODE) */
		}
#if defined (OPENGL_API)
		glNewList(texture_window->tw_lines,GL_COMPILE);
		/* step through linked list */
		glColor3b(0,255,0);
		glPushMatrix();
		glTranslatef(ximin[0],ximin[1],ximin[2]);
		p= *(current_texture->texture_curve_list);
		while (p)
		{
			if (1==p->type)
			{
				/* line */
				glBegin(GL_LINES);
				glVertex3f(p->point1[0],p->point1[1],p->point1[2]);
				glVertex3f(p->point2[0],p->point2[1],p->point2[2]);
				glEnd();
			}
			p=p->ptrnext;
		}
		glPopMatrix();
		glEndList();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		delobj(texture_window->tw_lines);
		makeobj(texture_window->tw_lines);
		/* step through linked list */
		RGBcolor(0,255,0);
		linewidth(3);
		pushmatrix();
		translate(ximin[0],ximin[1],ximin[2]);
		p= *(current_texture->texture_curve_list);
		while (p)
		{
			if (1==p->type)
			{
				/* line */
				move(p->point1[0],p->point1[1],p->point1[2]);
				draw(p->point2[0],p->point2[1],p->point2[2]);
			}
			p=p->ptrnext;
		}
		popmatrix();
		linewidth(1);
		closeobj();
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_texture_lines.  Invalid argument");
	}
	LEAVE;
} /* draw_texture_lines */

void draw_texture_blobs(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DECSRIPTION :
Traverses volume_textures line lists and draws the segments
==============================================================================*/
{
	double ximin[3];
	int i;
	struct VT_texture_curve *p;
	struct VT_volume_texture *current_texture;

	ENTER(draw_texture_blobs);
	/* checking arguments */
	if (texture_window&&(current_texture=texture_window->current_texture))
	{
		for (i=0;i<3;i++)
		{
			ximin[i]=texture_window->ximin[i];
#if defined (OLD_CODE)
			dim[i]=current_texture->dimension[i]+1;
			xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
				(texture_window->xires[i]);
#endif /* defined (OLD_CODE) */
		}
#if defined (OPENGL_API)
		glNewList(texture_window->tw_blobs,GL_COMPILE);
		/* step through linked list */
		glColor3b(255,0,255);
		glPushMatrix();
		glTranslatef(ximin[0],ximin[1],ximin[2]);
		p= *(current_texture->texture_curve_list);
		while (p)
		{
			if (0==p->type)
			{
				glBegin(GL_LINES);
				glVertex3f(p->point1[0],p->point1[1],p->point1[2]);
				glVertex3f(p->point2[0],p->point2[1],p->point2[2]);
				glEnd();
			}
			p=p->ptrnext;
		}
		glPopMatrix();
		glEndList();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		delobj(texture_window->tw_blobs);
		makeobj(texture_window->tw_blobs);
		/* step through linked list */
		RGBcolor(255,0,255);
		linewidth(3);
		pushmatrix();
		translate(ximin[0],ximin[1],ximin[2]);
		p= *(current_texture->texture_curve_list);
		while (p)
		{
			if (0==p->type)
			{
				/* line */
					/*???DB.  1 for draw_texture_lines */
				move(p->point1[0],p->point1[1],p->point1[2]);
				draw(p->point2[0],p->point2[1],p->point2[2]);
			}
			p=p->ptrnext;
		}
		popmatrix();
		linewidth(1);
		closeobj();
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_texture_blobs. Invalid argument");
	}
	LEAVE;
} /* draw_texture_blobs */

void draw_texture_softs(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DECSRIPTION :
Traverses volume_textures line lists and draws the segments
==============================================================================*/
{
	double ximin[3];
	int i;
	struct VT_texture_curve *p;
	struct VT_volume_texture *current_texture;

	ENTER(draw_texture_softs);
	/* checking arguments */
	if (texture_window&&(current_texture=texture_window->current_texture))
	{
		for (i=0;i<3;i++)
		{
			ximin[i]=texture_window->ximin[i];
#if defined (OLD_CODE)
			dim[i]=current_texture->dimension[i]+1;
			xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
				(texture_window->xires[i]);
#endif /* defined (OLD_CODE) */
		}
#if defined (OPENGL_API)
		glNewList(texture_window->tw_softs,GL_COMPILE);
		/* step through linked list */
		glColor3b(255,0,255);
		glPushMatrix();
		glTranslatef(ximin[0],ximin[1],ximin[2]);
		p= *(current_texture->texture_curve_list);
		while (p)
		{
			if (3==p->type)
			{
				glBegin(GL_POINTS);
				glVertex3f(p->point1[0],p->point1[1],p->point1[2]);
				glEnd();
				glPushMatrix();
				glTranslatef(p->point1[0],p->point1[1],p->point1[2]);
				glScalef(p->scalar_value[1],p->scalar_value[1], p->scalar_value[1]);
				glCallList(texture_window->tw_wiresphere);
				glPopMatrix();
			}
			p=p->ptrnext;
		}
		glPopMatrix();
		glEndList();
#endif /* defined (OPENGL_API) */

	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_texture_blobs. Invalid argument");
	}
	LEAVE;
} /* draw_texture_softs */

void draw_texture_curves(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Traverses volume_textures curve lists and draws the segments
==============================================================================*/
{
	double a,b,c,d,t,ximin[3];
	int i;
	struct VT_texture_curve *cp;
	struct VT_volume_texture *current_texture;

	ENTER(draw_texture_curves);
	/* checking arguments */
	if (texture_window&&(current_texture=texture_window->current_texture))
	{
		if ((0!=texture_window->edit_curve.index)&&(texture_window->curve_mode_on))
		{
			/* update as points may be being moved */
			select_curve(texture_window,0);
		}
		for (i=0;i<3;i++)
		{
			ximin[i]=texture_window->ximin[i];
#if defined (OLD_CODE)
			dim[i]=current_texture->dimension[i]+1;
			xistep[i]=(texture_window->ximax[i]-texture_window->ximin[i])/
				(texture_window->xires[i]);
#endif /* defined (OLD_CODE) */
		}
#if defined (OPENGL_API)
		glNewList(texture_window->tw_curves,GL_COMPILE);
		/* step through linked list */
		glPushMatrix();
		glTranslatef(ximin[0],ximin[1],ximin[2]);
		if (texture_window->edit_curve.index >= 2&&
			texture_window->edit_curve.type == 2)
		{
			cp = &(texture_window->edit_curve);
			glBegin(GL_LINE_STRIP);
			glVertex3f(cp->point1[0],cp->point1[1],cp->point1[2]);
			/*linewidth(3);*/
			glColor3b(0,255,0);
			for (t=0;t<=1.0;t += 0.1)
			{
				/* p1,p2, slope @1,slope @2 */
				a=(1-t)*(1-t)*(1-t);
				b=t*t*t;
				c=3.0*t*(1-t)*(1-t);
				d=3.0*t*t*(1-t);
				glVertex3f(
					a*cp->point1[0]+b*cp->point2[0]+c*cp->point3[0]+d*cp->point4[0],
					a*cp->point1[1]+b*cp->point2[1]+c*cp->point3[1]+d*cp->point4[1],
					a*cp->point1[2]+b*cp->point2[2]+c*cp->point3[2]+d*cp->point4[2]);
			}
			glEnd();
			glColor3b(255,0,0);
			glBegin(GL_LINES);
			/* draw lines to bezier points */
			glVertex3f(cp->point1[0],cp->point1[1],cp->point1[2]);
			glVertex3f(cp->point3[0],cp->point3[1],cp->point3[2]);
			glVertex3f(cp->point2[0],cp->point2[1],cp->point2[2]);
			glVertex3f(cp->point4[0],cp->point4[1],cp->point4[2]);
			glEnd();
		}
		cp= *(current_texture->texture_curve_list);
		while (cp)
		{
			if (2==cp->type)
			{
				glBegin(GL_LINE_STRIP);
				glVertex3f(cp->point1[0],cp->point1[1],cp->point1[2]);
				/*linewidth(3);*/
				glColor3f(0,255,0);
				for (t=0;t<=1.0;t += 0.1)
				{
					/* p1,p2, slope @1,slope @2 */
					a=(1-t)*(1-t)*(1-t);
					b=t*t*t;
					c=3.0*t*(1-t)*(1-t);
					d=3.0*t*t*(1-t);
					glVertex3f(
						a*cp->point1[0]+b*cp->point2[0]+c*cp->point3[0]+d*cp->point4[0],
						a*cp->point1[1]+b*cp->point2[1]+c*cp->point3[1]+d*cp->point4[1],
						a*cp->point1[2]+b*cp->point2[2]+c*cp->point3[2]+d*cp->point4[2]);
				}
				glEnd();
				glColor3b(255,255,0);
				/*linewidth(1);*/
				glBegin(GL_LINES);
				/* draw lines to bezier points */
				glVertex3f(cp->point1[0],cp->point1[1],cp->point1[2]);
				glVertex3f(cp->point3[0],cp->point3[1],cp->point3[2]);
				glVertex3f(cp->point2[0],cp->point2[1],cp->point2[2]);
				glVertex3f(cp->point4[0],cp->point4[1],cp->point4[2]);
				glEnd();
			}
			cp=cp->ptrnext;
		}
		glPopMatrix();
		glEndList();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		delobj(texture_window->tw_curves);
		makeobj(texture_window->tw_curves);
		/* step through linked list */
		pushmatrix();
		translate(ximin[0],ximin[1],ximin[2]);
		if (texture_window->edit_curve.index >= 2&&
			texture_window->edit_curve.type == 2)
		{
			cp=&(texture_window->edit_curve);
			move(cp->point1[0],cp->point1[1],cp->point1[2]);
			linewidth(3);
			RGBcolor(0,255,0);
			for (t=0;t<=1.0;t += 0.1)
			{
				/* p1,p2, slope @1,slope @2 */
				a=(1-t)*(1-t)*(1-t);
				b=t*t*t;
				c=3.0*t*(1-t)*(1-t);
				d=3.0*t*t*(1-t);
				draw(a*cp->point1[0]+b*cp->point2[0]+c*cp->point3[0]+d*cp->point4[0],
					a*cp->point1[1]+b*cp->point2[1]+c*cp->point3[1]+d*cp->point4[1],
					a*cp->point1[2]+b*cp->point2[2]+c*cp->point3[2]+d*cp->point4[2]);
			}
			RGBcolor(255,0,0);
			linewidth(1);
			/* draw lines to bezier points */
			move(cp->point1[0],cp->point1[1],cp->point1[2]);
			draw(cp->point3[0],cp->point3[1],cp->point3[2]);
			move(cp->point2[0],cp->point2[1],cp->point2[2]);
			draw(cp->point4[0],cp->point4[1],cp->point4[2]);
		}
		cp= *(current_texture->texture_curve_list);
		while (cp)
		{
			if (2==cp->type)
			{
				move(cp->point1[0],cp->point1[1],cp->point1[2]);
				linewidth(3);
				RGBcolor(0,255,0);
				for (t=0;t<=1.0;t += 0.1)
				{
					/* p1,p2, slope @1,slope @2 */
					a=(1-t)*(1-t)*(1-t);
					b=t*t*t;
					c=3.0*t*(1-t)*(1-t);
					d=3.0*t*t*(1-t);
					draw(a*cp->point1[0]+b*cp->point2[0]+c*cp->point3[0]+d*cp->point4[0],
					a*cp->point1[1]+b*cp->point2[1]+c*cp->point3[1]+d*cp->point4[1],
					a*cp->point1[2]+b*cp->point2[2]+c*cp->point3[2]+d*cp->point4[2]);
				}
				RGBcolor(255,255,0);
				linewidth(1);
				/* draw lines to bezier points */
				move(cp->point1[0],cp->point1[1],cp->point1[2]);
				draw(cp->point3[0],cp->point3[1],cp->point3[2]);
				move(cp->point2[0],cp->point2[1],cp->point2[2]);
				draw(cp->point4[0],cp->point4[1],cp->point4[2]);
			}
			cp=cp->ptrnext;
		}
		popmatrix();
		linewidth(1);
		closeobj();
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_texture_curves.  Invalid argument");
	}
	LEAVE;
} /* draw_texture_curves */

int detail_cell(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
Sets current cell detail value (defualt = 0) from tw select_value
==============================================================================*/
{
	double dim[3];
	int i,index[3],j,k,return_code;
	struct MC_iso_surface *mc_iso_surface;

	ENTER(detail_cell);
	/* checking arguments */
	if (texture_window)
	{
		return_code=1;
		mc_iso_surface=texture_window->current_texture->mc_iso_surface;
		/* turn cell mode on */
		texture_window->cell_mode=1;
		for (i=0;i<3;i++)
		{
			index[i]=(int)(texture_window->xires[i]*(texture_window->xival[i]-
				texture_window->ximin[i])/(texture_window->ximax[i]-
				texture_window->ximin[i]));
			dim[i]=texture_window->current_texture->dimension[i];
		}
		if (texture_window->cell_mode)
		{
			mc_iso_surface->active_block[0]=index[0]-1;
			mc_iso_surface->active_block[2]=index[1]-1;
			mc_iso_surface->active_block[4]=index[2]-1;
			mc_iso_surface->active_block[1]=index[0]+
				(int)texture_window->brushsize[0]+1;
			mc_iso_surface->active_block[3]=index[1]+
				(int)texture_window->brushsize[1]+1;
			mc_iso_surface->active_block[5]=index[2]+
				(int)texture_window->brushsize[2]+1;
			for (i=0;i<(int)texture_window->brushsize[0];i++)
			{
				for (j=0;j<(int)texture_window->brushsize[1];j++)
				{
					for (k=0;k<(int)texture_window->brushsize[2];k++)
					{
						if ((index[0]+i<dim[0])&&(index[1]+j<dim[1])&&(index[2]+k<dim[2]))
						{
							(texture_window->current_texture->texture_cell_list[
								(int)((index[0]+i)+dim[0]*(index[1]+j)+
								dim[0]*dim[1]*(index[2]+k))])->detail=
								(int)texture_window->select_value;
/*???debug */
printf("Set cell[%d %d %d] detail to %d\n",(index[0]+i)+1,(index[1]+j)+1,
	(index[2]+k)+1,(int)(texture_window->current_texture->texture_cell_list[
	(int)((index[0]+i)+dim[0]*(index[1]+j)+dim[0]*dim[1]*(index[2]+k))])->detail);
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"detail_cell.  Missing texture_window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* detail_cell */

int fill_cell(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
Sets current cell scalar value to be solid
==============================================================================*/
{
	double dim[3];
	int i,index[3],j,k,return_code;
	struct MC_iso_surface *mc_iso_surface;

	ENTER(fill_cell);
	/* checking arguments */
	if (texture_window)
	{
		return_code=1;
		mc_iso_surface = texture_window->current_texture->mc_iso_surface;
		/* turn cell mode on */
		texture_window->cell_mode=1;
		for (i=0;i<3;i++)
		{
			index[i]=(int)(texture_window->xires[i]*(texture_window->xival[i]-
				texture_window->ximin[i])/(texture_window->ximax[i]-
				texture_window->ximin[i]));
			dim[i]=texture_window->current_texture->dimension[i];
		}
		if (texture_window->cell_mode)
		{
			mc_iso_surface->active_block[0]=index[0]-1;
			mc_iso_surface->active_block[2]=index[1]-1;
			mc_iso_surface->active_block[4]=index[2]-1;
			mc_iso_surface->active_block[1]=index[0]+
				(int)texture_window->brushsize[0]+1;
			mc_iso_surface->active_block[3]=index[1]+
				(int)texture_window->brushsize[1]+1;
			mc_iso_surface->active_block[5]=index[2]+
				(int)texture_window->brushsize[2]+1;
			for (i=0;i<(int)texture_window->brushsize[0];i++)
			{
				for (j=0;j<(int)texture_window->brushsize[1];j++)
				{
					for (k=0;k<(int)texture_window->brushsize[2];k++)
					{
						if ((index[0]+i<dim[0])&&(index[1]+j<dim[1])&&(index[2]+k<dim[2]))
						{
							(texture_window->current_texture->texture_cell_list[
								(int)((index[0]+i)+dim[0]*(index[1]+j)+
							dim[0]*dim[1]*(index[2]+k))])->scalar_value=
								texture_window->select_value;
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"fill_cell.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* fill_cell */

int fill_node(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
Sets absolute node scalar value
==============================================================================*/
{
	double dim[3];
	int i,index[3],j,k,return_code;
	struct MC_iso_surface *mc_iso_surface;

	ENTER(fill_node);
	/* checking arguments */
	if (texture_window)
	{
		return_code=1;
		mc_iso_surface=texture_window->current_texture->mc_iso_surface;
		for (i=0;i<3;i++)
		{
			index[i]=(int)(texture_window->xires[i]*(texture_window->xival[i]-
				texture_window->ximin[i])/(texture_window->ximax[i]-
				texture_window->ximin[i]));
			dim[i]=texture_window->current_texture->dimension[i]+1;
		}
		mc_iso_surface->active_block[0]=index[0]-1;
		mc_iso_surface->active_block[2]=index[1]-1;
		mc_iso_surface->active_block[4]=index[2]-1;
		mc_iso_surface->active_block[1]=index[0]+
			(int)texture_window->brushsize[0]+1;
		mc_iso_surface->active_block[3]=index[1]+
			(int)texture_window->brushsize[1]+1;
		mc_iso_surface->active_block[5]=index[2]+
			(int)texture_window->brushsize[2]+1;
		for (i=0;i<5;i++)
		{
			if (mc_iso_surface->active_block[i] < 0)
			{
/*???debug */
printf("mc_ab[%d] = 0\n",i);
				mc_iso_surface->active_block[i]=0;
			}
			if (((1==i)||(3==i)||(5==i))&&(mc_iso_surface->active_block[i]>=
				texture_window->current_texture->dimension[(i+1)/2-1]))
			{
/*???debug */
printf("mc_ab[%d] = %d\n",i,
	texture_window->current_texture->dimension[(i+1)/2-1]-1);
				mc_iso_surface->active_block[i]=
					texture_window->current_texture->dimension[(i+1)/2-1]-1;
			}
		}
/*???debug */
printf("Index = %d %d %d\n",index[0],index[1],index[2]);
		for (i=0;i<(int)texture_window->brushsize[0];i++)
		{
			for (j=0;j<(int)texture_window->brushsize[1];j++)
			{
				for (k=0;k<(int)texture_window->brushsize[2];k++)
				{
					if ((index[0]+i<dim[0])&&(index[1]+j<dim[1])&&(index[2]+k<dim[2]))
					{
						(texture_window->current_texture->global_texture_node_list[
							(int)((index[0]+i)+dim[0]*(index[1]+j)+
							dim[0]*dim[1]*(index[2]+k))])->scalar_value=
							texture_window->select_value;
						(texture_window->current_texture->global_texture_node_list[
							(int)((index[0]+i)+dim[0]*(index[1]+j)+
							dim[0]*dim[1]*(index[2]+k))])->active=1;
/*???debug */
printf("---- Node %d %d %d activated = %lf ----\n", i, j, k,
	(texture_window->current_texture->global_texture_node_list[
	(int)((index[0]+i)+dim[0]*(index[1]+j)+
	dim[0]*dim[1]*(index[2]+k))])->scalar_value);
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"fill_node.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* fill_node */

int value_node(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
prints node scalar value
==============================================================================*/
{
	double dim[3];
	int i, index[3], return_code;

	ENTER(value_node);
	/* checking arguments */
	if (texture_window)
	{
		return_code=1;
		for (i=0;i<3;i++)
		{
			index[i]=(int)(texture_window->xires[i]*(texture_window->xival[i]-
				texture_window->ximin[i])/(texture_window->ximax[i]-
				texture_window->ximin[i]));
			dim[i]=texture_window->current_texture->dimension[i]+1;
		}
/*???debug */
printf("Node[%d]  scalar value [%d,%d,%d] (xi = %lf, %lf, %lf) = %lf\n",
	(int)((index[0])+dim[0]*(index[1])+dim[0]*dim[1]*(index[2])),index[0],
	index[1],index[2],texture_window->xival[0],texture_window->xival[1],
	texture_window->xival[2],
	(texture_window->current_texture->global_texture_node_list[
	(int)((index[0])+dim[0]*(index[1])+dim[0]*dim[1]*(index[2]))])->scalar_value);
if (texture_window->current_texture->scalar_field)
{
	printf("Isosurface scalar value = %lf\n",
		texture_window->current_texture->scalar_field->scalar[
		(int)(index[0] + index[1]*dim[0] + index[2]*dim[0]*dim[1])]);
}
	}
	else
	{
		display_message(ERROR_MESSAGE,"value_node.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* value_node */

int delete_active_node(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
deletes absolute node scalar value
==============================================================================*/
{
	double dim[3];
	int i,index[3],j,k,return_code;
	struct MC_iso_surface *mc_iso_surface;

	ENTER(fill_cell);
	/* checking arguments */
	if (texture_window)
	{
		return_code=1;
		mc_iso_surface=texture_window->current_texture->mc_iso_surface;
		for (i=0;i<3;i++)
		{
			index[i]=(int)(texture_window->xires[i]*(texture_window->xival[i]-
				texture_window->ximin[i])/(texture_window->ximax[i]-
				texture_window->ximin[i]));
			dim[i]=texture_window->current_texture->dimension[i]+1;
		}
		mc_iso_surface->active_block[0]=index[0]-1;
		mc_iso_surface->active_block[2]=index[1]-1;
		mc_iso_surface->active_block[4]=index[2]-1;
		mc_iso_surface->active_block[1]=index[0]+
			(int)texture_window->brushsize[0]+1;
		mc_iso_surface->active_block[3]=index[1]+
			(int)texture_window->brushsize[1]+1;
		mc_iso_surface->active_block[5]=index[2]+
			(int)texture_window->brushsize[2]+1;

		for (i=0;i<(int)texture_window->brushsize[0];i++)
		{
			for (j=0;j<(int)texture_window->brushsize[1];j++)
			{
				for (k=0;k<(int)texture_window->brushsize[2];k++)
				{
					if ((index[0]+i<dim[0])&&(index[1]+j<dim[1])&&(index[2]+k<dim[2]))
					{
						(texture_window->current_texture->global_texture_node_list[
							(int)((index[0]+i)+dim[0]*(index[1]+j)+
							dim[0]*dim[1]*(index[2]+k))])->scalar_value=0;
						(texture_window->current_texture->global_texture_node_list[
							(int)((index[0]+i)+dim[0]*(index[1]+j)+
							dim[0]*dim[1]*(index[2]+k))])->active=0;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"delete_active_node.  Invalid argument");
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* delete_active_node */

void paint_cell(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 20 June 1996

DESCRIPTION :
Sets current cell material to current texture_window material
==============================================================================*/
{
	int dim[3],i,ii,index[3],j,jj,kk;
	struct MC_iso_surface *mc_iso_surface;

	ENTER(paint_cell);
	mc_iso_surface = texture_window->current_texture->mc_iso_surface;
	/* checking arguments */
	if (texture_window)
	{
		for (i=0;i<3;i++)
		{
			index[i]=(int)(texture_window->xires[i]*(texture_window->xival[i]-
				texture_window->ximin[i])/(texture_window->ximax[i]-
				texture_window->ximin[i]));
			dim[i]=texture_window->current_texture->dimension[i];
		}

		if (texture_window->cell_mode)
		{
		mc_iso_surface->active_block[0]=index[0]-1;
		mc_iso_surface->active_block[2]=index[1]-1;
		mc_iso_surface->active_block[4]=index[2]-1;
		mc_iso_surface->active_block[1]=index[0]+(int)texture_window->brushsize[0]+1;
		mc_iso_surface->active_block[3]=index[1]+(int)texture_window->brushsize[1]+1;
		mc_iso_surface->active_block[5]=index[2]+(int)texture_window->brushsize[2]+1;
			for (ii=0;ii<(int)texture_window->brushsize[0];ii++)
			{
				for (jj=0;jj<(int)texture_window->brushsize[1];jj++)
				{
					for (kk=0;kk<(int)texture_window->brushsize[2];kk++)
					{
						if ((index[0]+ii<dim[0])&&(index[1]+jj<dim[1])&&
							(index[2]+kk<dim[2]))
						{
							if (texture_window->env_mode_on&&texture_window->current_env_map)
							{
								(texture_window->current_texture->texture_cell_list[(int)
									(index[0]+ii+dim[0]*(index[1]+jj)+dim[0]*dim[1]*(index[2]+
									kk))])->env_map=texture_window->current_env_map;
								for (j=0;j<3;j++)
								{
									(texture_window->current_texture->texture_cell_list[(int)
										(index[0]+ii+dim[0]*(index[1]+jj)+dim[0]*dim[1]*(index[2]+
										kk))])->cop[j]=texture_window->cop[j];
								}
/*???debug */
printf("Painted cell [%d,%d,%d] with Environment_map %s, COP=[%.2lf, %.2lf, %.2lf]\n",
	index[0]+ii,index[1]+jj,index[2]+kk,texture_window->current_env_map->name,
	texture_window->cop[0],texture_window->cop[1],texture_window->cop[2]);
							}
							else
							{
/*???debug */
printf("texture_cell_list index = %d\n",(index[0]+ii+
	dim[0]*(index[1]+jj)+dim[0]*dim[1]*(index[2]+kk)));
								/* edit material and cop separately */
								if (!texture_window->cop_mode_on)
								{
									(texture_window->current_texture->texture_cell_list[(int)
										(index[0]+ii+dim[0]*(index[1]+jj)+dim[0]*dim[1]*(index[2]+
										kk))])->material=texture_window->current_material;
								}
								for (j=0;j<3;j++)
								{
									(texture_window->current_texture->texture_cell_list[(int)
										(index[0]+ii+dim[0]*(index[1]+jj)+dim[0]*dim[1]*(index[2]+
										kk))])->cop[j]=texture_window->cop[j];
								}
/*???debug */
printf(
	"Painted cell [%d,%d,%d] with material %s, %s COP = [%.2lf, %.2lf, %.2lf]\n",
	index[0]+ii,index[1]+jj,index[2]+kk,Graphical_material_name((texture_window->
	current_texture->texture_cell_list[(int)
	(index[0]+ii+dim[0]*(index[1]+jj)+dim[0]*dim[1]*(index[2]+
	kk))])->material),Graphical_material_name(texture_window->current_material),
	texture_window->cop[0],texture_window->cop[1],texture_window->cop[2]);
							}
						}
						else
						{
							/* set node material */
							(texture_window->current_texture->global_texture_node_list[
								(int)(index[0]+(dim[0]+1)*index[1]+(dim[0]+1)*(dim[1]+1)*
								index[2])])->material=texture_window->current_material;
							for (j=0;j<3;j++)
							{
								(texture_window->current_texture->global_texture_node_list[
									(int)(index[0]+(dim[0]+1)*index[1]+(dim[0]+1)*(dim[1]+1)*
									index[2])])->cop[j]=texture_window->cop[j];
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"paint_cell.  Invalid argument");
	}
	LEAVE;
} /* paint_cell */

void delete_cell(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 21 December 1995

DESCRIPTION :
==============================================================================*/
{
	int dim[3],i,index[3],j,k;
	struct MC_iso_surface *mc_iso_surface;
	ENTER(delete_cell);
	mc_iso_surface = texture_window->current_texture->mc_iso_surface;
	/* checking arguments */
	if (texture_window)
	{
/*???debug */
printf("In delete_cell\n");
		for (i=0;i<3;i++)
		{
			index[i]=(int)(texture_window->xires[i]*(texture_window->xival[i]-
				texture_window->ximin[i])/(texture_window->ximax[i]-texture_window->
				ximin[i]));
			dim[i]=texture_window->current_texture->dimension[i];
		}
/*???debug */
printf("Deleting cell <%d, %d, %d>\n",index[0],index[1],index[2]);
		if (texture_window->cell_mode)
		{
		mc_iso_surface->active_block[0]=index[0]-1;
		mc_iso_surface->active_block[2]=index[1]-1;
		mc_iso_surface->active_block[4]=index[2]-1;
		mc_iso_surface->active_block[1]=index[0]+(int)texture_window->brushsize[0]+1;
		mc_iso_surface->active_block[3]=index[1]+(int)texture_window->brushsize[1]+1;
		mc_iso_surface->active_block[5]=index[2]+(int)texture_window->brushsize[2]+1;
			for (i=0;i<(int)texture_window->brushsize[0];i++)
			{
				for (j=0;j<(int)texture_window->brushsize[1];j++)
				{
					for (k=0;k<(int)texture_window->brushsize[2];k++)
					{
						if ((index[0]+i<dim[0])&&(index[1]+j<dim[1])&&(index[2]+k<dim[2]))
						{
							(texture_window->current_texture->texture_cell_list[(int)
								(index[0]+i+dim[0]*(index[1]+j)+dim[0]*dim[1]*(index[2]+k))])
								->scalar_value=0;
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"delete_cell.  Invalid argument");
	}
	LEAVE;
} /* delete_cell */

void delete_paint(struct Texture_window *texture_window)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Deletes current cell material and env map
==============================================================================*/
{
	int dim[3],i,index[3],j,k;
	struct MC_iso_surface *mc_iso_surface;
	ENTER(delete_paint);
	mc_iso_surface = texture_window->current_texture->mc_iso_surface;
	/* checking arguments */
	if (texture_window)
	{
/*???debug */
printf("In delete_paint\n");
		for (i=0;i<3;i++)
		{
			index[i]=(int)(texture_window->xires[i]*(texture_window->xival[i]-
				texture_window->ximin[i])/(texture_window->ximax[i]-texture_window->
				ximin[i]));
			dim[i]=texture_window->current_texture->dimension[i];
		}
/*???debug */
printf("Deleting cell <%d, %d, %d>\n",index[0],index[1],index[2]);
		if (texture_window->cell_mode)
		{
		mc_iso_surface->active_block[0]=index[0]-1;
		mc_iso_surface->active_block[2]=index[1]-1;
		mc_iso_surface->active_block[4]=index[2]-1;
		mc_iso_surface->active_block[1]=index[0]+(int)texture_window->brushsize[0]+1;
		mc_iso_surface->active_block[3]=index[1]+(int)texture_window->brushsize[1]+1;
		mc_iso_surface->active_block[5]=index[2]+(int)texture_window->brushsize[2]+1;
			for (i=0;i<(int)texture_window->brushsize[0];i++)
			{
				for (j=0;j<(int)texture_window->brushsize[1];j++)
				{
					for (k=0;k<(int)texture_window->brushsize[2];k++)
					{
						if ((index[0]+i<dim[0])&&(index[1]+j<dim[1])&&(index[2]+k<dim[2]))
						{
							(texture_window->current_texture->texture_cell_list[(int)(index[0]
								+i+dim[0]*(index[1]+j)+dim[0]*dim[1]*(index[2]+k))])->material=
								(struct Graphical_material *)NULL;
							(texture_window->current_texture->texture_cell_list[(int)(index[0]
								+i+dim[0]*(index[1]+j)+dim[0]*dim[1]*(index[2]+k))])->env_map=
								(struct Environment_map *)NULL;
						}
					}
				}
			}
		}
		else
		{
			(texture_window->current_texture->global_texture_node_list[(int)(index[0]
				+(dim[0]+1)*index[1]+(dim[0]+1)*(dim[1]+1)*index[2])])->material=
				(struct Graphical_material *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"delete_paint.  Invalid argument");
	}
	LEAVE;
} /* delete_paint */

void texture_graphics_initialize_callback(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1996

DESCRIPTION :
==============================================================================*/
{
	int i;
	struct Texture_window *texture_window;

	ENTER(texture_graphics_initialize_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (texture_window=(struct Texture_window *)user_data)
	{
/*???debug */
printf("In texture_graphics_initialize_callback %p\n",texture_window);
#if defined (OPENGL_API)
		X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		texture_window->tw_axes=glGenLists(1);
		texture_window->tw_grid=glGenLists(1);
		texture_window->tw_cube=glGenLists(1);
		texture_window->tw_wire_cube=glGenLists(1);
		texture_window->tw_mouseplane=glGenLists(1);
		texture_window->tw_sphere=glGenLists(1);
		texture_window->tw_wiresphere=glGenLists(1);
		texture_window->tw_small_sphere=glGenLists(1);
		texture_window->tw_env_map_sphere=glGenLists(1);
		texture_window->tw_box=glGenLists(1);
		texture_window->tw_3dtexture=glGenLists(1);
		texture_window->tw_isosurface=glGenLists(1);
		texture_window->tw_nodes=glGenLists(1);
		texture_window->tw_lines=glGenLists(1);
		texture_window->tw_curves=glGenLists(1);
		texture_window->tw_blobs=glGenLists(1);
		texture_window->tw_softs=glGenLists(1);
		texture_window->tw_cop=glGenLists(1);
		texture_window->tw_envsquare=glGenLists(1);
		for (i=0;i<6;i++)
		{
			texture_window->tw_cube_tex[i]=glGenLists(1);
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
#if defined (OLD_CODE)
		prefsize(500,500);
		texture_window->graphics_window=winopen("3D Texture");
		winconstraints();
		RGBmode();
		doublebuffer();
		lsetdepth(0, 0x7FFFFF);
		gconfig();
		qdevice(LEFTMOUSE);
		qdevice(RIGHTMOUSE);
		qdevice(MIDDLEMOUSE);
#endif /* defined (OLD_CODE) */
		blendfunction(BF_SA,BF_MSA);
		zbuffer(TRUE);
		texture_window->tw_axes=genobj();
		texture_window->tw_grid=genobj();
		texture_window->tw_cube=genobj();
		texture_window->tw_wire_cube=genobj();
		texture_window->tw_mouseplane=genobj();
		texture_window->tw_sphere=genobj();
		texture_window->tw_small_sphere=genobj();
		texture_window->tw_env_map_sphere=genobj();
		texture_window->tw_box=genobj();
		texture_window->tw_3dtexture=genobj();
		texture_window->tw_isosurface=genobj();
		texture_window->tw_nodes=genobj();
		texture_window->tw_lines=genobj();
		texture_window->tw_curves=genobj();
		texture_window->tw_blobs=genobj();
		texture_window->tw_cop=genobj();
		texture_window->tw_envsquare=genobj();
		for (i=0;i<6;i++)
		{
			texture_window->tw_cube_tex[i]=genobj();
		}
#endif /* defined (GL_API) */
		make_objects(texture_window);
		update_grid(texture_window);
#if defined (OPENGL_API)
		/* init model_matrix */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glGetFloatv(GL_MODELVIEW_MATRIX,texture_window->model_matrix);
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
		/* init model_matrix */
		mmode(MVIEWING);
		loadmatrix(idmat);
		getmatrix(texture_window->model_matrix);
#endif /* defined (GL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"texture_graphics_initialize_callback.  Invalid argument");
	}
/*???debug */
printf("leaving init_cb\n");
	LEAVE;

} /* texture_graphics_initialize_callback */

void texture_graphics_expose_callback(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
==============================================================================*/
{
	struct Texture_window *texture_window;

	ENTER(texture_graphics_expose_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	texture_window=(struct Texture_window *)user_data;
/*???debug */
printf("In texture_graphics_expose_callback\n");
	graphics_loop((XtPointer)texture_window);
	LEAVE;
} /* texture_graphics_expose_callback */

#if defined (OLD_CODE)
void texture_graphics_input_callback(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1996

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

	ENTER(texture_graphics_input_callback);
	if (texture_window=(struct Texture_window *)user_data)
	{
/*???debug */
printf("In texture_graphics_input_callback\n");
		if (callback=(X3dThreeDDrawCallbackStruct *)call_data)
		{
			if (X3dCR_INPUT==callback->reason)
			{
				/* find window coords */
				XGetGeometry(texture_window->user_interface->display,callback->window,
					&win,&x,&y,&width,&height,&border_width,&depth);
/*???debug */
printf("window size = %d, %d\n",width,height);
				if ((callback->event)&&((ButtonPress==callback->event->type)||
					(ButtonRelease==callback->event->type)))
				{
					event=&(callback->event->xbutton);
					if (ButtonPress==callback->event->type)
					{
/*???debug */
printf("button press at %d %d",event->x,event->y);
						norm_x=(double)event->x/(double)width;
						norm_y=1.0-(double)event->y/(double)height;
						select_material(texture_window,norm_x,norm_y);
					}
/*???debug */
else
{
	printf("button release at %d %d",event->x,event->y);
}
printf("normalized mouse coords = %lf,%lf",(double)event->x/(double)width,
	1.0-(double)event->y/(double)height);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"texture_graphics_input_callback.  Invalid X event");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"texture_graphics_input_callback.  Invalid reason");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"texture_graphics_input_callback.  Missing call_data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"texture_graphics_input_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* texture_graphics_input_callback */
#endif

void volume_window_motion(struct Texture_window  *texture_window, double x, double y, int button)
/*******************************************************************************
LAST MODIFIED : 18 Feb 1997

DESCRIPTION :
moves cursor or model from mouse motion
==============================================================================*/
{
#if defined(OPENGL_API)
	X3dThreeDDrawingMakeCurrent(texture_window->graphics_window);
	glMatrixMode(GL_MODELVIEW);

	switch (button) {
		case 1: /* tumble */
			glLoadIdentity();
			glRotated(100*x,0,1,0);
			glRotated(-100*y,1,0,0);
			glMultMatrixf(texture_window->model_matrix);
			glGetFloatv(GL_MODELVIEW_MATRIX,texture_window->model_matrix);
			break;
		case 2:	/* translate */
			glLoadIdentity();
			glTranslated(10*x,0,0);
			glTranslated(0,10*y,0);
			glMultMatrixf(texture_window->model_matrix);
			glGetFloatv(GL_MODELVIEW_MATRIX,texture_window->model_matrix);
			break;
		case 3:	/* dolly */
				{
					texture_window->fovy *= (1.0-0.5*(x+y));
				}
			break;
		default:
			break;
		}


	graphics_loop((XtPointer)texture_window);
	X3dThreeDDrawingSwapBuffers();

#endif

}

void texture_graphics_input_callback(Widget drawing_widget,
	XtPointer user_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
The callback for mouse or keyboard input in the drawing area.  Invokes the
function in the <drawing_structure> with the <drawing_widget>, the data from the
<drawing_structure> and the input event as arguments.
==============================================================================*/
{
	static int current_button = 0;
	static double cursor_x,cursor_y,motion_x,motion_y,old_motion_x,old_motion_y;
	struct Texture_window  *texture_window;
	X3dThreeDDrawCallbackStruct *select_callback_data;

	ENTER(texture_graphics_input_callback);
	/* checking arguments */
	if (drawing_widget&&(texture_window=(struct Texture_window *)user_data)&&
		(select_callback_data=(X3dThreeDDrawCallbackStruct *)call_data)&&
		(X3dCR_INPUT==select_callback_data->reason))
	{
/*???debug */

	int event_type;
	XButtonEvent *button_event;
	XEvent *event;
	XKeyEvent *key_event;
	XMotionEvent *motion_event;
	Dimension width, height;
	int shift;

	XtVaGetValues(drawing_widget, XtNheight, &height, XtNwidth, &width, NULL);

	if (event=select_callback_data->event)
	{
		event_type=event->type;

		switch (event_type)
		{
			case ButtonPress: case ButtonRelease:
			{
				button_event=&(event->xbutton);
				if (ButtonPress==event_type)
				{
					shift = 0;
					if ((button_event->state & ShiftMask))
					{
						shift = 1;
						printf("Shift_key depressed\n");
					}
					current_button = button_event->button;
					cursor_x = (double) button_event->x/(double) width;
					cursor_y = (double) (height-button_event->y)/ (double) height;

					printf("button %u press at %d %d [%.3lf %.3lf]\n",button_event->button,
						button_event->x,button_event->y,cursor_x,height-cursor_y);
					if (texture_window->pick_mode)
					{
						process_pick(texture_window, current_button, button_event->x,
							height-button_event->y, shift);
					}
					old_motion_x = cursor_x;
					old_motion_y = cursor_y;
				}
				else
				{
					current_button = 0;
					printf("button %u release at %d %d\n",button_event->button,
						button_event->x,button_event->y);

				}
			} break;
			case KeyPress: case KeyRelease:
			{
				key_event= &(event->xkey);
				if (KeyPress==event_type)
				{
					printf("key %u press at %d %d\n",key_event->keycode,key_event->x,
						key_event->y);
				}
				else
				{
					printf("key %u release at %d %d\n",key_event->keycode,key_event->x,
						key_event->y);
				}
			} break;
			case MotionNotify:
			{
				motion_event= &(event->xmotion);
				motion_x =  (double) motion_event->x/(double) width;
				motion_y =  (double) (height-motion_event->y)/ (double) height;



				/*printf("motion at %d %d [%lf, %lf]\n",motion_event->x,motion_event->y,
					motion_x-old_motion_x,motion_y-old_motion_y);*/
				volume_window_motion(texture_window,motion_x-old_motion_x,motion_y-old_motion_y,
					current_button);
				old_motion_x = motion_x;
				old_motion_y = motion_y;


			} break;
			default:
			{
				printf("input_callback.  Invalid X event");
			} break;
		}
	}
	else
	{
		printf("input_callback.  Missing X event");
	}

	}
	LEAVE;

} /* select_drawing_callback */
