/*******************************************************************************
FILE : delauney.c

LAST MODIFIED : 4 February 2002

DESCRIPTION :
Specialized implementations of Delauney triangulation for a cylinder and a
sphere.

???DB.  Do as 3-D instead.  Then would be general.  How to get convex hull?
==============================================================================*/

/*#define DEBUG*/

#include <stddef.h>
#if defined (DEBUG)
/*???debug */
#include <stdio.h>
#endif /* defined (DEBUG) */
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
LAST MODIFIED : 14 January 2002

DESCRIPTION :
cylinder.  Uses normalized theta and z for the vertex and centre coordinates.
sphere.  Uses longitude (theta) and latitude (mu) for the vertex and centre
	coordinates
???DB.  Vertex order is important for sphere (see sphere_calculate_circumcentre)
	Can order be used for cylinder?
==============================================================================*/
{
	float centre[2],radius2,vertices[6];
	int vertex_numbers[3];
}; /* struct Delauney_triangle */

/*
Module functions
----------------
*/
static int cylinder_normalize_vertex(float *xyz,float minimum_z,float maximum_z,
	float minimum_u0,float minimum_u1,float *uv)
/*******************************************************************************
LAST MODIFIED : 7 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(cylinder_normalize_vertex);
	return_code=0;
	if (xyz&&(minimum_z<maximum_z)&&uv)
	{
		uv[0]=(float)atan2(xyz[1],xyz[0])/(2*PI);
		uv[1]=(xyz[2]-minimum_z)/(maximum_z-minimum_z);
		uv[0] -= (1-uv[1])*minimum_u0+uv[1]*minimum_u1;
		if (uv[0]<0)
		{
			uv[0] += 1;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"cylinder_normalize_vertex.  "
			"Invalid argument(s).  %p %g %g %p",xyz,minimum_z,maximum_z,uv);
	}
	LEAVE;

	return (return_code);
} /* cylinder_normalize_vertex */

static int sphere_normalize_vertex(float *xyz,float *uv)
/*******************************************************************************
LAST MODIFIED : 14 January 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(sphere_normalize_vertex);
	return_code=0;
	if (xyz&&uv)
	{
		if ((xyz[0]!=0)||(xyz[1]!=0))
		{
			/* longitude (theta) */
			uv[0]=(float)atan2(xyz[1],xyz[0]);
			/* latitude (mu) */
			uv[1]=(float)atan2(xyz[2],sqrt(xyz[0]*xyz[0]+xyz[1]*xyz[1]));
		}
		else
		{
			/* longitude (theta) */
			uv[0]=0;
			/* latitude (mu) */
			if (xyz[2]>0)
			{
				uv[1]=PI/2;
			}
			else
			{
				if (xyz[2]<0)
				{
					uv[1]= -PI/2;
				}
				else
				{
					uv[1]=0;
				}
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"sphere_normalize_vertex.  "
			"Invalid argument(s).  %p %p",xyz,uv);
	}
	LEAVE;

	return (return_code);
} /* sphere_normalize_vertex */

static int cylinder_calculate_circumcentre(struct Delauney_triangle *triangle)
/*******************************************************************************
LAST MODIFIED : 15 October 2000

DESCRIPTION :
==============================================================================*/
{
	float s,temp_u,temp_v,*vertices;
	int return_code;

	ENTER(cylinder_calculate_circumcentre);
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
		display_message(ERROR_MESSAGE,"cylinder_calculate_circumcentre.  "
			"Invalid argument");
	}
	LEAVE;

	return (return_code);
} /* cylinder_calculate_circumcentre */

static float sphere_calculate_distance(float *uv1,float *uv2)
/*******************************************************************************
LAST MODIFIED : 14 January 2002

DESCRIPTION :
uv's are longitudes and latitudes.  The distance between them is the shorter
distance along the great circle through them.
==============================================================================*/
{
	float distance,dot_product,x1,x2,y1,y2,z1,z2;

	ENTER(sphere_calculate_distance);
	distance=0;
	if (uv1&&uv2)
	{
		x1=cos(uv1[1])*cos(uv1[0]);
		y1=cos(uv1[1])*sin(uv1[0]);
		z1=sin(uv1[1]);
		x2=cos(uv2[1])*cos(uv2[0]);
		y2=cos(uv2[1])*sin(uv2[0]);
		z2=sin(uv2[1]);
		dot_product=x1*x2+y1*y2+z1*z2;
		distance=acos(dot_product);
		if (distance<0)
		{
			distance= -distance;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"sphere_calculate_distance.  "
			"Invalid argument(s).  %p %p",uv1,uv2);
	}
	LEAVE;

	return (distance);
} /* sphere_calculate_distance */

static int sphere_calculate_circumcentre(struct Delauney_triangle *triangle)
/*******************************************************************************
LAST MODIFIED : 20 January 2002

DESCRIPTION :
For any 3 points there are two choices for the centre.  The straight line in
3-space between the two points goes through the centre of the sphere.  One is
chosen based on the order of the vertices.
==============================================================================*/
{
	float centre[3],*vertices,x1,x2,x3,y1,y2,y3,z1,z2,z3;
	int return_code;

	ENTER(sphere_calculate_circumcentre);
	return_code=0;
	if (triangle)
	{
		vertices=triangle->vertices;
		x1=cos(vertices[1])*cos(vertices[0]);
		y1=cos(vertices[1])*sin(vertices[0]);
		z1=sin(vertices[1]);
		x2=cos(vertices[3])*cos(vertices[2]);
		y2=cos(vertices[3])*sin(vertices[2]);
		z2=sin(vertices[3]);
		x3=cos(vertices[5])*cos(vertices[4]);
		y3=cos(vertices[5])*sin(vertices[4]);
		z3=sin(vertices[5]);
		centre[0]=(y1-y2)*(z1-z3)-(z1-z2)*(y1-y3);
		centre[1]=(z1-z2)*(x1-x3)-(x1-x2)*(z1-z3);
		centre[2]=(x1-x2)*(y1-y3)-(y1-y2)*(x1-x3);
		if (return_code=sphere_normalize_vertex(centre,triangle->centre))
		{
			triangle->radius2=sphere_calculate_distance(triangle->centre,vertices);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"sphere_calculate_circumcentre.  "
			"Invalid argument");
	}
	LEAVE;

	return (return_code);
} /* sphere_calculate_circumcentre */

/*
Global functions
----------------
*/
int cylinder_delauney(int number_of_vertices,float *vertices,
	int *number_of_triangles_address,int **triangles_address)
/*******************************************************************************
LAST MODIFIED : 3 February 2002

DESCRIPTION :
Calculates the Delauney triangulation of the <vertices> on a cylinder whose axis
is z.
<number_of_vertices> is the number of vertices to be triangulated
<vertices> is an array of length 3*<number_of_vertices> containing the x,y,z
	coordinates of the vertices
<*number_of_triangles_address> is the number of triangles calculated by the
	function
<*triangles_address> is an array of length 3*<*number_of_triangles_address>,
	allocated by the function, containing the vertex numbers for each triangle
==============================================================================*/
{
	float *adjacent_angles,*adjacent_angle,*adjacent_uv,*adjacent_uvs,angle,
		maximum_z,minimum_u0,minimum_u1,minimum_z,temp_u,temp_v,uv[2],*vertex;
	int *adjacent_vertices,*adjacent_vertex,i,ii,j,k,l,m,maximum_vertex,
		minimum_vertex,n,number_of_adjacent_vertices,number_of_returned_triangles,
		number_of_triangles,return_code,*returned_triangles,vertex_number;
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
			minimum_u0=0;
			minimum_u1=0;
			cylinder_normalize_vertex(vertices+(3*minimum_vertex),minimum_z,maximum_z,
				minimum_u0,minimum_u1,uv);
			minimum_u0=uv[0];
			cylinder_normalize_vertex(vertices+(3*maximum_vertex),minimum_z,maximum_z,
				minimum_u0,minimum_u1,uv);
			minimum_u1=uv[0];
#if defined (DEBUG)
			/*???debug */
			printf("minimum %g %g (%d), maximum %g %g (%d)\n",minimum_u0,minimum_z,
				minimum_vertex,minimum_u1,maximum_z,maximum_vertex);
#endif /* defined (DEBUG) */
			number_of_triangles=6;
			if (ALLOCATE(triangles,struct Delauney_triangle *,number_of_triangles))
			{
				i=number_of_triangles;
				do
				{
					i--;
				}
				while ((i>=0)&&ALLOCATE(triangles[i],struct Delauney_triangle,1));
				if (i<0)
				{
					(triangles[0]->vertex_numbers)[0]=minimum_vertex-number_of_vertices;
					(triangles[0]->vertices)[0]= -1;
					(triangles[0]->vertices)[1]=0;
					(triangles[0]->vertex_numbers)[1]=maximum_vertex-number_of_vertices;
					(triangles[0]->vertices)[2]= -1;
					(triangles[0]->vertices)[3]=1;
					(triangles[0]->vertex_numbers)[2]=minimum_vertex;
					(triangles[0]->vertices)[4]=0;
					(triangles[0]->vertices)[5]=0;
					(triangles[1]->vertex_numbers)[0]=maximum_vertex-number_of_vertices;
					(triangles[1]->vertices)[0]= -1;
					(triangles[1]->vertices)[1]=1;
					(triangles[1]->vertex_numbers)[1]=maximum_vertex;
					(triangles[1]->vertices)[2]=0;
					(triangles[1]->vertices)[3]=1;
					(triangles[1]->vertex_numbers)[2]=minimum_vertex;
					(triangles[1]->vertices)[4]=0;
					(triangles[1]->vertices)[5]=0;
					(triangles[2]->vertex_numbers)[0]=minimum_vertex;
					(triangles[2]->vertices)[0]=0;
					(triangles[2]->vertices)[1]=0;
					(triangles[2]->vertex_numbers)[1]=maximum_vertex;
					(triangles[2]->vertices)[2]=0;
					(triangles[2]->vertices)[3]=1;
					(triangles[2]->vertex_numbers)[2]=minimum_vertex+number_of_vertices;
					(triangles[2]->vertices)[4]=1;
					(triangles[2]->vertices)[5]=0;
					(triangles[3]->vertex_numbers)[0]=maximum_vertex;
					(triangles[3]->vertices)[0]=0;
					(triangles[3]->vertices)[1]=1;
					(triangles[3]->vertex_numbers)[1]=maximum_vertex+number_of_vertices;
					(triangles[3]->vertices)[2]=1;
					(triangles[3]->vertices)[3]=1;
					(triangles[3]->vertex_numbers)[2]=minimum_vertex+number_of_vertices;
					(triangles[3]->vertices)[4]=1;
					(triangles[3]->vertices)[5]=0;
					(triangles[4]->vertex_numbers)[0]=minimum_vertex+number_of_vertices;
					(triangles[4]->vertices)[0]=1;
					(triangles[4]->vertices)[1]=0;
					(triangles[4]->vertex_numbers)[1]=maximum_vertex+number_of_vertices;
					(triangles[4]->vertices)[2]=1;
					(triangles[4]->vertices)[3]=1;
					(triangles[4]->vertex_numbers)[2]=minimum_vertex+2*number_of_vertices;
					(triangles[4]->vertices)[4]=2;
					(triangles[4]->vertices)[5]=0;
					(triangles[5]->vertex_numbers)[0]=maximum_vertex+number_of_vertices;
					(triangles[5]->vertices)[0]=1;
					(triangles[5]->vertices)[1]=1;
					(triangles[5]->vertex_numbers)[1]=maximum_vertex+2*number_of_vertices;
					(triangles[5]->vertices)[2]=2;
					(triangles[5]->vertices)[3]=1;
					(triangles[5]->vertex_numbers)[2]=minimum_vertex+2*number_of_vertices;
					(triangles[5]->vertices)[4]=2;
					(triangles[5]->vertices)[5]=0;
					for (i=0;i<number_of_triangles;i++)
					{
						cylinder_calculate_circumcentre(triangles[i]);
					}
					return_code=1;
					ii=0;
					vertex=vertices;
					while (return_code&&(ii<2*number_of_vertices))
					{
						i=ii/2;
						if ((i!=minimum_vertex)&&(i!=maximum_vertex))
						{
#if defined (DEBUG)
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
#endif /* defined (DEBUG) */
							cylinder_normalize_vertex(vertex,minimum_z,maximum_z,minimum_u0,
								minimum_u1,uv);
							if (1==ii%2)
							{
								if (uv[0]<0.5)
								{
									uv[0] += 1;
									i += number_of_vertices;
								}
								else
								{
									uv[0] -= 1;
									i -= number_of_vertices;
								}
							}
#if defined (DEBUG)
							/*???debug */
							printf("vertex %d %g %g %g  %g %g\n",i,vertex[0],vertex[1],
								vertex[2],uv[0],uv[1]);
#endif /* defined (DEBUG) */
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
#if defined (DEBUG)
												/*???debug */
												printf("adjacent vertices\n");
												for (n=0;n<number_of_adjacent_vertices;n++)
												{
													printf("  %d.  %g %g  %g\n",adjacent_vertices[n],
														adjacent_uvs[2*n],adjacent_uvs[2*n+1],
														adjacent_angles[n]);
												}
#endif /* defined (DEBUG) */
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
										display_message(ERROR_MESSAGE,"cylinder_delauney.  "
											"Could not reallocate adjacent vertices");
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
											k++;
											if (cylinder_calculate_circumcentre(triangle))
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
						if (1==ii%2)
						{
							vertex += 3;
						}
						ii++;
					}
#if defined (DEBUG)
					/*???debug */
					printf("number_of_triangles=%d (end)\n",number_of_triangles);
					for (l=0;l<number_of_triangles;l++)
					{
						printf("  triangle %d  %g %g  %g\n",l,triangles[l]->centre[0],
							triangles[l]->centre[1],triangles[l]->radius2);
						m=triangles[l]->vertex_numbers[0];
						if (m<0)
						{
							m += number_of_vertices;
						}
						else
						{
							if (m>=number_of_vertices)
							{
								m -= number_of_vertices;
							}
						}
						printf("    %d (%d)  %g %g  %g %g %g\n",
							triangles[l]->vertex_numbers[0],m,
							(triangles[l]->vertices)[0],(triangles[l]->vertices)[1],
							vertices[3*m],vertices[3*m+1],vertices[3*m+2]);
						m=triangles[l]->vertex_numbers[1];
						if (m<0)
						{
							m += number_of_vertices;
						}
						else
						{
							if (m>=number_of_vertices)
							{
								m -= number_of_vertices;
							}
						}
						printf("    %d (%d)  %g %g  %g %g %g\n",
							triangles[l]->vertex_numbers[1],m,
							(triangles[l]->vertices)[2],(triangles[l]->vertices)[3],
							vertices[3*m],vertices[3*m+1],vertices[3*m+2]);
						m=triangles[l]->vertex_numbers[2];
						if (m<0)
						{
							m += number_of_vertices;
						}
						else
						{
							if (m>=number_of_vertices)
							{
								m -= number_of_vertices;
							}
						}
						printf("    %d (%d)  %g %g  %g %g %g\n",
							triangles[l]->vertex_numbers[2],m,
							(triangles[l]->vertices)[4],(triangles[l]->vertices)[5],
							vertices[3*m],vertices[3*m+1],vertices[3*m+2]);
					}
#endif /* defined (DEBUG) */
					if (return_code)
					{
						/* return results */
						number_of_returned_triangles=0;
						for (i=0;i<number_of_triangles;i++)
						{
							j=2;
							while ((j>=0)&&((((triangles[i])->vertex_numbers)[j]<0)||
								(number_of_vertices<=((triangles[i])->vertex_numbers)[j])))
							{
								j--;
							}
							if (j>=0)
							{
								number_of_returned_triangles++;
							}
						}
						if (ALLOCATE(returned_triangles,int,3*number_of_returned_triangles))
						{
							*triangles_address=returned_triangles;
							*number_of_triangles_address=number_of_returned_triangles;
							for (i=0;i<number_of_triangles;i++)
							{
								j=2;
								while ((j>=0)&&((((triangles[i])->vertex_numbers)[j]<0)||
									(number_of_vertices<=((triangles[i])->vertex_numbers)[j])))
								{
									j--;
								}
								if (j>=0)
								{
									for (j=0;j<3;j++)
									{
										*returned_triangles=((triangles[i])->vertex_numbers)[j];
										if (*returned_triangles>=number_of_vertices)
										{
											*returned_triangles -= number_of_vertices;
										}
										else
										{
											if (*returned_triangles<0)
											{
												*returned_triangles += number_of_vertices;
											}
										}
										returned_triangles++;
									}
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
					i++;
					while (i<number_of_triangles)
					{
						DEALLOCATE(triangles[i]);
						i++;
					}
					DEALLOCATE(triangles);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"cylinder_delauney.  Could not allocate initial triangles");
			}
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

int sphere_delauney(int number_of_vertices,float *vertices,
	int *number_of_triangles_address,int **triangles_address)
/*******************************************************************************
LAST MODIFIED : 4 February 2002

DESCRIPTION :
Calculates the Delauney triangulation of the <vertices> on a sphere whose centre
is the origin.
<number_of_vertices> is the number of vertices to be triangulated
<vertices> is an array of length 3*<number_of_vertices> containing the x,y,z
	coordinates of the vertices
<*number_of_triangles_address> is the number of triangles calculated by the
	function
<*triangles_address> is an array of length 3*<*number_of_triangles_address>,
	allocated by the function, containing the vertex numbers for each triangle
==============================================================================*/
{
	float *adjacent_angles,*adjacent_angle,*adjacent_uv,*adjacent_uvs,angle,
		length,uv[2],*vertex,xref1,xref2,x1,x2,x3,yref1,yref2,y1,y2,y3,zref1,zref2,
		z1,z2,z3;
	int *adjacent_vertices,*adjacent_vertex,i,j,k,l,m,n,
		number_of_adjacent_vertices,number_of_returned_triangles,
		number_of_triangles,return_code,*returned_triangles,vertex_number;
	struct Delauney_triangle **temp_triangles,*triangle,**triangles;

	ENTER(sphere_delauney);
	return_code=0;
	/* check the arguments */
	if ((2<number_of_vertices)&&vertices&&number_of_triangles_address&&
		triangles_address)
	{
		number_of_triangles=2;
		if (ALLOCATE(triangles,struct Delauney_triangle *,number_of_triangles))
		{
			i=number_of_triangles;
			do
			{
				i--;
			}
			while ((i>=0)&&ALLOCATE(triangles[i],struct Delauney_triangle,1));
			if (i<0)
			{
				/* initial triangulation covers the sphere */
				(triangles[0]->vertex_numbers)[0]=0;
				sphere_normalize_vertex(vertices,triangles[0]->vertices);
				(triangles[0]->vertex_numbers)[1]=1;
				sphere_normalize_vertex(vertices+3,(triangles[0]->vertices)+2);
				(triangles[0]->vertex_numbers)[2]=2;
				sphere_normalize_vertex(vertices+6,(triangles[0]->vertices)+4);
				(triangles[1]->vertex_numbers)[0]=0;
				sphere_normalize_vertex(vertices,triangles[1]->vertices);
				(triangles[1]->vertex_numbers)[1]=2;
				sphere_normalize_vertex(vertices+6,(triangles[1]->vertices)+2);
				(triangles[1]->vertex_numbers)[2]=1;
				sphere_normalize_vertex(vertices+3,(triangles[1]->vertices)+4);
				for (i=0;i<number_of_triangles;i++)
				{
					sphere_calculate_circumcentre(triangles[i]);
				}
				return_code=1;
				i=3;
				vertex=vertices+9;
				while (return_code&&(i<number_of_vertices))
				{
#if defined (DEBUG)
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
#endif /* defined (DEBUG) */
					sphere_normalize_vertex(vertex,uv);
#if defined (DEBUG)
					/*???debug */
					printf("vertex %d %g %g %g  %g %g\n",i,vertex[0],vertex[1],
						vertex[2],uv[0],uv[1]);
#endif /* defined (DEBUG) */
					/* delete the triangles that no longer satisfy the in-circle
						criterion */
					j=0;
					k=0;
					number_of_adjacent_vertices=0;
					adjacent_vertices=(int *)NULL;
					adjacent_angles=(float *)NULL;
					adjacent_uvs=(float *)NULL;
					x1=cos(uv[1])*cos(uv[0]);
					y1=cos(uv[1])*sin(uv[0]);
					z1=sin(uv[1]);
					/* calculate a coordinate vectors for the plane perpendicular to
						(x1,y1,z1) */
					if (x1<0)
					{
						x2= -x1;
					}
					else
					{
						x2=x1;
					}
					if (y1<0)
					{
						y2= -y1;
					}
					else
					{
						y2=y1;
					}
					if (z1<0)
					{
						z2= -z1;
					}
					else
					{
						z2=z1;
					}
					if (x2>y2)
					{
						if (x2>z2)
						{
							if (y2>z2)
							{
								xref1=y1;
								yref1= -x1;
								zref1=0;
							}
							else
							{
								xref1=z1;
								yref1=0;
								zref1= -x1;
							}
						}
						else
						{
							xref1= -z1;
							yref1=0;
							zref1=x1;
						}
					}
					else
					{
						if (y2>z2)
						{
							if (x2>z2)
							{
								xref1= -y1;
								yref1=x1;
								zref1=0;
							}
							else
							{
								xref1=0;
								yref1=z1;
								zref1= -y1;
							}
						}
						else
						{
							xref1=0;
							yref1= -z1;
							zref1=y1;
						}
					}
					length=sqrt(xref1*xref1+yref1*yref1+zref1*zref1);
					xref1 /= length;
					yref1 /= length;
					zref1 /= length;
					xref2=y1*zref1-z1*yref1;
					yref2=z1*xref1-x1*zref1;
					zref2=x1*yref1-y1*xref1;
					while (return_code&&(j<number_of_triangles))
					{
#if defined (DEBUG)
						/*???debug */
						printf("triangle %d.  %g %g\n",j,
							sphere_calculate_distance(uv,triangles[j]->centre),
							triangles[j]->radius2);
#endif /* defined (DEBUG) */
						if (sphere_calculate_distance(uv,triangles[j]->centre)<=
							triangles[j]->radius2)
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
									x2=cos((triangles[j]->vertices)[2*l+1])*
										cos((triangles[j]->vertices)[2*l]);
									y2=cos((triangles[j]->vertices)[2*l+1])*
										sin((triangles[j]->vertices)[2*l]);
									z2=sin((triangles[j]->vertices)[2*l+1]);
									/* cross-product */
									x3=y1*z2-z1*y2;
									y3=z1*x2-x1*z2;
									z3=x1*y2-y1*x2;
									angle=atan2(x3*xref1+y3*yref1+z3*zref1,
										x3*xref2+y3*yref2+z3*zref2);
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
#if defined (DEBUG)
										/*???debug */
										printf("adjacent vertices\n");
										for (n=0;n<number_of_adjacent_vertices;n++)
										{
											printf("  %d.  %g %g  %g\n",adjacent_vertices[n],
												adjacent_uvs[2*n],adjacent_uvs[2*n+1],
												adjacent_angles[n]);
										}
#endif /* defined (DEBUG) */
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
								display_message(ERROR_MESSAGE,"sphere_delauney.  "
									"Could not reallocate adjacent vertices");
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
									k++;
									if (sphere_calculate_circumcentre(triangle))
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
										"sphere_delauney.  Could not allocate triangle");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"sphere_delauney.  Could not reallocate triangles");
						}
					}
					DEALLOCATE(adjacent_vertices);
					DEALLOCATE(adjacent_angles);
					DEALLOCATE(adjacent_uvs);
					i++;
					vertex += 3;
				}
#if defined (DEBUG)
				/*???debug */
				printf("number_of_triangles=%d (end)\n",number_of_triangles);
				for (l=0;l<number_of_triangles;l++)
				{
					printf("  triangle %d  %g %g  %g\n",l,triangles[l]->centre[0],
						triangles[l]->centre[1],triangles[l]->radius2);
					m=triangles[l]->vertex_numbers[0];
					printf("    %d.  %g %g  %g %g %g\n",m,
						(triangles[l]->vertices)[0],(triangles[l]->vertices)[1],
						vertices[3*m],vertices[3*m+1],vertices[3*m+2]);
					m=triangles[l]->vertex_numbers[1];
					printf("    %d.  %g %g  %g %g %g\n",m,
						(triangles[l]->vertices)[2],(triangles[l]->vertices)[3],
						vertices[3*m],vertices[3*m+1],vertices[3*m+2]);
					m=triangles[l]->vertex_numbers[2];
					printf("    %d.  %g %g  %g %g %g\n",m,
						(triangles[l]->vertices)[4],(triangles[l]->vertices)[5],
						vertices[3*m],vertices[3*m+1],vertices[3*m+2]);
				}
#endif /* defined (DEBUG) */
				if (return_code)
				{
					/* return results */
					number_of_returned_triangles=number_of_triangles;
					if (ALLOCATE(returned_triangles,int,3*number_of_returned_triangles))
					{
						*triangles_address=returned_triangles;
						*number_of_triangles_address=number_of_returned_triangles;
						for (i=0;i<number_of_triangles;i++)
						{
							for (j=0;j<3;j++)
							{
								*returned_triangles=((triangles[i])->vertex_numbers)[j];
								returned_triangles++;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"sphere_delauney.  Could not allocate returned_triangles");
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
					"sphere_delauney.  Could not allocate initial triangles 2");
				i++;
				while (i<number_of_triangles)
				{
					DEALLOCATE(triangles[i]);
					i++;
				}
				DEALLOCATE(triangles);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"sphere_delauney.  Could not allocate initial triangles");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"sphere_delauney.  Invalid argument(s) %d %p %p %p",number_of_vertices,
			vertices,number_of_triangles_address,triangles_address);
	}
	LEAVE;

	return (return_code);
} /* sphere_delauney */
