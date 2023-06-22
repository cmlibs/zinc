/* Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * Renders gtObjects to WAVEFRONT file
 */

#pragma once

#include <string>
#include <vector>

struct cmzn_scene;
struct cmzn_scenefilter;

std::vector<std::string> export_to_wavefront(cmzn_scene *scene,
	cmzn_scenefilter *filter, int full_comments);
