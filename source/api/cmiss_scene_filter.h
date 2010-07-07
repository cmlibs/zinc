/*******************************************************************************
 * cmiss_scene_filter.h
 * 
 * Public interface to Cmiss_scene_filter objects for filtering graphics
 * displayed in a Cmiss_scene.
 */
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

#ifndef __CMISS_SCENE_FILTER_H__
#define __CMISS_SCENE_FILTER_H__

struct Cmiss_scene_filter;

#ifndef CMISS_SCENE_FILTER_ID_DEFINED
   typedef struct Cmiss_scene_filter *Cmiss_scene_filter_id;
   #define CMISS_SCENE_FILTER_ID_DEFINED
#endif /* CMISS_SCENE_FILTER_ID_DEFINED */

enum Cmiss_scene_filter_action
{
  CMISS_SCENE_FILTER_HIDE = 0,
  CMISS_SCENE_FILTER_SHOW = 1
};

/*******************************************************************************
 * Returns a new reference to the filter with reference count incremented.
 * Caller is responsible for destroying the new reference.
 * 
 * @param filter  The filter to obtain a new reference to.
 * @return  New filter reference with incremented reference count.
 */
Cmiss_scene_filter_id Cmiss_scene_filter_access(Cmiss_scene_filter_id filter);

/*******************************************************************************
 * Destroys this reference to the filter (and sets it to NULL).
 * Internally this just decrements the reference count.
 */
int Cmiss_scene_filter_destroy(Cmiss_scene_filter_id *filter_address);

/*******************************************************************************
 * Sets the action - hide or show - performed when this filter has a match.
 * 
 * @param filter  The filter to modify.
 * @param action  The action to set: hide or show.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_scene_filter_set_action(Cmiss_scene_filter_id filter,
	enum Cmiss_scene_filter_action action);

#endif /*__CMISS_SCENE_FILTER_H__*/
