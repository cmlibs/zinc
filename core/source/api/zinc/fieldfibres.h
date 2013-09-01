/***************************************************************************//**
 * FILE : cmiss_field_scene_viewer_projection.h
 *
 * A field which extract a transformation matrix from a scene viewer for use in
 * field expressions.
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#ifndef CMZN_FIELDFIBRES_H__
#define CMZN_FIELDFIBRES_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a "fibre axes" field type which returns a 9-component (3 x 3 vector)
 * field representing an orthonormal coordinate system which is rotated by
 * 3 Euler angles supplied by a fibre field. Three resulting 3 axes are:
 * fibre  = fibre direction
 * sheet  = fibre normal in the plane of the sheet
 * normal = normal to the fibre sheet
 * Both the fibre and coordinate fields must have no more than 3 components. The
 * fibre field is expected to have a FIBRE coordinate_system, although this is
 * not enforced.
 * Note that this initial orientation of the coordinate system (i.e. for all
 * Euler angles zero) is right handed coordinate system:
 * fibre axis = aligned with d(coordinates)/dxi1
 * sheet axis = in plane of fibre axis and d(coordinates)/dxi2 but corrected to
 * be closest vector to d(coordinates)/dxi2 which is normal to fibre axes.
 * normal axis = cross product fibre (x) sheet
 *
 * @param field_module  Region field module which will own new field.
 * @param fibre_field  The (numerical) fibre field with at most 3 components
 * @param coordinate_field  The (numerical) coordinate system with at most 3 components
 * @return  Fibre field with 9 components.
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_fibre_axes(
	Cmiss_field_module_id field_module,
	Cmiss_field_id fibre_field, Cmiss_field_id coordinate_field);

#ifdef __cplusplus
}
#endif

#endif
