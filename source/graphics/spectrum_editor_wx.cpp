/*******************************************************************************
FILE : spectrum_editor_wx.cpp

LAST MODIFIED : 22 Aug 2007

DESCRIPTION:
Provides the wxWidgets interface to manipulate spectrum settings.
==============================================================================*/

extern "C" {
#include <stdio.h>
#include <math.h>
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_settings.h"
#include "graphics/spectrum_editor_wx.h"
#include "graphics/spectrum_editor_dialog_wx.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/* SAB Trying to hide the guts of GT_object and its primitives,
	however the spectrum editor is modifying it's primitives quite a bit . */
#include "graphics/graphics_object_private.h"
}
#include "wx/wx.h"
#include <wx/splitter.h>
#include "wx/xrc/xmlres.h"
#include "choose/choose_manager_class.hpp"
#include "graphics/spectrum_editor_wx.xrch"
#include "icon/cmiss_icon.xpm"

class wxSpectrumEditor;


static int spectrum_editor_wx_update_scene_viewer(
	 struct Spectrum_editor *spectrum_editor);

struct Spectrum_editor
/*******************************************************************************
LAST MODIFIED : 22 August 2007

DESCRIPTION :
Contains all the information carried by the graphical element editor widget.
==============================================================================*/
{
	/* This editor_material is used when displaying the spectrum in the 
		3d widget */
	 struct Graphical_material *editor_material, *tick_material;
	 struct Spectrum_settings *current_settings;
	 struct Spectrum *edit_spectrum, *current_spectrum;
	 struct MANAGER(Scene) *scene_manager;
	 struct MANAGER(Spectrum) *spectrum_manager;
	 struct User_interface *user_interface;
	 struct Spectrum_editor_dialog **spectrum_editor_dialog_address;
	 void *material_manager_callback_id;
	 void *spectrum_manager_callback_id;
// 	 struct Callback_data update_callback;
	 struct Scene *spectrum_editor_scene, *autorange_scene;
	 struct Scene_viewer *spectrum_editor_scene_viewer;
	 struct GT_object *graphics_object, *tick_lines_graphics_object,
			*tick_labels_graphics_object;
	 int viewer_type;
	 wxSpectrumEditor *wx_spectrum_editor;
	 wxPanel *spectrum_panel;
	 wxListBox *spectrum_editor_check_list;
	 wxFrame *spectrum_editor_frame;
	 wxTextCtrl *spectrum_range_min_text, *spectrum_range_max_text, 
			*spectrum_exaggeration_text, *spectrum_data_component_text,
			*spectrum_number_of_bands_text, *spectrum_ratio_of_black_bands_text,
			*spectrum_editor_step_value_text, *spectrum_normalised_colour_range_min_text,
			*spectrum_normalised_colour_range_max_text;
	 wxCheckListBox *spectrum_settings_checklist;
	 wxButton *spectrum_settings_add_button, *spectrum_settings_del_button, 
			*spectrum_settings_up_button, *spectrum_settings_dn_button;
	 wxCheckBox *spectrum_reverse_checkbox, *spectrum_extend_below_check,
			*spectrum_extend_above_check, *spectrum_fix_minimum_check,
			*spectrum_fix_maximum_check;
	 wxChoice *spectrum_colour_mapping_choice, *spectrum_type_choice;
	 wxRadioBox *spectrum_left_right_radio_box;
	 wxScrolledWindow *spectrum_higher_panel, *spectrum_lower_panel;
}; /* spectrum_editor */

static int make_current_spectrum(
	struct Spectrum_editor *spectrum_editor,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Destroys the edit_spectrum member of <spectrum_editor> and rebuilds it as
a complete copy of <Spectrum>.
==============================================================================*/
{
	int return_code;

	ENTER(make_currentspectrum);
	if (spectrum_editor)
	{
		return_code=1;
		if (spectrum)
		{
			if (!IS_MANAGED(Spectrum)(spectrum,
				spectrum_editor->spectrum_manager))
			{
				display_message(ERROR_MESSAGE,
					"spectrum_editor_dialog_set_spectrum.  Spectrum not managed");
				spectrum=(struct Spectrum *)NULL;
				return_code=0;
			}
		}
		if (!spectrum)
		{
			spectrum=FIRST_OBJECT_IN_MANAGER_THAT(Spectrum)(
				(MANAGER_CONDITIONAL_FUNCTION(Spectrum) *)NULL,
				(void *)NULL,
				spectrum_editor->spectrum_manager);
		}
			spectrum_editor->current_spectrum=spectrum;
			spectrum_editor_wx_set_spectrum(
				 spectrum_editor,
				spectrum_editor->current_spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_set_spectrum.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* make_edit_spectrum */

static int make_edit_spectrum(
	struct Spectrum_editor *spectrum_editor,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Destroys the edit_spectrum member of <spectrum_editor> and rebuilds it as
a complete copy of <Spectrum>.
==============================================================================*/
{
	int return_code;

	ENTER(make_edit_spectrum);
	/* check arguments */
	if (spectrum_editor&&spectrum)
	{
		/* destroy current edit_spectrum */
		if (spectrum_editor->edit_spectrum)
		{
			DEACCESS(Spectrum)(&(spectrum_editor->edit_spectrum));
		}
		/* make an empty spectrum */
		if (spectrum_editor->edit_spectrum=ACCESS(Spectrum)(CREATE(Spectrum)("copy")))
		{
			/* copy general settings into new object */
			MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)
				(spectrum_editor->edit_spectrum,spectrum);

			set_GT_object_Spectrum(spectrum_editor->graphics_object,
				(void *)spectrum_editor->edit_spectrum);

			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"make_edit_spectrum.  Could not make copy of spectrum");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_edit_spectrum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* make_edit_spectrum */

static int create_settings_item_wx(struct Spectrum_settings *settings,
	void *spectrum_editor_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Clears then fills the settings list RowCol with descriptions of the settings
of the type spectrum_editor->settings_type.
==============================================================================*/
{
	 int num_children,return_code;
	 struct Spectrum_editor *spectrum_editor;
	 char *settings_string;

	 ENTER(create_settings_item_wx);
	 if (settings&&(spectrum_editor=
				 (struct Spectrum_editor *)spectrum_editor_void))
	 {
			if (settings_string=Spectrum_settings_string(settings,
						SPECTRUM_SETTINGS_STRING_COMPLETE_PLUS))
			{
				 if (spectrum_editor->spectrum_settings_checklist)
				 {
						spectrum_editor->spectrum_settings_checklist->Append(settings_string);
						num_children = spectrum_editor->spectrum_settings_checklist->GetCount();
						if (Spectrum_settings_get_active(settings))
						{
							 spectrum_editor->spectrum_settings_checklist->Check(num_children-1, true);
						}
						else
						{
							 spectrum_editor->spectrum_settings_checklist->Check(num_children-1, false);
						}
						return_code = 1;
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "create_settings_item_wx. Invalid checklist");
						return_code = 0;
				 }	 
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"create_settings_item_wx.  Could not get settings string");
				 return_code=0;
			}
			DEALLOCATE(settings_string);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "create_settings_item_wx.  Invalid argument(s)");
			return_code=0;
	 }
	 LEAVE;

	 return (return_code);
}

static int spectrum_editor_wx_make_settings_list(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Clears then fills the settings list RowColumn with descriptions of the settings
of the type spectrum_editor->settings_type.
==============================================================================*/
{
	 int return_code, selection, num_of_items;

	 ENTER(spectrum_editor_wx_make_settings_list);
	 if (spectrum_editor)
	 {
			if (spectrum_editor->spectrum_settings_checklist)
			{
				 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
				 spectrum_editor->spectrum_settings_checklist->Clear();
				 FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
						create_settings_item_wx,(void *)spectrum_editor,
						get_Spectrum_settings_list( spectrum_editor->edit_spectrum));
				 num_of_items = spectrum_editor->spectrum_settings_checklist->GetCount();
				 if (num_of_items>=selection+1)
				 {
						spectrum_editor->spectrum_settings_checklist->SetSelection(selection);
				 }
				 else
				 {
						spectrum_editor->spectrum_settings_checklist->SetSelection(selection-1);
				 }
				 return_code=1;
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"spectrum_editor_wx_make_settings_list. Invalid checklist");
				 return_code = 0;
			}
	 }
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_wx_make_settings_list.  Invalid argument(s)");
		return_code=0;
	}
	 LEAVE;

	return (return_code);
} /* spectrum_editor_wx_make_settings_list */

static int spectrum_editor_wx_set_type(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Sets the current settings type on the choose menu according to the
current settings in spectrum_editor_settings.
==============================================================================*/
{
	enum Spectrum_settings_type current_type;
	int return_code;

	ENTER(spectrum_editor_settings_set_type);
	if (spectrum_editor &&
		 spectrum_editor->current_settings &&
		 spectrum_editor->spectrum_type_choice)
	{
		current_type = Spectrum_settings_get_type(
			spectrum_editor->current_settings);
		switch (current_type)
		{
			 case SPECTRUM_LINEAR:
			 {
					spectrum_editor->spectrum_type_choice->SetStringSelection("Linear");
			 } break;
			 case SPECTRUM_LOG:
			 {
					spectrum_editor->spectrum_type_choice->SetStringSelection("Log");
			 } break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_wx_set_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_settings_set_type */

int spectrum_editor_wx_set_settings(struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Changes the currently chosen settings.
==============================================================================*/
{
 	 char temp_string[50];
 	 enum Spectrum_settings_type type;
	 enum Spectrum_settings_colour_mapping colour_mapping;
 	 float exaggeration, step_value, band_ratio;
 	 int component, number_of_bands, black_band_proportion, extend,fix; //,i, num_children, number_of_bands, black_band_proportion,
	 int return_code;
	 struct Spectrum_settings *new_settings;

	 ENTER(spectrum_editor_wx_set_settings);	 
	 if (spectrum_editor)
	 {
			if (new_settings = spectrum_editor->current_settings)
			{				 
				 spectrum_editor_wx_set_type(spectrum_editor);
				 type = Spectrum_settings_get_type(new_settings);
				 if (spectrum_editor->spectrum_reverse_checkbox)
				 {
						spectrum_editor->spectrum_reverse_checkbox->SetValue(
							 Spectrum_settings_get_reverse_flag(new_settings));
				 }
				 if (spectrum_editor->spectrum_colour_mapping_choice)
				 {
						colour_mapping = Spectrum_settings_get_colour_mapping(
							 new_settings);
						switch (colour_mapping)
						{
							 case SPECTRUM_ALPHA:
							 {
									spectrum_editor->spectrum_colour_mapping_choice->SetStringSelection("Alpha");
							 } break;
							 case SPECTRUM_BLUE:
							 {
									spectrum_editor->spectrum_colour_mapping_choice->SetStringSelection("Blue");
							 } break;
							 case SPECTRUM_GREEN:
							 {
									spectrum_editor->spectrum_colour_mapping_choice->SetStringSelection("Green");
							 } break;
							 case SPECTRUM_MONOCHROME:
							 {
									spectrum_editor->spectrum_colour_mapping_choice->SetStringSelection("Monochrome");
							 } break;
							 case SPECTRUM_RAINBOW:
							 {
									spectrum_editor->spectrum_colour_mapping_choice->SetStringSelection("Rainbow");
							 } break;
							 case SPECTRUM_RED:
							 {
									spectrum_editor->spectrum_colour_mapping_choice->SetStringSelection("Red");
							 } break;
							 case SPECTRUM_WHITE_TO_BLUE:
							 {
									spectrum_editor->spectrum_colour_mapping_choice->SetStringSelection("White to blue");
							 } break;
							 case SPECTRUM_STEP:
							 {
									spectrum_editor->spectrum_colour_mapping_choice->SetStringSelection("Step");
							 } break;
							 case SPECTRUM_BANDED:
							 {
									spectrum_editor->spectrum_colour_mapping_choice->SetStringSelection("Contour bands");
							 } break;
							 case SPECTRUM_WHITE_TO_RED:
							 {
									spectrum_editor->spectrum_colour_mapping_choice->SetStringSelection("White to red");
							 } break;
						}
				 }

				 if (spectrum_editor->spectrum_exaggeration_text)
				 {
						exaggeration = Spectrum_settings_get_exaggeration(new_settings);
						sprintf(temp_string,"%10g",fabs(exaggeration));
						spectrum_editor->spectrum_exaggeration_text->SetValue(temp_string);
						if (exaggeration >= 0.0)
						{
							 spectrum_editor->spectrum_left_right_radio_box->SetSelection(0);
						}
						else
						{
							 spectrum_editor->spectrum_left_right_radio_box->SetSelection(1);
						}
				 }
				 if (SPECTRUM_LOG == type)
				 {
						spectrum_editor->spectrum_exaggeration_text->Enable(true);
						spectrum_editor->spectrum_left_right_radio_box->Enable(true);
				 }
				 else
				 {
						spectrum_editor->spectrum_exaggeration_text->Enable(false);
						spectrum_editor->spectrum_left_right_radio_box->Enable(false);
				 }
				if (spectrum_editor->spectrum_data_component_text)
				{
					component = Spectrum_settings_get_component_number(new_settings);
					sprintf(temp_string,"%d",component);
					spectrum_editor->spectrum_data_component_text->SetValue(temp_string);
				}
				if (spectrum_editor->spectrum_number_of_bands_text)
				{
					 number_of_bands = Spectrum_settings_get_number_of_bands(new_settings);
					 sprintf(temp_string,"%10d",number_of_bands);
					 spectrum_editor->spectrum_number_of_bands_text->SetValue(temp_string);
				}
				if (spectrum_editor->spectrum_ratio_of_black_bands_text)
				{
					black_band_proportion = Spectrum_settings_get_black_band_proportion(new_settings);
					band_ratio = (float)(black_band_proportion) / 1000.0;
					sprintf(temp_string,"%10g",band_ratio);
					spectrum_editor->spectrum_ratio_of_black_bands_text->SetValue(temp_string);
				}
				if (SPECTRUM_BANDED == colour_mapping)
				{
					 spectrum_editor->spectrum_number_of_bands_text->Enable(true);
					 spectrum_editor->spectrum_ratio_of_black_bands_text->Enable(true);
				}
				else
				{
					 spectrum_editor->spectrum_number_of_bands_text->Enable(false);
					 spectrum_editor->spectrum_ratio_of_black_bands_text->Enable(false);
				}
				if (spectrum_editor->spectrum_editor_step_value_text)
				{
					step_value = Spectrum_settings_get_step_value(new_settings);
					sprintf(temp_string,"%10g",step_value);
					spectrum_editor->spectrum_editor_step_value_text->SetValue(temp_string);
				}
				if (SPECTRUM_STEP == colour_mapping)
				{
					 spectrum_editor->spectrum_editor_step_value_text->Enable(true);
				}
				else
				{
					 spectrum_editor->spectrum_editor_step_value_text->Enable(false);
				}
				if (spectrum_editor->spectrum_range_min_text)
				{
					sprintf(temp_string,"%10g",
						 Spectrum_settings_get_range_minimum(new_settings));
					spectrum_editor->spectrum_range_min_text->SetValue(temp_string);
				}
				if (spectrum_editor->spectrum_range_max_text)
				{
					sprintf(temp_string,"%10g",
						 Spectrum_settings_get_range_maximum(new_settings));
					spectrum_editor->spectrum_range_max_text->SetValue(temp_string);
				}
				if (spectrum_editor->spectrum_extend_below_check)
				{
					 extend = Spectrum_settings_get_extend_below_flag(new_settings);
					 spectrum_editor->spectrum_extend_below_check->SetValue(extend);
				}
				if (spectrum_editor->spectrum_extend_above_check)
				{
					 extend = Spectrum_settings_get_extend_above_flag(new_settings);
					 spectrum_editor->spectrum_extend_above_check->SetValue(extend);
				}
				if (spectrum_editor->spectrum_fix_maximum_check)
				{
					 fix = Spectrum_settings_get_fix_maximum_flag(new_settings);
					 spectrum_editor->spectrum_fix_maximum_check->SetValue(fix);
					 spectrum_editor->spectrum_range_max_text->Enable(!fix);
				}
				if (spectrum_editor->spectrum_fix_minimum_check)
				{
					 fix = Spectrum_settings_get_fix_minimum_flag(new_settings);
					 spectrum_editor->spectrum_fix_minimum_check->SetValue(fix);
					 spectrum_editor->spectrum_range_min_text->Enable(!fix);
				}
				if (spectrum_editor->spectrum_normalised_colour_range_min_text)
				{					 sprintf(temp_string,"%10g",
							Spectrum_settings_get_colour_value_minimum(new_settings));
					 spectrum_editor->spectrum_normalised_colour_range_min_text->SetValue(temp_string);
					 if (SPECTRUM_BANDED == colour_mapping
							|| SPECTRUM_STEP == colour_mapping)
					 {
							spectrum_editor->spectrum_normalised_colour_range_min_text->Enable(false);
					 }
					 else
					 {
							spectrum_editor->spectrum_normalised_colour_range_min_text->Enable(true);
					 }
				}
				if (spectrum_editor->spectrum_normalised_colour_range_max_text)
				{
					 sprintf(temp_string,"%10g",
							Spectrum_settings_get_colour_value_maximum(new_settings));
					 spectrum_editor->spectrum_normalised_colour_range_max_text->SetValue(temp_string);
					 if (SPECTRUM_BANDED == colour_mapping || SPECTRUM_STEP == colour_mapping)
					 {
							spectrum_editor->spectrum_normalised_colour_range_max_text->Enable(false);
					 }
					 else
					 {
							spectrum_editor->spectrum_normalised_colour_range_max_text->Enable(true);
					 }
				}			
				spectrum_editor->spectrum_lower_panel->Layout();
				spectrum_editor->spectrum_higher_panel->Layout();
				return_code = 1;
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"spectrum_editor_wx_set_settings.  Invalid argument(s)");
				 return_code = 0;
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "spectrum_editor_wx_set_settings.  Invalid argument(s)");
			return_code = 0;
	 }
	 LEAVE;

	 return (return_code);
}

static int spectrum_editor_wx_select_settings_item(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 30 August 

DESCRIPTION :
Checks if current_settings is in the settings_rowcol; if not (or it was NULL)
the first item in this list becomes the current_settings, or NULL if empty.
The line/settings editors for the previous settings are then Unmanaged, and that
for the new settings Managed and filled with the new values.
If current_settings is NULL, no editing fields are displayed.
==============================================================================*/
{
	int return_code,i,num_children;
	struct Spectrum_settings *temp_settings;
	struct LIST(Spectrum_settings) *list_of_settings;
	ENTER(spectrum_editor_select_settings_item);
	/* check arguments */
	if (spectrum_editor)
	{
		 /* get list of settings items */
		 list_of_settings = get_Spectrum_settings_list(spectrum_editor->edit_spectrum);
		 num_children = spectrum_editor->spectrum_settings_checklist->GetCount();
		 if (0<num_children)
		 {
				for (i=1;i<(num_children+1);i++)
				{
					 temp_settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,i);
					 if (!spectrum_editor->current_settings)
					 {
							spectrum_editor->current_settings=temp_settings;
					 }
					 if (temp_settings==spectrum_editor->current_settings)
					 {
							spectrum_editor->spectrum_settings_checklist->SetSelection(i-1, true);
					 }
				}
		 }
		 else
		 {
				spectrum_editor->current_settings=(struct Spectrum_settings *)NULL;
		 }
		/* Grey Delete and Priority buttons if no current_settings */
		 spectrum_editor->spectrum_settings_del_button->Enable(
				(struct Spectrum_settings *)NULL != spectrum_editor->current_settings);
		 spectrum_editor->spectrum_settings_up_button->Enable(
				(struct Spectrum_settings *)NULL != spectrum_editor->current_settings);
		 spectrum_editor->spectrum_settings_dn_button->Enable(
				(struct Spectrum_settings *)NULL != spectrum_editor->current_settings);
		 /* send selected object to settings editor */
		 return_code=spectrum_editor_wx_set_settings(
				spectrum_editor);
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"spectrum_editor_select_settings_item.  Invalid argument(s)");
		 return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* spectrum_editor_select_settings_item */

int spectrum_editor_settings_wx_key_presssed(struct Spectrum_editor *spectrum_editor, char *flag)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Called when a modify button - add, delete, up, down - is activated.
==============================================================================*/
{
	 int list_changed, position, return_code;
	 struct Spectrum_settings *settings;

	 ENTER(spectrum_editor_settings_wx_key_presssed);
	 if (spectrum_editor->edit_spectrum && flag)
	 {
			if (strcmp(flag, "Add") == 0)
			{
				 if (settings=CREATE(Spectrum_settings)())
				 {
						return_code=1;
						if (spectrum_editor->current_settings)
						{
							 /* copy current settings into new settings */
							 return_code=COPY(Spectrum_settings)(settings,
									spectrum_editor->current_settings);
							 Spectrum_settings_set_active(settings,1);
						}
						if (return_code&&Spectrum_add_settings(
									 spectrum_editor->edit_spectrum,settings,0))
						{
							 list_changed=1;
							 spectrum_editor->current_settings=settings;
						}
						else
						{
							 DESTROY(Spectrum_settings)(&settings);
						}
				 }
			}
			else if (strcmp(flag, "Del") == 0)
			{
				 list_changed=Spectrum_remove_settings(
						spectrum_editor->edit_spectrum,spectrum_editor->current_settings);
				 spectrum_editor->current_settings=(struct Spectrum_settings *)NULL;
			}
			else if (strcmp(flag, "Up") == 0)
			{
				 /* increase position of current_settings by 1 */
				 if (1<(position=Spectrum_get_settings_position(
									 spectrum_editor->edit_spectrum,spectrum_editor->current_settings)))
				 {
						list_changed=1;
						ACCESS(Spectrum_settings)(spectrum_editor->current_settings);
						Spectrum_remove_settings(spectrum_editor->edit_spectrum,
							 spectrum_editor->current_settings);
						Spectrum_add_settings(spectrum_editor->edit_spectrum,
							 spectrum_editor->current_settings,position-1);
						DEACCESS(Spectrum_settings)(&(spectrum_editor->current_settings));
				 }
			}
			else if (strcmp(flag, "Dn") == 0)
			{
				 /* decrease position of current_settings by 1 */
				 if (position=Spectrum_get_settings_position(
								spectrum_editor->edit_spectrum,spectrum_editor->current_settings))
				 {
						list_changed=1;
						ACCESS(Spectrum_settings)(spectrum_editor->current_settings);
						Spectrum_remove_settings(spectrum_editor->edit_spectrum,
							 spectrum_editor->current_settings);
						Spectrum_add_settings(spectrum_editor->edit_spectrum,
							 spectrum_editor->current_settings,position+1);
						DEACCESS(Spectrum_settings)(&(spectrum_editor->current_settings));
				 }
			}
			if (list_changed)
			{
				 spectrum_editor_wx_make_settings_list(spectrum_editor);
				 /* inform the client of the changes */
				 spectrum_editor_wx_select_settings_item(spectrum_editor);
				 spectrum_editor_wx_update_scene_viewer(spectrum_editor);
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "spectrum_editor_settings_wx_key_presssed.  Invalid argument(s)");
	 }
	 LEAVE;

	 return (return_code);
}

int spectrum_editor_wx_refresh(struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Clears all the settings_changed flags globally (later) and in the list of
settings.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_editor_wx_refresh);
	if (spectrum_editor)
	{
		if (spectrum_editor->edit_spectrum)
		{
			FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
				Spectrum_settings_clear_settings_changed,(void *)NULL,
				get_Spectrum_settings_list( spectrum_editor->edit_spectrum ));
			spectrum_editor_wx_make_settings_list(spectrum_editor);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_wx_refresh.  Missing edit_spectrum");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_wx_refresh.  Missing spectrum_editor");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_wx_refresh */

void spectrum_editor_wx_key_presssed(struct Spectrum_editor *spectrum_editor, char *flag)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Called when a button - OK, Apply, Revert or Cancel - is activated.
==============================================================================*/
{
	 ENTER(spectrum_editor_wx_key_presssed);
	 if ((strcmp(flag, "Cancel") == 0) ||
			(strcmp(flag, "Revert") == 0))
	 {
			spectrum_editor_wx_set_spectrum(spectrum_editor,
				 spectrum_editor->current_spectrum);
	 }
	 if ((strcmp(flag, "Apply") == 0) ||
			(strcmp(flag, "OK") == 0))
	 {
			if (spectrum_editor->current_spectrum&&
				 spectrum_editor->edit_spectrum)
			{
				 if (MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(
							spectrum_editor->current_spectrum, spectrum_editor->edit_spectrum,
								spectrum_editor->spectrum_manager))
				 {
						spectrum_editor_wx_refresh(spectrum_editor);
				 }
			}	
	 }

	 if ((strcmp(flag, "OK") == 0) ||
			(strcmp(flag, "Cancel") == 0))
	 {
			DESTROY(Spectrum_editor_dialog)(spectrum_editor->spectrum_editor_dialog_address);
	 }
	 LEAVE;
}

void spectrum_editor_wx_update_settings(struct Spectrum_editor *spectrum_editor, 
	 struct Spectrum_settings *new_settings)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for when changes made in the settings editor.
==============================================================================*/
{
	int active;

	ENTER(spectrum_editor_wx_update_settings);
	if (spectrum_editor && new_settings)
	{
		/* keep visibility of current_settings */
		 active = Spectrum_settings_get_active(spectrum_editor->current_settings);
		 Spectrum_settings_modify(spectrum_editor->current_settings,new_settings,
				get_Spectrum_settings_list( spectrum_editor->edit_spectrum ));
		 Spectrum_settings_set_active(spectrum_editor->current_settings,
				active);
		 spectrum_editor_wx_make_settings_list(spectrum_editor);
		 Spectrum_calculate_range(spectrum_editor->edit_spectrum);
		 spectrum_editor_wx_update_scene_viewer(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_settings.  Invalid argument(s)");
	}
	LEAVE;
}

class wxSpectrumEditor : public wxFrame
{
	 Spectrum_editor *spectrum_editor;
	 wxPanel *spectrum_scene_chooser_panel;
	 DEFINE_MANAGER_CLASS(Scene);
	 Managed_object_chooser<Scene,MANAGER_CLASS(Scene)>
	 *spectrum_scene_chooser;
public:

	 wxSpectrumEditor(Spectrum_editor *spectrum_editor):
			spectrum_editor(spectrum_editor)
	 {
			wxXmlInit_spectrum_editor_wx();
			wxXmlResource::Get()->LoadFrame(this,
				 (wxWindow *)NULL, _T("CmguiSpectrumEditor"));
			this->SetIcon(cmiss_icon_xpm);
			wxPanel *spectrum_scene_chooser_panel = 
				 XRCCTRL(*this, "wxSpectrumSceneChooserPanel", wxPanel);
			spectrum_scene_chooser = 
				 new Managed_object_chooser<Scene,MANAGER_CLASS(Scene)>
				 (spectrum_scene_chooser_panel, (struct Scene *)NULL, spectrum_editor->scene_manager,
						(MANAGER_CONDITIONAL_FUNCTION(Scene) *)NULL, (void *)NULL, spectrum_editor->user_interface);
			Callback_base< Scene* > *spectrum_scene_chooser_callback = 
				 new Callback_member_callback< Scene*, 
				 wxSpectrumEditor, int (wxSpectrumEditor::*)(Scene *) >
				 (this, &wxSpectrumEditor::spectrum_scene_chooser_callback);
			spectrum_scene_chooser->set_callback(spectrum_scene_chooser_callback);
	 };

	 wxSpectrumEditor()
	 {
	 };

	 ~wxSpectrumEditor()
	 {
			delete spectrum_scene_chooser;
	 };

int spectrum_scene_chooser_callback(Scene *scene)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Callback from wxChooser<Scene> when choice is made.
==============================================================================*/
{
	 spectrum_editor->autorange_scene = scene;
	 return 1;
}

int set_autorange_scene()
{
	 spectrum_editor->autorange_scene = spectrum_scene_chooser->get_object();
	 return 1;
}

void OnSpectrumListSelected(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 29 August 2007

DESCRIPTION :
Callback for Spectrum List.
==============================================================================*/
{
	 struct Spectrum *temp_spectrum;
	 int selection;
	 selection = spectrum_editor->spectrum_editor_check_list->GetSelection();
	 if (temp_spectrum = (struct Spectrum *)spectrum_editor->spectrum_editor_check_list->GetClientData(selection))
	 {
			spectrum_editor->current_spectrum = temp_spectrum;
			spectrum_editor_wx_set_spectrum(spectrum_editor,
				 temp_spectrum);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumListSelect.  Cannot get client data");
	 } 
}

void OnSpectrumSettingsSelected(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	 struct Spectrum_settings *temp_settings;
	 int selection;
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 temp_settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1);
	 if (temp_settings)
	 {
			if (temp_settings != spectrum_editor->current_settings)
			{
				 spectrum_editor->current_settings = temp_settings;
			}
			spectrum_editor_wx_select_settings_item(spectrum_editor);
	 }
}

void OnSpectrumSettingListAddPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	 spectrum_editor_settings_wx_key_presssed(spectrum_editor, "Add");
}

void OnSpectrumSettingListDelPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	 spectrum_editor_settings_wx_key_presssed(spectrum_editor, "Del");
}

void OnSpectrumSettingListUpPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	 spectrum_editor_settings_wx_key_presssed(spectrum_editor, "Up");
}

void OnSpectrumSettingListDnPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	 spectrum_editor_settings_wx_key_presssed(spectrum_editor, "Dn");
}

void OnSpectrumSettingRangeValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for the range text widgets.
==============================================================================*/
{
	 char *text;
	 float new_parameter;
	 int selection;
	 struct Spectrum_settings *settings;
	 
	 ENTER(OnSpectrumSettingRangeValueEntered);
	 /* check arguments */
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (settings =  get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1))
	 {
			if (spectrum_editor->spectrum_range_min_text)
			{
				 text = (char*)spectrum_editor->spectrum_range_min_text->GetValue().mb_str(wxConvUTF8);
				 if (text)
				 {
						sscanf(text,"%f",&new_parameter);
						if(new_parameter !=
							 Spectrum_settings_get_range_minimum(settings))
						{
							 Spectrum_settings_set_range_minimum(settings, new_parameter);
						}
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "OnSpectrumSettingRangeValueEntered.  Missing range minimum text");
				 }
			}
			if (spectrum_editor->spectrum_range_max_text)
			{
				 text = (char*)spectrum_editor->spectrum_range_max_text->GetValue().mb_str(wxConvUTF8);
				 if (text)
				 {
						sscanf(text,"%f",&new_parameter);
						if(new_parameter !=
							 Spectrum_settings_get_range_maximum(settings))
						{
							 Spectrum_settings_set_range_maximum(settings,new_parameter);
						}
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "OnSpectrumSettingRangeValueEntered.  Missing range maximum text");
				 }
			}
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumSettingRangeValueEntered.  Invalid argument(s)");
	 }
	 LEAVE;
} /* OnSpectrumSettingRangeEntered */

void OnSpectrumSettingRangeChecked(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for the range text widgets.
==============================================================================*/
{
	 int selection;
	 struct Spectrum_settings *settings;

	 ENTER(OnSpectrumSettingRangeChecked);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor && 
			(settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1)))
	 {
			if (spectrum_editor->spectrum_extend_above_check)
			{
				 Spectrum_settings_set_extend_above_flag(settings,
						spectrum_editor->spectrum_extend_above_check->IsChecked());
			}
			if (spectrum_editor->spectrum_extend_below_check)
			{
				 Spectrum_settings_set_extend_below_flag(settings,
						spectrum_editor->spectrum_extend_below_check->IsChecked());
			}	
			if (spectrum_editor->spectrum_fix_maximum_check)
			{
				 Spectrum_settings_set_fix_maximum_flag(settings, 
						spectrum_editor->spectrum_fix_maximum_check->IsChecked());
			}
			if (spectrum_editor->spectrum_fix_minimum_check)
			{
				 Spectrum_settings_set_fix_minimum_flag(settings,
						spectrum_editor->spectrum_fix_minimum_check->IsChecked());
			}
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumSettingRangeValueChecked.  Invalid argument(s)");
	 }
	 LEAVE;
}

void OnSpectrumSettingListOKPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	 spectrum_editor_wx_key_presssed(spectrum_editor, "OK");
}

void OnSpectrumSettingListApplyPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	 spectrum_editor_wx_key_presssed(spectrum_editor, "Apply");
}

void OnSpectrumSettingListRevertPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	 spectrum_editor_wx_key_presssed(spectrum_editor, "Revert");
}

void OnSpectrumSettingListCancelPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	 spectrum_editor_wx_key_presssed(spectrum_editor, "Cancel");
}

void OnSpectrumColourMappingChoice(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Callback for colour settings
==============================================================================*/
{
	enum Spectrum_settings_colour_mapping new_colour_mapping;
	struct Spectrum_settings *settings;
	char *string_selection;
	int selection;

	ENTER(OnSpectrumColourMappingChoice);

	selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	if (spectrum_editor && 
		 (settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1)))
	{
		/* get the widget from the call data */
		 if (string_selection = 
				(char*)(spectrum_editor->spectrum_colour_mapping_choice->GetStringSelection()).mb_str(wxConvUTF8))
		{
			 if (strcmp(string_selection, "Alpha") == 0)
			 {
					new_colour_mapping = SPECTRUM_ALPHA;
			 }
			 else if (strcmp(string_selection, "Blue") == 0)
			 {
					new_colour_mapping = SPECTRUM_BLUE;
			 }
			 else if (strcmp(string_selection, "Green") == 0)
			 {
					new_colour_mapping = SPECTRUM_GREEN;
			 }
			 else if (strcmp(string_selection, "Monochrome") == 0)
			 {
					new_colour_mapping = SPECTRUM_MONOCHROME;
			 }
			 else if (strcmp(string_selection, "Rainbow") == 0)
			 {
					new_colour_mapping = SPECTRUM_RAINBOW;
			 }
			 else if (strcmp(string_selection, "Red") == 0)
			 {
					new_colour_mapping = SPECTRUM_RED;
			 }
			 else if (strcmp(string_selection, "White to blue") == 0)
			 {
					new_colour_mapping = SPECTRUM_WHITE_TO_BLUE;
			 }
			 else if (strcmp(string_selection, "Step") == 0)
			 {
					new_colour_mapping = SPECTRUM_STEP;
			 }
			 else if (strcmp(string_selection, "Contour Bands") == 0)
			 {
					new_colour_mapping = SPECTRUM_BANDED;
			 }
			 else if (strcmp(string_selection, "White to red") == 0)
			 {
					new_colour_mapping = SPECTRUM_WHITE_TO_RED;
			 }
			if (Spectrum_settings_get_colour_mapping(settings) != new_colour_mapping)
			{
				 if (Spectrum_settings_set_colour_mapping(settings, new_colour_mapping))
				 {
						spectrum_editor_wx_update_settings(spectrum_editor, settings);
				 }
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"spectrum_editor_settings_colour_menu_CB.  "
				"Could not find the activated menu item");
		}
		/* make sure the correct render render is shown in case of error */
		 spectrum_editor_wx_set_settings(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"OnSpectrumColourMappingChoice.  Invalid argument(s)");
	}
	LEAVE;
}

void OnSpectrumNormalisedColourRangeValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 September

DESCRIPTION :
Callback for the colour_value text widgets.
==============================================================================*/
{
	 char *text;
	 float new_parameter;
	 struct Spectrum_settings *settings;
	 int selection;

	 ENTER(OnSpectrumColourMappingChoice);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor && 
			(settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1)))
	 {
			text = (char*)spectrum_editor->spectrum_normalised_colour_range_min_text->GetValue().mb_str(wxConvUTF8);
			if (text)
			{
				 sscanf(text,"%f",&new_parameter);
				 if(new_parameter !=
						Spectrum_settings_get_colour_value_minimum(settings))
				 {
						Spectrum_settings_set_colour_value_minimum(settings,new_parameter);
				 }
				 text = NULL;
			}
			text = (char*)spectrum_editor->spectrum_normalised_colour_range_max_text->GetValue().mb_str(wxConvUTF8);
			if (text)
			{
				 sscanf(text,"%f",&new_parameter);
				 if(new_parameter !=
						Spectrum_settings_get_colour_value_maximum(settings))
				 {
						Spectrum_settings_set_colour_value_maximum(settings,new_parameter);
				 }
			}
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumNormalisedColourRangeValueEntered.  Invalid argument(s)");
	 }
	 LEAVE;
}

void OnSpectrumSettingsReverseChecked(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 Sepetember 2007

DESCRIPTION :
Callback for the settings reverse toggle button.
==============================================================================*/
{
	 int selection;
	 struct Spectrum_settings *settings;
	 
	 ENTER(spectrum_editor_settings_reverse_toggle_CB);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor && 
			(settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1)))
	 {
			Spectrum_settings_set_reverse_flag(settings, 
				 spectrum_editor->spectrum_reverse_checkbox->GetValue());
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
			/* make sure the correct reverse flag is shown in case of error */
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumSettingsReverseChecked.  Invalid argument(s)");
	 }
	 LEAVE;
}

void OnSpectrumTypeChoice(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 Sepetember 2007

DESCRIPTION :
Callback for the settings type.
==============================================================================*/
{
	 enum Spectrum_settings_type new_spectrum_type;
	 struct Spectrum_settings *settings;
	 int selection;
	 char *string_selection;

	 ENTER(OnSpectrumTypeChoice);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor && 
			(settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1)))
	 {
			/* get the widget from the call data */
			if (string_selection = 
				 (char*)(spectrum_editor->spectrum_type_choice->GetStringSelection()).mb_str(wxConvUTF8))
			{
				 if (strcmp(string_selection, "Linear") == 0)
				 {
						new_spectrum_type = SPECTRUM_LINEAR;
				 }
				 else if (strcmp(string_selection, "Log") == 0)
				 {
						new_spectrum_type = SPECTRUM_LOG;
				 }
				 if (Spectrum_settings_get_type(settings) != new_spectrum_type)
				 {
						if (Spectrum_settings_set_type(settings, new_spectrum_type))
						{
							 spectrum_editor_wx_update_settings(spectrum_editor, settings);
						}
				 }
			}
			else
			{
				 display_message(ERROR_MESSAGE,"OnSpectrumTypeChoice.  "
						"Could not find the activated menu item");
			}
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }	
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumTypeChoice.  Invalid argument(s)");
	 }
	 LEAVE;
}

void OnSpectrumExaggerationSettingsChanged(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Callback for the exaggeration widgets.
==============================================================================*/
{
	 char *text;
	 float new_parameter;
	 struct Spectrum_settings *settings;
	 int selection;
	 
	 ENTER(OnSpectrumExaggerationSettingsChanged);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor && 
			(settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1)))
	 {	 
			text = (char*)spectrum_editor->spectrum_exaggeration_text->GetValue().mb_str(wxConvUTF8);
			if (text)
			{
				 sscanf(text,"%f",&new_parameter);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"OnSpectrumExaggerationSettingsChanged.  Missing widget text");
			}
			if (spectrum_editor->spectrum_left_right_radio_box->GetSelection() == 1)
			{
				 new_parameter *= -1.0;
			}	
			if(new_parameter !=
				 Spectrum_settings_get_exaggeration(settings))
			{
				 Spectrum_settings_set_exaggeration(settings,new_parameter);
				 spectrum_editor_wx_update_settings(spectrum_editor, settings);
			}
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumExaggerationSettingsChanged.  Invalid argument(s)");
	 }
	 LEAVE;
}

void OnSpectrumDataValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Callback for the component widgets.
==============================================================================*/
{
	 char *text;
	 int new_component, selection;
	 struct Spectrum_settings *settings;
	 
	 ENTER(OnSpectrumDataValueEntered);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor && 
			(settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1)))
	 {	 
			new_component = 1;
			text = (char*)spectrum_editor->spectrum_data_component_text->GetValue().mb_str(wxConvUTF8);
			if (text)
			{
				 sscanf(text,"%d",&new_component);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"OnSpectrumDataValueEntered.  Missing widget text");
			}
			if(new_component !=
				 Spectrum_settings_get_component_number(settings))
			{
				 Spectrum_settings_set_component_number(settings,new_component);
				 spectrum_editor_wx_update_settings(spectrum_editor, settings);
			}
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "spectrum_editor_settings_component_CB.  Invalid argument(s)");
	 } 
	 LEAVE;
}

void OnSpectrumBandsValuesEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Callback for the band  widgets.
==============================================================================*/
{
	 char *text;
	 int new_parameter, selection;
	 float new_ratio;
	 struct Spectrum_settings *settings;
	 
	 ENTER(OnSpectrumBandsValuesEntered);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor && 
			(settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1)))
	 {
			text = (char*)spectrum_editor->spectrum_number_of_bands_text->GetValue().mb_str(wxConvUTF8);
			if (text)
			{
				 sscanf(text,"%d",&new_parameter);
				 text = NULL;
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"OnSpectrumBandsValuesEntered.  Missing widget text");
			}
			if(new_parameter !=
				 Spectrum_settings_get_number_of_bands(settings))
			{
				 Spectrum_settings_set_number_of_bands(settings,new_parameter);
				 spectrum_editor_wx_update_settings(spectrum_editor, settings);
			}
			text = (char*)spectrum_editor->spectrum_ratio_of_black_bands_text->GetValue().mb_str(wxConvUTF8);
			if (text)
			{
				 sscanf(text,"%f",&new_ratio);
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"OnSpectrumBandsValuesEntered.  Missing widget text");
			}
			new_parameter = (int)(new_ratio * 1000.0 + 0.5);
			if(new_parameter !=
				 Spectrum_settings_get_black_band_proportion(settings))
			{
				 Spectrum_settings_set_black_band_proportion(settings,new_parameter);
				 spectrum_editor_wx_update_settings(spectrum_editor, settings);
			}
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "spectrum_editor_settings_bands_CB.  Invalid argument(s)");
	 }
	 LEAVE;
}

void OnSpectrumStepValueEntered(wxCommandEvent &event)
{
	 char *text;
	 float new_parameter;
	 int selection;
	 struct Spectrum_settings *settings;

	 ENTER(OnSpectrumStepValueEntered);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor && 
			(settings = get_settings_at_position_in_Spectrum(spectrum_editor->edit_spectrum,selection+1)))
	 {
			text = (char*)spectrum_editor->spectrum_editor_step_value_text->GetValue().mb_str(wxConvUTF8);
			if (text)
			{
				 sscanf(text,"%f",&new_parameter);
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"OnSpectrumStepValueEntered.  Missing widget text");
			}
			if(new_parameter !=
				Spectrum_settings_get_step_value(settings))
			{
				 Spectrum_settings_set_step_value(settings,new_parameter);
				 spectrum_editor_wx_update_settings(spectrum_editor, settings);
			}
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumStepValueEntered.  Invalid argument(s)");
	 }
	LEAVE;
}

void OnSpectrumAutorangePressed(wxCommandEvent &event)
{
	 float minimum, maximum;
	 int range_set;

	 ENTER(OnSpectrumAutorangePressed);
	 if (spectrum_editor)
	 {
			range_set = 0;
			Scene_get_data_range_for_spectrum(spectrum_editor->autorange_scene,
				 spectrum_editor->edit_spectrum,
				 &minimum, &maximum, &range_set);
			if ( range_set )
			{
				 Spectrum_set_minimum_and_maximum(
						spectrum_editor->edit_spectrum,
						minimum, maximum );
				 spectrum_editor_wx_set_settings(spectrum_editor);
				 spectrum_editor_wx_make_settings_list(spectrum_editor);
				 spectrum_editor_wx_update_scene_viewer(spectrum_editor);
			}
	 }
	else
	{
		display_message(ERROR_MESSAGE,
			"OnSpectrumAutorangePressed.  Invalid argument(s)");
	}
	 LEAVE;
}

DECLARE_EVENT_TABLE();
};
	 
BEGIN_EVENT_TABLE(wxSpectrumEditor, wxFrame)
	 EVT_LISTBOX(XRCID("SpectrumCheckList"), wxSpectrumEditor::OnSpectrumListSelected)
	 EVT_CHECKLISTBOX(XRCID("wxSpectrumSettingsCheckList"), wxSpectrumEditor::OnSpectrumSettingsSelected)
	 EVT_LISTBOX(XRCID("wxSpectrumSettingsCheckList"), wxSpectrumEditor::OnSpectrumSettingsSelected)
	 EVT_BUTTON(XRCID("wxSpectrumSettingsAdd"),wxSpectrumEditor::OnSpectrumSettingListAddPressed)
	 EVT_BUTTON(XRCID("wxSpectrumSettingsDel"),wxSpectrumEditor::OnSpectrumSettingListDelPressed)
	 EVT_BUTTON(XRCID("wxSpectrumSettingsUp"),wxSpectrumEditor::OnSpectrumSettingListUpPressed)
	 EVT_BUTTON(XRCID("wxSpectrumSettingsDn"),wxSpectrumEditor::OnSpectrumSettingListDnPressed)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumRangeMinText"), wxSpectrumEditor::OnSpectrumSettingRangeValueEntered)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumRangeMaxText"), wxSpectrumEditor::OnSpectrumSettingRangeValueEntered)
	 EVT_CHECKBOX(XRCID("wxSpectrumExtendAboveCheck"), wxSpectrumEditor::OnSpectrumSettingRangeChecked)
	 EVT_CHECKBOX(XRCID("wxSpectrumExtendBelowCheck"), wxSpectrumEditor::OnSpectrumSettingRangeChecked)
	 EVT_CHECKBOX(XRCID("wxSpectrumFixMinimumCheck"), wxSpectrumEditor::OnSpectrumSettingRangeChecked)
	 EVT_CHECKBOX(XRCID("wxSpectrumFixMaximumCheck"), wxSpectrumEditor::OnSpectrumSettingRangeChecked)
	 EVT_BUTTON(XRCID("wxSpectrumOKButton"),wxSpectrumEditor::OnSpectrumSettingListOKPressed)
	 EVT_BUTTON(XRCID("wxSpectrumApplyButton"),wxSpectrumEditor::OnSpectrumSettingListApplyPressed)
	 EVT_BUTTON(XRCID("wxSpectrumRevertButton"),wxSpectrumEditor::OnSpectrumSettingListRevertPressed)
	 EVT_BUTTON(XRCID("wxSpectrumCancelButton"),wxSpectrumEditor::OnSpectrumSettingListCancelPressed)
	 EVT_CHOICE(XRCID("wxSpectrumColourChoice"),wxSpectrumEditor::OnSpectrumColourMappingChoice)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumNormalisedColourRangeMin"), wxSpectrumEditor::OnSpectrumNormalisedColourRangeValueEntered)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumNormalisedColourRangeMax"), wxSpectrumEditor::OnSpectrumNormalisedColourRangeValueEntered)
	 EVT_CHECKBOX(XRCID("wxSpectrumReverseCheck"), wxSpectrumEditor::OnSpectrumSettingsReverseChecked)
	 EVT_CHOICE(XRCID("wxSpectrumTypeChoice"), wxSpectrumEditor::OnSpectrumTypeChoice)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumExaggerationTextCtrl"), wxSpectrumEditor::OnSpectrumExaggerationSettingsChanged)
	 EVT_RADIOBOX(XRCID("wxSpectrumLeftRightRadioBox"), wxSpectrumEditor::OnSpectrumExaggerationSettingsChanged)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumDataComponentText"), wxSpectrumEditor::OnSpectrumDataValueEntered)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumNumberOfBandsTextCtrl"), wxSpectrumEditor::OnSpectrumBandsValuesEntered)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumRatioOfBlackBandsTextCtrl"), wxSpectrumEditor::OnSpectrumBandsValuesEntered)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumStepValueTextCtrl"), wxSpectrumEditor::OnSpectrumStepValueEntered)
	 EVT_BUTTON(XRCID("wxSpectrumAutorangeButton"),wxSpectrumEditor::OnSpectrumAutorangePressed)
END_EVENT_TABLE()

static int spectrum_editor_wx_update_scene_viewer(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
==============================================================================*/
{
	char **strings, *string_data;
	const float extend_ends = 0.04;
	struct Colour black ={0, 0, 0}, white = {1.0, 1.0, 1.0};
	int i, j, npts1, npts2, number_of_points,
		tick_label_count, tick_line_count, return_code;
	float bar_min, bar_max, min, max, value_xi1;
	GTDATA *data;
	Triple *line_points, *label_points;
	struct GT_surface *surface;
	struct GT_polyline *tick_lines;
	struct GT_pointset *tick_labels;

	ENTER(spectrum_editor_wx_update_scene_viewer);
	/* check arguments */
	if (spectrum_editor && spectrum_editor->edit_spectrum 
		&& spectrum_editor->graphics_object
		&& spectrum_editor->tick_lines_graphics_object
		 && spectrum_editor->tick_labels_graphics_object)
	{
		surface = GT_OBJECT_GET(GT_surface)(spectrum_editor->graphics_object, 0);
		tick_lines = GT_OBJECT_GET(GT_polyline)(spectrum_editor->tick_lines_graphics_object, 0);
		tick_labels = GT_OBJECT_GET(GT_pointset)(spectrum_editor->tick_labels_graphics_object, 0);
		data = surface->data;
		npts1 = surface->n_pts1;
		npts2 = surface->n_pts2;
		strings = tick_labels->text;
		label_points = tick_labels->pointlist;
		tick_label_count = tick_labels->n_pts;
		line_points = tick_lines->pointlist;
		tick_line_count = tick_lines->n_pts;

		switch (spectrum_editor->viewer_type % 6)
		{
			case 0:
			{
				Graphical_material_set_ambient(spectrum_editor->editor_material, &black );
				Graphical_material_set_diffuse(spectrum_editor->editor_material, &black );
				number_of_points = 5;
			} break;
			case 1:
			{
				number_of_points = 11;
			} break;
			case 2:
			{
				number_of_points = 2;
			} break;
			case 3:
			{
				Graphical_material_set_ambient(spectrum_editor->editor_material, &white );
				Graphical_material_set_diffuse(spectrum_editor->editor_material, &white );
				number_of_points = 5;
			} break;
			case 4:
			{
				number_of_points = 11;
			} break;
			case 5:
			{
				number_of_points = 2;
			} break;
		}
		if ( tick_label_count != number_of_points 
			|| tick_line_count != number_of_points )
		{
			if ( strings )
			{
				string_data = strings[0];
			}
			else
			{
				string_data = (char *)NULL;
			}
			if (REALLOCATE(line_points, line_points, Triple, 2 * number_of_points)
				&& REALLOCATE(label_points, label_points, Triple, number_of_points)
				&& REALLOCATE(string_data, string_data, char, number_of_points * 15)
				&& REALLOCATE(strings, strings, char *, number_of_points))
			{
				tick_line_count = number_of_points;
				tick_label_count = number_of_points;
				for ( i = 0 ; i < number_of_points ; i++ )
				{
					value_xi1 = (-5.0 + 10.0 * (float) i / 
						(float)(number_of_points - 1))
						/ (1. + 2.0 * extend_ends);
					label_points[i][0] = value_xi1;
					label_points[i][1] = 0;
					label_points[i][2] = -1.0;
					strings[i] = string_data + 15 * i;
					/* the strings will be set below */
				}
				i = 0;
				while( i < 2 * number_of_points )
				{
					value_xi1 = (-5.0 + 10.0 * (float) i / 
						(float)(2 * number_of_points - 2))
						/ (1. + 2.0 * extend_ends);
					line_points[i][0] = value_xi1;
					line_points[i][1] = 0;
					line_points[i][2] = -0.5;
					i++;
					line_points[i][0] = value_xi1;
					line_points[i][1] = 0;
					line_points[i][2] = -1.0;
					i++;
				}
				tick_labels->text = strings;
				tick_labels->pointlist = label_points;
				tick_labels->n_pts = tick_label_count;
				tick_lines->pointlist = line_points;
				tick_lines->n_pts = tick_line_count;
			}

			GT_object_changed(spectrum_editor->tick_lines_graphics_object);
		}
		min = get_Spectrum_minimum(spectrum_editor->edit_spectrum);
		max = get_Spectrum_maximum(spectrum_editor->edit_spectrum);

		for ( i = 0 ; i < tick_label_count ; i++ )
		{
			sprintf(strings[i], "%6.2g", min + (max - min)
				* (float)i / (float)(tick_label_count - 1));
		}

		bar_min = min - extend_ends * (max - min);
		bar_max = max + extend_ends * (max - min);
		for ( i = 0 ; i < npts2 ; i++ )
		{
			for ( j = 0 ; j < npts1 ; j++ )
			{
				*data = bar_min  + (bar_max - bar_min)
					* (float)j / (float)(npts1 - 1);
				data++;
			}
		}
		GT_object_changed(spectrum_editor->graphics_object);
		GT_object_changed(spectrum_editor->tick_labels_graphics_object);
		Scene_viewer_redraw(spectrum_editor->spectrum_editor_scene_viewer);
		if (spectrum_editor->spectrum_panel)
		{
			 spectrum_editor->spectrum_panel->Update();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_wx_update_scene_viewer.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_wx_update_scene_viewer */

static void spectrum_editor_viewer_input_callback(
	struct Scene_viewer *scene_viewer, struct Graphics_buffer_input *input,
	void *spectrum_editor_void)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Callback for when input is received by the scene_viewer.
==============================================================================*/
{
	struct Spectrum_editor *spectrum_editor;

	ENTER(spectrum_editor_viewer_input_CB);
	USE_PARAMETER(scene_viewer);
	if (spectrum_editor=(struct Spectrum_editor *)spectrum_editor_void)
	{
		if (GRAPHICS_BUFFER_BUTTON_PRESS==input->type)
		{
			/* Increment the type */
			spectrum_editor->viewer_type++;
			spectrum_editor_wx_update_scene_viewer(spectrum_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_viewer_input_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* spectrum_editor_viewer_input_CB */

struct Spectrum_editor *CREATE(Spectrum_editor)(
	 struct Spectrum_editor_dialog **spectrum_editor_dialog_address,
	 struct Spectrum *spectrum,
	 struct Graphics_font *font,
	 struct Graphics_buffer_package *graphics_buffer_package,
	 struct User_interface *user_interface,
	 struct LIST(GT_object) *glyph_list,
	 struct MANAGER(Graphical_material) *graphical_material_manager,
	 struct MANAGER(Light) *light_manager,
	 struct MANAGER(Spectrum) *spectrum_manager,
	 struct MANAGER(Texture) *texture_manager,
	 struct MANAGER(Scene) *scene_manager )
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
Creates a spectrum_editor widget.
==============================================================================*/
{
	 char *name;
	 int i,j,return_code,surface_discretise_xi1=24,surface_discretise_xi2=108;
	 GTDATA *data;
	 struct Spectrum_editor *spectrum_editor;
	 struct Colour background_colour = {0.1, 0.1, 0.1},
			ambient_colour = {0.2, 0.2, 0.2}, black ={0, 0, 0},
			off_white = {0.9, 0.8, 0.8};
	 struct Graphics_buffer *graphics_buffer;
	 struct Light *viewer_light;
	 struct Light_model *viewer_light_model;
	 Triple *points, *normalpoints;
	 float value_xi1, value_xi2, light_direction[3] = {0, -0.2, -1.0};
	 struct GT_surface *cylinder_surface;
	 struct GT_polyline *tick_lines;
	 struct GT_pointset *tick_labels;
	 struct Graphical_material *tick_material;
	 char **strings, *string_data;
	 struct Spectrum *default_scene_spectrum;

	ENTER(CREATE(Spectrum_editor));
	spectrum_editor = (struct Spectrum_editor *)NULL;
	if (user_interface)
	{
			/* allocate memory */
			if (ALLOCATE(spectrum_editor,struct Spectrum_editor,1))
			{
				/* initialise the structure */
				 spectrum_editor->spectrum_editor_dialog_address = spectrum_editor_dialog_address;
				spectrum_editor->current_settings = (struct Spectrum_settings *)NULL;
				spectrum_editor->current_spectrum=(struct Spectrum *)NULL;
				spectrum_editor->edit_spectrum=(struct Spectrum *)NULL;
				spectrum_editor->user_interface=user_interface;
				spectrum_editor->material_manager_callback_id=(void *)NULL;
				spectrum_editor->spectrum_manager_callback_id=(void *)NULL;
				spectrum_editor->spectrum_manager = spectrum_manager;
				spectrum_editor->editor_material = (struct Graphical_material *)NULL;
				spectrum_editor->tick_material = (struct Graphical_material *)NULL;
				spectrum_editor->graphics_object = (struct GT_object *)NULL;
				spectrum_editor->tick_lines_graphics_object = (struct GT_object *)NULL;
				spectrum_editor->tick_labels_graphics_object = (struct GT_object *)NULL;
				spectrum_editor->autorange_scene = (struct Scene*)NULL;
				spectrum_editor->spectrum_editor_scene = (struct Scene *)NULL;
				spectrum_editor->spectrum_editor_scene_viewer = (struct Scene_viewer *)NULL;
				spectrum_editor->viewer_type = 0;
				spectrum_editor->scene_manager = scene_manager;
				spectrum_editor->spectrum_range_min_text = NULL;
				spectrum_editor->spectrum_range_max_text = NULL;
				spectrum_editor->spectrum_settings_checklist = NULL;
				spectrum_editor->spectrum_settings_add_button = NULL;
				spectrum_editor->spectrum_settings_del_button = NULL;
				spectrum_editor->spectrum_settings_up_button = NULL;
				spectrum_editor->spectrum_settings_dn_button = NULL;
				spectrum_editor->spectrum_reverse_checkbox = NULL;
				spectrum_editor->spectrum_colour_mapping_choice = NULL;
				spectrum_editor->spectrum_exaggeration_text = NULL;
				spectrum_editor->spectrum_left_right_radio_box = NULL;
				spectrum_editor->spectrum_data_component_text = NULL;
				spectrum_editor->spectrum_number_of_bands_text = NULL;
				spectrum_editor->spectrum_ratio_of_black_bands_text = NULL;
				spectrum_editor->spectrum_editor_step_value_text = NULL;
				spectrum_editor->spectrum_extend_below_check = NULL;
				spectrum_editor->spectrum_extend_above_check = NULL;
				spectrum_editor->spectrum_fix_minimum_check = NULL;
				spectrum_editor->spectrum_fix_maximum_check = NULL;	
				spectrum_editor->spectrum_normalised_colour_range_min_text = NULL;	
				spectrum_editor->spectrum_normalised_colour_range_max_text = NULL;
				spectrum_editor->spectrum_type_choice = NULL;
				spectrum_editor->spectrum_higher_panel = NULL;
				spectrum_editor->spectrum_lower_panel = NULL;
				// User_Interface
				spectrum_editor->wx_spectrum_editor = (wxSpectrumEditor *)NULL;
				spectrum_editor->wx_spectrum_editor = new wxSpectrumEditor(spectrum_editor);
				spectrum_editor->spectrum_panel = XRCCTRL(*spectrum_editor->wx_spectrum_editor
					 , "SpectrumPanel", wxPanel);
				spectrum_editor->spectrum_panel->SetSize(wxDefaultCoord,wxDefaultCoord,
					 400, 150);
				spectrum_editor->spectrum_panel->SetMinSize(wxSize(-1,150));
				spectrum_editor->spectrum_editor_check_list = XRCCTRL(*spectrum_editor->wx_spectrum_editor
					 , "SpectrumCheckList", wxListBox);
				spectrum_editor->spectrum_editor_check_list->SetSize(wxDefaultCoord,wxDefaultCoord,
					 400, 100);
				spectrum_editor->spectrum_editor_check_list->SetMinSize(wxSize(-1,100));
				spectrum_editor->spectrum_editor_frame = XRCCTRL(*spectrum_editor->wx_spectrum_editor
					 , "CmguiSpectrumEditor", wxFrame);
				spectrum_editor->spectrum_editor_frame->SetSize(wxDefaultCoord,wxDefaultCoord,
					 400, 700);
				spectrum_editor->spectrum_editor_frame->SetMinSize(wxSize(1,1));
				spectrum_editor->spectrum_range_min_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumRangeMinText", wxTextCtrl);
				spectrum_editor->spectrum_range_max_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumRangeMaxText", wxTextCtrl);
				spectrum_editor->spectrum_settings_checklist = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumSettingsCheckList", wxCheckListBox);
				spectrum_editor->spectrum_settings_add_button = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumSettingsAdd", wxButton);
				spectrum_editor->spectrum_settings_del_button = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumSettingsDel", wxButton);
				spectrum_editor->spectrum_settings_up_button = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumSettingsUp", wxButton);
				spectrum_editor->spectrum_settings_dn_button = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumSettingsDn", wxButton);
				spectrum_editor->spectrum_reverse_checkbox = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumReverseCheck", wxCheckBox);
				spectrum_editor->spectrum_colour_mapping_choice = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumColourChoice", wxChoice);
				spectrum_editor->spectrum_exaggeration_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumExaggerationTextCtrl", wxTextCtrl);
				spectrum_editor->spectrum_left_right_radio_box = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumLeftRightRadioBox", wxRadioBox);
				spectrum_editor->spectrum_data_component_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumDataComponentText", wxTextCtrl);
				spectrum_editor->spectrum_number_of_bands_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumNumberOfBandsTextCtrl", wxTextCtrl);
				spectrum_editor->spectrum_ratio_of_black_bands_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumRatioOfBlackBandsTextCtrl", wxTextCtrl);
				spectrum_editor->spectrum_editor_step_value_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumStepValueTextCtrl", wxTextCtrl);
				spectrum_editor->spectrum_extend_below_check = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumExtendBelowCheck", wxCheckBox);
				spectrum_editor->spectrum_extend_above_check = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumExtendAboveCheck", wxCheckBox);
				spectrum_editor->spectrum_fix_minimum_check = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumFixMinimumCheck", wxCheckBox);
				spectrum_editor->spectrum_fix_maximum_check = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumFixMaximumCheck", wxCheckBox);
				spectrum_editor->spectrum_normalised_colour_range_min_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumNormalisedColourRangeMin", wxTextCtrl);
				spectrum_editor->spectrum_normalised_colour_range_max_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumNormalisedColourRangeMax", wxTextCtrl);
				spectrum_editor->spectrum_type_choice = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumTypeChoice", wxChoice);
				spectrum_editor->spectrum_higher_panel = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumHigherPanel", wxScrolledWindow);
				spectrum_editor->spectrum_lower_panel = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumLowerPanel", wxScrolledWindow);
				if (spectrum_editor->wx_spectrum_editor != NULL)
				{
					 spectrum_editor->spectrum_lower_panel->SetScrollbars(10,10,10,40);
					 spectrum_editor->spectrum_higher_panel->SetScrollbars(10,10,40,40);
					 return_code = 1;
					 spectrum_editor->editor_material = ACCESS(Graphical_material)
							(CREATE(Graphical_material)("editor_material"));
					 Graphical_material_set_ambient(spectrum_editor->editor_material, &black );
					 Graphical_material_set_diffuse(spectrum_editor->editor_material, &black );
					 Graphical_material_set_shininess(spectrum_editor->editor_material, 0.8 );
					 tick_material = CREATE(Graphical_material)("editor_material");
					 spectrum_editor->tick_material = ACCESS(Graphical_material)(
							tick_material);
					 Graphical_material_set_ambient(tick_material, &off_white );
					 Graphical_material_set_diffuse(tick_material, &off_white );
					 Graphical_material_set_shininess(tick_material, 0.8 );
					 if ( ALLOCATE( points, Triple, surface_discretise_xi1 *
								 surface_discretise_xi2) &&
							ALLOCATE( normalpoints, Triple, surface_discretise_xi1 *
								 surface_discretise_xi2) &&
							ALLOCATE( data, GTDATA, surface_discretise_xi1 *
								 surface_discretise_xi2 ) )
					 {
							for ( i = 0 ; i < surface_discretise_xi1 ; i++ )
							{
								 value_xi1 = sin ( (float) i * 2.0 * PI / (float)(surface_discretise_xi1 - 1));
								 value_xi2 = cos ( (float) i * 2.0 * PI / (float)(surface_discretise_xi1 - 1));
								 for ( j = 0 ; j < surface_discretise_xi2 ; j++ )
								 {
										points[i * surface_discretise_xi2 + j][0] = -5.0 + 10.0 * (float) j / 
											 (float)(surface_discretise_xi2 - 1);
										points[i * surface_discretise_xi2 + j][1] = value_xi1;
										points[i * surface_discretise_xi2 + j][2] = 0.5 + value_xi2;
										/* Normals */
										normalpoints[i * surface_discretise_xi2 + j][0] = 0;
										normalpoints[i * surface_discretise_xi2 + j][1] = value_xi1;
										normalpoints[i * surface_discretise_xi2 + j][2] = value_xi2;
										/* Spectrum */
										data[i * surface_discretise_xi2 + j] = (float) j / 
											 (float)(surface_discretise_xi2 - 1);
								 }
							}
							if ((spectrum_editor->graphics_object =
										CREATE(GT_object)("spectrum_editor_surface",g_SURFACE,
											 spectrum_editor->editor_material)))
							{
								 ACCESS(GT_object)(spectrum_editor->graphics_object);
								 if (cylinder_surface=CREATE(GT_surface)(
												g_SHADED_TEXMAP, g_QUADRILATERAL,
												surface_discretise_xi2, surface_discretise_xi1,
												points, normalpoints, /*tangentpoints*/(Triple *)NULL,
												/*texturepoints*/(Triple *)NULL,
												/* n_data_components */1, data))
								 {
										GT_OBJECT_ADD(GT_surface)(
											 spectrum_editor->graphics_object, 0,
											 cylinder_surface);
								 }
								 else
								 {
										DEALLOCATE( points );
										DEALLOCATE( data );
										return_code = 0;
										display_message(ERROR_MESSAGE,
											 "CREATE(Spectrum_editor). Unable to create surface");
								 }
							}
							else
							{
								 DEALLOCATE( points );
								 DEALLOCATE( data );
								 return_code = 0;
								 display_message(ERROR_MESSAGE,
										"CREATE(Spectrum_editor). Unable to create graphics_object");
							}
					 }
					 if ( return_code )
					 {
							points = (Triple *)NULL;
							if ((spectrum_editor->tick_lines_graphics_object=
										CREATE(GT_object)("spectrum_editor_tick_lines",g_POLYLINE,
											 tick_material)))
							{
								 GT_object_set_next_object(spectrum_editor->graphics_object,
										spectrum_editor->tick_lines_graphics_object);
								 if (tick_lines = CREATE(GT_polyline)(
												g_PLAIN_DISCONTINUOUS, /*line_width=default*/0,
												0, points, /* normalpoints */(Triple *)NULL,
												g_NO_DATA, (GTDATA *)NULL))
								 {
										GT_OBJECT_ADD(GT_polyline)(
											 spectrum_editor->tick_lines_graphics_object, 0,
											 tick_lines);
								 }
								 else
								 {
										return_code = 0;
										display_message(ERROR_MESSAGE,
											 "CREATE(Spectrum_editor). Unable to create lines");
								 }
							}
							else
							{
								 return_code = 0;
								 display_message(ERROR_MESSAGE,
										"CREATE(Spectrum_editor). Unable to create tick line graphics_object");
							}
					 }
					 if ( return_code
							&& ALLOCATE( points, Triple, 1) &&
							ALLOCATE( strings, char *, 1) &&
							ALLOCATE( string_data, char, 1))
					 {
							points[0][0] = 0;
							points[0][1] = 0;
							points[0][2] = 0;
							strings[0] = string_data;
							string_data[0] = 0;
							if ((spectrum_editor->tick_labels_graphics_object =
										CREATE(GT_object)("spectrum_editor_tick_labels",
											 g_POINTSET,tick_material)))
							{
								 GT_object_set_next_object(spectrum_editor->tick_lines_graphics_object,
										spectrum_editor->tick_labels_graphics_object);
								 if (tick_labels = CREATE(GT_pointset)(1,
											 points, strings, g_NO_MARKER, 0.0,
											 g_NO_DATA, (GTDATA *)NULL, (int *)NULL, font))
								 {
										GT_OBJECT_ADD(GT_pointset)(
											 spectrum_editor->tick_labels_graphics_object, 0,
											 tick_labels);
								 }
								 else
								 {
										return_code = 0;
										display_message(ERROR_MESSAGE,
											 "CREATE(Spectrum_editor). Unable to create tick label pointset");
								 }
							}
							else
							{
								 return_code = 0;
								 display_message(ERROR_MESSAGE,
										"CREATE(Spectrum_editor). Unable to create tick label graphics_object");
							}
					 }
					 if ( return_code )
					 {
							default_scene_spectrum = CREATE(Spectrum)("default_scene_spectrum");
							spectrum_editor->spectrum_editor_scene = CREATE(Scene)("spectrum_editor_scene");
							Scene_enable_graphics( spectrum_editor->spectrum_editor_scene,
								 glyph_list, graphical_material_manager, 
								 spectrum_editor->editor_material, font, light_manager,
								 spectrum_manager, default_scene_spectrum, 
								 texture_manager);
							viewer_light = CREATE(Light)("spectrum_editor_light");
							set_Light_direction(viewer_light, light_direction);
							viewer_light_model = CREATE(Light_model)("spectrum_editor_light_model");
							Light_model_set_ambient(viewer_light_model, &ambient_colour);
							if (graphics_buffer = create_Graphics_buffer_wx(
										 graphics_buffer_package, spectrum_editor->spectrum_panel,
										 GRAPHICS_BUFFER_ANY_BUFFERING_MODE, GRAPHICS_BUFFER_ANY_STEREO_MODE,
										 /*minimum_colour_buffer_depth*/8,
										 /*minimum_depth_buffer_depth*/8,
										 /*minimum_accumulation_buffer_depth*/0))
							{
								 spectrum_editor->spectrum_editor_scene_viewer = 
										CREATE(Scene_viewer)(graphics_buffer,
											 &background_colour,
											 (struct MANAGER(Light) *)NULL,viewer_light,
											 (struct MANAGER(Light_model) *)NULL,viewer_light_model,
											 (struct MANAGER(Scene) *)NULL,
											 spectrum_editor->spectrum_editor_scene,
											 (struct MANAGER(Texture) *)NULL,
											 user_interface );
								 GET_NAME(GT_object)(spectrum_editor->graphics_object,
										&name);
								 return_code=Scene_add_graphics_object(
										spectrum_editor->spectrum_editor_scene,
										spectrum_editor->graphics_object, 0,
										name, /*fast_changing*/0);
								 DEALLOCATE(name);
								 Scene_viewer_set_input_mode(
										spectrum_editor->spectrum_editor_scene_viewer,
										SCENE_VIEWER_NO_INPUT );
								 Scene_viewer_add_input_callback(
										spectrum_editor->spectrum_editor_scene_viewer,
										spectrum_editor_viewer_input_callback,
										(void *)spectrum_editor);
								 Scene_viewer_set_viewport_size(
										spectrum_editor->spectrum_editor_scene_viewer,400,150);
								 Scene_viewer_set_lookat_parameters(
										spectrum_editor->spectrum_editor_scene_viewer,0,-1,0,0,0,
										0,0,0,1);
								 Scene_viewer_set_view_simple(
										spectrum_editor->spectrum_editor_scene_viewer,0,0,0,2.3,
										46,10);
								 Scene_viewer_redraw(
										spectrum_editor->spectrum_editor_scene_viewer);
							}
					 }
					 FOR_EACH_OBJECT_IN_MANAGER(Spectrum)(
							spectrum_editor_wx_add_item_to_spectrum_editor_check_list, 
							(void *)spectrum_editor, spectrum_manager);
					 if (spectrum)
					 {
							make_current_spectrum(spectrum_editor, spectrum);
					 }
					 spectrum_editor->wx_spectrum_editor->set_autorange_scene();
					 spectrum_editor->wx_spectrum_editor->Show();
				}
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"CREATE(Spectrum_editor).  "
						"Could not allocate spectrum_editor widget structure");
			}
	}
	else
	{
		 display_message(ERROR_MESSAGE,
			"CREATE(Spectrum_editor).  Invalid argument(s)");
	}
	LEAVE;

	return (spectrum_editor);
} /* CREATE(Spectrum_editor) */

int spectrum_editor_wx_set_spectrum(
	struct Spectrum_editor *spectrum_editor, struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the <spectrum> to be edited by the <spectrum_editor>.
==============================================================================*/
{
	int return_code;
// 	struct Callback_data callback;

	ENTER(spectrum_editor_wx_set_spectrum);
	if (spectrum_editor)
	{
		if (spectrum)
		{
			if (make_edit_spectrum(spectrum_editor,spectrum))
			{
				/* continue with the current_settings_type */
				 spectrum_editor_wx_make_settings_list(spectrum_editor);
				/* select the first settings item in the list (if any) */
				spectrum_editor->current_settings=(struct Spectrum_settings *)NULL;
				spectrum_editor_wx_select_settings_item(spectrum_editor);
				/* turn on callbacks from settings editor */
			}
			else
			{
				spectrum=(struct Spectrum *)NULL;
			}
		}
		if (!spectrum)
		{
			/* turn off settings editor by passing NULL settings */
			spectrum_editor->current_settings=(struct Spectrum_settings *)NULL;
			spectrum_editor_wx_set_settings(spectrum_editor);
			if (spectrum_editor->edit_spectrum)
			{
				DESTROY(Spectrum)(&(spectrum_editor->edit_spectrum));
			}
		}
		spectrum_editor_wx_update_scene_viewer(spectrum_editor);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_wx_set_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_set_spectrum */

int spectrum_editor_wx_add_item_to_spectrum_editor_check_list(struct Spectrum *spectrum, void *spectrum_editor_void)
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
Add spectrum item to the spectrum_editor.
==============================================================================*/
{
	 char *spectrum_name;
	 struct Spectrum_editor *spectrum_editor;
	 int number_of_spectrums, return_code;

	 ENTER(Spectrum_editor_wx_add_item_to_spectrum_editor_check_list);
	 if ((spectrum_editor = (struct Spectrum_editor *)spectrum_editor_void) &&
			(spectrum_editor->spectrum_editor_check_list) &&
			(spectrum_name =  Spectrum_get_name(spectrum)))
	 {
				 spectrum_editor->spectrum_editor_check_list->Append(spectrum_name);
				 number_of_spectrums = spectrum_editor->spectrum_editor_check_list->GetCount();
				 spectrum_editor->spectrum_editor_check_list->SetClientData(number_of_spectrums - 1,
						(void *)spectrum);
				 if (spectrum_editor->edit_spectrum == spectrum)
				 {
						spectrum_editor->spectrum_editor_check_list->SetSelection(number_of_spectrums - 1);
				 }
				 return_code = 1;
	 }
	 else
	 {
		display_message(ERROR_MESSAGE,
			"Spectrum_editor_wx_add_item_to_spectrum_editor_check_list.  Invalid argument(s)");
		return_code = 0;
	 }
	 LEAVE;

	 return (return_code);
} /* spectrum_editor_wx_add_item_to_spectrum_editor_check_list */

int DESTROY(Spectrum_editor)(struct Spectrum_editor **spectrum_editor_address)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*spectrum_editor_address> and sets
<*spectrum_editor_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct GT_pointset *tick_labels;
	struct Spectrum_editor *spectrum_editor;

	ENTER(DESTROY(Spectrum_editor));
	if (spectrum_editor_address &&
		(spectrum_editor = *spectrum_editor_address))
	{
		return_code = 1;
		DEACCESS(Graphical_material)(&spectrum_editor->editor_material);
		DEACCESS(Graphical_material)(&spectrum_editor->tick_material);
		/* The strings in the labels graphics object are stored in two 
			 ALLOCATED blocks instead of ALLOCATING each string individually.
			 So I will manually DEALLOCATE the strings and set them to NULL */
		tick_labels = GT_OBJECT_GET(GT_pointset)(
			spectrum_editor->tick_labels_graphics_object, 0);
		if ( tick_labels->text )
		{
			DEALLOCATE(tick_labels->text[0]);
			DEALLOCATE(tick_labels->text);
			tick_labels->text = (char **)NULL;
		}
		/* The DEACCESS for the first graphics object automatically works
			 down the linked list chain */
		DEACCESS(GT_object)(&spectrum_editor->graphics_object);
		DESTROY(Scene_viewer)(&spectrum_editor->spectrum_editor_scene_viewer);
		delete spectrum_editor->wx_spectrum_editor;
		/* destroy edit_spectrum */
		if (spectrum_editor->edit_spectrum)
		{
			DEACCESS(Spectrum)(
				&(spectrum_editor->edit_spectrum));
		}

		DEALLOCATE(*spectrum_editor_address);
		*spectrum_editor_address = (struct Spectrum_editor *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Spectrum_editor).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Spectrum_editor) */


