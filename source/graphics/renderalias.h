/*******************************************************************************
FILE : renderalias.h

LAST MODIFIED : 19 February 1998

DESCRIPTION :
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
#if !defined (RENDER_ALIAS_H)
#define RENDER_ALIAS_H

/*
Global functions
----------------
*/
#if defined (__cplusplus)
extern "C"  //  Makes this function available to C by not mangling function name
#endif /* defined (__cplusplus) */
int export_to_alias(struct Scene *scene,char *filename);
/******************************************************************************
LAST MODIFIED :  7 January 1998

DESCRIPTION :
Saves the visible objects in an Alias wire file. (Currently only volumes and
surfaces.)
==============================================================================*/

#if defined (__cplusplus)
extern "C"  //  Makes this function available to C by not mangling function name
#endif /* defined (__cplusplus) */
int export_to_alias_frames(struct Scene *scene,char *filename,float frame_in,
	float frame_out, int save_now,int destroy_when_saved);
/******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Saves the visible objects in an Alias wire file. (Currently only volumes and
surfaces.)
If <frame_in> or <frame_out> are non zero then the group node is set
so that the given object is visible only between the frame in and out values.
<save_now> is a flag that indicates that the current Alias world should be
written to the file indicated by filename.  Otherwise the objects are created
but not saved.
<destroy_when_saved> requests that the entire alias universe is destroyed
after it has been saved.
==============================================================================*/

#if defined (__cplusplus)
extern "C"  //  Makes this function available to C by not mangling function name
#endif /* defined (__cplusplus) */
int export_to_alias_sdl( struct Scene *scene, char *filename,
	char *retrieve_filename, float view_frame	);
/******************************************************************************
LAST MODIFIED : 2 February 1998

DESCRIPTION :
Saves all the visible objects in an Alias SDL file ready to be rendered.
All visible objects in the scene are created in the Alias universe.
The camera position and so on can be set by setting a animation
curves using Alias, saving a wire file and supplying that file for the
<retrieve_filename> which is read in, the <view_frame> command is issued to
get to the particular place in the animation curves and then the SDL (without
animation information) is written for that one frame.  The Alias universe is
cleared when the SDL has been written.
==============================================================================*/
#endif /* !defined (RENDER_ALIAS_H) */
