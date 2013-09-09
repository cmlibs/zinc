/*******************************************************************************
FILE : computed_field_scene_viewer_projection.h

LAST MODIFIED : 4 July 2000

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_SCENE_VIEWER_PROJECTION_H)
#define COMPUTED_FIELD_SCENE_VIEWER_PROJECTION_H

#include "zinc/types/fieldid.h"
#include "zinc/types/fieldmoduleid.h"
#include "zinc/types/sceneviewerid.h"
#include "zinc/types/scenecoordinatesystem.h"

int Computed_field_register_types_scene_viewer_projection(
	struct Computed_field_package *computed_field_package,
	struct MANAGER(Graphics_window) *graphics_window_manager);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_SCENE_VIEWER_PROJECTION_H) */
