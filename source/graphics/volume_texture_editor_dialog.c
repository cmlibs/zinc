/*******************************************************************************
FILE : volume_texture_editor_dialog.c

LAST MODIFIED : 29 April 1998

DESCRIPTION :
Functions prototypes for the create finite elements dialog for the volume
texture editor.
???DB.  There may be further dialogs
==============================================================================*/
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/ToggleB.h>
#include "finite_element/finite_element.h"
#include "general/debug.h"
#if !defined (EXAMPLE)
#include "graphics/mcubes.h"
#endif /* !defined (EXAMPLE) */
#include "graphics/volume_texture_editor_dialog.h"
#include "graphics/volume_texture_editor_dialog.uid64"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
static int volume_texture_editor_dialog_hierarchy_open=0;
static MrmHierarchy volume_texture_editor_dialog_hierarchy;

/*
Module functions
----------------
*/
static void identify_coordinate_field_toggl(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 January 1998

DESCRIPTION :
Sets the id of the coordinate field toggle in the create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_coordinate_field_toggl);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->coordinate_field_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"identify_coordinate_field_toggl.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_coordinate_field_toggl */

static void identify_undeformed_field_toggl(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 January 1998

DESCRIPTION :
Sets the id of the undeformed field toggle in the create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_undeformed_field_toggl);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->undeformed_field_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"identify_undeformed_field_toggl.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_undeformed_field_toggl */

static void identify_fibre_field_toggle(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 January 1998

DESCRIPTION :
Sets the id of the coordinate field toggle in the create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_fibre_field_toggle);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->fibre_field_toggle= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_fibre_field_toggle.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_fibre_field_toggle */

static void identify_x_interpolation_option(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the x interpolation option menu in the create finite elements
dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_x_interpolation_option);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->x_interpolation_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"identify_x_interpolation_option.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_x_interpolation_option */

static void identify_x_int_linear_lagrange(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the linear Lagrange option in the x interpolation menu in the
create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_x_int_linear_lagrange);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		(dialog->x_interpolation_option).linear_lagrange= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_x_int_linear_lagrange.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_x_int_linear_lagrange */

static void identify_x_int_cubic_hermite(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the cubic Hermite option in the x interpolation menu in the
create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_x_int_cubic_hermite);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		(dialog->x_interpolation_option).cubic_hermite= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_x_int_cubic_hermite.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_x_int_cubic_hermite */

static void identify_y_interpolation_option(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the y interpolation option menu in the create finite elements
dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_y_interpolation_option);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->y_interpolation_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"identify_y_interpolation_option.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_y_interpolation_option */

static void identify_y_int_linear_lagrange(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the linear Lagrange option in the y interpolation menu in the
create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_y_int_linear_lagrange);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		(dialog->y_interpolation_option).linear_lagrange= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_y_int_linear_lagrange.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_y_int_linear_lagrange */

static void identify_y_int_cubic_hermite(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the cubic Hermite option in the y interpolation menu in the
create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_y_int_cubic_hermite);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		(dialog->y_interpolation_option).cubic_hermite= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_y_int_cubic_hermite.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_y_int_cubic_hermite */

static void identify_z_interpolation_option(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the z interpolation option menu in the create finite elements
dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_z_interpolation_option);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->z_interpolation_option_menu= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"identify_z_interpolation_option.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_z_interpolation_option */

static void identify_z_int_linear_lagrange(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the linear Lagrange option in the z interpolation menu in the
create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_z_int_linear_lagrange);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		(dialog->z_interpolation_option).linear_lagrange= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_z_int_linear_lagrange.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_z_int_linear_lagrange */

static void identify_z_int_cubic_hermite(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the cubic Hermite option in the z interpolation menu in the
create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_z_int_cubic_hermite);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		(dialog->z_interpolation_option).cubic_hermite= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_z_int_cubic_hermite.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_z_int_cubic_hermite */

static void identify_starting_node_number(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 November 1996

DESCRIPTION :
Sets the id of the starting node number text field in the create finite elements
dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_starting_node_number);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->starting_node_number_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_starting_node_number.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_starting_node_number */

static void identify_starting_line_number(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 November 1996

DESCRIPTION :
Sets the id of the starting line number text field in the create finite elements
dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_starting_line_number);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->starting_line_number_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_starting_line_number.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_starting_line_number */

static void identify_starting_face_number(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the starting face number text field in the create finite elements
dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_starting_face_number);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->starting_face_number_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_starting_face_number.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_starting_face_number */

static void identify_starting_element_numbe(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the starting element number text field in the create finite
elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_starting_element_numbe);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->starting_element_number_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"identify_starting_element_numbe.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_starting_element_numbe */

static void identify_file_group_name_text(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the file/group name text field in the create finite elements
dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_file_group_name_text);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->file_group_name_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_file_group_name_text.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_file_group_name_text */

static void identify_x_output_elements_text(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
Sets the id of the x output elements text field in the create finite elements
dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_x_output_elements_text);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->x_output_elements_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"identify_x_output_elements_text.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_x_output_elements_text */

static void identify_y_output_elements_text(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
Sets the id of the y output elements text field in the create finite elements
dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_y_output_elements_text);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->y_output_elements_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"identify_y_output_elements_text.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_y_output_elements_text */

static void identify_z_output_elements_text(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 April 1998

DESCRIPTION :
Sets the id of the z output elements text field in the create finite elements
dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_z_output_elements_text);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->z_output_elements_text_field= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"identify_z_output_elements_text.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_z_output_elements_text */

static void identify_ok_button(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the ok button in the create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_ok_button);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->ok_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_ok_button.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_ok_button */

static void identify_cancel_button(Widget *widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Sets the id of the cancel button in the create finite elements dialog.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(identify_cancel_button);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		dialog->cancel_button= *widget_id;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"identify_cancel_button.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* identify_cancel_button */

static void destroy_create_finite_elements_dialog(Widget widget,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Tidys up when the user destroys the map dialog box.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(destroy_create_finite_elements_dialog);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		if (dialog->address)
		{
			*(dialog->address)=(struct Create_finite_elements_dialog *)NULL;
		}
		DEALLOCATE(dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
"destroy_create_finite_elements_dialog.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* destroy_create_finite_elements_dialog */

static void close_create_finite_elements_di(Widget widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 November 1996

DESCRIPTION :
Closes the dialog window.
==============================================================================*/
{
	struct Create_finite_elements_dialog *dialog;

	ENTER(close_create_finite_elements_di);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		XtUnmanageChild(dialog->dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"close_create_finite_elements_di.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* close_create_finite_elements_di */

static void write_finite_elements(Widget widget_id,
	XtPointer create_finite_elements_dialog,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Writes a finite element representation of the volume texture.
==============================================================================*/
{
	char *basis_name,*file_name, *file_name2,*temp_name,*value_string;
	double *coordinate_field_vector,iso_value,scale_factors[8],scale_factor_x,
		scale_factor_y,scale_factor_z,x0,x1,x_max,x_min,y0,y1,y_max,y_min,z0,z1,
		z_max,z_min;
	enum FE_basis_type x_interpolation,y_interpolation,z_interpolation;
	FILE *out_file;
	int basis_name_length,derivative_value,element_dimension,face_offset,i,index,
		j,k,l,line_offset,m,number,number_in_x,number_in_y,number_in_z,
		number_of_fields,number_of_values,starting_element_number,
		starting_face_number,starting_line_number,starting_node_number,*table,
		value_index,write_coordinate_field,write_fibre_field,write_undeformed_field,
		x_output_elements,y_output_elements,z_output_elements;
	struct Create_finite_elements_dialog *dialog;
	struct VT_texture_cell **texture_cell_list;
	struct VT_texture_node *texture_node,**texture_node_list;
	struct VT_volume_texture *volume_texture;
	unsigned char in_mesh[8],slit_type;
	Widget option_widget;

	ENTER(write_finite_elements);
	if (dialog=
		(struct Create_finite_elements_dialog *)create_finite_elements_dialog)
	{
		if (volume_texture=dialog->volume_texture)
		{
#if !defined (EXAMPLE)
			update_scalars(volume_texture,0);
#endif /* !defined (EXAMPLE) */
			number_in_x=(volume_texture->dimension)[0];
			number_in_y=(volume_texture->dimension)[1];
			number_in_z=(volume_texture->dimension)[2];
			/* get the settings from the dialog */
			number_of_fields=0;
			if (True==XmToggleButtonGetState(dialog->coordinate_field_toggle))
			{
				number_of_fields++;
				write_coordinate_field=1;
			}
			else
			{
				write_coordinate_field=0;
			}
			if (True==XmToggleButtonGetState(dialog->undeformed_field_toggle))
			{
				number_of_fields++;
				write_undeformed_field=1;
			}
			else
			{
				write_undeformed_field=0;
			}
			if (True==XmToggleButtonGetState(dialog->fibre_field_toggle))
			{
				number_of_fields++;
				write_fibre_field=1;
			}
			else
			{
				write_fibre_field=0;
			}
			XtVaGetValues(dialog->x_interpolation_option_menu,XmNmenuHistory,
				&option_widget,NULL);
			if (option_widget==(dialog->x_interpolation_option).linear_lagrange)
			{
				x_interpolation=LINEAR_LAGRANGE;
			}
			else
			{
				if (option_widget==(dialog->x_interpolation_option).cubic_hermite)
				{
					x_interpolation=CUBIC_HERMITE;
				}
				else
				{
					x_interpolation=LINEAR_LAGRANGE;
					display_message(ERROR_MESSAGE,
						"write_finite_elements.  Unknown x interpolation widget");
				}
			}
			XtVaGetValues(dialog->y_interpolation_option_menu,XmNmenuHistory,
				&option_widget,NULL);
			if (option_widget==(dialog->y_interpolation_option).linear_lagrange)
			{
				y_interpolation=LINEAR_LAGRANGE;
			}
			else
			{
				if (option_widget==(dialog->y_interpolation_option).cubic_hermite)
				{
					y_interpolation=CUBIC_HERMITE;
				}
				else
				{
					y_interpolation=LINEAR_LAGRANGE;
					display_message(ERROR_MESSAGE,
						"write_finite_elements.  Unknown y interpolation widget");
				}
			}
			XtVaGetValues(dialog->z_interpolation_option_menu,XmNmenuHistory,
				&option_widget,NULL);
			if (option_widget==(dialog->z_interpolation_option).linear_lagrange)
			{
				z_interpolation=LINEAR_LAGRANGE;
			}
			else
			{
				if (option_widget==(dialog->z_interpolation_option).cubic_hermite)
				{
					z_interpolation=CUBIC_HERMITE;
				}
				else
				{
					z_interpolation=LINEAR_LAGRANGE;
					display_message(ERROR_MESSAGE,
						"write_finite_elements.  Unknown z interpolation widget");
				}
			}
			XtVaGetValues(dialog->starting_node_number_text_field,XmNvalue,
				&value_string,NULL);
			if (1!=sscanf(value_string,"%d",&starting_node_number))
			{
				starting_node_number=1;
				display_message(ERROR_MESSAGE,
					"write_finite_elements.  Invalid starting node number");
			}
			XtVaGetValues(dialog->starting_line_number_text_field,XmNvalue,
				&value_string,NULL);
			if (1!=sscanf(value_string,"%d",&starting_line_number))
			{
				starting_line_number=1;
				display_message(ERROR_MESSAGE,
					"write_finite_elements.  Invalid starting line number");
			}
			XtVaGetValues(dialog->starting_face_number_text_field,XmNvalue,
				&value_string,NULL);
			if (1!=sscanf(value_string,"%d",&starting_face_number))
			{
				starting_face_number=1;
				display_message(ERROR_MESSAGE,
					"write_finite_elements.  Invalid starting face number");
			}
			XtVaGetValues(dialog->starting_element_number_text_field,XmNvalue,
				&value_string,NULL);
			if (1!=sscanf(value_string,"%d",&starting_element_number))
			{
				starting_element_number=1;
				display_message(ERROR_MESSAGE,
					"write_finite_elements.  Invalid starting element number");
			}
			XtVaGetValues(dialog->x_output_elements_text_field,XmNvalue,
				&value_string,NULL);
			if (0==strcmp(value_string,"*"))
			{
				x_output_elements= -1;
			}
			else
			{
				if (1==sscanf(value_string,"%d",&x_output_elements))
				{
					if (x_output_elements<0)
					{
						x_output_elements=0;
						XtVaSetValues(dialog->x_output_elements_text_field,XmNvalue,"0",
							NULL);
						display_message(ERROR_MESSAGE,"Output elements x number < 0");
					}
					else
					{
						if (x_output_elements>number_in_x)
						{
							x_output_elements=number_in_x;
							sprintf(global_temp_string,"%d",number_in_x);
							XtVaSetValues(dialog->x_output_elements_text_field,XmNvalue,
								global_temp_string,NULL);
							display_message(ERROR_MESSAGE,"Output elements x number > %d",
								global_temp_string);
						}
					}
				}
				else
				{
					x_output_elements= -1;
					XtVaSetValues(dialog->x_output_elements_text_field,XmNvalue,"*",NULL);
					display_message(ERROR_MESSAGE,"Invalid output elements x number");
				}
			}
			XtVaGetValues(dialog->y_output_elements_text_field,XmNvalue,
				&value_string,NULL);
			if (0==strcmp(value_string,"*"))
			{
				y_output_elements= -1;
			}
			else
			{
				if (1==sscanf(value_string,"%d",&y_output_elements))
				{
					if (y_output_elements<0)
					{
						y_output_elements=0;
						XtVaSetValues(dialog->y_output_elements_text_field,XmNvalue,"0",
							NULL);
						display_message(ERROR_MESSAGE,"Output elements y number < 0");
					}
					else
					{
						if (y_output_elements>number_in_y)
						{
							y_output_elements=number_in_y;
							sprintf(global_temp_string,"%d",number_in_y);
							XtVaSetValues(dialog->y_output_elements_text_field,XmNvalue,
								global_temp_string,NULL);
							display_message(ERROR_MESSAGE,"Output elements y number > %d",
								global_temp_string);
						}
					}
				}
				else
				{
					y_output_elements= -1;
					XtVaSetValues(dialog->y_output_elements_text_field,XmNvalue,"*",NULL);
					display_message(ERROR_MESSAGE,"Invalid output elements y number");
				}
			}
			XtVaGetValues(dialog->z_output_elements_text_field,XmNvalue,
				&value_string,NULL);
			if (0==strcmp(value_string,"*"))
			{
				z_output_elements= -1;
			}
			else
			{
				if (1==sscanf(value_string,"%d",&z_output_elements))
				{
					if (z_output_elements<0)
					{
						z_output_elements=0;
						XtVaSetValues(dialog->z_output_elements_text_field,XmNvalue,"0",
							NULL);
						display_message(ERROR_MESSAGE,"Output elements z number < 0");
					}
					else
					{
						if (z_output_elements>number_in_z)
						{
							z_output_elements=number_in_z;
							sprintf(global_temp_string,"%d",number_in_z);
							XtVaSetValues(dialog->z_output_elements_text_field,XmNvalue,
								global_temp_string,NULL);
							display_message(ERROR_MESSAGE,"Output elements z number > %d",
								global_temp_string);
						}
					}
				}
				else
				{
					z_output_elements= -1;
					XtVaSetValues(dialog->z_output_elements_text_field,XmNvalue,"*",NULL);
					display_message(ERROR_MESSAGE,"Invalid output elements z number");
				}
			}
			XtVaGetValues(dialog->file_group_name_text_field,XmNvalue,
				&value_string,NULL);
			/* write the finite elements */
			if (ALLOCATE(file_name,char,strlen(value_string)+8)&&
				ALLOCATE(file_name2,char,strlen(value_string)+16))
			{
				/* write the nodes */
				strcpy(file_name,value_string);
				strcat(file_name,".exnode");
				if (out_file=fopen(file_name,"w"))
				{
					if (volume_texture->coordinate_field)
					{
						coordinate_field_vector=volume_texture->coordinate_field->vector;
					}
					else
					{
						coordinate_field_vector=(double *)NULL;
					}
					iso_value=volume_texture->isovalue;
					texture_cell_list=volume_texture->texture_cell_list;
					texture_node_list=volume_texture->global_texture_node_list;
					x_min=(volume_texture->ximin)[0];
					x_max=(volume_texture->ximax)[0];
					scale_factor_x=(x_max-x_min)/(double)number_in_x;
					y_min=(volume_texture->ximin)[1];
					y_max=(volume_texture->ximax)[1];
					scale_factor_y=(y_max-y_min)/(double)number_in_y;
					z_min=(volume_texture->ximin)[2];
					z_max=(volume_texture->ximax)[2];
					scale_factor_z=(z_max-z_min)/(double)number_in_z;
					/* write heading */
					fprintf(out_file,"Group name : %s\n",value_string);
					fprintf(out_file,"#Fields=%d\n",number_of_fields);
					number_of_values=1;
					scale_factors[0]=1;
					if (-1==x_output_elements)
					{
						switch (x_interpolation)
						{
							case CUBIC_HERMITE:
							{
								for (i=0;i<number_of_values;i++)
								{
									scale_factors[i+number_of_values]=
										scale_factors[i]*scale_factor_x;
								}
								number_of_values *= 2;
							} break;
						}
					}
					if (-1==y_output_elements)
					{
						switch (y_interpolation)
						{
							case CUBIC_HERMITE:
							{
								for (i=0;i<number_of_values;i++)
								{
									scale_factors[i+number_of_values]=
										scale_factors[i]*scale_factor_y;
								}
								number_of_values *= 2;
							} break;
						}
					}
					if (-1==z_output_elements)
					{
						switch (z_interpolation)
						{
							case CUBIC_HERMITE:
							{
								for (i=0;i<number_of_values;i++)
								{
									scale_factors[i+number_of_values]=
										scale_factors[i]*scale_factor_z;
								}
								number_of_values *= 2;
							} break;
						}
					}
					number_of_fields=0;
					value_index=1;
					if (write_coordinate_field)
					{
						number_of_fields++;
						fprintf(out_file,
					"%d) coordinates, coordinate, rectangular cartesian, #Components=3\n",
							number_of_fields);
						fprintf(out_file,"  x.  Value index=%d, #Derivatives=%d\n",
							value_index,number_of_values-1);
						value_index += number_of_values;
						fprintf(out_file,"  y.  Value index=%d, #Derivatives=%d\n",
							value_index,number_of_values-1);
						value_index += number_of_values;
						fprintf(out_file,"  z.  Value index=%d, #Derivatives=%d\n",
							value_index,number_of_values-1);
						value_index += number_of_values;
					}
					if (write_undeformed_field)
					{
						number_of_fields++;
						fprintf(out_file,
					"%d) undeformed, coordinate, rectangular cartesian, #Components=3\n",
							number_of_fields);
						fprintf(out_file,"  x.  Value index=%d, #Derivatives=%d\n",
							value_index,number_of_values-1);
						value_index += number_of_values;
						fprintf(out_file,"  y.  Value index=%d, #Derivatives=%d\n",
							value_index,number_of_values-1);
						value_index += number_of_values;
						fprintf(out_file,"  z.  Value index=%d, #Derivatives=%d\n",
							value_index,number_of_values-1);
						value_index += number_of_values;
					}
					if (write_fibre_field)
					{
						number_of_fields++;
						fprintf(out_file,
							"%d) fibres, anatomical, fibre, #Components=3\n",
							number_of_fields);
						fprintf(out_file,
							"  fibre angle.  Value index=%d, #Derivatives=%d\n",
							value_index,number_of_values-1);
						value_index += number_of_values;
						fprintf(out_file,
							"  embrication angle.  Value index=%d, #Derivatives=%d\n",
							value_index,number_of_values-1);
						value_index += number_of_values;
						fprintf(out_file,
							"  sheet angle.  Value index=%d, #Derivatives=%d\n",
							value_index,number_of_values-1);
						value_index += number_of_values;
					}
					/* write nodes */
					number=starting_node_number;
					for (i=0;i<=number_in_x;i++)
					{
						for (j=0;j<=number_in_y;j++)
						{
							for (k=0;k<=number_in_z;k++)
							{
								for (l=0;l<8;l++)
								{
									in_mesh[l]=0;
								}
								texture_node=
									texture_node_list[i+(number_in_x+1)*(j+(number_in_y+1)*k)];
								slit_type=(unsigned char)(texture_node->node_type);
								if (i>0)
								{
									if (j>0)
									{
										if (k>0)
										{
											if ((texture_cell_list[i-1+number_in_x*(j-1+number_in_y*
												(k-1))])->scalar_value>iso_value)
											{
												in_mesh[0]=1;
											}
										}
										if (k<number_in_z)
										{
											if ((texture_cell_list[i-1+number_in_x*(j-1+number_in_y*
												k)])->scalar_value>iso_value)
											{
												if (slit_type&0x4)
												{
													in_mesh[4]=1;
												}
												else
												{
													in_mesh[0]=1;
												}
											}
										}
									}
									if (j<number_in_y)
									{
										if (k>0)
										{
											if ((texture_cell_list[i-1+number_in_x*(j+number_in_y*
												(k-1))])->scalar_value>iso_value)
											{
												if (slit_type&0x2)
												{
													in_mesh[2]=1;
												}
												else
												{
													in_mesh[0]=1;
												}
											}
										}
										if (k<number_in_z)
										{
											if ((texture_cell_list[i-1+number_in_x*(j+number_in_y*
												k)])->scalar_value>iso_value)
											{
												if (slit_type&0x4)
												{
													if (slit_type&0x2)
													{
														in_mesh[6]=1;
													}
													else
													{
														in_mesh[4]=1;
													}
												}
												else
												{
													if (slit_type&0x2)
													{
														in_mesh[2]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
										}
									}
								}
								if (i<number_in_x)
								{
									if (j>0)
									{
										if (k>0)
										{
											if ((texture_cell_list[i+number_in_x*(j-1+number_in_y*
												(k-1))])->scalar_value>iso_value)
											{
												if (slit_type&0x1)
												{
													in_mesh[1]=1;
												}
												else
												{
													in_mesh[0]=1;
												}
											}
										}
										if (k<number_in_z)
										{
											if ((texture_cell_list[i+number_in_x*(j-1+number_in_y*
												k)])->scalar_value>iso_value)
											{
												if (slit_type&0x4)
												{
													if (slit_type&0x1)
													{
														in_mesh[5]=1;
													}
													else
													{
														in_mesh[4]=1;
													}
												}
												else
												{
													if (slit_type&0x1)
													{
														in_mesh[1]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
										}
									}
									if (j<number_in_y)
									{
										if (k>0)
										{
											if ((texture_cell_list[i+number_in_x*(j+number_in_y*
												(k-1))])->scalar_value>iso_value)
											{
												if (slit_type&0x2)
												{
													if (slit_type&0x1)
													{
														in_mesh[3]=1;
													}
													else
													{
														in_mesh[2]=1;
													}
												}
												else
												{
													if (slit_type&0x1)
													{
														in_mesh[1]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
										}
										if (k<number_in_z)
										{
											if ((texture_cell_list[i+number_in_x*(j+number_in_y*k)])->
												scalar_value>iso_value)
											{
												if (slit_type&0x4)
												{
													if (slit_type&0x2)
													{
														if (slit_type&0x1)
														{
															in_mesh[7]=1;
														}
														else
														{
															in_mesh[6]=1;
														}
													}
													else
													{
														if (slit_type&0x1)
														{
															in_mesh[5]=1;
														}
														else
														{
															in_mesh[4]=1;
														}
													}
												}
												else
												{
													if (slit_type&0x2)
													{
														if (slit_type&0x1)
														{
															in_mesh[3]=1;
														}
														else
														{
															in_mesh[2]=1;
														}
													}
													else
													{
														if (slit_type&0x1)
														{
															in_mesh[1]=1;
														}
														else
														{
															in_mesh[0]=1;
														}
													}
												}
											}
										}
									}
								}
								if (coordinate_field_vector)
								{
									if (slit_type&0x1)
									{
										if (i>0)
										{
											x0=(coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))]+
												coordinate_field_vector[3*(i-1+(number_in_x+1)*
												(j+(number_in_y+1)*k))])/2;
										}
										else
										{
											x0=(3*coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))]-
												coordinate_field_vector[3*(i+1+(number_in_x+1)*
												(j+(number_in_y+1)*k))])/2;
										}
										if (i<number_in_x)
										{
											x1=(coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))]+
												coordinate_field_vector[3*(i+1+(number_in_x+1)*
												(j+(number_in_y+1)*k))])/2;
										}
										else
										{
											x1=(3*coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))]-
												coordinate_field_vector[3*(i-1+(number_in_x+1)*
												(j+(number_in_y+1)*k))])/2;
										}
									}
									else
									{
										x0=coordinate_field_vector[3*(i+(number_in_x+1)*
											(j+(number_in_y+1)*k))];
										x1=x0;
									}
									if (slit_type&0x2)
									{
										if (j>0)
										{
											y0=(coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))+1]+
												coordinate_field_vector[3*(i+(number_in_x+1)*
												(j-1+(number_in_y+1)*k))+1])/2;
										}
										else
										{
											y0=(3*coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))+1]-
												coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+1+(number_in_y+1)*k))+1])/2;
										}
										if (j<number_in_y)
										{
											y1=(coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))+1]+
												coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+1+(number_in_y+1)*k))+1])/2;
										}
										else
										{
											y1=(3*coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))+1]-
												coordinate_field_vector[3*(i+(number_in_x+1)*
												(j-1+(number_in_y+1)*k))+1])/2;
										}
									}
									else
									{
										y0=coordinate_field_vector[3*(i+(number_in_x+1)*
											(j+(number_in_y+1)*k))+1];
										y1=y0;
									}
									if (slit_type&0x4)
									{
										if (k>0)
										{
											z0=(coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))+2]+
												coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*(k-1)))+2])/2;
										}
										else
										{
											z0=(3*coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))+2]-
												coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*(k+1)))+2])/2;
										}
										if (k<number_in_z)
										{
											z1=(coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))+2]+
												coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*(k+1)))+2])/2;
										}
										else
										{
											z1=(3*coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*k))+2]-
												coordinate_field_vector[3*(i+(number_in_x+1)*
												(j+(number_in_y+1)*(k-1)))+2])/2;
										}
									}
									else
									{
										z0=coordinate_field_vector[3*(i+(number_in_x+1)*
											(j+(number_in_y+1)*k))+2];
										z1=z0;
									}
								}
								else
								{
									x0=x_min+((x_max-x_min)*(float)i)/(float)number_in_x;
									if (slit_type&0x1)
									{
										x0 -= (x_max-x_min)/(float)(2*number_in_x);
										x1=x0+(x_max-x_min)/(float)number_in_x;
									}
									else
									{
										x1=x0;
									}
									y0=y_min+((y_max-y_min)*(float)i)/(float)number_in_y;
									if (slit_type&0x2)
									{
										y0 -= (y_max-y_min)/(float)(2*number_in_y);
										y1=y0+(y_max-y_min)/(float)number_in_y;
									}
									else
									{
										y1=y0;
									}
									z0=z_min+((z_max-z_min)*(float)i)/(float)number_in_z;
									if (slit_type&0x4)
									{
										z0 -= (z_max-z_min)/(float)(2*number_in_z);
										z1=z0+(z_max-z_min)/(float)number_in_z;
									}
									else
									{
										z1=z0;
									}
								}
								if (((i==x_output_elements)||(-1==x_output_elements))&&
									((j==y_output_elements)||(-1==y_output_elements))&&
									((k==z_output_elements)||(-1==z_output_elements)))
								{
									for (l=0;l<8;l++)
									{
										if (in_mesh[l])
										{
											fprintf(out_file,"Node: %d\n",starting_node_number+
												(texture_node->cm_node_identifier)[l]);
											if (write_coordinate_field)
											{
												if ((unsigned char)l&0x1)
												{
													fprintf(out_file,"%g",x1);
												}
												else
												{
													fprintf(out_file,"%g",x0);
												}
												derivative_value=1;
												switch (x_interpolation)
												{
													case CUBIC_HERMITE:
													{
														fprintf(out_file," 1");
														derivative_value=2;
													} break;
												}
												for (m=derivative_value;m<number_of_values;m++)
												{
													fprintf(out_file," 0");
												}
												fprintf(out_file,"\n");
												if ((unsigned char)l&0x2)
												{
													fprintf(out_file,"%g",y1);
												}
												else
												{
													fprintf(out_file,"%g",y0);
												}
												m=1;
												while (m<derivative_value)
												{
													fprintf(out_file," 0");
													m++;
												}
												switch (y_interpolation)
												{
													case CUBIC_HERMITE:
													{
														fprintf(out_file," 1");
														derivative_value *= 2;
														m++;
													} break;
												}
												while (m<number_of_values)
												{
													fprintf(out_file," 0");
													m++;
												}
												fprintf(out_file,"\n");
												if ((unsigned char)l&0x4)
												{
													fprintf(out_file,"%g",z1);
												}
												else
												{
													fprintf(out_file,"%g",z0);
												}
												m=1;
												while (m<derivative_value)
												{
													fprintf(out_file," 0");
													m++;
												}
												switch (z_interpolation)
												{
													case CUBIC_HERMITE:
													{
														fprintf(out_file," 1");
														derivative_value *= 2;
														m++;
													} break;
												}
												while (m<number_of_values)
												{
													fprintf(out_file," 0");
													m++;
												}
												fprintf(out_file,"\n");
											}
											if (write_undeformed_field)
											{
												if ((unsigned char)l&0x1)
												{
													fprintf(out_file,"%g",x1);
												}
												else
												{
													fprintf(out_file,"%g",x0);
												}
												derivative_value=1;
												switch (x_interpolation)
												{
													case CUBIC_HERMITE:
													{
														fprintf(out_file," 1");
														derivative_value=2;
													} break;
												}
												for (m=derivative_value;m<number_of_values;m++)
												{
													fprintf(out_file," 0");
												}
												fprintf(out_file,"\n");
												if ((unsigned char)l&0x2)
												{
													fprintf(out_file,"%g",y1);
												}
												else
												{
													fprintf(out_file,"%g",y0);
												}
												m=1;
												while (m<derivative_value)
												{
													fprintf(out_file," 0");
													m++;
												}
												switch (y_interpolation)
												{
													case CUBIC_HERMITE:
													{
														fprintf(out_file," 1");
														derivative_value *= 2;
														m++;
													} break;
												}
												while (m<number_of_values)
												{
													fprintf(out_file," 0");
													m++;
												}
												fprintf(out_file,"\n");
												if ((unsigned char)l&0x4)
												{
													fprintf(out_file,"%g",z1);
												}
												else
												{
													fprintf(out_file,"%g",z0);
												}
												m=1;
												while (m<derivative_value)
												{
													fprintf(out_file," 0");
													m++;
												}
												switch (z_interpolation)
												{
													case CUBIC_HERMITE:
													{
														fprintf(out_file," 1");
														derivative_value *= 2;
														m++;
													} break;
												}
												while (m<number_of_values)
												{
													fprintf(out_file," 0");
													m++;
												}
												fprintf(out_file,"\n");
											}
											if (write_fibre_field)
											{
												/* fibre angle */
												fprintf(out_file,"0");
												for (m=1;m<number_of_values;m++)
												{
													fprintf(out_file," 0");
												}
												fprintf(out_file,"\n");
												/* embrication angle */
												fprintf(out_file,"0");
												for (m=1;m<number_of_values;m++)
												{
													fprintf(out_file," 0");
												}
												fprintf(out_file,"\n");
												/* sheet angle */
												fprintf(out_file,"0");
												for (m=1;m<number_of_values;m++)
												{
													fprintf(out_file," 0");
												}
												fprintf(out_file,"\n");
											}
										}
									}
								}
								number++;
							}
						}
					}
#if defined (OLD_CODE)
					/* write out node groups (if any) */
					for (i=0;i<volume_texture->n_groups;i++)
					{
						/* write heading */
						fprintf(out_file,"Group name : %s\n",
							(volume_texture->node_groups[i])->name);
						fprintf(out_file,"#Fields=0\n");
						for (j=0;j<(volume_texture->node_groups[i])->n_nodes;j++)
						{
							number=(volume_texture->node_groups[i])->nodes[j]+
								starting_node_number;
							fprintf(out_file,"Node: %d\n",number);
						}
					}
#endif /* defined (OLD_CODE) */
					fclose(out_file);
					/* write the elements */
					element_dimension=0;
					if (ALLOCATE(basis_name,char,1))
					{
						basis_name[0]='\0';
						if (-1==x_output_elements)
						{
							element_dimension++;
							switch (x_interpolation)
							{
								case LINEAR_LAGRANGE:
								{
									if (REALLOCATE(temp_name,basis_name,char,
										strlen(basis_name)+11))
									{
										basis_name=temp_name;
										strcat(basis_name,"l.Lagrange");
									}
									else
									{
										DEALLOCATE(basis_name);
									}
								} break;
								case CUBIC_HERMITE:
								{
									if (REALLOCATE(temp_name,basis_name,char,
										strlen(basis_name)+10))
									{
										basis_name=temp_name;
										strcat(basis_name,"c.Hermite");
									}
									else
									{
										DEALLOCATE(basis_name);
									}
								} break;
							}
						}
						if ((-1==y_output_elements)&&basis_name)
						{
							if (0!=element_dimension)
							{
								if (REALLOCATE(temp_name,basis_name,char,strlen(basis_name)+2))
								{
									basis_name=temp_name;
									strcat(basis_name,"*");
								}
								else
								{
									DEALLOCATE(basis_name);
								}
							}
							element_dimension++;
							if (basis_name)
							{
								switch (y_interpolation)
								{
									case LINEAR_LAGRANGE:
									{
										if (REALLOCATE(temp_name,basis_name,char,
											strlen(basis_name)+11))
										{
											basis_name=temp_name;
											strcat(basis_name,"l.Lagrange");
										}
										else
										{
											DEALLOCATE(basis_name);
										}
									} break;
									case CUBIC_HERMITE:
									{
										if (REALLOCATE(temp_name,basis_name,char,
											strlen(basis_name)+10))
										{
											basis_name=temp_name;
											strcat(basis_name,"c.Hermite");
										}
										else
										{
											DEALLOCATE(basis_name);
										}
									} break;
								}
							}
						}
						if ((-1==z_output_elements)&&basis_name)
						{
							if (0!=element_dimension)
							{
								if (REALLOCATE(temp_name,basis_name,char,strlen(basis_name)+2))
								{
									basis_name=temp_name;
									strcat(basis_name,"*");
								}
								else
								{
									DEALLOCATE(basis_name);
								}
							}
							element_dimension++;
							if (basis_name)
							{
								switch (z_interpolation)
								{
									case LINEAR_LAGRANGE:
									{
										if (REALLOCATE(temp_name,basis_name,char,
											strlen(basis_name)+11))
										{
											basis_name=temp_name;
											strcat(basis_name,"l.Lagrange");
										}
										else
										{
											DEALLOCATE(basis_name);
										}
									} break;
									case CUBIC_HERMITE:
									{
										if (REALLOCATE(temp_name,basis_name,char,
											strlen(basis_name)+10))
										{
											basis_name=temp_name;
											strcat(basis_name,"c.Hermite");
										}
										else
										{
											DEALLOCATE(basis_name);
										}
									} break;
								}
							}
						}
					}
					if ((0<element_dimension)&&basis_name)
					{
						strcpy(file_name,value_string);
						strcpy(file_name2,value_string);
						strcat(file_name,".exelem");
						strcat(file_name2,".exelem.layout");

						if (out_file=fopen(file_name,"w"))
						{
							/* write heading */
							fprintf(out_file,"Group name : %s\n",value_string);
							/* write the lines */
							fprintf(out_file,"Shape.  Dimension=1\n");
							if (1==element_dimension)
							{
								fprintf(out_file,"#Scale factor sets=1\n");
								fprintf(out_file,"  %s, #Scale factors=%d\n",basis_name,
									2*number_of_values);
								fprintf(out_file,"#Nodes=2\n");
								fprintf(out_file,"#Fields=%d\n",number_of_fields);
								number_of_fields=0;
								if (write_coordinate_field)
								{
									number_of_fields++;
									fprintf(out_file,
					"%d) coordinates, coordinate, rectangular cartesian, #Components=3\n",
										number_of_fields);
									fprintf(out_file,
										"  x.  %s, no modify, standard node based.\n",basis_name);
									fprintf(out_file,"    #Nodes=2\n");
									for (i=1;i<=2;i++)
									{
										fprintf(out_file,"      %d.  #Values=%d\n",i,
											number_of_values);
										fprintf(out_file,"        Value indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",j);
										}
										fprintf(out_file,"\n");
										fprintf(out_file,"        Scale factor indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",(i-1)*number_of_values+j);
										}
										fprintf(out_file,"\n");
									}
									fprintf(out_file,
										"  y.  %s, no modify, standard node based.\n",basis_name);
									fprintf(out_file,"    #Nodes=2\n");
									for (i=1;i<=2;i++)
									{
										fprintf(out_file,"      %d.  #Values=%d\n",i,
											number_of_values);
										fprintf(out_file,"        Value indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",j);
										}
										fprintf(out_file,"\n");
										fprintf(out_file,"        Scale factor indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",(i-1)*number_of_values+j);
										}
										fprintf(out_file,"\n");
									}
									fprintf(out_file,
										"  z.  %s, no modify, standard node based.\n",basis_name);
									fprintf(out_file,"    #Nodes=2\n");
									for (i=1;i<=2;i++)
									{
										fprintf(out_file,"      %d.  #Values=%d\n",i,
											number_of_values);
										fprintf(out_file,"        Value indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",j);
										}
										fprintf(out_file,"\n");
										fprintf(out_file,"        Scale factor indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",(i-1)*number_of_values+j);
										}
										fprintf(out_file,"\n");
									}
								}
								if (write_undeformed_field)
								{
									number_of_fields++;
									fprintf(out_file,
					"%d) undeformed, coordinate, rectangular cartesian, #Components=3\n",
										number_of_fields);
									fprintf(out_file,
										"  x.  %s, no modify, standard node based.\n",basis_name);
									fprintf(out_file,"    #Nodes=2\n");
									for (i=1;i<=2;i++)
									{
										fprintf(out_file,"      %d.  #Values=%d\n",i,
											number_of_values);
										fprintf(out_file,"        Value indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",j);
										}
										fprintf(out_file,"\n");
										fprintf(out_file,"        Scale factor indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",(i-1)*number_of_values+j);
										}
										fprintf(out_file,"\n");
									}
									fprintf(out_file,
										"  y.  %s, no modify, standard node based.\n",basis_name);
									fprintf(out_file,"    #Nodes=2\n");
									for (i=1;i<=2;i++)
									{
										fprintf(out_file,"      %d.  #Values=%d\n",i,
											number_of_values);
										fprintf(out_file,"        Value indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",j);
										}
										fprintf(out_file,"\n");
										fprintf(out_file,"        Scale factor indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",(i-1)*number_of_values+j);
										}
										fprintf(out_file,"\n");
									}
									fprintf(out_file,
										"  z.  %s, no modify, standard node based.\n",basis_name);
									fprintf(out_file,"    #Nodes=2\n");
									for (i=1;i<=2;i++)
									{
										fprintf(out_file,"      %d.  #Values=%d\n",i,
											number_of_values);
										fprintf(out_file,"        Value indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",j);
										}
										fprintf(out_file,"\n");
										fprintf(out_file,"        Scale factor indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",(i-1)*number_of_values+j);
										}
										fprintf(out_file,"\n");
									}
								}
								if (write_fibre_field)
								{
									number_of_fields++;
									fprintf(out_file,
										"%d) fibres, anatomical, fibre, #Components=3\n",
										number_of_fields);
									fprintf(out_file,
										"  fibre angle.  %s, no modify, standard node based.\n",
										basis_name);
									fprintf(out_file,"    #Nodes=2\n");
									for (i=1;i<=2;i++)
									{
										fprintf(out_file,"      %d.  #Values=%d\n",i,
											number_of_values);
										fprintf(out_file,"        Value indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",j);
										}
										fprintf(out_file,"\n");
										fprintf(out_file,"        Scale factor indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",(i-1)*number_of_values+j);
										}
										fprintf(out_file,"\n");
									}
									fprintf(out_file,
									"  embrication angle.  %s, no modify, standard node based.\n",
										basis_name);
									fprintf(out_file,"    #Nodes=2\n");
									for (i=1;i<=2;i++)
									{
										fprintf(out_file,"      %d.  #Values=%d\n",i,
											number_of_values);
										fprintf(out_file,"        Value indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",j);
										}
										fprintf(out_file,"\n");
										fprintf(out_file,"        Scale factor indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",(i-1)*number_of_values+j);
										}
										fprintf(out_file,"\n");
									}
									fprintf(out_file,
										"  sheet angle.  %s, no modify, standard node based.\n",
										basis_name);
									fprintf(out_file,"    #Nodes=2\n");
									for (i=1;i<=2;i++)
									{
										fprintf(out_file,"      %d.  #Values=%d\n",i,
											number_of_values);
										fprintf(out_file,"        Value indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",j);
										}
										fprintf(out_file,"\n");
										fprintf(out_file,"        Scale factor indices:");
										for (j=1;j<=number_of_values;j++)
										{
											fprintf(out_file," %d",(i-1)*number_of_values+j);
										}
										fprintf(out_file,"\n");
									}
								}
							}
							number=starting_line_number;
							line_offset=3*(number_in_x+1)*(number_in_y+1)*(number_in_z+1)-
								(number_in_x+1)*(number_in_y+1)-(number_in_x+1)*(number_in_z+1)-
								(number_in_y+1)*(number_in_z+1);
							for (i=0;i<=number_in_x;i++)
							{
								for (j=0;j<=number_in_y;j++)
								{
									for (k=0;k<number_in_z;k++)
									{
										for (l=0;l<4;l++)
										{
											in_mesh[l]=0;
										}
										slit_type=
											(unsigned char)((texture_node_list[
											i+(number_in_x+1)*(j+(number_in_y+1)*k)])->node_type)|
											(unsigned char)((texture_node_list[
											i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->node_type);
										if (i>0)
										{
											if (j>0)
											{
												if ((texture_cell_list[i-1+number_in_x*(j-1+number_in_y*
													k)])->scalar_value>iso_value)
												{
													in_mesh[0]=1;
												}
											}
											if (j<number_in_y)
											{
												if ((texture_cell_list[i-1+number_in_x*(j+number_in_y*
													k)])->scalar_value>iso_value)
												{
													if (slit_type&0x2)
													{
														in_mesh[2]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
										}
										if (i<number_in_x)
										{
											if (j>0)
											{
												if ((texture_cell_list[i+number_in_x*(j-1+number_in_y*
													k)])->scalar_value>iso_value)
												{
													if (slit_type&0x1)
													{
														in_mesh[1]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
											if (j<number_in_y)
											{
												if ((texture_cell_list[i+number_in_x*(j+number_in_y*
													k)])->scalar_value>iso_value)
												{
													if (slit_type&0x2)
													{
														if (slit_type&0x1)
														{
															in_mesh[3]=1;
														}
														else
														{
															in_mesh[2]=1;
														}
													}
													else
													{
														if (slit_type&0x1)
														{
															in_mesh[1]=1;
														}
														else
														{
															in_mesh[0]=1;
														}
													}
												}
											}
										}
										if (((i==x_output_elements)||(-1==x_output_elements))&&
											((j==y_output_elements)||(-1==y_output_elements))&&
											(-1==z_output_elements))
										{
											for (l=0;l<3;l++)
											{
												if (in_mesh[l])
												{
													fprintf(out_file,"Element: 0 0 %d\n",
														number+l*line_offset);
													if (1==element_dimension)
													{
														fprintf(out_file,"  Nodes:\n");
														fprintf(out_file,"    %d %d\n",
															starting_node_number+((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															cm_node_identifier)[7],
															starting_node_number+((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
															cm_node_identifier)[3]);
														fprintf(out_file,"  Scale factors:\n   ");
														/* node 1 */
														if (coordinate_field_vector)
														{
															index=i+(number_in_x+1)*(j+(number_in_y+1)*k);
															if (0==k)
															{
																scale_factor_z=coordinate_field_vector[3*(index+
																	(number_in_x+1)*(number_in_y+1))+2]-
																	coordinate_field_vector[3*index+2];
															}
															else
															{
																scale_factor_z=(coordinate_field_vector[
																	3*(index+(number_in_x+1)*(number_in_y+1))+2]-
																	coordinate_field_vector[3*(index-
																	(number_in_x+1)*(number_in_y+1))+2])/2;
															}
															derivative_value=1;
															scale_factors[0]=1;
															switch (z_interpolation)
															{
																case CUBIC_HERMITE:
																{
																	for (l=0;l<derivative_value;l++)
																	{
																		scale_factors[l+derivative_value]=
																			scale_factors[i]*scale_factor_z;
																	}
																	derivative_value *= 2;
																} break;
															}
														}
														for (l=0;l<number_of_values;l++)
														{
															fprintf(out_file," %g",scale_factors[l]);
														}
														fprintf(out_file,"\n   ");
														/* node 2 */
														if (coordinate_field_vector)
														{
															index=i+(number_in_x+1)*(j+(number_in_y+1)*(k+1));
															if (0==k)
															{
																scale_factor_z=coordinate_field_vector[3*(index+
																	(number_in_x+1)*(number_in_y+1))+2]-
																	coordinate_field_vector[3*index+2];
															}
															else
															{
																scale_factor_z=(coordinate_field_vector[
																	3*(index+(number_in_x+1)*(number_in_y+1))+2]-
																	coordinate_field_vector[3*(index-
																	(number_in_x+1)*(number_in_y+1))+2])/2;
															}
															derivative_value=1;
															scale_factors[0]=1;
															switch (z_interpolation)
															{
																case CUBIC_HERMITE:
																{
																	for (l=0;l<derivative_value;l++)
																	{
																		scale_factors[l+derivative_value]=
																			scale_factors[i]*scale_factor_z;
																	}
																	derivative_value *= 2;
																} break;
															}
														}
														for (l=0;l<number_of_values;l++)
														{
															fprintf(out_file," %g",scale_factors[l]);
														}
														fprintf(out_file,"\n");
													}
												}
											}
										}
										number++;
									}
								}
							}
							for (i=0;i<=number_in_x;i++)
							{
								for (j=0;j<number_in_y;j++)
								{
									for (k=0;k<=number_in_z;k++)
									{
										for (l=0;l<4;l++)
										{
											in_mesh[l]=0;
										}
										slit_type=
											(unsigned char)((texture_node_list[
											i+(number_in_x+1)*(j+(number_in_y+1)*k)])->node_type)|
											(unsigned char)((texture_node_list[
											i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->node_type);
										if (i>0)
										{
											if (k>0)
											{
												if ((texture_cell_list[i-1+number_in_x*(j+number_in_y*
													(k-1))])->scalar_value>iso_value)
												{
													in_mesh[0]=1;
												}
											}
											if (k<number_in_z)
											{
												if ((texture_cell_list[i-1+number_in_x*(j+number_in_y*
													k)])->scalar_value>iso_value)
												{
													if (slit_type&0x4)
													{
														in_mesh[2]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
										}
										if (i<number_in_x)
										{
											if (k>0)
											{
												if ((texture_cell_list[i+number_in_x*(j+number_in_y*
													(k-1))])->scalar_value>iso_value)
												{
													if (slit_type&0x1)
													{
														in_mesh[1]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
											if (k<number_in_z)
											{
												if ((texture_cell_list[i+number_in_x*(j+number_in_y*
													k)])->scalar_value>iso_value)
												{
													if (slit_type&0x4)
													{
														if (slit_type&0x1)
														{
															in_mesh[3]=1;
														}
														else
														{
															in_mesh[2]=1;
														}
													}
													else
													{
														if (slit_type&0x1)
														{
															in_mesh[1]=1;
														}
														else
														{
															in_mesh[0]=1;
														}
													}
												}
											}
										}
										if (((i==x_output_elements)||(-1==x_output_elements))&&
											(-1==y_output_elements)&&
											((k==z_output_elements)||(-1==z_output_elements)))
										{
											for (l=0;l<3;l++)
											{
												if (in_mesh[l])
												{
													fprintf(out_file,"Element: 0 0 %d\n",
														number+l*line_offset);
													if (1==element_dimension)
													{
														fprintf(out_file,"  Nodes:\n");
														fprintf(out_file,"    %d %d\n",
															starting_node_number+((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															cm_node_identifier)[7],
															starting_node_number+((texture_node_list[
															i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
															cm_node_identifier)[5]);
														fprintf(out_file,"  Scale factors:\n   ");
														/* node 1 */
														if (coordinate_field_vector)
														{
															index=i+(number_in_x+1)*(j+(number_in_y+1)*k);
															if (0==j)
															{
																scale_factor_y=coordinate_field_vector[
																	3*(index+number_in_x+1)+1]-
																	coordinate_field_vector[3*index+1];
															}
															else
															{
																scale_factor_y=(coordinate_field_vector[
																	3*(index+number_in_x+1)+1]-
																	coordinate_field_vector[
																	3*(index-number_in_x-1)+1])/2;
															}
															derivative_value=1;
															scale_factors[0]=1;
															switch (y_interpolation)
															{
																case CUBIC_HERMITE:
																{
																	for (l=0;l<derivative_value;l++)
																	{
																		scale_factors[l+derivative_value]=
																			scale_factors[l]*scale_factor_y;
																	}
																	derivative_value *= 2;
																} break;
															}
														}
														for (l=0;l<number_of_values;l++)
														{
															fprintf(out_file," %g",scale_factors[l]);
														}
														fprintf(out_file,"\n   ");
														/* node 2 */
														if (coordinate_field_vector)
														{
															index=i+(number_in_x+1)*(j+1+(number_in_y+1)*k);
															if (0==j)
															{
																scale_factor_y=coordinate_field_vector[
																	3*(index+number_in_x+1)+1]-
																	coordinate_field_vector[3*index+1];
															}
															else
															{
																scale_factor_y=(coordinate_field_vector[
																	3*(index+number_in_x+1)+1]-
																	coordinate_field_vector[
																	3*(index-number_in_x-1)+1])/2;
															}
															derivative_value=1;
															scale_factors[0]=1;
															switch (y_interpolation)
															{
																case CUBIC_HERMITE:
																{
																	for (l=0;l<derivative_value;l++)
																	{
																		scale_factors[l+derivative_value]=
																			scale_factors[l]*scale_factor_y;
																	}
																	derivative_value *= 2;
																} break;
															}
														}
														for (l=0;l<number_of_values;l++)
														{
															fprintf(out_file," %g",scale_factors[l]);
														}
														fprintf(out_file,"\n");
													}
												}
											}
										}
										number++;
									}
								}
							}
							for (i=0;i<number_in_x;i++)
							{
								for (j=0;j<=number_in_y;j++)
								{
									for (k=0;k<=number_in_z;k++)
									{
										for (l=0;l<4;l++)
										{
											in_mesh[l]=0;
										}
										slit_type=
											(unsigned char)((texture_node_list[
											i+(number_in_x+1)*(j+(number_in_y+1)*k)])->node_type)|
											(unsigned char)((texture_node_list[
											i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->node_type);
										if (j>0)
										{
											if (k>0)
											{
												if ((texture_cell_list[i+number_in_x*(j-1+number_in_y*
													(k-1))])->scalar_value>iso_value)
												{
													in_mesh[0]=1;
												}
											}
											if (k<number_in_z)
											{
												if ((texture_cell_list[i+number_in_x*(j-1+number_in_y*
													k)])->scalar_value>iso_value)
												{
													if (slit_type&0x4)
													{
														in_mesh[2]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
										}
										if (j<number_in_y)
										{
											if (k>0)
											{
												if ((texture_cell_list[i+number_in_x*(j+number_in_y*
													(k-1))])->scalar_value>iso_value)
												{
													if (slit_type&0x2)
													{
														in_mesh[1]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
											if (k<number_in_z)
											{
												if ((texture_cell_list[i+number_in_x*(j+number_in_y*
													k)])->scalar_value>iso_value)
												{
													if (slit_type&0x4)
													{
														if (slit_type&0x2)
														{
															in_mesh[3]=1;
														}
														else
														{
															in_mesh[2]=1;
														}
													}
													else
													{
														if (slit_type&0x2)
														{
															in_mesh[1]=1;
														}
														else
														{
															in_mesh[0]=1;
														}
													}
												}
											}
										}
										if ((-1==x_output_elements)&&
											((j==y_output_elements)||(-1==y_output_elements))&&
											((k==z_output_elements)||(-1==z_output_elements)))
										{
											for (l=0;l<3;l++)
											{
												if (in_mesh[l])
												{
													fprintf(out_file,"Element: 0 0 %d\n",
														number+l*line_offset);
													if (1==element_dimension)
													{
														fprintf(out_file,"  Nodes:\n");
														fprintf(out_file,"    %d %d\n",
															starting_node_number+((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															cm_node_identifier)[7],
															starting_node_number+((texture_node_list[
															i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															cm_node_identifier)[6]);
														fprintf(out_file,"  Scale factors:\n   ");
														/* node 1 */
														if (coordinate_field_vector)
														{
															index=i+(number_in_x+1)*(j+(number_in_y+1)*k);
															if (0==i)
															{
																scale_factor_x=
																	coordinate_field_vector[3*(index+1)]-
																	coordinate_field_vector[3*index];
															}
															else
															{
																scale_factor_x=
																	(coordinate_field_vector[3*(index+1)]-
																	coordinate_field_vector[3*(index-1)])/2;
															}
															derivative_value=1;
															scale_factors[0]=1;
															switch (x_interpolation)
															{
																case CUBIC_HERMITE:
																{
																	for (l=0;l<derivative_value;l++)
																	{
																		scale_factors[l+derivative_value]=
																			scale_factors[l]*scale_factor_x;
																	}
																	derivative_value *= 2;
																} break;
															}
														}
														for (l=0;l<number_of_values;l++)
														{
															fprintf(out_file," %g",scale_factors[l]);
														}
														fprintf(out_file,"\n   ");
														/* node 2 */
														if (coordinate_field_vector)
														{
															index=i+1+(number_in_x+1)*(j+(number_in_y+1)*k);
															if (0==i)
															{
																scale_factor_x=
																	coordinate_field_vector[3*(index+1)]-
																	coordinate_field_vector[3*index];
															}
															else
															{
																scale_factor_x=
																	(coordinate_field_vector[3*(index+1)]-
																	coordinate_field_vector[3*(index-1)])/2;
															}
															derivative_value=1;
															scale_factors[0]=1;
															switch (x_interpolation)
															{
																case CUBIC_HERMITE:
																{
																	for (l=0;l<derivative_value;l++)
																	{
																		scale_factors[l+derivative_value]=
																			scale_factors[l]*scale_factor_x;
																	}
																	derivative_value *= 2;
																} break;
															}
														}
														for (l=0;l<number_of_values;l++)
														{
															fprintf(out_file," %g",scale_factors[l]);
														}
														fprintf(out_file,"\n");
													}
												}
											}
										}
										number++;
									}
								}
							}
							if (1<element_dimension)
							{
								/* write the faces */
								fprintf(out_file,"Shape.  Dimension=2\n");
								if (2==element_dimension)
								{
									fprintf(out_file,"#Scale factor sets=1\n");
									fprintf(out_file,"  %s, #Scale factors=%d\n",basis_name,
										4*number_of_values);
									fprintf(out_file,"#Nodes=4\n");
									fprintf(out_file,"#Fields=%d\n",number_of_fields);
									number_of_fields=0;
									if (write_coordinate_field)
									{
										number_of_fields++;
										fprintf(out_file,
					"%d) coordinates, coordinate, rectangular cartesian, #Components=3\n",
											number_of_fields);
										fprintf(out_file,
											"  x.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=4\n");
										for (i=1;i<=4;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
											"  y.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=4\n");
										for (i=1;i<=4;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
											"  z.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=4\n");
										for (i=1;i<=4;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
									}
									if (write_undeformed_field)
									{
										number_of_fields++;
										fprintf(out_file,
					"%d) undeformed, coordinate, rectangular cartesian, #Components=3\n",
											number_of_fields);
										fprintf(out_file,
											"  x.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=4\n");
										for (i=1;i<=4;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
											"  y.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=4\n");
										for (i=1;i<=4;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
											"  z.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=4\n");
										for (i=1;i<=4;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
									}
									if (write_fibre_field)
									{
										number_of_fields++;
										fprintf(out_file,
											"%d) fibres, anatomical, fibre, #Components=3\n",
											number_of_fields);
										fprintf(out_file,
											"  fibre angle.  %s, no modify, standard node based.\n",
											basis_name);
										fprintf(out_file,"    #Nodes=4\n");
										for (i=1;i<=4;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
									"  embrication angle.  %s, no modify, standard node based.\n",
											basis_name);
										fprintf(out_file,"    #Nodes=4\n");
										for (i=1;i<=4;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
											"  sheet angle.  %s, no modify, standard node based.\n",
											basis_name);
										fprintf(out_file,"    #Nodes=4\n");
										for (i=1;i<=4;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
									}
								}
								number=starting_face_number;
								face_offset=3*number_in_x*number_in_y*number_in_z+
									number_in_x*number_in_y+number_in_x*number_in_z+
									number_in_y*number_in_z;
								for (i=0;i<=number_in_x;i++)
								{
									for (j=0;j<number_in_y;j++)
									{
										for (k=0;k<number_in_z;k++)
										{
											for (l=0;l<2;l++)
											{
												in_mesh[l]=0;
											}
											slit_type=
												(unsigned char)((texture_node_list[
												i+(number_in_x+1)*(j+(number_in_y+1)*k)])->node_type)|
												(unsigned char)((texture_node_list[
												i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
												node_type)|(unsigned char)((texture_node_list[
												i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->node_type)|
												(unsigned char)((texture_node_list[
												i+(number_in_x+1)*(j+1+(number_in_y+1)*(k+1))])->
												node_type);
											if (i>0)
											{
												if ((texture_cell_list[
													i-1+number_in_x*(j+number_in_y*k)])->scalar_value>
													iso_value)
												{
													in_mesh[0]=1;
												}
											}
											if (i<number_in_x)
											{
												if ((texture_cell_list[
													i+number_in_x*(j+number_in_y*k)])->
													scalar_value>iso_value)
												{
													if (slit_type&0x1)
													{
														in_mesh[1]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
											if (((-1==x_output_elements)||(i==x_output_elements))&&
												(-1==y_output_elements)&&(-1==z_output_elements))
											{
												for (l=0;l<2;l++)
												{
													if (in_mesh[l])
													{
														fprintf(out_file,"Element: 0 %d 0\n",
															number+l*face_offset);
														fprintf(out_file,"  Faces:\n");
														slit_type=
															(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															node_type)|(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
															node_type);
														if (slit_type&0x2)
														{
															if (slit_type&0x1)
															{
																m=2+l;
															}
															else
															{
																m=2;
															}
														}
														else
														{
															if (slit_type&0x1)
															{
																m=l;
															}
															else
															{
																m=0;
															}
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+(i*(number_in_y+1)+j)*
															number_in_z+k+m*line_offset);
														slit_type=(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
															node_type)|(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+1+(number_in_y+1)*(k+1))])->
															node_type);
														if (slit_type&0x1)
														{
															m=l;
														}
														else
														{
															m=0;
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+(i*(number_in_y+1)+j+1)*
															number_in_z+k+m*line_offset);
														slit_type=(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															node_type)|(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
															node_type);
														if (slit_type&0x4)
														{
															if (slit_type&0x1)
															{
																m=2+l;
															}
															else
															{
																m=2;
															}
														}
														else
														{
															if (slit_type&0x1)
															{
																m=l;
															}
															else
															{
																m=0;
															}
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+number_in_z*(number_in_y+1)*
															(number_in_x+1)+(i*number_in_y+j)*(number_in_z+1)+
															k+m*line_offset);
														slit_type=(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
															node_type)|(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+1+(number_in_y+1)*(k+1))])->
															node_type);
														if (slit_type&0x1)
														{
															m=l;
														}
														else
														{
															m=0;
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+number_in_z*(number_in_y+1)*
															(number_in_x+1)+(i*number_in_y+j)*(number_in_z+1)+
															k+1+m*line_offset);
														if (2==element_dimension)
														{
															fprintf(out_file,"  Nodes:\n");
															fprintf(out_file,"    %d %d %d %d\n",
																starting_node_number+((texture_node_list[
																i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
																cm_node_identifier)[7],
																starting_node_number+((texture_node_list[
																i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
																cm_node_identifier)[5],
																starting_node_number+((texture_node_list[
																i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
																cm_node_identifier)[3],
																starting_node_number+((texture_node_list[
																i+(number_in_x+1)*(j+1+(number_in_y+1)*
																(k+1))])->cm_node_identifier)[1]);
															fprintf(out_file,"  Scale factors:\n   ");
															/* node 1 */
															if (coordinate_field_vector)
															{
																index=i+(number_in_x+1)*(j+(number_in_y+1)*k);
																if (0==j)
																{
																	scale_factor_y=coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*index+1];
																}
																else
																{
																	scale_factor_y=(coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[
																		3*(index-number_in_x-1)+1])/2;
																}
																if (0==k)
																{
																	scale_factor_z=coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*index+2];
																}
																else
																{
																	scale_factor_z=(coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*(index-
																		(number_in_x+1)*(number_in_y+1))+2])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (y_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_y;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (z_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[i]*scale_factor_z;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n   ");
															/* node 2 */
															if (coordinate_field_vector)
															{
																index=i+(number_in_x+1)*(j+1+(number_in_y+1)*k);
																if (0==j)
																{
																	scale_factor_y=coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*index+1];
																}
																else
																{
																	scale_factor_y=(coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[
																		3*(index-number_in_x-1)+1])/2;
																}
																if (0==k)
																{
																	scale_factor_z=coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*index+2];
																}
																else
																{
																	scale_factor_z=(coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*(index-
																		(number_in_x+1)*(number_in_y+1))+2])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (y_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_y;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (z_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[i]*scale_factor_z;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n   ");
															/* node 3 */
															if (coordinate_field_vector)
															{
																index=i+(number_in_x+1)*(j+(number_in_y+1)*
																	(k+1));
																if (0==j)
																{
																	scale_factor_y=coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*index+1];
																}
																else
																{
																	scale_factor_y=(coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[
																		3*(index-number_in_x-1)+1])/2;
																}
																if (0==k)
																{
																	scale_factor_z=coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*index+2];
																}
																else
																{
																	scale_factor_z=(coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*(index-
																		(number_in_x+1)*(number_in_y+1))+2])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (y_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_y;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (z_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[i]*scale_factor_z;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n   ");
															/* node 4 */
															if (coordinate_field_vector)
															{
																index=i+(number_in_x+1)*(j+1+(number_in_y+1)*
																	(k+1));
																if (0==j)
																{
																	scale_factor_y=coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*index+1];
																}
																else
																{
																	scale_factor_y=(coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[
																		3*(index-number_in_x-1)+1])/2;
																}
																if (0==k)
																{
																	scale_factor_z=coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*index+2];
																}
																else
																{
																	scale_factor_z=(coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*(index-
																		(number_in_x+1)*(number_in_y+1))+2])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (y_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_y;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (z_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[i]*scale_factor_z;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n");
														}
													}
												}
											}
											number++;
										}
									}
								}
								for (i=0;i<number_in_x;i++)
								{
									for (j=0;j<=number_in_y;j++)
									{
										for (k=0;k<number_in_z;k++)
										{
											for (l=0;l<2;l++)
											{
												in_mesh[l]=0;
											}
											slit_type=
												(unsigned char)((texture_node_list[
												i+(number_in_x+1)*(j+(number_in_y+1)*k)])->node_type)|
												(unsigned char)((texture_node_list[
												i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
												node_type)|(unsigned char)((texture_node_list[
												i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->node_type)|
												(unsigned char)((texture_node_list[
												i+1+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
												node_type);
											if (j>0)
											{
												if ((texture_cell_list[i+number_in_x*(j-1+
													number_in_y*k)])->scalar_value>iso_value)
												{
													in_mesh[0]=1;
												}
											}
											if (j<number_in_y)
											{
												if ((texture_cell_list[i+number_in_x*
													(j+number_in_y*k)])->scalar_value>iso_value)
												{
													if (slit_type&0x2)
													{
														in_mesh[1]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
											if ((-1==x_output_elements)&&
												((-1==y_output_elements)||(j==y_output_elements))&&
												(-1==z_output_elements))
											{
												for (l=0;l<2;l++)
												{
													if (in_mesh[l])
													{
														fprintf(out_file,"Element: 0 %d 0\n",
															number+l*face_offset);
														fprintf(out_file,"  Faces:\n");
														slit_type=
															(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															node_type)|(unsigned char)((texture_node_list[
															i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															node_type);
														if (slit_type&0x4)
														{
															if (slit_type&0x2)
															{
																m=2+l;
															}
															else
															{
																m=2;
															}
														}
														else
														{
															if (slit_type&0x2)
															{
																m=l;
															}
															else
															{
																m=0;
															}
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+number_in_z*(number_in_y+1)*
															(number_in_x+1)+(number_in_z+1)*number_in_y*
															(number_in_x+1)+(i*(number_in_y+1)+j)*
															(number_in_z+1)+k+m*line_offset);
														slit_type=(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
															node_type)|(unsigned char)((texture_node_list[
															i+1+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
															node_type);
														if (slit_type&0x2)
														{
															m=l;
														}
														else
														{
															m=0;
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+number_in_z*(number_in_y+1)*
															(number_in_x+1)+(number_in_z+1)*number_in_y*
															(number_in_x+1)+(i*(number_in_y+1)+j)*
															(number_in_z+1)+k+1+m*line_offset);
														slit_type=(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															node_type)|(unsigned char)((texture_node_list[i+
															(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
															node_type);
														if (slit_type&0x2)
														{
															if (slit_type&0x1)
															{
																m=l*2+1;
															}
															else
															{
																m=l*2;
															}
														}
														else
														{
															if (slit_type&0x1)
															{
																m=1;
															}
															else
															{
																m=0;
															}
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+(i*(number_in_y+1)+j)*
															number_in_z+k+m*line_offset);
														slit_type=(unsigned char)((texture_node_list[
															i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															node_type)|(unsigned char)((texture_node_list[
															i+1+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
															node_type);
														if (slit_type&0x2)
														{
															m=l*2;
														}
														else
														{
															m=0;
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+((i+1)*(number_in_y+1)+j)*
															number_in_z+k+m*line_offset);
														if (2==element_dimension)
														{
															fprintf(out_file,"  Nodes:\n");
															fprintf(out_file,"    %d %d %d %d\n",
																starting_node_number+((texture_node_list[
																i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
																cm_node_identifier)[7],
																starting_node_number+((texture_node_list[
																i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
																cm_node_identifier)[3],
																starting_node_number+((texture_node_list[
																i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->
																cm_node_identifier)[6],
																starting_node_number+((texture_node_list[
																i+1+(number_in_x+1)*(j+(number_in_y+1)*
																(k+1))])->cm_node_identifier)[2]);
															fprintf(out_file,"  Scale factors:\n   ");
															/* node 1 */
															if (coordinate_field_vector)
															{
																index=i+(number_in_x+1)*(j+(number_in_y+1)*k);
																if (0==i)
																{
																	scale_factor_x=coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*index];
																}
																else
																{
																	scale_factor_x=(coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*(index-1)])/2;
																}
																if (0==k)
																{
																	scale_factor_z=coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*index+2];
																}
																else
																{
																	scale_factor_z=(coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*(index-
																		(number_in_x+1)*(number_in_y+1))+2])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (x_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_x;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (z_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[i]*scale_factor_z;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n   ");
															/* node 2 */
															if (coordinate_field_vector)
															{
																index=i+1+(number_in_x+1)*(j+(number_in_y+1)*k);
																if (0==i)
																{
																	scale_factor_x=coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*index];
																}
																else
																{
																	scale_factor_x=(coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*(index-1)])/2;
																}
																if (0==k)
																{
																	scale_factor_z=coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*index+2];
																}
																else
																{
																	scale_factor_z=(coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*(index-
																		(number_in_x+1)*(number_in_y+1))+2])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (x_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_x;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (z_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[i]*scale_factor_z;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n   ");
															/* node 3 */
															if (coordinate_field_vector)
															{
																index=
																	i+(number_in_x+1)*(j+(number_in_y+1)*(k+1));
																if (0==i)
																{
																	scale_factor_x=coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*index];
																}
																else
																{
																	scale_factor_x=(coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*(index-1)])/2;
																}
																if (0==k)
																{
																	scale_factor_z=coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*index+2];
																}
																else
																{
																	scale_factor_z=(coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*(index-
																		(number_in_x+1)*(number_in_y+1))+2])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (x_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_x;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (z_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[i]*scale_factor_z;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n   ");
															/* node 4 */
															if (coordinate_field_vector)
															{
																index=
																	i+1+(number_in_x+1)*(j+(number_in_y+1)*(k+1));
																if (0==i)
																{
																	scale_factor_x=coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*index];
																}
																else
																{
																	scale_factor_x=(coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*(index-1)])/2;
																}
																if (0==k)
																{
																	scale_factor_z=coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*index+2];
																}
																else
																{
																	scale_factor_z=(coordinate_field_vector[
																		3*(index+(number_in_x+1)*(number_in_y+1))+
																		2]-coordinate_field_vector[3*(index-
																		(number_in_x+1)*(number_in_y+1))+2])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (x_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_x;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (z_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[i]*scale_factor_z;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n");
														}
													}
												}
											}
											number++;
										}
									}
								}
								for (i=0;i<number_in_x;i++)
								{
									for (j=0;j<number_in_y;j++)
									{
										for (k=0;k<=number_in_z;k++)
										{
											for (l=0;l<2;l++)
											{
												in_mesh[l]=0;
											}
											slit_type=
												(unsigned char)((texture_node_list[
												i+(number_in_x+1)*(j+(number_in_y+1)*k)])->node_type)|
												(unsigned char)((texture_node_list[
												i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->node_type)|
												(unsigned char)((texture_node_list[
												i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->node_type)|
												(unsigned char)((texture_node_list[
												i+1+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
												node_type);
											if (k>0)
											{
												if ((texture_cell_list[i+number_in_x*(j+number_in_y*
													(k-1))])->scalar_value>iso_value)
												{
													in_mesh[0]=1;
												}
											}
											if (k<number_in_z)
											{
												if ((texture_cell_list[i+number_in_x*(j+
													number_in_y*k)])->scalar_value>iso_value)
												{
													if (slit_type&0x4)
													{
														in_mesh[1]=1;
													}
													else
													{
														in_mesh[0]=1;
													}
												}
											}
											if ((-1==x_output_elements)&&(-1==y_output_elements)&&
												((-1==z_output_elements)||(k==z_output_elements)))
											{
												for (l=0;l<2;l++)
												{
													if (in_mesh[l])
													{
														fprintf(out_file,"Element: 0 %d 0\n",
															number+l*face_offset);
														fprintf(out_file,"  Faces:\n");
														slit_type=(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															node_type)|(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
															node_type);
														if (slit_type&0x4)
														{
															if (slit_type&0x1)
															{
																m=l*2+1;
															}
															else
															{
																m=l*2;
															}
														}
														else
														{
															if (slit_type&0x1)
															{
																m=1;
															}
															else
															{
																m=0;
															}
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+number_in_z*(number_in_y+1)*
															(number_in_x+1)+(i*number_in_y+j)*(number_in_z+1)+
															k+m*line_offset);
														slit_type=(unsigned char)((texture_node_list[
															i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															node_type)|(unsigned char)((texture_node_list[
															i+1+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
															node_type);
														if (slit_type&0x4)
														{
															m=l*2;
														}
														else
														{
															m=0;
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+number_in_z*(number_in_y+1)*
															(number_in_x+1)+((i+1)*number_in_y+j)*
															(number_in_z+1)+k+m*line_offset);
														slit_type=(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															node_type)|(unsigned char)((texture_node_list[
															i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->
															node_type);
														if (slit_type&0x4)
														{
															if (slit_type&0x2)
															{
																m=l*2+1;
															}
															else
															{
																m=l*2;
															}
														}
														else
														{
															if (slit_type&0x2)
															{
																m=1;
															}
															else
															{
																m=0;
															}
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+number_in_z*(number_in_y+1)*
															(number_in_x+1)+(number_in_z+1)*number_in_y*
															(number_in_x+1)+(i*(number_in_y+1)+j)*
															(number_in_z+1)+k+m*line_offset);
														slit_type=(unsigned char)((texture_node_list[
															i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
															node_type)|(unsigned char)((texture_node_list[
															i+1+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
															node_type);
														if (slit_type&0x4)
														{
															m=l*2;
														}
														else
														{
															m=0;
														}
														fprintf(out_file,"    0 0 %d\n",
															starting_line_number+number_in_z*(number_in_y+1)*
															(number_in_x+1)+(number_in_z+1)*number_in_y*
															(number_in_x+1)+(i*(number_in_y+1)+j+1)*
															(number_in_z+1)+k+m*line_offset);
														if (2==element_dimension)
														{
															fprintf(out_file,"  Nodes:\n");
															fprintf(out_file,"    %d %d %d %d\n",
																starting_node_number+((texture_node_list[
																i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
																cm_node_identifier)[7],
																starting_node_number+((texture_node_list[
																i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->
																cm_node_identifier)[6],
																starting_node_number+((texture_node_list[
																i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
																cm_node_identifier)[5],
																starting_node_number+((texture_node_list[
																i+1+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
																cm_node_identifier)[4]);
															fprintf(out_file,"  Scale factors:\n   ");
															/* node 1 */
															if (coordinate_field_vector)
															{
																index=i+(number_in_x+1)*(j+(number_in_y+1)*k);
																if (0==i)
																{
																	scale_factor_x=coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*index];
																}
																else
																{
																	scale_factor_x=(coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*(index-1)])/2;
																}
																if (0==j)
																{
																	scale_factor_y=coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*index+1];
																}
																else
																{
																	scale_factor_y=(coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*(index-
																		number_in_x-1)+1])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (x_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_x;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (y_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_y;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n   ");
															/* node 2 */
															if (coordinate_field_vector)
															{
																index=i+1+(number_in_x+1)*(j+(number_in_y+1)*k);
																if (0==i)
																{
																	scale_factor_x=coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*index];
																}
																else
																{
																	scale_factor_x=(coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*(index-1)])/2;
																}
																if (0==j)
																{
																	scale_factor_y=coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*index+1];
																}
																else
																{
																	scale_factor_y=(coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*(index-
																		number_in_x-1)+1])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (x_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_x;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (y_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_y;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n   ");
															/* node 3 */
															if (coordinate_field_vector)
															{
																index=i+(number_in_x+1)*(j+1+(number_in_y+1)*k);
																if (0==i)
																{
																	scale_factor_x=coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*index];
																}
																else
																{
																	scale_factor_x=(coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*(index-1)])/2;
																}
																if (0==j)
																{
																	scale_factor_y=coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*index+1];
																}
																else
																{
																	scale_factor_y=(coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*(index-
																		number_in_x-1)+1])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (x_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_x;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (y_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_y;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n   ");
															/* node 4 */
															if (coordinate_field_vector)
															{
																index=
																	i+1+(number_in_x+1)*(j+1+(number_in_y+1)*k);
																if (0==i)
																{
																	scale_factor_x=coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*index];
																}
																else
																{
																	scale_factor_x=(coordinate_field_vector[
																		3*(index+1)]-coordinate_field_vector[
																		3*(index-1)])/2;
																}
																if (0==j)
																{
																	scale_factor_y=coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*index+1];
																}
																else
																{
																	scale_factor_y=(coordinate_field_vector[
																		3*(index+number_in_x+1)+1]-
																		coordinate_field_vector[3*(index-
																		number_in_x-1)+1])/2;
																}
																derivative_value=1;
																scale_factors[0]=1;
																switch (x_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_x;
																		}
																		derivative_value *= 2;
																	} break;
																}
																switch (y_interpolation)
																{
																	case CUBIC_HERMITE:
																	{
																		for (l=0;l<derivative_value;l++)
																		{
																			scale_factors[l+derivative_value]=
																				scale_factors[l]*scale_factor_y;
																		}
																		derivative_value *= 2;
																	} break;
																}
															}
															for (l=0;l<number_of_values;l++)
															{
																fprintf(out_file," %g",scale_factors[l]);
															}
															fprintf(out_file,"\n");
														}
													}
												}
											}
											number++;
										}
									}
								}
								if (2<element_dimension)
								{
									/* write the elements */
									fprintf(out_file,"Shape.  Dimension=3\n");
									fprintf(out_file,"#Scale factor sets=1\n");
									fprintf(out_file,"  %s, #Scale factors=%d\n",basis_name,
										8*number_of_values);
									fprintf(out_file,"#Nodes=8\n");
									fprintf(out_file,"#Fields=%d\n",number_of_fields);
									number_of_fields=0;
									if (write_coordinate_field)
									{
										number_of_fields++;
										fprintf(out_file,
					"%d) coordinates, coordinate, rectangular cartesian, #Components=3\n",
											number_of_fields);
										fprintf(out_file,
											"  x.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=8\n");
										for (i=1;i<=8;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
											"  y.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=8\n");
										for (i=1;i<=8;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
											"  z.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=8\n");
										for (i=1;i<=8;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
									}
									if (write_undeformed_field)
									{
										number_of_fields++;
										fprintf(out_file,
					"%d) undeformed, coordinate, rectangular cartesian, #Components=3\n",
											number_of_fields);
										fprintf(out_file,
											"  x.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=8\n");
										for (i=1;i<=8;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
											"  y.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=8\n");
										for (i=1;i<=8;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
											"  z.  %s, no modify, standard node based.\n",basis_name);
										fprintf(out_file,"    #Nodes=8\n");
										for (i=1;i<=8;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
									}
									if (write_fibre_field)
									{
										number_of_fields++;
										fprintf(out_file,
											"%d) fibres, anatomical, fibre, #Components=3\n",
											number_of_fields);
										fprintf(out_file,
											"  fibre angle.  %s, no modify, standard node based.\n",
											basis_name);
										fprintf(out_file,"    #Nodes=8\n");
										for (i=1;i<=8;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
									"  embrication angle.  %s, no modify, standard node based.\n",
											basis_name);
										fprintf(out_file,"    #Nodes=8\n");
										for (i=1;i<=8;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
										fprintf(out_file,
											"  sheet angle.  %s, no modify, standard node based.\n",
											basis_name);
										fprintf(out_file,"    #Nodes=8\n");
										for (i=1;i<=8;i++)
										{
											fprintf(out_file,"      %d.  #Values=%d\n",i,
												number_of_values);
											fprintf(out_file,"        Value indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",j);
											}
											fprintf(out_file,"\n");
											fprintf(out_file,"        Scale factor indices:");
											for (j=1;j<=number_of_values;j++)
											{
												fprintf(out_file," %d",(i-1)*number_of_values+j);
											}
											fprintf(out_file,"\n");
										}
									}
									number=starting_element_number;
									if (!ALLOCATE(table,int,number_in_x*number_in_y*number_in_z))
									{
										table = NULL;
										display_message(ERROR_MESSAGE,
										"write_finite_elements.  couldn't allocate element table");
									}
									for (i=0;i<number_in_x;i++)
									{
										for (j=0;j<number_in_y;j++)
										{
											for (k=0;k<number_in_z;k++)
											{
												if (table)
												{
													table[i*number_in_y*number_in_z+j*number_in_z+k]=0;
												}
												if ((texture_cell_list[i+number_in_x*(j+
													number_in_y*k)])->scalar_value>iso_value)
												{
													fprintf(out_file,"Element: %d 0 0\n",number);
													if (table)
													{
														table[i*number_in_y*number_in_z+j*number_in_z+k]=
															number;
													}
													fprintf(out_file,"  Faces:\n");
													slit_type=(unsigned char)((texture_node_list[
														i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
														node_type)|(unsigned char)((texture_node_list[
														i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
														node_type)|(unsigned char)((texture_node_list[
														i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
														node_type)|(unsigned char)((texture_node_list[
														i+(number_in_x+1)*(j+1+(number_in_y+1)*(k+1))])->
														node_type);
													if (slit_type&0x1)
													{
														l=1;
													}
													else
													{
														l=0;
													}
													fprintf(out_file,"    0 %d 0\n",starting_face_number+
														(i*number_in_y+j)*number_in_z+k+l*face_offset);
													fprintf(out_file,"    0 %d 0\n",starting_face_number+
														((i+1)*number_in_y+j)*number_in_z+k);
													slit_type=(unsigned char)((texture_node_list[
														i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
														node_type)|(unsigned char)((texture_node_list[
														i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->
														node_type)|(unsigned char)((texture_node_list[
														i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
														node_type)|(unsigned char)((texture_node_list[
														i+1+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
														node_type);
													if (slit_type&0x2)
													{
														l=1;
													}
													else
													{
														l=0;
													}
													fprintf(out_file,"    0 %d 0\n",starting_face_number+
														(number_in_x+1)*number_in_y*number_in_z+
														(i*(number_in_y+1)+j)*number_in_z+k+l*face_offset);
													fprintf(out_file,"    0 %d 0\n",starting_face_number+
														(number_in_x+1)*number_in_y*number_in_z+
														(i*(number_in_y+1)+j+1)*number_in_z+k);
													slit_type=(unsigned char)((texture_node_list[
														i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
														node_type)|(unsigned char)((texture_node_list[
														i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->
														node_type)|(unsigned char)((texture_node_list[
														i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
														node_type)|(unsigned char)((texture_node_list[
														i+1+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
														node_type);
													if (slit_type&0x4)
													{
														l=1;
													}
													else
													{
														l=0;
													}
													fprintf(out_file,"    0 %d 0\n",starting_face_number+
														(number_in_x+1)*number_in_y*number_in_z+
														number_in_x*(number_in_y+1)*number_in_z+
														(i*number_in_y+j)*(number_in_z+1)+k+l*face_offset);
													fprintf(out_file,"    0 %d 0\n",starting_face_number+
														(number_in_x+1)*number_in_y*number_in_z+
														number_in_x*(number_in_y+1)*number_in_z+
														(i*number_in_y+j)*(number_in_z+1)+k+1);
													fprintf(out_file,"  Nodes:\n");
													fprintf(out_file,"    %d %d %d %d %d %d %d %d\n",
														starting_node_number+((texture_node_list[
														i+(number_in_x+1)*(j+(number_in_y+1)*k)])->
														cm_node_identifier)[7],
														starting_node_number+((texture_node_list[
														i+1+(number_in_x+1)*(j+(number_in_y+1)*k)])->
														cm_node_identifier)[6],
														starting_node_number+((texture_node_list[
														i+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
														cm_node_identifier)[5],
														starting_node_number+((texture_node_list[
														i+1+(number_in_x+1)*(j+1+(number_in_y+1)*k)])->
														cm_node_identifier)[4],
														starting_node_number+((texture_node_list[
														i+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
														cm_node_identifier)[3],
														starting_node_number+((texture_node_list[
														i+1+(number_in_x+1)*(j+(number_in_y+1)*(k+1))])->
														cm_node_identifier)[2],
														starting_node_number+((texture_node_list[
														i+(number_in_x+1)*(j+1+(number_in_y+1)*(k+1))])->
														cm_node_identifier)[1],
														starting_node_number+((texture_node_list[
														i+1+(number_in_x+1)*(j+1+(number_in_y+1)*(k+1))])->
														cm_node_identifier)[0]);
													fprintf(out_file,"  Scale factors:\n   ");
													/* node 1 */
													if (coordinate_field_vector)
													{
														index=i+(number_in_x+1)*(j+(number_in_y+1)*k);
														if (0==i)
														{
															scale_factor_x=
																coordinate_field_vector[3*(index+1)]-
																coordinate_field_vector[3*index];
														}
														else
														{
															scale_factor_x=
																(coordinate_field_vector[3*(index+1)]-
																coordinate_field_vector[3*(index-1)])/2;
														}
														if (0==j)
														{
															scale_factor_y=coordinate_field_vector[
																3*(index+number_in_x+1)+1]-
																coordinate_field_vector[3*index+1];
														}
														else
														{
															scale_factor_y=(coordinate_field_vector[3*(index+
																number_in_x+1)+1]-coordinate_field_vector[
																3*(index-number_in_x-1)+1])/2;
														}
														if (0==k)
														{
															scale_factor_z=coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*index+2];
														}
														else
														{
															scale_factor_z=(coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2])/2;
														}
														derivative_value=1;
														scale_factors[0]=1;
														switch (x_interpolation)
														{
															case CUBIC_HERMITE:
															{
																derivative_value=2;
																scale_factors[1]=scale_factor_x;
															} break;
														}
														switch (y_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[l]*scale_factor_y;
																}
																derivative_value *= 2;
															} break;
														}
														switch (z_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[i]*scale_factor_z;
																}
																derivative_value *= 2;
															} break;
														}
													}
													for (l=0;l<number_of_values;l++)
													{
														fprintf(out_file," %g",scale_factors[l]);
													}
													fprintf(out_file,"\n   ");
													/* node 2 */
													if (coordinate_field_vector)
													{
														index=i+1+(number_in_x+1)*(j+(number_in_y+1)*k);
														if (number_in_x-1==i)
														{
															scale_factor_x=coordinate_field_vector[3*index]-
																coordinate_field_vector[3*(index-1)];
														}
														else
														{
															scale_factor_x=
																(coordinate_field_vector[3*(index+1)]-
																coordinate_field_vector[3*(index-1)])/2;
														}
														if (0==j)
														{
															scale_factor_y=coordinate_field_vector[
																3*(index+number_in_x+1)+1]-
																coordinate_field_vector[3*index+1];
														}
														else
														{
															scale_factor_y=(coordinate_field_vector[3*(index+
																number_in_x+1)+1]-coordinate_field_vector[
																3*(index-number_in_x-1)+1])/2;
														}
														if (0==k)
														{
															scale_factor_z=coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*index+2];
														}
														else
														{
															scale_factor_z=(coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2])/2;
														}
														derivative_value=1;
														scale_factors[0]=1;
														switch (x_interpolation)
														{
															case CUBIC_HERMITE:
															{
																derivative_value=2;
																scale_factors[1]=scale_factor_x;
															} break;
														}
														switch (y_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[l]*scale_factor_y;
																}
																derivative_value *= 2;
															} break;
														}
														switch (z_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[i]*scale_factor_z;
																}
																derivative_value *= 2;
															} break;
														}
													}
													for (l=0;l<number_of_values;l++)
													{
														fprintf(out_file," %g",scale_factors[l]);
													}
													fprintf(out_file,"\n   ");
													if (coordinate_field_vector)
													{
														index=i+(number_in_x+1)*((j+1)+(number_in_y+1)*k);
														if (0==i)
														{
															scale_factor_x=
																coordinate_field_vector[3*(index+1)]-
																coordinate_field_vector[3*index];
														}
														else
														{
															scale_factor_x=
																(coordinate_field_vector[3*(index+1)]-
																coordinate_field_vector[3*(index-1)])/2;
														}
														if (number_in_y-1==j)
														{
															scale_factor_y=coordinate_field_vector[3*index+1]-
																coordinate_field_vector[
																3*(index-number_in_x-1)+1];
														}
														else
														{
															scale_factor_y=(coordinate_field_vector[3*(index+
																number_in_x+1)+1]-coordinate_field_vector[
																3*(index-number_in_x-1)+1])/2;
														}
														if (0==k)
														{
															scale_factor_z=coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*index+2];
														}
														else
														{
															scale_factor_z=(coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2])/2;
														}
														derivative_value=1;
														scale_factors[0]=1;
														switch (x_interpolation)
														{
															case CUBIC_HERMITE:
															{
																derivative_value=2;
																scale_factors[1]=scale_factor_x;
															} break;
														}
														switch (y_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[l]*scale_factor_y;
																}
																derivative_value *= 2;
															} break;
														}
														switch (z_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[i]*scale_factor_z;
																}
																derivative_value *= 2;
															} break;
														}
													}
													for (l=0;l<number_of_values;l++)
													{
														fprintf(out_file," %g",scale_factors[l]);
													}
													fprintf(out_file,"\n   ");
													if (coordinate_field_vector)
													{
														index=i+1+(number_in_x+1)*((j+1)+(number_in_y+1)*k);
														if (number_in_x-1==i)
														{
															scale_factor_x=coordinate_field_vector[3*index]-
																coordinate_field_vector[3*(index-1)];
														}
														else
														{
															scale_factor_x=(coordinate_field_vector[
																3*(index+1)]-
																coordinate_field_vector[3*(index-1)])/2;
														}
														if (number_in_y-1==j)
														{
															scale_factor_y=coordinate_field_vector[3*index+1]-
																coordinate_field_vector[
																3*(index-number_in_x-1)+1];
														}
														else
														{
															scale_factor_y=(coordinate_field_vector[3*(index+
																number_in_x+1)+1]-coordinate_field_vector[
																3*(index-number_in_x-1)+1])/2;
														}
														if (0==k)
														{
															scale_factor_z=coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*index+2];
														}
														else
														{
															scale_factor_z=(coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2])/2;
														}
														derivative_value=1;
														scale_factors[0]=1;
														switch (x_interpolation)
														{
															case CUBIC_HERMITE:
															{
																derivative_value=2;
																scale_factors[1]=scale_factor_x;
															} break;
														}
														switch (y_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[l]*scale_factor_y;
																}
																derivative_value *= 2;
															} break;
														}
														switch (z_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[i]*scale_factor_z;
																}
																derivative_value *= 2;
															} break;
														}
													}
													for (l=0;l<number_of_values;l++)
													{
														fprintf(out_file," %g",scale_factors[l]);
													}
													fprintf(out_file,"\n   ");
													if (coordinate_field_vector)
													{
														index=i+(number_in_x+1)*(j+(number_in_y+1)*(k+1));
														if (0==i)
														{
															scale_factor_x=coordinate_field_vector[
																3*(index+1)]-coordinate_field_vector[3*index];
														}
														else
														{
															scale_factor_x=(coordinate_field_vector[
																3*(index+1)]-coordinate_field_vector[
																3*(index-1)])/2;
														}
														if (0==j)
														{
															scale_factor_y=coordinate_field_vector[
																3*(index+number_in_x+1)+1]-
																coordinate_field_vector[3*index+1];
														}
														else
														{
															scale_factor_y=(coordinate_field_vector[3*(index+
																number_in_x+1)+1]-coordinate_field_vector[
																3*(index-number_in_x-1)+1])/2;
														}
														if (number_in_z-1==k)
														{
															scale_factor_z=coordinate_field_vector[3*index+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2];
														}
														else
														{
															scale_factor_z=(coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2])/2;
														}
														derivative_value=1;
														scale_factors[0]=1;
														switch (x_interpolation)
														{
															case CUBIC_HERMITE:
															{
																derivative_value=2;
																scale_factors[1]=scale_factor_x;
															} break;
														}
														switch (y_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[l]*scale_factor_y;
																}
																derivative_value *= 2;
															} break;
														}
														switch (z_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[i]*scale_factor_z;
																}
																derivative_value *= 2;
															} break;
														}
													}
													for (l=0;l<number_of_values;l++)
													{
														fprintf(out_file," %g",scale_factors[l]);
													}
													fprintf(out_file,"\n   ");
													if (coordinate_field_vector)
													{
														index=i+1+(number_in_x+1)*(j+(number_in_y+1)*(k+1));
														if (number_in_x-1==i)
														{
															scale_factor_x=coordinate_field_vector[3*index]-
																coordinate_field_vector[3*(index-1)];
														}
														else
														{
															scale_factor_x=(coordinate_field_vector[
																3*(index+1)]-coordinate_field_vector[
																3*(index-1)])/2;
														}
														if (0==j)
														{
															scale_factor_y=coordinate_field_vector[
																3*(index+number_in_x+1)+1]-
																coordinate_field_vector[3*index+1];
														}
														else
														{
															scale_factor_y=(coordinate_field_vector[3*(index+
																number_in_x+1)+1]-coordinate_field_vector[
																3*(index-number_in_x-1)+1])/2;
														}
														if (number_in_z-1==k)
														{
															scale_factor_z=coordinate_field_vector[3*index+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2];
														}
														else
														{
															scale_factor_z=(coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2])/2;
														}
														derivative_value=1;
														scale_factors[0]=1;
														switch (x_interpolation)
														{
															case CUBIC_HERMITE:
															{
																derivative_value=2;
																scale_factors[1]=scale_factor_x;
															} break;
														}
														switch (y_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[l]*scale_factor_y;
																}
																derivative_value *= 2;
															} break;
														}
														switch (z_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[i]*scale_factor_z;
																}
																derivative_value *= 2;
															} break;
														}
													}
													for (l=0;l<number_of_values;l++)
													{
														fprintf(out_file," %g",scale_factors[l]);
													}
													fprintf(out_file,"\n   ");
													if (coordinate_field_vector)
													{
														index=i+(number_in_x+1)*(j+1+(number_in_y+1)*(k+1));
														if (0==i)
														{
															scale_factor_x=coordinate_field_vector[
																3*(index+1)]-coordinate_field_vector[3*index];
														}
														else
														{
															scale_factor_x=(coordinate_field_vector[
																3*(index+1)]-coordinate_field_vector[
																3*(index-1)])/2;
														}
														if (number_in_y-1==j)
														{
															scale_factor_y=coordinate_field_vector[3*index+1]-
																coordinate_field_vector[
																3*(index-number_in_x-1)+1];
														}
														else
														{
															scale_factor_y=(coordinate_field_vector[3*(index+
																number_in_x+1)+1]-coordinate_field_vector[
																3*(index-number_in_x-1)+1])/2;
														}
														if (number_in_z-1==k)
														{
															scale_factor_z=coordinate_field_vector[3*index+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2];
														}
														else
														{
															scale_factor_z=(coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2])/2;
														}
														derivative_value=1;
														scale_factors[0]=1;
														switch (x_interpolation)
														{
															case CUBIC_HERMITE:
															{
																derivative_value=2;
																scale_factors[1]=scale_factor_x;
															} break;
														}
														switch (y_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[l]*scale_factor_y;
																}
																derivative_value *= 2;
															} break;
														}
														switch (z_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[i]*scale_factor_z;
																}
																derivative_value *= 2;
															} break;
														}
													}
													for (l=0;l<number_of_values;l++)
													{
														fprintf(out_file," %g",scale_factors[l]);
													}
													fprintf(out_file,"\n   ");
													if (coordinate_field_vector)
													{
														index=
															i+1+(number_in_x+1)*(j+1+(number_in_y+1)*(k+1));
														if (number_in_x-1==i)
														{
															scale_factor_x=coordinate_field_vector[3*index]-
																coordinate_field_vector[3*(index-1)];
														}
														else
														{
															scale_factor_x=(coordinate_field_vector[
																3*(index+1)]-coordinate_field_vector[
																3*(index-1)])/2;
														}
														if (number_in_y-1==j)
														{
															scale_factor_y=coordinate_field_vector[3*index+1]-
																coordinate_field_vector[
																3*(index-number_in_x-1)+1];
														}
														else
														{
															scale_factor_y=(coordinate_field_vector[3*(index+
																number_in_x+1)+1]-coordinate_field_vector[
																3*(index-number_in_x-1)+1])/2;
														}
														if (number_in_z-1==k)
														{
															scale_factor_z=coordinate_field_vector[3*index+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2];
														}
														else
														{
															scale_factor_z=(coordinate_field_vector[3*(index+
																(number_in_x+1)*(number_in_y+1))+2]-
																coordinate_field_vector[3*(index-
																(number_in_x+1)*(number_in_y+1))+2])/2;
														}
														derivative_value=1;
														scale_factors[0]=1;
														switch (x_interpolation)
														{
															case CUBIC_HERMITE:
															{
																derivative_value=2;
																scale_factors[1]=scale_factor_x;
															} break;
														}
														switch (y_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[l]*scale_factor_y;
																}
																derivative_value *= 2;
															} break;
														}
														switch (z_interpolation)
														{
															case CUBIC_HERMITE:
															{
																for (l=0;l<derivative_value;l++)
																{
																	scale_factors[l+derivative_value]=
																		scale_factors[i]*scale_factor_z;
																}
																derivative_value *= 2;
															} break;
														}
													}
													for (l=0;l<number_of_values;l++)
													{
														fprintf(out_file," %g",scale_factors[l]);
													}
													fprintf(out_file,"\n");
												}
												number++;
											}
										}
									}
								}
							}
							fclose(out_file);
							/* print out element layout file */
							if (table)
							{
								if (out_file = fopen(file_name2, "w"))
								{
									fprintf(out_file, "%d %d %d\n", number_in_x, number_in_y,
										number_in_z);
									for (i=0;i<number_in_z;i++)
									{
										for (j=0;j<number_in_y;j++)
										{
											for (k=0;k<number_in_x;k++)
											{
												fprintf(out_file, "%d ",
													table[i*number_in_x*number_in_y+j*number_in_x +k]);
											}
											fprintf(out_file, "\n");
										}
										fprintf(out_file, "\n");
									}
									fclose(out_file);
									DEALLOCATE(table);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Could not open element file: %s",file_name2);
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Could not open element file: %s",file_name);
						}
						DEALLOCATE(basis_name);
					}
					else
					{
						if (basis_name)
						{
							DEALLOCATE(basis_name);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_finite_elements.  Could not allocate basis name");
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Could not open node file: %s",file_name);
				}
				DEALLOCATE(file_name);
				DEALLOCATE(file_name2);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_finite_elements.  Insufficient memory for file name");
			}
			/* write out node groups (if any) */
			if (0<volume_texture->n_groups)
			{
				display_message(WARNING_MESSAGE,"Adding node offset of %d to groups\n",
					starting_node_number);
			}
			for (i=0;i<volume_texture->n_groups;i++)
			{
				if (ALLOCATE(file_name,char,strlen(value_string)+
					strlen((volume_texture->node_groups[i])->name)+8+1))
				{
					/* write the nodes */
					strcpy(file_name,value_string);
					strcat(file_name,".");
					strcat(file_name,(volume_texture->node_groups[i])->name);
					strcat(file_name,".exnode");
					if (out_file=fopen(file_name,"w"))
					{
						/* write heading */
						fprintf(out_file,"Group name : %s\n",
							(volume_texture->node_groups[i])->name);
						fprintf(out_file,"#Fields=0\n");
						for (j=0;j<(volume_texture->node_groups[i])->n_nodes;j++)
						{
							number=(volume_texture->node_groups[i])->nodes[j]+
								starting_node_number;
							fprintf(out_file,"Node: %d\n",number);
						}
						fclose(out_file);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Could not open group file: %s",file_name);
					}
					DEALLOCATE(file_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_finite_elements.  Insufficient memory for file name");
				}
			}

		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_finite_elements.  Missing volume_texture");
		}
		/* close the dialog */
		close_create_finite_elements_di(widget_id,create_finite_elements_dialog,
			call_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_finite_elements.  Missing create_finite_elements_dialog");
	}
	LEAVE;
} /* write_finite_elements */

/*
Global functions
----------------
*/
int open_create_finite_elements_dialog(
	struct Create_finite_elements_dialog **address,
	struct VT_volume_texture *volume_texture,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	MrmType dummy_class;
	static MrmRegisterArg callback_list[]=
	{
		{"identify_coordinate_field_toggl",
			(XtPointer)identify_coordinate_field_toggl},
		{"identify_undeformed_field_toggl",
			(XtPointer)identify_undeformed_field_toggl},
		{"identify_fibre_field_toggle",(XtPointer)identify_fibre_field_toggle},
		{"identify_x_interpolation_option",
			(XtPointer)identify_x_interpolation_option},
		{"identify_x_int_linear_lagrange",
			(XtPointer)identify_x_int_linear_lagrange},
		{"identify_x_int_cubic_hermite",(XtPointer)identify_x_int_cubic_hermite},
		{"identify_y_interpolation_option",
			(XtPointer)identify_y_interpolation_option},
		{"identify_y_int_linear_lagrange",
			(XtPointer)identify_y_int_linear_lagrange},
		{"identify_y_int_cubic_hermite",(XtPointer)identify_y_int_cubic_hermite},
		{"identify_z_interpolation_option",
			(XtPointer)identify_z_interpolation_option},
		{"identify_z_int_linear_lagrange",
			(XtPointer)identify_z_int_linear_lagrange},
		{"identify_z_int_cubic_hermite",(XtPointer)identify_z_int_cubic_hermite},
		{"identify_starting_node_number",(XtPointer)identify_starting_node_number},
		{"identify_starting_line_number",(XtPointer)identify_starting_line_number},
		{"identify_starting_face_number",(XtPointer)identify_starting_face_number},
		{"identify_starting_element_numbe",
			(XtPointer)identify_starting_element_numbe},
		{"identify_file_group_name_text",(XtPointer)identify_file_group_name_text},
		{"identify_x_output_elements_text",
			(XtPointer)identify_x_output_elements_text},
		{"identify_y_output_elements_text",
			(XtPointer)identify_y_output_elements_text},
		{"identify_z_output_elements_text",
			(XtPointer)identify_z_output_elements_text},
		{"identify_ok_button",(XtPointer)identify_ok_button},
		{"write_finite_elements",(XtPointer)write_finite_elements},
		{"identify_cancel_button",(XtPointer)identify_cancel_button},
		{"close_create_finite_elements_di",
			(XtPointer)close_create_finite_elements_di}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"create_finite_elements_dialog_s",(XtPointer)NULL}
	};
	struct Create_finite_elements_dialog *create_finite_elements_dialog;

	ENTER(open_create_finite_elements_dialog);
	/* check argument */
	if (user_interface)
	{
		if (volume_texture)
		{
			/* if the dialog has not been created */
			if (!address||!(create_finite_elements_dialog= *address))
			{
				/* create the dialog */
				if (MrmOpenHierarchy_base64_string(volume_texture_editor_dialog_uid64,
					&volume_texture_editor_dialog_hierarchy,
					&volume_texture_editor_dialog_hierarchy_open))
				{
					if (ALLOCATE(create_finite_elements_dialog,
						struct Create_finite_elements_dialog,1))
					{
						create_finite_elements_dialog->address=address;
						create_finite_elements_dialog->user_interface=user_interface;
						create_finite_elements_dialog->dialog=(Widget)NULL;
						create_finite_elements_dialog->coordinate_field_toggle=(Widget)NULL;
						create_finite_elements_dialog->undeformed_field_toggle=(Widget)NULL;
						create_finite_elements_dialog->fibre_field_toggle=(Widget)NULL;
						create_finite_elements_dialog->
							x_interpolation_option_menu=(Widget)NULL;
						(create_finite_elements_dialog->
							x_interpolation_option).linear_lagrange=(Widget)NULL;
						(create_finite_elements_dialog->
							x_interpolation_option).cubic_hermite=(Widget)NULL;
						create_finite_elements_dialog->
							y_interpolation_option_menu=(Widget)NULL;
						(create_finite_elements_dialog->
							y_interpolation_option).linear_lagrange=(Widget)NULL;
						(create_finite_elements_dialog->
							y_interpolation_option).cubic_hermite=(Widget)NULL;
						create_finite_elements_dialog->
							z_interpolation_option_menu=(Widget)NULL;
						(create_finite_elements_dialog->
							z_interpolation_option).linear_lagrange=(Widget)NULL;
						(create_finite_elements_dialog->
							z_interpolation_option).cubic_hermite=(Widget)NULL;
						create_finite_elements_dialog->
							starting_node_number_text_field=(Widget)NULL;
						create_finite_elements_dialog->
							starting_line_number_text_field=(Widget)NULL;
						create_finite_elements_dialog->
							starting_face_number_text_field=(Widget)NULL;
						create_finite_elements_dialog->
							starting_element_number_text_field=(Widget)NULL;
						create_finite_elements_dialog->file_group_name_text_field=
							(Widget)NULL;
						create_finite_elements_dialog->x_output_elements_text_field=
							(Widget)NULL;
						create_finite_elements_dialog->y_output_elements_text_field=
							(Widget)NULL;
						create_finite_elements_dialog->z_output_elements_text_field=
							(Widget)NULL;
						create_finite_elements_dialog->ok_button=(Widget)NULL;
						create_finite_elements_dialog->cancel_button=(Widget)NULL;
						/* create shell */
						if (create_finite_elements_dialog->shell=XtVaCreatePopupShell(
							"create_finite_elements_dialog_shell",xmDialogShellWidgetClass,
							user_interface->application_shell,
							XmNmwmDecorations,MWM_DECOR_ALL|MWM_DECOR_RESIZEH,
							XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
							XmNtitle,"Save as finite elements",
							NULL))
						{
							/* add the destroy callback */
							XtAddCallback(create_finite_elements_dialog->shell,
								XmNdestroyCallback,destroy_create_finite_elements_dialog,
								(XtPointer)create_finite_elements_dialog);
							/* register the other callbacks */
							if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
								volume_texture_editor_dialog_hierarchy,callback_list,
								XtNumber(callback_list)))
							{
								/* register the identifiers */
								identifier_list[0].value=create_finite_elements_dialog;
								if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
									volume_texture_editor_dialog_hierarchy,identifier_list,
									XtNumber(identifier_list)))
								{
									/* fetch the widget */
									if (MrmSUCCESS==MrmFetchWidget(
										volume_texture_editor_dialog_hierarchy,
										"create_finite_elements_dialog",
										create_finite_elements_dialog->shell,
										&(create_finite_elements_dialog->dialog),&dummy_class))
									{
										/* initialize */
										XtVaSetValues(create_finite_elements_dialog->
											x_interpolation_option_menu,XmNmenuHistory,
											(create_finite_elements_dialog->x_interpolation_option).
											linear_lagrange,NULL);
										XtVaSetValues(create_finite_elements_dialog->
											y_interpolation_option_menu,XmNmenuHistory,
											(create_finite_elements_dialog->y_interpolation_option).
											linear_lagrange,NULL);
										XtVaSetValues(create_finite_elements_dialog->
											z_interpolation_option_menu,XmNmenuHistory,
											(create_finite_elements_dialog->z_interpolation_option).
											linear_lagrange,NULL);
										XtVaSetValues(create_finite_elements_dialog->
											starting_node_number_text_field,XmNvalue,"1",NULL);
										XtVaSetValues(create_finite_elements_dialog->
											starting_line_number_text_field,XmNvalue,"1",NULL);
										XtVaSetValues(create_finite_elements_dialog->
											starting_face_number_text_field,XmNvalue,"1",NULL);
										XtVaSetValues(create_finite_elements_dialog->
											starting_element_number_text_field,XmNvalue,"1",NULL);
										XtVaSetValues(create_finite_elements_dialog->
											file_group_name_text_field,XmNvalue,"volume_texture",
											NULL);
										XtVaSetValues(create_finite_elements_dialog->
											x_output_elements_text_field,XmNvalue,"*",NULL);
										XtVaSetValues(create_finite_elements_dialog->
											y_output_elements_text_field,XmNvalue,"*",NULL);
										XtVaSetValues(create_finite_elements_dialog->
											z_output_elements_text_field,XmNvalue,"*",NULL);
										XtManageChild(create_finite_elements_dialog->dialog);
										XtRealizeWidget(create_finite_elements_dialog->shell);
										if (address)
										{
											*address=create_finite_elements_dialog;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
						"open_create_finite_elements_dialog.  Could not fetch the widget");
										DEALLOCATE(create_finite_elements_dialog);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
				"open_create_finite_elements_dialog.  Could not register identifiers");
									DEALLOCATE(create_finite_elements_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
					"open_create_finite_elements_dialog.  Could not register callbacks");
								DEALLOCATE(create_finite_elements_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"open_create_finite_elements_dialog.  Could not create shell");
							DEALLOCATE(create_finite_elements_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
				"open_create_finite_elements_dialog.  Insufficient memory for dialog");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_create_finite_elements_dialog.  Could not open hierarchy");
				}
			}
			if (create_finite_elements_dialog)
			{
				create_finite_elements_dialog->volume_texture=volume_texture;
				/* open the dialog */
				XtManageChild(create_finite_elements_dialog->dialog);
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"No volume texture to create finite elements from");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"open_create_finite_elements_dialog.  Missing user_interface");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_create_finite_elements_dialog */
