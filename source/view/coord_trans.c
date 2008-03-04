/*******************************************************************************
FILE : coord_trans.c

LAST MODIFIED : 24 June 1996

DESCRIPTION :
This module translates between global and local values (relative to some
specified coordinate system).
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
#include <math.h>
#include "general/debug.h"
#include "view/coord_trans.h"
#include "user_interface/message.h"

/*
Global Functions
----------------
*/

void get_local_position(struct Dof3_data *global_position,
	struct Cmgui_coordinate *coordinate,struct Dof3_data *new_position)
/*******************************************************************************
LAST MODIFIED : 13 January 1995

DESCRIPTION :
Returns local position of a global point relative to <coordinate>.
NOTE: A change in this routine may have to be performed in view/poi.c in the
routine poi_update_position_coord.
==============================================================================*/
{
	int i;
	Gmatrix work;

	ENTER(get_local_position);
	/* subtract off the origin, so that we now just have the relative difference */
	for(i=0;i<3;i++)
	{
		new_position->data[i] = global_position->data[i]-
			coordinate->origin.position.data[i];
	}
	/* these relative values are in relation to the global axes, so they must be
		changed to reflect the orientation of the local axes */
	euler_matrix(&coordinate->origin.direction,&work);
	matrix_premult_vector(&(new_position->data[0]),&work);
	LEAVE;
} /* get_local_position */

void get_local_direction(struct Dof3_data *global_direction,
	struct Cmgui_coordinate *coordinate,struct Dof3_data *new_direction)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Returns local direction of a global point relative to <coordinate>.
==============================================================================*/
{
	Gmatrix origin,local,global,origin_trans;

	ENTER(get_local_direction);
	euler_matrix(global_direction,&global);
	euler_matrix(&coordinate->origin.direction,&origin);
	matrix_copy_transpose(&origin_trans,&origin);
	/* multiply, and put the result in local */
	matrix_mult(&global,&origin_trans,&local);
	matrix_euler(&local,new_direction);
	LEAVE;
} /* get_local_direction */

void get_global_position(struct Dof3_data *local_position,
	struct Cmgui_coordinate *coordinate,struct Dof3_data *new_position)
/*******************************************************************************
LAST MODIFIED : 13 January 1995

DESCRIPTION :
Returns global position of a global point relative to <coordinate>.
==============================================================================*/
{
	int i;
	Gmatrix work;

	ENTER(get_global_position);
	/* copy the values */
	for(i=0;i<3;i++)
	{
		new_position->data[i] = local_position->data[i];
	}
	/* these values are in relation to the local axes, so they must be
		changed to reflect the orientation of the global axes */
	euler_matrix(&coordinate->origin.direction,&work);
	matrix_postmult_vector(&(new_position->data[0]),&work);
	/* now add to these the position of the origin */
	for(i=0;i<3;i++)
	{
		new_position->data[i] += coordinate->origin.position.data[i];
	}
	LEAVE;
} /* get_global_position */

void get_global_direction(struct Dof3_data *local_direction,
	struct Cmgui_coordinate *coordinate,struct Dof3_data *new_direction)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Returns global direction of a local point relative to <coordinate>.
==============================================================================*/
{
	Gmatrix origin,local,global;

	ENTER(get_local_direction);
	euler_matrix(local_direction,&local);
	euler_matrix(&coordinate->origin.direction,&origin);
	/* multiply, and put the result in global */
	matrix_mult(&local,&origin,&global);
	matrix_euler(&global,new_direction);
	LEAVE;
} /* get_local_direction */
