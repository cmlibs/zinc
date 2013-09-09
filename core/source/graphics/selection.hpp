/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (SELECTION_HPP)
#define SELECTION_HPP

#include "zinc/selection.h"

struct cmzn_selection_event;

struct cmzn_selection_handler;

struct cmzn_selection_handler *cmzn_selection_handler_create_private();

int cmzn_selection_handler_set_scene(cmzn_selection_handler_id selection_handler,
	struct cmzn_scene *scene_in);

int cmzn_selection_handler_scene_destroyed(cmzn_selection_handler_id selection_handler);

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_selection_handler);
#endif /* (SELECTION_HPP) */
