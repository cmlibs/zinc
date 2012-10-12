





PROTOTYPE_OBJECT_FUNCTIONS(Light_model);

int modify_Light_model(struct Parse_state *parse_state,void *light_model_void,
	void *modify_light_model_data_void);
/*******************************************************************************
LAST MODIFIED : 17 February 1997

DESCRIPTION :
Modifies the properties of a light model.
==============================================================================*/



int set_Light_model(struct Parse_state *state,
	void *light_model_address_void,void *light_model_manager_void);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Modifier function to set the light model from a command.
==============================================================================*/
