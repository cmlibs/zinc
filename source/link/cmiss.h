/*******************************************************************************
FILE : cmiss.h

LAST MODIFIED : 3 September 2001

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
struct CMISS_connection;
/*******************************************************************************
LAST MODIFIED : 3 September 2001

DESCRIPTION :
Private structure representing the connection between cm and cmgui.
==============================================================================*/

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
