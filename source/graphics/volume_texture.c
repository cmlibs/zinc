/*******************************************************************************
FILE : volume_texture.c

LAST MODIFIED : 21 January 2002

DESCRIPTION :
Contains data structures for the 3d volumetric textures to be mapped onto
finite elements.  A volume texture is a group of texture 'elements' defined as
lists of texture 'nodes'.  Material values may be associated either with
elements, or with nodes.  The texture nodes are arranged as a 3d lattice in xi1,
xi2, xi3 space.
???DB.  OPENGL ?
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "command/parser.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/mcubes.h"
#include "graphics/texture_line.h"
#include "graphics/volume_texture.h"
#include "user_interface/message.h"

#define MAX_OBJ_VERTICES 100
#define OBJ_V_EPSILON 0.0000001

/*
Module types
------------
*/
FULL_DECLARE_INDEXED_LIST_TYPE(VT_volume_texture);

FULL_DECLARE_MANAGER_TYPE(VT_volume_texture);

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(VT_volume_texture,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(VT_volume_texture)

/*???Mark.  temp clip function for testing */
#if defined (OLDCLIP)
double plane(double *parameters)
{
	double a = 1.0;
	double b = 1.0;
	double c = 1.0;
	double d = -1.0;
	double result;

	result=a*parameters[0] + b*parameters[1] + c*parameters[2] + d;

	return(result);
} /* plane */

/* Use these values to clip the heart in half */
double a = 0.0;
double b = 0.0;
double c = 1.0;
double d = 0.75;/*.25*/
#endif

double plane(double *position,double *parameters)
{
	double result;

	result=position[0]*parameters[0] + position[1]*parameters[1] +
		position[2]*parameters[2] + parameters[3];

	return (result);
}

static int normalize_obj(struct VT_volume_texture *vtexture)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Normalizes and scales the xi data to fit in an element block.
(normally used for obj files)
==============================================================================*/
{
	int i,j,return_code;
	double v_max[3]={0, 0, 0},v_min[3]={0, 0, 0}, max=0, min=0, v_scale;
	double middle[3];

	ENTER(normalize_obj);
	return_code=0;
	/* check arguments */
	if (vtexture)
	{
		if (vtexture->mc_iso_surface)
		{
/*???debug */
printf("Normalizing vtexture\n");
			/* scan through vertices to find bounding box */
			for (i=0;i<vtexture->mc_iso_surface->n_vertices;i++)
			{
				for (j=0;j<3;j++)
				{
					if (vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j]>max)
					{
						max=vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j];
					}
					if (vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j]<min)
					{
						min=vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j];
					}
					if (vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j]>
						v_max[j])
					{
						v_max[j]=
							vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j];
					}
					if (vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j]<
						v_min[j])
					{
						v_min[j]=
							vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j];
					}
				}
			}
			/* calc centre of model and shift model to centre */
			for (i=0;i<3;i++)
			{
				middle[i] = (v_max[i]-v_min[i])/2.0 + v_min[i];
			}
			/* find greatest length */
			v_scale = 0;
			for (i=0;i<3;i++)
			{
				if (v_max[i] - middle[i] > v_scale)
				{
					v_scale = v_max[i] - middle[i];
				}
			}
			v_scale *= 2.0 * (1.0 + OBJ_V_EPSILON);

			for (i=0;i<vtexture->mc_iso_surface->n_vertices;i++)
			{
				/* only scale if deformable! */
				if (NULL!=vtexture->mc_iso_surface->deform)
				{
#if defined (DO_NOT_SCALE_ANYTHING)
					if (0==vtexture->mc_iso_surface->deform[i])
					{
						for (j=0;j<3;j++)
						{
							/* normalize xi, then scale to fit into element block */
							vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j]=
								vtexture->ximax[j]*
								(vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j]-
								middle[j])/v_scale+(vtexture->ximax[j])/2.0;
						}
					}
#endif /* defined (DO_NOT_SCALE_ANYTHING) */
				}
				else
				{
#if defined (DEBUG)
					/*???debug */
					printf("DEBUG: %p I am scaling, I am scaling, through the ...\n",
						vtexture->mc_iso_surface->deform);
#endif /* defined (DEBUG) */
					for (j=0;j<3;j++)
					{
						/* normalize xi, then scale to fit into element block */
						/* MS 16/9/98 altered to use ximin */
 						vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j]=
							((vtexture->ximax[j]-vtexture->ximin[j])*
							(vtexture->mc_iso_surface->compiled_vertex_list[i]->coord[j]-
							middle[j]+vtexture->offset_xi[j])+vtexture->ximin[j])
							*vtexture->scale_xi[j]/v_scale+(vtexture->ximax[j])/2.0;
					}
				}
			}
			display_message(WARNING_MESSAGE,
				"Volume data scaled by (%lf, %lf, %lf); shift = (%lf %lf %lf)",
 				vtexture->ximax[0]*vtexture->scale_xi[j]/v_scale,
 				vtexture->ximax[1]*vtexture->scale_xi[j]/v_scale,
 				vtexture->ximax[2]*vtexture->scale_xi[j]/v_scale,
 				vtexture->ximax[0]/v_scale*(vtexture->offset_xi[j]-middle[0]),
 				vtexture->ximax[1]/v_scale*(vtexture->offset_xi[j]-middle[1]),
 				vtexture->ximax[2]/v_scale*(vtexture->offset_xi[j]-middle[2]));
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"normalize_obj.  iso_surface = NULL");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"normalize_obj.  vtexture = NULL");
	}
	LEAVE;

	return (return_code);
} /* normalize_obj */

static int set_VT_volume_texture_file(struct Parse_state *state,
	void *volume_texture_void,void *data_void)
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Modifier function to set the volume texture file from a command.
==============================================================================*/
{
	char *file_name;
	FILE *in_file;
	int return_code;
	struct Modifier_entry *entry;
	struct Modify_VT_volume_texture_data *data;

	ENTER(set_VT_volume_texture_file);
	if (state)
	{
		if (volume_texture_void&&
			(data=(struct Modify_VT_volume_texture_data *)data_void))
		{
			if (entry=data->set_file_name_option_table)
			{
				file_name=(char *)NULL;
				while (entry->option)
				{
					entry->to_be_modified= &file_name;
					entry++;
				}
				entry->to_be_modified= &file_name;
				if (return_code=process_option(state,data->set_file_name_option_table))
				{
					if (in_file=fopen(file_name,"r"))
					{
						if (strstr(file_name, ".objv") != NULL )
						{
							display_message(WARNING_MESSAGE,
								"Importing .objv file : Volume functions disabled");
							if (read_volume_texture_from_obj_file(
								(struct VT_volume_texture *)volume_texture_void,in_file,
								data->graphical_material_manager,data->environment_map_manager,
								1))
							{
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Error reading volume texture objv file: %s",file_name);
								return_code=0;
							}
						}
						else
						{
							if (strstr(file_name, ".obj") != NULL )
							{
								display_message(WARNING_MESSAGE,
									"Importing .obj file : Volume functions disabled");
								if (read_volume_texture_from_obj_file(
									(struct VT_volume_texture *)volume_texture_void,in_file,
									data->graphical_material_manager,
									data->environment_map_manager,0))
								{
									return_code=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Error reading volume texture obj file: %s",file_name);
									return_code=0;
								}
							}
							else
							{
								if (read_volume_texture_from_file(
									(struct VT_volume_texture *)volume_texture_void,in_file,
									data->graphical_material_manager,
									data->environment_map_manager))
								{
									return_code=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Error reading volume texture file: %s",file_name);
									return_code=0;
								}
							}
						}
						fclose(in_file);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid file name: %s",file_name);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					return_code=1;
				}
				DEALLOCATE(file_name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_VT_volume_texture_file.  Missing set_file_name_option_table");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_VT_volume_texture_file.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_VT_volume_texture_file.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_VT_volume_texture_file */

/*
Global functions
----------------
*/
struct VT_volume_texture *CREATE(VT_volume_texture)(char *name)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Allocates memory and assigns fields for a volume texture.  Adds the texture to
the list of all volume textures.
???DB.  Trimming name ?
???DB.  Check if it already exists ?  Retrieve if it does already exist ?
???DB.  Added option for reading in nodal scalars rather than calculating from
	cell values
==============================================================================*/
{
	struct VT_volume_texture *texture;

	ENTER(CREATE(VT_volume_texture));
#if defined (OLD_CODE)
	/*???DB.  Is there a better place for this ? */
	load_mc_tables();
#endif /* defined (OLD_CODE) */
	/* allocate memory for structure */
	if (ALLOCATE(texture,struct VT_volume_texture,1)&&
		ALLOCATE(texture->scalar_field,struct VT_scalar_field,1)&&
		ALLOCATE(texture->clip_field,struct VT_scalar_field,1)&&
		ALLOCATE(texture->clip_field2,struct VT_scalar_field,1)&&
		ALLOCATE(texture->coordinate_field,struct VT_vector_field,1)&&
		ALLOCATE(texture->texture_curve_list,struct VT_texture_curve *,1))
	{
		if (name)
		{
			if (ALLOCATE(texture->name,char,strlen(name)+1))
			{
				strcpy(texture->name,name);
			}
		}
		else
		{
			if (ALLOCATE(texture->name,char,1))
			{
				*(texture->name)='\0';
			}
		}
		if (texture->name)
		{
			texture->access_count=0;
			(texture->dimension)[0]= -1;
			(texture->dimension)[1]= -1;
			(texture->dimension)[2]= -1;
			(texture->ximin)[0]=0;
			(texture->ximin)[1]=0;
			(texture->ximin)[2]=0;
			(texture->ximax)[0]=1;
			(texture->ximax)[1]=1;
			(texture->ximax)[2]=1;
			texture->scale_xi[0]=1;
			texture->scale_xi[1]=1;
			texture->scale_xi[2]=1;
			texture->offset_xi[0]=0;
			texture->offset_xi[1]=0;
			texture->offset_xi[2]=0;
			texture->n_groups=0;
			texture->node_groups=(struct VT_node_group **)NULL;
			texture->grid_spacing=(double *)NULL;
			texture->file_name=(char *)NULL;
			*(texture->texture_curve_list)=(struct VT_texture_curve *)NULL;
			texture->texture_cell_list=(struct VT_texture_cell **)NULL;
			texture->global_texture_node_list=(struct VT_texture_node **)NULL;
			(texture->scalar_field->dimension)[0]= -1;
			(texture->scalar_field->dimension)[1]= -1;
			(texture->scalar_field->dimension)[2]= -1;
			texture->scalar_field->scalar=(double *)NULL;
			(texture->clip_field->dimension)[0]= -1;
			(texture->clip_field->dimension)[1]= -1;
			(texture->clip_field->dimension)[2]= -1;
			texture->clip_field->scalar=(double *)NULL;
			(texture->clip_field2->dimension)[0]= -1;
			(texture->clip_field2->dimension)[1]= -1;
			(texture->clip_field2->dimension)[2]= -1;
			texture->clip_field2->scalar=(double *)NULL;
			(texture->coordinate_field->dimension)[0]= -1;
			(texture->coordinate_field->dimension)[1]= -1;
			(texture->coordinate_field->dimension)[2]= -1;
			texture->coordinate_field->vector=(double *)NULL;
			texture->mc_iso_surface=(struct MC_iso_surface *)NULL;
			/*???DB.  Added option for reading in nodal scalars rather than
				calculating from cell values */
			texture->calculate_nodal_values=1;
			texture->recalculate=1;
			texture->isovalue=0;
			texture->hollow_mode_on=0;
			texture->hollow_isovalue=0;
			texture->clipping_field_on=0;
			texture->cutting_plane_on=0;
			texture->cut_isovalue=0;
			texture->closed_surface=0;
			texture->decimation=0;
			texture->disable_volume_functions=0;
			/*???DB.  Should do more initialization of values */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(VT_volume_texture).  Insufficient memory for name");
			DEALLOCATE(texture->coordinate_field);
			DEALLOCATE(texture->clip_field2);
			DEALLOCATE(texture->clip_field);
			DEALLOCATE(texture->scalar_field);
			DEALLOCATE(texture);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(VT_volume_texture).  Insufficient memory for structure");
		if (texture)
		{
			if (texture->scalar_field)
			{
				if (texture->clip_field)
				{
					if (texture->clip_field2)
					{
						DEALLOCATE(texture->coordinate_field);
						DEALLOCATE(texture->clip_field2);
					}
					DEALLOCATE(texture->clip_field);
				}
				DEALLOCATE(texture->scalar_field);
			}
			DEALLOCATE(texture);
		}
	}
	LEAVE;

	return (texture);
} /* CREATE(VT_volume_texture) */

int DESTROY(VT_volume_texture)(struct VT_volume_texture **texture_address)
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Frees the memory for the volume texture and sets <*texture_address> to NULL.
==============================================================================*/
{
	int i,n_cells,n_nodes,number_of_scalar_fields,return_code;
	struct VT_node_group *node_group;
	struct VT_texture_cell **cell;
	struct VT_texture_curve *curve,*curve_next;
	struct VT_texture_node **node;
	struct VT_volume_texture *texture;

	ENTER(DESTROY(VT_volume_texture));
	if (texture_address)
	{
		if (texture= *texture_address)
		{
			if (texture->access_count<=0)
			{
				DEALLOCATE(texture->name);
				if (texture->texture_curve_list)
				{
					curve= *(texture->texture_curve_list);
					while (curve)
					{
						curve_next=curve->ptrnext;
						DEALLOCATE(curve);
						curve=curve_next;
					}
					DEALLOCATE(texture->texture_curve_list);
				}
				if (cell=texture->texture_cell_list)
				{
					n_cells=(texture->dimension[0])*(texture->dimension[1])*
						(texture->dimension[2]);
					for (i=n_cells;i>0;i--)
					{
						if (*cell)
						{
							DEACCESS(Graphical_material)(&((*cell)->material));
							DEACCESS(Environment_map)(&((*cell)->env_map));
						}
						cell++;
					}
					DEALLOCATE(*texture->texture_cell_list);
					DEALLOCATE(texture->texture_cell_list);
				}
				if (node=texture->global_texture_node_list)
				{
					n_nodes=(texture->dimension[0]+1)*(texture->dimension[1]+1)*
						(texture->dimension[2]+1);
					for (i=n_nodes;i>0;i--)
					{
						if (*node)
						{
							DEACCESS(Graphical_material)(&((*node)->material));
							DEACCESS(Graphical_material)(&((*node)->dominant_material));
						}
						node++;
					}
					DEALLOCATE(*texture->global_texture_node_list);
					DEALLOCATE(texture->global_texture_node_list);
				}
				if (texture->node_groups)
				{
					for (i=texture->n_groups;i>0;i--)
					{
						node_group=texture->node_groups[i];
						DEALLOCATE(node_group->nodes);
						DEALLOCATE(node_group);
					}
					DEALLOCATE(texture->node_groups);
					texture->n_groups = 0;
				}
				if (texture->mc_iso_surface)
				{
					number_of_scalar_fields=0;
					if (texture->scalar_field)
					{
						number_of_scalar_fields++;
					}
					if (texture->hollow_mode_on)
					{
						number_of_scalar_fields++;
					}
					if (texture->clip_field)
					{
/*???SAB.						number_of_scalar_fields++;*/

					}
#if defined (OLD_CODE)
/*???DB.  Doesn't seem to create a triangle list for the clip_field2 ? */
					if (texture->clip_field2)
					{
						number_of_scalar_fields++;
					}
#endif /* defined (OLD_CODE) */
					clean_mc_iso_surface(number_of_scalar_fields,texture->mc_iso_surface);
				}
				if (texture->scalar_field)
				{
					DEALLOCATE(texture->scalar_field->scalar);
					DEALLOCATE(texture->scalar_field);
				}
				if (texture->clip_field)
				{
					DEALLOCATE(texture->clip_field->scalar);
					DEALLOCATE(texture->clip_field);
				}
				if (texture->clip_field2)
				{
					DEALLOCATE(texture->clip_field2->scalar);
					DEALLOCATE(texture->clip_field2);
				}
				if (texture->coordinate_field)
				{
					DEALLOCATE(texture->coordinate_field->vector);
					DEALLOCATE(texture->coordinate_field);
				}
				if (texture->grid_spacing)
				{
					DEALLOCATE(texture->grid_spacing);
				}
				DEALLOCATE(texture->mc_iso_surface);
				/*???DB.  accessing and deaccessing iso_poly_material ? */
				DEALLOCATE(*texture_address);
				return_code=1;
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(VT_volume_texture).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(VT_volume_texture) */

DECLARE_OBJECT_FUNCTIONS(VT_volume_texture)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(VT_volume_texture)

DECLARE_INDEXED_LIST_FUNCTIONS(VT_volume_texture)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(VT_volume_texture,name,char *,
	strcmp)

DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(VT_volume_texture,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(VT_volume_texture,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(VT_volume_texture,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for name");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name)(
				destination, source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(VT_volume_texture,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(VT_volume_texture,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(VT_volume_texture,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(VT_volume_texture,name)
/*******************************************************************************
LAST MODIFIED : 29 September 1998

DESCRIPTION :
Copies volume texture from source to destination.
Syntax: MANAGER_COPY_WITH_IDENTIFIER(VT_volume_texture,name)(destination,source)
???RC Previously did not clean up existing fields of destination, if set.
==============================================================================*/
{
	char *file_name;
	double *destination_double,*grid_spacing,*source_double;
	int *destination_int,*group_nodes,i,j,k,l,m,n_cells,n_groups,n_mc_cells,
		n_nodes,n_points,n_triangle_lists,return_code,*source_deform,
		*source_int,maxk;
	struct Environment_map *env_map;
	struct MC_cell *cell,**cells,*source_cell,**source_cells;
	struct MC_iso_surface *iso_surface,*source_iso_surface;
	struct MC_triangle **compiled_triangle,**source_compiled_triangle,
		*source_triangle,**source_triangle_ptr,*triangle,**triangle_ptr;
	struct MC_vertex **compiled_vertex,**source_compiled_vertex,
		*source_vertex,*vertex;
#if defined (OLD_CODE)
/*???DB.  Needs to be replaced by new iso surface structure */
	struct VT_iso_surface *iso_surface,*source_iso_surface;
	struct VT_iso_vertex *destination_iso_vertex,*source_iso_vertex;
#endif /* defined (OLD_CODE) */
	struct VT_node_group **destination_node_groups,**node_groups,
		**source_node_groups;
	struct VT_scalar_field *clip_field,*clip_field2,*scalar_field;
	struct VT_texture_cell **destination_texture_cell,*destination_cell_block,
		**source_texture_cell,**texture_cell_list;
	struct VT_texture_curve *destination_curve,*source_curve,**texture_curve_list;
	struct VT_texture_node **destination_node,*destination_node_block,
		**source_node,**texture_node_list;
	struct VT_vector_field *coordinate_field;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name));
/*???debug */
	/*printf("enter MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name)\n");*/
	/* check arguments */
	if (source&&destination)
	{
		return_code=1;
		file_name=(char *)NULL;
		texture_curve_list=(struct VT_texture_curve **)NULL;
		texture_cell_list=(struct VT_texture_cell **)NULL;
		texture_node_list=(struct VT_texture_node **)NULL;
		scalar_field=(struct VT_scalar_field *)NULL;
		clip_field=(struct VT_scalar_field *)NULL;
		clip_field2=(struct VT_scalar_field *)NULL;
		coordinate_field=(struct VT_vector_field *)NULL;
		iso_surface=(struct MC_iso_surface *)NULL;
		grid_spacing=(double *)NULL;
		node_groups=(struct VT_node_group **)NULL;

#if defined (OLD_CODE)
/*???DB.  Needs to be replaced by new iso surface structure */
		iso_surface=(struct VT_iso_surface *)NULL;
#endif /* defined (OLD_CODE) */
		if (source->file_name)
		{
			if (ALLOCATE(file_name,char,strlen(source->file_name)+1))
			{
				strcpy(file_name,source->file_name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for image file name");
				return_code=0;
			}
		}

		if (return_code)
		{
			/* texture curve list */
			if (texture_curve_list=source->texture_curve_list)
			{
				source_curve= *texture_curve_list;
				if (ALLOCATE(texture_curve_list,struct VT_texture_curve *,1))
				{
					if (source_curve)
					{
						if (ALLOCATE(destination_curve,struct VT_texture_curve,1))
						{
							*texture_curve_list=destination_curve;
							do
							{
								destination_curve->type=source_curve->type;
								destination_curve->index=source_curve->index;
								(destination_curve->scalar_value)[0]=
									(source_curve->scalar_value)[0];
								(destination_curve->scalar_value)[1]=
									(source_curve->scalar_value)[1];
								(destination_curve->point1)[0]=(source_curve->point1)[0];
								(destination_curve->point1)[1]=(source_curve->point1)[1];
								(destination_curve->point1)[2]=(source_curve->point1)[2];
								(destination_curve->point2)[0]=(source_curve->point2)[0];
								(destination_curve->point2)[1]=(source_curve->point2)[1];
								(destination_curve->point2)[2]=(source_curve->point2)[2];
								(destination_curve->point3)[0]=(source_curve->point3)[0];
								(destination_curve->point3)[1]=(source_curve->point3)[1];
								(destination_curve->point3)[2]=(source_curve->point3)[2];
								(destination_curve->point4)[0]=(source_curve->point4)[0];
								(destination_curve->point4)[1]=(source_curve->point4)[1];
								(destination_curve->point4)[2]=(source_curve->point4)[2];
								destination_curve->ptrnext=(struct VT_texture_curve *)NULL;
								if (source_curve=source_curve->ptrnext)
								{
									if (ALLOCATE(destination_curve->ptrnext,
										struct VT_texture_curve,1))
									{
										destination_curve=destination_curve->ptrnext;
									}
									else
									{
										display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture curve");
										return_code=0;
									}
								}
							}
							while (return_code&&source_curve);
						}
						else
						{
							display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture curve");
							return_code=0;
							DEALLOCATE(texture_curve_list);
						}
					}
					else
					{
						*texture_curve_list=(struct VT_texture_curve *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture curve list");
					return_code=0;
				}
			}
			if (return_code)
			{
				/* texture cell list */
				if (source_texture_cell=source->texture_cell_list)
				{
					n_cells=(source->dimension[0])*(source->dimension[1])*
						(source->dimension[2]);
					if (ALLOCATE(texture_cell_list,struct VT_texture_cell *,n_cells))
					{
						destination_texture_cell=texture_cell_list;
						i=n_cells;
						while (i>0)
						{
							*destination_texture_cell=(struct VT_texture_cell *)NULL;
							destination_texture_cell++;
							i--;
						}
						destination_texture_cell=texture_cell_list;
						i=0;
						if (ALLOCATE(destination_cell_block,struct VT_texture_cell,n_cells))
						{
							while (return_code&&(i<n_cells))
							{
								if (*source_texture_cell)
								{
									*destination_texture_cell = destination_cell_block+i;
									(*destination_texture_cell)->index=
										(*source_texture_cell)->index;
									if ((*source_texture_cell)->material)
									{
										(*destination_texture_cell)->material=
											ACCESS(Graphical_material)(
											(*source_texture_cell)->material);
									}
									else
									{
										(*destination_texture_cell)->material=
											(struct Graphical_material *)NULL;
									}
									if ((*source_texture_cell)->env_map)
									{
										(*destination_texture_cell)->env_map=
											ACCESS(Environment_map)((*source_texture_cell)->env_map);
									}
									else
									{
										(*destination_texture_cell)->env_map=
											(struct Environment_map *)NULL;
									}
									/*???RC detail was not being copied! */
									(*destination_texture_cell)->detail=
										(*source_texture_cell)->detail;
									((*destination_texture_cell)->cop)[0]=
										((*source_texture_cell)->cop)[0];
									((*destination_texture_cell)->cop)[1]=
										((*source_texture_cell)->cop)[1];
									((*destination_texture_cell)->cop)[2]=
										((*source_texture_cell)->cop)[2];
									(*destination_texture_cell)->interpolation_fn=
										(*source_texture_cell)->interpolation_fn;
									(*destination_texture_cell)->scalar_value=
										(*source_texture_cell)->scalar_value;
								}
								else
								{
									display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture cell");
									return_code=0;
								}
							}
							source_texture_cell++;
							destination_texture_cell++;
							i++;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture cell list");
						return_code=0;
					}
				}
				if (return_code)
				{
					/* node_groups */
					if (source_node_groups=source->node_groups)
					{
						n_groups=source->n_groups;
						if (ALLOCATE(node_groups,struct VT_node_group *,n_groups))
						{
							destination_node_groups=node_groups;
							i=n_groups;
							while (i>0)
							{
								*destination_node_groups=(struct VT_node_group *)NULL;
								destination_node_groups++;
								i--;
							}
							destination_node_groups=node_groups;
							i=n_groups;
							while (return_code&&(i>0))
							{
								if (*source_node_groups)
								{
									if (ALLOCATE(*destination_node_groups,struct VT_node_group,1))
									{
										(*destination_node_groups)->n_nodes=
											(*source_node_groups)->n_nodes;
										if (ALLOCATE((*destination_node_groups)->name,char,
											strlen((*source_node_groups)->name)+1))
										{
											strcpy((*destination_node_groups)->name,
												(*source_node_groups)->name);
											if ((ALLOCATE(group_nodes,int,
												(*source_node_groups)->n_nodes)))
											{
												(*destination_node_groups)->nodes=group_nodes;
												for (j=0;j<(*source_node_groups)->n_nodes;j++)
												{
													(*destination_node_groups)->nodes[j]=
														(*source_node_groups)->nodes[j];
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture node group nodes 3");
												DEALLOCATE(*destination_node_groups);
												DEALLOCATE((*destination_node_groups)->name);
												return_code=0;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture node_group 2");
											DEALLOCATE(*destination_node_groups);
											return_code=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture node_group 1");
										return_code=0;
									}
								}
								else
								{
									*destination_node_groups=(struct VT_node_group *)NULL;
								}
								source_node_groups++;
								destination_node_groups++;
								i--;
							}
							if (!return_code)
							{
								i++;
								destination_node_groups--;
								while (i<n_groups)
								{
									i++;
									destination_node_groups--;
									if (*destination_node_groups)
									{
										DEALLOCATE((*destination_node_groups)->nodes);
										DEALLOCATE((*destination_node_groups)->name);
										DEALLOCATE(*destination_node_groups);
									}
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture node groups");
							return_code=0;
						}
					}
				}
				if (return_code)
				{
					/* global node list */
					if (source_node=source->global_texture_node_list)
					{
						n_nodes=(source->dimension[0]+1)*(source->dimension[1]+1)*
							(source->dimension[2]+1);
						if (ALLOCATE(texture_node_list,struct VT_texture_node *,n_nodes))
						{
							destination_node=texture_node_list;
							i=n_nodes;
							while (i>0)
							{
								*destination_node=(struct VT_texture_node *)NULL;
								destination_node++;
								i--;
							}
							destination_node=texture_node_list;
							i=0;
							if (ALLOCATE(destination_node_block,struct VT_texture_node,n_nodes))
							{
								while (return_code&&(i<n_nodes))
								{
									if (*source_node)
									{
										*destination_node = destination_node_block+i;
										(*destination_node)->index=(*source_node)->index;
										if ((*source_node)->material)
										{
											(*destination_node)->material=
												ACCESS(Graphical_material)((*source_node)->material);
										}
										else
										{
											(*destination_node)->material=
												(struct Graphical_material *)NULL;
										}
										if ((*source_node)->dominant_material)
										{
											(*destination_node)->dominant_material=
												ACCESS(Graphical_material)(
												(*source_node)->dominant_material);
										}
										else
										{
											(*destination_node)->dominant_material=
												(struct Graphical_material *)NULL;
										}
										((*destination_node)->scalar_gradient)[0]=
											((*source_node)->scalar_gradient)[0];
										((*destination_node)->scalar_gradient)[1]=
											((*source_node)->scalar_gradient)[1];
										((*destination_node)->scalar_gradient)[2]=
											((*source_node)->scalar_gradient)[2];
										((*destination_node)->cop)[0]=((*source_node)->cop)[0];
										((*destination_node)->cop)[1]=((*source_node)->cop)[1];
										((*destination_node)->cop)[2]=((*source_node)->cop)[2];
										(*destination_node)->clipping_fn_value=
											(*source_node)->clipping_fn_value;
										(*destination_node)->scalar_value=
											(*source_node)->scalar_value;
										(*destination_node)->active=(*source_node)->active;
										(*destination_node)->node_type=(*source_node)->node_type;
										for (j=0;j<8;j++)
										{
											(*destination_node)->cm_node_identifier[j]=
												(*source_node)->cm_node_identifier[j];
										}
									}
									source_node++;
									destination_node++;
									i++;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture node");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for texture node list");
							return_code=0;
						}
					}
					if (return_code)
					{
						/* scalar field */
						if (source->scalar_field)
						{
							if (ALLOCATE(scalar_field,struct VT_scalar_field,1))
							{
								(scalar_field->dimension)[0]=
									(source->scalar_field->dimension)[0];
								(scalar_field->dimension)[1]=
									(source->scalar_field->dimension)[1];
								(scalar_field->dimension)[2]=
									(source->scalar_field->dimension)[2];
								if ((0<(scalar_field->dimension)[0])&&
									(0<(scalar_field->dimension)[1])&&
									(0<(scalar_field->dimension)[2]))
								{
									n_points=((scalar_field->dimension)[0]+1)*
										((scalar_field->dimension)[1]+1)*
										((scalar_field->dimension)[2]+1);
									if (ALLOCATE(scalar_field->scalar,double,n_points))
									{
										destination_double=scalar_field->scalar;
										source_double=source->scalar_field->scalar;
										for (i=n_points;i>0;i--)
										{
											*destination_double= *source_double;
											destination_double++;
											source_double++;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for scalar field values");
										return_code=0;
									}
								}
								else
								{
									scalar_field->scalar=(double *)NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for scalar field");
								return_code=0;
							}
						}
						if (return_code)
						{
							/* clip field */
							if (source->clip_field)
							{
								if (ALLOCATE(clip_field,struct VT_scalar_field,1))
								{
									(clip_field->dimension)[0]=(source->clip_field->dimension)[0];
									(clip_field->dimension)[1]=(source->clip_field->dimension)[1];
									(clip_field->dimension)[2]=(source->clip_field->dimension)[2];
									if ((0<(clip_field->dimension)[0])&&
										(0<(clip_field->dimension)[1])&&
										(0<(clip_field->dimension)[2]))
									{
										n_points=((clip_field->dimension)[0]+1)*
											((clip_field->dimension)[1]+1)*
											((clip_field->dimension)[2]+1);
										if (ALLOCATE(clip_field->scalar,double,n_points))
										{
											destination_double=clip_field->scalar;
											source_double=source->clip_field->scalar;
											for (i=n_points;i>0;i--)
											{
												*destination_double= *source_double;
												destination_double++;
												source_double++;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for clip field values");
											return_code=0;
										}
									}
									else
									{
										clip_field->scalar=(double *)NULL;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for clip field");
									return_code=0;
								}
							}
							if (return_code)
							{
								/* clip field 2 */
								if (source->clip_field2)
								{
									if (ALLOCATE(clip_field2,struct VT_scalar_field,1))
									{
										(clip_field2->dimension)[0]=
											(source->clip_field2->dimension)[0];
										(clip_field2->dimension)[1]=
											(source->clip_field2->dimension)[1];
										(clip_field2->dimension)[2]=
											(source->clip_field2->dimension)[2];
										if ((0<(clip_field2->dimension)[0])&&
											(0<(clip_field2->dimension)[1])&&
											(0<(clip_field2->dimension)[2]))
										{
											n_points=((clip_field2->dimension)[0]+1)*
												((clip_field2->dimension)[1]+1)*
												((clip_field2->dimension)[2]+1);
											if (ALLOCATE(clip_field2->scalar,double,n_points))
											{
												destination_double=clip_field2->scalar;
												source_double=source->clip_field2->scalar;
												for (i=n_points;i>0;i--)
												{
													*destination_double= *source_double;
													destination_double++;
													source_double++;
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for clip field 2 values");
												return_code=0;
											}
										}
										else
										{
											clip_field2->scalar=(double *)NULL;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for clip field 2");
										return_code=0;
									}
								}
								if (return_code)
								{
									/* coordinate field */
									if (source->coordinate_field)
									{
										if (ALLOCATE(coordinate_field,struct VT_vector_field,1))
										{
											(coordinate_field->dimension)[0]=
												(source->coordinate_field->dimension)[0];
											(coordinate_field->dimension)[1]=
												(source->coordinate_field->dimension)[1];
											(coordinate_field->dimension)[2]=
												(source->coordinate_field->dimension)[2];
											if ((0<(coordinate_field->dimension)[0])&&
												(0<(coordinate_field->dimension)[1])&&
												(0<(coordinate_field->dimension)[2]))
											{
												n_points=((coordinate_field->dimension)[0]+1)*
													((coordinate_field->dimension)[1]+1)*
													((coordinate_field->dimension)[2]+1);
												if (ALLOCATE(coordinate_field->vector,double,
													3*n_points))
												{
													destination_double=coordinate_field->vector;
													source_double=source->coordinate_field->vector;
													for (i=3*n_points;i>0;i--)
													{
														*destination_double= *source_double;
														destination_double++;
														source_double++;
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for coordinate field values");
													return_code=0;
												}
											}
											else
											{
												coordinate_field->vector=(double *)NULL;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for coordinate field");
											return_code=0;
										}
									}
									if (return_code)
									{
										/* iso surface */
										if (source_iso_surface=source->mc_iso_surface)
										{
											if (ALLOCATE(iso_surface,struct MC_iso_surface,1))
											{
												(iso_surface->dimension)[0]=
													(source_iso_surface->dimension)[0];
												(iso_surface->dimension)[1]=
													(source_iso_surface->dimension)[1];
												(iso_surface->dimension)[2]=
													(source_iso_surface->dimension)[2];
												(iso_surface->active_block)[0]=
													(source_iso_surface->active_block)[0];
												(iso_surface->active_block)[1]=
													(source_iso_surface->active_block)[1];
												(iso_surface->active_block)[2]=
													(source_iso_surface->active_block)[2];
												(iso_surface->active_block)[3]=
													(source_iso_surface->active_block)[3];
												(iso_surface->active_block)[4]=
													(source_iso_surface->active_block)[4];
												(iso_surface->active_block)[5]=
													(source_iso_surface->active_block)[5];
												iso_surface->n_scalar_fields=
													source_iso_surface->n_scalar_fields;
												iso_surface->n_vertices=source_iso_surface->n_vertices;
												iso_surface->n_triangles=
													source_iso_surface->n_triangles;
												(iso_surface->xi_face_poly_index)[0]=
													(source_iso_surface->xi_face_poly_index)[0];
												(iso_surface->xi_face_poly_index)[1]=
													(source_iso_surface->xi_face_poly_index)[1];
												(iso_surface->xi_face_poly_index)[2]=
													(source_iso_surface->xi_face_poly_index)[2];
												(iso_surface->xi_face_poly_index)[3]=
													(source_iso_surface->xi_face_poly_index)[3];
												(iso_surface->xi_face_poly_index)[4]=
													(source_iso_surface->xi_face_poly_index)[4];
												(iso_surface->xi_face_poly_index)[5]=
													(source_iso_surface->xi_face_poly_index)[5];

												/* 1. copy over deformable vertices if list not NULL */
												if (source_deform=source_iso_surface->deform)
												{
													if (ALLOCATE(iso_surface->deform,int,
														source_iso_surface->n_vertices))
													{
														for (i=0;i<source_iso_surface->n_vertices;i++)
														{
															iso_surface->deform[i]=source_deform[i];
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  "
															"Could not copy deform field");
														return_code=0;
													}
												}
												else
												{
													iso_surface->deform = NULL;
												}

												/* 2. Copy the compiled vertex list, if any */
												if (return_code&&(0<source_iso_surface->n_vertices)&&
													(source_compiled_vertex=source_iso_surface->compiled_vertex_list))
												{
													iso_surface->n_vertices=source_iso_surface->n_vertices;
													if (ALLOCATE(iso_surface->compiled_vertex_list,
														struct MC_vertex *,iso_surface->n_vertices))
													{
														compiled_vertex=iso_surface->compiled_vertex_list;
														for (i=0;return_code&&(i<iso_surface->n_vertices);i++)
														{
															source_vertex= *source_compiled_vertex;
															source_compiled_vertex++;
															/* create a copy of the source vertex */
															if (ALLOCATE(vertex,struct MC_vertex,1)&&
																ALLOCATE(vertex->triangle_ptrs,struct MC_triangle *,
																	source_vertex->n_triangle_ptrs))
															{
																*compiled_vertex=vertex;
																compiled_vertex++;
																vertex->vertex_index=source_vertex->vertex_index;
																(vertex->coord)[0]=(source_vertex->coord)[0];
																(vertex->coord)[1]=(source_vertex->coord)[1];
																(vertex->coord)[2]=(source_vertex->coord)[2];
																(vertex->normal)[0]=(source_vertex->normal)[0];
																(vertex->normal)[1]=(source_vertex->normal)[1];
																(vertex->normal)[2]=(source_vertex->normal)[2];
																vertex->n_triangle_ptrs=source_vertex->n_triangle_ptrs;
																/* set the following once the triangles are copied */
																for (m=0;m < vertex->n_triangle_ptrs;m++)
																{
																	(vertex->triangle_ptrs)[m]=NULL;
																}
															}
															else
															{
																return_code=0;
																if (vertex)
																{
																	DEALLOCATE(vertex);
																}
																/* only the first i vertices need cleaning up */
																iso_surface->n_vertices=i;
															}
														}
													}
													else
													{
														return_code=0;
													}
													if (!return_code)
													{
														display_message(ERROR_MESSAGE,
															"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  "
															"Could not copy iso_surface compiled vertex list");
													}
												}
												else
												{
													iso_surface->compiled_vertex_list=(struct MC_vertex **)NULL;
													iso_surface->n_vertices=0;
												}

												/* 3. Copy the compiled triangle list, if any */
												if (return_code&&(0<source_iso_surface->n_triangles)&&
													(source_compiled_triangle=
														source_iso_surface->compiled_triangle_list))
												{
													iso_surface->n_triangles=source_iso_surface->n_triangles;
													if (ALLOCATE(iso_surface->compiled_triangle_list,
														struct MC_triangle *,iso_surface->n_triangles))
													{
														compiled_triangle=iso_surface->compiled_triangle_list;
														for (i=0;return_code&&(i<iso_surface->n_triangles);i++)
														{
															source_triangle= *source_compiled_triangle;
															source_compiled_triangle++;
															/* create a copy of the source triangle */
															if (ALLOCATE(triangle,struct MC_triangle,1))
															{
																*compiled_triangle=triangle;
																compiled_triangle++;
																triangle->triangle_index=source_triangle->triangle_index;
																for (j=0;j<3;j++)
																{
																	(triangle->vertex_index)[j]=
																		(source_triangle->vertex_index)[j];
																	(triangle->vertices)[j]=
																		(iso_surface->compiled_vertex_list)
																		[(triangle->vertex_index)[j]];
																	for (k=0;k<3;k++)
																	{
																		(triangle->texture_coord)[j][k]=
																			(source_triangle->texture_coord)[j][k];
																		(triangle->iso_poly_cop)[j][k]=
																			(source_triangle->iso_poly_cop)[j][k];
																	}
																	if ((source_triangle->material)[j])
																	{
																		(triangle->material)[j]=
																			ACCESS(Graphical_material)(
																				(source_triangle->material)[j]);
																	}
																	else
																	{
																		(triangle->material)[j]=
																			(struct Graphical_material *)NULL;
																	}
																	(triangle->env_map_index)[j]=
																		(source_triangle->env_map_index)[j];
																	if (env_map=(source_triangle->env_map)[j])
																	{
																		ACCESS(Environment_map)(env_map);
																	}
																	(triangle->env_map)[j]=env_map;
																}
																triangle->triangle_list_index=
																	source_triangle->triangle_list_index;
																/* set the following once cells are copied */
																triangle->cell_ptr=(struct MC_cell *)NULL;
															}
															else
															{
																return_code=0;
																/* only the first i triangles need cleaning up */
																iso_surface->n_triangles=i;
															}
														}
													}
													else
													{
														return_code=0;
													}
													if (!return_code)
													{
														display_message(ERROR_MESSAGE,
															"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  "
															"Could not copy iso_surface compiled triangle list");
													}
												}
												else
												{
													iso_surface->compiled_triangle_list=(struct MC_triangle **)NULL;
													iso_surface->n_triangles=0;
												}

												/* 4. Get vertex->triangle_ptrs to point at correct triangles */
												if (return_code&&
													(compiled_vertex=iso_surface->compiled_vertex_list)&&
													(source_compiled_vertex=source_iso_surface->compiled_vertex_list))
												{
													for (i=0;return_code&&(i<iso_surface->n_vertices);i++)
													{
														vertex= *compiled_vertex;
														compiled_vertex++;
														source_vertex= *source_compiled_vertex;
														source_compiled_vertex++;
														for (m=0;m < vertex->n_triangle_ptrs;m++)
														{
															(vertex->triangle_ptrs)[m]=
																(iso_surface->compiled_triangle_list)
																	[((source_vertex->triangle_ptrs)[m])->triangle_index];
														}
													}
												}

												/* 5. Copy the marching cubes cells */
												if (return_code&&(source_cells=source_iso_surface->mc_cells))
												{
													n_mc_cells=
														((iso_surface->dimension)[0]+2)*
														((iso_surface->dimension)[1]+2)*
														((iso_surface->dimension)[2]+2);
													if (ALLOCATE(iso_surface->mc_cells,
														struct MC_cell *,n_mc_cells))
													{
														cells=iso_surface->mc_cells;
														n_triangle_lists=(iso_surface->n_scalar_fields)+6;
														for (i=0;return_code&&(i<n_mc_cells);i++)
														{
															source_cell = *source_cells;
															source_cells++;
															if (source_cell)
															{
																if (ALLOCATE(cell,struct MC_cell,1)&&
																	(ALLOCATE(cell->n_triangles,int,n_triangle_lists))&&
																	ALLOCATE(cell->triangle_list,
																		struct MC_triangle **,n_triangle_lists))
																{
																	(cell->index)[0]=(source_cell->index)[0];
																	(cell->index)[1]=(source_cell->index)[1];
																	(cell->index)[2]=(source_cell->index)[2];
																	for (j=0;return_code&&(j<n_triangle_lists);j++)
																	{
																		maxk=(cell->n_triangles)[j]=
																			(source_cell->n_triangles)[j];
																		if ((source_cell->triangle_list)[j])
																		{
																			if (ALLOCATE((cell->triangle_list)[j],
																				struct MC_triangle *,maxk))
																			{
																				triangle_ptr=(cell->triangle_list)[j];
																				source_triangle_ptr=(source_cell->triangle_list)[j];
																				for (k=0;return_code&&(k<maxk);k++)
																				{
																					if (iso_surface->compiled_triangle_list&&
																						(source_triangle = *source_triangle_ptr)&&
																						(triangle=(iso_surface->compiled_triangle_list)
																							[source_triangle->triangle_index]))
																					{
																						source_triangle_ptr++;
																						/* get triangle to point back at cell */
																						triangle->cell_ptr = cell;
																						*triangle_ptr = triangle;
																						triangle_ptr++;
																					}
																					else
																					{
																						DEALLOCATE((cell->triangle_list)[j]);
																						return_code=0;
																					}
																				}
																			}
																			else
																			{
																				return_code=0;
																			}
																			if (!return_code)
																			{
																				/* clean up triangle lists allocated up to now */
																				for (k=0;k<j;k++)
																				{
																					if ((cell->triangle_list)[k])
																					{
																						DEALLOCATE((cell->triangle_list)[k]);
																					}
																				}
																			}
																		}
																		else
																		{
																			(cell->triangle_list)[j]=
																				(struct MC_triangle **)NULL;
																			(cell->n_triangles)[j]=0;
																		}
																	}
																}
																else
																{
																	return_code=0;
																}
																if (!return_code)
																{
																	if (cell)
																	{
																		if (cell->n_triangles)
																		{
																			if (cell->triangle_list)
																			{
																				DEALLOCATE(cell->triangle_list);
																			}
																			DEALLOCATE(cell->n_triangles);
																		}
																		DEALLOCATE(cell);
																	}
																	return_code=0;
																}
															}
															else /* if (source_cell) */
															{
																cell=(struct MC_cell *)NULL;
															}
															*cells = cell;
															cells++;
															if (!return_code)
															{
																/* error; only the first i cells need cleaning up */
																n_mc_cells=i;
															}
														}
													}
													else
													{
														return_code=0;
													}
													if (!return_code)
													{
														display_message(ERROR_MESSAGE,
															"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  "
															"Could not copy iso_surface cell list");
													}
												}
												else
												{
													iso_surface->mc_cells=(struct MC_cell **)NULL;
													n_mc_cells=0;
												}

												/* 6. Copy detail map - for variable cell triangulation */
												if (return_code&&source_iso_surface->detail_map)
												{
													/* n_mc_cells already set above if return_code not 0 */
													if (ALLOCATE(iso_surface->detail_map,int,n_mc_cells))
													{
														destination_int=iso_surface->detail_map;
														source_int=source_iso_surface->detail_map;
														for (i=0;i<n_mc_cells;i++)
														{
															*destination_int= *source_int;
															destination_int++;
															source_int++;
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  "
															"Insufficient memory for detail map");
														return_code=0;
													}
												}
												else
												{
													iso_surface->detail_map=(int *)NULL;
												}
#if defined (DEBUG)
												/*???debug*/
												printf("Vertices:\n");
												for (i=0;i<iso_surface->n_vertices;i++)
												{
													vertex=iso_surface->compiled_vertex_list[i];
													source_vertex=source_iso_surface->compiled_vertex_list[i];
													if ((vertex->coord[0] != source_vertex->coord[0])||
														(vertex->coord[1] != source_vertex->coord[1])||
														(vertex->coord[2] != source_vertex->coord[2]))
													{
														printf("vertex %i (%i/%i) coords do not match\n",i,
															vertex->vertex_index,source_vertex->vertex_index);
													}
												}
												printf("Triangles:\n");
												for (i=0;i<iso_surface->n_triangles;i++)
												{
													triangle=iso_surface->compiled_triangle_list[i];
													source_triangle=source_iso_surface->compiled_triangle_list[i];
													if (
														((triangle->vertices[0])->vertex_index !=
															(source_triangle->vertices[0])->vertex_index)||
														((triangle->vertices[1])->vertex_index !=
															(source_triangle->vertices[1])->vertex_index)||
														((triangle->vertices[2])->vertex_index !=
															(source_triangle->vertices[2])->vertex_index))
													{
														printf("  v %i: %i / %i\n",i,source_triangle->triangle_index,
															triangle->triangle_index);
													}
													cell=triangle->cell_ptr;
													source_cell=source_triangle->cell_ptr;
													if (cell&&source_cell)
													{
														if (
															(cell->index[0] != source_cell->index[0])||
															(cell->index[1] != source_cell->index[1])||
															(cell->index[2] != source_cell->index[2]))
														{
															printf("  cell %i indices differ: %d %d %d / %d %d %d\n",i,
																cell->index[0],cell->index[1],cell->index[2],
																source_cell->index[0],
																source_cell->index[1],
																source_cell->index[2]);
														}
													}
													else
													{
														printf("  cell %i NULL! %p %p",i,cell,source_cell);
													}
												}
#endif /* defined (DEBUG) */

												/* 7. Clean up iso_surface in case of error */
												if (!return_code)
												{
													/* detail map */
													if (iso_surface->detail_map)
													{
														DEALLOCATE(iso_surface->detail_map);
													}
													/* cells */
													if (cells=iso_surface->mc_cells)
													{
														n_triangle_lists=(iso_surface->n_scalar_fields)+6;
														for (i=0;i<n_mc_cells;i++)
														{
															if (cell= *cells)
															{
																for (j=0;j<n_triangle_lists;j++)
																{
																	if ((cell->triangle_list)[j])
																	{
																		DEALLOCATE((cell->triangle_list)[j]);
																	}
																}
																DEALLOCATE(cell->n_triangles);
																DEALLOCATE(cell->triangle_list);
																DEALLOCATE(*cells);
															}
															cells++;
														}
														DEALLOCATE(iso_surface->mc_cells);
													}
													/* triangles */
													if (compiled_triangle=iso_surface->compiled_triangle_list)
													{
														for (i=0;i<iso_surface->n_triangles;i++)
														{
															/* deaccess materials and environment maps */
															for (j=0;j<3;j++)
															{
																DEACCESS(Graphical_material)(
																	&(((*compiled_triangle)->material)[j]));
																DEACCESS(Environment_map)(
																	&(((*compiled_triangle)->env_map)[j]));
															}
															DEALLOCATE(*compiled_triangle);
															compiled_triangle++;
														}
														DEALLOCATE(iso_surface->compiled_triangle_list);
													}
													/* vertices */
													if (compiled_vertex=iso_surface->compiled_vertex_list)
													{
														for (i=0;i<iso_surface->n_vertices;i++)
														{
															DEALLOCATE((*compiled_vertex)->triangle_ptrs);
															DEALLOCATE(*compiled_vertex);
															compiled_vertex++;
														}
														DEALLOCATE(iso_surface->compiled_vertex_list);
													}
													/* deformable vertices */
													if (iso_surface->deform)
													{
														DEALLOCATE(iso_surface->deform);
													}
												}
											}
											else
											{
												return_code=0;
											}
											if (!return_code)
											{
												display_message(ERROR_MESSAGE,
													"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  "
													"Could not copy iso_surface");
											}
										}
										else
										{
											iso_surface=(struct MC_iso_surface *)NULL;
										}

#if defined (OLD_CODE)
/*???DB.  Needs to be replaced by new iso surface structure */
										/* iso surface */
										if (source_iso_surface=source->iso_surface)
										{
											if (ALLOCATE(iso_surface,struct VT_iso_surface,1))
											{
												(iso_surface->dimension)[0]=
													(source_iso_surface->dimension)[0];
												(iso_surface->dimension)[1]=
													(source_iso_surface->dimension)[1];
												(iso_surface->dimension)[2]=
													(source_iso_surface->dimension)[2];
												iso_surface->n_vertices=source_iso_surface->n_vertices;
												iso_surface->n_iso_polys=
													source_iso_surface->n_iso_polys;
												(iso_surface->xi_face_poly_index)[0]=
													(source_iso_surface->xi_face_poly_index)[0];
												(iso_surface->xi_face_poly_index)[1]=
													(source_iso_surface->xi_face_poly_index)[1];
												(iso_surface->xi_face_poly_index)[2]=
													(source_iso_surface->xi_face_poly_index)[2];
												(iso_surface->xi_face_poly_index)[3]=
													(source_iso_surface->xi_face_poly_index)[3];
												(iso_surface->xi_face_poly_index)[4]=
													(source_iso_surface->xi_face_poly_index)[4];
												(iso_surface->xi_face_poly_index)[5]=
													(source_iso_surface->xi_face_poly_index)[5];
												for (i=source_iso_surface->n_iso_polys-1;i>=0;i--)
												{
													source_int=(source_iso_surface->triangle_list)[i];
													destination_int=(iso_surface->triangle_list)[i];
													destination_int[0]=source_int[0];
													destination_int[1]=source_int[1];
													destination_int[2]=source_int[2];
												}
												source_iso_vertex=source_iso_surface->vertex_list;
												destination_iso_vertex=iso_surface->vertex_list;
												for (i=source_iso_surface->n_vertices;i>0;i--)
												{
													(destination_iso_vertex->coord)[0]=
														(source_iso_vertex->coord)[0];
													(destination_iso_vertex->coord)[1]=
														(source_iso_vertex->coord)[1];
													(destination_iso_vertex->coord)[2]=
														(source_iso_vertex->coord)[2];
													(destination_iso_vertex->normal)[0]=
														(source_iso_vertex->normal)[0];
													(destination_iso_vertex->normal)[1]=
														(source_iso_vertex->normal)[1];
													(destination_iso_vertex->normal)[2]=
														(source_iso_vertex->normal)[2];
													destination_iso_vertex->n_ptrs=
														source_iso_vertex->n_ptrs;
													source_int=source_iso_vertex->ptrs;
													destination_int=destination_iso_vertex->ptrs;
													for (j=source_iso_vertex->n_ptrs;j>0;i--)
													{
														*destination_int= *source_int;
														destination_int++;
														source_int++;
													}
													destination_iso_vertex->class=
														source_iso_vertex->class;
													destination_iso_vertex->scalar=
														source_iso_vertex->scalar;
													source_iso_vertex++;
													destination_iso_vertex++;
												}
												source_int=source_iso_surface->cell_index;
												destination_int=iso_surface->cell_index;
												for (i=source_iso_surface->n_iso_polys;i>0;i--)
												{
													*destination_int= *source_int;
													destination_int++;
													source_int++;
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for iso surface");
												return_code=0;
											}
										}
#endif /* defined (OLD_CODE) */
										if (return_code)
										{
											if (grid_spacing=source->grid_spacing)
											{
												i=(source->dimension)[0]+(source->dimension)[1]+
													(source->dimension)[2]+3;
												if (ALLOCATE(grid_spacing,double,i))
												{
													source_double=source->grid_spacing;
													destination_double=grid_spacing;
													while (i>0)
													{
														*destination_double= *source_double;
														destination_double++;
														source_double++;
														i--;
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Insufficient memory for grid spacing");
													return_code=0;
												}
											}
											if (return_code)
											{
												/* copy values */
												/*???RC Was not clearing existing fields of destination.
													Now cleaning it up as we go. This is begging for
													destroy functions to do this. */
												/* file_name */
												DEALLOCATE(destination->file_name);
												destination->file_name=file_name;

												/* texture_curve_list */
												if (destination->texture_curve_list)
												{
													/*printf("destroy texture_curve_list\n");*/
													source_curve= *(destination->texture_curve_list);
													while (destination_curve=source_curve)
													{
														source_curve=source_curve->ptrnext;
														DEALLOCATE(destination_curve);
													}
													DEALLOCATE(destination->texture_curve_list);
												}
												destination->texture_curve_list=texture_curve_list;

												/* texture_cell_list */
												if (destination->texture_cell_list)
												{
													/*printf("destroy texture_cell_list\n");*/
													destination_texture_cell=destination->texture_cell_list;
													/* get number of cells in original destination */
													n_cells=(destination->dimension[0])*
														(destination->dimension[1])*
														(destination->dimension[2]);
													i=n_cells;
													while (i>0)
													{
														if (*destination_texture_cell)
														{
															if ((*destination_texture_cell)->material)
															{
																DEACCESS(Graphical_material)(
																	&((*destination_texture_cell)->material));
															}
															if ((*destination_texture_cell)->env_map)
															{
																DEACCESS(Environment_map)(
																	&((*destination_texture_cell)->env_map));
															}
														}
														destination_texture_cell++;
														i--;
													}
													DEALLOCATE(*destination->texture_cell_list);
													DEALLOCATE(destination->texture_cell_list);
												}
												destination->texture_cell_list=texture_cell_list;

												/* texture_node_list */
												if (destination->global_texture_node_list)
												{
													/*printf("destroy texture_node_list\n");*/
													destination_node=destination->global_texture_node_list;
													/* get number of nodes in original destination */
													n_nodes=(destination->dimension[0]+1)*
														(destination->dimension[1]+1)*
														(destination->dimension[2]+1);
													i=n_nodes;
													while (i>0)
													{
														if (*destination_node)
														{
															if ((*destination_node)->material)
															{
																DEACCESS(Graphical_material)(
																	&((*destination_node)->material));
															}
															if ((*destination_node)->dominant_material)
															{
																DEACCESS(Graphical_material)(
																	&((*destination_node)->dominant_material));
															}
														}
														destination_node++;
														i--;
													}
													DEALLOCATE(*destination->global_texture_node_list);
													DEALLOCATE(destination->global_texture_node_list);
												}
												destination->global_texture_node_list=texture_node_list;

												/* scalar_field */
												if (destination->scalar_field)
												{
													/*printf("destroy scalar_field\n");*/
													DEALLOCATE(destination->scalar_field->scalar);
													DEALLOCATE(destination->scalar_field);
												}

												destination->scalar_field=scalar_field;
												/* clip_field */
												if (destination->clip_field)
												{
													/*printf("destroy clip_field\n");*/
													DEALLOCATE(destination->clip_field->scalar);
													DEALLOCATE(destination->clip_field);
												}
												destination->clip_field=clip_field;

												/* clip_field2 */
												if (destination->clip_field2)
												{
													/*printf("destroy clip_field2\n");*/
													DEALLOCATE(destination->clip_field2->scalar);
													DEALLOCATE(destination->clip_field2);
												}
												destination->clip_field2=clip_field2;

												/* coordinate_field */
												if (destination->coordinate_field)
												{
													/*printf("destroy coordinate_field\n");*/
													DEALLOCATE(destination->coordinate_field->vector);
													DEALLOCATE(destination->coordinate_field);
												}
												destination->coordinate_field=coordinate_field;

												/* mc_iso_surface */
												if (destination->mc_iso_surface)
												{
													/*printf("destroy mc_iso_surface\n");*/
													clean_mc_iso_surface(
														destination->mc_iso_surface->n_scalar_fields,
														destination->mc_iso_surface);
													DEALLOCATE(destination->mc_iso_surface);
												}
												destination->mc_iso_surface=iso_surface;

#if defined (OLD_CODE)
												if (destination->iso_surface=iso_surface)
												{
													for (i=3*(iso_surface->n_iso_polys)-1;i>0;i--)
													{
														(destination->iso_poly_material)[i]=
															ACCESS(Graphical_material)(
															&((source->iso_poly_material)[i][0]));
														(destination->iso_poly_cop)[i][0]=
															(source->iso_poly_cop)[i][0];
														(destination->iso_poly_cop)[i][1]=
															(source->iso_poly_cop)[i][1];
														(destination->iso_poly_cop)[i][2]=
															(source->iso_poly_cop)[i][2];
														(destination->texturemap_coord)[i][0]=
															(source->texturemap_coord)[i][0];
														(destination->texturemap_coord)[i][1]=
															(source->texturemap_coord)[i][1];
														(destination->texturemap_coord)[i][2]=
															(source->texturemap_coord)[i][2];
														(destination->texturemap_index)[i]=
															(source->texturemap_index)[i];
														(destination->iso_env_map)[i]=
															ACCESS(Environment_map)(
															&((source->iso_env_map)[i][0]));
													}
												}
#endif /* defined (OLD_CODE) */
												/*???RC these parameters copied after dynamic members
													deallocated, since they need information such as
													original destination->dimension */
												destination->index=source->index;
												(destination->ximin)[0]=(source->ximin)[0];
												(destination->ximin)[1]=(source->ximin)[1];
												(destination->ximin)[2]=(source->ximin)[2];
												(destination->ximax)[0]=(source->ximax)[0];
												(destination->ximax)[1]=(source->ximax)[1];
												(destination->ximax)[2]=(source->ximax)[2];
												(destination->dimension)[0]=(source->dimension)[0];
												(destination->dimension)[1]=(source->dimension)[1];
												(destination->dimension)[2]=(source->dimension)[2];
												destination->grid_spacing=grid_spacing;

												destination->isovalue=source->isovalue;
												destination->hollow_mode_on=source->hollow_mode_on;
												destination->hollow_isovalue=source->hollow_isovalue;
												destination->clipping_field_on=
													source->clipping_field_on;
												destination->cutting_plane_on=source->cutting_plane_on;
												destination->disable_volume_functions=
													source->disable_volume_functions;
												destination->cut_isovalue=source->cut_isovalue;
												(destination->cutting_plane)[0]=
													(source->cutting_plane)[0];
												(destination->cutting_plane)[1]=
													(source->cutting_plane)[1];
												(destination->cutting_plane)[2]=
													(source->cutting_plane)[2];
												(destination->cutting_plane)[3]=
													(source->cutting_plane)[3];
												destination->closed_surface=source->closed_surface;
												destination->decimation=source->decimation;
												destination->calculate_nodal_values=
													source->calculate_nodal_values;
												destination->recalculate=source->recalculate;
											}
										}
									} /* if (return_code) */
								}
							}
						}
					}
				}
			}
		}
		if (!return_code)
		{
			/* free allocated memory */
			DEALLOCATE(file_name);
			if (texture_curve_list)
			{
				source_curve= *texture_curve_list;
				while (destination_curve=source_curve)
				{
					source_curve=source_curve->ptrnext;
					DEALLOCATE(destination_curve);
				}
				DEALLOCATE(texture_curve_list);
			}
			if (texture_cell_list)
			{
				destination_texture_cell=texture_cell_list;
				i=n_cells;
				while (i>0)
				{
					if (*destination_texture_cell)
					{
						if ((*destination_texture_cell)->material)
						{
							DEACCESS(Graphical_material)(
								&((*destination_texture_cell)->material));
						}
						if ((*destination_texture_cell)->env_map)
						{
							DEACCESS(Environment_map)(
								&((*destination_texture_cell)->env_map));
						}
					}
					destination_texture_cell++;
					i--;
				}
				DEALLOCATE(*texture_cell_list);
				DEALLOCATE(texture_cell_list);
			}
			if (texture_node_list)
			{
				destination_node=texture_node_list;
				i=n_nodes;
				while (i>0)
				{
					if (*destination_node)
					{
						if ((*destination_node)->material)
						{
							DEACCESS(Graphical_material)(&((*destination_node)->material));
						}
						if ((*destination_node)->dominant_material)
						{
							DEACCESS(Graphical_material)(
								&((*destination_node)->dominant_material));
						}
					}
					destination_node++;
					i--;
				}
				DEALLOCATE(*texture_node_list);
				DEALLOCATE(texture_node_list);
			}
			if (scalar_field)
			{
				DEALLOCATE(scalar_field->scalar);
				DEALLOCATE(scalar_field);
			}
			if (clip_field)
			{
				DEALLOCATE(clip_field->scalar);
				DEALLOCATE(clip_field);
			}
			if (clip_field2)
			{
				DEALLOCATE(clip_field2->scalar);
				DEALLOCATE(clip_field2);
			}
			if (coordinate_field)
			{
				DEALLOCATE(coordinate_field->vector);
				DEALLOCATE(coordinate_field);
			}
			if (iso_surface)
			{
				if (iso_surface->mc_cells)
				{
					i=((iso_surface->dimension)[0]+2)*
						((iso_surface->dimension)[1]+2)*
						((iso_surface->dimension)[2]+2);
					cells=(iso_surface->mc_cells)+i;
					while (i<0)
					{
						cells--;
						if (cell= *cells)
						{
							j=n_triangle_lists;
							while (j>0)
							{
								j--;
								if ((cell->triangle_list)[j])
								{
									k=(cell->n_triangles)[j];
									triangle_ptr=
										((cell->triangle_list)[j])+k;
									while (k>0)
									{
										triangle_ptr--;
										triangle= *triangle_ptr;
										k--;
										DEACCESS(Graphical_material)(
											&((triangle->material)[0]));
										DEACCESS(Graphical_material)(
											&((triangle->material)[1]));
										DEACCESS(Graphical_material)(
											&((triangle->material)[2]));
										DEACCESS(Environment_map)(
											&((triangle->env_map)[0]));
										DEACCESS(Environment_map)(
											&((triangle->env_map)[1]));
										DEACCESS(Environment_map)(
											&((triangle->env_map)[2]));
										l=3;
										while (l>0)
										{
											l--;
											vertex=(triangle->vertices)[l];
											if (triangle==(vertex->
												triangle_ptrs)[0])
											{
												DEALLOCATE(vertex->
													triangle_ptrs);
												DEALLOCATE(vertex);
											}
										}
									}
								}
							}
							DEALLOCATE(cell->n_triangles);
							DEALLOCATE(cell->triangle_list);
							DEALLOCATE(*cells);
						}
						i++;
					}
					DEALLOCATE(iso_surface->compiled_vertex_list);
					DEALLOCATE(iso_surface->compiled_triangle_list);
					DEALLOCATE(iso_surface->mc_cells);
				}
				DEALLOCATE(iso_surface->detail_map);
				DEALLOCATE(iso_surface);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name).  Invalid argument(s)");
		return_code=0;
	}
/*???debug */
	/*printf("leave MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name) %d\n",
		return_code);*/
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(VT_volume_texture,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(VT_volume_texture,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(VT_volume_texture,name));
	/* check arguments */
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))

			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(VT_volume_texture,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(VT_volume_texture,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(VT_volume_texture,name) */

DECLARE_MANAGER_FUNCTIONS(VT_volume_texture)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(VT_volume_texture)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(VT_volume_texture,name,char *)

int modify_VT_volume_texture(struct Parse_state *state,void *texture_void,
	void *modify_VT_volume_texture_data_void)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Modifies the properties of a volume texture.
???DB.  Doesn't do help properly ?
==============================================================================*/
{
	auto struct Modifier_entry
		help_option_table[]=
		{
			{"VOLUME_TEXTURE_NAME",NULL,NULL,modify_VT_volume_texture},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"file",NULL,NULL,set_VT_volume_texture_file},
			{"iso_value",NULL,NULL,set_double},
			{"no_normalise",NULL,NULL,set_char_flag},
			{"offset_xi",NULL,NULL,set_double_vector},
			{"scale_xi",NULL,NULL,set_double_vector},
			{"xi_min",NULL,NULL,set_double_vector},
			{"xi_max",NULL,NULL,set_double_vector},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token, no_normalise;
	int number_of_components,process,return_code;
	struct Modify_VT_volume_texture_data *modify_VT_volume_texture_data;
	struct VT_volume_texture *volume_texture_to_be_modified,
		*volume_texture_to_be_modified_copy;

	ENTER(modify_VT_volume_texture);
	/* check the arguments */
	if (state)
	{
		if (modify_VT_volume_texture_data=(struct Modify_VT_volume_texture_data *)
			modify_VT_volume_texture_data_void)
		{
			if (current_token=state->current_token)
			{
				process=0;
				if (volume_texture_to_be_modified=
					(struct VT_volume_texture *)texture_void)
				{
					if (IS_MANAGED(VT_volume_texture)(volume_texture_to_be_modified,
						modify_VT_volume_texture_data->volume_texture_manager))
					{
						if (volume_texture_to_be_modified_copy=
							CREATE(VT_volume_texture)((char *)NULL))
						{
							MANAGER_COPY_WITH_IDENTIFIER(VT_volume_texture,name)(
								volume_texture_to_be_modified_copy,
								volume_texture_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"modify_Texture.  Could not create volume_texture copy");
							return_code=0;
						}
					}
					else
					{
						volume_texture_to_be_modified_copy=volume_texture_to_be_modified;
						volume_texture_to_be_modified=(struct VT_volume_texture *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (volume_texture_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							VT_volume_texture,name)(current_token,
							modify_VT_volume_texture_data->volume_texture_manager))
						{
							if (return_code=shift_Parse_state(state,1))
							{
								if (volume_texture_to_be_modified_copy=
									CREATE(VT_volume_texture)((char *)NULL))
								{
									MANAGER_COPY_WITH_IDENTIFIER(VT_volume_texture,name)(
										volume_texture_to_be_modified_copy,
										volume_texture_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"modify_Texture.  Could not create volume_texture copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown volume texture: %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						if (volume_texture_to_be_modified=
							CREATE(VT_volume_texture)((char *)NULL))
						{
							(help_option_table[0]).to_be_modified=
								(void *)volume_texture_to_be_modified;
							(help_option_table[0]).user_data=
								modify_VT_volume_texture_data_void;
							return_code=process_option(state,help_option_table);
							DESTROY(VT_volume_texture)(&volume_texture_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"modify_VT_volume_texture.  Could not create dummy texture");
							return_code=0;
						}
					}
				}
				if (process)
				{
					no_normalise = 0;
					number_of_components=3;
					(option_table[0]).to_be_modified=volume_texture_to_be_modified_copy;
					(option_table[0]).user_data=modify_VT_volume_texture_data_void;
					(option_table[1]).to_be_modified=
						&(volume_texture_to_be_modified_copy->isovalue);
					(option_table[2]).to_be_modified=&no_normalise;
 					(option_table[3]).to_be_modified=
 						&(volume_texture_to_be_modified_copy->offset_xi);
 					(option_table[3]).user_data=(void *)&number_of_components;
 					(option_table[4]).to_be_modified=
 						&(volume_texture_to_be_modified_copy->scale_xi);
 					(option_table[4]).user_data=(void *)&number_of_components;
					(option_table[5]).to_be_modified=
						&(volume_texture_to_be_modified_copy->ximin);
					(option_table[5]).user_data=(void *)&number_of_components;
					(option_table[6]).to_be_modified=
						&(volume_texture_to_be_modified_copy->ximax);
					(option_table[6]).user_data=(void *)&number_of_components;
					if (return_code=process_multiple_options(state,option_table))
					{
						if ((!no_normalise) &&
							volume_texture_to_be_modified_copy->disable_volume_functions)
						{
#if defined (DEBUG)
							/*???debug */
							/*???SAB.  This debug causes a crash on an O2 if the
								volume_texture to be modified is not managed as when it gets
								here the volume_texture_to_be_modified = NULL */
							printf("DEBUG: copy with identifier: copy = %p,  orig = %p\n",
								volume_texture_to_be_modified_copy->mc_iso_surface->deform,
								volume_texture_to_be_modified->mc_iso_surface->deform);
#endif /* defined (DEBUG) */
							if (!(normalize_obj(volume_texture_to_be_modified_copy)))
							{
								display_message(WARNING_MESSAGE,
									"modify_VT_volume_texture.  normalize_obj failed");
							}
						}
#if defined (DEBUG)
						/*???debug */
						printf(">>>>> ximax = %f %f %f\n",
							(volume_texture_to_be_modified_copy->ximax)[0],
							(volume_texture_to_be_modified_copy->ximax)[1],
							(volume_texture_to_be_modified_copy->ximax)[2]);
#endif /* defined (DEBUG) */
						if (volume_texture_to_be_modified)
						{
							/* make sure iso_surface recalculated after modify */
							/*???RC this flag should be set by individual commands
								for setting VT_volume texture parameters. These do not
								yet exist, though! */
							volume_texture_to_be_modified_copy->recalculate=1;
							MANAGER_MODIFY_NOT_IDENTIFIER(VT_volume_texture,name)(
								volume_texture_to_be_modified,
								volume_texture_to_be_modified_copy,
								modify_VT_volume_texture_data->volume_texture_manager);
							DESTROY(VT_volume_texture)(&volume_texture_to_be_modified_copy);
						}
					}
				}
			}
			else
			{
				if (texture_void)
				{
					display_message(WARNING_MESSAGE,
						"Missing volume texture modifications");
				}
				else
				{
					display_message(WARNING_MESSAGE,"Missing volume texture name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"modify_VT_volume_texture.  Missing modify_VT_volume_texture_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"modify_VT_volume_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_VT_volume_texture */

int list_VT_volume_texture(struct VT_volume_texture *texture)
/*******************************************************************************
LAST MODIFIED : 21 September 1998

DESCRIPTION :
Writes the properties of the <texture> to the command window.
==============================================================================*/
{
	char line[80];
	int return_code;

	ENTER(list_VT_volume_texture);
	/* check the arguments */
	if (texture)
	{
		return_code=1;
		/* write the name */
		display_message(INFORMATION_MESSAGE,"volume_texture : ");
		display_message(INFORMATION_MESSAGE,texture->name);
		display_message(INFORMATION_MESSAGE,"\n");
		/* write the name of the file */
		display_message(INFORMATION_MESSAGE,"  file name : ");
		if (texture->file_name)
		{
			display_message(INFORMATION_MESSAGE,texture->file_name);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		sprintf(line,"%.3g <= xi1 <= %.3g, %d divisions\n",(texture->ximin)[0],
			(texture->ximax)[0],(texture->dimension)[0]);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"%.3g <= xi2 <= %.3g, %d divisions\n",(texture->ximin)[1],
			(texture->ximax)[1],(texture->dimension)[1]);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"%.3g <= xi3 <= %.3g, %d divisions\n",(texture->ximin)[2],
			(texture->ximax)[2],(texture->dimension)[2]);
		display_message(INFORMATION_MESSAGE,line);
		display_message(INFORMATION_MESSAGE,"iso_surface %p\n", texture->mc_iso_surface);
		display_message(INFORMATION_MESSAGE,"access_count = %d\n",
			texture->access_count);
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_VT_volume_texture.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_VT_volume_texture */

int set_VT_volume_texture(struct Parse_state *state,void *texture_address_void,
	void *volume_texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the volume texture from a command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct VT_volume_texture *temp_texture,**texture_address;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;

	ENTER(set_VT_volume_texture);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((texture_address=
					(struct VT_volume_texture **)texture_address_void)&&
					(volume_texture_manager=(struct MANAGER(VT_volume_texture) *)
					volume_texture_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*texture_address)
						{
							DEACCESS(VT_volume_texture)(texture_address);
							*texture_address=(struct VT_volume_texture *)NULL;
						}
						return_code=1;
					}
					else
					{
						if (temp_texture=FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,
							name)(current_token,volume_texture_manager))
						{
							if (*texture_address!=temp_texture)
							{
								DEACCESS(VT_volume_texture)(texture_address);
								*texture_address=ACCESS(VT_volume_texture)(temp_texture);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Volume texture does not exist: %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_VT_volume_texture.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," VTEXTURE_NAME|none");
				if (texture_address=(struct VT_volume_texture **)texture_address_void)
				{
					if (temp_texture= *texture_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_texture->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing environment map name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_VT_volume_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_VT_volume_texture */

int read_volume_texture_from_file(struct VT_volume_texture *texture,
	FILE *in_file,struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Environment_map) *environment_map_manager)
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Reads the volume <texture> from the <in_file>.
???DB.  Memory deallocation on failure needs tidying
minimum_xi1_value minimum_xi2_value minimum_xi3_value
maximum_xi1_value maximum_xi2_value maximum_xi3_value
#cells_in_xi1 #cells_in_xi2 #cells_in_xi3
#materials
material_names  !#materials material names
material_to_node_assignment  !(#cells_in_xi1+1)*(#cells_in_xi2+1)*
														!(#cells_in_xi3+1) indices, suggest that all 1
scalar_value_to_cell_assignment  !#cells_in_xi1*#cells_in_xi2*#cells_in_xi3
																! scalar values
iso_value  !a number
nodal_values  !an optional keyword
scalar_value_to_node_assignment  !(#cells_in_xi1+1)*(#cells_in_xi2+1)*
																!(#cells_in_xi3+1) scalar values.  Read if
																!nodal_values keyword is present.  Means that
																!the cell values are not used
!there are other options that can follow, but ignore for present

Example
0 0 0
1 1 1
3 3 3
1
default
1 1 1
1 1 1
1 1 1
1 1 1
1 1 1
1 1 1
1 1 1
1 1 1
1 1 1
0 0 0
0 0 0
0 0 0
0 0 0
0 0 0
0 0 0
0 0 0
0 0 0
0 0 0
5.5
nodal_values
0 1 2 3
2 3 4 5
4 5 6 7
6 7 8 9
0 1 2 3
2 3 4 5
4 5 6 7
6 7 8 9
0 1 2 3
2 3 4 5
4 5 6 7
6 7 8 9
0 1 2 3
2 3 4 5
4 5 6 7
6 7 8 9
==============================================================================*/
{
	char *temp_string;
	int dimension,i,ii,j,index,n,n_cells,n_nodes,node_index,n_slits,
		number_of_materials,number_of_scalar_fields,return_code,*vt_group_nodes;
	struct Graphical_material **index_to_material,*material;
	int number_of_env_maps;
	struct Environment_map **index_to_env_map,*env_map;
	struct VT_node_group **node_groups,*node_group;
	struct VT_texture_cell *cell,*cell_block,**cell_list;
	struct VT_texture_curve *curve,*curve_next;
	struct VT_texture_node *node,*node_block,**node_list;

	ENTER(read_volume_texture_from_file);
#if defined (DEBUG)
/*???debug */
printf("enter read_volume_texture_from_file\n");
#endif /* defined (DEBUG) */
	/* check arguments */
	if (in_file&&texture&&graphical_material_manager)
	{
		return_code=1;
		/* set modes */
		texture->hollow_mode_on=0;
		texture->closed_surface=0;
		texture->cutting_plane_on=0;
		texture->disable_volume_functions=0;
		/* deallocate curves */
		if (texture->texture_curve_list)
		{
			curve= *(texture->texture_curve_list);
			while (curve)
			{
				curve_next=curve->ptrnext;
				DEALLOCATE(curve);
				curve=curve_next;
			}
			*(texture->texture_curve_list)= NULL;
		}
#if defined (DEBUG)
/*???debug */
printf("deallocated curves\n");
#endif /* defined (DEBUG) */
		/* deallocate cells */
		if (cell_list=texture->texture_cell_list)
		{
			n_cells=(texture->dimension[0])*(texture->dimension[1])*
				(texture->dimension[2]);
			for (i=n_cells;i>0;i--)
			{
				DEACCESS(Graphical_material)(&((*cell_list)->material));
#if defined (OLD_CODE)
				if ((*cell_list)->env_map)
				{
					for (j=0;j<6;j++)
					{
						if ((*cell_list)->env_map->face_material[j])
						{
							DEACCESS(Graphical_material)(
								&((*cell_list)->env_map->face_material[j]));
						}
					}
				}
#endif /* defined (OLD_CODE) */
				DEACCESS(Environment_map)(&(*cell_list)->env_map);
				cell_list++;
			}
			DEALLOCATE(*texture->texture_cell_list);
			DEALLOCATE(texture->texture_cell_list);
		}
#if defined (DEBUG)
/*???debug */
printf("deallocated cells\n");
#endif /* defined (DEBUG) */
		/* deallocate nodes */
		if (node_list=texture->global_texture_node_list)
		{
			n_nodes=(texture->dimension[0]+1)*(texture->dimension[1]+1)*
				(texture->dimension[2]+1);
			for (i=n_nodes;i>0;i--)
			{
				DEACCESS(Graphical_material)(&((*node_list)->material));
				DEACCESS(Graphical_material)(&((*node_list)->dominant_material));
				node_list++;
			}
			DEALLOCATE(*texture->global_texture_node_list);
			DEALLOCATE(texture->global_texture_node_list);
		}
#if defined (DEBUG)
/*???debug */
printf("deallocated nodes\n");
#endif /* defined (DEBUG) */
		if (texture->node_groups)
		{
			for (i=texture->n_groups;i>0;i--)
			{
				node_group=texture->node_groups[i];
				DEALLOCATE(node_group->nodes);
				DEALLOCATE(node_group);
			}
			DEALLOCATE(texture->node_groups);
		}
		texture->n_groups=0;
#if defined (DEBUG)
/*???debug */
printf("deallocated texture->node_groups\n");
#endif /* defined (DEBUG) */
		if (texture->mc_iso_surface)
		{
			number_of_scalar_fields=0;
			if (texture->scalar_field)
			{
				number_of_scalar_fields++;
			}
			if (texture->clip_field)
			{
/*???SAB.						number_of_scalar_fields++;*/

			}
#if defined (OLD_CODE)
/*???DB.  Doesn't seem to create a triangle list for the clip_field2 ? */
			if (texture->clip_field2)
			{
				number_of_scalar_fields++;
			}
#endif /* defined (OLD_CODE) */
#if defined (DEBUG)
/*???debug */
printf("before clean_mc_iso_surface %d %p\n",number_of_scalar_fields,
	texture->mc_iso_surface);
#endif /* defined (DEBUG) */
			clean_mc_iso_surface(number_of_scalar_fields,texture->mc_iso_surface);
#if defined (DEBUG)
/*???debug */
printf("after clean_mc_iso_surface\n");
#endif /* defined (DEBUG) */
			DEALLOCATE(texture->mc_iso_surface);
		}
#if defined (DEBUG)
/*???debug */
printf("deallocated texture->mc_iso_surface\n");
#endif /* defined (DEBUG) */
		if (texture->scalar_field)
		{
			DEALLOCATE(texture->scalar_field->scalar);
		}
		if (texture->clip_field)
		{
			DEALLOCATE(texture->clip_field->scalar);
		}
		if (texture->clip_field2)
		{
			DEALLOCATE(texture->clip_field2->scalar);
		}
		if (texture->coordinate_field)
		{
			DEALLOCATE(texture->coordinate_field->vector);
		}
		if (texture->grid_spacing)
		{
			DEALLOCATE(texture->grid_spacing);
		}
#if defined (DEBUG)
/*???debug */
printf("deallocated\n");
printf("%p\n",in_file);
#endif /* defined (DEBUG) */
		/* read in xi ranges */
		for (i=0;i<3;i++)
		{
			fscanf(in_file,"%lf",&((texture->ximin)[i]));
#if defined (DEBUG)
/*???debug */
printf("%g\n",(texture->ximin)[i]);
#endif /* defined (DEBUG) */
		}
		for (i=0;i<3;i++)
		{
			fscanf(in_file,"%lf",&(texture->ximax[i]));
#if defined (DEBUG)
/*???debug */
printf("%g\n",(texture->ximax)[i]);
#endif /* defined (DEBUG) */
		}
#if defined (DEBUG)
/*???debug */
printf("read ranges\n");
#endif /* defined (DEBUG) */
		/* read in discretization */
		n_cells=1;
		n_nodes=1;
		for (i=0;i<3;i++)
		{
			fscanf(in_file,"%d",&dimension);
			texture->dimension[i]=dimension;
			texture->scalar_field->dimension[i]=dimension;
			texture->clip_field->dimension[i]=dimension;
			texture->clip_field2->dimension[i]=dimension;
			texture->coordinate_field->dimension[i]=dimension;
			texture->recalculate=1;
			n_cells *= dimension;
			n_nodes *= dimension+1;
		}
#if defined (DEBUG)
/*???debug */
printf("read discretization\n");
#endif /* defined (DEBUG) */
		/* read in the table for translating from file indicies to materials */
		fscanf(in_file,"%d",&number_of_materials);
#if defined (DEBUG)
/*???debug */
printf("number_of_materials=%d\n",number_of_materials);
#endif /* defined (DEBUG) */
		if ((0<number_of_materials)&&ALLOCATE(index_to_material,
			struct Graphical_material *,number_of_materials))
		{
			i=0;
			while ((i<number_of_materials)&&file_read_Graphical_material_name(in_file,
				&material,graphical_material_manager))
			{
				index_to_material[i]=material;
				i++;
			}
			if (i==number_of_materials)
			{
				if (ALLOCATE(texture->scalar_field->scalar,double,n_nodes)&&
					ALLOCATE(texture->clip_field->scalar,double,n_nodes)&&
					ALLOCATE(texture->clip_field2->scalar,double,n_nodes)&&
					ALLOCATE(texture->coordinate_field->vector,double,3*n_nodes)&&
					ALLOCATE(texture->mc_iso_surface,struct MC_iso_surface,1)&&
					ALLOCATE(cell_list,struct VT_texture_cell *,n_cells)&&
					ALLOCATE(node_list,struct VT_texture_node *,n_nodes)&&
					ALLOCATE(cell_block,struct VT_texture_cell,n_cells)&&
					ALLOCATE(node_block,struct VT_texture_node,n_nodes))
				{
					texture->mc_iso_surface->n_scalar_fields=0;
					texture->mc_iso_surface->n_triangles=0;
					texture->mc_iso_surface->n_vertices=0;
					texture->mc_iso_surface->compiled_vertex_list=NULL;
					texture->mc_iso_surface->compiled_triangle_list=NULL;
					texture->mc_iso_surface->deform=NULL;
					for (ii=0;ii<3;ii++)
					{
						texture->mc_iso_surface->dimension[ii]=texture->dimension[ii];
						texture->mc_iso_surface->active_block[ii*2]=1;
						texture->mc_iso_surface->active_block[ii*2+1]=
							texture->mc_iso_surface->dimension[ii];
					}
/*???MS.  WHAT IS THIS??? commented out as exceeds bounds active_block[6] */
/*					texture->mc_iso_surface->active_block[ii*3]=0;*/
					/*???RC Why +3 here? Other code uses +2! */
					if (ALLOCATE(texture->mc_iso_surface->mc_cells,struct MC_cell *,
						(texture->dimension[0]+3)*(texture->dimension[1]+3)*
						(texture->dimension[2]+3)))
					{
						for (ii=0;ii<(texture->dimension[0]+3)*(texture->dimension[1]+3)*
							(texture->dimension[2]+3);ii++)
						{
							texture->mc_iso_surface->mc_cells[ii] = NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_volume_texture_from_file.  Error in allocating mc_cells");
					}
#if defined (DEBUG)
/*???debug */
printf("alloc mc->detail_map %d\n",(texture->dimension[0]+3)*
	(texture->dimension[1]+3)*(texture->dimension[2]+3));
#endif /* defined (DEBUG) */
					if (ALLOCATE(texture->mc_iso_surface->detail_map,int,
						(texture->dimension[0]+3)*(texture->dimension[1]+3)*
						(texture->dimension[2]+3)))
					{
#if defined (DEBUG)
/*???debug */
printf("det_map = %p\n", texture->mc_iso_surface->detail_map);
#endif /* defined (DEBUG) */
						for (ii=0;ii<(texture->dimension[0]+3)*(texture->dimension[1]+3)*
							(texture->dimension[2]+3);ii++)
						{
							texture->mc_iso_surface->detail_map[ii]=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_volume_texture_from_file.  Error in allocating detail_map");
					}
					texture->texture_cell_list=cell_list;
					texture->global_texture_node_list=node_list;
					i=0;
					while ((i<n_cells)&&
						(1==fscanf(in_file,"%d",&index)))
					{
						cell=cell_block+i;
						cell_list[i]=cell;
						/* tempoarily set cop */
						(cell->cop)[0]=0.5;
						(cell->cop)[1]=0.5;
						(cell->cop)[2]=0.5;
						if (0<index)
						{
							if (index_to_material[index-1])
							{
								cell->material=
									ACCESS(Graphical_material)(index_to_material[index-1]);
							}
							else
							{
								cell->material=(struct Graphical_material *)NULL;
							}
							cell->scalar_value=1.0;
						}
						else
						{
							cell->material=(struct Graphical_material *)NULL;
							cell->scalar_value=0.0;
						}
						cell->detail=0;
						cell->env_map=(struct Environment_map *)NULL;
						i++;
					}
					if (i==n_cells)
					{
						/*???DB.  Added option for reading in nodal scalars rather than
							calculating from cell values */
						texture->calculate_nodal_values=1;
						/* read in the cell scalar values */
						for (i=0;i<n_cells;i++)
						{
							fscanf(in_file,"%lf",&((texture->texture_cell_list)[i]->
								scalar_value));
						}
						i=0;
						while ((i<n_nodes)&&
							(1==fscanf(in_file,"%d",&index)))
						{
							node=node_block+i;
							node_list[i]=node;
							node->active=0;
							node->node_type=0;
							for (j=0;j<8;j++)
							{
								node->cm_node_identifier[j]=i;
							}
							/* temporarily set cop values */
							node->cop[0]=node->cop[1]=node->cop[2]=0.5;
							if ((0<index)&&index_to_material[index-1])
							{
								node->material=
									ACCESS(Graphical_material)(index_to_material[index-1]);
							}
							else
							{
								node->material=(struct Graphical_material *)NULL;
							}

							node->dominant_material=(struct Graphical_material *)NULL;

							if (node->material)
							{
								node->scalar_value=1.0;
							}
							else
							{
								node->scalar_value=0.0;
							}
							i++;
						}
						if (i==n_nodes)
						{
							/* the isovalue */
							fscanf(in_file,"%lf",&(texture->isovalue));
#if defined (DEBUG)
/*???debug */
printf("isovalue = %lf\n",texture->isovalue);
#endif /* defined (DEBUG) */
							/*???Mark.  Temp until put in properly */
							texture->hollow_isovalue=0.75*texture->isovalue;
							texture->hollow_mode_on=0;
							texture->cutting_plane_on=0;
							texture->cut_isovalue=0;
							texture->cutting_plane[0]=0;
							texture->cutting_plane[1]=0;
							texture->cutting_plane[2]=0;
							texture->cutting_plane[3]=0;
							texture->closed_surface=0;
							texture->decimation=0;
							/* load any curves if present */
							temp_string = (char *)NULL;
							while (read_string(in_file,"s",&temp_string)&&!feof(in_file))
							{
#if defined (DEBUG)
/*???debug */
printf("read string: %s\n",temp_string);
#endif /* defined (DEBUG) */
								if (0==strcmp(temp_string,"Environment_maps"))
								{
									/* read in the table for translating from file indicies to
										materials */
									fscanf(in_file,"%d",&number_of_env_maps);
#if defined (DEBUG)
/*???debug */
printf("number_of_env_maps=%d\n",number_of_env_maps);
#endif /* defined (DEBUG) */
									if ((0<number_of_env_maps)&&ALLOCATE(index_to_env_map,
										struct Environment_map *,number_of_env_maps))
									{
										i=0;
										while ((i<number_of_env_maps)&&
											file_read_Environment_map_name(in_file,&env_map,
											environment_map_manager))
										{
											index_to_env_map[i]=env_map;
											i++;
										}
										if (i==number_of_env_maps)
										{
											i=0;
											while ((i<n_cells)&&(1==fscanf(in_file,"%d",&index)))
											{
												if (index>0)
												{
													(texture->texture_cell_list)[i]->env_map=
													ACCESS(Environment_map)(index_to_env_map[index-1]);
												}
												else
												{
													(texture->texture_cell_list)[i]->env_map=
														(struct Environment_map *)NULL;

												}
												i++;
											}
										}
										DEALLOCATE(index_to_env_map);
									}
								}
								if (0==strcmp(temp_string,"Projection_centres"))
								{
									for(i=0;i<n_cells;i++)
									{
										fscanf(in_file,"%lf %lf %lf",&(cell_list[i]->cop[0]),
											&(cell_list[i]->cop[1]),&(cell_list[i]->cop[2]));
									}
								}
								if (0==strcmp(temp_string,"Detail"))
								{
									for(i=0;i<n_cells;i++)
									{
										fscanf(in_file,"%d",&(cell_list[i]->detail));
									}
								}
								if (0==strcmp(temp_string,"Active_nodes"))
								{
#if defined (DEBUG)
									/*???debug */
									printf("Reading active nodes\n");
#endif /* defined (DEBUG) */
									do
									{
										fscanf(in_file, "%d", &i);
										fscanf(in_file, "%lf", &(node_list[i]->scalar_value));
										node_list[i]->active = 1;
										DEALLOCATE(temp_string);
										read_string(in_file,"s",&temp_string);
									} while (0!=strcmp(temp_string,"End_of_active_nodes"));
								}
								if (0==strcmp(temp_string,"VT_texture_curves:"))
								{
#if defined (DEBUG)
/*???debug */
printf("Reading VT_texture_curves\n");
#endif /* defined (DEBUG) */
									do
									{
										if (ALLOCATE(curve,struct VT_texture_curve,1))
										{
											fscanf(in_file,"%d",&(curve->type));
											fscanf(in_file,"%lf %lf %lf %lf %lf %lf",
												&(curve->point1[0]),&(curve->point1[1]),
												&(curve->point1[2]),&(curve->point2[0]),
												&(curve->point2[1]),&(curve->point2[2]));
											fscanf(in_file,"%lf %lf %lf %lf %lf %lf",
												&(curve->point3[0]),&(curve->point3[1]),
												&(curve->point3[2]),&(curve->point4[0]),
												&(curve->point4[1]),&(curve->point4[2]));
											fscanf(in_file,"%lf %lf",&(curve->scalar_value[0]),
												&(curve->scalar_value[1]));
											/*???DB.  Change to standard lists */
											add_curve_to_list(texture->texture_curve_list,curve);
											DEALLOCATE(temp_string);
#if defined (DEBUG)
/*???debug */
printf("Read curve. Type [%d] coords (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf)\n",
	curve->type,curve->point1[0],curve->point1[1],curve->point1[2],
	curve->point2[0],curve->point2[1],curve->point2[2],
	curve->point3[0],curve->point3[1],curve->point3[2],
	curve->point4[0],curve->point4[1],curve->point4[2]);
#endif /* defined (DEBUG) */
											read_string(in_file,"s",&temp_string);
										}
									} while (0!=strcmp(temp_string,"End_of_curves"));
#if defined (DEBUG)
/*???debug */
printf("*End of curves*\n");
#endif /* defined (DEBUG) */
								}
								if (0==strcmp(temp_string,"Hollow_mode_on"))
								{
#if defined (DEBUG)
/*???debug */
printf("#### WARNING #### Setting Hollow Mode from file\n");
#endif /* defined (DEBUG) */
									display_message(WARNING_MESSAGE,
										"Setting Hollow Mode from file");
									texture->hollow_mode_on=1;
									fscanf(in_file,"%lf",&texture->hollow_isovalue);
								}
								if (0==strcmp(temp_string,"Closed_surface"))
								{
#if defined (DEBUG)
/*???debug */
printf("#### WARNING #### Setting Closed Surface from file\n");
#endif /* defined (DEBUG) */
									display_message(WARNING_MESSAGE,
										"Setting Closed surface from file");
									texture->closed_surface=1;
								}
								if (0==strcmp(temp_string,"Clip_field"))
								{
#if defined (DEBUG)
/*???debug */
printf("#### WARNING #### Setting Clip Field from file\n");
#endif /* defined (DEBUG) */
									display_message(WARNING_MESSAGE,
										"Setting Clip field from file");
									texture->cutting_plane_on=1;
									fscanf(in_file,"%lf",&texture->cut_isovalue);
									fscanf(in_file,"%lf %lf %lf %lf",&texture->cutting_plane[0],
										&texture->cutting_plane[1],&texture->cutting_plane[2],
										&texture->cutting_plane[3]);
								}
								if (0==strcmp(temp_string,"nodal_values"))
								{
#if defined (DEBUG)
/*???debug */
printf("Reading nodal values\n");
#endif /* defined (DEBUG) */
									i=0;
									while ((i<n_nodes)&&
										(1==fscanf(in_file,"%lf",&((node_list[i])->scalar_value))))
									{
										i++;
									}
									if (i==n_nodes)
									{
										texture->calculate_nodal_values=0;
									}
									else
									{
										display_message(ERROR_MESSAGE,
									"read_volume_texture_from_file.  Error reading nodal_values");
										return_code=0;
									}
								}
								if (0==strcmp(temp_string,"Grid_spacing"))
								{
									display_message(WARNING_MESSAGE,
										"Setting Grid spacing from file");
									n=(texture->dimension)[0]+(texture->dimension)[1]+
										(texture->dimension)[2]+3;
									if (ALLOCATE(texture->grid_spacing,double,n))
									{
										for (i=0;i<n;i++)
										{
											fscanf(in_file,"%lf",&texture->grid_spacing[i]);
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
							"read_volume_texture_from_file.  Error allocating grid_spacing");
										return_code=0;
									}
								}
								if (0==strcmp(temp_string,"Xi_slits"))
								{
									display_message(WARNING_MESSAGE,"Setting Xi_slits from file");
									fscanf(in_file, "%d", &n_slits);
									for (i=0;i<n_slits;i++)
									{
										fscanf(in_file,"%d",&node_index);
										fscanf(in_file,"%d",&(texture->
											global_texture_node_list[node_index]->node_type));
										for (j=0;j<8;j++)
										{
											fscanf(in_file,"%d",&(texture->global_texture_node_list[
												node_index]->cm_node_identifier[j]));
										}
									}
								}
								if (0==strcmp(temp_string,"Node_groups"))
								{
									display_message(WARNING_MESSAGE,
										"Setting Node_groups from file");
									fscanf(in_file,"%d",&texture->n_groups);
									if (ALLOCATE(node_groups,struct VT_node_group *,
										texture->n_groups))
									{
										i=0;
										while (return_code&&(i<texture->n_groups))
										{
											if (ALLOCATE(node_group,struct VT_node_group,1))
											{
												if (read_string(in_file,"s",&(node_group->name)))
												{
													fscanf(in_file,"%d",&(node_group->n_nodes));
													if (ALLOCATE(vt_group_nodes,int,node_group->n_nodes))
													{
														node_group->nodes=vt_group_nodes;
														for (j=0;j<node_group->n_nodes;j++)
														{
															fscanf(in_file,"%d",&(node_group->nodes[j]));
														}
														node_groups[i]=node_group;
													}
													else
													{
														printf("DEBUG:ERROR\n");
														display_message(ERROR_MESSAGE,
								"read_volume_texture_from_file.  Error allocating group nodes");
														return_code=0;
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
										"read_volume_texture_from_file.  Error reading group name");
													node_group->name=(char *)NULL;
													return_code=0;
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
								"read_volume_texture_from_file.  Error allocating node_group");
												return_code=0;
											}
											i++;
										}
										texture->node_groups=node_groups;
									}
									else
									{
										display_message(ERROR_MESSAGE,
								"read_volume_texture_from_file.  Error allocating node_groups");
										return_code=0;
									}
								}
								DEALLOCATE(temp_string);
							}
#if defined (OLD_CODE)
/*???debug */
printf("generating initial isosurface\n");
							generate_isosurface(texture);
#endif /* defined (OLD_CODE) */
							if (temp_string)
							{
								DEALLOCATE(temp_string);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_volume_texture_from_file.  Error reading nodes");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_volume_texture_from_file.  Error reading cells");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
			"read_volume_texture_from_file.  Could not allocate memory for texture");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_volume_texture_from_file.  Error reading translation table");
				return_code=0;
			}
			DEALLOCATE(index_to_material);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_volume_texture_from_file.  Could not allocate translation table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_volume_texture_from_file.  Invalid argument(s)");
		return_code=0;
	}
#if defined (DEBUG)
/*???debug */
printf("leave read_volume_texture_from_file\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* read_volume_texture_from_file */

int read_volume_texture_from_obj_file(struct VT_volume_texture *texture,
	FILE *in_file,struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Environment_map) *environment_map_manager,int deformable)
/*******************************************************************************
LAST MODIFIED : 21 December 2000

DESCRIPTION :
Reads the volume <texture> from the obj <in_file>.

Parses an Alias/Wavefront .obj file and creates a triangulated surface which is
stored as an isosurface so that it can treated as a surface generated from a
volume texture.  This allows wrinkling,  deformation by FE's etc.  It is
important to maintain the connectivities and vertex indices so that .obj files
can be exported with only vertex positions changed.
==============================================================================*/
{
	char face_word[MAX_OBJ_VERTICES][128], text[512], *word, matname[128];
	double v[3],v_min,v_max;
	float *new_normal_vertices, *normal_vertices,
		*new_texture_vertices, *texture_vertices;
	int *deform,dimension,face_index,face_vertex[MAX_OBJ_VERTICES][3],i,ii,
		j, line_number, n_cells,n_face_vertices,nfv_ptr,n_nodes,n_obj_triangles,
		n_obj_vertices,n_obj_texture_vertices,n_obj_normal_vertices,
		return_code, triangle_index, warning_multiple_normals, vertex_index;
	struct Graphical_material *default_material, *scanned_material;
	struct MC_triangle **compiled_triangle_list,*mc_triangle,**temp_triangle_ptrs;
	struct MC_vertex **compiled_vertex_list,*mc_vertex;
	struct VT_texture_cell **cell_list;
	struct VT_texture_curve *curve,*curve_next;
	struct VT_texture_node **node_list;

	ENTER(read_volume_texture_from_obj_file);
	USE_PARAMETER(environment_map_manager);
#if defined (DEBUG)
	/*???debug */
	printf("enter read_volume_texture_from_obj_file\n");
#endif /* defined (DEBUG) */
	warning_multiple_normals = 0;
	/* check arguments */
	if (in_file&&texture&&graphical_material_manager)
	{
		/* default material is NULL so that it gets controlled by the graphics_object above */
		default_material=(struct Graphical_material *)NULL;
		scanned_material=default_material;
		/* set modes */
		texture->hollow_mode_on=0;
		texture->closed_surface=0;
		texture->cutting_plane_on=0;
		texture->disable_volume_functions=1;
		/* deallocate curves */
		if (texture->texture_curve_list)
		{
			curve= *(texture->texture_curve_list);
			while (curve)
			{
				curve_next=curve->ptrnext;
				DEALLOCATE(curve);
				curve=curve_next;
			}
			*(texture->texture_curve_list)= NULL;
		}
#if defined (DEBUG)
printf("deallocated curves\n");
#endif /* defined (DEBUG) */
		/* deallocate cells */
		if (cell_list=texture->texture_cell_list)
		{
			n_cells=(texture->dimension[0])*(texture->dimension[1])*
				(texture->dimension[2]);
			for (i=n_cells;i>0;i--)
			{
				DEACCESS(Graphical_material)(&((*cell_list)->material));
#if defined (OLD_CODE)
				if ((*cell_list)->env_map)
				{
					for (j=0;j<6;j++)
					{
						if ((*cell_list)->env_map->face_material[j])
						{
							DEACCESS(Graphical_material)(
								&((*cell_list)->env_map->face_material[j]));
						}
					}
				}
#endif /* defined (OLD_CODE) */
				DEACCESS(Environment_map)(&(*cell_list)->env_map);
				cell_list++;
			}
			DEALLOCATE(*texture->texture_cell_list);
			DEALLOCATE(texture->texture_cell_list);
		}
#if defined (DEBUG)
printf("deallocated cells\n");
#endif /* defined (DEBUG) */
		/* deallocate nodes */
		if (node_list=texture->global_texture_node_list)
		{
			n_nodes=(texture->dimension[0]+1)*(texture->dimension[1]+1)*
				(texture->dimension[2]+1);
			for (i=n_nodes;i>0;i--)
			{
				DEACCESS(Graphical_material)(&((*node_list)->material));
				node_list++;
			}
			DEALLOCATE(*texture->global_texture_node_list);
			DEALLOCATE(texture->global_texture_node_list);
		}
#if defined (DEBUG)
printf("deallocated nodes\n");
#endif /* defined (DEBUG) */
		DEALLOCATE(texture->scalar_field->scalar);
		DEALLOCATE(texture->clip_field->scalar);
		DEALLOCATE(texture->clip_field2->scalar);
		DEALLOCATE(texture->coordinate_field->vector);
		DEALLOCATE(texture->mc_iso_surface);
#if defined (DEBUG)
printf("deallocated\n");
#endif /* defined (DEBUG) */
#if defined (OLD_CODE)
		/* read in xi ranges */
		for (i=0;i<3;i++)
		{
			fscanf(in_file,"%lf",&((texture->ximin)[i]));
/*???debug */
printf("%g\n",(texture->ximin)[i]);
		}
		for (i=0;i<3;i++)
		{
			fscanf(in_file,"%lf",&(texture->ximax[i]));
/*???debug */
printf("%g\n",(texture->ximax)[i]);
		}
/*???debug */
		for (i=0;i<3;i++)
		{
			fscanf(in_file,"%d",&dimension);
			texture->dimension[i]=dimension;
			texture->scalar_field->dimension[i]=dimension;
			texture->clip_field->dimension[i]=dimension;
			texture->clip_field2->dimension[i]=dimension;
			texture->coordinate_field->dimension[i]=dimension;
			texture->recalculate=1;
			n_cells *= dimension;
			n_nodes *= dimension+1;
		}
#endif /* defined (OLD_CODE) */
/* for now set ximax range to be one, later set from command line */
/*		for (i=0;i<3;i++)
		{
			texture->ximin[i]=0;
			v_min = 0;
		}
		for (i=0;i<3;i++)
		{
			texture->ximax[i]=1;
			v_max = 0;
		}
*/
		v_min=0;
		v_max=0;
#if defined (DEBUG)
		/* This information is about to be overwritten by the values on the command line */
printf("texture->ximin = %lf %lf %lf, texture->ximax = %lf %lf %lf\n",
	texture->ximin[0],texture->ximin[1],texture->ximin[2],texture->ximax[0],
	texture->ximax[1],texture->ximax[2]);
#endif /* defined (DEBUG) */
		/* set all vt data to NULL or 0 apart from isosurface */
		/* for now use default graphical material */
		n_cells=0;
		n_nodes=0;
		for (i=0;i<3;i++)
		{
			texture->dimension[i]=0;
			texture->scalar_field->dimension[i]=dimension;
			texture->clip_field->dimension[i]=dimension;
			texture->clip_field2->dimension[i]=dimension;
			texture->coordinate_field->dimension[i]=dimension;
			texture->recalculate=0;
			n_cells *= dimension;
			n_nodes *= dimension+1;
		}
		if (ALLOCATE(texture->mc_iso_surface,struct MC_iso_surface,1))
		{
			texture->mc_iso_surface->n_scalar_fields=0; /* ??? */
			texture->mc_iso_surface->n_triangles=0;
			texture->mc_iso_surface->n_vertices=0;
			texture->mc_iso_surface->compiled_vertex_list=NULL;
			texture->mc_iso_surface->compiled_triangle_list=NULL;
			texture->mc_iso_surface->detail_map=NULL;
			texture->mc_iso_surface->mc_cells=NULL;
			texture->mc_iso_surface->xi_face_poly_index[0]=0;
			texture->mc_iso_surface->deform=NULL;
			for (ii=0;ii<3;ii++)
			{
				texture->mc_iso_surface->dimension[ii]=1;
				texture->mc_iso_surface->active_block[ii*2]=0;
				texture->mc_iso_surface->active_block[ii*2+1]=0;
			}
			texture->texture_cell_list=NULL;
			texture->global_texture_node_list=NULL;
			texture->hollow_isovalue=0;
			texture->hollow_mode_on=0;
			texture->cutting_plane_on=0;
			texture->cut_isovalue=0;
			texture->cutting_plane[0]=0;
			texture->cutting_plane[1]=0;
			texture->cutting_plane[2]=0;
			texture->cutting_plane[3]=0;
			texture->closed_surface=0;
			texture->decimation=0;
			texture->grid_spacing= NULL;
			return_code = 1;
			/* now read in obj data from file and convert to isosurface */
#if defined (DEBUG)
			/*???debug */
			printf("***** Reading OBJ file *****\n");
#endif /* defined (DEBUG) */
			/* perform one pass to determine number of vertices and triangles for
				memory allocation */
			n_obj_vertices=0;
			n_obj_triangles=0;
			n_obj_normal_vertices=0;
			n_obj_texture_vertices=0;
			ALLOCATE(normal_vertices, float, 1);
			ALLOCATE(texture_vertices, float, 1);
			while (NULL!=fgets(text,512,in_file))
			{
				/* parse line */
				word=strtok(text, " \t\n");
				if (word)
				{
					if ((0==strcmp(word,"v"))||(0==strcmp(word,"vxi")))
					{
						n_obj_vertices++;
					}
					else
					{
						if (0==strcmp(word, "f"))
						{
							n_face_vertices=0;
							while (word=strtok(NULL," "))
							{
								strcpy(face_word[n_face_vertices],word);
								n_face_vertices++;
							}
							/* #triangles = #vertices - 2 */
							n_obj_triangles += n_face_vertices-2;
						}
					}
				}
			}
#if defined (DEBUG)
			/*???debug */
			printf("#obj_vertices = %d,  #obj_triangles = %d\n",n_obj_vertices,
				n_obj_triangles);
#endif /* defined (DEBUG) */
			texture->mc_iso_surface->n_vertices=n_obj_vertices;
			texture->mc_iso_surface->n_triangles=n_obj_triangles;
#if defined (DEBUG)
			/*???debug */
			printf("allocating vertices and triangles....\n");
			printf("db2: deform = %p\n", texture->mc_iso_surface->deform);
#endif /* defined (DEBUG) */
			deform=texture->mc_iso_surface->deform;
			if (deformable)
			{
#if defined (DEBUG)
				/*???debug */
				printf("Allocating deformable vertices...\n");
#endif /* defined (DEBUG) */
				if (!ALLOCATE(deform,int,n_obj_vertices))
				{
					display_message(ERROR_MESSAGE,"Could not allocate objv deform list");
					deform=NULL;
				}
				else
				{
					texture->mc_iso_surface->deform=deform;
				}
			}
			if (ALLOCATE(compiled_vertex_list,struct MC_vertex *,n_obj_vertices) &&
				ALLOCATE(compiled_triangle_list,struct MC_triangle *,n_obj_triangles))
			{
				rewind(in_file);
#if defined (DEBUG)
				/*???debug */
				printf("done\n");
#endif /* defined (DEBUG) */
				vertex_index=0;
				triangle_index=0;
				line_number=0;
				while (NULL!=fgets(text, 512, in_file))
				{
					line_number++;
					/* parse line */
					word=strtok(text," \t\n");
					if (word)
					{
						if (0==strcmp(word,"#"))
						{
#if defined (DEBUG)
							/*???debug */
							printf("%d : Comment",line_number);
							while (word=strtok(NULL," "))
							{
								/* print group name */
								printf(" %s",word);
							}
#endif /* defined (DEBUG) */
						}
						else if (0==strcmp(word,"g"))
						{
#if defined (DEBUG)
							/*???debug */
							printf("%d : Group",line_number);
							while (word=strtok(NULL," "))
							{
								/* print group name */
								printf(" %s",word);
							}
#endif /* defined (DEBUG) */
						}
						else if (0==strcmp(word,"g\n"))
						{
#if defined (DEBUG)
							/*???debug */
							printf("%d : Group default\n",line_number);
#endif /* defined (DEBUG) */
						}
						else if (0==strcmp(word,"s"))
						{
#if defined (DEBUG)
							/*???debug */
							printf("%d : Smooth",line_number);
							while (word=strtok(NULL," "))
							{
								/* print group name */
								printf(" %s",word);
							}
#endif /* defined (DEBUG) */
						}
						else if (0==strcmp(word,"usemtl"))
						{
#if defined (DEBUG)
							/*???debug */
							printf("%d : usemtl",line_number);
#endif /* defined (DEBUG) */
							word=strtok(NULL," \n");
							if (word)
							{
								/* print material name */
								strncpy(matname, word, strlen(word));
								matname[strlen(word)] = 0;
								scanned_material=FIND_BY_IDENTIFIER_IN_MANAGER(
									Graphical_material,name)(matname,
										graphical_material_manager);
								/* ???RC Allow token "none" to represent NULL material */
								if (!(scanned_material ||
									fuzzy_string_compare_same_length(matname,"NONE")))
								{
#if defined (DEBUG)
									/*???debug */
									printf(
										"Could not find .obj material %s: Creating new one\n",
										matname);
#endif /* defined (DEBUG) */
									if((scanned_material = CREATE(Graphical_material)
										(matname)) && 
										(ADD_OBJECT_TO_MANAGER(Graphical_material)
										(scanned_material, graphical_material_manager)))
									{
										/* OK */
									}
									else
									{
										scanned_material = default_material;
									}

								}
								else
								{
#if defined (DEBUG)
									/*???debug */
									printf("Found .obj material %s\n",word);
#endif /* defined (DEBUG) */
								}
							}
							else
							{
								printf("Error: Missing material\n");
							}
						}
						else if (0==strcmp(word,"mtllib"))
						{
#if defined (DEBUG)
							/*???debug */
							printf("%d : mtllib",line_number);
							word=strtok(NULL," ");
							if (word)
							{
								/* print material name */
								printf(" %s",word);
							}
							else
							{
								printf("Error: Missing material library\n");
							}
#endif /* defined (DEBUG) */
						}
						else if ((0==strcmp(word, "v"))||(0==strcmp(word, "vxi")))
						{
							if (deform)
							{
								if (0==strcmp(word, "vxi"))
								{
									deform[vertex_index]=1;
								}
								else
								{
									deform[vertex_index]=0;
								}
							}
							/* process vertices */
							i=0;
							while (word=strtok(NULL," "))
							{
								v[i]=atof(word);
								i++;
							}
							if (ALLOCATE(mc_vertex,struct MC_vertex,1))
							{
								compiled_vertex_list[vertex_index]=mc_vertex;
								mc_vertex->vertex_index=vertex_index;
								for (i=0;i<3;i++)
								{
									mc_vertex->coord[i]=v[i];
									mc_vertex->normal[i]=0;
									/* find range of vertices */
									if (v[i]>v_max)
									{
										v_max=v[i];
									}
									if (v[i]<v_min)
									{
										v_min=v[i];
									}
								}
								mc_vertex->n_triangle_ptrs=0;
								mc_vertex->triangle_ptrs=NULL;
								vertex_index++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_volume_texture_from_file.  Could not allocate mc_vertex");
								return_code = 0;
							}
						}
						else if (0==strcmp(word,"vt"))
						{
							/* process texture vertices */
							if(REALLOCATE(new_texture_vertices, texture_vertices, float, 
								3 * (n_obj_texture_vertices + 1)))
							{
								texture_vertices = new_texture_vertices;
								
								new_texture_vertices = texture_vertices + 3 * n_obj_texture_vertices;
								i=0;
								while (word=strtok(NULL," "))
								{
									new_texture_vertices[i]=atof(word);
									i++;
								}
								while (i < 3)
								{
									new_texture_vertices[i]=0;
									i++;
								}
								n_obj_texture_vertices++;
							}
							
						}
						else if (0==strcmp(word,"vn"))
						{
							/* process vertex normals */
							if(REALLOCATE(new_normal_vertices, normal_vertices, float, 
								3 * (n_obj_normal_vertices + 1)))
							{
								normal_vertices = new_normal_vertices;
								
								new_normal_vertices = normal_vertices + 3 * n_obj_normal_vertices;
								i=0;
								while (word=strtok(NULL," "))
								{
									new_normal_vertices[i]=atof(word);
									i++;
								}
								while (i < 3)
								{
									new_normal_vertices[i]=0;
									i++;
								}

								n_obj_normal_vertices++;
							}
						}
						else if (0==strcmp(word,"f"))
						{
							/* process faces */
							/* SAB This should be improved, it thinks that there is
								an extra face when there is a blank on the end of a
								line */
							n_face_vertices=0;
							while (word=strtok(NULL," "))
							{
								if (n_face_vertices<MAX_OBJ_VERTICES)
								{
									strcpy(face_word[n_face_vertices],word);
								}
								n_face_vertices++;
							}
							if (n_face_vertices>MAX_OBJ_VERTICES)
							{
								n_face_vertices=MAX_OBJ_VERTICES;
								display_message(ERROR_MESSAGE,
									"read_volume_texture_from_file.  Exceeded MAX_OBJ_VERTICES");
								/*???debug */
								printf("Error: exceeded MAX_OBJ_VERTICES\n");
							}
							for (i=0;i<n_face_vertices;i++)
							{
								face_vertex[i][0]=0;
								face_vertex[i][1]=0;
								face_vertex[i][2]=0;
								if (!strstr(face_word[i],"/"))
								{
									face_vertex[i][0]=atoi(face_word[i]);
								}
								else
								{
									if (strstr(face_word[i],"//"))
									{
										if (word=strtok(face_word[i],"//"))
										{
											face_vertex[i][0]=atoi(word);
											if(word=strtok(NULL,"//"))
											{
												face_vertex[i][1]=0;
												face_vertex[i][2]=atoi(word);
												word=strtok(NULL,"/");
											}
										}
										else
										{
											face_vertex[i][0]=atoi(face_word[i]);
										}
									}
									else
									{
										if (word=strtok(face_word[i], "/"))
										{
											face_vertex[i][0]=atoi(word);
											if (word=strtok(NULL,"/"))
											{
												face_vertex[i][1]=atoi(word);
												if(word=strtok(NULL,"/"))
												{
													face_vertex[i][2]=atoi(word);
												}
											}
										}
										else
										{
											face_vertex[i][0]=atoi(face_word[i]);
										}
									}
								}
							}
							/*???debug */
							/*for (i=0;i<n_face_vertices;i++)
							  {
							  printf(" %d/%d/%d",face_vertex[i][0],face_vertex[i][1],face_vertex[i][2]);
							  }*/
							/* if n_face_vertices > 3 we must add an extra
								triangle for each additional vertex */
							/* for time being calculate a fan
								triangulation */
							if (n_face_vertices>=3)
							{
								nfv_ptr=0;
								for (ii=1;ii<n_face_vertices-1;ii++)
								{
									/* fan from face_vertex[0] */
									if (ALLOCATE(mc_triangle,struct MC_triangle,
										1))
									{
										/*???debug */
										/*printf("allocating triangle %d\n", triangle_index);*/
										compiled_triangle_list[triangle_index]=
											mc_triangle;
										mc_triangle->triangle_index=
											triangle_index;
										for (i=0;i<3;i++)
										{
											/* NB: we subtract 1 from the obj file
												vertex index for consistency with C
												arrays */
											if(i==0)
											{
												face_index = 0;
											}
											else
											{
												face_index = i+ii-1;
											}
											if((face_vertex[face_index][0] > 0) &&
												(face_vertex[face_index][0] <= vertex_index))
											{
												mc_triangle->vertex_index[i]=
													face_vertex[face_index][0]-1;
											}
											else
											{
												display_message(WARNING_MESSAGE,
													"read_volume_texture_from_file: vertex"
													" %d not defined when used", face_vertex[face_index][0]);
												mc_triangle->vertex_index[i]=0;
											}
											if(face_vertex[face_index][1])
											{
												if((face_vertex[face_index][1] > 0) &&
													(face_vertex[face_index][1] <= n_obj_texture_vertices))
												{
													for (j=0;j<3;j++)
													{
														mc_triangle->texture_coord[i][j]=
															texture_vertices[3 * (face_vertex[face_index][1] - 1) + j];
													}
												}
												else
												{
													display_message(WARNING_MESSAGE,
														"read_volume_texture_from_file: texture vertex"
														" %d not defined when used", face_vertex[face_index][1]);
												}
											}
											else
											{
												for (j=0;j<3;j++)
												{
													mc_triangle->texture_coord[i][j]=0;
												}
											}

											if(face_vertex[face_index][2])
											{
												if((face_vertex[face_index][2] > 0) &&
													(face_vertex[face_index][2] <= n_obj_normal_vertices))
												{
													mc_vertex = compiled_vertex_list[mc_triangle->vertex_index[i]];
													if(mc_vertex->normal[0] || mc_vertex->normal[1]
														|| mc_vertex->normal[2])
													{
														if((mc_vertex->normal[0] != 
															normal_vertices[3 * (face_vertex[face_index][2] - 1)])
															|| (mc_vertex->normal[1] != 
															normal_vertices[3 * (face_vertex[face_index][2] - 1) + 1])
															|| (mc_vertex->normal[2] != 
															normal_vertices[3 * (face_vertex[face_index][2] - 1) + 2]))
														{
															if(!warning_multiple_normals)
															{
																display_message(WARNING_MESSAGE,
																	"read_volume_texture_from_file: multiple normals defined"
																	" for vertices, first normals used");
																warning_multiple_normals = 1;
															}
														}
													}
													else
													{
														for (j=0;j<3;j++)
														{
															mc_vertex->normal[j] = 
																normal_vertices[3 * (face_vertex[face_index][2] - 1) + j];
														}
													}
												}
												else
												{
													display_message(WARNING_MESSAGE,
														"read_volume_texture_from_file: normal vertex"
														" %d not defined when used", face_vertex[face_index][1]);
												}
											}

											mc_triangle->material[i]=
												scanned_material;
											mc_triangle->env_map[i]=NULL;
											mc_triangle->env_map_index[i]=0;
										}
#if defined (OLD_CODE)
										/* SAB I don't think this is supposed to be here */
										mc_vertex->n_triangle_ptrs=0;
										mc_vertex->triangle_ptrs=NULL;
#endif /* defined (OLD_CODE) */
										mc_triangle->triangle_list_index=0;
										mc_triangle->cell_ptr=0;
										triangle_index++;
										nfv_ptr++;
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_volume_texture_from_file: Could not allocate mc_vertex");
										return_code = 0;
									}
								}
							}
							else
							{
								display_message(WARNING_MESSAGE,
									"Less than 3 vertices in face");
							}
						}
					}
				}
#if defined (DEBUG)
				/*???debug */
				printf("db2: deform = %p\n", texture->mc_iso_surface->deform);
#endif /* defined (DEBUG) */
				/*	At this point each vertex has a unique index, but has no
					information on which triangles it is shared by. each triangle knows
					the vertices it contains,  and has an integer index.  To complete the
					data structure we need to assign a back pointer to the triangle owners
					of each vertex. This requires scanning through the triangle list and
					updating each vertix.
					Note that the compiled_v/t_list array indices start at 0 as opposed to
					the OBJ file 1 */
				/* scan through triangle list */
				for (i=0;i<n_obj_triangles;i++)
				{
					/* update each vertex used by the triangle */
					for (j=0;j<3;j++)
					{
						mc_vertex=
							compiled_vertex_list[compiled_triangle_list[i]->vertex_index[j]];
						if (ALLOCATE(temp_triangle_ptrs,struct MC_triangle *,
							mc_vertex->n_triangle_ptrs+1))
						{
							for (ii=0;ii<mc_vertex->n_triangle_ptrs;ii++)
							{
								temp_triangle_ptrs[ii]=mc_vertex->triangle_ptrs[ii];
							}
							temp_triangle_ptrs[mc_vertex->n_triangle_ptrs]=
								compiled_triangle_list[i];
							mc_vertex->n_triangle_ptrs++;
							DEALLOCATE(mc_vertex->triangle_ptrs);
							mc_vertex->triangle_ptrs=temp_triangle_ptrs;
						}
						else
						{
							display_message(ERROR_MESSAGE,
						"read_volume_texture_from_obj_file.   Alloc triangle_ptrs failed");
						}
					}
				}
				/* assign structures */
				texture->mc_iso_surface->compiled_triangle_list=compiled_triangle_list;
				texture->mc_iso_surface->compiled_vertex_list=compiled_vertex_list;
			}
			else
			{
				display_message(ERROR_MESSAGE,
		"read_volume_texture_from_obj_file.  Could not allocate isosurface memory");
				return_code = 0;
			}
			DEALLOCATE(texture_vertices);
			DEALLOCATE(normal_vertices);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_volume_texture_from_obj_file.  Could not allocate isosurface");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_volume_texture_from_obj_file.  Invalid argument(s)");
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave read_volume_texture_from_obj_file  deform = %p\n",
		texture->mc_iso_surface->deform);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* read_volume_texture_from_obj_file */

int write_volume_texture_to_file(struct VT_volume_texture *texture,
	FILE *out_file)
/*******************************************************************************
LAST MODIFIED : 15 January 1998

DESCRIPTION :
Writes the volume <texture> to a <out_file>.
==============================================================================*/
{
	int *cell_index,*cell_index_table,found,i,j,last_node,n,n_cells,n_nodes,
		*node_index,*node_index_table,number_of_materials,number_of_env_maps,
		output_active_nodes,return_code;
	struct Environment_map **index_to_env_map,**index_to_env_map_table,*env_map;
	struct Graphical_material **index_to_material,**index_to_material_table,
		*material;
	struct VT_texture_cell **cell;
	struct VT_texture_curve *curve;
	struct VT_texture_node **node;

	ENTER(write_volume_texture_to_file);
	return_code=1;
	printf("Writing VT to file\n");
	if (texture&&out_file)
	{
		n_cells=(texture->dimension[0])*(texture->dimension[1])*
			(texture->dimension[2]);
		n_nodes=(texture->dimension[0]+1)*(texture->dimension[1]+1)*
			(texture->dimension[2]+1);
/*???debug */
printf("#cells=%d, #nodes=%d\n",n_cells,n_nodes);
		/* write the xi ranges */
		for (i=0;i<3;i++)
		{
			fprintf(out_file,"%lf ",texture->ximin[i]);
		}
		for (i=0;i<3;i++)
		{
			fprintf(out_file,"%lf ",texture->ximax[i]);
		}
		fprintf(out_file,"\n");
		/* write the discretization */
		for (i=0;i<3;i++)
		{
			fprintf(out_file,"%d ",texture->dimension[i]);
		}
		fprintf(out_file,"\n");

		/* allocate memory for cell and node index tables */
		if (ALLOCATE(cell_index_table,int,n_cells)&&ALLOCATE(node_index_table,int,
			n_nodes)&&ALLOCATE(index_to_material_table,struct Graphical_material *,1))
		{
			/* create the table for translating indicies into materials */
printf("Creating material conversion table\n");
			number_of_materials=0;
			*index_to_material_table=(struct Graphical_material *)NULL;
			cell=texture->texture_cell_list;
			i=n_cells;
			cell_index=cell_index_table;
			while (return_code&&(i>0))
			{
				if (material=(*cell)->material)
				{
					/* check if the material is already in the table */
					j=number_of_materials;
					index_to_material=index_to_material_table+(j-1);
					while ((j>0)&&(material!= *index_to_material))
					{
						j--;
						index_to_material--;
					}
					if (j>0)
					{
						*cell_index=j;
					}
					else
					{
/*???debug */
printf("new material cell %p %p\n",material,*index_to_material);
						/* new_material */
						if (REALLOCATE(index_to_material,index_to_material_table,
							struct Graphical_material *,number_of_materials+1))
						{
							index_to_material_table=index_to_material;
							index_to_material_table[number_of_materials]=material;
							number_of_materials++;
							*cell_index=number_of_materials;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					*cell_index=0;
				}
				cell++;
				cell_index++;
				i--;
			}
			node=texture->global_texture_node_list;
			i=n_nodes;
			node_index=node_index_table;
			while (return_code&&(i>0))
			{
				if (material=(*node)->material)
				{
					/* check if the material is already in the table */
					j=number_of_materials;
					index_to_material=index_to_material_table+(j-1);
					while ((j>0)&&(material!= *index_to_material))
					{
						j--;
						index_to_material--;
					}
					if (j>0)
					{
						*node_index=j;
					}
					else
					{
/*???debug */
printf("new material cell %p %p\n",material,*index_to_material);
						/* new_material */
						if (REALLOCATE(index_to_material,index_to_material_table,
							struct Graphical_material *,number_of_materials+1))
						{
							index_to_material_table=index_to_material;
							index_to_material_table[number_of_materials]=material;
							number_of_materials++;
							*node_index=number_of_materials;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					*node_index=0;
				}
				node++;
				node_index++;
				i--;
			}
#if defined (OLD_CODE)
			index_to_material_table[0] = NULL;
			number_of_materials = 0;
			return_code = 1;
			for (i=0;i<n_cells;i++)
			{
				cell_index_table[i] = 0;
				material = texture->texture_cell_list[i]->material;
				found=0;
				/* search list for material - if not there reallocate and add to list */
				for (j=0;j<number_of_materials;j++)
				{
				if (index_to_material_table[j] == material)
					{
						if (material == NULL)
						{
							cell_index_table[i] = 0;
						}
						else
						{
							cell_index_table[i] = j+1;
						}
						found = 1;
					}
				}
				if ((!found || (number_of_materials == 0)) && material)
				{
					/* increase size of index table by 1, set the new material values */

					REALLOCATE(index_to_material,index_to_material_table,
						struct Graphical_material *,number_of_materials+1);
					index_to_material_table = index_to_material;
					index_to_material_table[number_of_materials] = material;

					cell_index_table[i] = number_of_materials+1;
					number_of_materials++;

				}

			} /* i */

			for (i=0;i<n_nodes;i++)
			{
				node_index_table[i] = 0;
				material = texture->global_texture_node_list[i]->material;
				found=0;
				/* search list for material - if not there reallocate and add to list */
				for (j=0;j<number_of_materials;j++)
				{
					if (index_to_material_table[j] == material)
					{
						if (material == NULL)
						{
							node_index_table[i] = 0;
						}
						else
						{
							node_index_table[i] = j+1;
						}
						found = 1;
					}
				}
				if (!found || (number_of_materials == 0) && material)
				{
					/* increase size of index table by 1, set the new material values */
					REALLOCATE(index_to_material,index_to_material_table,
						struct Graphical_material *,number_of_materials+1);
					index_to_material_table = index_to_material;

					index_to_material_table[number_of_materials] = material;
					node_index_table[i] = number_of_materials+1;
					number_of_materials++;
				}

			} /* i */
#endif /* defined (OLD_CODE) */
			if (return_code)
			{
				/* write the material list */
				fprintf(out_file,"%d \n",number_of_materials);
				for (i=0;i<number_of_materials;i++)
				{
					fprintf(out_file,"%s\n",
						Graphical_material_name(index_to_material_table[i]));
				}

				/* write the cells */
				for (i=0;i<n_cells;i++)
				{
					fprintf(out_file,"%d ",cell_index_table[i]);
					if (19==(i%20))
					{
						fprintf(out_file,"\n");
					}
				}
				fprintf(out_file,"\n");

				for (i=0;i<n_cells;i++)
				{
					fprintf(out_file,"%.2lf ",
						texture->texture_cell_list[i]->scalar_value);
					if (9==(i%10))
					{
						fprintf(out_file,"\n");
					}
				}
				fprintf(out_file,"\n");

				/* write the nodes */
				for (i=0;i<n_nodes;i++)
				{
					fprintf(out_file,"%d ",node_index_table[i]);
					if (19==(i%20))
					{
						fprintf(out_file,"\n");
					}
				}
				fprintf(out_file,"\n");

				/* the isovalue */
			}
			fprintf(out_file,"%lf\n",texture->isovalue);
/***********************************************************/
			fprintf(out_file,"Environment_maps\n");

			return_code = 1;
			/* allocate memory for environment index tables */
			if (ALLOCATE(cell_index_table,int,n_cells)&&
				ALLOCATE(index_to_env_map_table,struct Environment_map *,1))
			{
			/* create the table for translating indicies into materials */
printf("Creating material conversion table\n");
			number_of_env_maps=0;
			*index_to_env_map_table=(struct Environment_map *)NULL;
			cell=texture->texture_cell_list;
			i=n_cells;
			cell_index=cell_index_table;
			while (return_code&&(i>0))
			{
				if (env_map=(*cell)->env_map)
				{
					/* check if the environment map is already in the table */
					j=number_of_env_maps;
					index_to_env_map=index_to_env_map_table+(j-1);
					while ((j>0)&&(env_map!= *index_to_env_map))
					{
						j--;
						index_to_env_map--;
					}
					if (j>0)
					{
						*cell_index=j;
					}
					else
					{
/*???debug */
printf("new env_map cell %p %p\n",env_map,*index_to_env_map);
						/* new env_map */
						if (REALLOCATE(index_to_env_map,index_to_env_map_table,
							struct Environment_map *,number_of_env_maps+1))
						{
							index_to_env_map_table=index_to_env_map;
							index_to_env_map_table[number_of_env_maps]=env_map;
							number_of_env_maps++;
							*cell_index=number_of_env_maps;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					*cell_index=0;
				}
				cell++;
				cell_index++;
				i--;
			}


			if (return_code)
			{
				/* write the environment map list */

				fprintf(out_file,"%d \n",number_of_env_maps);
				for (i=0;i<number_of_env_maps;i++)
				{
					fprintf(out_file,"%s\n",index_to_env_map_table[i]->name);
				}

				/* write the cells */
				for (i=0;i<n_cells;i++)
				{
					fprintf(out_file,"%d ",cell_index_table[i]);
					if (19==(i%20))
					{
						fprintf(out_file,"\n");
					}
				}
				fprintf(out_file,"\n");
			}
/***********************************************************/
				/* print centres of projection (cells only at this stage) */
				fprintf(out_file,"Projection_centres\n");
				for (i=0;i<n_cells;i++)
				{
					fprintf(out_file,"%.2lf %.2lf %.2lf\n",
						texture->texture_cell_list[i]->cop[0],texture->texture_cell_list[i]->cop[1],
						texture->texture_cell_list[i]->cop[2]);
				}
				fprintf(out_file,"Detail\n");
				for (i=0;i<n_cells;i++)
				{
					fprintf(out_file,"%d ",
						texture->texture_cell_list[i]->detail);
					if (19==(i%20) || i == n_cells-1)
					{
						fprintf(out_file,"\n");
					}
				}
				/* curves */
/*???debug */
printf("Writing curves\n");
				if (curve= *(texture->texture_curve_list))
				{
					fprintf(out_file,"VT_texture_curves:\n");
					while (curve)
					{
						fprintf(out_file,"%d\n",curve->type);
						fprintf(out_file,
							"%lf %lf %lf %lf %lf %lf\n %lf %lf %lf %lf %lf %lf\n %lf %lf\n",
							curve->point1[0],curve->point1[1],curve->point1[2],
							curve->point2[0],curve->point2[1],curve->point2[2],
							curve->point3[0],curve->point3[1],curve->point3[2],
							curve->point4[0],curve->point4[1],curve->point4[2],
							curve->scalar_value[0],curve->scalar_value[1]);
						if (curve=curve->ptrnext)
						{
							fprintf(out_file,"next_curve:\n");
						}
					}
					fprintf(out_file,"End_of_curves\n");
				}
				/* write out any active nodes */
				output_active_nodes=0;
				last_node=0;
				for (i=0;i<n_nodes;i++)
				{
					if (texture->global_texture_node_list[i]->active)
					{
						output_active_nodes=1;
						last_node=i;
					}
				}
				if (output_active_nodes)
				{
					fprintf(out_file,"Active_nodes\n");
					for (i=0;i<n_nodes;i++)
					{
						if (texture->global_texture_node_list[i]->active)
						{
							fprintf(out_file,"%d %lf\n",i,
								texture->global_texture_node_list[i]->scalar_value);
							if (i!=last_node)
							{
								fprintf(out_file,"Next_node\n");
							}
						}
					}
					fprintf(out_file,"End_of_active_nodes\n");
				}
				if (texture->hollow_mode_on)
				{
/*???debug */
printf("#### WARNING #### Setting Hollow mode in file\n");
					fprintf(out_file,"Hollow_mode_on\n");
					fprintf(out_file,"%lf\n",texture->hollow_isovalue);
				}
				if (texture->closed_surface)
				{
/*???debug */
printf("#### WARNING #### Setting Closed Surface in file\n");
					fprintf(out_file,"Closed_surface\n");
				}
				if (texture->cutting_plane_on)
				{
/*???debug */
printf("#### WARNING #### Setting Cutting Plane in file\n");
					fprintf(out_file,"Clip_field\n");
					fprintf(out_file,"%lf\n",texture->cut_isovalue);
					fprintf(out_file,"%lf %lf %lf %lf\n",
					texture->cutting_plane[0],texture->cutting_plane[1],
					texture->cutting_plane[2],texture->cutting_plane[3]);
				}
				if (texture->grid_spacing)
				{
					fprintf(out_file,"Grid_spacing\n");
					n=texture->dimension[0]+texture->dimension[1]+texture->dimension[2]+3;
					for (i=0;i<n;i++)
					{
						fprintf(out_file,"%lf ",texture->grid_spacing[i]);
						if ((i%20==0)&&(i!=0))
						{
							fprintf(out_file,"\n");
						}
					}
					fprintf(out_file,"\n");
				}
				/* scan nodes to see if any nodes are split */
				found=0;
				for (i=0;i<n_nodes;i++)
				{
					if (texture->global_texture_node_list[i]->node_type>0)
					{
						found++;
					}
				}
				if (found)
				{
					fprintf(out_file,"Xi_slits\n");
					fprintf(out_file,"%d\n", found);
					for (i=0;i<n_nodes;i++)
					{
						if (texture->global_texture_node_list[i]->node_type>0)
						{
							fprintf(out_file,"%d %d  ",i,
								texture->global_texture_node_list[i]->node_type);
							for (j=0;j<8;j++)
							{
								fprintf(out_file,"%d ",
									texture->global_texture_node_list[i]->cm_node_identifier[j]);
							}
							fprintf(out_file,"\n");
						}
					}
				}
				if (texture->n_groups>0)
				{
					fprintf(out_file,"Node_groups\n");
					fprintf(out_file,"%d\n", texture->n_groups);
					for (i=0;i<texture->n_groups;i++)
					{
						fprintf(out_file,"%s %d\n",(texture->node_groups[i])->name,
							(texture->node_groups[i])->n_nodes);
						for (j=0;j<(texture->node_groups[i])->n_nodes;j++)
						{
							fprintf(out_file,"%d ",(texture->node_groups[i])->nodes[j]);
							if ((j%20==0)&&(j!=0))
							{
								fprintf(out_file,"\n");
							}
						}
						fprintf(out_file,"\n");
					}
				}
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"write_volume_texture_to_file.  Could not allocate translation table");
			}
			DEALLOCATE(index_to_material_table);
			DEALLOCATE(cell_index_table);
			DEALLOCATE(node_index_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"write_volume_texture_to_file.  Could not allocate cell and node tables");
			DEALLOCATE(cell_index_table);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_volume_texture_to_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_volume_texture_to_file */

void generate_isosurface(struct VT_volume_texture *texture)
/*******************************************************************************
LAST MODIFIED : 7 April 1997

DESCRIPTION :
Creates/Updates isosurface for basic volume_texture
==============================================================================*/
{
	int i,face_index;
	int n_scalar_fields;
	struct VT_scalar_field *scalar_field_list[MAX_SCALAR_FIELDS];
	double isovalue_list[MAX_SCALAR_FIELDS];
	struct MC_triangle *triangle;

	ENTER(generate_isosurface);
	/* calculate scalar field using filter */
	update_scalars(texture,texture->cutting_plane);
	scalar_field_list[0] = texture->scalar_field;
	isovalue_list[0] = texture->isovalue;
	if (texture->hollow_mode_on)
	{
		/* should combine clip field and this field in some way */
		for (i=(texture->clip_field2->dimension[0]+1)*
			(texture->clip_field2->dimension[1]+1)*
			(texture->clip_field2->dimension[2]+1)-1;i>=0;i--)
		{
			texture->clip_field2->scalar[i] = 1.0-texture->scalar_field->scalar[i];
		}
	}
	n_scalar_fields = 1;
	if (texture->cutting_plane_on)
	{
		scalar_field_list[n_scalar_fields] = texture->clip_field;
		isovalue_list[n_scalar_fields] = texture->cutting_plane[3];
		n_scalar_fields++;
	}
 	if (texture->hollow_mode_on)
	{
		scalar_field_list[n_scalar_fields] = texture->clip_field2;
		isovalue_list[n_scalar_fields] = texture->hollow_isovalue*texture->isovalue;
		n_scalar_fields++;
	}
/*???debug */
/*printf("Marching cubes: isovalue = %lf : n_scalar_fields = %d : mode [hollow=%d] [closed = %d] [clip = %d]\n",
	texture->isovalue,n_scalar_fields,texture->hollow_mode_on,
	texture->closed_surface,texture->cutting_plane_on);*/
	/* create isosurface */
/*???DB.  Why does the iso value have to be between 0 and 1 ? */
/*  if (texture->isovalue >= 0.0 && texture->isovalue <= 1.0)*/
	{
		/* ( cutting plane[3] = clip isovalue ) */
		if (NULL==texture->mc_iso_surface)
		{
			display_message(ERROR_MESSAGE,
				"generate_isosurface.  mc_iso_surface = NULL");
		}
		marching_cubes(scalar_field_list,n_scalar_fields,texture->coordinate_field,
			texture->mc_iso_surface,isovalue_list,texture->closed_surface,
			(texture->cutting_plane_on)||(texture->hollow_mode_on),
			texture->decimation);
		/* set polygon materials from texture data */
		calculate_mc_material(texture,texture->mc_iso_surface);
		for (i=0;i<texture->mc_iso_surface->n_triangles;i++)
		{
			triangle=texture->mc_iso_surface->compiled_triangle_list[i];
			face_index=triangle->triangle_list_index;
			if ((face_index>0)&&(face_index<=6))
			{
				/* special case - texture projection on actual face */
				mc_face_cube_map_function(&(triangle->env_map_index[0]),
					triangle->texture_coord[0],triangle->vertices[0],
					triangle->iso_poly_cop[0],face_index,texture->ximin,texture->ximax);
				mc_face_cube_map_function(&(triangle->env_map_index[1]),
					triangle->texture_coord[1],triangle->vertices[1],
					triangle->iso_poly_cop[1],face_index,texture->ximin,texture->ximax);
				mc_face_cube_map_function(&(triangle->env_map_index[2]),
					triangle->texture_coord[2],triangle->vertices[2],
					triangle->iso_poly_cop[2],face_index,texture->ximin,texture->ximax);
			}
			else
			{
				/* general case */
				mc_cube_map_function(&(triangle->env_map_index[0]),
					triangle->texture_coord[0],triangle->vertices[0],
					triangle->iso_poly_cop[0],texture->ximin,texture->ximax);
				mc_cube_map_function(&(triangle->env_map_index[1]),
					triangle->texture_coord[1],triangle->vertices[1],
					triangle->iso_poly_cop[1],texture->ximin,texture->ximax);
				mc_cube_map_function(&(triangle->env_map_index[2]),
					triangle->texture_coord[2],triangle->vertices[2],
					triangle->iso_poly_cop[2],texture->ximin,texture->ximax);
			}
		}
	}
/*???debug */
/*else
{
	printf("error : isovalue is %f\n",texture->isovalue);
}*/
	LEAVE;
} /* generate_isosurface */

struct Clipping *CREATE(Clipping)(void)
/*******************************************************************************
LAST MODIFIED : 26 January 1996

DESCRIPTION :
Allocates memory for a clipping.
==============================================================================*/
{
	struct Clipping *clipping;

	ENTER(CREATE(Clipping));
	if (ALLOCATE(clipping,struct Clipping,1))
	{
		clipping->function=NULL;
		clipping->parameters=NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE, "CREATE(Clipping).  Insufficient memory");
	}
	LEAVE;

	return (clipping);
} /* CREATE(Clipping) */

int DESTROY(Clipping)(struct Clipping **clipping_address)
/*******************************************************************************
LAST MODIFIED : 26 January 1996

DESCRIPTION :
Frees the memory for the clipping and sets <*clipping_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Clipping *clipping;

	ENTER(DESTROY(Clipping));
	if (clipping_address)
	{
		if (clipping= *clipping_address)
		{
			DEALLOCATE(clipping->parameters);
			DEALLOCATE(*clipping_address);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Clipping).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Clipping) */

int set_Clipping(struct Parse_state *state,void *clipping_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the clipping from a command.
==============================================================================*/
{
	char *current_token;
	double *clipping_parameters;
	int return_code;
	struct Clipping *clipping,**clipping_address;

	ENTER(set_Clipping);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (clipping_address=(struct Clipping **)clipping_address_void)
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*clipping_address)
						{
							DESTROY(Clipping)(clipping_address);
						}
						shift_Parse_state(state,1);
						return_code=1;
					}
					else
					{
						if (ALLOCATE(clipping_parameters,double,4))
						{
							if (1==sscanf(current_token,"%lf",clipping_parameters))
							{
								if (shift_Parse_state(state,1))
								{
									if (1==sscanf(state->current_token,"%lf",
										clipping_parameters+1))
									{
										if (shift_Parse_state(state,1))
										{
											if (1==sscanf(state->current_token,"%lf",
												clipping_parameters+2))
											{
												if (shift_Parse_state(state,1))
												{
													if (1==sscanf(state->current_token,"%lf",
														clipping_parameters+3))
													{
														shift_Parse_state(state,1);
														if (clipping=CREATE(Clipping)())
														{
															if (*clipping_address)
															{
																DESTROY(Clipping)(clipping_address);
															}
															clipping->function=plane;
															clipping->parameters=clipping_parameters;
															*clipping_address=clipping;
															return_code=1;
														}
														else
														{
															display_message(WARNING_MESSAGE,
																"set_Clipping.  Insufficient memory");
															DEALLOCATE(clipping_parameters);
															return_code=0;
														}
													}
													else
													{
														display_message(WARNING_MESSAGE,
															"Invalid clipping parameter 4: %s",
															state->current_token);
														display_parse_state_location(state);
														DEALLOCATE(clipping_parameters);
														return_code=0;
													}
												}
												else
												{
													display_message(WARNING_MESSAGE,
														"Missing clipping parameter 4");
													display_parse_state_location(state);
													DEALLOCATE(clipping_parameters);
													return_code=0;
												}
											}
											else
											{
												display_message(WARNING_MESSAGE,
													"Invalid clipping parameter 3: %s",
													state->current_token);
												display_parse_state_location(state);
												DEALLOCATE(clipping_parameters);
												return_code=0;
											}
										}
										else
										{
											display_message(WARNING_MESSAGE,
												"Missing clipping parameter 3");
											display_parse_state_location(state);
											DEALLOCATE(clipping_parameters);
											return_code=0;
										}
									}
									else
									{
										display_message(WARNING_MESSAGE,
											"Invalid clipping parameter 2: %s",state->current_token);
										display_parse_state_location(state);
										DEALLOCATE(clipping_parameters);
										return_code=0;
									}
								}
								else
								{
									display_message(WARNING_MESSAGE,
										"Missing clipping parameter 2");
									display_parse_state_location(state);
									DEALLOCATE(clipping_parameters);
									return_code=0;
								}
							}
							else
							{
								display_message(WARNING_MESSAGE,
									"Invalid clipping parameter 1: %s",current_token);
								display_parse_state_location(state);
								DEALLOCATE(clipping_parameters);
								return_code=0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"set_Clipping.  Insufficient memory");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_VT_volume_texture.  Missing clipping_address_void");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," # # # #|none");
				if ((clipping_address=(struct Clipping **)clipping_address_void)&&
					(*clipping_address)&&
					(clipping_parameters=(*clipping_address)->parameters))
				{
					display_message(INFORMATION_MESSAGE,"[%g %g %g %g]",
						clipping_parameters[0],clipping_parameters[1],
						clipping_parameters[2],clipping_parameters[3]);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[none]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing clipping parameters");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_VT_volume_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Clipping */
