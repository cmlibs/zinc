/*******************************************************************************
FILE : projection.c

LAST MODIFIED : 2 September 2001

DESCRIPTION :
???DB.  Started as mapping.c in emap
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
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/postscript.h"
#include "graphics/spectrum.h"
#include "projection/projection.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 5 November 1995

DESCRIPTION :
==============================================================================*/
{
	char *background_drawing_colour;
	char *boundary_colour;
	char *contour_colour;
	char *fibre_colour;
	char *landmark_colour;
	char *node_marker_colour;
	char *node_text_colour;
	char *spectrum_marker_colour;
	char *spectrum_text_colour;
	Font drawing_font;
	int pixels_between_contour_values;
} Projection_settings;

/*
Module constants
----------------
*/
#define MAX_SPECTRUM_COLOURS 256

/*
Module functions
----------------
*/
static int add_element_nodes_to_list(struct FE_element *element,
	void *void_node_list)
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Add the <element>'s nodes to the node list if they are not already in it.
???DB.  Does not work correctly unless the fields are defined for the element.
==============================================================================*/
{
	int i,return_code;
	struct FE_node **node;
	struct LIST(FE_node) *node_list;

	ENTER(add_element_nodes_to_list);
	/* check arguments */
	if (element&&(node_list=(struct LIST(FE_node) *)void_node_list))
	{
		return_code=1;
		/* does not look at parents */
		if (element->information)
		{
			node=element->information->nodes;
			for (i=element->information->number_of_nodes;i>0;i--)
			{
				if (!FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(
					get_FE_node_cm_node_identifier(*node),node_list))
				{
					ADD_OBJECT_TO_LIST(FE_node)(*node,node_list);
				}
				node++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_element_nodes_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_element_nodes_to_list */

static int project_point(enum Projection_type projection_type,
	enum Coordinate_system_type coordinate_system_type,float coordinate_1,
	float coordinate_2,float coordinate_3,float *project_1,float *project_2,
	float *project_3,float *jacobian)
/*******************************************************************************
LAST MODIFIED : 18 December 1995

DESCRIPTION :
The <projection_type> of the point in the given <coordinate_system> is
calculated.  If <jacobian> is not NULL it should be a 9 element array and the
Jacobian for the projection will be calculated.
==============================================================================*/
{
	float jacobian_1[9],jacobian_2[9],jacobian_3[6],lambda,mu,
		projection_jacobian[4],r,temp_float,theta,x,y,z;
	int return_code;

	ENTER(project_point);
	/* check arguments */
	if (project_1&&project_2&&project_3)
	{
		if (jacobian)
		{
			switch (projection_type)
			{
				case CYLINDRICAL_PROJECTION:
				{
					switch (coordinate_system_type)
					{
						case CYLINDRICAL_POLAR:
						{
							r=coordinate_1;
							theta=coordinate_2;
							z=coordinate_3;
							jacobian[0]=0;
							jacobian[1]=1;
							jacobian[2]=0;
							jacobian[3]=0;
							jacobian[4]=0;
							jacobian[5]=1;
							jacobian[6]=1;
							jacobian[7]=0;
							jacobian[8]=0;
							return_code=1;
						} break;
						case PROLATE_SPHEROIDAL:
						{
							prolate_spheroidal_to_cartesian(coordinate_1,coordinate_2,
								coordinate_3,1.,&x,&y,&temp_float,jacobian_1);
							/*???DB.  NB permutation */
							return_code=cartesian_to_cylindrical_polar(y,temp_float,x,&r,
								&theta,&z,jacobian_2);
							/* make apex at bottom */
							z= -z;
							jacobian[0]=jacobian_1[3]*jacobian_2[3]+
								jacobian_1[6]*jacobian_2[4]+jacobian_1[0]*jacobian_2[5];
							jacobian[1]=jacobian_1[4]*jacobian_2[3]+
								jacobian_1[7]*jacobian_2[4]+jacobian_1[1]*jacobian_2[5];
							jacobian[2]=jacobian_1[5]*jacobian_2[3]+
								jacobian_1[8]*jacobian_2[4]+jacobian_1[2]*jacobian_2[5];
							jacobian[3]= -(jacobian_1[3]*jacobian_2[6]+
								jacobian_1[6]*jacobian_2[7]+jacobian_1[0]*jacobian_2[8]);
							jacobian[4]= -(jacobian_1[4]*jacobian_2[6]+
								jacobian_1[7]*jacobian_2[7]+jacobian_1[1]*jacobian_2[8]);
							jacobian[5]= -(jacobian_1[5]*jacobian_2[6]+
								jacobian_1[8]*jacobian_2[7]+jacobian_1[2]*jacobian_2[8]);
							jacobian[6]=jacobian_1[3]*jacobian_2[0]+
								jacobian_1[6]*jacobian_2[1]+jacobian_1[0]*jacobian_2[2];
							jacobian[7]=jacobian_1[4]*jacobian_2[0]+
								jacobian_1[7]*jacobian_2[1]+jacobian_1[1]*jacobian_2[2];
							jacobian[8]=jacobian_1[5]*jacobian_2[0]+
								jacobian_1[8]*jacobian_2[1]+jacobian_1[2]*jacobian_2[2];
						} break;
						case RECTANGULAR_CARTESIAN:
						{
							return_code=cartesian_to_cylindrical_polar(coordinate_1,
								coordinate_2,coordinate_3,&r,&theta,&z,jacobian_1);
							jacobian[0]=jacobian_1[3];
							jacobian[1]=jacobian_1[4];
							jacobian[2]=jacobian_1[5];
							jacobian[3]=jacobian_1[6];
							jacobian[4]=jacobian_1[7];
							jacobian[5]=jacobian_1[8];
							jacobian[6]=jacobian_1[0];
							jacobian[7]=jacobian_1[1];
							jacobian[8]=jacobian_1[2];
						} break;
						case SPHERICAL_POLAR:
						{
							spherical_polar_to_cartesian(coordinate_1,coordinate_2,
								coordinate_3,&x,&y,&temp_float,jacobian_1);
							return_code=cartesian_to_cylindrical_polar(x,y,temp_float,&r,
								&theta,&z,jacobian_2);
							jacobian[0]=jacobian_1[0]*jacobian_2[3]+
								jacobian_1[3]*jacobian_2[4]+jacobian_1[6]*jacobian_2[5];
							jacobian[1]=jacobian_1[1]*jacobian_2[3]+
								jacobian_1[4]*jacobian_2[4]+jacobian_1[7]*jacobian_2[5];
							jacobian[2]=jacobian_1[2]*jacobian_2[3]+
								jacobian_1[5]*jacobian_2[4]+jacobian_1[8]*jacobian_2[5];
							jacobian[3]=jacobian_1[0]*jacobian_2[6]+
								jacobian_1[3]*jacobian_2[7]+jacobian_1[6]*jacobian_2[8];
							jacobian[4]=jacobian_1[1]*jacobian_2[6]+
								jacobian_1[4]*jacobian_2[7]+jacobian_1[7]*jacobian_2[8];
							jacobian[5]=jacobian_1[2]*jacobian_2[6]+
								jacobian_1[5]*jacobian_2[7]+jacobian_1[8]*jacobian_2[8];
							jacobian[6]=jacobian_1[0]*jacobian_2[0]+
								jacobian_1[3]*jacobian_2[1]+jacobian_1[6]*jacobian_2[2];
							jacobian[7]=jacobian_1[1]*jacobian_2[0]+
								jacobian_1[4]*jacobian_2[1]+jacobian_1[7]*jacobian_2[2];
							jacobian[8]=jacobian_1[2]*jacobian_2[0]+
								jacobian_1[5]*jacobian_2[1]+jacobian_1[8]*jacobian_2[2];
						} break;
						case FIBRE:
						case OBLATE_SPHEROIDAL:
						{
							display_message(ERROR_MESSAGE,
								"project_point.  Invalid coordinate system");
							return_code=0;
						} break;
					}
					if (return_code)
					{
						*(project_1)=theta;
						*(project_2)=z;
						*(project_3)=r;
					}
				} break;
				case HAMMER_PROJECTION:
				case POLAR_PROJECTION:
				{
					switch (coordinate_system_type)
					{
						case CYLINDRICAL_POLAR:
						{
							cylindrical_polar_to_cartesian(coordinate_1,coordinate_2,
								coordinate_3,&x,&y,&z,jacobian_1);
							return_code=cartesian_to_prolate_spheroidal(x,y,z,1.,&lambda,&mu,
								&theta,jacobian_2);
							jacobian_3[0]=jacobian_1[0]*jacobian_2[3]+
								jacobian_1[3]*jacobian_2[4]+jacobian_1[6]*jacobian_2[5];
							jacobian_3[1]=jacobian_1[1]*jacobian_2[3]+
								jacobian_1[4]*jacobian_2[4]+jacobian_1[7]*jacobian_2[5];
							jacobian_3[2]=jacobian_1[2]*jacobian_2[3]+
								jacobian_1[5]*jacobian_2[4]+jacobian_1[8]*jacobian_2[5];
							jacobian_3[3]=jacobian_1[0]*jacobian_2[6]+
								jacobian_1[3]*jacobian_2[7]+jacobian_1[6]*jacobian_2[8];
							jacobian_3[4]=jacobian_1[1]*jacobian_2[6]+
								jacobian_1[4]*jacobian_2[7]+jacobian_1[7]*jacobian_2[8];
							jacobian_3[5]=jacobian_1[2]*jacobian_2[6]+
								jacobian_1[5]*jacobian_2[7]+jacobian_1[8]*jacobian_2[8];
							jacobian[6]=jacobian_1[0]*jacobian_2[0]+
								jacobian_1[3]*jacobian_2[1]+jacobian_1[6]*jacobian_2[2];
							jacobian[7]=jacobian_1[1]*jacobian_2[0]+
								jacobian_1[4]*jacobian_2[1]+jacobian_1[7]*jacobian_2[2];
							jacobian[8]=jacobian_1[2]*jacobian_2[0]+
								jacobian_1[5]*jacobian_2[1]+jacobian_1[8]*jacobian_2[2];
						} break;
						case PROLATE_SPHEROIDAL:
						{
							lambda=coordinate_1;
							mu=coordinate_2;
							theta=coordinate_3;
							jacobian_3[0]=0;
							jacobian_3[1]=1;
							jacobian_3[2]=0;
							jacobian_3[3]=0;
							jacobian_3[4]=0;
							jacobian_3[5]=1;
							jacobian[6]=1;
							jacobian[7]=0;
							jacobian[8]=0;
							return_code=1;
						} break;
						case RECTANGULAR_CARTESIAN:
						{
							return_code=cartesian_to_prolate_spheroidal(coordinate_1,
								coordinate_2,coordinate_3,1.,&lambda,&mu,&theta,jacobian_1);
							jacobian_3[0]=jacobian_1[3];
							jacobian_3[1]=jacobian_1[4];
							jacobian_3[2]=jacobian_1[5];
							jacobian_3[3]=jacobian_1[6];
							jacobian_3[4]=jacobian_1[7];
							jacobian_3[5]=jacobian_1[8];
							jacobian[6]=jacobian_1[0];
							jacobian[7]=jacobian_1[1];
							jacobian[8]=jacobian_1[2];
						} break;
						case SPHERICAL_POLAR:
						{
							spherical_polar_to_cartesian(coordinate_1,coordinate_2,
								coordinate_3,&x,&y,&z,jacobian_1);
							return_code=cartesian_to_prolate_spheroidal(x,y,z,1.,&lambda,&mu,
								&theta,jacobian_2);
							jacobian_3[0]=jacobian_1[0]*jacobian_2[3]+
								jacobian_1[3]*jacobian_2[4]+jacobian_1[6]*jacobian_2[5];
							jacobian_3[1]=jacobian_1[1]*jacobian_2[3]+
								jacobian_1[4]*jacobian_2[4]+jacobian_1[7]*jacobian_2[5];
							jacobian_3[2]=jacobian_1[2]*jacobian_2[3]+
								jacobian_1[5]*jacobian_2[4]+jacobian_1[8]*jacobian_2[5];
							jacobian_3[3]=jacobian_1[0]*jacobian_2[6]+
								jacobian_1[3]*jacobian_2[7]+jacobian_1[6]*jacobian_2[8];
							jacobian_3[4]=jacobian_1[1]*jacobian_2[6]+
								jacobian_1[4]*jacobian_2[7]+jacobian_1[7]*jacobian_2[8];
							jacobian_3[5]=jacobian_1[2]*jacobian_2[6]+
								jacobian_1[5]*jacobian_2[7]+jacobian_1[8]*jacobian_2[8];
							jacobian[6]=jacobian_1[0]*jacobian_2[0]+
								jacobian_1[3]*jacobian_2[1]+jacobian_1[6]*jacobian_2[2];
							jacobian[7]=jacobian_1[1]*jacobian_2[0]+
								jacobian_1[4]*jacobian_2[1]+jacobian_1[7]*jacobian_2[2];
							jacobian[8]=jacobian_1[2]*jacobian_2[0]+
								jacobian_1[5]*jacobian_2[1]+jacobian_1[8]*jacobian_2[2];
						} break;
						case FIBRE:
						case OBLATE_SPHEROIDAL:
						{
							display_message(ERROR_MESSAGE,
								"project_point.  Invalid coordinate system");
							return_code=0;
						} break;
					}
					if (return_code)
					{
						*(project_3)=lambda;
						if (HAMMER_PROJECTION==projection_type)
						{
							return_code=Hammer_projection(mu,theta,project_1,project_2,
								projection_jacobian);
						}
						else
						{
							return_code=polar_projection(mu,theta,project_1,project_2,
								projection_jacobian);
						}
						jacobian[0]=projection_jacobian[0]*jacobian_3[0]+
							projection_jacobian[1]*jacobian_3[3];
						jacobian[1]=projection_jacobian[0]*jacobian_3[1]+
							projection_jacobian[1]*jacobian_3[4];
						jacobian[2]=projection_jacobian[0]*jacobian_3[2]+
							projection_jacobian[1]*jacobian_3[5];
						jacobian[3]=projection_jacobian[2]*jacobian_3[0]+
							projection_jacobian[3]*jacobian_3[3];
						jacobian[4]=projection_jacobian[2]*jacobian_3[1]+
							projection_jacobian[3]*jacobian_3[4];
						jacobian[5]=projection_jacobian[2]*jacobian_3[2]+
							projection_jacobian[3]*jacobian_3[5];
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"project_point.  Unknown projection");
					return_code=0;
				} break;
			}
		}
		else
		{
			switch (projection_type)
			{
				case CYLINDRICAL_PROJECTION:
				{
					switch (coordinate_system_type)
					{
						case CYLINDRICAL_POLAR:
						{
							r=coordinate_1;
							theta=coordinate_2;
							z=coordinate_3;
							return_code=1;
						} break;
						case PROLATE_SPHEROIDAL:
						{
							prolate_spheroidal_to_cartesian(coordinate_1,coordinate_2,
								coordinate_3,1.,&x,&y,&temp_float,(float *)NULL);
							/*???DB.  NB permuatation */
							return_code=cartesian_to_cylindrical_polar(y,temp_float,x,&r,
								&theta,&z,(float *)NULL);
							/* make apex at bottom */
							z= -z;
						} break;
						case RECTANGULAR_CARTESIAN:
						{
							return_code=cartesian_to_cylindrical_polar(coordinate_1,
								coordinate_2,coordinate_3,&r,&theta,&z,(float *)NULL);
						} break;
						case SPHERICAL_POLAR:
						{
							spherical_polar_to_cartesian(coordinate_1,coordinate_2,
								coordinate_3,&x,&y,&temp_float,(float *)NULL);
							return_code=cartesian_to_cylindrical_polar(x,y,temp_float,&r,
								&theta,&z,(float *)NULL);
						} break;
						case FIBRE:
						case OBLATE_SPHEROIDAL:
						{
							display_message(ERROR_MESSAGE,
								"project_point.  Invalid coordinate system");
							return_code=0;
						} break;
					}
					if (return_code)
					{
						*(project_1)=theta;
						*(project_2)=z;
						*(project_3)=r;
					}
				} break;
				case HAMMER_PROJECTION:
				case POLAR_PROJECTION:
				{
					switch (coordinate_system_type)
					{
						case CYLINDRICAL_POLAR:
						{
							cylindrical_polar_to_cartesian(coordinate_1,coordinate_2,
								coordinate_3,&x,&y,&z,(float *)NULL);
							return_code=cartesian_to_prolate_spheroidal(x,y,z,1.,&lambda,&mu,
								&theta,(float *)NULL);
						} break;
						case PROLATE_SPHEROIDAL:
						{
							lambda=coordinate_1;
							mu=coordinate_2;
							theta=coordinate_3;
							return_code=1;
						} break;
						case RECTANGULAR_CARTESIAN:
						{
							return_code=cartesian_to_prolate_spheroidal(coordinate_1,
								coordinate_2,coordinate_3,1.,&lambda,&mu,&theta,(float *)NULL);
						} break;
						case SPHERICAL_POLAR:
						{
							spherical_polar_to_cartesian(coordinate_1,coordinate_2,
								coordinate_3,&x,&y,&z,(float *)NULL);
							return_code=cartesian_to_prolate_spheroidal(x,y,z,1.,&lambda,&mu,
								&theta,(float *)NULL);
						} break;
						case FIBRE:
						case OBLATE_SPHEROIDAL:
						{
							display_message(ERROR_MESSAGE,
								"project_point.  Invalid coordinate system");
							return_code=0;
						} break;
					}
					if (return_code)
					{
						*(project_3)=lambda;
						if (HAMMER_PROJECTION==projection_type)
						{
							return_code=Hammer_projection(mu,theta,project_1,project_2,
								(float *)NULL);
						}
						else
						{
							return_code=polar_projection(mu,theta,project_1,project_2,
								(float *)NULL);
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"project_point.  Unknown projection");
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"project_point.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* project_point */

struct Project_node_data
{
	char first;
	enum Projection_type projection_type;
	float max_x,max_y,min_x,min_y,*node_x,*node_y,*node_depth;
	struct FE_field *coordinate_field;
	struct FE_node **node;
}; /* struct Project_node_data */

static int project_node(struct FE_node *node,void *void_project_node_data)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Project the node position.
==============================================================================*/
{
	FE_value values[3];
	int return_code;
	struct Project_node_data *project_node_data;

	ENTER(project_node);
	/* check arguments */
	if (node&&
		(project_node_data=(struct Project_node_data *)void_project_node_data)&&
		(project_node_data->coordinate_field)&&
		(3==get_FE_field_number_of_components(project_node_data->coordinate_field)))
	{
		if (calculate_FE_field(project_node_data->coordinate_field,-1,node,
				 (struct FE_element *)NULL,(FE_value *)NULL,/*time*/0,values))
		{
			return_code=project_point(project_node_data->projection_type,
				get_coordinate_system_type(
					get_FE_field_coordinate_system(project_node_data->coordinate_field)),
				(float)values[0],(float)values[1],(float)values[2],
				project_node_data->node_x,project_node_data->node_y,
				project_node_data->node_depth,(float *)NULL);
			if (return_code)
			{
				if (project_node_data->first)
				{
					project_node_data->min_x= *(project_node_data->node_x);
					project_node_data->max_x= *(project_node_data->node_x);
					project_node_data->min_y= *(project_node_data->node_y);
					project_node_data->max_y= *(project_node_data->node_y);
					project_node_data->first=0;
				}
				else
				{
					if (*(project_node_data->node_x)<project_node_data->min_x)
					{
						project_node_data->min_x= *(project_node_data->node_x);
					}
					else
					{
						if (*(project_node_data->node_x)>project_node_data->max_x)
						{
							project_node_data->max_x= *(project_node_data->node_x);
						}
					}
					if (*(project_node_data->node_y)<project_node_data->min_y)
					{
						project_node_data->min_y= *(project_node_data->node_y);
					}
					else
					{
						if (*(project_node_data->node_y)>project_node_data->max_y)
						{
							project_node_data->max_y= *(project_node_data->node_y);
						}
					}
				}
				*(project_node_data->node)=node;
				(project_node_data->node)++;
				(project_node_data->node_x)++;
				(project_node_data->node_y)++;
				(project_node_data->node_depth)++;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"project_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* project_node */

static int project_xi_point(FE_value *xi,
	struct FE_element_field_values *coordinate_element_field_values,
	enum Projection_type projection_type,float start_x,float start_y,
	float stretch_x,float stretch_y,float min_x,float min_y,float *screen_x,
	float *screen_y,float *depth,float *derivative_xi)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
==============================================================================*/
{
	FE_value coordinate[3],coordinate_jacobian[9];
	float project_1,project_2,projection_jacobian[9];
	int return_code;

	ENTER(project_xi_point);
	if (derivative_xi)
	{
		/* calculate coordinate field */
		if (calculate_FE_element_field(-1,coordinate_element_field_values,xi,
			coordinate,coordinate_jacobian))
		{
			/* transform to projection coordinates */
			if (project_point(projection_type,get_coordinate_system_type(
				get_FE_field_coordinate_system(coordinate_element_field_values->field)),
				coordinate[0],coordinate[1],coordinate[2],&project_1,&project_2,depth,
				projection_jacobian))
			{
				/* scale to screen coordinates */
				*screen_x=start_x+(project_1-min_x)*stretch_x;
				*screen_y=start_y-(project_2-min_y)*stretch_y;
				if (2==coordinate_element_field_values->element->shape->dimension)
				{
					derivative_xi[0]=stretch_x*(
						projection_jacobian[0]*coordinate_jacobian[0]+
						projection_jacobian[1]*coordinate_jacobian[2]+
						projection_jacobian[2]*coordinate_jacobian[4]);
					derivative_xi[1]=stretch_x*(
						projection_jacobian[0]*coordinate_jacobian[1]+
						projection_jacobian[1]*coordinate_jacobian[3]+
						projection_jacobian[2]*coordinate_jacobian[5]);
					derivative_xi[2]= -stretch_y*(
						projection_jacobian[3]*coordinate_jacobian[0]+
						projection_jacobian[4]*coordinate_jacobian[2]+
						projection_jacobian[5]*coordinate_jacobian[4]);
					derivative_xi[3]= -stretch_y*(
						projection_jacobian[3]*coordinate_jacobian[1]+
						projection_jacobian[4]*coordinate_jacobian[3]+
						projection_jacobian[5]*coordinate_jacobian[5]);
				}
				else
				{
					derivative_xi[0]=stretch_x*(
						projection_jacobian[0]*coordinate_jacobian[0]+
						projection_jacobian[1]*coordinate_jacobian[3]+
						projection_jacobian[2]*coordinate_jacobian[6]);
					derivative_xi[1]=stretch_x*(
						projection_jacobian[0]*coordinate_jacobian[1]+
						projection_jacobian[1]*coordinate_jacobian[4]+
						projection_jacobian[2]*coordinate_jacobian[7]);
					derivative_xi[2]= -stretch_y*(
						projection_jacobian[3]*coordinate_jacobian[0]+
						projection_jacobian[4]*coordinate_jacobian[3]+
						projection_jacobian[5]*coordinate_jacobian[6]);
					derivative_xi[3]= -stretch_y*(
						projection_jacobian[3]*coordinate_jacobian[1]+
						projection_jacobian[4]*coordinate_jacobian[4]+
						projection_jacobian[5]*coordinate_jacobian[7]);
				}
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"project_xi_point.  Error projecting");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"project_xi_point.  Error calculating coordinate field");
			return_code=0;
		}
	}
	else
	{
		/* calculate coordinate field */
		if (calculate_FE_element_field(-1,coordinate_element_field_values,xi,
			coordinate,(FE_value *)NULL))
		{
			/* transform to projection coordinates */
			if (project_point(projection_type,get_coordinate_system_type(
				get_FE_field_coordinate_system(coordinate_element_field_values->field)),
				coordinate[0],coordinate[1],coordinate[2],&project_1,&project_2,
				depth,(float *)NULL))
			{
				/* scale to screen coordinates */
				*screen_x=start_x+(project_1-min_x)*stretch_x;
				*screen_y=start_y-(project_2-min_y)*stretch_y;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"project_xi_point.  Error projecting");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"project_xi_point.  Error calculating coordinate field");
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* project_xi_point */

static int inverse_project(FE_value *xi,
	struct FE_element_field_values *coordinate_element_field_values,
	enum Projection_type projection_type,float start_x,float start_y,
	float stretch_x,float stretch_y,float min_x,float min_y,float x,float y,
	float *depth)
/*******************************************************************************
LAST MODIFIED : 4 October 1998

DESCRIPTION :
==============================================================================*/
{
	const float maximum_step_size=100,tolerance_newton_raphson=1.e-4;
	const int number_newton_raphson=5;
	float derivative_xi[4],det,error_x,error_y,screen_error,screen_tolerance,
		screen_x,screen_y,step_xi_1,step_xi_2,xi_error,xi_tolerance;
	int i,return_code;

	ENTER(inverse_project);
	i=number_newton_raphson;
	xi_tolerance=tolerance_newton_raphson*tolerance_newton_raphson;
	screen_tolerance=((fabs(x)+1)*(fabs(x)+1)+(fabs(y)+1)*(fabs(y)+1))*
		xi_tolerance;
	if (project_xi_point(xi,coordinate_element_field_values,projection_type,
		start_x,start_y,stretch_x,stretch_y,min_x,min_y,&screen_x,&screen_y,depth,
		derivative_xi))
	{
		/* calculate error */
		error_x=screen_x-x;
		error_y=screen_y-y;
		screen_error=error_x*error_x+error_y*error_y;
		/* calculate Newton-Raphson step */
		if (0!=(det=derivative_xi[0]*derivative_xi[3]-
			derivative_xi[1]*derivative_xi[2]))
		{
			step_xi_1=(error_x*derivative_xi[3]-error_y*derivative_xi[1])/det;
			step_xi_2=(error_y*derivative_xi[0]-error_x*derivative_xi[2])/det;
			xi_error=step_xi_1*step_xi_1+step_xi_2*step_xi_2;
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
			"inverse_project.  Error projecting xi point");
		return_code=0;
	}
	while (return_code&&((screen_error>screen_tolerance)||
		(xi_error>xi_tolerance))&&(i>0)&&(xi_error<maximum_step_size))
	{
		/* make Newton-Raphson step */
		xi[0] -= step_xi_1;
		xi[1] -= step_xi_2;
		if (project_xi_point(xi,coordinate_element_field_values,projection_type,
			start_x,start_y,stretch_x,stretch_y,min_x,min_y,&screen_x,&screen_y,
			depth,derivative_xi))
		{
			/* calculate error */
			error_x=screen_x-x;
			error_y=screen_y-y;
			screen_error=error_x*error_x+error_y*error_y;
			/* calculate Newton-Raphson step */
			if (0!=(det=derivative_xi[0]*derivative_xi[3]-
				derivative_xi[1]*derivative_xi[2]))
			{
				step_xi_1=(error_x*derivative_xi[3]-error_y*derivative_xi[1])/det;
				step_xi_2=(error_y*derivative_xi[0]-error_x*derivative_xi[2])/det;
				xi_error=step_xi_1*step_xi_1+step_xi_2*step_xi_2;
				i--;
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"inverse_project.  Error projecting xi point");
			return_code=0;
		}
	}
if ((x==399)&&((y==79)||(y==80)))
{
	printf("inverse_project %g %g %g %g %g %g\n",x,y,screen_tolerance,
		xi_tolerance,maximum_step_size,tolerance_newton_raphson);
	printf("  %d %g %g %d %g %g %g\n",return_code,screen_error,xi_error,i,
		xi_error,xi[0],xi[1]);
}
	if (return_code)
	{
		if (!((screen_error<=screen_tolerance)&&(xi_error<=xi_tolerance)&&
			(xi[0]>= -tolerance_newton_raphson)&&(xi[0]<=1+tolerance_newton_raphson)&&
			(xi[1]>= -tolerance_newton_raphson)&&(xi[1]<=1+tolerance_newton_raphson)))
		{
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* inverse_project */

struct Project_element_data
{
	enum Projection_type projection_type;
	FE_value xi_3;
	float min_x,min_y,start_x,start_y,stretch_x,stretch_y;
	float *pixel_depths,*pixel_fibre_angles,*pixel_values;
	int drawing_height,drawing_width,field_component_number,
		seed_point_discretization;
	struct FE_field *coordinate_field,*data_field,*fibre_field;
}; /* struct Project_element_data */

/*???DB.  Should be using list.h ? */
struct Fill_pixel
{
	char interior;
	FE_value xi[3];
	int x,y;
	struct Fill_pixel *next,*previous;
}; /* struct Fill_pixel */

static int project_element(struct FE_element *element,
	void *void_project_element_data)
/*******************************************************************************
LAST MODIFIED : 1 July 1999

DESCRIPTION :
Project the element using a "rasterization" method
{
	set up a list of seed points in xi space (regularly spaced, 10x10 ?)
	while the seed point list is not empty
	{
		choose the next seed point, (xi1,xi2)
		project the seed point to (x1,x2) in screen space
		use Newton-Raphson to do the inverse projection to find if any of
			(floor(x1),floor(x2)) or (floor(x1),ceil(x2)) or (ceil(x1),floor(x2)) or
			(ceil(x1),ceil(x2)) are in the element.
		{
			call the first one the seed pixel (p1,p2)
			"fill" the screen projection starting from (p1,p2)
		}
		remove (xi1,xi2) from the list of seed points
	}
}

"fill" the screen projection starting from (p1,p2)
{
	initialize a boundary pixel list as empty
		NB.  Each element in the list is a pixel and the list is ordered by first
			pixel coordinate and then second pixel coordinate
	initialize a prospective fill pixel with
		[(p1,p2),(xi1,xi2),(p1-1,p2)]
		[(p1,p2),(xi1,xi2),(p1,p2-1)]
		[(p1,p2),(xi1,xi2),(p1,p2+1)]
		[(p1,p2),(xi1,xi2),(p1+1,p2)]
		NB.  Each element in the list is a triple of coordinate pairs.  The first
			pair of coordinates is a pixel that is filled.  The second pair is the xi
			location of the filled pixel.  The third pair is the location of a
			prospective fill pixel.  The list is ordered by each of the pixel
			coordinates in turn with the first being most significant.
	while the prospective fill pixel list is not empty
	{
		choose the first in the list, [(a1,a2),(b1,b2),(c1,c2)]
		determine if (c1,c2) is in the element by starting Newton-Raphson at (b1,b2)
		if (c1,c2) is in the element
		{
			"add" [(c1,c2),(cxi1,cxi2),(c1-1,c2)], [(c1,c2),(cxi1,cxi2),(c1,c2-1)],
				[(c1,c2),(cxi1,cxi2),(c1,c2+1)] and [(c1,c2),(cxi1,cxi2),(c1+1,c2)] to
				the list of prospective fill pixels
			remove all seed points within in a distance
				sqrt((b1-cxi1)**2+(b2-cxi2)**2) of (cxi1,cxi2) from the list of seed
				points
		}
		else
		{
			add (c1,c2) to the list of boundary pixels
		}
		remove [(a1,a2),(b1,b2),(c1,c2)] from the list of prospective fill pixels
	}
}

"add" [(e1,e2),(f1,f2),(g1,g2)] to the list of prospective pixels
{
	if (g1,g2) is in the list of boundary pixels
	{
		don't add
	}
	else
	{
		if (g1,g2) is present as a filled pixel in the list
		{
			don't add
		}
		else
		{
			if (g1,g2) is present as a prospective fill pixel in the list
			{
				don't add
			}
			else
			{
				add
			}
		}
	}
}
==============================================================================*/
{
	const char interior_0m1=0x01,interior_0p1=0x02,interior_m10=0x04,
		interior_p10=0x08;
	char interior;
	enum Coordinate_system_type coordinate_system_type;
	enum Projection_type projection_type;
	FE_value a[3],b[3],c[3],seed_point_spacing,x,xi[3],xi_1,xi_2,xi_3,xi_save[3],
		y,z;
	float depth,jacobian[9],min_x,min_y,*pixel_depths,*pixel_fibre_angles,
		*pixel_values,project_1,project_2,project_3,screen_x,screen_y,start_x,
		start_y,stretch_x,stretch_y,value;
	int add_0m1,add_0p1,add_m10,add_p10,drawing_height,drawing_width,
		field_component_number,i,index,j,number_of_fibre_components,pixel_x,
		pixel_x_m1,pixel_x_p1,pixel_y,pixel_y_m1,pixel_y_p1,return_code;
	struct Fill_pixel *boundary_pixel,*boundary_pixel_list,*temp_boundary_pixel;
	struct FE_element_field_values coordinate_element_field_values,
		data_element_field_values,fibre_element_field_values;
	struct FE_field *data_field,*fibre_field;
	struct Fill_pixel *fill_pixel,**fill_pixel_address,*fill_pixel_list,
		*fill_pixel_0m1,*fill_pixel_0p1,*fill_pixel_m10,*fill_pixel_p10;
	struct Project_element_data *project_element_data;

	ENTER(project_element);
	/* check arguments */
	if (element&&(project_element_data=
		(struct Project_element_data *)void_project_element_data)&&
		(2<=project_element_data->seed_point_discretization)&&
		(project_element_data->coordinate_field)&&
		(3==get_FE_field_number_of_components(project_element_data->
		coordinate_field)))
	{
		/* do not look at parents */
		if (element->information)
		{
			/* determine if the field is defined over the element */
			if (return_code=calculate_FE_element_field_values(element,
				project_element_data->coordinate_field,/*time*/0,1,
				&coordinate_element_field_values, 
				/*top_level_element*/(struct FE_element *)NULL))
			{
				if (data_field=project_element_data->data_field)
				{
					if (!(return_code=calculate_FE_element_field_values(element,
						data_field,/*time*/0,0,&data_element_field_values,
						/*top_level_element*/(struct FE_element *)NULL)))
					{
						display_message(ERROR_MESSAGE,
							"project_element.  Data field not defined for element");
						clear_FE_element_field_values(&coordinate_element_field_values);
					}
				}
				if (return_code&&(fibre_field=project_element_data->fibre_field))
				{
					if (return_code=calculate_FE_element_field_values(element,fibre_field,
						/*time*/0,0,&fibre_element_field_values,
						/*top_level_element*/(struct FE_element *)NULL))
					{
						number_of_fibre_components=
							fibre_element_field_values.number_of_components;
						if (!((3==element->shape->dimension)&&
							(3==number_of_fibre_components))&&!
							((2==element->shape->dimension)&&(1<=number_of_fibre_components)))
						{
							clear_FE_element_field_values(&fibre_element_field_values);
							return_code=0;
						}
					}
					if (!return_code)
					{
						display_message(ERROR_MESSAGE,
							"project_element.  Fibre field not defined for element");
						clear_FE_element_field_values(&coordinate_element_field_values);
						if (data_field)
						{
							clear_FE_element_field_values(&data_element_field_values);
						}
					}
				}
				if (return_code)
				{
					coordinate_system_type=get_coordinate_system_type(
            get_FE_field_coordinate_system(
            coordinate_element_field_values.field));
					projection_type=project_element_data->projection_type;
					drawing_width=project_element_data->drawing_width;
					drawing_height=project_element_data->drawing_height;
					start_x=project_element_data->start_x;
					start_y=project_element_data->start_y;
					stretch_x=project_element_data->stretch_x;
					stretch_y=project_element_data->stretch_y;
					min_x=project_element_data->min_x;
					min_y=project_element_data->min_y;
					pixel_depths=project_element_data->pixel_depths;
					pixel_fibre_angles=project_element_data->pixel_fibre_angles;
					pixel_values=project_element_data->pixel_values;
					/* initialize the boundary pixel list as empty.  NB.  Each
						item in the list is a pixel and the list is ordered by first
						pixel coordinate and then second pixel coordinate */
					boundary_pixel_list=(struct Fill_pixel *)NULL;
					boundary_pixel=(struct Fill_pixel *)NULL;
					/* initialize the prospective fill pixel list.  NB.  Each item
						in the list is a triple of coordinate pairs.  The first pair
						of coordinates is a pixel that is filled.  The second pair is
						the xi location of the filled pixel.  The third pair is the
						location of a prospective fill pixel.  The list is ordered by
						each of the pixel coordinates in turn with the first being
						most significant */
					fill_pixel_list=(struct Fill_pixel *)NULL;
					/* project the seed points */
					xi_3=project_element_data->xi_3;
					i=project_element_data->seed_point_discretization;
					seed_point_spacing=
						1./(float)((project_element_data->seed_point_discretization)-1);
					xi_1=0;
					while (return_code&&(i>0))
					{
						j=project_element_data->seed_point_discretization;
						xi_2=0;
						while (return_code&&(j>0))
						{
							xi[0]=xi_1;
							xi[1]=xi_2;
							xi[2]=xi_3;
							/* project the seed point to the screen */
							if (project_xi_point(xi,&coordinate_element_field_values,
								projection_type,start_x,start_y,stretch_x,stretch_y,min_x,
								min_y,&screen_x,&screen_y,&depth,(float *)NULL))
							{
								/* use Newton-Raphson to do the inverse projection to find if
									any of (floor(x1),floor(x2)) or (floor(x1),ceil(x2)) or
									(ceil(x1),floor(x2)) or (ceil(x1),ceil(x2)) are in the
									element */
								xi_save[0]=xi[0];
								xi_save[1]=xi[1];
								xi_save[2]=xi[2];
								if (screen_x<0)
								{
									pixel_x=(int)(screen_x-1);
								}
								else
								{
									pixel_x=(int)screen_x;
								}
								if (screen_y<0)
								{
									pixel_y=(int)(screen_y-1);
								}
								else
								{
									pixel_y=(int)screen_y;
								}
								if (!inverse_project(xi,&coordinate_element_field_values,
									projection_type,start_x,start_y,stretch_x,stretch_y,min_x,
									min_y,(float)pixel_x,(float)pixel_y,&depth))
								{
									xi[0]=xi_save[0];
									xi[1]=xi_save[1];
									xi[2]=xi_save[2];
									pixel_x++;
									if (!inverse_project(xi,&coordinate_element_field_values,
										projection_type,start_x,start_y,stretch_x,stretch_y,min_x,
										min_y,(float)pixel_x,(float)pixel_y,&depth))
									{
										xi[0]=xi_save[0];
										xi[1]=xi_save[1];
										xi[2]=xi_save[2];
										pixel_y++;
										if (!inverse_project(xi,&coordinate_element_field_values,
											projection_type,start_x,start_y,stretch_x,stretch_y,
											min_x,min_y,(float)pixel_x,(float)pixel_y,&depth))
										{
											xi[0]=xi_save[0];
											xi[1]=xi_save[1];
											xi[2]=xi_save[2];
											pixel_x--;
											if (!inverse_project(xi,
												&coordinate_element_field_values,projection_type,
												start_x,start_y,stretch_x,stretch_y,min_x,min_y,
												(float)pixel_x,(float)pixel_y,&depth))
											{
												return_code=0;
											}
										}
									}
								}
								if (return_code&&(0<=pixel_x)&&(pixel_x<drawing_width)&&
									(0<=pixel_y)&&(pixel_y<drawing_height))
								{
									/* add to the fill pixel list */
									fill_pixel_address= &fill_pixel_list;
									while ((fill_pixel= *fill_pixel_address)&&
										((fill_pixel->x<pixel_x)||((fill_pixel->x==pixel_x)&&
										(fill_pixel->y<pixel_y))))
									{
										fill_pixel_address= &(fill_pixel->next);
									}
									if (!fill_pixel||(fill_pixel->x!=pixel_x)||
										(fill_pixel->y!=pixel_y))
									{
										if (ALLOCATE(fill_pixel,struct Fill_pixel,1))
										{
											fill_pixel->next= *fill_pixel_address;
											/* fill pixel list is only singly linked */
											fill_pixel->previous=(struct Fill_pixel *)NULL;
											*fill_pixel_address=fill_pixel;
											fill_pixel->interior=0;
											(fill_pixel->xi)[0]=xi[0];
											(fill_pixel->xi)[1]=xi[1];
											(fill_pixel->xi)[2]=xi[2];
											fill_pixel->x=pixel_x;
											fill_pixel->y=pixel_y;
										}
										else
										{
											display_message(ERROR_MESSAGE,
											"project_element.  Insufficient memory for fill pixel");
											return_code=0;
										}
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
									"project_element.  Error projecting seed point");
								return_code=0;
							}
							xi_2 += seed_point_spacing;
							j--;
						}
						xi_1 += seed_point_spacing;
						i--;
					}
					/* while the prospective fill pixel list is not empty */
					while (return_code&&(fill_pixel_list))
					{
						/* choose the first in the list, [(a1,a2),(b1,b2),(c1,c2)]*/
						/* determine if (c1,c2) is in the element by starting
							Newton-Raphson at (b1,b2) */
						/* if (c1,c2) is in the element */
						pixel_x=fill_pixel_list->x;
						pixel_y=fill_pixel_list->y;
						if ((0<=pixel_x)&&(pixel_x<drawing_width)&&(0<=pixel_y)&&
							(pixel_y<drawing_height)&&
							inverse_project(fill_pixel_list->xi,
							&coordinate_element_field_values,projection_type,start_x,
							start_y,stretch_x,stretch_y,min_x,min_y,(float)pixel_x,
							(float)pixel_y,&depth))
						{
/*???debug */
/*printf("interior %d %d\n",pixel_x,pixel_y);*/
							xi_save[0]=(fill_pixel_list->xi)[0];
							xi_save[1]=(fill_pixel_list->xi)[1];
							xi_save[2]=(fill_pixel_list->xi)[2];
							interior=fill_pixel_list->interior;
							/* remove [(a1,a2),(b1,b2),(c1,c2)] from the list of
								prospective fill pixels */
							fill_pixel=fill_pixel_list;
							fill_pixel_list=fill_pixel->next;
							DEALLOCATE(fill_pixel);
							/* "colour" the pixel */
							index=pixel_y*(int)drawing_width+pixel_x;
							/* check if projection is in front of existing */
								/*???DB.  Allow behind and in front ? */
							if (depth>pixel_depths[index])
							{
								if (fibre_field)
								{
									/* calculate the fibre vectors */
									if (calculate_FE_element_anatomical(
										&coordinate_element_field_values,
										&fibre_element_field_values,xi_save,&x,&y,&z,a,b,c,
										(FE_value *)NULL))
									{
										/* different coordinate systems have different
											principal axes */
										switch (coordinate_system_type)
										{
											case PROLATE_SPHEROIDAL:
											{
												switch (projection_type)
												{
													case CYLINDRICAL_PROJECTION:
													{
														/* also see project_point */
														project_1=x;
														project_2=y;
														project_3=z;
														x=project_2;
														y=project_3;
														z= -project_1;
														project_1=a[0];
														project_2=a[1];
														project_3=a[2];
														a[0]=project_2;
														a[1]=project_3;
														a[2]= -project_1;
													} break;
												}
											} break;
										}
										if (project_point(projection_type,RECTANGULAR_CARTESIAN,
											x,y,z,&project_1,&project_2,&project_3,jacobian))
										{
											x=stretch_x*(jacobian[0]*a[0]+jacobian[1]*a[1]+
												jacobian[2]*a[2]);
											y= -stretch_y*(jacobian[3]*a[0]+jacobian[4]*a[1]+
												jacobian[5]*a[2]);
											if ((x!=0)||(y!=0))
											{
												pixel_fibre_angles[index]=atan2(y,x);
											}
											else
											{
												pixel_fibre_angles[index]=0;
											}
										}
										else
										{
											display_message(WARNING_MESSAGE,
												"project_element.  Error projecting fibre vector");
											pixel_fibre_angles[index]=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"project_element.  Error calculating fibre field");
										return_code=0;
									}
								}
								if (data_field)
								{
									if (calculate_FE_element_field(field_component_number,
										&data_element_field_values,xi_save,&value,
										(FE_value *)NULL))
									{
										pixel_depths[index]=depth;
										pixel_values[index]=value;
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"project_element.  Error calculating data field");
										return_code=0;
									}
								}
								else
								{
									pixel_depths[index]=depth;
									pixel_values[index]=depth;
								}
							}
							/* "add" [(c1,c2),(cxi1,cxi2),(c1-1,c2)],
								[(c1,c2),(cxi1,cxi2),(c1,c2-1)],
								[(c1,c2),(cxi1,cxi2),(c1,c2+1)] and
								[(c1,c2),(cxi1,cxi2),(c1+1,c2)] to the list of prospective
								fill pixels */
							add_m10= !(interior&interior_m10);
							add_0m1= !(interior&interior_0m1);
							add_0p1= !(interior&interior_0p1);
							add_p10= !(interior&interior_p10);
							fill_pixel_m10=(struct Fill_pixel *)NULL;
							fill_pixel_0m1=(struct Fill_pixel *)NULL;
							fill_pixel_0p1=(struct Fill_pixel *)NULL;
							fill_pixel_p10=(struct Fill_pixel *)NULL;
							pixel_x_m1=pixel_x-1;
							pixel_x_p1=pixel_x+1;
							pixel_y_m1=pixel_y-1;
							pixel_y_p1=pixel_y+1;
							/* don't add if prospective pixel is in the boundary pixel
								list */
							if (boundary_pixel&&(add_m10||add_0m1||add_0p1||add_p10))
							{
								if ((pixel_x_m1<boundary_pixel->x)||
									((pixel_x_m1==boundary_pixel->x)&&
									(pixel_y<boundary_pixel->y)))
								{
									while ((boundary_pixel->previous)&&
										((pixel_x_m1<boundary_pixel->x)||
										((pixel_x_m1==boundary_pixel->x)&&
										(pixel_y<boundary_pixel->y))))
									{
										boundary_pixel=boundary_pixel->previous;
									}
								}
								else
								{
									while ((boundary_pixel->next)&&
										((pixel_x_m1>boundary_pixel->x)||
										((pixel_x_m1==boundary_pixel->x)&&
										(pixel_y>boundary_pixel->y))))
									{
										boundary_pixel=boundary_pixel->next;
									}
								}
								if (add_m10&&(pixel_x_m1==boundary_pixel->x)&&
									(pixel_y==boundary_pixel->y))
								{
									/* put back in the fill pixel list with a new starting
										estimate */
									fill_pixel_m10=boundary_pixel;
									if (boundary_pixel=fill_pixel_m10->previous)
									{
										fill_pixel_m10->previous->next=fill_pixel_m10->next;
									}
									else
									{
										boundary_pixel_list=fill_pixel_m10->next;
									}
									if (fill_pixel_m10->next)
									{
										boundary_pixel=fill_pixel_m10->next;
										fill_pixel_m10->next->previous=fill_pixel_m10->previous;
									}
									fill_pixel_m10->interior |= interior_p10;
								}
								if (add_0m1&&boundary_pixel)
								{
									while ((boundary_pixel->next)&&
										((pixel_x>boundary_pixel->x)||
										((pixel_x==boundary_pixel->x)&&
										(pixel_y_m1>boundary_pixel->y))))
									{
										boundary_pixel=boundary_pixel->next;
									}
									if ((pixel_x==boundary_pixel->x)&&
										(pixel_y_m1==boundary_pixel->y))
									{
										/* put back in the fill pixel list with a new starting
											estimate */
										fill_pixel_0m1=boundary_pixel;
										if (boundary_pixel=fill_pixel_0m1->previous)
										{
											fill_pixel_0m1->previous->next=fill_pixel_0m1->next;
										}
										else
										{
											boundary_pixel_list=fill_pixel_0m1->next;
										}
										if (fill_pixel_0m1->next)
										{
											boundary_pixel=fill_pixel_0m1->next;
											fill_pixel_0m1->next->previous=fill_pixel_0m1->previous;
										}
										fill_pixel_0m1->interior |= interior_0p1;
									}
								}
								if (add_0p1&&boundary_pixel)
								{
									while ((boundary_pixel->next)&&
										((pixel_x>boundary_pixel->x)||
										((pixel_x==boundary_pixel->x)&&
										(pixel_y_p1>boundary_pixel->y))))
									{
										boundary_pixel=boundary_pixel->next;
									}
									if ((pixel_x==boundary_pixel->x)&&
										(pixel_y_p1==boundary_pixel->y))
									{
										/* put back in the fill pixel list with a new starting
											estimate */
										fill_pixel_0p1=boundary_pixel;
										if (boundary_pixel=fill_pixel_0p1->previous)
										{
											fill_pixel_0p1->previous->next=fill_pixel_0p1->next;
										}
										else
										{
											boundary_pixel_list=fill_pixel_0p1->next;
										}
										if (fill_pixel_0p1->next)
										{
											boundary_pixel=fill_pixel_0p1->next;
											fill_pixel_0p1->next->previous=fill_pixel_0p1->previous;
										}
										fill_pixel_0p1->interior |= interior_0m1;
									}
								}
								if (add_p10&&boundary_pixel)
								{
									while ((boundary_pixel->next)&&
										((pixel_x_p1>boundary_pixel->x)||
										((pixel_x_p1==boundary_pixel->x)&&
										(pixel_y>boundary_pixel->y))))
									{
										boundary_pixel=boundary_pixel->next;
									}
									if ((pixel_x_p1==boundary_pixel->x)&&
										(pixel_y==boundary_pixel->y))
									{
										add_p10=0;
										/* put back in the fill pixel list with a new starting
											estimate */
										fill_pixel_p10=boundary_pixel;
										if (boundary_pixel=fill_pixel_p10->previous)
										{
											fill_pixel_p10->previous->next=fill_pixel_p10->next;
										}
										else
										{
											boundary_pixel_list=fill_pixel_p10->next;
										}
										if (fill_pixel_p10->next)
										{
											boundary_pixel=fill_pixel_p10->next;
											fill_pixel_p10->next->previous=fill_pixel_p10->previous;
										}
										fill_pixel_p10->interior |= interior_m10;
									}
								}
							}
							/* don't add if prospective pixel appears as a prospective
								pixel in the fill pixel list */
							fill_pixel_address= &fill_pixel_list;
							if (add_m10)
							{
								while ((fill_pixel= *fill_pixel_address)&&
									((fill_pixel->x<pixel_x_m1)||
									((fill_pixel->x==pixel_x_m1)&&
									(fill_pixel->y<pixel_y))))
								{
									fill_pixel_address= &(fill_pixel->next);
								}
								if (fill_pixel_m10)
								{
									fill_pixel=fill_pixel_m10;
								}
								else
								{
									if (fill_pixel&&(pixel_x_m1==fill_pixel->x)&&
										(pixel_y==fill_pixel->y))
									{
										fill_pixel->interior |= interior_p10;
										fill_pixel=(struct Fill_pixel *)NULL;
									}
									else
									{
										if (ALLOCATE(fill_pixel,struct Fill_pixel,1))
										{
											fill_pixel->interior=interior_p10;
											fill_pixel->x=pixel_x_m1;
											fill_pixel->y=pixel_y;
										}
										else
										{
											display_message(ERROR_MESSAGE,
											"project_element.  Insufficient memory for fill pixel");
											return_code=0;
										}
									}
								}
								if (fill_pixel)
								{
									/* fill pixel list is only singly linked */
									fill_pixel->next= *fill_pixel_address;
									*fill_pixel_address=fill_pixel;
									fill_pixel_address= &(fill_pixel->next);
									(fill_pixel->xi)[0]=xi_save[0];
									(fill_pixel->xi)[1]=xi_save[1];
									(fill_pixel->xi)[2]=xi_save[2];
								}
							}
							if (add_0m1)
							{
								while ((fill_pixel= *fill_pixel_address)&&
									((fill_pixel->x<pixel_x)||
									((fill_pixel->x==pixel_x)&&
									(fill_pixel->y<pixel_y_m1))))
								{
									fill_pixel_address= &(fill_pixel->next);
								}
								if (fill_pixel_0m1)
								{
									fill_pixel=fill_pixel_0m1;
								}
								else
								{
									if (fill_pixel&&(pixel_x==fill_pixel->x)&&
										(pixel_y_m1==fill_pixel->y))
									{
										fill_pixel->interior |= interior_0p1;
										fill_pixel=(struct Fill_pixel *)NULL;
									}
									else
									{
										if (ALLOCATE(fill_pixel,struct Fill_pixel,1))
										{
											fill_pixel->interior=interior_0p1;
											fill_pixel->x=pixel_x;
											fill_pixel->y=pixel_y_m1;
										}
										else
										{
											display_message(ERROR_MESSAGE,
											"project_element.  Insufficient memory for fill pixel");
											return_code=0;
										}
									}
								}
								if (fill_pixel)
								{
									/* fill pixel list is only singly linked */
									fill_pixel->next= *fill_pixel_address;
									*fill_pixel_address=fill_pixel;
									fill_pixel_address= &(fill_pixel->next);
									(fill_pixel->xi)[0]=xi_save[0];
									(fill_pixel->xi)[1]=xi_save[1];
									(fill_pixel->xi)[2]=xi_save[2];
								}
							}
							if (add_0p1)
							{
								while ((fill_pixel= *fill_pixel_address)&&
									((fill_pixel->x<pixel_x)||
									((fill_pixel->x==pixel_x)&&
									(fill_pixel->y<pixel_y_p1))))
								{
									fill_pixel_address= &(fill_pixel->next);
								}
								if (fill_pixel_0p1)
								{
									fill_pixel=fill_pixel_0p1;
								}
								else
								{
									if (fill_pixel&&(pixel_x==fill_pixel->x)&&
										(pixel_y_p1==fill_pixel->y))
									{
										fill_pixel->interior |= interior_0m1;
										fill_pixel=(struct Fill_pixel *)NULL;
									}
									else
									{
										if (ALLOCATE(fill_pixel,struct Fill_pixel,1))
										{
											fill_pixel->interior=interior_0m1;
											fill_pixel->x=pixel_x;
											fill_pixel->y=pixel_y_p1;
										}
										else
										{
											display_message(ERROR_MESSAGE,
											"project_element.  Insufficient memory for fill pixel");
											return_code=0;
										}
									}
								}
								if (fill_pixel)
								{
									/* fill pixel list is only singly linked */
									fill_pixel->next= *fill_pixel_address;
									*fill_pixel_address=fill_pixel;
									fill_pixel_address= &(fill_pixel->next);
									(fill_pixel->xi)[0]=xi_save[0];
									(fill_pixel->xi)[1]=xi_save[1];
									(fill_pixel->xi)[2]=xi_save[2];
								}
							}
							if (add_p10)
							{
								while ((fill_pixel= *fill_pixel_address)&&
									((fill_pixel->x<pixel_x_p1)||
									((fill_pixel->x==pixel_x_p1)&&
									(fill_pixel->y<pixel_y))))
								{
									fill_pixel_address= &(fill_pixel->next);
								}
								if (fill_pixel_p10)
								{
									fill_pixel=fill_pixel_p10;
								}
								else
								{
									if (fill_pixel&&(pixel_x_p1==fill_pixel->x)&&
										(pixel_y==fill_pixel->y))
									{
										fill_pixel->interior |= interior_m10;
										fill_pixel=(struct Fill_pixel *)NULL;
									}
									else
									{
										if (ALLOCATE(fill_pixel,struct Fill_pixel,1))
										{
											fill_pixel->interior=interior_m10;
											fill_pixel->x=pixel_x_p1;
											fill_pixel->y=pixel_y;
										}
										else
										{
											display_message(ERROR_MESSAGE,
											"project_element.  Insufficient memory for fill pixel");
											return_code=0;
										}
									}
								}
								if (fill_pixel)
								{
									/* fill pixel list is only singly linked */
									fill_pixel->next= *fill_pixel_address;
									*fill_pixel_address=fill_pixel;
									fill_pixel_address= &(fill_pixel->next);
									(fill_pixel->xi)[0]=xi_save[0];
									(fill_pixel->xi)[1]=xi_save[1];
									(fill_pixel->xi)[2]=xi_save[2];
								}
							}
						}
						else
						{
/*???debug */
/*printf("boundary %d %d\n",pixel_x,pixel_y);*/
							/* add (c1,c2) to the list of boundary pixels */
							if (boundary_pixel_list)
							{
								/* remove [(a1,a2),(b1,b2),(c1,c2)] from the list of
									prospective fill pixels */
								temp_boundary_pixel=fill_pixel_list;
								fill_pixel_list=fill_pixel_list->next;
								if ((pixel_x<boundary_pixel->x)||
									((pixel_x==boundary_pixel->x)&&
									(pixel_y<boundary_pixel->y)))
								{
									while ((boundary_pixel->previous)&&
										((pixel_x<boundary_pixel->x)||
										((pixel_x==boundary_pixel->x)&&
										(pixel_y<boundary_pixel->y))))
									{
										boundary_pixel=boundary_pixel->previous;
									}
									if ((pixel_x<boundary_pixel->x)||
										((pixel_x==boundary_pixel->x)&&
										(pixel_y<boundary_pixel->y)))
									{
										temp_boundary_pixel->next=boundary_pixel;
										boundary_pixel->previous=temp_boundary_pixel;
										temp_boundary_pixel->previous=
											(struct Fill_pixel *)NULL;
										boundary_pixel_list=temp_boundary_pixel;
									}
									else
									{
										if (temp_boundary_pixel->next=boundary_pixel->next)
										{
											boundary_pixel->next->previous=
												temp_boundary_pixel;
										}
										temp_boundary_pixel->previous=boundary_pixel;
										boundary_pixel->next=temp_boundary_pixel;
									}
									boundary_pixel=temp_boundary_pixel;
								}
								else
								{
									while ((boundary_pixel->next)&&
										((pixel_x>boundary_pixel->x)||
										((pixel_x==boundary_pixel->x)&&
										(pixel_y>boundary_pixel->y))))
									{
										boundary_pixel=boundary_pixel->next;
									}
									if ((pixel_x>boundary_pixel->x)||
										((pixel_x==boundary_pixel->x)&&
										(pixel_y>boundary_pixel->y)))
									{
										boundary_pixel->next=temp_boundary_pixel;
										temp_boundary_pixel->previous=boundary_pixel;
										temp_boundary_pixel->next=(struct Fill_pixel *)NULL;
									}
									else
									{
										if (temp_boundary_pixel->previous=
											boundary_pixel->previous)
										{
											boundary_pixel->previous->next=
												temp_boundary_pixel;
										}
										temp_boundary_pixel->next=boundary_pixel;
										boundary_pixel->previous=temp_boundary_pixel;
									}
									boundary_pixel=temp_boundary_pixel;
								}
							}
							else
							{
								/* remove [(a1,a2),(b1,b2),(c1,c2)] from the list of
									prospective fill pixels */
								boundary_pixel=fill_pixel_list;
								fill_pixel_list=fill_pixel_list->next;
								boundary_pixel_list=boundary_pixel;
								boundary_pixel->x=pixel_x;
								boundary_pixel->y=pixel_y;
								boundary_pixel->next=(struct Fill_pixel *)NULL;
								boundary_pixel->previous=(struct Fill_pixel *)NULL;
							}
						}
					}
					while (fill_pixel=fill_pixel_list)
					{
						fill_pixel_list=fill_pixel->next;
						DEALLOCATE(fill_pixel);
					}
					while (boundary_pixel=boundary_pixel_list)
					{
						boundary_pixel_list=boundary_pixel->next;
						DEALLOCATE(boundary_pixel);
					}
					/* tidy up */
					clear_FE_element_field_values(&coordinate_element_field_values);
					if (data_field)
					{
						clear_FE_element_field_values(&data_element_field_values);
					}
					if (fibre_field)
					{
						clear_FE_element_field_values(&fibre_element_field_values);
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
					"project_element.  Coordinate field not defined for element");
				return_code=1;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"project_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* project_element */

static int find_coordinate_field(struct FE_element *element,void *void_field)
/*******************************************************************************
LAST MODIFIED : 2 September 2001

DESCRIPTION :
Succeeds if a coordinate field is defined for the element.
==============================================================================*/
{
	int return_code;
	struct FE_element_field *element_field;
	struct FE_field **field;

	ENTER(find_coordinate_field);
	/* check arguments */
	if (element&&(field=(struct FE_field **)void_field))
	{
		/* does not look at parents */
		if ((element->information)&&(element->information->fields)&&
			(element->information->fields->element_field_list))
		{
			if (element_field=FIRST_OBJECT_IN_LIST_THAT(FE_element_field)(
				 FE_element_field_is_coordinate_field,(void *)NULL,
				element->information->fields->element_field_list))
			{
				return_code=1;
				*field=element_field->field;
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"find_coordinate_field.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* find_coordinate_field */

static int find_anatomical_field(struct FE_element *element,void *void_field)
/*******************************************************************************
LAST MODIFIED : 2 September 2001

DESCRIPTION :
Succeeds if a anatomical field is defined for the element.
==============================================================================*/
{
	int return_code;
	struct FE_element_field *element_field;
	struct FE_field **field;

	ENTER(find_anatomical_field);
	/* check arguments */
	if (element&&(field=(struct FE_field **)void_field))
	{
		/* does not look at parents */
		if ((element->information)&&(element->information->fields)&&
			(element->information->fields->element_field_list))
		{
			if (element_field=FIRST_OBJECT_IN_LIST_THAT(FE_element_field)(
				 FE_element_field_is_anatomical_fibre_field,(void *)NULL,
				element->information->fields->element_field_list))
			{
				return_code=1;
				*field=element_field->field;
			}
			else
			{
				return_code=0;
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
			"find_anatomical_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* find_anatomical_field */

struct Draw_node_data
{
	char *node_drawn;
	enum Nodes_option nodes_option;
	float min_x,min_y,*node_depth,*pixel_depths,stretch_x,stretch_y,*x,*y;
	struct
	{
		GC text_colour;
		GC marker_colour;
	} graphics_context;
	XFontStruct *font;
	int drawing_height,drawing_width,component_number,*node_x,*node_y,start_x,
		start_y;
	struct FE_field *field;
	struct Drawing_2d *drawing;
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
	struct Projection_drawing *drawing;
#endif /* defined (OLD_CODE) */
};

static int draw_node(struct FE_node *node,void *void_draw_node_data)
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Add the <element>'s nodes to the node list if they are not already in it.
???DB.  Does not work correctly unless the fields are defined for the element.
==============================================================================*/
{
	char name[20];
	FE_value value;
	float draw_x,draw_y;
	int ascent,descent,direction,name_length,return_code,screen_x,screen_y,
		x_string,y_string;
	struct Draw_node_data *draw_node_data;
	struct Drawing_2d *drawing;
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
	struct Projection_drawing *drawing;
#endif /* defined (OLD_CODE) */
	XCharStruct bounds;

	ENTER(draw_node);
	/* check arguments */
	if (node&&(draw_node_data=(struct Draw_node_data *)void_draw_node_data))
	{
		/* calculate screen position */
		draw_x=(float)(draw_node_data->start_x)+
			(*(draw_node_data->x)-(draw_node_data->min_x))*
			(draw_node_data->stretch_x);
		screen_x=(int)draw_x;
		*(draw_node_data->node_x)=screen_x;
		draw_y=(float)(draw_node_data->start_y)-
			(*(draw_node_data->y)-(draw_node_data->min_y))*
			(draw_node_data->stretch_y);
		screen_y=(int)draw_y;
		*(draw_node_data->node_y)=screen_y;
		if (!(draw_node_data->pixel_depths)||(*(draw_node_data->node_depth)>
			(draw_node_data->pixel_depths)[screen_y*(draw_node_data->drawing_width)+
			screen_x]))
		{
			*(draw_node_data->node_drawn)=1;
			switch (draw_node_data->nodes_option)
			{
				case SHOW_NODE_NAMES:
				{
					sprintf(name,"%d",get_FE_node_cm_node_identifier(node));
				} break;
				case SHOW_NODE_VALUES:
				{
					/* calculate value */
					if ((draw_node_data->field)&&calculate_FE_field(draw_node_data->field,
						draw_node_data->component_number,node,(struct FE_element *)NULL,
						(FE_value *)NULL,/*time*/0,&value))
					{
						sprintf(name,"%g",value);
					}
					else
					{
						sprintf(name," ");
					}
				} break;
				default:
				{
					*(draw_node_data->node_drawn)=0;
				} break;
			}
			if (*(draw_node_data->node_drawn))
			{
				drawing=draw_node_data->drawing;
				/* draw plus */
				XPSDrawLineFloat(User_interface_get_display(drawing->user_interface),drawing->pixel_map,
					(draw_node_data->graphics_context).marker_colour,
					draw_x-2,draw_y,draw_x+2,draw_y);
				XPSDrawLineFloat(User_interface_get_display(drawing->user_interface),drawing->pixel_map,
					(draw_node_data->graphics_context).marker_colour,
					draw_x,draw_y-2,draw_x,draw_y+2);
				/* write name */
				name_length=strlen(name);
				XTextExtents(draw_node_data->font,name,name_length,&direction,&ascent,
					&descent,&bounds);
				x_string=screen_x+(bounds.lbearing-bounds.rbearing+1)/2;
				y_string=screen_y-descent-1;
				/* make sure that the string doesn't extend outside the
					window */
				if (x_string-bounds.lbearing<0)
				{
					x_string=bounds.lbearing;
				}
				else
				{
					if (x_string+bounds.rbearing>draw_node_data->drawing_width)
					{
						x_string=draw_node_data->drawing_width-bounds.rbearing;
					}
				}
				if (y_string-ascent<0)
				{
					y_string=ascent;
				}
				else
				{
					if (y_string+descent>draw_node_data->drawing_height)
					{
						y_string=draw_node_data->drawing_height-descent;
					}
				}
				XPSDrawString(User_interface_get_display(drawing->user_interface),drawing->pixel_map,
					(draw_node_data->graphics_context).text_colour,x_string,y_string,name,
					name_length);
			}
		}
		else
		{
			*(draw_node_data->node_drawn)=0;
		}
		(draw_node_data->node_drawn)++;
		(draw_node_data->node_depth)++;
		(draw_node_data->x)++;
		(draw_node_data->y)++;
		(draw_node_data->node_x)++;
		(draw_node_data->node_y)++;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_node */

struct Draw_element_data
{
	enum Elements_option elements_option;
	enum Projection_type projection_type;
	FE_value xi_3;
	float min_x,min_y,start_x,start_y,stretch_x,stretch_y;
	int drawing_height,drawing_width,element_line_discretization;
	struct
	{
		GC text_colour;
		GC marker_colour;
	} graphics_context;
	struct FE_field *coordinate_field;
	struct Drawing_2d *drawing;
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
	struct Projection_drawing *drawing;
#endif /* defined (OLD_CODE) */
	XFontStruct *font;
};

static int draw_element(struct FE_element *element,void *void_draw_element_data)
/*******************************************************************************
LAST MODIFIED : 1 July 1999

DESCRIPTION :
==============================================================================*/
{
	char name[20];
	Display *display;
	enum Projection_type projection_type;
	FE_value xi[3];
	float depth,first_screen_x,first_screen_y,min_x,min_y,previous_screen_x,
		previous_screen_y,screen_x,screen_y,spacing,start_x,start_y,stretch_x,
		stretch_y;
	GC marker_colour_context;
	int ascent,cut_height,cut_width,descent,direction,drawing_height,
		drawing_width,i,name_length,return_code,x_string,y_string;
	Pixmap pixel_map;
	struct Draw_element_data *draw_element_data;
	struct FE_element_field_values coordinate_element_field_values;
	struct Drawing_2d *drawing;
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
	struct Projection_drawing *drawing;
#endif /* defined (OLD_CODE) */
	XCharStruct bounds;

	ENTER(draw_element);
	/* check arguments */
	if (element&&
		(draw_element_data=(struct Draw_element_data *)void_draw_element_data)&&
		(2<=draw_element_data->element_line_discretization)&&
		(draw_element_data->coordinate_field)&&
		(3==get_FE_field_number_of_components(draw_element_data->coordinate_field)))
	{
		/* do not look at parents */
		if (element->information)
		{
			/* determine if the field is defined over the element */
			if (return_code=calculate_FE_element_field_values(element,
				draw_element_data->coordinate_field,/*time*/0,1,
				&coordinate_element_field_values,
				/*top_level_element*/(struct FE_element *)NULL))
			{
#if defined (OLD_CODE)
				number_of_values_1=(coordinate_element_field_values.
					component_number_of_values)[0];
				maximum_number_of_values=number_of_values_1;
				number_of_values_2=(coordinate_element_field_values.
					component_number_of_values)[1];
				if (number_of_values_2>maximum_number_of_values)
				{
					maximum_number_of_values=number_of_values_2;
				}
				number_of_values_3=(coordinate_element_field_values.
					component_number_of_values)[2];
				if (number_of_values_3>maximum_number_of_values)
				{
					maximum_number_of_values=number_of_values_3;
				}
				if (ALLOCATE(basis_function_values,FE_value,maximum_number_of_values))
				{
#endif /* defined (OLD_CODE) */
					projection_type=draw_element_data->projection_type;
					drawing=draw_element_data->drawing;
					drawing_width=draw_element_data->drawing_width;
					drawing_height=draw_element_data->drawing_height;
					start_x=draw_element_data->start_x;
					start_y=draw_element_data->start_y;
					stretch_x=draw_element_data->stretch_x;
					stretch_y=draw_element_data->stretch_y;
					min_x=draw_element_data->min_x;
					min_y=draw_element_data->min_y;
					pixel_map=drawing->pixel_map;
					display=User_interface_get_display(drawing->user_interface);
					marker_colour_context=
						(draw_element_data->graphics_context).marker_colour;
					cut_height=drawing_height/3;
					cut_width=drawing_width/3;
					/* project the element boundaries */
					xi[0]=0;
					xi[1]=0;
					xi[2]=draw_element_data->xi_3;
					spacing=
						1./(float)((draw_element_data->element_line_discretization)-1);
					if (project_xi_point(xi,&coordinate_element_field_values,
						projection_type,start_x,start_y,stretch_x,stretch_y,min_x,min_y,
						&screen_x,&screen_y,&depth,(float *)NULL))
					{
						first_screen_x=screen_x;
						first_screen_y=screen_y;
						return_code=1;
						i=(draw_element_data->element_line_discretization)-2;
						while (return_code&&(i>0))
						{
							previous_screen_x=screen_x;
							previous_screen_y=screen_y;
							xi[0] += spacing;
							if (project_xi_point(xi,&coordinate_element_field_values,
								projection_type,start_x,start_y,stretch_x,stretch_y,min_x,min_y,
								&screen_x,&screen_y,&depth,(float *)NULL))
							{
								if ((abs((int)(previous_screen_x-screen_x))<cut_width)&&
									(abs((int)(previous_screen_y-screen_y))<cut_height))
								{
									XPSDrawLineFloat(display,pixel_map,marker_colour_context,
										previous_screen_x,previous_screen_y,screen_x,screen_y);
								}
								i--;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"draw_element.  Error projecting (%g,0)",xi[0]);
								return_code=0;
							}
						}
						if (return_code)
						{
							previous_screen_x=screen_x;
							previous_screen_y=screen_y;
							xi[0]=1;
							if (project_xi_point(xi,&coordinate_element_field_values,
								projection_type,start_x,start_y,stretch_x,stretch_y,min_x,min_y,
								&screen_x,&screen_y,&depth,(float *)NULL))
							{
								if ((abs((int)(previous_screen_x-screen_x))<cut_width)&&
									(abs((int)(previous_screen_y-screen_y))<cut_height))
								{
									XPSDrawLineFloat(display,pixel_map,marker_colour_context,
										previous_screen_x,previous_screen_y,screen_x,screen_y);
								}
								i=(draw_element_data->element_line_discretization)-2;
								while (return_code&&(i>0))
								{
									previous_screen_x=screen_x;
									previous_screen_y=screen_y;
									xi[1] += spacing;
									if (project_xi_point(xi,&coordinate_element_field_values,
										projection_type,start_x,start_y,stretch_x,stretch_y,min_x,
										min_y,&screen_x,&screen_y,&depth,(float *)NULL))
									{
										if ((abs((int)(previous_screen_x-screen_x))<cut_width)&&
											(abs((int)(previous_screen_y-screen_y))<cut_height))
										{
											XPSDrawLineFloat(display,pixel_map,marker_colour_context,
												previous_screen_x,previous_screen_y,screen_x,screen_y);
										}
										i--;
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"draw_element.  Error projecting (1,%g)",xi[1]);
										return_code=0;
									}
								}
								if (return_code)
								{
									previous_screen_x=screen_x;
									previous_screen_y=screen_y;
									xi[1]=1;
									if (project_xi_point(xi,&coordinate_element_field_values,
										projection_type,start_x,start_y,stretch_x,stretch_y,min_x,
										min_y,&screen_x,&screen_y,&depth,(float *)NULL))
									{
										if ((abs((int)(previous_screen_x-screen_x))<cut_width)&&
											(abs((int)(previous_screen_y-screen_y))<cut_height))
										{
											XPSDrawLineFloat(display,pixel_map,marker_colour_context,
												previous_screen_x,previous_screen_y,screen_x,screen_y);
										}
										i=(draw_element_data->element_line_discretization)-2;
										while (return_code&&(i>0))
										{
											previous_screen_x=screen_x;
											previous_screen_y=screen_y;
											xi[0] -= spacing;
											if (project_xi_point(xi,&coordinate_element_field_values,
												projection_type,start_x,start_y,stretch_x,stretch_y,
												min_x,min_y,&screen_x,&screen_y,&depth,(float *)NULL))
											{
												if ((abs((int)(previous_screen_x-screen_x))<cut_width)&&
													(abs((int)(previous_screen_y-screen_y))<cut_height))
												{
													XPSDrawLineFloat(display,pixel_map,
														marker_colour_context,previous_screen_x,
														previous_screen_y,screen_x,screen_y);
												}
												i--;
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"draw_element.  Error projecting (%g,1)",xi[0]);
												return_code=0;
											}
										}
										if (return_code)
										{
											previous_screen_x=screen_x;
											previous_screen_y=screen_y;
											xi[0]=0;
											if (project_xi_point(xi,&coordinate_element_field_values,
												projection_type,start_x,start_y,stretch_x,stretch_y,
												min_x,min_y,&screen_x,&screen_y,&depth,(float *)NULL))
											{
												if ((abs((int)(previous_screen_x-screen_x))<cut_width)&&
													(abs((int)(previous_screen_y-screen_y))<cut_height))
												{
													XPSDrawLineFloat(display,pixel_map,
														marker_colour_context,previous_screen_x,
														previous_screen_y,screen_x,screen_y);
												}
												i=(draw_element_data->element_line_discretization)-2;
												while (return_code&&(i>0))
												{
													previous_screen_x=screen_x;
													previous_screen_y=screen_y;
													xi[1] -= spacing;
													if (project_xi_point(xi,
														&coordinate_element_field_values,projection_type,
														start_x,start_y,stretch_x,stretch_y,min_x,min_y,
														&screen_x,&screen_y,&depth,(float *)NULL))
													{
														if ((abs((int)(previous_screen_x-screen_x))<
															cut_width)&&
															(abs((int)(previous_screen_y-screen_y))<
															cut_height))
														{
															XPSDrawLineFloat(display,pixel_map,
																marker_colour_context,previous_screen_x,
																previous_screen_y,screen_x,screen_y);
														}
														i--;
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"draw_element.  Error projecting (0,%g)",xi[1]);
														return_code=0;
													}
												}
												if (return_code)
												{
													if ((abs((int)(first_screen_x-screen_x))<cut_width)&&
														(abs((int)(first_screen_y-screen_y))<cut_height))
													{
														XPSDrawLineFloat(display,pixel_map,
															marker_colour_context,screen_x,screen_y,
															first_screen_x,first_screen_y);
													}
													switch (draw_element_data->elements_option)
													{
														case SHOW_ELEMENT_NAMES_AND_BOUNDARIES:
														{
															/* write the element number */
															xi[0]=0.5;
															xi[1]=0.5;
															if (project_xi_point(xi,
																&coordinate_element_field_values,
																projection_type,start_x,start_y,stretch_x,
																stretch_y,min_x,min_y,&screen_x,&screen_y,
																&depth,(float *)NULL))
															{
																sprintf(name,"%d",
																	(element->cm).number);
																if (return_code)
																{
																	/* write name */
																	name_length=strlen(name);
																	XTextExtents(draw_element_data->font,name,
																		name_length,&direction,&ascent,&descent,
																		&bounds);
																	x_string=screen_x+
																		(bounds.lbearing-bounds.rbearing+1)/2;
																	y_string=screen_y+(ascent-descent)/2;
																	/* make sure that the string doesn't extend
																		outside the window */
																	if (x_string-bounds.lbearing<0)
																	{
																		x_string=bounds.lbearing;
																	}
																	else
																	{
																		if (x_string+bounds.rbearing>drawing_width)
																		{
																			x_string=drawing_width-bounds.rbearing;
																		}
																	}
																	if (y_string-ascent<0)
																	{
																		y_string=ascent;
																	}
																	else
																	{
																		if (y_string+descent>drawing_height)
																		{
																			y_string=drawing_height-descent;
																		}
																	}
																	XPSDrawString(display,pixel_map,
																		(draw_element_data->graphics_context).
																		text_colour,x_string,y_string,name,
																		name_length);
																}
															}
															else
															{
																display_message(ERROR_MESSAGE,
																	"draw_element.  Error projecting (0.5,0.5)");
																return_code=0;
															}
														} break;
														case SHOW_ELEMENT_BOUNDARIES_ONLY:
														{
															/* do nothing */
														} break;
														default:
														{
															display_message(ERROR_MESSAGE,
																"draw_element.  Invalid elements_option");
															return_code=0;
														} break;
													}
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"draw_element.  Error projecting (0,1)");
												return_code=0;
											}
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"draw_element.  Error projecting (1,1)");
										return_code=0;
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"draw_element.  Error projecting (1,0)");
								return_code=0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"draw_element.  Error projecting (0,0)");
						return_code=0;
					}
					return_code=1;
#if defined (OLD_CODE)
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"draw_element.  Insufficient memory for basis function values");
					return_code=0;
				}
#endif /* defined (OLD_CODE) */
				/* tidy up */
				clear_FE_element_field_values(&coordinate_element_field_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"draw_element.  Coordinate field not defined for element");
				return_code=1;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_element */

/*
Global functions
----------------
*/
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
struct Projection_drawing *create_Projection_drawing(Widget widget,int width,
	int height,char create_image,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
This function allocates memory for a drawing and initializes the fields to the
specified values.  It returns a pointer to the created drawing if successful
and NULL if unsuccessful.
???DB.  Came from Drawing in emap
==============================================================================*/
{
	char *image_data;
	int bit_map_pad,bit_map_unit,scan_line_bytes;
	struct Projection_drawing *drawing;

	ENTER(create_Projection_drawing);
	if (widget&&user_interface)
	{
		/* create drawing */
		if (ALLOCATE(drawing,struct Projection_drawing,1))
		{
			drawing->widget=widget;
			drawing->width=width;
			drawing->height=height;
			drawing->user_interface=user_interface;
			/* create pixel map */
			XtVaGetValues(widget,
				XmNdepth,&(drawing->depth),
				NULL);
			if (drawing->pixel_map=XCreatePixmap(User_interface_get_display(user_interface),
				XRootWindow(User_interface_get_display(user_interface),
				XDefaultScreen(User_interface_get_display(user_interface))),(unsigned int)width,
				(unsigned int)height,(unsigned int)(drawing->depth)))
			{
				if (create_image)
				{
					/* the number of bits for each pixel */
					bit_map_unit=BitmapUnit(User_interface_get_display(user_interface));
					/* each scan line occupies a multiple of this number of bits */
					bit_map_pad=BitmapPad(User_interface_get_display(user_interface));
					/* create image */
					if (!((ALLOCATE(image_data,char,drawing->height*
						(scan_line_bytes=(((drawing->width*bit_map_unit-1)/
						bit_map_pad+1)*bit_map_pad-1)/8+1)))&&
						(drawing->image=XCreateImage(User_interface_get_display(user_interface),
						XDefaultVisual(User_interface_get_display(user_interface),
						XDefaultScreen(User_interface_get_display(user_interface))),drawing->depth,
						ZPixmap,0,image_data,drawing->width,drawing->height,bit_map_pad,
						scan_line_bytes))))
					{
						display_message(ERROR_MESSAGE,
							"create_Drawing.  Could not create image");
						DEALLOCATE(image_data);
						drawing->image=(XImage *)NULL;
					}
				}
				else
				{
					/* do not create image */
					drawing->image=(XImage *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Projection_drawing.  Could not create pixel map");
				DEALLOCATE(drawing);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Projection_drawing.  Could not allocate drawing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Projection_drawing.  Invalid argument(s)");
		drawing=(struct Projection_drawing *)NULL;
	}
	LEAVE;

	return (drawing);
} /* create_Projection_drawing */

int destroy_Projection_drawing(struct Projection_drawing **drawing)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
This function frees the memory associated with the fields of <**drawing>, frees
the memory for <**drawing> and changes <*drawing> to NULL.
???DB.  Came from Drawing in emap
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Projection_drawing);
	return_code=1;
	if (*drawing)
	{
		/* free the pixel map */
		XFreePixmap((*drawing)->User_interface_get_display(user_interface),(*drawing)->pixel_map);
		/* free the image */
		if ((*drawing)->image)
		{
			DEALLOCATE((*drawing)->image->data);
			XFree((char *)((*drawing)->image));
		}
		/* free the drawing structure */
		DEALLOCATE(*drawing);
	}
	LEAVE;

	return (return_code);
} /* destroy_Projection_drawing */
#endif /* defined (OLD_CODE) */

struct Projection *CREATE(Projection)(Widget window,
	struct FE_field *coordinate_field,struct FE_field *fibre_field,
	struct FE_field *field,int field_component,
	struct GROUP(FE_element) *element_group,FE_value xi_3,
	enum Colour_option colour_option,enum Contours_option contours_option,
	enum Nodes_option nodes_option,enum Elements_option elements_option,
	enum Fibres_option fibres_option,enum Landmarks_option landmarks_option,
	enum Extrema_option extrema_option,int maintain_aspect_ratio,
	int print_spectrum,enum Projection_type projection_type,
	enum Contour_thickness contour_thickness,struct Spectrum *spectrum,
	int seed_point_discretization,int element_line_discretization,
	struct MANAGER(FE_element) *element_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 18 October 1997

DESCRIPTION :
This function allocates memory for a projection and initializes the fields to
the specified values.  It returns a pointer to the created projection if
successful and NULL if not successful.
==============================================================================*/
{
	Display *display;
	int number_of_spectrum_colours;
	Pixel *spectrum_pixels;
	Projection_settings projection_settings;
#define XmNboundaryColour "boundaryColour"
#define XmCBoundaryColour "BoundaryColour"
#define XmNcontourColour "contourColour"
#define XmCContourColour "ContourColour"
#define XmNdrawingBackgroundColour "drawingBackgroundColour"
#define XmCDrawingBackgroundColour "DrawingBackgroundColour"
#define XmNdrawingFont "drawingFont"
#define XmCDrawingFont "DrawingFont"
#define XmNfibreColour "fibreColour"
#define XmCFibreColour "FibreColour"
#define XmNlandmarkColour "landmarkColour"
#define XmCLandmarkColour "LandmarkColour"
#define XmNnodeMarkerColour "nodeMarkerColour"
#define XmCNodeMarkerColour "NodeMarkerColour"
#define XmNnodeTextColour "nodeTextColour"
#define XmCNodeTextColour "NodeTextColour"
#define XmNpixelsBetweenContourValues "pixelsBetweenContourValues"
#define XmCPixelsBetweenContourValues "PixelsBetweenContourValues"
#define XmNspectrumMarkerColour "spectrumMarkerColour"
#define XmCSpectrumMarkerColour "SpectrumMarkerColour"
#define XmNspectrumTextColour "spectrumTextColour"
#define XmCSpectrumTextColour "SpectrumTextColour"
	static XtResource resources[]=
	{
		{
			XmNboundaryColour,
			XmCBoundaryColour,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Projection_settings,boundary_colour),
			XmRString,
			"black"
		},
		{
			XmNcontourColour,
			XmCContourColour,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Projection_settings,contour_colour),
			XmRString,
			"black"
		},
		{
			XmNdrawingBackgroundColour,
			XmCDrawingBackgroundColour,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Projection_settings,background_drawing_colour),
			XmRString,
			"lightgray"
		},
		{
			XmNdrawingFont,
			XmCDrawingFont,
			XmRFont,
			sizeof(Font),
			XtOffsetOf(Projection_settings,drawing_font),
			XmRString,
			"*-Helvetica-medium-R-*--*-120-*"
		},
		{
			XmNfibreColour,
			XmCFibreColour,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Projection_settings,fibre_colour),
			XmRString,
			"black"
		},
		{
			XmNlandmarkColour,
			XmCLandmarkColour,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Projection_settings,landmark_colour),
			XmRString,
			"black"
		},
		{
			XmNnodeMarkerColour,
			XmCNodeMarkerColour,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Projection_settings,node_marker_colour),
			XmRString,
			"black"
		},
		{
			XmNnodeTextColour,
			XmCNodeTextColour,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Projection_settings,node_text_colour),
			XmRString,
			"black"
		},
		{
			XmNspectrumMarkerColour,
			XmCSpectrumMarkerColour,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Projection_settings,spectrum_marker_colour),
			XmRString,
			"black"
		},
		{
			XmNspectrumTextColour,
			XmCSpectrumTextColour,
			XmRString,
			sizeof(char *),
			XtOffsetOf(Projection_settings,spectrum_text_colour),
			XmRString,
			"black"
		},
		{
			XmNpixelsBetweenContourValues,
			XmCPixelsBetweenContourValues,
			XmRInt,
			sizeof(int),
			XtOffsetOf(Projection_settings,pixels_between_contour_values),
			XmRString,
			"100"
		}
	};
	struct Projection *projection;
	unsigned long mask;
	Window root_window;
	XGCValues values;

	ENTER(CREATE(Projection));
	/* check arguments */
	if (spectrum&&window&&(2<=seed_point_discretization)&&element_manager&&
		user_interface)
	{
		/* allocate memory */
		if (ALLOCATE(projection,struct Projection,1))
		{
			projection->user_interface=user_interface;
			projection->element_manager=element_manager;
			display=User_interface_get_display(user_interface);
			XtVaGetValues(window,XtNcolormap,&(projection->colour_map),NULL);
			if (projection->colour_map)
			{
				if (XAllocColorCells(display,projection->colour_map,False,&mask,0,
					&((projection->pixel).background_colour),1)&&
					XAllocColorCells(display,projection->colour_map,False,&mask,0,
					&((projection->pixel).boundary_colour),1)&&
					XAllocColorCells(display,projection->colour_map,False,&mask,0,
					&((projection->pixel).contour_colour),1)&&
					XAllocColorCells(display,projection->colour_map,False,&mask,0,
					&((projection->pixel).fibre_colour),1)&&
					XAllocColorCells(display,projection->colour_map,False,&mask,0,
					&((projection->pixel).landmark_colour),1)&&
					XAllocColorCells(display,projection->colour_map,False,&mask,0,
					&((projection->pixel).node_marker_colour),1)&&
					XAllocColorCells(display,projection->colour_map,False,&mask,0,
					&((projection->pixel).node_text_colour),1)&&
					XAllocColorCells(display,projection->colour_map,False,&mask,0,
					&((projection->pixel).spectrum_marker_colour),1)&&
					XAllocColorCells(display,projection->colour_map,False,&mask,0,
					&((projection->pixel).spectrum_text_colour),1))
				{
					if (ALLOCATE(spectrum_pixels,Pixel,MAX_SPECTRUM_COLOURS))
					{
						number_of_spectrum_colours=MAX_SPECTRUM_COLOURS;
						while ((number_of_spectrum_colours>0)&&!XAllocColorCells(display,
							projection->colour_map,False,&mask,0,spectrum_pixels,
							number_of_spectrum_colours))
						{
							number_of_spectrum_colours--;
						}
						if ((number_of_spectrum_colours>0)&&
							REALLOCATE((projection->pixel).spectrum,spectrum_pixels,Pixel,
							number_of_spectrum_colours))
						{
							/* retrieve the settings */
							XtVaGetApplicationResources(window,&projection_settings,
								resources,XtNumber(resources),NULL);
							if (projection->font=XQueryFont(display,
								projection_settings.drawing_font))
							{
								/* assign fields */
								if (coordinate_field)
								{
									projection->coordinate_field=
										ACCESS(FE_field)(coordinate_field);
								}
								else
								{
									projection->coordinate_field=(struct FE_field *)NULL;
								}
								if (fibre_field)
								{
									projection->fibre_field=ACCESS(FE_field)(fibre_field);
								}
								else
								{
									projection->fibre_field=(struct FE_field *)NULL;
								}
								if (field)
								{
									projection->field=ACCESS(FE_field)(field);
								}
								else
								{
									projection->field=(struct FE_field *)NULL;
								}
								projection->field_component=field_component;
								if (element_group)
								{
									projection->element_group=
										ACCESS(GROUP(FE_element))(element_group);
								}
								else
								{
									projection->element_group=(struct GROUP(FE_element) *)NULL;
								}
								projection->xi_3=xi_3;
								projection->colour_option=colour_option;
								projection->contours_option=contours_option;
								projection->nodes_option=nodes_option;
								projection->elements_option=elements_option;
								projection->fibres_option=fibres_option;
								projection->landmarks_option=landmarks_option;
								projection->extrema_option=extrema_option;
								projection->maintain_aspect_ratio=maintain_aspect_ratio;
								projection->print_spectrum=print_spectrum;
								projection->type=projection_type;
								projection->contour_thickness=contour_thickness;
								projection->number_of_nodes=0;
								projection->nodes=(struct FE_node **)NULL;
								projection->node_x=(int *)NULL;
								projection->node_y=(int *)NULL;
								projection->node_drawn=(char *)NULL;
								projection->node_depth=(FE_value *)NULL;
								projection->contour_minimum=1;
								projection->contour_maximum=0;
								projection->number_of_contours=2;
								projection->contour_x=(short int *)NULL;
								projection->contour_y=(short int *)NULL;
								projection->pixel_values=(FE_value *)NULL;
								projection->pixel_depths=(FE_value *)NULL;
								projection->pixel_fibre_angles=(FE_value *)NULL;
								projection->number_of_spectrum_colours=
									number_of_spectrum_colours;
								projection->seed_point_discretization=seed_point_discretization;
								projection->element_line_discretization=
									element_line_discretization;
								projection->spectrum=spectrum;
								projection->expand_spectrum=1;
								XStoreNamedColor(display,projection->colour_map,
									projection_settings.background_drawing_colour,
									(projection->pixel).background_colour,DoRed|DoGreen|DoBlue);
								XStoreNamedColor(display,projection->colour_map,
									projection_settings.boundary_colour,
									(projection->pixel).boundary_colour,DoRed|DoGreen|DoBlue);
								XStoreNamedColor(display,projection->colour_map,
									projection_settings.contour_colour,
									(projection->pixel).contour_colour,DoRed|DoGreen|DoBlue);
								XStoreNamedColor(display,projection->colour_map,
									projection_settings.fibre_colour,
									(projection->pixel).fibre_colour,DoRed|DoGreen|DoBlue);
								XStoreNamedColor(display,projection->colour_map,
									projection_settings.landmark_colour,
									(projection->pixel).landmark_colour,DoRed|DoGreen|DoBlue);
								XStoreNamedColor(display,projection->colour_map,
									projection_settings.node_marker_colour,
									(projection->pixel).node_marker_colour,DoRed|DoGreen|DoBlue);
								XStoreNamedColor(display,projection->colour_map,
									projection_settings.node_text_colour,
									(projection->pixel).node_text_colour,DoRed|DoGreen|DoBlue);
								XStoreNamedColor(display,projection->colour_map,
									projection_settings.spectrum_marker_colour,
									(projection->pixel).spectrum_marker_colour,
									DoRed|DoGreen|DoBlue);
								XStoreNamedColor(display,projection->colour_map,
									projection_settings.spectrum_text_colour,
									(projection->pixel).spectrum_text_colour,
									DoRed|DoGreen|DoBlue);
								projection->print=0;
								root_window=XRootWindow(display,XDefaultScreen(display));
								mask=GCLineStyle|GCBackground|GCFont|GCForeground|GCFunction;
								values.font=projection_settings.drawing_font;
								values.background=(projection->pixel).background_colour;
								values.foreground=(projection->pixel).background_colour;
								values.function=GXcopy;
								values.line_style=LineSolid;
								(projection->graphics_context).background_colour=XCreateGC(
									display,root_window,mask,&values);
								values.foreground=(projection->pixel).contour_colour;
								(projection->graphics_context).contour_colour=XCreateGC(
									display,root_window,mask,&values);
								values.foreground=(projection->pixel).fibre_colour;
								(projection->graphics_context).fibre_colour=XCreateGC(
									display,root_window,mask,&values);
								(projection->graphics_context).copy=XCreateGC(
									display,root_window,GCFunction,&values);
								values.foreground=(projection->pixel).node_marker_colour;
								(projection->graphics_context).node_marker_colour=XCreateGC(
									display,root_window,mask,&values);
								values.foreground=(projection->pixel).node_text_colour;
								(projection->graphics_context).node_text_colour=XCreateGC(
									display,root_window,mask,&values);
								(projection->graphics_context).spectrum=XCreateGC(
									display,root_window,mask,&values);
								values.foreground=(projection->pixel).spectrum_marker_colour;
								(projection->graphics_context).spectrum_marker_colour=XCreateGC(
									display,root_window,mask,&values);
								values.foreground=(projection->pixel).spectrum_text_colour;
								(projection->graphics_context).spectrum_text_colour=XCreateGC(
									display,root_window,mask,&values);
								projection->border_thickness_pixels=5;
								projection->pixels_between_contour_values=
									projection_settings.pixels_between_contour_values;
								projection->maximum=0;
								projection->maximum_x= -1;
								projection->maximum_y= -1;
								projection->minimum=0;
								projection->minimum_x= -1;
								projection->minimum_y= -1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(Projection).  Could not find drawing font");
								DEALLOCATE((projection->pixel).spectrum);
								XFreeColormap(display,projection->colour_map);
								DEALLOCATE(projection);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Projection).  Could not allocate spectrum pixels");
							DEALLOCATE(spectrum_pixels);
							XFreeColormap(display,projection->colour_map);
							DEALLOCATE(projection);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Projection).  Could not allocate spectrum pixel memory");
						XFreeColormap(display,projection->colour_map);
						DEALLOCATE(projection);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Projection).  Could not allocate named pixels");
					XFreeColormap(display,projection->colour_map);
					DEALLOCATE(projection);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Projection).  Could not create colour map");
				DEALLOCATE(projection);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Projection).  Could not allocate projection");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Projection).  Invalid argument(s)");
		projection=(struct Projection *)NULL;
	}
	LEAVE;

	return (projection);
} /* CREATE(Projection) */

int DESTROY(Projection)(struct Projection **projection)
/*******************************************************************************
LAST MODIFIED : 8 October 1996

DESCRIPTION :
This function deaccesses the fields of <**projection>, deallocates the memory
for <**projection> and sets <*projection> to NULL.
==============================================================================*/
{
	Display *display;
	int i,return_code;
	struct FE_node **node;

	ENTER(DESTROY(Projection));
	return_code=1;
	if (projection&&(*projection))
	{
		display=User_interface_get_display((*projection)->user_interface);
		XFreeGC(display,((*projection)->graphics_context).background_colour);
		XFreeGC(display,((*projection)->graphics_context).contour_colour);
		XFreeGC(display,((*projection)->graphics_context).copy);
		XFreeGC(display,((*projection)->graphics_context).node_marker_colour);
		XFreeGC(display,((*projection)->graphics_context).node_text_colour);
		XFreeGC(display,((*projection)->graphics_context).spectrum);
		XFreeGC(display,((*projection)->graphics_context).spectrum_marker_colour);
		XFreeGC(display,((*projection)->graphics_context).spectrum_text_colour);
		XFreeColormap(display,(*projection)->colour_map);
		DEALLOCATE(((*projection)->pixel).spectrum);
		DEACCESS(FE_field)(&((*projection)->coordinate_field));
		DEACCESS(FE_field)(&((*projection)->field));
		DEACCESS(GROUP(FE_element))(&((*projection)->element_group));
		node=(*projection)->nodes;
		for (i=(*projection)->number_of_nodes;i>0;i--)
		{
			DEACCESS(FE_node)(node);
			node++;
		}
		DEALLOCATE((*projection)->nodes);
		DEALLOCATE((*projection)->node_x);
		DEALLOCATE((*projection)->node_y);
		DEALLOCATE((*projection)->node_drawn);
		DEALLOCATE((*projection)->node_depth);
		DEALLOCATE((*projection)->contour_x);
		DEALLOCATE((*projection)->contour_y);
		DEALLOCATE((*projection)->pixel_values);
		DEALLOCATE((*projection)->pixel_depths);
		DEALLOCATE(*projection);
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Projection) */

int update_colour_map(struct Projection *projection)
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
Updates the colour map being used for <projection>.
==============================================================================*/
{
	float blue,contour_maximum,contour_minimum,green,maximum_value,minimum_value,
		red,scale,theta;
	int cell_number,end_cell,i,number_of_contours,number_of_spectrum_colours,
		return_code,start_cell;
	XColor colour,spectrum_rgb[MAX_SPECTRUM_COLOURS];
	Colormap colour_map;
	Pixel boundary_pixel,*spectrum_pixels;

	ENTER (update_colour_map);
	if (projection)
	{
		return_code=1;
		minimum_value=get_Spectrum_minimum(projection->spectrum);
		maximum_value=get_Spectrum_maximum(projection->spectrum);
		contour_minimum=projection->contour_minimum;
		contour_maximum=projection->contour_maximum;
		number_of_spectrum_colours=projection->number_of_spectrum_colours;
		colour_map=projection->colour_map;
		boundary_pixel=(projection->pixel).boundary_colour;
		spectrum_pixels=(projection->pixel).spectrum;
		if (maximum_value==minimum_value)
		{
			start_cell=0;
			end_cell=number_of_spectrum_colours;
		}
		else
		{
			start_cell=(int)((contour_minimum-minimum_value)/
				(maximum_value-minimum_value)*
				(float)(number_of_spectrum_colours-1)+0.5);
			end_cell=(int)((contour_maximum-minimum_value)/
				(maximum_value-minimum_value)*
				(float)(number_of_spectrum_colours-1)+0.5);
		}
		scale=(contour_maximum-contour_minimum)/(float)(end_cell-start_cell);
		/* adjust the computer colour map for colour map */
		if (projection->colour_option==SHOW_COLOUR)
		{
			spectrum_value_to_rgb(projection->spectrum,/* number_of_data_components */1,
				&contour_minimum,&red,&green,&blue);
			for (i=0;i<=start_cell;i++)
			{
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				spectrum_rgb[i].red=(unsigned short)(red*65535.);
				spectrum_rgb[i].green=(unsigned short)(green*65535.);
				spectrum_rgb[i].blue=(unsigned short)(blue*65535.);
			}
			for (i=start_cell+1;i<end_cell;i++)
			{
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				theta=contour_minimum+(float)(i-start_cell)*scale;
				spectrum_value_to_rgb(projection->spectrum,/* number_of_data_components */1,
					&theta,&red,&green,&blue);
				spectrum_rgb[i].red=(unsigned short)(red*65535.);
				spectrum_rgb[i].green=(unsigned short)(green*65535.);
				spectrum_rgb[i].blue=(unsigned short)(blue*65535.);
			}
			spectrum_value_to_rgb(projection->spectrum,/* number_of_data_components */1,
				&contour_maximum,&red,&green,&blue);
			for (i=end_cell;i<number_of_spectrum_colours;i++)
			{
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				spectrum_rgb[i].red=(unsigned short)(red*65535.);
				spectrum_rgb[i].green=(unsigned short)(green*65535.);
				spectrum_rgb[i].blue=(unsigned short)(blue*65535.);
			}
			/* hide the map boundary */
			colour.pixel=(projection->pixel).background_colour;
			XQueryColor(User_interface_get_display(projection->user_interface),colour_map,&colour);
			colour.pixel=boundary_pixel;
			colour.flags=DoRed|DoGreen|DoBlue;
			XStoreColor(User_interface_get_display(projection->user_interface),colour_map,&colour);
		}
		else
		{
			/* use background drawing colour for the whole spectrum */
			colour.pixel=(projection->pixel).background_colour;
			XQueryColor(User_interface_get_display(projection->user_interface),colour_map,&colour);
			for (i=0;i<number_of_spectrum_colours;i++)
			{
				spectrum_rgb[i].pixel=spectrum_pixels[i];
				spectrum_rgb[i].flags=DoRed|DoGreen|DoBlue;
				spectrum_rgb[i].red=colour.red;
				spectrum_rgb[i].blue=colour.blue;
				spectrum_rgb[i].green=colour.green;
			}
			/* show the map boundary */
			colour.pixel=(projection->pixel).contour_colour;
			XQueryColor(User_interface_get_display(projection->user_interface),colour_map,&colour);
			colour.pixel=(projection->pixel).boundary_colour;
			colour.flags=DoRed|DoGreen|DoBlue;
			XStoreColor(User_interface_get_display(projection->user_interface),colour_map,&colour);
		}
		/* adjust the computer colour map for contours */
		if (SHOW_CONTOURS==projection->contours_option)
		{
			if ((VARIABLE_THICKNESS==projection->contour_thickness)||
				!(projection->pixel_values))
			{
				colour.pixel=(projection->pixel).contour_colour;
				XQueryColor(User_interface_get_display(projection->user_interface),colour_map,&colour);
				number_of_contours=projection->number_of_contours;
				for (i=0;i<number_of_contours;i++)
				{
					cell_number=(int)(((contour_maximum*(float)i+contour_minimum*
						(float)(number_of_contours-1-i))/(float)(number_of_contours-1)-
						minimum_value)/(maximum_value-minimum_value)*
						(float)(number_of_spectrum_colours-1)+0.5);
					spectrum_rgb[cell_number].pixel=spectrum_pixels[cell_number];
					spectrum_rgb[cell_number].flags=DoRed|DoGreen|DoBlue;
					spectrum_rgb[cell_number].red=colour.red;
					spectrum_rgb[cell_number].blue=colour.blue;
					spectrum_rgb[cell_number].green=colour.green;
				}
			}
		}
		XStoreColors(User_interface_get_display(projection->user_interface),colour_map,spectrum_rgb,
			number_of_spectrum_colours);
	}
	else
	{
		return_code=0;
		display_message(ERROR_MESSAGE,"update_colour_map.  Missing projection");
	}
	LEAVE;

	return (return_code);
} /* update_colour_map */

int draw_projection(struct Projection *projection,int recalculate,
	struct Drawing_2d *drawing)
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
	struct Projection_drawing *drawing)
#endif /* defined (OLD_CODE) */
/*******************************************************************************
LAST MODIFIED : 18 October 1997

DESCRIPTION :
This function draws the <projection> in the <drawing>.  If <recalculate> is >0
then the colours for the pixels are recalculated.  If <recalculate> is >1 then
the projection is also recalculated.
==============================================================================*/
{
	char draw_boundary,draw_contours,draw_contour_value,*node_drawn,
		value_string[11];
	Display *display;
	float a,b,contour_maximum,contour_minimum,contour_step,fibre_angle,f_i_j,
		f_i_jm1,f_im1_j,f_im1_jm1,max_f,maximum_value,max_x,max_y,min_f,
		minimum_value,min_x,min_y,*node_depth,pi,pi_over_2,pixel_aspect_ratio,
		*pixel_depth,*pixel_fibre_angle,*pixel_value,range_f,stretch_x,stretch_y,
		temp_value,*x,*y;
	int ascent,boundary_type,cell_number,cell_range,contour_area,
		contour_areas_in_x,contour_areas_in_y,contour_x_spacing,contour_y_spacing,
		descent,direction,drawing_height,drawing_width,end,fibre_spacing,fibre_x,
		fibre_y,i,index,j,k,maximum_x,maximum_y,minimum_x,minimum_y,*node_x,
		*node_y,number_of_contour_areas,number_of_contours,next_contour_x,
		number_of_nodes,number_of_spectrum_colours,pixel_left,pixel_top,return_code,
		start,start_x,start_y,string_length,temp_int,valid_i_j,valid_i_jm1,
		valid_im1_j,valid_im1_jm1,x_border,x_offset,x_pixel,x_separation,y_border,
		y_offset,y_pixel,y_separation;
	Pixel background_pixel,boundary_pixel,pixel,*spectrum_pixels;
	short int *contour_x,*contour_y;
	struct Draw_element_data draw_element_data;
	struct Draw_node_data draw_node_data;
	struct FE_node **nodes;
	struct LIST(FE_node) *node_list;
	struct Project_element_data project_element_data;
	struct Project_node_data project_node_data;
	XCharStruct bounds;
	XImage *image;

	ENTER(draw_projection);
	return_code=1;
	/* check arguments */
	if (projection&&drawing)
	{
		if (node_list=CREATE_LIST(FE_node)())
		{
			display=User_interface_get_display(drawing->user_interface);
			number_of_spectrum_colours=projection->number_of_spectrum_colours;
			number_of_contours=projection->number_of_contours;
			drawing_width=drawing->width;
			drawing_height=drawing->height;
			background_pixel=(projection->pixel).background_colour;
			boundary_pixel=(projection->pixel).boundary_colour;
			spectrum_pixels=(projection->pixel).spectrum;
			/* clear the projection drawing area (not needed for PostScript) */
			XPSFillRectangle(display,drawing->pixel_map,
				(projection->graphics_context).background_colour,
				0,0,drawing_width,drawing_height);
			pi_over_2=2*atan(1);
			pi=2*pi_over_2;
			/* count the nodes */
			if (projection->element_group)
			{
				return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
					add_element_nodes_to_list,(void *)(node_list),
					projection->element_group);
			}
			else
			{
#if defined (OLD_CODE)
				return_code=FOR_EACH_OBJECT_IN_LIST(FE_element)(
					add_element_nodes_to_list,(void *)(node_list),
					all_FE_element);
#endif /* defined (OLD_CODE) */
	      return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
					add_element_nodes_to_list,(void *)(node_list),
					projection->element_manager);
			}
			if (return_code&&(0<(number_of_nodes=NUMBER_IN_LIST(FE_node)(node_list))))
			{
				/* allocate memory for the nodes */
				ALLOCATE(nodes,struct FE_node *,number_of_nodes);
				ALLOCATE(node_drawn,char,number_of_nodes);
				ALLOCATE(node_x,int,number_of_nodes);
				ALLOCATE(node_y,int,number_of_nodes);
				ALLOCATE(x,float,number_of_nodes);
				ALLOCATE(y,float,number_of_nodes);
				ALLOCATE(node_depth,float,number_of_nodes);
				if (nodes&&node_drawn&&node_x&&node_y&&node_depth&&x&&y)
				{
					/* calculate the projections of the node positions and the x and y
						ranges */
					project_node_data.first=1;
					project_node_data.projection_type=projection->type;
					project_node_data.node=nodes;
					project_node_data.node_x=x;
					project_node_data.node_y=y;
					project_node_data.node_depth=node_depth;
					if (!(project_node_data.coordinate_field=
						projection->coordinate_field))
					{
						if (projection->element_group)
						{
							if (!FIRST_OBJECT_IN_GROUP_THAT(FE_element)(find_coordinate_field,
								(void *)&(project_node_data.coordinate_field),projection->
								element_group))
							{
								return_code=0;
							}
						}
						else
						{
#if defined (OLD_CODE)
							if (!FIRST_OBJECT_IN_LIST_THAT(FE_element)(find_coordinate_field,
								(void *)&(project_node_data.coordinate_field),all_FE_element))
#endif /* defined (OLD_CODE) */
	            if (!FIRST_OBJECT_IN_MANAGER_THAT(FE_element)(
								find_coordinate_field,
								(void *)&(project_node_data.coordinate_field),
								projection->element_manager))
							{
								return_code=0;
							}
						}
					}
					if (return_code&&FOR_EACH_OBJECT_IN_LIST(FE_node)(project_node,
						(void *)&(project_node_data),node_list))
					{
						/* free existing memory */
						DEALLOCATE(projection->nodes);
						DEALLOCATE(projection->node_drawn);
						DEALLOCATE(projection->node_x);
						DEALLOCATE(projection->node_y);
						DEALLOCATE(projection->node_depth);
						projection->number_of_nodes=number_of_nodes;
						projection->nodes=nodes;
						projection->node_drawn=node_drawn;
						projection->node_x=node_x;
						projection->node_y=node_y;
						projection->node_depth=node_depth;
						switch (projection->type)
						{
							case HAMMER_PROJECTION:
							{
								min_x= -1;
								max_x=1;
								min_y= -1;
								max_y=project_node_data.max_y;
							} break;
							case POLAR_PROJECTION:
							{
#if defined (OLD_CODE)
								max_x=project_node_data.max_x;
#endif /* defined (OLD_CODE) */
								max_x=fabs(project_node_data.max_x);
								min_x=fabs(project_node_data.min_x);
								if (min_x>max_x)
								{
									max_x=min_x;
								}
								max_y=fabs(project_node_data.max_y);
								min_y=fabs(project_node_data.min_y);
								if (min_y>max_y)
								{
									max_y=min_y;
								}
								if (max_y>max_x)
								{
									max_x=max_y;
								}
								min_x= -max_x;
								min_y=min_x;
								max_y=max_x;
							} break;
							case CYLINDRICAL_PROJECTION:
							{
								min_x= -pi;
								max_x=pi;
								min_y=project_node_data.min_y;
								max_y=project_node_data.max_y;
							} break;
						}
						x_border=4;
						y_border=4;
						/* calculate the transformation from projection coordinates to
							screen coordinates */
						/* allow room to write the node names */
						XTextExtents(projection->font,"H",1,&direction,&ascent,&descent,
							&bounds);
						if ((projection->maintain_aspect_ratio)&&(max_x!=min_x)&&
							(drawing_width>2*x_border+1)&&(max_y!=min_y)&&
							(drawing_height>2*y_border+ascent+descent+1))
						{
							pixel_aspect_ratio=get_pixel_aspect_ratio(display);
							if ((float)((max_y-min_y)*(drawing_width-(2*x_border+1)))<
								(float)((max_x-min_x)*(drawing_height-
								(2*y_border+ascent+descent+1)))*pixel_aspect_ratio)
							{
								/* fill width */
								start_x=x_border;
								stretch_x=((float)(drawing_width-(2*x_border+1)))/(max_x-min_x);
								stretch_y=stretch_x/pixel_aspect_ratio;
								start_y=(drawing_height+(int)((max_y-min_y)*stretch_y))/2;
							}
							else
							{
								/* fill height */
								start_y=drawing_height-(y_border+1);
								stretch_y=((float)(drawing_height-
									(2*y_border+ascent+descent+1)))/(max_y-min_y);
								stretch_x=stretch_y*pixel_aspect_ratio;
								start_x=(drawing_width-(int)((max_x-min_x)*stretch_x))/2;
							}
						}
						else
						{
							if ((max_x==min_x)||(drawing_width<=2*x_border+1))
							{
								start_x=drawing_width/2;
								stretch_x=0;
							}
							else
							{
								start_x=x_border;
								stretch_x=((float)(drawing_width-(2*x_border+1)))/(max_x-min_x);
							}
							if ((max_y==min_y)||(drawing_height<=2*y_border+ascent+descent+1))
							{
								start_y=drawing_height/2;
								stretch_y=0;
							}
							else
							{
								start_y=drawing_height-(y_border+1);
								stretch_y=((float)(drawing_height-
									(2*y_border+ascent+descent+1)))/(max_y-min_y);
							}
						}
						if ((projection->field)||(projection->fibre_field))
						{
							/* construct a colour map image for colour map or contours or
								values */
							/* draw colour map and contours first (background) */
							if (image=drawing->image)
							{
								if (recalculate>0)
								{
									/* allocate memory for drawing the projection boundary and
										drawing contour values */
									contour_x_spacing=projection->pixels_between_contour_values;
									contour_areas_in_x=drawing_width/contour_x_spacing;
									if (contour_areas_in_x<1)
									{
										contour_areas_in_x=1;
									}
									contour_x_spacing=drawing_width/contour_areas_in_x;
									if (contour_x_spacing*contour_areas_in_x<drawing_width)
									{
										if ((contour_x_spacing+1)*(contour_areas_in_x-1)<
											(contour_x_spacing-1)*(contour_areas_in_x+1))
										{
											contour_x_spacing++;
										}
										else
										{
											contour_areas_in_x++;
										}
									}
									contour_y_spacing=projection->pixels_between_contour_values;
									contour_areas_in_y=drawing_height/contour_y_spacing;
									if (contour_areas_in_y<1)
									{
										contour_areas_in_y=1;
									}
									contour_y_spacing=drawing_height/contour_areas_in_y;
									if (contour_y_spacing*contour_areas_in_y<drawing_height)
									{
										if ((contour_y_spacing+1)*(contour_areas_in_y-1)<
											(contour_y_spacing-1)*(contour_areas_in_y+1))
										{
											contour_y_spacing++;
										}
										else
										{
											contour_areas_in_y++;
										}
									}
									number_of_contour_areas=contour_areas_in_x*contour_areas_in_y;
									if (ALLOCATE(contour_x,short int,number_of_contour_areas*
										number_of_spectrum_colours)&&ALLOCATE(contour_y,short int,
										number_of_contour_areas*number_of_spectrum_colours))
									{
										/* free memory for drawing contour values */
										DEALLOCATE(projection->contour_x);
										DEALLOCATE(projection->contour_y);
										projection->number_of_contour_areas=number_of_contour_areas;
										projection->number_of_contour_areas_in_x=contour_areas_in_x;
										projection->contour_x=contour_x;
										projection->contour_y=contour_y;
										/* initialize the contour areas */
										for (i=number_of_contour_areas*number_of_spectrum_colours;
											i>0;i--)
										{
											*contour_x= -1;
											*contour_y= -1;
											contour_x++;
											contour_y++;
										}
										/* calculate the interpolation function(s) and determine the
											range of function values */
										if (!(project_element_data.fibre_field=
											projection->fibre_field))
										{
											if (projection->element_group)
											{
												FIRST_OBJECT_IN_GROUP_THAT(FE_element)(
													find_anatomical_field,
													(void *)&(project_element_data.fibre_field),
													projection->element_group);
											}
											else
											{
#if defined (OLD_CODE)
/*???DB.  temp */
												FIRST_OBJECT_IN_LIST_THAT(FE_element)(
													find_anatomical_field,
													(void *)&(project_element_data.fibre_field),
													all_FE_element);
#endif /* defined (OLD_CODE) */
		                    FIRST_OBJECT_IN_MANAGER_THAT(FE_element)(
													find_anatomical_field,
													(void *)&(project_element_data.fibre_field),
													projection->element_manager);
											}
										}
										if (recalculate>1)
										{
											REALLOCATE(pixel_value,projection->pixel_values,float,
												drawing_width*drawing_height);
											REALLOCATE(pixel_depth,projection->pixel_depths,float,
												drawing_width*drawing_height);
											if (project_element_data.fibre_field)
											{
												REALLOCATE(pixel_fibre_angle,
													projection->pixel_fibre_angles,float,
													drawing_width*drawing_height);
											}
											else
											{
												DEALLOCATE(projection->pixel_fibre_angles);
												pixel_fibre_angle=(float *)NULL;
											}
										}
										else
										{
											pixel_value=projection->pixel_values;
											pixel_depth=projection->pixel_depths;
											pixel_fibre_angle=projection->pixel_fibre_angles;
										}
										if (pixel_value&&pixel_depth&&
											(!(project_element_data.fibre_field)||
											((project_element_data.fibre_field)&&pixel_fibre_angle)))
										{
											if (recalculate>1)
											{
												projection->pixel_values=pixel_value;
												projection->pixel_depths=pixel_depth;
												projection->pixel_fibre_angles=pixel_fibre_angle;
												/* clear the image */
												x_pixel=drawing_width;
												if (pixel_fibre_angle)
												{
													while (x_pixel>0)
													{
														x_pixel--;
														y_pixel=drawing_height;
														while (y_pixel>0)
														{
															y_pixel--;
															XPutPixel(image,x_pixel,y_pixel,
																background_pixel);
															*pixel_depth= -1;
															*pixel_fibre_angle=0;
															pixel_depth++;
															pixel_fibre_angle++;
														}
													}
												}
												else
												{
													while (x_pixel>0)
													{
														x_pixel--;
														y_pixel=drawing_height;
														while (y_pixel>0)
														{
															y_pixel--;
															XPutPixel(image,x_pixel,y_pixel,
																background_pixel);
															*pixel_depth= -1;
															pixel_depth++;
														}
													}
												}
												/* project the elements */
												project_element_data.projection_type=projection->type;
												project_element_data.seed_point_discretization=
													projection->seed_point_discretization;
												project_element_data.xi_3=projection->xi_3;
												project_element_data.min_x=min_x;
												project_element_data.min_y=min_y;
												project_element_data.start_x=start_x;
												project_element_data.start_y=start_y;
												project_element_data.stretch_x=stretch_x;
												project_element_data.stretch_y=stretch_y;
												project_element_data.drawing_width=drawing_width;
												project_element_data.drawing_height=drawing_height;
												project_element_data.field_component_number=
													projection->field_component;
												project_element_data.coordinate_field=
													project_node_data.coordinate_field;
												project_element_data.data_field=projection->field;
												project_element_data.pixel_values=
													projection->pixel_values;
												project_element_data.pixel_depths=
													projection->pixel_depths;
												project_element_data.pixel_fibre_angles=
													projection->pixel_fibre_angles;
												if (projection->element_group)
												{
													return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
														project_element,(void *)(&project_element_data),
														projection->element_group);
												}
												else
												{
#if defined (OLD_CODE)
													return_code=FOR_EACH_OBJECT_IN_LIST(FE_element)(
														project_element,(void *)(&project_element_data),
														all_FE_element);
#endif /* defined (OLD_CODE) */
													return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
														project_element,(void *)(&project_element_data),
														projection->element_manager);
												}
												/* calculate minimum and maximum */
												pixel_value=projection->pixel_values;
												pixel_depth=projection->pixel_depths;
												y_pixel=drawing_height;
												while ((y_pixel>0)&&(*pixel_depth<0))
												{
													y_pixel--;
													x_pixel=drawing_width;
													while ((x_pixel>0)&&(*pixel_depth<0))
													{
														x_pixel--;
														pixel_depth++;
														pixel_value++;
													}
												}
												if ((x_pixel>0)||(y_pixel>0))
												{
													min_f= *pixel_value;
													max_f=min_f;
													minimum_x=x_pixel;
													minimum_y=y_pixel;
													maximum_x=x_pixel;
													maximum_y=y_pixel;
													while (x_pixel>0)
													{
														if (*pixel_depth>=0)
														{
															temp_value= *pixel_value;
															if (temp_value<min_f)
															{
																min_f=temp_value;
																minimum_x=x_pixel;
																minimum_y=y_pixel;
															}
															else
															{
																if (temp_value>max_f)
																{
																	max_f=temp_value;
																	maximum_x=x_pixel;
																	maximum_y=y_pixel;
																}
															}
														}
														x_pixel--;
														pixel_depth++;
														pixel_value++;
													}
													while (y_pixel>0)
													{
														y_pixel--;
														x_pixel=drawing_width;
														while (x_pixel>0)
														{
															if (*pixel_depth>=0)
															{
																temp_value= *pixel_value;
																if (temp_value<min_f)
																{
																	min_f=temp_value;
																	minimum_x=x_pixel;
																	minimum_y=y_pixel;
																}
																else
																{
																	if (temp_value>max_f)
																	{
																		max_f=temp_value;
																		maximum_x=x_pixel;
																		maximum_y=y_pixel;
																	}
																}
															}
															x_pixel--;
															pixel_depth++;
															pixel_value++;
														}
													}
													if (projection->expand_spectrum)
													{
														projection->contour_minimum=min_f;
														projection->contour_maximum=max_f;
														Spectrum_set_minimum_and_maximum(
															projection->spectrum,min_f,max_f);
													}
													if (projection->contour_maximum<
														projection->contour_minimum)
													{
														projection->contour_minimum=
															get_Spectrum_minimum(projection->spectrum);
														projection->contour_maximum=
															get_Spectrum_maximum(projection->spectrum);
													}
													if (max_f<min_f)
													{
														projection->maximum=0;
														projection->maximum_x= -1;
														projection->maximum_y= -1;
														projection->minimum=0;
														projection->minimum_x= -1;
														projection->minimum_y= -1;
													}
													else
													{
														projection->maximum=max_f;
														projection->maximum_x=drawing_width-maximum_x;
														projection->maximum_y=drawing_height-maximum_y;
														projection->minimum=min_f;
														projection->minimum_x=drawing_width-minimum_x;
														projection->minimum_y=drawing_height-minimum_y;
													}
												}
												else
												{
													if (projection->expand_spectrum)
													{
														projection->contour_minimum=0;
														projection->contour_maximum=0;
														Spectrum_set_minimum_and_maximum(
															projection->spectrum,0,0);
													}
													projection->maximum=0;
													projection->maximum_x= -1;
													projection->maximum_y= -1;
													projection->minimum=0;
													projection->minimum_x= -1;
													projection->minimum_y= -1;
												}
											}
											min_f=get_Spectrum_minimum(projection->spectrum);
											max_f=get_Spectrum_maximum(projection->spectrum);
											/* calculate range of values */
											range_f=max_f-min_f;
											if (range_f<=0)
											{
												range_f=1;
											}
											pixel_value=projection->pixel_values;
											pixel_depth=projection->pixel_depths;
											for (y_pixel=0;y_pixel<drawing_height;y_pixel++)
											{
												contour_area=
													(y_pixel/contour_y_spacing)*contour_areas_in_x;
												next_contour_x=contour_x_spacing-1;
												contour_x=(projection->contour_x)+contour_area;
												contour_y=(projection->contour_y)+contour_area;
												for (x_pixel=0;x_pixel<drawing_width;x_pixel++)
												{
													if (*pixel_depth>=0)
													{
														a= *pixel_value;
														if (a<min_f)
														{
															a=min_f;
														}
														else
														{
															if (a>max_f)
															{
																a=max_f;
															}
														}
														cell_number=(int)((a-min_f)*
															(float)(number_of_spectrum_colours-1)/
															range_f+0.5);
														XPutPixel(image,x_pixel,y_pixel,
															spectrum_pixels[cell_number]);
														cell_number *= number_of_contour_areas;
														contour_x[cell_number]=x_pixel;
														contour_y[cell_number]=y_pixel;
													}
													else
													{
														if (((x_pixel>0)&&(pixel_depth[-1]>=0))||
															((x_pixel<drawing_width-1)&&(pixel_depth[1]>=0))||
															((y_pixel>0)&&(pixel_depth[-drawing_width]>=0))||
															((y_pixel<drawing_height-1)&&
															(pixel_depth[drawing_width]>=0)))
														{
															XPutPixel(image,x_pixel,y_pixel,boundary_pixel);
														}
													}
													pixel_value++;
													pixel_depth++;
													if (x_pixel>=next_contour_x)
													{
														contour_x++;
														contour_y++;
														next_contour_x += contour_x_spacing;
													}
												}
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
											"draw_projection.  Insufficient memory for pixel values");
											/*???DB.  Ghost the constant thickness button ? */
											DEALLOCATE(pixel_value);
											DEALLOCATE(pixel_depth);
											DEALLOCATE(projection->pixel_values);
											DEALLOCATE(projection->pixel_depths);
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
			"draw_projection.  Insufficient memory for constant thickness contours");
									}
								}
								update_colour_map(projection);
								if ((CONSTANT_THICKNESS==projection->contour_thickness)&&
									(pixel_value=projection->pixel_values)&&
									(pixel_depth=projection->pixel_depths))
								{
									if (SHOW_COLOUR==projection->colour_option)
									{
										XPSPutImage(display,drawing->pixel_map,
											(projection->graphics_context).copy,image,0,0,0,0,
											drawing_width,drawing_height);
										draw_boundary=0;
									}
									else
									{
										if (projection->print)
										{
											draw_boundary=1;
										}
										else
										{
											XPSPutImage(display,drawing->pixel_map,
												(projection->graphics_context).copy,image,0,0,0,0,
												drawing_width,drawing_height);
											draw_boundary=0;
										}
									}
									if ((SHOW_CONTOURS==projection->contours_option)&&
										((contour_minimum=projection->contour_minimum)<
										(contour_maximum=projection->contour_maximum)))
									{
										draw_contours=1;
									}
									else
									{
										draw_contours=0;
									}
									if (draw_contours||draw_boundary)
									{
										contour_step=(contour_maximum-contour_minimum)/
											(float)(number_of_contours-1);
										for (j=1;j<drawing_height;j++)
										{
											if ((background_pixel==(pixel=XGetPixel(image,0,j-1)))||
												(boundary_pixel==pixel))
											{
												valid_i_jm1=0;
											}
											else
											{
												valid_i_jm1=1;
												f_i_jm1= *pixel_value;
											}
											if ((background_pixel==(pixel=XGetPixel(image,0,j)))||
												(boundary_pixel==pixel))
											{
												valid_i_j=0;
											}
											else
											{
												valid_i_j=1;
												f_i_j=pixel_value[drawing_width];
											}
											pixel_value++;
											for (i=1;i<drawing_width;i++)
											{
												valid_im1_jm1=valid_i_jm1;
												f_im1_jm1=f_i_jm1;
												valid_im1_j=valid_i_j;
												f_im1_j=f_i_j;
												if ((background_pixel==(pixel=XGetPixel(image,i,j-1)))||
													(boundary_pixel==pixel))
												{
													valid_i_jm1=0;
												}
												else
												{
													valid_i_jm1=1;
													f_i_jm1= *pixel_value;
												}
												if ((background_pixel==(pixel=XGetPixel(image,i,j)))||
													(boundary_pixel==pixel))
												{
													valid_i_j=0;
												}
												else
												{
													valid_i_j=1;
													f_i_j=pixel_value[drawing_width];
												}
												pixel_value++;
												boundary_type=((valid_im1_jm1*2+valid_im1_j)*2+
													valid_i_jm1)*2+valid_i_j;
												if (draw_contours&&(15==boundary_type))
												{
													/* calculate contour using bilinear */
													if (f_im1_jm1<f_i_j)
													{
														min_f=f_im1_jm1;
														max_f=f_i_j;
													}
													else
													{
														min_f=f_i_j;
														max_f=f_im1_jm1;
													}
													if (f_im1_j<min_f)
													{
														min_f=f_im1_j;
													}
													else
													{
														if (f_im1_j>max_f)
														{
															max_f=f_im1_j;
														}
													}
													if (f_i_jm1<min_f)
													{
														min_f=f_i_jm1;
													}
													else
													{
														if (f_i_jm1>max_f)
														{
															max_f=f_i_jm1;
														}
													}
													if ((min_f<=contour_maximum)&&
														(contour_minimum<=max_f))
													{
														if (min_f<=contour_minimum)
														{
															start=0;
														}
														else
														{
															start=1+(int)((min_f-contour_minimum)/
																contour_step);
														}
														if (contour_maximum<=max_f)
														{
															end=number_of_contours;
														}
														else
														{
															end=(int)((max_f-contour_minimum)/contour_step);
														}
														for (k=start;k<=end;k++)
														{
															a=contour_minimum+contour_step*(float)k;
															if (fabs(a)<0.00001*(fabs(contour_maximum)+
																fabs(contour_minimum)))
															{
																a=0;
															}
															/* dashed lines for -ve contours */
															if ((a>=0)||((i+j)%5<2))
															{
																if (((f_im1_jm1<=a)&&(a<f_i_jm1))||
																	((f_im1_jm1>=a)&&(a>f_i_jm1)))
																{
																	if (((f_im1_jm1<=a)&&(a<f_im1_j))||
																		((f_im1_jm1>=a)&&(a>f_im1_j)))
																	{
																		if ((((f_i_jm1<=a)&&(a<f_i_j))||
																			((f_i_jm1>=a)&&(a>f_i_j)))&&
																			(((f_im1_j<=a)&&(a<f_i_j))||
																			((f_im1_j>=a)&&(a>f_i_j))))
																		{
																			b=(a-f_im1_jm1)*(f_im1_jm1+f_i_j-f_im1_j-
																				f_i_jm1)+(f_im1_j-f_im1_jm1)*
																				(f_i_jm1-f_im1_jm1);
																			if (b<0)
																			{
																				XPSDrawLineFloat(display,
																					drawing->pixel_map,
																					(projection->graphics_context).
																					contour_colour,(float)i-(a-f_i_jm1)/
																					(f_im1_jm1-f_i_jm1),(float)(j-1),
																					(float)(i-1),(float)j-(a-f_im1_j)/
																					(f_im1_jm1-f_im1_j));
																				XPSDrawLineFloat(display,
																					drawing->pixel_map,
																					(projection->graphics_context).
																					contour_colour,
																					(float)i-(a-f_i_j)/(f_im1_j-f_i_j),
																					(float)j,(float)i,(float)j-(a-f_i_j)/
																					(f_i_jm1-f_i_j));
																			}
																			else
																			{
																				if (b>0)
																				{
																					XPSDrawLineFloat(display,
																						drawing->pixel_map,
																						(projection->graphics_context).
																						contour_colour,(float)i-(a-f_i_jm1)/
																						(f_im1_jm1-f_i_jm1),(float)(j-1),
																						(float)i,(float)j-(a-f_i_j)/
																						(f_i_jm1-f_i_j));
																					XPSDrawLineFloat(display,
																						drawing->pixel_map,
																						(projection->graphics_context).
																						contour_colour,
																						(float)i-(a-f_i_j)/(f_im1_j-f_i_j),
																						(float)j,(float)(i-1),(float)j-
																						(a-f_im1_j)/(f_im1_jm1-f_im1_j));
																				}
																				else
																				{
																					XPSDrawLineFloat(display,
																						drawing->pixel_map,
																						(projection->graphics_context).
																						contour_colour,
																						(float)(i-1),(float)j-(a-f_im1_j)/
																						(f_im1_jm1-f_im1_j),(float)i,
																						(float)j-(a-f_i_j)/(f_i_jm1-f_i_j));
																					XPSDrawLineFloat(display,
																						drawing->pixel_map,
																						(projection->graphics_context).
																						contour_colour,(float)i-(a-f_i_jm1)/
																						(f_im1_jm1-f_i_jm1),(float)(j-1),
																						(float)i-(a-f_i_j)/(f_im1_j-f_i_j),
																						(float)j);
																				}
																			}
																		}
																		else
																		{
																			XPSDrawLineFloat(display,
																				drawing->pixel_map,
																				(projection->graphics_context).
																				contour_colour,(float)i-(a-f_i_jm1)/
																				(f_im1_jm1-f_i_jm1),(float)(j-1),
																				(float)(i-1),(float)j-
																				(a-f_im1_j)/(f_im1_jm1-f_im1_j));
																		}
																	}
																	else
																	{
																		if (((f_i_jm1<=a)&&(a<f_i_j))||
																			((f_i_jm1>=a)&&(a>f_i_j)))
																		{
																			XPSDrawLineFloat(display,
																				drawing->pixel_map,
																				(projection->graphics_context).
																				contour_colour,(float)i-(a-f_i_jm1)/
																				(f_im1_jm1-f_i_jm1),(float)(j-1),
																				(float)i,
																				(float)j-(a-f_i_j)/(f_i_jm1-f_i_j));
																		}
																		else
																		{
																			if (((f_im1_j<=a)&&(a<f_i_j))||
																				((f_im1_j>=a)&&(a>f_i_j)))
																			{
																				XPSDrawLineFloat(display,
																					drawing->pixel_map,
																					(projection->graphics_context).
																					contour_colour,
																					(float)i-(a-f_i_jm1)/
																					(f_im1_jm1-f_i_jm1),(float)(j-1),
																					(float)i-(a-f_i_j)/
																					(f_im1_j-f_i_j),(float)j);
																			}
																		}
																	}
																}
																else
																{
																	if (((f_im1_jm1<=a)&&(a<f_im1_j))||
																		((f_im1_jm1>=a)&&(a>f_im1_j)))
																	{
																		if (((f_i_jm1<=a)&&(a<f_i_j))||
																			((f_i_jm1>=a)&&(a>f_i_j)))
																		{
																			XPSDrawLineFloat(display,
																				drawing->pixel_map,
																				(projection->graphics_context).
																				contour_colour,
																				(float)(i-1),(float)j-(a-f_im1_j)/
																				(f_im1_jm1-f_im1_j),(float)i,(float)j-
																				(a-f_i_j)/(f_i_jm1-f_i_j));
																		}
																		else
																		{
																			if (((f_im1_j<=a)&&(a<f_i_j))||
																				((f_im1_j>=a)&&(a>f_i_j)))
																			{
																				XPSDrawLineFloat(display,
																					drawing->pixel_map,
																					(projection->graphics_context).
																					contour_colour,
																					(float)(i-1),(float)j-(a-f_im1_j)/
																					(f_im1_jm1-f_im1_j),(float)i-
																					(a-f_i_j)/(f_im1_j-f_i_j),(float)j);
																			}
																		}
																	}
																	else
																	{
																		if ((((f_i_jm1<=a)&&(a<f_i_j))||
																			((f_i_jm1>=a)&&(a>f_i_j)))&&
																			(((f_im1_j<=a)&&(a<f_i_j))||
																			((f_im1_j>=a)&&(a>f_i_j))))
																		{
																			XPSDrawLineFloat(display,
																				drawing->pixel_map,
																				(projection->graphics_context).
																				contour_colour,
																				(float)i-(a-f_i_j)/(f_im1_j-f_i_j),
																				(float)j,(float)i,(float)j-(a-f_i_j)/
																				(f_i_jm1-f_i_j));
																		}
																	}
																}
															}
														}
													}
												}
												if (draw_boundary&&(0<boundary_type)&&
													(boundary_type<15))
												{
													switch (boundary_type)
													{
														case 1: case 8:
														{
															XPSDrawLine(display,drawing->pixel_map,
																(projection->graphics_context).contour_colour,
																i-1,j,i,j-1);
														} break;
														case 2: case 4:
														{
															XPSDrawLine(display,drawing->pixel_map,
																(projection->graphics_context).contour_colour,
																i-1,j-1,i,j);
														} break;
														case 3:
														{
															XPSDrawLine(display,drawing->pixel_map,
																(projection->graphics_context).contour_colour,
																i-1,j-1,i-1,j);
														} break;
														case 5:
														{
															XPSDrawLine(display,drawing->pixel_map,
																(projection->graphics_context).contour_colour,
																i-1,j-1,i,j-1);
														} break;
														case 10:
														{
															XPSDrawLine(display,drawing->pixel_map,
																(projection->graphics_context).contour_colour,
																i-1,j,i,j);
														} break;
														case 12:
														{
															XPSDrawLine(display,drawing->pixel_map,
																(projection->graphics_context).contour_colour,i,
																j-1,i,j);
														} break;
													}
												}
											}
										}
									}
								}
								else
								{
									XPSPutImage(display,drawing->pixel_map,
										(projection->graphics_context).copy,image,0,0,0,0,
										drawing_width,drawing_height);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"draw_projection.  Missing image");
							}
							/* write contour values */
							if ((HIDE_COLOUR==projection->colour_option)&&
								(SHOW_CONTOURS==projection->contours_option))
							{
								if ((projection->contour_x)&&(projection->contour_y))
								{
									number_of_contour_areas=projection->number_of_contour_areas;
									contour_areas_in_x=projection->number_of_contour_areas_in_x;
									minimum_value=get_Spectrum_minimum(projection->spectrum);
									maximum_value=get_Spectrum_maximum(projection->spectrum);
									contour_minimum=projection->contour_minimum;
									contour_maximum=projection->contour_maximum;
									if (maximum_value==minimum_value)
									{
										start=0;
										end=number_of_spectrum_colours;
									}
									else
									{
										start=(int)((contour_minimum-minimum_value)/
											(maximum_value-minimum_value)*
											(float)(number_of_spectrum_colours-1)+0.5);
										end=(int)((contour_maximum-minimum_value)/
											(maximum_value-minimum_value)*
											(float)(number_of_spectrum_colours-1)+0.5);
									}
									cell_range=end-start;
									number_of_contours=projection->number_of_contours;
									for (i=number_of_contours;i>0;)
									{
										i--;
										cell_number=start+(int)((float)(i*cell_range)/
											(float)(number_of_contours-1)+0.5);
										a=(contour_maximum*(float)i+contour_minimum*
											(float)(number_of_contours-i-1))/
											(float)(number_of_contours-1);
										if (fabs(a)<
											0.00001*(fabs(contour_maximum)+fabs(contour_minimum)))
										{
											a=0;
										}
										sprintf(value_string,"%.4g",a);
										string_length=strlen(value_string);
										XTextExtents(projection->font,value_string,string_length,
											&direction,&ascent,&descent,&bounds);
										x_offset=(bounds.lbearing-bounds.rbearing)/2;
										y_offset=(bounds.ascent-bounds.descent)/2;
										x_separation=(bounds.lbearing+bounds.rbearing);
										y_separation=(bounds.ascent+bounds.descent);
										contour_x=(projection->contour_x)+
											(cell_number*number_of_contour_areas);
										contour_y=(projection->contour_y)+
											(cell_number*number_of_contour_areas);
										for (j=0;j<number_of_contour_areas;j++)
										{
											if ((x_pixel= *contour_x)>=0)
											{
												y_pixel= *contour_y;
												/* check that its not too close to previously drawn
													values */
												draw_contour_value=1;
												if (0<j%contour_areas_in_x)
												{
													if ((pixel_left= *(contour_x-1))>=0)
													{
														temp_int=y_pixel-(*(contour_y-1));
														if (temp_int<0)
														{
															temp_int= -temp_int;
														}
														if ((x_pixel-pixel_left<=x_separation)&&
															(temp_int<=y_separation))
														{
															draw_contour_value=0;
														}
													}
													if (draw_contour_value&&(j>contour_areas_in_x)&&
														((pixel_top= *(contour_x-
														(contour_areas_in_x+1)))>=0))
													{
														if ((x_pixel-pixel_top<=x_separation)&&
															(y_pixel-(*(contour_y-(contour_areas_in_x+1)))<=
															y_separation))
														{
															draw_contour_value=0;
														}
													}
												}
												if (draw_contour_value&&(j>=contour_areas_in_x)&&
													((pixel_top= *(contour_x-contour_areas_in_x))>=0))
												{
													temp_int=x_pixel-pixel_top;
													if (temp_int<0)
													{
														temp_int= -temp_int;
													}
													if ((temp_int<=x_separation)&&
														(y_pixel-(*(contour_y-contour_areas_in_x))<=
														y_separation))
													{
														draw_contour_value=0;
													}
												}
												if (draw_contour_value)
												{
													XPSDrawString(display,drawing->pixel_map,
														(projection->graphics_context).contour_colour,
														x_pixel+x_offset,y_pixel+y_offset,value_string,
														string_length);
												}
											}
											contour_x++;
											contour_y++;
										}
									}
								}
							}
							/* draw the fibres */
								/*???DB.  Use fibre field for elements */
							if (HIDE_FIBRES!=projection->fibres_option)
							{
								/* determine the fibre spacing */
								switch (projection->fibres_option)
								{
									case SHOW_FIBRES_FINE:
									{
										fibre_spacing=10;
									} break;
									case SHOW_FIBRES_MEDIUM:
									{
										fibre_spacing=20;
									} break;
									case SHOW_FIBRES_COARSE:
									{
										fibre_spacing=30;
									} break;
								}
								pixel_fibre_angle=projection->pixel_fibre_angles;
								pixel_depth=projection->pixel_depths;
								/* draw fibres */
								pixel_left=fibre_spacing/2;
								pixel_top=fibre_spacing/2;
								y_pixel=pixel_top;
								for (j=(int)drawing_height/fibre_spacing;j>0;j--)
								{
									x_pixel=pixel_left;
									for (i=(int)drawing_width/fibre_spacing;i>0;i--)
									{
										index=y_pixel*(int)drawing_width+x_pixel;
										if (pixel_depth[index]>=0)
										{
											fibre_angle=pixel_fibre_angle[index];
											fibre_x=(int)(cos(fibre_angle)*(float)fibre_spacing/2);
											fibre_y=(int)(sin(fibre_angle)*(float)fibre_spacing/2);
											XPSDrawLine(display,drawing->pixel_map,
												(projection->graphics_context).fibre_colour,
												x_pixel-(short)fibre_x,y_pixel-(short)fibre_y,
												x_pixel+(short)fibre_x,y_pixel+(short)fibre_y);
										}
										x_pixel += fibre_spacing;
									}
									y_pixel += fibre_spacing;
								}
							}
						}
						/* draw the landmarks */
						if (SHOW_LANDMARKS==projection->landmarks_option)
						{
#if defined (OLD_CODE)
							/* set the colour for the landmarks */
							XSetForeground(display,(projection->graphics_context).spectrum,
								user_settings.landmark_colour);
							landmark_point=landmark_points;
							for (i=NUMBER_OF_LANDMARK_POINTS;i>0;i--)
							{
								cartesian_to_prolate_spheroidal(landmark_point[0],
									landmark_point[1],landmark_point[2],LANDMARK_FOCUS,&lambda,
									&mu,&theta);
								switch (projection->type)
								{
									case HAMMER_PROJECTION:
									{
										Hammer_projection(mu,theta,&x_screen,&y_screen);
									} break;
									case POLAR_PROJECTION:
									{
										polar_projection(mu,theta,&x_screen,&y_screen);
									} break;
								}
								region_item=rig->region_list;
								for (region_number=0;region_number<number_of_regions;
									region_number++)
								{
									if (SOCK==region_item->region->type)
									{
										x_pixel=start_x[region_number]+
											(int)((x_screen-min_x[region_number])*
											stretch_x[region_number]);
										y_pixel=start_y[region_number]-
											(int)((y_screen-min_y[region_number])*
											stretch_y[region_number]);
										/* draw asterisk */
										XPSDrawLine(display,drawing->pixel_map,
											(projection->graphics_context).spectrum,
											x_pixel-2,y_pixel,x_pixel+2,y_pixel);
										XPSDrawLine(display,drawing->pixel_map,
											(projection->graphics_context).spectrum,
											x_pixel,y_pixel-2,x_pixel,y_pixel+2);
										XPSDrawLine(display,drawing->pixel_map,
											(projection->graphics_context).spectrum,
											x_pixel-2,y_pixel-2,x_pixel+2,y_pixel+2);
										XPSDrawLine(display,drawing->pixel_map,
											(projection->graphics_context).spectrum,
											x_pixel+2,y_pixel+2,x_pixel-2,y_pixel-2);
									}
									region_item=region_item->next;
								}
								landmark_point += 3;
							}
#endif
						}
						/* draw the extrema */
						if (SHOW_EXTREMA==projection->extrema_option)
						{
							/* set the colour for the extrema */
							XSetForeground(display,(projection->graphics_context).spectrum,
								(projection->pixel).landmark_colour);
							/* draw plus */
							XPSFillRectangle(display,drawing->pixel_map,
								(projection->graphics_context).spectrum,
								projection->maximum_x-5,projection->maximum_y-1,11,3);
							XPSFillRectangle(display,drawing->pixel_map,
								(projection->graphics_context).spectrum,
								projection->maximum_x-1,projection->maximum_y-5,3,11);
							/* write value */
							sprintf(value_string,"%.4g",projection->maximum);
							string_length=strlen(value_string);
							x_pixel=projection->maximum_x;
							XTextExtents(projection->font,value_string,string_length,
								&direction,&ascent,&descent,&bounds);
							x_pixel += (bounds.lbearing-bounds.rbearing+1)/2;
							if (projection->maximum_y>drawing_height/2)
							{
								y_pixel=projection->maximum_y-6;
								y_pixel -= descent+1;
							}
							else
							{
								y_pixel=projection->maximum_y+6;
								y_pixel += ascent+1;
							}
							/* make sure that the string doesn't extend outside the window */
							if (x_pixel-bounds.lbearing<0)
							{
								x_pixel=bounds.lbearing;
							}
							else
							{
								if (x_pixel+bounds.rbearing>drawing_width)
								{
									x_pixel=drawing_width-bounds.rbearing;
								}
							}
							if (y_pixel-ascent<0)
							{
								y_pixel=ascent;
							}
							else
							{
								if (y_pixel+descent>drawing_height)
								{
									y_pixel=drawing_height-descent;
								}
							}
							XPSDrawString(display,drawing->pixel_map,
								(projection->graphics_context).spectrum,x_pixel,y_pixel,
								value_string,string_length);
							/* draw minus */
							XPSFillRectangle(display,drawing->pixel_map,
								(projection->graphics_context).spectrum,
								projection->minimum_x-5,projection->minimum_y-1,11,3);
							/* write value */
							sprintf(value_string,"%.4g",projection->minimum);
							string_length=strlen(value_string);
							x_pixel=projection->minimum_x;
							XTextExtents(projection->font,value_string,string_length,
								&direction,&ascent,&descent,&bounds);
							x_pixel += (bounds.lbearing-bounds.rbearing+1)/2;
							if (projection->minimum_y>drawing_height/2)
							{
								y_pixel=projection->minimum_y-6;
								y_pixel -= descent+1;
							}
							else
							{
								y_pixel=projection->minimum_y+6;
								y_pixel += ascent+1;
							}
							/* make sure that the string doesn't extend outside the
								window */
							if (x_pixel-bounds.lbearing<0)
							{
								x_pixel=bounds.lbearing;
							}
							else
							{
								if (x_pixel+bounds.rbearing>drawing_width)
								{
									x_pixel=drawing_width-bounds.rbearing;
								}
							}
							if (y_pixel-ascent<0)
							{
								y_pixel=ascent;
							}
							else
							{
								if (y_pixel+descent>drawing_height)
								{
									y_pixel=drawing_height-descent;
								}
							}
							XPSDrawString(display,drawing->pixel_map,
								(projection->graphics_context).spectrum,x_pixel,y_pixel,
								value_string,string_length);
						}
						/* draw elements */
						if (HIDE_ELEMENTS!=projection->elements_option)
						{
							draw_element_data.elements_option=projection->elements_option;
							draw_element_data.projection_type=projection->type;
							draw_element_data.xi_3=projection->xi_3;
							draw_element_data.min_x=min_x;
							draw_element_data.min_y=min_y;
							draw_element_data.start_x=start_x;
							draw_element_data.start_y=start_y;
							draw_element_data.stretch_x=stretch_x;
							draw_element_data.stretch_y=stretch_y;
							draw_element_data.drawing_width=drawing_width;
							draw_element_data.drawing_height=drawing_height;
							draw_element_data.element_line_discretization=
								projection->element_line_discretization;
							draw_element_data.graphics_context.marker_colour=
								(projection->graphics_context).node_marker_colour;
							draw_element_data.graphics_context.text_colour=
								(projection->graphics_context).node_text_colour;
							draw_element_data.coordinate_field=
								project_node_data.coordinate_field;
							draw_element_data.drawing=drawing;
							draw_element_data.font=projection->font;
							if (projection->element_group)
							{
								return_code=FOR_EACH_OBJECT_IN_GROUP(FE_element)(
									draw_element,(void *)(&draw_element_data),
									projection->element_group);
							}
							else
							{
#if defined (OLD_CODE)
								return_code=FOR_EACH_OBJECT_IN_LIST(FE_element)(
									draw_element,(void *)(&draw_element_data),
									all_FE_element);
#endif /* defined (OLD_CODE) */
                return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
									draw_element,(void *)(&draw_element_data),
									projection->element_manager);
							}
						}
						/* draw electrodes last */
						if (HIDE_NODES!=projection->nodes_option)
						{
							draw_node_data.x=x;
							draw_node_data.y=y;
							draw_node_data.node_x=projection->node_x;
							draw_node_data.node_y=projection->node_y;
							draw_node_data.node_depth=projection->node_depth;
							draw_node_data.node_drawn=projection->node_drawn;
							draw_node_data.pixel_depths=projection->pixel_depths;
							draw_node_data.nodes_option=projection->nodes_option;
							draw_node_data.field=projection->field;
							draw_node_data.component_number=projection->field_component;
							draw_node_data.min_x=min_x;
							draw_node_data.min_y=min_y;
							draw_node_data.start_x=start_x;
							draw_node_data.start_y=start_y;
							draw_node_data.stretch_x=stretch_x;
							draw_node_data.stretch_y=stretch_y;
							draw_node_data.drawing=drawing;
							draw_node_data.drawing_width=drawing_width;
							draw_node_data.drawing_height=drawing_height;
							draw_node_data.graphics_context.marker_colour=
								(projection->graphics_context).node_marker_colour;
							draw_node_data.graphics_context.text_colour=
								(projection->graphics_context).node_text_colour;
							draw_node_data.font=projection->font;
							FOR_EACH_OBJECT_IN_LIST(FE_node)(draw_node,
								(void *)(&draw_node_data),node_list);
						}
					}
					else
					{
						DEALLOCATE(nodes);
						DEALLOCATE(node_x);
						DEALLOCATE(node_y);
						DEALLOCATE(node_depth);
						DEALLOCATE(node_drawn);
						display_message(ERROR_MESSAGE,
							"draw_projection.  Could not project nodes");
						return_code=0;
					}
					DEALLOCATE(x);
					DEALLOCATE(y);
				}
				else
				{
					DEALLOCATE(x);
					DEALLOCATE(y);
					DEALLOCATE(nodes);
					DEALLOCATE(node_x);
					DEALLOCATE(node_y);
					DEALLOCATE(node_depth);
					DEALLOCATE(node_drawn);
					display_message(ERROR_MESSAGE,
						"draw_projection.  Could not allocate x and/or y and/or nodes");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"draw_projection.  No nodes");
				return_code=0;
			}
			DESTROY_LIST(FE_node)(&node_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"draw_projection.  Could not create node list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_projection */

int draw_spectrum_area(struct Projection *projection,
	struct Drawing_2d *drawing)
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
	struct Projection_drawing *drawing)
#endif /* defined (OLD_CODE) */
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
This function draws the colour bar in the <drawing>.
==============================================================================*/
{
	char value_string[11];
	Display *display;
	float contour_maximum,contour_minimum,contour_value,maximum_value,
		minimum_value,spectrum_left,spectrum_right,text_x;
	int ascent,colour_bar_bottom,colour_bar_left,colour_bar_right,colour_bar_top,
		descent,direction,i,number_of_spectrum_colours,return_code,string_length,
		text_y,x,x_range;
	Pixel *spectrum_pixels;
	XCharStruct bounds;

	ENTER(draw_spectrum_area);
	return_code=1;
	/* check arguments */
	if (projection&&drawing)
	{
		display=User_interface_get_display(drawing->user_interface);
		spectrum_pixels=(projection->pixel).spectrum;
		number_of_spectrum_colours=projection->number_of_spectrum_colours;
		/* clear the colour or auxiliary drawing area */
		XPSFillRectangle(display,drawing->pixel_map,
			(projection->graphics_context).background_colour,
			0,0,drawing->width,drawing->height);
		if ((projection->colour_option==SHOW_COLOUR)||
			(projection->contours_option==SHOW_CONTOURS))
		{
			/* draw the colour bar */
				/*???Use XImage ? */
			minimum_value=get_Spectrum_minimum(projection->spectrum);
			maximum_value=get_Spectrum_maximum(projection->spectrum);
			contour_minimum=projection->contour_minimum;
			contour_maximum=projection->contour_maximum;
			colour_bar_left=projection->border_thickness_pixels;
			colour_bar_right=drawing->width-(projection->border_thickness_pixels);
			spectrum_left=(float)colour_bar_left+(contour_minimum-minimum_value)*
				(float)(colour_bar_right-colour_bar_left)/(maximum_value-minimum_value);
			spectrum_right=(float)colour_bar_left+(contour_maximum-minimum_value)*
				(float)(colour_bar_right-colour_bar_left)/(maximum_value-minimum_value);
			/* write the minimum value */
			sprintf(value_string,"%.4g",contour_minimum);
			string_length=strlen(value_string);
			XTextExtents(projection->font,value_string,string_length,&direction,
				&ascent,&descent,&bounds);
			text_x=spectrum_left-(float)bounds.rbearing;
			if (text_x+(float)bounds.lbearing<(float)colour_bar_left)
			{
				text_x=(float)(colour_bar_left-bounds.lbearing);
			}
			text_y=(projection->border_thickness_pixels)+ascent;
			XPSDrawString(display,drawing->pixel_map,
				(projection->graphics_context).spectrum_text_colour,(int)(text_x+0.5),
				text_y,value_string,string_length);
			/* write the maximum value */
			sprintf(value_string,"%.4g",contour_maximum);
			string_length=strlen(value_string);
			XTextExtents(projection->font,value_string,string_length,&direction,
				&ascent,&descent,&bounds);
			text_x=spectrum_right-(float)bounds.lbearing;
			if (text_x+(float)bounds.rbearing>(float)colour_bar_right)
			{
				text_x=(float)(colour_bar_right-bounds.rbearing);
			}
			text_y=(projection->border_thickness_pixels)+ascent;
			XPSDrawString(display,drawing->pixel_map,
				(projection->graphics_context).spectrum_text_colour,(int)(text_x+0.5),
				text_y,value_string,string_length);
			colour_bar_top=text_y+descent+2*(projection->border_thickness_pixels);
			colour_bar_bottom=drawing->height-(projection->border_thickness_pixels);
			/* draw the colour bar */
			if ((colour_bar_left<colour_bar_right)&&
				(colour_bar_top<colour_bar_bottom))
			{
				x_range=colour_bar_right-colour_bar_left;
				for (x=colour_bar_left;x<=colour_bar_right;x++)
				{
					XSetForeground(display,(projection->graphics_context).spectrum,
						spectrum_pixels[(int)((float)((x-colour_bar_left)*
						(number_of_spectrum_colours-1))/(float)x_range+0.5)]);
					XPSFillRectangle(display,drawing->pixel_map,
						(projection->graphics_context).spectrum,x,colour_bar_top,1,
						colour_bar_bottom-colour_bar_top);
				}
			}
			if ((SHOW_CONTOURS==projection->contours_option)&&
				(2<projection->number_of_contours)&&(contour_minimum<contour_maximum))
			{
				/* draw the contour markers */
				XPSDrawLineFloat(display,drawing->pixel_map,
					(projection->graphics_context).contour_colour,spectrum_left,
					(float)(colour_bar_top),spectrum_left,(float)colour_bar_bottom);
				for (i=(projection->number_of_contours)-2;i>0;i--)
				{
					text_x=(spectrum_right*(float)i+spectrum_left*
						(float)(projection->number_of_contours-i-1))/
						(float)(projection->number_of_contours-1);
					XPSDrawLineFloat(display,drawing->pixel_map,
						(projection->graphics_context).contour_colour,text_x,
						(float)colour_bar_top,text_x,(float)colour_bar_bottom);
					/* write the contour value */
					contour_value=(contour_maximum*(float)i+contour_minimum*
						(float)(projection->number_of_contours-i-1))/
						(float)(projection->number_of_contours-1);
					if (fabs(contour_value)<
						0.00001*(fabs(contour_maximum)+fabs(contour_minimum)))
					{
						contour_value=0;
					}
					sprintf(value_string,"%.4g",contour_value);
					string_length=strlen(value_string);
					XTextExtents(projection->font,value_string,string_length,&direction,
						&ascent,&descent,&bounds);
					text_x += (float)(bounds.lbearing-bounds.rbearing)/2;
					text_y=(projection->border_thickness_pixels)+ascent;
					XPSDrawString(display,drawing->pixel_map,
						(projection->graphics_context).contour_colour,(int)(text_x+0.5),
						text_y,value_string,string_length);
				}
				XPSDrawLineFloat(display,drawing->pixel_map,
					(projection->graphics_context).contour_colour,spectrum_right,
					(float)(colour_bar_top),spectrum_right,(float)colour_bar_bottom);
			}
			/* draw the spectrum left and right markers */
			XPSDrawLineFloat(display,drawing->pixel_map,
				(projection->graphics_context).spectrum_marker_colour,spectrum_left,
				(float)(colour_bar_top-(projection->border_thickness_pixels)),
				spectrum_left,(float)colour_bar_top);
			XPSDrawLineFloat(display,drawing->pixel_map,
				(projection->graphics_context).spectrum_marker_colour,spectrum_right,
				(float)(colour_bar_top-(projection->border_thickness_pixels)),
				spectrum_right,(float)colour_bar_top);
			/* save values */
			projection->colour_bar_left=colour_bar_left;
			projection->colour_bar_right=colour_bar_right;
			projection->colour_bar_bottom=colour_bar_bottom;
			projection->colour_bar_top=colour_bar_top;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_spectrum_area.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_spectrum_area */
