/*******************************************************************************
FILE : volume_texture_editor.h

LAST MODIFIED : 3 December 2001

DESCRIPTION :
==============================================================================*/
#if !defined (VOLUME_TEXTURE_EDITOR_H)
#define VOLUME_TEXTURE_EDITOR_H

#define SLIDERMAX 100.0
#include <Mrm/MrmPublic.h>
#include "graphics/environment_map.h"
#include "graphics/material.h"
#include "graphics/volume_texture.h"
#include "graphics/volume_texture_editor_dialog.h"
#include "graphics/graphics_object.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
struct Texture_window
/*******************************************************************************
LAST MODIFIED : 6 janurary 1998

DESCRIPTION :
==============================================================================*/
{
	struct User_interface *user_interface;
	struct MANAGER(Environment_map) *environment_map_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct MANAGER(Texture) *texture_manager;
	struct Graphical_material *default_graphical_material;
	struct Create_finite_elements_dialog *create_finite_elements_dialog;
	/* shell for the graphical material palette */
	Widget graphical_material_palette_shell;
	/* shell for the env map palette */
	Widget envmap_palette_shell;
	/* popup shell for the texture window */
	Widget texture_window_shell;
	/* top widget of the texture window */
	Widget texture_window_widget;
	/* 3d panel where materials and env maps are displayed */
	Widget select_3d_widget;
	/* widget reference info */
	Widget ximin_tf[3],        /* tf1,5,9 */
		ximax_tf[3],    /* tf3,7,11 */
		xival_tf[3],    /* tf2,6,10 */
		xires_tf[3],    /* tf4,8,12 */
		xi_sb[3],    /* sb1,2,3 */
		tb[20],      /* pulldown toggles */
		material_val_tf,      /* tf13 */
		material_sb,    /* sb4 */
		displacement_val_tf[3],  /* tf14,15,16 */
		select_val_tf,    /* tf17 */
		cutting_plane_sb[4],   /* sb5,6,7,8 */
		rotate_sb[3],    /* sb9,10,11 */
		zoom_sb,    /* sb12 */
		res_sb,      /* sb13 */
		isovalue_sb,    /* sb14 */
		prompt_text_field, /* tf0 */
		vt_3d_select_form,  /* for material selection */
		node_group_list;
		/* workproc for graphics inputs */
		XtWorkProcId workproc_id;
	/* node or cell based editing */
	char current_node_group[100];
	int pick_mode;
	int fill_mode;
	int cell_mode;
	int paint_mode;
	int delete_paint_mode;
	int delete_mode;
	int auto_on;
	int cubes_on;
	int isosurface_on;
	int grid_on;
	int see_paint_on;
	int closed_surface;
	int shaded_surfaces;
	int wireframe;
	int normals;
	int cutting_plane_on;
	int line_mode_on;
	int curve_mode_on;
	int blob_mode_on;
	int soft_mode_on;
	int hollow_mode_on;
	int cop_mode_on;
	int env_mode_on;
	int decimation;
	int detail_mode;
	int set_input_limits;
	int input_move_model;
	int irregular_mesh_request;
	int edit_group_mode;
	/* slit mode[0] = on/off, [1] = xi1 [2] = xi2 [3] = xi3 */
	int slit_mode[4];
	/* GL structures */
	int current_axis;   /* 0 xy 1 yz 2 zx */
	double mouse_x,mouse_y;
	double mouse_xi1,mouse_xi2,mouse_xi3;
	double mouse_x_rel,mouse_y_rel;
	double mouse_scale;
	Widget graphics_window,graphics_window_shell;
#if defined (OPENGL_API)
	GLuint tw_axes;
	GLuint tw_grid;
	GLuint tw_cube;
	GLuint tw_wire_cube;
	GLuint tw_texture;
	GLuint tw_mouseplane;
	GLuint tw_sphere;
	GLuint tw_wiresphere;
	GLuint tw_small_sphere;
	GLuint tw_env_map_sphere;
	GLuint tw_box;
	GLuint tw_3dtexture;
	GLuint tw_isosurface;
	GLuint tw_nodes;
	GLuint tw_lines;
	GLuint tw_curves;
	GLuint tw_blobs;
	GLuint tw_softs;
	GLuint tw_cop;
	GLuint tw_envsquare;
	GLuint tw_cube_tex[6];
	GLuint raster_font_base;
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
	int tw_axes ;
	int tw_grid ;
	int tw_cube ;
	int tw_wire_cube ;
	int tw_texture;
	int tw_mouseplane;
	int tw_sphere;
	int tw_small_sphere;
	int tw_env_map_sphere;
	int tw_box;
	int tw_3dtexture;
	int tw_isosurface;
	int tw_nodes;
	int tw_lines;
	int tw_curves;
	int tw_blobs;
	int tw_softs;
	int tw_cop;
	int tw_envsquare;
	int tw_cube_tex[6];
#endif /* defined (GL_API) */
	/* side length of voxel brush */
	float brushsize[3];
	/* view parameters */
	double r,theta,phi,twist;
	double fovy;
	double front_plane,back_plane;
	double drawing_scale;
	double rot_angle[3];
#if defined (OPENGL_API)
	float model_matrix[16];
	GLdouble projection_matrix[16];
	GLdouble modelview_matrix[16];
	GLint viewport[4];
#endif /* defined (OPENGL_API) */
#if defined (GL_API)
	float model_matrix[4][4];
#endif /* defined (GL_API) */
	/* 3D texture data structures */
	struct VT_volume_texture *current_texture;
	/* when the index of the edit_curve is non zero, it is active */
	struct VT_texture_curve edit_curve;
	/* input */
	double  ximin[3],
		ximax[3],
		xival[3],
		xires[3];
	double cop[3];
	double  cutting_plane[4],
		xirotate[3],
		zoom,
		resolution,
		isovalue;
	double   displacement[3];
	double select_value, select_value2;
	Widget *material_editor_address;
	struct Graphical_material *current_material;
	struct Environment_map *current_env_map;
	struct GT_voltex *voltex;
}; /* struct Texture_window */

/*
Global variables
----------------
*/
extern struct Light *vt_ed_light;
extern struct Light_model *vt_ed_light_model;

/*
Global functions
----------------
*/
void adjustxi(struct Texture_window *texture_window,int index);
/*******************************************************************************
LAST MODIFIED : 31 August 1996

DESCRIPTION :
==============================================================================*/

void printVT(struct VT_volume_texture *texture);
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Debug
==============================================================================*/

struct Texture_window *create_texture_edit_window(
	struct Graphical_material *default_graphical_material,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(Environment_map) *environment_map_manager,
	struct MANAGER(Texture) *texture_manager,Widget *material_editor_address,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Create the structures and retrieve the texture window from the uil file.
==============================================================================*/

void adjust_material_sb(struct Texture_window *texture_window);
/*******************************************************************************
LAST MODIFIED : 28 December 1995

DESCRIPTION :
???DB.  Is this needed ?
==============================================================================*/

void select_line(struct Texture_window *tw);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Here the 1D line texture structure is invoked - if no line is currently being
edited (edit_line.index = 0) the first point is selected, and a line is drawn to
the current node cursor position. If a line is being edited then the second
point is recorded, the curve is complete and stored in the volume_texturedata
structure. edit_curve.index is reset to zero.
==============================================================================*/

void select_blob(struct Texture_window *tw);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Here the 1D blob texture structure is invoked - if no line is currently being
edited (edit_line.index = 0) the first point is selected, and a line is drawn to
the current node cursor position. If a line is being edited then the second
point is recorded, the curve is complete and stored in the volume_texture data
structure. edit_curve.index is reset to zero.
==============================================================================*/

void select_curve(struct Texture_window *tw,int next);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
If next = 0, this just updates the appropriate point, otherwise it stores it and
if index = 3 the line is complete and it is stored in the list. The points are
p1,p2 and the slope control points p3,p4.
==============================================================================*/

void deselect_curve(struct Texture_window *tw);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Two points are selected and the coordinates stored in edit_line.  After two
deselect operations have been performed, the list is searched for the line
segment and it is removed.
==============================================================================*/

void select_soft(struct Texture_window *tw);
/*******************************************************************************
LAST MODIFIED : 30 August 1996

DESCRIPTION :
Here the 1D soft texture structure is invoked - if no line is currently being
edited (edit_line.index = 0) the first point is selected, and a line is drawn to
the current node cursor position. If a line is being edited then the second
point is recorded, the curve is complete and stored in the volume_texture data
structure. edit_curve.index is reset to zero.
==============================================================================*/
#endif /* !defined (VOLUME_TEXTURE_EDITOR_H) */
