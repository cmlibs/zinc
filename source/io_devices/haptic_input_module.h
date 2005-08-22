/*******************************************************************************
FILE : haptic_input_module.h

LAST MODIFIED : 14 May 1998

DESCRIPTION :
Contains all the code needed to handle input from the haptic device.  Sets up
callbacks for whatever users are interested in.  Additionally when the
input from the haptic device has been initiated a scene can be realised in
the haptic environment.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
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
