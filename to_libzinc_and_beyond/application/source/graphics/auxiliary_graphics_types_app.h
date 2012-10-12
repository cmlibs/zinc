




struct User_interface;
struct Parse_state;


typedef GLfloat Triple[3];

typedef float Triple[3];


int set_exterior(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 June 1996

DESCRIPTION :
A modifier function for setting exterior flag and face number.
==============================================================================*/

int set_Circle_discretization(struct Parse_state *state,
	void *circle_discretization_void,void *user_interface_void);
/*******************************************************************************
LAST MODIFIED : 2 June 1998

DESCRIPTION :
A modifier function for setting number of segments used to draw circles.
==============================================================================*/

int set_Element_discretization(struct Parse_state *state,
	void *element_discretization_void,void *user_interface_void);
/*******************************************************************************
LAST MODIFIED : 30 October 1996

DESCRIPTION :
A modifier function for setting discretization in each element direction.
==============================================================================*/
