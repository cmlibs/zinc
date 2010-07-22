/*******************************************************************************
FILE : cmiss_rendition.h

LAST MODIFIED : 04 Nov 2009

DESCRIPTION :
The public interface to the Cmiss_rendition.
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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
#ifndef __CMISS_RENDITION_H__
#define __CMISS_RENDITION_H__

#include "api/cmiss_field.h"
#include "api/cmiss_graphic.h"
#include "api/cmiss_region.h"

struct Cmiss_rendition;

#ifndef CMISS_RENDITION_ID_DEFINED
/***************************************************************************//**
 * A handle to a Cmiss rendition. 
 * Cmiss rendition is the graphical represnetation of a region, each region
 * has maximum one rendition at a time. Rendition is created when requested.
 */
typedef struct Cmiss_rendition * Cmiss_rendition_id;
#define CMISS_RENDITION_ID_DEFINED
#endif /* CMISS_RENDITION_ID_DEFINED */

/*******************************************************************************
 * Returns a new reference to the rendition with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param rendition  The rendition to obtain a new reference to.
 * @return  New rendition reference with incremented reference count.
 */
Cmiss_rendition_id Cmiss_rendition_access(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Adds the <graphic> to <rendition> at the given <position>, where 1 is
 * the top of the list (rendered first), and values less than 1 or greater than the
 * last position in the list cause the graphic to be added at its end, with a
 * position one greater than the last.
 *
 * @param rendition  The handle to the rendition.
 * @param graphic  The handle to the cmiss graphic which will be added to the
 *   rendition.
 * @param pos  The position to put the target graphic to.
 * @return  Returns 1 if successfully add graphic to rendition at pos, otherwise
 *   returns 0.
 */
int Cmiss_rendition_add_graphic(Cmiss_rendition_id rendition, Cmiss_graphic_id graphic, int pos);

/***************************************************************************//**
 * Get graphic at position <pos> in the internal graphics list of rendition.
 *
 * @param rendition  The handle to the rendition of which the graphic is located.
 * @param pos  The position of the graphic, starting from 0.
 * @return  returns the found graphic if found at pos, otherwise returns NULL.
 */
Cmiss_graphic_id Cmiss_rendition_get_graphic_at_position(
	Cmiss_rendition_id rendition, int pos);

/***************************************************************************//**
 * Returns the number of graphics in <rendition>.
 *
 * @param rendition  The handle to the rendition
 * @return  Returns the number of graphic in rendition.
 */
int Cmiss_rendition_get_number_of_graphic(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Removes the <graphic> from <rendition> and decrements the position
 * of all subsequent graphics.
 *
 * @param rendition  The handle to the rendition of which the graphic is removed
 *   from.
 * @param graphic  The handle to a cmiss graphic object which will be removed
 *   from the rendition.
 * @return  Returns 1 if successfully remove graphic from rendition, otherwise 0.
 */
int Cmiss_rendition_remove_graphic(Cmiss_rendition_id rendition,
	Cmiss_graphic_id graphic);

int Cmiss_rendition_destroy(Cmiss_rendition_id * rendition);

/***************************************************************************//**
 * Use this function with Cmiss_rendition_end_change.
 *
 * Use this function before making multiple changes on the rendition, this 
 * will stop rendition from executing any immediate changes made in 
 * rendition. After multiple changes have been made, use
 * Cmiss_rendition_end_change to execute all changes made previously in rendition
 * at once.
 *
 * @param rendition  The handle to the rendition.
 * @return  Returns 1 if successfully cache the rendition, otherwise 0.
 */
int Cmiss_rendition_begin_change(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Use this function with Cmiss_rendition_begin_change.
 *
 * Use Cmiss_rendition_begin_change before making multiple changes on the 
 * rendition, it will stop rendition from executing any immediate changes made in 
 * rendition. After multiple changes have been made, use
 * this function to execute all changes made previously in rendition
 * at once.
 *
 * @param rendition  The handle to the rendition.
 * @return  Returns 1 if successfully end the cache on the rendition, otherwise 0.
 */
int Cmiss_rendition_end_change(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Returns the default coordinate field of the <rendition>.
 *
 * @param rendition  The handle to cmiss rendition.
 * @return  Returns the default coordinate field of rendition if found,
 * otherwise NULL.
 */
Cmiss_field_id Cmiss_rendition_get_default_coordinate_field(
 	Cmiss_rendition_id renditon);

/***************************************************************************//**
 * Returns the status of the rendition's visibility flag. Note this only affects
 * scene filters that act on the state of this flag.
 *
 * @param rendition  The handle to the rendition.
 * @return  1 for visible, 0 for not visible.  
 */
int Cmiss_rendition_get_visibility_flag(struct Cmiss_rendition *rendition);

/***************************************************************************//**
 * Check either region has rendition or not.
 *
 * @param region  The handle to cmiss_region.
 * @return  Returns 1 if rendition is found in region, otherwise 0.
 */
int Cmiss_region_has_rendition(Cmiss_region_id cmiss_region);

/***************************************************************************//**
 * Create a line representation for rendition. This graphical representation can
 * be modified through Cmiss_graphic_set functions.
 *
 * @param rendition  The handle to the rendition which the lines are created for.
 * @return  If successfully create lines for rendition returns the handle to the
 *   created graphic, otherwise NULL.
 */
Cmiss_graphic_id Cmiss_rendition_create_lines(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Create a surface representation for rendition. This graphical representation 
 * can be modified through Cmiss_graphic_set functions.
 *
 * @param rendition  The handle to the rendition which the surface are created 
 *   for.
 * @return  If successfully create surface for rendition returns the handle to 
 *   the created graphic, otherwise NULL.
 */
Cmiss_graphic_id Cmiss_rendition_create_surfaces(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Create a node point representation for rendition. This graphical 
 * representation can be modified through Cmiss_graphic_set functions.
 *
 * @param rendition  The handle to the rendition which the node point are 
 *   created for.
 * @return  If successfully create node points for rendition returns the handle
 *   to the created graphic, otherwise NULL.
 */
Cmiss_graphic_id Cmiss_rendition_create_node_points(Cmiss_rendition_id rendition);

/***************************************************************************//**
 * Create a static graphics representation for rendition. This graphical 
 * representation can be modified through Cmiss_graphic_set functions.
 * This graphic type is different from others, as the graphics object of this is
 * created by user instead of generated from finite element models, it does not 
 * require a coordinate field in the rendition. To get an idea of what graphics 
 * objects are, take a look at the glyphs used in points representation  
 * they are a set of preset graphics object in cmgui. 
 * More functions will be created to support this graphics type.
 *
 * @warning  This function is under development and may subject to change.
 * @param rendition  The handle to the rendition which the static graphics are
 *   object created for.
 * @return  If successfully create static graphics objects for rendition returns 
 *   the handle to the created graphic, otherwise NULL.
 */
Cmiss_graphic_id Cmiss_rendition_create_static(Cmiss_rendition_id rendition);

#endif /* __CMISS_RENDITION_H__ */
