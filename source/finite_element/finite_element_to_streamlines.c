/*******************************************************************************
FILE : finite_element_to_streamlines.c

LAST MODIFIED : 22 June 2000

DESCRIPTION :
Functions for calculating streamlines in finite elements.
???DB.  Put into finite_element_to_graphics_object or split
	finite_element_to_graphics_object further ?
==============================================================================*/
#include <math.h>
#include "finite_element/computed_field.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/matrix_vector.h"
#include "general/object.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_window.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
struct Interactive_streamline
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Interactive_streamline type is private.
==============================================================================*/
{
	/* line/ribbon etc. */
	enum Streamline_type type;
	char *name;
	/* The element in which the streamline starts */
	struct FE_element *element;
	/* The coordinate field which the streamline exists in */
	struct Computed_field *coordinate_field;
	/* The vector field which the streamline represents */
	struct Computed_field *stream_vector_field;
	/* data field */
	enum Streamline_data_type data_type;
	struct Computed_field *data_field;
	/* The streamlines length */
	float length;
	/* The stream ribbon width */
	float width;
	/* The starting position of the streamline in the element */
	FE_value xi[3];
	int reverse_track;
	/* The pointset which represents the streamline start in the window */
	struct GT_object *graphics_object;
	/* The polyline which is the graphical streamline object */
	struct GT_object *streamline_graphics_object;
	int selected;
	FE_value previous_point[3];
	FE_value previous_zdepth;
	/* the number of structures that point to this spectrum.  The spectrum
		cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct Interactive_streamline */

FULL_DECLARE_LIST_TYPE(Interactive_streamline);

FULL_DECLARE_MANAGER_TYPE(Interactive_streamline);

struct Streampoint
{
	/* The element which the streampoint is in */
	struct FE_element *element;
	/* The position in the element */
	FE_value xi[3];
	/* The index to the point in the pointlist */
	int index;
	/* The location where the pointer to the pointlist is stored,
		this is done with a second reference so that the pointlist
		can be reallocated and shifted around but only needs
		updating in the Pointset object itself, this points to the
		same location */
	Triple **pointlist;
	/* Points to the place in the pointset object which contains this
	streampoints position*/
/*	Triple *position;*/
	/* The Graphics Object to which this streampoint belongs, therefore whenever
		the streampoints position is changed the graphics object's status should be reset */
	gtObject *graphics_object;

	struct Streampoint *next;
}; /* struct Streampoint */

/*
Module functions
----------------
*/
DECLARE_LOCAL_MANAGER_FUNCTIONS(Interactive_streamline)

static int parent_not_equal_to_element(struct FE_element_parent *object,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 21 October 1997

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(parent_not_equal_to_element);
	if (object&&user_data )
	{
		if (object->parent!=(struct FE_element *)user_data)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"parent_not_equal_to_element.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* parent_not_equal_to_element */

static int change_element(struct FE_element **element,FE_value *xi,
	int face_index,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
For changing between elements when the streamline reaches the interface. Swaps
to the neighbouring element from <element> across the <face_index>. Need the
coordinate_field for resolving problems when crossing polygon sides.
==============================================================================*/
{
	FE_value *face_to_element,facexi[2],pointA[3],pointB[3];
	int number_of_vertices,return_code;
	struct FE_element *face;
	struct FE_element_parent *parent_struct;

	ENTER(change_element);
	return_code=1;

	/* check arguments */
	if (element&&(*element)&&(*element)->shape)
	{
		face=((*element)->faces)[face_index];
		if (face)
		{
			/* find the other parent */
			parent_struct=FIRST_OBJECT_IN_LIST_THAT(FE_element_parent)(
				parent_not_equal_to_element,(void *)*element,face->parent_list );
			if (parent_struct)
			{
				/* go to the face * dimension ^ 2 position in face to element */
				face_to_element=((*element)->shape->face_to_element)+face_index*9;
				/* first column is translation, subtract from xi */
				xi[0] -= face_to_element[0];
				xi[1] -= face_to_element[3];
				xi[2] -= face_to_element[6];
				/* find the non zero entries in the second part, assuming one to one
					mapping */
				if (face_to_element[1])
				{
					facexi[0]=xi[0]/face_to_element[1];
				}
				else
				{
					if (face_to_element[4])
					{
						facexi[0]=xi[1]/face_to_element[4];
					}
					else
					{
						if (face_to_element[7])
						{
							facexi[0]=xi[2]/face_to_element[7];
						}
						else
						{
							facexi[0]=0.0;
						}
					}
				}
				if (face_to_element[2])
				{
					facexi[1]=xi[0]/face_to_element[2];
				}
				else
				{
					if (face_to_element[5])
					{
						facexi[1]=xi[1]/face_to_element[5];
					}
					else
					{
						if (face_to_element[8])
						{
							facexi[1]=xi[2]/face_to_element[8];
						}
						else
						{
							facexi[1]=0.0;
						}
					}
				}
				/* Fixup for incompatible circumferential xi coordinate in Polygons
					(Part 1) */
				if (POLYGON_SHAPE==(*element)->shape->type[0])
				{
					number_of_vertices=((*element)->shape->type)[1];
					if (0==number_of_vertices)
					{
						number_of_vertices=((*element)->shape->type)[2];
					}
					if (face_index<number_of_vertices)
					{
						/* if polygon, need to check ordering of nodes to ensure that xi
							coordinates will match, so store pointA now and compare at end */
						xi[0]=face_to_element[0]+face_to_element[1]*facexi[0]+
							face_to_element[2]*facexi[1];
						xi[1]=face_to_element[3]+face_to_element[4]*facexi[0]+
							face_to_element[5]*facexi[1];
						xi[2]=face_to_element[6]+face_to_element[7]*facexi[0]+
							face_to_element[8]*facexi[1];
						if (!Computed_field_evaluate_in_element(coordinate_field,
							*element,xi,(struct FE_element *)NULL,pointA,(FE_value*)NULL))
						{
							display_message(ERROR_MESSAGE,
								"change_element.  Error calculating coordinate field");
							return_code = 0;
						}
					}
				}
				else
				{
					if (POLYGON_SHAPE==(*element)->shape->type[3])
					{
						number_of_vertices=((*element)->shape->type)[4];
						if (face_index<number_of_vertices)
						{
							/* if polygon, need to check ordering of nodes to
								ensure that xi coordinates will match, so store pointA now
								and compare at end */
							xi[0]=face_to_element[0]+face_to_element[1]*facexi[0]+
								face_to_element[2]*facexi[1];
							xi[1]=face_to_element[3]+face_to_element[4]*facexi[0]+
								face_to_element[5]*facexi[1];
							xi[2]=face_to_element[6]+face_to_element[7]*facexi[0]+
								face_to_element[8]*facexi[1];
							if (!Computed_field_evaluate_in_element(coordinate_field,
								*element,xi,(struct FE_element *)NULL,pointA,(FE_value*)NULL))
							{
								display_message(ERROR_MESSAGE,
									"change_element.  Error calculating coordinate field");
								return_code = 0;
							}
						}
					}
				}
				/* End Fixup for incompatible circumferential xi coorodinate in Polygons
					(Part 1) */
				if ( return_code )
				{
					*element=parent_struct->parent;
					face_to_element=((*element)->shape->face_to_element)+
						(parent_struct->face_number)*9;
					xi[0]=face_to_element[0]+face_to_element[1]*facexi[0]+
						face_to_element[2]*facexi[1];
					xi[1]=face_to_element[3]+face_to_element[4]*facexi[0]+
						face_to_element[5]*facexi[1];
					xi[2]=face_to_element[6]+face_to_element[7]*facexi[0]+
						face_to_element[8]*facexi[1];
					/* Fixup for incompatible circumferential xi coorodinate in
						 Polygons (Part 2) */
					if (POLYGON_SHAPE==(*element)->shape->type[0])
					{
						number_of_vertices=((*element)->shape->type)[1];
						if (0==number_of_vertices)
						{
							number_of_vertices=((*element)->shape->type)[2];
						}
						if (face_index<number_of_vertices)
						{
							/* if polygon, compare pointA and pointB */
							if (!Computed_field_evaluate_in_element(coordinate_field,
								*element,xi,(struct FE_element *)NULL,pointB,(FE_value*)NULL))
							{
								display_message(ERROR_MESSAGE,
									"change_element.  Error calculating coordinate field");
								return_code=0;
							}
							if ((fabs(pointA[0]-pointB[0])>1e-5)||
								(fabs(pointA[1]-pointB[1])>1e-5)||
								(fabs(pointA[2]-pointB[2])>1e-5))
							{
								/* Assume that xi are valid other way round */
								facexi[0]=1.0-facexi[0];
								xi[0]=face_to_element[0]+face_to_element[1]*facexi[0]+
									face_to_element[2]*facexi[1];
								xi[1]=face_to_element[3]+face_to_element[4]*facexi[0]+
									face_to_element[5]*facexi[1];
								xi[2]=face_to_element[6]+face_to_element[7]*facexi[0]+
									face_to_element[8]*facexi[1];
								return_code=1;
							}
						}
					}
					else
					{
						if (POLYGON_SHAPE==(*element)->shape->type[3])
						{
							number_of_vertices=((*element)->shape->type)[4];
							if (face_index<number_of_vertices)
							{
								/* if polygon, need to check ordering of nodes to
									 ensure that xi coordinates will match, so store pointA
									 now and compare at end */
								if (!Computed_field_evaluate_in_element(coordinate_field,
									*element,xi,(struct FE_element *)NULL,pointB,(FE_value*)NULL))
								{
									display_message(ERROR_MESSAGE,
										"change_element.  Error calculating coordinate field");
									return_code = 0;
								}
								if ((fabs(pointA[0]-pointB[0])>1e-5)||
									(fabs(pointA[1]-pointB[1])>1e-5)||
									(fabs(pointA[2]-pointB[2])>1e-5))
								{
									/* Assume that xi are valid other way round */
									facexi[0]=1.0-facexi[0];
									xi[0]=face_to_element[0]+face_to_element[1]*facexi[0]+
										face_to_element[2]*facexi[1];
									xi[1]=face_to_element[3]+face_to_element[4]*facexi[0]+
										face_to_element[5]*facexi[1];
									xi[2]=face_to_element[6]+face_to_element[7]*facexi[0]+
										face_to_element[8]*facexi[1];
									return_code = 1;
								}
							}
						}
					}
					/* End Fixup for incompatible circumferential xi coorodinate in
						 Polygons (Part 2) */
				}
			}
			else
			{
				/* reached edge of mesh, streamline stops */
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"change_element.  Face not found element %d for face index %d",
				(*element)->cm.number, face_index);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"change_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* change_element */

static int check_xi_limits(int change_xi, struct FE_element **element,
	FE_value *xi,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Check the xi limits when the element reaches the interface. If on an element
boundary the element is changed and therefore this function should only be
called if the integration has indicated that the direction changes element.
==============================================================================*/
{
	int face_index,return_code;

	ENTER(check_xi_limits);
	return_code=1;
	if ((1==change_xi)&&(xi[0]<=0.0))
	{
		if (POLYGON_SHAPE==((*element)->shape->type)[0])
		{
			/* Polygon */
			xi[0]=xi[0]+1;
		}
		else
		{
			/* Line */
			/*???SAB.  Could be improved to check shape->faces for the face */
			if (POLYGON_SHAPE==((*element)->shape->type)[3])
			{
				face_index=0;
			}
			else
			{
				face_index=0;
			}
			return_code=change_element(element,xi,face_index,coordinate_field);
		}
	}
	else
	{
		if ((1==change_xi)&&(xi[0]>=1.0))
		{
			if (POLYGON_SHAPE==((*element)->shape->type)[0])
			{
				/* Polygon */
				xi[0]=xi[0]-1;
			}
			else
			{
				/* Line */
				/* SAB Could be improved to check shape->faces for the face */
				if (POLYGON_SHAPE==((*element)->shape->type)[3])
				{
					face_index=1;
				}
				else
				{
					face_index=1;
				}
				return_code=change_element(element,xi,face_index,coordinate_field);
			}
		}
	}
	if ((2==change_xi)&&(xi[1]<=0))
	{
		if (POLYGON_SHAPE==((*element)->shape->type)[3])
		{
			/* Polygon */
			if (((*element)->shape->type)[1])
			{
				/* radial xi */
				/* Crude */
				xi[1]= -xi[1];
				if (xi[0]<0.5)
				{
					xi[0] += 0.5;
				}
				else
				{
					xi[0] -= 0.5;
				}
			}
			else
			{
				/* circumferential xi */
				/* Not done yet */
				return_code=0;
			}
		}
		else
		{
			/* Line */
			/* SAB Could be improved to check shape->faces for the face */
			if (POLYGON_SHAPE==((*element)->shape->type)[0])
			{
				face_index=((*element)->shape->type)[1];
				if (0==face_index)
				{
					face_index=((*element)->shape->type)[2];
				}
			}
			else
			{
				face_index=2;
			}
			return_code=change_element(element,xi,face_index,coordinate_field);
		}
	}
	else
	{
		if ((2==change_xi)&&(xi[1]>=1.0))
		{
			if (POLYGON_SHAPE==((*element)->shape->type)[3])
			{
				/* Polygon */
				if (((*element)->shape->type)[1])
				{
					/* radial xi */
					/* Assumes faces are stored in order of increasing xi0 */
					face_index=(int)floor(xi[0]*(FE_value)((*element)->shape->type)[1]);
					return_code=change_element(element,xi,face_index,coordinate_field);
				}
				else
				{
					/* circumferential xi */
					/* Not done yet */
					return_code = 0;
				}
			}
			else
			{
				/* Line */
				/* SAB Could be improved to check shape->faces for the face */
				if (POLYGON_SHAPE==((*element)->shape->type)[0])
				{
					face_index=((*element)->shape->type)[1]+1;
					if (0==face_index)
					{
						face_index=((*element)->shape->type)[2]+1;
					}
				}
				else
				{
					face_index=3;
				}
				return_code=change_element(element,xi,face_index,coordinate_field);
			}
		}
	}
	if ((3==change_xi)&&(xi[2]<=0))
	{
		if (POLYGON_SHAPE==((*element)->shape->type)[5])
		{
			/* Polygon */
			/* Not done yet */
			return_code=0;
		}
		else
		{
			/* Line */
			/* SAB Could be improved to check shape->faces for the
				face, now it assumes the order of the faces, all other faces first,
				then xi3 = 0 and then xi3 = 1 */
			if (POLYGON_SHAPE==((*element)->shape->type)[0])
			{
				face_index=((*element)->shape->type)[1];
				if (0==face_index)
				{
					face_index=((*element)->shape->type)[2];
				}
			}
			else
			{
				face_index=4;
			}
			return_code=change_element(element,xi,face_index,coordinate_field);
		}
	}
	else
	{
		if ((3==change_xi)&&(xi[2]>=1.0))
		{
			if (POLYGON_SHAPE==((*element)->shape->type)[5])
			{
				/* Polygon */
				/* Not done yet */
				return_code=0;
			}
			else
			{
				/* Line */
				/* SAB Could be improved to check shape->faces for the
					face, now it assumes the order of the faces, all other faces first,
					then xi3 = 0 and then xi3 = 1 */
				if (POLYGON_SHAPE==((*element)->shape->type)[0])
				{
					face_index=((*element)->shape->type)[1]+1;
					if (0==face_index)
					{
						face_index=((*element)->shape->type)[2]+1;
					}
				}
				else
				{
					face_index=5;
				}
				return_code=change_element(element,xi,face_index,coordinate_field);
			}
		}
	}
	LEAVE;

	return (return_code);
} /* check_xi_limits */

static int calculate_delta_xi(FE_value *vector,FE_value *dx_dxi,
	FE_value *deltaxi)
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Converts a vector <vector> in world space into xi space <deltaxi> by
calculating the inverse of the Jacobian matrix <dxdxi> and multiplying.
==============================================================================*/
{
	FE_value dxi_dx[9];
	int return_code;

	ENTER(calculate_delta_xi);
	if (vector&&deltaxi&&invert_FE_value_matrix3(dx_dxi,dxi_dx))
	{
		deltaxi[0]=vector[0]*dxi_dx[0]+vector[1]*dxi_dx[1]+vector[2]*dxi_dx[2];
		deltaxi[1]=vector[0]*dxi_dx[3]+vector[1]*dxi_dx[4]+vector[2]*dxi_dx[5];
		deltaxi[2]=vector[0]*dxi_dx[6]+vector[1]*dxi_dx[7]+vector[2]*dxi_dx[8];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_delta_xi.  Failed");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* calculate_delta_xi */

static FE_value find_valid_portion( FE_value *xiA, FE_value *xiB, int *change_element )
{
	FE_value new_proportion, proportion;

	*change_element = 0;

	proportion = 1.0;
	if ( xiB[0] < 0.0 )
	{
		proportion = ( 0.0 - xiA[0] ) / ( xiB[0] - xiA[0] );
		*change_element = 1;
	}
	if ( xiB[0] > 1.0 )
	{
		proportion = ( 1.0 - xiA[0] ) / ( xiB[0] - xiA[0] );
		*change_element = 1;
	}

	if ( xiB[1] < 0.0 )
	{
		new_proportion = ( 0.0 - xiA[1] ) / ( xiB[1] - xiA[1] );
		if ( new_proportion < proportion )
		{
			proportion = new_proportion;
			*change_element = 2;
		}
	}
	if ( xiB[1] > 1.0 )
	{
		new_proportion = ( 1.0 - xiA[1] ) / ( xiB[1] - xiA[1] );
		if ( new_proportion < proportion )
		{
			proportion = new_proportion;
			*change_element = 2;
		}
	}

	if ( xiB[2] < 0.0 )
	{
		new_proportion = ( 0.0 - xiA[2] ) / ( xiB[2] - xiA[2] );
		if ( new_proportion < proportion )
		{
			proportion = new_proportion;
			*change_element = 3;
		}
	}
	if ( xiB[2] > 1.0 )
	{
		new_proportion = ( 1.0 - xiA[2] ) / ( xiB[2] - xiA[2] );
		if ( new_proportion < proportion )
		{
			proportion = new_proportion;
			*change_element = 3;
		}
	}

	return( proportion );
} /* find_valid_portion */

static int update_adaptive_imp_euler(struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	struct FE_element *element,FE_value *xi,FE_value *point,FE_value *step_size,
	FE_value *total_stepped,int *change_element)
/*******************************************************************************
LAST MODIFIED : 30 June 1999

DESCRIPTION :
Update the xi coordinates using the <stream_vector_field> with adaptive step
size control and the improved euler method.  <change_element> indicates whether
an element boundary has been reached and therefore the element should be
changed. The function updates the <total_stepped>.
If <reverse_track> is true, the reverse of vector field is tracked.
==============================================================================*/
{
	int return_code;
	FE_value deltaxi[3],deltaxiA[3],
		deltaxiC[3], deltaxiD[3], deltaxiE[3], dxdxi[9], error,
		proportion, tolerance, vector[9], xiA[3], xiB[3], xiC[3],
		xiD[3], xiE[3], xiF[3];

	ENTER(update_adaptive_imp_euler);
	/* clear coordinates in case fewer than 3 components */
	point[0]=0.0;
	point[1]=0.0;
	point[2]=0.0;
	tolerance=1.0e-4;
	return_code=Computed_field_evaluate_in_element(coordinate_field,element,xi,
		(struct FE_element *)NULL,point,dxdxi)&&Computed_field_evaluate_in_element(stream_vector_field,
			element,xi,(struct FE_element *)NULL,vector,(FE_value *)NULL);
	if (reverse_track)
	{
		vector[0] = -vector[0];
		vector[1] = -vector[1];
		vector[2] = -vector[2];
	}
	if (return_code)
	{
		return_code=calculate_delta_xi(vector,dxdxi,deltaxi);
	}
	if (return_code)
	{
		if (*step_size == 0.0)
		{
			/* This is the first step, set the step_size to make the
			 magnitude of deltaxi 0.01 */
			*step_size=1.0e-2/sqrt(deltaxi[0]*deltaxi[0]+deltaxi[1]*deltaxi[1]
				+deltaxi[2]*deltaxi[2]);
			
		}
		/* whole step */
		xiA[0]=xi[0]+(*step_size)*deltaxi[0];
		xiA[1]=xi[1]+(*step_size)*deltaxi[1];
		xiA[2]=xi[2]+(*step_size)*deltaxi[2];
		return_code=Computed_field_evaluate_in_element(coordinate_field,element,xiA,
			(struct FE_element *)NULL,point,dxdxi)&&Computed_field_evaluate_in_element(stream_vector_field,
				element,xiA,(struct FE_element *)NULL,vector,(FE_value *)NULL);
		if (reverse_track)
		{
			vector[0] = -vector[0];
			vector[1] = -vector[1];
			vector[2] = -vector[2];
		}
		if (return_code)
		{
			return_code=calculate_delta_xi(vector,dxdxi,deltaxiA);
		}
		if (return_code)
		{
			xiB[0]=xi[0]+(*step_size)*(deltaxi[0]+deltaxiA[0])/2.0;
			xiB[1]=xi[1]+(*step_size)*(deltaxi[1]+deltaxiA[1])/2.0;
			xiB[2]=xi[2]+(*step_size)*(deltaxi[2]+deltaxiA[2])/2.0;
		}
	}
	error=1.0;
	while ((error>tolerance)&&return_code)
	{
		xiC[0]=xi[0]+(*step_size)*deltaxi[0]/2.0;
		xiC[1]=xi[1]+(*step_size)*deltaxi[1]/2.0;
		xiC[2]=xi[2]+(*step_size)*deltaxi[2]/2.0;
		return_code=Computed_field_evaluate_in_element(coordinate_field,element,xiC,
			(struct FE_element *)NULL,point,dxdxi)&&Computed_field_evaluate_in_element(stream_vector_field,
				element,xiC,(struct FE_element *)NULL,vector,(FE_value *)NULL);
		if (reverse_track)
		{
			vector[0] = -vector[0];
			vector[1] = -vector[1];
			vector[2] = -vector[2];
		}
		if (return_code)
		{
			return_code=calculate_delta_xi(vector,dxdxi,deltaxiC);
		}
		if (return_code)
		{
			xiD[0]=xi[0]+(*step_size)*(deltaxi[0]+deltaxiC[0])/4.0;
			xiD[1]=xi[1]+(*step_size)*(deltaxi[1]+deltaxiC[1])/4.0;
			xiD[2]=xi[2]+(*step_size)*(deltaxi[2]+deltaxiC[2])/4.0;
			return_code=Computed_field_evaluate_in_element(coordinate_field,element,
				xiD,(struct FE_element *)NULL,point,dxdxi)&&Computed_field_evaluate_in_element(
					stream_vector_field,element,xiD,(struct FE_element *)NULL,vector,(FE_value *)NULL);
			if (reverse_track)
			{
				vector[0] = -vector[0];
				vector[1] = -vector[1];
				vector[2] = -vector[2];
			}
		}
		if (return_code)
		{
			return_code=calculate_delta_xi(vector,dxdxi,deltaxiD);
		}
		if (return_code)
		{
			xiE[0]=xiD[0]+(*step_size)*deltaxiD[0]/2.0;
			xiE[1]=xiD[1]+(*step_size)*deltaxiD[1]/2.0;
			xiE[2]=xiD[2]+(*step_size)*deltaxiD[2]/2.0;
			return_code=Computed_field_evaluate_in_element(coordinate_field,element,
				xiE,(struct FE_element *)NULL,point,dxdxi)&&Computed_field_evaluate_in_element(
					stream_vector_field,element,xiE,(struct FE_element *)NULL,vector,(FE_value *)NULL);
			if (reverse_track)
			{
				vector[0] = -vector[0];
				vector[1] = -vector[1];
				vector[2] = -vector[2];
			}
		}
		if (return_code)
		{
			return_code=calculate_delta_xi(vector,dxdxi,deltaxiE);
		}
		if (return_code)
		{
			xiF[0]=xiD[0]+(*step_size)*(deltaxiD[0]+deltaxiE[0])/4.0;
			xiF[1]=xiD[1]+(*step_size)*(deltaxiD[1]+deltaxiE[1])/4.0;
			xiF[2]=xiD[2]+(*step_size)*(deltaxiD[2]+deltaxiE[2])/4.0;
			error=sqrt((xiF[0]-xiB[0])*(xiF[0]-xiB[0])+
				(xiF[1]-xiB[1])*(xiF[1]-xiB[1])+(xiF[2]-xiB[2])*(xiF[2]-xiB[2]));
			if (((*step_size)*sqrt(deltaxiC[0]*deltaxiC[0]+
				deltaxiC[1]*deltaxiC[1]+deltaxiC[2]*deltaxiC[2]))<1.0e-3)
			{
				error=0.0;
			}
		}
		if ((error>tolerance)&&return_code)
		{
			*step_size /= 2.0;
			xiB[0]=xiD[0];
			xiB[1]=xiD[1];
			xiB[2]=xiD[2];
		}
	}
	if (return_code)
	{
		/* is the whole integration within the element ? */
		if ((xiF[0]<0.0)||(xiF[0]>1.0)||(xiF[1]<0.0)||(xiF[1]>1.0)||(xiF[2]<0.0)||
			(xiF[2]>1.0))
		{
			/* was the first part of the double step all inside the element... */
			if ((xiD[0]<0.0)||(xiD[0]>1.0)||(xiD[1]<0.0)||(xiD[1]>1.0)||
				(xiD[2]<0.0)||(xiD[2]>1.0))
			{
				/* else interpolate the first part */
				proportion=find_valid_portion(xi,xiD,change_element);
				*total_stepped += (*step_size)*0.5*proportion;
				xi[0]=xi[0]+proportion*(*step_size)*(deltaxi[0]+deltaxiC[0])/4.0;
				xi[1]=xi[1]+proportion*(*step_size)*(deltaxi[1]+deltaxiC[1])/4.0;
				xi[2]=xi[2]+proportion*(*step_size)*(deltaxi[2]+deltaxiC[2])/4.0;
			}
			else
			{
				/* then interpolate the second part */
				proportion=find_valid_portion(xiD,xiF,change_element);
				*total_stepped += (*step_size)*0.5*(1.0+proportion);
				xi[0]=xiD[0]+proportion*(*step_size)*(deltaxiD[0]+deltaxiE[0])/4.0;
				xi[1]=xiD[1]+proportion*(*step_size)*(deltaxiD[1]+deltaxiE[1])/4.0;
				xi[2]=xiD[2]+proportion*(*step_size)*(deltaxiD[2]+deltaxiE[2])/4.0;
			}
			/*???RC the proportion returned by find_valid_portion, when plugged into
				the above formulae can leave xi slightly inside the element, even when
				change element is indicated. Hence make sure calculation leaves xi
				exactly on the boundary. */
			switch (*change_element)
			{
				case 1:
				{
					/* xi[0] outside 0 to 1 range */
					if (xi[0]>0.5)
					{
						xi[0]=1.0;
					}
					else
					{
						xi[0]=0.0;
					}
				} break;
				case 2:
				{
					/* xi[1] outside 0 to 1 range */
					if (xi[1]>0.5)
					{
						xi[1]=1.0;
					}
					else
					{
						xi[1]=0.0;
					}
				} break;
				case 3:
				{
					/* xi[2] outside 0 to 1 range */
					if (xi[2]>0.5)
					{
						xi[2]=1.0;
					}
					else
					{
						xi[2]=0.0;
					}
				} break;
			}
		}
		else
		{
			*change_element=0;
			*total_stepped += *step_size;
			if (error<tolerance/10.0)
			{
				*step_size *= 2.0;
			}
			xi[0]=xiF[0];
			xi[1]=xiF[1];
			xi[2]=xiF[2];
		}
	}
	LEAVE;

	return (return_code);
} /* update_adaptive_imp_euler */

static int update_interactive_streampoint(FE_value *point_coordinates,
	struct FE_element **element,struct Computed_field *coordinate_field,
	FE_value *xi, FE_value *translate)
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Updates the <GT_pointset> streampoint using the xi coordinates.  If the given
translation in world coordinates is non NULL the xi coordinates are updated
(this is transformed by the local dxdxi derivatives to xi values so only
accurate if small), also ensuring that the element is updated.
==============================================================================*/
{
	FE_value deltaxi[3],dxdxi[9],magnitude,point[3],proportion,xiA[3];
	int change_element,return_code;

	ENTER(update_interactive_streampoint);
	/* check the arguments */
	if ((*element)&&((*element)->shape)&&(3==(*element)->shape->dimension)&&
		coordinate_field&&
		(3>=Computed_field_get_number_of_components(coordinate_field)))
	{
		point[0]=0.0;
		point[1]=0.0;
		point[2]=0.0;
		if ( translate )
		{
			if (Computed_field_evaluate_in_element(coordinate_field,*element,xi,
				(struct FE_element *)NULL,point,dxdxi))
			{
				if (!(calculate_delta_xi(translate,dxdxi,deltaxi)))
				{
					/* dxdxi tensor is singular, peturb xi by random small amount */
					deltaxi[0]=0.002*rand()-0.001;
					deltaxi[1]=0.002*rand()-0.001;
					deltaxi[2]=0.002*rand()-0.001;
				}
				return_code=0;
				while (!return_code)
				{
#if defined (DEBUG)
					/*???debug */
					printf("update interactive streampoint.  deltaxi %f %f %f\n",
						deltaxi[0], deltaxi[1], deltaxi[2] );
					printf("update interactive streampoint.  xi %f %f %f\n",
						xi[0], xi[1], xi[2] );
					/*???debug end */
#endif /* defined (DEBUG) */
					xiA[0]=xi[0]+deltaxi[0];
					xiA[1]=xi[1]+deltaxi[1];
					xiA[2]=xi[2]+deltaxi[2];
					if ((xiA[0]<0.0)||(xiA[0]>1.0)||(xiA[1]<0.0)||(xiA[1]>1.0)||
						(xiA[2]<0.0)||(xiA[2]>1.0))
					{
						proportion=find_valid_portion(xi,xiA,&change_element);
						xi[0] += proportion*deltaxi[0];
						xi[1] += proportion*deltaxi[1];
						xi[2] += proportion*deltaxi[2];
					}
					else
					{
						change_element=0;
						xi[0]=xiA[0];
						xi[1]=xiA[1];
						xi[2]=xiA[2];
						return_code=1;
					}
					if (change_element)
					{
						if (check_xi_limits(change_element,element,xi,coordinate_field))
						{
							return_code=1;
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"update_interactive_streampoint.  Reducing step magnitude");
							magnitude=sqrt(deltaxi[0]*deltaxi[0]+
								deltaxi[1]*deltaxi[1]+deltaxi[2]*deltaxi[2]);
							if (magnitude>1.0)
							{
								deltaxi[0] /= 10.*magnitude;
								deltaxi[1] /= 10.*magnitude;
								deltaxi[2] /= 10.*magnitude;
							}
							else
							{
								deltaxi[0] /= 10.;
								deltaxi[1] /= 10.;
								deltaxi[2] /= 10.;
							}
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"update_interactive_streampoint.  "
					"Error calculating coordinate field with derivatives");
				return_code = 0;
			}
		}
		else
		{
			/* No translation required */
			return_code = 1;
		}
		if ( return_code )
		{
			if (!Computed_field_evaluate_in_element(coordinate_field,*element,xi,
				(struct FE_element *)NULL,point_coordinates,(FE_value *)NULL))
			{
				display_message(ERROR_MESSAGE,
			"update_interactive_streampoint.  Error calculating coordinate field");
				return_code = 0;
			}
		}
		Computed_field_clear_cache(coordinate_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_interactive_streampoint.  Invalid argument(s)");
		display_message(ERROR_MESSAGE,"  element %p",*element);
		if (*element)
		{
			display_message(ERROR_MESSAGE,"  element->shape  %p",(*element)->shape);
		}
		return_code=0;
	}
	LEAVE;

	return(return_code);
} /* update_interactive_streampoint */

struct Streamline_event
{
	struct MANAGER(Interactive_streamline) *streamline_manager;
	Widget caller;
	XmAnyCallbackStruct *caller_data;
};

#if defined(NEW_CODE)
static int handle_interactive_streamline_event(
	struct Interactive_streamline *streamline,void *streamline_event_void)
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Iterator function for updating interactive streamlines with mouse events
==============================================================================*/
{
	Dimension height,width;
	double cursor_x,cursor_y;
	int event_type, return_code;
	FE_value *point, translate[3];
	float distance, select_radius = 0.01;
	GLdouble model_matrix[16], new_point[3], projection_matrix[16],
		window_x,window_y,window_z;
	GLint default_viewport[4]={0,0,1,1};
	struct GT_polyline *polyline;
	struct GT_surface *surface;
#if defined (OLD_GFX_WINDOW)
	struct MANAGER_MESSAGE(Interactive_streamline) message;
#endif
	struct Streamline_event *streamline_event;
	XButtonEvent *button_event;
	XEvent *event;

	ENTER(handle_interactive_streamline_event);
	if (streamline&&streamline_event_void)
	{
		streamline_event=(struct Streamline_event *)streamline_event_void;
		if (event=streamline_event->caller_data->event)
		{
			XtVaGetValues(streamline_event->caller,XtNheight,&height,XtNwidth,&width,
				NULL);
			event_type=event->type;
			switch (event_type)
			{
				case ButtonPress:
				{
					button_event= &(event->xbutton);
					cursor_x=(double)(button_event->x)/(double)width;
					cursor_y=(double)(height-(button_event->y))/(double)height;
					point=(FE_value *)
						(*(streamline->graphics_object->gu.gt_pointset))->pointlist;
					glGetDoublev(GL_MODELVIEW_MATRIX,model_matrix);
					glGetDoublev(GL_PROJECTION_MATRIX,projection_matrix);
					if (GL_TRUE==gluProject(point[0],point[1],point[2],
						model_matrix,projection_matrix,default_viewport,
						&window_x,&window_y,&window_z))
					{
						distance = fabs (window_x - cursor_x);
						if ( fabs ( window_y - cursor_y ) > distance )
						{
							distance = fabs ( window_y - cursor_y );
						}

						if ( distance < select_radius )
						{
							streamline->selected = 1;
							streamline->previous_zdepth = window_z;
							streamline->previous_point[0] = point[0];
							streamline->previous_point[1] = point[1];
							streamline->previous_point[2] = point[2];
						}
					}
				} break;
				case MotionNotify:
				case ButtonRelease:
				{
					button_event= &(event->xbutton);
					cursor_x=(double)(button_event->x)/(double)width;
					cursor_y=(double)(height-(button_event->y))/(double)height;

					if ( streamline->selected )
					{
						if ( ButtonRelease == event_type )
						{
							streamline->selected = 0;
						}

						glGetDoublev(GL_MODELVIEW_MATRIX,model_matrix);
						glGetDoublev(GL_PROJECTION_MATRIX,projection_matrix);

						if (GL_TRUE==gluUnProject(cursor_x,cursor_y,
							streamline->previous_zdepth,model_matrix,projection_matrix,
							default_viewport,&(new_point[0]),&(new_point[1]),&(new_point[2])))
						{
							translate[0] = new_point[0] - (streamline->previous_point)[0];
							translate[1] = new_point[1] - (streamline->previous_point)[1];
							translate[2] = new_point[2] - (streamline->previous_point)[2];
/*???debug */
printf("Move in world coordinates %f %f %f\n",translate[0],translate[1],
	translate[2]);
							if (update_interactive_streampoint((FE_value *)
								(*(streamline->graphics_object->gu.gt_pointset))->pointlist,
								&(streamline->element), streamline->coordinate_field,
								streamline->xi, translate ) )
							{
								/* rebuild the streamline */
								if (STREAM_LINE==streamline->type)
								{
									if (polyline=create_GT_polyline_streamline_FE_element(
										streamline->element,
										streamline->xi,streamline->coordinate_field,
										streamline->stream_vector_field,streamline->reverse_track,
										streamline->length,streamline->data_type,
										streamline->data_field))
									{
										if (!(return_code=GT_OBJECT_ADD(GT_polyline)(
											streamline->streamline_graphics_object,
											/*time*/0.0,polyline)))
										{
											DESTROY(GT_polyline)(&polyline);
										}
									}
									else
									{
										return_code=0;
									}
								}
								else if ((streamline->type == STREAM_RIBBON)||
									(streamline->type == STREAM_EXTRUDED_RECTANGLE)||
									(streamline->type == STREAM_EXTRUDED_ELLIPSE))
								{
									if (surface=create_GT_surface_streamribbon_FE_element(
										streamline->element,
										streamline->xi,streamline->coordinate_field,
										streamline->stream_vector_field,streamline->reverse_track,
										streamline->length,streamline->width,streamline->type,
										streamline->data_type,streamline->data_field))
									{
										if (!(return_code=GT_OBJECT_ADD(GT_surface)(
											streamline->streamline_graphics_object,
											/*time*/0.0,surface)))
										{
											DESTROY(GT_surface)(&surface);
										}
									}
									else
									{
										return_code=0;
									}
								}
#if defined (OLD_GFX_WINDOW)
								message.change=
									MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Interactive_streamline);
								message.object_changed = streamline;
#endif /* defined (OLD_GFX_WINDOW) */
/*???debug */
printf("handle_interactive_streamline_event.  Temporarily disabled\n");
display_message(WARNING_MESSAGE,
	"handle_interactive_streamline_event.  Temporarily disabled");
#if defined (OLD_GFX_WINDOW)
								MANAGER_UPDATE(Interactive_streamline)(&message,
									streamline_event->streamline_manager);
#endif /* defined (OLD_GFX_WINDOW) */
							}
							if ( MotionNotify == event_type )
							{
								(streamline->previous_point)[0] = new_point[0];
								(streamline->previous_point)[1] = new_point[1];
								(streamline->previous_point)[2] = new_point[2];
							}
						}
					}
				} break;
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"handle_interactive_streamline_event.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return(return_code);
} /* handle_interactive_streamline_event */
#endif /* #if defined(NEW_CODE) */
static int track_streamline_from_FE_element(struct FE_element **element,
	FE_value *xi,struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	float length,enum Streamline_data_type data_type,
	struct Computed_field *data_field,int *number_of_points,
	Triple **stream_points,Triple **stream_vectors,Triple **stream_normals,
	GTDATA **stream_data)
/*******************************************************************************
LAST MODIFIED : 24 March 2000

DESCRIPTION :
Tracks the stream following <stream_vector_field> starting in the <*element> at
the supplied <xi> coordinates. The streamline is returned in the following
dynamically allocated and reallocated arrays:
<*stream_points> = points along the streamline;
<*stream_vectors> = stream vectors at each point on the streamline;
<*stream_normals> = unit normals to the vectors, appropriate to field tracked;
<*stream_data> (if data_type not STREAM_NO_DATA)=scalar values at each point on
  the line. STREAM_FIELD_SCALAR calculates the <field_scalar> along the line.
The <*element> and <xi> values are updated to where the stream is tracked to, so
that tracking can be continued.
On unsuccessful return the above arrays are deallocated.

If <reverse_track> is true, the reverse of <stream_vector_field> is tracked, and
the negative travel_scalar is recorded, if requested.

The <stream_vector_field> may have 3, 6 or 9 components, the first 3 components
of which returns the vector along which the streamline is tracked. Additional
information about lateral direction and normal to a streamribbon are found by
different meands depending on the number of components in this field in the
following way:
3 = 1 3-D vector (lateral direction and normal worked out from curl of field);
6 = 2 3-D vectors (2nd vector is lateral direction. Stream ribbon normal found
    from cross product);
9 = 3 3-D vectors (2nd vector is lateral direction; 3rd vector is stream ribbon
    normal).
==============================================================================*/
{
	FE_value angle,coordinates[3],cos_angle,curl[3],curl_component,data_value,
		displacement[3],dv_dxi[9],dv_dx[9],dx_dxi[9],dxi_dx[9],magnitude,normal[3],
		normal2[3],old_normal[3],previous_curl_component,previous_total_stepped,
		sin_angle,step_size,stream_vector_values[9],temp,total_stepped,vector[3],
		vector_magnitude;
	GTDATA *stream_datum,*tmp_stream_data;
	int allocated_number_of_points,change_element,
		i,keep_tracking,previous_change_element,
		number_of_stream_vector_components,return_code;
	struct FE_element_shape *shape;
	Triple *stream_point,*stream_vector,*stream_normal,*tmp_triples;

	ENTER(track_streamline_from_FE_element);
	/*	step_size of zero indicates first step */
	step_size = 0;
	total_stepped = 0.0;
	if (element&&(*element)&&(shape=(*element)->shape)&&(3==shape->dimension)&&
		xi&&(0.0 <= xi[0])&&(1.0 >= xi[0])&&(0.0 <= xi[1])&&(1.0 >= xi[1])&&
		(0.0 <= xi[2])&&(1.0 >= xi[2])&&coordinate_field&&
		(3>=Computed_field_get_number_of_components(coordinate_field))&&
		stream_vector_field&&((3==(number_of_stream_vector_components=
			Computed_field_get_number_of_components(stream_vector_field)))||
			(6==number_of_stream_vector_components)||
			(9==number_of_stream_vector_components))&&
		(0.0<length)&&((data_type!=STREAM_FIELD_SCALAR) ||
			(data_field&&(1==Computed_field_get_number_of_components(data_field))))&&
		number_of_points&&stream_points&&stream_vectors&&stream_normals&&
		((STREAM_NO_DATA==data_type)|| stream_data))
	{
		return_code=1;
		/* clear coordinates in case coordinate field is not 3 component */
		coordinates[0]=0.0;
		coordinates[1]=0.0;
		coordinates[2]=0.0;
		*stream_points=(Triple *)NULL;
		*stream_vectors=(Triple *)NULL;
		*stream_normals=(Triple *)NULL;
		*stream_data=(GTDATA *)NULL;
		*number_of_points=0;
		allocated_number_of_points=100;
		if (ALLOCATE(*stream_points,Triple,allocated_number_of_points)&&
			ALLOCATE(*stream_vectors,Triple,allocated_number_of_points)&&
			ALLOCATE(*stream_normals,Triple,allocated_number_of_points)&&
			((STREAM_NO_DATA==data_type)||
				ALLOCATE(*stream_data,GTDATA,allocated_number_of_points)))
		{
			stream_point= *stream_points;
			stream_vector= *stream_vectors;
			stream_normal= *stream_normals;
			stream_datum= *stream_data;
			i=0;
			previous_change_element=0;
			keep_tracking=1;
			while (keep_tracking)
			{
				while (keep_tracking&&(i<allocated_number_of_points))
				{
					/* evaluate the coordinate and stream_vector fields */
					if (3==number_of_stream_vector_components)
					{
						/* need derivatives of coordinate_field and stream_vector_field
							 for calculation of curl. Calculate coordinates first, since
							 may be required for evaluating the stream_vector_field. */
						keep_tracking=Computed_field_evaluate_in_element(coordinate_field,
							*element,xi,(struct FE_element *)NULL,coordinates,dx_dxi)&&
							Computed_field_evaluate_in_element(stream_vector_field,
								*element,xi,(struct FE_element *)NULL,stream_vector_values,dv_dxi);
					}
					else
					{
						/* derivatives are not required with 6 or 9 components in the
							 stream_vector_field, since the lateral direction and ribbon
							 normal are taken or calculated directly from it. Want to
							 evaluate stream_vector_field first, since in many cases it will
							 force a calculation of the coordinate field with derivatives,
							 thus saving the coordinates from being recalculated */
						keep_tracking=Computed_field_evaluate_in_element(
							stream_vector_field,*element,xi,(struct FE_element *)NULL,stream_vector_values,
							(FE_value *)NULL)&&Computed_field_evaluate_in_element(
								coordinate_field,*element,xi,(struct FE_element *)NULL,coordinates,(FE_value *)NULL);
					}
					/* extract stream vector from stream_vector_values */
					if (reverse_track)
					{
						vector[0]=-stream_vector_values[0];
						vector[1]=-stream_vector_values[1];
						vector[2]=-stream_vector_values[2];
					}
					else
					{
						vector[0]=stream_vector_values[0];
						vector[1]=stream_vector_values[1];
						vector[2]=stream_vector_values[2];
					}
					if (1e-30>(vector_magnitude=sqrt(
						vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2])))
					{
						/* streamline is not going anywhere */
						keep_tracking=0;
					}
					if (keep_tracking)
					{
						/* get vector, normal and normal2 - latter are unit vectors normal
							 to the streamline - normal is in lateral direction across sheet,
							 normal2 is normal to the ribbon sheet */
						switch (number_of_stream_vector_components)
						{
							case 3:
							{
								/* evaluate the curl, and its component along vector */
								if (invert_FE_value_matrix3(dx_dxi,dxi_dx)&&
									multiply_FE_value_matrix3(dv_dxi,dxi_dx,dv_dx))
								{
									curl[0] = dv_dx[7] - dv_dx[5];
									curl[1] = dv_dx[2] - dv_dx[6];
									curl[2] = dv_dx[3] - dv_dx[1];
									curl_component=(curl[0]*vector[0]+curl[1]*vector[1]+
										curl[2]*vector[2])/vector_magnitude;
								}
								else
								{
									curl_component=0.0;
								}
								if (0.0==total_stepped)
								{
									angle = 0.0;
									previous_total_stepped = 0.0;
									/* get normal2 = a vector not co-linear with stream vector */
									normal2[0]=0.0;
									normal2[1]=0.0;
									normal2[2]=0.0;
									if (fabs(vector[0]) < fabs(vector[1]))
									{
										if (fabs(vector[2]) < fabs(vector[0]))
										{
											normal2[2]=1.0;
										}
										else
										{
											normal2[0]=1.0;
										}
									}
									else
									{
										if (fabs(vector[2]) < fabs(vector[1]))
										{
											normal2[2]=1.0;
										}
										else
										{
											normal2[1]=1.0;
										}
									}
								}
								else
								{
									/* get angle from average curl along segment of streamline */
									angle = 0.5*(previous_curl_component + curl_component)
										* (total_stepped - previous_total_stepped);
									if (reverse_track)
									{
										angle = -angle;
									}
									/* get displacement from old to current coordinates */
									displacement[0]=coordinates[0]-displacement[0];
									displacement[1]=coordinates[1]-displacement[1];
									displacement[2]=coordinates[2]-displacement[2];
									magnitude=displacement[0]*displacement[0]+
										displacement[1]*displacement[1]+
										displacement[2]*displacement[2];
									if (0.0<magnitude)
									{
										/* get normal2 = displacement (x) old_normal */
										normal2[0]=displacement[1]*old_normal[2]-
											displacement[2]*old_normal[1];
										normal2[1]=displacement[2]*old_normal[0]-
											displacement[0]*old_normal[2];
										normal2[2]=displacement[0]*old_normal[1]-
											displacement[1]*old_normal[0];
									}
									if (0.0==(magnitude=normal2[0]*normal2[0]+
										normal2[1]*normal2[1]+normal2[2]*normal2[2]))
									{
										printf("temp: alternative normal2 calculation!\n");
										/* displacement and old_normal are co-linear */
										/* get normal2 = vector (x) old_normal */
										normal2[0]=vector[1]*old_normal[2]-vector[2]*old_normal[1];
										normal2[1]=vector[2]*old_normal[0]-vector[0]*old_normal[2];
										normal2[2]=vector[0]*old_normal[1]-vector[1]*old_normal[0];
									}
								}
								/* get normal = normal2 (x) vector */
								normal[0]=normal2[1]*vector[2]-normal2[2]*vector[1];
								normal[1]=normal2[2]*vector[0]-normal2[0]*vector[2];
								normal[2]=normal2[0]*vector[1]-normal2[1]*vector[0];
								/* make normal unit length */
								magnitude=sqrt(normal[0]*normal[0]+normal[1]*normal[1]+
									normal[2]*normal[2]);
								normal[0] /= magnitude;
								normal[1] /= magnitude;
								normal[2] /= magnitude;
								/* get normal2 = vector (x) normal */
								normal2[0]=vector[1]*normal[2]-vector[2]*normal[1];
								normal2[1]=vector[2]*normal[0]-vector[0]*normal[2];
								normal2[2]=vector[0]*normal[1]-vector[1]*normal[0];
								/* make normal2 unit length */
								magnitude=sqrt(normal2[0]*normal2[0]+normal2[1]*normal2[1]+
									normal2[2]*normal2[2]);
								normal2[0] /= magnitude;
								normal2[1] /= magnitude;
								normal2[2] /= magnitude;
								/* rotate normals by angle */
								cos_angle=cos(angle);
								sin_angle=sin(angle);
								temp = normal[0];
								normal[0]  = normal[0]  * cos_angle + normal2[0] * sin_angle;
								normal2[0] = normal2[0] * cos_angle - temp       * sin_angle;
								temp = normal[1];
								normal[1]  = normal[1]  * cos_angle + normal2[1] * sin_angle;
								normal2[1] = normal2[1] * cos_angle - temp       * sin_angle;
								temp = normal[2];
								normal[2]  = normal[2]  * cos_angle + normal2[2] * sin_angle;
								normal2[2] = normal2[2] * cos_angle - temp       * sin_angle;
								/* store old_normal, and old coordinates in displacement */
								old_normal[0]=normal[0];
								old_normal[1]=normal[1];
								old_normal[2]=normal[2];
								displacement[0]=coordinates[0];
								displacement[1]=coordinates[1];
								displacement[2]=coordinates[2];
								previous_curl_component = curl_component;
#if defined (OLD_CODE)
								/* Get an initial orthogonal vector */
								if (vector_magnitude > 1e-30)
								{
									if ( fabs(vector[1])>(0.1 * vector_magnitude) || 
										fabs(vector[2])>(0.1 * vector_magnitude))
									{
										/*???RC why change magnitude here? Need for data. */
										magnitude = sqrt(vector[1]*vector[1] + vector[2]*vector[2]);
										normal[0] = 0.0;
										normal[1] = vector[2] / magnitude;
										normal[2] = -vector[1] / magnitude;
										magnitude *= vector_magnitude;
										normal2[0] = -(vector[2] * vector[2] +
											vector[1] * vector[1]) / magnitude;
										normal2[1] = vector[0] * vector[1] / magnitude;
										normal2[2] = vector[0] * vector[2] / magnitude;
									}
									else
									{
										/*???RC why change magnitude here? Need for data. */
										magnitude = sqrt(vector[0]*vector[0] + vector[2]*vector[2]);
										normal[0] = -vector[2] / magnitude;
										normal[1] = 0.0;
										normal[2] = vector[0] / magnitude;
										magnitude *= vector_magnitude;
										normal2[0] = vector[0] * vector[1] / magnitude;
										normal2[1] = -(vector[0] * vector[0] +
											vector[2] * vector[2]) / magnitude;
										normal2[2] = vector[1] * vector[2] / magnitude;
									}
									temp = normal[0];
									normal[0] = normal[0] * cos(angle) + normal2[0] * sin(angle);
									normal2[0] = normal2[0] * cos(angle) - temp * sin(angle);
									temp = normal[1];
									normal[1] = normal[1] * cos(angle) + normal2[1] * sin(angle);
									normal2[1] = normal2[1] * cos(angle) - temp * sin(angle);
									temp = normal[2];
									normal[2] = normal[2] * cos(angle) + normal2[2] * sin(angle);
									normal2[2] = normal2[2] * cos(angle) - temp * sin(angle);
								}
#endif /* defined (OLD_CODE) */
							} break;
							case 6:
							{
								/* get stream vector and normal from stream_vector_values */
								if (reverse_track)
								{
									normal[0]=-stream_vector_values[3];
									normal[1]=-stream_vector_values[4];
									normal[2]=-stream_vector_values[5];
								}
								else
								{
									normal[0]=stream_vector_values[3];
									normal[1]=stream_vector_values[4];
									normal[2]=stream_vector_values[5];
								}
								/* get normal2 = vector (x) normal */
								normal2[0]=vector[1]*normal[2]-vector[2]*normal[1];
								normal2[1]=vector[2]*normal[0]-vector[0]*normal[2];
								normal2[2]=vector[0]*normal[1]-vector[1]*normal[0];
								/* make normal2 unit length */
								if (0.0<(magnitude=sqrt(normal2[0]*normal2[0]+
									normal2[1]*normal2[1]+normal2[2]*normal2[2])))
								{
									normal2[0] /= magnitude;
									normal2[1] /= magnitude;
									normal2[2] /= magnitude;
								}
							} break;
							case 9:
							{
								/* get vector, normal and normal2 from stream_vector_values */
								if (reverse_track)
								{
									normal[0]=-stream_vector_values[3];
									normal[1]=-stream_vector_values[4];
									normal[2]=-stream_vector_values[5];
								}
								else
								{
									normal[0]=stream_vector_values[3];
									normal[1]=stream_vector_values[4];
									normal[2]=stream_vector_values[5];
								}
								normal2[0]=stream_vector_values[6];
								normal2[1]=stream_vector_values[7];
								normal2[2]=stream_vector_values[8];
							} break;
						}
						/* calculate data */
						switch (data_type)
						{
							case STREAM_NO_DATA:
							{
								/* do nothing */
							} break;
							case STREAM_MAGNITUDE_SCALAR:
							{
								data_value = vector_magnitude;
							} break;
							case STREAM_FIELD_SCALAR:
							{
								if (!Computed_field_evaluate_in_element(
									data_field,*element,xi,(struct FE_element *)NULL,&data_value,(FE_value *)NULL))
								{
									display_message(ERROR_MESSAGE,
										"track_streamline_from_FE_element.  "
										"Error calculating data field");
									keep_tracking = 0;
								}
							} break;
							case STREAM_TRAVEL_SCALAR:
							{
								if (reverse_track)
								{
									data_value = -total_stepped;
								}
								else
								{
									data_value = total_stepped;
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"track_streamline_from_FE_element.  Unknown data type");
								keep_tracking = 0;
							} break;
						}
						/* set point, normal and data */
						(*stream_point)[0] = coordinates[0];
						(*stream_point)[1] = coordinates[1];
						(*stream_point)[2] = coordinates[2];
						stream_point++;
						(*stream_vector)[0] = vector[0];
						(*stream_vector)[1] = vector[1];
						(*stream_vector)[2] = vector[2];
						stream_vector++;
						(*stream_normal)[0] = normal2[0];
						(*stream_normal)[1] = normal2[1];
						(*stream_normal)[2] = normal2[2];
						stream_normal++;
						if (STREAM_NO_DATA != data_type)
						{
							*stream_datum = (GTDATA)data_value;
							stream_datum++;
						}
						i++;
						previous_total_stepped=total_stepped;
						/* perform the tracking, changing elements as necessary */
						if (total_stepped<length)
						{
							keep_tracking=update_adaptive_imp_euler(coordinate_field,
								stream_vector_field,reverse_track,*element,xi,coordinates,
								&step_size,&total_stepped,&change_element);
							if (keep_tracking&&change_element)
							{
								if (previous_change_element==change_element)
								{
									/* trapped between two elements with opposing directions */
									/*???debug*/
									printf("track_streamline_from_FE_element.  "
										"trapped between two elements with opposing directions\n");
									keep_tracking = 0;
								}
								else
								{
									keep_tracking=check_xi_limits(change_element,element,xi,
										coordinate_field);
								}
							}
							previous_change_element=change_element;
						}
						else
						{
							keep_tracking = 0;
						}
					}
				}
				if (keep_tracking||(i<allocated_number_of_points))
				{
					if (keep_tracking)
					{
						/* allocate 100 more spaces for points, normals and data */
						allocated_number_of_points += 100;
					}
					else
					{
						/* free unused space in arrays */
						allocated_number_of_points = i;
					}
					if (REALLOCATE(tmp_triples,*stream_points,Triple,
						allocated_number_of_points))
					{
						*stream_points=tmp_triples;
						stream_point = (*stream_points) + i;
					}
					else
					{
						return_code=0;
					}
					if (REALLOCATE(tmp_triples,*stream_vectors,Triple,
						allocated_number_of_points))
					{
						*stream_vectors=tmp_triples;
						stream_vector = (*stream_vectors) + i;
					}
					else
					{
						return_code=0;
					}
					if (REALLOCATE(tmp_triples,*stream_normals,Triple,
						allocated_number_of_points))
					{
						*stream_normals=tmp_triples;
						stream_normal = (*stream_normals) + i;
					}
					else
					{
						return_code=0;
					}
					if (STREAM_NO_DATA != data_type)
					{
						if (REALLOCATE(tmp_stream_data,*stream_data,GTDATA,
							allocated_number_of_points))
						{
							*stream_data=tmp_stream_data;
							stream_datum = (*stream_data) + i;
						}
						else
						{
							return_code=0;
						}
					}
					if (!return_code)
					{
						display_message(ERROR_MESSAGE,
							"track_streamline_from_FE_element.  Could not reallocate");
						keep_tracking=0;
					}
				}
			}
			*number_of_points = i;
			Computed_field_clear_cache(coordinate_field);
			Computed_field_clear_cache(stream_vector_field);
			if (STREAM_FIELD_SCALAR==data_type)
			{
				Computed_field_clear_cache(data_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"track_streamline_from_FE_element.  "
				"Not enough memory for streamline");
			return_code=0;
		}
		if (!return_code)
		{
			/* free up any space allocated for the streamline */
			if (*stream_points)
			{
				DEALLOCATE(*stream_points);
			}
			if (*stream_vectors)
			{
				DEALLOCATE(*stream_vectors);
			}
			if (*stream_normals)
			{
				DEALLOCATE(*stream_normals);
			}
			if (*stream_data)
			{
				DEALLOCATE(*stream_data);
			}
			*number_of_points=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"track_streamline_from_FE_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* track_streamline_from_FE_element */

/*
Global functions
----------------
*/
DECLARE_OBJECT_FUNCTIONS(Interactive_streamline)

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Interactive_streamline)

DECLARE_LIST_FUNCTIONS(Interactive_streamline)

DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Interactive_streamline,name,char *,
	strcmp)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Interactive_streamline,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Interactive_streamline,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Interactive_streamline,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_streamline,name)(
				destination,source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Interactive_streamline,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Interactive_streamline,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Interactive_streamline,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Interactive_streamline,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_streamline,name));
	/* check arguments */
	if (source&&destination)
	{
		/* copy values */
		destination->element = source->element;
		destination->coordinate_field = source->coordinate_field;
		destination->stream_vector_field = source->stream_vector_field;
		destination->length = source->length;
		destination->xi[0] = source->xi[0];
		destination->xi[1] = source->xi[1];
		destination->xi[2] = source->xi[2];
		destination->previous_point[0] = source->previous_point[0];
		destination->previous_point[1] = source->previous_point[1];
		destination->previous_point[2] = source->previous_point[2];
		destination->selected = 0;
		destination->graphics_object = (struct GT_object *)NULL;
		destination->streamline_graphics_object = (struct GT_object *)NULL;
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_streamline,name).  Not copying graphics objects yet");
/*		return_code=0;*/
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_streamline,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_streamline,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Interactive_streamline,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Interactive_streamline,name));
	/* check arguments */
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"MANAGER_COPY_IDENTIFIER(Interactive_streamline,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
	"MANAGER_COPY_IDENTIFIER(Interactive_streamline,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Interactive_streamline,name) */

DECLARE_MANAGER_FUNCTIONS(Interactive_streamline)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Interactive_streamline,name,char *)

void interactive_streamline_callback (Widget caller,
	XtPointer window_void,
	XtPointer caller_data)
/*******************************************************************************
LAST MODIFIED : 16 May 1998

DESCRIPTION :
Handles the mouse events which may affect interactive streamlines.
==============================================================================*/
{
#if defined (OLD_GFX_WINDOW)
	struct Graphics_window *window;
	struct MANAGER_MESSAGE(Interactive_streamline) message;
	struct Streamline_event streamline_event;
#endif
	ENTER(interactive_streamline_callback);
	if ( window_void )
	{
#if defined (OLD_GFX_WINDOW)
		window = (struct Graphics_window *) window_void;
		streamline_event.caller_data = (XmAnyCallbackStruct *) caller_data;
		streamline_event.caller = caller;
#else
		USE_PARAMETER(caller_data);
		USE_PARAMETER(caller);
#endif
/*???debug */
printf("interactive_streamline_callback.  Temporarily disabled\n");
display_message(WARNING_MESSAGE,
	"interactive_streamline_callback.  Temporarily disabled");
#if defined (OLD_GFX_WINDOW)
		if (window->streamline_manager)
		{
			FOR_EACH_OBJECT_IN_MANAGER(Interactive_streamline)(
				handle_interactive_streamline_event,
				(void *) &streamline_event,
				window->streamline_manager );
			message.change = MANAGER_CHANGE_ALL(Interactive_streamline);
			message.object_changed = (struct Interactive_streamline *)NULL;
			MANAGER_UPDATE(Interactive_streamline)(&message, streamline_manager);
		}
		else
		{
			display_message(ERROR_MESSAGE,
					"interactive_streamline_callback.  Invalid streamline manager");
		}
#endif /* defined (OLD_GFX_WINDOW) */

	}
	else
	{
		display_message(ERROR_MESSAGE,
				"interactive_streamline_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* interactive_streamline_callback */

int update_interactive_streamline(struct Interactive_streamline *streamline,
	FE_value *new_point,
	struct MANAGER(Interactive_streamline) *streamline_manager )
/*******************************************************************************
LAST MODIFIED : 22 June 2000

DESCRIPTION :
Moves a streampoint to the new_point position.  This function works by an
incremental step change to allow it to calculate the correct element and the
xi coordinates.  To be accurate the change in position must be small.
==============================================================================*/
{
	FE_value translate[3];
	int return_code; 
	struct GT_polyline *polyline;
	struct GT_surface *surface;

	ENTER(update_interactive_streamline);
	if (streamline&&new_point)
	{
		translate[0]=new_point[0]-(streamline->previous_point)[0];
		translate[1]=new_point[1]-(streamline->previous_point)[1];
		translate[2]=new_point[2]-(streamline->previous_point)[2];
#if defined (DEBUG)
		/*???debug */
		printf("Move in world coordinates %f %f %f\n",translate[0],translate[1],
			translate[2]);
		/*???debug end */
#endif /* defined (DEBUG) */
		if (update_interactive_streampoint((FE_value *)
			(*(streamline->graphics_object->gu.gt_pointset))->pointlist,
			&(streamline->element), streamline->coordinate_field,
			streamline->xi, translate ) )
		{
			(streamline->previous_point)[0]=
				((*(streamline->graphics_object->gu.gt_pointset))->pointlist)[0][0];
			(streamline->previous_point)[1]=
				((*(streamline->graphics_object->gu.gt_pointset))->pointlist)[0][1];
			(streamline->previous_point)[2]=
				((*(streamline->graphics_object->gu.gt_pointset))->pointlist)[0][2];
			/* rebuild the streamline */
			if (STREAM_LINE==streamline->type)
			{
				if (polyline=create_GT_polyline_streamline_FE_element(
					streamline->element,streamline->xi,streamline->coordinate_field,
					streamline->stream_vector_field,streamline->reverse_track,
					streamline->length,streamline->data_type,streamline->data_field))
				{
					if (!(return_code=GT_OBJECT_ADD(GT_polyline)(
						streamline->streamline_graphics_object,
						/*time*/0.0,polyline)))
					{
						DESTROY(GT_polyline)(&polyline);
					}
				}
				else
				{
					return_code=0;
				}
			}
			else if ((streamline->type == STREAM_RIBBON)||
				(streamline->type == STREAM_EXTRUDED_RECTANGLE)||
				(streamline->type == STREAM_EXTRUDED_ELLIPSE))
			{
				if (surface=create_GT_surface_streamribbon_FE_element(
					streamline->element,streamline->xi,streamline->coordinate_field,
					streamline->stream_vector_field,streamline->reverse_track,
					streamline->length,streamline->width,streamline->type,
					streamline->data_type,streamline->data_field))
				{
					if (!(return_code=GT_OBJECT_ADD(GT_surface)(
						streamline->streamline_graphics_object,
						/*time*/0.0,surface)))
					{
						DESTROY(GT_surface)(&surface);
					}
				}
				else
				{
					return_code=0;
				}
			}
			MANAGER_NOTE_CHANGE(Interactive_streamline)(
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Interactive_streamline),
				streamline,streamline_manager);
			return_code=1;
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"update_interactive_streamline.  Could not update streampoint");
		}
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,
			"update_interactive_streamline.  Invalid argument(s)");
	}
	LEAVE;

	return ( return_code );
} /* update_interactive_streamline */

int interactive_streamline_set_changed ( struct Interactive_streamline *streamline)
/*******************************************************************************
LAST MODIFIED : 29 October 1997

DESCRIPTION :
Sets the graphics object status as uncreated so that they get regenrated when
window is updated.
==============================================================================*/
{
	int return_code;

	if ( streamline )
	{
		GT_object_changed(streamline->graphics_object);
		GT_object_changed(streamline->streamline_graphics_object);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"interactive_streamline_set_changed.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;
	return (return_code);
} /* interactive_streamline_set_changed */

struct Interactive_streamline *CREATE(Interactive_streamline)(char *name,
	enum Streamline_type type,struct FE_element *element,FE_value *xi,
	struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	float length,float width,enum Streamline_data_type data_type,
	struct Computed_field *data_field,struct GT_object *graphics_object,
	struct GT_object *streamline_graphics_object)
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Allocates memory and assigns fields for a Interactive_streamline object.
???RC Changed to access Computed_fields so that wrappers were not destroyed.
Should also access other objects.
==============================================================================*/
{
	struct Interactive_streamline *streamline;

	ENTER(CREATE(Interactive_streamline));
	if (ALLOCATE(streamline,struct Interactive_streamline,1))
	{
		if (name)
		{
			if (ALLOCATE(streamline->name,char,strlen(name)+1))
			{
				strcpy(streamline->name,name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"CREATE(Interactive_streamline).  Could not allocate streamline->name");
				DEALLOCATE(streamline);
			}
		}
		else
		{
			streamline->name=(char *)NULL;
		}
		if (streamline)
		{
			streamline->type=type;
			streamline->element=element;
			streamline->coordinate_field=ACCESS(Computed_field)(coordinate_field);
			streamline->stream_vector_field=
				ACCESS(Computed_field)(stream_vector_field);
			streamline->reverse_track=reverse_track;
			streamline->length=length;
			streamline->width=width;
			streamline->xi[0]=xi[0];
			streamline->xi[1]=xi[1];
			streamline->xi[2]=xi[2];
			streamline->data_type=data_type;
			streamline->data_field=data_field;
			if (data_field)
			{
				ACCESS(Computed_field)(data_field);
			}
			streamline->graphics_object=graphics_object;
			streamline->streamline_graphics_object=streamline_graphics_object;
			if (streamline->graphics_object)
			{
				(streamline->previous_point)[0]=
					((*(streamline->graphics_object->gu.gt_pointset))->pointlist)[0][0];
				(streamline->previous_point)[1]=
					((*(streamline->graphics_object->gu.gt_pointset))->pointlist)[0][1];
				(streamline->previous_point)[2]=
					((*(streamline->graphics_object->gu.gt_pointset))->pointlist)[0][2];
			}
			streamline->selected=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Interactive_streamline).  Could not allocate streamline");
	}
	LEAVE;

	return (streamline);
} /* CREATE(Interactive_streamline) */

int DESTROY(Interactive_streamline)(
	struct Interactive_streamline **streamline_ptr)
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Frees the memory for the fields of <**streamline>, frees the memory for
<**streamline> and sets <*streamline> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Interactive_streamline));
	if (streamline_ptr)
	{
		if (*streamline_ptr)
		{
			DEACCESS(Computed_field)(&((*streamline_ptr)->coordinate_field));
			DEACCESS(Computed_field)(&((*streamline_ptr)->stream_vector_field));
			if ((*streamline_ptr)->data_field)
			{
				DEACCESS(Computed_field)(&((*streamline_ptr)->data_field));
			}
			DEALLOCATE(*streamline_ptr);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Interactive_streamline).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Interactive_streamline) */

int get_streamline_gt_object( struct Interactive_streamline *streamline,
	struct GT_object **gt_object )
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Returns the GT_object which represents the streamline.
==============================================================================*/
{
	int return_code;

	ENTER(interactive_streamline_callback);

	if ( streamline )
	{
		*gt_object = streamline->graphics_object;
		return_code = 1;
	}
	else
	{
		*gt_object = (struct GT_object *)NULL;
		display_message(ERROR_MESSAGE,
				"get_streamline_gt_object.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;
	return ( return_code );
} /* get_streamline_gt_object */

struct GT_polyline *create_GT_polyline_streamline_FE_element(
	struct FE_element *element,FE_value *start_xi,
	struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	float length,enum Streamline_data_type data_type,
	struct Computed_field *data_field)
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Creates a <GT_polyline> streamline from the <coordinate_field> following
<stream_vector_field> (with 3, 6 or 9 components) starting in the element and at
the xi coordinates supplied. If <reverse_track> is true, the reverse of the
stream vector is tracked, and the travel_scalar is made negative.
==============================================================================*/
{
	gtDataType gt_data_type;
	GTDATA *stream_data;
	int number_of_stream_points,number_of_stream_vector_components;
	struct GT_polyline *polyline;
	Triple *stream_points,*stream_normals,*stream_vectors;

	ENTER(create_GT_polyline_streamline_FE_element);
	if (element&&(element->shape)&&(3==element->shape->dimension)&&start_xi&&
		coordinate_field&&
		(3>=Computed_field_get_number_of_components(coordinate_field))&&
		stream_vector_field&&((3==(number_of_stream_vector_components=
			Computed_field_get_number_of_components(stream_vector_field)))||
			(6==number_of_stream_vector_components)||
			(9==number_of_stream_vector_components))&&
		(0.0<length)&&((data_type!=STREAM_FIELD_SCALAR) || data_field))
	{
		/* track points and normals on streamline, and data if requested */
		if (track_streamline_from_FE_element(&element,start_xi,
			coordinate_field,stream_vector_field,reverse_track,length,
			data_type,data_field,&number_of_stream_points,&stream_points,
			&stream_vectors,&stream_normals,&stream_data))
		{
			if (0<number_of_stream_points)
			{
				/* now create a polyline from the points */
				if (STREAM_NO_DATA != data_type)
				{
					gt_data_type=g_SCALAR;
				}
				else
				{
					gt_data_type=g_NO_DATA;
				}
				if (!(polyline=CREATE(GT_polyline)(g_PLAIN,number_of_stream_points,
					stream_points,/* normals */(Triple *)NULL,
					gt_data_type,stream_data)))
				{
					display_message(ERROR_MESSAGE,
						"create_GT_polyline_streamline_FE_element.  "
						"Could not create polyline");
					DEALLOCATE(stream_points);
					DEALLOCATE(stream_data);
				}
				/* didn't want vectors and normals anyway */
				DEALLOCATE(stream_vectors);
			}
			else
			{
				/* no error: streamline empty */
				polyline = (struct GT_polyline *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_GT_polyline_streamline_FE_element.  "
				"failed to track streamline");
			polyline = (struct GT_polyline *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_GT_polyline_streamline_FE_element.  Invalid argument(s)");
		polyline=(struct GT_polyline *)NULL;
	}
	LEAVE;

	return (polyline);
} /* create_GT_polyline_streamline_FE_element */

struct GT_surface *create_GT_surface_streamribbon_FE_element(
	struct FE_element *element,FE_value *start_xi,
	struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	float length,float width,enum Streamline_type type,
	enum Streamline_data_type data_type,struct Computed_field *data_field)
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Creates a <GT_surface> streamline from the <coordinate_field> following
<stream_vector_field> (with 3, 6 or 9 components) starting in the element and at
the xi coordinates supplied. If <reverse_track> is true, the reverse of the
stream vector is tracked, and the travel_scalar is made negative.
==============================================================================*/
{
	float cosw,magnitude,sinw,thickness;
	gtDataType gt_data_type;
	GTDATA *data,*datum,*stream_data,stream_datum;
	int d,i,number_of_stream_points,number_of_stream_vector_components,
		surface_points_per_step;
	struct GT_surface *surface;
	Triple cross_thickness,cross_width,*normal,*normalpoints,*point,*points,stream_cross,
		stream_normal,*stream_normals,stream_point,*stream_points,
		stream_unit_vector,stream_vector,*stream_vectors;

	ENTER(create_GT_surface_streamribbon_FE_element);
	thickness = 0.2 * width;
	if (element&&(element->shape)&&(3==element->shape->dimension)&&start_xi&&
		coordinate_field&&
		(3>=Computed_field_get_number_of_components(coordinate_field))&&
		stream_vector_field&&((3==(number_of_stream_vector_components=
			Computed_field_get_number_of_components(stream_vector_field)))||
			(6==number_of_stream_vector_components)||
			(9==number_of_stream_vector_components))&&
		(0.0<length)&&((data_type!=STREAM_FIELD_SCALAR) || data_field))
	{
		/* track points and normals on streamline, and data if requested */
		if (track_streamline_from_FE_element(&element,start_xi,
			coordinate_field,stream_vector_field,reverse_track,length,
			data_type,data_field,&number_of_stream_points,&stream_points,
			&stream_vectors,&stream_normals,&stream_data))
		{
			if (0<number_of_stream_points)
			{
				switch (type)
				{
					case STREAM_EXTRUDED_RECTANGLE:
					{
						surface_points_per_step = 8;
					} break;
					case STREAM_EXTRUDED_ELLIPSE:
					{
						surface_points_per_step = 20;
					} break;
					case STREAM_RIBBON:
					default:
					{
						surface_points_per_step = 2;
					} break;
				}
				/* now create a surface from the points */
				points=(Triple *)NULL;
				data=(GTDATA *)NULL;
				if (STREAM_NO_DATA != data_type)
				{
					gt_data_type=g_SCALAR;
					ALLOCATE(data,GTDATA,surface_points_per_step*number_of_stream_points);
				}
				else
				{
					gt_data_type=g_NO_DATA;
				}
				if (ALLOCATE(points,Triple,surface_points_per_step*number_of_stream_points)&&
					ALLOCATE(normalpoints,Triple,surface_points_per_step*number_of_stream_points)
					&&((g_NO_DATA==gt_data_type)||data))
				{
					if (surface=CREATE(GT_surface)(g_SHADED_TEXMAP,g_QUADRILATERAL,
						surface_points_per_step,number_of_stream_points,points,
						normalpoints, /*Texturepoints*/(Triple *)NULL,gt_data_type,data))
					{
						point = points;
						normal = normalpoints;
						datum = data;
						/* now fill in the points and data from the streamline */
						for (i=0;i<number_of_stream_points;i++)
						{
							stream_point[0]=stream_points[i][0];
							stream_point[1]=stream_points[i][1];
							stream_point[2]=stream_points[i][2];
							stream_vector[0]=stream_vectors[i][0];
							stream_vector[1]=stream_vectors[i][1];
							stream_vector[2]=stream_vectors[i][2];
							stream_normal[0]=stream_normals[i][0];
							stream_normal[1]=stream_normals[i][1];
							stream_normal[2]=stream_normals[i][2];
							if (stream_data)
							{
								stream_datum=stream_data[i];
							}
							if (0.0 < (magnitude = sqrt(stream_vector[0]*stream_vector[0]+
								stream_vector[1]*stream_vector[1]+
								stream_vector[2]*stream_vector[2])))
							{
								stream_unit_vector[0] = stream_vector[0] / magnitude;
								stream_unit_vector[1] = stream_vector[1] / magnitude;
								stream_unit_vector[2] = stream_vector[2] / magnitude;
							}
							/* get stream_cross = stream_normal (x) stream_unit_vector */
							stream_cross[0]=stream_normal[1]*stream_unit_vector[2]-
								stream_normal[2]*stream_unit_vector[1];
							stream_cross[1]=stream_normal[2]*stream_unit_vector[0]-
								stream_normal[0]*stream_unit_vector[2];
							stream_cross[2]=stream_normal[0]*stream_unit_vector[1]-
								stream_normal[1]*stream_unit_vector[0];
							cross_width[0] = stream_cross[0] * 0.5 * width;
							cross_width[1] = stream_cross[1] * 0.5 * width;
							cross_width[2] = stream_cross[2] * 0.5 * width;
							cross_thickness[0] = stream_normal[0] * 0.5 * thickness;
							cross_thickness[1] = stream_normal[1] * 0.5 * thickness;
							cross_thickness[2] = stream_normal[2] * 0.5 * thickness;
							switch (type)
							{
								case STREAM_RIBBON:
								{
									(*point)[0] = stream_point[0] + cross_width[0];
									(*point)[1] = stream_point[1] + cross_width[1];
									(*point)[2] = stream_point[2] + cross_width[2];
									point++;
									(*normal)[0] = stream_normal[0];
									(*normal)[1] = stream_normal[1];
									(*normal)[2] = stream_normal[2];
									normal++;
									if (datum)
									{
										*datum = stream_datum;
										datum++;
									}
									(*point)[0] = stream_point[0] - cross_width[0];
									(*point)[1] = stream_point[1] - cross_width[1];
									(*point)[2] = stream_point[2] - cross_width[2];
									point++;
									(*normal)[0] = stream_normal[0];
									(*normal)[1] = stream_normal[1];
									(*normal)[2] = stream_normal[2];
									normal++;
									if (datum)
									{
										*datum = stream_datum;
										datum++;
									}
								} break;
								case STREAM_EXTRUDED_ELLIPSE:
								{
									for (d = 0 ; d < surface_points_per_step ; d++)
									{
										sinw = sin( 2 * PI * (double)d /
											(double)(surface_points_per_step - 1));
										cosw = cos( 2 * PI * (double)d /
											(double)(surface_points_per_step - 1));
										(*point)[0] = stream_point[0] + sinw * cross_width[0]
											+ cosw * cross_thickness[0];
										(*point)[1] = stream_point[1] + sinw * cross_width[1]
											+ cosw * cross_thickness[1];
										(*point)[2] = stream_point[2] + sinw * cross_width[2]
											+ cosw * cross_thickness[2];
										point++;
										(*normal)[0] = -sinw * stream_cross[0] * 0.5 * thickness -
											cosw * stream_normal[0] * 0.5 * width;
										(*normal)[1] = -sinw * stream_cross[1] * 0.5 * thickness -
											cosw * stream_normal[1] * 0.5 * thickness;
										(*normal)[2] = -sinw * stream_cross[2] * 0.5 * thickness -
											cosw * stream_normal[2] * 0.5 * thickness;
										magnitude = sqrt((*normal)[0] * (*normal)[0]
											+ (*normal)[1] * (*normal)[1]
											+ (*normal)[2] * (*normal)[2]);
										(*normal)[0] /= magnitude;
										(*normal)[1] /= magnitude;
										(*normal)[2] /= magnitude;
										normal++;
										if (datum)
										{
											*datum = stream_datum;
											datum++;
										}
									}
								} break;
								case STREAM_EXTRUDED_RECTANGLE:
								{
									(*point)[0] = stream_point[0] + cross_width[0]
										+ cross_thickness[0];
									(*point)[1] = stream_point[1] + cross_width[1]
										+ cross_thickness[1];
									(*point)[2] = stream_point[2] + cross_width[2]
										+ cross_thickness[2];
									point++;
									(*normal)[0] = stream_normal[0];
									(*normal)[1] = stream_normal[1];
									(*normal)[2] = stream_normal[2];
									normal++;
									if (datum)
									{
										*datum = stream_datum;
										datum++;
									}
									(*point)[0] = stream_point[0] - cross_width[0]
										+ cross_thickness[0];
									(*point)[1] = stream_point[1] - cross_width[1]
										+ cross_thickness[1];
									(*point)[2] = stream_point[2] - cross_width[2]
										+ cross_thickness[2];
									point++;
									(*normal)[0] = stream_normal[0];
									(*normal)[1] = stream_normal[1];
									(*normal)[2] = stream_normal[2];
									normal++;
									if (datum)
									{
										*datum = stream_datum;
										datum++;
									}
									
									(*point)[0] = stream_point[0] - cross_width[0]
										+ cross_thickness[0];
									(*point)[1] = stream_point[1] - cross_width[1]
										+ cross_thickness[1];
									(*point)[2] = stream_point[2] - cross_width[2]
										+ cross_thickness[2];
									point++;
									(*normal)[0] = -stream_cross[0];
									(*normal)[1] = -stream_cross[1];
									(*normal)[2] = -stream_cross[2];
									normal++;
									if (datum)
									{
										*datum = stream_datum;
										datum++;
									}
									(*point)[0] = stream_point[0] - cross_width[0]
										- cross_thickness[0];
									(*point)[1] = stream_point[1] - cross_width[1]
										- cross_thickness[1];
									(*point)[2] = stream_point[2] - cross_width[2]
										- cross_thickness[2];
									point++;
									(*normal)[0] = -stream_cross[0];
									(*normal)[1] = -stream_cross[1];
									(*normal)[2] = -stream_cross[2];
									normal++;
									if (datum)
									{
										*datum = stream_datum;
										datum++;
									}
									
									(*point)[0] = stream_point[0] - cross_width[0]
										- cross_thickness[0];
									(*point)[1] = stream_point[1] - cross_width[1]
										- cross_thickness[1];
									(*point)[2] = stream_point[2] - cross_width[2]
										- cross_thickness[2];
									point++;
									(*normal)[0] = -stream_normal[0];
									(*normal)[1] = -stream_normal[1];
									(*normal)[2] = -stream_normal[2];
									normal++;
									if (datum)
									{
										*datum = stream_datum;
										datum++;
									}
									(*point)[0] = stream_point[0] + cross_width[0]
										- cross_thickness[0];
									(*point)[1] = stream_point[1] + cross_width[1]
										- cross_thickness[1];
									(*point)[2] = stream_point[2] + cross_width[2]
										- cross_thickness[2];
									point++;
									(*normal)[0] = -stream_normal[0];
									(*normal)[1] = -stream_normal[1];
									(*normal)[2] = -stream_normal[2];
									normal++;
									if (datum)
									{
										*datum = stream_datum;
										datum++;
									}
									
									(*point)[0] = stream_point[0] + cross_width[0]
										- cross_thickness[0];
									(*point)[1] = stream_point[1] + cross_width[1]
										- cross_thickness[1];
									(*point)[2] = stream_point[2] + cross_width[2]
										- cross_thickness[2];
									point++;
									(*normal)[0] = stream_cross[0];
									(*normal)[1] = stream_cross[1];
									(*normal)[2] = stream_cross[2];
									normal++;
									if (datum)
									{
										*datum = stream_datum;
										datum++;
									}
									(*point)[0] = stream_point[0] + cross_width[0]
										+ cross_thickness[0];
									(*point)[1] = stream_point[1] + cross_width[1]
										+ cross_thickness[1];
									(*point)[2] = stream_point[2] + cross_width[2]
										+ cross_thickness[2];
									point++;
									(*normal)[0] = stream_cross[0];
									(*normal)[1] = stream_cross[1];
									(*normal)[2] = stream_cross[2];
									normal++;
									if (datum)
									{
										*datum = stream_datum;
										datum++;
									}
								} break;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_GT_surface_streamribbon_FE_element. "
							"Could not create surface");
						DEALLOCATE(data);
						DEALLOCATE(points);
						DEALLOCATE(normalpoints);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_GT_surface_streamribbon_FE_element. "
						"Could not allocate memory for points");
					DEALLOCATE(data);
					surface=(struct GT_surface *)NULL;
				}
				/* no longer need original streamline */
				DEALLOCATE(stream_points);
				DEALLOCATE(stream_vectors);
				DEALLOCATE(stream_normals);
				DEALLOCATE(stream_data);
			}
			else
			{
				/* no error: streamline empty */
				surface=(struct GT_surface *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_GT_surface_streamline_FE_element.  "
				"failed to track streamline");
			surface=(struct GT_surface *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_GT_surface_streamline_FE_element.  Invalid argument(s)");
		surface=(struct GT_surface *)NULL;
	}
	LEAVE;

	return (surface);
} /* create_GT_surface_streamribbon_FE_element */

struct GT_pointset *create_interactive_streampoint(struct FE_element *element,
	struct Computed_field *coordinate_field,float length,FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 30 June 1999

DESCRIPTION :
Creates a <GT_pointset> streampoint which can be manipulated with the mouse.
==============================================================================*/
{
	FE_value coordinates[3];
	float point_size;
	struct GT_pointset *point_set;
	Triple *point;

	ENTER(create_interactive_streampoint);
	point_size=length/100;
	/* check the arguments */
	if (element&&(element->shape)&&(3==element->shape->dimension)&&(length>0.0)&&
		coordinate_field&&
		(3>=Computed_field_get_number_of_components(coordinate_field)))
	{
		coordinates[0]=0.0;
		coordinates[1]=0.0;
		coordinates[2]=0.0;
		if (Computed_field_evaluate_in_element(coordinate_field,element,xi,
			(struct FE_element *)NULL,coordinates,(FE_value *)NULL))
		{
			if (ALLOCATE(point,Triple,1))
			{
				(*point)[0]=coordinates[0];
				(*point)[1]=coordinates[1];
				(*point)[2]=coordinates[2];
				if (!(point_set=CREATE(GT_pointset)(1, point,(char **)NULL,
					g_PLUS_MARKER,point_size,g_NO_DATA,(GTDATA *)NULL,(int *)NULL)))
				{
					display_message(ERROR_MESSAGE,
						"create_interactive_streampoint.  Unable to create pointset");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_interactive_streampoint.  Unable to allocate point space");
				point_set=(struct GT_pointset *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_interactive_streampoint.  Error calculating coordinate field");
			point_set=(struct GT_pointset *)NULL;
		}
		Computed_field_clear_cache(coordinate_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_interactive_streampoint.  Invalid argument(s)");
		point_set=(struct GT_pointset *)NULL;
	}
	LEAVE;

	return(point_set);
} /* create_interactive_streampoint */

int add_flow_particle(struct Streampoint **list,FE_value *xi,
	struct FE_element *element,Triple **pointlist,int index,
	struct Computed_field *coordinate_field,gtObject *graphics_object)
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Adds a new flow particle structure to the start of the Streampoint list
==============================================================================*/
{
	int return_code;
	struct Streampoint *point;

	ENTER( add_flow_particle );

	if ( list && xi && element && pointlist )
	{
		if ( ALLOCATE( point, struct Streampoint, 1 ))
		{
			point->xi[0] = xi[0];
			point->xi[1] = xi[1];
			point->xi[2] = xi[2];
			point->element = element;
			point->pointlist = pointlist;
			point->index = index;
			point->graphics_object = graphics_object;
			point->next = *list;

			*list = point;

			return_code = update_interactive_streampoint(
				(FE_value *)((*pointlist) + index),
				&element, coordinate_field, xi, (FE_value *)NULL );
		}
		else
		{
			display_message(ERROR_MESSAGE,"add_flow_particle.  Could not allocate memory for streampoint");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"add_flow_particle.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_flow_particle */

int update_flow_particle_list(struct Streampoint *point,
	struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,FE_value step,float time)
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Uses RungeKutta integration to update the position of the given streampoints
using the vector/gradient field and stepsize.  If time is 0 then the previous
point positions are updated adding no new objects.  Otherwise a new pointset is
created with the given timestamp.
==============================================================================*/
{
	FE_value coordinates[3],step_size,total_stepped;
	int change_element,index,number_of_points,previous_change_element,return_code;
	struct GT_pointset *pointset;
	struct Streampoint *point2;
	Triple *particle_positions;

	ENTER(update_flow_particle_list);
	return_code = 1;
	if (time&&point&&coordinate_field&&stream_vector_field&&
		(3>=Computed_field_get_number_of_components(coordinate_field)))
	{
		coordinates[0]=0.0;
		coordinates[1]=0.0;
		coordinates[2]=0.0;
		/* add a new time value and copy the entire graphics primitives */
			/*???DB.  Yet to be done ? */
		/* count number of points */
		point2=point;
		number_of_points=0;
		while (point2)
		{
			number_of_points++;
			point2=point2->next;
		}
		if (ALLOCATE(particle_positions,Triple,number_of_points))
		{
			if ((pointset=CREATE(GT_pointset)(number_of_points,particle_positions,
				(char **)NULL,g_POINT_MARKER,1,g_NO_DATA,(GTDATA *)NULL,(int *)NULL))&&
				GT_OBJECT_ADD(GT_pointset)(point->graphics_object,time,pointset))
			{
				/* copy point positions and then point to new position space */
				point2=point;
				/* streampoint list is created by prefixing new points so the
					particle positions are in the reverse order */
				index=number_of_points-1;
				while (point2)
				{
					(*(particle_positions+index))[0]=
						(*((*(point2->pointlist))+point2->index))[0];
					(*(particle_positions+index))[1]=
						(*((*(point2->pointlist))+point2->index))[1];
					(*(particle_positions+index))[2]=
						(*((*(point2->pointlist))+point2->index))[2];
					point2->pointlist= &(pointset->pointlist);
					point2->index=index;
					index--;
					point2=point2->next;
				}
			}
			else
			{
				DEALLOCATE(particle_positions);
				display_message(ERROR_MESSAGE,
					"gfx_create_flow_particles.  Unable to create new pointset");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
"gfx_create_flow_particles.  Unable to allocate memory for particle positions");
			return_code=0;
		}
	}
	while (return_code&&point&&(point->element))
	{
		total_stepped=0;
		step_size=step;
		previous_change_element=0;
		while (return_code&&(total_stepped<step))
		{
			if (total_stepped+step_size>step)
			{
				step_size=step-total_stepped;
			}
			return_code=update_adaptive_imp_euler(coordinate_field,
				stream_vector_field,/*reverse_track*/0,point->element,point->xi,
				coordinates,&step_size,&total_stepped,&change_element);
			(*((point->pointlist)[point->index]))[0]=coordinates[0];
			(*((point->pointlist)[point->index]))[1]=coordinates[1];
			(*((point->pointlist)[point->index]))[2]=coordinates[2];
			if (return_code&&change_element)
			{
				if (change_element==previous_change_element)
				{
					/* trapped between two elements with opposing directions */
					return_code=0;
				}
				else
				{
					return_code=check_xi_limits(change_element,&(point->element),
						point->xi,coordinate_field);
				}
			}
			previous_change_element=change_element;
		}
		if (total_stepped)
		{
			/* the particle has moved so the graphics object needs updating */
			GT_object_changed(point->graphics_object);
		}
		point=point->next;
		Computed_field_clear_cache(coordinate_field);
		Computed_field_clear_cache(stream_vector_field);
	}
	LEAVE;

	return (return_code);
} /* update_flow_particle_list */

int element_to_streamline(struct FE_element *element,
	void *void_element_to_streamline_data)
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Converts a 3-D element into an array of streamlines.
==============================================================================*/
{
	FE_value initial_xi[3];
	int return_code;
	struct Element_to_streamline_data *element_to_streamline_data;
	struct GT_polyline *polyline;
	struct GT_surface *surface;

	ENTER(element_to_streamline);
	if (element&&(element_to_streamline_data=
		(struct Element_to_streamline_data *)void_element_to_streamline_data))
	{
		/* determine if the element is required */
		if ((element->shape)&&(3==element->shape->dimension))
		{
			/* use local copy of seed_xi since tracking function updates it */
			initial_xi[0]=element_to_streamline_data->seed_xi[0];
			initial_xi[1]=element_to_streamline_data->seed_xi[1];
			initial_xi[2]=element_to_streamline_data->seed_xi[2];
			if (STREAM_LINE==element_to_streamline_data->type)
			{
				if (polyline=create_GT_polyline_streamline_FE_element(element,
					initial_xi,element_to_streamline_data->coordinate_field,
					element_to_streamline_data->stream_vector_field,
					element_to_streamline_data->reverse_track,
					element_to_streamline_data->length,
					element_to_streamline_data->data_type,
					element_to_streamline_data->data_field))
				{
					if (!(return_code=GT_OBJECT_ADD(GT_polyline)(
						element_to_streamline_data->graphics_object,
						element_to_streamline_data->time,polyline)))
					{
						DESTROY(GT_polyline)(&polyline);
					}
				}
				else
				{
					return_code=0;
				}
			}
			else if ((element_to_streamline_data->type == STREAM_RIBBON)||
				(element_to_streamline_data->type == STREAM_EXTRUDED_RECTANGLE)||
				(element_to_streamline_data->type == STREAM_EXTRUDED_ELLIPSE))
			{
				if (surface=create_GT_surface_streamribbon_FE_element(element,
					initial_xi,element_to_streamline_data->coordinate_field,
					element_to_streamline_data->stream_vector_field,
					element_to_streamline_data->reverse_track,
					element_to_streamline_data->length,element_to_streamline_data->width,
					element_to_streamline_data->type,
					element_to_streamline_data->data_type,
					element_to_streamline_data->data_field))
				{
					if (!(return_code=GT_OBJECT_ADD(GT_surface)(
						element_to_streamline_data->graphics_object,
						element_to_streamline_data->time,surface)))
					{
						DESTROY(GT_surface)(&surface);
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"element_to_streamline.  Unknown streamline type");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_to_streamline.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_to_streamline */

int node_to_streamline(struct FE_node *node,
	void *void_node_to_streamline_data)
/*******************************************************************************
LAST MODIFIED : 4 May 1999

DESCRIPTION :
Converts a 3-D element into an array of streamlines.
==============================================================================*/
{
	FE_value initial_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int return_code;
	struct FE_element *element;
	struct FE_field *fe_field;
	struct Node_to_streamline_data *node_to_streamline_data;
	struct GT_polyline *polyline;
	struct GT_surface *surface;

	ENTER(node_to_streamline);

	if (node&&(node_to_streamline_data=
		(struct Node_to_streamline_data *)void_node_to_streamline_data)&&
		(fe_field=node_to_streamline_data->seed_data_field))
	{
		/* determine if the element is required */
		if (FE_field_is_defined_at_node(fe_field, node) &&
			get_FE_nodal_element_xi_value(node, fe_field, 0 /* component_number */,
			0 /*  version */, FE_NODAL_VALUE, &element, initial_xi) && element &&
			(element->shape)&&(3==element->shape->dimension) &&
			((!node_to_streamline_data->seed_element)|| 
			(element==node_to_streamline_data->seed_element)) &&
			((!node_to_streamline_data->element_group)||IS_OBJECT_IN_GROUP(FE_element)(
				element,node_to_streamline_data->element_group)))
		{
			printf("node_to_streamline %p xi %f %f %f\n",
				element, initial_xi[0], initial_xi[1], initial_xi[2]);
			if (STREAM_LINE==node_to_streamline_data->type)
			{
				if (polyline=create_GT_polyline_streamline_FE_element(element,
					initial_xi,node_to_streamline_data->coordinate_field,
					node_to_streamline_data->stream_vector_field,
					node_to_streamline_data->reverse_track,
					node_to_streamline_data->length,
					node_to_streamline_data->data_type,
					node_to_streamline_data->data_field))
				{
					if (!(return_code=GT_OBJECT_ADD(GT_polyline)(
						node_to_streamline_data->graphics_object,
						node_to_streamline_data->time,polyline)))
					{
						DESTROY(GT_polyline)(&polyline);
					}
				}
				else
				{
					return_code=0;
				}
			}
			else if ((node_to_streamline_data->type == STREAM_RIBBON)||
				(node_to_streamline_data->type == STREAM_EXTRUDED_RECTANGLE)||
				(node_to_streamline_data->type == STREAM_EXTRUDED_ELLIPSE))
			{
				if (surface=create_GT_surface_streamribbon_FE_element(element,
					initial_xi,node_to_streamline_data->coordinate_field,
					node_to_streamline_data->stream_vector_field,
					node_to_streamline_data->reverse_track,
					node_to_streamline_data->length,node_to_streamline_data->width,
					node_to_streamline_data->type,
					node_to_streamline_data->data_type,
					node_to_streamline_data->data_field))
				{
					if (!(return_code=GT_OBJECT_ADD(GT_surface)(
						node_to_streamline_data->graphics_object,
						node_to_streamline_data->time,surface)))
					{
						DESTROY(GT_surface)(&surface);
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"node_to_streamline.  Unknown streamline type");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_to_streamline.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_to_streamline */

int element_to_particle(struct FE_element *element,
	void *void_element_to_particle_data)
/*******************************************************************************
LAST MODIFIED : 17 March 1999

DESCRIPTION :
Converts a 3-D element into an array of particles.
==============================================================================*/
{	
	int return_code;
	struct Element_to_particle_data *element_to_particle_data;

	ENTER(element_to_particle);
	if (element&&(element_to_particle_data=
		(struct Element_to_particle_data *)void_element_to_particle_data))
	{
		/* determine if the element is required */
		if ((element->shape)&&(3==element->shape->dimension)&&
			((!element_to_particle_data->element_number)||
			(element_to_particle_data->element_number==element->cm.number)))
		{
			if (add_flow_particle(element_to_particle_data->list, 
				element_to_particle_data->xi,element,
				element_to_particle_data->pointlist,
				element_to_particle_data->index,
				element_to_particle_data->coordinate_field,
				element_to_particle_data->graphics_object))
			{
				element_to_particle_data->index++;
				element_to_particle_data->number_of_particles++;
			}
			return_code=1;
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_to_particle.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_to_particle */

