/***************************************************************************//**
 * FILE : cmiss_element_private.hpp
 *
 * Private header file of cmzn_element, finite element meshes.
 *
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
#if !defined (CMISS_ELEMENT_PRIVATE_HPP)
#define CMISS_ELEMENT_PRIVATE_HPP

#include "zinc/element.h"
#include "zinc/region.h"
#include "zinc/fieldsubobjectgroup.h"

struct FE_region;

/***************************************************************************//**
 * Ensures all faces of the supplied element are in this mesh_group.
 * Candidate for external API.
 *
 * @param mesh_group  The mesh group to add faces to. Must be of dimension 1
 * less than that of element and be a subgroup for the master mesh expected to
 * own the element's faces.
 * @param element  The element whose faces are to be added.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int cmzn_mesh_group_add_element_faces(cmzn_mesh_group_id mesh_group,
	cmzn_element_id element);

/***************************************************************************//**
 * Ensures all faces of the supplied element are not in this mesh_group.
 * Candidate for external API.
 *
 * @param mesh_group  The mesh group to remove faces from. Must be of dimension
 * 1 less than that of element and be a subgroup for the master mesh expected
 * to own the element's faces.
 * @param element  The element whose faces are to be removed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int cmzn_mesh_group_remove_element_faces(cmzn_mesh_group_id mesh_group,
	cmzn_element_id element);

/** Internal use only.
 * Create a related element list to that in mesh.
 * @return  New element list.
 */
struct LIST(FE_element) *cmzn_mesh_create_element_list_internal(cmzn_mesh_id mesh);

/** Internal use only
 * @return non-accessed fe_region for this mesh.
 */
FE_region *cmzn_mesh_get_FE_region_internal(cmzn_mesh_id mesh);

/** Internal use only.
 * @return non-accessed region for this mesh.
 */
cmzn_region_id cmzn_mesh_get_region_internal(cmzn_mesh_id mesh);

/** Internal use only.
 * @return non-accessed master region for this mesh.
 */
cmzn_region_id cmzn_mesh_get_master_region_internal(cmzn_mesh_id mesh);

#endif /* !defined (CMISS_ELEMENT_PRIVATE_HPP) */
