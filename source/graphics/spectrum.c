/*******************************************************************************
FILE : spectrum.c

LAST MODIFIED : 21 December 2000

DESCRIPTION :
Spectrum functions and support code.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#if defined (OPENGL_API)
#include <GL/glu.h>
#endif /* defined (OPENGL_API) */
#include "command/parser.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/object.h"
#include "general/mystring.h"
#include "graphics/material.h"
#include "graphics/spectrum_settings.h"
#include "graphics/spectrum.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
struct Spectrum
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Spectrum type is private.
==============================================================================*/
{
	float maximum,minimum;
	char *name;
	int clear_colour_before_settings;
	struct LIST(Spectrum_settings) *list_of_settings;
	/* the number of structures that point to this spectrum.  The spectrum
		cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct Spectrum */

FULL_DECLARE_INDEXED_LIST_TYPE(Spectrum);

FULL_DECLARE_MANAGER_TYPE(Spectrum);

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Spectrum,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Spectrum)

/*
Global functions
----------------
*/
DECLARE_OBJECT_FUNCTIONS(Spectrum)

DECLARE_INDEXED_LIST_FUNCTIONS(Spectrum)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Spectrum,name,char *,strcmp)

DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Spectrum,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Spectrum,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(
				destination, source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Spectrum,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Spectrum,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name));
	/* check arguments */
	if (source&&destination)
	{
		/* copy values */
		destination->maximum = source->maximum;
		destination->minimum = source->minimum;
		destination->clear_colour_before_settings = 
			source->clear_colour_before_settings;

		/* empty original list_of_settings */
		REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)(
			destination->list_of_settings);
		/* put copy of each settings in source list in destination list */
		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_copy_and_put_in_list,
			(void *)destination->list_of_settings,source->list_of_settings);
		return_code=1;		
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Spectrum,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Spectrum,name));
	/* check arguments */
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Spectrum,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Spectrum,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Spectrum,name) */

DECLARE_MANAGER_FUNCTIONS(Spectrum)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Spectrum,name,char *)

/*
Global functions
----------------
*/

int Spectrum_set_simple_type(struct Spectrum *spectrum,
	enum Spectrum_simple_type type)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
A convienience routine that allows a spectrum to be automatically set into
some predetermined simple types.
==============================================================================*/
{
	struct LIST(Spectrum_settings) *spectrum_settings_list;
	struct Spectrum_settings *settings, *second_settings,*third_settings,
		*fourth_settings,*fifth_settings,*sixth_settings;
	int number_in_list, return_code;

	ENTER(Spectrum_set_simple_type);
	if (spectrum)
	{
		return_code = 1;
		switch(type)
		{				
			case RED_TO_BLUE_SPECTRUM:
			case BLUE_TO_RED_SPECTRUM:		
			{
				spectrum_settings_list = get_Spectrum_settings_list(spectrum);
				number_in_list = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
				if ( number_in_list > 0 )
				{
					REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)(spectrum_settings_list);
				}
				settings = CREATE(Spectrum_settings)();
				Spectrum_settings_add(settings, /* end of list = 0 */0,
					spectrum_settings_list);
				
				Spectrum_settings_set_type(settings, SPECTRUM_LINEAR);
				Spectrum_settings_set_colour_mapping(settings, SPECTRUM_RAINBOW);
				Spectrum_settings_set_extend_above_flag(settings, 1);
				Spectrum_settings_set_extend_below_flag(settings, 1);
				switch (type)
				{
					case RED_TO_BLUE_SPECTRUM:
					{
						Spectrum_settings_set_reverse_flag(settings, 0);
					} break;
					case BLUE_TO_RED_SPECTRUM:
					{
						Spectrum_settings_set_reverse_flag(settings, 1);
					} break;
				}
			} break;
			case LOG_RED_TO_BLUE_SPECTRUM:
			case LOG_BLUE_TO_RED_SPECTRUM:			
			{
				spectrum_settings_list = get_Spectrum_settings_list(spectrum);
				number_in_list = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
				if ( number_in_list > 0 )
				{
					REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)(spectrum_settings_list);
				}
				settings = CREATE(Spectrum_settings)();
				second_settings = CREATE(Spectrum_settings)();				
				Spectrum_settings_add(settings, /* end of list = 0 */0,
					spectrum_settings_list);
				Spectrum_settings_add(second_settings, /* end of list = 0 */0,
					spectrum_settings_list);
			
				
				Spectrum_settings_set_type(settings, SPECTRUM_LOG);
				Spectrum_settings_set_exaggeration(settings, 1.0);
				Spectrum_settings_set_colour_mapping(settings, SPECTRUM_RAINBOW);
				Spectrum_settings_set_range_minimum(settings, -1.0);
				Spectrum_settings_set_range_maximum(settings, 0.0);
				
				Spectrum_settings_set_type(second_settings, SPECTRUM_LOG);
				Spectrum_settings_set_exaggeration(second_settings, -1.0);
				Spectrum_settings_set_colour_mapping(second_settings, SPECTRUM_RAINBOW);
				Spectrum_settings_set_range_minimum(second_settings, 0.0);
				Spectrum_settings_set_range_maximum(second_settings, 1.0);

				Spectrum_settings_set_extend_below_flag(settings, 1);
				Spectrum_settings_set_extend_above_flag(second_settings, 1);

				switch(type)
				{
					case LOG_RED_TO_BLUE_SPECTRUM:
					{
						Spectrum_settings_set_reverse_flag(settings, 0);
						Spectrum_settings_set_colour_value_minimum(settings, 0);
						Spectrum_settings_set_colour_value_maximum(settings, 0.5);
						Spectrum_settings_set_reverse_flag(second_settings, 0);
						Spectrum_settings_set_colour_value_minimum(second_settings, 0.5);
						Spectrum_settings_set_colour_value_maximum(second_settings, 1.0);
					} break;
					case LOG_BLUE_TO_RED_SPECTRUM:
					{
						Spectrum_settings_set_reverse_flag(settings, 1);
						Spectrum_settings_set_colour_value_minimum(settings, 0.5);
						Spectrum_settings_set_colour_value_maximum(settings, 1.0);
						Spectrum_settings_set_reverse_flag(second_settings, 1);
						Spectrum_settings_set_colour_value_minimum(second_settings, 0.0);
						Spectrum_settings_set_colour_value_maximum(second_settings, 0.5);
					} break;
				
				}			
			} break;
			case BLUE_WHITE_RED_SPECTRUM:
			{
				spectrum_settings_list = get_Spectrum_settings_list(spectrum);
				number_in_list = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
				if ( number_in_list > 0 )
				{
					REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)(spectrum_settings_list);
				}
				settings = CREATE(Spectrum_settings)();
				second_settings = CREATE(Spectrum_settings)();
				third_settings = CREATE(Spectrum_settings)();
				fourth_settings = CREATE(Spectrum_settings)();
				fifth_settings = CREATE(Spectrum_settings)();
				sixth_settings = CREATE(Spectrum_settings)();
				Spectrum_settings_add(settings, /* end of list = 0 */0,
					spectrum_settings_list);
				Spectrum_settings_add(second_settings, /* end of list = 0 */0,
					spectrum_settings_list);
				Spectrum_settings_add(third_settings, /* end of list = 0 */0,
					spectrum_settings_list);
				Spectrum_settings_add(fourth_settings, /* end of list = 0 */0,
					spectrum_settings_list);
				Spectrum_settings_add(fifth_settings, /* end of list = 0 */0,
					spectrum_settings_list);
				Spectrum_settings_add(sixth_settings, /* end of list = 0 */0,
					spectrum_settings_list);

				Spectrum_settings_set_type(settings, SPECTRUM_LINEAR);
				Spectrum_settings_set_reverse_flag(settings, 1);	
				Spectrum_settings_set_range_minimum(settings, 0.0);
				Spectrum_settings_set_range_maximum(settings, 0.5);	
				Spectrum_settings_set_extend_below_flag(settings, 1);			
				Spectrum_settings_set_colour_mapping(settings, SPECTRUM_BLUE);						
				Spectrum_settings_set_colour_value_minimum(settings, 1);
				Spectrum_settings_set_colour_value_maximum(settings, 1);

				Spectrum_settings_set_type(second_settings, SPECTRUM_LOG);
				Spectrum_settings_set_exaggeration(second_settings, -10.0);
				Spectrum_settings_set_range_minimum(second_settings, 0.0);
				Spectrum_settings_set_range_maximum(second_settings, 0.5);
				Spectrum_settings_set_extend_below_flag(second_settings, 1);			
				Spectrum_settings_set_colour_mapping(second_settings, SPECTRUM_GREEN);
				Spectrum_settings_set_colour_value_minimum(second_settings, 0);
				Spectrum_settings_set_colour_value_maximum(second_settings, 1);

				Spectrum_settings_set_type(third_settings, SPECTRUM_LOG);
				Spectrum_settings_set_exaggeration(third_settings, -10.0);
				Spectrum_settings_set_range_minimum(third_settings, 0.0);
				Spectrum_settings_set_range_maximum(third_settings, 0.5);
				Spectrum_settings_set_extend_below_flag(third_settings, 1);			
				Spectrum_settings_set_colour_mapping(third_settings, SPECTRUM_RED);						
				Spectrum_settings_set_colour_value_minimum(third_settings, 0);
				Spectrum_settings_set_colour_value_maximum(third_settings, 1);

				Spectrum_settings_set_type(fourth_settings, SPECTRUM_LINEAR);
				Spectrum_settings_set_range_minimum(fourth_settings, 0.5);
				Spectrum_settings_set_range_maximum(fourth_settings, 1.0);	
				Spectrum_settings_set_extend_above_flag(fourth_settings, 1);			
				Spectrum_settings_set_colour_mapping(fourth_settings, SPECTRUM_RED);
				Spectrum_settings_set_colour_value_minimum(fourth_settings, 1);
				Spectrum_settings_set_colour_value_maximum(fourth_settings, 1);

				Spectrum_settings_set_type(fifth_settings, SPECTRUM_LOG);
				Spectrum_settings_set_exaggeration(fifth_settings, 10.0);
				Spectrum_settings_set_reverse_flag(fifth_settings, 1);	
				Spectrum_settings_set_range_minimum(fifth_settings, 0.5);
				Spectrum_settings_set_range_maximum(fifth_settings, 1.0);
				Spectrum_settings_set_extend_above_flag(fifth_settings, 1);			
				Spectrum_settings_set_colour_mapping(fifth_settings, SPECTRUM_GREEN);
				Spectrum_settings_set_colour_value_minimum(fifth_settings, 0);
				Spectrum_settings_set_colour_value_maximum(fifth_settings, 1);

				Spectrum_settings_set_type(sixth_settings, SPECTRUM_LOG);
				Spectrum_settings_set_exaggeration(sixth_settings, 10.0);
				Spectrum_settings_set_reverse_flag(sixth_settings, 1);	
				Spectrum_settings_set_range_minimum(sixth_settings, 0.5);
				Spectrum_settings_set_range_maximum(sixth_settings, 1.0);
				Spectrum_settings_set_extend_above_flag(sixth_settings, 1);			
				Spectrum_settings_set_colour_mapping(sixth_settings, SPECTRUM_BLUE);
				Spectrum_settings_set_colour_value_minimum(sixth_settings, 0);
				Spectrum_settings_set_colour_value_maximum(sixth_settings, 1);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_set_simple_type.  Unknown simple spectrum type");
				return_code=0;
			} break;
		}
		Spectrum_calculate_range(spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_set_simple_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_set_simple_type */

enum Spectrum_simple_type Spectrum_get_simple_type(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
A convienience routine that interrogates a spectrum to see if it is one of the
simple types.  If it does not comform exactly to one of the simple types then
it returns UNKNOWN_SPECTRUM
==============================================================================*/
{
	struct LIST(Spectrum_settings) *spectrum_settings_list;
	struct Spectrum_settings *settings, *second_settings,*third_settings,
		*fourth_settings,*fifth_settings,*sixth_settings;
	enum Spectrum_settings_type settings_type, second_settings_type;
	int number_in_list, reverse, second_reverse;
	enum Spectrum_settings_colour_mapping colour_mapping, second_colour_mapping;
	enum Spectrum_simple_type type;

	ENTER(Spectrum_get_simple_type);

	if (spectrum)
	{
		type = UNKNOWN_SPECTRUM;	
		
		spectrum_settings_list = get_Spectrum_settings_list(spectrum);
		number_in_list = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
		switch( number_in_list )
		{
			case 1:
			{
				settings = FIRST_OBJECT_IN_LIST_THAT(Spectrum_settings)
					((LIST_CONDITIONAL_FUNCTION(Spectrum_settings) *)NULL, NULL,
					spectrum_settings_list);
				settings_type = Spectrum_settings_get_type(settings);
				reverse = Spectrum_settings_get_reverse_flag(settings);
				colour_mapping = Spectrum_settings_get_colour_mapping(settings);
				
				if ( settings_type == SPECTRUM_LINEAR )
				{
					if ( colour_mapping == SPECTRUM_RAINBOW )
					{
						if ( reverse )
						{
							type = BLUE_TO_RED_SPECTRUM;
						}
						else
						{
							type = RED_TO_BLUE_SPECTRUM;
						}
					}
				}
			} break;
			case 2:
			{
				settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
					(1, spectrum_settings_list);
				second_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
					(2, spectrum_settings_list);
				if ( settings && second_settings )
				{
					settings_type = Spectrum_settings_get_type(settings);
					reverse = Spectrum_settings_get_reverse_flag(settings);
					colour_mapping = Spectrum_settings_get_colour_mapping(settings);
					second_settings_type = Spectrum_settings_get_type(second_settings);
					second_reverse = Spectrum_settings_get_reverse_flag(second_settings);
					second_colour_mapping = Spectrum_settings_get_colour_mapping
						(second_settings);
					
					if((settings_type == SPECTRUM_LOG)
						&& (second_settings_type == SPECTRUM_LOG))
					{
						if ((colour_mapping == SPECTRUM_RAINBOW)
							&& (second_colour_mapping == SPECTRUM_RAINBOW))
						{
							if ( reverse && second_reverse )
							{
								type = LOG_BLUE_TO_RED_SPECTRUM;
							}
							else if (!(reverse || second_reverse))
							{
								type = LOG_RED_TO_BLUE_SPECTRUM;
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Spectrum_set_simple_type.  Bad position numbers in settings");
				}
			}break;
			case 6:
			{	
				settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
					(1, spectrum_settings_list);
				second_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
					(2, spectrum_settings_list);
				third_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
					(3, spectrum_settings_list);
				fourth_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
					(4, spectrum_settings_list);
				fifth_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
					(5, spectrum_settings_list);
				sixth_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
					(6, spectrum_settings_list);
				if (settings&&second_settings&&third_settings&&fourth_settings&&
					fifth_settings&&sixth_settings)
				{
					/* Could do a more exhaustive check, I think but this is sufficient.*/
					 if((SPECTRUM_BLUE==Spectrum_settings_get_colour_mapping(settings))&&
						 (SPECTRUM_GREEN==Spectrum_settings_get_colour_mapping(second_settings))&&
						 (SPECTRUM_RED==Spectrum_settings_get_colour_mapping(third_settings))&&
						 (SPECTRUM_RED==Spectrum_settings_get_colour_mapping(fourth_settings))&&
						 (SPECTRUM_GREEN==Spectrum_settings_get_colour_mapping(fifth_settings))&&
						 (SPECTRUM_BLUE==Spectrum_settings_get_colour_mapping(sixth_settings)))
					 {
						 type = BLUE_WHITE_RED_SPECTRUM;
					 }
				}				
			}break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_set_simple_type.  Invalid argument(s)");
		type = UNKNOWN_SPECTRUM;
	}
	LEAVE;

	return (type);
} /* Spectrum_get_simple_type */

enum Spectrum_simple_type Spectrum_get_contoured_simple_type(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
A convienience routine that interrogates a spectrum to see if it is one of the
simple types, or a simple type with a contour(colour_mapping==SPECTRUM_BANDED) 
added as an extra, last, setting If it does not comform exactly to one of the 
simple types (or a simple type with a contour) then it returns UNKNOWN_SPECTRUM. 
See also Spectrum_get_simple_type.
==============================================================================*/
{	
	enum Spectrum_settings_colour_mapping colour_mapping,second_colour_mapping;
	enum Spectrum_settings_type settings_type, second_settings_type;
	enum Spectrum_simple_type spectrum_simple_type;	
	int number_of_settings,reverse,second_reverse;
	struct LIST(Spectrum_settings) *spectrum_settings_list=
		(struct LIST(Spectrum_settings) *)NULL;	
	struct Spectrum_settings *settings,*second_settings,*third_settings,
		*fourth_settings,*fifth_settings,*sixth_settings,*spectrum_settings;
	
	ENTER(Spectrum_get_contoured_simple_type);
	settings=(struct Spectrum_settings *)NULL;
	second_settings=(struct Spectrum_settings *)NULL;
	third_settings=(struct Spectrum_settings *)NULL;
	fourth_settings=(struct Spectrum_settings *)NULL;
	fifth_settings=(struct Spectrum_settings *)NULL;
	sixth_settings=(struct Spectrum_settings *)NULL;
	spectrum_settings=(struct Spectrum_settings *)NULL;
	spectrum_simple_type=UNKNOWN_SPECTRUM;
	if(spectrum)
	{	
		/* if spectrum is a simple type, nothing else to do*/
		spectrum_simple_type=Spectrum_get_simple_type(spectrum);
		if(spectrum_simple_type==UNKNOWN_SPECTRUM)
		{
			/* is the last settings a contour? (SPECTRUM_BANDED)*/
			spectrum_settings_list = get_Spectrum_settings_list(spectrum);
			number_of_settings = NUMBER_IN_LIST(Spectrum_settings)(spectrum_settings_list);
			spectrum_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
				(number_of_settings, spectrum_settings_list);						
			colour_mapping=	Spectrum_settings_get_colour_mapping(spectrum_settings);
			/*if so, proceed as for Spectrum_get_simple_type */
			if(colour_mapping==SPECTRUM_BANDED)
			{			
				switch( number_of_settings )
				{
					case 2:
					{
						settings = FIRST_OBJECT_IN_LIST_THAT(Spectrum_settings)
							((LIST_CONDITIONAL_FUNCTION(Spectrum_settings) *)NULL, NULL,
								spectrum_settings_list);
						settings_type = Spectrum_settings_get_type(settings);
						reverse = Spectrum_settings_get_reverse_flag(settings);
						colour_mapping = Spectrum_settings_get_colour_mapping(settings);
				
						if ( settings_type == SPECTRUM_LINEAR )
						{
							if ( colour_mapping == SPECTRUM_RAINBOW )
							{
								if ( reverse )
								{
									spectrum_simple_type = BLUE_TO_RED_SPECTRUM;
								}
								else
								{
									spectrum_simple_type = RED_TO_BLUE_SPECTRUM;
								}
							}
						}
					} break;
					case 3:
					{
						settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
							(1, spectrum_settings_list);
						second_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
							(2, spectrum_settings_list);
						if ( settings && second_settings )
						{
							settings_type = Spectrum_settings_get_type(settings);
							reverse = Spectrum_settings_get_reverse_flag(settings);
							colour_mapping = Spectrum_settings_get_colour_mapping(settings);
							second_settings_type = Spectrum_settings_get_type(second_settings);
							second_reverse = Spectrum_settings_get_reverse_flag(second_settings);
							second_colour_mapping = Spectrum_settings_get_colour_mapping
								(second_settings);
					
							if((settings_type == SPECTRUM_LOG)
								&& (second_settings_type == SPECTRUM_LOG))
							{
								if ((colour_mapping == SPECTRUM_RAINBOW)
									&& (second_colour_mapping == SPECTRUM_RAINBOW))
								{
									if ( reverse && second_reverse )
									{
										spectrum_simple_type = LOG_BLUE_TO_RED_SPECTRUM;
									}
									else if (!(reverse || second_reverse))
									{
										spectrum_simple_type = LOG_RED_TO_BLUE_SPECTRUM;
									}
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Spectrum_set_simple_type.  Bad position numbers in settings");
						}
					}break;
					case 7:
					{	
						settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
							(1, spectrum_settings_list);
						second_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
							(2, spectrum_settings_list);
						third_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
							(3, spectrum_settings_list);
						fourth_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
							(4, spectrum_settings_list);
						fifth_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
							(5, spectrum_settings_list);
						sixth_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
							(6, spectrum_settings_list);
						if (settings&&second_settings&&third_settings&&fourth_settings&&
							fifth_settings&&sixth_settings)
						{
							/* Could do a more exhaustive check, I think but this is sufficient.*/
							if((SPECTRUM_BLUE==Spectrum_settings_get_colour_mapping(settings))&&
								(SPECTRUM_GREEN==Spectrum_settings_get_colour_mapping(second_settings))&&
								(SPECTRUM_RED==Spectrum_settings_get_colour_mapping(third_settings))&&
								(SPECTRUM_RED==Spectrum_settings_get_colour_mapping(fourth_settings))&&
								(SPECTRUM_GREEN==Spectrum_settings_get_colour_mapping(fifth_settings))&&
								(SPECTRUM_BLUE==Spectrum_settings_get_colour_mapping(sixth_settings)))
							{
								spectrum_simple_type = BLUE_WHITE_RED_SPECTRUM;
							}
						}				
					}break;
				}/*switch( number_in_list ) */
			}/* if(spectrum_settings_colour_mapping==SPECTRUM_BANDED) */
			else
			{
				display_message(WARNING_MESSAGE,
					"Spectrum_get_contoured_simple_type. Spectrum not simple type or\n"
					" contoured simple type ");
			}
		}/* if(spectrum_simple_type==UNKNOWN_SPECTRUM) */
	}/* if(spectrum) */
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_contoured_simple_type. Invalid argument(s)");
	}
	LEAVE;
	return(spectrum_simple_type);
}/* Spectrum_get_contoured_simple_type */

int Spectrum_overlay_contours(struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *spectrum,int number_of_bands,int band_proportions)
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Checks if the last spectrum setting is SPECTRUM_BANDED, removes it if it is,
then adds a SPECTRUM_BANDED setting to the <spectrum> with <number_of_bands>,
<band_proportions>. Setting is added at the end of the list.
This function assumes the <spectum> is a simple with an added SPECTRUM_BANDED 
settings holding for the contours.
If <number_of_bands>==0, simply removes any existing contour band settings.
==============================================================================*/
{
	int return_code;	
	enum Spectrum_settings_colour_mapping spectrum_settings_colour_mapping;
	FE_value min,max,number_of_settings;
	struct LIST(Spectrum_settings) *spectrum_settings_list=
					(struct LIST(Spectrum_settings) *)NULL;
	struct Spectrum *spectrum_to_be_modified_copy=
		(struct Spectrum *)NULL;
	struct Spectrum_settings *spectrum_settings=
					(struct Spectrum_settings *)NULL;

	ENTER(Spectrum_overlay_contours);
	if(spectrum_manager&&spectrum)
	{
		if (IS_MANAGED(Spectrum)(spectrum,spectrum_manager))
		{	
			return_code=1;
			/* get the last settings */
			spectrum_settings_list = get_Spectrum_settings_list(spectrum);
			number_of_settings=NUMBER_IN_LIST(Spectrum_settings)
				(spectrum_settings_list);
			spectrum_settings = FIND_BY_IDENTIFIER_IN_LIST(Spectrum_settings,position)
				(number_of_settings, spectrum_settings_list);
			/*if a contour, SPECTRUM_BANDED, remove */			
			spectrum_settings_colour_mapping=
				Spectrum_settings_get_colour_mapping(spectrum_settings);
			if(spectrum_settings_colour_mapping==SPECTRUM_BANDED)
			{
				Spectrum_settings_remove(spectrum_settings,spectrum_settings_list);
				spectrum_settings=(struct Spectrum_settings *)NULL;
			}	
			if (spectrum_to_be_modified_copy=CREATE(Spectrum)
				("spectrum_modify_temp"))
			{
				/* if required,generate and set the contours setting */
				if(number_of_bands)
				{					
					MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)
						(spectrum_to_be_modified_copy,spectrum);				
					spectrum_settings=CREATE(Spectrum_settings)();
					max=get_Spectrum_maximum(spectrum);
					min=get_Spectrum_minimum(spectrum);
					Spectrum_settings_set_range_maximum(spectrum_settings,max);
					Spectrum_settings_set_range_minimum(spectrum_settings,min);
					Spectrum_settings_set_extend_below_flag(spectrum_settings,1);
					Spectrum_settings_set_extend_above_flag(spectrum_settings,1);	
					Spectrum_settings_set_type(spectrum_settings,SPECTRUM_LINEAR);
					Spectrum_settings_set_number_of_bands(spectrum_settings,number_of_bands);
					Spectrum_settings_set_black_band_proportion(spectrum_settings,
						band_proportions);
					Spectrum_settings_set_colour_mapping(spectrum_settings,SPECTRUM_BANDED);
					Spectrum_add_settings(spectrum_to_be_modified_copy,spectrum_settings,0);
					MANAGER_MODIFY_NOT_IDENTIFIER(Spectrum,name)(spectrum,
						spectrum_to_be_modified_copy,spectrum_manager);
					DESTROY(Spectrum)(&spectrum_to_be_modified_copy);
				}			
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_overlay_contours. Could not create spectrum copy.");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Spectrum_overlay_contours. Spectrum is not in manager!");
			return_code=0;
		}								
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_overlay_contours.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
}/* Spectrum_overlay_contours() */
int Spectrum_add_settings(struct Spectrum *spectrum,
	struct Spectrum_settings *settings,int position)
/*******************************************************************************
LAST MODIFIED : 24 June 1998

DESCRIPTION :
Adds the <settings> to <spectrum> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the settings to be added at its end, with a
position one greater than the last.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_add_settings);
	if (spectrum&&settings)
	{
		return_code=Spectrum_settings_add(settings,position,
			spectrum->list_of_settings);
		Spectrum_calculate_range(spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_add_settings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_add_settings */

int Spectrum_remove_settings(struct Spectrum *spectrum,
	struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 24 June 1998

DESCRIPTION :
Removes the <settings> from <spectrum> and decrements the position
of all subsequent settings.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_remove_settings);
	if (spectrum&&settings)
	{
		return_code=Spectrum_settings_remove(settings,
			spectrum->list_of_settings);
		Spectrum_calculate_range(spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_remove_settings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_remove_settings */

int Spectrum_remove_all_settings(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 4 August 1998

DESCRIPTION :
Removes the all the settings from <spectrum>.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_remove_all_settings);
	if (spectrum)
	{
		REMOVE_ALL_OBJECTS_FROM_LIST(Spectrum_settings)
			(get_Spectrum_settings_list(spectrum));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_remove_all_settings.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_remove_all_settings */

int Spectrum_get_settings_position(struct Spectrum *spectrum,
	struct Spectrum_settings *settings)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Returns the position of <settings> in <spectrum>.
==============================================================================*/
{
	int position;

	ENTER(Spectrum_get_settings_position);
	if (spectrum&&settings)
	{
		position=Spectrum_settings_get_position(settings,
			spectrum->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_settings_position.  Invalid argument(s)");
		position=0;
	}
	LEAVE;

	return (position);
} /* Spectrum_get_settings_position */

int set_Spectrum(struct Parse_state *state,void *spectrum_address_void,
	void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
A modifier function to set the spectrum by finding in the spectrum manager
the name given in the next token of the parser
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Spectrum *temp_spectrum,**spectrum_address;
	struct MANAGER(Spectrum) *spectrum_manager;

	ENTER(set_Spectrum);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((spectrum_address=
					(struct Spectrum **)spectrum_address_void)&&
					(spectrum_manager=(struct MANAGER(Spectrum) *)
					spectrum_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*spectrum_address)
						{
							DEACCESS(Spectrum)(spectrum_address);
							*spectrum_address=(struct Spectrum *)NULL;
						}
						return_code=1;
					}
					else
					{
						if (temp_spectrum=FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum,
							name)(current_token,spectrum_manager))
						{
							if (*spectrum_address!=temp_spectrum)
							{
								DEACCESS(Spectrum)(spectrum_address);
								*spectrum_address=ACCESS(Spectrum)(temp_spectrum);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown spectrum : %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
"set_Spectrum.  Invalid argument(s).  spectrum_address %p.  spectrum_manager %p",
						spectrum_address_void,spectrum_manager_void);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," SPECTRUM_NAME|none");
				/* if possible, then write the name */
				if (spectrum_address=
					(struct Spectrum **)spectrum_address_void)
				{
					if (temp_spectrum= *spectrum_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_spectrum->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing material name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum */

int set_Spectrum_minimum(struct Spectrum *spectrum,float minimum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum minimum.
==============================================================================*/
{
	float maximum;
	int return_code;

	ENTER(set_Spectrum_minimum);
	if (spectrum)
	{
		if (spectrum->maximum < minimum)
		{
			maximum = minimum;
		}
		else
		{
			maximum = spectrum->maximum;
		}
		Spectrum_set_minimum_and_maximum(spectrum, minimum, maximum);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
							"set_Spectrum_minimum.  Invalid spectrum object.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum_minimum */

int set_Spectrum_maximum(struct Spectrum *spectrum,float maximum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum maximum.
==============================================================================*/
{
	float minimum;
	int return_code;

	ENTER(set_Spectrum_maximum);
	if (spectrum)
	{
		if (spectrum->minimum > maximum)
		{
			minimum = maximum;
		}
		else
		{
			minimum = spectrum->minimum;
		}
		Spectrum_set_minimum_and_maximum(spectrum, minimum, maximum);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
							"set_Spectrum_maximum.  Invalid spectrum object.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum_maximum */

int set_Spectrum_minimum_command(struct Parse_state *state,
	void *spectrum_ptr_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
A modifier function to set the spectrum minimum.
==============================================================================*/
{
	char *current_token;
	float value;
	int return_code;
	struct Spectrum *spectrum;

	ENTER(set_Spectrum_minimum);
	if (state && (!dummy_user_data))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (spectrum= *((struct Spectrum **)spectrum_ptr_void))
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						set_Spectrum_minimum(spectrum,value);

						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid spectrum minimum : %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Spectrum_minimum_command.  Missing spectrum_ptr_void");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," MINIMUM_VALUE#");
				if (spectrum= *((struct Spectrum **)spectrum_ptr_void))
				{
					display_message(INFORMATION_MESSAGE,"[%g]",spectrum->minimum);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[calculated]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing spectrum minimum");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Spectrum_minimum_command.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum_minimum_command */

int set_Spectrum_maximum_command(struct Parse_state *state,
	void *spectrum_ptr_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
A modifier function to set the spectrum maximum.
==============================================================================*/
{
	char *current_token;
	float value;
	int return_code;
	struct Spectrum *spectrum;

	ENTER(set_Spectrum_maximum);
	if (state && (!dummy_user_data))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (spectrum= *((struct Spectrum **)spectrum_ptr_void))
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						set_Spectrum_maximum(spectrum,value);
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(WARNING_MESSAGE,"Invalid spectrum maximum : %s",
							current_token);
						display_parse_state_location(state);
						return_code=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Spectrum_maximum_command.  Missing spectrum_ptr_void");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," MAXIMUM_VALUE#");
				if (spectrum= *((struct Spectrum **)spectrum_ptr_void))
				{
					display_message(INFORMATION_MESSAGE,"[%g]",spectrum->maximum);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[calculated]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing spectrum maximum");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Spectrum_maximum_command.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum_maximum_command */

float get_Spectrum_minimum(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Returns the value of the spectrum minimum.
==============================================================================*/
{
	float minimum;
	ENTER(get_Spectrum_minimum);

	if (spectrum)
	{
		minimum=spectrum->minimum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
							"get_Spectrum_minimum.  Invalid spectrum object.");
		minimum=0;
	}

	LEAVE;
	return (minimum);
} /* get_Spectrum_minimum */

float get_Spectrum_maximum(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Returns the value of the spectrum maximum.
==============================================================================*/
{
	float maximum;

	ENTER(get_Spectrum_maximum);
	if (spectrum)
	{
		maximum=spectrum->maximum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Spectrum_maximum.  Invalid spectrum object.");
		maximum=0;
	}
	LEAVE;

	return (maximum);
} /* get_Spectrum_maximum */

int Spectrum_get_opaque_colour_flag(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Returns the value of the spectrum opaque flag which indicates whether the
spectrum clears the material colour before applying the settings or not.
==============================================================================*/
{
	int opaque;

	ENTER(get_Spectrum_maximum);
	if (spectrum)
	{
		opaque = spectrum->clear_colour_before_settings;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_opaque_colour_flag.  Invalid spectrum object.");
		opaque = 0;
	}
	LEAVE;

	return (opaque);
} /* Spectrum_get_opaque_colour_flag */

int Spectrum_set_opaque_colour_flag(struct Spectrum *spectrum, int opaque)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Sets the value of the spectrum opaque flag which indicates whether the
spectrum clears the material colour before applying the settings or not.
==============================================================================*/
{
	int return_code;

	ENTER(get_Spectrum_maximum);
	if (spectrum)
	{
		spectrum->clear_colour_before_settings = opaque;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_set_opaque_colour_flag.  Invalid spectrum object.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_set_opaque_colour_flag */

#if defined (OPENGL_API)
struct Spectrum_render_data *spectrum_start_renderGL(
	struct Spectrum *spectrum,struct Graphical_material *material,
	int number_of_data_components)
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
Initialises the graphics state for rendering values on the current material.
==============================================================================*/
{
	struct Spectrum_render_data *render_data;

	ENTER(spectrum_start_renderGL);
	if (spectrum)
	{
		if (material)
		{
			if (ALLOCATE(render_data,struct Spectrum_render_data,1))
			{
				render_data->rendering_flags = 0;
				render_data->number_of_data_components = number_of_data_components;
				if ((render_data->material=
					CREATE(Graphical_material)("spectrum_copy"))&&
					MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)
					(render_data->material,material))
				{
					FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
						Spectrum_settings_enable,(void *)render_data,
						spectrum->list_of_settings);
					switch(render_data->rendering_flags)
					{
						case SPECTRUM_AMBIENT_AND_DIFFUSE:
						{
							glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
							glEnable(GL_COLOR_MATERIAL);
						} break;
						case SPECTRUM_AMBIENT:
						{
							glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
							glEnable(GL_COLOR_MATERIAL);
						} break;
						case SPECTRUM_DIFFUSE:
						{
							glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
							glEnable(GL_COLOR_MATERIAL);
						} break;
						case SPECTRUM_EMISSION:
						{
							glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);
							glEnable(GL_COLOR_MATERIAL);
						} break;
						case SPECTRUM_SPECULAR:
						{
							glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
							glEnable(GL_COLOR_MATERIAL);
						} break;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"spectrum_start_renderGL.  Unable to copy material.");
					DEALLOCATE(render_data);
					render_data=(struct Spectrum_render_data *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"spectrum_start_renderGL.  Unable to allocate render data.");
				render_data=(struct Spectrum_render_data *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_start_renderGL.  Invalid material.");
			render_data=(struct Spectrum_render_data *)NULL;
		}
	}
	else
	{
		render_data=(struct Spectrum_render_data *)NULL;
	}
	LEAVE;

	return (render_data);
} /* spectrum_start_renderGL */

int spectrum_renderGL_value(struct Spectrum *spectrum,
	struct Graphical_material *material,struct Spectrum_render_data *render_data,
	float *data)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Sets the graphics rendering state to represent the value 'data' in
accordance with the spectrum.
==============================================================================*/
{
	GLfloat values[4];
	float alpha;
	struct Colour black = {0, 0, 0}, value;
	int return_code;

	ENTER(spectrum_renderGL_value);
	if (spectrum&&material&&render_data&&(render_data->material))
	{
		render_data->data=data;
		MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)
			(render_data->material,material);
		if (spectrum->clear_colour_before_settings)
		{
			Graphical_material_set_ambient(render_data->material, &black);
			Graphical_material_set_diffuse(render_data->material, &black);			
		}
		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_activate,(void *)render_data,
			spectrum->list_of_settings);
		switch(render_data->rendering_flags)
		{
			case SPECTRUM_AMBIENT_AND_DIFFUSE:
			case SPECTRUM_DIFFUSE:
			{
				Graphical_material_get_diffuse(render_data->material, &value);
				Graphical_material_get_alpha(render_data->material, &alpha);
				values[0] = value.red;
				values[1] = value.green;
				values[2] = value.blue;
				values[3] = alpha;
				glColor4fv(values);				
			} break;
			case SPECTRUM_AMBIENT:
			{
				Graphical_material_get_ambient(render_data->material, &value);
				values[0] = value.red;
				values[1] = value.green;
				values[2] = value.blue;
				glColor3fv(values);				
			} break;
			case SPECTRUM_EMISSION:
			{
				Graphical_material_get_emission(render_data->material, &value);
				values[0] = value.red;
				values[1] = value.green;
				values[2] = value.blue;
				glColor3fv(values);				
			} break;
			case SPECTRUM_SPECULAR:
			{
				Graphical_material_get_specular(render_data->material, &value);
				values[0] = value.red;
				values[1] = value.green;
				values[2] = value.blue;
				glColor3fv(values);				
			} break;
			case 0:
			{
				/* do nothing, there still could be bands or a step though */
			} break;
			default:
			{
				direct_render_Graphical_material(render_data->material);
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_renderGL_value.  Invalid arguments given.");
		return_code=0;
	}
	LEAVE;
	
	return (return_code);
} /* spectrum_renderGL_value */

int spectrum_end_renderGL(struct Spectrum *spectrum,
	struct Spectrum_render_data *render_data)
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Resets the graphics state after rendering values on current material.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_end_renderGL);
	if (spectrum&&render_data)
	{
		switch(render_data->rendering_flags)
		{
			case SPECTRUM_AMBIENT_AND_DIFFUSE:
			case SPECTRUM_AMBIENT:
			case SPECTRUM_DIFFUSE:
			case SPECTRUM_EMISSION:
			case SPECTRUM_SPECULAR:
			{
				glDisable(GL_COLOR_MATERIAL);
			} break;
		}

		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_disable,(void *)render_data,
			spectrum->list_of_settings);
		DESTROY(Graphical_material)(&(render_data->material));
		DEALLOCATE(render_data);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_end_renderGL.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_end_renderGL */
#endif /* defined (OPENGL_API) */

struct Spectrum_calculate_range_iterator_data
{
	int first;
	float min;
	float max;
};

static int Spectrum_calculate_range_iterator(
	struct Spectrum_settings *settings, void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Iterator function to calculate the range of the spectrum by expanding
the range from the settings.  Could be in spectrum_settings.h but putting
it here means that the iterator data structure is local and these two interdependent
functions are in one place and the iterator can have local scope.
==============================================================================*/
{
	float min, max;
	int return_code;
	struct Spectrum_calculate_range_iterator_data *data;

	ENTER(spectrum_calculate_range_iterator);
	if (settings && (data = (struct Spectrum_calculate_range_iterator_data *)data_void))
	{
		min = Spectrum_settings_get_range_minimum(settings);
		max = Spectrum_settings_get_range_maximum(settings);

		if ( data->first )
		{
			data->min = min;
			data->max = max;
		}
		else
		{
			if ( min < data->min )
			{
				data->min = min;
			}
			if ( max > data->max )
			{
				data->max = max;
			}
		}
		data->first = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_calculate_range_iterator.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_calculate_range_iterator */

int Spectrum_calculate_range(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Calculates the range of the spectrum from the settings it contains and updates
the minimum and maximum contained inside it.
==============================================================================*/
{
	int return_code;
	struct Spectrum_calculate_range_iterator_data data;

	ENTER(spectrum_calculate_range);
	if (spectrum)
	{
		data.first = 1;
		data.min = 0;
		data.max = 0;
		FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_calculate_range_iterator,
			(void *)&data, spectrum->list_of_settings);
		spectrum->minimum = data.min;
		spectrum->maximum = data.max;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_calculate_range.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_calculate_range */

struct Spectrum_rerange_data
{
	float old_min, old_range, min, range;
};
static int Spectrum_rerange_iterator(
	struct Spectrum_settings *settings, void *data_void)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Iterator function to calculate the range of the spectrum by expanding
the range from the settings.  Could be in spectrum_settings.h but putting
it here means that the iterator data structure is local and these two interdependent
functions are in one place and the iterator can have local scope.
==============================================================================*/
{
	float min, max;
	int return_code;
	struct Spectrum_rerange_data *data;

	ENTER(spectrum_rerange_iterator);
	if (settings && (data = (struct Spectrum_rerange_data *)data_void))
	{
		min = Spectrum_settings_get_range_minimum(settings);
		max = Spectrum_settings_get_range_maximum(settings);

		if ( data->old_range > 0.0 )
		{
			min = data->min + data->range *
				((min - data->old_min) / data->old_range);
			max = data->min + data->range *
				((max - data->old_min) / data->old_range);
		}
		else
		{
			min = data->min;
			max = data->min + data->range;
		}

		Spectrum_settings_set_range_minimum(settings, min);
		Spectrum_settings_set_range_maximum(settings, max);

		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_rerange_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_rerange_iterator */

int Spectrum_set_minimum_and_maximum(struct Spectrum *spectrum,
	float minimum, float maximum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Expands the range of this spectrum by adjusting the range of each settings
it contains.  The ratios of the different settings are preserved.
==============================================================================*/
{
	int return_code;
	struct Spectrum_rerange_data data;

	ENTER(spectrum_set_minimum_and_maximumcalculate_range);
	if (spectrum || minimum > maximum)
	{
		if ( minimum != spectrum->minimum 
			|| maximum != spectrum->maximum )
		{
			data.old_min = spectrum->minimum;
			data.old_range = spectrum->maximum - spectrum->minimum;
			data.min = minimum;
			data.range = maximum - minimum;
			FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
				Spectrum_rerange_iterator,
				(void *)&data, spectrum->list_of_settings);
			spectrum->minimum = minimum;
			spectrum->maximum = maximum;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_set_minimum_and_maximum.  Invalid spectrum or range");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_set_minimum_and_maximum */

int spectrum_render_value_on_material(struct Spectrum *spectrum,
	struct Graphical_material *material, int number_of_data_components,
	float *data)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Uses the <spectrum> to modify the <material> to represent the <number_of_data_components>
<data> values given.
==============================================================================*/
{
	int return_code;
	struct Colour black = {0,0,0};
	struct Spectrum_render_data render_data;

	ENTER(spectrum_render_value_on_material);
	if (spectrum && material)
	{
		render_data.data = data;
		render_data.material = material;
		render_data.number_of_data_components = number_of_data_components;
		
		if (spectrum->clear_colour_before_settings)
		{
			Graphical_material_set_ambient(material, &black);
			Graphical_material_set_diffuse(material, &black);
		}

		return_code = FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_activate,(void *)&render_data,
			spectrum->list_of_settings);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_render_value_on_material.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_render_value_on_material */

int spectrum_value_to_rgb(struct Spectrum *spectrum,int number_of_data_components,
	float *data,float *red, float *green,float *blue)
/*******************************************************************************
LAST MODIFIED : 2 June 1999

DESCRIPTION :
Uses the <spectrum> to calculate RGB components to represent the 
<number_of_data_components> <data> values.
The colour returned is diffuse colour value of a spectrum modified black material.
This function is inefficient as a material is created and destroyed every time,
preferable to make your own base material, use spectrum_render_value_on_material,
and then interpret the resulting material how you want.
==============================================================================*/
{
	int return_code;
	struct Graphical_material *material;
	struct Colour black = {0,0,0}, result;

	ENTER(spectrum_value_to_rgb);
	if (spectrum)
	{
		if (material = CREATE(Graphical_material)("value_to_rgb_material"))
		{
			Graphical_material_set_diffuse(material, &black);

			if (return_code = spectrum_render_value_on_material(spectrum,
				material, number_of_data_components, data))
			{
				Graphical_material_get_diffuse(material, &result);
				
				*red = result.red;
				*green = result.green;
				*blue = result.blue;
			}
			DESTROY(Graphical_material)(&material);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_value_to_rgb.  Unable to create temporary material");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_value_to_rgb.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_value_to_rgb */

struct LIST(Spectrum_settings) *get_Spectrum_settings_list(
	struct Spectrum *spectrum )
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Returns the settings list that describes the spectrum.  This is the pointer to
the object inside the spectrum so do not destroy it, any changes to it change
that spectrum.
==============================================================================*/
{
	struct LIST(Spectrum_settings) *settings_list;

	ENTER(get_Spectrum_settings_list);
	if (spectrum)
	{
		settings_list=spectrum->list_of_settings;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Spectrum_settings_list.  Invalid argument(s)");
		settings_list=(struct LIST(Spectrum_settings) *)NULL;
	}
	LEAVE;

	return (settings_list);
} /* get_Spectrum_settings_list */

int gfx_destroy_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Executes a GFX DESTROY SPECTRUM command.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *spectrum;

	ENTER(gfx_destroy_spectrum);
	if (state && (!dummy_to_be_modified))
	{
		if (spectrum_manager=
			(struct MANAGER(Spectrum) *)spectrum_manager_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (spectrum=FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum,name)(
						current_token,spectrum_manager))
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(Spectrum)(spectrum,
							spectrum_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown spectrum: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," SPECTRUM_NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing spectrum name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_spectrum.  Missing spectrum_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_spectrum */

int Spectrum_list_commands(struct Spectrum *spectrum,
	char *command_prefix,char *command_suffix)
/*******************************************************************************
LAST MODIFIED : 21 December 1998

DESCRIPTION :
Writes the properties of the <spectrum> to the command window.
==============================================================================*/
{
	char *line_prefix,*name;
	int return_code;
	struct Spectrum_settings_list_data list_data;

	ENTER(Spectrum_list_commands);
	/* check the arguments */
	if (spectrum)
	{
		if (name=duplicate_string(spectrum->name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			/* add spectrum name to command_prefix */
			if (ALLOCATE(line_prefix,char,strlen(command_prefix)+strlen(name)+3))
			{
				sprintf(line_prefix,"%s %s ",command_prefix,name);
				display_message(INFORMATION_MESSAGE,line_prefix);
				display_message(INFORMATION_MESSAGE,"clear");
				if (spectrum->clear_colour_before_settings)
				{
					display_message(INFORMATION_MESSAGE," overwrite_colour");
				}
				else
				{
					display_message(INFORMATION_MESSAGE," overlay_colour");
				}
				if (command_suffix)
				{
					display_message(INFORMATION_MESSAGE,command_suffix);
				}
				display_message(INFORMATION_MESSAGE,"\n");
				list_data.settings_string_detail=SPECTRUM_SETTINGS_STRING_COMPLETE;
				list_data.line_prefix=line_prefix;
				list_data.line_suffix=command_suffix;
				return_code=FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
					Spectrum_settings_list_contents,(void *)&list_data,
					spectrum->list_of_settings);
				DEALLOCATE(line_prefix);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_list_commands.  Unable to allocate string");
				return_code=0;
			}
			DEALLOCATE(name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Spectrum_list_commands.  Unable to duplicate string");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_list_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_list_commands */

int Spectrum_list_contents(struct Spectrum *spectrum,void *dummy)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Writes the properties of the <spectrum> to the command window.
==============================================================================*/
{
	enum Spectrum_simple_type simple_type;
	int return_code;
	struct Spectrum_settings_list_data list_data;

	ENTER(Spectrum_list_contents);
	/* check the arguments */
	if (spectrum && (!dummy))
	{
		display_message(INFORMATION_MESSAGE,"spectrum : ");
		display_message(INFORMATION_MESSAGE,spectrum->name);
		display_message(INFORMATION_MESSAGE,"\n");
		simple_type = Spectrum_get_simple_type(spectrum);
		switch(simple_type)
		{
			case BLUE_TO_RED_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: BLUE_TO_RED\n");
			} break;
			case RED_TO_BLUE_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: RED_TO_BLUE\n");
			} break;
			case LOG_BLUE_TO_RED_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: LOG_BLUE_TO_RED\n");
			} break;
			case LOG_RED_TO_BLUE_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: LOG_RED_TO_BLUE\n");
			} break;
			case BLUE_WHITE_RED_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: BLUE_WHITE_RED\n");
			} break;
		}
		display_message(INFORMATION_MESSAGE,"  minimum=%.3g, maximum=%.3g\n",
			spectrum->minimum,spectrum->maximum);
		if (spectrum->clear_colour_before_settings)
		{
			display_message(INFORMATION_MESSAGE,"  clear before settings\n");
		}
		list_data.settings_string_detail=SPECTRUM_SETTINGS_STRING_COMPLETE_PLUS;
		list_data.line_prefix="  ";
		list_data.line_suffix="";
		return_code=FOR_EACH_OBJECT_IN_LIST(Spectrum_settings)(
			Spectrum_settings_list_contents,(void *)&list_data,
			spectrum->list_of_settings);
		display_message(INFORMATION_MESSAGE,"  access count=%d\n",
			spectrum->access_count);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_list_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_list_contents */

struct Spectrum *CREATE(Spectrum)(char *name)
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Allocates memory and assigns fields for a Spectrum object.
==============================================================================*/
{
	struct Spectrum *spectrum;

	ENTER(CREATE(Spectrum));
	if (name)
	{
		if (ALLOCATE(spectrum,struct Spectrum,1))
		{
			spectrum->maximum=0;
			spectrum->minimum=0;
			spectrum->clear_colour_before_settings = 1;
			spectrum->access_count=0;
			if (spectrum->list_of_settings=CREATE(LIST(Spectrum_settings))())
			{
				if (name)
				{
					if (ALLOCATE(spectrum->name,char,strlen(name)+1))
					{
						strcpy(spectrum->name,name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Spectrum).  Cannot allocate spectrum name");
						DEALLOCATE(spectrum);
						spectrum=(struct Spectrum *)NULL;
					}
				}
				else
				{
					spectrum->name=NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Spectrum).  Cannot create settings list");
				DEALLOCATE(spectrum);
				spectrum=(struct Spectrum *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Spectrum).  Cannot allocate spectrum");
			spectrum=(struct Spectrum *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Spectrum).  Missing name");
		spectrum=(struct Spectrum *)NULL;
	}
	LEAVE;

	return (spectrum);
} /* CREATE(Spectrum) */

int DESTROY(Spectrum)(struct Spectrum **spectrum_ptr)
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Frees the memory for the fields of <**spectrum>, frees the memory for
<**spectrum> and sets <*spectrum> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Spectrum));
	if (spectrum_ptr)
	{
		if (*spectrum_ptr)
		{
			if ((*spectrum_ptr)->name)
			{
				DEALLOCATE((*spectrum_ptr)->name);
			}
			DESTROY(LIST(Spectrum_settings))(&((*spectrum_ptr)->list_of_settings));
			DEALLOCATE(*spectrum_ptr);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Spectrum).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Spectrum) */

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Spectrum)
