/*******************************************************************************
FILE : mapping.h

LAST MODIFIED : 31 May 2000

DESCRIPTION :
==============================================================================*/
#if !defined (MAPPING_H)
#define MAPPING_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#endif /* defined (MOTIF) */
#include "general/geometry.h"
#include "graphics/spectrum.h"
#if defined (UNEMAP_USE_3D)
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "graphics/graphics_window.h"
#include "graphics/graphical_element.h"
#include "graphics/element_group_settings.h"
#include "interaction/interactive_tool.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"  
#include "selection/node_selection.h"
#endif
#include "unemap/drawing_2d.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
enum Map_type
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
==============================================================================*/
{
	NO_MAP_FIELD,
	SINGLE_ACTIVATION,
	MULTIPLE_ACTIVATION,
	INTEGRAL,
	POTENTIAL
}; /* enum Map_type */

enum Interpolation_type
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
==============================================================================*/
{
	NO_INTERPOLATION,
	BICUBIC_INTERPOLATION,
	BILINEAR_INTERPOLATION
}; /* enum Interpolation_type */

#if defined (OLD_CODE)
/*???DB.  Replaced by Projection_type in general/geometry.h */
enum Projection
/*******************************************************************************
LAST MODIFIED : 24 June 1995

DESCRIPTION :
???DB.  There will be different types of CYLINDRICAL, depending on where the
cut is and if the projection should overlap (3/2 times round).
==============================================================================*/
{
	CYLINDRICAL,
	HAMMER,
	POLAR
}; /* enum Projection */
#endif /* defined (OLD_CODE) */

enum Colour_option
/*******************************************************************************
LAST MODIFIED : 3 February 1993

DESCRIPTION :
==============================================================================*/
{
	SHOW_COLOUR,
	HIDE_COLOUR
}; /* enum Colour_option */

enum Contours_option
/*******************************************************************************
LAST MODIFIED : 3 February 1993

DESCRIPTION :
==============================================================================*/
{
	SHOW_CONTOURS,
	HIDE_CONTOURS
}; /* enum Contours_option */

enum Electrodes_option
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION : text accompanying electrodes
==============================================================================*/
{
	SHOW_ELECTRODE_NAMES,
	SHOW_ELECTRODE_VALUES,
	SHOW_CHANNEL_NUMBERS,
	HIDE_ELECTRODES
}; /* enum Electrodes_option */

enum Electrodes_marker_type
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
==============================================================================*/
{
	CIRCLE_ELECTRODE_MARKER,
	HIDE_ELECTRODE_MARKER,
	PLUS_ELECTRODE_MARKER,
	SQUARE_ELECTRODE_MARKER
}; /* enum Electrodes_marker_type */

enum Fibres_option
/*******************************************************************************
LAST MODIFIED : 4 February 1994

DESCRIPTION :
==============================================================================*/
{
	HIDE_FIBRES,
	SHOW_FIBRES_FINE,
	SHOW_FIBRES_MEDIUM,
	SHOW_FIBRES_COARSE
}; /* enum Fibres_option */

enum Landmarks_option
/*******************************************************************************
LAST MODIFIED : 4 February 1994

DESCRIPTION :
==============================================================================*/
{
	SHOW_LANDMARKS,
	HIDE_LANDMARKS
}; /* enum Landmarks_option */

enum Extrema_option
/*******************************************************************************
LAST MODIFIED : 22 May 1997

DESCRIPTION :
==============================================================================*/
{
	SHOW_EXTREMA,
	HIDE_EXTREMA
}; /* enum Extrema_option */

enum Contour_thickness
/*******************************************************************************
LAST MODIFIED : 7 August 1994

DESCRIPTION :
==============================================================================*/
{
	CONSTANT_THICKNESS,
	VARIABLE_THICKNESS
}; /* enum Contour_thickness */

#if defined (UNEMAP_USE_3D)
struct Map_3d_package	
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION : Information specific to each individual 3d map.
??JW possibly scene and scene_viewer should be moved here from 
map_drawing_information.
==============================================================================*/
{	
	char *fit_name;
	enum Electrodes_option electrodes_option;
	FE_value electrode_size;
	/* a flag  */
	int colour_electrodes_with_signal;
	int access_count,number_of_contours,number_of_map_columns,number_of_map_rows;
	struct GT_element_settings **contour_settings;
	struct FE_field *map_position_field,*map_fit_field;	
	struct FE_node_order_info *node_order_info;	
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *node_group;		
	struct GT_object *electrode_glyph;	
	struct GT_object *torso_arm_labels;/*FOR AJP*/
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(FE_node) *data_manager;
	struct MANAGER(GROUP(FE_element))	*element_group_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager,*node_group_manager;  
};
#endif /* defined (UNEMAP_USE_3D) */

struct Map_drawing_information
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Information needed for drawing a map.  Windowing system dependent
This information is common to all maps.
??JW possibly scene and scene_viewer should be moved to map_drawing_information
from here.
==============================================================================*/
{
#if defined (MOTIF)
	Boolean maintain_aspect_ratio;
	Colormap colour_map;
	int read_only_colour_map;
#endif /* defined (MOTIF) */
	int number_of_spectrum_colours,pixels_between_contour_values;
#if defined (MOTIF)
	Pixel background_drawing_colour,boundary_colour,contour_colour,fibre_colour,
		highlighted_colour,landmark_colour,node_text_colour,*spectrum_colours,
		spectrum_text_colour,unhighlighted_colour;
	struct
	{
		GC background_drawing_colour,contour_colour,copy,fibre_colour,
			highlighted_colour,node_marker_colour,spectrum,spectrum_marker_colour,
			unhighlighted_colour;
		/*???DB.  When using an RS/6000 model with the Color Graphics Display
			Adapter (#2770), GCs stop writing text to the pixel map after they've
			been used for drawing lines to the window.  So I duplicate them */
		/*???DB.  How should background_drawing_colour,copy be shared ? */
		GC node_text_colour,spectrum_text_colour;
	} graphics_context;
#endif /* defined (MOTIF) */
	struct Spectrum *spectrum;
#if defined (MOTIF)
	XColor *spectrum_rgb;
	XFontStruct *font;
#endif /* defined (MOTIF) */
#if defined (UNEMAP_USE_3D)
	/* These are for the map 3d graphics */
	/* Flag to record if done "view all" on scene*/
	int viewed_scene; 
	struct Colour *background_colour;
	struct Colour *electrode_colour;
	struct Colour *no_interpolation_colour;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection,*node_selection; 
	struct Light *light;	
	struct Light_model *light_model;	
	struct Scene *scene;	
	struct Scene_viewer *scene_viewer;
	struct Graphical_material *map_graphical_material;
	struct Graphical_material *electrode_graphical_material;
	struct Time_keeper *time_keeper;	
	struct Computed_field_package *computed_field_package;
	struct LIST(GT_object) *glyph_list;
	struct MANAGER(Texture) *texture_manager;
	struct MANAGER(Scene) *scene_manager;
	struct MANAGER(Light_model) *light_model_manager;
	struct MANAGER(Light) *light_manager;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct MANAGER(FE_node) *data_manager;	
	struct MANAGER(Interactive_tool) *interactive_tool_manager;		
#endif /* defined (UNEMAP_USE_3D) */
	struct User_interface *user_interface;
}; /* struct Map_drawing_information */

struct Map_frame
/*******************************************************************************
LAST MODIFIED : 7 October 1997

DESCRIPTION :
==============================================================================*/
{
	/* save the interpolated value at each pixel to be used for contouring */
		/*???DB.  Is this the best place for this ?  Drawing size dependent */
	float *pixel_values;
	/* extrema */
	float maximum,minimum;
	int maximum_x,maximum_y,minimum_x,minimum_y;
	struct Region *maximum_region,*minimum_region;
	/* for writing contour values */
	short int *contour_x,*contour_y;
#if defined (MOTIF)
	XImage *image;
		/* stored by the X client (application) machine */
#endif /* defined (MOTIF) */
}; /* struct Map_frame */

struct Map
/*******************************************************************************
LAST MODIFIED : 22 June 1998

DESCRIPTION :
==============================================================================*/
{
	enum Map_type *type;
	int *event_number;
	int *potential_time;
	int *datum;
	/* for integral maps */
	/* a flag 1 = use signal  value to get colour */
	int colour_electrodes_with_signal; 
	int *end_search_interval,*start_search_interval;
	enum Colour_option colour_option;
	enum Contours_option contours_option;
	enum Electrodes_option electrodes_option;
	enum Electrodes_marker_type electrodes_marker_type;
	int electrodes_marker_size;
	enum Interpolation_type interpolation_type;
	enum Fibres_option fibres_option;
	enum Landmarks_option landmarks_option;
	enum Extrema_option extrema_option;
	enum Projection_type projection_type;
	int maintain_aspect_ratio;
	int print_spectrum;
	char undecided_accepted;
	struct Rig **rig_pointer;
	int number_of_electrodes;
	struct Device **electrodes;
	int *electrode_x,*electrode_y;
	float *electrode_value;
	int number_of_auxiliary;
	char *electrode_drawn;
	struct Device **auxiliary;
	int *auxiliary_x,*auxiliary_y;
	int fixed_range;
		/*???DB.  Should be combined with spectrum_range */
	int range_changed;
	float contour_maximum,contour_minimum,maximum_value,minimum_value;
	int activation_front,number_of_contours;
		/*???DB.  Replace activation_front by frame_number ? */
	int colour_bar_bottom,colour_bar_left,colour_bar_right,colour_bar_top;
	/* for drawing values on contours.  The drawing area is divided into equal
		sized area (number depends on a "density" specified in the user settings).
		For each area and each possible contour value there is a point (possibly
		(-1,-1)) for putting the contour value at */
	int number_of_contour_areas,number_of_contour_areas_in_x;
	enum Contour_thickness contour_thickness;
	/* for potential movies */
	float frame_end_time,frame_start_time;
	int frame_number,number_of_frames;
	struct Map_frame *frames;
	/* set if the map is being printed */
	char print;
	/*???DB.  Used to be in user_settings */
	float membrane_smoothing,plate_bending_smoothing;
	int finite_element_mesh_columns,finite_element_mesh_rows;
	struct Map_drawing_information *drawing_information;
#if defined (OLD_CODE)
	struct User_interface *user_interface;
#endif /* defined (OLD_CODE) */
	struct Unemap_package *unemap_package;
}; /* struct Map */

/*
Global functions
----------------
*/
struct Map *create_Map(enum Map_type *map_type,enum Colour_option colour_option,
	enum Contours_option contours_option,enum Electrodes_option electrodes_option,
	enum Fibres_option fibres_option,enum Landmarks_option landmarks_option,
	enum Extrema_option extrema_option,int maintain_aspect_ratio,
	int print_spectrum,enum Projection_type projection_type,
	enum Contour_thickness contour_thickness,struct Rig **rig_pointer,
	int *event_number_address,int *potential_time_address,int *datum_address,
	int *start_search_interval_address,int *end_search_interval_address,
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface,struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
This function allocates memory for a map and initializes the fields to the
specified values.  It returns a pointer to the created map if successful and
NULL if not successful.
==============================================================================*/

int destroy_Map(struct Map **map);
/*******************************************************************************
LAST MODIFIED : 19 May 1992

DESCRIPTION :
This function deallocates the memory associated with the fields of <**map>
(except the <rig_pointer> field), deallocates the memory for <**map> and sets
<*map> to NULL.
==============================================================================*/

int update_colour_map_unemap(struct Map *map,struct Drawing_2d *drawing);
/*******************************************************************************
LAST MODIFIED : 6 July 1997

DESCRIPTION :
Updates the colour map being used for map.
???DB.  <drawing> added because of read only colour maps.
==============================================================================*/

int draw_map(struct Map *map,int recalculate,struct Drawing_2d *drawing);
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION:
Call draw_map_2d or draw_map_3d depending upon <map>->projection_type.
==============================================================================*/

#if defined (UNEMAP_USE_3D)
int map_remove_torso_arms(struct Map *map);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Removes the torso arms from the scene.
??JW perhaps should have a scene and scene_viewer in each Map_3d_package,
and do Scene_remove_graphics_object in the DESTROY Map_3d_package
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#if defined (UNEMAP_USE_3D)
int draw_map_3d(struct Map *map);
/*******************************************************************************
LAST MODIFIED : 15 May 2000

DESCRIPTION :
This function draws the <map> in as a 3D CMGUI scene.
==============================================================================*/
#endif /* defined (UNEMAP_USE_NODES)*/

int draw_map_2d(struct Map *map,int recalculate,struct Drawing_2d *drawing);
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
This function draws the <map> in the <drawing>.  If <recalculate> is >0 then the
colours for the pixels are recalculated.  If <recalculate> is >1 then the
interpolation functions are also recalculated.  If <recalculate> is >2 then the
<map> is resized to match the <drawing>.
==============================================================================*/

int draw_colour_or_auxiliary_area(struct Map *map,struct Drawing_2d *drawing);
/*******************************************************************************
LAST MODIFIED : 20 December 1996

DESCRIPTION :
This function draws the colour bar or the auxiliary inputs in the <drawing>.
==============================================================================*/

struct Map_drawing_information *create_Map_drawing_information(
	struct User_interface *user_interface
#if defined (UNEMAP_USE_3D) 
	,struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Light_model) *light_model_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(FE_node) *data_manager,
	struct LIST(GT_object) *glyph_list,
	struct Graphical_material *graphical_material,
	struct Computed_field_package *computed_field_package,
	struct Light *light,
	struct Light_model *light_model
#endif /* defined (UNEMAP_USE_NODES) */
       );
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/

int destroy_Map_drawing_information(
	struct Map_drawing_information **map_drawing_information_address);
/*******************************************************************************
LAST MODIFIED : 21 June 1997

DESCRIPTION :
==============================================================================*/

#if defined (UNEMAP_USE_3D)
PROTOTYPE_OBJECT_FUNCTIONS(Map_3d_package);

struct Map_3d_package *CREATE(Map_3d_package)(	int number_of_map_rows,
	int number_of_map_columns,
	char *fit_name,
	struct FE_node_order_info *node_order_info,
	struct FE_field *map_position_field,struct FE_field *map_fit_field,
	struct GROUP(FE_node) *node_group,struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(GROUP(FE_element))	*element_group_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager, 
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(Computed_field) *computed_field_manager);
/*******************************************************************************
LAST MODIFIED : 8 September 1999

DESCRIPTION:
Create and  and set it's components 
==============================================================================*/

int DESTROY(Map_3d_package)(struct Map_3d_package **map_3d_package_address);
/*******************************************************************************
LAST MODIFIED : 8 September 1999

DESCRIPTION :
Frees the memory for the Map_3d_package and sets <*package_address>
to NULL.
==============================================================================*/

int free_map_3d_package_map_contours(struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Frees the array of map contour GT_element_settings stored in the <map_3d_package>
==============================================================================*/

struct GT_object *get_map_3d_package_electrode_glyph(
	struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the map_electrode_glyph for map_3d_package 
??JW perhaps should maintain a list of GT_objects, cf CMGUI
==============================================================================*/

int set_map_3d_package_electrode_glyph(struct Map_3d_package *map_3d_package,
	struct GT_object *electrode_glyph);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Sets the <electrode_glyph>  for <map_3d_package>
??JW perhaps should maintain a list of GT_objects, cf CMGUI
==============================================================================*/

struct GT_object *get_map_3d_package_torso_arm_labels(
	struct Map_3d_package *map_3d_package);/*FOR AJP*/
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the map_torso_arm_labels for map_3d_package 
??JW perhaps should maintain a list of GT_objects, cf CMGUI
==============================================================================*/

int set_map_3d_package_torso_arm_labels(struct Map_3d_package *map_3d_package,
	struct GT_object *torso_arm_labels);/*FOR AJP*/
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Sets the torso_arm_labels  for map_3d_package
??JW perhaps should maintain a list of GT_objects, cf CMGUI
==============================================================================*/

FE_value get_map_3d_package_electrode_size(struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
gets the map_electrode_size for map_3d_package 
==============================================================================*/

int set_map_3d_package_electrode_size(struct Map_3d_package *map_3d_package,
	FE_value electrode_size);
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Sets the electrode_size  for map_3d_package 
==============================================================================*/

enum Electrodes_option get_map_3d_package_electrodes_option(
	struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the map_electrodes_option for map_3d_package 
==============================================================================*/

int set_map_3d_package_electrodes_option(struct Map_3d_package *map_3d_package,
	enum Electrodes_option electrodes_option);
/*******************************************************************************
LAST MODIFIED :  5 July 2000

DESCRIPTION :
Sets the electrodes_option  for map_3d_package
==============================================================================*/

int get_map_3d_package_colour_electrodes_with_signal(
	struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the map_colour_electrodes_with_signal for map_3d_package 
==============================================================================*/

int set_map_3d_package_colour_electrodes_with_signal(
	struct Map_3d_package *map_3d_package,int colour_electrodes_with_signal);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Sets the colour_electrodes_with_signal  for map_3d_package 
==============================================================================*/


int get_map_3d_package_number_of_map_rows(	struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the number_of_map_rows for map_3d_package 
Returns -1 on error
==============================================================================*/

int get_map_3d_package_number_of_map_columns(struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the number_of_map_columns for map_3d_package
Returns -1 on error
==============================================================================*/

int get_map_3d_package_contours(struct Map_3d_package *map_3d_package,
	int *number_of_contours,struct GT_element_settings ***contour_settings);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
gets the <number_of_contours> and <contour_settings> for map_3d_package 
==============================================================================*/

int set_map_3d_package_contours(struct Map_3d_package *map_3d_package,
	int number_of_contours,struct GT_element_settings **contour_settings);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
sets the <number_of_contours> and <contour_settings> for map_3d_package 
==============================================================================*/

char *get_map_3d_package_fit_name(struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the fit_name for map_3d_package 
==============================================================================*/

struct FE_node_order_info *get_map_3d_package_node_order_info(
	struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the node_order_info for map_3d_package 
==============================================================================*/

struct FE_field *get_map_3d_package_position_field(struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 8 July 2000

DESCRIPTION :
gets the map_position_field for map_3d_package 
==============================================================================*/

struct FE_field *get_map_3d_package_fit_field(struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : September 8 1999

DESCRIPTION :
gets the map_fit_field for map_3d_package
==============================================================================*/

struct GROUP(FE_node) *get_map_3d_package_node_group(struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the node_group for map_3d_package 
==============================================================================*/

struct GROUP(FE_element) *get_map_3d_package_element_group(
	struct Map_3d_package *map_3d_package);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
gets the element_group for map_3d_package
==============================================================================*/

int set_map_3d_package_element_group(struct Map_3d_package *map_3d_package,
	struct GROUP(FE_element) *map_element_group);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Sets the element_group for map_3d_package
==============================================================================*/

int get_map_drawing_information_viewed_scene(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets the viewed_scene flag of the <map_drawing_information>
==============================================================================*/

int set_map_drawing_information_viewed_scene(
struct Map_drawing_information *map_drawing_information,int scene_viewed);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
sets the viewed_scene flag of the <map_drawing_information>
 to 1 if <scene_viewed> >0,
0 if <scene_viewed> = 0.
==============================================================================*/

struct Colour *get_map_drawing_information_background_colour(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Colour of the <map_drawing_information>.
==============================================================================*/

int set_map_drawing_information_background_colour(
	struct Map_drawing_information *map_drawing_information,
	struct Colour *background_colour);
/*******************************************************************************
LAST MODIFIED : 14 July  2000

DESCRIPTION :
Sets the Colour of the <map_drawing_information>.
==============================================================================*/

struct Colour *get_map_drawing_information_no_interpolation_colour(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets the no_interpolation_colour of the <map_drawing_information>.
==============================================================================*/

struct MANAGER(Light) *get_map_drawing_information_Light_manager(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets a manager of the <map_drawing_information>.
==============================================================================*/

struct MANAGER(Texture) *get_map_drawing_information_Texture_manager(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets a manager of the <map_drawing_information>.
==============================================================================*/

struct MANAGER(Scene) *get_map_drawing_information_Scene_manager(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets a manager of the <map_drawing_information>.
==============================================================================*/

struct MANAGER(Light_model) *get_map_drawing_information_Light_model_manager(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets a manager of the <map_drawing_information>.
==============================================================================*/

struct MANAGER(Graphical_material) 
		 *get_map_drawing_information_Graphical_material_manager(
			 struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets a manager of the <map_drawing_information>.
==============================================================================*/

struct Light *get_map_drawing_information_light(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Light of the <map_drawing_information>.
==============================================================================*/

int set_map_drawing_information_light(
	struct Map_drawing_information *map_drawing_information,struct Light *light);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the Light of the <map_drawing_information>.
==============================================================================*/

struct Light_model *get_map_drawing_information_light_model(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Light_model of the <map_drawing_information>.
==============================================================================*/

int set_map_drawing_information_light_model(
	struct Map_drawing_information *map_drawing_information,
	struct Light_model *light_model);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the Light_model of the <map_drawing_information>.
==============================================================================*/

struct Scene_viewer *get_map_drawing_information_scene_viewer(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Scene_viewer of the <map_drawing_information>.
==============================================================================*/

int set_map_drawing_information_scene_viewer(
	struct Map_drawing_information *map_drawing_information,
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the Scene_viewer of the <map_drawing_information>.
==============================================================================*/

struct Scene *get_map_drawing_information_scene(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Scene of the <map_drawing_information>.
==============================================================================*/

int set_map_drawing_information_scene(
	struct Map_drawing_information *map_drawing_information,
	struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the Scene of the <map_drawing_information>.
==============================================================================*/

struct User_interface *get_map_drawing_information_user_interface(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the User_interface of the <map_drawing_information>.
==============================================================================*/

int set_map_drawing_information_user_interface(
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the User_interface of the <map_drawing_information>.
==============================================================================*/

struct Graphical_material *get_map_drawing_information_map_graphical_material(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the map_graphical_material of the <map_drawing_information>.
==============================================================================*/

int set_map_drawing_information_map_graphical_material(
	struct Map_drawing_information *map_drawing_information,
	struct Graphical_material *map_graphical_material);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the map_graphical_material of the <map_drawing_information>.
==============================================================================*/

struct Graphical_material *get_map_drawing_information_electrode_graphical_material(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED : 14 July  2000

DESCRIPTION :
gets the electrode_graphical_material of the <map_drawing_information>.
==============================================================================*/

struct Computed_field_package *get_map_drawing_information_computed_field_package(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED :  14 July  2000

DESCRIPTION :
gets the Computed_field_package of the <map_drawing_information>.
==============================================================================*/

int set_map_drawing_information_computed_field_package(
	struct Map_drawing_information *map_drawing_information,
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 14 July  2000

DESCRIPTION :
Sets the Computed_field_package of the <map_drawing_information>.
==============================================================================*/

struct Time_keeper *get_map_drawing_information_time_keeper(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED :  14 July 2000

DESCRIPTION :
gets the Time_keeper of the <map_drawing_information>.
==============================================================================*/

int set_map_drawing_information_time_keeper(
	struct Map_drawing_information *map_drawing_information,
	struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets the Time_keeper of the <map_drawing_information>.
==============================================================================*/

int map_drawing_information_make_map_scene(
	struct Map_drawing_information *map_drawing_information,
	struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Creates the map_drawing_information scene, if isn't already present.
==============================================================================*/	

struct MANAGER(Spectrum) *get_map_drawing_information_spectrum_manager(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
gets the spectrum_manager of the <map_drawing_information>.
==============================================================================*/

struct LIST(GT_object) *get_map_drawing_information_glyph_list(
	struct Map_drawing_information *map_drawing_information);
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
gets the glyph_list of the <map_drawing_information>.
==============================================================================*/

int map_drawing_information_align_scene(
	struct Map_drawing_information *map_drawing_information); /*FOR AJP*/
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION : Aligns the <package> scene so that the largest cardinal dimension 
component of the scene is the scene viewer's up vector, and the viewer is 
looking along the scene's smallest cardinal dimension component towards the 
centre of the scene. Eg if a scene is 100 by 30 by 150 (in x,y,z)
then we'd look along the y axis, with the z axis as the up vector. 

cf Scene_viewer_view_all.
Should be in Scene_viewer? RC doesn't think so.
==============================================================================*/
#endif /* defined (UNEMAP_USE_3D)*/

#endif /* !defined (MAPPING_H) */
