/*******************************************************************************
FILE : haptic_input_module.cpp

LAST MODIFIED : 23 November 2001

DESCRIPTION :
Contains all the code needed to handle input from the haptic device.  Sets up
callbacks for whatever users are interested in.  Additionally when the
input from the haptic device has been initiated a scene can be realised in
the haptic environment.
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
#if defined (HAPTIC) && defined (EXT_INPUT)
#include <stdlib.h>
#include <unistd.h>
#include <gstBasic.h>
#include <gstScene.h>
#include <gstShape.h>
#include <gstSphere.h>
#include <gstPolyMesh.h>
#include <gstSeparator.h>
#include <gstPHANToM.h>
#endif /* defined (HAPTIC) && defined (EXT_INPUT) */

extern "C"
{
#include "general/debug.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "io_devices/matrix.h"
#include "io_devices/input_module.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
}
#include "io_devices/haptic_input_module.h"

/*
Module Constants
----------------
*/
#if defined (HAPTIC) && defined (EXT_INPUT)
#define HAPTIC_INIT_NAME "phantom.ini"
/* window dimensions. */
#define RESET_WINDOW_WIDTH 500
#define RESET_WINDOW_HEIGHT 350
#endif /* defined (HAPTIC) && defined (EXT_INPUT) */

/*
Module Variables
----------------
*/
#if defined (HAPTIC) && defined (EXT_INPUT)
static gstPHANToM *phantom = NULL;
static gstScene *scene = NULL;
static gstSeparator *phantomSep = NULL, *model = NULL;
static gstSeparator *current_scene = NULL;
#endif /* defined (HAPTIC) && defined (EXT_INPUT) */

/*
Module Types
------------
*/
#ifdef HAPTIC
struct Haptic_scene_data
{
	struct Scene *scene;
	gstSeparator *top;
};
#endif /* HAPTIC */

/*
Global functions
----------------
*/
int input_module_haptic_init(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Sets the haptic device so that any input is recognised, and messages are
received.
==============================================================================*/
{
#ifdef PHANTOM_HAPTIC
	char input[100];
#endif /* PHANTOM_HAPTIC */
	int return_code;

	ENTER(input_module_haptic_init);
	if (user_interface)
	{
#ifdef PHANTOM_HAPTIC
		/* SAB Temporary code to test stuff while haptic unavailable */
		printf("Do you want the dummy haptic device present? (y for yes)\n");
		scanf("%s", input);
		if ( strncmp ("y", input, 1))
		{
			return_code = 0;
		}
		else
		{
			return_code = 1;
		}
#else /* PHANTOM_HAPTIC */
#if defined (HAPTIC) && defined (EXT_INPUT)
		phantom=new gstPHANToM(HAPTIC_INIT_NAME);
		if (phantom->getValidConstruction())
		{
			// Create root separator of scene graph for model
			model=new gstSeparator;
			model->setName(gstNodeName("model"));

			printf("\nHold PHANToM in reset position and press enter to continue\n");
			getchar();

			// Put phantom into separator.  This separator should be used to
			// translate and orient the phantom in the scene.  For now leave phantom
			// at origin.
			phantomSep=new gstSeparator;
			phantomSep->addChild(phantom);
			model->addChild(phantomSep);
			// Create gstScene object to handle haptic simulation
			scene=new gstScene;
			// Setting model as root of gstScene makes model the root of the scene
				// graph.  After setting model as root of scene, model and all nodes
				// under model have been put in scene graph and their state reflects
				// that.
			scene->setRoot(model);
			// Start servo loop.
			if (scene->startServoLoop())
			{
				return_code=1;
			}
			else
			{
				display_message(WARNING_MESSAGE,"servoLoop unable to start.\n");
				return_code=0;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"gstPHANToM constructor failed\n");
			return_code=0;
		}
#else /* defined (HAPTIC) && defined (EXT_INPUT) */
		display_message(ERROR_MESSAGE,"Haptic device support is not present");
		return_code=0;
#endif /* defined (HAPTIC) && defined (EXT_INPUT) */
#endif /* PHANTOM_HAPTIC */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_haptic_init.  Missing user_interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* input_module_haptic_init */

int input_module_haptic_close(struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
Closes the haptic device.
==============================================================================*/
{
	int return_code;

	ENTER(input_module_haptic_close);
	USE_PARAMETER(user_interface);
#if ! defined PHANTOM_HAPTIC
#if defined (HAPTIC) && defined (EXT_INPUT)
	delete phantom;
	delete model;
	delete phantomSep;
	delete scene;
#endif /* defined (HAPTIC) && defined (EXT_INPUT) */
#endif /* not PHANTOM_HAPTIC */
	return_code=1;
	LEAVE;

	return (return_code);
} /* input_module_haptic_close */

int input_module_haptic_position(struct User_interface *user_interface,
	Input_module_message message)
/*******************************************************************************
LAST MODIFIED : 6 January 1998

DESCRIPTION :
Gets the position of the haptic device.
==============================================================================*/
{
#ifdef PHANTOM_HAPTIC
	static double haptic_previous_position[3] = { 0.0, 0.0, 0.0 };
#endif /* PHANTOM_HAPTIC */
	int return_code;
#if defined (HAPTIC) && defined (EXT_INPUT)
	gstPoint PhantomPos;
#endif /* defined (HAPTIC) && defined (EXT_INPUT) */

	ENTER(input_module_haptic_position);
	return_code=0;
	// Check arguments
	if (user_interface)
	{
#ifdef PHANTOM_HAPTIC
	/* SAB Temporary code to test stuff while haptic unavailable */
			haptic_previous_position[0] += 1.0;
		haptic_previous_position[1] += 1.0;
		haptic_previous_position[2] += 1.0;
		message->source=IM_SOURCE_HAPTIC;
		message->type=IM_TYPE_MOTION;
		message->data[0] = haptic_previous_position[0];
		message->data[1] = haptic_previous_position[1];
		message->data[2] = haptic_previous_position[2];
		/* Rotations */
		message->data[3]=0;
		message->data[4]=0;
		message->data[5]=0;
		return_code=1;
#else /* PHANTOM_HAPTIC */

#if defined (HAPTIC) && defined (EXT_INPUT)
		// Get the position of the haptic phantom
		PhantomPos=phantom->getPosition();
		// Tell the clients the position of the haptic device
		message->source=IM_SOURCE_HAPTIC;
		message->type=IM_TYPE_MOTION;
		message->data[0] = PhantomPos.x();
		message->data[1] = PhantomPos.y();
		message->data[2] = PhantomPos.z();
		/* Rotations */
		message->data[3]=0;
		message->data[4]=0;
		message->data[5]=0;
		return_code=1;
#else /* defined (HAPTIC) && defined (EXT_INPUT) */
		USE_PARAMETER(message);
		display_message(ERROR_MESSAGE,"Haptic device support is not present");
		return_code=0;
#endif /* defined (HAPTIC) && defined (EXT_INPUT) */
#endif /* PHANTOM_HAPTIC */

	}
	else
	{
		display_message(WARNING_MESSAGE,
			"input_module_update.  Missing user_interface");
	}
	LEAVE;

	return (return_code);
} /* input_module_haptic_position */

#ifdef HAPTIC
gstTransform *haptic_make_surface_texmap(Triple *surfpts, GTDATA *data,
	struct Graphical_material *material, struct Spectrum *spectrum,
	int npts1, int npts2)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
==============================================================================*/
{
	gstPolyMesh *polymesh;
	int i,j,nptsij, ntriangles;
	int index;
	double (*vertex_coords)[3];
	int (*polymesh_array)[3], *polygon_index;

	ENTER(haptic_make_surface_texmap);
	/* checking arguments */
	if (surfpts&&(1<npts1)&&(1<npts2))
	{
		nptsij = npts1 * npts2;
		ntriangles = 2 * (npts1 - 1) * (npts2 - 1);
		if ( vertex_coords = new double[nptsij][3])
		{
			if ( polymesh_array = new int[ntriangles][3] )
			{
				if ( polymesh = new gstPolyMesh )
				{
					index = 0;
					for (i=0;i<npts1;i++)
					{
						for (j=0;j<npts2;j++)
						{
							vertex_coords[index][0] = surfpts[i+npts1*j][0];
							vertex_coords[index][1] = surfpts[i+npts1*j][1];
							vertex_coords[index][2] = surfpts[i+npts1*j][2];
							index++;
						}
					}

					/* Debug */
					/*for (i=0 ; i<nptsij ; i++)
					{
						printf("   %d     %f %f %f\n", i, vertex_coords[i][0],
							vertex_coords[i][1], vertex_coords[i][2] );
					}*/

					index = 0;
					polygon_index = polymesh_array[0];
					for (i=0;i<npts1-1;i++)
					{
						for (j=0;j<npts2-1;j++)
						{
							*(polygon_index++) = index;
							*(polygon_index++) = index + 1;
							*(polygon_index++) = index + npts2 + 1;
							*(polygon_index++) = index;
							*(polygon_index++) = index + npts2 + 1;
							*(polygon_index++) = index + npts2;

							index++;
						}
						index++;
					}

					/* Debug */
					/*for( i=0 ; i < ntriangles ; i++ )
					{
						printf("   %d    %d %d %d\n", i, polymesh_array[i][0],
							polymesh_array[i][1], polymesh_array[i][2]);
					}*/

					polymesh = new gstPolyMesh( nptsij, vertex_coords,
						ntriangles, polymesh_array);
				}
				else
				{
					display_message(ERROR_MESSAGE,"haptic_make_surface_texmap.  Could not create gstPolyMesh");
					polymesh = NULL;
				}
				delete polymesh_array;
			}
			else
			{
				display_message(ERROR_MESSAGE,"haptic_make_surface_texmap.  Could not allocate polymesh triangle array");
				polymesh = NULL;
			}
			delete vertex_coords;
		}
		else
		{
			display_message(ERROR_MESSAGE,"haptic_make_surface_texmap.  Could not allocate vertex coordinate array");
				polymesh = NULL;

		}
	}
	else
	{
		if ((1<npts1)&&(1<npts2))
		{
			display_message(ERROR_MESSAGE,
				"haptic_make_surface_texmap.  Invalid argument(s)");

			polymesh = NULL;
		}
		else
		{
			polymesh = NULL;
		}
	}
	LEAVE;

	return (polymesh);
} /* haptic_make_surface_texmap */

gstTransform *makehaptic(gtObject *object,float time,float t,float u)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Convert graphical object into objects that the Haptic device will interact with.

==============================================================================*/
{
	enum Spectrum_type type;
	float proportion,*times;
	gstTransform *objectNode;
	gstSeparator *groupNode;
	int i, itime, j, k, return_code;
	struct GT_element_group *gt_element_group;
	struct GT_nurbs *nurbs;
	struct GT_point *point;
	struct GT_pointset *interpolate_point_set,*point_set,*point_set_2;
	struct GT_polyline *interpolate_line,*line,*line_2;
	struct GT_surface *interpolate_surface,*surface,*surface_2;
	struct GT_userdef *userdef;
	struct GT_voltex *voltex;
#ifdef MERGE_TIMES
	GTDATA **field_array;
	struct GT_surface **surface_array;
	int merge;

	merge = 0;
#endif
	ENTER(makegtobject);

	/* check arguments */
	if (object)
	{
		objectNode = NULL;
		if ((itime=object->number_of_times)>0)
		{
#ifdef MERGE_TIMES
			get_Spectrum_type(object->spectrum, &type);
			if (object->spectrum && MERGE_RGB_SPECTRUM==type)
			{
				times=object->times;
				itime = 1;
				merge = 1;
			}
#endif
			if ((itime>1)&&(times=object->times)
#ifdef MERGE_TIMES
				&&!merge
#endif
				)
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


			switch (object->objecttype)
			{
				case g_POINT:
				{
					if (point=(object->gu.gtPoint)[itime])
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
					if (point_set=(object->gu.gtPointset)[itime])
					{
						if (proportion>0)
						{
							point_set_2=(object->gu.gtPointset)[itime+1];
							while (point_set&&point_set_2)
							{
								if (interpolate_point_set=morph_GTPOINTSET(proportion,point_set,
									(object->gu.gtPointset)[itime+1]))
								{
/* 									drawpointsetGL(interpolate_point_set->n_pts, */
/* 										interpolate_point_set->pointlist, */
/* 										interpolate_point_set->text, */
/* 										interpolate_point_set->marker_type, */
/* 										interpolate_point_set->marker_size, */
/* 										interpolate_point_set->data_type, */
/* 										interpolate_point_set->data, */
/* 										object->default_material, object->spectrum); */
									destroy_GTPOINTSET(&interpolate_point_set);
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
					if (voltex=(object->gu.gtVoltex)[itime])
					{
						if ( groupNode = new gstSeparator )
						{
							while (voltex)
							{
								/* SAB To be done yet
								if ( objectNode = drawvoltexalias(
									voltex->n_iso_polys,voltex->triangle_list,
									voltex->vertex_list,voltex->n_vertices,voltex->n_rep,
									voltex->iso_poly_material,voltex->iso_env_map,
									voltex->iso_poly_cop, voltex->texturemap_coord,
									voltex->texturemap_index,voltex->draw_field,
									object->default_material, object->spectrum))
								{
									groupNode->addChildNode( objectNode );
									delete objectNode;  // Delete the wrapper
								}
								*/
								voltex=voltex->ptrnext;
							}
							objectNode = groupNode;
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"makehaptic.  Could not create group gstSeparator");
							return_code=0;
						}
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
								if (g_NO_DATA==line->data_type)
								{
									if (proportion>0)
									{
										while (line&&line_2)
										{
											if (interpolate_line=morph_GT_polyline(proportion,line,
												line_2))
											{
/* 												drawpolylineGL(interpolate_line->pointlist, */
/* 													interpolate_line->n_pts); */
												destroy_GT_polyline(&interpolate_line);
											}
											line=line->ptrnext;
											line_2=line_2->ptrnext;
										}
									}
									else
									{
										while (line)
										{
/* 											drawpolylineGL(line->pointlist,line->n_pts); */
											line=line->ptrnext;
										}
									}
								}
								else
								{
									if (proportion>0)
									{
										while (line&&line_2)
										{
											if (interpolate_line=morph_GT_polyline(proportion,line,
												line_2))
											{
/* 												drawdatapolylineGL(interpolate_line->pointlist, */
/* 													interpolate_line->data,object->default_material,  */
/* 													object->spectrum, interpolate_line->n_pts); */
												destroy_GT_polyline(&interpolate_line);
											}
											line=line->ptrnext;
											line_2=line_2->ptrnext;
										}
									}
									else
									{
										while (line)
										{
/* 											drawdatapolylineGL(line->pointlist,line->data, */
/* 												object->default_material, object->spectrum, */
/* 												line->n_pts); */
											line=line->ptrnext;
										}
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
											if (g_NO_DATA==line->data_type)
											{
/* 											draw_dc_polylineGL(interpolate_line->pointlist, */
/* 												interpolate_line->n_pts); */
											}
											else
											{
/* 											draw_data_dc_polylineGL(interpolate_line->pointlist, */
/* 												interpolate_line->data, interpolate_line->n_pts, */
/* 												object->default_material, object->spectrum); */
											}
											destroy_GT_polyline(&interpolate_line);
										}
										line=line->ptrnext;
										line_2=line_2->ptrnext;
									}
								}
								else
								{
									while (line)
									{
										if (g_NO_DATA==line->data_type)
										{
/* 											draw_dc_polylineGL(line->pointlist,line->n_pts); */
										}
										else
										{
/* 											draw_data_dc_polylineGL(line->pointlist, */
/* 												line->data, line->n_pts, object->default_material, */
/* 												object->spectrum); */
										}
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
											destroy_GT_polyline(&interpolate_line);
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
											destroy_GT_polyline(&interpolate_line);
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
					if (surface=(object->gu.gtSurface)[itime])
					{
						if (proportion>0)
						{
							surface_2=(object->gu.gtSurface)[itime+1];
						}
						switch (surface->surfacetype)
						{
							case g_SHADED:
							{
								if (g_NO_DATA==surface->data_type)
								{
									if (proportion>0)
									{
										while (surface&&surface_2)
										{
											if (interpolate_surface=morph_GTSURFACE(proportion,
												surface,surface_2))
											{
/* 												drawsurfaceGL(interpolate_surface->pointlist, */
/* 													interpolate_surface->n_pts1, */
/* 													interpolate_surface->n_pts2); */
												destroy_GTSURFACE(&interpolate_surface);
											}
											surface=surface->ptrnext;
											surface_2=surface_2->ptrnext;
										}
									}
									else
									{
										while (surface)
										{
/* 											drawsurfaceGL(surface->pointlist,surface->n_pts1, */
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
											if (interpolate_surface=morph_GTSURFACE(proportion,
												surface,surface_2))
											{
/* 												drawdatasurfaceGL(interpolate_surface->pointlist, */
/* 													interpolate_surface->data, */
/* 													object->default_material, object->spectrum, */
/* 													interpolate_surface->n_pts1, */
/* 													interpolate_surface->n_pts2); */
												destroy_GTSURFACE(&interpolate_surface);
											}
											surface=surface->ptrnext;
											surface_2=surface_2->ptrnext;
										}
									}
									else
									{
										while (surface)
										{
/* 											drawdatasurfaceGL(surface->pointlist,surface->data, */
/* 														object->default_material, object->spectrum, */
/* 														surface->n_pts1, surface->n_pts2); */
											surface=surface->ptrnext;
										}
									}
								}
								return_code=1;
							} break;
							case g_SHADED_TEXMAP:
							{
								if (g_NO_DATA==surface->data_type)
								{
									if (proportion>0)
									{
										if ( groupNode = new gstSeparator )
										{
											while (surface&&surface_2)
											{
												if (interpolate_surface=morph_GTSURFACE(proportion,
													surface,surface_2))
												{
													if ( objectNode = haptic_make_surface_texmap(
														interpolate_surface->pointlist,
														(GTDATA *) NULL, object->default_material,
														(Spectrum *) NULL,
														interpolate_surface->n_pts1,
														interpolate_surface->n_pts2))
													{
														groupNode->addChild( objectNode );
													}
													destroy_GTSURFACE(&interpolate_surface);
												}
												surface=surface->ptrnext;
												surface_2=surface_2->ptrnext;
											}
											objectNode = groupNode;
											return_code=1;
										}
										else
										{
											display_message(ERROR_MESSAGE,"makehaptic.  Could not create group gstSeparator");
											return_code=0;
										}
									}
									else
									{
										if ( groupNode = new gstSeparator )
										{
											while (surface)
											{
												if ( objectNode = haptic_make_surface_texmap(
													surface->pointlist,
													(GTDATA *)NULL, object->default_material,
													(Spectrum *)NULL,
														surface->n_pts1, surface->n_pts2))
												{
													groupNode->addChild( objectNode );
												}
												surface=surface->ptrnext;
											}
											objectNode = groupNode;
											return_code=1;
										}
										else
										{
											display_message(ERROR_MESSAGE,"makehaptic.  Could not create group gstSeparator");
											return_code=0;
										}
									}
								}
								else
								{
									if (proportion>0)
									{
										if ( groupNode = new gstSeparator )
										{
											while (surface&&surface_2)
											{
												if (interpolate_surface=morph_GTSURFACE(proportion,
													surface,surface_2))
												{
													if ( objectNode = haptic_make_surface_texmap(
														interpolate_surface->pointlist,
														interpolate_surface->data,
														object->default_material, object->spectrum,
														interpolate_surface->n_pts1,
														interpolate_surface->n_pts2))
													{
														groupNode->addChild( objectNode );
													}
													destroy_GTSURFACE(&interpolate_surface);
												}
												surface=surface->ptrnext;
												surface_2=surface_2->ptrnext;
											}
											objectNode = groupNode;
											return_code=1;
										}
										else
										{
											display_message(ERROR_MESSAGE,"makehaptic.  Could not create group gstSeparator");
											return_code=0;
										}
									}
									else
									{
#ifdef MERGE_TIMES
										if ( merge )
										{
											ALLOCATE(field_array, GTDATA *, object->number_of_times);
											ALLOCATE(surface_array, struct GT_surface *, object->number_of_times);
											for ( i = 0 ; i < object->number_of_times ; i++ )
											{
												if( surface_array[i] = (object->gu.gtSurface)[i] )
													field_array[i] = surface_array[i]->data;
												else
												{
													merge = 0;
													display_message(ERROR_MESSAGE,
																"makegtobject.  Invalid surface in one time");
													return_code=0;
												}
											}
											while (merge)
											{
/* 												drawdatamergesurfaceGL(surface_array[0]->pointlist,object->number_of_times,field_array, */
/* 														 object->default_material, object->spectrum, */
/* 														 surface_array[0]->n_pts1, surface_array[0]->n_pts2); */
												for ( i = 0 ; i < object->number_of_times ; i++ )
												{
													if( surface_array[i]=surface_array[i]->ptrnext )
														field_array[i] = surface_array[i]->data;
													else
														merge = 0;  /* Finished */
												}
											}
											DEALLOCATE(field_array);
											DEALLOCATE(surface_array);
										}
										else
#endif
										{
											if ( groupNode = new gstSeparator )
											{
												while (surface)
												{
													if ( objectNode = haptic_make_surface_texmap(
														surface->pointlist,
														surface->data,object->default_material,
														object->spectrum,surface->n_pts1,
														surface->n_pts2))
													{
														groupNode->addChild( objectNode );
													}
													surface=surface->ptrnext;
												}
												objectNode = groupNode;
												return_code=1;
											}
											else
											{
												display_message(ERROR_MESSAGE,"makehaptic.  Could not create group node");
												return_code=0;
											}
										}
									}
								}
								return_code=1;
							} break;
							case g_OUTLINE:
							{
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										if (interpolate_surface=morph_GTSURFACE(proportion,surface,
											surface_2))
										{
/* 											drawoutlinesurfaceGL(interpolate_surface->pointlist, */
/* 												interpolate_surface->n_pts1, */
/* 												interpolate_surface->n_pts2); */
											destroy_GTSURFACE(&interpolate_surface);
										}
										surface=surface->ptrnext;
										surface_2=surface_2->ptrnext;
									}
								}
								else
								{
									while (surface)
									{
/* 										drawoutlinesurfaceGL(surface->pointlist,surface->n_pts1, */
/* 											surface->n_pts2); */
										surface=surface->ptrnext;
									}
								}
								return_code=1;
							} break;
							case g_WIREFRAME:
							{
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										if (interpolate_surface=morph_GTSURFACE(proportion,surface,
											surface_2))
										{
/* 											drawwireframesurfaceGL(interpolate_surface->pointlist, */
/* 												interpolate_surface->n_pts1, */
/* 												interpolate_surface->n_pts2); */
											destroy_GTSURFACE(&interpolate_surface);
										}
										surface=surface->ptrnext;
										surface_2=surface_2->ptrnext;
									}
								}
								else
								{
									while (surface)
									{
/* 										drawwireframesurfaceGL(surface->pointlist,surface->n_pts1, */
/* 											surface->n_pts2); */
										surface=surface->ptrnext;
									}
								}
								return_code=1;
							} break;
							case g_SH_DISCONTINUOUS:
							{
								if (g_NO_DATA==surface->data_type)
								{
									if (proportion>0)
									{
										while (surface&&surface_2)
										{
											if (interpolate_surface=morph_GTSURFACE(proportion,
												surface,surface_2))
											{
/* 												draw_dc_surfaceGL(interpolate_surface->pointlist, */
/* 													interpolate_surface->n_pts1, */
/* 													interpolate_surface->n_pts2); */
												destroy_GTSURFACE(&interpolate_surface);
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
											if (interpolate_surface=morph_GTSURFACE(proportion,
												surface,surface_2))
											{
/* 												draw_data_dc_surfaceGL(interpolate_surface->pointlist, */
/* 													interpolate_surface->data, */
/* 													object->default_material, object->spectrum, */
/* 													interpolate_surface->n_pts1, */
/* 													interpolate_surface->n_pts2); */
												destroy_GTSURFACE(&interpolate_surface);
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
							case g_SH_DISCONTINUOUS_TEXMAP:
							{
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										if (interpolate_surface=morph_GTSURFACE(proportion,surface,
											surface_2))
										{
/* 											draw_dc_surface_texmapGL(interpolate_surface->pointlist, */
/* 												interpolate_surface->n_pts1, */
/* 												interpolate_surface->n_pts2); */
											destroy_GTSURFACE(&interpolate_surface);
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
							case g_SH_DISCONTINUOUS_RAMP:
							{
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										if (interpolate_surface=morph_GTSURFACE(proportion,surface,
											surface_2))
										{
/* 											draw_dcramp_surfaceGL(interpolate_surface->pointlist, */
/* 												interpolate_surface->n_pts1, */
/* 												interpolate_surface->n_pts2); */
											destroy_GTSURFACE(&interpolate_surface);
										}
										surface=surface->ptrnext;
										surface_2=surface_2->ptrnext;
									}
								}
								else
								{
									while (surface)
									{
/* 										draw_dcramp_surfaceGL(surface->pointlist,surface->n_pts1, */
/* 											surface->n_pts2); */
										surface=surface->ptrnext;
									}
								}
								return_code=1;
							} break;
							case g_SH_DISCONTINUOUS_RAMP_MORPH:
							{
								while (surface&&surface_2)
								{
									/*???DB.  Why 2*n+1 ? */
									for (i=0;i<(2*(surface->n_pts1)+1)*(surface->n_pts2);i++)
									{
										for (j=0;j<3;j++)
										{
											surface->pointlist[i][j]=
												(1-u)*(surface->mpointlist)[0][i][j]+
												u*(surface->mpointlist)[1][i][j];
										}
									}
/* 									draw_dcramp_surfaceGL(surface->pointlist,surface->n_pts1, */
/* 										surface->n_pts2); */
									surface=surface->ptrnext;
								}
								return_code=1;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"makehaptic.  Invalid surface type");
								return_code=0;
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makehaptic.  Missing surface");
						return_code=0;
					}
				} break;
				case g_NURBS:
				{
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
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"makehaptic.  Invalid nurbs_type");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makehaptic.  Missing nurbs");
						return_code=0;
					}
				} break;
				case g_ELEMENT_GROUP:
				{
					if (gt_element_group= *(object->gu.gt_element_group))
					{
						/* do nothing */
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"makehaptic.  Missing graphical element group");
						return_code=0;
					}
				} break;
				case g_USERDEF:
				{
					if (userdef=(object->gu.gtUserDef)[itime])
					{
						if (userdef->render)
						{
							(userdef->render)(userdef->data);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"makehaptic.  Missing render function user defined object");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makehaptic.  Missing userdef");
						return_code=0;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"makehaptic.  Invalid object type");
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"makehaptic.  Missing object");
		return_code=0;
	}
	LEAVE;

	return (objectNode);
} /* makehaptic */

static int haptic_create_window_object(struct Window_object *window_object,
	void *haptic_scene_data_void)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Creates the haptic class objects representing the given window object
==============================================================================*/
{
	gstTransform *haptic_scene;
	int return_code;
	struct Haptic_scene_data *haptic_scene_data;
	struct GT_object *gt_object;

	ENTER(haptic_create_window_object);

	/* check arguments */
	if (window_object&&(gt_object=Window_object_get_gt_object(window_object))&&
		(haptic_scene_data = (struct Haptic_scene_data *)haptic_scene_data_void))
	{
		if ((g_VISIBLE==Window_object_get_visibility(window_object))&&
			(g_CREATED==gt_object->status))
		{
			if ( haptic_scene = makehaptic( gt_object,
				Scene_time(haptic_scene_data->scene), 0.0, 0.0 ))
			{
				haptic_scene_data->top->addChild(haptic_scene);
				return_code = 1;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"haptic_create_window_object.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* haptic_create_window_object */
#endif /* HAPTIC */

int haptic_create_scene( struct Scene *scene )
/*******************************************************************************
LAST MODIFIED : 2 March 1998

DESCRIPTION :
Creates the haptic class objects representing the given window scene
==============================================================================*/
{
	int return_code;
#ifdef HAPTIC
	gstSeparator *groupNode;
	gstSphere *sphere;
	struct Haptic_scene_data haptic_scene_data;
#endif /* HAPTIC */

	ENTER (haptic_create_scene);

#ifdef HAPTIC
	if ( scene )
	{
		if ( current_scene )
		{
			/* SAB Does deleting the parent delete all its children?
			Probably not so we have to follow down the tree ourselves.
			Instead the program should dynamically change the scene
			according to scene update messages.  Will need to have some
			pointer in scene objects to the corresponding haptic object. */
			delete current_scene;
		}
		if ( phantom && scene && model )
		{
			if ( groupNode = new gstSeparator )
			{
				haptic_scene_data.scene = scene;
				haptic_scene_data.top = groupNode;
				return_code=for_each_Window_object_in_Scene(scene,
					haptic_create_window_object, (void *)&haptic_scene_data);

				current_scene = groupNode;
				model->addChild ( current_scene );

				// Create a sphere.
				//sphere = new gstSphere;
				//sphere->setRadius(30);
				// Position by default is at origin.  Keep it there
				//current_scene->addChild(sphere);

			}
			else
			{
				return_code = 0;
				display_message(ERROR_MESSAGE,
						"haptic_create_scene.  Unable to create new gstSeparator");
		}
		}
		else
		{
			return_code = 0;
			display_message(ERROR_MESSAGE,
				"haptic_create_scene.  Phantom device not initialised (must be enabled as an input device)");
		}
	}
	else
	{
		return_code = 0;
		display_message(ERROR_MESSAGE,
			"haptic_create_scene.  Missing scene");
	}
#else /* HAPTIC */
	USE_PARAMETER(scene);
	return_code = 0;
	display_message(ERROR_MESSAGE,
		"haptic_create_scene.  Haptic support was not enabled when compiled");
#endif /* HAPTIC */

	LEAVE;

	return ( return_code );
} /* haptic_create_scene */

int haptic_set_surface_defaults( float dynamic_friction, float static_friction,
	float damping, float spring_k )
/*******************************************************************************
LAST MODIFIED : 2 March 1998

DESCRIPTION :
Sets the default force response of surfaces in the model
==============================================================================*/
{
	int return_code;

	ENTER (haptic_set_surface_defaults);
#ifdef HAPTIC
		gstShape::setDefaultSurfaceFdynamic( dynamic_friction );
	gstShape::setDefaultSurfaceFstatic( static_friction );
	gstShape::setDefaultSurfaceKdamping( damping );
	gstShape::setDefaultSurfaceKspring( spring_k );
	return_code = 1;
#else /* HAPTIC */
	USE_PARAMETER(dynamic_friction);
	USE_PARAMETER(static_friction);
	USE_PARAMETER(damping);
	USE_PARAMETER(spring_k);
	return_code = 0;
	display_message(ERROR_MESSAGE, "haptic_set_surface_defaults.  "
		"Haptic support was not enabled when compiled");
#endif /* HAPTIC */
	LEAVE;

	return ( return_code );
} /* haptic_set_surface_defaults */

int haptic_set_scale ( double x, double y, double z)
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Sets the absolute values of the scale of the haptic environment
==============================================================================*/
{
	int return_code;

	ENTER (haptic_set_scale);
	USE_PARAMETER(y);
	USE_PARAMETER(z);
#ifdef HAPTIC
	if ( model )
	{
		/* gstSeparator nodes do not support nonuniform scaling */
		phantomSep->setScale( x );
		return_code = 1;
	}
	else
	{
		return_code = 0;
		display_message(ERROR_MESSAGE,
			"haptic_set_scale.  Haptic space not initialised");
	}

#else /* HAPTIC */
	USE_PARAMETER(x);
	return_code = 0;
	display_message(ERROR_MESSAGE,
		"haptic_set_scale.  Haptic support was not enabled when compiled");
#endif /* HAPTIC */
	LEAVE;

	return ( return_code );
} /* haptic_set_scale */
