/*******************************************************************************
FILE : movie.h

LAST MODIFIED : 28 March 2000

DESCRIPTION :
The data types and function prototypes used for digitizing Mirage movies.

How it works
------------
Alternatives
a) There are <number_of_views>+1 coordinate fields - the 3D field and a field
	for each view.
		???DB.  Trouble is that you won't know which has been changed and so can't
			keep in sync
b) One coordinate field - 3D
		???DB.  Trouble is that its difficult to do showing and hiding of nodes and
			its dificult to do the combining of projections
c) One coordinate field (3D) plus information stored in Mirage structures
==============================================================================*/
#if !defined (MIRAGE_MOVIE_H)
#define MIRAGE_MOVIE_H
#include "finite_element/finite_element.h"
#include "graphics/graphics_object.h"
#include "graphics/light.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "mirage/tracking_editor_data.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
???DB.  Not sure how much of this needs to be global ?
*/
struct Mirage_view
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
Stores information associated with a view.  A view corresponds to looking from a
single eye point.
==============================================================================*/
{
	/* The name of the view as a string - eg. "1" */
	char *name;
	/* The following string contains the path, name and extension of the image
		files for this view, with contiguous # characters denoting where the
		frame number is to be placed to complete the file name
		eg. /images/imafish.#####.view1.rgb */
	char *image_file_name_template;
	/* Image position and size may differ in even and odd frames due to
		interlacing. Hence, specify crop size and placement for even and odd frames
		separately. Note that specifying a zero crop width or height gives the
		whole image. */
	/* distortion correction parameters for the final coordinates */
	double dist_centre_x,dist_centre_y,dist_factor_k1;
	/* cropping information for images in even frames*/
	int crop0_left,crop0_bottom,crop0_width,crop0_height;
	/* cropping information for images in odd frames*/
	int crop1_left,crop1_bottom,crop1_width,crop1_height;
	/* size and where to put cropped images in even frame numbers */
	double image0_left,image0_bottom,image0_width,image0_height;
	/* size and where to put cropped images in odd frame numbers */
	double image1_left,image1_bottom,image1_width,image1_height;
	/* These parameters describe the area the projection matrix projects on to */
	double NDC_left,NDC_bottom,NDC_width,NDC_height;
	/* View transformation consists of the near and far clipping plane and the */
	/* 4 rows x 3 columns photogrammetry transformation matrix */
	double near_clipping_plane,far_clipping_plane,transformation43[12];
	/* Specify the nodes that can be digitized in this view and the order in which
		they should be digitized.  The nodes are specified with numbers rather than
		pointers to the nodes because the nodes may not always exist */
	int *node_numbers,number_of_nodes;
	/* Centre of mass radius used for each node in view (same order as above) */
	int *com_radius;
	/* Colour indices for dots in the centre-of-mass calculations. For the moment,
		0=black and 1=white. Later index into a look-up table for various RGB
		colours */
	int *com_colour_indices;
	struct LIST(Node_status) *placed_list;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct MANAGER(Scene) *scene_manager;
	struct MANAGER(Texture) *texture_manager;
	struct GROUP(FE_element) *pending_elements,*placed_elements,*problem_elements;
	struct GROUP(FE_node) *pending_nodes,*placed_nodes,*problem_nodes;
	struct Scene *scene;
	struct Texture *texture;
}; /* struct Mirage_view */

enum Mirage_movie_modifiers
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Modifiers to the node_changed response in the Tracking_editor_dialog.
These are intended to be binary flags so they can be |ed together.
==============================================================================*/
{
	MIRAGE_MOVIE_MODIFIERS_NONE = 0,
	MIRAGE_MOVIE_MODIFIERS_TOGGLE_SELECT = 1
};

struct Mirage_movie
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Stores information for moving through and digitizing the movie.
==============================================================================*/
{
	char *name;
	char *working_directory_name;
	/* information about the base node and element groups and files */
	char *all_node_file_name,*all_element_file_name,*all_node_group_name;
	/* frames */
	int current_frame_no,number_of_frames,start_frame_no;
	/* total number of nodes to track (not used at present) */
	int total_nodes;
	/* the following string contains the path, name and extension of the node */
	/* files for this movie, with contiguous # characters denoting where the */
	/* frame number is to be placed to complete the file name */
	/* eg. /nodes/imafish.#####.exnode */
	char *node_file_name_template;
	/*struct Mirage_frame *frames;*/
	/* views */
	int number_of_views;
	struct LIST(GT_object) *glyph_list;
	struct LIST(Node_status) *placed_list,*pending_list,*problem_list;
	struct Mirage_view **views;
	struct Computed_field_package *computed_field_package;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct MANAGER(FE_node) *data_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
	struct MANAGER(Scene) *scene_manager;
	struct GROUP(FE_element) *all_element_group,*pending_elements_3d,
		*placed_elements_3d,*problem_elements_3d;
	struct GROUP(FE_node) *all_node_group,*pending_nodes_3d,*placed_nodes_3d,
		*problem_nodes_3d;
	struct Graphical_material *placed_points_material,*pending_points_material,
		*problem_points_material,*lines_2d_material,*lines_3d_material,
		*surfaces_2d_material,*surfaces_3d_material;
	struct Scene *scene;
	struct Mirage_node_editor *node_editor;
	/* global centre of mass filter radii and threshold for passes 1 and 2 */
	int com_radius1,com_radius2;
	double com_threshold1,com_threshold2;
	/* SAB Added to allow modifiers to the node_changed response in the 
		Tracking_editor_dialog, specifically allowing the digitiser window to
		make nodes not pending */
	enum Mirage_movie_modifiers modifiers;
}; /* struct Mirage_movie */

struct Add_elements_with_node_data
{
	int max_dimension;
	struct FE_node *node;
	struct GROUP(FE_node) *node_group;
	struct GROUP(FE_element) *element_group;
};

struct Remove_elements_with_node_data
{
	struct FE_node *node;
	struct GROUP(FE_element) *element_group;
};

/*
Global functions
----------------
*/
struct Mirage_movie *CREATE(Mirage_movie)(void);
/*******************************************************************************
LAST MODIFIED : 3 February 1998

DESCRIPTION :
Allocates space for and initializes the Mirage_movie structure.
==============================================================================*/

int DESTROY(Mirage_movie)(struct Mirage_movie **mirage_movie_ptr);
/*******************************************************************************
LAST MODIFIED : 3 February 1998

DESCRIPTION :
Cleans up space used by Mirage_movie structure.
==============================================================================*/

char *make_Mirage_file_name(char *template,int number);
/*******************************************************************************
LAST MODIFIED : 4 February 1998

DESCRIPTION :
Copies the template string and replaces the characters from the first to the
last # with the <number> as text with leading zeros.
eg. "imafish.#####.v1.rgb" with number 53 gives: "imafish.00053.v1.rgb".
Note: number of # characters must be less than 100!
==============================================================================*/

struct Mirage_movie *read_Mirage_movie(char *file_name);
/*******************************************************************************
LAST MODIFIED : 4 February 1998

DESCRIPTION :
Creates and fills a Mirage movie structure from the file <file_name>. The movie
extension .cmmov is automatically added to the filename.
==============================================================================*/

int write_Mirage_movie(struct Mirage_movie *movie,char *extra_extension);
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :
Writes mirage_movie to its file name with the extra_extension added on the end.
If the extra_extension is the empty string "" it first writes the movie to
the movie's file name with the string "_tmp" appended on the end,
then it uses a sys "cp" command to overwrite the actual file.
If extra_extension is not an empty string, the file is written normally.
==============================================================================*/

int Mirage_movie_read_node_status_lists(struct Mirage_movie *movie,
	char *extra_extension);
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Clears and reads the node_status_lists for <movie>. The filename for each list
is the movie name with a predefined extension added, eg. "_pending" for the
pending list and "_view1" for a view named "1". The <extra_extension> is added
after that to enable temporary saves of these lists to be restored as an undo
function - use "" for normal reads.
==============================================================================*/

int Mirage_movie_write_node_status_lists(struct Mirage_movie *movie,
	char *extra_extension);
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Writes the node_status_lists for <movie>. The filename for each list
is the movie name with a predefined extension added, eg. "_pending" for the
pending list and "_view1" for a view named "1". The <extra_extension> is added
after that to enable temporary saves of these lists to be later restored as an
undo function.
==============================================================================*/

int enable_Mirage_movie_graphics(struct Mirage_movie *movie,
	struct MANAGER(FE_basis) *basis_manager,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_graphical_material,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct MANAGER(Scene) *scene_manager,
	struct Scene *default_scene,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION :
From an already-created movie - eg. read in from read_Mirage_movie - creates
the node and element groups, textures and scenes for each view and for the
resulting 3-D display.
==============================================================================*/

int Mirage_movie_graphics_show_2d_points(struct Mirage_movie *movie,
	int show_node_numbers);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turn on placed, pending and problem points on the 2-D views. Regenerates the
GFE graphics and updates the 3-D scenes. The other option allows node numbers to
be displayed with the placed points. Node numbers are never shown with the
pending points, but always with the problem points.
==============================================================================*/

int Mirage_movie_graphics_hide_2d_points(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turn on placed, pending and problem points on the 3-D view. Regenerates the
GFE graphics and updates the 3-D scene. The other option allows node numbers to
be displayed with the placed points. Node numbers are never shown with the
pending points, but always with the problem points.
==============================================================================*/

int Mirage_movie_graphics_show_2d_lines(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns on lines for the placed element GFEs in the 2-D views.
==============================================================================*/

int Mirage_movie_graphics_hide_2d_lines(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns off lines for the placed element GFEs in the 2-D views.
==============================================================================*/

int Mirage_movie_graphics_show_2d_surfaces(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns on surfaces for the placed element GFEs in the 2-D views.
==============================================================================*/

int Mirage_movie_graphics_hide_2d_surfaces(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns off surfaces for the placed element GFEs in the 2-D views.
==============================================================================*/

int Mirage_movie_graphics_show_3d_points(struct Mirage_movie *movie,
	int show_node_numbers);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turn on placed, pending and problem points on the 3-D view. Regenerates the
GFE graphics and updates the 3-D scene. The other option allows node numbers to
be displayed with the placed points. Node numbers are never shown with the
pending points, but always with the problem points.
==============================================================================*/

int Mirage_movie_graphics_hide_3d_points(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turn on placed, pending and problem points on the 3-D view. Regenerates the
GFE graphics and updates the 3-D scene. The other option allows node numbers to
be displayed with the placed points. Node numbers are never shown with the
pending points, but always with the problem points.
==============================================================================*/

int Mirage_movie_graphics_show_3d_lines(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns on lines for the placed elements in the 3-D view, updates scene.
==============================================================================*/

int Mirage_movie_graphics_hide_3d_lines(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns off lines for the placed elements in the 3-D view, updates scene.
==============================================================================*/

int Mirage_movie_graphics_show_3d_surfaces(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns on surfaces for the placed elements in the 3-D view, updates scene.
==============================================================================*/

int Mirage_movie_graphics_hide_3d_surfaces(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Turns off surfaces for the placed elements in the 3-D view, updates scene.
==============================================================================*/

int read_Mirage_movie_frame(struct Mirage_movie *movie,
	int frame_no);
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Reads the images and nodes for frame <frame_no> in <movie>.
This will become the new current_frame_no of the movie.
==============================================================================*/

int Mirage_movie_frame_nodes_exist_or_error(struct Mirage_movie *movie,
	int frame_no);
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :
Returns true if there exists a node file for <frame_no> or there is an error.
If this routine returns 0, you know there are no nodes for frame <frame_no>,
useful for creating initial meshes.
==============================================================================*/

int Mirage_movie_read_frame_nodes(struct Mirage_movie *movie,
	int frame_no);
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Reads the nodes for frame <frame_no> in <mirage_movie>.
The frame_no does not have to be the current_frame_number of the movie although
it is potentially damaging if it is not.
==============================================================================*/

int Mirage_movie_write_frame_nodes(struct Mirage_movie *movie,
	int frame_no);
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Writes the nodes in memory to the file for frame <frame_no> of <movie>.
The frame_no does not have to be the current_frame_number of the movie although
it is dangerous (but possibly useful) if it is not.
==============================================================================*/

int Mirage_movie_full_save(struct Mirage_movie *movie,char *extra_extension);
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :
Writes the movie file and node status lists with names based on the movie->name
and the <extra_extension> added on the end. The current node file is also
written - but with its usual name.
==============================================================================*/

int Mirage_movie_refresh_node_groups(struct Mirage_movie *movie);
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Refreshes the placed, pending and problem node groups so that they match the
entries in the Node_status_lists for the current_frame_no of the movie.
Should be called after reading and changing frames.
==============================================================================*/

int add_elements_with_node_to_group(struct FE_element *element,
	void *add_data_void);
/*******************************************************************************
LAST MODIFIED : 25 February 1998

DESCRIPTION :
==============================================================================*/

int remove_elements_with_node_from_group(struct FE_element *element,
	void *rem_data_void);
/*******************************************************************************
LAST MODIFIED : 25 February 1998

DESCRIPTION :
==============================================================================*/
#endif /* !defined (MIRAGE_MOVIE_H) */
