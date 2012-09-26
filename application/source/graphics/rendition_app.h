




/*
 * Executes a GFX MODIFY RENDITION GENERAL command.
 * Allows general rendition to be changed (eg. discretization) and
 * updates graphics of settings affected by the changes (probably all).
 */
int gfx_modify_rendition_general(struct Parse_state *state,
	void *cmiss_region_void, void *dummy_void);

/***************************************************************************//**
 * Lists the general graphic defined for <rendition>.
 */
int Cmiss_rendition_list_contents(struct Cmiss_rendition *rendition);

/***************************************************************************//**/

/***************************************************************************//**
 * Shared gfx commands used via Cmiss_rendition_execute_command API and
 * command/cmiss
 * @param group  Optional group field for migrating group regions.
 */
int Cmiss_rendition_execute_command_internal(Cmiss_rendition_id rendition,
	Cmiss_field_group_id group, struct Parse_state *state);

