/*******************************************************************************
FILE : node_transform.h

LAST MODIFIED : 29 January 1999

DESCRIPTION :
Handles transformation of 2d nodes to 3d.
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
