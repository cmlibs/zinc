/*******************************************************************************
FILE : import_graphics_object.c

LAST MODIFIED : 19 March 2003

DESCRIPTION :
Functions for reading graphics object data from a file.
???DB.  7 June 1994.  Merged GTTEXT into GTPOINT and GTPOINTSET and added a
	marker type and a marker size
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/graphics_object.h"
#include "graphics/import_graphics_object.h"
#include "graphics/userdef_objects.h"
#include "graphics/volume_texture.h"
#include "user_interface/message.h"

/*
Module functions
----------------
*/
#define FLOAT_ZERO_TOLERANCE (1e-8)

static int normalize(float vector[3])
/*******************************************************************************
LAST MODIFIED : 27 December 1995

DESCRIPTION :
Normalizes the given <vector>.
???DB.  Move to geometry.c ?
==============================================================================*/
{
	float norm;
	int return_code;

	ENTER(normalize);
	if (vector)
	{
		if ((norm=vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2])>0)
		{
			norm=(float)sqrt((double)norm);
			if (norm > FE_VALUE_ZERO_TOLERANCE)
			{
				vector[0] /= norm;
				vector[1] /= norm;
				vector[2] /= norm;
			}
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* normalize */

static int normalized_crossproduct(float vector_1[3],float vector_2[3],
	float result[3])
/*******************************************************************************
LAST MODIFIED : 28 December 1995

DESCRIPTION :
Calculates the normalized cross product of <vector_1> and <vector_2> and puts
it in <result>.
???DB.  Move to geometry.c ?
==============================================================================*/
{
	int return_code;

	ENTER(normalized_crossproduct);
	if (vector_1&&vector_2&&result)
	{
		result[0]=vector_1[1]*vector_2[2] - vector_2[1]*vector_1[2];
		result[1]=vector_1[2]*vector_2[0] - vector_2[2]*vector_1[0];
		result[2]=vector_1[0]*vector_2[1] - vector_1[1]*vector_2[0];
		return_code=normalize(result);
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* normalized_crossproduct */

static int file_read_GT_object_type(FILE *file,
	enum GT_object_type *object_type)
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Reads an object type, as a string, from a <file> and translates it into the
enumerated <object_type>.
==============================================================================*/
{
	char *type_string;
	int return_code;

	ENTER(file_read_GT_object_type);
	if (file&&object_type)
	{
		if (read_string(file,"s",&type_string))
		{
			return_code=get_GT_object_type_from_string(type_string,object_type);
			DEALLOCATE(type_string);
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"file_read_GT_object_type.  Error reading object type string");
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"file_read_GT_object_type.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* file_read_GT_object_type */

static int file_read_GT_polyline_type(FILE *file,
	enum GT_polyline_type *polyline_type)
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Reads an polyline type, as a string, from a <file> and translates it into the
enumerated <polyline_type>.
==============================================================================*/
{
	char *type_string;
	int return_code;

	ENTER(file_read_GT_polyline_type);
	if (file&&polyline_type)
	{
		if (read_string(file,"s",&type_string))
		{
			return_code=get_GT_polyline_type_from_string(type_string,polyline_type);
			DEALLOCATE(type_string);
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"file_read_GT_polyline_type.  Error reading polyline type string");
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"file_read_GT_polyline_type.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* file_read_GT_polyline_type */

static int file_read_GT_surface_type(FILE *file,
	enum GT_surface_type *surface_type)
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Reads an surface type, as a string, from a <file> and translates it into the
enumerated <surface_type>.
==============================================================================*/
{
	char *type_string;
	int return_code;

	ENTER(file_read_GT_surface_type);
	if (file&&surface_type)
	{
		if (read_string(file,"s",&type_string))
		{
			return_code=get_GT_surface_type_from_string(type_string,surface_type);
			DEALLOCATE(type_string);
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"file_read_GT_surface_type.  Error reading surface type string");
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"file_read_GT_surface_type.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* file_read_GT_surface_type */

/*
Global functions
----------------
*/
int file_read_graphics_objects(char *file_name,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct LIST(GT_object) *object_list)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
???RC.  26 June 97.  Added nurbs_type for NURBS. NURBSMORPH are NURBS with
	nurbs_type=1
==============================================================================*/
{
	int return_code;
	FILE *fp;
	gtObject *obj;
	char objname[100];
	int dummy, i,j,k;
	int n_data_components, npts1,npts2;
	Triple *pointlist,*normallist,*texturelist;
	enum GT_surface_type surface_type;
	enum GT_polyline_type polyline_type;
	GTDATA *data;
	int sorder,torder,corder;
	int sknotcnt,tknotcnt,cknotcnt,pwlcnt,ccount;
	int maxs,maxt;
	double *sknots,*tknots,*cknots;
	double *controlpts,*trimarray,*pwlarray;
	enum GT_object_type object_type;
	gtMatrix transform;
	gtTransformType transtype;
	struct Graphical_material *object_material;
	struct GT_nurbs *nurbs;
	struct GT_point *point;
	struct GT_pointset *pointset;
	struct GT_polyline *polyline;
	struct GT_surface *surface;
	struct GT_userdef *userdef;
	int version;
	float time;

	ENTER(file_read_graphics_objects);
#if defined (DEBUG)
	/*???debug*/
	printf("ENTER(file_read_graphics_objects)\n");
#endif /* defined (DEBUG) */
	if (file_name)
	{
		/* files without a header default to 1 */
		version=1;
		if (fp = fopen(file_name,"r"))
		{
			return_code=1;
			while (return_code&&(fscanf(fp,"%s",objname)!=EOF)&&
				(strcmp(objname,"END_OF_FILE")))
			{
#if defined (DEBUG)
				/*???debug*/
				printf("\nReading object: %s\n",objname);
				display_message(INFORMATION_MESSAGE,"Reading object: %s\n",objname);
#endif /* defined (DEBUG) */
				if (fuzzy_string_compare(objname,"CMISS"))
				{
					/* version information */
					if (EOF==(fscanf(fp,"%s",objname))||
						(!fuzzy_string_compare(objname,"exobj")))
					{
						display_message(WARNING_MESSAGE,
							"file_read_graphics_objects.  Header does not have exobj "
							"keyword\nValid header is 'CMISS exobj File Version 3'\n");
					}
					if ((EOF==fscanf(fp,"%s",objname))||
						(!fuzzy_string_compare(objname,"file")))
					{
						display_message(WARNING_MESSAGE,
							"file_read_graphics_objects.  Header does not have file keyword\n"
							"Valid header is 'CMISS exobj File Version 3'\n");						
					}
					if ((EOF==fscanf(fp,"%s",objname))||
						(!fuzzy_string_compare(objname,"version")))
					{
						display_message(WARNING_MESSAGE,
							"file_read_graphics_objects.  Header does not have version "
							"keyword\nValid header is 'CMISS exobj File Version 2'\n");
					}
					if (EOF==fscanf(fp,"%d",&version))
					{
						display_message(ERROR_MESSAGE,
							"file_read_graphics_objects. Unable to read version");
						return_code=0;
					}
					fscanf(fp,"%s",objname);
				}
				if (return_code)
				{
					/* read the object type */
					if (file_read_GT_object_type(fp,&object_type))
					{
						if(version < 3)
						{
							fscanf(fp,"%d",&dummy);
							fscanf(fp,"%d",&dummy);
							time = 0;
							display_message(WARNING_MESSAGE,
								"file_read_graphics_objects.  Activity type and default attribute are obsolete, values ignored\n");
						}
						else
						{
							fscanf(fp,"%f", &time);
						}
						file_read_Graphical_material_name(fp,&object_material,
							graphical_material_manager);
						if (version<2)
						{
							transtype=g_ID;
							for (i=0;i<4;i++)
							{
								for(j=0;j<4;j++)
								{
									fscanf(fp,"%f",&(transform[i][j]));
									if (((i==j) && (transform[i][j] != 1)) ||
										((i != j) && (transform[i][j] != 0)))
									{
										transtype = g_NOT_ID;
									}
								}
							}
							if(transtype == g_NOT_ID)
							{
								display_message(WARNING_MESSAGE,
									"file_read_graphics_objects.  Transformation in .exobj file "
									"is now obsolete\nIt will be ignored!\n"
									"Instead add the following command to your comfile after the"
									"object has been drawn.\n"
									"gfx set transformation name %s %f %f %f %f %f %f %f %f %f "
									"%f %f %f %f %f %f %f\n",
									objname,transform[0][0],transform[0][1],transform[0][2],
									transform[0][3],transform[1][0],transform[1][1],
									transform[1][2],transform[1][3],transform[2][0],
									transform[2][1],transform[2][2],transform[2][3],
									transform[3][0],transform[3][1],transform[3][2],
									transform[3][3]);
							}
						}
						if (obj=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
							objname,object_list))
						{
							if (GT_object_has_time(obj,time))
							{
								display_message(WARNING_MESSAGE,
									"Overwriting time %g in graphics object '%s'",time,
									objname);
								return_code = GT_object_remove_primitives_at_time(obj, time,
									(GT_object_primitive_object_name_conditional_function *)NULL,
									(void *)NULL);
							}
						}
						else
						{
							obj=CREATE(GT_object)(objname,object_type,object_material);
						}
#if defined (DEBUG)
						/*???debug */
						printf("object type = %d (%s)\n",object_type,
							get_GT_object_type_string(object_type));
						printf("object name = %s\n",objname);
#endif /* defined (DEBUG) */
						switch (object_type)
						{
							case g_POINT:
							{
								/*???DB.  Check allocation */
								ALLOCATE(pointlist,Triple,1);
								for (i=0;i<3;i++)
								{
									fscanf(fp,"%f",&((*pointlist)[i]));
								}
								/*???DB.  Merging GTTEXT into GTPOINT and GTPOINTSET */
								point = CREATE(GT_point)(pointlist,(char *)NULL,
									g_PLUS_MARKER,global_point_size,g_NO_DATA,
									/*object_name*/0,(GTDATA *)NULL);
								GT_OBJECT_ADD(GT_point)(obj,time,point);
							} break;
							case g_POINTSET:
							{
#if defined (DEBUG)
								/*???debug */
								printf("Reading g_POINTSET\n");
#endif /* defined (DEBUG) */
								fscanf(fp,"%d",&npts1);
								/*???DB.  Check allocation */
								ALLOCATE(pointlist,Triple,npts1);
								for (j=0;j<npts1;j++)
								{
									for (i=0;i<3;i++)
									{
										fscanf(fp,"%f",&(pointlist[j][i]));
									}
								}
								/*???DB.  Merging GTTEXT into GTPOINT and GTPOINTSET */
								pointset = CREATE(GT_pointset)(npts1,pointlist,
									(char **)NULL,g_PLUS_MARKER,global_point_size,g_NO_DATA,
									(GTDATA *)NULL,(int *)NULL);
								GT_OBJECT_ADD(GT_pointset)(obj,time,pointset);
#if defined (DEBUG)
								/*???debug */
								printf(" end of g_POINTSET\n");
#endif /* defined (DEBUG) */
							} break;
							case g_POLYLINE:
							{
#if defined (DEBUG)
								/*???debug */
								printf("Reading g_POLYLINE\n");
#endif /* defined (DEBUG) */
								/*fscanf(fp,"%d",&polyline_type);*/
								if (file_read_GT_polyline_type(fp,&polyline_type))
								{
#if defined (DEBUG)
									/*???debug */
									printf("  polyline_type = %d (%s)\n",polyline_type,
										get_GT_polyline_type_string(polyline_type));
#endif /* defined (DEBUG) */
									fscanf(fp,"%d",&npts1);
									/* must clear the following since passed directly to
										 CREATE(GT_surface) unless specifically allocated */
									pointlist=(Triple *)NULL;
									normallist = (Triple *)NULL;
									data=(GTDATA *)NULL;
									switch (polyline_type)
									{
										case g_PLAIN:
										{
											ALLOCATE(pointlist,Triple,npts1);
											for (i=0;i<npts1;i++)
											{
												for (j=0;j<3;j++)
												{
													fscanf(fp,"%f",&(pointlist[i][j]));
												}
											}
										} break;
										case g_PLAIN_DISCONTINUOUS:
										{
											ALLOCATE(pointlist,Triple,2*npts1);
											for (i=0;i<2*npts1;i++)
											{
												for (j=0;j<3;j++)
												{
													fscanf(fp,"%f",&(pointlist[i][j]));
												}
											}
										} break;
										case g_NORMAL_DISCONTINUOUS:
										{
											ALLOCATE(pointlist,Triple,2*npts1);
											ALLOCATE(normallist,Triple,2*npts1);
											for (i=0;i<2*npts1;i++)
											{
												for (j=0;j<3;j++)
												{
													fscanf(fp,"%f",&(pointlist[i][j]));
												}
												for (j=0;j<3;j++)
												{
													fscanf(fp,"%f",&(normallist[i][j]));
												}
											}
										} break;
										case g_NORMAL:
										{
											ALLOCATE(pointlist,Triple,npts1);
											ALLOCATE(normallist,Triple,npts1);
											for (i=0;i<npts1;i++)
											{
												for (j=0;j<3;j++)
												{
													fscanf(fp,"%f",&(pointlist[i][j]));
												}
												for (j=0;j<3;j++)
												{
													fscanf(fp,"%f",&(normallist[i][j]));
												}
											}
										} break;
									}
									polyline = CREATE(GT_polyline)(polyline_type,
										/*line_width=default*/0,npts1,
										pointlist, normallist, g_NO_DATA, (GTDATA *)NULL);
									GT_OBJECT_ADD(GT_polyline)(obj,time,polyline);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"file_read_graphics_objects.  Unknown polyline type");
									return_code=0;
								}
#if defined (DEBUG)
								/*???debug */
								printf(" end of g_POLYLINE\n");
#endif /* defined (DEBUG) */
							} break;
							case g_SURFACE:
							{
#if defined (DEBUG)
								/*???debug */
								printf("Reading g_SURFACE\n");
#endif /* defined (DEBUG) */
								/*fscanf(fp,"%d",&surface_type);*/
								if (file_read_GT_surface_type(fp,&surface_type))
								{
#if defined (DEBUG)
									/*???debug */
									printf("  surface_type = %d (%s)\n",surface_type,
										get_GT_surface_type_string(surface_type));
#endif /* defined (DEBUG) */
									fscanf(fp,"%d",&n_data_components);
									fscanf(fp,"%d",&npts1);
									fscanf(fp,"%d",&npts2);
									/* must clear the following since passed directly to
										 CREATE(GT_surface) unless specifically allocated */
									pointlist=(Triple *)NULL;
									normallist=(Triple *)NULL;
									texturelist=(Triple *)NULL;
									data=(GTDATA *)NULL;
									/* Note: A Discontinuous surface is set up differently from
										the more economical continuous surface in which points and
										normals are shared between adjacent polys in an array
										[npts1][npts2][2]. The discontinuous surface is made from
										discrete polygons in an array [npolys][npoints/poly][2]. */
									switch (surface_type)
									{
										case g_SH_DISCONTINUOUS:
										case g_SH_DISCONTINUOUS_STRIP:
										{
#if defined (DEBUG)
											/*???debug */
											printf("g_SH_DISCONTINUOUS\n");
#endif /* defined (DEBUG) */
											ALLOCATE(pointlist,Triple,npts1*npts2);
											ALLOCATE(normallist,Triple,npts1*npts2);
											for (i=0;i<npts1;i++)
											{
												for (j=0;j<npts2;j++)
												{
													for (k=0;k<3;k++)
													{
														fscanf(fp,"%f",&(pointlist[j+npts2 * i][k]));
													}
													for (k=0;k<3;k++)
													{
														fscanf(fp,"%f",&(normallist[j+npts2*i][k]));
													}
												}
											}
											if (n_data_components)
											{
												ALLOCATE(data,GTDATA,n_data_components*npts1*npts2);
												for (i=0;i<npts1;i++)
												{
													for (j=0;j<npts2;j++)
													{
														for (k=0;k<n_data_components;k++)
														{
															fscanf(fp,"%f",&(data[k+n_data_components*(j+npts2*i)]));
														}
													}
												}
											}
										} break;
										default:      /* continuous types */
										{
											/* Only reads points, and normals, No texture coordinates */
#if defined (DEBUG)
											/*???debug */
											printf("default\n");
#endif /* defined (DEBUG) */
											ALLOCATE(pointlist,Triple,npts1*npts2);
											ALLOCATE(normallist,Triple,npts1*npts2);
											for (i=0;i<npts1;i++)
											{
												for (j=0;j<npts2;j++)
												{
													for (k=0;k<3;k++)
													{
														fscanf(fp,"%f",&(pointlist[i+npts1 * j][k]));
													}
													for (k=0;k<3;k++)
													{
														fscanf(fp,"%f",
															&(normallist[i+npts1 * j][k]));
													}
												}
											}
											if (n_data_components)
											{
												ALLOCATE(data,GTDATA,n_data_components*npts1*npts2);
												for (i=0;i<npts1;i++)
												{
													for (j=0;j<npts2;j++)
													{
														for (k=0;k<n_data_components;k++)
														{
															fscanf(fp,"%f",&(data[k+n_data_components*(i+npts2*j)]));
														}
													}
												}
											}
										} break;
									}
									surface=CREATE(GT_surface)(surface_type,g_QUADRILATERAL,
										npts1,npts2,pointlist,normallist,
										/*tangentlist*/(Triple *)NULL,texturelist,
										n_data_components,data);
									GT_OBJECT_ADD(GT_surface)(obj,time,surface);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"file_read_graphics_objects.  Unknown surface type");
									return_code=0;
								}
#if defined (DEBUG)
								/*???debug */
								printf(" end of g_SURFACE\n");
#endif /* defined (DEBUG) */
							} break;
							case g_NURBS:
							{
#if defined (DEBUG)
								/*???debug */
								printf("Reading g_NURBS\n");
#endif /* defined (DEBUG) */
								/* 26 June 97. Added nurbs_type */
								if (version < 3)
								{
									display_message(WARNING_MESSAGE,
										"file_read_graphics_objects.  Nurb type now redundant, value is ignored");
									fscanf(fp,"%d",&dummy);
								}
								fscanf(fp,"%d %d %d",&sorder,&torder,&corder);
								fscanf(fp,"%d %d",&sknotcnt,&tknotcnt);
								/* must clear the following since passed directly to
									CREATE(GT_nurbs) unless specifically allocated */
								sknots=(double *)NULL;
								tknots=(double *)NULL;
								controlpts=(double *)NULL;
								cknots=(double *)NULL;
								trimarray=(double *)NULL;
								pwlarray=(double *)NULL;
								ALLOCATE(sknots,double,sknotcnt);
								for (i=0;i<sknotcnt;i++)
								{
									fscanf(fp,"%lf",&(sknots[i]));
								}
								ALLOCATE(tknots,double,tknotcnt);
								for (i=0;i<tknotcnt;i++)
								{
									fscanf(fp,"%lf",&(tknots[i]));
								}
								fscanf(fp,"%d %d",&maxs,&maxt);
								ALLOCATE(controlpts,double,4*maxs*maxt);
								for (i=0;i<maxs;i++)
								{
									for (j=0;j<maxt;j++)
									{
										fscanf(fp,"%lf %lf %lf %lf",
											&(controlpts[4*(i + maxs*j)+0]),
											&(controlpts[4*(i + maxs*j)+1]),
											&(controlpts[4*(i + maxs*j)+2]),
											&(controlpts[4*(i + maxs*j)+3]));
									}
								}
								fscanf(fp,"%d",&cknotcnt);
								if(cknotcnt)
								{
									ALLOCATE(cknots,double,cknotcnt);
									for (i=0;i<cknotcnt;i++)
									{
										fscanf(fp,"%lf",&(cknots[i]));
									}
								}
								fscanf(fp,"%d",&ccount);
								if(ccount)
								{
									ALLOCATE(trimarray,double,3*ccount);
									for (i=0;i<ccount;i++)
									{
										for (j = 0;j<3;j++)
										{
											fscanf(fp,"%lf",&(trimarray[3*i+j]));
										}
									}
								}
								fscanf(fp,"%d",&pwlcnt);
								if(pwlcnt)
								{
									ALLOCATE(pwlarray,double,3*pwlcnt);
									for (i=0;i<pwlcnt;i++)
									{
										for (j=0;j<3;j++)
										{
											fscanf(fp,"%lf",&(pwlarray[3*i+j]));
										}
									}
								}
								if(nurbs=CREATE(GT_nurbs)())
								{
									GT_nurbs_set_surface(nurbs, sorder, torder,
										sknotcnt, tknotcnt, sknots, tknots,
										maxs, maxt, controlpts);
									if(cknotcnt)
									{
										GT_nurbs_set_nurb_trim_curve(nurbs,
											corder, cknotcnt, cknots,
											ccount, trimarray);
									}
									if(pwlcnt)
									{
										GT_nurbs_set_piecewise_linear_trim_curve(nurbs,
											pwlcnt, pwlarray);
									}
									GT_OBJECT_ADD(GT_nurbs)(obj,time,nurbs);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"file_read_graphics_objects.  Unable to create GT_nurbs object.");
									return_code=0;
								}
#if defined (DEBUG)
								/*???debug */
								printf(" end of g_NURBS\n");
#endif /* defined (DEBUG) */
							} break;
							case g_USERDEF:
							{
								/*--------- application specific user defined --------*/
#if defined (DEBUG)
								/*???debug */
								printf("Reading g_USERDEF\n");
#endif /* defined (DEBUG) */
								file_read_userdef(fp,&userdef);
								GT_OBJECT_ADD(GT_userdef)(obj,time,userdef);
#if defined (DEBUG)
								/*???debug */
								printf(" end of g_USERDEF\n");
#endif /* defined (DEBUG) */
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,"file_read_graphics_objects.  "
									"Cannot read in objects of type '%s'",
									get_GT_object_type_string(object_type));
								return_code=0;
							} break;
						} /* switch (object_type) */
						if (obj)
						{
							if (return_code)
							{
								if(!FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
									objname,object_list))
								{
									ADD_OBJECT_TO_LIST(GT_object)(obj, object_list);
								}
							}
							else
							{
								DESTROY(GT_object)(&obj);
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"file_read_graphics_objects.  Unknown object type");
						return_code=0;
					}
				}
			}
			fclose(fp);
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"file_read_graphics_objects.  Could not open file");
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"file_read_graphics_objects.  Invalid argument(s)");
	}
#if defined (DEBUG)
	/*???debug */
	printf("LEAVE(file_read_graphics_objects)\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* file_read_graphics_objects */

int file_read_voltex_graphics_object_from_obj(char *file_name,
	char *graphics_object_name, enum Render_type render_type,
	float time, struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct LIST(GT_object) *object_list)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
==============================================================================*/
{
	double *iso_poly_cop;
	enum GT_voltex_type voltex_type;
	float *texturemap_coord;
	int acc_triangle_index, i, j, k, n_iso_polys, n_triangles, n_vertices, 
		return_code, *texturemap_index, triangle, *triangle_list;
	FE_value rmag, result[3], vector1[3], vector2[3], vectorsum[3], vertex0[3],
		vertex1[3], vertex2[3];
	FILE *file;
	gtObject *obj;
	char objname[100];
	struct Environment_map *environment_map;
	struct Graphical_material *material;
	struct GT_voltex *voltex;
	struct VT_volume_texture *vtexture;
	struct VT_iso_vertex *vertex_list;

	ENTER(file_read_voltex_graphics_objects_from_obj);
#if defined (DEBUG)
	/*???debug*/
	printf("ENTER(file_read_voltex_graphics_object_from_obj\n");
#endif /* defined (DEBUG) */
	return_code = 1;
	if (file_name)
	{
		if(file = fopen(file_name, "r"))
		{
			if(vtexture = CREATE(VT_volume_texture)("temp_read_volume"))
			{
				if(read_volume_texture_from_obj_file(vtexture,
					file, graphical_material_manager,
					(struct MANAGER(Environment_map) *)NULL, 0))
				{
					n_vertices = vtexture->mc_iso_surface->n_vertices;
					n_triangles = vtexture->mc_iso_surface->n_triangles;
					n_iso_polys = n_triangles;

					if (ALLOCATE(triangle_list, int, 3*n_triangles)
						&& ALLOCATE(iso_poly_cop,double,3*n_triangles*3)
						&& ALLOCATE(texturemap_coord,float,3*n_triangles*3)
						&& ALLOCATE(texturemap_index,int,3*n_triangles)
						&& ALLOCATE(vertex_list,struct VT_iso_vertex, n_vertices))
					{
						if(graphics_object_name)
						{
							sprintf(objname, "%s", graphics_object_name);
						}
						else
						{
							sprintf(objname, "%s", file_name);
						}
						if(obj=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)(
							objname, object_list))
						{
							if (g_VOLTEX==obj->object_type)
							{
								if (GT_object_has_time(obj, time))
								{
									display_message(WARNING_MESSAGE,
										"Overwriting time %g in graphics object '%s'",
										time, objname);
									return_code = GT_object_remove_primitives_at_time(obj, time,
										(GT_object_primitive_object_name_conditional_function *)NULL,
										(void *)NULL);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Object of different type named '%s' already exists",
									objname);
								return_code=0;
							}
						}
						else
						{
							obj=CREATE(GT_object)(objname, g_VOLTEX, default_material);
							if (obj)
							{
								ADD_OBJECT_TO_LIST(GT_object)(obj, object_list);
							}
						}
						switch (render_type)
						{
							case RENDER_TYPE_SHADED:
							{
								voltex_type = g_VOLTEX_SHADED_TEXMAP;
							} break;
							case RENDER_TYPE_WIREFRAME:
							{
								voltex_type = g_VOLTEX_WIREFRAME_SHADED_TEXMAP;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"file_read_voltex_graphics_object_from_obj.  "
									"Unknown render type");
								return_code = 0;
							} break;
						}
						if (return_code)
						{
							voltex = CREATE(GT_voltex)(n_triangles, n_vertices,
								triangle_list, vertex_list, 
								iso_poly_cop, texturemap_coord, texturemap_index, /*n_rep*/1,
								/*n_data_components*/0, (GTDATA *)NULL, voltex_type);
							if (voltex)
							{
								acc_triangle_index=0;
								for (i=0;i<n_iso_polys;i++)
								{
									for (j=0;j<3;j++)
									{
										triangle_list[3*acc_triangle_index+j]=
											vtexture->mc_iso_surface->
											compiled_triangle_list[i]->vertex_index[j];
										if (material = vtexture->mc_iso_surface->
											compiled_triangle_list[i]->material[j])
										{
											GT_voltex_set_triangle_vertex_material(voltex,
												acc_triangle_index, j, material);
										}
										if (environment_map = vtexture->mc_iso_surface->
											compiled_triangle_list[i]->env_map[j])
										{
											GT_voltex_set_triangle_vertex_environment_map(voltex,
												acc_triangle_index, j, environment_map);
										}
										texturemap_index[3*acc_triangle_index+j]=
											vtexture->mc_iso_surface->
											compiled_triangle_list[i]->env_map_index[j];
										texturemap_coord[3*(3*acc_triangle_index+0)+j]=
											vtexture->mc_iso_surface->
											compiled_triangle_list[i]->texture_coord[0][j];
										texturemap_coord[3*(3*acc_triangle_index+1)+j]=
											vtexture->mc_iso_surface->
											compiled_triangle_list[i]->texture_coord[1][j];
										texturemap_coord[3*(3*acc_triangle_index+2)+j]=
											vtexture->mc_iso_surface->
											compiled_triangle_list[i]->texture_coord[2][j];
										iso_poly_cop[3*(3*acc_triangle_index+0)+j]=
											vtexture->mc_iso_surface->
											compiled_triangle_list[i]->iso_poly_cop[0][j];
										iso_poly_cop[3*(3*acc_triangle_index+1)+j]=
											vtexture->mc_iso_surface->
											compiled_triangle_list[i]->iso_poly_cop[1][j];
										iso_poly_cop[3*(3*acc_triangle_index+2)+j]=
											vtexture->mc_iso_surface->
											compiled_triangle_list[i]->iso_poly_cop[2][j];
									}
									acc_triangle_index++;
								}
								for (i=0;i<n_vertices;i++)
								{
									vertex_list[i].n_ptrs=(vtexture->mc_iso_surface->
										compiled_vertex_list)[i]->n_triangle_ptrs;
									for (j=0;j<vertex_list[i].n_ptrs;j++)
									{
										vertex_list[i].ptrs[j]=(vtexture->mc_iso_surface->
											compiled_vertex_list[i]->triangle_ptrs)[j]->
											triangle_index;
									}
									vertex_list[i].coord[0] = ((vtexture->mc_iso_surface->
										compiled_vertex_list)[i]->coord)[0];
									vertex_list[i].coord[1] = ((vtexture->mc_iso_surface->
										compiled_vertex_list)[i]->coord)[1];
									vertex_list[i].coord[2] = ((vtexture->mc_iso_surface->
										compiled_vertex_list)[i]->coord)[2];
								}
								/* now calculate vertex normals in cartesian space by
									averaging normals of surrounding faces */
								/*???DB.  Can the normals be transformed ? */
								for (i=0;i<n_vertices;i++)
								{
									for (k=0;k<3;k++)
									{
										vectorsum[k]=0;
									}
									if (vertex_list[i].n_ptrs > MAXPTRS)
									{
										vertex_list[i].n_ptrs = MAXPTRS;
										display_message(ERROR_MESSAGE,
											"file_read_voltex_graphics_object_from_obj.  "
											"More than %d vertices connected to vertex %d, unable to read correctly.",
											MAXPTRS, i);
									}
									for (j=0;j<vertex_list[i].n_ptrs;j++)
									{
										triangle=vertex_list[i].ptrs[j];
										for (k=0;k<3;k++)
										{
											vertex0[k]=vertex_list[triangle_list[triangle*3+0]].coord[k];
											vertex1[k]=vertex_list[triangle_list[triangle*3+1]].coord[k];
											vertex2[k]=vertex_list[triangle_list[triangle*3+2]].coord[k];
											vector1[k]=vertex1[k]-vertex0[k];
											vector2[k]=vertex2[k]-vertex0[k];
										}
										normalized_crossproduct(vector1,vector2,result);
										for (k=0;k<3;k++)
										{
											vectorsum[k] += result[k];
										}
									}
									/* now set normal as the average & normalize */
									rmag=sqrt((double)(vectorsum[0]*vectorsum[0]+
										vectorsum[1]*vectorsum[1]+vectorsum[2]*vectorsum[2]));
									if (rmag < FE_VALUE_ZERO_TOLERANCE)
									{
										/* If the magnitude is zero then just copy the normal values */
										rmag = 1.0;
									}
									for (k=0;k<3;k++)
									{
										vertex_list[i].normal[k]=-vectorsum[k]/rmag;
										/*???Mark.  This should be + */
									}
								} /* i */
								GT_OBJECT_ADD(GT_voltex)(obj, time, voltex);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"file_read_voltex_graphics_object_from_obj.  "
									"Unable create GT_voltex");
								return_code=0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"file_read_voltex_graphics_object_from_obj.  "
							"Unable to ALLOCATE GT_voltex data");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"file_read_voltex_graphics_object_from_obj.  "
						"Unable to read obj file");
					return_code = 0;
				}
				DESTROY(VT_volume_texture)(&vtexture);
			}
			else
			{
				return_code=0;
				display_message(ERROR_MESSAGE,
					"file_read_voltex_graphics_object_from_obj.  Unable to create temporary volume texture");
			}			
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"file_read_voltex_graphics_object_from_obj.  Unable to open file %s", file_name);
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"file_read_voltex_graphics_object_from_obj.  Invalid argument(s)");
	}
#if defined (DEBUG)
	/*???debug */
	printf("LEAVE(file_read_voltex_graphics_object_from_obj)\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* file_read_voltex_graphics_object_from_obj */
