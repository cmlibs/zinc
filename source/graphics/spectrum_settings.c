/*******************************************************************************
FILE : spectrum_settings.c

Largely pillaged from graphics/element_group_settings.c

LAST MODIFIED : 15 March 2002

DESCRIPTION :
Spectrum_settings structure and routines for describing and manipulating the
appearance of spectrums.
==============================================================================*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/list.h"
#include "general/compare.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_settings.h"
#include "user_interface/message.h"

/*
Global variables
----------------
*/
/*
Module types
------------
*/

struct Spectrum_settings
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Stores one group of settings for a single part of a spectrum rendition.
==============================================================================*/
{
	/* unique identifier for each settings */
	int position;
	int component_number; /* Which data component this settings uses (0 is first component)*/
	int active; /* This corresponds to visiblity for graphical finite elements */
	enum Spectrum_settings_type settings_type;
	char settings_changed;	
	/* These specify the range of values over which the settings operates */
	float maximum, minimum;
	/* These flags control whether the maximum, minumum values can be changed */
	int fix_maximum,fix_minimum;
	/* These flags control whether a settings is transparent (has no effect)
		or is clamped at its extreme values outside it's minimum and maximum */
	int extend_above, extend_below;
	/* These specify the limits of the converted value before it is rendered to
		a colour, i.e. red varies from <min_value> red at the <minimum> to 
		<max_value> red at the <maximum> */
	float max_value, min_value;
	int reverse;
	enum Spectrum_settings_colour_mapping colour_mapping;
	enum Spectrum_settings_render_type render_type;
	float exaggeration, step_value;
	/* The number of bands in a banded contour and the proportion (out of 1000)
		of the black bands */
	int number_of_bands, black_band_proportion;

#if defined (OPENGL_API)
	/* Texture number for banded and step spectrums */
	unsigned int texture_id;
#endif /* defined (OPENGL_API) */
	 
	/* For accessing objects */
	int access_count;
};

FULL_DECLARE_INDEXED_LIST_TYPE(Spectrum_settings);

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Spectrum_settings,position,int, \
	compare_int)


static int Spectrum_settings_set_changed(struct Spectrum_settings *settings,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Sets settings->settings_changed to 1.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_changed);
	USE_PARAMETER(dummy_void);
	/* check arguments */
	if (settings)
	{
		settings->settings_changed=1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_changed.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_changed */

int Spectrum_settings_set_active(struct Spectrum_settings *settings,
	int active)
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Sets settings->settings_active to the <active> value;
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_active);
	/* check arguments */
	if (settings)
	{
		settings->active=active;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_active.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_active */

int Spectrum_settings_get_active(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 27 July 1998

DESCRIPTION :
Gets the settings->active value;
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_get_active);
	/* check arguments */
	if (settings)
	{
		return_code = settings->active;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_active.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_get_active */

static int Spectrum_settings_changed_if_type(
	struct Spectrum_settings *settings,void *settings_type_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
If the settings are of type settings_type, sets settings->settings_changed to 1.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_changed_if_type);
	/* check arguments */
	if (settings)
	{
		if ((enum Spectrum_settings_type)settings_type_void ==
			settings->settings_type)
		{
			settings->settings_changed=1;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_changed_if_type.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_changed_if_type */

/*
Global functions
----------------
*/
DECLARE_OBJECT_FUNCTIONS(Spectrum_settings)
DECLARE_INDEXED_LIST_FUNCTIONS(Spectrum_settings)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Spectrum_settings, \
	position,int,compare_int)

struct Spectrum_settings *CREATE(Spectrum_settings)(void)
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Allocates memory and assigns fields for a struct Spectrum_settings.
==============================================================================*/
{
	struct Spectrum_settings *settings;

	ENTER(CREATE(Spectrum_settings));
	if (ALLOCATE(settings,struct Spectrum_settings,1))
	{
		settings->component_number = 0;
		settings->settings_type=SPECTRUM_LINEAR;
		settings->settings_changed=1;
		settings->minimum = 0;
		settings->maximum = 1;
		settings->fix_maximum=0;
		settings->fix_minimum=0;
		settings->extend_above = 0;
		settings->extend_below = 0;
		settings->min_value = 0;
		settings->max_value = 1;
		settings->colour_mapping = SPECTRUM_RAINBOW;
		settings->render_type = SPECTRUM_AMBIENT_AND_DIFFUSE;
		settings->reverse = 0;
		settings->exaggeration = 1.0;
		settings->step_value = 0.5;
		settings->number_of_bands = 10;
		settings->black_band_proportion = 200;
		settings->active=1;
#if defined (OPENGL_API)
		settings->texture_id=0;
#endif /* defined (OPENGL_API) */
		settings->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Spectrum_settings).  "
			"Insufficient memory");
	}
	LEAVE;

	return (settings);
} /* CREATE(Spectrum_settings) */

int DESTROY(Spectrum_settings)(struct Spectrum_settings **settings_ptr)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Frees the memory for the fields of <**settings_ptr>, frees the memory for
<**settings_ptr> and sets <*settings_ptr> to NULL.
==============================================================================*/
{
	struct Spectrum_settings *settings;
	int return_code;

	ENTER(DESTROY(Spectrum_settings));
	if (settings_ptr)
	{
		if (settings= *settings_ptr)
		{
#if defined (OPENGL_API)
			if (settings->texture_id)
			{
				glDeleteTextures(1, &(settings->texture_id));
			}
#endif /* defined (OPENGL_API) */
			switch (settings->settings_type)
			{
				case SPECTRUM_LINEAR:
				case SPECTRUM_LOG:
				case SPECTRUM_BANDED:
				case SPECTRUM_STEP:
				{
					/* Don't need to do anything */
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"DESTROY(Spectrum_settings).  Unknown element settings type");
				} break;
			}
			/*???RC check temp access_count is zero! */
			if (0!=settings->access_count)
			{
				display_message(ERROR_MESSAGE,
					"DESTROY(Spectrum_settings).  Non-zero access_count");
			}
			DEALLOCATE(*settings_ptr);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Spectrum_settings).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Spectrum_settings) */

PROTOTYPE_COPY_OBJECT_FUNCTION(Spectrum_settings)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
syntax: COPY(Spectrum_settings)(destination,source)
Copies the Spectrum contents from source to destination.
Note: destination->access_count is not changed by COPY.
==============================================================================*/
{
	int return_code;

	ENTER(COPY(Spectrum_settings));
	/* check arguments */
	if (source&&destination)
	{
		destination->settings_changed = 1;
		/* copy settings used by all settings_types */
		destination->component_number = source->component_number;
		destination->settings_type = source->settings_type;
		destination->reverse = source->reverse;
		destination->position = source->position;
		destination->active = source->active;
		destination->minimum = source->minimum;
		destination->maximum = source->maximum;	
		destination->fix_maximum=source->fix_maximum;
		destination->fix_minimum=source->fix_minimum;
		destination->extend_above = source->extend_above;
		destination->extend_below = source->extend_below;
		destination->min_value = source->min_value;
		destination->max_value = source->max_value;
		destination->render_type = source->render_type;
		destination->colour_mapping = source->colour_mapping;
		destination->exaggeration = source->exaggeration;
		destination->number_of_bands = source->number_of_bands;
		destination->black_band_proportion = source->black_band_proportion;
		destination->step_value = source->step_value;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"COPY(Spectrum_settings).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* COPY(Spectrum_settings) */

int Spectrum_settings_clear_fixed(struct Spectrum_settings *settings,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Sets settings->fix_minimum,settings->fix_maximum to 0.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_clear_fixed);
	USE_PARAMETER(dummy_void);
	/* check arguments */
	if (settings)
	{
		settings->fix_minimum=0;
		settings->fix_maximum=0;
		settings->settings_changed=1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_clear_fixed.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_clear_fixed */

int Spectrum_settings_copy_and_put_in_list(
	struct Spectrum_settings *settings,void *list_of_settings_void)
/*******************************************************************************
LAST MODIFIED : 27 July 1998

DESCRIPTION :
Spectrum_settings iterator function for copying a list_of_settings.
Makes a copy of the settings and puts it in the list_of_settings.
==============================================================================*/
{
	int return_code;
	struct Spectrum_settings *copy_settings;
	struct LIST(Spectrum_settings) *list_of_settings;

	ENTER(Spectrum_settings_copy_and_put_in_list);
	if (settings&&(list_of_settings=
		(struct LIST(Spectrum_settings) *)list_of_settings_void))
	{
		/* create new settings to take the copy */
		if (copy_settings=CREATE(Spectrum_settings)())
		{
			/* copy and insert in list */
			if (!(return_code=COPY(Spectrum_settings)(copy_settings,settings)&&
				ADD_OBJECT_TO_LIST(Spectrum_settings)(copy_settings,
					list_of_settings)))
			{
				DESTROY(Spectrum_settings)(&copy_settings);
				display_message(ERROR_MESSAGE,
					"Spectrum_settings_copy_and_put_in_list.  "
					"Could not put copy in list");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Spectrum_settings_copy_and_put_in_list.  Could not create copy");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_settings_copy_and_put_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_copy_and_put_in_list */

int Spectrum_settings_type_matches(struct Spectrum_settings *settings,
	void *settings_type_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns 1 if the settings are of the specified settings_type.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_type_matches);
	/* check arguments */
	if (settings)
	{
		return_code=(settings->settings_type ==
			(enum Spectrum_settings_type)settings_type_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_type_matches.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_type_matches */

int Spectrum_settings_add(struct Spectrum_settings *settings,
	int position,
	struct LIST(Spectrum_settings) *list_of_settings)
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Adds the new_settings in the list_of_settings at the given <priority>
==============================================================================*/
{
	int last_position,return_code;
	struct Spectrum_settings *settings_in_way;

	ENTER(Spectrum_settings_add);
	/* check arguments */
	if (settings&&list_of_settings&&
		!IS_OBJECT_IN_LIST(Spectrum_settings)(settings,list_of_settings))
	{
		return_code=1;
		last_position=NUMBER_IN_LIST(Spectrum_settings)(list_of_settings);
		if ((1>position)||(position>last_position))
		{
			/* add to end of list */
			position=last_position+1;
		}
		ACCESS(Spectrum_settings)(settings);
		while (return_code&&settings)
		{
			settings->position=position;
			/* is there already a settings with that position? */
			if (settings_in_way=FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,
				position)(position,list_of_settings))
			{
				/* remove the old settings to make way for the new */
				ACCESS(Spectrum_settings)(settings_in_way);
				REMOVE_OBJECT_FROM_LIST(Spectrum_settings)(
					settings_in_way,list_of_settings);
			}
			if (ADD_OBJECT_TO_LIST(Spectrum_settings)(settings,list_of_settings))
			{
				DEACCESS(Spectrum_settings)(&settings);
				/* the old, in-the-way settings now become the new settings */
				settings=settings_in_way;
				position++;
			}
			else
			{
				DEACCESS(Spectrum_settings)(&settings);
				if (settings_in_way)
				{
					DEACCESS(Spectrum_settings)(&settings_in_way);
				}
				display_message(ERROR_MESSAGE,"Spectrum_settings_add.  "
					"Could not add settings - settings lost");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_add.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_add */

int Spectrum_settings_remove(struct Spectrum_settings *settings,
	struct LIST(Spectrum_settings) *list_of_settings)
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Removes settings from list_of_settings and decrements the position of
all subsequent settings.
Also sets settings_changed for any other settings affected by its removal.
==============================================================================*/
{
	int return_code,next_position;

	ENTER(Spectrum_settings_remove);
	/* check arguments */
	if (settings&&list_of_settings)
	{
		if (IS_OBJECT_IN_LIST(Spectrum_settings)(settings,list_of_settings))
		{
			next_position=settings->position+1;
			return_code=REMOVE_OBJECT_FROM_LIST(Spectrum_settings)(
				settings,list_of_settings);
			/* decrement position of all remaining settings */
			while (return_code&&(settings=FIND_BY_IDENTIFIER_IN_LIST(
				Spectrum_settings,position)(next_position,list_of_settings)))
			{
				ACCESS(Spectrum_settings)(settings);
				REMOVE_OBJECT_FROM_LIST(Spectrum_settings)(settings,list_of_settings);
				(settings->position)--;
				if (ADD_OBJECT_TO_LIST(Spectrum_settings)(settings,list_of_settings))
				{
					next_position++;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Spectrum_settings_remove_from_list.  "
						"Could not readjust positions - settings lost");
					return_code=0;
				}
				DEACCESS(Spectrum_settings)(&settings);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Spectrum_settings_remove_from_list.  Settings not in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_remove.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_remove */

int Spectrum_settings_modify(struct Spectrum_settings *settings,
	struct Spectrum_settings *new_settings,
	struct LIST(Spectrum_settings) *list_of_settings)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Changes the contents of settings to match new_settings, with no change in
priority. Sets settings->settings_changed to force graphics to be regenerated.
==============================================================================*/
{
	int return_code,old_position;

	ENTER(Spectrum_settings_modify);
	/* check arguments */
	if (settings&&new_settings&&list_of_settings)
	{
		/* make sure graphics for these settings are regenerated */
		settings->settings_changed=1;
		/* make sure position stays the same */
		old_position=settings->position;
		return_code=COPY(Spectrum_settings)(settings,new_settings);
		settings->position=old_position;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_modify.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_modify */

int Spectrum_settings_get_position(
	struct Spectrum_settings *settings,
	struct LIST(Spectrum_settings) *list_of_settings)
/*******************************************************************************
LAST MODIFIED : 24 June 1998

DESCRIPTION :
Returns the position of <settings> in <list_of_settings>.
==============================================================================*/
{
	int position;

	ENTER(Spectrum_settings_get_position);
	if (settings&&list_of_settings)
	{
		if (IS_OBJECT_IN_LIST(Spectrum_settings)(settings,list_of_settings))
		{
			position=settings->position;
		}
		else
		{
			position=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_settings_get_position.  Invalid argument(s)");
		position=0;
	}
	LEAVE;

	return (position);
} /* Spectrum_settings_get_position */

int Spectrum_settings_all_changed(
	struct LIST(Spectrum_settings) *list_of_settings)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Sets the settings->changed flag of all settings in the list.
For use after eg. discretization change.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_all_changed);
	/* check arguments */
	if (list_of_settings)
	{
		if (!(return_code=FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_set_changed,(void *)NULL,list_of_settings)))
		{
			display_message(ERROR_MESSAGE,
				"Spectrum_settings_all_changed.  Error setting changes");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_all_changed.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_all_changed */

int Spectrum_settings_type_changed(
	enum Spectrum_settings_type settings_type,
	struct LIST(Spectrum_settings) *list_of_settings)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Sets the settings->changed flag of all settings of the given type in the list.
For use after eg. discretization change.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_type_changed);
	/* check arguments */
	if (list_of_settings)
	{
		if (!(return_code=FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_changed_if_type,(void *)settings_type,
			list_of_settings)))
		{
			display_message(ERROR_MESSAGE,
				"Spectrum_settings_type_changed.  Error setting changes");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_type_changed.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_type_changed */

int Spectrum_settings_same_space(struct Spectrum_settings *settings,
	void *second_settings_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Spectrum_settings list conditional function returning 1 iff the two
Spectrum_settings describe the same space.
==============================================================================*/
{
	int return_code;
	struct Spectrum_settings *second_settings;

	ENTER(Spectrum_settings_same_space);
	/* check arguments */
	if (settings
		&&(second_settings=(struct Spectrum_settings *)second_settings_void))
	{
		if (settings->settings_type==second_settings->settings_type)
		{
			switch (settings->settings_type)
			{
				default:
				{
					display_message(WARNING_MESSAGE,
						"Spectrum_settings_same_space.  Unknown element settings type");
					return_code=1;
					/*display_message(ERROR_MESSAGE,
						"Spectrum_settings_same_space.  Unknown element settings type");
					return_code=0;*/
				} break;
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_same_space.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_same_space */

char *Spectrum_settings_string(struct Spectrum_settings *settings,
	enum Spectrum_settings_string_details settings_detail)
/*******************************************************************************
LAST MODIFIED : 15 March 2002

DESCRIPTION :
Returns a string describing the settings, suitable for entry into the command
line. Parameter <settings_detail> selects whether appearance settings are
included in the string. User must remember to DEALLOCATE the name afterwards.
==============================================================================*/
{
	char *settings_string,temp_string[80];
	int error;

	ENTER(Spectrum_settings_string);
	settings_string=(char *)NULL;
	error=0;
	/* check arguments */
	if (settings&&(
		(SPECTRUM_SETTINGS_STRING_SPACE_ONLY==settings_detail)||
		(SPECTRUM_SETTINGS_STRING_COMPLETE==settings_detail)||
		(SPECTRUM_SETTINGS_STRING_COMPLETE_PLUS==settings_detail)))
	{
		if (SPECTRUM_SETTINGS_STRING_COMPLETE_PLUS==settings_detail)
		{
			sprintf(temp_string,"%i. ",settings->position);
			append_string(&settings_string,temp_string,&error);
		}
		switch (settings->settings_type)
		{
			case SPECTRUM_LINEAR:
			{
				append_string(&settings_string,"linear",&error);
			} break;
			case SPECTRUM_LOG:
			{
				sprintf(temp_string,"log exaggeration %g",fabs(settings->exaggeration));
				append_string(&settings_string,temp_string,&error);
				if (settings->exaggeration >= 0)
				{
					append_string(&settings_string," left",&error);
				}
				else
				{
					append_string(&settings_string," right",&error);
				}					
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_settings_string.  Unknown element settings type");
			} break;
		}
		if ( settings->reverse )
		{
			append_string(&settings_string," reverse",&error);
		}
		sprintf(temp_string," range %g %g",settings->minimum,
			settings->maximum);
		append_string(&settings_string,temp_string,&error);
		if ((settings->extend_above)&&(settings->settings_type!=SPECTRUM_STEP))
		{
			append_string(&settings_string," extend_above",&error);
		}
		if ((settings->extend_below)&&(settings->settings_type!=SPECTRUM_STEP))
		{
			append_string(&settings_string," extend_below",&error);
		}
		if (settings->fix_maximum)
		{
			append_string(&settings_string," fix_maximum",&error);
		}
		if (settings->fix_minimum)
		{
			append_string(&settings_string," fix_minimum",&error);
		}
		if (settings->settings_type == SPECTRUM_LINEAR ||
			settings->settings_type == SPECTRUM_LOG )
		{
			switch (settings->colour_mapping)
			{
				case SPECTRUM_RAINBOW:
				{
					sprintf(temp_string," rainbow colour_range %g %g",
						settings->min_value, settings->max_value);
					append_string(&settings_string,temp_string,&error);	
				} break;
				case SPECTRUM_RED:
				{
					sprintf(temp_string," red colour_range %g %g",
						settings->min_value, settings->max_value);
					append_string(&settings_string,temp_string,&error);	
				} break;
				case SPECTRUM_GREEN:
				{
					sprintf(temp_string," green colour_range %g %g",
						settings->min_value, settings->max_value);
					append_string(&settings_string,temp_string,&error);	
				} break;
				case SPECTRUM_BLUE:
				{
					sprintf(temp_string," blue colour_range %g %g",
						settings->min_value, settings->max_value);
					append_string(&settings_string,temp_string,&error);	
				} break;
				case SPECTRUM_ALPHA:
				{
					sprintf(temp_string," alpha colour_range %g %g",
						settings->min_value, settings->max_value);
					append_string(&settings_string,temp_string,&error);	
				} break;
				case SPECTRUM_BANDED:
				{
					sprintf(temp_string," banded number_of_bands %d band_ratio %g",
						settings->number_of_bands,
						(float)(settings->black_band_proportion)/1000.0);
					append_string(&settings_string,temp_string,&error);
				} break;
				case SPECTRUM_STEP:
				{
					sprintf(temp_string," step_texture step_value %g",settings->step_value);
					append_string(&settings_string,temp_string,&error);
				} break;	
				case SPECTRUM_WHITE_TO_BLUE:
				{
					sprintf(temp_string," white_to_blue colour_range %g %g",
						settings->min_value, settings->max_value);
					append_string(&settings_string,temp_string,&error);	
				} break;	
				case SPECTRUM_WHITE_TO_RED:
				{
					sprintf(temp_string," white_to_red colour_range %g %g",
						settings->min_value, settings->max_value);
					append_string(&settings_string,temp_string,&error);	
				} break;
			}
		}
		if ((SPECTRUM_ALPHA!=settings->colour_mapping)&& 
			(SPECTRUM_BANDED!=settings->colour_mapping)&&
			(SPECTRUM_STEP!=settings->colour_mapping))
		{
			switch (settings->render_type)
			{
				case SPECTRUM_DIFFUSE:
				{
					append_string(&settings_string," diffuse",&error);
				} break;
				case SPECTRUM_AMBIENT:
				{
					append_string(&settings_string," ambient",&error);
				} break;
				case SPECTRUM_EMISSION:
				{
					append_string(&settings_string," emission",&error);
				} break;
				case SPECTRUM_SPECULAR:
				{
					append_string(&settings_string," specular",&error);
				} break;
				case SPECTRUM_AMBIENT_AND_DIFFUSE:
				{
					append_string(&settings_string," ambient diffuse",&error);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Spectrum_settings_string.  Unknown data render type");
				} break;
			}
		}
		sprintf(temp_string," component %d",settings->component_number + 1);
		append_string(&settings_string,temp_string,&error);	
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_string.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return settings_string;
} /* Spectrum_settings_string */

int Spectrum_settings_show(struct Spectrum_settings *settings,
	void *settings_detail_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Writes out the settings as a text string.
==============================================================================*/
{
	int return_code;
	char *settings_string,line[40];
	enum Spectrum_settings_string_details settings_detail;

	ENTER(Spectrum_settings_show);
	settings_detail=(enum Spectrum_settings_string_details)settings_detail_void;
	/* check arguments */
	if (settings&&(
		(SPECTRUM_SETTINGS_STRING_SPACE_ONLY==settings_detail)||
		(SPECTRUM_SETTINGS_STRING_COMPLETE==settings_detail)||
		(SPECTRUM_SETTINGS_STRING_COMPLETE_PLUS==settings_detail)))
	{
		if (settings_string=Spectrum_settings_string(settings,settings_detail))
		{
			sprintf(line,"%4i.",settings->position);
			display_message(INFORMATION_MESSAGE,line);
			display_message(INFORMATION_MESSAGE,settings_string);
			/*???RC temp */
			if (settings->access_count != 1)
			{
				sprintf(line," (access count = %i)",settings->access_count);
				display_message(INFORMATION_MESSAGE,line);
			}
			display_message(INFORMATION_MESSAGE,"\n");
			DEALLOCATE(settings_string);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_show.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_show */

enum Spectrum_settings_type Spectrum_settings_get_type(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the type of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	enum Spectrum_settings_type type;

	ENTER(Spectrum_settings_get_type);

	if (settings)
	{
		type = settings->settings_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_type.  "
			"Invalid argument(s)");
		type = SPECTRUM_INVALID_TYPE;
	}
	LEAVE;

	return (type);
} /* Spectrum_settings_get_type */

int Spectrum_settings_set_type(struct Spectrum_settings *settings,
	enum Spectrum_settings_type type)
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Sets the type of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_get_type);

	if (settings)
	{
		settings->settings_type = type;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_type.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Spectrum_settings_get_component_number(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
Returns the component_number of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	int component_number;

	ENTER(Spectrum_settings_get_component_number);

	if (settings)
	{
		component_number = settings->component_number + 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_component_number.  "
			"Invalid argument(s)");
		component_number = 0;
	}
	LEAVE;

	return (component_number);
} /* Spectrum_settings_get_component_number */

int Spectrum_settings_set_component_number(struct Spectrum_settings *settings,
	int component_number)
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
Sets the component_number of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_component_number);

	if (settings)
	{
		settings->component_number = component_number - 1;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_component_number.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_component_number */

int Spectrum_settings_get_reverse_flag(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Returns the reverse flag of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	int reverse;

	ENTER(Spectrum_settings_get_reverse_flag);

	if (settings)
	{
		reverse = settings->reverse;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_reverse_flag.  "
			"Invalid argument(s)");
		reverse = 0;
	}
	LEAVE;

	return (reverse);
} /* Spectrum_settings_get_reverse_flag */

int Spectrum_settings_set_reverse_flag(struct Spectrum_settings *settings,
	int reverse)
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Sets the reverse flag of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_reverse_flag);

	if (settings)
	{
		settings->reverse = reverse;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_reverse_flag.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_reverse_flag */

enum Spectrum_settings_colour_mapping Spectrum_settings_get_colour_mapping(
	struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Returns the colour mapping of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	enum Spectrum_settings_colour_mapping type;

	ENTER(Spectrum_settings_get_colour_mapping);

	if (settings)
	{
		type = settings->colour_mapping;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_colour_mapping.  "
			"Invalid argument(s)");
		type = SPECTRUM_RAINBOW;
	}
	LEAVE;

	return (type);
} /* Spectrum_settings_get_colour_mapping */

int Spectrum_settings_set_colour_mapping(struct Spectrum_settings *settings,
	enum Spectrum_settings_colour_mapping type)
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Sets the colour mapping of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_colour_mapping);

	if (settings)
	{
		settings->colour_mapping = type;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_colour_mapping.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_colour_mapping */

float Spectrum_settings_get_exaggeration(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Returns the first type parameter of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	float param1;

	ENTER(Spectrum_settings_get_exaggeration);

	if (settings)
	{
		param1 = settings->exaggeration;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_exaggeration.  "
			"Invalid argument(s)");
		param1 = 0;
	}
	LEAVE;

	return (param1);
} /* Spectrum_settings_get_exaggeration */

int Spectrum_settings_set_exaggeration(struct Spectrum_settings *settings,
	float param1)
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Sets the first type parameter of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_colour_mapping);

	if (settings)
	{
		settings->exaggeration = param1;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_exaggeration.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_exaggeration */

int Spectrum_settings_get_number_of_bands(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
==============================================================================*/
{
	int bands;

	ENTER(Spectrum_settings_get_number_of_bands);

	if (settings)
	{
		bands = settings->number_of_bands;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_number_of_bands.  "
			"Invalid argument(s)");
		bands = 0;
	}
	LEAVE;

	return (bands);
} /* Spectrum_settings_get_number_of_bands */

int Spectrum_settings_set_number_of_bands(struct Spectrum_settings *settings,
	int bands)
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
==============================================================================*/

{
	int return_code;

	ENTER(Spectrum_settings_set_number_of_bands);

	if (settings)
	{
		settings->number_of_bands = bands;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_number_of_bands.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_number_of_bands */

int Spectrum_settings_get_black_band_proportion(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
==============================================================================*/
{
	int proportion;

	ENTER(Spectrum_settings_get_black_band_proportion);

	if (settings)
	{
		proportion = settings->black_band_proportion;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_black_band_proportion.  "
			"Invalid argument(s)");
		proportion = 0;
	}
	LEAVE;

	return (proportion);
} /* Spectrum_settings_get_black_band_proportion */

int Spectrum_settings_set_black_band_proportion(struct Spectrum_settings *settings,
	int proportion)
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_black_band_proportion);

	if (settings)
	{
		settings->black_band_proportion = proportion;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_black_band_proportion.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_black_band_proportion */

float Spectrum_settings_get_step_value(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
Returns the step value parameter of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	float param1;

	ENTER(Spectrum_settings_get_step_value);

	if (settings)
	{
		param1 = settings->step_value;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_step_value.  "
			"Invalid argument(s)");
		param1 = 0;
	}
	LEAVE;

	return (param1);
} /* Spectrum_settings_get_step_value */

int Spectrum_settings_set_step_value(struct Spectrum_settings *settings,
	float param1)
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
Sets the step value of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_colour_mapping);

	if (settings)
	{
		settings->step_value = param1;
		if ( settings->step_value <= settings->minimum 
			|| settings->step_value >= settings->maximum )
		{
			settings->step_value = 0.5 * (settings->maximum + settings->minimum );
		}
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_step_value.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_step_value */

float Spectrum_settings_get_range_minimum(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
==============================================================================*/
{
	float value;

	ENTER(Spectrum_settings_get_range_minimum);

	if (settings)
	{
		value = settings->minimum;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_range_minimum.  "
			"Invalid argument(s)");
		value = 0;
	}
	LEAVE;

	return (value);
} /* Spectrum_settings_get_range_minimum */

int Spectrum_settings_set_range_minimum(struct Spectrum_settings *settings,
	float value)
/*******************************************************************************
LAST MODIFIED : 15 January 2001

DESCRIPTION :
If <settings> ->fix_minimum is NOT set, set <settings> ->minimum to <value>
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_range_minimum);

	if (settings)
	{
		if(!settings->fix_minimum)
		{
			settings->minimum = value;
			if ( settings->step_value <= settings->minimum 
				|| settings->step_value >= settings->maximum )
			{
				settings->step_value = 0.5 * (settings->maximum + settings->minimum );
			}
			settings->settings_changed = 1;
		}		
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_range_minimum.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_range_minimum */

float Spectrum_settings_get_range_maximum(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
==============================================================================*/
{
	float value;

	ENTER(Spectrum_settings_get_range_max);

	if (settings)
	{
		value = settings->maximum;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_range_maximum.  "
			"Invalid argument(s)");
		value = 0;
	}
	LEAVE;

	return (value);
} /* Spectrum_settings_get_range_maximum */

int Spectrum_settings_set_range_maximum(struct Spectrum_settings *settings,
	float value)
/*******************************************************************************
LAST MODIFIED : 15 January 20001

DESCRIPTION : 
If <settings> ->fix_maximum is NOT set, set <settings> ->maximum to <value>
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_colour_mapping);

	if (settings)
	{
		if(!settings->fix_maximum)
		{
			settings->maximum = value;
			if ( settings->step_value <= settings->minimum 
				|| settings->step_value >= settings->maximum )
			{
				settings->step_value = 0.5 * (settings->maximum + settings->minimum );
			}
			settings->settings_changed = 1;
		}		
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_range_maximum.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_range_maximum */

int Spectrum_settings_get_extend_above_flag(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Returns the extend_above flag of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	int extend_above;

	ENTER(Spectrum_settings_get_colour_mapping);

	if (settings)
	{
		extend_above = settings->extend_above;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_extend_above_flag.  "
			"Invalid argument(s)");
		extend_above = 0;
	}
	LEAVE;

	return (extend_above);
} /* Spectrum_settings_get_extend_above_flag */

int Spectrum_settings_set_extend_above_flag(struct Spectrum_settings *settings,
	int extend_above)
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Sets the extend_above flag of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_extend_above_flag);

	if (settings)
	{
		settings->extend_above = extend_above;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_extend_above_flag.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_extend_above_flag */


int Spectrum_settings_get_extend_below_flag(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Returns the extend_below flag of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	int extend_below;

	ENTER(Spectrum_settings_get_colour_mapping);

	if (settings)
	{
		extend_below = settings->extend_below;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_extend_below_flag.  "
			"Invalid argument(s)");
		extend_below = 0;
	}
	LEAVE;

	return (extend_below);
} /* Spectrum_settings_get_extend_below_flag */

int Spectrum_settings_set_extend_below_flag(struct Spectrum_settings *settings,
	int extend_below)
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Sets the extend_below flag of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_extend_below_flag);

	if (settings)
	{
		settings->extend_below = extend_below;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_extend_below_flag.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_extend_below_flag */

int Spectrum_settings_get_fix_minimum_flag(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Returns the fix_minimum flag of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	int fix_minimum;

	ENTER(Spectrum_settings_get_colour_mapping);

	if (settings)
	{
		fix_minimum = settings->fix_minimum;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_fix_minimum_flag.  "
			"Invalid argument(s)");
		fix_minimum = 0;
	}
	LEAVE;

	return (fix_minimum);
} /* Spectrum_settings_get_fix_minimum_flag */

int Spectrum_settings_set_fix_minimum_flag(struct Spectrum_settings *settings,
	int fix_minimum)
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Sets the fix_minimum flag of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_fix_minimum_flag);

	if (settings)
	{
		settings->fix_minimum = fix_minimum;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_fix_minimum_flag.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_fix_minimum_flag */

int Spectrum_settings_get_fix_maximum_flag(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Returns the fix_maximum flag of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	int fix_maximum;

	ENTER(Spectrum_settings_get_colour_mapping);

	if (settings)
	{
		fix_maximum = settings->fix_maximum;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_fix_maximum_flag.  "
			"Invalid argument(s)");
		fix_maximum = 0;
	}
	LEAVE;

	return (fix_maximum);
} /* Spectrum_settings_get_fix_maximum_flag */

int Spectrum_settings_set_fix_maximum_flag(struct Spectrum_settings *settings,
	int fix_maximum)
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Sets the fix_maximum flag of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_fix_maximum_flag);

	if (settings)
	{
		settings->fix_maximum = fix_maximum;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_fix_maximum_flag.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_fix_maximum_flag */

float Spectrum_settings_get_colour_value_minimum(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
==============================================================================*/
{
	float value;

	ENTER(Spectrum_settings_get_colour_value_minimum);

	if (settings)
	{
		value = settings->min_value;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_colour_value_minimum.  "
			"Invalid argument(s)");
		value = 0;
	}
	LEAVE;

	return (value);
} /* Spectrum_settings_get_colour_value_minimum */

int Spectrum_settings_set_colour_value_minimum(struct Spectrum_settings *settings,
	float value)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_colour_value_minimum);

	if (settings && value <= 1.0 && value >= 0.0)
	{
		settings->min_value = value;
		if (value > settings->max_value)
		{
			settings->max_value = value;
		}
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_colour_value_minimum.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_colour_value_minimum */

float Spectrum_settings_get_colour_value_maximum(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
==============================================================================*/
{
	float value;

	ENTER(Spectrum_settings_get_colour_value_max);

	if (settings)
	{
		value = settings->max_value;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_colour_value_maximum.  "
			"Invalid argument(s)");
		value = 0;
	}
	LEAVE;

	return (value);
} /* Spectrum_settings_get_colour_value_maximum */

int Spectrum_settings_set_colour_value_maximum(struct Spectrum_settings *settings,
	float value)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_colour_mapping);

	if (settings && value <= 1.0 && value >= 0.0)
	{
		settings->max_value = value;
		if (value < settings->min_value)
		{
			settings->min_value = value;
		}
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_colour_value_maximum.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_colour_value_maximum */

enum Spectrum_settings_render_type Spectrum_settings_get_render_type(struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 13 July 1998

DESCRIPTION :
Returns the type of the Spectrum_settings <spectrum>.
==============================================================================*/
{
	enum Spectrum_settings_render_type type;

	ENTER(Spectrum_settings_get_render_type);

	if (settings)
	{
		type = settings->render_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_get_render_type.  "
			"Invalid argument(s)");
		type = SPECTRUM_AMBIENT_AND_DIFFUSE;
	}
	LEAVE;

	return (type);
} /* Spectrum_settings_get_render_type */

int Spectrum_settings_set_render_type(struct Spectrum_settings *settings,
	enum Spectrum_settings_render_type type)
/*******************************************************************************
LAST MODIFIED : 13 July 1998

DESCRIPTION :
Sets the type of the Spectrum_settings <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_set_render_type);

	if (settings)
	{
		settings->render_type = type;
		settings->settings_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_set_render_type.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_set_render_type */

int Spectrum_settings_clear_settings_changed(
	struct Spectrum_settings *settings,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Iterator function to set settings->settings_changed to 0 (unchanged).
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_settings_clear_settings_changed);
	USE_PARAMETER(dummy_void);
	/* check arguments */
	if (settings)
	{
		settings->settings_changed=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_settings_clear_settings_changed.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_clear_settings_changed */

int Spectrum_settings_enable(struct Spectrum_settings *settings,
	void *render_data_void)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
==============================================================================*/
{
#if defined (OPENGL_API)
	unsigned char pixels[3*1024];
	int high_flag,low_flag,texel,texels_in_band,texels_per_band,i;
#endif /* defined (OPENGL_API) */
	int return_code;
	struct Spectrum_render_data *render_data;

	ENTER(Spectrum_settings_enable);
	/* check arguments */
	if (settings&&(render_data=(struct Spectrum_render_data *)render_data_void))
	{
		return_code=1;
		if (settings->active)
		{
			switch (settings->colour_mapping)
			{
				case SPECTRUM_RAINBOW:
				case SPECTRUM_RED:
				case SPECTRUM_GREEN:
				case SPECTRUM_BLUE:	
				case SPECTRUM_WHITE_TO_BLUE:
				case SPECTRUM_WHITE_TO_RED:
				{
					render_data->rendering_flags |= settings->render_type;
				} break;
				case SPECTRUM_ALPHA:
				{
					render_data->rendering_flags |= SPECTRUM_DIFFUSE;
				} break;
				case SPECTRUM_BANDED:
				{
#if defined (OPENGL_API)
					if ((settings->black_band_proportion)&&(settings->number_of_bands))
					{
						if ((settings->black_band_proportion)%(settings->number_of_bands))
						{
#if defined (DEBUG)
							printf("  proportion %d number %d >>", 
								settings->black_band_proportion,settings->number_of_bands);
#endif /* defined (DEBUG) */
							settings->black_band_proportion += settings->number_of_bands-
								settings->black_band_proportion%settings->number_of_bands;
#if defined (DEBUG)
							printf("%d\n",settings->black_band_proportion);
#endif /* defined (DEBUG) */
						}
						texels_in_band=(settings->black_band_proportion)/
							(settings->number_of_bands);
						texels_per_band=1021/(settings->number_of_bands);
					
						/* the first and last texel are white to allow transparency
							outside the band range */
						i=0;
						pixels[i]=255;
						i++;
						pixels[i]=255;
						i++;
						pixels[i]=255;
						i++;
						while (i<3*1023)
						{
							texel = (i-3)/3+(texels_in_band/2);
							if (settings->reverse)
							{
								low_flag = settings->extend_above;
								high_flag = settings->extend_below;
							}
							else
							{
								low_flag = settings->extend_below;
								high_flag = settings->extend_above;
							}
							if ((low_flag || texel>texels_in_band) && (high_flag || texel<1020) &&
								((texel%texels_per_band)<texels_in_band))
							{
#if defined (DEBUG)
								printf("  band pixel %d texel %d\n",i/3, texel);
#endif /* defined (DEBUG) */
								pixels[i]=0;
								i++;
								pixels[i]=0;
								i++;
								pixels[i]=0;
								i++;
							}
							else
							{
								pixels[i]=255;
								i++;
								pixels[i]=255;
								i++;
								pixels[i]=255;
								i++;
							}
						}
						pixels[i]=255;
						i++;
						pixels[i]=255;
						i++;
						pixels[i]=255;
						i++;
					
#if defined (DEBUG)
						while (error=glGetError())
						{
							printf("GL ERROR 0: %s\n",gluErrorString(error));
						}
#endif /* defined (DEBUG) */
						if (!settings->texture_id)
						{
							glGenTextures(1, &(settings->texture_id));
						}
						if (settings->settings_changed)
						{
							glBindTexture(GL_TEXTURE_1D, settings->texture_id);
							glTexImage1D(GL_TEXTURE_1D,0,3,1024,0,GL_RGB,GL_UNSIGNED_BYTE,
								pixels);
							glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
							glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP);
							glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
							glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
						}
						else
						{
							glBindTexture(GL_TEXTURE_1D, settings->texture_id);
						}
						glEnable(GL_TEXTURE_1D);
#if defined (OLD_CODE) /* ! defined (GL_EXT_texture_object) */				
						glTexImage1D(GL_TEXTURE_1D,0,3,1024,0,GL_RGB,GL_UNSIGNED_BYTE,
							pixels);
						glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
						glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP);
						glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
						glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
						glEnable(GL_TEXTURE_1D);
#endif /* defined (OLD_CODE) */				
#if defined (DEBUG)
						while (error=glGetError())
						{
							printf("GL ERROR 1: %s\n",gluErrorString(error));
						}
#endif /* defined (DEBUG) */
					}
					else
					{
						if (!settings->number_of_bands)
						{
							display_message(ERROR_MESSAGE,
								"Spectrum_settings_enable.  Invalid number_of_bands");
							return_code=0;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Spectrum_settings_enable.  Invalid band_proportion");
							return_code=0;
						}
					}
#endif /* defined (OPENGL_API) */
				} break;
				case SPECTRUM_STEP:
				{
#if defined (OPENGL_API)
					pixels[0]=255;
					pixels[1]=0;
					pixels[2]=0;
					pixels[3]=0;
					pixels[4]=255;
					pixels[5]=0;
					/* set up a texture */
#if defined (DEBUG)
					while (error=glGetError())
					{
						printf("GL ERROR 0: %s\n",gluErrorString(error));
					}
#endif /* defined (DEBUG) */
					if (!settings->texture_id)
					{
						glGenTextures(1, &(settings->texture_id));
					}
					if (settings->settings_changed)
					{
						glBindTexture(GL_TEXTURE_1D, settings->texture_id);
						glTexImage1D(GL_TEXTURE_1D,0,3,2,0,GL_RGB,GL_UNSIGNED_BYTE,pixels);
						glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
						glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP);
						glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
						glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
					}
					else
					{
						glBindTexture(GL_TEXTURE_1D, settings->texture_id);
					}
					glEnable(GL_TEXTURE_1D);
#if defined (OLD_CODE) /* ! defined (GL_EXT_texture_object) */				
					glTexImage1D(GL_TEXTURE_1D,0,3,2,0,GL_RGB,GL_UNSIGNED_BYTE,pixels);
					glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
					glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP);
					glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
					glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
					glEnable(GL_TEXTURE_1D);
#endif /* defined (OLD_CODE) */				
#if defined (DEBUG)
					while (error=glGetError())
					{
						printf("GL ERROR 1: %s\n",gluErrorString(error));
					}
#endif /* defined (DEBUG) */
#endif /* defined (OPENGL_API) */
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Spectrum_settings_enable.  Unknown colour mapping");
					return_code=0;
				} break;
			}
		}
		settings->settings_changed = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_enable.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_enable */

int Spectrum_settings_activate(struct Spectrum_settings *settings,
	void *render_data_void)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Modifies the material in the render data to represent the data value
passed in render data.
==============================================================================*/
{
	int i,number_of_colours,return_code,texels_per_band;
	float data_component,value,step_xi,total_texels;
	struct Colour *colour,colours[2];
	struct Spectrum_render_data *render_data;

	ENTER(Spectrum_settings_activate);
	/* check arguments */
	if (settings&&(render_data=(struct Spectrum_render_data *)render_data_void))
	{
		return_code=1;
		/* Ignore inactive settings or settings which act on a component for which
			there is no data */
		if (settings->active && 
			(settings->component_number < render_data->number_of_data_components))
		{
			data_component = render_data->data[settings->component_number];
			/* Always set a value for texture_coordinate based spectrums */
			if ((SPECTRUM_BANDED==settings->colour_mapping)
				|| (SPECTRUM_STEP==settings->colour_mapping)
				|| (((data_component>=settings->minimum)||settings->extend_below)&&
				((data_component<=settings->maximum)||settings->extend_above)))
			{
				/* first get value (normalised 0 to 1) from type */
				if (settings->maximum != settings->minimum)
				{
					switch (settings->settings_type)
					{
						case SPECTRUM_LINEAR:
						{
							value=(data_component-settings->minimum)/
								(settings->maximum-settings->minimum);
						} break;
						case SPECTRUM_LOG:
						{
							if (settings->exaggeration<0)
							{
								value=1.0-log(1-settings->exaggeration*
									(settings->maximum-data_component)/
									(settings->maximum-settings->minimum))/
									log(1-settings->exaggeration);
							}
							else
							{
								value=log(1+settings->exaggeration*
									(data_component-settings->minimum)/
									(settings->maximum-settings->minimum))/
									log(1+settings->exaggeration);
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Spectrum_settings_activate.  Unknown type");
							return_code=0;
						} break;
					}
					/* ensure 0 - 1 */
					if (value>1.0)
					{
						value=1.0;
					}
					if (value<0.0)
					{
						value=0.0;
					}
				}
				else
				{
					if (data_component <= settings->minimum)
					{
						value = 0.0;
					}
					else
					{
						value = 1.0;
					}
				}
				/* reverse the direction if necessary */
				if (settings->reverse)
				{
					value=1.0-value;
				}
				/* apply the value minimums and maximums */
				value=settings->min_value+(settings->max_value-settings->min_value)*
					value;
				switch (settings->colour_mapping)
				{
					case SPECTRUM_BANDED:
					{
						if ((settings->number_of_bands)&&(settings->black_band_proportion))
						{
							texels_per_band=1021/(settings->number_of_bands);
							total_texels=(float)(texels_per_band*settings->number_of_bands);
							if ((settings->black_band_proportion/settings->number_of_bands)%2)
							{
								value=((value*total_texels+1.5)/1024.0);
							}
							else
							{
								value=((value*total_texels+1.0)/1024.0);
							}
#if defined (OPENGL_API)
							glTexCoord1f(value);
#endif /* defined (OPENGL_API) */
						}
					} break;
					case SPECTRUM_STEP:
					{
						step_xi=(settings->step_value-settings->minimum)/
							(settings->maximum-settings->minimum);
						if ((0.0==step_xi)||(1.0==step_xi))
						{
							step_xi=0.5;
						}
						if (settings->reverse)
						{
							step_xi=1.0-step_xi;
						}
						value=0.5*value*(1.0-value)/(step_xi*(1.0-step_xi))+
							value*(value-step_xi)/(1.0-step_xi);
#if defined (OPENGL_API)
						glTexCoord1f(value);
#endif /* defined (OPENGL_API) */
					} break;
					case SPECTRUM_ALPHA:
					{
						Graphical_material_set_alpha(render_data->material,value);
					} break;
					case SPECTRUM_RAINBOW:
					case SPECTRUM_RED:
					case SPECTRUM_GREEN:
					case SPECTRUM_BLUE:	
					case SPECTRUM_WHITE_TO_BLUE:
					case SPECTRUM_WHITE_TO_RED:
					{
						number_of_colours=1;
						/* get current colour */
						switch (settings->render_type)
						{
							case SPECTRUM_DIFFUSE:
							{
								Graphical_material_get_diffuse(render_data->material,colours);
							} break;
							case SPECTRUM_AMBIENT:
							{
								Graphical_material_get_ambient(render_data->material,colours);
							} break;
							case SPECTRUM_EMISSION:
							{
								Graphical_material_get_emission(render_data->material,colours);
							} break;
							case SPECTRUM_SPECULAR:
							{
								Graphical_material_get_specular(render_data->material,colours);
							} break;
							case SPECTRUM_AMBIENT_AND_DIFFUSE:
							{
								number_of_colours=2;
								Graphical_material_get_ambient(render_data->material,colours);
								Graphical_material_get_diffuse(render_data->material,colours+1);
							} break;	
						}
						/* modify colours according to value */
						for (i=0;i<number_of_colours;i++)
						{
							colour=colours+i;
							switch (settings->colour_mapping)
							{
								case SPECTRUM_RAINBOW:
								{
									if (value<1.0/3.0)
									{
										colour->red=1.0;
										colour->blue=0.0;
										if (value<1.0/6.0)
										{
											colour->green=value*4.5;
										}
										else
										{
											colour->green=0.75+(value-1.0/6.0)*1.5;
										}
									}
									else
									{
										if (value<2.0/3.0)
										{
											colour->red=(2.0/3.0-value)*3.0;
											colour->green=1.0;
											colour->blue=(value-1.0/3.0)*3.0;
										}
										else
										{
											colour->red=0.0;
											colour->blue=1.0;
											if (value<5.0/6.0)
											{
												colour->green=1.0-(value-2.0/3.0)*1.5;
											}
											else
											{
												colour->green=0.75-(value-5.0/6.0)*4.5;
											}
										}
									}
								} break;
								case SPECTRUM_RED:
								{
									colour->red=value;
								} break;
								case SPECTRUM_GREEN:
								{
									colour->green=value;
								} break;
								case SPECTRUM_BLUE:
								{
									colour->blue=value;
								} break;
							
								case SPECTRUM_WHITE_TO_BLUE:
								{
									colour->blue=1.0;
									colour->red=(1-value);
									colour->green=(1-value);
								} break;	
								case SPECTRUM_WHITE_TO_RED:
								{
									colour->red=1;
									colour->blue=(1-value);
									colour->green=(1-value);
								} break;
							}
						}
						/* apply spectrum to material */
						switch (settings->render_type)
						{
							case SPECTRUM_DIFFUSE:
							{
								Graphical_material_set_diffuse(render_data->material,colours);
							} break;
							case SPECTRUM_AMBIENT:
							{
								Graphical_material_set_ambient(render_data->material,colours);
							} break;
							case SPECTRUM_EMISSION:
							{
								Graphical_material_set_emission(render_data->material,colours);
							} break;
							case SPECTRUM_SPECULAR:
							{
								Graphical_material_set_specular(render_data->material,colours);
							} break;
							case SPECTRUM_AMBIENT_AND_DIFFUSE:
							{
								Graphical_material_set_ambient(render_data->material,colours);
								Graphical_material_set_diffuse(render_data->material,colours+1);
							} break;
						}
					} break;
				}
			}
			else
			{
				switch (settings->settings_type)
				{
					case SPECTRUM_BANDED:
					case SPECTRUM_STEP:
					{
						/* the values are large so they quickly transition to the last
							texel */
						if (data_component>settings->maximum)
						{
							value=1000.0;
						}
						else
						{
							value= -999.0;
						}
#if defined (OPENGL_API)
						glTexCoord1f(value);
#endif /* defined (OPENGL_API) */
					} break;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_activate.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_activate */

int Spectrum_settings_disable(struct Spectrum_settings *settings,
	void *render_data_void)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Spectrum_render_data *render_data;

	ENTER(Spectrum_settings_disable);
	/* check arguments */
	if (settings&&(render_data=(struct Spectrum_render_data *)render_data_void))
	{
		USE_PARAMETER(render_data);
		return_code=1;
		if (settings->active)
		{
			switch (settings->colour_mapping)
			{
				case SPECTRUM_RAINBOW:
				case SPECTRUM_RED:
				case SPECTRUM_GREEN:
				case SPECTRUM_BLUE:
				case SPECTRUM_ALPHA:
				case SPECTRUM_WHITE_TO_BLUE:
				case SPECTRUM_WHITE_TO_RED:
				{
					/* do nothing */
				} break;
				case SPECTRUM_BANDED:
				case SPECTRUM_STEP:
				{
#if defined (OPENGL_API)
					glDisable(GL_TEXTURE_1D);
#endif /* defined (OPENGL_API) */
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Spectrum_settings_disable.  Unknown type");
					return_code = 0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_settings_disable.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_disable */

int Spectrum_settings_list_contents(struct Spectrum_settings *settings,
	void *list_data_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Writes out the <settings> as a text string in the command window with the
<settings_string_detail>, <line_prefix> and <line_suffix> given in the
<list_data>.
==============================================================================*/
{
	int return_code;
	char *settings_string,line[80];
	struct Spectrum_settings_list_data *list_data;

	ENTER(Spectrum_settings_list_contents);
	if (settings&&
		(list_data=(struct Spectrum_settings_list_data *)list_data_void))
	{
		if (settings_string=Spectrum_settings_string(settings,
			list_data->settings_string_detail))
		{
			if (list_data->line_prefix)
			{
				display_message(INFORMATION_MESSAGE,list_data->line_prefix);
			}
			display_message(INFORMATION_MESSAGE,settings_string);
			if (list_data->line_suffix)
			{
				display_message(INFORMATION_MESSAGE,list_data->line_suffix);
			}
			/*???RC temp */
			if ((SPECTRUM_SETTINGS_STRING_COMPLETE_PLUS==list_data->settings_string_detail)&&
				(settings->access_count != 1))
			{
				sprintf(line," (access count = %i)",settings->access_count);
				display_message(INFORMATION_MESSAGE,line);
			}
			display_message(INFORMATION_MESSAGE,";\n");
			DEALLOCATE(settings_string);
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_settings_list_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_settings_list_contents */

int gfx_modify_spectrum_settings_linear(struct Parse_state *state,
	void *modify_spectrum_data_void,void *spectrum_command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LINEAR command.
If return_code is 1, returns the completed Modify_spectrum_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char alpha,ambient,amb_diff,banded,blue,diffuse,emission,extend_above,
		extend_below,fix_maximum,fix_minimum,green, rainbow,white_to_blue,
		white_to_red,red,reverse,specular,step,transparent_above,transparent_below;
	int black_band_int,component,number_of_bands,range_components,return_code;
	float band_ratio,colour_range[2],step_value,range[2];
	struct Modify_spectrum_data *modify_spectrum_data;
	struct Spectrum_settings *settings;
	static struct Modifier_entry
		colour_option_table[]=
	  {
			{"alpha",NULL,NULL,set_char_flag},
			{"banded",NULL,NULL,set_char_flag},
			{"blue",NULL,NULL,set_char_flag},			
			{"green",NULL,NULL,set_char_flag},
			{"rainbow",NULL,NULL,set_char_flag},
			{"red",NULL,NULL,set_char_flag},			
			{"step_texture",NULL,NULL,set_char_flag},
			{"white_to_blue",NULL,NULL,set_char_flag},
			{"white_to_red",NULL,NULL,set_char_flag},					
			{NULL,NULL,NULL,NULL}			
		},
		render_option_table[]=
	  {
			{"ambient",NULL,NULL,set_char_flag},
			{"amb_diff",NULL,NULL,set_char_flag},
			{"diffuse",NULL,NULL,set_char_flag},
			{"emission",NULL,NULL,set_char_flag},
			{"specular",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"band_ratio",NULL,NULL,set_float},
			{"colour_range",NULL,NULL,set_float_vector},
			{"component",NULL,NULL,set_int_positive},
			{"extend_above",NULL,NULL,set_char_flag},
			{"extend_below",NULL,NULL,set_char_flag},
			{"fix_maximum",NULL,NULL,set_char_flag},
			{"fix_minimum",NULL,NULL,set_char_flag},
			{"number_of_bands",NULL,NULL,set_int_positive},
			{"position",NULL,NULL,set_int_non_negative},
			{"range",NULL,NULL,set_float_vector},
			{"reverse",NULL,NULL,set_char_flag},
			{"step_value",NULL,NULL,set_float},
			{"transparent_above",NULL,NULL,set_char_flag},
			{"transparent_below",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL}
		};

	ENTER(gfx_modify_spectrum_settings_linear);
	USE_PARAMETER(spectrum_command_data_void);
	if (state)
	{
		if (modify_spectrum_data=
			(struct Modify_spectrum_data *)modify_spectrum_data_void)
		{
			/* create the spectrum_settings: */
			if (settings=modify_spectrum_data->settings=
				CREATE(Spectrum_settings)())
			{
				/* access since deaccessed in gfx_modify_spectrum */
				ACCESS(Spectrum_settings)(modify_spectrum_data->settings);

				Spectrum_settings_set_type(settings,SPECTRUM_LINEAR);

				alpha = 0;
				ambient = 0;
				amb_diff = 0;
				banded = 0;
				band_ratio = 0.01;
				blue = 0;
				diffuse = 0;
				emission = 0;
				extend_above = 0;
				extend_below = 0;
				fix_maximum=0;
				fix_minimum=0;
				green = 0;
				number_of_bands = 10;
				rainbow = 0;
				white_to_blue = 0;
				white_to_red = 0;
				red = 0;
				reverse = 0;
				specular = 0;
				step = 0;
				step_value = 0.5;
				transparent_above = 0;
				transparent_below = 0;
				range_components = 2;
				colour_range[0] = 0.0;
				colour_range[1] = 1.0;
				range[0] = modify_spectrum_data->spectrum_minimum;
				range[1] = modify_spectrum_data->spectrum_maximum;
				component = Spectrum_settings_get_component_number(settings);

				(colour_option_table[0]).to_be_modified = &alpha;
				(colour_option_table[1]).to_be_modified = &banded;
				(colour_option_table[2]).to_be_modified = &blue;
				(colour_option_table[3]).to_be_modified = &green;
				(colour_option_table[4]).to_be_modified = &rainbow;
				(colour_option_table[5]).to_be_modified = &red;
				(colour_option_table[6]).to_be_modified = &step;
				(colour_option_table[7]).to_be_modified = &white_to_blue;
				(colour_option_table[8]).to_be_modified = &white_to_red;
				(render_option_table[0]).to_be_modified = &ambient;
				(render_option_table[1]).to_be_modified = &amb_diff;
				(render_option_table[2]).to_be_modified = &diffuse;
				(render_option_table[3]).to_be_modified = &emission;
				(render_option_table[4]).to_be_modified = &specular;
				(option_table[0]).to_be_modified = &band_ratio;
				(option_table[1]).to_be_modified = colour_range;
				(option_table[1]).user_data = &range_components;
				(option_table[2]).to_be_modified = &component;
				(option_table[3]).to_be_modified = &extend_above;
				(option_table[4]).to_be_modified = &extend_below;
				(option_table[5]).to_be_modified = &fix_maximum;
				(option_table[6]).to_be_modified = &fix_minimum;
				(option_table[7]).to_be_modified = &number_of_bands;
				(option_table[8]).to_be_modified = &(modify_spectrum_data->position);
				(option_table[9]).to_be_modified = range;
				(option_table[9]).user_data = &range_components;
				(option_table[10]).to_be_modified = &reverse;
				(option_table[11]).to_be_modified = &step_value;
				(option_table[12]).to_be_modified = &transparent_above;
				(option_table[13]).to_be_modified = &transparent_below;
				(option_table[14]).user_data = colour_option_table;
				(option_table[15]).user_data = render_option_table;

				if (!(return_code=process_multiple_options(state,option_table)))
				{
					DEACCESS(Spectrum_settings)(&(modify_spectrum_data->settings));
				}
				if (return_code)
				{
					if (alpha + banded + blue + green + red + rainbow + white_to_blue + white_to_red
						+ step > 1 )
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"only specify one of alpha, banded, blue, green, red, white_to_blue, "
							" white_to_red rainbow or step");
						return_code=0;							
					}
					else if (alpha)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_ALPHA);	
					}
					else if (banded)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_BANDED);	
					}
					else if (blue)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_BLUE);	
					}
					else if (green)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_GREEN);	
					}
					else if (red)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_RED);	
					}
					else if (rainbow)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_RAINBOW);	
					}	
					else if (white_to_blue)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_WHITE_TO_BLUE);	
					}	
					else if (white_to_red)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_WHITE_TO_RED);	
					}
					else if (step)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_STEP);	
					}
				}
				if ( return_code )
				{
					if ((ambient || diffuse || amb_diff) + specular + emission > 1)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"Specify only one of ambient, diffuse, amb_diff, specular, emission.\n  (Specifying ambient and diffuse is valid and equivalent to amb_diff");
						return_code=0;							
					}					
					else if ((ambient && diffuse)|| amb_diff)
					{
						Spectrum_settings_set_render_type(settings,
							SPECTRUM_AMBIENT_AND_DIFFUSE);
					}
					else if (ambient)
					{
						Spectrum_settings_set_render_type(settings,
							SPECTRUM_AMBIENT);
					}
					else if (diffuse)
					{
						Spectrum_settings_set_render_type(settings,
							SPECTRUM_DIFFUSE);
					}
					else if (emission)
					{
						Spectrum_settings_set_render_type(settings,
							SPECTRUM_EMISSION);
					}
					else if (specular)
					{
						Spectrum_settings_set_render_type(settings,
							SPECTRUM_SPECULAR);
					}
				}
				if ( return_code )
				{
					if ( extend_above && transparent_above)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"Specify only one of extend_above and transparent_above");
						return_code=0;							
					}
					else if (extend_above)
					{
						Spectrum_settings_set_extend_above_flag(settings, 1);
					}
					else if (transparent_above)
					{
						Spectrum_settings_set_extend_above_flag(settings, 0);
					}
				}
				if ( return_code )
				{
					if ( extend_below && transparent_below)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"Specify only one of extend_below and transparent_below");
						return_code=0;							
					}
					else if (extend_below)
					{
						Spectrum_settings_set_extend_below_flag(settings, 1);
					}
					else if (transparent_below)
					{
						Spectrum_settings_set_extend_below_flag(settings, 0);
					}
				}			
				if ( return_code )
				{
					Spectrum_settings_set_component_number(settings,
						component);
					Spectrum_settings_set_colour_value_minimum(settings,
						colour_range[0]);
					Spectrum_settings_set_colour_value_maximum(settings,
						colour_range[1]);
					Spectrum_settings_set_range_minimum(settings,
						range[0]);
					Spectrum_settings_set_range_maximum(settings,
						range[1]);
					Spectrum_settings_set_reverse_flag(settings,
						reverse);
					Spectrum_settings_set_number_of_bands(settings,
						number_of_bands);
					black_band_int = (band_ratio * 1000.0 + 0.5);
					Spectrum_settings_set_black_band_proportion(settings,
						black_band_int);
					/* Must set step value after setting minimum and maximum range */
					Spectrum_settings_set_step_value(settings, step_value);
				}
				/* Must set fix_maximum,fix_minimum after setting minimum and maximum range */
				if ( return_code )
				{
					if (fix_maximum)
					{
						Spectrum_settings_set_fix_maximum_flag(settings, 1);
					}
				}
				if ( return_code )
				{
					if (fix_minimum)
					{
						Spectrum_settings_set_fix_minimum_flag(settings, 1);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
					"Could not create settings");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
				"No modify data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_spectrum_settings_linear */

int gfx_modify_spectrum_settings_log(struct Parse_state *state,
	void *modify_spectrum_data_void,void *spectrum_command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LOG command.
If return_code is 1, returns the completed Modify_spectrum_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char alpha,ambient,amb_diff,banded,blue,diffuse,emission,extend_above,
		extend_below,fix_maximum,fix_minimum,green,left,rainbow,white_to_blue,
		white_to_red,red,reverse,right,specular,step,transparent_above,transparent_below;
	int black_band_int,component,number_of_bands,range_components,return_code;
	float band_ratio,colour_range[2],exaggeration,step_value,range[2];
	struct Modify_spectrum_data *modify_spectrum_data;
	struct Spectrum_settings *settings;
	static struct Modifier_entry
		colour_option_table[]=
	  {
			{"alpha",NULL,NULL,set_char_flag},
			{"banded",NULL,NULL,set_char_flag},
			{"blue",NULL,NULL,set_char_flag},
			{"green",NULL,NULL,set_char_flag},
			{"rainbow",NULL,NULL,set_char_flag},
			{"red",NULL,NULL,set_char_flag},			
			{"step",NULL,NULL,set_char_flag},			
			{"white_to_blue",NULL,NULL,set_char_flag},
			{"white_to_red",NULL,NULL,set_char_flag},		
			{NULL,NULL,NULL,NULL}			
		},
		render_option_table[]=
	  {
			{"ambient",NULL,NULL,set_char_flag},
			{"amb_diff",NULL,NULL,set_char_flag},
			{"diffuse",NULL,NULL,set_char_flag},
			{"emission",NULL,NULL,set_char_flag},
			{"specular",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"band_ratio",NULL,NULL,set_float},
			{"colour_range",NULL,NULL,set_float_vector},
			{"component",NULL,NULL,set_int_positive},
			{"exaggeration",NULL,NULL,set_float},
			{"extend_above",NULL,NULL,set_char_flag},
			{"extend_below",NULL,NULL,set_char_flag},
			{"fix_maximum",NULL,NULL,set_char_flag},
			{"fix_minimum",NULL,NULL,set_char_flag},
			{"left",NULL,NULL,set_char_flag},
			{"number_of_bands",NULL,NULL,set_int_positive},
			{"position",NULL,NULL,set_int_non_negative},
			{"range",NULL,NULL,set_float_vector},
			{"reverse",NULL,NULL,set_char_flag},
			{"right",NULL,NULL,set_char_flag},
			{"step_value",NULL,NULL,set_float},
			{"transparent_above",NULL,NULL,set_char_flag},
			{"transparent_below",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL}
		};

	ENTER(gfx_modify_spectrum_settings_log);
	USE_PARAMETER(spectrum_command_data_void);
	if (state)
	{
		if (modify_spectrum_data=
			(struct Modify_spectrum_data *)modify_spectrum_data_void)
		{
			/* create the spectrum_settings: */
			if (settings=modify_spectrum_data->settings=
				CREATE(Spectrum_settings)())
			{
				/* access since deaccessed in gfx_modify_spectrum */
				ACCESS(Spectrum_settings)(modify_spectrum_data->settings);

				Spectrum_settings_set_type(settings,SPECTRUM_LOG);

				alpha = 0;
				ambient = 0;
				amb_diff = 0;
				banded = 0;
				band_ratio = 0.01;
				blue = 0;
				diffuse = 0;
				emission = 0;
				extend_above = 0;
				extend_below = 0;
				fix_maximum=0;
				fix_minimum=0;
				green = 0;
				left = 0;
				number_of_bands = 10;
				rainbow = 0;
				white_to_blue = 0;
				white_to_red = 0;
				red = 0;
				reverse = 0;
				right = 0;
				specular = 0;
				step = 0;
				step_value = 0.5;
				transparent_above = 0;
				transparent_below = 0;
				range_components = 2;
				colour_range[0] = 0.0;
				colour_range[1] = 1.0;
				range[0] = modify_spectrum_data->spectrum_minimum;
				range[1] = modify_spectrum_data->spectrum_maximum;
				component = Spectrum_settings_get_component_number(settings);
				exaggeration = Spectrum_settings_get_exaggeration(settings);

				(colour_option_table[0]).to_be_modified = &alpha;
				(colour_option_table[1]).to_be_modified = &banded;
				(colour_option_table[2]).to_be_modified = &blue;
				(colour_option_table[3]).to_be_modified = &green;
				(colour_option_table[4]).to_be_modified = &rainbow;
				(colour_option_table[5]).to_be_modified = &red;
				(colour_option_table[6]).to_be_modified = &step;
				(colour_option_table[7]).to_be_modified = &white_to_blue;
				(colour_option_table[8]).to_be_modified = &white_to_red;
				(render_option_table[0]).to_be_modified = &ambient;
				(render_option_table[1]).to_be_modified = &amb_diff;
				(render_option_table[2]).to_be_modified = &diffuse;
				(render_option_table[3]).to_be_modified = &emission;
				(render_option_table[4]).to_be_modified = &specular;
				(option_table[0]).to_be_modified = &band_ratio;
				(option_table[1]).to_be_modified = colour_range;
				(option_table[1]).user_data = &range_components;
				(option_table[2]).to_be_modified = &component;
				(option_table[3]).to_be_modified = &exaggeration;
				(option_table[4]).to_be_modified = &extend_above;
				(option_table[5]).to_be_modified = &extend_below;
				(option_table[6]).to_be_modified = &fix_maximum;
				(option_table[7]).to_be_modified = &fix_minimum;
				(option_table[8]).to_be_modified = &left;
				(option_table[9]).to_be_modified = &number_of_bands;
				(option_table[10]).to_be_modified = &(modify_spectrum_data->position);
				(option_table[11]).to_be_modified = range;
				(option_table[11]).user_data = &range_components;
				(option_table[12]).to_be_modified = &reverse;
				(option_table[13]).to_be_modified = &right;
				(option_table[14]).to_be_modified = &step_value;
				(option_table[15]).to_be_modified = &transparent_above;
				(option_table[16]).to_be_modified = &transparent_below;
				(option_table[17]).user_data = colour_option_table;
				(option_table[18]).user_data = render_option_table;

				if (!(return_code=process_multiple_options(state,option_table)))
				{
					DEACCESS(Spectrum_settings)(&(modify_spectrum_data->settings));
				}
				if (return_code)
				{
					if (alpha + banded + blue + green + red + rainbow + white_to_blue + white_to_red
						+ step > 1 )
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"only specify one of alpha, banded, blue, green, red, rainbow,  "
							"white_to_blue white_to_red or step");
						return_code=0;							
					}
					else if (alpha)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_ALPHA);	
					}
					else if (banded)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_BANDED);	
					}
					else if (blue)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_BLUE);	
					}
					else if (green)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_GREEN);	
					}
					else if (red)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_RED);	
					}
					else if (rainbow)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_RAINBOW);	
					}
					else if (white_to_blue)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_WHITE_TO_BLUE);	
					}	
					else if (white_to_red)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_WHITE_TO_RED);	
					}
					else if (step)
					{
						Spectrum_settings_set_colour_mapping(settings,
							SPECTRUM_STEP);	
					}
					Spectrum_settings_set_number_of_bands(settings,
						number_of_bands);
					black_band_int = (band_ratio * 1000.0 + 0.5);
					Spectrum_settings_set_black_band_proportion(settings,
						black_band_int);
					Spectrum_settings_set_step_value(settings, step_value);
				}
				if ( return_code )
				{
					if ((ambient || diffuse || amb_diff) + specular + emission > 1)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"Specify only one of ambient, diffuse, amb_diff, specular, emission.\n  (Specifying ambient and diffuse is valid and equivalent to amb_diff");
						return_code=0;							
					}					
					else if ((ambient && diffuse)|| amb_diff)
					{
						Spectrum_settings_set_render_type(settings,
							SPECTRUM_AMBIENT_AND_DIFFUSE);
					}
					else if (ambient)
					{
						Spectrum_settings_set_render_type(settings,
							SPECTRUM_AMBIENT);
					}
					else if (diffuse)
					{
						Spectrum_settings_set_render_type(settings,
							SPECTRUM_DIFFUSE);
					}
					else if (emission)
					{
						Spectrum_settings_set_render_type(settings,
							SPECTRUM_EMISSION);
					}
					else if (specular)
					{
						Spectrum_settings_set_render_type(settings,
							SPECTRUM_SPECULAR);
					}
				}
				if ( return_code )
				{
					if ( extend_above && transparent_above)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"Specify only one of extend_above and transparent_above");
						return_code=0;							
					}
					else if (extend_above)
					{
						Spectrum_settings_set_extend_above_flag(settings, 1);
					}
					else if (transparent_above)
					{
						Spectrum_settings_set_extend_above_flag(settings, 0);
					}
				}
				if ( return_code )
				{
					if ( extend_below && transparent_below)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"Specify only one of extend_below and transparent_below");
						return_code=0;							
					}
					else if (extend_below)
					{
						Spectrum_settings_set_extend_below_flag(settings, 1);
					}
					else if (transparent_below)
					{
						Spectrum_settings_set_extend_below_flag(settings, 0);
					}
				}
				if ( return_code )
				{
					if (left && right)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"Specify only one of left or right");
						return_code=0;							
					}
					else if (left)
					{
						exaggeration = fabs(exaggeration);
					}
					else if (right)
					{
						exaggeration = -fabs(exaggeration);
					}
					Spectrum_settings_set_exaggeration(settings,
						exaggeration);
				}
				if ( return_code )
				{
					Spectrum_settings_set_component_number(settings,
						component);
					Spectrum_settings_set_colour_value_minimum(settings,
						colour_range[0]);
					Spectrum_settings_set_colour_value_maximum(settings,
						colour_range[1]);
					Spectrum_settings_set_range_minimum(settings,
						range[0]);
					Spectrum_settings_set_range_maximum(settings,
						range[1]);
					Spectrum_settings_set_reverse_flag(settings,
						reverse);
				}
				/* Must set fix_maximum,fix_minimum after setting minimum and maximum range */
				if ( return_code )
				{
					if (fix_maximum)
					{
						Spectrum_settings_set_fix_maximum_flag(settings, 1);
					}
				}
				if ( return_code )
				{
					if (fix_minimum)
					{
						Spectrum_settings_set_fix_minimum_flag(settings, 1);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
					"Could not create settings");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
				"No modify data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_spectrum_log */

