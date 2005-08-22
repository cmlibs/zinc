/*******************************************************************************
FILE : coord_trans.h

LAST MODIFIED : 6 January 1998

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
#if !defined (COORD_TRANS_H)
#define COORD_TRANS_H

#include "dof3/dof3.h"
#include "io_devices/matrix.h"
#include "view/coord.h"

#define CT_PRECISION double
#define CT_SMALL 1.0E-12
#define CT_PRECISION_STRING "lf"
#define CT_STRING_SIZE 100

/*
Global Functions
----------------
*/
void matrix_euler(Gmatrix *direction,struct Dof3_data *euler);
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Takes a direction cosine matrix and returns the equivalent euler angles in degrees.
Note that when the x axis is aligned with the z axis, then the distribution
between azimuth and roll is arbitrary, so we will say that it is solely made
up of roll.  Inverse formulae are taken from the Polhemus manual, page 156.
==============================================================================*/

void euler_matrix(struct Dof3_data *euler,Gmatrix *direction);
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Returns the equivalent direction cosine matrix of the passed euler values.
Formulae are taken from the Polhemus manual, page 156.
==============================================================================*/

void get_local_position(struct Dof3_data *global_position,
	struct Cmgui_coordinate *coordinate,struct Dof3_data *new_position);
/*******************************************************************************
LAST MODIFIED : 13 January 1995

DESCRIPTION :
Returns local position of a global point relative to <coordinate>.
==============================================================================*/

void get_local_direction(struct Dof3_data *global_direction,
	struct Cmgui_coordinate *coordinate,struct Dof3_data *new_direction);
/*******************************************************************************
LAST MODIFIED : 13 January 1995

DESCRIPTION :
Returns local direction of a global point relative to <coordinate>.
==============================================================================*/

void get_global_position(struct Dof3_data *local_position,
	struct Cmgui_coordinate *coordinate,struct Dof3_data *new_position);
/*******************************************************************************
LAST MODIFIED : 13 January 1995

DESCRIPTION :
Returns global position of a local point relative to <coordinate>.
==============================================================================*/

void get_global_direction(struct Dof3_data *local_direction,
	struct Cmgui_coordinate *coordinate,struct Dof3_data *new_direction);
/*******************************************************************************
LAST MODIFIED : 13 January 1995

DESCRIPTION :
Returns global direction of a local point relative to <coordinate>.
==============================================================================*/

#endif










