/*******************************************************************************
FILE : node_transform.h

LAST MODIFIED : 29 January 1999

DESCRIPTION :
Handles transformation of 2d nodes to 3d.
==============================================================================*/
#if !defined (NODE_TRANSFORM_H)
#define NODE_TRANSFORM_H

#include "command/parser.h"
#include "finite_element/finite_element.h"

/*
Global types
------------
*/
struct Node_transform_data
{
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
};

/*
Global functions
---------------
*/
int gfx_transform_node(struct Parse_state *parse_state,
	void *dummy_to_be_modified,void *node_transform_data_void);
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Executes a GFX TRANSFORM NODE command.  This command transforms nodes (ie 2d)
to 3d versions (ie used when digitising image data)
==============================================================================*/

int export_nodes(struct LIST(GROUP(FE_node)) *groups,int base_number,
	struct Execute_command *execute_command);
/*******************************************************************************
LAST MODIFIED : 7 November 1998

DESCRIPTION :
Executes the command FEM define data number %d x=%f y=%f z=%f
==============================================================================*/

int gfx_filter_node(struct Parse_state *parse_state,void *dummy_to_be_modified,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 19 June 1996

DESCRIPTION :
Executes a GFX FILTER NODE command.  This command tests nodes against a set
criteria and sends them to the destination groups.
==============================================================================*/
#endif /* !defined (NODE_TRANSFORM_H) */
