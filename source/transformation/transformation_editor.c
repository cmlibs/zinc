/*******************************************************************************
FILE : transformation_editor.c

LAST MODIFIED : 5 December 2001

DESCRIPTION :
This module creates a free transformation_editor input device, using two dof3,
two control and one input widget.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include "dof3/dof3.h"
#include "dof3/dof3_control.h"
#if defined (EXT_INPUT)
#include "dof3/dof3_input.h"
#endif
#include "general/debug.h"
#include "transformation/transformation_editor.h"
#include "transformation/transformation_editor.uidh"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "view/coord.h"
#include "view/coord_trans.h"

#define TRANSFORMATION_EDITOR_PRECISION double
#define TRANSFORMATION_EDITOR_PRECISION_STRING "lf"
#define TRANSFORMATION_EDITOR_STRING_SIZE 100
/* make this large so that huge numbers do not cause an overflow */

/*
UIL Identifiers
---------------
*/
#define transformation_editor_menu_ID          1
#define transformation_editor_pos_form_ID      2
#define transformation_editor_dir_form_ID      3
#define transformation_editor_coord_ID         4
#define transformation_editor_pos_menu_ID      5
#define transformation_editor_dir_menu_ID      6
#define transformation_editor_warning_form_ID  7

/*
Module Types
------------
*/

struct Transformation_editor
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Contains all the information carried by the transformation_editor widget
==============================================================================*/
{
	gtMatrix transformation_matrix;
	struct Callback_data update_callback;
	struct Cmgui_coordinate *parent_coordinate;
	Widget coord_widget, dir_form, dirctrl_widget, direction_widget, input_widget,
		menu, pos_form, posctrl_widget, position_widget, widget_parent, widget,
		coord_form, pos_menu, dir_menu, warning_form, *widget_address;
}; /* Transformation_editor */

/*
Module variables
----------------
*/

#if defined (MOTIF)
static int transformation_editor_hierarchy_open = 0;
static MrmHierarchy transformation_editor_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/

static void transformation_editor_update(
	struct Transformation_editor *transformation_editor)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Call the update_callback with the current transformation to tell the client
about the current state -- which has usually just changed.
Returns call_data is a gtMatrix *.
==============================================================================*/
{
	ENTER(transformation_editor_update);
	if (transformation_editor)
	{
		if (transformation_editor->update_callback.procedure)
		{
			/* now call the procedure with the user data and the transformation_editor
				 data */
			(transformation_editor->update_callback.procedure)(
				transformation_editor->widget,
				transformation_editor->update_callback.data,
				(void *)&(transformation_editor->transformation_matrix));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_update.  Invalid argument(s)");
	}
	LEAVE;
} /* transformation_editor_update */

static void transformation_editor_update_coord(Widget coord_widget,
	void *transformation_editor_void, void *coordinate_void)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
==============================================================================*/
{
	struct Transformation_editor *transformation_editor;
	struct Cmgui_coordinate *coordinate;
	struct Dof3_data new_dof3;

	ENTER(transformation_editor_update_coord);
	USE_PARAMETER(coord_widget);
	if ((transformation_editor =
		(struct Transformation_editor *)transformation_editor_void) &&
		(coordinate = (struct Cmgui_coordinate *)coordinate_void))
	{
		transformation_editor->parent_coordinate = coordinate;
		get_local_position((struct Dof3_data *)dof3_get_data(
			transformation_editor->position_widget, DOF3_DATA),
			coordinate, &new_dof3);
		dof3_set_data(transformation_editor->position_widget,
			DOF3_DATA, &new_dof3);
		get_local_direction((struct Dof3_data *)dof3_get_data(
			transformation_editor->direction_widget, DOF3_DATA),
			coordinate, &new_dof3);
		dof3_set_data(transformation_editor->direction_widget,
			DOF3_DATA, &new_dof3);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_update_coord.  Invalid argument(s)");
	}
	LEAVE;
} /* transformation_editor_update_coord */

static int position_direction_to_transformation_matrix(
	struct Dof3_data *position, struct Dof3_data *direction,
	gtMatrix *transformation_matrix)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Takes the 3 position and 3 direction values and puts them into the
4x4 transformation_matrix.
==============================================================================*/
{
	Gmatrix gmatrix;
	int i, j, return_code;

	ENTER(position_direction_to_transformation_matrix);
	if (position && direction && transformation_matrix)
	{
		euler_matrix(direction, &gmatrix);
		for (i = 0; i < GMATRIX_SIZE; i++)
		{
			for (j = 0; j < GMATRIX_SIZE; j++)
			{
				(*transformation_matrix)[i][j] = gmatrix.data[i][j];
			}
		}
		(*transformation_matrix)[3][0] = position->data[0];
		(*transformation_matrix)[3][1] = position->data[1];
		(*transformation_matrix)[3][2] = position->data[2];
		(*transformation_matrix)[0][3] = 0.0;
		(*transformation_matrix)[1][3] = 0.0;
		(*transformation_matrix)[2][3] = 0.0;
		(*transformation_matrix)[3][3] = 1.0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"position_direction_to_transformation_matrix.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* position_direction_to_transformation_matrix */

static void transformation_editor_update_position(
	Widget transformation_editor_widget, void *transformation_editor_void,
	void *position_dof3_void)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
The position has changed; incorporate it into the transformation and tell the
rest of the program.
==============================================================================*/
{
	struct Transformation_editor *transformation_editor;
	struct Dof3_data *direction, global_direction, global_position, *position;

	ENTER(transformation_editor_update_position);
	USE_PARAMETER(transformation_editor_widget);
	if ((transformation_editor =
		(struct Transformation_editor *)transformation_editor_void) &&
		(position = (struct Dof3_data *)position_dof3_void) &&
		(direction = (struct Dof3_data *)dof3_get_data(
			transformation_editor->direction_widget, DOF3_DATA)))
	{
		get_global_position(position, transformation_editor->parent_coordinate,
			&global_position);
		get_global_direction(direction, transformation_editor->parent_coordinate,
			&global_direction);
		if (position_direction_to_transformation_matrix(
			&global_position, &global_direction,
			&(transformation_editor->transformation_matrix)))
		{
			transformation_editor_update(transformation_editor);
		}
		XtUnmanageChild(transformation_editor->warning_form);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_update_position.  Invalid argument(s)");
	}
	LEAVE;
} /* transformation_editor_update_position */

static void transformation_editor_update_direction(
	Widget transformation_editor_widget, void *transformation_editor_void,
	void *direction_dof3_void)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
The direction has changed; incorporate it into the transformation and tell the
rest of the program.
==============================================================================*/
{
	struct Transformation_editor *transformation_editor;
	struct Dof3_data *direction, global_direction, global_position, *position;

	ENTER(transformation_editor_update_direction);
	USE_PARAMETER(transformation_editor_widget);
	if ((transformation_editor =
		(struct Transformation_editor *)transformation_editor_void) &&
		(direction = (struct Dof3_data *)direction_dof3_void) &&
		(position = (struct Dof3_data *)dof3_get_data(
			transformation_editor->position_widget, DOF3_DATA)))
	{
		get_global_position(position, transformation_editor->parent_coordinate,
			&global_position);
		get_global_direction(direction, transformation_editor->parent_coordinate,
			&global_direction);
		if (position_direction_to_transformation_matrix(
			&global_position, &global_direction,
			&(transformation_editor->transformation_matrix)))
		{
			transformation_editor_update(transformation_editor);
		}
		XtUnmanageChild(transformation_editor->warning_form);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_update_direction.  Invalid argument(s)");
	}
	LEAVE;
} /* transformation_editor_update_direction */

static void transformation_editor_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Finds the id of the buttons on the transformation_editor widget.
==============================================================================*/
{
	struct Transformation_editor *transformation_editor = NULL;

	ENTER(transformation_editor_identify_button);
	USE_PARAMETER(reason);
	/* find out which transformation_editor widget we are in */
	XtVaGetValues(w, XmNuserData, &transformation_editor, NULL);
	if (transformation_editor)
	{
		switch (button_num)
		{
			case transformation_editor_menu_ID:
			{
				transformation_editor->menu = w;
			} break;
			case transformation_editor_pos_form_ID:
			{
				transformation_editor->pos_form = w;
			} break;
			case transformation_editor_dir_form_ID:
			{
				transformation_editor->dir_form = w;
			} break;
			case transformation_editor_coord_ID:
			{
				transformation_editor->coord_form = w;
			} break;
			case transformation_editor_pos_menu_ID:
			{
				transformation_editor->pos_menu = w;
			} break;
			case transformation_editor_dir_menu_ID:
			{
				transformation_editor->dir_menu = w;
			} break;
			case transformation_editor_warning_form_ID:
			{
				transformation_editor->warning_form = w;
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"transformation_editor_identify_button.  Invalid button number");
			} break;
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"transformation_editor_identify_button.  Missing transformation editor");
	}
	LEAVE;
} /* transformation_editor_identify_button */

static void transformation_editor_destroy_CB(Widget w, int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Callback for the transformation_editorment dialog - tidies up all details -
mem etc
==============================================================================*/
{
	struct Transformation_editor *transformation_editor;

	ENTER(transformation_editor_destroy_CB);
	USE_PARAMETER(reason);
	USE_PARAMETER(tag);
	/* Get the pointer to the data for the transformation_editor widget */
	XtVaGetValues(w,XmNuserData,&transformation_editor,NULL);
	/* deallocate the memory for the user data */
	*(transformation_editor->widget_address) = (Widget)NULL;
	DEALLOCATE(transformation_editor);
	LEAVE;
} /* transformation_editor_destroy_CB */

/*
Global Functions
----------------
*/

Widget create_transformation_editor_widget(
	Widget *transformation_editor_widget_address,
	Widget parent, gtMatrix *initial_transformation_matrix)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Creates a widget for editing a gtMatrix.
Note: currently restricted to editing 4x4 matrices that represent a rotation
and a translation only.
==============================================================================*/
{
	gtMatrix default_initial_transformation_matrix =
	{
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 }
	};
	int init_widgets;
	MrmType transformation_editor_dialog_class;
	struct Callback_data callback;
	struct Dof3_data temp_dof3;
	struct Transformation_editor *transformation_editor = NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"trans_editor_identify_button",
		(XtPointer)transformation_editor_identify_button},
		{"trans_editor_destroy_CB",
		(XtPointer)transformation_editor_destroy_CB}
	};
	static MrmRegisterArg identifier_list[] =
	{
		{"trans_editor_structure",(XtPointer)NULL},
		{"trans_editor_menu_ID",(XtPointer)transformation_editor_menu_ID},
		{"trans_editor_pos_form_ID",(XtPointer)transformation_editor_pos_form_ID},
		{"trans_editor_dir_form_ID",(XtPointer)transformation_editor_dir_form_ID},
		{"trans_editor_coord_ID",(XtPointer)transformation_editor_coord_ID},
		{"trans_editor_pos_menu_ID",(XtPointer)transformation_editor_pos_menu_ID},
		{"trans_editor_dir_menu_ID",(XtPointer)transformation_editor_dir_menu_ID},
		{"trans_editor_warning_form_ID",
		 (XtPointer)transformation_editor_warning_form_ID}
	};
	Widget return_widget;

	ENTER(create_transformation_editor_widget);
	return_widget = (Widget)NULL;
	if (transformation_editor_widget_address && parent)
	{
		if (MrmOpenHierarchy_base64_string(transformation_editor_uidh,
			&transformation_editor_hierarchy, &transformation_editor_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(transformation_editor, struct Transformation_editor,
				1))
			{
				/* initialise the structure */
				transformation_editor->widget_parent = parent;
				transformation_editor->widget = (Widget)NULL;
				transformation_editor->widget_address =
					transformation_editor_widget_address;
				transformation_editor->menu = (Widget)NULL;
				transformation_editor->pos_form = (Widget)NULL;
				transformation_editor->dir_form = (Widget)NULL;
				transformation_editor->pos_menu = (Widget)NULL;
				transformation_editor->dir_menu = (Widget)NULL;
				transformation_editor->warning_form = (Widget)NULL;
				transformation_editor->coord_form = (Widget)NULL;
				transformation_editor->position_widget = (Widget)NULL;
				transformation_editor->direction_widget = (Widget)NULL;
				transformation_editor->posctrl_widget = (Widget)NULL;
				transformation_editor->dirctrl_widget = (Widget)NULL;
				transformation_editor->input_widget = (Widget)NULL;
				transformation_editor->parent_coordinate =
					(struct Cmgui_coordinate *)NULL;
				transformation_editor->update_callback.procedure =
					(Callback_procedure *)NULL;
				transformation_editor->update_callback.data = (void *)NULL;
				/* register the callbacks */
				if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
					transformation_editor_hierarchy, callback_list,
					XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value = (XtPointer)transformation_editor;
					if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
						transformation_editor_hierarchy, identifier_list,
						XtNumber(identifier_list)))
					{
						/* fetch transformation_editor window widget */
						if (MrmSUCCESS == MrmFetchWidget(transformation_editor_hierarchy,
							"trans_editor_widget", transformation_editor->widget_parent,
							&(transformation_editor->widget),
							&transformation_editor_dialog_class))
						{
							XtManageChild(transformation_editor->widget);
							/* set the mode toggle to the correct transformation_editor */
							init_widgets = 1;
							/* do we want the dof3 widgets to be relative or absolute */
							temp_dof3.data[0] = 0.0;
							temp_dof3.data[1] = 0.0;
							temp_dof3.data[2] = 0.0;
							if (!create_dof3_widget(
								&transformation_editor->position_widget,
								transformation_editor->pos_form, DOF3_POSITION, DOF3_ABSOLUTE,
								CONV_RECTANGULAR_CARTESIAN, &temp_dof3))
							{
								display_message(ERROR_MESSAGE,
									"create_transformation_editor_widget.  "
									"Could not create position dof3 widget.");
								init_widgets = 0;
							}
							if (!create_dof3_widget(
								&(transformation_editor->direction_widget),
								transformation_editor->dir_form, DOF3_DIRECTION, DOF3_ABSOLUTE,
								CONV_DIR_EULER, &temp_dof3))
							{
								display_message(ERROR_MESSAGE,
									"create_transformation_editor_widget.  "
									"Could not create direction dof3 widget.");
								init_widgets = 0;
							}
#if defined (EXT_INPUT)
							if (!(transformation_editor->input_widget=create_input_widget(
								transformation_editor->menu,
								transformation_editor->widget)))
							{
								display_message(ERROR_MESSAGE,
									"create_transformation_editor_widget.  "
									"Could not create input widget.");
								init_widgets = 0;
							}
#endif
							if (!(transformation_editor->posctrl_widget=
								create_control_widget(transformation_editor->pos_menu,
									"Control")))
							{
								display_message(ERROR_MESSAGE,
									"create_transformation_editor_widget.  "
									"Could not create position control widget.");
								init_widgets = 0;
							}
							if (!(transformation_editor->dirctrl_widget=
								create_control_widget(transformation_editor->dir_menu,
									"Control")))
							{
								display_message(ERROR_MESSAGE,
									"create_transformation_editor_widget.  "
									"Could not create direction control widget.");
								init_widgets = 0;
							}
							if (!(transformation_editor->coord_widget =
								create_coord_widget(transformation_editor->coord_form)))
							{
								display_message(ERROR_MESSAGE,
									"create_transformation_editor_widget.  "
									"Could not create coord widget.");
								init_widgets = 0;
							}
							if (init_widgets)
							{
#if defined (EXT_INPUT)
								/* now link all the widgets together */
								input_set_data(transformation_editor->input_widget,
									INPUT_POSITION_WIDGET,
									transformation_editor->position_widget);
								input_set_data(transformation_editor->input_widget,
									INPUT_DIRECTION_WIDGET,
									transformation_editor->direction_widget);
								input_set_data(transformation_editor->input_widget,
									INPUT_POSCTRL_WIDGET, transformation_editor->posctrl_widget);
								input_set_data(transformation_editor->input_widget,
									INPUT_DIRCTRL_WIDGET, transformation_editor->dirctrl_widget);
#endif
								control_set_data(transformation_editor->posctrl_widget,
									CONTROL_DOF3_WIDGET, transformation_editor->position_widget);
								control_set_data(transformation_editor->dirctrl_widget,
									CONTROL_DOF3_WIDGET, transformation_editor->direction_widget);
								/* tell the direction that it has a transformation_editor
									 sibling */
								dof3_set_data(transformation_editor->direction_widget,
									DOF3_POSITION_WIDGET,
									transformation_editor->position_widget);
								/* get the global coordinate system */
								/*???GMH.  a bit of a hack at the moment */
								transformation_editor->parent_coordinate=
									global_coordinate_ptr;
								coord_set_data(transformation_editor->coord_widget,
									COORD_COORD_DATA, global_coordinate_ptr);
								callback.procedure = transformation_editor_update_coord;
								callback.data = transformation_editor;
								coord_set_data(transformation_editor->coord_widget,
									COORD_UPDATE_CB, &callback);
								callback.procedure = transformation_editor_update_position;
								callback.data = transformation_editor;
								dof3_set_data(transformation_editor->position_widget,
									DOF3_UPDATE_CB, &callback);
								callback.procedure = transformation_editor_update_direction;
								callback.data = transformation_editor;
								dof3_set_data(transformation_editor->direction_widget,
									DOF3_UPDATE_CB, &callback);

								if (initial_transformation_matrix)
								{
									transformation_editor_set_transformation(
										transformation_editor->widget,
										initial_transformation_matrix);
								}
								else
								{
									transformation_editor_set_transformation(
										transformation_editor->widget,
										&default_initial_transformation_matrix);
								}

								return_widget = transformation_editor->widget;
							}
							else
							{
								DEALLOCATE(transformation_editor);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_transformation_editor_widget.  "
								"Could not fetch transformation_editor dialog");
							DEALLOCATE(transformation_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_transformation_editor_widget.  "
							"Could not register identifiers");
						DEALLOCATE(transformation_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_transformation_editor_widget.  "
						"Could not register callbacks");
					DEALLOCATE(transformation_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_transformation_editor_widget.  "
					"Could not allocate transformation_editor widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_transformation_editor_widget.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_transformation_editor_widget.  Invalid argument(s)");
	}
	if (transformation_editor_widget_address)
	{
		*transformation_editor_widget_address = return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_transformation_editor_widget */

int transformation_editor_get_callback(
	Widget transformation_editor_widget, struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Fills the <callback> with the current update callback procedure and data for
the <transformation_editor>.
==============================================================================*/
{
	int return_code;
	struct Transformation_editor *transformation_editor;

	ENTER(transformation_editor_get_callback);
	if (transformation_editor_widget && callback)
	{
		/* Get the pointer to the data for the dialog */
		XtVaGetValues(transformation_editor_widget,
			XmNuserData, &transformation_editor, NULL);
		if (transformation_editor)
		{
			callback->procedure = transformation_editor->update_callback.procedure;
			callback->data = transformation_editor->update_callback.data;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"transformation_editor_get_callback.  Missing editor");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_get_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* transformation_editor_get_callback */

int transformation_editor_set_callback(Widget transformation_editor_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Changes the callback function for the transformation_editor_widget, which
will be called when the transformation changes in any way.
==============================================================================*/
{
	int return_code;
	struct Transformation_editor *transformation_editor;

	ENTER(transformation_editor_set_callback);
	/* check arguments */
	if (transformation_editor_widget && callback)
	{
		/* Get the pointer to the data for the choose_settings dialog */
		XtVaGetValues(transformation_editor_widget,
			XmNuserData, &transformation_editor, NULL);
		if (transformation_editor)
		{
			transformation_editor->update_callback.procedure = callback->procedure;
			transformation_editor->update_callback.data = callback->data;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"transformation_editor_set_callback.  Missing editor");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_set_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* transformation_editor_set_callback */

int transformation_editor_get_transformation(
	Widget transformation_editor_widget, gtMatrix *transformation_matrix)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Puts the transformation being edited in the <transformation_editor> out to
<transformation_matrix>.
==============================================================================*/
{
	int i, j, return_code;
	struct Transformation_editor *transformation_editor;

	ENTER(transformation_editor_get_transformation);
	if (transformation_editor_widget && transformation_matrix)
	{
		/* Get the pointer to the data for the transformation_editor dialog */
		transformation_editor = (struct Transformation_editor *)NULL;
		XtVaGetValues(transformation_editor_widget,
			XmNuserData, &transformation_editor, NULL);
		if (transformation_editor)
		{
			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 4; j++)
				{
					(*transformation_matrix)[i][j] =
						transformation_editor->transformation_matrix[i][j];
				}
			}

			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"transformation_editor_get_transformation.  Missing editor");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_get_transformation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* transformation_editor_get_transformation */

int transformation_editor_set_transformation(
	Widget transformation_editor_widget, gtMatrix *transformation_matrix)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Sets the <transformation_editor> to editor the transformation encoded in
4x4 <transformation_matrix>.
==============================================================================*/
{
	Gmatrix gmatrix;
	gtMatrix resolved_transformation_matrix;
	int i, j, return_code;
	struct Dof3_data direction, position;
	struct Transformation_editor *transformation_editor;

	ENTER(transformation_editor_set_transformation);
	if (transformation_editor_widget && transformation_matrix)
	{
		/* Get the pointer to the data for the transformation_editor dialog */
		transformation_editor = (struct Transformation_editor *)NULL;
		XtVaGetValues(transformation_editor_widget,
			XmNuserData, &transformation_editor, NULL);
		if (transformation_editor)
		{
			/* 1. store the 4x4 transformation_matrix */
			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 4; j++)
				{
					transformation_editor->transformation_matrix[i][j] =
						(*transformation_matrix)[i][j];
				}
			}

			/* 1. convert gtMatrix into position and direction vectors, if possible */

			/* clear direction in case following fails */
			direction.data[0] = 0;
			direction.data[1] = 0;
			direction.data[2] = 0;
			/* convert the gtMatrix into position vector and direction
				 Euler angles. GMATRIX_SIZE is 3; following fails if changed! */
			for (i = 0; i < GMATRIX_SIZE; i++)
			{
				for (j = 0; j < GMATRIX_SIZE; j++)
				{
					gmatrix.data[i][j] = (*transformation_matrix)[i][j];
				}
			}
			/* convert the matrix to Euler angles */
			matrix_euler(&gmatrix, &direction);
			/* extract the position from the 4x4 matrix */
			position.data[0] = (*transformation_matrix)[3][0];
			position.data[1] = (*transformation_matrix)[3][1];
			position.data[2] = (*transformation_matrix)[3][2];

			/*???RC later check transformation by direction and position is
				equivalent to 4x4 matrix; otherwise warn user */

			/* 2. put the direction and position values into widgets */
			dof3_set_data(transformation_editor->position_widget, DOF3_DATA,
				(void *)&position);
			dof3_set_data(transformation_editor->direction_widget, DOF3_DATA,
				(void *)&direction);
			/* we must change to the global coord system */
			transformation_editor->parent_coordinate = global_coordinate_ptr;
			coord_set_data(transformation_editor->coord_widget, COORD_COORD_DATA,
				transformation_editor->parent_coordinate);

			/* determine by back calculation whether the transformation is purely
				 a position/rotation. If not, display the warning form */
			if (position_direction_to_transformation_matrix(
				&position, &direction, &resolved_transformation_matrix) &&
				gtMatrix_match_with_tolerance(transformation_matrix,
					&resolved_transformation_matrix, /*tolerance*/1.0E-6))
			{
				XtUnmanageChild(transformation_editor->warning_form);
			}
			else
			{
				XtManageChild(transformation_editor->warning_form);
			}

			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"transformation_editor_set_transformation.  Missing editor");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_set_transformation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* transformation_editor_set_transformation */
