/*******************************************************************************
FILE : acquisition_work_area.c

LAST MODIFIED : 24 November 1999

DESCRIPTION :
UNIMA_ACQUISITION refers to the acquisition window used with the UNIMA/EMAP
	hardware
==============================================================================*/
#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "unemap/acquisition_work_area.h"
#include "user_interface/message.h"

/*
Module functions
----------------
*/

/*
Global functions
----------------
*/
#if defined (MOTIF)
void close_acquisition_work_area(Widget widget,
	XtPointer acquisition_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Closes the windows associated with the acquisition work area.
==============================================================================*/
{
	struct Acquisition_work_area *acquisition;

	ENTER(close_acquisition_work_area);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (acquisition=(struct Acquisition_work_area *)acquisition_work_area)
	{
		if (acquisition->window_shell)
		{
			/* close the acquisition shell */
			XtPopdown(acquisition->window_shell);
			/* unghost the acquisition activation button */
			XtSetSensitive(acquisition->activation,True);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"close_acquisition_work_area.  acquisition window shell is missing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"close_acquisition_work_area.  Missing client_data");
	}
	LEAVE;
} /* close_acquisition_work_area */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
void set_mapping_acquisition_region(Widget widget,
	XtPointer acquisition_work_area,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Called when a new rig region is selected from the mapping window in the
acquisition work area.
==============================================================================*/
{
	char found;
	struct Acquisition_work_area *acquisition;
	struct Mapping_window *mapping;
	struct Rig *rig;
	struct Region_list_item *region_item;
	Widget *region,selected_region;

	ENTER(set_mapping_acquisition_region);
	USE_PARAMETER(widget);
	if (acquisition=(struct Acquisition_work_area *)acquisition_work_area)
	{
		if ((mapping=acquisition->mapping_window)&&(rig=acquisition->rig))
		{
			/* determine the new region selected */
			selected_region=((XmRowColumnCallbackStruct *)call_data)->widget;
			if (selected_region!=mapping->current_region)
			{
				found=0;
				region=mapping->regions;
				/* check for "all regions" */
				if (mapping->number_of_regions>1)
				{
					if (selected_region== *region)
					{
						/* update the acquisition rig */
						rig->current_region=(struct Region *)NULL;
						found=1;
					}
					else
					{
						region++;
					}
				}
				if (!found)
				{
					region_item=rig->region_list;
					while (region_item&&(selected_region!= *region))
					{
						region_item=region_item->next;
						region++;
					}
					if (region_item)
					{
						found=1;
						/* update the acquisition rig */
						rig->current_region=region_item->region;
					}
				}
				if (found)
				{
					mapping->current_region=selected_region;
					/* update the mapping window */
					update_mapping_drawing_area(mapping,2);
					update_mapping_colour_or_auxili(mapping);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_mapping_acquisition_region.  Invalid region");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_mapping_acquisition_region.  Missing mapping window or rig");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_mapping_acquisition_region.  Missing acquisition_work_area");
	}
	LEAVE;
} /* set_mapping_acquisition_region */
#endif /* defined (MOTIF) */

int create_acquisition_work_area(struct Acquisition_work_area *acquisition,
#if defined (MOTIF)
	Widget activation,Widget parent,
#endif /* defined (MOTIF) */
	int pointer_sensitivity,
#if defined (MOTIF)
	Pixel identifying_colour,
#endif /* defined (MOTIF) */
	char *signal_file_extension_write,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Creates the windows associated with the acquisition work area.
???Assign storage for work area ?
???system_window_structure ?
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
#if defined (UNIMA_ACQUISITION)
	static MrmRegisterArg callback_list[]={
		{"close_acquisition_work_area",(XtPointer)close_acquisition_work_area}};
	static MrmRegisterArg identifier_list[]=
	{
		{"acquisition_work_area_structure",(XtPointer)NULL}
	};
#endif /* defined (UNIMA_ACQUISITION) */
#endif /* defined (MOTIF) */

	ENTER(create_acquisition_work_area);
	return_code=1;
	if (acquisition&&user_interface)
	{
#if defined (MOTIF)
		acquisition->activation=activation;
#endif /* defined (MOTIF) */
		acquisition->user_interface=user_interface;
#if defined (MOTIF)
		/* if there is not an acquisition window shell */
		if (!(acquisition->window_shell))
		{
			/* create the acquisition window shell */
			if (!
#if defined (UNIMA_ACQUISITION)
				create_acquisition_window_shell
#else /* defined (UNIMA_ACQUISITION) */
				create_page_window_shell
#endif /* defined (UNIMA_ACQUISITION) */
				(&(acquisition->window_shell),parent,
				user_interface->screen_width,user_interface->screen_height,
				user_interface))
			{
				display_message(ERROR_MESSAGE,
		"create_acquisition_work_area.  Could not create acquisition window shell");
				return_code=0;
			}
		}
#endif /* defined (MOTIF) */
		if (return_code)
		{
#if defined (MOTIF)
#if defined (UNIMA_ACQUISITION)
			/* register the callbacks (don't have any uid files so use the global
				hierarchy */
			if (MrmSUCCESS==MrmRegisterNames(callback_list,XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)acquisition;
				if (MrmSUCCESS==MrmRegisterNames(identifier_list,
					XtNumber(identifier_list)))
				{
#endif /* defined (UNIMA_ACQUISITION) */
#endif /* defined (MOTIF) */
					/* if there is not an acquisition window */
					if (!(acquisition->window))
					{
						/* create the acquisition window */
						if (
#if defined (UNIMA_ACQUISITION)
							create_Acquisition_window
#else /* defined (UNIMA_ACQUISITION) */
							create_Page_window
#endif /* defined (UNIMA_ACQUISITION) */
							(&(acquisition->window),
#if defined (MOTIF)
							activation,acquisition->window_shell,
#endif /* defined (MOTIF) */
							&(acquisition->rig),
#if defined (MOTIF)
							identifying_colour,
#endif /* defined (MOTIF) */
							&(acquisition->mapping_window),pointer_sensitivity,
							signal_file_extension_write,user_interface))
						{
#if defined (MOTIF)
#if !defined (UNIMA_ACQUISITION)
							XtAddCallback(get_page_window_close_button(acquisition->window),
								XmNactivateCallback,close_acquisition_work_area,
								(XtPointer)acquisition);
#endif /* !defined (UNIMA_ACQUISITION) */
#if defined (OLD_CODE)
							/* manage the acquisition window */
							XtManageChild(acquisition->window->window);
#endif /* defined (OLD_CODE) */
#if defined (OLD_CODE)
							/* realize the acquisition shell */
							XtRealizeWidget(acquisition->window_shell);
#endif /* defined (OLD_CODE) */
#endif /* defined (MOTIF) */
						}
						else
						{
							display_message(ERROR_MESSAGE,
					"create_acquisition_work_area.  Could not create acquisition window");
#if defined (MOTIF)
							XtDestroyWidget(acquisition->window_shell);
							acquisition->window_shell=(Widget)NULL;
#endif /* defined (MOTIF) */
							return_code=0;
						}
					}
#if defined (MOTIF)
#if defined (UNIMA_ACQUISITION)
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_acquisition_work_area.  Could not register identifiers");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_acquisition_work_area.  Could not register callbacks");
			}
#endif /* defined (UNIMA_ACQUISITION) */
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_acquisition_work_area.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_acquisition_work_area */
