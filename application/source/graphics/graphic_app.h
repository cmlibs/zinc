
/* * Executes a GFX MODIFY RENDITION GRAPHIC_TYPE command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_graphic(struct Parse_state *state,
	enum Cmiss_graphic_type graphic_type, const char *help_text,
	struct Modify_rendition_data *modify_rendition_data,
	struct Rendition_command_data *rendition_command_data);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION CYLINDERS command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_cylinders(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION DATA_POINTS command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_data_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION ELEMENT_POINTS command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_element_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION ISO_SURFACES command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_iso_surfaces(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION LINES command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_lines(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION NODE_POINTS command.
 * If return_code is 1, returns the completed Modify_g_element_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_node_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION POINT command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_point(struct Parse_state *state,
		void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION STREAMLINES command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_streamlines(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION SURFACES command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 * @param state Parse state
 * @param modify_rendition_data_void void pointer to a container object
 * @param command_data_void void pointer to a container object
 * @return if successfully modify surface returns 1, else 0
 */
int gfx_modify_rendition_surfaces(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**/
