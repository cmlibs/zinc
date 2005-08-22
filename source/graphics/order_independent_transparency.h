/*******************************************************************************
FILE : order_independent_transparency.h

LAST MODIFIED : 14 April 2003

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
#if !defined (ORDER_INDEPENDENT_TRANSPARENCY_H)
#define ORDER_INDEPENDENT_TRANSPARENCY_H

struct Scene_viewer_order_independent_transparency_data;
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
The private user data for this order independent transparency rendering pass.
==============================================================================*/

int order_independent_capable(void);
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Returns true if the current display is capable of order independent transparency.
==============================================================================*/

struct Scene_viewer_order_independent_transparency_data *
   order_independent_initialise(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Initialises the order independent transparency extension to render a scene
of <width> by <height>.
==============================================================================*/

int order_independent_reshape(
	struct Scene_viewer_order_independent_transparency_data *data,
	int width, int height, int layers, int using_stencil_overlay);
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Initialises per rendering parts of this extension.
==============================================================================*/

void order_independent_display(struct Scene_viewer_rendering_data *rendering_data,
	struct Scene_viewer_order_independent_transparency_data *data,
	double *projection_matrix, double *modelview_matrix,
	enum Scene_viewer_blending_mode blending_mode);
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Actually preforms the rendering pass.
==============================================================================*/

int order_independent_finalise(
	struct Scene_viewer_order_independent_transparency_data **data_address);
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Frees the memory associated with the <data_address> and sets <data_address> to NULL.
==============================================================================*/
#endif /* !defined (ORDER_INDEPENDENT_TRANSPARENCY_H) */
