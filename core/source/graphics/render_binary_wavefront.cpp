/*******************************************************************************
FILE : renderbinarywavefront.c

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Renders gtObjects to Wavefront OBJ file
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "general/debug.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/renderwavefront.h"
#include "graphics/renderbinarywavefront.h"
#include "graphics/spectrum.h"
#include "user_interface/message.h"
#include "graphics/graphics_object_private.h"

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
	struct Environment_map **per_vertex_environment_maps,
	int *iso_poly_environment_map_index,
	double *iso_poly_cop, int n_data_components,
	struct Graphical_material *default_material, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
==============================================================================*/
{
	float vt[3][3], *u_texture_index, *v_texture_index;
	int i,ii,j,k,return_code, *connection_index, *polygon_index, *texture_index;
	struct Environment_map *environment_map;
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
		((!iso_poly_material_index) || per_vertex_materials) &&
		((!iso_poly_environment_map_index) || per_vertex_environment_maps) &&
		(0<n_rep)&&(0<n_iso_polys))
	{
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

static int make_binary_wavefront(struct Binary_wavefront_data *data,
	int full_write,
	gtObject *object, float time)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Convert graphical object into Wavefront object file.
==============================================================================*/
{
	float proportion, *times;
	int itime, number_of_times, return_code;
	struct GT_pointset *interpolate_point_set,*point_set,*point_set_2;
	struct GT_polyline *interpolate_line,*line,*line_2;
	struct GT_surface *interpolate_surface,*surface,*surface_2;
	struct GT_voltex *voltex;
#if defined (OLD_CODE)
	struct GT_point *point;
	struct GT_nurbs *nurbs;
	struct GT_userdef *userdef;
#endif /* defined (OLD_CODE) */
	union GT_primitive_list *primitive_list1, *primitive_list2;

	ENTER(make_binary_wavefront);
	return_code = 1;
	if (object)
	{
		number_of_times = object->number_of_times;
		if (0 < number_of_times)
		{
			itime = number_of_times;
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
			if (object->primitive_lists &&
				(primitive_list1 = object->primitive_lists + itime))
			{
				if (proportion > 0)
				{
					if (!(primitive_list2 = object->primitive_lists + itime + 1))
					{
						display_message(ERROR_MESSAGE,
							"make_binary_wavefront.  Invalid primitive_list");
						return_code = 0;
					}
				}
				else
				{
					primitive_list2 = (union GT_primitive_list *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"make_binary_wavefront.  Invalid primitive_lists");
				return_code = 0;
			}
		}
		if ((0 < number_of_times) && return_code)
		{
			switch (GT_object_get_type(object))
			{
				case g_POINT:
				{
					if (/*point=*/primitive_list1->gt_point.first)
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
					if (point_set = primitive_list1->gt_pointset.first)
					{
						if (proportion>0)
						{
							point_set_2 = primitive_list2->gt_pointset.first;
							while (point_set && point_set_2)
							{
								if (interpolate_point_set =
									morph_GT_pointset(proportion, point_set, point_set_2))
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
					if (voltex = primitive_list1->gt_voltex.first)
					{
						while (voltex)
						{
							draw_voltex_wavefront(data, full_write,
								voltex->n_iso_polys,voltex->triangle_list,
								voltex->vertex_list,voltex->n_vertices,voltex->n_rep,
								voltex->per_vertex_materials,
								voltex->iso_poly_material_index,
								voltex->per_vertex_environment_maps,
								voltex->iso_poly_environment_map_index,
								voltex->iso_poly_cop, voltex->n_data_components,
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
					if (line = primitive_list1->gt_polyline.first)
					{
						if (proportion>0)
						{
							line_2 = primitive_list2->gt_polyline.first;
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
					if (surface = primitive_list1->gt_surface.first)
					{
						if (proportion>0)
						{
							surface_2 = primitive_list2->gt_surface.first;
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
					if (nurbs = primitive_list1->gt_nurbs.first)
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

