/*******************************************************************************
FILE : delauney.c

LAST MODIFIED : 15 October 2000

DESCRIPTION :
Specialized implementation of Delauney triangulation for a cylinder.

???DB.  Do as 3-D instead.  Then would be general.  How to ge convex hull?
==============================================================================*/
#include <stddef.h>
/*???debug */
#include <stdio.h>
#include <math.h>
#include "general/debug.h"
#include "general/geometry.h"
#include "unemap/delauney.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
struct Delauney_triangle
/*******************************************************************************
LAST MODIFIED : 28 May 2000

DESCRIPTION :
Uses normalized theta and z for the vertex and centre coordinates.
==============================================================================*/
{
	float centre[2],radius2,vertices[6];
	int vertex_numbers[3];
}; /* struct Delauney_triangle */

/*
Module functions
----------------
*/
static int normalize_vertex(float *xyz,float minimum_z,float maximum_z,
	int number_of_cut_uv,float *cut_uv,float *uv)
/*******************************************************************************
LAST MODIFIED : 15 October 2000

DESCRIPTION :
==============================================================================*/
{
	float minimum_u,u,v;
	int lower,next,return_code,upper;

	ENTER(normalize_vertex);
	return_code=0;
	if (xyz&&(minimum_z<maximum_z)&&(2<=number_of_cut_uv)&&cut_uv&&uv)
	{
		u=(float)atan2(xyz[1],xyz[0])/(2*PI);
		v=(xyz[2]-minimum_z)/(maximum_z-minimum_z);
		lower=0;
		upper=number_of_cut_uv-1;
		while (upper-lower>1)
		{
			next=(upper+lower)/2;
			if (cut_uv[2*next+1]<v)
			{
				lower=next;
			}
			else
			{
				upper=next;
			}
		}
		minimum_u=((cut_uv[2*upper+1]-v)*cut_uv[2*upper]+
			(v-cut_uv[2*lower+1])*cut_uv[2*lower])/
			(cut_uv[2*upper+1]-cut_uv[2*lower+1]);
		if (u<minimum_u)
		{
			u += 1;
		}
		uv[0]=u;
		uv[1]=v;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"normalize_vertex.  Invalid argument(s).  %p %g %g %d %p %p",xyz,
			minimum_z,maximum_z,number_of_cut_uv,cut_uv,uv);
	}
	LEAVE;

	return (return_code);
} /* normalize_vertex */

#if defined (OLD_CODE)
static int normalize_vertex(float *xyz,float minimum_z,float maximum_z,
	float minimum_u0,float minimum_u1,float *uv)
/*******************************************************************************
LAST MODIFIED : 7 October 2000

DESCRIPTION :
==============================================================================*/
{
	float minimum_u;
	int return_code;

	ENTER(normalize_vertex);
	return_code=0;
	if (xyz&&(minimum_z<maximum_z)&&uv)
	{
		uv[0]=(float)atan2(xyz[1],xyz[0])/(2*PI);
		uv[1]=(xyz[2]-minimum_z)/(maximum_z-minimum_z);
		minimum_u=(1-uv[1])*minimum_u0+uv[1]*minimum_u1;
		if (uv[0]<minimum_u)
		{
			uv[0] += 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"normalize_vertex.  Invalid argument(s).  %p %g %g %p",xyz,minimum_z,
			maximum_z,uv);
	}
	LEAVE;

	return (return_code);
} /* normalize_vertex */
#endif /* defined (OLD_CODE) */

static int calculate_circumcentre(struct Delauney_triangle *triangle)
/*******************************************************************************
LAST MODIFIED : 15 October 2000

DESCRIPTION :
==============================================================================*/
{
	float s,temp_u,temp_v,*vertices;
	int return_code;

	ENTER(calculate_circumcentre);
	return_code=0;
	if (triangle)
	{
		vertices=triangle->vertices;
		s=(vertices[1]-vertices[5])*(vertices[2]-vertices[0])-
			(vertices[1]-vertices[3])*(vertices[4]-vertices[0]);
		if (s!=0)
		{
			s=((vertices[2]-vertices[4])*(vertices[2]-vertices[0])-
				(vertices[1]-vertices[3])*(vertices[3]-vertices[5]))/s;
			(triangle->centre)[0]=
				((vertices[0]+vertices[4])+s*(vertices[1]-vertices[5]))/2;
			(triangle->centre)[1]=
				((vertices[1]+vertices[5])+s*(vertices[4]-vertices[0]))/2;
			temp_u=(triangle->centre)[0]-vertices[0];
			temp_v=(triangle->centre)[1]-vertices[1];
			triangle->radius2=temp_u*temp_u+temp_v*temp_v;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_circumcentre.  Invalid argument");
	}
	LEAVE;

	return (return_code);
} /* calculate_circumcentre */

/*
Global functions
----------------
*/
int cylinder_delauney(int number_of_vertices,float *vertices,
	int *number_of_triangles_address,int **triangles_address)
/*******************************************************************************
LAST MODIFIED : 15 October 2000

DESCRIPTION :
Calculates the Delauney triangulation of the <vertices> on a cylinder whose axis
is z.
<number_of_vertices> is the number of vertices to be triangulated
<vertices> is an array of length 3*<number_of_vertices>, allocated by the
	function, containing the x,y,z coordinates of the vertices
<*number_of_triangles_address> is the number of triangles calculated by the
	function
<*triangles_address> is an array of length 3*<*number_of_triangles_address>
	containing the vertex numbers for each triangle
==============================================================================*/
{
	float *adjacent_angles,*adjacent_angle,*adjacent_uv,*adjacent_uvs,angle,
		*cut_uv,*cut_uv_temp,maximum_z,minimum_u0,minimum_u1,minimum_z,temp_u,
		temp_v,uv[2],*vertex;
	int *adjacent_vertices,*adjacent_vertex,*cut_vertices,*cut_vertices_temp,i,j,
		k,l,lower,m,maximum_vertex,minimum_vertex,n,next,
		number_of_adjacent_vertices,number_of_cut_uv,number_of_triangles,on_cut,
		return_code,*returned_triangles,upper,vertex_number;
	struct Delauney_triangle **temp_triangles,*triangle,**triangles;

	ENTER(cylinder_delauney);
	return_code=0;
	/* check the arguments */
	if ((1<number_of_vertices)&&vertices&&number_of_triangles_address&&
		triangles_address)
	{
		/* find the vertex with minimum z and the vertex with maximum z */
		vertex=vertices+2;
		minimum_vertex=0;
		maximum_vertex=0;
		minimum_z= *vertex;
		maximum_z=minimum_z;
		for (i=1;i<number_of_vertices;i++)
		{
			vertex += 3;
			if (*vertex<minimum_z)
			{
				minimum_vertex=i;
				minimum_z= *vertex;
			}
			else
			{
				if (*vertex>maximum_z)
				{
					maximum_vertex=i;
					maximum_z= *vertex;
				}
			}
		}
		if (minimum_z<maximum_z)
		{
			/*???debug */
			printf("minimum %g, maximum %g\n",minimum_z,maximum_z);
			number_of_triangles=2;
			number_of_cut_uv=2;
			ALLOCATE(triangles,struct Delauney_triangle *,number_of_triangles);
			ALLOCATE(cut_uv,float,2*number_of_cut_uv);
			ALLOCATE(cut_vertices,int,number_of_cut_uv);
			if (triangles&&cut_uv&&cut_vertices)
			{
				cut_uv[0]=0;
				cut_uv[1]=0;
				cut_uv[2]=0;
				cut_uv[3]=1;
#if defined (OLD_CODE)
			minimum_u0=0;
			minimum_u1=0;
			if (ALLOCATE(triangles,struct Delauney_triangle *,number_of_triangles))
			{
#endif /* defined (OLD_CODE) */
				ALLOCATE(triangles[0],struct Delauney_triangle,1);
				ALLOCATE(triangles[1],struct Delauney_triangle,1);
				if (triangles[0]&&triangles[1])
				{
					triangles[0]->vertex_numbers[0]=minimum_vertex;
					normalize_vertex(vertices+(3*minimum_vertex),minimum_z,maximum_z,
						number_of_cut_uv,cut_uv,triangles[0]->vertices);
#if defined (OLD_CODE)
					normalize_vertex(vertices+(3*minimum_vertex),minimum_z,maximum_z,
						minimum_u0,minimum_u1,triangles[0]->vertices);
#endif /* defined (OLD_CODE) */
					/*???debug */
/*					printf("minimum %d %g %g %g  %g %g\n",minimum_vertex,
						vertices[3*minimum_vertex],vertices[3*minimum_vertex+1],
						vertices[3*minimum_vertex+2],(triangles[0]->vertices)[0],
						(triangles[0]->vertices)[1]);*/
					triangles[0]->vertex_numbers[1]=maximum_vertex;
					normalize_vertex(vertices+(3*maximum_vertex),minimum_z,maximum_z,
						number_of_cut_uv,cut_uv,triangles[0]->vertices+2);
#if defined (OLD_CODE)
					normalize_vertex(vertices+(3*maximum_vertex),minimum_z,maximum_z,
						minimum_u0,minimum_u1,(triangles[0]->vertices)+2);
#endif /* defined (OLD_CODE) */
					triangles[0]->vertex_numbers[2]=minimum_vertex+number_of_vertices;
					(triangles[0]->vertices)[4]=(triangles[0]->vertices)[0]+1;
					(triangles[0]->vertices)[5]=(triangles[0]->vertices)[1];
					/*???debug */
/*					printf("maximum %d %g %g %g  %g %g\n",maximum_vertex,
						vertices[3*maximum_vertex],vertices[3*maximum_vertex+1],
						vertices[3*maximum_vertex+2],(triangles[0]->vertices)[4],
						(triangles[0]->vertices)[5]);*/
					calculate_circumcentre(triangles[0]);
					triangles[1]->vertex_numbers[0]=maximum_vertex;
					(triangles[1]->vertices)[0]=(triangles[0]->vertices)[2];
					(triangles[1]->vertices)[1]=(triangles[0]->vertices)[3];
					triangles[1]->vertex_numbers[1]=maximum_vertex+number_of_vertices;
					(triangles[1]->vertices)[2]=(triangles[1]->vertices)[0]+1;
					(triangles[1]->vertices)[3]=(triangles[1]->vertices)[1];
					triangles[1]->vertex_numbers[2]=minimum_vertex+number_of_vertices;
					(triangles[1]->vertices)[4]=(triangles[0]->vertices)[0]+1;
					(triangles[1]->vertices)[5]=(triangles[0]->vertices)[1];
					calculate_circumcentre(triangles[1]);
					cut_uv[0]=(triangles[0]->vertices)[0];
					cut_uv[2]=(triangles[0]->vertices)[2];
					cut_vertices[0]=minimum_vertex;
					cut_vertices[1]=maximum_vertex;
#if defined (OLD_CODE)
					minimum_u0=(triangles[0]->vertices)[0];
					minimum_u1=(triangles[0]->vertices)[2];
#endif /* defined (OLD_CODE) */
					return_code=1;
					i=0;
					vertex=vertices;
					while (return_code&&(i<number_of_vertices))
					{
						if ((i!=minimum_vertex)&&(i!=maximum_vertex))
						{
							/*???debug */
							printf("number_of_triangles=%d\n",number_of_triangles);
							for (l=0;l<number_of_triangles;l++)
							{
								printf("  triangle %d  %g %g  %g\n",l,triangles[l]->centre[0],
									triangles[l]->centre[1],triangles[l]->radius2);
								printf("    %d  %g %g\n",triangles[l]->vertex_numbers[0],
									(triangles[l]->vertices)[0],(triangles[l]->vertices)[1]);
								printf("    %d  %g %g\n",triangles[l]->vertex_numbers[1],
									(triangles[l]->vertices)[2],(triangles[l]->vertices)[3]);
								printf("    %d  %g %g\n",triangles[l]->vertex_numbers[2],
									(triangles[l]->vertices)[4],(triangles[l]->vertices)[5]);
							}
							normalize_vertex(vertex,minimum_z,maximum_z,number_of_cut_uv,
								cut_uv,uv);
#if defined (OLD_CODE)
							normalize_vertex(vertex,minimum_z,maximum_z,minimum_u0,minimum_u1,
								uv);
#endif /* defined (OLD_CODE) */
							/*???debug */
							printf("vertex %d %g %g %g  %g %g\n",i,vertex[0],vertex[1],
								vertex[2],uv[0],uv[1]);
							/* delete the triangles that no longer satisfy the in-circle
								criterion */
							j=0;
							k=0;
							number_of_adjacent_vertices=0;
							adjacent_vertices=(int *)NULL;
							adjacent_angles=(float *)NULL;
							adjacent_uvs=(float *)NULL;
							while (return_code&&(j<number_of_triangles))
							{
								temp_u=uv[0]-(triangles[j]->centre)[0];
								temp_v=uv[1]-(triangles[j]->centre)[1];
#if defined (NOT_DEBUG)
								/*???DB.  modulo 1 in u */
								if (temp_u>0.5)
								{
									temp_u -= 1;
								}
								else
								{
									if (temp_u< -0.5)
									{
										temp_u += 1;
									}
								}
#endif /* defined (NOT_DEBUG) */
								if (temp_u*temp_u+temp_v*temp_v<=triangles[j]->radius2)
								{
									k++;
									/* add the vertices to the list of adjacent vertices,
										ordering the adjacent vertices so that they go
										anti-clockwise around the new vertex */
									REALLOCATE(adjacent_vertex,adjacent_vertices,int,
										number_of_adjacent_vertices+4);
									REALLOCATE(adjacent_angle,adjacent_angles,float,
										number_of_adjacent_vertices+3);
									REALLOCATE(adjacent_uv,adjacent_uvs,float,
										2*number_of_adjacent_vertices+8);
									if (adjacent_vertex&&adjacent_angle&&adjacent_uv)
									{
										adjacent_vertices=adjacent_vertex;
										adjacent_angles=adjacent_angle;
										adjacent_uvs=adjacent_uv;
										for (l=0;l<3;l++)
										{
											m=0;
											vertex_number=(triangles[j]->vertex_numbers)[l];
											angle=(triangles[j]->vertices)[2*l]-uv[0];
#if defined (NOT_DEBUG)
											/*???DB.  modulo 1 in u */
											if (angle< -0.5)
											{
												angle += 1.;
											}
											else
											{
												if (angle>0.5)
												{
													angle -= 1.;
												}
											}
#endif /* defined (NOT_DEBUG) */
											angle=atan2((triangles[j]->vertices)[2*l+1]-uv[1],angle);
											while ((m<number_of_adjacent_vertices)&&
												(vertex_number!=adjacent_vertices[m])&&
												(angle<adjacent_angles[m]))
											{
												m++;
											}
											if ((m>=number_of_adjacent_vertices)||
												(vertex_number!=adjacent_vertices[m]))
											{
												adjacent_vertex=adjacent_vertices+
													number_of_adjacent_vertices;
												adjacent_angle=adjacent_angles+
													number_of_adjacent_vertices;
												adjacent_uv=adjacent_uvs+
													2*number_of_adjacent_vertices;
												for (n=number_of_adjacent_vertices;n>m;n--)
												{
													adjacent_vertex--;
													adjacent_vertex[1]=adjacent_vertex[0];
													adjacent_angle--;
													adjacent_angle[1]=adjacent_angle[0];
													adjacent_uv--;
													adjacent_uv[2]=adjacent_uv[0];
													adjacent_uv--;
													adjacent_uv[2]=adjacent_uv[0];
												}
												*adjacent_vertex=vertex_number;
												*adjacent_angle=angle;
												*adjacent_uv=(triangles[j]->vertices)[2*l];
												adjacent_uv++;
												*adjacent_uv=(triangles[j]->vertices)[2*l+1];
												number_of_adjacent_vertices++;
								/*???debug */
/*								printf("adjacent vertices\n");
								for (n=0;n<number_of_adjacent_vertices;n++)
								{
									printf("  %d.  %g %g  %g\n",adjacent_vertices[n],
										adjacent_uvs[2*n],adjacent_uvs[2*n+1],adjacent_angles[n]);
								}*/
											}
										}
									}
									else
									{
										if (adjacent_vertex)
										{
											adjacent_vertices=adjacent_vertex;
										}
										if (adjacent_angle)
										{
											adjacent_angles=adjacent_angle;
										}
										if (adjacent_uv)
										{
											adjacent_uvs=adjacent_uv;
										}
										return_code=0;
										display_message(ERROR_MESSAGE,
									"cylinder_delauney.  Could not reallocate adjacent vertices");
									}
									DEALLOCATE(triangles[j]);
								}
								else
								{
									if (k>0)
									{
										triangles[j-k]=triangles[j];
									}
								}
								j++;
							}
							if (return_code&&(k>0))
							{
								number_of_triangles -= k;;
								/* determine new triangles */
								if (REALLOCATE(temp_triangles,triangles,
									struct Delauney_triangle *,
									number_of_triangles+number_of_adjacent_vertices))
								{
									triangles=temp_triangles;
									adjacent_vertex=adjacent_vertices;
									adjacent_angle=adjacent_angles;
									adjacent_uv=adjacent_uvs;
									adjacent_vertices[number_of_adjacent_vertices]=
										adjacent_vertices[0];
									adjacent_uvs[2*number_of_adjacent_vertices]=adjacent_uvs[0];
									adjacent_uvs[2*number_of_adjacent_vertices+1]=adjacent_uvs[1];
									k=0;
									while (return_code&&(k<number_of_adjacent_vertices))
									{
										if (triangle=ALLOCATE(triangles[number_of_triangles],
											struct Delauney_triangle,1))
										{
											/* check for triangle on cut line */
											on_cut=0;
#if defined (NOT_DEBUG)
											if ((adjacent_uv[1]<uv[1])&&(uv[1]<adjacent_uv[3]))
											{
												/*???debug */
												printf("cut %g %g %g\n",adjacent_uv[3],uv[1],
													adjacent_uv[1]);
												lower=0;
												upper=number_of_cut_uv-1;
												while (upper-lower>1)
												{
													next=(upper+lower)/2;
													if (cut_uv[2*next+1]<uv[1])
													{
														lower=next;
													}
													else
													{
														upper=next;
													}
												}
												/*???debug */
												printf("  %d %d  %d %d  %d %d\n",lower,upper,
													cut_vertices[lower],adjacent_vertex[0],
													cut_vertices[upper],adjacent_vertex[1]);
												if ((cut_vertices[lower]==adjacent_vertex[0])&&
													(cut_vertices[upper]==adjacent_vertex[1]))
												{
													REALLOCATE(cut_uv_temp,cut_uv,float,
														2*number_of_cut_uv+2);
													REALLOCATE(cut_vertices_temp,cut_vertices,int,
														number_of_cut_uv+1);
													if (cut_uv_temp&&cut_vertices_temp)
													{
														cut_uv=cut_uv_temp;
														cut_vertices=cut_vertices_temp;
														on_cut=1;
														for (l=number_of_cut_uv;l>upper;l--)
														{
															cut_vertices[l]=cut_vertices[l-1];
															cut_uv[2*l]=cut_uv[2*l-2];
															cut_uv[2*l+1]=cut_uv[2*l-1];
														}
														cut_vertices[upper]=i;
														cut_uv[2*upper]=uv[0];
														cut_uv[2*upper+1]=uv[1];
														number_of_cut_uv++;
					/*???debug */
					printf("number_of_cut_uv=%d\n",number_of_cut_uv);
					for (l=0;l<number_of_cut_uv;l++)
					{
						printf("  %d  %g %g\n",cut_vertices[l],cut_uv[2*l],cut_uv[2*l+1]);
					}
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"cylinder_delauney.  Could not reallocate cut");
														return_code=0;
													}
												}
											}
#endif /* defined (NOT_DEBUG) */
											if (on_cut)
											{
												triangle->vertex_numbers[0]=i+number_of_vertices;
												(triangle->vertices)[0]=uv[0]+1;
												(triangle->vertices)[1]=uv[1];
												triangle->vertex_numbers[1]=(*adjacent_vertex)+
													number_of_vertices;
												adjacent_vertex++;
												(triangle->vertices)[2]=(*adjacent_uv)+1;
												adjacent_uv++;
												(triangle->vertices)[3]= *adjacent_uv;
												adjacent_uv++;
												triangle->vertex_numbers[2]=(*adjacent_vertex)+
													number_of_vertices;
												(triangle->vertices)[4]=adjacent_uv[0]+1;
												(triangle->vertices)[5]=adjacent_uv[1];
											}
											else
											{
												triangle->vertex_numbers[0]=i;
												(triangle->vertices)[0]=uv[0];
												(triangle->vertices)[1]=uv[1];
												triangle->vertex_numbers[1]= *adjacent_vertex;
												adjacent_vertex++;
												(triangle->vertices)[2]= *adjacent_uv;
												adjacent_uv++;
												(triangle->vertices)[3]= *adjacent_uv;
												adjacent_uv++;
												triangle->vertex_numbers[2]= *adjacent_vertex;
												(triangle->vertices)[4]=adjacent_uv[0];
												(triangle->vertices)[5]=adjacent_uv[1];
											}
											k++;
											if (calculate_circumcentre(triangle))
											{
												number_of_triangles++;
											}
											else
											{
												DEALLOCATE(triangles[number_of_triangles]);
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"cylinder_delauney.  Could not allocate triangle");
											return_code=0;
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"cylinder_delauney.  Could not reallocate triangles");
								}
							}
							DEALLOCATE(adjacent_vertices);
							DEALLOCATE(adjacent_angles);
							DEALLOCATE(adjacent_uvs);
						}
						i++;
						vertex += 3;
					}
					/*???debug */
					printf("number_of_triangles=%d (end)\n",number_of_triangles);
/*					for (l=0;l<number_of_triangles;l++)
					{
						printf("  triangle %d  %g %g  %g\n",l,triangles[l]->centre[0],
							triangles[l]->centre[1],triangles[l]->radius2);
						printf("    %d  %g %g\n",triangles[l]->vertex_numbers[0],
							(triangles[l]->vertices)[0],(triangles[l]->vertices)[1]);
						printf("    %d  %g %g\n",triangles[l]->vertex_numbers[1],
							(triangles[l]->vertices)[2],(triangles[l]->vertices)[3]);
						printf("    %d  %g %g\n",triangles[l]->vertex_numbers[2],
							(triangles[l]->vertices)[4],(triangles[l]->vertices)[5]);
					}*/
					/*???debug */
					printf("number_of_cut_uv=%d\n",number_of_cut_uv);
					for (l=0;l<number_of_cut_uv;l++)
					{
						printf("  %d  %g %g\n",cut_vertices[l],cut_uv[2*l],cut_uv[2*l+1]);
					}
					if (return_code)
					{
						/* return results */
						if (ALLOCATE(returned_triangles,int,3*number_of_triangles))
						{
							*triangles_address=returned_triangles;
							*number_of_triangles_address=number_of_triangles;
							for (i=0;i<number_of_triangles;i++)
							{
								for (j=0;j<3;j++)
								{
									*returned_triangles=((triangles[i])->vertex_numbers)[j];
									if (*returned_triangles>=number_of_vertices)
									{
										*returned_triangles -= number_of_vertices;
									}
									returned_triangles++;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"cylinder_delauney.  Could not allocate returned_triangles");
						}
					}
					/* tidy up */
					for (i=0;i<number_of_triangles;i++)
					{
						DEALLOCATE(triangles[i]);
					}
					DEALLOCATE(triangles);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"cylinder_delauney.  Could not allocate initial triangles 2");
					DEALLOCATE(triangles[0]);
					DEALLOCATE(triangles[1]);
					DEALLOCATE(triangles);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"cylinder_delauney.  Could not allocate initial triangles and cut");
			}
			DEALLOCATE(triangles);
			DEALLOCATE(cut_uv);
			DEALLOCATE(cut_vertices);
#if defined (OLD_CODE)
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"cylinder_delauney.  Could not allocate initial triangles");
			}
#endif /* defined (OLD_CODE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cylinder_delauney.  All vertices have same z.  No triangulation");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cylinder_delauney.  Invalid argument(s) %d %p %p %p",number_of_vertices,
			vertices,number_of_triangles_address,triangles_address);
	}
	LEAVE;

	return (return_code);
} /* cylinder_delauney */
