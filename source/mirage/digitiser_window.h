/*******************************************************************************
FILE : digitiser_window.h

LAST MODIFIED : 21 June 1999

DESCRIPTION :
Routines for generating and managing digitiser_windows. The digitiser window
utilises a Scene_viewer for display of a 3-D Scene with a 2-D Texture shown in
the background. A CUSTOM projection and ABSOLUTE viewport mode are used to
align the 3-D view with the texture image it is digitised from.
==============================================================================*/
#if !defined (DIGITISER_WINDOW_H)
#define DIGITISER_WINDOW_H

#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>

#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "mirage/movie.h"
#include "user_interface/user_interface.h"

/*
Global/Public types
-------------------
*/
enum Digitiser_window_buffer_mode
{
	DIGITISER_WINDOW_SINGLE_BUFFER,
	DIGITISER_WINDOW_DOUBLE_BUFFER
};

struct Digitiser_window;
/*******************************************************************************
LAST MODIFIED : 10 December 1997

DESCRIPTION :
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Digitiser_window);

DECLARE_MANAGER_TYPES(Digitiser_window);

/*
Global/Public functions
-----------------------
*/
struct Digitiser_window *CREATE(Digitiser_window)(char *name,
	struct Mirage_movie *movie,int view_no,
	enum Digitiser_window_buffer_mode buffer_mode,
	struct Colour *background_colour,
	struct MANAGER(Light) *light_manager,
	struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION:
Creates a Digitiser_window object, window shell and widgets. Returns a pointer
to the newly created object. The Digitiser_window maintains a pointer to the
manager it is to live in, since users will want to close windows with the
window manager widgets.
Each window has a unique <name> that can be used to identify it, and which
will be printed on the windows title bar.
==============================================================================*/

int DESTROY(Digitiser_window)(
	struct Digitiser_window **digitiser_window_address);
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION:
Frees the contents of the Digitiser_window structure and then the object itself,
then closes down the window shell and widgets it uses. Note that responsibility
for removing the digitiser_window from a global list of windows is left with the
calling routine. See also Digitiser_window_close_CB and
Digitiser_window_destroy_CB.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Digitiser_window);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Digitiser_window);

PROTOTYPE_LIST_FUNCTIONS(Digitiser_window);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Digitiser_window,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Digitiser_window,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Digitiser_window);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Digitiser_window,name,char *);

char *Digitiser_window_manager_get_new_name(
	struct MANAGER(Digitiser_window) *digitiser_window_manager);
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Makes up a default name string for a graphics window, based on numbers and
starting at "1". Up to the calling routine to deallocate the returned string.
==============================================================================*/

int Digitiser_window_add_light(struct Digitiser_window *digitiser_window,
	void *light_void);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Iterator function for adding a light to <digitiser_window>.
==============================================================================*/

int Digitiser_window_remove_light(struct Digitiser_window *digitiser_window,
	void *light_void);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Iterator function for removing a light from the <digitiser_window>.
==============================================================================*/

struct Light_model *Digitiser_window_get_light_model(
	struct Digitiser_window *digitiser_window);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Returns the light_model from the <digitiser_window>.
==============================================================================*/

int Digitiser_window_set_light_model(struct Digitiser_window *digitiser_window,
	void *light_model_void);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Iterator function for setting the light_model for the <digitiser_window>.
==============================================================================*/

int Digitiser_window_update(struct Digitiser_window *digitiser_window);
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
Iterator function for forcing a redraw on <digitiser_window> at the next
idle moment.
==============================================================================*/

int Digitiser_window_update_now(struct Digitiser_window *digitiser_window);
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
Iterator function for forcing a redraw on <digitiser_window>.
==============================================================================*/

int Digitiser_window_update_view(struct Digitiser_window *digitiser_window,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Iterator function for forcing a redraw on <digitiser_window> AFTER ensuring the
current view is properly re-established - must call this for all digitiser
windows after changing or re-reading frames.
==============================================================================*/
#endif /* !defined (DIGITISER_WINDOW_H) */
