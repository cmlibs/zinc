/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (SELECTION_HPP)
#define SELECTION_HPP

#include "zinc/selection.h"

struct cmzn_selectionevent;

struct cmzn_selectionnotifier;

struct cmzn_selectionnotifier *cmzn_selectionnotifier_create_private();

int cmzn_selectionnotifier_set_scene(cmzn_selectionnotifier_id selectionnotifier,
	struct cmzn_scene *scene_in);

int cmzn_selectionnotifier_scene_destroyed(cmzn_selectionnotifier_id selectionnotifier);

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_selectionnotifier);
#endif /* (SELECTION_HPP) */
