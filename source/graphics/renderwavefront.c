/*******************************************************************************
FILE : renderwavefront.c

LAST MODIFIED : 19 October 1998

DESCRIPTION :
Renders gtObjects to Wavefront OBJ file
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "graphics/animation_window.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_window.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/renderwavefront.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "user_interface/message.h"


/*
Module types
------------
*/

struct Wavefront_vertex_position
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
==============================================================================*/
{
	float x;
	float y;
	float z;
}; /* struct Wavefront_vertex_position */

struct Wavefront_vertex
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
==============================================================================*/
{
	struct Wavefront_vertex_position *position;
	int index;

	int access_count;
}; /* struct Wavefront_vertex */

/*
Module variables
----------------
*/
static int file_vertex_index, file_normal_vertex_index, file_texture_vertex_index;

/*
Module functions
----------------
*/
int compare_vertex_location(struct Wavefront_vertex_position *position_1,
	struct Wavefront_vertex_position *position_2)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
Returns -1 if location_1 < location_2, 0 if location_1 = location_2 within a
small tolerance and 1 if location_1 > location_2.  X is considered first, then
Y and the Z.
==============================================================================*/
{
	const float tolerance = 1e-3;
	int return_code;

	ENTER(compare_vertex_location);
	if (position_1->x < (position_2->x - tolerance))
	{
		return_code= -1;
	}
	else
	{
		if (position_1->x > (position_2->x + tolerance))
		{
			return_code=1;
		}
		else
		{
			if (position_1->y < (position_2->y - tolerance))
			{
				return_code= -1;
			}
			else
			{
				if (position_1->y > (position_2->y + tolerance))
				{
					return_code=1;
				}
				else
				{
					if (position_1->z < (position_2->z - tolerance))
					{
						return_code= -1;
					}
					else
					{
						if (position_1->z > (position_2->z + tolerance))
						{
							return_code=1;
						}
						else
						{
							return_code=0;
						}
					}
				}
			}
		}
	}
	LEAVE;
	
	return (return_code);
} /* compare_position */

struct Wavefront_vertex *CREATE(Wavefront_vertex)
	(int index, float x, float y, float z)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
==============================================================================*/
{
	struct Wavefront_vertex *vertex;
	struct Wavefront_vertex_position *vertex_position;

	ENTER(CREATE(Wavefront_vertex));
	
	if (ALLOCATE(vertex,struct Wavefront_vertex,1)&&
		 (ALLOCATE(vertex_position,struct Wavefront_vertex_position,1)))
	{
		vertex->position = vertex_position;
		vertex_position->x = x;
		vertex_position->y = y;
		vertex_position->z = z;
		vertex->index = index;
		vertex->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Wavefront_vertex).  Not enough memory");
		vertex = (struct Wavefront_vertex *)NULL;
	}
	LEAVE;

	return (vertex);
} /* CREATE(Wavefront_vertex) */

int DESTROY(Wavefront_vertex)(struct Wavefront_vertex **vertex_address)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
Frees memory/deaccess mapping at <*vertex_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Wavefront_vertex));
	if (vertex_address&&*vertex_address)
	{
		if (0 >= (*vertex_address)->access_count)
		{
			DEALLOCATE((*vertex_address)->position);
			DEALLOCATE(*vertex_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Wavefront_vertex).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Wavefront_vertex).  Missing mapping");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Wavefront_vertex) */

DECLARE_OBJECT_FUNCTIONS(Wavefront_vertex)
DECLARE_LIST_TYPES(Wavefront_vertex);
FULL_DECLARE_INDEXED_LIST_TYPE(Wavefront_vertex);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Wavefront_vertex,
	position,struct Wavefront_vertex_position *,compare_vertex_location)
DECLARE_INDEXED_LIST_FUNCTIONS(Wavefront_vertex)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(
	Wavefront_vertex,
	position,struct Wavefront_vertex_position *,compare_vertex_location)

static int activate_material_wavefront(FILE *file,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Writes Wavefront object file that defines the material
==============================================================================*/
{
	int return_code;

	ENTER(activate_material_wavefront);
	if (material&&file)
	{
		fprintf(file,"usemtl %s\n",Graphical_material_name(material));
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"activate_material_wavefront.  Missing material or FILE handle");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* activate_material_wavefront */

static int makewavefront(FILE *wavefront_file, int full_comments,
	gtObject *object, float time);

int draw_glyph_set_wavefront(FILE *wavefront_file, int number_of_points,
	Triple *point_list,Triple *axis1_list,
	Triple *axis2_list,Triple *axis3_list,struct GT_object *glyph,char **labels,
	int number_of_data_components,GTDATA *data,struct Graphical_material *material,
	struct Spectrum *spectrum,float time)
/*******************************************************************************
LAST MODIFIED : 11 May 1999

DESCRIPTION :
Defines an object for the <glyph> and then draws that at <number_of_points>
points  given by the positions in <point_list> and oriented and scaled by 
<axis1_list>, <axis2_list> and <axis3_list>. 
==============================================================================*/
{
	float transformation[16];
	int i,return_code;
	struct GT_object *transformed_object;
	Triple *point,*axis1,*axis2,*axis3;

	ENTER(draw_glyph_set_vrml);
	/* Keep a similar interface to all the other render implementations */
	USE_PARAMETER(labels);
	return_code=0;
	if ((0<number_of_points)&&point_list&&axis1_list&&axis2_list&&axis3_list&&
		glyph&&((!number_of_data_components)||(data&&material&&spectrum)))
	{
		if (!data||(data&&spectrum))
		{
			point=point_list;
			axis1=axis1_list;
			axis2=axis2_list;
			axis3=axis3_list;
			/* try to draw points and lines faster */
			if (0==strcmp(glyph->name,"point"))
			{
				display_message(WARNING_MESSAGE,"draw_glyph_set_wavefront.  "
					"pointset glyphs not currently rendered in wavefront files (use a surface glyph).");
			}
			else if (0==strcmp(glyph->name,"line"))
			{
				display_message(WARNING_MESSAGE,"draw_glyph_set_wavefront.  "
					"pointset glyphs not currently rendered in wavefront files (use a surface glyph).");
			}
			else
			{
				for (i=0;i<number_of_points;i++)
				{
					transformation[ 0] = (*axis1)[0];
					transformation[ 1] = (*axis1)[1];
					transformation[ 2] = (*axis1)[2];
					axis1++;
					transformation[ 4] = (*axis2)[0];
					transformation[ 5] = (*axis2)[1];
					transformation[ 6] = (*axis2)[2];
					axis2++;
					transformation[ 8] = (*axis3)[0];
					transformation[ 9] = (*axis3)[1];
					transformation[10] = (*axis3)[2];
					axis3++;
					transformation[12] = (*point)[0];
					transformation[13] = (*point)[1];
					transformation[14] = (*point)[2];
					point++;
					transformation[ 3] = 0.0;
					transformation[ 7] = 0.0;
					transformation[11] = 0.0;
					transformation[15] = 1.0;
					if(transformed_object = transform_GT_object(glyph,
						transformation))
					{
						set_GT_object_default_material(transformed_object,
							material);
						makewavefront(wavefront_file, 1, transformed_object, time);
						DESTROY(GT_object)(&transformed_object);
					}
				}
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"drawglyphsetGL.  Missing spectrum");
			return_code=0;
		}
	}
	else
	{
		if (0 == number_of_points)
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"draw_glyph_set_vrml. Invalid argument(s)");
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_glyph_set_vrml */

static int draw_surface_wavefront(FILE *file, Triple *surfpts, Triple *normalpts,
	Triple *texturepts, int number_of_data_components, GTDATA *data,
	struct Graphical_material *material, struct Spectrum *spectrum,
	int npts1,int npts2, struct LIST(Wavefront_vertex) *vertex_list)
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
==============================================================================*/
{
	int i,j,index,npts12,return_code, *vertex_index_array, *vertex_index;
	struct Wavefront_vertex *vertex;
	struct Wavefront_vertex_position position;

	ENTER(draw_surface_wavefront);
	/* Keep a similar interface to all the other render implementations */
	USE_PARAMETER(normalpts);
	USE_PARAMETER(number_of_data_components);
	USE_PARAMETER(data);
	USE_PARAMETER(spectrum);
	if (surfpts&&(1<npts1)&&(1<npts2))
	{
		npts12 = npts1 * npts2;
		activate_material_wavefront( file, material );
		ALLOCATE(vertex_index_array,int,npts12);
		vertex_index = vertex_index_array;
		for (i=0;i<npts1;i++)
		{
			for (j=0;j<npts2;j++)
			{
				if (vertex_list&&((i==0)||(i==(npts1-1))||(j==0)||(j==(npts2-1))))
				{
					position.x = surfpts[i+npts1*j][0];
					position.y = surfpts[i+npts1*j][1];
					position.z = surfpts[i+npts1*j][2];
					if (vertex = FIND_BY_IDENTIFIER_IN_LIST(Wavefront_vertex,position)(&position,
						vertex_list))
					{
						*vertex_index = vertex->index;
					}
					else
					{
						fprintf(file, "v %f %f %f\n",
							surfpts[i+npts1*j][0],
							surfpts[i+npts1*j][1],
							surfpts[i+npts1*j][2]);
						file_vertex_index++;
						*vertex_index = file_vertex_index;
						if (vertex = CREATE(Wavefront_vertex)(file_vertex_index,
							surfpts[i+npts1*j][0], surfpts[i+npts1*j][1],
							surfpts[i+npts1*j][2]))
						{
							ADD_OBJECT_TO_LIST(Wavefront_vertex)(vertex, vertex_list);
						}
					}
				}
				else
				{
					fprintf(file, "v %f %f %f\n",
						surfpts[i+npts1*j][0],
						surfpts[i+npts1*j][1],
						surfpts[i+npts1*j][2]);
					file_vertex_index++;
					*vertex_index = file_vertex_index;
				}
				vertex_index++;
			}
		}
#if defined (OLD_CODE)
		/* Normals are not used well by Maya so not bothering to put
			them out */
		if (normalpts)
		{
			for (i=0;i<npts1;i++)
			{
				for (j=0;j<npts2;j++)
				{
					fprintf(file, "vn %f %f %f\n",
						normalpts[i+npts1*j][0],
						normalpts[i+npts1*j][1],
						normalpts[i+npts1*j][2]);
					file_normal_vertex_index++;
				}
			}
		}
#endif /* defined (OLD_CODE) */
		if (texturepts)
		{
			for (i=0;i<npts1;i++)
			{
				for (j=0;j<npts2;j++)
				{
					fprintf(file, "vt %f %f %f\n",
						texturepts[i+npts1*j][0],
						texturepts[i+npts1*j][1],
						texturepts[i+npts1*j][2]);
					file_texture_vertex_index++;
				}
			}
		}

		index = file_texture_vertex_index-npts1*npts2+1;
		vertex_index = vertex_index_array;
		for (i=0;i<npts1-1;i++)
		{
			for (j=0;j<npts2-1;j++)
			{
				if (texturepts)
				{
					fprintf(file, "f %d/%d %d/%d %d/%d %d/%d\n",
						*vertex_index, index,
						*(vertex_index + 1), (index + 1),
						*(vertex_index + npts2 + 1), (index + npts2 + 1),
						*(vertex_index + npts2), (index + npts2));
				}
				else
				{
					fprintf(file, "f %d %d %d %d\n",
						*vertex_index,
						*(vertex_index + 1),
						*(vertex_index + npts2 + 1),
						*(vertex_index + npts2));
				}
				vertex_index++;
				index++;
			}
			vertex_index++;
			index++;
		}
		return_code=1;
	}
	else
	{
		if ((1<npts1)&&(1<npts2))
		{
			display_message(ERROR_MESSAGE,
				"draw_surface_wavefront.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}


	LEAVE;

	return (return_code);
} /* draw_surface_wavefront */

static int drawvoltexwavefront(FILE *out_file, int full_comments,
	int n_iso_polys,int *triangle_list,
	struct VT_iso_vertex *vertex_list,int n_vertices,int n_rep,
	struct Graphical_material **iso_poly_material,
	struct Environment_map **iso_env_map,double *iso_poly_cop,
	float *texturemap_coord,int *texturemap_index,int number_of_data_components,
	struct Graphical_material *default_material, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 2 October 1997

DESCRIPTION :
==============================================================================*/
{
	float vt[3][3];
	int i,ii,j,k,return_code;
	struct Graphical_material *last_material,*next_material, *obj_material;

	ENTER(drawvoltexwavefront);
	/* Keep a similar interface to all the other render implementations */
	USE_PARAMETER(number_of_data_components);
	USE_PARAMETER(iso_poly_cop);
	USE_PARAMETER(default_material);
	USE_PARAMETER(spectrum);
	return_code=0;
	if (triangle_list&&vertex_list&&iso_poly_material&&iso_env_map&&
		texturemap_coord&&texturemap_index&&(0<n_rep)&&(0<n_iso_polys))
	{
		last_material=(struct Graphical_material *)NULL;
		for (ii=0;ii<n_rep;ii++)
		{
			if ( full_comments )
			{
				fprintf(out_file,"#vertex list\n");
			}
			for (i=0;i<n_vertices;i++)
			{
				if ( full_comments )
				{
					fprintf(out_file,"#vertex %d\n",i+1);
				}
				fprintf(out_file,"v %lf %lf %lf\n",vertex_list[i].coord[0],
					vertex_list[i].coord[1],vertex_list[i].coord[2]);
			}
			if ( full_comments )
			{
				fprintf(out_file,"#texture vertices %d\n",n_vertices+1);
			}
			for (i=0;i<n_iso_polys;i++)
			{
				if ( full_comments )
				{
					fprintf(out_file,"#polygon %d\n",i);
				}
				j=0;
				for (k=0;k<3;k++)
				{
					/*	v[j][k]=vertex_list[triangle_list[i*3+0]+n_vertices*ii].coord[k]; */
					vt[j][k]=texturemap_coord[3*(3*i+0)+k];
					/* vn[j][k]=vertex_list[triangle_list[i*3+0]+n_vertices*ii].normal[k]; */
				}
				j=1;
				for (k=0;k<3;k++)
				{
					/* v[j][k]=vertex_list[triangle_list[i*3+2]+n_vertices*ii].coord[k]; */
					vt[j][k]=texturemap_coord[3*(3*i+1)+k];
					/* vn[j][k]=vertex_list[triangle_list[i*3+2]+n_vertices*ii].normal[k]; */
				}
				j=2;
				for(k=0;k<3;k++)
				{
					/* v[j][k]=vertex_list[triangle_list[i*3+1]+n_vertices*ii].coord[k]; */
					vt[j][k]=texturemap_coord[3*(3*i+2)+k];
					/* vn[j][k]=vertex_list[triangle_list[i*3+1]+n_vertices*ii].normal[k]; */
				}
				/* print to obj file */
				/* Note these vertices are probbaly excessive - It puts out a differnt one for each vertex,
				* which is cool if different textures are defined on different polygons,  but less than
				* efficient if only one big texture map is used. To think about...
				*/
				for (j=0;j<3;j++)
				{
					fprintf(out_file,"vt	%f %f %f\n",vt[j][0],vt[j][1],vt[j][2]);
				}
			} /* for i */
			if ( full_comments )
			{
				fprintf(out_file,"#normal list\n");
			}
			for (i=0;i<n_vertices;i++)
			{
				if ( full_comments )
				{
					fprintf(out_file,"#normal %d\n",i+1);
				}
				fprintf(out_file,"vn %lf %lf %lf\n",-(vertex_list[i].normal[0]),
					-(vertex_list[i].normal[1]),-(vertex_list[i].normal[2]));
			}
			for (i=0;i<n_iso_polys;i++)
			{
				if ( full_comments )
				{
					fprintf(out_file,"# polygon %d\n",i+1);
				}
				/* if an environment map exists use it in preference to a material */
				/*???MS.  What am I doing with these crazy index orders? */
				/* I don't know how to implement a different material on each
					vertex so I only evaluate the iso_env_map at the first vertex */
				if (iso_env_map[i*3])
				{
					if (iso_env_map[i*3]->face_material[texturemap_index[i*3]])
					{
						if (last_material!=(next_material=iso_env_map[i*3]->
							face_material[texturemap_index[i*3]]))
						{
							obj_material=next_material;
							last_material=next_material;
							fprintf(out_file, "usemtl %s\n",
								Graphical_material_name(obj_material));
						}
					}
				}
				else
				{
					if (last_material!=(next_material=iso_poly_material[i*3]))
					{
						obj_material=next_material;
						last_material=next_material;
						fprintf(out_file, "usemtl %s\n",
							Graphical_material_name(obj_material));
					}
				}
				fprintf(out_file,"f   %d/%d/%d  %d/%d/%d  %d/%d/%d\n",
					triangle_list[i*3+0]+file_vertex_index+1,
					3*i+0+file_texture_vertex_index+1,
					triangle_list[i*3+0]+file_normal_vertex_index+1,
					triangle_list[i*3+1]+file_vertex_index+1,
					3*i+1+file_texture_vertex_index+1,
					triangle_list[i*3+1]+file_normal_vertex_index+1,
					triangle_list[i*3+2]+file_vertex_index+1,
					3*i+2+file_texture_vertex_index+1,
					triangle_list[i*3+2]+file_normal_vertex_index+1);
			} /* for i */
			file_vertex_index += n_vertices;
			file_normal_vertex_index += n_vertices;
			file_texture_vertex_index += 3 * n_iso_polys;
		} /* for ii */
	}
	else
	{
		if ((0<n_rep)&&(0<n_iso_polys))
		{
			display_message(ERROR_MESSAGE,"drawvoltexwavefront.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* drawvoltexwavefront */

int drawnurbswavefront(FILE *file, struct GT_nurbs *nurbptr)
/*******************************************************************************
LAST MODIFIED : 9 March 1999

DESCRIPTION :
==============================================================================*/
{
	int i, number_of_control_points, return_code;

	ENTER(drawnurbsGL);

	return_code=0;
	if (nurbptr)
	{
		number_of_control_points = nurbptr->maxs * nurbptr->maxt;
		for( i = 0 ; i < number_of_control_points ; i++)
		{
			if(nurbptr->controlpts[4 * i + 3] == 1.0)
			{
				fprintf(file, "v %f %f %f\n",
					nurbptr->controlpts[4 * i],
					nurbptr->controlpts[4 * i + 1],
					nurbptr->controlpts[4 * i + 2]);
			}
			else
			{
				fprintf(file, "v %f %f %f %f\n",
					nurbptr->controlpts[4 * i],
					nurbptr->controlpts[4 * i + 1],
					nurbptr->controlpts[4 * i + 2],
					nurbptr->controlpts[4 * i + 3]);
			}
		}
		if (nurbptr->texture_control_points)
		{
			for( i = 0 ; i < number_of_control_points ; i++)
			{
				if(nurbptr->texture_control_points[4 * i + 3] == 1.0)
				{
					fprintf(file, "vt %f %f %f\n",
						nurbptr->texture_control_points[4 * i],
						nurbptr->texture_control_points[4 * i + 1],
						nurbptr->texture_control_points[4 * i + 2]);
				}
				else
				{
					fprintf(file, "vt %f %f %f %f\n",
						nurbptr->texture_control_points[4 * i],
						nurbptr->texture_control_points[4 * i + 1],
						nurbptr->texture_control_points[4 * i + 2],
						nurbptr->texture_control_points[4 * i + 3]);
				}
			}
		}
		fprintf(file, "cstype rat bspline\n");
		fprintf(file, "deg %d %d\n", nurbptr->sorder - 1, nurbptr->torder - 1);
		fprintf(file, "surf %f %f %f %f",
			nurbptr->sknots[0], nurbptr->sknots[nurbptr->sknotcnt - 1],
			nurbptr->tknots[0], nurbptr->tknots[nurbptr->tknotcnt - 1]);
		if (!nurbptr->texture_control_points)
		{
			for( i = 0 ; i < number_of_control_points ; i++)
			{
				fprintf(file, " %d", file_vertex_index + i + 1);
			}
		}
		else
		{
			for( i = 0 ; i < number_of_control_points ; i++)
			{
				fprintf(file, " %d/%d", file_vertex_index + i + 1,
					file_vertex_index + i + 1);
			}
		}			
		fprintf(file, "\nparm u");
		for( i = 0 ; i < nurbptr->sknotcnt ; i++)
		{
			fprintf(file, " %f", nurbptr->sknots[i]);
		}
		fprintf(file, "\nparm v");
		for( i = 0 ; i < nurbptr->tknotcnt ; i++)
		{
			fprintf(file, " %f", nurbptr->tknots[i]);
		}
		fprintf(file, "\nend\n");
  
		file_vertex_index += number_of_control_points;

		if (nurbptr->cknotcnt>0)
		{
			display_message(ERROR_MESSAGE,"drawnurbswavefront.	Trimming not yet implemented");
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"drawnurbsGL.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* drawnurbsGL */

static int makewavefront(FILE *wavefront_file, int full_comments,
	gtObject *object, float time)
/*******************************************************************************
LAST MODIFIED : 8 October 1997

DESCRIPTION :
Convert graphical object into Wavefront object file.

==============================================================================*/
{
	float proportion,*times;
	int itime,return_code;
	struct GT_glyph_set *interpolate_glyph_set,*glyph_set,*glyph_set_2;
	struct GT_nurbs *nurbs;
	struct GT_surface *interpolate_surface,*surface,*surface_2;
	struct GT_voltex *voltex;
	struct LIST(Wavefront_vertex) *vertex_list;

	ENTER(makewavefront);

	/* check arguments */
	return_code = 1;
	if (object)
	{
		if ((itime=object->number_of_times)>0)
		{
			if ((itime>1)&&(times=object->times))
			{
				itime--;
				times += itime;
				if (time>= *times)
				{
					proportion=0;
				}
				else
				{
					while ((itime>0)&&(time< *times))
					{
						itime--;
						times--;
					}
					if (time< *times)
					{
						proportion=0;
					}
					else
					{
						proportion=times[1]-times[0];
						if (proportion>0)
						{
							proportion=time-times[0]/proportion;
						}
						else
						{
							proportion=0;
						}
					}
				}
			}
			else
			{
				itime=0;
				proportion=0;
			}
			switch (object->object_type)
			{
				case g_GLYPH_SET:
				{
					if (glyph_set=(object->gu.gt_glyph_set)[itime])
					{
						if (proportion>0)
						{
							glyph_set_2=(object->gu.gt_glyph_set)[itime+1];
							while (glyph_set&&glyph_set_2)
							{
								if (interpolate_glyph_set=morph_GT_glyph_set(proportion,
									glyph_set,glyph_set_2))
								{
									draw_glyph_set_wavefront(wavefront_file,
										interpolate_glyph_set->number_of_points,
										interpolate_glyph_set->point_list,
										interpolate_glyph_set->axis1_list,
										interpolate_glyph_set->axis2_list,
										interpolate_glyph_set->axis3_list,
										interpolate_glyph_set->glyph,
										interpolate_glyph_set->labels,
										interpolate_glyph_set->n_data_components,
										interpolate_glyph_set->data,
										object->default_material,object->spectrum,
									   time);
									DESTROY(GT_glyph_set)(&interpolate_glyph_set);
								}
								glyph_set=glyph_set->ptrnext;
								glyph_set_2=glyph_set_2->ptrnext;
							}
						}
						else
						{
							while (glyph_set)
							{
								draw_glyph_set_wavefront(wavefront_file,
									glyph_set->number_of_points,
									glyph_set->point_list,glyph_set->axis1_list,
									glyph_set->axis2_list,glyph_set->axis3_list,glyph_set->glyph,
									glyph_set->labels,glyph_set->n_data_components,
									glyph_set->data,object->default_material,object->spectrum,
									time);
								glyph_set=glyph_set->ptrnext;
							}
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makewavefront.  Missing glyph_set");
						return_code=0;
					}
				} break;
				case g_VOLTEX:
				{
					if (voltex=(object->gu.gt_voltex)[itime])
					{
						while (voltex)
						{
							drawvoltexwavefront(wavefront_file, full_comments,
								voltex->n_iso_polys,voltex->triangle_list,
								voltex->vertex_list,voltex->n_vertices,voltex->n_rep,
								voltex->iso_poly_material,voltex->iso_env_map,
								voltex->iso_poly_cop, voltex->texturemap_coord,
								voltex->texturemap_index,voltex->n_data_components,
								object->default_material, object->spectrum);
							voltex=voltex->ptrnext;
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing voltex");
						return_code=0;
					}
				} break;
				case g_SURFACE:
				{
					if (surface=(object->gu.gt_surface)[itime])
					{
						if (proportion>0)
						{
							surface_2=(object->gu.gt_surface)[itime+1];
						}
						switch (surface->surface_type)
						{
							case g_SHADED:
							case g_SHADED_TEXMAP:
							{
								vertex_list = CREATE_LIST(Wavefront_vertex)();
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										if (interpolate_surface=morph_GT_surface(proportion,
											surface,surface_2))
										{
											draw_surface_wavefront(
												wavefront_file,
												interpolate_surface->pointlist,
												interpolate_surface->normallist,
												interpolate_surface->texturelist,
												interpolate_surface->n_data_components,
												interpolate_surface->data,
												object->default_material, object->spectrum,
												interpolate_surface->n_pts1,
												interpolate_surface->n_pts2,
												vertex_list);
											DESTROY(GT_surface)(&interpolate_surface);
										}
										surface=surface->ptrnext;
										surface_2=surface_2->ptrnext;
									}
								}
								else
								{
									{
										while (surface)
										{
											draw_surface_wavefront(wavefront_file,
												surface->pointlist,surface->normallist,
												surface->texturelist, surface->n_data_components,
												surface->data,object->default_material,
												object->spectrum,surface->n_pts1,
												surface->n_pts2,
												vertex_list);
											surface=surface->ptrnext;
										}
									}
								}
								DESTROY_LIST(Wavefront_vertex)(&vertex_list);
								return_code=1;
							} break;
							case g_SH_DISCONTINUOUS:
							case g_SH_DISCONTINUOUS_TEXMAP:
							{
								if (g_NO_DATA==surface->n_data_components)
								{
									if (proportion>0)
									{
										while (surface&&surface_2)
										{
											if (interpolate_surface=morph_GT_surface(proportion,
												surface,surface_2))
											{
/* 												draw_dc_surfaceGL(interpolate_surface->pointlist, */
/* 													interpolate_surface->n_pts1, */
/* 													interpolate_surface->n_pts2); */
												DESTROY(GT_surface)(&interpolate_surface);
											}
											surface=surface->ptrnext;
											surface_2=surface_2->ptrnext;
										}
									}
									else
									{
										while (surface)
										{
/* 											draw_dc_surfaceGL(surface->pointlist,surface->n_pts1, */
/* 												surface->n_pts2); */
											surface=surface->ptrnext;
										}
									}
								}
								else
								{
									if (proportion>0)
									{
										while (surface&&surface_2)
										{
											if (interpolate_surface=morph_GT_surface(proportion,
												surface,surface_2))
											{
/* 												draw_data_dc_surfaceGL(interpolate_surface->pointlist, */
/* 													interpolate_surface->data, */
/* 													object->default_material, object->spectrum, */
/* 													interpolate_surface->n_pts1, */
/* 													interpolate_surface->n_pts2); */
												DESTROY(GT_surface)(&interpolate_surface);
											}
											surface=surface->ptrnext;
											surface_2=surface_2->ptrnext;
										}
									}
									else
									{
										while (surface)
										{
/* 											draw_data_dc_surfaceGL(surface->pointlist,surface->data, */
/* 												object->default_material, object->spectrum, */
/* 												surface->n_pts1,surface->n_pts2); */
											surface=surface->ptrnext;
										}
									}
								}
								return_code=1;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"makewavefront.  Invalid surface type");
								return_code=0;
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makewavefront.  Missing surface");
						return_code=0;
					}
				} break;
				case g_NURBS:
				{
					if (nurbs=(object->gu.gt_nurbs)[itime])
					{
						return_code=1;
						while(return_code && nurbs)
						{
							return_code = drawnurbswavefront(wavefront_file, nurbs);
							nurbs=nurbs->ptrnext;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makewavefront.  Missing nurbs");
						return_code=0;
					}
				} break;
				case g_USERDEF:
				{
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"makewavefront.  Invalid object type");
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"makewavefront.  Missing object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* makewavefront */

struct Export_to_wavefront_data
{
	char *base_filename;
	FILE *wavefront_file;
	int full_comments;
	struct Scene *scene;
}; /* struct Export_to_wavefront_data */

static int graphics_object_export_to_wavefront(
	struct GT_object *gt_object, double time, void *export_to_wavefront_data_void)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Write the window object to the <wavefront_file_void>.
==============================================================================*/
{
	char filename[150];
	FILE *wavefront_global_file, *wavefront_object_file;
	int return_code;
	struct Export_to_wavefront_data *export_to_wavefront_data;

	ENTER(graphics_object_export_to_wavefront);
	/* check arguments */
	if (gt_object && (export_to_wavefront_data=
		(struct Export_to_wavefront_data *)export_to_wavefront_data_void))
	{
		wavefront_global_file = export_to_wavefront_data->wavefront_file;
		switch(gt_object->object_type)
		{
			case g_GLYPH_SET:
			case g_VOLTEX:
			case g_SURFACE:
			case g_NURBS:
			{
				sprintf(filename,"%s_%s.obj",export_to_wavefront_data->base_filename,
					gt_object->name);
				fprintf(wavefront_global_file,"call %s\n",filename);
					
				if ( wavefront_object_file = fopen(filename, "w"))
				{
					fprintf(wavefront_object_file,
						"# CMGUI Wavefront Object file generator\n#%s \n",filename);
					fprintf(wavefront_object_file,"mtllib global.mtl\n\n");
					file_vertex_index = 0;
					file_normal_vertex_index = 0;
					file_texture_vertex_index = 0;
					return_code=makewavefront(wavefront_object_file,
						export_to_wavefront_data->full_comments,
						gt_object, time);
					fclose(wavefront_object_file);
				}
				else
				{
					display_message(ERROR_MESSAGE,"graphics_object_export_to_wavefront.  "
						"Could not open wavefront object file %s", filename);
					return_code=0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"graphics_object_export_to_wavefront.  "
					"The graphics object %s is of a type not yet supported", gt_object->name);
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"graphics_object_export_to_wavefront.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* graphics_object_export_to_wavefront */

/*
Global functions
----------------
*/
int export_to_wavefront(char *file_name,struct Scene *scene,
	struct Scene_object *scene_object, int full_comments)
/******************************************************************************
LAST MODIFIED : 15 May 1998

DESCRIPTION :
Renders the visible objects to Wavefront object files.
==============================================================================*/
{
	char *extension;
	float time;
	FILE *wavefront_global_file, *wavefront_object_file;
	int return_code;
	struct Export_to_wavefront_data export_to_wavefront_data;

	ENTER(export_to_wavefront);
	/* checking arguments */
	if (scene)
	{
		if (!scene_object)
		{
			if (1==Scene_get_number_of_scene_objects(scene))
			{
				scene_object = Scene_get_first_scene_object_that(scene,
					(LIST_CONDITIONAL_FUNCTION(Scene_object) *)NULL, NULL);
			}
		}
		if (scene_object && Scene_object_has_gt_object(scene_object,
			(struct GT_object *)NULL))
		{
			/* Write just this graphics object */
			if ( wavefront_object_file = fopen(file_name, "w"))
			{
				fprintf(wavefront_object_file,
					"# CMGUI Wavefront Object file generator\n#%s \n",file_name);
				fprintf(wavefront_object_file,"mtllib global.mtl\n\n");
				file_vertex_index = 0;
				file_normal_vertex_index = 0;
				file_texture_vertex_index = 0;
				if(Scene_object_has_time(scene_object))
				{
					time = Scene_object_get_time(scene_object);
				}
				else
				{
					time = 0.0;
				}
				return_code=makewavefront(wavefront_object_file,
					full_comments, Scene_object_get_gt_object(scene_object),
					time);
				fclose(wavefront_object_file);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"export_to_wavefront.  "
					"Could not open wavefront object file %s", file_name);
				return_code=0;
			}
		}
		else
		{
			/* Write all the graphics objects in the scene */
			/* Open file and add header */
			if (wavefront_global_file = fopen(file_name, "w"))
			{
				/*???debug */
				display_message(WARNING_MESSAGE,
					"export_to_wavefront.  Not fully implemented");

				fprintf(wavefront_global_file,
					"# CMGUI Wavefront Object file generator\n");
				/* Transform.... */

				/* Draw objects */

				export_to_wavefront_data.wavefront_file=wavefront_global_file;
				export_to_wavefront_data.scene=scene;
				if ( extension = strrchr ( file_name, '.' ))
				{
					*extension = 0;
				}
				export_to_wavefront_data.base_filename = file_name;
				export_to_wavefront_data.full_comments = full_comments;
				return_code=for_each_graphics_object_in_scene(scene,
					graphics_object_export_to_wavefront,(void *)&export_to_wavefront_data);
				/* set lights... */

				fclose (wavefront_global_file);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"export_to_wavefront.  Could not open wavefront global file");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"export_to_wavefront.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return( return_code);
} /* export_to_wavefront */
