/*******************************************************************************
FILE : conversion.c

LAST MODIFIED : 6 January 1998

DESCRIPTION :
Converts between different coordinate systems.

NOTE :
Conversion functions use the same prototype - two arrays of data values of the
same precision.  Two typedefs are used -
	struct Dof3_data: calling
	type_ext: extended for calculations.
An array of procedures is created to convert between coordinates.  A typical
calling example would be to go from RECT to CYL_POLARC -, just call
(*(conversion_position[CONV_RECTANGULAR_CARTESIAN][CONV_CYLINDRICAL_POLAR]))
	(old_values,new_values);
Incremental functions have an extra parameter - initial values in the
destination coord system.  Therefore, to add a RECT increment to a CYL_POLAR
value and get the answer in CYL_POLAR, just call
(*(conversion_increment_position[CONV_RECTANGULAR_CARTESIAN]
	[CONV_CYLINDRICAL_POLAR]))(increment,old_values,new_values);
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "general/debug.h"
#include "io_devices/conversion.h"
#include "user_interface/message.h"

/*
Global variables
----------------
*/
conversion_proc *conversion_position
	[CONVERSION_NUM_POSITION][CONVERSION_NUM_POSITION];
conversion_proc *conversion_direction
	[CONVERSION_NUM_DIRECTION][CONVERSION_NUM_DIRECTION];
conversion_proc *conversion_vector
	[CONVERSION_NUM_VECTOR][CONVERSION_NUM_VECTOR];
conversion_increment_proc *conversion_increment_position
	[CONVERSION_INCREMENT_NUM_POSITION][CONVERSION_NUM_POSITION];
conversion_increment_proc *conversion_increment_vector
	[CONVERSION_INCREMENT_NUM_VECTOR][CONVERSION_NUM_VECTOR];

/*
Module functions
----------------
*/
static int no_conversion(struct Dof3_data *initial,struct Dof3_data *converted)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Does not change the data - is used for 1-1 transformations, and for any
that havent been defined yet.
initial->data[0]=?
initial->data[1]=?
initial->data[2]=?
converted->data[0]=?
converted->data[0]=?
converted->data[0]=?
==============================================================================*/
{
	int return_code=1,i;

	ENTER(no_conversion);
	for(i=0;i<3;i++)
	{
		converted->data[i]=initial->data[i];
	}
	LEAVE;

	return (return_code);
} /* no_conversion */

static int cylindrical_polar_to_rect_cartesian(struct Dof3_data *initial,
	struct Dof3_data *converted)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
For transforming from cylindrical polar to cartesian coordinates.
initial->data[0]=r
initial->data[1]=theta
initial->data[2]=z_in
converted->data[0]=x
converted->data[0]=y
converted->data[0]=z
	x=r*cos(theta)
	y=r*sin(theta)
	z=z_in
==============================================================================*/
{
	int return_code=1;

	ENTER(cylindrical_polar_to_rect_cartesian);
	converted->data[0]=initial->data[0]*cos(initial->data[1]*PI_180);
	converted->data[1]=initial->data[0]*sin(initial->data[1]*PI_180);
	converted->data[2]=initial->data[2];
	LEAVE;

	return (return_code);
} /* cylindrical_polar_to_rect_cartesian */

static int rect_cartesian_to_cylindrical_polar(struct Dof3_data *initial,
	struct Dof3_data *converted)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
For transforming from cylindrical polar to cartesian coordinates.
initial->data[0]=x
initial->data[1]=y
initial->data[2]=z_in
converted->data[0]=r
converted->data[0]=theta
converted->data[0]=z
	r=sqrt(x^2+y^2)
	theta=atan(y/x)
	z=z_in
==============================================================================*/
{
	int return_code=1;

	ENTER(rect_cartesian_to_cylindrical_polar);
	converted->data[0]=sqrt(initial->data[0]*initial->data[0]+
		initial->data[1]*initial->data[1]);
	converted->data[1]=atan2(initial->data[1],initial->data[0])/PI_180;
	converted->data[2]=initial->data[2];
	LEAVE;

	return (return_code);
} /* rect_cartesian_to_cylindrical_polar */

static int spherical_polar_to_rect_cartesian(struct Dof3_data *initial,
	struct Dof3_data *converted)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
For transforming from spherical polar to cartesian coordinates.
initial->data[0]=r
initial->data[1]=theta
initial->data[2]=phi
converted->data[0]=x
converted->data[0]=y
converted->data[0]=z
	x=r*cos(phi)*cos(theta)
	y=r*cos(phi)*sin(theta)
	z=r*sin(phi)
==============================================================================*/
{
	int return_code=1;

	ENTER(spherical_polar_to_rect_cartesian);
	converted->data[0]=initial->data[0]*cos(initial->data[2]*PI_180);
	converted->data[1]=converted->data[0]*sin(initial->data[1]*PI_180);
	converted->data[0]*= cos(initial->data[1]*PI_180);
	converted->data[2]=initial->data[0]*sin(initial->data[2]*PI_180);
	LEAVE;

	return (return_code);
} /* spherical_polar_to_rect_cartesian */

static int rect_cartesian_to_spherical_polar(struct Dof3_data *initial,
	struct Dof3_data *converted)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
For transforming from cartesian to spherical polar coordinates.
initial->data[0]=x
initial->data[1]=y
initial->data[2]=z
converted->data[0]=r
converted->data[0]=theta
converted->data[0]=phi
	r=sqrt(x^2+y^2+z^2)
	theta=atan(y/x)
	phi=atan(z/(x/cos(theta)))
==============================================================================*/
{
	int return_code=1;
	double temp;

	ENTER(rect_cartesian_to_spherical_polar);
	converted->data[0]=sqrt(initial->data[0]*initial->data[0]+
		initial->data[1]*initial->data[1]+initial->data[2]*initial->data[2]);
	converted->data[1]=atan2(initial->data[1],initial->data[0]);
	temp=cos(converted->data[1]);
	if (temp!=0)
	{
		converted->data[2]=atan2(initial->data[2],initial->data[0]/temp)/PI_180;
	}
	else
	{
		converted->data[2]=atan2(initial->data[2],
			initial->data[1]/sin(converted->data[1]))/PI_180;
	}
	converted->data[1]/=PI_180;
	LEAVE;

	return (return_code);
} /* rect_cartesian_to_spherical_polar */

static int cylindrical_polar_to_spherical_polar(struct Dof3_data *initial,
	struct Dof3_data *converted)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
For transforming from cartesian to spherical polar coordinates.
initial->data[0]=r_in
initial->data[1]=theta_in
initial->data[2]=z
converted->data[0]=r
converted->data[0]=theta
converted->data[0]=phi
	r=sqrt(r_in^2+z^2)
	theta=theta_in
	phi=atan(z/r_in)
==============================================================================*/
{
	int return_code=1;

	ENTER(cylindrical_polar_to_spherical_polar);
	converted->data[0]=sqrt(initial->data[0]*initial->data[0]+
		initial->data[2]*initial->data[2]);
	converted->data[1]=initial->data[1];
	converted->data[2]=atan2(initial->data[2],initial->data[0])/PI_180;
	LEAVE;

	return (return_code);
} /* cylindrical_polar_to_spherical_polar */

static int spherical_polar_to_cylindrical_polar(struct Dof3_data *initial,
	struct Dof3_data *converted)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
For transforming from cartesian to spherical polar coordinates.
initial->data[0]=r_in
initial->data[1]=theta_in
initial->data[2]=phi
converted->data[0]=r
converted->data[0]=theta
converted->data[0]=z
	r=r_in*cos(phi)
	theta=theta_in
	z=r_in*sin(phi)
==============================================================================*/
{
	int return_code=1;

	ENTER(spherical_polar_to_cylindrical_polar);
	converted->data[0]=initial->data[0]*cos(initial->data[2]*PI_180);
	converted->data[1]=initial->data[1];
	converted->data[2]=initial->data[0]*sin(initial->data[2]*PI_180);
	LEAVE;

	return (return_code);
} /* spherical_polar_to_cylindrical_polar */

static int increment_no_conversion(struct Dof3_data *increment,
	struct Dof3_data *initial,struct Dof3_data *converted)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Does not change the data - is used for 1-1 transformations, and for any
that havent been defined yet.
initial->data[0]=?
initial->data[1]=?
initial->data[2]=?
converted->data[0]=?
converted->data[0]=?
converted->data[0]=?
==============================================================================*/
{
	int i;
	int return_code=1;

	ENTER(increment_no_conversion);
	for (i=0;i<3;i++)
	{
		converted->data[i]=initial->data[i]+increment->data[i];
	}
	LEAVE;

	return (return_code);
} /* increment_no_conversion */

static int increment_rect_cartesian_to_cylindrical_polar(
	struct Dof3_data *increment,struct Dof3_data *initial,
	struct Dof3_data *converted)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Adds a cartesian increment to cylindrical polar values, and then
returns in cylindrical polar
increment->data[0]=x
increment->data[1]=y
increment->data[2]=z_inc
initial->data[0]=r_in
initial->data[1]=theta_in
initial->data[2]=z_in
converted->data[0]=r
converted->data[0]=theta
converted->data[0]=z
	r=sqrt(r_in^2+x^2+y^2+2*r_in*(x*cos(theta_in)+y*sin(theta_in)))
	theta=atan(r_in*sin(theta_in)+y,r_in*cos(theta_in)+x)
	z=z_in+z_inc
==============================================================================*/
{
	int return_code=1;
	double rsintheta,rcostheta;

	ENTER(increment_rect_cartesian_to_cylindrical_polar);
	rsintheta=initial->data[0]*sin(initial->data[1]*PI_180);
	rcostheta=initial->data[0]*cos(initial->data[1]*PI_180);
	converted->data[0]=sqrt(initial->data[0]*initial->data[0]+
		increment->data[0]*increment->data[0]+
		increment->data[1]*increment->data[1]+2*
		(increment->data[0]*rcostheta+increment->data[1]*rsintheta));
	converted->data[1]=atan2(rsintheta+increment->data[1],
		rcostheta+increment->data[0])/PI_180;
	converted->data[2]=initial->data[2] + increment->data[2];
	LEAVE;

	return (return_code);
} /* increment_rect_cartesian_to_cylindrical_polar */

static int increment_rect_cartesian_to_spherical_polar(
	struct Dof3_data *increment,struct Dof3_data *initial,
	struct Dof3_data *converted)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Adds a cartesian increment to cylindrical polar values, and then
returns in cylindrical polar
increment->data[0]=x
increment->data[1]=y
increment->data[2]=z
initial->data[0]=r_in
initial->data[1]=theta_in
initial->data[2]=phi_in
converted->data[0]=r
converted->data[0]=theta
converted->data[0]=phi
	r=sqrt(r_in^2+x^2+y^2+2*r_in*(x*cos(theta_in)+y*sin(theta_in)))
	theta=atan(r_in*sin(theta_in)+y,r_in*cos(theta_in)+x)
	z=z_in+z_inc
==============================================================================*/
{
	int return_code=1;
	double rcosphi_sintheta,rcosphi_costheta,rsinphi,temp;

	ENTER(increment_rect_cartesian_to_spherical_polar);
	rcosphi_sintheta=initial->data[0]*cos(initial->data[2]*PI_180);
	rcosphi_costheta=rcosphi_sintheta*cos(initial->data[1]*PI_180);
	rcosphi_sintheta *= sin(initial->data[1]*PI_180);
	rsinphi=initial->data[0]*sin(initial->data[2]*PI_180);
	converted->data[0]=sqrt(initial->data[0]*initial->data[0]+
		increment->data[0]*increment->data[0]+increment->data[1]*increment->data[1]+
		increment->data[2]*increment->data[2]+
		2*(increment->data[0]*rcosphi_costheta+increment->data[1]*rcosphi_sintheta+
		increment->data[2]*rsinphi));
	converted->data[1]=atan2(rcosphi_sintheta+increment->data[1],rcosphi_costheta+
		increment->data[0]);
	temp=cos(converted->data[1]);
	if (temp!=0)
	{
		converted->data[2]=atan2(rsinphi+increment->data[2],
			(rcosphi_costheta+increment->data[0])/temp)/PI_180;
	}
	else
	{
		converted->data[2]=atan2(rsinphi+increment->data[2],
			(rcosphi_sintheta+increment->data[1])/sin(converted->data[1]))/PI_180;
	}
	converted->data[2]=atan2(rsinphi+increment->data[2],
		(rcosphi_costheta+increment->data[0])/cos(converted->data[1]))/PI_180;
	converted->data[1] /= PI_180;
	LEAVE;

	return (return_code);
} /* increment_rect_cartesian_to_spherical_polar */

static int set_Conv_coordinate_system(struct Parse_state *state,
	void *conv_coordinate_system_void,void *conv_coordinate_system_value)
/*******************************************************************************
LAST MODIFIED : 19 September 1996

DESCRIPTION :
==============================================================================*/
{
	char *current_token;
	enum Conv_coordinate_system *value,*variable;
	int return_code;

	ENTER(set_Conv_coordinate_system);
	if (state)
	{
		if (!(current_token=state->current_token)||
			(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			if (
				(variable=(enum Conv_coordinate_system *)conv_coordinate_system_void)&&
				(value=(enum Conv_coordinate_system *)conv_coordinate_system_value))
			{
				*variable= *value;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_Conv_coordinate_system.  Invalid argument(s)");
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
		display_message(ERROR_MESSAGE,"set_Conv_coordinate_system.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Conv_coordinate_system */

/*
Global functions
----------------
*/
int conversion_init(void)
/*******************************************************************************
LAST MODIFIED : 16 May 1994

DESCRIPTION :
Sets up the array of conversion routines.
==============================================================================*/
{
	int i,j,return_code;

	ENTER(conversion_init);
	/* initialise the data */
	for (i=0;i<CONVERSION_NUM_POSITION;i++)
		for (j=0;j<CONVERSION_NUM_POSITION;j++)
			conversion_position[i][j]=no_conversion;
	conversion_position[CONV_CYLINDRICAL_POLAR][CONV_RECTANGULAR_CARTESIAN] =
		cylindrical_polar_to_rect_cartesian;
	conversion_position[CONV_RECTANGULAR_CARTESIAN][CONV_CYLINDRICAL_POLAR] =
		rect_cartesian_to_cylindrical_polar;
	conversion_position[CONV_SPHERICAL_POLAR][CONV_RECTANGULAR_CARTESIAN] =
		spherical_polar_to_rect_cartesian;
	conversion_position[CONV_RECTANGULAR_CARTESIAN][CONV_SPHERICAL_POLAR] =
		rect_cartesian_to_spherical_polar;
	conversion_position[CONV_CYLINDRICAL_POLAR][CONV_SPHERICAL_POLAR] =
		cylindrical_polar_to_spherical_polar;
	conversion_position[CONV_SPHERICAL_POLAR][CONV_CYLINDRICAL_POLAR] =
		spherical_polar_to_cylindrical_polar;

	/* initialise the data */
	for (i=0;i<CONVERSION_NUM_DIRECTION;i++)
		for (j=0;j<CONVERSION_NUM_DIRECTION;j++)
			conversion_direction[i][j]=no_conversion;

	/* initialise the data */
	for (i=0;i<CONVERSION_NUM_VECTOR;i++)
		for (j=0;j<CONVERSION_NUM_VECTOR;j++)
			conversion_vector[i][j]=no_conversion;
	/* this uses the same equations */
	conversion_vector[CONV_VEC_SPHERICAL_POLAR][CONV_VEC_COMPONENT] =
		spherical_polar_to_rect_cartesian;
	conversion_vector[CONV_VEC_COMPONENT][CONV_VEC_SPHERICAL_POLAR] =
		rect_cartesian_to_spherical_polar;

	/* initialise the data */
	for (i=0;i<CONVERSION_INCREMENT_NUM_POSITION;i++)
		for (j=0;j<CONVERSION_NUM_POSITION;j++)
			conversion_increment_position[i][j]=increment_no_conversion;
	conversion_increment_position[CONV_RECTANGULAR_CARTESIAN]
		[CONV_CYLINDRICAL_POLAR]=increment_rect_cartesian_to_cylindrical_polar;
	conversion_increment_position[CONV_RECTANGULAR_CARTESIAN]
		[CONV_SPHERICAL_POLAR]=increment_rect_cartesian_to_spherical_polar;

	/* initialise the data */
	for (i=0;i<CONVERSION_INCREMENT_NUM_VECTOR;i++)
		for (j=0;j<CONVERSION_NUM_VECTOR;j++)
			conversion_increment_vector[i][j]=increment_no_conversion;
	/* this uses the same equations */
	conversion_increment_vector[CONV_VEC_COMPONENT]
		[CONV_VEC_SPHERICAL_POLAR]=increment_rect_cartesian_to_spherical_polar;

	return_code=1;
	LEAVE;

	return (return_code);
} /* conversion_init */

int set_Dof3_position(struct Parse_state *state,void *position_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 7 October 1996

DESCRIPTION :
Modifier function to set the position from a command.
==============================================================================*/
{
	char *current_token;
	enum Conv_coordinate_system coordinate_system;
	int return_code;
	static enum Conv_coordinate_system
		cylindrical_polar_coordinate_system=CONV_CYLINDRICAL_POLAR,
		oblate_spheroidal_coordinate_system=CONV_OBLATE_SPHEROIDAL,
		prolate_spheroidal_coordinate_system=CONV_PROLATE_SPHEROIDAL,
		rectangular_cartesian_coordinate_system=CONV_RECTANGULAR_CARTESIAN,
		spherical_polar_coordinate_system=CONV_CYLINDRICAL_POLAR;
	static struct Modifier_entry option_table[]=
	{
		{"cylindrical_polar",NULL,&cylindrical_polar_coordinate_system,
			set_Conv_coordinate_system},
		{"oblate_spheroidal",NULL,&oblate_spheroidal_coordinate_system,
			set_Conv_coordinate_system},
		{"prolate_spheroidal",NULL,&prolate_spheroidal_coordinate_system,
			set_Conv_coordinate_system},
		{"rectangular_cartesian",NULL,&rectangular_cartesian_coordinate_system,
			set_Conv_coordinate_system},
		{"spherical_polar",NULL,&spherical_polar_coordinate_system,
			set_Conv_coordinate_system},
		{NULL,NULL,NULL,set_nothing}
	};
	struct Dof3_data *temp_position,temp_data;
	struct Modifier_entry *entry;

	ENTER(set_Dof3_position);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (temp_position=(struct Dof3_data *)position_void)
				{
					return_code=1;
					coordinate_system=CONV_RECTANGULAR_CARTESIAN;
					(option_table[0]).to_be_modified= &coordinate_system;
					(option_table[1]).to_be_modified= &coordinate_system;
					(option_table[2]).to_be_modified= &coordinate_system;
					(option_table[3]).to_be_modified= &coordinate_system;
					(option_table[4]).to_be_modified= &coordinate_system;
					process_option(state,option_table);
					if (current_token=state->current_token)
					{
						if (1==sscanf(current_token," %"DOF3_PRECISION_STRING" ",
							&temp_data.data[0]))
						{
							if (shift_Parse_state(state,1)&&
								(current_token=state->current_token))
							{
								if (1==sscanf(current_token," %"DOF3_PRECISION_STRING" ",
									&temp_data.data[1]))
								{
									if (shift_Parse_state(state,1)&&
										(current_token=state->current_token))
									{
										if (1==sscanf(current_token," %"DOF3_PRECISION_STRING" ",
											&temp_data.data[2]))
										{
											/* convert to rectangular cartesian */
											(*(conversion_position[coordinate_system]
												[CONV_RECTANGULAR_CARTESIAN]))(&temp_data,
												temp_position);
											return_code=shift_Parse_state(state,1);
										}
										else
										{
											display_message(WARNING_MESSAGE,
												"Invalid coordinate 3: %s",current_token);
											display_parse_state_location(state);
											return_code=0;
										}
									}
									else
									{
										display_message(WARNING_MESSAGE,"Missing coordinate 3");
										display_parse_state_location(state);
										return_code=0;
									}
								}
								else
								{
									display_message(WARNING_MESSAGE,"Invalid coordinate 2: %s",
										current_token);
									display_parse_state_location(state);
									return_code=0;
								}
							}
							else
							{
								display_message(WARNING_MESSAGE,"Missing coordinate 2");
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,"Invalid coordinate 1: %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,"Missing coordinate 1");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Dof3_position.  Missing position_void");
					return_code=0;
				}
			}
			else
			{
				/*???DB.  Can this be included inside process_option some how ? */
				entry=option_table;
				if (entry->option)
				{
					display_message(INFORMATION_MESSAGE," %s",entry->option);
					entry++;
					while (entry->option)
					{
						display_message(INFORMATION_MESSAGE,"|%s",entry->option);
						entry++;
					}
				}
#if defined (OLD_CODE)
				return_code=process_option(state,(void *)option_table);
				display_message(INFORMATION_MESSAGE,"# # #");
#endif /* defined (OLD_CODE) */
				/* write the default */
					/*???DB.  In rectangular cartesian */
				if (temp_position=(struct Dof3_data *)position_void)
				{
					display_message(INFORMATION_MESSAGE,"[rectangular_cartesian] #[%"
						DOF3_PRECISION_STRING"] #[%"DOF3_PRECISION_STRING"] #[%"
						DOF3_PRECISION_STRING"]",temp_position->data[0],
						temp_position->data[1],temp_position->data[2]);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," # # #");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing coordinate system");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Dof3_position.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Dof3_position */
