/***************************************************************************//**
 * FILE : cmiss_element_private.hpp
 *
 * Private header file of cmzn_element, finite element meshes.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (CMZN_ELEMENT_PRIVATE_HPP)
#define CMZN_ELEMENT_PRIVATE_HPP

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
 * @return  Status CMZN_OK on success, any other value on failure.
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
 * @return  Status CMZN_OK on success, any other value on failure.
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

#endif /* !defined (CMZN_ELEMENT_PRIVATE_HPP) */
