/*******************************************************************************
FILE : cell_cmiss_interface.c

LAST MODIFIED : 11 June 2001

DESCRIPTION :
Routines for using the Cell_cmiss_interface objects which map Cell_variables
into cmiss arrays
==============================================================================*/

#include <stdio.h>
#include <string.h>

#include "cell/cell_cmiss_interface.h"
#include "cell/cell_interface.h"
#include "command/parser.h"

/*
Module objects
--------------
*/
static char *CMISS_array_names[12] = {
  "AII",
  "AIO",
  "ARI",
  "ARO",
  "CONTROL",
  "DERIVED",
  "MODEL",
  "PROTOCOL",
  "PARAMETERS",
  "SIZES",
  "STATE",
  "TIME"
};

enum CMISS_arrays
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
All the possible CMISS arrays. Need to ensure their values for use with the
CMISS array names array above.
==============================================================================*/
{
  AII=0, /* Additional integer input */
  AIO=1, /* Additional integer output */
  ARI=2, /* Additional real input */
  ARO=3, /* Additional real output */
  CONTROL=4, /* Control parameters (integer) */
  DERIVED=5, /* Dervied variables (real) */
  MODEL=6, /* Model parameters (integer) */
  PROTOCOL=7, /* Protocol parameters (real) */
  PARAMETERS=8, /* Material parameters (real) */
  SIZES=9, /* The sizes of the other arrays (integer) */
  STATE=10, /* The state variables (real) */
  TIME=11, /* The time array (real) */
  UNKNOWN_CMISS_ARRAY=12
}; /* enum CMISS_arrays */

struct Cell_cmiss_interface
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
A data object which stores mapping information to transform Cell_variable
objects into cmiss arrays
==============================================================================*/
{
  /* The CMISS array to put the variable in */
  enum CMISS_arrays cmiss_array;
  /* The variable's position in that array */
  int position;
	/* Set this to true for variables which are ODE's */
	int ode;
}; /* struct Cell_cmiss_interface */

/*
Module functions
----------------
*/
static CELL_DOUBLE *add_variable_to_real_cmiss_array(CELL_DOUBLE *array,
  int *size,CELL_DOUBLE value,int position)
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Adds the given variable <value> to the specified <position> (starting at 1) in
the <array>. <size> enters as the current size of the <array> and is returned
set to the final size. Memory is allocated if the required position falls
outside <size>. The final array is returned.
==============================================================================*/
{
  CELL_DOUBLE *new_array = (CELL_DOUBLE *)NULL;

  ENTER(add_variable_to_real_cmiss_array);
  /* check for any memory allocation required */
  if (!array)
  {
    /* need to allocate the array */
    if (ALLOCATE(new_array,CELL_DOUBLE,position))
    {
      *size = position;
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_variable_to_real_cmiss_array.  "
        "Unable to allocate memory for the array");
      new_array = (CELL_DOUBLE *)NULL;
    }
  }
  else if (position >= *size)
  {
    /* need to make the array bigger */
    if (REALLOCATE(new_array,array,CELL_DOUBLE,position))
    {
      *size = position;
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_variable_to_real_cmiss_array.  "
        "Unable to reallocate memory for the array");
      new_array = (CELL_DOUBLE *)NULL;
    }
  }
  else
  {
    /* the size of the current array is sufficient */
    new_array = array;
  }
  if (new_array)
  {
    new_array[position-1] = value;
  }
  LEAVE;
  return(new_array);
} /* add_variable_to_real_cmiss_array() */

static CELL_INTEGER *add_variable_to_integer_cmiss_array(CELL_INTEGER *array,
  int *size,CELL_INTEGER value,int position)
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Adds the given variable <value> to the specified <position> (starting at 1) in
the <array>. <size> enters as the current size of the <array> and is returned
set to the final size. Memory is allocated if the required position falls
outside <size>. The final array is returned.
==============================================================================*/
{
  CELL_INTEGER *new_array = (CELL_INTEGER *)NULL;

  ENTER(add_variable_to_integer_cmiss_array);
  /* check for any memory allocation required */
  if (!array)
  {
    /* need to allocate the array */
    if (ALLOCATE(new_array,CELL_INTEGER,position))
    {
      *size = position;
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_variable_to_integer_cmiss_array.  "
        "Unable to allocate memory for the array");
      new_array = (CELL_INTEGER *)NULL;
    }
  }
  else if (position >= *size)
  {
    /* need to make the array bigger */
    if (REALLOCATE(new_array,array,CELL_INTEGER,position))
    {
      *size = position;
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_variable_to_integer_cmiss_array.  "
        "Unable to reallocate memory for the array");
      new_array = (CELL_INTEGER *)NULL;
    }
  }
  else
  {
    /* the size of the current array is sufficient */
    new_array = array;
  }
  if (new_array)
  {
    new_array[position-1] = value;
  }
  LEAVE;
  return(new_array);
} /* add_variable_to_integer_cmiss_array() */

static char **add_variable_name_to_cmiss_array(char **array,int *size,
  char *name,int position)
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Adds the given variable <name> to the specified <position> (starting at 1) in
the <array>. <size> enters as the current size of the <array> and is returned
set to the final size. Memory is allocated if the required position falls
outside <size>. The final array is returned.
==============================================================================*/
{
  char **new_array = (char **)NULL;

  ENTER(add_variable_name_to_cmiss_array);
  /* check for any memory allocation required */
  if (!array)
  {
    /* need to allocate the array */
    if (ALLOCATE(new_array,char *,position))
    {
      *size = position;
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_variable_name_to_cmiss_array.  "
        "Unable to allocate memory for the array");
      new_array = (char **)NULL;
    }
  }
  else if (position >= *size)
  {
    /* need to make the array bigger */
    if (REALLOCATE(new_array,array,char *,position))
    {
      *size = position;
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_variable_name_to_cmiss_array.  "
        "Unable to reallocate memory for the array");
      new_array = (char **)NULL;
    }
  }
  else
  {
    /* the size of the current array is sufficient */
    new_array = array;
  }
  if (new_array && name && ALLOCATE(new_array[position-1],char,strlen(name)+1))
  {
    strcpy(new_array[position-1],name);
  }
  LEAVE;
  return(new_array);
} /* add_variable_name_to_cmiss_array() */

static int cmiss_interface_arrays_add_variable(
  struct Cell_variable *cell_variable,
  void *cmiss_interface_arrays_void)
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Iterator function used to add variables to the appropriate CMISS arrays in
the <cmiss_interface_arrays> object.
==============================================================================*/
{
  int return_code = 0,size;
  struct Cell_cmiss_interface_arrays *cmiss_interface_arrays;
  struct Cell_cmiss_interface *cmiss_interface;
  char *name;

  ENTER(cmiss_interface_arrays_add_variable);
  if (cell_variable && cmiss_interface_arrays_void &&
    (cmiss_interface_arrays =
      (struct Cell_cmiss_interface_arrays *)cmiss_interface_arrays_void))
  {
    /* get the variable's cmiss interface - if it has one */
    if (cmiss_interface = Cell_variable_get_cmiss_interface(cell_variable))
    {
      name = Cell_variable_get_name(cell_variable);
      /* and add the variable to the appropriate array */
      switch (cmiss_interface->cmiss_array)
      {
        case TIME:
        {
          size = cmiss_interface_arrays->size_time;
          cmiss_interface_arrays->time =
            (CellTime *)add_variable_to_real_cmiss_array(
              (CELL_DOUBLE *)cmiss_interface_arrays->time,
              &(cmiss_interface_arrays->size_time),
              (CELL_DOUBLE)Cell_variable_get_real_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->time_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->time_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case STATE:
        {
          /* Count the number of ODE variables */
          if (cmiss_interface->ode)
          {
            cmiss_interface_arrays->num_odes++;
          }
          size = cmiss_interface_arrays->size_y;
          cmiss_interface_arrays->y =
            (CellY *)add_variable_to_real_cmiss_array(
              (CELL_DOUBLE *)cmiss_interface_arrays->y,
              &(cmiss_interface_arrays->size_y),
              (CELL_DOUBLE)Cell_variable_get_real_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->y_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->y_names,&size,
              name,
              cmiss_interface->position);
          /* special case - need to make DY the same size as Y and intialise */
          size = cmiss_interface_arrays->size_dy;
          cmiss_interface_arrays->dy =
            (CellDY *)add_variable_to_real_cmiss_array(
              (CELL_DOUBLE *)cmiss_interface_arrays->dy,
              &(cmiss_interface_arrays->size_dy),
              (CELL_DOUBLE)0.0,
              cmiss_interface->position);
          cmiss_interface_arrays->dy_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->dy_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case DERIVED:
        {
          size = cmiss_interface_arrays->size_derived;
          cmiss_interface_arrays->derived =
            (CellDerived *)add_variable_to_real_cmiss_array(
              (CELL_DOUBLE *)cmiss_interface_arrays->derived,
              &(cmiss_interface_arrays->size_derived),
              (CELL_DOUBLE)Cell_variable_get_real_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->derived_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->derived_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case PARAMETERS:
        {
          size = cmiss_interface_arrays->size_parameters;
          cmiss_interface_arrays->parameters =
            (CellParameters *)add_variable_to_real_cmiss_array(
              (CELL_DOUBLE *)cmiss_interface_arrays->parameters,
              &(cmiss_interface_arrays->size_parameters),
              (CELL_DOUBLE)Cell_variable_get_real_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->parameters_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->parameters_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case PROTOCOL:
        {
          size = cmiss_interface_arrays->size_protocol;
          cmiss_interface_arrays->protocol =
            (CellProtocol *)add_variable_to_real_cmiss_array(
              (CELL_DOUBLE *)cmiss_interface_arrays->protocol,
              &(cmiss_interface_arrays->size_protocol),
              (CELL_DOUBLE)Cell_variable_get_real_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->protocol_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->protocol_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case ARI:
        {
          size = cmiss_interface_arrays->size_ari;
          cmiss_interface_arrays->ari =
            (CellARI *)add_variable_to_real_cmiss_array(
              (CELL_DOUBLE *)cmiss_interface_arrays->ari,
              &(cmiss_interface_arrays->size_ari),
              (CELL_DOUBLE)Cell_variable_get_real_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->ari_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->ari_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case ARO:
        {
          size = cmiss_interface_arrays->size_aro;
          cmiss_interface_arrays->aro =
            (CellARO *)add_variable_to_real_cmiss_array(
              (CELL_DOUBLE *)cmiss_interface_arrays->aro,
              &(cmiss_interface_arrays->size_aro),
              (CELL_DOUBLE)Cell_variable_get_real_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->aro_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->aro_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case CONTROL:
        {
          size = cmiss_interface_arrays->size_control;
          cmiss_interface_arrays->control =
            (CellControl *)add_variable_to_integer_cmiss_array(
              (CELL_INTEGER *)cmiss_interface_arrays->control,
              &(cmiss_interface_arrays->size_control),
              (CELL_INTEGER)Cell_variable_get_integer_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->control_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->control_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case MODEL:
        {
          size = cmiss_interface_arrays->size_model;
          cmiss_interface_arrays->model =
            (CellModel *)add_variable_to_integer_cmiss_array(
              (CELL_INTEGER *)cmiss_interface_arrays->model,
              &(cmiss_interface_arrays->size_model),
              (CELL_INTEGER)Cell_variable_get_integer_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->model_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->model_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case SIZES:
        {
          size = cmiss_interface_arrays->size_sizes;
          cmiss_interface_arrays->sizes =
            (CellSizes *)add_variable_to_integer_cmiss_array(
              (CELL_INTEGER *)cmiss_interface_arrays->sizes,
              &(cmiss_interface_arrays->size_sizes),
              (CELL_INTEGER)Cell_variable_get_integer_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->sizes_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->sizes_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case AII:
        {
          size = cmiss_interface_arrays->size_aii;
          cmiss_interface_arrays->aii =
            (CellAII *)add_variable_to_integer_cmiss_array(
              (CELL_INTEGER *)cmiss_interface_arrays->aii,
              &(cmiss_interface_arrays->size_aii),
              (CELL_INTEGER)Cell_variable_get_integer_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->aii_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->aii_names,&size,
              name,
              cmiss_interface->position);
        } break;
        case AIO:
        {
          size = cmiss_interface_arrays->size_aio;
          cmiss_interface_arrays->aio =
            (CellAIO *)add_variable_to_integer_cmiss_array(
              (CELL_INTEGER *)cmiss_interface_arrays->aio,
              &(cmiss_interface_arrays->size_aio),
              (CELL_INTEGER)Cell_variable_get_integer_value(cell_variable),
              cmiss_interface->position);
          cmiss_interface_arrays->aio_names =
            add_variable_name_to_cmiss_array(
              cmiss_interface_arrays->aio_names,&size,
              name,
              cmiss_interface->position);
        } break;
      } /* switch (cmiss_interface->cmiss_array) */
      if (name) DEALLOCATE(name);
      return_code = 1;
    }
    else
    {
      /* its not an error to not have a cmiss interface, just ignore the
       * variable
       */
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"cmiss_interface_arrays_add_variable.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* cmiss_interface_arrays_add_variable() */

void set_cmiss_array_from_string(
  struct Cell_cmiss_interface *cell_cmiss_interface,char *array_string)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Sets the interface's array field from the given <array_string>
==============================================================================*/
{
  int i;
  enum CMISS_arrays arrays[13] = {AII,AIO,ARI,ARO,CONTROL,DERIVED,MODEL,
                                  PROTOCOL,PARAMETERS,SIZES,STATE,TIME,
                                  UNKNOWN_CMISS_ARRAY};
  
  ENTER(set_cmiss_array_from_string);
  if (cell_cmiss_interface && array_string)
  {
    /* find the corresponding array name */
    i=0;
    while ((i<12) &&
      !fuzzy_string_compare_same_length(array_string,CMISS_array_names[i]))
    {
      i++;
    }
    /* and set the cmiss array field in the interface object */
    cell_cmiss_interface->cmiss_array = arrays[i];
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_cmiss_array_from_string.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* set_cmiss_array_from_string() */

/*
Global functions
----------------
*/
struct Cell_cmiss_interface *CREATE(Cell_cmiss_interface)(char *array_string,
  int position)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Creates a Cell_cmiss_interface object. <array_string> is used to set the
variable's CMISS array and <position> is the variable's position in that array.
==============================================================================*/
{
  struct Cell_cmiss_interface *cell_cmiss_interface;

  ENTER(CREATE(Cell_cmiss_interface));
  if (ALLOCATE(cell_cmiss_interface,struct Cell_cmiss_interface,1))
  {
    /* Initialise data objects */
    cell_cmiss_interface->cmiss_array = UNKNOWN_CMISS_ARRAY;
    cell_cmiss_interface->position = position;
    cell_cmiss_interface->ode = 0;
    /* Set the cmiss array */
    set_cmiss_array_from_string(cell_cmiss_interface,array_string);
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_cmiss_interface).  "
      "Unable to allocate memory for the Cell CMISS interface object");
    cell_cmiss_interface = (struct Cell_cmiss_interface *)NULL;
  }
  LEAVE;
  return(cell_cmiss_interface);
} /* CREATE(Cell_cmiss_interface) */

int DESTROY(Cell_cmiss_interface)(
  struct Cell_cmiss_interface **cell_cmiss_interface_address)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Destroys a Cell_cmiss_interface object.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Cell_cmiss_interface));
	if (cell_cmiss_interface_address)
	{
    DEALLOCATE(*cell_cmiss_interface_address);
    *cell_cmiss_interface_address = (struct Cell_cmiss_interface *)NULL;
    return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_cmiss_interface).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* DESTROY(Cell_cmiss_interface) */

void Cell_cmiss_interface_list(
  struct Cell_cmiss_interface *cell_cmiss_interface)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Lists out the <cell_cmiss_interface>
==============================================================================*/
{
	ENTER(Cell_cmiss_interface_list)
	if (cell_cmiss_interface)
	{
    if (cell_cmiss_interface->cmiss_array != UNKNOWN_CMISS_ARRAY)
    {
      display_message(INFORMATION_MESSAGE,"  CMISS array: %s[%d]\n",
        CMISS_array_names[cell_cmiss_interface->cmiss_array],
        cell_cmiss_interface->position);
    }
    else
    {
      display_message(INFORMATION_MESSAGE,"  Unknown CMISS array\n");
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_cmiss_interface_list.  Invalid argument(s)");
	}
	LEAVE;
} /* Cell_cmiss_interface_list */

struct Cell_cmiss_interface_arrays *CREATE(Cell_cmiss_interface_arrays)(
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Creates a Cell_cmiss_interface_array object. If <variable_list> is non-NULL it
is used to create the CMISS arrays, otherwise they will all be NULL.
==============================================================================*/
{
  struct Cell_cmiss_interface_arrays *cell_cmiss_interface_arrays;

  ENTER(CREATE(Cell_cmiss_interface_arrays));
  if (ALLOCATE(cell_cmiss_interface_arrays,
    struct Cell_cmiss_interface_arrays,1))
  {
    /* Initialise data objects */
    cell_cmiss_interface_arrays->size_time = 0;
    cell_cmiss_interface_arrays->time = (CellTime *)NULL;
    cell_cmiss_interface_arrays->time_names = (char **)NULL;
    cell_cmiss_interface_arrays->num_odes = 0;
    cell_cmiss_interface_arrays->size_y = 0;
    cell_cmiss_interface_arrays->y = (CellY *)NULL;
    cell_cmiss_interface_arrays->y_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_dy = 0;
    cell_cmiss_interface_arrays->dy = (CellDY *)NULL;
    cell_cmiss_interface_arrays->dy_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_derived = 0;
    cell_cmiss_interface_arrays->derived = (CellDerived *)NULL;
    cell_cmiss_interface_arrays->derived_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_parameters = 0;
    cell_cmiss_interface_arrays->parameters = (CellParameters *)NULL;
    cell_cmiss_interface_arrays->parameters_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_protocol = 0;
    cell_cmiss_interface_arrays->protocol = (CellProtocol *)NULL;
    cell_cmiss_interface_arrays->protocol_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_ari = 0;
    cell_cmiss_interface_arrays->ari = (CellARI *)NULL;
    cell_cmiss_interface_arrays->ari_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_aro = 0;
    cell_cmiss_interface_arrays->aro = (CellARO *)NULL;
    cell_cmiss_interface_arrays->aro_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_control = 0;
    cell_cmiss_interface_arrays->control = (CellControl *)NULL;
    cell_cmiss_interface_arrays->control_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_model = 0;
    cell_cmiss_interface_arrays->model = (CellModel *)NULL;
    cell_cmiss_interface_arrays->model_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_sizes = 0;
    cell_cmiss_interface_arrays->sizes = (CellSizes *)NULL;
    cell_cmiss_interface_arrays->sizes_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_variant = 0;
    cell_cmiss_interface_arrays->variant = (CellVariant *)NULL;
    cell_cmiss_interface_arrays->variant_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_aii = 0;
    cell_cmiss_interface_arrays->aii = (CellAII *)NULL;
    cell_cmiss_interface_arrays->aii_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_aio = 0;
    cell_cmiss_interface_arrays->aio = (CellAIO *)NULL;
    cell_cmiss_interface_arrays->aio_names = (char **)NULL;
    cell_cmiss_interface_arrays->size_error_code = 0;
    cell_cmiss_interface_arrays->error_code = (CellErrorCode *)NULL;
    cell_cmiss_interface_arrays->error_code_names = (char **)NULL;
    /* Set-up the arrays from any user defined data */
    if (variable_list)
    {
      FOR_EACH_OBJECT_IN_LIST(Cell_variable)(
        cmiss_interface_arrays_add_variable,(void *)cell_cmiss_interface_arrays,
        variable_list);
    }
    /* and make sure everything is all right */
    /* The TIME array must have at least 2 elements */
    if (!(cell_cmiss_interface_arrays->time))
    {
      if (!ALLOCATE(cell_cmiss_interface_arrays->time,CellTime,2))
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_cmiss_interface_arrays).  "
          "Unable to allocate memory for the TIME array");
      }
    }
    else if (cell_cmiss_interface_arrays->size_time < 2)
    {
      if (!REALLOCATE(cell_cmiss_interface_arrays->time,
        cell_cmiss_interface_arrays->time,CellTime,2))
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_cmiss_interface_arrays).  "
          "Unable to reallocate memory for the TIME array");
      }
    }
    /* The CONTROL array must have at least 1 element */
    if (!(cell_cmiss_interface_arrays->control))
    {
      if (!ALLOCATE(cell_cmiss_interface_arrays->control,CellControl,1))
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_cmiss_interface_arrays).  "
          "Unable to allocate memory for the CONTROL array");
      }
    }
    /* The VARIANT must have 1 element */
    if (!(cell_cmiss_interface_arrays->variant))
    {
      if (!ALLOCATE(cell_cmiss_interface_arrays->variant,CellVariant,1))
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_cmiss_interface_arrays).  "
          "Unable to allocate memory for the VARIANT array");
      }
    }
    /* The ERROR_CODE must have 1 element */
    if (!(cell_cmiss_interface_arrays->error_code))
    {
      if (!ALLOCATE(cell_cmiss_interface_arrays->error_code,CellErrorCode,1))
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_cmiss_interface_arrays).  "
          "Unable to allocate memory for the ERROR_CODE array");
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_cmiss_interface_arrays).  "
      "Unable to allocate memory for the Cell CMISS interface arrays object");
    cell_cmiss_interface_arrays = (struct Cell_cmiss_interface_arrays *)NULL;
  }
  LEAVE;
  return(cell_cmiss_interface_arrays);
} /* CREATE(Cell_cmiss_interface_arrays) */

int DESTROY(Cell_cmiss_interface_arrays)(
  struct Cell_cmiss_interface_arrays **cell_cmiss_interface_arrays_address)
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Destroys a Cell_cmiss_interface_arrays object.
==============================================================================*/
{
	int return_code,i;
  struct Cell_cmiss_interface_arrays *cell_cmiss_interface_arrays;

	ENTER(DESTROY(Cell_cmiss_interface_arrays));
	if (cell_cmiss_interface_arrays_address &&
    (cell_cmiss_interface_arrays = *cell_cmiss_interface_arrays_address))
	{
    if (cell_cmiss_interface_arrays->time)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->time);
      for (i=0;i<cell_cmiss_interface_arrays->size_time;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->time_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->time_names);
      cell_cmiss_interface_arrays->size_time = 0;
    }
    if (cell_cmiss_interface_arrays->y)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->y);
      for (i=0;i<cell_cmiss_interface_arrays->size_y;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->y_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->y_names);
      cell_cmiss_interface_arrays->size_y = 0;
      cell_cmiss_interface_arrays->num_odes = 0;
    }
    if (cell_cmiss_interface_arrays->dy)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->dy);
      for (i=0;i<cell_cmiss_interface_arrays->size_dy;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->dy_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->dy_names);
      cell_cmiss_interface_arrays->size_dy = 0;
    }
    if (cell_cmiss_interface_arrays->derived)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->derived);
      for (i=0;i<cell_cmiss_interface_arrays->size_derived;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->derived_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->derived_names);
      cell_cmiss_interface_arrays->size_derived = 0;
    }
    if (cell_cmiss_interface_arrays->parameters)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->parameters);
      for (i=0;i<cell_cmiss_interface_arrays->size_parameters;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->parameters_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->parameters_names);
      cell_cmiss_interface_arrays->size_parameters = 0;
    }
    if (cell_cmiss_interface_arrays->protocol)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->protocol);
      for (i=0;i<cell_cmiss_interface_arrays->size_protocol;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->protocol_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->protocol_names);
      cell_cmiss_interface_arrays->size_protocol = 0;
    }
    if (cell_cmiss_interface_arrays->ari)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->ari);
      for (i=0;i<cell_cmiss_interface_arrays->size_ari;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->ari_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->ari_names);
      cell_cmiss_interface_arrays->size_ari = 0;
    }
    if (cell_cmiss_interface_arrays->aro)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->aro);
      for (i=0;i<cell_cmiss_interface_arrays->size_aro;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->aro_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->aro_names);
      cell_cmiss_interface_arrays->size_aro = 0;
    }
    if (cell_cmiss_interface_arrays->control)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->control);
      for (i=0;i<cell_cmiss_interface_arrays->size_control;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->control_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->control_names);
      cell_cmiss_interface_arrays->size_control = 0;
    }
    if (cell_cmiss_interface_arrays->model)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->model);
      for (i=0;i<cell_cmiss_interface_arrays->size_model;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->model_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->model_names);
      cell_cmiss_interface_arrays->size_model = 0;
    }
    if (cell_cmiss_interface_arrays->sizes)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->sizes);
      for (i=0;i<cell_cmiss_interface_arrays->size_sizes;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->sizes_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->sizes_names);
      cell_cmiss_interface_arrays->size_sizes = 0;
    }
    if (cell_cmiss_interface_arrays->variant)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->variant);
      for (i=0;i<cell_cmiss_interface_arrays->size_variant;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->variant_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->variant_names);
      cell_cmiss_interface_arrays->size_variant = 0;
    }
    if (cell_cmiss_interface_arrays->aii)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->aii);
      for (i=0;i<cell_cmiss_interface_arrays->size_aii;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->aii_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->aii_names);
      cell_cmiss_interface_arrays->size_aii = 0;
    }
    if (cell_cmiss_interface_arrays->aio)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->aio);
      for (i=0;i<cell_cmiss_interface_arrays->size_aio;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->aio_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->aio_names);
      cell_cmiss_interface_arrays->size_aio = 0;
    }
    if (cell_cmiss_interface_arrays->error_code)
    {
      DEALLOCATE(cell_cmiss_interface_arrays->error_code);
      for (i=0;i<cell_cmiss_interface_arrays->size_error_code;i++)
      {
        DEALLOCATE(cell_cmiss_interface_arrays->error_code_names[i]);
      }
      DEALLOCATE(cell_cmiss_interface_arrays->error_code_names);
      cell_cmiss_interface_arrays->size_error_code = 0;
    }
    DEALLOCATE(*cell_cmiss_interface_arrays_address);
    *cell_cmiss_interface_arrays_address =
      (struct Cell_cmiss_interface_arrays *)NULL;
    return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_cmiss_interface_arrays).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* DESTROY(Cell_cmiss_interface_arrays) */

union Cell_cmiss_interface_value_address
*Cell_cmiss_interface_get_variable_value_address(
  struct Cell_cmiss_interface *cmiss_interface,
  struct Cell_cmiss_interface_arrays *cell_cmiss_interface_arrays)
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Returns a pointer to the address of the given variable's position in the
<cmiss_arrays>.
==============================================================================*/
{
  union Cell_cmiss_interface_value_address *value_address;

  ENTER(Cell_cmiss_interface_get_value_address);
  if (cmiss_interface && cell_cmiss_interface_arrays)
  {
    if (ALLOCATE(value_address,union Cell_cmiss_interface_value_address,1))
    {
      switch (cmiss_interface->cmiss_array)
      {
        case TIME:
        {
          value_address->double_value_address = (CELL_DOUBLE *)
            ((cell_cmiss_interface_arrays->time)+
              ((cmiss_interface->position)-1));
        } break;
        case STATE:
        {
          value_address->double_value_address = (CELL_DOUBLE *)
            ((cell_cmiss_interface_arrays->y)+((cmiss_interface->position)-1));
        } break;
        case DERIVED:
        {
          value_address->double_value_address = (CELL_DOUBLE *)
            ((cell_cmiss_interface_arrays->derived)+
              ((cmiss_interface->position)-1));
        } break;
        case PARAMETERS:
        {
          value_address->double_value_address = (CELL_DOUBLE *)
            ((cell_cmiss_interface_arrays->parameters)+
              ((cmiss_interface->position)-1));
        } break;
        case PROTOCOL:
        {
          value_address->double_value_address = (CELL_DOUBLE *)
            ((cell_cmiss_interface_arrays->protocol)+
              ((cmiss_interface->position)-1));
        } break;
        case ARI:
        {
          value_address->double_value_address = (CELL_DOUBLE *)
            ((cell_cmiss_interface_arrays->ari)+
              ((cmiss_interface->position)-1));
        } break;
        case ARO:
        {
          value_address->double_value_address = (CELL_DOUBLE *)
            ((cell_cmiss_interface_arrays->aro)+
              ((cmiss_interface->position)-1));
        } break;
        case CONTROL:
        {
          value_address->integer_value_address = (CELL_INTEGER *)
            ((cell_cmiss_interface_arrays->control)+
              ((cmiss_interface->position)-1));
        } break;
        case MODEL:
        {
          value_address->integer_value_address = (CELL_INTEGER *)
            ((cell_cmiss_interface_arrays->model)+
              ((cmiss_interface->position)-1));
        } break;
        case SIZES:
        {
          value_address->integer_value_address = (CELL_INTEGER *)
            ((cell_cmiss_interface_arrays->sizes)+
              ((cmiss_interface->position)-1));
        } break;
        case AII:
        {
          value_address->integer_value_address = (CELL_INTEGER *)
            ((cell_cmiss_interface_arrays->aii)+
              ((cmiss_interface->position)-1));
        } break;
        case AIO:
        {
          value_address->integer_value_address = (CELL_INTEGER *)
            ((cell_cmiss_interface_arrays->aio)+
              ((cmiss_interface->position)-1));
        } break;
      } /* switch (cmiss_interface->cmiss_array) */
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_cmiss_interface_get_value_address.  "
        "Unable to allocate memory for the value_address");
      value_address = (union Cell_cmiss_interface_value_address *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_cmiss_interface_get_value_address.  "
      "Invalid argument(s)");
    value_address = (union Cell_cmiss_interface_value_address *)NULL;
  }
  LEAVE;
  return(value_address);
} /* Cell_cmiss_interface_get_value_address() */

int Cell_cmiss_interface_set_ode(struct Cell_cmiss_interface *cmiss_interface)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Sets the ode field of the given <cmiss_interface> object to be true.
==============================================================================*/
{
  int return_code;

  ENTER(Cell_cmiss_interface_set_ode);
  if (cmiss_interface)
  {
    cmiss_interface->ode = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_cmiss_interface_set_ode.  "
      "Invalid argument(s)");
    return_code = -1;
  }
  LEAVE;
  return(return_code);
} /* Cell_cmiss_interface_set_ode() */
