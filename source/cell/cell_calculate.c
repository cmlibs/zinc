/*******************************************************************************
FILE : cell_calculate.c

LAST MODIFIED : 15 March 2001

DESCRIPTION :
Routines for model calculation
==============================================================================*/

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "cell/cell_calculate.h"
#include "cell/cell_calculate_dialog.h"

/* Control array indices - from
 *   /product/cmiss/cm/source/integrator_reserved.inc
 */
#define SIZE_INTEGER_CONTROL 10
#define SIZE_REAL_CONTROL 3
#define ERROR_CODE_IDX 0 /* 1 - 1 */
#define INTEGER_WORK_SIZE_IDX 6 /* 7 - 1 */
#define REAL_WORK_SIZE_IDX 7 /* 8 - 1 */
#define GET_WORK_SIZES_IDX 8 /* 9 - 1 */
#define SET_DEFAULT_VALUES_IDX 9 /* 10 - 1 */

/*
Module objects
--------------
*/
struct Unemap_interface_value
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
A local data object used to keep track of the required CMISS variables to be
viewed in UnEmap.
==============================================================================*/
{
  struct Cell_variable_unemap_interface *variable_unemap_interface;
  union Cell_cmiss_interface_value_address *variable_value_address;
  struct Unemap_interface_value *next;
}; /* struct Unemap_interface_value */

struct Wrapper
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Wrapper used in a cell variabl iterator function.
==============================================================================*/
{
  struct Unemap_interface_value *value;
  struct Cell_cmiss_interface_arrays *cmiss_arrays;
}; /* struct Wrapper */

struct Cell_calculate
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
A data object which stores information used in the calculation of models
==============================================================================*/
{
  /* The cell model's main routine name */
  char *model_routine_name;
  /* The name of the file (DSO) containing the above routine - a NULL here
   * simply means that the routine is compiled into Cell */
  char *dso_name;
  /* The routine name and DSO for the time integrator */
  char *intg_routine_name;
  char *intg_dso_name;
  /* Integration parameters */
  float Tstart,Tend,dT,tabT;
  /* Need a handle on the UnEmap interface */
  struct Cell_unemap_interface *cell_unemap_interface;
  /* The calculate dialog */
  struct Cell_calculate_dialog *dialog;
}; /* struct Cell_calculate */

/*
Module functions
----------------
*/
int unemap_interface_value_add_variable(struct Cell_variable *cell_variable,
  void *wrapper_void)
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Iterator function used to add variables to the UnEmap interface values.
==============================================================================*/
{
  int return_code = 0;
  struct Wrapper *wrapper;
  struct Unemap_interface_value *value;
  struct Cell_variable_unemap_interface *unemap_interface;

  ENTER(unemap_interface_value_add_variable);
  if (cell_variable && (wrapper = (struct Wrapper *)wrapper_void))
  {
    if (unemap_interface = Cell_variable_get_unemap_interface(cell_variable))
    {
      /* The variable has a UnEmap interface, so need to allocate a
         Unemap_interface_value for it  - first get the last current value */
      value = wrapper->value;
      if (value)
      {
        while (value->next)
        {
          value = value->next;
        }
        if (ALLOCATE(value->next,struct Unemap_interface_value,1))
        {
          value = value->next;
        }
        else
        {
          display_message(ERROR_MESSAGE,"unemap_interface_value_add_variable.  "
            "Unable to allocate memory for the new value");
          value = (struct Unemap_interface_value *)NULL;
          return_code = 0;
        }
      }
      else if (ALLOCATE(value,struct Unemap_interface_value,1))
      {
        /* need to keep a handle on the list of values for the interator
           function */
        wrapper->value = value;
      }
      else
      {
        display_message(ERROR_MESSAGE,"unemap_interface_value_add_variable.  "
          "Unable to allocate memory for the new value");
        value = (struct Unemap_interface_value *)NULL;
        return_code = 0;
      }
      if (value)
      {
        /* Initialise data objects */
        value->variable_unemap_interface = unemap_interface;
        value->variable_value_address =
          (union Cell_cmiss_interface_value_address *)NULL;
        value->next = (struct Unemap_interface_value *)NULL;
        /* Set-up the data */
        if (value->variable_value_address =
          Cell_cmiss_interface_get_variable_value_address(
            Cell_variable_get_cmiss_interface(cell_variable),
            wrapper->cmiss_arrays))
        {
          return_code = 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,"unemap_interface_value_add_variable.  "
            "Unable to get the value address for this variable");
          return_code = 0;
        }
      } /* if (value) */
    }
    else
    {
      /* This variable doesn't have a UnEmap interface, so don't need to worry
         about it */
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"unemap_interface_value_add_variable.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* unemap_interface_value_add_variable() */

static struct Unemap_interface_value *CREATE(Unemap_interface_value)(
  struct LIST(Cell_variable) *variable_list,
  struct Cell_cmiss_interface_arrays *cmiss_arrays)
/*******************************************************************************
LAST MODIFIED : 02 November 2000

DESCRIPTION :
Creates all the UnEmap interface value structures.
==============================================================================*/
{
  struct Wrapper wrapper;
  
  ENTER(CREATE(Unemap_interface_value));
  if (variable_list && cmiss_arrays)
  {
    wrapper.value = (struct Unemap_interface_value *)NULL;
    wrapper.cmiss_arrays = cmiss_arrays;
    FOR_EACH_OBJECT_IN_LIST(Cell_variable)(
      unemap_interface_value_add_variable,(void *)(&wrapper),
      variable_list);
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Unemap_interface_value).  "
      "Invalid argument(s)");
    wrapper.value = (struct Unemap_interface_value *)NULL;
  }
  LEAVE;
  return(wrapper.value);
} /* CREATE(Unemap_interface_value) */

static int DESTROY(Unemap_interface_value)(
  struct Unemap_interface_value **values)
/*******************************************************************************
LAST MODIFIED : 02 November 2000

DESCRIPTION :
Destroys all the UnEmap interface value structures.
==============================================================================*/
{
  struct Unemap_interface_value *unemap_interface_value,*next;
  int return_code = 0;
  
  ENTER(DESTROY(Unemap_interface_value));
  if (values && (unemap_interface_value = *values))
  {
    next = unemap_interface_value->next;
    DEALLOCATE(unemap_interface_value);
    *values = (struct Unemap_interface_value *)NULL;
    if (next)
    {
      DESTROY(Unemap_interface_value)(&next);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"DESTROY(Unemap_interface_value).  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* DESTROY(Unemap_interface_value) */

static int update_unemap_interface_values(struct Unemap_interface_value *values)
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Updates the values stored through time for each of the UnEmap variables.
==============================================================================*/
{
  int return_code = 0;
  struct Unemap_interface_value *value;
  
  ENTER(update_unemap_interface_values);
  if (values)
  {
    /* loop through all the values updating them */
    value = values;
    return_code = 1;
    while (value && return_code)
    {
      return_code = Cell_variable_unemap_interface_add_value(
        value->variable_unemap_interface,value->variable_value_address);
      value = value->next;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"update_unemap_interface_values.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* update_unemap_interface_values() */

static int reset_unemap_interface_values(struct Unemap_interface_value *values)
/*******************************************************************************
LAST MODIFIED : 06 November 2000

DESCRIPTION :
Resets the values stored through time for each of the UnEmap variables.
==============================================================================*/
{
  int return_code = 0;
  struct Unemap_interface_value *value;
  
  ENTER(reset_unemap_interface_values);
  if (values)
  {
    /* loop through all the values reseting them */
    value = values;
    return_code = 1;
    while (value && return_code)
    {
      return_code = Cell_variable_unemap_interface_reset_values(
        value->variable_unemap_interface);
      value = value->next;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"reset_unemap_interface_values.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* reset_unemap_interface_values() */

static int initialise_control_arrays(CELL_INTEGER *integer_control,
  CELL_DOUBLE *real_control,struct Cell_cmiss_interface_arrays *cmiss_arrays,
  CellModelRoutine model_routine_handle,
  CellIntegratorRoutine intg_routine_handle)
/*******************************************************************************
LAST MODIFIED : 22 February 2001

DESCRIPTION :
Initialises the integrator control arrays and sets the work array sizes
==============================================================================*/
{
  int return_code = 0;
  CELL_DOUBLE T,DT;
  CELL_INTEGER NUM_ODES,NUM_VARS;
  char error_string[1000];
  
  ENTER(intialise_control_arrays);
  if (cmiss_arrays && integer_control && real_control &&
    model_routine_handle && intg_routine_handle)
  {
    T = (CELL_DOUBLE)0.0;
    DT = (CELL_DOUBLE)0.0;
    NUM_ODES = (CELL_INTEGER)cmiss_arrays->num_odes;
    NUM_VARS = (CELL_INTEGER)cmiss_arrays->size_y;
    /* Get the default control values from the integration routine */
    integer_control[ERROR_CODE_IDX] = (CELL_INTEGER)0;
    integer_control[GET_WORK_SIZES_IDX] = (CELL_INTEGER)0;
    integer_control[SET_DEFAULT_VALUES_IDX] = (CELL_INTEGER)1;
    intg_routine_handle(cmiss_arrays->aii,cmiss_arrays->aio,
      cmiss_arrays->control,cmiss_arrays->model,&NUM_ODES,&NUM_VARS,
      cmiss_arrays->sizes,cmiss_arrays->variant,cmiss_arrays->ari,
      cmiss_arrays->aro,cmiss_arrays->derived,cmiss_arrays->parameters,
      cmiss_arrays->protocol,&T,&DT,cmiss_arrays->time,cmiss_arrays->y,
      model_routine_handle,integer_control,real_control,(CELL_INTEGER *)NULL,
      (CELL_DOUBLE *)NULL,error_string);
    /* Get the work array sizes from the integration routine */
    integer_control[ERROR_CODE_IDX] = (CELL_INTEGER)0;
    integer_control[GET_WORK_SIZES_IDX] = (CELL_INTEGER)1;
    integer_control[SET_DEFAULT_VALUES_IDX] = (CELL_INTEGER)0;
    intg_routine_handle(cmiss_arrays->aii,cmiss_arrays->aio,
      cmiss_arrays->control,cmiss_arrays->model,&NUM_ODES,&NUM_VARS,
      cmiss_arrays->sizes,cmiss_arrays->variant,cmiss_arrays->ari,
      cmiss_arrays->aro,cmiss_arrays->derived,cmiss_arrays->parameters,
      cmiss_arrays->protocol,&T,&DT,cmiss_arrays->time,cmiss_arrays->y,
      model_routine_handle,integer_control,real_control,(CELL_INTEGER *)NULL,
      (CELL_DOUBLE *)NULL,error_string);
    /* Check the sizes, can have trouble allocating 0 size arrays! */
    if (integer_control[INTEGER_WORK_SIZE_IDX] < 1)
    {
      integer_control[INTEGER_WORK_SIZE_IDX] = 1;
    }
    if (integer_control[REAL_WORK_SIZE_IDX] < 1)
    {
      integer_control[REAL_WORK_SIZE_IDX] = 1;
    }
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"initialise_control_arrays.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* initialise_control_arrays() */

static int integrate_model(float Tstart,float Tend,float dT,
  struct Cell_cmiss_interface_arrays *cmiss_arrays,
  CELL_INTEGER *integer_control,CELL_DOUBLE *real_control,
  CELL_INTEGER *integer_work,CELL_DOUBLE *real_work,
  CellModelRoutine model_routine_handle,
  CellIntegratorRoutine intg_routine_handle)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Integrates a cell model from <Tstart> to <Tend> in steps of <dT>
==============================================================================*/
{
  int return_code = 0;
  float time;
  CELL_DOUBLE T,DT;
  CELL_INTEGER NUM_ODES,NUM_VARS;
  char error_string[1000];
  
  ENTER(integrate_model);
  if (cmiss_arrays && model_routine_handle && intg_routine_handle)
  {
    DT = (CELL_DOUBLE)dT;
    NUM_ODES = (CELL_INTEGER)cmiss_arrays->num_odes;
    NUM_VARS = (CELL_INTEGER)cmiss_arrays->size_y;
    for (time=Tstart;time<Tend;time+=dT)
    {
      T = (CELL_DOUBLE)time;
      /* Integrate the ODE variables */
      cmiss_arrays->control[0] = (CellControl)1; /* ODE */
      intg_routine_handle(cmiss_arrays->aii,cmiss_arrays->aio,
        cmiss_arrays->control,cmiss_arrays->model,&NUM_ODES,&NUM_VARS,
        cmiss_arrays->sizes,cmiss_arrays->variant,cmiss_arrays->ari,
        cmiss_arrays->aro,cmiss_arrays->derived,cmiss_arrays->parameters,
        cmiss_arrays->protocol,&T,&DT,cmiss_arrays->time,cmiss_arrays->y,
        model_routine_handle,integer_control,real_control,integer_work,
        real_work,error_string);
    } /* time loop */
    /* Evaluate the non-ODE variables */
    cmiss_arrays->control[0] = (CellControl)2; /* Not-ODE */
    model_routine_handle(cmiss_arrays->time,cmiss_arrays->y,cmiss_arrays->dy,
      cmiss_arrays->control,cmiss_arrays->model,cmiss_arrays->sizes,
      cmiss_arrays->variant,cmiss_arrays->derived,cmiss_arrays->parameters,
      cmiss_arrays->protocol,cmiss_arrays->aii,cmiss_arrays->aio,
      cmiss_arrays->ari,cmiss_arrays->aro,cmiss_arrays->error_code);
    /* ?? Should probably do something with error_code[0] to set
       ?? the return_code properly ?? */
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"integrate_model.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* integrate_model() */

static void *get_routine_handle(void *dso_handle,char *name)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Returns a handle on the function given be the specified <name> if found in the
given <dso_handle>. Will also check for <name>_ and <name>__ to look for SGI and
Linux fortran routines. Linux uses one _ for Fortran routines and two _'s for
Fortran routines which have at least one _ in the routine name.
==============================================================================*/
{
  void *handle;
  char *routine_name;

  ENTER(get_routine_handle);
  if (dso_handle && name)
  {
    if (ALLOCATE(routine_name,char,strlen(name)+3))
    {
      handle = (void *)NULL;
      /* first try the name given */
      strcpy(routine_name,name);
      if (!(handle = dlsym(dso_handle,routine_name)))
      {
        /* the name given didn't work, maybe its a fortran routine */
        strcat(routine_name,"_");
        if (!(handle = dlsym(dso_handle,routine_name)))
        {
          /* the name given didn't work, try another _ for fortran under
             linux with a "_" (or more) in the routine name */
          strcat(routine_name,"_");
          if (!(handle = dlsym(dso_handle,routine_name)))
          {
            display_message(ERROR_MESSAGE,"get_routine_handle.  "
              "Unable to find routine \"%s\"",
              name);
            handle = (void *)NULL;
          }
        }
      }
      DEALLOCATE(routine_name);
    }
    else
    {
      display_message(WARNING_MESSAGE,"get_routine_handle.  "
        "Unable to allocate memory for the routine name string");
      handle = (void *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"get_routine_handle.  "
      "Invalid argument(s)");
    handle = (void *)NULL;
  }
  LEAVE;
  return(handle);
} /* get_routine_handle() */

static int calculate_model(struct Cell_calculate *cell_calculate,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Calculates a cell model
==============================================================================*/
{
  int return_code = 0,number_of_signals;
  void *model_dso_handle;
  void *intg_dso_handle;
  CellModelRoutine model_routine_handle;
  CellIntegratorRoutine intg_routine_handle;
  struct Cell_cmiss_interface_arrays *cmiss_interface_arrays;
  struct Unemap_interface_value *unemap_interface_values,*value;
  struct Cell_variable_unemap_interface **variable_unemap_interfaces;
  float current_time,end_time;
  CELL_INTEGER integer_control[SIZE_INTEGER_CONTROL],*integer_work;
  CELL_DOUBLE real_control[SIZE_REAL_CONTROL],*real_work;

  ENTER(calculate_model);
  if (cell_calculate && variable_list)
  {
    /* first try and get a handle on the model routine */
    if (model_dso_handle =
      dlopen(cell_calculate->dso_name,RTLD_LAZY|RTLD_LOCAL))
    {
      if (model_routine_handle =
        (CellModelRoutine)get_routine_handle(model_dso_handle,
        cell_calculate->model_routine_name))
      {
        /* and then try and get a handle on the integrator routine */
        if (intg_dso_handle =
          dlopen(cell_calculate->intg_dso_name,RTLD_LAZY|RTLD_LOCAL))
        {
          if (intg_routine_handle =
            (CellIntegratorRoutine)get_routine_handle(intg_dso_handle,
            cell_calculate->intg_routine_name))
          {
            /* Successfully obtained a handle on the model routine and the
             * integrator routine, can now solve the model!!
             */
            /* First need to build all the arrays to pass through */
            if (cmiss_interface_arrays =
              CREATE(Cell_cmiss_interface_arrays)(variable_list))
            {
              if (unemap_interface_values =
                CREATE(Unemap_interface_value)(variable_list,
                  cmiss_interface_arrays))
              {
                /* Reset the variable UnEmap values */
                reset_unemap_interface_values(unemap_interface_values);
                /* store the intial values */
                if (update_unemap_interface_values(unemap_interface_values))
                {
                  /* Set-up the integrator - first set the default control
                     parameter values and the work array sizes */
                  if (initialise_control_arrays(integer_control,real_control,
                    cmiss_interface_arrays,model_routine_handle,
                    intg_routine_handle))
                  {
                    /* And then allocate memory for the work arrays */
                    integer_work = (CELL_INTEGER *)NULL;
                    real_work = (CELL_DOUBLE *)NULL;
                    if (ALLOCATE(integer_work,CELL_INTEGER,
                      integer_control[INTEGER_WORK_SIZE_IDX]) &&
                      ALLOCATE(real_work,CELL_DOUBLE,
                        integer_control[REAL_WORK_SIZE_IDX]))
                    {
                      return_code = 1;
                      /* Now integrate through time, grabbing the required
                       * variable values at each tabT
                       */
                      integer_control[ERROR_CODE_IDX] = (CELL_INTEGER)0;
                      integer_control[GET_WORK_SIZES_IDX] = (CELL_INTEGER)0;
                      integer_control[SET_DEFAULT_VALUES_IDX] =
                        (CELL_INTEGER)0;
                      current_time = cell_calculate->Tstart;
                      while (return_code &&
                        (current_time < cell_calculate->Tend))
                      {
                        end_time = current_time+cell_calculate->tabT;
                        if (return_code = integrate_model(current_time,
                          end_time,cell_calculate->dT,cmiss_interface_arrays,
                          integer_control,real_control,integer_work,real_work,
                          model_routine_handle,intg_routine_handle))
                        {
                          return_code = update_unemap_interface_values(
                            unemap_interface_values);
                          current_time = end_time;
                        }
                      } /* while (return_code &&
                           (current_time < cell_calculate->Tend)) */
                      if (return_code)
                      {
                        /* The model integrated all the way, so set-up the
                         * UnEmap signals?? */
                        /* Assemble an array of variable UnEmap interfaces to
                           use for the creation of the UnEmap signals */
                        number_of_signals = 0;
                        variable_unemap_interfaces =
                          (struct Cell_variable_unemap_interface **)NULL;
                        value = unemap_interface_values;
                        while (value && return_code)
                        {
                          number_of_signals++;
                          if (REALLOCATE(variable_unemap_interfaces,
                            variable_unemap_interfaces,
                            struct Cell_variable_unemap_interface *,
                            number_of_signals))
                          {
                            variable_unemap_interfaces[number_of_signals-1] =
                              value->variable_unemap_interface;
                            value = value->next;
                          }
                          else
                          {
                            display_message(ERROR_MESSAGE,"calculate_model.  "
                              "Unable to allocate memory for the variable "
                              "UnEmap interfaces");
                            return_code = 0;
                            if (variable_unemap_interfaces)
                            {
                              DEALLOCATE(variable_unemap_interfaces);
                              variable_unemap_interfaces =
                                (struct Cell_variable_unemap_interface **)NULL;
                            }
                          }
                        } /* while (value && return_code) */
                        /* Now create the UnEmap signals */
                        return_code = Cell_unemap_interface_add_signals(
                          cell_calculate->cell_unemap_interface,
                          number_of_signals,variable_unemap_interfaces,
                          cell_calculate->tabT);
                        if (return_code)
                        {
                          Cell_unemap_interface_update_analysis_work_area(
                            cell_calculate->cell_unemap_interface);
                        }
                        else
                        {
                          Cell_unemap_interface_clear_analysis_work_area(
                            cell_calculate->cell_unemap_interface);
                        }
                      }
                      else
                      {
                        display_message(ERROR_MESSAGE,"calculate_model.  "
                          "There was an error integrating the model (end time "
                          "= %f)",current_time);
                        return_code = 0;
                      }
                    }
                    else
                    {
                      display_message(ERROR_MESSAGE,"calculate_model.  "
                        "There was an error allocating the work arrays");
                      return_code = 0;
                      if (integer_work != (CELL_INTEGER *)NULL)
                      {
                        DEALLOCATE(integer_work);
                      }
                    }
                  }
                  else
                  {
                    display_message(ERROR_MESSAGE,"calculate_model.  "
                      "There was an error initialising the control arrays");
                    return_code = 0;
                  }
                }
                else
                {
                  display_message(ERROR_MESSAGE,"calculate_model.  "
                    "There was an error updating the UnEmap variable values");
                  return_code = 0;
                }
                DESTROY(Unemap_interface_value)(&unemap_interface_values);
              }
              else
              {
                display_message(WARNING_MESSAGE,"calculate_model.  "
                  "Unable to create the UnEmap interface arrays");
                return_code = 0;
              }
              DESTROY(Cell_cmiss_interface_arrays)(&cmiss_interface_arrays);
            }
            else
            {
              display_message(WARNING_MESSAGE,"calculate_model.  "
                "Unable to create the CMISS interface arrays");
              return_code = 0;
            }
          }
          if (dlclose(intg_dso_handle) != 0)
          {
            display_message(WARNING_MESSAGE,"calculate_model.  "
              "An error occured while closing the integrator DSO - "
              "dlerror: \"%s\"",dlerror());
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"calculate_model.  "
            "Unable to get a handle on the specified integrator DSO - "
            "dlerror: \"%s\"",dlerror());
          return_code = 0;
        }
      }
      if (dlclose(model_dso_handle) != 0)
      {
        display_message(WARNING_MESSAGE,"calculate_model.  "
          "An error occured while closing the DSO - dlerror: \"%s\"",
          dlerror());
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"calculate_model.  "
        "Unable to get a handle on the specified DSO - dlerror: \"%s\"",
        dlerror());
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"calculate_model.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* calculate_model() */

/*
Global functions
----------------
*/
struct Cell_calculate *CREATE(Cell_calculate)(
  struct Cell_unemap_interface *cell_unemap_interface)
/*******************************************************************************
LAST MODIFIED : 04 November 2000

DESCRIPTION :
Creates a Cell_calculate object.
==============================================================================*/
{
  struct Cell_calculate *cell_calculate;

  ENTER(CREATE(Cell_calculate));
  if (ALLOCATE(cell_calculate,struct Cell_calculate,1))
  {
    /* Initialise data objects */
    cell_calculate->model_routine_name = (char *)NULL;
    cell_calculate->dso_name = (char *)NULL;
    cell_calculate->intg_routine_name = (char *)NULL;
    cell_calculate->intg_dso_name = (char *)NULL;
    cell_calculate->Tstart = 0.0;
    cell_calculate->Tend = 0.0;
    cell_calculate->dT = 0.0;
    cell_calculate->tabT = 0.0;
    cell_calculate->cell_unemap_interface = cell_unemap_interface;
    cell_calculate->dialog = (struct Cell_calculate_dialog *)NULL;
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_calculate).  "
      "Unable to allocate memory for the Cell calculate object");
    cell_calculate = (struct Cell_calculate *)NULL;
  }
  LEAVE;
  return(cell_calculate);
} /* CREATE(Cell_calculate) */

int DESTROY(Cell_calculate)(struct Cell_calculate **cell_calculate_address)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Destroys a Cell_calculate object.
==============================================================================*/
{
	int return_code;
  struct Cell_calculate *cell_calculate = (struct Cell_calculate *)NULL;

	ENTER(DESTROY(Cell_calculate));
	if (cell_calculate_address && (cell_calculate = *cell_calculate_address))
	{
    if (cell_calculate->model_routine_name)
    {
      DEALLOCATE(cell_calculate->model_routine_name);
    }
    if (cell_calculate->dso_name)
    {
      DEALLOCATE(cell_calculate->dso_name);
    }
    if (cell_calculate->intg_routine_name)
    {
      DEALLOCATE(cell_calculate->intg_routine_name);
    }
    if (cell_calculate->intg_dso_name)
    {
      DEALLOCATE(cell_calculate->intg_dso_name);
    }
    if (cell_calculate->dialog)
    {
      DESTROY(Cell_calculate_dialog)(&(cell_calculate->dialog));
    }
    DEALLOCATE(*cell_calculate_address);
    *cell_calculate_address = (struct Cell_calculate *)NULL;
    return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_calculate).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* DESTROY(Cell_calculate) */

int Cell_calculate_set_model_routine_name(struct Cell_calculate *cell_calculate,
  char *name)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Sets the model routine name for the given <cell_calculate> object.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_calculate_set_model_routine_name);
  if (cell_calculate && name)
  {
    /* Free up any existing name */
    if (cell_calculate->model_routine_name)
    {
      DEALLOCATE(cell_calculate->model_routine_name);
      cell_calculate->model_routine_name = (char *)NULL;
    }
    /* grab a copy of the name string for the object */
    if (ALLOCATE(cell_calculate->model_routine_name,char,strlen(name)+1))
    {
      strcpy(cell_calculate->model_routine_name,name);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_calculate_set_model_routine_name.  "
        "Unable to allocate memory for the routine name string");
      cell_calculate->model_routine_name = (char *)NULL;
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_set_model_routine_name.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_calculate_set_model_routine_name() */

char *Cell_calculate_get_model_routine_name(
  struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 18 December 2000

DESCRIPTION :
Gets the model routine name for the given <cell_calculate> object.
==============================================================================*/
{
  char *name;

  ENTER(Cell_calculate_get_model_routine_name);
  if (cell_calculate)
  {
    if (cell_calculate->model_routine_name)
    {
      /* grab a copy of the name string for the object */
      if (ALLOCATE(name,char,strlen(cell_calculate->model_routine_name)+1))
      {
        strcpy(name,cell_calculate->model_routine_name);
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_calculate_get_model_routine_name.  "
          "Unable to allocate memory for the routine name string");
        name = (char *)NULL;
      }
    }
    else
    {
      display_message(WARNING_MESSAGE,"Cell_calculate_get_model_routine_name.  "
        "No routine name string");
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_get_model_routine_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_calculate_get_model_routine_name() */

int Cell_calculate_set_dso_name(struct Cell_calculate *cell_calculate,
  char *name)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Sets the DSO name for the given <cell_calculate> object. <name> can be NULL,
which simply indictes that the model routine is compiled into the executable.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_calculate_set_dso_name);
  if (cell_calculate)
  {
    /* Free up any existing name */
    if (cell_calculate->dso_name)
    {
      DEALLOCATE(cell_calculate->dso_name);
      cell_calculate->dso_name = (char *)NULL;
    }
    if (name && (strlen(name) > 0))
    {
      /* grab a copy of the name string for the object */
      if (ALLOCATE(cell_calculate->dso_name,char,strlen(name)+1))
      {
        strcpy(cell_calculate->dso_name,name);
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_calculate_set_dso_name.  "
          "Unable to allocate memory for the DSO name string");
        cell_calculate->dso_name = (char *)NULL;
        return_code = 0;
      }
    }
    else
    {
      /* don't need a name */
      cell_calculate->dso_name = (char *)NULL;
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_set_dso_name.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_calculate_set_dso_name() */

char *Cell_calculate_get_dso_name(struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 18 December 2000

DESCRIPTION :
Gets the DSO name for the given <cell_calculate> object.
==============================================================================*/
{
  char *name;

  ENTER(Cell_calculate_get_dso_name);
  if (cell_calculate)
  {
    /* Don't have to have a DSO name */
    if (cell_calculate->dso_name)
    {
      /* grab a copy of the name string for the object */
      if (ALLOCATE(name,char,strlen(cell_calculate->dso_name)+1))
      {
        strcpy(name,cell_calculate->dso_name);
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_calculate_get_dso_name.  "
          "Unable to allocate memory for the routine name string");
        name = (char *)NULL;
      }
    }
    else
    {
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_get_dso_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_calculate_get_dso_name() */

int Cell_calculate_set_intg_routine_name(struct Cell_calculate *cell_calculate,
  char *name)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Sets the integrator routine name for the given <cell_calculate> object.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_calculate_set_intg_routine_name);
  if (cell_calculate && name)
  {
    /* Free up any existing name */
    if (cell_calculate->intg_routine_name)
    {
      DEALLOCATE(cell_calculate->intg_routine_name);
      cell_calculate->intg_routine_name = (char *)NULL;
    }
    /* grab a copy of the name string for the object */
    if (ALLOCATE(cell_calculate->intg_routine_name,char,strlen(name)+1))
    {
      strcpy(cell_calculate->intg_routine_name,name);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_calculate_set_intg_routine_name.  "
        "Unable to allocate memory for the routine name string");
      cell_calculate->intg_routine_name = (char *)NULL;
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_set_intg_routine_name.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_calculate_set_intg_routine_name() */

char *Cell_calculate_get_intg_routine_name(
  struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Gets the integrator routine name for the given <cell_calculate> object.
==============================================================================*/
{
  char *name;

  ENTER(Cell_calculate_get_intg_routine_name);
  if (cell_calculate)
  {
    if (cell_calculate->intg_routine_name)
    {
      /* grab a copy of the name string for the object */
      if (ALLOCATE(name,char,strlen(cell_calculate->intg_routine_name)+1))
      {
        strcpy(name,cell_calculate->intg_routine_name);
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_calculate_get_intg_routine_name.  "
          "Unable to allocate memory for the routine name string");
        name = (char *)NULL;
      }
    }
    else
    {
      display_message(WARNING_MESSAGE,"Cell_calculate_get_intg_routine_name.  "
        "Missing routine name string");
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_get_intg_routine_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_calculate_get_intg_routine_name() */

int Cell_calculate_set_intg_dso_name(struct Cell_calculate *cell_calculate,
  char *name)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Sets the integrator DSO name for the given <cell_calculate> object. <name> can
be NULL, which simply indictes that the model routine is compiled into the
executable.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_calculate_set_intg_dso_name);
  if (cell_calculate)
  {
    /* Free up any existing name */
    if (cell_calculate->intg_dso_name)
    {
      DEALLOCATE(cell_calculate->intg_dso_name);
      cell_calculate->intg_dso_name = (char *)NULL;
    }
    if (name && (strlen(name) > 0))
    {
      /* grab a copy of the name string for the object */
      if (ALLOCATE(cell_calculate->intg_dso_name,char,strlen(name)+1))
      {
        strcpy(cell_calculate->intg_dso_name,name);
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_calculate_set_intg_dso_name.  "
          "Unable to allocate memory for the DSO name string");
        cell_calculate->intg_dso_name = (char *)NULL;
        return_code = 0;
      }
    }
    else
    {
      /* don't need a name */
      cell_calculate->intg_dso_name = (char *)NULL;
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_set_intg_dso_name.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_calculate_set_intg_dso_name() */

char *Cell_calculate_get_intg_dso_name(struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Gets the integrator DSO name for the given <cell_calculate> object.
==============================================================================*/
{
  char *name;

  ENTER(Cell_calculate_get_intg_dso_name);
  if (cell_calculate)
  {
    /* Don't have to have a DSO name */
    if (cell_calculate->intg_dso_name)
    {
      /* grab a copy of the name string for the object */
      if (ALLOCATE(name,char,strlen(cell_calculate->intg_dso_name)+1))
      {
        strcpy(name,cell_calculate->intg_dso_name);
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_calculate_get_intg_dso_name.  "
          "Unable to allocate memory for the DSO name string");
        name = (char *)NULL;
      }
    }
    else
    {
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_get_intg_dso_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_calculate_get_intg_dso_name() */

int Cell_calculate_calculate_model(struct Cell_calculate *cell_calculate,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Calculates the current model
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_calculate_calculate_model);
  if (cell_calculate && variable_list)
  {
    /* check for required information */
    if (cell_calculate->model_routine_name)
    {
      /* Solve the model */
      return_code = calculate_model(cell_calculate,variable_list);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_calculate_calculate_model.  "
        "Need to specify a model routine name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_calculate_model.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_calculate_calculate_model() */

int Cell_calculate_pop_up_dialog(
  struct Cell_calculate *cell_calculate,Widget parent,
  struct Cell_interface *cell_interface,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Brings up the calculate dialog - creating it if required.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_calculate_pop_up_dialog);
  if (cell_calculate)
  {
    if (cell_calculate->dialog == (struct Cell_calculate_dialog *)NULL)
    {
      cell_calculate->dialog = CREATE(Cell_calculate_dialog)(cell_calculate,
        parent,cell_interface,user_interface);
    }
    if (cell_calculate->dialog)
    {
      return_code = Cell_calculate_dialog_pop_up(cell_calculate->dialog);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_calculate_pop_up_dialog.  "
        "Missing calculate dialog");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_pop_up_calculate_dialog.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_calculate_pop_up_dialog() */

int Cell_calculate_pop_down_dialog(struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the calculation dialog.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_calculate_pop_down_dialog);
  if (cell_calculate)
  {
    if (cell_calculate->dialog)
    {
      return_code = Cell_calculate_dialog_pop_down(cell_calculate->dialog);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_calculate_pop_down_dialog.  "
        "Missing calculate dialog");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_pop_down_dialog.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_calculate_pop_down_dialog() */

char *Cell_calculate_get_start_time_as_string(
  struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Returns the current integration start time as a string.
==============================================================================*/
{
  char *value_string,temp_string[512];

  ENTER(Cell_calculate_get_start_time_as_string);
  if (cell_calculate)
  {
    sprintf(temp_string,"%g",cell_calculate->Tstart);
    if (ALLOCATE(value_string,char,strlen(temp_string)+1))
    {
      strcpy(value_string,temp_string);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_calculate_get_start_time_as_string,  "
        "Unable to allocate memory for the value string");
      value_string = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_get_start_time_as_string,  "
      "Invalid argument(s)");
    value_string = (char *)NULL;
  }
  LEAVE;
  return(value_string);
} /* Cell_calculate_get_start_time_as_string() */

char *Cell_calculate_get_end_time_as_string(
  struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Returns the current integration end time as a string.
==============================================================================*/
{
  char *value_string,temp_string[512];

  ENTER(Cell_calculate_get_end_time_as_string);
  if (cell_calculate)
  {
    sprintf(temp_string,"%g",cell_calculate->Tend);
    if (ALLOCATE(value_string,char,strlen(temp_string)+1))
    {
      strcpy(value_string,temp_string);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_calculate_get_end_time_as_string,  "
        "Unable to allocate memory for the value string");
      value_string = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_get_end_time_as_string,  "
      "Invalid argument(s)");
    value_string = (char *)NULL;
  }
  LEAVE;
  return(value_string);
} /* Cell_calculate_get_end_time_as_string() */

char *Cell_calculate_get_dt_as_string(struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Returns the current integration dT as a string.
==============================================================================*/
{
  char *value_string,temp_string[512];

  ENTER(Cell_calculate_get_dt_as_string);
  if (cell_calculate)
  {
    sprintf(temp_string,"%g",cell_calculate->dT);
    if (ALLOCATE(value_string,char,strlen(temp_string)+1))
    {
      strcpy(value_string,temp_string);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_calculate_get_dt_as_string,  "
        "Unable to allocate memory for the value string");
      value_string = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_get_dt_as_string,  "
      "Invalid argument(s)");
    value_string = (char *)NULL;
  }
  LEAVE;
  return(value_string);
} /* Cell_calculate_get_dt_as_string() */

char *Cell_calculate_get_tabt_as_string(struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Returns the current integration tabT as a string.
==============================================================================*/
{
  char *value_string,temp_string[512];

  ENTER(Cell_calculate_get_tabt_as_string);
  if (cell_calculate)
  {
    sprintf(temp_string,"%g",cell_calculate->tabT);
    if (ALLOCATE(value_string,char,strlen(temp_string)+1))
    {
      strcpy(value_string,temp_string);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_calculate_get_tabt_as_string,  "
        "Unable to allocate memory for the value string");
      value_string = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_get_tabt_as_string,  "
      "Invalid argument(s)");
    value_string = (char *)NULL;
  }
  LEAVE;
  return(value_string);
} /* Cell_calculate_get_tabt_as_string() */

char *Cell_calculate_set_start_time_from_string(
  struct Cell_calculate *cell_calculate,char *value_string)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Checks the <value_string> for a valid value for the integration start time and
if one is found, returns the value in the correct format string. Returns NULL
if a valid value is not found.
==============================================================================*/
{
  char *new_value_string,temp_string[512];
  float value;

  ENTER(Cell_calculate_set_start_time_from_string);
  if (cell_calculate && value_string)
  {
    /* check the value string for a valid value */
    if (sscanf(value_string,"%g",&value))
    {
      cell_calculate->Tstart = value;
      sprintf(temp_string,"%g",cell_calculate->Tstart);
      if (ALLOCATE(new_value_string,char,strlen(temp_string)+1))
      {
        strcpy(new_value_string,temp_string);
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "Cell_calculate_set_start_time_from_string,  "
          "Unable to allocate memory for the value string");
        new_value_string = (char *)NULL;
      }
    }
    else
    {
      /* The value string contained an invalid number */
      new_value_string = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_set_start_time_from_string,  "
      "Invalid argument(s)");
    new_value_string = (char *)NULL;
  }
  LEAVE;
  return(new_value_string);
} /* Cell_calculate_set_start_time_from_string() */

char *Cell_calculate_set_end_time_from_string(
  struct Cell_calculate *cell_calculate,char *value_string)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Checks the <value_string> for a valid value for the integration end time and
if one is found, returns the value in the correct format string. Returns NULL
if a valid value is not found.
==============================================================================*/
{
  char *new_value_string,temp_string[512];
  float value;

  ENTER(Cell_calculate_set_end_time_from_string);
  if (cell_calculate && value_string)
  {
    /* check the value string for a valid value */
    if (sscanf(value_string,"%g",&value))
    {
      cell_calculate->Tend = value;
      sprintf(temp_string,"%g",cell_calculate->Tend);
      if (ALLOCATE(new_value_string,char,strlen(temp_string)+1))
      {
        strcpy(new_value_string,temp_string);
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "Cell_calculate_set_end_time_from_string,  "
          "Unable to allocate memory for the value string");
        new_value_string = (char *)NULL;
      }
    }
    else
    {
      /* The value string contained an invalid number */
      new_value_string = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_set_end_time_from_string,  "
      "Invalid argument(s)");
    new_value_string = (char *)NULL;
  }
  LEAVE;
  return(new_value_string);
} /* Cell_calculate_set_end_time_from_string() */

char *Cell_calculate_set_dt_from_string(
  struct Cell_calculate *cell_calculate,char *value_string)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Checks the <value_string> for a valid value for the integration dt and
if one is found, returns the value in the correct format string. Returns NULL
if a valid value is not found.
==============================================================================*/
{
  char *new_value_string,temp_string[512];
  float value;

  ENTER(Cell_calculate_set_dt_from_string);
  if (cell_calculate && value_string)
  {
    /* check the value string for a valid value */
    if (sscanf(value_string,"%g",&value))
    {
      cell_calculate->dT = value;
      sprintf(temp_string,"%g",cell_calculate->dT);
      if (ALLOCATE(new_value_string,char,strlen(temp_string)+1))
      {
        strcpy(new_value_string,temp_string);
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "Cell_calculate_set_dt_from_string,  "
          "Unable to allocate memory for the value string");
        new_value_string = (char *)NULL;
      }
    }
    else
    {
      /* The value string contained an invalid number */
      new_value_string = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_set_dt_from_string,  "
      "Invalid argument(s)");
    new_value_string = (char *)NULL;
  }
  LEAVE;
  return(new_value_string);
} /* Cell_calculate_set_dt_from_string() */

char *Cell_calculate_set_tabt_from_string(
  struct Cell_calculate *cell_calculate,char *value_string)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Checks the <value_string> for a valid value for the integration tabt and
if one is found, returns the value in the correct format string. Returns NULL
if a valid value is not found.
==============================================================================*/
{
  char *new_value_string,temp_string[512];
  float value;

  ENTER(Cell_calculate_set_tabt_from_string);
  if (cell_calculate && value_string)
  {
    /* check the value string for a valid value */
    if (sscanf(value_string,"%g",&value))
    {
      cell_calculate->tabT = value;
      sprintf(temp_string,"%g",cell_calculate->tabT);
      if (ALLOCATE(new_value_string,char,strlen(temp_string)+1))
      {
        strcpy(new_value_string,temp_string);
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "Cell_calculate_set_tabt_from_string,  "
          "Unable to allocate memory for the value string");
        new_value_string = (char *)NULL;
      }
    }
    else
    {
      /* The value string contained an invalid number */
      new_value_string = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_calculate_set_tabt_from_string,  "
      "Invalid argument(s)");
    new_value_string = (char *)NULL;
  }
  LEAVE;
  return(new_value_string);
} /* Cell_calculate_set_tabt_from_string() */

