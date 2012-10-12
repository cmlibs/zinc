





PROTOTYPE_OBJECT_FUNCTIONS(Light);




int get_Light_attenuation(struct Light *light, float *constant_attenuation,
	float *linear_attenuation, float *quadratic_attenuation);




int set_Light_attenuation(struct Light *light, float constant_attenuation,
	float linear_attenuation, float quadratic_attenuation);



int get_Light_direction(struct Light *light,float direction[3]);



int set_Light_direction(struct Light *light,float direction[3]);



int get_Light_position(struct Light *light,float position[3]);



int set_Light_position(struct Light *light,float position[3]);



int get_Light_spot_cutoff(struct Light *light, float *spot_cutoff);



int set_Light_spot_cutoff(struct Light *light, float spot_cutoff);



int get_Light_spot_exponent(struct Light *light, float *spot_exponent);



int set_Light_spot_exponent(struct Light *light, float spot_exponent);

int modify_Light(struct Parse_state *state,void *light_void,
	void *modify_light_data_void);
/*******************************************************************************
LAST MODIFIED : 17 February 1997

DESCRIPTION :
Modifies the properties of a light.
==============================================================================*/


/*==============================================================================*/

int set_Light(struct Parse_state *state,
	void *light_address_void,void *light_manager_void);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Modifier function to set the light from a command.
*/
