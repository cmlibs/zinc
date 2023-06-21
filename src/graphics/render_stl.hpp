/* Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * Renders gtObjects to STL stereolithography file.
 */

#pragma once

#include <string>

struct cmzn_scene;
struct cmzn_scenefilter;

/**
 * Renders the visible objects to a string in STL format.
 * 
 * @param scene The scene to output
 * @param filter The filter on scene
 * @return @c std::string in STL format on success, an empty string on failure
 */
std::string export_to_stl(cmzn_scene *scene, cmzn_scenefilter *filter);
