/*******************************************************************************
FILE : cell.c

LAST MODIFIED : 16 January 2001

DESCRIPTION :
Main program for cell.  Based on unemap.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "cell/cell_interface.h"
#include "cell/cell_window.h"
#include "general/debug.h"
#include "general/error_handler.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

#include "graphics/glyph.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "graphics/transform_tool.h"
#include "interaction/interactive_tool.h"
#include "interaction/select_tool.h"
#include "selection/any_object_selection.h"
#include "time/time_keeper.h"

/*
Module constants
----------------
*/
#define CHARSET XmSTRING_DEFAULT_CHARSET
#define MAX_NUMBER_MESSAGES 10

/*
Module variables
----------------
*/
static struct Cell_interface *cell_interface = (struct Cell_interface *)NULL;

/*
Module types
------------
*/

/*
Module functions
----------------
*/
static int display_error_message(char *message,void *dummy)
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Display a Cell error message.
==============================================================================*/
{
	int return_code;

  ENTER(display_error_message);
  USE_PARAMETER(dummy);
  return_code = printf("ERROR: %s\n",message);
  LEAVE;
  return (return_code);
} /* display_error_message */

static int display_information_message(char *message,void *dummy)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Display a Cell information message.
==============================================================================*/
{
	int return_code;

  ENTER(display_error_message);
  USE_PARAMETER(dummy);
  return_code = printf("%s",message);
  LEAVE;
  return (return_code);
} /* display_information_message */

static int display_warning_message(char *message,void *dummy)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Display a Cell warning message.
==============================================================================*/
{
	int return_code;

  ENTER(display_warning_message);
  USE_PARAMETER(dummy);
  return_code = printf("Warning: %s\n",message);
  LEAVE;
  return (return_code);
} /* display_warning_message */


/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Main program for cell
==============================================================================*/
{
	int return_code=0;
  float default_light_direction[3]={0.0,-0.5,-1.0};
  struct Colour default_colour,ambient_colour;
	struct GT_object *glyph;
  struct Select_tool *select_tool;
  struct Interactive_tool *transform_tool;
	struct User_interface user_interface;
  struct Any_object_selection *any_object_selection;
  struct Colour background_colour;
  struct Graphical_material *default_graphical_material;
  struct Light *default_light;
  struct Light_model *default_light_model;
  struct Scene *default_scene;
  struct Spectrum *default_spectrum;
  struct Time_keeper *time_keeper;
  struct LIST(GT_object) *graphics_object_list;
  struct LIST(GT_object) *glyph_list;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
  struct MANAGER(Light) *light_manager;
  struct MANAGER(Light_model) *light_model_manager;
  struct MANAGER(Graphical_material) *graphical_material_manager;
  struct MANAGER(Scene) *scene_manager;
  struct MANAGER(Spectrum) *spectrum_manager;
  struct MANAGER(Texture) *texture_manager;
  
	ENTER(main);
	/* display the version */
	display_message(INFORMATION_MESSAGE, VERSION "\n");
	/* open the user interface */
	user_interface.application_context=(XtAppContext)NULL;
	user_interface.application_name="cell";
	user_interface.application_shell=(Widget)NULL;
	user_interface.argc_address= &argc;
	user_interface.argv=argv;
	user_interface.class_name="Cell";
	user_interface.display=(Display *)NULL;
	if (open_user_interface(&user_interface))
	{
    /* set up messages */
    set_display_message_function(ERROR_MESSAGE,display_error_message,
      (void *)NULL);
    set_display_message_function(INFORMATION_MESSAGE,
      display_information_message,(void *)NULL);
    set_display_message_function(WARNING_MESSAGE,display_warning_message,
      (void *)NULL);

    /* Create the CMGUI objects/managers */
    any_object_selection = CREATE(Any_object_selection)();
    background_colour.red=(float)0;
    background_colour.green=(float)0;
    background_colour.blue=(float)0;
    if (graphical_material_manager = CREATE(MANAGER(Graphical_material))())
    {
      if (default_graphical_material = CREATE(Graphical_material)("default"))
      {
        /* ACCESS so can never be destroyed */
        ACCESS(Graphical_material)(default_graphical_material);
        if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(
          default_graphical_material,
          graphical_material_manager))
        {
          DEACCESS(Graphical_material)(
            &default_graphical_material);
        }
      }
    }
    if (light_manager = CREATE(MANAGER(Light))())
    {
      if (default_light = CREATE(Light)("default"))
      {
        set_Light_type(default_light,INFINITE_LIGHT);
        default_colour.red=1.0;
        default_colour.green=1.0;
        default_colour.blue=1.0;		
        set_Light_colour(default_light,&default_colour); 
        set_Light_direction(default_light,default_light_direction);
        ACCESS(Light)(default_light);
        if (!ADD_OBJECT_TO_MANAGER(Light)(default_light,light_manager))
        {
          DEACCESS(Light)(&default_light);
        }
      }
    }
    if (light_model_manager = CREATE(MANAGER(Light_model))())
    {
      if (default_light_model = CREATE(Light_model)("default"))
      {
        ambient_colour.red=0.2;
        ambient_colour.green=0.2;
        ambient_colour.blue=0.2;
        Light_model_set_ambient(default_light_model,&ambient_colour);
        Light_model_set_side_mode(default_light_model,
          LIGHT_MODEL_TWO_SIDED);
        ACCESS(Light_model)(default_light_model);
        if (!ADD_OBJECT_TO_MANAGER(Light_model)(
          default_light_model,light_model_manager))
        {
          DEACCESS(Light_model)(&default_light_model);
        }			
      }
    }
    if (glyph_list = CREATE(LIST(GT_object))())
    {
      /* add standard glyphs */
      if (glyph = make_glyph_arrow_line("arrow_line",0.25,0.125))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_arrow_solid("arrow_solid",12,2./3.,1./6.))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_axes("axes",0.1,0.025,0.1))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_cone("cone",12))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_cross("cross"))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_cylinder("cylinder6",6))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_cylinder("cylinder",12))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_cylinder("cylinder_hires",48))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_sphere("diamond",4,2))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_line("line"))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_point("point",g_POINT_MARKER,0))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_sheet("sheet"))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_sphere("sphere",12,6))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
      if (glyph=make_glyph_sphere("sphere_hires",48,24))
      {
        ADD_OBJECT_TO_LIST(GT_object)(glyph,glyph_list);
      }
    }
    graphics_object_list = CREATE(LIST(GT_object))();
    if (spectrum_manager = CREATE(MANAGER(Spectrum))())
    {
      if (default_spectrum = CREATE(Spectrum)("default"))
      {
        Spectrum_set_simple_type(default_spectrum,
          BLUE_TO_RED_SPECTRUM);
        Spectrum_set_minimum_and_maximum(default_spectrum,0,1);
        /* ACCESS so can never be destroyed */
        ACCESS(Spectrum)(default_spectrum);
        if (!ADD_OBJECT_TO_MANAGER(Spectrum)(default_spectrum,
          spectrum_manager))
        {
          DEACCESS(Spectrum)(&default_spectrum);
        }
      }
    }
    texture_manager = CREATE(MANAGER(Texture))();
    if (scene_manager = CREATE(MANAGER(Scene))())
    {
      if (default_scene = CREATE(Scene)("default"))
      {
        Scene_enable_graphics(default_scene,glyph_list,
          graphical_material_manager,
          default_graphical_material,light_manager,
          spectrum_manager,default_spectrum,
          texture_manager);
        Scene_disable_time_behaviour(default_scene);
        ACCESS(Scene)(default_scene);
        if (!ADD_OBJECT_TO_MANAGER(Scene)(default_scene,scene_manager))
        {
          DEACCESS(Scene)(&default_scene);
        }
      }
    }
    interactive_tool_manager=CREATE(MANAGER(Interactive_tool))();
    transform_tool = CREATE(Interactive_tool_transform)(
      &user_interface);
    ADD_OBJECT_TO_MANAGER(Interactive_tool)(transform_tool,
      interactive_tool_manager);
		select_tool = CREATE(Select_tool)(interactive_tool_manager,
			any_object_selection,default_graphical_material);
    time_keeper=ACCESS(Time_keeper)(CREATE(Time_keeper)("default",
      &user_interface));
    
    /* create the cell interface */
    if (cell_interface = CREATE(Cell_interface)(any_object_selection,
      &background_colour,default_graphical_material,default_light,
      default_light_model,default_scene,default_spectrum,time_keeper,
      graphics_object_list,glyph_list,interactive_tool_manager,light_manager,
      light_model_manager,graphical_material_manager,scene_manager,
      spectrum_manager,texture_manager,&user_interface,exit_cell_window
#if defined (CELL_DISTRIBUTED)
      ,(struct Element_point_ranges_selection *)NULL
#endif /* defined (CELL_DISTRIBUTED) */
      ))
    {
      /* the GUI loop */
      return_code = application_main_loop(&user_interface);
      /* clean-up */
      DESTROY(Cell_interface)(&cell_interface);
      /* reset up messages */
      set_display_message_function(ERROR_MESSAGE,
        (Display_message_function *)NULL, NULL);
      set_display_message_function(INFORMATION_MESSAGE,
        (Display_message_function *)NULL, NULL);
      set_display_message_function(WARNING_MESSAGE,
        (Display_message_function *)NULL, NULL);
      /* close the user interface */
      close_user_interface(&user_interface);
      DEACCESS(Time_keeper)(&time_keeper);
      if (select_tool)
      {
        DESTROY(Select_tool)(&select_tool);
      }
      if (transform_tool)
      {
        DESTROY(Interactive_tool)(&transform_tool);
      }
      DESTROY(MANAGER(Interactive_tool))(
        &interactive_tool_manager);
      DEACCESS(Scene)(&default_scene);
      DESTROY(MANAGER(Scene))(&scene_manager);
      DESTROY(Any_object_selection)(&any_object_selection);
      DESTROY(LIST(GT_object))(&graphics_object_list);
      DESTROY(LIST(GT_object))(&glyph_list);
      DEACCESS(Spectrum)(&default_spectrum);
      DESTROY(MANAGER(Spectrum))(&spectrum_manager);
      DEACCESS(Graphical_material)(&default_graphical_material);			
      DESTROY(MANAGER(Graphical_material))(&graphical_material_manager);
      DESTROY(MANAGER(Texture))(&texture_manager);
      DEACCESS(Light_model)(&default_light_model);
      DESTROY(MANAGER(Light_model))(&light_model_manager);
      DEACCESS(Light)(&default_light);
      DESTROY(MANAGER(Light))(&light_manager);
    }
    else
    {
      display_message(ERROR_MESSAGE,"main. "
        "Unable to create Cell interface");
    }
    /* close the user interface */
    close_user_interface(&user_interface);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Could not open user interface");
    return_code=0;
  }
  LEAVE;
  return(return_code);
} /* main */
