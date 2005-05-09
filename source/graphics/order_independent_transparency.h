/*******************************************************************************
FILE : order_independent_transparency.h

LAST MODIFIED : 14 April 2003

DESCRIPTION :
==============================================================================*/
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
