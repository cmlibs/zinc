/*******************************************************************************
FILE : cmiss.h

LAST MODIFIED : 29 January 1999

DESCRIPTION :
This module keeps the data structures within CMISS and CMGUI 'in sync'.  It does
this by creating two wormhole connections.

The first connection handles commands to and from CMISS.

The second connection handles data about nodes, elements etc.

Note:  A connection comprises two wormholes - input and output.
==============================================================================*/
#if !defined (CMISS_H)
#define CMISS_H
#include "wormhole.h"
#include "finite_element/finite_element.h"
#include "command/command.h"
#include "prompt/prompt_window.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
enum CMISS_connection_data_input_state
/*******************************************************************************
LAST MODIFIED : 24 February 1997

DESCRIPTION :
States occupied by the data stream (on input).
???GMH.  Isnt this an implementation issue?
==============================================================================*/
{
	/* node, element etc */
	CMISS_DATA_INPUT_STATE_TYPE,
	/* about to receive element field info */
	CMISS_DATA_INPUT_STATE_ELEMENT_FIELD_INFO,
	/* about to receive element data */
	CMISS_DATA_INPUT_STATE_ELEMENT_DATA,
	/* about to receive deleted elements */
	CMISS_DATA_INPUT_STATE_ELEMENT_DELETE,
	/* about to receive element group number */
	CMISS_DATA_INPUT_STATE_ELEMENT_GROUP,
	/* about to receive element group data */
	CMISS_DATA_INPUT_STATE_ELEMENT_GROUP_DATA,
	/* about to receive deleted element groups */
	CMISS_DATA_INPUT_STATE_ELEMENT_GROUP_DELETE,
	/* about to receive node field info */
	CMISS_DATA_INPUT_STATE_NODE_FIELD_INFO,
	/* about to receive node data */
	CMISS_DATA_INPUT_STATE_NODE_DATA,
	/* about to receive deleted nodes */
	CMISS_DATA_INPUT_STATE_NODE_DELETE,
	/* about to receive node group number */
	CMISS_DATA_INPUT_STATE_NODE_GROUP,
	/* about to receive node group data */
	CMISS_DATA_INPUT_STATE_NODE_GROUP_DATA,
	/* about to receive deleted node groups */
	CMISS_DATA_INPUT_STATE_NODE_GROUP_DELETE
}; /* enum CMISS_connection_data_input_state */

struct CMISS_connection
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
This contains all details about a connection to CMISS
???DB.  Should this be hidden
==============================================================================*/
{
	/* name of the machine this is running on */
	char *name;
	/* finite state information */
	enum CMISS_connection_data_input_state data_input_state;
	int data_type;
	struct Execute_command *execute_command;
	struct FE_element_field_info *current_element_field_info;
	struct FE_node *template_node;
	struct GROUP(FE_element) *new_element_group;
	struct GROUP(FE_node) *new_node_group;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *data_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
	struct Prompt_window **prompt_window_address;
	struct User_interface *user_interface;
	struct Wh_input *command_input,*data_input,*prompt_input;
	struct Wh_output *command_output,*data_output,*prompt_output;
	void *data_manager_callback_id;
	void *element_manager_callback_id;
	void *node_manager_callback_id;
}; /* struct CMISS_connection */

/*
Global Functions
----------------
*/
struct CMISS_connection *CREATE(CMISS_connection)(char *machine,
	enum Machine_type type,int attach,double wormhole_timeout,char mycm_flag,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct Prompt_window **prompt_window_address,char *parameters_file_name,
	char *examples_directory_path,struct Execute_command *execute_command,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Creates a connection to the machine specified in <machine>.  If <attach> is set,
then we attach rather than spawn.
==============================================================================*/

int DESTROY(CMISS_connection)(struct CMISS_connection **connection_address);
/*******************************************************************************
LAST MODIFIED : 3 December 1996

DESCRIPTION :
Frees the memory for the connection, sets <*node_address> to NULL.
==============================================================================*/

int CMISS_connection_process_command(struct CMISS_connection *connection,
	char *command);
/*******************************************************************************
LAST MODIFIED : 3 December 1996

DESCRIPTION :
Executes the given command within CMISS.
==============================================================================*/

int CMISS_connection_process_prompt_reply(struct CMISS_connection *connection,
	char *reply);
/*******************************************************************************
LAST MODIFIED : 3 December 1996

DESCRIPTION :
Sends the given text in response to the prompt.
==============================================================================*/

int CMISS_connection_update(struct CMISS_connection *connection);
/*******************************************************************************
LAST MODIFIED : 3 December 1996

DESCRIPTION :
Performs any updating necessary.
NOTE:  This routine may cause the link to 'commit suicide'.  Do not rely on
the CMISS_connection being valid after a call to this routine.
==============================================================================*/
#endif /* CMISS_H */
