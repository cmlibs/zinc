/*******************************************************************************
FILE : sync_2d_3d.h

LAST MODIFIED : 29 January 1999

DESCRIPTION :
Allows 2d digitisation of data from an arbitrary client.
==============================================================================*/
#if !defined (SYNC_2D_3D_H)
#define SYNC_2D_3D_H

#include <Xm/Xm.h>
#include "command/command.h"
#include "finite_element/finite_element.h"

/*
Identifiers
-----------
*/
#define Sync_2d_3d_base       100
#define IDC_image_base_name        (Sync_2d_3d_base + 0)
#define IDC_axis_name              (Sync_2d_3d_base + 1)
#define IDC_depth                  (Sync_2d_3d_base + 2)
#define IDC_thickness              (Sync_2d_3d_base + 3)
#define IDC_node_group_form        (Sync_2d_3d_base + 4)
#define IDC_sync_button            (Sync_2d_3d_base + 10)

#if !defined (RC_INVOKED)
#include <stddef.h>
#include "user_interface/gui_prototype.h"

/* necessary if we want to create with initial values */
typedef struct
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
	Initialisation data.
==============================================================================*/
{
	char *image_base_name,axis_name;
	double depth,thickness;
	struct GROUP(FE_node) *source_node_group;
	struct Execute_command *execute_command;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
} DIALOG_INITIALISATION_STRUCT(Sync_2d_3d);

struct DIALOG_DATA_STRUCT(Sync_2d_3d)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
==============================================================================*/
{
	char *image_base_name,axis_name;
	double depth,thickness;
	struct GROUP(FE_node) *source_node_group,*destination_node_group;
	struct GROUP(FE_node) *temp_source_node_group,*temp_destination_node_group;
	struct Execute_command *execute_command;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
}; /* DIALOG_DATA_STRUCT(Sync_2d_3d) */

struct DIALOG_STRUCT(Sync_2d_3d)
/*******************************************************************************
LAST MODIFIED : 5 November 1994

DESCRIPTION :
==============================================================================*/
{
	struct DIALOG_DATA_STRUCT(Sync_2d_3d) data;
#if defined (WINDOWS)
	HWND dialog,parent;
	HWND image_base_name;
#endif
	/* window is the widget that we read into the dialog */
#if defined (MOTIF)
	Widget dialog,parent,window;
	Widget image_base_name;
	Widget axis_name;
	Widget depth;
	Widget thickness;
	Widget node_group_form;
	Widget node_group;
	Widget sync_button;
#endif /* MOTIF */
}; /* DIALOG_STRUCT(Sync_2d_3d) */

/*
Global Functions
---------------
*/
PROTOTYPE_GLOBAL_GUI_DIALOG_FUNCTIONS(Sync_2d_3d);

int bring_up_sync_2d_3d_dialog(Widget *sync_2d_3d_dialog_address,
	Widget parent,struct Execute_command *execute_command,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager);
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
If there is a sync_2d_3d dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/

int DIALOG_FUNCTION(Sync_2d_3d,set_depth)(DIALOG_GLOBAL_PARAM(Sync_2d_3d),
	double depth);

int DIALOG_FUNCTION(Sync_2d_3d,set_thickness)(DIALOG_GLOBAL_PARAM(Sync_2d_3d),
	double thickness);

int DIALOG_FUNCTION(Sync_2d_3d,set_axis_name)(DIALOG_GLOBAL_PARAM(Sync_2d_3d),
	char axis_name);

int DIALOG_FUNCTION(Sync_2d_3d,update_windows)(DIALOG_GLOBAL_PARAM(Sync_2d_3d));

int DIALOG_FUNCTION(Sync_2d_3d,merge_nodes)(DIALOG_GLOBAL_PARAM(Sync_2d_3d));
#endif /* !defined (RC_INVOKED) */
#endif /* !defined (SYNC_2D_3D_H) */
