/*******************************************************************************
FILE : calculate.c

LAST MODIFIED : 17 November 1999

DESCRIPTION :
Functions for calculating and displaying the model solutions.
==============================================================================*/
#include <stdio.h>
#include "curve/control_curve.h"
#include "general/debug.h"
#include "user_interface/message.h"
#include "cell/calculate.h"
#include "cell/cell_parameter.h"
#include "cell/cell_variable.h"
#include "unemap/analysis_work_area.h"
#include "unemap/analysis_window.h"
#include "unemap/mapping.h"

/*
Local types
===========
*/
#if defined (MOTIF)
typedef struct
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Unemap user settings.
==============================================================================*/
{
  Pixel analysis_colour;
} User_settings;
#endif /* defined (MOTIF) */

/*
Local functions
===============
*/
static void associate_mapping_analysis(Widget widget,XtPointer system_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 01 March 1999

DESCRIPTION :
need this for analysis window focus callback (analysis_window.uil)
??? CELL doesn't do any mapping ???
==============================================================================*/
{
  ENTER(associate_mapping_analysis);
  USE_PARAMETER(widget);
  USE_PARAMETER(system_window);
  USE_PARAMETER(call_data);
  LEAVE;
} /* END associate_mapping_analysis */

static float *get_control_curve_values(struct Control_curve *control_curve,
  float tstart,float tend,float dt,int *number_of_values)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Returns the values from the time variable <control_curve>.
Uses <tstart>, <tend>, and <dt> to calulate the number of values required.
Ignores the values of the starting and finishing times of the time variable and
assumes the range is to be scaled to fit the range <tstart> to <tend>.
==============================================================================*/
{
	FE_value *values;
  int i,num_steps;
  float *results,scaled_step,tv_start,tv_end;
  
  ENTER(get_control_curve_values);
  if ((control_curve != (struct Control_curve *)NULL) && (tend > tstart) &&
    (dt > 0))
  {
    num_steps = 1+(int)((tend - tstart) / dt);
    *number_of_values = num_steps;
    if (Control_curve_get_parameter_range(control_curve,&tv_start,&tv_end))
    {
      scaled_step = (tv_end - tv_start) / (float)num_steps;
      if (ALLOCATE(results,float,num_steps))
      {
				if (ALLOCATE(values,FE_value,
					Control_curve_get_number_of_components(control_curve)))
				{
					for (i=0;i<=num_steps;i++)
					{
						/* get the values (assume the extension ratio is always the first
							 component of the time variable) */
						Control_curve_get_values_at_parameter(control_curve,
							tv_start+((float)i)*scaled_step,values,(FE_value *)NULL);
						results[i]=(float)(values[0]);
					}
					DEALLOCATE(values);
				}
				else
				{
					results = (float *)NULL;
					display_message(ERROR_MESSAGE,"get_control_curve_values. Unable to "
						"allocate values array");
				}
      }
      else
      {
        results = (float *)NULL;
        display_message(ERROR_MESSAGE,"get_control_curve_values. Unable to "
          "allocate result array");
      }
    }
    else
    {
      results = (float *)NULL;
      display_message(ERROR_MESSAGE,"get_control_curve_values. "
        "Unable to get the starting or finishing time from the time variable");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"get_control_curve_values. "
      "Invalid arguments");
    results = (float *)NULL;
  }
  LEAVE;
  return (results);
}/* END get_control_curve_values() */

#if 0
#if defined (IBM)
extern void hodgkin_huxley
#endif
#if defined (SGI)
extern void hodgkin_huxley_
#endif
#if defined (GENERIC_PC)
extern void hodgkin_huxley__
#endif
  (double *RPARAM,int *NT_CURR,int *NT_DAT,int *NT_EQN,float *Y_FULL);
#endif /* 0 */
#if defined (IBM)
extern void hodgkin_huxley(
#endif
#if defined (SGI)
extern void hodgkin_huxley_(
#endif
#if defined (GENERIC_PC)
extern void hodgkin_huxley__(
#endif
  int *AII,int *AIO,int *CONTROL,int *ERROR_TYPE,int *IFAIL,int *IWORK,
  int *L_AII,int *L_AIO,int *L_ARI,int *L_ARO,int *L_CONTROL,int *L_IWORK,
  int *L_MODEL,int *L_PARAMETERS,int *L_WORK,int *MAX_ITERS,int *MAX_ORDER,
  int *MODEL,double *ABS_ERR,double *ARI,double *ARO,double *MAX_STEP,
  double *PARAMETERS,double *REL_ERR,double *WORK,float *Y_FULL,
  int *EXTEND_INTERVAL,int *STIFF_EQNS,int *USE_ROUND_CTRL,char *ERROR);

#if defined (IBM)
extern void luo_rudy(
#endif
#if defined (SGI)
extern void luo_rudy_(
#endif
#if defined (GENERIC_PC)
extern void luo_rudy__(
#endif
  int *AII,int *AIO,int *CONTROL,int *ERROR_TYPE,int *IFAIL,int *IWORK,
  int *L_AII,int *L_AIO,int *L_ARI,int *L_ARO,int *L_CONTROL,int *L_IWORK,
  int *L_MODEL,int *L_PARAMETERS,int *L_WORK,int *MAX_ITERS,int *MAX_ORDER,
  int *MODEL,double *ABS_ERR,double *ARI,double *ARO,double *MAX_STEP,
  double *PARAMETERS,double *REL_ERR,double *WORK,float *Y_FULL,
  int *EXTEND_INTERVAL,int *STIFF_EQNS,int *USE_ROUND_CTRL,char *ERROR);

#if defined (IBM)
extern void coupled_system(
#endif
#if defined (SGI)
extern void coupled_system_(
#endif
#if defined (GENERIC_PC)
extern void coupled_system__(
#endif
  int *MECH_IPARAM,int *MEM_IPARAM1,int*MEM_IPARAM2,int *MEM_LPARAM,
  double *MECH_RPARAM1,double *MECH_RPARAM2,double *MEM_RPARAM1,
  double *MEM_RPARAM2,double *MEM_RPARAM3,int *MODEL_DEFINITION,int *NT_DAT,
  float *Y_FULL,int *CURRENT_SWITCHES,float *EXTENSION_RATIO_VALUES);

static int calculate_cell_model(struct Cell_parameter *parameters,
  int number_of_outputs,int number_of_samples,float *signal_values,
  int number_of_equations,int number_of_currents,int number_of_rates,
  int number_of_mechanics,char *model,struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Calculates the cell model.
==============================================================================*/
{
  int return_code = 0;
  int i;
  double *rparam = (double *)NULL;
  int number_of_parameters;
  struct Cell_parameter *current = (struct Cell_parameter *)NULL;
  int *AII,*AIO,*CONTROL,ERROR_TYPE,IFAIL,*IWORK,L_AII,L_AIO,L_ARI,L_ARO,
    L_CONTROL,L_IWORK,L_MODEL,L_WORK,MAX_ITERS,MAX_ORDER,*MODEL;
  double ABS_ERR,*ARI,*ARO,MAX_STEP,REL_ERR,*WORK;
  int EXTEND_INTERVAL,STIFF_EQNS,USE_ROUND_CTRL;
  char ERROR[500];
  int model_id,ari_start_position,number_of_values;
  float *control_curve_values;

  /* tmp for HMT */
  double rparam_tmp[9][21],rparam_2[9],rparam_lr[70],rparam_mech[52],
    rparam_mech2[61];
  int iparam_1[9],iparam_2[13],lparam_2[5];
  int iparam_mech[3],model_defn[5],current_switches[30];

  ENTER(calculate_cell_model);
  USE_PARAMETER(number_of_outputs);
  if ((parameters != (struct Cell_parameter *)NULL) &&
    (signal_values != (float *)NULL))
  {
    if (!strncmp(model,"Hodg",2))
    {
      model_id = 1;
    }
    else if (!strncmp(model,"Luo",2))
    {
      model_id = 2;
    }
    else if (!strncmp(model,"Hunter",5))
    {
      model_id = 3;
    }
    else
    {
      model_id = -1;
    }
    if ((model_id > 0) && (model_id != 3))
    {
      /* initialise stuff */
      L_AII = 0;
      L_AIO = 0;
      L_ARI = 0;
      L_ARO = 0;
      L_CONTROL = 0;
      CONTROL = (int *)NULL;
      L_IWORK = 0;
      L_MODEL = 0;
      L_WORK = 0;
      ERROR_TYPE = 1;
      MAX_ITERS = 1000;
      MAX_ORDER = 10;
      ABS_ERR = 1.0e-6;
      MAX_STEP = 1.0;
      REL_ERR = ABS_ERR;
      EXTEND_INTERVAL = 1;
      STIFF_EQNS = 0;
      USE_ROUND_CTRL = 0;
      /* initialise work arrays */
      L_IWORK = 7;
      L_WORK = 7+7*MAX_ORDER+number_of_equations*(MAX_ORDER+8);
      if (ALLOCATE(IWORK,int,L_IWORK) && ALLOCATE(WORK,double,L_WORK))
      {
        L_AII = 3;
        L_AIO = 2; /* used to store the number of stimulii applied and the
                   current position in the stimulus time variable array*/
        L_ARI = 3; /* used internally in FE19 */
        ari_start_position = 4;
        L_ARO = number_of_currents+number_of_rates;
        if (L_ARO < 1)
        {
          L_ARO = 1;
        }
        if (ALLOCATE(AII,int,L_AII) && ALLOCATE(AIO,int,L_AIO) &&
          ALLOCATE(ARI,double,L_ARI) && ALLOCATE(ARO,double,L_ARO))
        {
          AIO[0] = ari_start_position; /* for first position in t.v. array */
          AII[0] = number_of_samples;
          AII[1] = number_of_equations;
          AII[2] = number_of_currents+number_of_rates+number_of_mechanics;
          L_MODEL = 1;
          if (ALLOCATE(MODEL,int,L_MODEL))
          {
            /* place parameters into parameter array */
            current = parameters;
            L_CONTROL = 0;
            CONTROL = (int *)NULL;
            number_of_parameters = 0;
            return_code = 1;
            while ((current != (struct Cell_parameter *)NULL) && return_code)
            {
              number_of_parameters++;
              if (rparam == (double *)NULL)
              {
                if (!ALLOCATE(rparam,double,1))
                {
                  rparam = (double *)NULL;
                }
              }
              else
              {
                if (!REALLOCATE(rparam,rparam,double,number_of_parameters))
                {
                  rparam = (double *)NULL;
                }
              }
              if (rparam != (double *)NULL)
              {
                rparam[number_of_parameters-1] = (double)(current->value);
                if (current->control_curve_switch)
                {
                  /* this parameter is using a time variable to set its value */
                  L_CONTROL += 2;
                  if (CONTROL == (int *)NULL)
                  {
                    if (!ALLOCATE(CONTROL,int,L_CONTROL))
                    {
                      CONTROL = (int *)NULL;
                    }
                  }
                  else
                  {
                    if (!REALLOCATE(CONTROL,CONTROL,int,L_CONTROL))
                    {
                      CONTROL = (int *)NULL;
                    }
                  }
                  if (CONTROL != (int *)NULL)
                  {
                    CONTROL[L_CONTROL-2] = number_of_parameters;
                    CONTROL[L_CONTROL-1] = ari_start_position;
                    control_curve_values = get_control_curve_values(
                      current->control_curve[0],
                      *((cell->time_steps).TSTART),
                      *((cell->time_steps).TEND),
                      *((cell->time_steps).TABT),&number_of_values);
                    if (control_curve_values != (float *)NULL)
                    {
                      L_ARI += number_of_values;
                      if (REALLOCATE(ARI,ARI,double,L_ARI))
                      {
                        for (i=0;i<number_of_values;i++)
                        {
                          ARI[ari_start_position-1] = control_curve_values[i];
                          ari_start_position++;
                        }
                      }
                      else
                      {
                        display_message(ERROR_MESSAGE,"calculate_cell_model. "
                          "Unable to reallocate memory for ARI");
                        return_code = 0;
                      }
                      DEALLOCATE(control_curve_values);
                    }
                    else
                    {
                      display_message(ERROR_MESSAGE,"calculate_cell_model. "
                        "Unable to get the time variable values");
                      return_code = 0;
                    }
                  }
                  else
                  {
                    display_message(ERROR_MESSAGE,"calculate_cell_model. "
                      "Unable to allocate memory for the control array");
                    return_code = 0;
                  }
                } /* if (control_curve_switch) */
              } /* if (rparam...) */
              else
              {
                display_message(ERROR_MESSAGE,"calculate_cell_model. "
                  "Unable to allocate memory for the rparam array");
                return_code = 0;
              }
              current = current->next;
            } /* while (current...) */
            if (L_CONTROL == 0)
            {
              L_CONTROL = 1;
              if (ALLOCATE(CONTROL,int,L_CONTROL))
              {
                CONTROL[0] = -1;
              }
              else
              {
                display_message(ERROR_MESSAGE,"calculate_cell_model. "
                  "Unable to allocate memory for the control array (no)");
                return_code = 0;
              }
            }
            if (return_code)
            {
              if (model_id == 1)
              {
#if defined (IBM)
                hodgkin_huxley
#endif
#if defined (SGI)
                  hodgkin_huxley_
#endif
#if defined (GENERIC_PC)
                  hodgkin_huxley__
#endif
                  (AII,AIO,CONTROL,&ERROR_TYPE,&IFAIL,IWORK,&L_AII,&L_AIO,&L_ARI,
                    &L_ARO,&L_CONTROL,&L_IWORK,&L_MODEL,&number_of_parameters,
                    &L_WORK,&MAX_ITERS,&MAX_ORDER,MODEL,&ABS_ERR,ARI,ARO,
                    &MAX_STEP,rparam,&REL_ERR,WORK,signal_values,
                    &EXTEND_INTERVAL,&STIFF_EQNS,&USE_ROUND_CTRL,ERROR);
              } /* if (model_id == 1) */
              else if (model_id == 2)
              {
#if defined (IBM)
                luo_rudy
#endif
#if defined (SGI)
                  luo_rudy_
#endif
#if defined (GENERIC_PC)
                  luo_rudy__
#endif
                  (AII,AIO,CONTROL,&ERROR_TYPE,&IFAIL,IWORK,&L_AII,&L_AIO,&L_ARI,
                    &L_ARO,&L_CONTROL,&L_IWORK,&L_MODEL,&number_of_parameters,
                    &L_WORK,&MAX_ITERS,&MAX_ORDER,MODEL,&ABS_ERR,ARI,ARO,
                    &MAX_STEP,rparam,&REL_ERR,WORK,signal_values,&EXTEND_INTERVAL,
                    &STIFF_EQNS,&USE_ROUND_CTRL,ERROR);
              } /* if (model_id == 2) */
              return_code = 1;
            } /* if (return_code) */
          }
          else
          {
            display_message(ERROR_MESSAGE,"calculate_cell_model. "
              "Unable to allocate memory for the model array");
            return_code = 0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"calculate_cell_model. "
            "Unable to allocate memory for the additional arrays");
          return_code = 0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"calculate_cell_model. "
          "Unable to allocate memory for the work arrays");
        return_code = 0;
      }
    }
    else if (model_id == 3)
    {
      /* solve hmt model */
      /* only works if hmt.model in right order !! */
      /* assign parameters */
      if (cell->variables->control_curve != (struct Control_curve *)NULL)
      {
        control_curve_values = get_control_curve_values(
          cell->variables->control_curve,
          *((cell->time_steps).TSTART),
          *((cell->time_steps).TEND),
          *((cell->time_steps).TABT),&number_of_values);
        rparam_mech[1] = control_curve_values[0];
        current = cell->parameters;
        i = 2;
        while (current != (struct Cell_parameter *)NULL)
        {
          rparam_mech[i] = (double)(current->value);
          i++;
          current = current->next;
        } /* while (current) */
        iparam_mech[0] = 99;
        model_defn[1] = 0;
        model_defn[0] = 4;
        model_defn[3] = 0;
        model_defn[4] = 0;
#if defined (IBM)
        coupled_system
#endif
#if defined (SGI)
        coupled_system_
#endif
#if defined (GENERIC_PC)
        coupled_system__
#endif
          (iparam_mech,iparam_1,iparam_2,lparam_2,rparam_mech,rparam_mech2,
            (double *)rparam_tmp,rparam_2,rparam_lr,model_defn,
            &number_of_samples,signal_values,current_switches,
            control_curve_values);
      }
      else
      {
        display_message(ERROR_MESSAGE,"calculate_cell_model. "
          "Need to define the extension ratio time variable");
      }
    } /* if (model_id == 3) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"calculate_cell_model. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END calculate_cell_model() */

static void update_outputs_from_variables(struct Cell_variable *variables,
  struct Cell_output *outputs)
/*******************************************************************************
LAST MODIFIED : 03 March 1999

DESCRIPTION :
Updates the value field of the <outputs> from the <variables>.
==============================================================================*/
{
  struct Cell_variable *current_variable = (struct Cell_variable *)NULL;
  struct Cell_output *current_output = (struct Cell_output *)NULL;
  
  ENTER(update_outputs_from_variables);
  if ((variables != (struct Cell_variable *)NULL) &&
    (outputs != (struct Cell_output *)NULL))
  {
    /* for each variable, update the value of the output */
    current_variable = variables;
    current_output = outputs;
    while ((current_variable != (struct Cell_variable *)NULL) &&
      (current_output != (struct Cell_output *)NULL))
    {
      current_output->value = current_variable->value;
      current_variable = current_variable->next;
      current_output = current_output->next;
    }
  }
  else
  {
    display_message(WARNING_MESSAGE,"update_outputs_from_variables. "
      "No variables or outputs");
  }
  LEAVE;
} /* END update_outputs_from_variables() */

/*
Global functions
================
*/
void calculate_cell_window (struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Calculates the cell window.
==============================================================================*/
{
  Boolean save_signals,variable_calculate,currents;
  char device_name[10];
  float frequency;
  int device_number,existing_device_number,existing_variable_number,i,
    number_of_devices,number_of_samples,number_of_saved_buffers,
    number_of_signals,signal_number,*time,variable_number;
  int number_of_equations,number_of_currents,number_of_outputs,number_of_rates,
    number_of_mechanics;
  struct Analysis_work_area *analysis;
  struct Channel *channel;
  struct Device **device,**devices,**existing_device;
  struct Device_description *description;
  struct Map *map;
  struct Region *region;
  struct Region_list_item *region_list;
  struct Rig *calculate_rig;
  struct Signal *signal;
  struct Signal_buffer *buffer,**saved_buffers;
  static MrmRegisterArg callback_list[]={
    {"associate_mapping_analysis",(XtPointer)associate_mapping_analysis}};
  static MrmRegisterArg identifier_list[]=
  {
    {"system_window_structure",(XtPointer)NULL}
  };
  struct Cell_output *current_output = (struct Cell_output *)NULL;
  struct Cell_output *current_output_inner = (struct Cell_output *)NULL;
#if defined (MOTIF)
#define XmNanalysisColour "analysisColour"
#define XmCAnalysisColour "AnalysisColour"
  static XtResource resources[] = {
    {
      XmNanalysisColour,
			XmCAnalysisColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,analysis_colour),
			XmRString,
			"aquamarine"
    }
  }; /* resources */
  User_settings *user_settings = (User_settings *)NULL;
#endif /* defined (MOTIF) */

  ENTER(calculate_cell_window);
  if (cell != (struct Cell_window *)NULL)
  {
    /*debug = (cell->menu).debug;*/
    save_signals = (cell->menu).save;
    /* update the outputs */
    update_outputs_from_variables(cell->variables,cell->outputs);
    /* count the number of outputs */
    number_of_equations = 0;
    number_of_currents = 0;
    number_of_rates = 0;
    number_of_mechanics = 0;
    number_of_outputs = 0;
    current_output = cell->outputs;
    while (current_output != (struct Cell_output *)NULL)
    {
      number_of_outputs++;
      switch (current_output->type)
      {
        case VARIABLE:
        {
          number_of_equations++;
        } break;
        case CURRENT:
        {
          number_of_currents++;
        } break;
        case RATE:
        {
          number_of_rates++;
        } break;
        case MECHANICS:
        {
          number_of_mechanics++;
        } break;
      } /* switch (type) */
      current_output = current_output->next;
    }
    if (cell->current_model != (char *)NULL)
    {
      analysis = &((cell->unemap).analysis);
      /* clear previous calculation if required */
      number_of_saved_buffers=(cell->unemap).number_of_saved_buffers;
      saved_buffers=(cell->unemap).saved_buffers;
      if (True==save_signals)
      {
        number_of_saved_buffers++;
        REALLOCATE(saved_buffers,saved_buffers,struct Signal_buffer *,
          number_of_saved_buffers);
      }
      else
      {
        if (0<number_of_saved_buffers)
        {
          buffer=saved_buffers[number_of_saved_buffers-1];
          existing_device=analysis->rig->devices;
          for (i=analysis->rig->number_of_devices;i>0;i--)
          {
            if ((signal=(*existing_device)->signal)&&(buffer==signal->buffer))
            {
              if (signal->next)
              {
                (*existing_device)->signal=signal->next;
                destroy_Signal(&signal);
              }
              else
              {
                destroy_Device(existing_device);
              }
            }
            existing_device++;
          }
          destroy_Signal_buffer(&buffer);
        }
        else
        {
          number_of_saved_buffers=1;
          REALLOCATE(saved_buffers,saved_buffers,struct Signal_buffer *,
            number_of_saved_buffers);
        }
      }
      if (saved_buffers)
      {
        (cell->unemap).number_of_saved_buffers=number_of_saved_buffers;
        (cell->unemap).saved_buffers=saved_buffers;
        /* calculate solution */
        /* count the number of devices and the number of signals for the
        solution */
        number_of_devices=0;
        number_of_signals=0;
        signal_number=0;
        if (analysis->rig)
        {
          existing_device=analysis->rig->devices;
          existing_device_number=analysis->rig->number_of_devices;
          while ((existing_device_number>0)&&!(*existing_device))
          {
            existing_device_number--;
            existing_device++;
          }
          if (0<existing_device_number)
          {
            existing_variable_number = 0;
            current_output = cell->outputs;
            while ((current_output != (struct Cell_output *)NULL) &&
              (strcmp((*existing_device)->description->name,
                current_output->name) != 0))
            {
              existing_variable_number++;
              current_output = current_output->next;
            }
          }
          else
          {
            existing_device=(struct Device **)NULL;
          }
        }
        else
        {
          existing_device=(struct Device **)NULL;
        }
        variable_number = 0;
        current_output = cell->outputs;
        while (current_output != (struct Cell_output *)NULL)
        {
          variable_calculate = True;
          if (existing_device)
          {
            if (variable_number==existing_variable_number)
            {
              number_of_devices++;
              if (True==variable_calculate)
              {
                signal_number++;
                number_of_signals++;
              }
              existing_device_number--;
              existing_device++;
              while ((existing_device_number>0)&&!(*existing_device))
              {
                existing_device_number--;
                existing_device++;
              }
              if (0<existing_device_number)
              {
                existing_variable_number = 0;
                current_output_inner = cell->outputs;
                while ((current_output_inner != (struct Cell_output *)NULL)
                  && (strcmp((*existing_device)->description->name,
                    current_output_inner->name) != 0))
                {
                  existing_variable_number++;
                  current_output_inner = current_output_inner->next;
                }
              }
              else
              {
                existing_device=(struct Device **)NULL;
              }
            }
            else
            {
              if (True==variable_calculate)
              {
                signal_number++;
                number_of_devices++;
                number_of_signals++;
              }
            }
          }
          else
          {
            if (True==variable_calculate)
            {
              signal_number++;
              number_of_devices++;
              number_of_signals++;
            }
          }
          current_output = current_output->next;
          variable_number++;
        } /* while current_output ... */
        if (0<(*(cell->time_steps).TABT))
        {
          number_of_samples=1+(int)(((*(cell->time_steps).TEND)-
            (*(cell->time_steps).TSTART))/(*(cell->time_steps).TABT));
          frequency=1./(*(cell->time_steps).TABT);
        }
        else
        {
          number_of_samples=0;
        }
        if ((signal_number>0)&&(number_of_samples>0))
        {
          /* allocate space for signals */
          if (buffer=create_Signal_buffer(FLOAT_VALUE,number_of_signals,
            number_of_samples,frequency))
          {
            time=buffer->times;
            for (i=0;i<number_of_samples;i++)
            {
              *time=i;
              time++;
            }
            saved_buffers[number_of_saved_buffers-1]=buffer;
            /* set up the devices */
            if (ALLOCATE(devices,struct Device *,number_of_devices)&&
              (region=create_Region("cell",PATCH,0,0))&&(region_list=
                create_Region_list_item(region,
                  (struct Region_list_item *)NULL)))
            {
              device_number=0;
              signal_number=0;
              device=devices;
              variable_number=0;
              if (analysis->rig)
              {
                existing_device=analysis->rig->devices;
                existing_device_number=analysis->rig->number_of_devices;
                while ((existing_device_number>0)&&!(*existing_device))
                {
                  existing_device_number--;
                  existing_device++;
                }
                if (0<existing_device_number)
                {
                  existing_variable_number = 0;
                  current_output_inner = cell->outputs;
                  while ((current_output != (struct Cell_output *)NULL)
                    && (strcmp((*existing_device)->description->name,
                      current_output->name) != 0))
                  {
                    existing_variable_number++;
                    current_output = current_output->next;
                  }
                }
                else
                {
                  existing_device=(struct Device **)NULL;
                }
              }
              else
              {
                existing_device=(struct Device **)NULL;
              }
              variable_number = 0;
              current_output = cell->outputs;
              while ((current_output != (struct Cell_output *)NULL)
                && devices)
              {
                variable_calculate = True;
                if (True==variable_calculate)
                {
                  ((buffer->signals).float_values)[signal_number]=
                    current_output->value;
                  if (existing_device&&
                    (variable_number==existing_variable_number))
                  {
                    *device= *existing_device;
                    /* force the signal maximum and minimum to be
                    recalculated */
                    (*device)->signal_minimum=1;
                    (*device)->signal_maximum=0;
                    *existing_device=(struct Device *)NULL;
                    existing_device_number--;
                    existing_device++;
                    while ((existing_device_number>0)&&!(*existing_device))
                    {
                      existing_device_number--;
                      existing_device++;
                    }
                    if (0<existing_device_number)
                    {
                      existing_variable_number = 0;
                      current_output_inner = cell->outputs;
                      while ((current_output_inner !=
                        (struct Cell_output *)NULL)
                        && (strcmp((*existing_device)->description->name,
                          current_output_inner->name) != 0))
                      {
                        existing_variable_number++;
                        current_output_inner = current_output_inner->next;
                      }
                    }
                    else
                    {
                      existing_device=(struct Device **)NULL;
                    }
                    currents = True;
                    if (currents)
                    {
                      if (signal=create_Signal(signal_number,buffer,REJECTED,
                        number_of_saved_buffers-1))
                      {
                        signal->next=(*device)->signal;
                        (*device)->signal=signal;
                        device++;
                        device_number++;
                      }
                      else
                      {
                        *existing_device= *device;
                        while (device_number>0)
                        {
                          device--;
                          device_number--;
                          destroy_Device(device);
                        }
                        DEALLOCATE(devices);
                        display_message(ERROR_MESSAGE,"calculate_cell_window. "
                          "Could not create signal");
                      }
                    }
                    else
                    {
                      device++;
                      device_number++;
                    }
                  }
                  else
                  {
                    currents = True;
                    if (currents)
                    {
                      sprintf(device_name,"%s\0",
                        current_output->name);
                      if ((description=create_Device_description(device_name,
                        AUXILIARY,region))&&(channel=create_Channel(
                          signal_number+1,0,1))&&(signal=create_Signal(
                            signal_number,buffer,REJECTED,
                            number_of_saved_buffers-1))&&(*device=create_Device(
                              signal_number,description,channel,signal)))
                      {
                        device++;
                        device_number++;
                      }
                      else
                      {
                        if (description)
                        {
                          destroy_Device_description(&description);
                          if (channel)
                          {
                            destroy_Channel(&channel);
                            if (signal)
                            {
                              destroy_Signal(&signal);
                            }
                          }
                        }
                        while (device_number>0)
                        {
                          device--;
                          device_number--;
                          destroy_Device(device);
                        }
                        DEALLOCATE(devices);
                        display_message(ERROR_MESSAGE,
                          "calculate_cell_window.  Could not create device");
                      }
                    }
                  }
                  signal_number++;
                }
                else
                {
                  if (existing_device&&
                    (variable_number==existing_variable_number))
                  {
                    *device= *existing_device;
                    *existing_device=(struct Device *)NULL;
                    device++;
                    device_number++;
                    existing_device_number--;
                    existing_device++;
                    while ((existing_device_number>0)&&!(*existing_device))
                    {
                      existing_device_number--;
                      existing_device++;
                    }
                    if (0<existing_device_number)
                    {
                      existing_variable_number = 0;
                      current_output_inner = cell->outputs;
                      while ((current_output_inner !=
                        (struct Cell_output *)NULL)
                        && (strcmp((*existing_device)->description->name,
                          current_output_inner->name) != 0))
                      {
                        existing_variable_number++;
                        current_output_inner = current_output_inner->next;
                      }
                    }
                    else
                    {
                      existing_device=(struct Device **)NULL;
                    }
                  }
                }
                variable_number++;
                current_output = current_output->next;
              } /* while current_output */
              if (devices)
              {
                if (calculate_rig=create_Rig("cell",MONITORING_OFF,
                  EXPERIMENT_OFF,
                  number_of_devices,devices,(struct Page_list_item *)NULL,0,
                  region_list,(struct Region *)NULL))
                {
                  /* register the callbacks */
                  /* the focus callback on the analysis window requires
                  the system_window_structure identifier to be registered
                  ??? should check if these are already registered (ie. unemap
                  system window has been created) ??? */
                  if ((MrmSUCCESS==MrmRegisterNames(callback_list,
                    XtNumber(callback_list))) && (MrmSUCCESS ==
                      MrmRegisterNames(identifier_list,
                        XtNumber(identifier_list))))
                  {
                    if (ALLOCATE(user_settings,User_settings,1))
                    {
                      XtVaGetApplicationResources(cell->window,
                        (XtPointer)user_settings,resources,XtNumber(resources),
                        NULL);
                      /* display solution */
                      if (!create_analysis_work_area(&((cell->unemap).analysis),
                        cell->window,cell->shell,
                        /*global_cell->pointer_sensitivity*/2,
                        /*global_cell->signal_file_extension_read*/".sig*",
                        /*global_cell->signal_file_extension_write*/".signal",
                        /*global_cell->postscript_file_extension*/".ps",
                        /*global_cell->configuration_file_extension*/".cnfg",
                        user_settings->analysis_colour,
                        (struct Map_drawing_information *)NULL,
                        cell->user_interface,(struct Time_keeper *)NULL,
												(cell->unemap).package))
                      {
                        destroy_Rig(&calculate_rig);
                        clear_signals_window((Widget *)NULL,
                          (XtPointer)cell,
                          (XtPointer)NULL);
                        display_message(ERROR_MESSAGE,
                          "calculate_cell_window.  "
                          "Could not create analysis work area");
                      }
                      if ((cell->unemap).analysis.window_shell)
                      {
                        /* calculate the signals */
                        calculate_cell_model(cell->parameters,number_of_outputs,
                          number_of_samples,buffer->signals.float_values,
                          number_of_equations,number_of_currents,
                          number_of_rates,number_of_mechanics,
                          cell->current_model,cell);
                        /* clear the old analysis */
                        if (analysis->rig)
                        {
                          destroy_Rig(&(analysis->rig));
                        }
                        /* initialize the new analysis */
                        analysis->rig=calculate_rig;
                        analysis->datum=0;
                        analysis->potential_time=0;
                        analysis->highlight=(struct Device **)NULL;
                        analysis->map_type=NO_MAP_FIELD;
                        if (analysis->mapping_window)
                        {
                          /*analysis->mapping_window->activation_front= -1;*/
                          XtSetSensitive(
                            analysis->mapping_window->animate_button,False);
                          if (map=analysis->mapping_window->map)
                          {
                            map->colour_option=HIDE_COLOUR;
                            map->contours_option=HIDE_CONTOURS;
                            map->electrodes_option=SHOW_ELECTRODE_NAMES;
                            /*map->projection=HAMMER;*/
                          }
                          /* unghost the mapping window file button */
                          XtSetSensitive(analysis->mapping_window->file_button,
                            True);
                        }
                        /* unghost the write interval button */
                        XtSetSensitive(
                          analysis->window->file_menu.save_interval_button,True);
                        /* unghost the write interval as button */
                        XtSetSensitive(
                          analysis->window->file_menu.save_interval_as_button,
                          True);
                        /* unghost the print selected signals button */
                        XtSetSensitive(
                          analysis->window->print_menu.selected_button,True);
                        /* unghost the print all signals button */
                        XtSetSensitive(analysis->window->print_menu.all_button,
                          True);
                        /* unghost the display potential map button */
                        XtSetSensitive(
                          analysis->window->map_menu.potential_button,True);
                        /* higlight the first device */
                        if (analysis->highlight=analysis->rig->devices)
                        {
                          if (buffer=(*(analysis->highlight))->signal->buffer)
                          {
                            /* initialize the search interval */
                            analysis->start_search_interval=buffer->start;
                            analysis->end_search_interval=buffer->end;
                            /* initialize potential time */
                            analysis->potential_time=
                              (buffer->number_of_samples-1)/3;
                            /* initialize datum */
                            analysis->datum=2*(analysis->potential_time);
                          }
                        }
                        XtSetSensitive(
                          (cell->unemap).analysis.window->map_button,False);
                        /* pop up the analysis window shell */
                        XtPopup((cell->unemap).analysis.window_shell,XtGrabNone);
                        /* add analysis window to the shell_list */
                        /*XXXXXXXXX*/
                        update_signals_drawing_area(analysis->window);
                        update_interval_drawing_area(analysis->window);
                        trace_change_rig(analysis->trace);
                      }
                    }
                    else
                    {
                      display_message(ERROR_MESSAGE,"calculate_cell_window. "
                        "Unable to allocate memory for the user settings");
                    }
                  }
                  else
                  {
                    destroy_Rig(&calculate_rig);
                    clear_signals_window((Widget *)NULL,(XtPointer)cell,
                      (XtPointer)NULL);
                    display_message(ERROR_MESSAGE,
                      "calculate_cell_window.  Could not register callbacks");
                  }
                }
                else
                {
                  while (device_number>0)
                  {
                    device--;
                    device_number--;
                    destroy_Device(device);
                  }
                  DEALLOCATE(devices);
                  destroy_Region(&region);
                  clear_signals_window((Widget *)NULL,(XtPointer)cell,
                    (XtPointer)NULL);
                  display_message(ERROR_MESSAGE,"calculate_cell_window.  "
                    "Could not create rig");
                }
              }
              else
              {
                destroy_Region(&region);
                clear_signals_window((Widget *)NULL,(XtPointer)cell,
                  (XtPointer)NULL);
              }
            }
            else
            {
              if (devices)
              {
                DEALLOCATE(devices);
                if (region)
                {
                  destroy_Region(&region);
                }
              }
              clear_signals_window((Widget *)NULL,(XtPointer)cell,
                (XtPointer)NULL);
              display_message(ERROR_MESSAGE,"calculate_cell_window.  "
                "Could not create devices");
            }
          }
          else
          {
            ((cell->unemap).number_of_saved_buffers)--;
            clear_signals_window((Widget *)NULL,(XtPointer)cell,
              (XtPointer)NULL);
            display_message(ERROR_MESSAGE,
              "calculate_cell_window.  Could not create signal buffer");
          }
        }
        else
        {
          ((cell->unemap).number_of_saved_buffers)--;
          clear_signals_window((Widget *)NULL,(XtPointer)cell,
            (XtPointer)NULL);
          display_message(ERROR_MESSAGE,"Nothing to display");
        }
      }
      else
      {
        clear_signals_window((Widget *)NULL,(XtPointer)cell,
          (XtPointer)NULL);
        display_message(ERROR_MESSAGE,"calculate_cell_window. "
          "Could not save buffer");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"calculate_cell_window. "
        "No model defined");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"calculate_cell_window. "
      "Missing cell_window");
  }
  LEAVE;
} /* END calculate_cell_window() */

void clear_signals_window(Widget *widget_id,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 01 March 1999

DESCRIPTION :
Clears the signals window.
==============================================================================*/
{
  int i,number_of_saved_buffers;
  struct Analysis_work_area *analysis;
  struct Cell_window *cell;
  struct Signal_buffer **saved_buffers;

  ENTER(clear_signals_window);
  USE_PARAMETER(widget_id);
  USE_PARAMETER(call_data);
  if ((cell=(struct Cell_window *)cell_window) &&
    (analysis= &((cell->unemap).analysis)))
  {
    if (analysis->rig)
    {
      /* clear the signals */
      number_of_saved_buffers=(cell->unemap).number_of_saved_buffers;
      saved_buffers=(cell->unemap).saved_buffers;
      for (i=0;i<number_of_saved_buffers;i++)
      {
        destroy_Signal_buffer(saved_buffers+i);
      }
      DEALLOCATE(saved_buffers);
      (cell->unemap).number_of_saved_buffers=0;
      (cell->unemap).saved_buffers=(struct Signal_buffer **)NULL;
      destroy_Rig(&(analysis->rig));
      analysis->datum=0;
      analysis->potential_time=0;
      analysis->highlight=(struct Device **)NULL;
      analysis->map_type=NO_MAP_FIELD;
      update_signals_drawing_area(analysis->window);
      update_interval_drawing_area(analysis->window);
      trace_change_rig(analysis->trace);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"clear_signals_window. "
      "Missing cell_window");
  }
  LEAVE;
} /* END clear_signals_window */

int set_output_information(struct Cell_window *cell,char *name,char *type)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the information in the output's structure for use with UnEMAP. Should only
be called when reading the default files.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_output *new = (struct Cell_output *)NULL;
  struct Cell_output *current = (struct Cell_output *)NULL;
  char VARIABLE_string[] = {"Variable"};
  char CURRENT_string[] = {"Current"};
  char RATE_string[] = {"Rate"};
  char MECHANICS_string[] = {"Mechanics"};

  ENTER(set_output_information);
  if (cell && name && type)
  {
    if (ALLOCATE(new,struct Cell_output,1))
    {
      /* initialise */
      new->name = (char *)NULL;
      new->next = (struct Cell_output *)NULL;
      /* set the type */
      if (!strcmp(type,VARIABLE_string))
      {
        new->type = VARIABLE;
      }
      else if (!strcmp(type,CURRENT_string))
      {
        new->type = CURRENT;
      }
      else if (!strcmp(type,RATE_string))
      {
        new->type = RATE;
      }
      else if (!strcmp(type,MECHANICS_string))
      {
        new->type = MECHANICS;
      }
      else
      {
        display_message(WARNING_MESSAGE,"set_output_information. "
          "Unknown output type: %s",type);
        new->type = CELL_OUTPUT_UNKNOWN;
      }
      if (ALLOCATE(new->name,char,strlen(name)+1))
      {
        sprintf(new->name,"%s\0",name);
        /* now add to the output list */
        if (cell->outputs != (struct Cell_output *)NULL)
        {
          /* add to the end of the list */
          current = cell->outputs;
          while (current->next != (struct Cell_output *)NULL)
          {
            current = current->next;
          }
          current->next = new;
        }
        else
        {
          /* create the list */
          cell->outputs = new;
        }
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"set_output_information. "
          "Unable to allocate memory for the new output's name");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"set_output_information. "
        "Unable to allocate memory for the new output");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_output_information. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END set_output_information() */

void destroy_cell_outputs(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 03 March 1999

DESCRIPTION :
Destroys the current list of outputs.
==============================================================================*/
{
  struct Cell_output *current,*tmp;
  ENTER(destroy_cell_outputs);
  if (cell)
  {
    if (cell->outputs != (struct Cell_output *)NULL)
    {
      current = cell->outputs;
      while (current->next != (struct Cell_output *)NULL)
      {
        tmp = current->next;
        DEALLOCATE(current);
        current = tmp;
      }
      DEALLOCATE(current);
      cell->outputs = (struct Cell_output *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"destroy_cell_outputs. "
      "Missing Cell window");
  }
  LEAVE;
} /* END destroy_cell_outputs() */
