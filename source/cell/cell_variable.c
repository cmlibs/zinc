/*******************************************************************************
FILE : cell_variable.c

LAST MODIFIED : 21 June 2001

DESCRIPTION :
Routines for using the Cell_variable objects
==============================================================================*/

#include <stdio.h>
#include <string.h>

#include "cell/cell_cmiss_interface.h"
#include "cell/cell_interface.h"
#include "cell/cell_variable.h"
#include "cell/cell_variable_editing_dialog.h"
#include "cell/cell_variable_unemap_interface.h"
#include "cell/cell_types.h"

#include "general/indexed_list_private.h"
#include "general/compare.h"

/*
Module objects
--------------
*/
struct Cell_variable
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
A data object used to store information for all variables found in the CellML.
==============================================================================*/
{
  /* The variable's ID, used for the indexed list because the name is
   * not guaranteed to be unique - unused ??
   */
  int id;
  /* The access counter */
  int access_count;
  /* The name of the variable */
  char *name;
  /* A more descriptive name */
  char *display_name;
  /* A flag used to determine which variables have changed from their
   * default values (from the CellML)
   */
  int changed;
  /* The units for the variable */
  struct Cell_units *units;
  /* The type of the variable */
  enum Cell_value_type value_type;
  /* The actual value of the variable */
  union
  {
    CELL_INTEGER integer_value;
    CELL_REAL real_value;
    CELL_STRING string_value;
  } value;
  /* The information required to map the variable to its CMISS array */
  struct Cell_cmiss_interface *cmiss_interface;
  /* The information required to view the variable in UnEmap */
  struct Cell_variable_unemap_interface *unemap_interface;
  /* The list of text field widgets displaying the value of this variable */
  struct Cell_variable_widget_list *value_text_field_list;
}; /* struct Cell_variable */

FULL_DECLARE_INDEXED_LIST_TYPE(Cell_variable);

/*
Module functions
----------------
*/
/*DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cell_variable,id,int,compare_int)*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cell_variable,name,char *,strcmp)

static void Cell_variable_list_full(struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Does a full listing of the <cell_variable>.
==============================================================================*/
{
  ENTER(Cell_variable_list_full);
  if (cell_variable)
  {
    display_message(INFORMATION_MESSAGE,"Variable: %s\n",cell_variable->name);
    display_message(INFORMATION_MESSAGE,"  Display name: %s\n",
      cell_variable->display_name);
    display_message(INFORMATION_MESSAGE,"  ID: %d\n",cell_variable->id);
    display_message(INFORMATION_MESSAGE,"  access count: %d\n",
      cell_variable->access_count);
    switch (cell_variable->value_type)
    {
      case CELL_INTEGER_VALUE:
      {
        display_message(INFORMATION_MESSAGE,
          "  Integer value: "CELL_INTEGER_FORMAT"\n",
          cell_variable->value.integer_value);
      } break;
      case CELL_REAL_VALUE:
      {
        display_message(INFORMATION_MESSAGE,
          "  Real value: "CELL_REAL_FORMAT"\n",
          cell_variable->value.real_value);
      } break;
      case CELL_STRING_VALUE:
      {
        display_message(INFORMATION_MESSAGE,
          "  String value: "CELL_STRING_FORMAT"\n",
          cell_variable->value.string_value);
      } break;
      case CELL_UNKNOWN_VALUE:
      default:
      {
        display_message(INFORMATION_MESSAGE,"  Unknown value.\n");
      } break;
    } /* switch (cell_variable->value_type) */
    if (cell_variable->cmiss_interface)
    {
      Cell_cmiss_interface_list(cell_variable->cmiss_interface);
    }
    else
    {
      display_message(INFORMATION_MESSAGE,"  No CMISS interface information\n");
    }
    if (cell_variable->unemap_interface)
    {
      display_message(INFORMATION_MESSAGE,"  Contains UnEmap information\n");
    }
    else
    {
      display_message(INFORMATION_MESSAGE,
        "  No UnEmap interface information\n");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_list_full.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* Cell_variable_list_full() */
  
static void Cell_variable_list_brief(struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Does a brief listing of the <cell_variable>.
==============================================================================*/
{
  ENTER(Cell_variable_list_brief);
  if (cell_variable)
  {
    display_message(INFORMATION_MESSAGE,"Variable: %s\n",cell_variable->name);
    switch (cell_variable->value_type)
    {
      case CELL_INTEGER_VALUE:
      {
        display_message(INFORMATION_MESSAGE,
          "  Integer value: "CELL_INTEGER_FORMAT"\n",
          cell_variable->value.integer_value);
      } break;
      case CELL_REAL_VALUE:
      {
        display_message(INFORMATION_MESSAGE,
          "  Real value: "CELL_REAL_FORMAT"\n",
          cell_variable->value.real_value);
      } break;
      case CELL_STRING_VALUE:
      {
        display_message(INFORMATION_MESSAGE,
          "  String value: "CELL_STRING_FORMAT"\n",
          cell_variable->value.string_value);
      } break;
      case CELL_UNKNOWN_VALUE:
      default:
      {
        display_message(INFORMATION_MESSAGE,"  Unknown value.\n");
      } break;
    } /* switch (cell_variable->value_type) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_list_brief.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* Cell_variable_list_brief() */

/*
Global functions
----------------
*/

DECLARE_OBJECT_FUNCTIONS(Cell_variable)
DECLARE_INDEXED_LIST_FUNCTIONS(Cell_variable)
  /*DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cell_variable,id,
    int,compare_int)*/
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cell_variable,name,
  char *,strcmp)

struct Cell_variable *CREATE(Cell_variable)(char *name)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Creates a Cell_variable object.
==============================================================================*/
{
  static int current_variable_number; /* used to assign id's to each variable
                                        * as it is created */
  struct Cell_variable *cell_variable;

  ENTER(CREATE(Cell_variable));
  if (name)
  {
    if (ALLOCATE(cell_variable,struct Cell_variable,1))
    {
      /* Assign an ID to each component as it is created to guarantee a unique
       * reference to each compnent object
       */
      current_variable_number++;
      cell_variable->id = current_variable_number;
      /* initialise data objects */
      cell_variable->access_count = 0;
      cell_variable->name = (char *)NULL;
      cell_variable->display_name = (char *)NULL;
      cell_variable->changed = 0;
      cell_variable->units = (struct Cell_units *)NULL;
      cell_variable->value_type = CELL_UNKNOWN_VALUE;
      cell_variable->cmiss_interface = (struct Cell_cmiss_interface *)NULL;
      cell_variable->unemap_interface =
        (struct Cell_variable_unemap_interface *)NULL;
      cell_variable->value_text_field_list =
        (struct Cell_variable_widget_list *)NULL;
      /* set the variable's name */
      Cell_variable_set_name(cell_variable,name);
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_variable).  "
        "Unable to allocate memory for the Cell variable object");
      cell_variable = (struct Cell_variable *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_variable).  "
      "Invalid argument(s)");
    cell_variable = (struct Cell_variable *)NULL;
  }
  LEAVE;
  return(cell_variable);
} /* CREATE(Cell_variable) */

int DESTROY(Cell_variable)(struct Cell_variable **cell_variable_address)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Destroys a Cell_variable object.
==============================================================================*/
{
	int return_code;
  struct Cell_variable *cell_variable;

	ENTER(DESTROY(Cell_variable));
	if (cell_variable_address && (cell_variable = *cell_variable_address))
	{
    if (cell_variable->access_count == 0)
    {
      if (cell_variable->name)
      {
        DEALLOCATE(cell_variable->name);
      }
      if (cell_variable->display_name)
      {
        DEALLOCATE(cell_variable->display_name);
      }
      if (cell_variable->cmiss_interface)
      {
        DESTROY(Cell_cmiss_interface)(&(cell_variable->cmiss_interface));
      }
      if (cell_variable->unemap_interface)
      {
        DESTROY(Cell_variable_unemap_interface)(
          &(cell_variable->unemap_interface));
      }
      if (cell_variable->value_text_field_list)
      {
        DESTROY(Cell_variable_widget_list)(
          &(cell_variable->value_text_field_list));
      }
      DEALLOCATE(*cell_variable_address);
      *cell_variable_address = (struct Cell_variable *)NULL;
      return_code=1;
    }
    else
    {
      display_message(WARNING_MESSAGE,"DESTROY(Cell_variable).  "
        "Access count is not zero - cannot destroy object");
      return_code = 0;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_variable).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* DESTROY(Cell_variable) */

int Cell_variable_set_name(struct Cell_variable *cell_variable,
  char *name)
/*******************************************************************************
LAST MODIFIED : 21 June 2001

DESCRIPTION :
Sets the <name> of the <cell_variable> - copies the <name> so the calling
routine should deallocate it.
==============================================================================*/
{
  int return_code = 0;
  char *tmp_name;
  
  ENTER(Cell_variable_set_name);
  if (cell_variable && name)
  {
    if (ALLOCATE(tmp_name,char,strlen(name)+1))
    {
      strcpy(tmp_name,name);
      if (cell_variable->name)
      {
        DEALLOCATE(cell_variable->name);
      }
      cell_variable->name = tmp_name;
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_variable_set_name.  "
        "Unable to allocate memory for the name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_set_name.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_set_name() */

int Cell_variable_set_display_name(struct Cell_variable *cell_variable,
  char *display_name)
/*******************************************************************************
LAST MODIFIED : 21 June 2001

DESCRIPTION :
Sets the <display_name> of the <cell_variable> - copies the <display_name> so
the calling routine should deallocate it.
==============================================================================*/
{
  int return_code = 0;
  char *tmp_display_name;
  
  ENTER(Cell_variable_set_display_name);
  if (cell_variable && display_name)
  {
    if (ALLOCATE(tmp_display_name,char,strlen(display_name)+1))
    {
      strcpy(tmp_display_name,display_name);
      if (cell_variable->display_name)
      {
        DEALLOCATE(cell_variable->display_name);
      }
      cell_variable->display_name = tmp_display_name;
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_variable_set_display_name.  "
        "Unable to allocate memory for the display name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_set_display_name.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_set_display_name() */

int Cell_variable_set_value_type(struct Cell_variable *cell_variable,
  enum Cell_value_type value_type)
/*******************************************************************************
LAST MODIFIED : 21 June 2001

DESCRIPTION :
Sets the value type of the <cell_variable> to be <value_type>.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_variable_set_value_type);
  if (cell_variable && value_type)
  {
    if (cell_variable->value_type != value_type)
    {
      /* Only need to do anything if the value type changes, in which case the
         value of the variable needs to be initialised to something sensible -
         and any memory already allocated needs to be deallocated */
      if ((cell_variable->value_type == CELL_STRING_VALUE) &&
        (cell_variable->value.string_value))
      {
        DEALLOCATE(cell_variable->value.string_value);
      }
      switch (value_type)
      {
        case CELL_INTEGER_VALUE:
        {
          cell_variable->value.integer_value = -9999;
          cell_variable->value_type = value_type;
          cell_variable->changed = 1;
          return_code = 1;
        } break;
        case CELL_REAL_VALUE:
        {
          cell_variable->value.real_value = -9999.99;
          cell_variable->value_type = value_type;
          cell_variable->changed = 1;
          return_code = 1;
        } break;
        case CELL_STRING_VALUE:
        {
          cell_variable->value.string_value = (char *)NULL;
          cell_variable->value_type = value_type;
          cell_variable->changed = 1;
          return_code = 1;
        } break;
        case CELL_UNKNOWN_VALUE:
        default:
        {
          display_message(ERROR_MESSAGE,"Cell_variable_set_value_type.  "
            "Invalid value type");
          return_code = 0;
        }
      } /* switch (value_type) */
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_set_value_type.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_set_value_type() */

int Cell_variable_set_value_from_string(struct Cell_variable *cell_variable,
  char *value_string)
/*******************************************************************************
LAST MODIFIED : 20 October 2000

DESCRIPTION :
Sets the value of the <cell_variable> from the given <value_string>
==============================================================================*/
{
  int return_code = 0;
  CELL_INTEGER tmp_int;
  CELL_REAL tmp_real;
  
  ENTER(Cell_variable_set_value_from_string);
  if (cell_variable && value_string)
  {
    switch (cell_variable->value_type)
    {
      case CELL_INTEGER_VALUE:
      {
        if (sscanf(value_string,CELL_INTEGER_FORMAT,
          &tmp_int))
        {
          if (cell_variable->value.integer_value != tmp_int)
          {
            cell_variable->value.integer_value = tmp_int;
            cell_variable->changed = 1;
            return_code = 2;
          }
          else
          {
            /* do nothing */
            return_code = 1;
          }
        }
        else
        {
          return_code = 0;
        }
      } break;
      case CELL_REAL_VALUE:
      { 
        if (sscanf(value_string,CELL_REAL_FORMAT,
          &tmp_real))
        {
          if (cell_variable->value.real_value != tmp_real)
          {
            cell_variable->value.real_value = tmp_real;
            cell_variable->changed = 1;
            return_code = 2;
          }
          else
          {
            /* do nothing */
            return_code = 1;
          }
        }
      } break;
      case CELL_STRING_VALUE:
      {
        if (cell_variable->value.string_value &&
          strcmp(value_string,cell_variable->value.string_value))
        {
          /* The value string already exists, and is different to the new
             value */
          DEALLOCATE(cell_variable->value.string_value);
          if (ALLOCATE(cell_variable->value.string_value,char,
            strlen(value_string)+1))
          {
            if (strcpy(cell_variable->value.string_value,value_string))
            {
              cell_variable->changed = 1;
              return_code = 2;
            }
            else
            {
              return_code = 0;
            }
          }
          else
          {
            return_code = 0;
          }
        }
        else
        {
          /* No value already exists */
          if (ALLOCATE(cell_variable->value.string_value,char,
            strlen(value_string)+1))
          {
            if (strcpy(cell_variable->value.string_value,value_string))
            {
              cell_variable->changed = 1;
              return_code = 2;
            }
            else
            {
              return_code = 0;
            }
          }
          else
          {
            return_code = 0;
          }
        }
      } break;
      case CELL_UNKNOWN_VALUE:
      default:
      {
        /* do nothing ?? */
        return_code = 0;
      } break;
    } /* switch (cell_variable->value_type) */
    if (return_code > 1)
    {
      /* The value of the variable has changed, so update all the value text
         field widgets */
      if (cell_variable->value_text_field_list)
      {
        Cell_variable_widget_list_update_text_field_value(
          cell_variable->value_text_field_list,
          Cell_variable_get_value_as_string(cell_variable));
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_set_value_from_string.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_set_value_from_string() */

char *Cell_variable_get_value_as_string(struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Returns the value of the <cell_variable> as a string.
==============================================================================*/
{
  char *value_string,temp_string[512];
  
  ENTER(Cell_variable_get_value_as_string);
  if (cell_variable)
  {
    switch (cell_variable->value_type)
    {
      case CELL_INTEGER_VALUE:
      {
        sprintf(temp_string,CELL_INTEGER_FORMAT,
          cell_variable->value.integer_value);
      } break;
      case CELL_REAL_VALUE:
      {
        sprintf(temp_string,CELL_REAL_FORMAT,
          cell_variable->value.real_value);
      } break;
      case CELL_STRING_VALUE:
      {
        sprintf(temp_string,CELL_STRING_FORMAT,
          cell_variable->value.string_value);
      } break;
      case CELL_UNKNOWN_VALUE:
      default:
      {
        /* do nothing ?? */
        sprintf(temp_string,"");
      } break;
    } /* switch (cell_variable->value_type) */
    if (ALLOCATE(value_string,char,strlen(temp_string)+1))
    {
      strcpy(value_string,temp_string);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_variable_get_value_as_string.  "
        "Unable to allocate memory for the value string");
      value_string = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_get_value_as_string.  "
      "Invalid argument(s)");
    value_string = (char *)NULL;
  }
  LEAVE;
  return(value_string);
} /* Cell_variable_get_value_as_string() */

int Cell_variable_list(struct Cell_variable *cell_variable,void *full_void)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Iterator function used to list out the current variables. If <full> is not 0,
then a full listing of the <cell_variable> is given, otherwise just a brief
listing.
==============================================================================*/
{
  int return_code = 0;
  int *full = (int *)full_void;

  ENTER(Cell_variable_list);
  if (cell_variable)
  {
    if (*full)
    {
      /* do a full listing */
      Cell_variable_list_full(cell_variable);
    }
    else
    {
      /* do a brief listing */
      Cell_variable_list_brief(cell_variable);
    }
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_list.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_list() */

int Cell_variable_set_cmiss_interface(struct Cell_variable *cell_variable,
  char *array_string,char *position_string)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Sets the CMISS variable interface information for the given <cell_variable>
==============================================================================*/
{
  int return_code = 0,position;

  ENTER(Cell_variable_set_cmiss_interface);
  if (cell_variable && array_string && position_string)
  {
    if (cell_variable->cmiss_interface)
    {
      DESTROY(Cell_cmiss_interface)(&(cell_variable->cmiss_interface));
    }
    if (sscanf(position_string,"%d",&position))
    {
      if (cell_variable->cmiss_interface =
        CREATE(Cell_cmiss_interface)(array_string,position))
      {
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_variable_set_cmiss_interface.  "
          "Unable to create the cmiss interface object");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_variable_set_cmiss_interface.  "
        "Unable to get a integer position from the position string");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_set_cmiss_interface.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_set_cmiss_interface() */

struct Cell_cmiss_interface *Cell_variable_get_cmiss_interface(
  struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Gets the CMISS variable interface for the given <cell_variable>
==============================================================================*/
{
  struct Cell_cmiss_interface *cell_cmiss_interface =
    (struct Cell_cmiss_interface *)NULL;
  
  ENTER(Cell_variable_get_cmiss_interface);
  if (cell_variable)
  {
    cell_cmiss_interface = cell_variable->cmiss_interface;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_get_cmiss_interface.  "
      "Invalid argument(s)");
    cell_cmiss_interface = (struct Cell_cmiss_interface *)NULL;
  }
  LEAVE;
  return(cell_cmiss_interface);
} /* Cell_variable_get_cmiss_interface() */

struct Cell_variable_unemap_interface *Cell_variable_get_unemap_interface(
  struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Gets the UnEmap variable interface for the given <cell_variable>
==============================================================================*/
{
  struct Cell_variable_unemap_interface *cell_variable_unemap_interface;
  
  ENTER(Cell_variable_get_unemap_interface);
  if (cell_variable)
  {
    cell_variable_unemap_interface = cell_variable->unemap_interface;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_get_unemap_interface.  "
      "Invalid argument(s)");
    cell_variable_unemap_interface =
      (struct Cell_variable_unemap_interface *)NULL;
  }
  LEAVE;
  return(cell_variable_unemap_interface);
} /* Cell_variable_get_unemap_interface() */

CELL_REAL Cell_variable_get_real_value(struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Returns the variable's value as a real.
==============================================================================*/
{
  CELL_REAL real_value;

  ENTER(Cell_variable_get_real_value);
  if (cell_variable)
  {
    switch (cell_variable->value_type)
    {
      case CELL_REAL_VALUE:
      {
        real_value = cell_variable->value.real_value;
      } break;
      case CELL_INTEGER_VALUE:
      {
        real_value = (CELL_REAL)(cell_variable->value.integer_value);
        display_message(WARNING_MESSAGE,"Cell_variable_get_real_value.  "
          "Type mismatch - variable: \"%s\"",cell_variable->name);
      } break;
      default:
      {
        real_value = (CELL_REAL)0.0;
        display_message(ERROR_MESSAGE,"Cell_variable_get_real_value.  "
          "Unable to get a real value - variable: \"%s\"",cell_variable->name);
      }
    } /* switch (cell_variable->value_type) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_get_real_value.  "
      "Invalid argument(s)");
    real_value = (CELL_REAL)0.0;
  }
  LEAVE;
  return(real_value);
} /* Cell_variable_get_real_value() */

CELL_INTEGER Cell_variable_get_integer_value(
  struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Returns the variable's value as a integer.
==============================================================================*/
{
  CELL_INTEGER integer_value;

  ENTER(Cell_variable_get_integer_value);
  if (cell_variable)
  {
    switch (cell_variable->value_type)
    {
      case CELL_INTEGER_VALUE:
      {
        integer_value = cell_variable->value.integer_value;
      } break;
      case CELL_REAL_VALUE:
      {
        integer_value = (CELL_INTEGER)(cell_variable->value.real_value);
        display_message(WARNING_MESSAGE,"Cell_variable_get_integer_value.  "
          "Type mismatch - variable: \"%s\"",cell_variable->name);
      } break;
      default:
      {
        integer_value = (CELL_INTEGER)0;
        display_message(ERROR_MESSAGE,"Cell_variable_get_integer_value.  "
          "Unable to get a integer value - variable: \"%s\"",
          cell_variable->name);
      }
    } /* switch (cell_variable->value_type) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_get_integer_value.  "
      "Invalid argument(s)");
    integer_value = (CELL_INTEGER)0;
  }
  LEAVE;
  return(integer_value);
} /* Cell_variable_get_integer_value() */

int Cell_variable_set_unemap_interface(struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Sets the UnEmap variable interface information for the given <cell_variable>
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_variable_set_unemap_interface);
  if (cell_variable)
  {
    /* Only a variable with a CMISS interface can have a UnEmap interface */
    if (cell_variable->cmiss_interface)
    {
      if (cell_variable->unemap_interface)
      {
        DESTROY(Cell_variable_unemap_interface)(
          &(cell_variable->unemap_interface));
      }
      if (cell_variable->unemap_interface =
        CREATE(Cell_variable_unemap_interface)(cell_variable->value_type,
          cell_variable->name))
      {
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_variable_set_unemap_interface.  "
          "Unable to create the UnEmap interface object");
        return_code = 0;
      }
    }
    else
    {
      /* No error message required ?? */
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_set_unemap_interface.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_set_unemap_interface() */

char *Cell_variable_get_name(struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 08 November 2000

DESCRIPTION :
Returns a copy of the name for the given <cell_variable>
==============================================================================*/
{
  char *name;

  ENTER(Cell_variable_get_name);
  if (cell_variable)
  {
    if (cell_variable->name && ALLOCATE(name,char,
      strlen(cell_variable->name)+1))
    {
      strcpy(name,cell_variable->name);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_variable_get_name.  "
        "Unable to allocate memory for the variable name");
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_get_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_variable_get_name() */

char *Cell_variable_get_display_name(struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 08 November 2000

DESCRIPTION :
Returns a copy of the display name for the given <cell_variable>
==============================================================================*/
{
  char *display_name;

  ENTER(Cell_variable_get_display_name);
  if (cell_variable)
  {
    if (cell_variable->display_name && ALLOCATE(display_name,char,
      strlen(cell_variable->display_name)+1))
    {
      strcpy(display_name,cell_variable->display_name);
    }
    else
    {
      display_name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_get_display_name.  "
      "Invalid argument(s)");
    display_name = (char *)NULL;
  }
  LEAVE;
  return(display_name);
} /* Cell_variable_get_display_name() */

int Cell_variable_variable_has_unemap_interface(
  struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Returns 1 if the given <cell_variable> has a UnEmap interface defined.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_variable_variable_has_unemap_interface);
  if (cell_variable)
  {
    if (cell_variable->unemap_interface)
    {
      return_code = 1;
    }
    else
    {
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_variable_variable_has_unemap_interface.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_variable_has_unemap_interface() */

int Cell_variable_variable_has_cmiss_interface(
  struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Returns 1 if the given <cell_variable> has a CMISS interface defined.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_variable_variable_has_cmiss_interface);
  if (cell_variable)
  {
    if (cell_variable->cmiss_interface)
    {
      return_code = 1;
    }
    else
    {
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_variable_variable_has_cmiss_interface.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_variable_has_cmiss_interface() */

int Cell_variable_destroy_unemap_interface(struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
If the <cell_variable> has a UnEmap interface, it is destroyed.
==============================================================================*/
{
  int return_code;

  ENTER(Cell_variable_destroy_unemap_interface);
  if (cell_variable)
  {
    if (cell_variable->unemap_interface)
    {
      return_code = DESTROY(Cell_variable_unemap_interface)(
        &(cell_variable->unemap_interface));
    }
    else
    {
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_destroy_unemap_interface.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_destroy_unemap_interface() */

int Cell_variable_set_changed(struct Cell_variable *cell_variable,int value)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Sets the changed field of the given <cell_variable> object to <value>.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_variable_set_changed);
  if (cell_variable)
  {
    cell_variable->changed = value;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_set_changed.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_set_changed() */

int Cell_variable_get_changed(struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Gets the changed field of the given <cell_variable>.
==============================================================================*/
{
  int changed;

  ENTER(Cell_variable_get_changed);
  if (cell_variable)
  {
    changed = cell_variable->changed;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_get_changed.  "
      "Invalid argument(s)");
    changed = 0;
  }
  LEAVE;
  return(changed);
} /* Cell_variable_get_changed() */

char *Cell_variable_check_value_string(struct Cell_variable *cell_variable,
  char *value_string)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Checks the <value_string> for a valid value for the given <cell_variable> and
returns the value in "Cell format" if a valid value is found, otherwise a
NULL string is returned.
==============================================================================*/
{
  char *checked_string,temp[512];
  CELL_INTEGER integer_value;
  CELL_REAL real_value;
  int error = 0;
  
  ENTER(Cell_variable_check_value_string);
  if (cell_variable && value_string)
  {
    /* Check for a valid value and set-up a temp string */
    switch (cell_variable->value_type)
    {
      case CELL_INTEGER_VALUE:
      {
        if (sscanf(value_string,CELL_INTEGER_FORMAT,
          &(integer_value)))
        {
          sprintf(temp,CELL_INTEGER_FORMAT,integer_value);
        }
        else
        {
          error = 1;
        }
      } break;
      case CELL_REAL_VALUE:
      {
        if (sscanf(value_string,CELL_REAL_FORMAT,
          &(real_value)))
        {
          sprintf(temp,CELL_REAL_FORMAT,real_value);
        }
        else
        {
          error = 1;
        }
      } break;
      case CELL_STRING_VALUE:
      {
        if (!(sprintf(temp,CELL_STRING_FORMAT,value_string)))
        {
         error = 1;
        }
      } break;
      case CELL_UNKNOWN_VALUE:
      default:
      {
        error = 0;
      } break;
    } /* switch (cell_variable->value_type) */
    if (!error)
    {
      /* Create the string to return */
      if (ALLOCATE(checked_string,char,strlen(temp)+1))
      {
        strcpy(checked_string,temp);
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_variable_check_value_string.  "
          "Unable to allocate memory for the checked string");
        checked_string = (char *)NULL;
      }
    }
    else
    {
      /* No valid value found */
      checked_string = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_check_value_string.  "
      "Invalid argument(s)");
    checked_string = (char *)NULL;
  }
  LEAVE;
  return(checked_string);
} /* Cell_variable_check_value_string() */

int Cell_variable_add_value_text_field(struct Cell_variable *variable,
  void *widget_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Adds the given text field <widget> to the <variable>'s list of value widgets.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_variable_add_value_text_field);
  if (variable && widget_void)
  {
    if (!variable->value_text_field_list)
    {
      if (!(variable->value_text_field_list =
        CREATE(Cell_variable_widget_list)()))
      {
        display_message(ERROR_MESSAGE,"Cell_variable_add_value_text_field.  "
          "Unable to create the widget list");
        return_code = 0;
      }
    }
    if (variable->value_text_field_list)
    {
      return_code = Cell_variable_widget_list_add_widget(
        variable->value_text_field_list,widget_void);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_add_value_text_field.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_add_value_text_field() */

int Cell_variable_delete_value_text_fields(struct Cell_variable *variable,
  void *user_data_void)
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Iterator function used to delete all value text field widgets for the given
<variable>.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_variable_delete_value_text_fields);
  USE_PARAMETER(user_data_void);
  if (variable)
  {
    if (variable->value_text_field_list)
    {
      return_code = DESTROY(Cell_variable_widget_list)(
        &(variable->value_text_field_list));
    }
    else
    {
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_delete_value_text_fields.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_delete_value_text_fields() */

int Cell_variable_set_ode(struct Cell_variable *cell_variable)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Sets the ODE field of the given <cell_variable>'s CMISS interface object to be
true.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_variable_set_ode);
  if (cell_variable)
  {
    if (cell_variable->cmiss_interface)
    {
      Cell_cmiss_interface_set_ode(cell_variable->cmiss_interface);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_variable_set_ode.  "
        "Missing CMISS interface object");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_set_ode.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_set_ode() */
