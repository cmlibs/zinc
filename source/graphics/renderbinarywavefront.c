/*******************************************************************************
FILE : renderbinarywavefront.c

LAST MODIFIED : 27 November 2001

DESCRIPTION :
Renders gtObjects to Wavefront OBJ file
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "general/debug.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_window.h"
#include "graphics/material.h"
#include "graphics/renderwavefront.h"
#include "graphics/renderbinarywavefront.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
struct vertex
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
This is the data structure defined by the plugin written by Patrick Palmer for MAYA
==============================================================================*/
{
	float x, y, z, pad;
};

struct Binary_wavefront_data
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
This is the data structure defined by the plugin written by Patrick Palmer for MAYA
==============================================================================*/
{
	int number_of_vertices;
	struct vertex *vertices;
	int number_of_polygons;
	int *polygons;                     /* Number of connections for this polygon */
	int number_of_polygon_connections; /* Total number of connections for all polygons */
	int *connections;
	int number_of_texture_coordinates;
	float *u_texture_coordinates;
	float *v_texture_coordinates;
	int number_of_texture_connections;
	int *texture_connections;
	int number_of_groups; /* Not generating any groups yet so this is zero */
};

/*
Module functions
----------------
*/
static int activate_material_wavefront(struct Binary_wavefront_data *data,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Writes Wavefront object file that defines the material
==============================================================================*/
{
	int return_code;

	ENTER(activate_material_wavefront);
	if (material&&data)
	{
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"activate_material_wavefront.  Missing material or data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* activate_material_wavefront */

#if defined (OLD_CODE)
static int spectrum_start_renderwavefront(struct Binary_wavefront_data *data, struct Spectrum *spectrum, struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 1 October 1997

DESCRIPTION :
Sets WAVEFRONT file for rendering values on the current material.
==============================================================================*/
{
	int return_code;
	ENTER(spectrum_start_renderwavefront);

	USE_PARAMETER(data);
	USE_PARAMETER(material);
	if ( spectrum )
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
							"spectrum_start_renderwavefront.  Invalid spectrum object.");
		return_code=0;
	}

	LEAVE;
	return (return_code);
} /* spectrum_start_renderwavefront */

static int spectrum_renderwavefront_value(struct Binary_wavefront_data *data, struct Spectrum *spectrum,
	struct Graphical_material *material, float data_value)
/*******************************************************************************
LAST MODIFIED :  1 October 1997

DESCRIPTION :
Writes WAVEFRONT to represent the value 'data' in
accordance with the spectrum.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_renderwavefront_value);
	USE_PARAMETER(data);
	USE_PARAMETER(data_value);
	if ( spectrum && material )
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"spectrum_renderwavefront_value.  Invalid arguments given.");
		return_code = 0;
	}

	LEAVE;
	return (return_code);
} /* spectrum_renderwavefront_value */
#endif /* defined (OLD_CODE) */

#ifdef MERGE_TIMES
static int spectrum_renderwavefront_merge(struct Binary_wavefront_data *data, struct Spectrum *spectrum,
				struct Graphical_material *material,
				int merge_number, float *data)
/*******************************************************************************
LAST MODIFIED : 1 October 1997

DESCRIPTION :
Writes wavefront to represent the vector of values 'data'
in accordance with the spectrum.
==============================================================================*/
{
	enum Spectrum_type type;
	float alpha,blue,green,red;
	int return_code;

	ENTER(spectrum_renderwavefront_merge);

	if ( spectrum )
	{
		get_Spectrum_type(spectrum,&type);
		if (MERGE_RGB_SPECTRUM==type)
		{
			if ( merge_number > 2 && data )
			{
				display_message(ERROR_MESSAGE,
									"spectrum_renderwavefront_merge. Not yet implemented.");
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
									"spectrum_renderwavefront_merge.  Require three merge times.");
				return_code=0;
			}
		}
		else
		{
		display_message(ERROR_MESSAGE,
							"spectrum_renderwavefront_merge.  Invalid spectrum type for merge.");
		return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
							"spectrum_renderwavefront_merge.  Invalid spectrum object.");
		return_code=0;
	}

	LEAVE;
	return (return_code);
} /* spectrum_renderwavefront_merge */
#endif

#if defined (OLD_CODE)
static int spectrum_end_renderwavefront(struct Binary_wavefront_data *data, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 1 October 1997

DESCRIPTION :
Resets the graphics state after rendering values on current material.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_end_renderGL);
	USE_PARAMETER(data);
	if ( spectrum )
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
							"spectrum_end_renderwavefront.  Invalid spectrum object.");
		return_code=0;
	}

	LEAVE;
	return (return_code);
} /* spectrum_end_renderwavefront */
#endif /* defined (OLD_CODE) */

static int draw_surface_wavefront(struct Binary_wavefront_data *data, Triple *surfpts,
	Triple *normalpts, Triple *texturepts,
	struct Graphical_material *material, int npts1,int npts2)
/*******************************************************************************
LAST MODIFIED : 1 October 1997

DESCRIPTION :
==============================================================================*/
{
	float *u_texture_index, *v_texture_index;
	int i,j,nptsij,return_code, *connection_index, *polygon_index, *texture_index;
	int index;
	struct vertex *point_index;

	ENTER(draw_surface_texmapwavefront);
	USE_PARAMETER(normalpts);
	if (surfpts&&(1<npts1)&&(1<npts2))
	{
		activate_material_wavefront( data, material );

		nptsij = npts1 * npts2;
		REALLOCATE(data->vertices, data->vertices, struct vertex,
			data->number_of_vertices + nptsij );
		point_index = data->vertices + data->number_of_vertices;
		for (i=0;i<npts1;i++)
		{
			for (j=0;j<npts2;j++)
			{
				point_index->x = surfpts[i+npts1*j][0];
				point_index->y = surfpts[i+npts1*j][1];
				point_index->z = surfpts[i+npts1*j][2];
				point_index->pad = 1;
				point_index++;
			}
		}
		REALLOCATE(data->u_texture_coordinates, data->u_texture_coordinates,
			float, data->number_of_texture_coordinates + nptsij);
		REALLOCATE(data->v_texture_coordinates, data->v_texture_coordinates,
			float, data->number_of_texture_coordinates + nptsij);
		u_texture_index = data->u_texture_coordinates + data->number_of_texture_coordinates;
		v_texture_index = data->v_texture_coordinates + data->number_of_texture_coordinates;
		for (i=0;i<npts1;i++)
		{
			for (j=0;j<npts2;j++)
			{
				*u_texture_index = texturepts[i+npts1*j][0];
				*v_texture_index = texturepts[i+npts1*j][1];
				u_texture_index++;
				v_texture_index++;
			}
		}
		
		REALLOCATE(data->connections, data->connections, int,
			data->number_of_polygon_connections + 4 * (npts1-1) * (npts2-1));
		connection_index = data->connections + data->number_of_polygon_connections;
		data->number_of_polygon_connections += 4 * (npts1-1) * (npts2-1);

		REALLOCATE(data->texture_connections, data->texture_connections, int,
			data->number_of_texture_connections + 4 * (npts1-1) * (npts2-1));
		texture_index = data->texture_connections + data->number_of_texture_connections;
		data->number_of_texture_connections += 4 * (npts1-1) * (npts2-1);
		
		REALLOCATE(data->polygons, data->polygons, int,
			data->number_of_polygons + (npts1-1) * (npts2-1));
		polygon_index = data->polygons + data->number_of_polygons;
		data->number_of_polygons += (npts1-1) * (npts2-1);

		index = 0;
		for (i=0;i<npts1-1;i++)
		{
			for (j=0;j<npts2-1;j++)
			{
				*polygon_index = 4;
				polygon_index++;

				*connection_index = data->number_of_vertices + index;
				*texture_index = data->number_of_texture_coordinates + index;
				connection_index++;
				texture_index++;
				*connection_index = data->number_of_vertices + index + 1;
				*texture_index = data->number_of_texture_coordinates + index + 1;
				connection_index++;
				texture_index++;
				*connection_index = data->number_of_vertices + index + npts2 + 1;
				*texture_index = data->number_of_texture_coordinates + index + npts2 + 1;
				connection_index++;
				texture_index++;
				*connection_index = data->number_of_vertices + index + npts2;
				*texture_index = data->number_of_texture_coordinates + index + npts2;
				connection_index++;
				texture_index++;

				index++;
			}
			index++;
		}
		data->number_of_vertices += nptsij;
		data->number_of_texture_coordinates += nptsij;
		return_code=1;
	}
	else
	{
		if ((1<npts1)&&(1<npts2))
		{
			display_message(ERROR_MESSAGE,
				"draw_surface_texmapwavefront.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_surface_texmapwavefront */

static int draw_voltex_wavefront(struct Binary_wavefront_data *data,
	int full_write, int n_iso_polys,int *triangle_list,
	struct VT_iso_vertex *vertex_list,int n_vertices,int n_rep,
	struct Graphical_material **per_vertex_materials,
	int *iso_poly_material_index,
	struct Environment_map **iso_env_map, double *iso_poly_cop,
	float *texturemap_coord,int *texturemap_index,int n_data_components,
	struct Graphical_material *default_material, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 8 March 2002

DESCRIPTION :
==============================================================================*/
{
	float vt[3][3], *u_texture_index, *v_texture_index;
	int i,ii,j,k,return_code, *connection_index, *polygon_index, *texture_index;
	struct Graphical_material *last_material,*next_material;
	struct vertex *point_index;

	ENTER(draw_voltex_wavefront);
	USE_PARAMETER(full_write);
	USE_PARAMETER(iso_poly_cop);
	USE_PARAMETER(n_data_components);
	USE_PARAMETER(default_material);
	USE_PARAMETER(spectrum);
	/* default return code */
	return_code=0;
	if (triangle_list && vertex_list &&
		((!iso_poly_material_index) || per_vertex_materials) && iso_env_map &&
		texturemap_coord&&texturemap_index&&(0<n_rep)&&(0<n_iso_polys))
	{
		last_material=(struct Graphical_material *)NULL;
		for (ii=0;ii<n_rep;ii++)
		{
			REALLOCATE(data->vertices, data->vertices, struct vertex, data->number_of_vertices + n_vertices);
			point_index = data->vertices + data->number_of_vertices;
			for (i=0;i<n_vertices;i++)
			{
				point_index->x = vertex_list[i].coord[0];
				point_index->y = vertex_list[i].coord[1];
				point_index->z = vertex_list[i].coord[2];
				point_index->pad = 1;
				point_index++;
			}
			REALLOCATE(data->u_texture_coordinates, data->u_texture_coordinates,
				float, data->number_of_texture_coordinates + n_iso_polys * 3);
			REALLOCATE(data->v_texture_coordinates, data->v_texture_coordinates,
				float, data->number_of_texture_coordinates + n_iso_polys * 3);
			u_texture_index = data->u_texture_coordinates + data->number_of_texture_coordinates;
			v_texture_index = data->v_texture_coordinates + data->number_of_texture_coordinates;
			for (i=0;i<n_iso_polys;i++)
			{
				/* if an environment map exists use it in preference to a material */
				/*???MS.  What am I doing with these crazy index orders? */
				/*???RC.  Don't do anything with these materials here anyway! */
				if (iso_env_map[i*3])
				{
					if ((iso_env_map[i*3]->face_material)[texturemap_index[i*3]])
					{
						next_material =
							iso_env_map[i*3]->face_material[texturemap_index[i*3]];
					}
				}
				else
				{
					if (iso_poly_material_index && iso_poly_material_index[i*3])
					{
						next_material =
							per_vertex_materials[iso_poly_material_index[i*3] - 1];
					}
				}
				if (!next_material)
				{
					next_material = default_material;
				}
				if (next_material != last_material)
				{
					last_material = next_material;
				}

				j=0;
				for (k=0;k<3;k++)
				{
					/*	v[j][k]=vertex_list[triangle_list[i*3+0]+n_vertices*ii].coord[k]; */
					vt[j][k]=texturemap_coord[3*(3*i+0)+k];
					/* vn[j][k]=vertex_list[triangle_list[i*3+0]+n_vertices*ii].normal[k]; */
				}

				next_material = default_material;
				if (iso_env_map[i*3+2])
				{
					if (iso_env_map[i*3+2]->face_material[texturemap_index[i*3+2]])
					{
						next_material=
							iso_env_map[i*3+2]->face_material[texturemap_index[i*3+2]];
					}
				}
				else
				{
					if (iso_poly_material_index && iso_poly_material_index[i*3+2])
					{
						next_material =
							per_vertex_materials[iso_poly_material_index[i*3+2] - 1];
					}
				}
				if (!next_material)
				{
					next_material = default_material;
				}
				if (next_material != last_material)
				{
					last_material=next_material;
				}

				j=1;
				for (k=0;k<3;k++)
				{
					/* v[j][k]=vertex_list[triangle_list[i*3+2]+n_vertices*ii].coord[k]; */
					vt[j][k]=texturemap_coord[3*(3*i+1)+k];
					/* vn[j][k]=vertex_list[triangle_list[i*3+2]+n_vertices*ii].normal[k]; */
				}

				next_material=default_material;
				if (iso_env_map[i*3+1])
				{
					if (iso_env_map[i*3+1]->face_material[texturemap_index[i*3+1]])
					{
						next_material=
							iso_env_map[i*3+1]->face_material[texturemap_index[i*3+1]];
					}
				}
				else
				{
					if (iso_poly_material_index && iso_poly_material_index[i*3+1])
					{
						next_material =
							per_vertex_materials[iso_poly_material_index[i*3+1] - 1];
					}
				}
				if (!next_material)
				{
					next_material = default_material;
				}
				if (next_material != last_material)
				{
					last_material = next_material;
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
					*u_texture_index = vt[j][0];
					*v_texture_index = vt[j][1];
					u_texture_index++;
					v_texture_index++;
				}
			} /* for i */

			/* Normals are not currently supported in bobj files */

			REALLOCATE(data->connections, data->connections, int,
				data->number_of_polygon_connections + 3 * n_iso_polys );
			connection_index = data->connections + data->number_of_polygon_connections;
			data->number_of_polygon_connections += 3 * n_iso_polys;

			REALLOCATE(data->texture_connections, data->texture_connections, int,
				data->number_of_texture_connections + 3 * n_iso_polys );
			texture_index = data->texture_connections + data->number_of_texture_connections;
			data->number_of_texture_connections += 3 * n_iso_polys;

			REALLOCATE(data->polygons, data->polygons, int,
				data->number_of_polygons + n_iso_polys );
			polygon_index = data->polygons + data->number_of_polygons;
			data->number_of_polygons += n_iso_polys;

			for (i=0;i<n_iso_polys;i++)
			{
				*polygon_index = 3;
				polygon_index++;

				*connection_index = triangle_list[i*3+0]+data->number_of_vertices;
				*texture_index = 3*i+0+data->number_of_texture_coordinates;
				connection_index++;
				texture_index++;
				*connection_index = triangle_list[i*3+1]+data->number_of_vertices;
				*texture_index = 3*i+1+data->number_of_texture_coordinates;
				connection_index++;
				texture_index++;
				*connection_index = triangle_list[i*3+2]+data->number_of_vertices;
				*texture_index = 3*i+2+data->number_of_texture_coordinates;
				connection_index++;
				texture_index++;
			} /* for i */
			data->number_of_vertices += n_vertices;
			data->number_of_texture_coordinates += n_iso_polys * 3;
		} /* for ii */
	}
	else
	{
		if ((0<n_rep)&&(0<n_iso_polys))
		{
			display_message(ERROR_MESSAGE,"draw_voltex_wavefront.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_voltex_wavefront */

static int make_binary_wavefront(struct Binary_wavefront_data *data, int full_write,
	gtObject *object, float time)
/*******************************************************************************
LAST MODIFIED : 8 March 2002

DESCRIPTION :
Convert graphical object into Wavefront object file.
==============================================================================*/
{
	float proportion, *times;
	int itime, return_code;
	struct GT_pointset *interpolate_point_set,*point_set,*point_set_2;
	struct GT_polyline *interpolate_line,*line,*line_2;
	struct GT_surface *interpolate_surface,*surface,*surface_2;
	struct GT_voltex *voltex;
#if defined (OLD_CODE)
	struct GT_point *point;
	struct GT_nurbs *nurbs;
	struct GT_userdef *userdef;
#endif /* defined (OLD_CODE) */

	ENTER(make_binary_wavefront);
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
				case g_POINT:
				{
					if (/*point=*/(object->gu.gt_point)[itime])
					{
/* 						drawpointGL(point->position,point->text,point->marker_type, */
/* 							point->marker_size,point->data_type,point->data); */
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing point");
						return_code=0;
					}
				} break;
				case g_POINTSET:
				{
					if (point_set=(object->gu.gt_pointset)[itime])
					{
						if (proportion>0)
						{
							point_set_2=(object->gu.gt_pointset)[itime+1];
							while (point_set&&point_set_2)
							{
								if (interpolate_point_set=morph_GT_pointset(proportion,point_set,
									(object->gu.gt_pointset)[itime+1]))
								{
/* 									drawpointsetGL(interpolate_point_set->n_pts, */
/* 										interpolate_point_set->pointlist, */
/* 										interpolate_point_set->text, */
/* 										interpolate_point_set->marker_type, */
/* 										interpolate_point_set->marker_size, */
/* 										interpolate_point_set->data_type, */
/* 										interpolate_point_set->data, */
/* 										object->default_material, object->spectrum); */
									DESTROY(GT_pointset)(&interpolate_point_set);
								}
								point_set=point_set->ptrnext;
								point_set_2=point_set_2->ptrnext;
							}
						}
						else
						{
/* 							drawpointsetGL(point_set->n_pts,point_set->pointlist, */
/* 								point_set->text,point_set->marker_type,point_set->marker_size, */
/* 								point_set->data_type,point_set->data, */
/* 								object->default_material, object->spectrum); */
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing point");
						return_code=0;
					}
				} break;
				case g_VOLTEX:
				{
					if (voltex=(object->gu.gt_voltex)[itime])
					{
						while (voltex)
						{
							draw_voltex_wavefront(data, full_write,
								voltex->n_iso_polys,voltex->triangle_list,
								voltex->vertex_list,voltex->n_vertices,voltex->n_rep,
								voltex->per_vertex_materials,
								voltex->iso_poly_material_index,voltex->iso_env_map,
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
				case g_POLYLINE:
				{
					if (line=(object->gu.gt_polyline)[itime])
					{
						if (proportion>0)
						{
							line_2=(object->gu.gt_polyline)[itime+1];
						}
						switch (line->polyline_type)
						{
							case g_PLAIN:
							{
								if (proportion>0)
								{
									while (line&&line_2)
									{
										if (interpolate_line=morph_GT_polyline(proportion,line,
											line_2))
										{
											/* drawdatapolylineGL(interpolate_line->pointlist, */
											/* interpolate_line->data,object->default_material,  */
											/* object->spectrum, interpolate_line->n_pts); */
											DESTROY(GT_polyline)(&interpolate_line);
										}
										line=line->ptrnext;
										line_2=line_2->ptrnext;
									}
								}
								else
								{
									while (line)
									{
										/* drawdatapolylineGL(line->pointlist,line->data, */
										/* object->default_material, object->spectrum, */
										/* line->n_pts); */
										line=line->ptrnext;
									}
								}
								return_code=1;
							} break;
							case g_PLAIN_DISCONTINUOUS:
							{
								if (proportion>0)
								{
									while (line&&line_2)
									{
										if (interpolate_line=morph_GT_polyline(proportion,line,
											line_2))
										{
											/* draw_data_dc_polylineGL(interpolate_line->pointlist, */
											/* interpolate_line->data, interpolate_line->n_pts, */
											/*  object->default_material, object->spectrum); */
										}
										DESTROY(GT_polyline)(&interpolate_line);
										line=line->ptrnext;
										line_2=line_2->ptrnext;
									}
								}
								else
								{
									while (line)
									{
										/* draw_data_dc_polylineGL(line->pointlist, */
										/* line->data, line->n_pts, object->default_material, */
										/* object->spectrum); */
										line=line->ptrnext;
									}
								}
								return_code=1;
							} break;
							case g_NORMAL_DISCONTINUOUS:
							{
								if (proportion>0)
								{
									while (line&&line_2)
									{
										if (interpolate_line=morph_GT_polyline(proportion,line,
											line_2))
										{
/* 											draw_dc_polyline_n_GL(interpolate_line->pointlist, */
/* 												interpolate_line->n_pts); */
											DESTROY(GT_polyline)(&interpolate_line);
										}
										line=line->ptrnext;
										line_2=line_2->ptrnext;
									}
								}
								else
								{
									while (line)
									{
/* 										draw_dc_polyline_n_GL(line->pointlist,line->n_pts); */
										line=line->ptrnext;
									}
								}
								return_code=1;
							} break;
							case g_NORMAL:
							{
								if (proportion>0)
								{
									while (line&&line_2)
									{
										if (interpolate_line=morph_GT_polyline(proportion,line,
											line_2))
										{
/* 											drawpolylinenormalGL(interpolate_line->pointlist, */
/* 												interpolate_line->n_pts); */
											DESTROY(GT_polyline)(&interpolate_line);
										}
										line=line->ptrnext;
										line_2=line_2->ptrnext;
									}
								}
								else
								{
									while (line)
									{
/* 										drawpolylinenormalGL(line->pointlist,line->n_pts); */
										line=line->ptrnext;
									}
								}
								return_code=1;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"makegtobject.  Invalid line type");
								return_code=0;
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing line");
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
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										if (interpolate_surface=morph_GT_surface(proportion,
											surface,surface_2))
										{
											draw_surface_wavefront(data, interpolate_surface->pointlist,
												interpolate_surface->normallist, interpolate_surface->texturelist,
												object->default_material,
												interpolate_surface->n_pts1,
												interpolate_surface->n_pts2);
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
										draw_surface_wavefront(data, surface->pointlist,
											surface->normallist, surface->texturelist,
											object->default_material,
											surface->n_pts1, surface->n_pts2);
										surface=surface->ptrnext;
									}
								}
								return_code=1;
							} break;
							case g_SH_DISCONTINUOUS:
							{
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										if (interpolate_surface=morph_GT_surface(proportion,
											surface,surface_2))
										{
											/* draw_data_dc_surfaceGL(interpolate_surface->pointlist, */
											/* interpolate_surface->data, */
											/* object->default_material, object->spectrum, */
											/* interpolate_surface->n_pts1, */
											/* interpolate_surface->n_pts2); */
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
										/* draw_data_dc_surfaceGL(surface->pointlist,surface->data, */
										/* object->default_material, object->spectrum, */
										/* surface->n_pts1,surface->n_pts2); */
										surface=surface->ptrnext;
									}
								}
								return_code=1;
							} break;
							case g_SH_DISCONTINUOUS_TEXMAP:
							{
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										if (interpolate_surface=morph_GT_surface(proportion,surface,
											surface_2))
										{
/* 											draw_dc_surface_texmapGL(interpolate_surface->pointlist, */
/* 												interpolate_surface->n_pts1, */
/* 												interpolate_surface->n_pts2); */
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
/* 										draw_dc_surface_texmapGL(surface->pointlist,surface->n_pts1, */
/* 											surface->n_pts2); */
										surface=surface->ptrnext;
									}
								}
								return_code=1;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"make_binary_wavefront.  Invalid surface type");
								return_code=0;
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"make_binary_wavefront.  Missing surface");
						return_code=0;
					}
				} break;
				case g_NURBS:
				{
#if defined (OLD_CODE)
					if (nurbs=(object->gu.gt_nurbs)[itime])
					{
						if ((g_NURBS_NOT_MORPH==nurbs->nurbs_type)||
							(g_NURBS_MORPH==nurbs->nurbs_type))
						{
							if (g_NURBS_MORPH==nurbs->nurbs_type)
							{
								/* perform quadratic interpolation */
								for (i=0;i<nurbs->maxs;i++)
								{
									for (j=0;j<nurbs->maxt;j++)
									{
										for (k=0;k<4;k++)
										{
											(nurbs->controlpts)[4*(i+(nurbs->maxs)*j)+k]=
												2*(1-t)*(.5-t)*
												(nurbs->mcontrolpts)[0][4*(i+(nurbs->maxs)*j)+k]+
												4*t*(1-t)*
												(nurbs->mcontrolpts)[1][4*(i+(nurbs->maxs)*j)+k]+
												2*t*(t-.5)*
												(nurbs->mcontrolpts)[2][4*(i+(nurbs->maxs)*j)+k];
										}
									}
								}
							}
/* 							drawnurbsGL(nurbs); */
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"make_binary_wavefront.  Invalid nurbs_type");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"make_binary_wavefront.  Missing nurbs");
						return_code=0;
					}
#endif /* defined (OLD_CODE) */
					return_code=1;
				} break;
				case g_USERDEF:
				{
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"make_binary_wavefront.  Invalid object type");
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_binary_wavefront.  Missing object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* make_binary_wavefront */

/*
Global functions
----------------
*/
int export_to_binary_wavefront(char *file_name,
	struct Scene_object *scene_object, int number_of_frames, int version,
	int frame_number)
/******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
Renders the visible objects to binary Wavefront object files.
It is currently assumed that the length of each filename inside the bobj
remains a fixed length and the total number of frames is given each time
so that the entire directory structure can be allowed for when the file is
created.  The different versions of Bobj that Patrick Palmer has made are
not compatible and so using this flag it is possible to set version 2 or version 3
A <frame_number> of zero specifies that the data is added after that last valid 
frame, otherwise the <frame_number> specifies that the frame that the current 
data is for (1 to number_of_frames), possibly overwriting old data.  
==============================================================================*/
{
	char buffer[100], *scene_object_name;
	float object_time;
	FILE *binary_wavefront_file;
	int i, frame_one, name_length, number_of_files, number_of_vertices, return_code,
		start_frame, end_frame, dummy;
	long creation_time, file_offset, file_directory_index, file_number_index,
		file_end_index, first_file_index, first_file_offset, end_frame_index;
	struct Binary_wavefront_data data;

	ENTER(export_to_binary_wavefront);
	/* checking arguments */
	if (file_name && number_of_frames && version && (frame_number <= number_of_frames))
	{
		if (version >= 1 && version <=3 )
		{
			if ( scene_object && Scene_object_has_gt_object(scene_object, 
				(struct GT_object *)NULL))
			{
				GET_NAME(Scene_object)(scene_object, &scene_object_name);
				/* Initialise the data structure */
				data.number_of_vertices = 0;
				ALLOCATE(data.vertices, struct vertex, 1 );
				data.number_of_polygons = 0;
				ALLOCATE(data.polygons, int, 1 );
				data.number_of_polygon_connections = 0;
				ALLOCATE(data.connections, int, 1 );
				data.number_of_texture_coordinates = 0;
				ALLOCATE(data.u_texture_coordinates, float, 1 );
				ALLOCATE(data.v_texture_coordinates, float, 1 );
				data.number_of_texture_connections = 0;
				ALLOCATE(data.texture_connections, int, 1 );
				data.number_of_groups = 0;

				if ( binary_wavefront_file = fopen(file_name, "r+") )
				{
					/* File exists */
					fread( buffer, sizeof(char), 4, binary_wavefront_file );
					if ( !strncmp( buffer, "BObj", 4 ))
					{
						fread( &version, sizeof(int), 1, binary_wavefront_file );
						if ( version == 1 || version == 2 || version == 3 )
						{
							fread( &creation_time, sizeof(long), 1, binary_wavefront_file );
							if ( version >= 3 )
							{
								fread( &name_length, sizeof(int), 1, binary_wavefront_file );
								fread( buffer, sizeof(char), name_length, binary_wavefront_file );
								fread ( &start_frame, sizeof(int), 1, binary_wavefront_file );
								end_frame_index = ftell( binary_wavefront_file );
								fread ( &end_frame, sizeof(int), 1, binary_wavefront_file );
								fread ( &dummy, sizeof(int), 1, binary_wavefront_file );
								fread ( &dummy, sizeof(int), 1, binary_wavefront_file );
							}
							file_number_index = ftell( binary_wavefront_file );
							fread( &number_of_files, sizeof(int), 1, binary_wavefront_file );
							if (frame_number)
							{
								frame_number--;
							}
							else
							{
								frame_number = number_of_files;
							}
							if ( frame_number <= number_of_files)
							{
								if ( frame_number < number_of_frames )
								{
									if (frame_number)
									{
										fread( &name_length, sizeof(int), 1, binary_wavefront_file );
										fread( buffer, sizeof(char), name_length, binary_wavefront_file );
										fread( &first_file_offset, sizeof(long), 1, binary_wavefront_file );
										/* Find the place for this directory entry */
										for ( i = 1 ; i < frame_number ; i++ )
										{
											fread( &name_length, sizeof(int), 1, binary_wavefront_file );
											fread( buffer, sizeof(char), name_length, binary_wavefront_file );
											fread( &file_offset, sizeof(long), 1, binary_wavefront_file );
										}
									}
									else
									{
										first_file_index = ftell( binary_wavefront_file );
										fread( &name_length, sizeof(int), 1, binary_wavefront_file );
										fread( buffer, sizeof(char), name_length, binary_wavefront_file );
										fread( &first_file_offset, sizeof(long), 1, binary_wavefront_file );
										fseek ( binary_wavefront_file, first_file_index, SEEK_SET );
									}
									/* The name and the name length should already have been written */
									fread( &name_length, sizeof(int), 1, binary_wavefront_file );
									fread( buffer, sizeof(char), name_length, binary_wavefront_file );

									file_directory_index = ftell( binary_wavefront_file );
									if ( file_directory_index < first_file_offset )
									{
										fseek ( binary_wavefront_file, first_file_offset, SEEK_SET );
										fread ( &number_of_vertices, sizeof(int), 1, binary_wavefront_file );

										fseek ( binary_wavefront_file, 0, SEEK_END );
										file_end_index = ftell( binary_wavefront_file );
										if (frame_number == number_of_files)
										{
											number_of_files++;
											fseek ( binary_wavefront_file, file_number_index, SEEK_SET );
											fwrite( &number_of_files, sizeof(int), 1, binary_wavefront_file );
											if ( version >= 3 )
											{
												end_frame++;
												fseek ( binary_wavefront_file, end_frame_index, SEEK_SET );
												fwrite( &end_frame, sizeof(int), 1, binary_wavefront_file );
											}
											fseek ( binary_wavefront_file, file_directory_index, SEEK_SET );
											fwrite( &file_end_index, sizeof(long), 1, binary_wavefront_file );
											fseek ( binary_wavefront_file, 0, SEEK_END );
										}
										else
										{
											fseek ( binary_wavefront_file, file_directory_index, SEEK_SET );
											fread( &file_offset, sizeof(long), 1, binary_wavefront_file );
											if (file_offset)
											{
												fseek ( binary_wavefront_file, file_offset, SEEK_SET );
											}
											else
											{
												fseek ( binary_wavefront_file, file_directory_index, SEEK_SET );
												fwrite( &file_end_index, sizeof(long), 1, binary_wavefront_file );
												fseek ( binary_wavefront_file, 0, SEEK_END );
											}
										}
									
										if(Scene_object_has_time(scene_object))
										{
											object_time = Scene_object_get_time(scene_object);
										}
										else
										{
											object_time = 0.0;
										}
										make_binary_wavefront( &data, 
											0, Scene_object_get_gt_object(scene_object),
											object_time);

										fwrite( &(data.number_of_vertices), sizeof(int), 1, binary_wavefront_file );
										fwrite( data.vertices, sizeof(struct vertex), data.number_of_vertices,
											binary_wavefront_file );

										if ( data.number_of_vertices != number_of_vertices )
										{
											display_message(ERROR_MESSAGE,"export_to_binary_wavefront.  "
												"Frame exported but inconsistent number of vertices");
										}
										return_code = 1;
									}
									else
									{
										display_message(ERROR_MESSAGE,"export_to_binary_wavefront.  "
											"File %s has insufficient directory space", file_name);
										return_code=0;
									}						
								}
							else
							{
								display_message(ERROR_MESSAGE,"export_to_binary_wavefront.  "
									"File %s already has a full directory", file_name);
								return_code=0;
							}						
							}
							else
							{
								display_message(ERROR_MESSAGE,"export_to_binary_wavefront.  "
									"File %s only contains %d frames", file_name, number_of_files);
								return_code=0;
							}						
						}
						else
						{
							display_message(ERROR_MESSAGE,"export_to_binary_wavefront.  "
								"File %s is a unknown version of a binary object file", file_name);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"export_to_binary_wavefront.  "
							"File %s does not appear to be a binary wavefront file", file_name);
						return_code=0;
					}
					fclose(binary_wavefront_file);
				}
				else
				{
					/* Do a full write */
					if ( binary_wavefront_file = fopen(file_name, "w"))
					{
						sprintf( buffer, "BObj" );
						fwrite( buffer, sizeof(char), 4, binary_wavefront_file );
						fwrite( &version, sizeof(int), 1, binary_wavefront_file );
						creation_time = time(0);
						fwrite( &creation_time, sizeof(long), 1, binary_wavefront_file );
						if ( version >= 3 )
						{
							sprintf( buffer, "%s.####.obj", scene_object_name );
							name_length = strlen( buffer ) + 1;
							fwrite( &name_length, sizeof(int), 1, binary_wavefront_file );
							fwrite( buffer, sizeof(char), name_length, binary_wavefront_file );
							start_frame = 1;
							fwrite( &start_frame, sizeof(int), 1, binary_wavefront_file );
							end_frame = 1;
							fwrite( &end_frame, sizeof(int), 1, binary_wavefront_file );
							dummy = 0;
							fwrite( &dummy, sizeof(int), 1, binary_wavefront_file );
							fwrite( &dummy, sizeof(int), 1, binary_wavefront_file );
						}
						frame_one = 1;
						fwrite( &frame_one, sizeof(int), 1, binary_wavefront_file );
						sprintf( buffer, "%s.%04d.obj", scene_object_name, 1 );
						name_length = strlen( buffer ) + 1;
						fwrite( &name_length, sizeof(int), 1, binary_wavefront_file );
						fwrite( buffer, sizeof(char), name_length, binary_wavefront_file );
						file_directory_index = ftell( binary_wavefront_file );
						file_offset = 0;
						fwrite( &file_offset, sizeof(long), 1, binary_wavefront_file );
						for ( i = 1 ; i < number_of_frames ; i++ )
						{
							sprintf( buffer, "%s.%04d.obj", scene_object_name, i + 1 );
							fwrite( &name_length, sizeof(int), 1, binary_wavefront_file );
							fwrite( buffer, sizeof(char), name_length, binary_wavefront_file );
							fwrite( &file_offset, sizeof(long), 1, binary_wavefront_file );
						}
						first_file_offset = ftell( binary_wavefront_file );
						fseek( binary_wavefront_file, file_directory_index, SEEK_SET );
						fwrite( &first_file_offset, sizeof(long), 1, binary_wavefront_file );
						fseek( binary_wavefront_file, 0, SEEK_END );

						if(Scene_object_has_time(scene_object))
						{
							object_time = Scene_object_get_time(scene_object);
						}
						else
						{
							object_time = 0.0;
						}
						return_code=make_binary_wavefront(&data, 
							1, Scene_object_get_gt_object(scene_object),
							object_time);

						fwrite( &(data.number_of_vertices), sizeof(int), 1, binary_wavefront_file );
						fwrite( data.vertices, sizeof(struct vertex), data.number_of_vertices,
							binary_wavefront_file );
						fwrite( &(data.number_of_polygons), sizeof(int), 1, binary_wavefront_file );
						fwrite( data.polygons, sizeof(int), data.number_of_polygons,
							binary_wavefront_file );
						fwrite( &(data.number_of_polygon_connections), sizeof(int), 1,
							binary_wavefront_file );
						fwrite( data.connections, sizeof(int), data.number_of_polygon_connections,
							binary_wavefront_file );
						fwrite( &(data.number_of_texture_coordinates), sizeof(int), 1,
							binary_wavefront_file );
						fwrite( data.u_texture_coordinates, sizeof(float),
							data.number_of_texture_coordinates, binary_wavefront_file );
						fwrite( data.v_texture_coordinates, sizeof(float),
							data.number_of_texture_coordinates, binary_wavefront_file );
						fwrite( &(data.number_of_texture_connections), sizeof(int), 1,
							binary_wavefront_file );
						fwrite( data.texture_connections, sizeof(int),
							data.number_of_texture_connections, binary_wavefront_file );
						if ( version > 1 )
						{
							if ( data.number_of_groups > 0 )
							{
								display_message(ERROR_MESSAGE,"export_to_binary_wavefront.  "
									"Export of group information not currently supported, exporting zero groups");
								data.number_of_groups = 0;
							}
							fwrite( &(data.number_of_groups), sizeof(int), 1,
								binary_wavefront_file );
						}

						fclose(binary_wavefront_file);
						/* display_message(INFORMATION_MESSAGE,
							"export_to_binary_wavefront.  File %s written, %d vertices", 
							file_name, data.number_of_vertices );*/
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"export_to_binary_wavefront.  "
							"Could not open wavefront object file %s", file_name);
						return_code=0;
					}
				}
				DEALLOCATE(data.vertices);
				DEALLOCATE(data.polygons);
				DEALLOCATE(data.connections);
				DEALLOCATE(data.u_texture_coordinates);
				DEALLOCATE(data.v_texture_coordinates);
				DEALLOCATE(data.texture_connections);
				DEALLOCATE(scene_object_name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"export_to_binary_wavefront.  Scene export not supported for binary files, specify a graphics object");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"export_to_binary_wavefront.  Unsupported version number");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"export_to_binary_wavefront.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return( return_code);
} /* export_to_binary_wavefront */
