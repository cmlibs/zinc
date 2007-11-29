/*******************************************************************************
FILE : tile_graphics_objects.c

LAST MODIFIED : 29 November 2007

DESCRIPTION :
Tiling routines that split graphics objects based on texture coordinates
and Texture_tiling boundaries.
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
#include <stdio.h>
#include <math.h>
#include "general/debug.h"
#include "graphics/graphics_object.h"
#include "user_interface/message.h"
#include "graphics/graphics_object_private.h"

static struct GT_surface *rendergl_create_GT_surface(struct GT_surface *original_surface,
	int size, int tile_number, int polygon_size)
{
	int return_code;
	struct GT_surface *current_surface;

	if (ALLOCATE(current_surface, struct GT_surface, 1))
	{
		return_code = 1;
		current_surface->n_pts1 = 0;
		current_surface->n_pts2 = polygon_size;
		current_surface->object_name = original_surface->object_name;
		current_surface->n_data_components = original_surface->n_data_components;
		current_surface->surface_type = g_SH_DISCONTINUOUS_TEXMAP;
		current_surface->ptrnext = (struct GT_surface *)NULL;
		switch (polygon_size)
		{
			case 3:
			{
				current_surface->polygon = g_TRIANGLE;
			} break;
			case 4:
			{
				current_surface->polygon = g_QUADRILATERAL;
			} break;
		}
		current_surface->tile_number = tile_number;
		if (!ALLOCATE(current_surface->pointlist, Triple,
				size))
		{
			return_code = 0;
		}
		if (original_surface->normallist)
		{
			if (!ALLOCATE(current_surface->normallist, Triple,
					size))
			{
				return_code = 0;
			}
		}
		else
		{
			current_surface->normallist = (Triple *)NULL;
		}
		if (original_surface->tangentlist)
		{
			if (!ALLOCATE(current_surface->tangentlist, Triple,
					size))
			{
				return_code = 0;
			}
		}
		else
		{
			current_surface->tangentlist = (Triple *)NULL;
		}
		if (!ALLOCATE(current_surface->texturelist, Triple,
				size))
		{
			return_code = 0;
		}
		if (original_surface->data)
		{
			if (!ALLOCATE(current_surface->data, GTDATA,
					original_surface->n_data_components * size))
			{
				return_code = 0;
			}
		}
		else
		{
			current_surface->data = (GTDATA *)NULL;
		}
		if (return_code)
		{
			current_surface->allocated_size = size;
		}
		else
		{
			current_surface = (struct GT_surface *)NULL;
		}
	}
	else
	{
		current_surface = (struct GT_surface *)NULL;
	}
	return(current_surface);
}

static int rendergl_reallocate_GT_surface(struct GT_surface *surface,
	int size)
{
	int return_code;

	return_code = 1;
	if (!REALLOCATE(surface->pointlist, surface->pointlist, Triple,
			size))
	{
		return_code = 0;
	}
	if (surface->normallist)
	{
		if (!REALLOCATE(surface->normallist, surface->normallist, Triple,
				size))
		{
			return_code = 0;
		}
	}
	if (surface->tangentlist)
	{
		if (!REALLOCATE(surface->tangentlist, surface->tangentlist, Triple,
				size))
		{
			return_code = 0;
		}
	}
	if (!REALLOCATE(surface->texturelist, surface->texturelist, Triple,
			size))
	{
		return_code = 0;
	}
	if (surface->data)
	{
		if (!REALLOCATE(surface->data, surface->data, GTDATA,
				surface->n_data_components * size))
		{
			return_code = 0;
		}
	}
	if (return_code)
	{
		surface->allocated_size = size;
	}
	else
	{
		surface->allocated_size = 0;
	}
	return(return_code);
}

static int rendergl_interpolate_triangle(struct GT_surface *new_surface,
	int index_new, struct GT_surface *surface, int index,
	int vertexstart, int vertexi, int vertexj,
	float xi0, float xii, float xij, int stepindex)
{
	int k2, return_code;
	Triple *texturetri, *newtexturetri, *pointtri, *newpointtri,
		*normaltri, *newnormaltri, *tangenttri, *newtangenttri;
	GTDATA *datatri, *newdatatri;

	return_code = 1;
	if (index_new + 2 >= new_surface->allocated_size)
	{
		return_code = rendergl_reallocate_GT_surface(
			new_surface, 2 * new_surface->allocated_size);
	}

	if (return_code)
	{
		texturetri = surface->texturelist;
		newtexturetri = new_surface->texturelist;

		pointtri = surface->pointlist;
		newpointtri = new_surface->pointlist;

		normaltri = surface->normallist;
		newnormaltri = new_surface->normallist;

		tangenttri = surface->tangentlist;
		newtangenttri = new_surface->tangentlist;

		datatri = surface->data;
		newdatatri = new_surface->data;

		for (k2 = 0 ; k2 < 3 ; k2++)
		{
			if (0 == stepindex)
			{
				newtexturetri[index_new][k2] = 
					(1.0 - xi0) * texturetri[index+vertexstart][k2]
					+ xi0 * texturetri[index+vertexi][k2];
			}
			else
			{
				newtexturetri[index_new][k2] = 
					(1.0 - xi0) * texturetri[index+vertexstart][k2]
					+ xi0 * texturetri[index+vertexj][k2];
			}
			newtexturetri[index_new+1][k2] =
				(1.0 - xii) * texturetri[index+vertexstart][k2]
				+ xii * texturetri[index+vertexi][k2];
			newtexturetri[index_new+2][k2] =
				(1.0 - xij) * texturetri[index+vertexstart][k2]
				+ xij * texturetri[index+vertexj][k2];

			if (0 == stepindex)
			{
				newpointtri[index_new][k2] = 
					(1.0 - xi0) * pointtri[index+vertexstart][k2]
					+ xi0 * pointtri[index+vertexi][k2];
			}
			else
			{
				newpointtri[index_new][k2] = 
					(1.0 - xi0) * pointtri[index+vertexstart][k2]
					+ xi0 * pointtri[index+vertexj][k2];
			}
			newpointtri[index_new+1][k2] =
				(1.0 - xii) * pointtri[index+vertexstart][k2]
				+ xii * pointtri[index+vertexi][k2];
			newpointtri[index_new+2][k2] =
				(1.0 - xij) * pointtri[index+vertexstart][k2]
				+ xij * pointtri[index+vertexj][k2];

			if (normaltri)
			{
				if (0 == stepindex)
				{
					newnormaltri[index_new][k2] = 
						(1.0 - xi0) * normaltri[index+vertexstart][k2]
						+ xi0 * normaltri[index+vertexi][k2];
				}
				else
				{
					newnormaltri[index_new][k2] = 
						(1.0 - xi0) * normaltri[index+vertexstart][k2]
						+ xi0 * normaltri[index+vertexj][k2];
				}
				newnormaltri[index_new+1][k2] =
					(1.0 - xii) * normaltri[index+vertexstart][k2]
					+ xii * normaltri[index+vertexi][k2];
				newnormaltri[index_new+2][k2] =
					(1.0 - xij) * normaltri[index+vertexstart][k2]
					+ xij * normaltri[index+vertexj][k2];
			}

			if (tangenttri)
			{
				if (0 == stepindex)
				{
					newtangenttri[index_new][k2] = 
						(1.0 - xi0) * tangenttri[index+vertexstart][k2]
						+ xi0 * tangenttri[index+vertexi][k2];
				}
				else
				{
					newtangenttri[index_new][k2] = 
						(1.0 - xi0) * tangenttri[index+vertexstart][k2]
						+ xi0 * tangenttri[index+vertexj][k2];
				}
				newtangenttri[index_new+1][k2] =
					(1.0 - xii) * tangenttri[index+vertexstart][k2]
					+ xii * tangenttri[index+vertexi][k2];
				newtangenttri[index_new+2][k2] =
					(1.0 - xij) * tangenttri[index+vertexstart][k2]
					+ xij * tangenttri[index+vertexj][k2];
			}
		}
		if (datatri)
		{
			int new_data_index = index_new * surface->n_data_components;
			int data_index = index * surface->n_data_components;
			for (k2 = 0 ; k2 < surface->n_data_components ; k2++)
			{
				if (0 == stepindex)
				{
					newdatatri[new_data_index+k2] = 
						(1.0 - xi0) * datatri[data_index+
							vertexstart * surface->n_data_components+k2]
						+ xi0 * datatri[data_index+
							vertexi * surface->n_data_components+k2];
				}
				else
				{
					newdatatri[new_data_index+k2] = 
						(1.0 - xi0) * datatri[data_index+
							vertexstart* surface->n_data_components+k2]
						+ xi0 * datatri[data_index+
							vertexj* surface->n_data_components+k2];
				}
				newdatatri[new_data_index+surface->n_data_components+k2] = 
					(1.0 - xii) * datatri[data_index+
						vertexstart* surface->n_data_components+k2]
					+ xii * datatri[data_index+
						vertexi* surface->n_data_components+k2];
				newdatatri[new_data_index+2*surface->n_data_components+k2] = 
					(1.0 - xij) * datatri[data_index+
						vertexstart* surface->n_data_components+k2]
					+ xij * datatri[data_index+
						vertexj* surface->n_data_components+k2];
			}
		}
	}
	return(return_code);
}

static int rendergl_copy_quad_strip_to_dc(struct GT_surface *new_surface,
	int index, struct GT_surface *surface, int i, int j,
	struct Texture_tiling *texture_tiling)
{
	int npts1 = surface->n_pts1;
	float overlap_range, texture_offset, texture_scaling;
	Triple *texturepoints = surface->texturelist;
	int k, texture_tile;

	if (index + 3 >= new_surface->allocated_size)
	{
		rendergl_reallocate_GT_surface(new_surface, 2 * new_surface->allocated_size);
	}

	for (k = 0 ; k < 3 ; k++)
	{
		new_surface->pointlist[index][k] = 
			surface->pointlist[i+npts1*j][k];
		new_surface->pointlist[index+1][k] = 
			surface->pointlist[i+1+npts1*j][k];
		new_surface->pointlist[index+3][k] = 
			surface->pointlist[i+npts1*(j+1)][k];
		new_surface->pointlist[index+2][k] = 
			surface->pointlist[i+1+npts1*(j+1)][k];
	}
	if (surface->normallist)
	{
		for (k = 0 ; k < 3 ; k++)
		{
			new_surface->normallist[index][k] = 
				surface->normallist[i+npts1*j][k];
			new_surface->normallist[index+1][k] = 
				surface->normallist[i+1+npts1*j][k];
			new_surface->normallist[index+3][k] = 
				surface->normallist[i+npts1*(j+1)][k];
		new_surface->normallist[index+2][k] = 
				surface->normallist[i+1+npts1*(j+1)][k];
		}
	}
	for (k = 0 ; k < 3 ; k++)
	{
		if (k < texture_tiling->dimension)
		{
			overlap_range = (float)texture_tiling->overlap * 
				texture_tiling->tile_coordinate_range[k] /
				(float)texture_tiling->tile_size[k];
			texture_tile = (int)((texturepoints[i+npts1*j][k] - 0.5 * overlap_range) /
				(texture_tiling->tile_coordinate_range[k] - overlap_range));
			/* Need to handle the boundaries specially */
			texture_offset = 
				(texture_tile % texture_tiling->texture_tiles[k]) *
				(texture_tiling->tile_coordinate_range[k]- overlap_range);
			texture_scaling = 
				texture_tiling->coordinate_scaling[k];
		}
		else
		{
			texture_offset = 0.0;
			texture_scaling = 1.0;
		}
		new_surface->texturelist[index][k] = 
			(texturepoints[i+npts1*j][k] - texture_offset)
			* texture_scaling;
		new_surface->texturelist[index+1][k] = 
			(texturepoints[i+1+npts1*j][k] - texture_offset)
			* texture_scaling;
		new_surface->texturelist[index+3][k] = 
			(texturepoints[i+npts1*(j+1)][k] - texture_offset)
			* texture_scaling;
		new_surface->texturelist[index+2][k] = 
			(texturepoints[i+1+npts1*(j+1)][k] - texture_offset)
			* texture_scaling;
	}
	if (surface->tangentlist)
	{
		for (k = 0 ; k < 3 ; k++)
		{
			new_surface->tangentlist[index][k] = 
				surface->tangentlist[i+npts1*j][k];
			new_surface->tangentlist[index+1][k] = 
				surface->tangentlist[i+1+npts1*j][k];
			new_surface->tangentlist[index+3][k] = 
				surface->tangentlist[i+npts1*(j+1)][k];
			new_surface->tangentlist[index+2][k] = 
				surface->tangentlist[i+1+npts1*(j+1)][k];
		}
	}
	if (surface->data)
	{
		index *= surface->n_data_components;
		for (k = 0 ; k < surface->n_data_components ; k++)
		{
			new_surface->data[index+k] = 
				surface->data[(i+npts1*j)*surface->n_data_components+k];
			new_surface->data[index+surface->n_data_components+k] = 
				surface->data[(i+1+npts1*j)*surface->n_data_components+k];
			new_surface->data[index+3*surface->n_data_components+k] = 
				surface->data[(i+npts1*(j+1))*surface->n_data_components+k];
			new_surface->data[index+2*surface->n_data_components+k] = 
				surface->data[(i+1+npts1*(j+1))*surface->n_data_components+k];
		}
	}
	return(1);
}

static int rendergl_copy_polygon(struct GT_surface *new_surface,
	int index_new, struct GT_surface *surface, int index,
	struct Texture_tiling *texture_tiling)
{
	float overlap_range;
	int i, k2, texture_tile;
	Triple *texturetri, *newtexturetri, *pointtri, *newpointtri,
		*normaltri, *newnormaltri, *tangenttri, *newtangenttri;
	GTDATA *datatri, *newdatatri;

	float texture_average, texture_offset, texture_scaling;
	int local_index, local_index_new;

	int return_code = 1;
	if (index_new + new_surface->n_pts2 >= new_surface->allocated_size)
	{
		return_code = rendergl_reallocate_GT_surface(new_surface,
			2 * new_surface->allocated_size);
	}

	if (return_code)
	{
		texturetri = surface->texturelist;
		newtexturetri = new_surface->texturelist;

		pointtri = surface->pointlist;
		newpointtri = new_surface->pointlist;

		normaltri = surface->normallist;
		newnormaltri = new_surface->normallist;

		tangenttri = surface->tangentlist;
		newtangenttri = new_surface->tangentlist;

		datatri = surface->data;
		newdatatri = new_surface->data;

		if (new_surface->n_pts2 == surface->n_pts2)
		{
			for (k2 = 0 ; k2 < 3 ; k2++)
			{
				local_index = index;
				local_index_new = index_new;

				if (k2 < texture_tiling->dimension)
				{
					texture_average = 0.0;
					for (i = 0 ; i < surface->n_pts2 ; i++)
					{
						texture_average += texturetri[index+i][k2];
					}
					texture_average /= (float)surface->n_pts2;
					overlap_range = (float)texture_tiling->overlap * 
						texture_tiling->tile_coordinate_range[k2] /
						(float)texture_tiling->tile_size[k2];
					texture_tile = (int)((texture_average - 0.5 * overlap_range) /
						(texture_tiling->tile_coordinate_range[k2] - overlap_range));
					/* Need to handle the boundaries specially */
					texture_offset =
						(texture_tile % texture_tiling->texture_tiles[k2]) *
						(texture_tiling->tile_coordinate_range[k2] - overlap_range);
					texture_scaling = 
						texture_tiling->coordinate_scaling[k2];
				}
				else
				{
					texture_offset = 0.0;
					texture_scaling = 1.0;
				}
				for (i = 0 ; i < surface->n_pts2 ; i++)
				{
					newtexturetri[local_index_new][k2] = 
						(texturetri[local_index][k2] - texture_offset) * texture_scaling;

					newpointtri[local_index_new][k2] = 
						pointtri[local_index][k2];

					if (normaltri)
					{
						newnormaltri[local_index_new][k2] = 
							normaltri[local_index][k2];
					}

					if (tangenttri)
					{
						newtangenttri[local_index_new][k2] = 
							tangenttri[local_index][k2];
					}

					local_index++;
					local_index_new++;
				}
			}
			if (datatri)
			{
				int new_data_index = index_new * surface->n_data_components;
				int data_index = index * surface->n_data_components;
				for (i = 0 ; i < surface->n_pts2 ; i++)
				{
					for (k2 = 0 ; k2 < surface->n_data_components ; k2++)
					{
						newdatatri[new_data_index] = 
							datatri[data_index];
						new_data_index++;
						data_index++;
					}
				}
			}
		}
		else
		{
			return_code = 0;
		}
	}
	return (return_code);
}

static int rendergl_select_tile_bin(Triple texture_point,
	struct Texture_tiling *texture_tiling, Triple overlap_range)
{
	int tile, tilex, tiley, tilez;
	switch (texture_tiling->dimension)
	{
		case 1:
		{
			tilex = (int)floor((texture_point[0] - 0.5 * overlap_range[0]) / 
				(texture_tiling->tile_coordinate_range[0] - overlap_range[0]));
			if (tilex < 0)
			{
				tilex = 0;
			}
			tile = tilex % texture_tiling->texture_tiles[0];
		} break;
		case 2:
		{
			tilex = (int)floor((texture_point[0] - 0.5 * overlap_range[0]) / 
				(texture_tiling->tile_coordinate_range[0] - overlap_range[0]));
			if (tilex < 0)
			{
				tilex = 0;
			}
			tiley = (int)floor((texture_point[1] - 0.5 * overlap_range[1]) / 
				(texture_tiling->tile_coordinate_range[1] - overlap_range[1]));
			if (tiley < 0)
			{
				tiley = 0;
			}
			tile = tilex % texture_tiling->texture_tiles[0] +
				texture_tiling->texture_tiles[0] *
				(tiley % texture_tiling->texture_tiles[1]);
		} break;
		case 3:
		{
			tilex = (int)floor((texture_point[0] - 0.5 * overlap_range[0]) / 
				(texture_tiling->tile_coordinate_range[0] - overlap_range[0]));
			if (tilex < 0)
			{
				tilex = 0;
			}
			tiley = (int)floor((texture_point[1] - 0.5 * overlap_range[1]) / 
				(texture_tiling->tile_coordinate_range[1] - overlap_range[1]));
			if (tiley < 0)
			{
				tiley = 0;
			}
			tilez = (int)floor((texture_point[2] - 0.5 * overlap_range[2]) / 
				(texture_tiling->tile_coordinate_range[2] - overlap_range[2]));
			if (tilez < 0)
			{
				tilez = 0;
			}
			tile = tilex % texture_tiling->texture_tiles[0] +
				texture_tiling->texture_tiles[0] *
				((tiley % texture_tiling->texture_tiles[1]) + 
				texture_tiling->texture_tiles[1] *
				(tilez % texture_tiling->texture_tiles[2]));
		} break;
		default:
		{
			tile = 0;
		}
	}
	return (tile);
}

struct GT_surface *tile_GT_surface(struct GT_surface *surface, 
	struct Texture_tiling *texture_tiling)
/*******************************************************************************
LAST MODIFIED : 29 November 2007

DESCRIPTION :
Split a GT_surface <surface> based on its texture coordinates and 
<texture_tiling> boundaries.  Returns a surface or linked list of surfaces
that have equivalent geometry separated into separate surfaces for separate
tiles.
==============================================================================*/
{
	float nextcuti, nextcutj, xi0, xii, xij;
	int finished, i, index, j, k, l, initial_points, npts1, npts2,
		number_of_tiles, index_new, return_code, stepindex, 
		vertex1, vertex2, vertex3, vertex4, vertexstart, vertexi, vertexj;
	struct GT_surface *current_surface, *new_surface, *return_surface,
		**surface_tiles, **triangle_tiles;
	Triple texture_centre, *normals, overlap_range, *points, *texturepoints,
		*texturetri, vertexk;
;

	ENTER(tile_GT_surface);

	if (surface && texture_tiling)
	{
		return_code = 1;
		npts1 = surface->n_pts1;
		npts2 = surface->n_pts2;
		number_of_tiles = texture_tiling->texture_tiles[0];
		overlap_range[0] = (float)texture_tiling->overlap * 
			texture_tiling->tile_coordinate_range[0] /
			(float)texture_tiling->tile_size[0];
		if (texture_tiling->dimension > 1)
		{
			number_of_tiles *= texture_tiling->texture_tiles[1];
			overlap_range[1] = (float)texture_tiling->overlap * 
				texture_tiling->tile_coordinate_range[1] /
				(float)texture_tiling->tile_size[1];
		}
		if (texture_tiling->dimension > 2)
		{
			number_of_tiles *= texture_tiling->texture_tiles[2];
			overlap_range[2] = (float)texture_tiling->overlap * 
				texture_tiling->tile_coordinate_range[2] /
				(float)texture_tiling->tile_size[2];
		}
		ALLOCATE(surface_tiles, struct GT_surface *, number_of_tiles);
		for (i = 0 ; i < number_of_tiles ; i++)
		{
			surface_tiles[i] = (struct GT_surface *)NULL;
		}
		ALLOCATE(triangle_tiles, struct GT_surface *, number_of_tiles);
		for (i = 0 ; i < number_of_tiles ; i++)
		{
			triangle_tiles[i] = (struct GT_surface *)NULL;
		}
		texturepoints = surface->texturelist;
		points = surface->pointlist;
		normals = surface->normallist;
		switch (surface->polygon)
		{
			case g_QUADRILATERAL:
			{
				initial_points = 100;
				for (i=0;return_code && (i<npts1-1);i++)
				{
					for (j=0;return_code && (j<npts2-1);j++)
					{
						/* Determine which tile each vertex belongs too */
						vertex1 = rendergl_select_tile_bin(texturepoints[i+npts1*j],
							texture_tiling, overlap_range);
						vertex2 = rendergl_select_tile_bin(texturepoints[i+1+npts1*j],
							texture_tiling, overlap_range);
						vertex3 = rendergl_select_tile_bin(texturepoints[i+npts1*(j+1)],
							texture_tiling, overlap_range);
						vertex4 = rendergl_select_tile_bin(texturepoints[i+1+npts1*(j+1)],
							texture_tiling, overlap_range);
						if ((vertex1 == vertex2) &&
							(vertex1 == vertex3) &&
							(vertex1 == vertex4))
						{
							/* Simplest case, 1 bin */
							if (!surface_tiles[vertex1])
							{
								if (current_surface = rendergl_create_GT_surface(surface,
										initial_points, vertex1, /*polygon_size*/4))
								{
									surface_tiles[vertex1] = current_surface;
								}
								else
								{
									return_code = 0;
								}
							}
							else
							{
								current_surface = surface_tiles[vertex1];
							}
							if (return_code)
							{
								index = 4 * current_surface->n_pts1;
								return_code = rendergl_copy_quad_strip_to_dc(
									current_surface, index, surface, i, j,
									texture_tiling);
								current_surface->n_pts1++;
							}
						}
						else
						{
							/* Make a surface to store our triangles as we split them */
							current_surface = rendergl_create_GT_surface(surface,
								initial_points, 3, /*polygon_size*/3);
							/* Copy the texture coordinates for the quad we are 
								working on */
							index = 0;
							for (k = 0 ; k < 3 ; k++)
							{
								current_surface->texturelist[index][k] = 
									texturepoints[i+npts1*j][k];
								current_surface->texturelist[index+1][k] = 
									texturepoints[i+1+npts1*j][k];
								current_surface->texturelist[index+2][k] = 
									texturepoints[i+npts1*(j+1)][k];
								current_surface->pointlist[index][k] = 
									points[i+npts1*j][k];
								current_surface->pointlist[index+1][k] = 
									points[i+1+npts1*j][k];
								current_surface->pointlist[index+2][k] = 
									points[i+npts1*(j+1)][k];
								if (normals)
								{
									current_surface->normallist[index][k] = 
										normals[i+npts1*j][k];
									current_surface->normallist[index+1][k] = 
										normals[i+1+npts1*j][k];
									current_surface->normallist[index+2][k] = 
										normals[i+npts1*(j+1)][k];
								}
							}
							if (surface->data)
							{
								index *= surface->n_data_components;
								for (k = 0 ; k < surface->n_data_components ; k++)
								{
									current_surface->data[index+k] = 
										surface->data[(i+npts1*j)*surface->n_data_components+k];
									current_surface->data[index+surface->n_data_components+k] = 
										surface->data[(i+1+npts1*j)*surface->n_data_components+k];
									current_surface->data[index+2*surface->n_data_components+k] = 
										surface->data[(i+npts1*(j+1))*surface->n_data_components+k];
								}
							}
							index = 3;
							for (k = 0 ; k < 3 ; k++)
							{
								current_surface->texturelist[index][k] = 
									texturepoints[i+1+npts1*j][k];
								current_surface->texturelist[index+1][k] = 
									texturepoints[i+1+npts1*(j+1)][k];
								current_surface->texturelist[index+2][k] = 
									texturepoints[i+npts1*(j+1)][k];
								current_surface->pointlist[index][k] = 
									points[i+1+npts1*j][k];
								current_surface->pointlist[index+1][k] = 
									points[i+1+npts1*(j+1)][k];
								current_surface->pointlist[index+2][k] = 
									points[i+npts1*(j+1)][k];
								if (normals)
								{
									current_surface->normallist[index][k] = 
										normals[i+1+npts1*j][k];
									current_surface->normallist[index+1][k] = 
										normals[i+1+npts1*(j+1)][k];
									current_surface->normallist[index+2][k] = 
										normals[i+npts1*(j+1)][k];
								}
							}
							if (surface->data)
							{
								index *= surface->n_data_components;
								for (k = 0 ; k < surface->n_data_components ; k++)
								{
									current_surface->data[index+k] = 
										surface->data[(i+1+npts1*j)*surface->n_data_components+k];
									current_surface->data[index+surface->n_data_components+k] = 
										surface->data[(i+1+npts1*(j+1))*surface->n_data_components+k];
									current_surface->data[index+2*surface->n_data_components+k] = 
										surface->data[(i+npts1*(j+1))*surface->n_data_components+k];
								}
							}
							current_surface->n_pts1 = 2;
							for (k = 0 ; return_code && (k < 3) &&
								(k < texture_tiling->dimension) ; k++)
							{
#if defined (DEBUG)
								printf("direction %d\n", k);
#endif /* defined (DEBUG) */
								new_surface = rendergl_create_GT_surface(surface,
									initial_points, 0, /*polygon_size*/3);
								
								for (l = 0 ; return_code &&
									(l < current_surface->n_pts1) ; l++)
								{
									index = 3 * l;
									texturetri = current_surface->texturelist;

									vertexk[0] = (texturetri[index][k] - 0.5 * overlap_range[k]) / 
										(texture_tiling->tile_coordinate_range[k] - overlap_range[k]);
									vertexk[1] = (texturetri[index+1][k] - 0.5 * overlap_range[k]) / 
										(texture_tiling->tile_coordinate_range[k] - overlap_range[k]);
									vertexk[2] = (texturetri[index+2][k] - 0.5 * overlap_range[k]) / 
										(texture_tiling->tile_coordinate_range[k] - overlap_range[k]);
#if defined (DEBUG)
									printf(" tricoordinates %g %g %g\n",
										texturetri[index][k], texturetri[index+1][k], texturetri[index+2][k]);
									printf(" vertexk %g %g %g\n",
										vertexk[0], vertexk[1], vertexk[2]);
#endif /* defined (DEBUG) */
									if ((vertexk[1] >= vertexk[0]) && (vertexk[2] >= vertexk[0]))
									{
										vertexstart = 0;
										vertexi = 1;
										vertexj = 2;
									}
									else
									{
										if ((vertexk[2] >= vertexk[1]) && (vertexk[0] >= vertexk[1]))
										{
											vertexstart = 1;
											vertexi = 2;
											vertexj = 0;
										}
										else
										{
											vertexstart = 2;
											vertexi = 0;
											vertexj = 1;
										}
									}

#if defined (DEBUG)
									printf(" vertexstart %d %g %g %g\n",
										vertexstart, vertexk[vertexstart],
										vertexk[vertexi], vertexk[vertexj]);
#endif /* defined (DEBUG) */

									nextcuti = floor(vertexk[vertexstart] + 1.00001);
									nextcutj = nextcuti;
									/* Which side did we last step on */
									stepindex = -1;
									if (nextcuti > vertexk[vertexi])
									{
										nextcuti = vertexk[vertexi];
										/* Don't step the other side */
										stepindex = 1;
									}
									if (nextcutj > vertexk[vertexj])
									{
										nextcutj = vertexk[vertexj];
										/* Don't step the other side */
										stepindex = 0;
									}
									/* What was the previous xi on the step side */
									xi0 = 0.0;

									finished = 0;
									while (!finished)
									{
										if (vertexk[vertexi] > vertexk[vertexstart])
										{
											xii = (nextcuti - vertexk[vertexstart]) /
												(vertexk[vertexi] - vertexk[vertexstart]);
										}
										else
										{
											xii = 1.0;
										}
										if (vertexk[vertexj] - vertexk[vertexstart])
										{
											xij = (nextcutj - vertexk[vertexstart]) /
												(vertexk[vertexj] - vertexk[vertexstart]);
										}
										else
										{
											xij = 1.0;
										}

#if defined (DEBUG)
										float lastcut;
										if (stepindex == 0)
										{
											lastcut = vertexk[vertexstart] +
												xi0 * (vertexk[vertexi] - vertexk[vertexstart]);
										}
										else
										{
											lastcut = vertexk[vertexstart] +
												xi0 * (vertexk[vertexj] - vertexk[vertexstart]);
										}
										printf(" triangle %d (%g %g %g) %g %g %g\n",
											stepindex, lastcut,
											nextcuti, nextcutj,
											vertexk[vertexi], vertexk[vertexj], xi0);
#endif /* defined (DEBUG) */

										index_new = 3 * new_surface->n_pts1;
										if (rendergl_interpolate_triangle(new_surface,
											index_new, current_surface, index,
											vertexstart, vertexi, vertexj,
												xi0, xii, xij, stepindex))
										{
											new_surface->n_pts1++;
										}
										else
										{
											return_code = 0;
											finished = 1;
										}

										/* Test != as initially -1 and either step 
											would be OK */
 										if (((stepindex != 0) && (nextcuti < vertexk[vertexi]) && (nextcuti <= nextcutj)) ||
											((stepindex != 1) && (nextcutj < vertexk[vertexj]) && (nextcutj <= nextcuti)))
										{
											if ((stepindex != 0) && (nextcuti < vertexk[vertexi]) && (nextcuti <= nextcutj))
											{
												nextcuti++;
												stepindex = 0;
												xi0 = xii;
												if (nextcuti > vertexk[vertexi])
												{
													nextcuti = vertexk[vertexi];
												}
											}
											else
											{
												nextcutj++;
												stepindex = 1;
												xi0 = xij;
												if (nextcutj > vertexk[vertexj])
												{
													nextcutj = vertexk[vertexj];
												}
											}
										}
										else
										{
											finished = 1;
											if ((nextcuti != vertexk[vertexi])
												|| (nextcutj != vertexk[vertexj]))
											{
#if defined (DEBUG)
												printf("  not finished %g %g  %g %g\n",
													nextcuti, nextcutj,
													vertexk[vertexi], vertexk[vertexj]);
#endif /* defined (DEBUG) */
												/* Add the remainder to the old list */
												if (nextcuti < vertexk[vertexi])
												{
													stepindex = 0;
													xi0 = xii;
													nextcuti = vertexk[vertexi];
													xii = 1.0;
												}
												else
												{
													stepindex = 1;
													xi0 = xij;
													nextcutj = vertexk[vertexj];
													xij = 1.0;
												}

												index_new = 3 * current_surface->n_pts1;
												if (rendergl_interpolate_triangle(current_surface,
													index_new, current_surface, index,
													vertexstart, vertexi, vertexj,
														xi0, xii, xij, stepindex))
												{
													current_surface->n_pts1++;
												}
												else
												{
													finished = 1;
													return_code = 0;
												}
											}
#if defined (DEBUG)
											else
											{
												printf("  finished %g %g  %g %g\n",
													nextcuti, nextcutj,
													vertexk[vertexi], vertexk[vertexj]);
											}
#endif /* defined (DEBUG) */
										}
									}

								}
								DESTROY(GT_surface)(&current_surface);
								current_surface = new_surface;	
							}
							
							/* Put the triangles in their appropriate bin */
							for (l = 0 ; return_code && (l < current_surface->n_pts1) ; l++)
							{
								index = 3 * l;
								/* Use the centre of a triangle to determine its bin */
								
								texturetri = current_surface->texturelist;
								texture_centre[0] = (texturetri[index][0]
									+ texturetri[index+1][0] + 
									texturetri[index+2][0]) / 3.0;
								texture_centre[1] = (texturetri[index][1]
									+ texturetri[index+1][1] + 
									texturetri[index+2][1]) / 3.0;
								texture_centre[2] = (texturetri[index][2]
									+ texturetri[index+1][2] + 
									texturetri[index+2][2]) / 3.0;
								vertex1 = rendergl_select_tile_bin(texture_centre,
									texture_tiling, overlap_range);
								if ((vertex1 >= 0) && (vertex1 < number_of_tiles))
								{
									if (!triangle_tiles[vertex1])
									{
										if (new_surface = rendergl_create_GT_surface(
											surface, initial_points, 
											vertex1, /*polygon_size*/3))
										{
											triangle_tiles[vertex1] = new_surface;
										}
										else
										{
											return_code = 0;
										}
									}
									else
									{
										new_surface = triangle_tiles[vertex1];
									}
									if (return_code)
									{
										if (rendergl_copy_polygon(new_surface,
											3 * new_surface->n_pts1,
											current_surface, index, texture_tiling))
										{
											new_surface->n_pts1++;
										}
										else
										{
											return_code = 0;
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE, "tile_GT_surface.  "
										"Invalid triangle tile %d", vertex1);
								}
							}
							DESTROY(GT_surface)(&current_surface);
						}						
					}
				}
			} break;
			case g_TRIANGLE:
			{
				printf ("Not implemented triangles yet\n");
#if defined (OLD_CODE)
				surface_point_1=surfpts;
				surface_point_2=surfpts+npts1;
				if (normalpoints)
				{
					normal_point_1=normalpoints;
					normal_point_2=normalpoints+npts1;
				}
#if defined GL_VERSION_1_3
				if (tangentpoints)
				{
					tangent_point_1=tangentpoints;
					tangent_point_2=tangentpoints+npts1;
				}
#endif /* defined GL_VERSION_1_3 */
				if (texturepoints)
				{
					texture_point_1 = texturepoints;
					texture_point_2 = texturepoints+npts1;				
				}
				if (data)
				{
					data_1=data;
					data_2=data+npts1*number_of_data_components;
				}
				for (i=npts1-1;i>0;i--)
				{
					glBegin(GL_TRIANGLE_STRIP);
					if (normalpoints)
					{
						glNormal3fv(*normal_point_1);
						normal_point_1++;
					}
#if defined GL_VERSION_1_3
					if (tangentpoints)
					{
						glMultiTexCoord3fv(GL_TEXTURE1_ARB,*tangent_point_1);
						tangent_point_1++;
					}
#endif /* defined GL_VERSION_1_3 */
					if (texturepoints)
					{
						glTexCoord3fv(*texture_point_1);
						texture_point_1++;
					}
					if (data)
					{
						spectrum_renderGL_value(spectrum,material,render_data,
							data_1);
						data_1 += number_of_data_components;
					}
					glVertex3fv(*surface_point_1);
					surface_point_1++;
					for (j=i;j>0;j--)
					{
						if (normalpoints)
						{
							glNormal3fv(*normal_point_2);
							normal_point_2++;
						}
#if defined GL_VERSION_1_3
						if (tangentpoints)
						{
							glMultiTexCoord3fv(GL_TEXTURE1_ARB,*tangent_point_2);
							tangent_point_2++;
						}
#endif /* defined GL_VERSION_1_3 */
						if (texturepoints)
						{
							glTexCoord3fv(*texture_point_2);
							texture_point_2++;
						}
						if (data)
						{
							spectrum_renderGL_value(spectrum,material,render_data,
								data_2);
							data_2 += number_of_data_components;
						}
						glVertex3fv(*surface_point_2);
						surface_point_2++;
						if (normalpoints)
						{
							glNormal3fv(*normal_point_1);
							normal_point_1++;
						}
#if defined GL_VERSION_1_3
						if (tangentpoints)
						{
							glMultiTexCoord3fv(GL_TEXTURE1_ARB,*tangent_point_1);
							tangent_point_1++;
						}
#endif /* defined GL_VERSION_1_3 */
						if (texturepoints)
						{
							glTexCoord3fv(*texture_point_1);
							texture_point_1++;
						}
						if (data)
						{
							spectrum_renderGL_value(spectrum,material,render_data,
								data_1);
							data_1 += number_of_data_components;
						}
						glVertex3fv(*surface_point_1);
						surface_point_1++;
					}
					glEnd();
				}
#endif /* defined (OLD_CODE) */
			} break;
		}
		current_surface = (struct GT_surface *)NULL;
		return_surface = (struct GT_surface *)NULL;
		for (i = 0 ; return_code && (i < number_of_tiles) ; i++)
		{
			if (surface_tiles[i])
			{
				if (current_surface)
				{
					current_surface->ptrnext = surface_tiles[i];
				}
				else
				{
					return_surface = surface_tiles[i];
				}
				current_surface = surface_tiles[i];
				if (index != current_surface->allocated_size)
				{
					rendergl_reallocate_GT_surface(current_surface,
						index);
				}
			}
			if (triangle_tiles[i])
			{
				if (current_surface)
				{
					current_surface->ptrnext = triangle_tiles[i];
				}
				else
				{
					return_surface = triangle_tiles[i];
				}
				current_surface = triangle_tiles[i];
				index = current_surface->n_pts1 * current_surface->n_pts2;
			
				if (index != current_surface->allocated_size)
				{
					rendergl_reallocate_GT_surface(current_surface,
						index);
				}
			}
		}
		DEALLOCATE(surface_tiles);
		DEALLOCATE(triangle_tiles);
		if (!return_code)
		{
			return_surface=(struct GT_surface *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"tile_GT_surface. Invalid argument(s)");
		return_surface=(struct GT_surface *)NULL;
	}
	LEAVE;

	return (return_surface);
} /* tile_GT_surface */

