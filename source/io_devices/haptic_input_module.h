/*******************************************************************************
FILE : haptic_input_module.h

LAST MODIFIED : 14 May 1998

DESCRIPTION :
Contains all the code needed to handle input from the haptic device.  Sets up
callbacks for whatever users are interested in.  Additionally when the
input from the haptic device has been initiated a scene can be realised in
the haptic environment.
==============================================================================*/
#if !defined (HAPTIC_INPUT_MODULE_H)
#define HAPTIC_INPUT_MODULE_H

struct Scene;

/*
Global functions
----------------
*/

#if defined (HAPTIC)
#if defined (__cplusplus)
extern "C"  //  Makes this function available to C by not mangling function name
#endif /* defined (__cplusplus) */
int input_module_haptic_init(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 10 December 1997

DESCRIPTION :
Sets the haptic device so that any input is recognised, and messages are
received.
==============================================================================*/

#if defined (__cplusplus)
extern "C"  //  Makes this function available to C by not mangling function name
#endif /* defined (__cplusplus) */
int input_module_haptic_close(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Closes the haptic device.
==============================================================================*/

#if defined (__cplusplus)
extern "C"  //  Makes this function available to C by not mangling function name
#endif /* defined (__cplusplus) */
int input_module_haptic_position(struct User_interface *user_interface,
	Input_module_message message);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Gets the position of the haptic device.
==============================================================================*/

#if defined (__cplusplus)
extern "C"  //  Makes this function available to C by not mangling function name
#endif /* defined (__cplusplus) */
void input_module_haptic_set_origin();
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Sets the origin for the haptic device
==============================================================================*/

#if defined (__cplusplus)
extern "C"  //  Makes this function available to C by not mangling function name
#endif /* defined (__cplusplus) */
int haptic_create_scene( struct Scene *scene );
/*******************************************************************************
LAST MODIFIED : 2 March 1998

DESCRIPTION :
Creates the haptic class objects representing the given window scene thereby
representing the current scene in the haptic environment
==============================================================================*/

#if defined (__cplusplus)
extern "C"  //  Makes this function available to C by not mangling function name
#endif /* defined (__cplusplus) */
int haptic_set_surface_defaults( float dynamic_friction, float static_friction,
	float damping, float spring_k );
/*******************************************************************************
LAST MODIFIED : 2 March 1998

DESCRIPTION :
Sets the default force response of surfaces in the model
==============================================================================*/

#if defined (__cplusplus)
extern "C"  //  Makes this function available to C by not mangling function name
#endif /* defined (__cplusplus) */
int haptic_set_scale ( double x, double y, double z);
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Sets the absolute values of the scale of the haptic environment
==============================================================================*/
#endif /* HAPTIC */

#endif /* !defined (HAPTIC_INPUT_MODULE_H) */
