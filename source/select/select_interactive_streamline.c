/*******************************************************************************
FILE : select_interactive_streamline.c

LAST MODIFIED : 20 April 2000

DESCRIPTION :
Declares select widget functions for interactive_streamline objects.
==============================================================================*/

#include <Xm/Xm.h>
#include "graphics/graphics_object.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "select/select_interactive_streamline.h"
#include "select/select_private.h"

/*
Module types
------------
*/
FULL_DECLARE_SELECT_STRUCT_TYPE(Interactive_streamline);

/*
Module functions
----------------
*/
DECLARE_DEFAULT_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION(
	Interactive_streamline)
DECLARE_DEFAULT_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION(
	Interactive_streamline)

PROTOTYPE_SELECT_MANAGER_CREATE_FUNCTION(Interactive_streamline)
/*****************************************************************************
LAST MODIFIED : 20 April 2000

DESCRIPTION :
Can only make a copy of the template_object
============================================================================*/
{
	char *new_object_name,*temp_name;
	FE_value xi[3];
	struct Interactive_streamline *new_object;

	ENTER(SELECT_MANAGER_CREATE(Interactive_streamline));
	if (object_manager)
	{
		/* get a unique name for the new object */
		if (!(template_object&&
			GET_NAME(Interactive_streamline)(template_object,&new_object_name)))
		{
			display_message(ERROR_MESSAGE,
				"SELECT_MANAGER_CREATE(Interactive_streamline).  "
				"Need to have an object to duplicate");
			new_object_name = (char *)NULL;
		}
		/* Ensure the new identifier is not used by some object in the manager */
		/* Keep on appending "+" onto the name until it is unique: */
		while ((new_object_name)&&(FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_streamline,name)
			(new_object_name,object_manager)))
		{
			if (REALLOCATE(temp_name,new_object_name,char,
				strlen(new_object_name)+2))
			{
				new_object_name=temp_name;
				strcat(new_object_name,"+");
			}
			else
			{
				DEALLOCATE(new_object_name);
				display_message(ERROR_MESSAGE,
					"SELECT_MANAGER_CREATE(Interactive_streamline).  "
					"Could not give object a unique name.");
			}
		}
		if (new_object_name)
		{
			xi[0] = 0;
			xi[1] = 0;
			xi[2] = 0;
			new_object=CREATE(Interactive_streamline)(new_object_name,
				STREAM_LINE,(struct FE_element *)NULL,xi,(struct Computed_field *)NULL,
				(struct Computed_field *)NULL,/*reverse_track*/0,/*length*/0,/*width*/0,
				STREAM_NO_DATA,(struct Computed_field *)NULL,
				(struct GT_object *)NULL, (struct GT_object *)NULL);
			MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_streamline,name)(
				new_object,template_object);
			DEALLOCATE(new_object_name);
		}
		else
		{
			new_object=(struct Interactive_streamline *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"SELECT_MANAGER_CREATE(Interactive_streamline).  Missing object manager");
		new_object=(struct Interactive_streamline *)NULL;
	}
	LEAVE;

	return (new_object);
} /* SELECT_MANAGER_CREATE(Interactive_streamline) */

DECLARE_SELECT_MODULE_FUNCTIONS(Interactive_streamline,"Streamline named:")

/*
Global functions
----------------
*/
DECLARE_SELECT_GLOBAL_FUNCTIONS(Interactive_streamline)

