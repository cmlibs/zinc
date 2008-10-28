/*******************************************************************************
FILE : renderwavefront.cpp

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Renders gtObjects to Wavefront OBJ file
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
#include <stdlib.h>
#include <string.h>
#include <math.h>
extern "C" {
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/renderwavefront.h"
#include "graphics/scene.h"
}
#include "graphics/scene.hpp"
extern "C" {
#include "graphics/spectrum.h"
#include "user_interface/message.h"
}
#include "graphics/graphics_object_private.hpp"


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
	struct Graphical_material *material,
	struct Graphical_material **current_material)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Writes Wavefront object file that defines the material
==============================================================================*/
{
	int return_code;

	ENTER(activate_material_wavefront);
	if (material&&file)
	{
		if (current_material)
		{
			if (*current_material == material)
			{
				return_code = 1;
			}
			else
			{
				fprintf(file,"usemtl %s\n",Graphical_material_name(material));
				*current_material = material;
				return_code=1;
			}
		}
		else
		{
			fprintf(file,"usemtl %s\n",Graphical_material_name(material));
			return_code=1;
		}
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
	Triple *texturepts, int npts1,int npts2, gtPolygonType polygon_type,
  int number_of_data_components, GTDATA *data,
	struct Graphical_material *material, struct Spectrum *spectrum,
	struct LIST(Wavefront_vertex) *vertex_list,
	struct Graphical_material **current_material)
/*******************************************************************************
LAST MODIFIED : 3 October 2000

DESCRIPTION :
==============================================================================*/
{
	int i,j,index,npts12,return_code, *vertex_index_array, *vertex_index;
	struct Wavefront_vertex *vertex;
	struct Wavefront_vertex_position position;
	Triple *surface_point_1, *texture_point;

	ENTER(draw_surface_wavefront);
	/* Keep a similar interface to all the other render implementations */
	USE_PARAMETER(normalpts);
	USE_PARAMETER(number_of_data_components);
	USE_PARAMETER(data);
	USE_PARAMETER(spectrum);
	if (surfpts&&(1<npts1)&&(1<npts2))
	{
		switch (polygon_type)
		{
			case g_QUADRILATERAL:
			{
				npts12 = npts1 * npts2;
				activate_material_wavefront(file, material, current_material);
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
				DEALLOCATE(vertex_index_array);
				return_code=1;
			} break;
			case g_TRIANGLE:
			{
				npts12 = (npts1 + 1) * npts1 / 2;
				activate_material_wavefront(file, material, current_material);
				ALLOCATE(vertex_index_array,int,npts12);
				vertex_index = vertex_index_array;
				texture_point=texturepts;
				surface_point_1=surfpts;
				for (i = npts1;i>0;i--)
				{
					for (j = 0 ; j < i ; j++)
					{
						if (vertex_list&&((i==0)||(i==(npts1-1))||(j==0)||(j==(i-1))))
						{
							position.x = (*surface_point_1)[0];
							position.y = (*surface_point_1)[1];
							position.z = (*surface_point_1)[2];
							if (vertex = FIND_BY_IDENTIFIER_IN_LIST(Wavefront_vertex,position)(&position,
								vertex_list))
							{
								*vertex_index = vertex->index;
							}
							else
							{
								fprintf(file, "v %f %f %f\n",
									(*surface_point_1)[0],
									(*surface_point_1)[1],
									(*surface_point_1)[2]);
								file_vertex_index++;
								*vertex_index = file_vertex_index;
								if (vertex = CREATE(Wavefront_vertex)(file_vertex_index,
									(*surface_point_1)[0], (*surface_point_1)[1],
									(*surface_point_1)[2]))
								{
									ADD_OBJECT_TO_LIST(Wavefront_vertex)(vertex, vertex_list);
								}
							}
						}
						else
						{
							fprintf(file, "v %f %f %f\n",
								(*surface_point_1)[0],
								(*surface_point_1)[1],
								(*surface_point_1)[2]);
							file_vertex_index++;
							*vertex_index = file_vertex_index;
						}
						vertex_index++;
						surface_point_1++;
						if (texturepts)
						{
							fprintf(file, "vt %f %f %f\n",
								(*texture_point)[0],
								(*texture_point)[1],
								(*texture_point)[2]);
							file_texture_vertex_index++;
							texture_point++;
						}
					}
				}
				index = file_texture_vertex_index-npts12+1;
				vertex_index = vertex_index_array;
				for (i= npts1;i>1;i--)
				{
					for (j=0;j<i-1;j++)
					{
						if (texturepts)
						{
							fprintf(file, "f %d/%d %d/%d %d/%d\n",
								*vertex_index, index,
								*(vertex_index + 1), (index + 1),
								*(vertex_index + i), (index + i));
							if (j < i-2)
							{
								fprintf(file, "f %d/%d %d/%d %d/%d\n",
									*(vertex_index + i), (index + i),
									*(vertex_index + 1), (index + 1),
									*(vertex_index + i + 1), (index + i + 1));
							}
						}
						else
						{
							fprintf(file, "f %d %d %d\n",
								*vertex_index,
								*(vertex_index + 1),
								*(vertex_index + i));
							if (j < i-2)
							{
								fprintf(file, "f %d %d %d\n",
									*(vertex_index + i),
									*(vertex_index + 1),
									*(vertex_index + i + 1));
							}
						}
						vertex_index++;
						index++;
					}
					vertex_index++;
					index++;
				}
				DEALLOCATE(vertex_index_array);
				return_code=1;
			} break;
		}
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
	int number_of_vertices, struct VT_iso_vertex **vertex_list,
	int number_of_triangles, struct VT_iso_triangle **triangle_list,
	int number_of_data_components,
	struct Graphical_material *default_material, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 10 November 2005

DESCRIPTION :
==============================================================================*/
{
	int i,return_code;

	ENTER(drawvoltexwavefront);
	/* Keep a similar interface to all the other render implementations */
	USE_PARAMETER(number_of_data_components);
	USE_PARAMETER(default_material);
	USE_PARAMETER(spectrum);
	return_code=0;
	if (triangle_list && vertex_list && (0<number_of_triangles))
	{
		if ( full_comments )
		{
			fprintf(out_file,"#vertex list\n");
		}
		for (i=0;i<number_of_vertices;i++)
		{
			if ( full_comments )
			{
				fprintf(out_file,"#vertex %d\n",i+1);
			}
			fprintf(out_file,"v %lf %lf %lf\n",vertex_list[i]->coordinates[0],
				vertex_list[i]->coordinates[1],vertex_list[i]->coordinates[2]);
		}
		if ( full_comments )
		{
			fprintf(out_file,"#texture vertices %d\n",number_of_vertices+1);
		}
		for (i=0;i<number_of_vertices;i++)
		{
			if ( full_comments )
			{
				fprintf(out_file,"#vertex %d\n",i+1);
			}
			fprintf(out_file,"vt %lf %lf %lf\n",vertex_list[i]->texture_coordinates[0],
				vertex_list[i]->texture_coordinates[1],vertex_list[i]->texture_coordinates[2]);
		}
		if ( full_comments )
		{
			fprintf(out_file,"#normal list\n");
		}
		for (i=0;i<number_of_vertices;i++)
		{
			if ( full_comments )
			{
				fprintf(out_file,"#normal %d\n",i+1);
			}
			fprintf(out_file,"vn %lf %lf %lf\n",-(vertex_list[i]->normal[0]),
				-(vertex_list[i]->normal[1]),-(vertex_list[i]->normal[2]));
		}
		for (i=0;i<number_of_triangles;i++)
		{
			if ( full_comments )
			{
				fprintf(out_file,"# polygon %d\n",i+1);
			}
			fprintf(out_file,"f   %d/%d/%d  %d/%d/%d  %d/%d/%d\n",
				triangle_list[i]->vertices[0]->index+file_vertex_index+1,
				triangle_list[i]->vertices[0]->index+file_texture_vertex_index+1,
				triangle_list[i]->vertices[0]->index+file_normal_vertex_index+1,
				triangle_list[i]->vertices[1]->index+file_vertex_index+1,
				triangle_list[i]->vertices[1]->index+file_texture_vertex_index+1,
				triangle_list[i]->vertices[1]->index+file_normal_vertex_index+1,
				triangle_list[i]->vertices[2]->index+file_vertex_index+1,
				triangle_list[i]->vertices[2]->index+file_texture_vertex_index+1,
				triangle_list[i]->vertices[2]->index+file_normal_vertex_index+1);
		} /* for i */
		file_vertex_index += number_of_vertices;
		file_normal_vertex_index += number_of_vertices;
		file_texture_vertex_index += number_of_vertices;
	}
	else
	{
		display_message(ERROR_MESSAGE,"drawvoltexwavefront.  Invalid argument(s)");
		return_code=0;
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
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Convert graphical object into Wavefront object file.
==============================================================================*/
{
	float proportion,*times;
	int itime, number_of_times, return_code;
	struct Graphical_material *current_material;
	struct GT_glyph_set *interpolate_glyph_set,*glyph_set,*glyph_set_2;
	struct GT_nurbs *nurbs;
	struct GT_surface *interpolate_surface,*surface,*surface_2;
	struct GT_voltex *voltex;
	struct LIST(Wavefront_vertex) *vertex_list;
	union GT_primitive_list *primitive_list1, *primitive_list2;

	ENTER(makewavefront);
	/* check arguments */
	return_code = 1;
	current_material = (struct Graphical_material *)NULL;
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
							"makewavefront.  Invalid primitive_list");
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
					"makewavefront.  Invalid primitive_lists");
				return_code = 0;
			}
		}
		if ((0 < number_of_times) && return_code)
		{
			switch (object->object_type)
			{
				case g_GLYPH_SET:
				{
					if (glyph_set = primitive_list1->gt_glyph_set.first)
					{
						if (proportion>0)
						{
							glyph_set_2 = primitive_list2->gt_glyph_set.first;
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
					if (voltex = primitive_list1->gt_voltex.first)
					{
						while (voltex)
						{
							drawvoltexwavefront(wavefront_file, full_comments,
								voltex->number_of_vertices, voltex->vertex_list,
								voltex->number_of_triangles, voltex->triangle_list,
								voltex->n_data_components, 
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
												interpolate_surface->n_pts1,
												interpolate_surface->n_pts2,
												interpolate_surface->polygon,
												interpolate_surface->n_data_components,
												interpolate_surface->data,
												object->default_material, object->spectrum,
												vertex_list, &current_material);
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
												surface->texturelist, surface->n_pts1,
												surface->n_pts2, surface->polygon,
												surface->n_data_components,
												surface->data,object->default_material,
												object->spectrum,
												vertex_list, &current_material);
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
					if (nurbs = primitive_list1->gt_nurbs.first)
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
LAST MODIFIED : 31 May 2000

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
	if (scene)
	{
		build_Scene(scene);
		if (!scene_object)
		{
			if (1==Scene_get_number_of_scene_objects(scene))
			{
				scene_object = first_Scene_object_in_Scene_that(scene,
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
