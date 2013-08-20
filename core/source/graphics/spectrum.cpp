/*******************************************************************************
FILE : spectrum.c

LAST MODIFIED : 22 January 2002

DESCRIPTION :
Spectrum functions and support code.
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
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "zinc/spectrum.h"
#include "zinc/status.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/object.h"
#include "general/mystring.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_module.h"
#include "graphics/material.h"
#include "graphics/spectrum_component.h"
#include "graphics/spectrum.h"
#include "general/message.h"
#include "general/enumerator_conversion.hpp"
#include "graphics/render_gl.h"
#include "graphics/spectrum.hpp"

/*
Module types
------------
*/

struct Cmiss_spectrum *Cmiss_spectrum_create_private();

struct Cmiss_spectrum_module
{

private:

	struct MANAGER(Cmiss_spectrum) *spectrumManager;
	Cmiss_spectrum *defaultSpectrum;
	int access_count;

	Cmiss_spectrum_module() :
		spectrumManager(CREATE(MANAGER(Cmiss_spectrum))()),
		defaultSpectrum(0),
		access_count(1)
	{
	}

	~Cmiss_spectrum_module()
	{
		if (defaultSpectrum)
		{
			DEACCESS(Cmiss_spectrum)(&(this->defaultSpectrum));
		}
		DESTROY(MANAGER(Cmiss_spectrum))(&(this->spectrumManager));
	}

public:

	static Cmiss_spectrum_module *create()
	{
		return new Cmiss_spectrum_module();
	}

	Cmiss_spectrum_module *access()

	{
		++access_count;
		return this;
	}

	static int deaccess(Cmiss_spectrum_module* &spectrum_module)
	{
		if (spectrum_module)
		{
			--(spectrum_module->access_count);
			if (spectrum_module->access_count <= 0)
			{
				delete spectrum_module;
			}
			spectrum_module = 0;
			return CMISS_OK;
		}
		return CMISS_ERROR_ARGUMENT;
	}

	struct MANAGER(Cmiss_spectrum) *getManager()
	{
		return this->spectrumManager;
	}

	int beginChange()
	{
		return MANAGER_BEGIN_CACHE(Cmiss_spectrum)(this->spectrumManager);
	}

	int endChange()
	{
		return MANAGER_END_CACHE(Cmiss_spectrum)(this->spectrumManager);
	}

	Cmiss_spectrum_id createSpectrum()
	{
		Cmiss_spectrum_id spectrum = NULL;
		char temp_name[20];
		int i = NUMBER_IN_MANAGER(Cmiss_spectrum)(this->spectrumManager);
		do
		{
			i++;
			sprintf(temp_name, "temp%d",i);
		}
		while (FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_spectrum,name)(temp_name,
			this->spectrumManager));
		spectrum = Cmiss_spectrum_create_private();
		Cmiss_spectrum_set_name(spectrum, temp_name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_spectrum)(spectrum, this->spectrumManager))
		{
			DEACCESS(Cmiss_spectrum)(&spectrum);
		}
		return spectrum;
	}

	Cmiss_spectrum *findSpectrumByName(const char *name)
	{
		Cmiss_spectrum *spectrum = FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_spectrum,name)(name,
			this->spectrumManager);
		if (spectrum)
		{
			return ACCESS(Cmiss_spectrum)(spectrum);
		}
		return 0;
	}

	Cmiss_spectrum *getDefaultSpectrum()
	{
		if (this->defaultSpectrum)
		{
			ACCESS(Cmiss_spectrum)(this->defaultSpectrum);
			return this->defaultSpectrum;
		}
		else
		{
			const char *default_spectrum_name = "default";
			struct Cmiss_spectrum *spectrum = findSpectrumByName(default_spectrum_name);
			if (0 == spectrum)
			{
				spectrum = createSpectrum();
				Cmiss_spectrum_set_name(spectrum, "default");
				Spectrum_set_simple_type(spectrum,	BLUE_TO_RED_SPECTRUM);
				Spectrum_set_minimum_and_maximum(spectrum,0,1);
			}
			if (spectrum)
			{
				Cmiss_spectrum_set_managed(spectrum, true);
				setDefaultSpectrum(spectrum);
				return spectrum;
			}
		}
		return 0;
	}

	int setDefaultSpectrum(Cmiss_spectrum *spectrum)
	{
		REACCESS(Cmiss_spectrum)(&this->defaultSpectrum, spectrum);
		return CMISS_OK;
	}

};

Cmiss_spectrum_module_id Cmiss_spectrum_module_create()
{
	return Cmiss_spectrum_module::create();
}

Cmiss_spectrum_module_id Cmiss_spectrum_module_access(
	Cmiss_spectrum_module_id spectrum_module)
{
	if (spectrum_module)
		return spectrum_module->access();
	return 0;
}

int Cmiss_spectrum_module_destroy(Cmiss_spectrum_module_id *spectrum_module_address)
{
	if (spectrum_module_address)
		return Cmiss_spectrum_module::deaccess(*spectrum_module_address);
	return CMISS_ERROR_ARGUMENT;
}

Cmiss_spectrum_id Cmiss_spectrum_module_create_spectrum(
	Cmiss_spectrum_module_id spectrum_module)
{
	if (spectrum_module)
		return spectrum_module->createSpectrum();
	return 0;
}

struct MANAGER(Cmiss_spectrum) *Cmiss_spectrum_module_get_manager(
	Cmiss_spectrum_module_id spectrum_module)
{
	if (spectrum_module)
		return spectrum_module->getManager();
	return 0;
}

int Cmiss_spectrum_module_begin_change(Cmiss_spectrum_module_id spectrum_module)
{
	if (spectrum_module)
		return spectrum_module->beginChange();
	return CMISS_ERROR_ARGUMENT;
}

int Cmiss_spectrum_module_end_change(Cmiss_spectrum_module_id spectrum_module)
{
	if (spectrum_module)
		return spectrum_module->endChange();
	return CMISS_ERROR_ARGUMENT;
}

Cmiss_spectrum_id Cmiss_spectrum_module_find_spectrum_by_name(
	Cmiss_spectrum_module_id spectrum_module, const char *name)
{
	if (spectrum_module)
		return spectrum_module->findSpectrumByName(name);
	return 0;
}

Cmiss_spectrum_id Cmiss_spectrum_module_get_default_spectrum(
	Cmiss_spectrum_module_id spectrum_module)
{
	if (spectrum_module)
		return spectrum_module->getDefaultSpectrum();
	return 0;
}

int Cmiss_spectrum_module_set_default_spectrum(
	Cmiss_spectrum_module_id spectrum_module,
	Cmiss_spectrum_id spectrum)
{
	if (spectrum_module)
		return spectrum_module->setDefaultSpectrum(spectrum);
	return 0;
}


struct Cmiss_spectrum *Cmiss_spectrum_create_private()
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Allocates memory and assigns fields for a Spectrum object.
==============================================================================*/
{
	struct Spectrum *spectrum = 0;
	if (ALLOCATE(spectrum,struct Spectrum,1))
	{
		spectrum->maximum=0;
		spectrum->minimum=0;
		spectrum->overwrite_colour = true;
		spectrum->manager = (struct MANAGER(Spectrum) *)NULL;
		spectrum->manager_change_status = MANAGER_CHANGE_NONE(Spectrum);
		spectrum->access_count=1;
		spectrum->colour_lookup_texture = (struct Texture *)NULL;
		spectrum->is_managed_flag = false;
		spectrum->list_of_components=CREATE(LIST(Cmiss_spectrum_component))();
		spectrum->name=NULL;
		spectrum->cache = 0;
		spectrum->changed = 0;
		if (!spectrum->list_of_components)
		{
			DEALLOCATE(spectrum);
			spectrum=(struct Spectrum *)NULL;
		}
	}
	else
	{
		spectrum=(struct Spectrum *)NULL;
	}
	return spectrum;
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
			if ((*spectrum_ptr)->colour_lookup_texture)
			{
				DEACCESS(Texture)(&((*spectrum_ptr)->colour_lookup_texture));
			}
			DESTROY(LIST(Cmiss_spectrum_component))(&((*spectrum_ptr)->list_of_components));
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

FULL_DECLARE_INDEXED_LIST_TYPE(Spectrum);

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Spectrum, Cmiss_spectrum_module, void *);

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Spectrum,name,const char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Spectrum)

/*
Global functions
----------------
*/
DECLARE_OBJECT_FUNCTIONS(Spectrum)

DECLARE_INDEXED_LIST_FUNCTIONS(Spectrum)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Spectrum,name,const char *,strcmp)

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
			return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Spectrum,name)(destination, source);
			if (return_code)
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
		destination->overwrite_colour =
			source->overwrite_colour;

		REACCESS(Texture)(&destination->colour_lookup_texture,
			source->colour_lookup_texture);

		/* empty original list_of_components */
		REMOVE_ALL_OBJECTS_FROM_LIST(Cmiss_spectrum_component)(
			destination->list_of_components);
		/* put copy of each component in source list in destination list */
		FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
			Cmiss_spectrum_component_copy_and_put_in_list,
			(void *)destination->list_of_components,source->list_of_components);

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

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Spectrum,name,const char *)
{
	char *destination_name = NULL;
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

DECLARE_MANAGER_FUNCTIONS(Spectrum,manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Spectrum,manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Spectrum,name,const char *,manager)

DECLARE_MANAGER_OWNER_FUNCTIONS(Spectrum, struct Cmiss_spectrum_module)

int Spectrum_manager_set_owner(struct MANAGER(Spectrum) *manager,
	struct Cmiss_spectrum_module *spectrum_module)
{
	return MANAGER_SET_OWNER(Spectrum)(manager, spectrum_module);
}
/*
Global functions
----------------
*/

struct Cmiss_spectrum_component *Cmiss_spectrum_get_component_at_position(
	 struct Spectrum *spectrum,int position)
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Wrapper for accessing the component in <spectrum>.
==============================================================================*/
{
	 struct Cmiss_spectrum_component *component;

	 ENTER(get_component_at_position_in_GT_element_group);
	 if (spectrum)
	 {
			component=FIND_BY_IDENTIFIER_IN_LIST(Cmiss_spectrum_component,
				 position)(position,spectrum->list_of_components);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Cmiss_spectrum_get_component_at_position.  Invalid arguments");
			component=(struct Cmiss_spectrum_component *)NULL;
	 }
	 LEAVE;

	 return (component);
} /* get_component_at_position_in_GT_element_group */

int Spectrum_set_simple_type(struct Spectrum *spectrum,
	enum Spectrum_simple_type type)
/*******************************************************************************
LAST MODIFIED : 7 February 2002

DESCRIPTION :
A convienience routine that allows a spectrum to be automatically set into
some predetermined simple types.
==============================================================================*/
{
	struct LIST(Cmiss_spectrum_component) *spectrum_component_list;
	struct Cmiss_spectrum_component *component, *second_component;
	ZnReal maximum, minimum;
	int number_in_list, return_code;

	ENTER(Spectrum_set_simple_type);
	if (spectrum)
	{
		Cmiss_spectrum_begin_change(spectrum);
		return_code = 1;
		minimum = spectrum->minimum;
		maximum = spectrum->maximum;
		switch(type)
		{
			case RED_TO_BLUE_SPECTRUM:
			case BLUE_TO_RED_SPECTRUM:
			{
				spectrum_component_list = get_Cmiss_spectrum_component_list(spectrum);
				number_in_list = NUMBER_IN_LIST(Cmiss_spectrum_component)(spectrum_component_list);
				if ( number_in_list > 0 )
				{
					REMOVE_ALL_OBJECTS_FROM_LIST(Cmiss_spectrum_component)(spectrum_component_list);
				}
				component = CREATE(Cmiss_spectrum_component)();

				Spectrum_add_component(spectrum, component, /* end of list = 0 */0);
				Cmiss_spectrum_component_set_scale_type(component, CMISS_SPECTRUM_COMPONENT_SCALE_LINEAR);
				component->is_field_lookup = false;
				Cmiss_spectrum_component_set_colour_mapping(component,
					CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RAINBOW);
				Cmiss_spectrum_component_set_extend_above(component,true);
				Cmiss_spectrum_component_set_extend_below(component, true);
				switch (type)
				{
					case RED_TO_BLUE_SPECTRUM:
					{
						Cmiss_spectrum_component_set_colour_reverse(component,false);
					} break;
					case BLUE_TO_RED_SPECTRUM:
					{
						Cmiss_spectrum_component_set_colour_reverse(component, true);
					} break;
					default:
					{
					} break;
				}
				Cmiss_spectrum_component_destroy(&component);
			} break;
			case LOG_RED_TO_BLUE_SPECTRUM:
			case LOG_BLUE_TO_RED_SPECTRUM:
			{
				spectrum_component_list = get_Cmiss_spectrum_component_list(spectrum);
				number_in_list = NUMBER_IN_LIST(Cmiss_spectrum_component)(spectrum_component_list);
				if ( number_in_list > 0 )
				{
					REMOVE_ALL_OBJECTS_FROM_LIST(Cmiss_spectrum_component)(spectrum_component_list);
				}
				component = CREATE(Cmiss_spectrum_component)();
				second_component = CREATE(Cmiss_spectrum_component)();
				Spectrum_add_component(spectrum, component, /* end of list = 0 */0);
				Spectrum_add_component(spectrum, second_component, /* end of list = 0 */0);


				Cmiss_spectrum_component_set_scale_type(component, CMISS_SPECTRUM_COMPONENT_SCALE_LOG);
				component->is_field_lookup = false;
				Cmiss_spectrum_component_set_exaggeration(component, 1.0);
				Cmiss_spectrum_component_set_colour_mapping(component, CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RAINBOW);
				Cmiss_spectrum_component_set_range_minimum(component, -1.0);
				Cmiss_spectrum_component_set_range_maximum(component, 0.0);

				Cmiss_spectrum_component_set_scale_type(second_component, CMISS_SPECTRUM_COMPONENT_SCALE_LOG);
				second_component->is_field_lookup = false;
				Cmiss_spectrum_component_set_exaggeration(second_component, -1.0);
				Cmiss_spectrum_component_set_colour_mapping(second_component, CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RAINBOW);
				Cmiss_spectrum_component_set_range_minimum(second_component, 0.0);
				Cmiss_spectrum_component_set_range_maximum(second_component, 1.0);

				Cmiss_spectrum_component_set_extend_below(component, true);
				Cmiss_spectrum_component_set_extend_above(second_component, true);

				switch(type)
				{
					case LOG_RED_TO_BLUE_SPECTRUM:
					{
						Cmiss_spectrum_component_set_colour_reverse(component, false);
						Cmiss_spectrum_component_set_colour_minimum(component, 0);
						Cmiss_spectrum_component_set_colour_maximum(component, 0.5);
						Cmiss_spectrum_component_set_colour_reverse(second_component, false);
						Cmiss_spectrum_component_set_colour_minimum(second_component, 0.5);
						Cmiss_spectrum_component_set_colour_maximum(second_component, 1.0);
					} break;
					case LOG_BLUE_TO_RED_SPECTRUM:
					{
						Cmiss_spectrum_component_set_colour_reverse(component, true);
						Cmiss_spectrum_component_set_colour_minimum(component, 0.5);
						Cmiss_spectrum_component_set_colour_maximum(component, 1.0);
						Cmiss_spectrum_component_set_colour_reverse(second_component, true);
						Cmiss_spectrum_component_set_colour_minimum(second_component, 0.0);
						Cmiss_spectrum_component_set_colour_maximum(second_component, 0.5);
					} break;
					default:
					{
					} break;
				}
				Cmiss_spectrum_component_destroy(&component);
				Cmiss_spectrum_component_destroy(&second_component);
			} break;
			case BLUE_WHITE_RED_SPECTRUM:
			{
				spectrum_component_list = get_Cmiss_spectrum_component_list(spectrum);
				number_in_list = NUMBER_IN_LIST(Cmiss_spectrum_component)(spectrum_component_list);
				if ( number_in_list > 0 )
				{
					REMOVE_ALL_OBJECTS_FROM_LIST(Cmiss_spectrum_component)(spectrum_component_list);
				}
				component = CREATE(Cmiss_spectrum_component)();
				second_component = CREATE(Cmiss_spectrum_component)();
				Spectrum_add_component(spectrum, component, /* end of list = 0 */0);
				Spectrum_add_component(spectrum, second_component, /* end of list = 0 */0);

				Cmiss_spectrum_component_set_scale_type(component, CMISS_SPECTRUM_COMPONENT_SCALE_LOG);
				component->is_field_lookup = false;
				Cmiss_spectrum_component_set_exaggeration(component, -10.0);
				Cmiss_spectrum_component_set_range_minimum(component, -1.0);
				Cmiss_spectrum_component_set_range_maximum(component, 0.0);
				Cmiss_spectrum_component_set_colour_reverse(component, true);
				/* fix the maximum (white ) at zero */
				Cmiss_spectrum_component_set_fix_maximum_flag(component,1);
				Cmiss_spectrum_component_set_extend_below(component, true);
				Cmiss_spectrum_component_set_colour_mapping(component, CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_BLUE);
				Cmiss_spectrum_component_set_colour_minimum(component, 0);
				Cmiss_spectrum_component_set_colour_maximum(component, 1);

				Cmiss_spectrum_component_set_scale_type(second_component, CMISS_SPECTRUM_COMPONENT_SCALE_LOG);
				second_component->is_field_lookup = false;
				Cmiss_spectrum_component_set_exaggeration(second_component, 10.0);
				Cmiss_spectrum_component_set_range_minimum(second_component, 0.0);
				Cmiss_spectrum_component_set_range_maximum(second_component, 1.0);
				/* fix the minimum (white ) at zero */
				Cmiss_spectrum_component_set_fix_minimum_flag(second_component,1);
				Cmiss_spectrum_component_set_extend_above(second_component, true);
				Cmiss_spectrum_component_set_colour_mapping(second_component, CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_RED);
				Cmiss_spectrum_component_set_colour_minimum(second_component, 0);
				Cmiss_spectrum_component_set_colour_maximum(second_component, 1);
			} break;
			Cmiss_spectrum_component_destroy(&component);
			Cmiss_spectrum_component_destroy(&second_component);
			default:
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_set_simple_type.  Unknown simple spectrum type");
				return_code=0;
			} break;
		}
		/* Rerange the component so that it matches the minimum and maximum of the spectrum */
		Spectrum_calculate_range(spectrum);
		Spectrum_set_minimum_and_maximum(spectrum, minimum, maximum);
		Cmiss_spectrum_end_change(spectrum);
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
	struct LIST(Cmiss_spectrum_component) *spectrum_component_list;
	struct Cmiss_spectrum_component *component, *second_component;
	enum Cmiss_spectrum_component_scale_type component_scale, second_component_scale;
	int number_in_list;
	bool reverse, second_reverse;
	enum Cmiss_spectrum_component_colour_mapping colour_mapping, second_colour_mapping;
	enum Spectrum_simple_type type;

	ENTER(Spectrum_get_simple_type);

	if (spectrum)
	{
		type = UNKNOWN_SPECTRUM;

		spectrum_component_list = get_Cmiss_spectrum_component_list(spectrum);
		number_in_list = NUMBER_IN_LIST(Cmiss_spectrum_component)(spectrum_component_list);
		switch( number_in_list )
		{
			case 1:
			{
				component = FIRST_OBJECT_IN_LIST_THAT(Cmiss_spectrum_component)
					((LIST_CONDITIONAL_FUNCTION(Cmiss_spectrum_component) *)NULL, NULL,
					spectrum_component_list);
				component_scale = Cmiss_spectrum_component_get_scale_type(component);
				reverse = Cmiss_spectrum_component_is_colour_reverse(component);
				colour_mapping = Cmiss_spectrum_component_get_colour_mapping(component);

				if ( component_scale == CMISS_SPECTRUM_COMPONENT_SCALE_LINEAR )
				{
					if ( colour_mapping == CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RAINBOW )
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
				component = FIND_BY_IDENTIFIER_IN_LIST(Cmiss_spectrum_component,position)
					(1, spectrum_component_list);
				second_component = FIND_BY_IDENTIFIER_IN_LIST(Cmiss_spectrum_component,position)
					(2, spectrum_component_list);
				if ( component && second_component )
				{
					component_scale = Cmiss_spectrum_component_get_scale_type(component);
					reverse = Cmiss_spectrum_component_is_colour_reverse(component);
					colour_mapping = Cmiss_spectrum_component_get_colour_mapping(component);
					second_component_scale = Cmiss_spectrum_component_get_scale_type(second_component);
					second_reverse = Cmiss_spectrum_component_is_colour_reverse(second_component);
					second_colour_mapping = Cmiss_spectrum_component_get_colour_mapping
						(second_component);

					if((component_scale == CMISS_SPECTRUM_COMPONENT_SCALE_LOG)
						&& (second_component_scale == CMISS_SPECTRUM_COMPONENT_SCALE_LOG))
					{
						if ((colour_mapping == CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RAINBOW)
							&& (second_colour_mapping == CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RAINBOW))
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
						else if ((colour_mapping == CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_BLUE)
							&& (second_colour_mapping == CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_RED))
						{
							type = BLUE_WHITE_RED_SPECTRUM;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Spectrum_set_simple_type.  Bad position numbers in component");
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

int Spectrum_add_component(struct Spectrum *spectrum,
	struct Cmiss_spectrum_component *component,int position)
/*******************************************************************************
LAST MODIFIED : 24 June 1998

DESCRIPTION :
Adds the <component> to <spectrum> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the component to be added at its end, with a
position one greater than the last.
==============================================================================*/
{
	int return_code = 0;

	if (spectrum&&component&&spectrum->list_of_components)
	{
		if (component->spectrum == 0)
			component->spectrum = spectrum;
		struct LIST(Cmiss_spectrum_component) *list_of_components = spectrum->list_of_components;
		if (!IS_OBJECT_IN_LIST(Cmiss_spectrum_component)(component,list_of_components))
		{
			return_code=1;
			int last_position=NUMBER_IN_LIST(Cmiss_spectrum_component)(list_of_components);
			if ((1>position)||(position>last_position))
			{
				/* add to end of list */
				position=last_position+1;
			}
			ACCESS(Cmiss_spectrum_component)(component);
			while (return_code&&component)
			{
				component->position=position;
				/* is there already a component with that position? */
				struct Cmiss_spectrum_component *component_in_way=FIND_BY_IDENTIFIER_IN_LIST(Cmiss_spectrum_component,
					position)(position,list_of_components);
				if (component_in_way)
				{
					/* remove the old component to make way for the new */
					ACCESS(Cmiss_spectrum_component)(component_in_way);
					REMOVE_OBJECT_FROM_LIST(Cmiss_spectrum_component)(
						component_in_way,list_of_components);
				}
				if (ADD_OBJECT_TO_LIST(Cmiss_spectrum_component)(component,list_of_components))
				{
					DEACCESS(Cmiss_spectrum_component)(&component);
					/* the old, in-the-way component now become the new component */
					component=component_in_way;
					position++;
				}
				else
				{
					DEACCESS(Cmiss_spectrum_component)(&component);
					if (component_in_way)
					{
						DEACCESS(Cmiss_spectrum_component)(&component_in_way);
					}
					return_code=0;
				}
			}
		}
		else
		{
			return_code=0;
		}
		Spectrum_calculate_range(spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_add_component.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_add_component */

int Cmiss_spectrum_remove_component(Cmiss_spectrum_id spectrum,
	struct Cmiss_spectrum_component *component)
/*******************************************************************************
LAST MODIFIED : 24 June 1998

DESCRIPTION :
Removes the <component> from <spectrum> and decrements the position
of all subsequent component.
==============================================================================*/
{
	int return_code = 0;

	if (spectrum&&component&&spectrum->list_of_components)
	{
		struct LIST(Cmiss_spectrum_component) *list_of_components = spectrum->list_of_components;
		if (IS_OBJECT_IN_LIST(Cmiss_spectrum_component)(component,list_of_components))
		{
			return_code = 1;
			int next_position=component->position+1;
			return_code=REMOVE_OBJECT_FROM_LIST(Cmiss_spectrum_component)(
				component,list_of_components);
			/* decrement position of all remaining component */
			while (return_code&&(component=FIND_BY_IDENTIFIER_IN_LIST(
				Cmiss_spectrum_component,position)(next_position,list_of_components)))
			{
				ACCESS(Cmiss_spectrum_component)(component);
				REMOVE_OBJECT_FROM_LIST(Cmiss_spectrum_component)(component,list_of_components);
				(component->position)--;
				if (ADD_OBJECT_TO_LIST(Cmiss_spectrum_component)(component,list_of_components))
				{
					next_position++;
				}
				else
				{
					return_code=0;
				}
				DEACCESS(Cmiss_spectrum_component)(&component);
			}
		}
		else
		{

			return_code=0;
		}
		Spectrum_calculate_range(spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_spectrum_remove_all_components.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Cmiss_spectrum_remove_all_components */


int Cmiss_spectrum_get_component_position(struct Spectrum *spectrum,
	struct Cmiss_spectrum_component *component)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Returns the position of <component> in <spectrum>.
==============================================================================*/
{
	if (spectrum&&component&&spectrum->list_of_components)
	{
		if (IS_OBJECT_IN_LIST(Cmiss_spectrum_component)(component,spectrum->list_of_components))
		{
			return component->position;
		}
	}
	return 0;

} /* Cmiss_spectrum_get_component_position */

int set_Spectrum_minimum(struct Spectrum *spectrum,ZnReal minimum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum minimum.
==============================================================================*/
{
	ZnReal maximum;
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

int set_Spectrum_maximum(struct Spectrum *spectrum,ZnReal maximum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum maximum.
==============================================================================*/
{
	ZnReal minimum;
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

int Spectrum_get_number_of_data_components(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 29 September 2006

DESCRIPTION :
Returns the number_of_components used by the spectrum.
==============================================================================*/
{
	int number_of_components;

	ENTER(Spectrum_get_number_of_data_components);
	if (spectrum)
	{
		number_of_components = 0;
		FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
			Cmiss_spectrum_component_expand_maximum_component_index,
			(void *)&number_of_components, spectrum->list_of_components);

		/* indices start at 0, so add one for number */
		number_of_components++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_number_of_data_components.  Invalid spectrum object.");
		number_of_components = 0;
	}
	LEAVE;

	return (number_of_components);
} /* Spectrum_get_number_of_data_components */

enum Spectrum_colour_components
	Spectrum_get_colour_components(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Returns a bit mask for the colour components modified by the spectrum.
==============================================================================*/
{
	enum Spectrum_colour_components colour_components;

	ENTER(Spectrum_get_colour_components);
	if (spectrum)
	{
		colour_components = SPECTRUM_COMPONENT_NONE;
		FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
			Cmiss_spectrum_component_get_colour_components,
			(void *)&colour_components, spectrum->list_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_colour_components.  Invalid spectrum object.");
		colour_components = SPECTRUM_COMPONENT_NONE;
	}
	LEAVE;

	return (colour_components);
} /* Spectrum_get_colour_components */

char *Spectrum_get_name(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
Returns the string of the spectrum.
==============================================================================*/
{
	char *name;

	ENTER(Spectrum_get_name);
	if (spectrum)
	{
		 name = spectrum->name;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_get_name.  Invalid spectrum object.");
		name = (char *)NULL;
	}
	LEAVE;

	return (name);
} /* Spectrum_get_name */

bool Cmiss_spectrum_is_material_overwrite(Cmiss_spectrum_id spectrum)
{
	int overwrite_material = 0;

	if (spectrum)
	{
		overwrite_material = spectrum->overwrite_colour;
	}

	return (overwrite_material);
}

int Cmiss_spectrum_set_material_overwrite(Cmiss_spectrum_id spectrum,
	bool overwrite)
{
	if (spectrum)
	{
		if (spectrum->overwrite_colour != overwrite)
		{
			spectrum->overwrite_colour = overwrite;
			Cmiss_spectrum_changed(spectrum);
		}
		return CMISS_OK;
	}

	return CMISS_ERROR_ARGUMENT;
}

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
	ZnReal alpha;
	struct Colour value;
	struct Spectrum_render_data *render_data;

	ENTER(spectrum_start_renderGL);
	if (spectrum)
	{
		if (material)
		{
			if (ALLOCATE(render_data,struct Spectrum_render_data,1))
			{
				render_data->number_of_data_components = number_of_data_components;

				if (spectrum->overwrite_colour)
				{
					render_data->material_rgba[0] = 0.0;
					render_data->material_rgba[1] = 0.0;
					render_data->material_rgba[2] = 0.0;
					render_data->material_rgba[3] = 1.0;
				}
				else
				{
					Graphical_material_get_diffuse(material, &value);
					Graphical_material_get_alpha(material, &alpha);
					render_data->material_rgba[0] = value.red;
					render_data->material_rgba[1] = value.green;
					render_data->material_rgba[2] = value.blue;
					render_data->material_rgba[3] = alpha;
				}

				FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
					Cmiss_spectrum_component_enable,(void *)render_data,
					spectrum->list_of_components);

				/* Always ambient and diffuse */
				glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
				glEnable(GL_COLOR_MATERIAL);
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
	GLfloat *data)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Sets the graphics rendering state to represent the value 'data' in
accordance with the spectrum.
==============================================================================*/
{
	GLfloat rgba[4];
	int return_code = 1;

	ENTER(spectrum_renderGL_value);
	USE_PARAMETER(material);
	if (spectrum&&render_data)
	{
		rgba[0] = render_data->material_rgba[0];
		rgba[1] = render_data->material_rgba[1];
		rgba[2] = render_data->material_rgba[2];
		rgba[3] = render_data->material_rgba[3];

		render_data->rgba = rgba;
		render_data->data = data;

		FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
			Cmiss_spectrum_component_activate,(void *)render_data,
			spectrum->list_of_components);

		glColor4fv(rgba);
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
		glDisable(GL_COLOR_MATERIAL);

		FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
			Cmiss_spectrum_component_disable,(void *)render_data,
			spectrum->list_of_components);
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
	ZnReal min;
	ZnReal max;
};

static int Spectrum_calculate_range_iterator(
	struct Cmiss_spectrum_component *component, void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Iterator function to calculate the range of the spectrum by expanding
the range from the component.  Could be in spectrum_component.h but putting
it here means that the iterator data structure is local and these two interdependent
functions are in one place and the iterator can have local scope.
==============================================================================*/
{
	ZnReal min, max;
	int fixed_minimum, fixed_maximum, return_code;
	struct Spectrum_calculate_range_iterator_data *data;

	ENTER(spectrum_calculate_range_iterator);
	if (component && (data = (struct Spectrum_calculate_range_iterator_data *)data_void))
	{
		min = Cmiss_spectrum_component_get_range_minimum(component);
		max = Cmiss_spectrum_component_get_range_maximum(component);
		fixed_minimum = Cmiss_spectrum_component_get_fix_minimum_flag(component);
		fixed_maximum = Cmiss_spectrum_component_get_fix_maximum_flag(component);

		if ( data->first )
		{
// 			if (!fixed_minimum && !fixed_maximum)
// 			{
				data->min = min;
				data->max = max;
				data->first = 0;
// 			}
// 			else if (!fixed_minimum)
// 			{
// 				data->min = min;
// 				data->max = min;
// 				data->first = 0;
// 			}
// 			else if (!fixed_maximum)
// 			{
// 				data->min = max;
// 				data->max = max;
// 				data->first = 0;
// 			}
		}
		else
		{
			if (!fixed_minimum && (min < data->min))
			{
				data->min = min;
			}
			if (!fixed_maximum && (max > data->max))
			{
				data->max = max;
			}
		}
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
Calculates the range of the spectrum from the component it contains and updates
the minimum and maximum contained inside it.
==============================================================================*/
{
	int return_code = 0;
	struct Spectrum_calculate_range_iterator_data data;

	ENTER(spectrum_calculate_range);
	if (spectrum)
	{
		data.first = 1;
		data.min = 0;
		data.max = 0;
		FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
			Spectrum_calculate_range_iterator,
			(void *)&data, spectrum->list_of_components);
		if (!data.first)
		{
			spectrum->minimum = data.min;
			spectrum->maximum = data.max;
		}
		return_code = 1;
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
	ZnReal old_min, old_range, old_max, min, range, max;
};
static int Spectrum_rerange_iterator(
	struct Cmiss_spectrum_component *component, void *data_void)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Iterator function to calculate the range of the spectrum by expanding
the range from the component.  Could be in spectrum_component.h but putting
it here means that the iterator data structure is local and these two interdependent
functions are in one place and the iterator can have local scope.
==============================================================================*/
{
	ZnReal min, max;
	int return_code;
	struct Spectrum_rerange_data *data;

	ENTER(spectrum_rerange_iterator);
	if (component && (data = (struct Spectrum_rerange_data *)data_void))
	{
		min = Cmiss_spectrum_component_get_range_minimum(component);
		max = Cmiss_spectrum_component_get_range_maximum(component);

		if ( data->old_range > 0.0 )
		{
			min = data->min + data->range *
				((min - data->old_min) / data->old_range);
			max = data->max - data->range *
				((data->old_max - max) / data->old_range);
		}
		else
		{
			min = data->min;
			max = data->max;
		}

		Cmiss_spectrum_component_set_range_minimum(component, min);
		Cmiss_spectrum_component_set_range_maximum(component, max);

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

static int Cmiss_spectrum_inform_clients(Cmiss_spectrum_id spectrum)
{
	if (spectrum && spectrum->manager)
	{
		spectrum->changed = 0;
		return MANAGED_OBJECT_CHANGE(Spectrum)(spectrum,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Spectrum));
	}
	return 0;
}

int Cmiss_spectrum_begin_change(Cmiss_spectrum_id spectrum)
{
	if (spectrum)
	{
		/* increment cache to allow nesting */
		(spectrum->cache)++;
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

int Cmiss_spectrum_end_change(Cmiss_spectrum_id spectrum)
{
	if (spectrum)
	{
		/* decrement cache to allow nesting */
		(spectrum->cache)--;
		/* once cache has run out, inform clients of any changes */
		if (0 == spectrum->cache)
		{
			if (spectrum->changed)
			{
				return Cmiss_spectrum_inform_clients(spectrum);
			}
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

/*******************************************************************************
 * Mark spectrum as changed; inform clients of its manager that it has changed
 */
int Cmiss_spectrum_changed(Cmiss_spectrum_id spectrum)
{
	if (spectrum)
	{
		spectrum->changed = 1;
		if (0 == spectrum->cache)
		{
			return Cmiss_spectrum_inform_clients(spectrum);
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

int Spectrum_set_minimum_and_maximum(struct Spectrum *spectrum,
	ZnReal minimum, ZnReal maximum)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Expands the range of this spectrum by adjusting the range of each component
it contains.  The ratios of the different component are preserved.
==============================================================================*/
{
	int return_code = 0;
	struct Spectrum_rerange_data data;

	ENTER(spectrum_set_minimum_and_maximum);
	if (spectrum && (minimum <= maximum))
	{
		if ( minimum != spectrum->minimum
			|| maximum != spectrum->maximum )
		{
			data.old_min = spectrum->minimum;
			/* Keep the range to speed calculation and the
				maximum so we get exact values */
			data.old_range = spectrum->maximum - spectrum->minimum;
			data.old_max = spectrum->maximum;
			data.min = minimum;
			data.range = maximum - minimum;
			data.max = maximum;
			FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
				Spectrum_rerange_iterator,
				(void *)&data, spectrum->list_of_components);
			/* Cannot assume that the minimum and maximum are now what was requested
				as the fix minimum and fix maximum flags may have overridden our re-range. */
			Spectrum_calculate_range(spectrum);
			Cmiss_spectrum_changed(spectrum);
		}
		return_code = 1;
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

int Spectrum_render_value_on_material(struct Spectrum *spectrum,
	struct Graphical_material *material, int number_of_data_components,
	GLfloat *data)
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Uses the <spectrum> to modify the <material> to represent the <number_of_data_components>
<data> values given.
==============================================================================*/
{
	GLfloat rgba[4];
	int return_code;
	struct Colour diffuse;
	struct Spectrum_render_data render_data;

	ENTER(Spectrum_render_value_on_material);
	if (spectrum && material)
	{
		if (spectrum->overwrite_colour)
		{
			rgba[0] = 0.0;
			rgba[1] = 0.0;
			rgba[2] = 0.0;
			rgba[3] = 1.0;
		}
		else
		{
			Graphical_material_get_diffuse(material, &diffuse);
			rgba[0] = (GLfloat)diffuse.red;
			rgba[1] = (GLfloat)diffuse.green;
			rgba[2] = (GLfloat)diffuse.blue;
			MATERIAL_PRECISION value;
			Graphical_material_get_alpha(material, &value);
			rgba[3] = (GLfloat)value;
		}
		render_data.rgba = rgba;
		render_data.data = data;
		render_data.number_of_data_components = number_of_data_components;

		return_code = FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
			Cmiss_spectrum_component_activate,(void *)&render_data,
			spectrum->list_of_components);

		diffuse.red = rgba[0];
		diffuse.green = rgba[1];
		diffuse.blue = rgba[2];

		Graphical_material_set_ambient(material, &diffuse);
		Graphical_material_set_diffuse(material, &diffuse);
		Graphical_material_set_alpha(material, rgba[3]);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_render_value_on_material.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_render_value_on_material */

int Spectrum_value_to_rgba(struct Spectrum *spectrum,int number_of_data_components,
	FE_value *data, ZnReal *rgba)
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Uses the <spectrum> to calculate RGBA components to represent the
<number_of_data_components> <data> values.
<rgba> is assumed to be an array of four values for red, green, blue and alpha.
==============================================================================*/
{
	int return_code;
	struct Spectrum_render_data render_data;

	ENTER(spectrum_value_to_rgba);
	if (spectrum)
	{
		GLfloat frgba[4];
		if (spectrum->overwrite_colour)
		{
			rgba[0] = 0.0;
			rgba[1] = 0.0;
			rgba[2] = 0.0;
			rgba[3] = 1.0;
			CAST_TO_OTHER(frgba, rgba, GLfloat, 4);
		}
		render_data.rgba = frgba;
		GLfloat *fData = new GLfloat[number_of_data_components];
		CAST_TO_OTHER(fData,data,GLfloat,number_of_data_components);
		render_data.data = fData;
		render_data.number_of_data_components = number_of_data_components;

		return_code = FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
			Cmiss_spectrum_component_activate,(void *)&render_data,
			spectrum->list_of_components);

		CAST_TO_OTHER(rgba, frgba, ZnReal, 4);

		delete[] fData;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_value_to_rgba.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_value_to_rgba */

int Spectrum_end_value_to_rgba(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 13 September 2007

DESCRIPTION :
Resets the caches and graphics state after rendering values.
==============================================================================*/
{
	int return_code;
	struct Spectrum_render_data render_data;

	ENTER(Spectrum_end_value_to_rgba);
	if (spectrum)
	{
		/* render data is currently not used in disable */
		FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
			Cmiss_spectrum_component_disable,(void *)&render_data,
			spectrum->list_of_components);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_end_value_to_rgba.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_end_value_to_rgba */

struct LIST(Cmiss_spectrum_component) *get_Cmiss_spectrum_component_list(
	struct Spectrum *spectrum )
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Returns the component list that describes the spectrum.  This is the pointer to
the object inside the spectrum so do not destroy it, any changes to it change
that spectrum.
==============================================================================*/
{
	struct LIST(Cmiss_spectrum_component) *component_list;

	ENTER(get_Cmiss_spectrum_component_list);
	if (spectrum)
	{
		component_list=spectrum->list_of_components;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Cmiss_spectrum_component_list.  Invalid argument(s)");
		component_list=(struct LIST(Cmiss_spectrum_component) *)NULL;
	}
	LEAVE;

	return (component_list);
} /* get_Cmiss_spectrum_component_list */

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Spectrum)

static int Spectrum_render_colour_lookup(struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 23 May 2005

DESCRIPTION :
Rebuilds the display_list for <spectrum> if it is not current.
==============================================================================*/
{
	enum Spectrum_colour_components colour_components;
	int i, indices[3], number_of_data_components, number_of_texture_components,
		number_of_values, return_code, table_size;
	GLfloat data[3], rgba[4];
	unsigned char *colour_table, *colour_table_ptr;
	struct Spectrum_render_data render_data;
	enum Texture_storage_type storage;

	ENTER(Spectrum_render_colour_lookup);
	if (spectrum)
	{
		colour_components = Spectrum_get_colour_components(spectrum);
		/* The component layout here must match the understanding of
			the texture components in the material programs */
		/* Could save memory by having a special treatment for MONOCHROME
			spectrums */
		if (colour_components & SPECTRUM_COMPONENT_ALPHA)
		{
			if (colour_components == SPECTRUM_COMPONENT_ALPHA)
			{
				/* Alpha only */
				number_of_texture_components = 1;
				/* We don't have an ALPHA only format and the material program
					now does the interpretation anyway */
				storage = TEXTURE_LUMINANCE;
			}
			else
			{
				/* Colour and alpha */
				number_of_texture_components = 4;
				storage = TEXTURE_RGBA;
			}
		}
		else
		{
			/* Colour only */
			number_of_texture_components = 3;
			storage = TEXTURE_RGB;
		}

		number_of_data_components = Spectrum_get_number_of_data_components(
			spectrum);
		switch (number_of_data_components)
		{
			case 1:
			{
				number_of_values = 1024;
			} break;
			case 2:
			{
				number_of_values = 256;
			} break;
			case 3:
			{
				number_of_values = 32;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_render_colour_lookup.  "
					"The spectrum %d uses more than 3 components, only the first 3 will be used.",
					spectrum->name);
				number_of_data_components = 3;
				number_of_values = 32;
			}
		}
		table_size = (int) (number_of_texture_components *
			 pow((double) number_of_values, number_of_data_components));
		if (ALLOCATE(colour_table, unsigned char, table_size))
		{
			colour_table_ptr = colour_table;

			for (i = 0 ; i < number_of_data_components ; i++)
			{
				indices[i] = 0;
				data[i] = 0.0;
			}
			colour_table_ptr = colour_table;

			render_data.rgba = rgba;
			render_data.data = data;
			render_data.number_of_data_components = number_of_data_components;

			FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
				Cmiss_spectrum_component_enable,(void *)&render_data,
				spectrum->list_of_components);

			while (indices[number_of_data_components - 1] < number_of_values)
			{
				if (spectrum->overwrite_colour)
				{
					rgba[0] = 0.0;
					rgba[1] = 0.0;
					rgba[2] = 0.0;
					rgba[3] = 1.0;
				}
				return_code = FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
					Cmiss_spectrum_component_activate,(void *)&render_data,spectrum->list_of_components);
				if (return_code)
				{
					if (colour_components != SPECTRUM_COMPONENT_ALPHA)
					{
						/* Not alpha only */
						 *colour_table_ptr =(unsigned char) (rgba[0] * 255.0);
						colour_table_ptr++;
						*colour_table_ptr = (unsigned char) (rgba[1] * 255.0);
						colour_table_ptr++;
						*colour_table_ptr = (unsigned char) (rgba[2] * 255.0);
						colour_table_ptr++;
					}
					if (colour_components & SPECTRUM_COMPONENT_ALPHA)
					{
						/* Alpha with or without colour */
						 *colour_table_ptr = (unsigned char) (rgba[3] * 255.0);
						colour_table_ptr++;
					}
				}

				indices[0]++;
				i = 0;
				data[0] = (GLfloat)indices[0] / (GLfloat)(number_of_values - 1);
				while ((i < number_of_data_components - 1) &&
					(indices[i] == number_of_values))
				{
					indices[i] = 0;
					data[i] = 0.0;
					i++;
					indices[i]++;
					data[i] = (GLfloat)indices[i] / (GLfloat)(number_of_values - 1);
				}
			}

			/* Finished using the spectrum for now (clear computed field cache) */
			FOR_EACH_OBJECT_IN_LIST(Cmiss_spectrum_component)(
				Cmiss_spectrum_component_disable,(void *)&render_data,
				spectrum->list_of_components);

			{
				struct Texture *texture;

				if (spectrum->colour_lookup_texture)
				{
					DEACCESS(Texture)(&spectrum->colour_lookup_texture);
				}

				texture = CREATE(Texture)("spectrum_texture");
				/* The mode of this texture must match that relied upon by
					the fragment program in material.c.  Specifically, to provide
					correct linear interpolation the input value has to be offset
					by half a pixel and scaled by (number_of_pixels-1)/(number_of_pixels)
					as linear interpolation starts at the centres of each pixel. */
				Texture_set_filter_mode(texture, TEXTURE_LINEAR_FILTER);
				Texture_set_wrap_mode(texture, TEXTURE_CLAMP_WRAP);
				colour_table_ptr = colour_table;
				switch (number_of_data_components)
				{
					case 1:
					{
						Texture_allocate_image(texture, number_of_values, 1,
							1, storage,
							/*number_of_bytes_per_component*/1, "bob");
						Texture_set_image_block(texture,
							/*left*/0, /*bottom*/0, number_of_values, 1,
							/*depth_plane*/0, number_of_values * number_of_texture_components,
							colour_table_ptr);
					} break;
					case 2:
					{
						Texture_allocate_image(texture, number_of_values, number_of_values,
							1, storage,
							/*number_of_bytes_per_component*/1, "bob");
						Texture_set_image_block(texture,
							/*left*/0, /*bottom*/0, number_of_values, number_of_values,
							/*depth_plane*/0, number_of_values * number_of_texture_components,
							colour_table_ptr);
					} break;
					case 3:
					{
						Texture_allocate_image(texture, number_of_values, number_of_values,
							number_of_values, storage,
							/*number_of_bytes_per_component*/1, "bob");
						for (i = 0 ; i < number_of_values ; i++)
						{
							Texture_set_image_block(texture,
								/*left*/0, /*bottom*/0, number_of_values, number_of_values,
								/*depth_plane*/i,
								number_of_values * number_of_texture_components,
								colour_table_ptr);
							colour_table_ptr += number_of_values * number_of_values *
								number_of_texture_components;
						}
					} break;
				}


				spectrum->colour_lookup_texture = ACCESS(Texture)(texture);
			}

			return_code = 1;
			DEALLOCATE(colour_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"compile_Graphical_spectrum.  Could not allocate temporary storage.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_render_colour_lookup.  Missing spectrum");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_render_colour_lookup */

int Spectrum_compile_colour_lookup(struct Spectrum *spectrum,
	Render_graphics_opengl *renderer)
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Rebuilds the display_list for <spectrum> if it is not current.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_compile_colour_lookup);
	if (spectrum)
	{
		Spectrum_render_colour_lookup(spectrum);

		return_code = renderer->Texture_compile(spectrum->colour_lookup_texture);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Spectrum_compile_colour_lookup.  Missing spectrum");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_compile_colour_lookup */

int Spectrum_execute_colour_lookup(struct Spectrum *spectrum,
	Render_graphics_opengl *renderer)
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Activates <spectrum> by calling its display list. If the display list is not
current, an error is reported.
If a NULL <spectrum> is supplied, spectrums are disabled.
==============================================================================*/
{
	int return_code;

	ENTER(Spectrum_execute_colour_lookup);
	return_code=0;
	if (spectrum && spectrum->colour_lookup_texture)
	{
		return_code = renderer->Texture_execute(spectrum->colour_lookup_texture);
	}
	else
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_execute_colour_lookup */

int Spectrum_get_colour_lookup_sizes(struct Spectrum *spectrum,
	int *lookup_dimension, int **lookup_sizes)
/*******************************************************************************
LAST MODIFIED : 2 May 2007

DESCRIPTION :
Returns the sizes used for the colour lookup spectrums internal texture.
==============================================================================*/
{
	int return_code, width, height, depth;

	ENTER(Spectrum_get_colour_lookup_sizes);
	return_code=0;
	if (spectrum && spectrum->colour_lookup_texture)
	{
		Texture_get_dimension(spectrum->colour_lookup_texture, lookup_dimension);

		ALLOCATE(*lookup_sizes, int, *lookup_dimension);
		Texture_get_size(spectrum->colour_lookup_texture,
			&width, &height, &depth);
		if (0 < *lookup_dimension)
		{
			(*lookup_sizes)[0] = width;
		}
		if (1 < *lookup_dimension)
		{
			(*lookup_sizes)[1] = height;
		}
		if (2 < *lookup_dimension)
		{
			(*lookup_sizes)[2] = depth;
		}
		return_code = 1;
	}
	else
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_get_colour_lookup_sizes */

double Cmiss_spectrum_get_minimum(Cmiss_spectrum_id spectrum)
{
	double value = 0.0;
	if (spectrum)
	{
		value = spectrum->minimum;
	}

	return value;
}

double Cmiss_spectrum_get_maximum(Cmiss_spectrum_id spectrum)
{
	double value = 0.0;
	if (spectrum)
	{
		value = spectrum->maximum;
	}

	return value;
}

int Cmiss_spectrum_set_minimum_and_maximum(Cmiss_spectrum_id spectrum, double minimum, double maximum)
{
	int return_code = 0;
	if (spectrum)
	{
		return_code = Spectrum_set_minimum_and_maximum(spectrum, minimum, maximum);
	}

	return return_code;
}

int Cmiss_spectrum_set_name(
	Cmiss_spectrum_id spectrum, const char *name)
{
	int return_code = 0;

	if (spectrum && name)
	{
		return_code = 1;
		if (spectrum->manager)
		{
			return_code = MANAGER_MODIFY_IDENTIFIER(Cmiss_spectrum, name)(
				spectrum, name, spectrum->manager);
		}
		else
		{
			char *new_name = duplicate_string(name);
			if (new_name)
			{
				DEALLOCATE(spectrum->name);
				spectrum->name = new_name;
			}
			else
			{
				return_code = 0;
			}
		}
	}

	return return_code;
}

char *Cmiss_spectrum_get_name(Cmiss_spectrum_id spectrum)
{
	char *name = NULL;
	if (spectrum)
	{
		name = duplicate_string(spectrum->name);
	}

	return name;
}

Cmiss_spectrum_id Cmiss_spectrum_access(Cmiss_spectrum_id spectrum)
{
	ENTER(Cmiss_spectrum_destroy);
	if (spectrum)
	{
		return ACCESS(Spectrum)(spectrum);
	}

	return NULL;
}

int Cmiss_spectrum_destroy(Cmiss_spectrum_id *spectrum_address)
{
	int return_code = 0;
	struct Spectrum *spectrum;

	ENTER(Cmiss_spectrum_destroy);
	if (spectrum_address && (spectrum = *spectrum_address))
	{
		(spectrum->access_count)--;
		if (spectrum->access_count <= 0)
		{
			return_code = DESTROY(Spectrum)(spectrum_address);
		}
		else if ((!spectrum->is_managed_flag) && (spectrum->manager) &&
			((1 == spectrum->access_count) || ((2 == spectrum->access_count) &&
				(MANAGER_CHANGE_NONE(Spectrum) != spectrum->manager_change_status))))
		{
			return_code = REMOVE_OBJECT_FROM_MANAGER(Spectrum)(spectrum, spectrum->manager);
		}
		else
		{
			return_code = 1;
		}
		*spectrum_address = (struct Spectrum *)NULL;
	}
	LEAVE;

	return return_code;
}

bool Cmiss_spectrum_is_managed(Cmiss_spectrum_id spectrum)
{
	if (spectrum)
	{
		return spectrum->is_managed_flag;
	}
	return 0;
}

int Cmiss_spectrum_set_managed(Cmiss_spectrum_id spectrum,  bool value)
{
	if (spectrum)
	{
		bool old_value = spectrum->is_managed_flag;
		spectrum->is_managed_flag = (value != 0);
		if (value != old_value)
		{
			MANAGED_OBJECT_CHANGE(Spectrum)(spectrum, MANAGER_CHANGE_NOT_RESULT(Spectrum));
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

Cmiss_spectrum_component_id Cmiss_spectrum_create_component(Cmiss_spectrum_id spectrum)
{
	if (spectrum)
	{
		struct Cmiss_spectrum_component *component = CREATE(Cmiss_spectrum_component)();
		if (0 == Spectrum_add_component(spectrum, component, 0))
		{
			DEACCESS(Cmiss_spectrum_component)(&component);
		}
		else
		{
			Cmiss_spectrum_changed(spectrum);
		}
		return component;
	}
	return 0;
}

Cmiss_spectrum_component_id Cmiss_spectrum_get_first_component(Cmiss_spectrum_id spectrum)
{
	struct Cmiss_spectrum_component *component = NULL;
	if (spectrum)
	{
		component=FIND_BY_IDENTIFIER_IN_LIST(Cmiss_spectrum_component, position)(
			1, spectrum->list_of_components);
		if (component)
		{
			ACCESS(Cmiss_spectrum_component)(component);
		}
	}
	return component;
}

Cmiss_spectrum_component_id Cmiss_spectrum_get_next_component(Cmiss_spectrum_id spectrum,
	Cmiss_spectrum_component_id ref_component)
{
	struct Cmiss_spectrum_component *component = NULL;
	if (spectrum)
	{
		int ref_pos = Cmiss_spectrum_get_component_position(spectrum, ref_component);
		if (ref_pos > 0)
		{
			component=FIND_BY_IDENTIFIER_IN_LIST(Cmiss_spectrum_component,position)(
				ref_pos+1, spectrum->list_of_components);
			if (component)
			{
				ACCESS(Cmiss_spectrum_component)(component);
			}
		}
	}
	return component;
}

Cmiss_spectrum_component_id Cmiss_spectrum_get_previous_component(Cmiss_spectrum_id spectrum,
	Cmiss_spectrum_component_id ref_component)
{
	struct Cmiss_spectrum_component *component = NULL;
	if (spectrum)
	{
		int ref_pos = Cmiss_spectrum_get_component_position(spectrum, ref_component);
		if (ref_pos > 1)
		{
			component=FIND_BY_IDENTIFIER_IN_LIST(Cmiss_spectrum_component,position)(
				ref_pos-1, spectrum->list_of_components);
			if (component)
			{
				ACCESS(Cmiss_spectrum_component)(component);
			}
		}
	}
	return component;
}

int Cmiss_spectrum_move_component_before(Cmiss_spectrum_id spectrum,
	Cmiss_spectrum_component_id component, Cmiss_spectrum_component_id ref_component)
{
	int return_code = 0;
	if (spectrum && component && (component->spectrum == spectrum) &&
		((0 == ref_component) || (component->spectrum == ref_component->spectrum)))
	{
		Cmiss_spectrum_component_id current_component = ACCESS(Cmiss_spectrum_component)(component);
		int position = Cmiss_spectrum_get_component_position(spectrum, ref_component);
		if (Cmiss_spectrum_remove_component(spectrum, current_component))
			return_code = Spectrum_add_component(spectrum, current_component, position);
		if (return_code)
			Cmiss_spectrum_changed(spectrum);
		DEACCESS(Cmiss_spectrum_component)(&current_component);
	}
	return return_code;
}

int Cmiss_spectrum_remove_all_components(Cmiss_spectrum_id spectrum)
{
	if (spectrum)
	{
		Cmiss_spectrum_begin_change(spectrum);
		REMOVE_ALL_OBJECTS_FROM_LIST(Cmiss_spectrum_component)
			(get_Cmiss_spectrum_component_list(spectrum));
		Cmiss_spectrum_changed(spectrum);
		Cmiss_spectrum_end_change(spectrum);
		return CMISS_OK;
	}

	return CMISS_ERROR_ARGUMENT;
}

int Cmiss_spectrum_get_number_of_components(Cmiss_spectrum_id spectrum)
{
	if (spectrum)
	{
		return NUMBER_IN_LIST(Cmiss_spectrum_component)(
			get_Cmiss_spectrum_component_list(spectrum));
	}

	return 0;
} /* Cmiss_group_get_number_of_graphic */
