/*******************************************************************************
FILE : cell_variable_unemap_interface.c

LAST MODIFIED : 06 November 2000

DESCRIPTION :
The interface between Cell_variable's and UnEMAP
==============================================================================*/

#include <stdio.h>
#include <string.h>

#include "cell/cell_interface.h"
#include "cell/cell_variable_unemap_interface.h"
#include "unemap/analysis_work_area.h"

/*
Module objects
--------------
*/
struct Cell_variable_unemap_interface
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
A data object which stores all the information which enables Cell_variable's
to access UnEMAP
==============================================================================*/
{
  /* Arrays to store the variables values through time, which will be used to
   * create the UnEmap signals
   */
  union
  {
    CELL_REAL *real_values;
    CELL_INTEGER *integer_values;
  } values;
  int number_of_values;
  enum Cell_value_type value_type;
  char *name;
}; /* struct Cell_variable_unemap_interface */

/*
Module functions
----------------
*/

/*
Global functions
----------------
*/
struct Cell_variable_unemap_interface *CREATE(Cell_variable_unemap_interface)(
  enum Cell_value_type value_type,char *name)
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Creates a Cell_variable_unemap_interface object.
==============================================================================*/
{
  struct Cell_variable_unemap_interface *cell_variable_unemap_interface;
  
  ENTER(CREATE(Cell_variable_unemap_interface));
  if (ALLOCATE(cell_variable_unemap_interface,
    struct Cell_variable_unemap_interface,1))
  {
    /* Initialise data objects */
    cell_variable_unemap_interface->name = (char *)NULL;
    cell_variable_unemap_interface->value_type = value_type;
    cell_variable_unemap_interface->number_of_values = 0;
    switch (value_type)
    {
      case CELL_REAL_VALUE:
      {
        cell_variable_unemap_interface->values.real_values =
          (CELL_REAL *)NULL;
      } break;
      case CELL_INTEGER_VALUE:
      {
        cell_variable_unemap_interface->values.integer_values =
          (CELL_INTEGER *)NULL;
      } break;
      case CELL_STRING_VALUE:
      {
        display_message(ERROR_MESSAGE,
          "CREATE(Cell_variable_unemap_interface).  "
          "Unable to create UnEmap signals of this string type");
        DEALLOCATE(cell_variable_unemap_interface);
        cell_variable_unemap_interface =
          (struct Cell_variable_unemap_interface *)NULL;
      } break;
      case CELL_UNKNOWN_VALUE:
      default:
      {
        display_message(ERROR_MESSAGE,
          "CREATE(Cell_variable_unemap_interface).  "
          "Unable to create UnEmap signals of unknown type");
        DEALLOCATE(cell_variable_unemap_interface);
        cell_variable_unemap_interface =
          (struct Cell_variable_unemap_interface *)NULL;
      } break;
    } /* switch (value_type) */
    if (ALLOCATE(cell_variable_unemap_interface->name,char,strlen(name)+1))
    {
      strcpy(cell_variable_unemap_interface->name,name);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "CREATE(Cell_variable_unemap_interface).  "
        "Unable to allocate memory for the interface's name");
      DESTROY(Cell_variable_unemap_interface)(&cell_variable_unemap_interface);
      cell_variable_unemap_interface =
        (struct Cell_variable_unemap_interface *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_variable_unemap_interface).  "
      "Unable to allocate memory for the Cell variable UnEMAP interface "
      "object");
    cell_variable_unemap_interface =
      (struct Cell_variable_unemap_interface *)NULL;
  }
  LEAVE;
  return(cell_variable_unemap_interface);
} /* CREATE(Cell_variable_unemap_interface) */

int DESTROY(Cell_variable_unemap_interface)(
  struct Cell_variable_unemap_interface
  **cell_variable_unemap_interface_address)
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Destroys a Cell_variable_unemap_interface object.
==============================================================================*/
{
	int return_code;
  struct Cell_variable_unemap_interface *cell_variable_unemap_interface;

	ENTER(DESTROY(Cell_variable_unemap_interface));
	if (cell_variable_unemap_interface_address &&
    (cell_variable_unemap_interface = *cell_variable_unemap_interface_address))
	{
    if (cell_variable_unemap_interface->name)
    {
      DEALLOCATE(cell_variable_unemap_interface->name);
    }
    switch (cell_variable_unemap_interface->value_type)
    {
      case CELL_REAL_VALUE:
      {
        if (cell_variable_unemap_interface->values.real_values)
        {
          DEALLOCATE(cell_variable_unemap_interface->values.real_values);
        }
      } break;
      case CELL_INTEGER_VALUE:
      {
        if (cell_variable_unemap_interface->values.integer_values)
        {
          DEALLOCATE(cell_variable_unemap_interface->values.integer_values);
        }
      } break;
    }
    DEALLOCATE(*cell_variable_unemap_interface_address);
    *cell_variable_unemap_interface_address =
      (struct Cell_variable_unemap_interface *)NULL;
    return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cell_variable_unemap_interface).  "
      "Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* DESTROY(Cell_variable_unemap_interface) */

int Cell_variable_unemap_interface_add_value(
  struct Cell_variable_unemap_interface *unemap_interface,
  union Cell_cmiss_interface_value_address *value_address)
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Adds the given <value> to the end of the <unemap_interface> time history
array. Allocates the memory required.
==============================================================================*/
{
  int return_code = 0;
  CELL_REAL real_value;
  CELL_INTEGER integer_value;

  ENTER(Cell_variable_unemap_interface_add_value);
  if (unemap_interface && value_address)
  {
    switch (unemap_interface->value_type)
    {
      case CELL_REAL_VALUE:
      {
        unemap_interface->number_of_values++;
        if (REALLOCATE(unemap_interface->values.real_values,
          unemap_interface->values.real_values,CELL_REAL,
          unemap_interface->number_of_values))
        {
          real_value = (CELL_REAL)(*(value_address->double_value_address));
          unemap_interface->values.real_values[
            unemap_interface->number_of_values-1] = real_value;
          return_code = 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,
            "Cell_variable_unemap_interface_add_value.  "
            "Unable to re-allocate memory for the new real value");
          return_code = 0;
        }
      } break;
      case CELL_INTEGER_VALUE:
      {
        unemap_interface->number_of_values++;
        if (REALLOCATE(unemap_interface->values.integer_values,
          unemap_interface->values.integer_values,CELL_INTEGER,
          unemap_interface->number_of_values))
        {
          integer_value = (CELL_INTEGER)
            (*(value_address->integer_value_address));
          unemap_interface->values.integer_values[
            unemap_interface->number_of_values-1] = integer_value;
          return_code = 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,
            "Cell_variable_unemap_interface_add_value.  "
            "Unable to re-allocate memory for the new integer value");
          return_code = 0;
        }
      } break;
      default:
      {
        display_message(ERROR_MESSAGE,
          "Cell_variable_unemap_interface_add_value.  "
          "Bad value type");
        return_code = 0;
      }
    } /* switch (unemap_interface->value_type) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_unemap_interface_add_value.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_unemap_interface_add_value() */

int Cell_variable_unemap_interface_reset_values(
  struct Cell_variable_unemap_interface *unemap_interface)
/*******************************************************************************
LAST MODIFIED : 06 November 2000

DESCRIPTION :
Resets the value array of the given variable <unemap_interface>.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_variable_unemap_interface_reset_values);
  if (unemap_interface)
  {
    unemap_interface->number_of_values = 0;
    switch (unemap_interface->value_type)
    {
      case CELL_REAL_VALUE:
      {
        if (unemap_interface->values.real_values)
        {
          DEALLOCATE(unemap_interface->values.real_values);
          unemap_interface->values.real_values = (CELL_REAL *)NULL;
        }
        return_code = 1;
      } break;
      case CELL_INTEGER_VALUE:
      {
        if (unemap_interface->values.integer_values)
        {
          DEALLOCATE(unemap_interface->values.integer_values);
          unemap_interface->values.integer_values = (CELL_INTEGER *)NULL;
        }
        return_code = 1;
      } break;
      default:
      {
        display_message(ERROR_MESSAGE,
          "Cell_variable_unemap_interface_reset_values.  "
          "Bad value type");
        return_code = 0;
      }
    } /* switch (unemap_interface->value_type) */
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_variable_unemap_interface_reset_values.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_unemap_interface_reset_values() */

char *Cell_variable_unemap_interface_get_value_as_string_at_position(
  struct Cell_variable_unemap_interface *unemap_interface,int position)
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Returns the value of the given <unemap_interface> at the specified position as
a string.
==============================================================================*/
{
  char *value,temp[512];

  ENTER(Cell_variable_unemap_interface_get_value_as_string_at_position);
  if (unemap_interface && (position < unemap_interface->number_of_values))
  {
    switch (unemap_interface->value_type)
    {
      case CELL_REAL_VALUE:
      {
        sprintf(temp,CELL_REAL_FORMAT,
          unemap_interface->values.real_values[position]);
      } break;
      case CELL_INTEGER_VALUE:
      {
        sprintf(temp,CELL_INTEGER_FORMAT,
          unemap_interface->values.integer_values[position]);
      } break;
      default:
      {
        display_message(ERROR_MESSAGE,
          "Cell_variable_unemap_interface_get_value_as_string_at_position.  "
          "Invalid value type");
        sprintf(temp,"");
      } break;
    } /* switch (unemap_interface->value_type) */
    if (ALLOCATE(value,char,strlen(temp)+1))
    {
      strcpy(value,temp);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_variable_unemap_interface_get_value_as_string_at_position.  "
        "Unable to allocate memory for the value string");
      value = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_variable_unemap_interface_get_value_as_string_at_position.  "
      "Invalid argument(s)");
    value = (char *)NULL;
  }
  LEAVE;
  return(value);
} /* Cell_variable_unemap_interface_get_value_as_string_at_position() */

float Cell_variable_unemap_interface_get_value_as_float_at_position(
  struct Cell_variable_unemap_interface *unemap_interface,int position)
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Returns the value of the given <unemap_interface> at the specified position as
a float.
==============================================================================*/
{
  float value;
  
  ENTER(Cell_variable_unemap_interface_get_value_as_float_at_position);
  if (unemap_interface && (position < unemap_interface->number_of_values))
  {
    switch (unemap_interface->value_type)
    {
      case CELL_REAL_VALUE:
      {
        value = (float)(unemap_interface->values.real_values[position]);
      } break;
      case CELL_INTEGER_VALUE:
      {
        value = (float)(unemap_interface->values.integer_values[position]);
      } break;
      default:
      {
        display_message(ERROR_MESSAGE,
          "Cell_variable_unemap_interface_get_value_as_float_at_position.  "
          "Invalid value type");
        value = 0.0;
      } break;
    } /* switch (unemap_interface->value_type) */
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_variable_unemap_interface_get_value_as_float_at_position.  "
      "Invalid argument(s)");
    value = 0.0;
  }
  LEAVE;
  return(value);
} /* Cell_variable_unemap_interface_get_value_as_float_at_position() */

int Cell_variable_unemap_interface_get_number_of_values(
  struct Cell_variable_unemap_interface *unemap_interface)
/*******************************************************************************
LAST MODIFIED : 04 November 2000

DESCRIPTION :
Returns the number of values stored in the <unemap_interface>
==============================================================================*/
{
  int number_of_values;
  
  ENTER(Cell_variable_unemap_interface_get_number_of_values);
  if (unemap_interface)
  {
    number_of_values = unemap_interface->number_of_values;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_variable_unemap_interface_get_number_of_values.  "
      "Invalid argument(s)");
    number_of_values = 0;
  }
  LEAVE;
  return(number_of_values);
} /* Cell_variable_unemap_interface_get_number_of_values() */

char *Cell_variable_unemap_interface_get_name(
  struct Cell_variable_unemap_interface *unemap_interface)
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Returns a copy of the name of the <unemap_interface>
==============================================================================*/
{
  char *name;
  
  ENTER(Cell_variable_unemap_interface_get_name);
  if (unemap_interface)
  {
    if (ALLOCATE(name,char,strlen(unemap_interface->name)+1))
    {
      strcpy(name,unemap_interface->name);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_variable_unemap_interface_get_name.  "
        "Unable to allocate memory for the name");
      name = (char *)NULL;;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_variable_unemap_interface_get_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;;
  }
  LEAVE;
  return(name);
} /* Cell_variable_unemap_interface_get_name() */
