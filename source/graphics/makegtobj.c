/*******************************************************************************
FILE : makegtobj.c

LAST MODIFIED : 19 March 2003

DESCRIPTION :
Call graphics routines in the API.
???DB.  7 June 1994.  Merged GTTEXT into GTPOINT and GTPOINTSET and added a
	marker type and a marker size
???DB.  18 October 1997.  As I understand it MERGE_TIMES is a way to put
	multiple fields into a spectrum.  There needs to be a better way of doing it.
	Graphical finite elements ?
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include "general/debug.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_library.h"
#include "graphics/makegtobj.h"
#include "graphics/rendergl.h"
#include "user_interface/message.h"

int makegtobject(gtObject *object,float time,int draw_selected)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Convert graphical object into API object.
If <draw_selected> is set, only selected graphics are drawn, otherwise only
un-selected graphics are drawn.
==============================================================================*/
{
	float proportion,*times;
	int itime, name_selected, number_of_times, picking_names, return_code, strip,
		wireframe_flag;
#if defined (OPENGL_API)
	int lighting_off;
#endif /* defined (OPENGL_API) */
	struct Graphical_material *material;
	struct GT_glyph_set *interpolate_glyph_set,*glyph_set,*glyph_set_2;
	struct GT_nurbs *nurbs;
	struct GT_point *point;
	struct GT_pointset *interpolate_point_set,*point_set,*point_set_2;
	struct GT_polyline *interpolate_line,*line,*line_2;
	struct GT_surface *interpolate_surface,*surface,*surface_2;
	struct GT_userdef *userdef;
	struct GT_voltex *voltex;
	struct Multi_range *selected_name_ranges;
	struct Spectrum *spectrum;
	union GT_primitive_list *primitive_list1, *primitive_list2;

	ENTER(makegtobject);
/*???debug */
/*printf("enter makegtobject %d  %d %d %d\n",object->object_type,g_POINTSET,
	g_POLYLINE,g_SURFACE);*/
	/* check arguments */
	if (object)
	{
		return_code = 1;
		spectrum=object->spectrum;
		/* determine if picking names are to be output */
		picking_names=(GRAPHICS_NO_SELECT != GT_object_get_select_mode(object));
		/* determine which material to use */
		if (draw_selected)
		{
			material = object->selected_material;
		}
		else
		{
			material = object->default_material;
		}
		number_of_times = object->number_of_times;
		if (0 < number_of_times)
		{
			itime = number_of_times;
			if ((itime > 1) && (times = object->times))
			{
				itime--;
				times += itime;
				if (time>= *times)
				{
					proportion = 0;
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
							proportion=(time-times[0])/proportion;
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
				itime = 0;
				proportion = 0;
			}
			if (object->primitive_lists &&
				(primitive_list1 = object->primitive_lists + itime))
			{
				if (proportion > 0)
				{
					if (!(primitive_list2 = object->primitive_lists + itime + 1))
					{
						display_message(ERROR_MESSAGE,
							"makegtobject.  Invalid primitive_list");
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
					"makegtobject.  Invalid primitive_lists");
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
#if defined (OPENGL_API)
						/* store the transform attribute group to save current matrix mode
							 and GL_NORMALIZE flag. */
						glPushAttrib(GL_TRANSFORM_BIT);
						/* Must enable GL_NORMALIZE so that normals are normalized after
							 scaling and shear by the transformations in the glyph set -
							 otherwise lighting will be wrong. Since this may reduce
							 performance, only enable for glyph_sets. */
						glEnable(GL_NORMALIZE);
						if (picking_names)
						{
							glPushName(0);
						}
#endif /* defined (OPENGL_API) */
						if (proportion > 0)
						{
							glyph_set_2 = primitive_list2->gt_glyph_set.first;
							while (glyph_set&&glyph_set_2)
							{
								if (interpolate_glyph_set=morph_GT_glyph_set(proportion,
									glyph_set,glyph_set_2))
								{
									if (picking_names)
									{
										/* put out name for picking - cast to GLuint */
										glLoadName((GLuint)interpolate_glyph_set->object_name);
									}
									/* work out if subobjects selected */
									selected_name_ranges=(struct Multi_range *)NULL;
									name_selected=GT_object_is_graphic_selected(object,
										glyph_set->object_name,&selected_name_ranges);
									draw_glyphsetGL(interpolate_glyph_set->number_of_points,
										interpolate_glyph_set->point_list,
										interpolate_glyph_set->axis1_list,
										interpolate_glyph_set->axis2_list,
										interpolate_glyph_set->axis3_list,
										interpolate_glyph_set->scale_list,
										interpolate_glyph_set->glyph,
										interpolate_glyph_set->labels,
										interpolate_glyph_set->n_data_components,
										interpolate_glyph_set->data,
										interpolate_glyph_set->names,
										material,spectrum,
										draw_selected,name_selected,selected_name_ranges);
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
								if (picking_names)
								{
									/* put out name for picking - cast to GLuint */
									glLoadName((GLuint)glyph_set->object_name);
								}
								/* work out if subobjects selected */
								selected_name_ranges=(struct Multi_range *)NULL;
								name_selected=GT_object_is_graphic_selected(object,
									glyph_set->object_name,&selected_name_ranges);
								draw_glyphsetGL(glyph_set->number_of_points,
									glyph_set->point_list, glyph_set->axis1_list,
									glyph_set->axis2_list, glyph_set->axis3_list,
									glyph_set->scale_list, glyph_set->glyph,
									glyph_set->labels, glyph_set->n_data_components,
									glyph_set->data, glyph_set->names,
									material, spectrum,
									draw_selected, name_selected, selected_name_ranges);
								glyph_set=glyph_set->ptrnext;
							}
						}
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPopName();
						}
						/* restore the transform attribute group */
						glPopAttrib();
#endif /* defined (OPENGL_API) */
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing glyph_set");
						return_code=0;
					}
				} break;
				case g_POINT:
				{
					if (point = primitive_list1->gt_point.first)
					{
						draw_pointsetGL(1, point->position, &(point->text),
							point->marker_type,
							point->marker_size, /*names*/(int *)NULL, 
							point->n_data_components, point->data,
							material,spectrum);
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
#if defined (OPENGL_API)
						/* disable lighting so rendered in flat diffuse colour */
						/*???RC glPushAttrib and glPopAttrib are *very* slow */
						glPushAttrib(GL_ENABLE_BIT);
						glDisable(GL_LIGHTING);
						glDisable(GL_VERTEX_PROGRAM_ARB);
						glDisable(GL_FRAGMENT_PROGRAM_ARB);
						glDisable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
#endif /* defined (OPENGL_API) */
						if (proportion>0)
						{
							point_set_2 = primitive_list2->gt_pointset.first;
							while (point_set&&point_set_2)
							{
								if (interpolate_point_set=morph_GT_pointset(proportion,
									point_set,point_set_2))
								{
									draw_pointsetGL(interpolate_point_set->n_pts,
										interpolate_point_set->pointlist,
										interpolate_point_set->text,
										interpolate_point_set->marker_type,
										interpolate_point_set->marker_size, point_set->names,
										interpolate_point_set->n_data_components,
										interpolate_point_set->data,
										material,spectrum);
									DESTROY(GT_pointset)(&interpolate_point_set);
								}
								point_set=point_set->ptrnext;
								point_set_2=point_set_2->ptrnext;
							}
						}
						else
						{
							while (point_set)
							{
								draw_pointsetGL(point_set->n_pts,point_set->pointlist,
									point_set->text,point_set->marker_type,point_set->marker_size,
									point_set->names,point_set->n_data_components,point_set->data,
									material,spectrum);
								point_set=point_set->ptrnext;
							}
						}
#if defined (OPENGL_API)
						/* restore previous lighting state */
						glPopAttrib();
#endif /* defined (OPENGL_API) */
						return_code=1;
					}
					else
					{
						/*???debug*/printf("! makegtobject.  Missing point");
						display_message(ERROR_MESSAGE,"makegtobject.  Missing point");
						return_code=0;
					}
				} break;
				case g_VOLTEX:
				{
					voltex = primitive_list1->gt_voltex.first;
#if defined (OPENGL_API)
					/* save transformation attributes state */
					if (voltex)
					{
						if (voltex->voltex_type == g_VOLTEX_WIREFRAME_SHADED_TEXMAP)
						{
							glPushAttrib(GL_TRANSFORM_BIT | GL_POLYGON_BIT);
							glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);							
						}
						else
						{
							glPushAttrib(GL_TRANSFORM_BIT);
						}
						/*???RC Why do we need NORMALIZE on for voltex? */
						glEnable(GL_NORMALIZE);
						if (picking_names)
						{
							glPushName(0);
						}
#endif /* defined (OPENGL_API) */
						while (voltex)
						{
							/* work out if subobjects selected */
							selected_name_ranges=(struct Multi_range *)NULL;
							name_selected=GT_object_is_graphic_selected(object,
								voltex->object_name,&selected_name_ranges);
							if ((name_selected&&draw_selected)||
								((!name_selected)&&(!draw_selected)))
							{
								if (picking_names)
								{
									/* put out name for picking - cast to GLuint */
									glLoadName((GLuint)voltex->object_name);
								}
								draw_voltexGL(voltex->n_iso_polys,voltex->triangle_list,
									voltex->vertex_list,voltex->n_vertices,voltex->n_rep,
									voltex->per_vertex_materials,
									voltex->iso_poly_material_index,
									voltex->per_vertex_environment_maps,
									voltex->iso_poly_environment_map_index,
									voltex->texturemap_coord,voltex->texturemap_index,
									voltex->n_data_components,voltex->data,
									material,spectrum);
							}
							voltex=voltex->ptrnext;
						}
						return_code=1;
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPopName();
						}
						/* restore previous coloring state */
						glPopAttrib();
#endif /* defined (OPENGL_API) */
					}
				} break;
				case g_POLYLINE:
				{
					/*???debug */
					/*printf("g_POLYLINE time=%g proportion=%g\n",time,proportion);*/
					if (line = primitive_list1->gt_polyline.first)
					{
						if (proportion>0)
						{
							line_2 = primitive_list2->gt_polyline.first;
						}
#if defined (OPENGL_API)
						if (lighting_off=((g_PLAIN == line->polyline_type)||
							(g_PLAIN_DISCONTINUOUS == line->polyline_type)))
						{
							/* disable lighting so rendered in flat diffuse colour */
							/*???RC glPushAttrib and glPopAttrib are *very* slow */
							glPushAttrib(GL_ENABLE_BIT);
							glDisable(GL_LIGHTING);
							glDisable(GL_VERTEX_PROGRAM_ARB);
							glDisable(GL_FRAGMENT_PROGRAM_ARB);
							glDisable(GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
						}
						if (picking_names)
						{
							glPushName(0);
						}
#endif /* defined (OPENGL_API) */
						switch (line->polyline_type)
						{
							case g_PLAIN:
							case g_NORMAL:
							{
								if (proportion>0)
								{
									while (line&&line_2)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											line->object_name,&selected_name_ranges);
										if ((name_selected&&draw_selected)||
											((!name_selected)&&(!draw_selected)))
										{
											if (interpolate_line=morph_GT_polyline(proportion,line,
												line_2))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)interpolate_line->object_name);
												}
												draw_polylineGL(interpolate_line->pointlist,
													interpolate_line->normallist, interpolate_line->n_pts,
													interpolate_line->n_data_components,
													interpolate_line->data, material,
													spectrum);
												DESTROY(GT_polyline)(&interpolate_line);
											}
										}
										line=line->ptrnext;
										line_2=line_2->ptrnext;
									}
								}
								else
								{
									while (line)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											line->object_name,&selected_name_ranges);
										if ((name_selected&&draw_selected)||
											((!name_selected)&&(!draw_selected)))
										{
											if (picking_names)
											{
												/* put out name for picking - cast to GLuint */
												glLoadName((GLuint)line->object_name);
											}
											draw_polylineGL(line->pointlist,line->normallist,
												line->n_pts, line->n_data_components, line->data,
												material,spectrum);
										}
										line=line->ptrnext;
									}
								}
								return_code=1;
							} break;
							case g_PLAIN_DISCONTINUOUS:
							case g_NORMAL_DISCONTINUOUS:
							{
								if (proportion>0)
								{
									while (line&&line_2)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											line->object_name,&selected_name_ranges);
										if ((name_selected&&draw_selected)||
											((!name_selected)&&(!draw_selected)))
										{
											if (interpolate_line=morph_GT_polyline(proportion,line,
												line_2))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)interpolate_line->object_name);
												}
												draw_dc_polylineGL(interpolate_line->pointlist,
													interpolate_line->normallist, interpolate_line->n_pts,
													interpolate_line->n_data_components,
													interpolate_line->data,
													material,spectrum);
												DESTROY(GT_polyline)(&interpolate_line);
											}
										}
										line=line->ptrnext;
										line_2=line_2->ptrnext;
									}
								}
								else
								{
									while (line)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											line->object_name,&selected_name_ranges);
										if ((name_selected&&draw_selected)||
											((!name_selected)&&(!draw_selected)))
										{
											if (picking_names)
											{
												/* put out name for picking - cast to GLuint */
												glLoadName((GLuint)line->object_name);
											}
											draw_dc_polylineGL(line->pointlist,line->normallist, 
												line->n_pts,line->n_data_components,line->data,
												material,spectrum);
										}
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
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPopName();
						}
						if (lighting_off)
						{
							/* restore previous lighting state */
							glPopAttrib();
						}
#endif /* defined (OPENGL_API) */
					}
					else
					{
						/*???debug*/printf("! makegtobject.  Missing line");
						display_message(ERROR_MESSAGE,"makegtobject.  Missing line");
						return_code=0;
					}
				} break;
				case g_SURFACE:
				{
					if (surface = primitive_list1->gt_surface.first)
					{
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPushName(0);
						}
#endif /* defined (OPENGL_API) */
						if (proportion>0)
						{
							surface_2 = primitive_list2->gt_surface.first;
						}
						switch (surface->surface_type)
						{
							case g_SHADED:
							case g_SHADED_TEXMAP:
							case g_WIREFRAME_SHADED_TEXMAP:
							{
								if (surface->surface_type == g_WIREFRAME_SHADED_TEXMAP)
								{
									glPushAttrib(GL_POLYGON_BIT);
									glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
									wireframe_flag = 1;
								}
								else
								{
									wireframe_flag = 0;
								}
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											surface->object_name,&selected_name_ranges);
										if ((name_selected&&draw_selected)||
											((!name_selected)&&(!draw_selected)))
										{
											if (interpolate_surface=morph_GT_surface(proportion,
												surface,surface_2))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)interpolate_surface->object_name);
												}
												draw_surfaceGL(interpolate_surface->pointlist,
													interpolate_surface->normallist,
													interpolate_surface->texturelist,
													interpolate_surface->n_pts1,
													interpolate_surface->n_pts2,
													interpolate_surface->polygon,
													interpolate_surface->n_data_components,
													interpolate_surface->data,
													material, spectrum);
												DESTROY(GT_surface)(&interpolate_surface);
											}
										}
										surface=surface->ptrnext;
										surface_2=surface_2->ptrnext;
									}
								}
								else
								{
									while (surface)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											surface->object_name,&selected_name_ranges);
										if ((name_selected&&draw_selected)||
											((!name_selected)&&(!draw_selected)))
										{
											if (picking_names)
											{
												/* put out name for picking - cast to GLuint */
												glLoadName((GLuint)surface->object_name);
											}
											draw_surfaceGL(surface->pointlist, surface->normallist,
												surface->texturelist, surface->n_pts1,
												surface->n_pts2, surface->polygon,
												surface->n_data_components, surface->data,
												material, spectrum);
										}
										surface=surface->ptrnext;
									}
								}
								if (wireframe_flag)
								{
									glPopAttrib();
								}
								return_code=1;
							} break;
							case g_SH_DISCONTINUOUS:
							case g_SH_DISCONTINUOUS_STRIP:
							case g_SH_DISCONTINUOUS_TEXMAP:
							case g_SH_DISCONTINUOUS_STRIP_TEXMAP:
							{
								strip=((g_SH_DISCONTINUOUS_STRIP_TEXMAP==surface->surface_type)
									||(g_SH_DISCONTINUOUS_STRIP==surface->surface_type));
								if (proportion>0)
								{
									while (surface&&surface_2)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											surface->object_name,&selected_name_ranges);
										if ((name_selected&&draw_selected)||
											((!name_selected)&&(!draw_selected)))
										{
											if (interpolate_surface=morph_GT_surface(proportion,
												surface,surface_2))
											{
												if (picking_names)
												{
													/* put out name for picking - cast to GLuint */
													glLoadName((GLuint)interpolate_surface->object_name);
												}
												draw_dc_surfaceGL(interpolate_surface->pointlist,
													interpolate_surface->normallist,
													interpolate_surface->texturelist,
													interpolate_surface->n_pts1,
													interpolate_surface->n_pts2,
													interpolate_surface->polygon,strip,
													interpolate_surface->n_data_components,
													interpolate_surface->data,
													material,spectrum);
												DESTROY(GT_surface)(&interpolate_surface);
											}
										}
										surface=surface->ptrnext;
										surface_2=surface_2->ptrnext;
									}
								}
								else
								{
									while (surface)
									{
										/* work out if subobjects selected */
										selected_name_ranges=(struct Multi_range *)NULL;
										name_selected=GT_object_is_graphic_selected(object,
											surface->object_name,&selected_name_ranges);
										if ((name_selected&&draw_selected)||
											((!name_selected)&&(!draw_selected)))
										{
											if (picking_names)
											{
												/* put out name for picking - cast to GLuint */
												glLoadName((GLuint)surface->object_name);
											}
											draw_dc_surfaceGL(surface->pointlist,surface->normallist,
												surface->texturelist,surface->n_pts1,surface->n_pts2,
												surface->polygon,strip, surface->n_data_components,
												surface->data,material,spectrum);
										}
										surface=surface->ptrnext;
									}
								}
								return_code=1;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"makegtobject.  Invalid surface type");
								return_code=0;
							} break;
						}
#if defined (OPENGL_API)
						if (picking_names)
						{
							glPopName();
						}
#endif /* defined (OPENGL_API) */
					}
					else
					{
						/*???debug*/printf("! makegtobject.  Missing surface");
						display_message(ERROR_MESSAGE,"makegtobject.  Missing surface");
						return_code=0;
					}
				} break;
				case g_NURBS:
				{
#if defined (OPENGL_API)
					/* store transformation attributes and GL_NORMALIZE */
					glPushAttrib(GL_TRANSFORM_BIT);
					glEnable(GL_NORMALIZE);
					if (picking_names)
					{
						glPushName(0);
					}
#endif /* defined (OPENGL_API) */
					if (nurbs = primitive_list1->gt_nurbs.first)
					{
						return_code = 1;
						while(return_code && nurbs)
						{
							if (picking_names)
							{
								/* put out name for picking - cast to GLuint */
								glLoadName((GLuint)nurbs->object_name);
							}
							/* work out if subobjects selected */
							selected_name_ranges=(struct Multi_range *)NULL;
							name_selected=GT_object_is_graphic_selected(object,
								nurbs->object_name,&selected_name_ranges);
							if ((name_selected&&draw_selected)||
								((!name_selected)&&(!draw_selected)))
							{
								return_code = draw_nurbsGL(nurbs);
							}
							nurbs=nurbs->ptrnext;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing nurbs");
						return_code=0;
					}
#if defined (OPENGL_API)
					if (picking_names)
					{
						glPopName();
					}
					/* restore the transform attribute group */
					glPopAttrib();
#endif /* defined (OPENGL_API) */
				} break;
				case g_USERDEF:
				{
#if defined (OPENGL_API)
					/* save transformation attributes state */
					glPushAttrib(GL_TRANSFORM_BIT);
					glEnable(GL_NORMALIZE);
#endif /* defined (OPENGL_API) */
					if (userdef = primitive_list1->gt_userdef.first)
					{
						if (userdef->render_function)
						{
							(userdef->render_function)(userdef->data);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"makegtobject.  Missing render function user defined object");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makegtobject.  Missing userdef");
						return_code=0;
					}
#if defined (OPENGL_API)
					/* restore previous transformation attributes */
					glPopAttrib();
#endif /* defined (OPENGL_API) */
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"makegtobject.  Invalid object type");
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"makegtobject.  Missing object");
		return_code=0;
	}
/*???debug */
/*printf("leave makegtobject\n");*/
	LEAVE;

	return (return_code);
} /* makegtobject */
