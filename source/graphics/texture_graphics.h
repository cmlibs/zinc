/*******************************************************************************
FILE : texture_graphics.h

LAST MODIFIED : 4 May 2004

DESCRIPTION :
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
#if !defined (TEXTURE_GRAPHICS_H)
#define TEXTURE_GRAPHICS_H

#include "graphics/volume_texture_editor.h"

/*
Global functions
----------------
*/
void create_texture_graphics(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

void close_texture_graphics(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

Boolean graphics_loop(XtPointer texture_window);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

void update_grid(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

void make_objects(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

void draw_current_cell(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

void get_mouse_coords(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

void draw_mouse_3d(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

void makesphere(double r,double phi1,double phi2,int dis1,int dis2);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

void makewiresphere();
/*******************************************************************************
LAST MODIFIED : 25 September 1996

DESCRIPTION :
==============================================================================*/

void select_material(struct Texture_window *texture_window,double x,double y);
/*******************************************************************************
LAST MODIFIED : 31 July 1996

DESCRIPTION :
==============================================================================*/

void draw_texture_cells(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 22 November 1994

DESCRIPTION :
Scans through the volume texture and on finding non empty cells draws a cube of
the appropriate material
==============================================================================*/

void paint_cell(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 22 November 1994

DESCRIPTION :
Sets current cell material to current texture_window material
==============================================================================*/

void delete_cell(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 22 November 1994

DESCRIPTION :
Deletes current cell material
==============================================================================*/

int detail_cell(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
Sets current cell detail value (defualt = 0) from tw select_value
==============================================================================*/

int fill_cell(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
Sets current cell scalar value to be solid
==============================================================================*/

int fill_node(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
Sets absolute node scalar value
==============================================================================*/

int value_node(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
prints node scalar value
==============================================================================*/

int delete_active_node(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
deletes absolute node scalar value
==============================================================================*/

void draw_texture_isosurface(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 22 November 1994

DESCRIPTION :
Calculates an isosurface from the volume texture, and creates an isosurface
object.
==============================================================================*/

void draw_texture_nodes(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 22 November 1994

DESCRIPTION :
Scans through the volume texture and on finding non empty cells draws a cube of
the appropriate material.  1D arrays are used where
A[i][j][k] = A'[i + j*dimi + k*dimi*dimj]
==============================================================================*/

void draw_texture_lines(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 22 November 1994

DESCRIPTION :
Traverses volume_textures line lists and draws the segments
==============================================================================*/

void draw_texture_curves(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 22 November 1994

DESCRIPTION :
Traverses volume_textures curve lists and draws the segments
==============================================================================*/

void draw_texture_blobs(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 22 November 1994

DESCRIPTION :
Traverses volume_textures line lists and draws the segments
==============================================================================*/

void draw_texture_softs(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 22 November 1994

DESCRIPTION :
Traverses volume_textures line lists and draws the segments
==============================================================================*/

void delete_paint(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 21 March 1996

DESCRIPTION :
Deletes current cell material and env map
==============================================================================*/
#endif
