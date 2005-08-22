/*******************************************************************************
FILE : cell_plot.h

LAST MODIFIED : 13 March 2001

DESCRIPTION :
The object used to draw plots in Cell.
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
#if !defined (CELL_PLOT_H)
#define CELL_PLOT_H

#include "cell/cell_interface.h"
#include "cell/cell_variable.h"

/*
Module objects
--------------
*/
struct Cell_plot;
/*******************************************************************************
LAST MODIFIED : 08 March 2001

DESCRIPTION :
The cell plot object
==============================================================================*/

/*
Global functions
----------------
*/
struct Cell_plot *CREATE(Cell_plot)(Widget parent);
/*******************************************************************************
LAST MODIFIED : 08 March 2001

DESCRIPTION :
Creates a cell plot object. <parent> must be a form??
==============================================================================*/
int DESTROY(Cell_plot)(struct Cell_plot **plot_address);
/*******************************************************************************
LAST MODIFIED : 08 March 2001

DESCRIPTION :
Destroys the given <plot>.
==============================================================================*/
int Cell_plot_set_pane_sizes(struct Cell_plot *plot,Widget shell);
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Sets the sizes of the plot and key drawing area's, proportional to the height of
the given <shell> widget. The <shell> will also be resized to accomodate the new
dimensions of the paned window.
==============================================================================*/
int Cell_plot_add_variable(struct Cell_plot *plot,
  struct Cell_variable *variable);
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Adds the given <variable> to the <plot>'s current data sets.
==============================================================================*/
int Cell_plot_remove_variable(struct Cell_plot *plot,
  struct Cell_variable *variable);
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Removes the given <variable> from the <plot>'s current data sets.
==============================================================================*/

#endif /* !defined (CELL_PLOT_H) */
