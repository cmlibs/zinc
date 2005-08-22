/*******************************************************************************
FILE : finite_element_to_streamlines.c

LAST MODIFIED : 23 May 2001

DESCRIPTION :
Functions for calculating streamlines in finite elements.
???DB.  Put into finite_element_to_graphics_object or split
	finite_element_to_graphics_object further ?
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
#include <stdlib.h>
#include <math.h>
#include "computed_field/computed_field.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/matrix_vector.h"
#include "general/random.h"
#include "graphics/graphics_object.h"
#include "user_interface/message.h"

/* SAB Trying to hide the guts of GT_object and its primitives,
	however the stream point stuff currently messes around in the guts
	of a pointset. */
#include "graphics/graphics_object_private.h"

/*
Module types
------------
*/

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

static int calculate_delta_xi(int vector_dimension,FE_value *vector,
	int element_dimension, FE_value *dx_dxi, FE_value *delta_xi)
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Converts a vector <vector> in world space into xi space <delta_xi> by
calculating the inverse of the Jacobian matrix <dxdxi> and multiplying.
==============================================================================*/
{
	double a[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS],
		b[MAXIMUM_ELEMENT_XI_DIMENSIONS], d;
	int i, index[MAXIMUM_ELEMENT_XI_DIMENSIONS], j, k, return_code;

	ENTER(calculate_delta_xi);
	if (vector && dx_dxi && delta_xi)
	{
		if (element_dimension == vector_dimension)
		{
			/* Solve directly */
			for (i = 0 ; i < element_dimension * element_dimension ; i++)
			{
				a[i] = dx_dxi[i];
			}
			for (i = 0 ; i < element_dimension ; i++)
			{
				b[i] = vector[i];
			}
			i = 0;
			if (LU_decompose(element_dimension, a, index, &d,/*singular_tolerance*/1.0e-12) &&
				LU_backsubstitute(element_dimension, a, index, b))
			{
				for (i = 0 ; i < element_dimension ; i++)
				{
					delta_xi[i] = b[i];
				}
			}
			/* Clear out the other delta_xi's until the rest of the code is
				generalised to an arbitrary number of dimensions */
			for ( ; i < MAXIMUM_ELEMENT_XI_DIMENSIONS ; i++)
			{
				delta_xi[i] = 0.0;
			}
			return_code = 1;
		}
		else if (element_dimension < vector_dimension)
		{
			/* Overdetermined system, solve least squares */
			/* A transpose A x = A transpose b */
			for (i = 0 ; i < element_dimension ; i++)
			{
				for (j = 0 ; j < element_dimension ; j++)
				{
					a[i * element_dimension + j] = 0.0;
					for (k = 0 ; k < vector_dimension ; k++)
					{
						a[i * element_dimension + j] += dx_dxi[i + element_dimension + k] * 
							dx_dxi[j + element_dimension + k];
					}
				}
				b[i] = 0.0;
				for (k = 0 ; k < vector_dimension ; k++)
				{
					b[i] += dx_dxi[i + element_dimension + k] * vector[k];
				}
			}
			i = 0;
			if (LU_decompose(element_dimension, a, index, &d,/*singular_tolerance*/1.0e-12) &&
				LU_backsubstitute(element_dimension, a, index, b))
			{
				for (i = 0 ; i < element_dimension ; i++)
				{
					delta_xi[i] = b[i];
				}
			}
			/* Clear out the other delta_xi's until the rest of the code is
				generalised to an arbitrary number of dimensions */
			for ( ; i < MAXIMUM_ELEMENT_XI_DIMENSIONS ; i++)
			{
				delta_xi[i] = 0.0;
			}
			return_code = 1;
		}
		else
		{
			/* Underdetermined system, not implemented */
			display_message(ERROR_MESSAGE, "calculate_delta_xi.  "
				"Underdetermined systems not implemented.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"calculate_delta_xi.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* calculate_delta_xi */

static int update_adaptive_imp_euler(struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	struct FE_region *fe_region,struct FE_element **element,FE_value *xi,
	FE_value time,FE_value *point,FE_value *step_size,
	FE_value *total_stepped, int *keep_tracking)
/*******************************************************************************
LAST MODIFIED : 23 June 2004

DESCRIPTION :
Update the xi coordinates using the <stream_vector_field> with adaptive step
size control and the improved euler method.  The function updates the <total_stepped>.
If <reverse_track> is true, the reverse of vector field is tracked.
==============================================================================*/
{
	int element_dimension,face_number,i,return_code,vector_dimension;
	FE_value deltaxi[MAXIMUM_ELEMENT_XI_DIMENSIONS],deltaxiA[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		deltaxiC[MAXIMUM_ELEMENT_XI_DIMENSIONS], deltaxiD[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		deltaxiE[MAXIMUM_ELEMENT_XI_DIMENSIONS], 
		dxdxi[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS], error, fraction,
		increment_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], local_step_size, tolerance, 
		vector[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS], 
		xiA[MAXIMUM_ELEMENT_XI_DIMENSIONS], xiB[MAXIMUM_ELEMENT_XI_DIMENSIONS], 
		xiC[MAXIMUM_ELEMENT_XI_DIMENSIONS], xiD[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		xiE[MAXIMUM_ELEMENT_XI_DIMENSIONS], xiF[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		xi_face[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	ENTER(update_adaptive_imp_euler);
	/* clear coordinates in case fewer than 3 components */
	element_dimension = get_FE_element_dimension(*element);
	/* It is expected that the coordinate dimension and vector dimension match,
		the vector field may have extra components related to the cross directions */
	vector_dimension = Computed_field_get_number_of_components(coordinate_field);
	point[0]=0.0;
	point[1]=0.0;
	point[2]=0.0;
	tolerance=1.0e-4;
	error=1.0;
	local_step_size = *step_size;
	return_code=Computed_field_evaluate_in_element(coordinate_field,*element,xi,
		time,(struct FE_element *)NULL,point,dxdxi)&&
		Computed_field_evaluate_in_element(stream_vector_field,
		*element,xi,time,(struct FE_element *)NULL,vector,(FE_value *)NULL);
	if (reverse_track)
	{
		for (i = 0 ; i < vector_dimension ; i++)
		{
			vector[i] = -vector[i];
		}
	}
	if (return_code)
	{
		return_code=calculate_delta_xi(vector_dimension,vector,element_dimension,
			dxdxi,deltaxi);
	}
	if (return_code)
	{
		if (local_step_size == 0.0)
		{
			/* This is the first step, set the step_size to make the
			 magnitude of deltaxi 0.01 */
			local_step_size=1.0e-2/sqrt(deltaxi[0]*deltaxi[0]+deltaxi[1]*deltaxi[1]
				+deltaxi[2]*deltaxi[2]);
			
		}
		/* whole step */
		for (i = 0 ; i < element_dimension ; i++)
		{
			xiA[i] = xi[i];
			increment_xi[i] = local_step_size * deltaxi[i];
		}
		return_code = FE_element_xi_increment_within_element(*element, xiA, increment_xi, &fraction,
		   &face_number, xi_face);
		/* If an element boundary is reached then xiA will automatically be on this boundary.
			We want to go to the same location with the more accurate xiB integration so 
			leave the step size alone and adjust the stepsize after xiB */
		if (return_code)
		{
			return_code=Computed_field_evaluate_in_element(coordinate_field, *element, xiA,
				time,(struct FE_element *)NULL,point,dxdxi)&&
				Computed_field_evaluate_in_element(stream_vector_field,
				*element,xiA,time,(struct FE_element *)NULL,vector,(FE_value *)NULL);
			if (reverse_track)
			{
				for (i = 0 ; i < vector_dimension ; i++)
				{
					vector[i] = -vector[i];
				}
			}
		}
		if (return_code)
		{
			return_code=calculate_delta_xi(vector_dimension,vector,element_dimension,
				dxdxi,deltaxiA);
		}
		if (return_code)
		{
			for (i = 0 ; i < element_dimension ; i++)
			{
				xiB[i] = xi[i];
				increment_xi[i] = local_step_size*(deltaxi[i]+deltaxiA[i])/2.0;
			}
			return_code = FE_element_xi_increment_within_element(*element, xiB,
				increment_xi, &fraction, &face_number, xi_face);
		}
		if (face_number != -1)
		{
			if (0.0 == fraction)
			{
				/* Don't go into the loop, so set error and xiF */
				error = 0.0;
				local_step_size = 0.0;
				for (i = 0 ; i < element_dimension ; i++)
				{
					xiF[i] = xiB[i];
				}
			}
			else
			{
				/* Reduce the step to the fraction that was successful plus a bit 
					to make it likely that the more accurate integration below will
					still reach the boundary */
				local_step_size *= (fraction + 0.001);
			}
		}
	}
	while ((error>tolerance)&&return_code)
	{
		for (i = 0 ; i < element_dimension ; i++)
		{
			xiC[i] = xi[i];
			increment_xi[i] = local_step_size*deltaxi[i]/2.0;
		}
		return_code = FE_element_xi_increment_within_element(*element, xiC,
			increment_xi, &fraction, &face_number, xi_face);
		if (return_code)
		{
			return_code=Computed_field_evaluate_in_element(coordinate_field,*element,xiC,
				time,(struct FE_element *)NULL,point,dxdxi)&&
				Computed_field_evaluate_in_element(stream_vector_field,
				*element,xiC,time,(struct FE_element *)NULL,vector,(FE_value *)NULL);
			if (reverse_track)
			{
				for (i = 0 ; i < vector_dimension ; i++)
				{
					vector[i] = -vector[i];
				}
			}
		}
		if (return_code)
		{
			return_code=calculate_delta_xi(vector_dimension,vector,element_dimension,
				dxdxi,deltaxiC);
		}
		if (return_code)
		{
			for (i = 0 ; i < element_dimension ; i++)
			{
				xiD[i] = xi[i];
				increment_xi[i] = local_step_size*(deltaxi[i]+deltaxiC[i])/4.0;
			}
			return_code = FE_element_xi_increment_within_element(*element, xiD,
				increment_xi, &fraction, &face_number, xi_face);
		}
		/* If we are really close to the boundary then the extra bit added on
			above may push us to the boundary in this half step so we should check
			here.  Steps xi->xiD and xiD->xiF are our final steps so check these */
		if (face_number == -1)
		{
			if (return_code)
			{
				return_code=Computed_field_evaluate_in_element(coordinate_field,*element,
					xiD,time,(struct FE_element *)NULL,point,dxdxi)&&
					Computed_field_evaluate_in_element(stream_vector_field,*element,
						xiD,time,(struct FE_element *)NULL,vector,(FE_value *)NULL);
				if (reverse_track)
				{
					for (i = 0 ; i < vector_dimension ; i++)
					{
						vector[i] = -vector[i];
					}
				}
			}
			if (return_code)
			{
				return_code=calculate_delta_xi(vector_dimension,vector,element_dimension,
					dxdxi,deltaxiD);
			}
			if (return_code)
			{
				for (i = 0 ; i < element_dimension ; i++)
				{
					xiE[i] = xiD[i];
					increment_xi[i] = local_step_size*deltaxiD[i]/2.0;
				}
				return_code = FE_element_xi_increment_within_element(*element, xiE,
					increment_xi, &fraction, &face_number, xi_face);
			}
			if (return_code)
			{
				return_code=Computed_field_evaluate_in_element(coordinate_field,*element,
					xiE,time,(struct FE_element *)NULL,point,dxdxi)&&
					Computed_field_evaluate_in_element(stream_vector_field,*element,
						xiE,time,(struct FE_element *)NULL,vector,(FE_value *)NULL);
				if (reverse_track)
				{
					for (i = 0 ; i < vector_dimension ; i++)
					{
						vector[i] = -vector[i];
					}
				}
			}
			if (return_code)
			{
				return_code=calculate_delta_xi(vector_dimension,vector,element_dimension,
					dxdxi,deltaxiE);
			}
			if (return_code)
			{
				for (i = 0 ; i < element_dimension ; i++)
				{
					xiF[i] = xiD[i];
					increment_xi[i] = local_step_size*(deltaxiD[i]+deltaxiE[i])/4.0;
				}
				return_code = FE_element_xi_increment_within_element(*element, xiF,
					increment_xi, &fraction, &face_number, xi_face);
				if (face_number != -1)
				{
					/* Reduce the step size to that which was actually taken */
					local_step_size *= (0.5 + fraction / 2.0);
				}
			}
		}
		else
		{
			/* Reduce the step size to that which was actually taken */
			local_step_size *= fraction / 2.0;
			for (i = 0 ; i < element_dimension ; i++)
			{
				/* Copy the values from the half step result */
				xiF[i] = xiD[i];
			}			
		}
		if (return_code)
		{
			error=sqrt((xiF[0]-xiB[0])*(xiF[0]-xiB[0])+
				(xiF[1]-xiB[1])*(xiF[1]-xiB[1])+(xiF[2]-xiB[2])*(xiF[2]-xiB[2]));
			if ((local_step_size*sqrt(deltaxiC[0]*deltaxiC[0]+
				deltaxiC[1]*deltaxiC[1]+deltaxiC[2]*deltaxiC[2]))<1.0e-3)
			{
				error=0.0;
			}
		}
		if ((error>tolerance)&&return_code)
		{
			local_step_size /= 2.0;
			xiB[0]=xiD[0];
			xiB[1]=xiD[1];
			xiB[2]=xiD[2];
		}
	}
	if (return_code)
	{
		*total_stepped += local_step_size;
		if (face_number != -1)
		{
			/* The last increment should have been the most accurate, if
				it wants to change then change element if we can */
			return_code = FE_element_change_to_adjacent_element(element,
				xiF, (FE_value *)NULL, &face_number, xi_face, fe_region);
			if (face_number == -1)
			{
				/* There is no adjacent element */
				*keep_tracking = 0;
			}
		}
		else
		{
			/* Update the global step_size */
			if (error<tolerance/10.0)
			{
				local_step_size *= 2.0;
			}
			*step_size = local_step_size;
		}
	}
	if (return_code)
	{
		xi[0]=xiF[0];
		xi[1]=xiF[1];
		xi[2]=xiF[2];
	}
	LEAVE;

	return (return_code);
} /* update_adaptive_imp_euler */

static int update_interactive_streampoint(FE_value *point_coordinates,
	struct FE_element **element,struct Computed_field *coordinate_field,
	FE_value *xi, FE_value time, FE_value *translate)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Updates the <GT_pointset> streampoint using the xi coordinates.  If the given
translation in world coordinates is non NULL the xi coordinates are updated
(this is transformed by the local dxdxi derivatives to xi values so only
accurate if small), also ensuring that the element is updated.
==============================================================================*/
{
	FE_value deltaxi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
	   dxdxi[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS],
	   point[MAXIMUM_ELEMENT_XI_DIMENSIONS],xiA[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int return_code;

	ENTER(update_interactive_streampoint);
	/* check the arguments */
	if ((*element)&&(3==get_FE_element_dimension(*element))&&
		coordinate_field&&
		(3>=Computed_field_get_number_of_components(coordinate_field)))
	{
		point[0]=0.0;
		point[1]=0.0;
		point[2]=0.0;
		if ( translate )
		{
			if (Computed_field_evaluate_in_element(coordinate_field,*element,xi,
				time, (struct FE_element *)NULL,point,dxdxi))
			{
				if (!(calculate_delta_xi(/*vector_dimension*/3,translate,
				   /*element_dimension*/3,dxdxi,deltaxi)))
				{
					/* dxdxi tensor is singular, peturb xi by random small amount */
					deltaxi[0] = 0.002*CMGUI_RANDOM(FE_value) - 0.001;
					deltaxi[1] = 0.002*CMGUI_RANDOM(FE_value) - 0.001;
					deltaxi[2] = 0.002*CMGUI_RANDOM(FE_value) - 0.001;
				}
				return_code = FE_element_xi_increment(element, xiA, deltaxi);
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
				time, (struct FE_element *)NULL,point_coordinates,(FE_value *)NULL))
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
		return_code=0;
	}
	LEAVE;

	return(return_code);
} /* update_interactive_streampoint */

static int track_streamline_from_FE_element(struct FE_element **element,
	FE_value *xi,struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	float length,enum Streamline_data_type data_type,
	struct Computed_field *data_field,int *number_of_points,
	Triple **stream_points,Triple **stream_vectors,Triple **stream_normals,
	GTDATA **stream_data, FE_value time, struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 23 June 2004

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

If <fe_region> is not NULL then the function will restrict itself to elements
in that region.
==============================================================================*/
{
	FE_value angle,coordinates[3],cos_angle,curl[3],curl_component,data_value,
		displacement[3],dv_dxi[9],dv_dx[9],dx_dxi[9],dxi_dx[9],magnitude,normal[3],
		normal2[3],old_normal[3],previous_curl_component,previous_total_stepped_A,
		previous_total_stepped_B,sin_angle,step_size,stream_vector_values[9],
		temp,total_stepped,vector[3],vector_magnitude;
	GTDATA *stream_datum,*tmp_stream_data;
	int add_point,allocated_number_of_points,element_dimension,
		i,keep_tracking,number_of_coordinate_components,
		number_of_stream_vector_components,return_code;
	struct FE_element *previous_element_A, *previous_element_B;
	Triple *stream_point,*stream_vector,*stream_normal,*tmp_triples;

	ENTER(track_streamline_from_FE_element);
	/*	step_size of zero indicates first step */
	step_size = 0;
	total_stepped = 0.0;
	if (element&&(*element)&&FE_element_is_top_level(*element, NULL)&&
		(element_dimension = get_FE_element_dimension(*element))&&
		((3 == element_dimension) || (2 == element_dimension)) &&
		xi&&(0.0 <= xi[0])&&(1.0 >= xi[0])&&(0.0 <= xi[1])&&(1.0 >= xi[1])&&
		(0.0 <= xi[2])&&(1.0 >= xi[2])&&coordinate_field&&
		(number_of_coordinate_components=Computed_field_get_number_of_components(coordinate_field))&&
		stream_vector_field&&(number_of_stream_vector_components=
		Computed_field_get_number_of_components(stream_vector_field))
		&& (((3 == number_of_coordinate_components) &&
		((3==number_of_stream_vector_components)||
		(6==number_of_stream_vector_components)||
		(9==number_of_stream_vector_components)))
		|| ((2 == number_of_coordinate_components) &&
		(2==number_of_stream_vector_components)))&&
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
			add_point = 1;
			keep_tracking = 1;
			while (return_code && add_point)
			{
				while (return_code && add_point && (i<allocated_number_of_points))
				{
					/* evaluate the coordinate and stream_vector fields */
					switch (number_of_stream_vector_components)
					{
						case 2:
						{
							stream_vector_values[2] = 0.0;
						} /* no break */
						case 3:
						{
							/* need derivatives of coordinate_field and stream_vector_field
								for calculation of curl. Calculate coordinates first, since
								may be required for evaluating the stream_vector_field. */
							return_code=Computed_field_evaluate_in_element(coordinate_field,
								*element,xi,time,(struct FE_element *)NULL,coordinates,dx_dxi)&&
								Computed_field_evaluate_in_element(stream_vector_field,
								*element,xi,time,(struct FE_element *)NULL,stream_vector_values,
								dv_dxi);
						} break;
						case 6:
						case 9:
						{
							/* derivatives are not required with 6 or 9 components in the
								stream_vector_field, since the lateral direction and ribbon
								normal are taken or calculated directly from it. Want to
								evaluate stream_vector_field first, since in many cases it will
								force a calculation of the coordinate field with derivatives,
								thus saving the coordinates from being recalculated */
							return_code=Computed_field_evaluate_in_element(
							   stream_vector_field,*element,xi,time,(struct FE_element *)NULL,
							   stream_vector_values,(FE_value *)NULL)&&
							   Computed_field_evaluate_in_element(coordinate_field,*element,
							   xi,time,(struct FE_element *)NULL,coordinates,(FE_value *)NULL);
						} break;
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
						add_point = 0;
					}
					if (add_point)
					{
						/* get vector, normal and normal2 - latter are unit vectors normal
							 to the streamline - normal is in lateral direction across sheet,
							 normal2 is normal to the ribbon sheet */
						switch (element_dimension)
						{
							case 2:
							{
								switch (number_of_stream_vector_components)
								{
									case 2:
									{
										/* Normal2 is out of the 2D plane */
										normal2[0] = 0.0;
										normal2[1] = 0.0;
										normal2[2] = 1.0;
										/* get normal = vector (x) normal2 */
										normal[0]=vector[1]*normal2[2]-vector[2]*normal2[1];
										normal[1]=vector[2]*normal2[0]-vector[0]*normal2[2];
										normal[2]=vector[0]*normal2[1]-vector[1]*normal2[0];
										/* make normal unit length */
										magnitude=sqrt(normal[0]*normal[0]+normal[1]*normal[1]+
											normal[2]*normal[2]);
										normal[0] /= magnitude;
										normal[1] /= magnitude;
										normal[2] /= magnitude;
									} break;
									case 3:
									{
										/* Normal2 is the cross of the derivatives */
										normal2[0] = dx_dxi[2] * dx_dxi[5] - dx_dxi[4] * dx_dxi[3];
										normal2[1] = dx_dxi[4] * dx_dxi[1] - dx_dxi[0] * dx_dxi[5];
										normal2[2] = dx_dxi[0] * dx_dxi[3] - dx_dxi[2] * dx_dxi[1];
										/* make normal2 unit length */
										magnitude=sqrt(normal2[0]*normal2[0]+normal2[1]*normal2[1]+
											normal2[2]*normal2[2]);
										normal2[0] /= magnitude;
										normal2[1] /= magnitude;
										normal2[2] /= magnitude;
										/* get normal = vector (x) normal2 */
										normal[0]=vector[1]*normal2[2]-vector[2]*normal2[1];
										normal[1]=vector[2]*normal2[0]-vector[0]*normal2[2];
										normal[2]=vector[0]*normal2[1]-vector[1]*normal2[0];
										/* make normal unit length */
										magnitude=sqrt(normal[0]*normal[0]+normal[1]*normal[1]+
											normal[2]*normal[2]);
										normal[0] /= magnitude;
										normal[1] /= magnitude;
										normal[2] /= magnitude;
									} break;
									default:
									{
										display_message(ERROR_MESSAGE,
											"track_streamline_from_FE_element.  "
											"Incompatible element dimension and vector components.");
										return_code = 0;
									}
								}
							} break;
							case 3:
							{
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
											previous_total_stepped_A = 0.0;
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
												* (total_stepped - previous_total_stepped_A);
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
									default:
									{
										display_message(ERROR_MESSAGE,
											"track_streamline_from_FE_element.  "
											"Incompatible element dimension and vector components.");
										return_code = 0;
									}
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"track_streamline_from_FE_element.  "
									"Unsupported number of element dimension.");
								return_code = 0;
							}
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
									data_field,*element,xi,time,(struct FE_element *)NULL,
									&data_value,(FE_value *)NULL))
								{
									display_message(ERROR_MESSAGE,
										"track_streamline_from_FE_element.  "
										"Error calculating data field");
									return_code = 0;
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
								return_code = 0;
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
						if (keep_tracking && (total_stepped<length))
						{
							/* perform the tracking, changing elements as necessary */
							if (total_stepped + step_size > length)
							{
								step_size = length - total_stepped;
							}
							previous_total_stepped_B = previous_total_stepped_A;
							previous_total_stepped_A = total_stepped;
							previous_element_B = previous_element_A;
							previous_element_A = *element;
							return_code=update_adaptive_imp_euler(coordinate_field,
								stream_vector_field,reverse_track,fe_region,element,xi,
								time,coordinates,&step_size,&total_stepped,&keep_tracking);
							/* If we haven't gone anywhere and are changing back to the previous
								element then we are stuck */
							if ((total_stepped == previous_total_stepped_B) && 
								(*element == previous_element_B))
							{
								printf("track_streamline_from_FE_element.  "
									"trapped between two elements with opposing directions\n");
								add_point = 0;
							}
						}
						else
						{
							add_point = 0;
						}
					}
				}
				if (add_point||(i<allocated_number_of_points))
				{
					if (add_point)
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
						return_code=0;
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

struct GT_polyline *create_GT_polyline_streamline_FE_element(
	struct FE_element *element,FE_value *start_xi,
	struct Computed_field *coordinate_field,
	struct Computed_field *stream_vector_field,int reverse_track,
	float length,enum Streamline_data_type data_type,
	struct Computed_field *data_field, FE_value time,
	struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 23 June 2004

DESCRIPTION :
Creates a <GT_polyline> streamline from the <coordinate_field> following
<stream_vector_field> (with 3, 6 or 9 components) starting in the element and at
the xi coordinates supplied. If <reverse_track> is true, the reverse of the
stream vector is tracked, and the travel_scalar is made negative.
If <fe_region> is not NULL then the function will restrict itself to elements
in that region.
==============================================================================*/
{
	gtDataType gt_data_type;
	GTDATA *stream_data;
	int element_dimension,number_of_stream_points,number_of_coordinate_components,
		number_of_stream_vector_components;
	struct GT_polyline *polyline;
	Triple *stream_points,*stream_normals,*stream_vectors;

	ENTER(create_GT_polyline_streamline_FE_element);
	if (element&&FE_element_is_top_level(element, NULL)&&
		(element_dimension = get_FE_element_dimension(element))&&
		((3 == element_dimension) || (2 == element_dimension))
		&&start_xi&&coordinate_field&&
		(number_of_coordinate_components=Computed_field_get_number_of_components(coordinate_field))&&
		stream_vector_field&&(number_of_stream_vector_components=
		Computed_field_get_number_of_components(stream_vector_field))
		&& (((3 == number_of_coordinate_components) &&
		((3==number_of_stream_vector_components)||
		(6==number_of_stream_vector_components)||
		(9==number_of_stream_vector_components)))
		|| ((2 == number_of_coordinate_components) &&
		(2==number_of_stream_vector_components)))&&
		(0.0<length)&&((data_type!=STREAM_FIELD_SCALAR) || data_field))
	{
		/* track points and normals on streamline, and data if requested */
		if (track_streamline_from_FE_element(&element,start_xi,
			coordinate_field,stream_vector_field,reverse_track,length,
			data_type,data_field,&number_of_stream_points,&stream_points,
				&stream_vectors,&stream_normals,&stream_data,time,fe_region))
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
				if (!(polyline=CREATE(GT_polyline)(g_PLAIN,/*line_width=default*/0,
					number_of_stream_points,stream_points,/* normals */(Triple *)NULL,
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
	enum Streamline_data_type data_type,struct Computed_field *data_field,
	FE_value time, struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 23 June 2004

DESCRIPTION :
Creates a <GT_surface> streamline from the <coordinate_field> following
<stream_vector_field> (with 3, 6 or 9 components) starting in the element and at
the xi coordinates supplied. If <reverse_track> is true, the reverse of the
stream vector is tracked, and the travel_scalar is made negative.
If <fe_region> is not NULL then the function will restrict itself to elements
in that region.
==============================================================================*/
{
	float cosw,magnitude,sinw,thickness;
	gtDataType gt_data_type;
	GTDATA *data,*datum,*stream_data,stream_datum;
	int d,element_dimension,i,number_of_stream_points,number_of_coordinate_components,
		number_of_stream_vector_components,surface_points_per_step;
	struct GT_surface *surface;
	Triple cross_thickness,cross_width,*normal,*normalpoints,*point,*points,stream_cross,
		stream_normal,*stream_normals,stream_point,*stream_points,
		stream_unit_vector,stream_vector,*stream_vectors;

	ENTER(create_GT_surface_streamribbon_FE_element);
	if (element&&FE_element_is_top_level(element, NULL)&&
		(element_dimension = get_FE_element_dimension(element))&&
		((3 == element_dimension) || (2 == element_dimension))
		&&start_xi&&coordinate_field&&
		(number_of_coordinate_components=Computed_field_get_number_of_components(coordinate_field))&&
		stream_vector_field&&(number_of_stream_vector_components=
		Computed_field_get_number_of_components(stream_vector_field))
		&& (((3 == number_of_coordinate_components) &&
		((3==number_of_stream_vector_components)||
		(6==number_of_stream_vector_components)||
		(9==number_of_stream_vector_components)))
		|| ((2 == number_of_coordinate_components) &&
		(2==number_of_stream_vector_components)))&&
		(0.0<length)&&((data_type!=STREAM_FIELD_SCALAR) || data_field))
	{
		if (type == STREAM_EXTRUDED_CIRCLE)
		{
			thickness = width;
		}
		else
		{
			thickness = 0.2 * width;
		}
		/* track points and normals on streamline, and data if requested */
		if (track_streamline_from_FE_element(&element,start_xi,
			coordinate_field,stream_vector_field,reverse_track,length,
			data_type,data_field,&number_of_stream_points,&stream_points,
				&stream_vectors,&stream_normals,&stream_data,time,fe_region))
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
					case STREAM_EXTRUDED_CIRCLE:
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
						normalpoints, /*tangentpoints*/(Triple *)NULL,
						/*texturepoints*/(Triple *)NULL,gt_data_type,data))
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
								case STREAM_EXTRUDED_CIRCLE:
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
	struct Computed_field *coordinate_field,float length,FE_value *xi,
	FE_value time)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

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
	if (element&&(3==get_FE_element_dimension(element))&&(length>0.0)&&
		coordinate_field&&
		(3>=Computed_field_get_number_of_components(coordinate_field)))
	{
		coordinates[0]=0.0;
		coordinates[1]=0.0;
		coordinates[2]=0.0;
		if (Computed_field_evaluate_in_element(coordinate_field,element,xi,
			time,(struct FE_element *)NULL,coordinates,(FE_value *)NULL))
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
				&element, coordinate_field, xi, /*time*/0, (FE_value *)NULL );
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
	struct Computed_field *stream_vector_field,FE_value step, FE_value time)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Uses RungeKutta integration to update the position of the given streampoints
using the vector/gradient field and stepsize.  If time is 0 then the previous
point positions are updated adding no new objects.  Otherwise a new pointset is
created with the given timestamp.
==============================================================================*/
{
	FE_value coordinates[3],step_size,total_stepped;
	int keep_tracking,index,number_of_points,return_code;
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
		keep_tracking=1;
		while (return_code&&(total_stepped<step))
		{
			if (total_stepped+step_size>step)
			{
				step_size=step-total_stepped;
			}
			return_code=update_adaptive_imp_euler(coordinate_field,
				stream_vector_field,/*reverse_track*/0,(struct FE_region *)NULL,
				&(point->element),point->xi,time,coordinates,&step_size,
				&total_stepped,&keep_tracking);
			(*((point->pointlist)[point->index]))[0]=coordinates[0];
			(*((point->pointlist)[point->index]))[1]=coordinates[1];
			(*((point->pointlist)[point->index]))[2]=coordinates[2];
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
		if (3==get_FE_element_dimension(element))
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
					element_to_streamline_data->data_field,
					element_to_streamline_data->time,
					(struct FE_region *)NULL))
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
				(element_to_streamline_data->type == STREAM_EXTRUDED_ELLIPSE)||
				(element_to_streamline_data->type == STREAM_EXTRUDED_CIRCLE))
			{
				if (surface=create_GT_surface_streamribbon_FE_element(element,
					initial_xi,element_to_streamline_data->coordinate_field,
					element_to_streamline_data->stream_vector_field,
					element_to_streamline_data->reverse_track,
					element_to_streamline_data->length,element_to_streamline_data->width,
					element_to_streamline_data->type,
					element_to_streamline_data->data_type,
					element_to_streamline_data->data_field,
					element_to_streamline_data->time,
					(struct FE_region *)NULL))
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
			(3==get_FE_element_dimension(element)) &&
			((!node_to_streamline_data->seed_element)|| 
				(element==node_to_streamline_data->seed_element)) &&
			((!node_to_streamline_data->fe_region) ||
			FE_region_contains_FE_element(node_to_streamline_data->fe_region,
			element)))
		{
#if defined (DEBUG)
			/*???debug*/
			printf("node_to_streamline: element %d, xi %f %f %f\n",
				element->cm.number, initial_xi[0], initial_xi[1], initial_xi[2]);
#endif /* defined (DEBUG) */
			if (STREAM_LINE==node_to_streamline_data->type)
			{
				if (polyline=create_GT_polyline_streamline_FE_element(element,
					initial_xi,node_to_streamline_data->coordinate_field,
					node_to_streamline_data->stream_vector_field,
					node_to_streamline_data->reverse_track,
					node_to_streamline_data->length,
					node_to_streamline_data->data_type,
					node_to_streamline_data->data_field,
					node_to_streamline_data->time,
					(struct FE_region *)NULL))
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
				(node_to_streamline_data->type == STREAM_EXTRUDED_ELLIPSE)||
				(node_to_streamline_data->type == STREAM_EXTRUDED_CIRCLE))
			{
				if (surface=create_GT_surface_streamribbon_FE_element(element,
					initial_xi,node_to_streamline_data->coordinate_field,
					node_to_streamline_data->stream_vector_field,
					node_to_streamline_data->reverse_track,
					node_to_streamline_data->length,node_to_streamline_data->width,
					node_to_streamline_data->type,
					node_to_streamline_data->data_type,
					node_to_streamline_data->data_field,
					node_to_streamline_data->time,
					(struct FE_region *)NULL))
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
		if ((3==get_FE_element_dimension(element))&&
			((!element_to_particle_data->element_number)||
			(element_to_particle_data->element_number==FE_element_get_cm_number(element))))
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
