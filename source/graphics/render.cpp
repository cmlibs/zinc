/***************************************************************************//**
 * render.cpp
 * Rendering calls - Non API specific.
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#include <stdio.h>
#include <math.h>
extern "C" {
#include "general/debug.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_library.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/light_model.h"
#include "graphics/mcubes.h"
#include "graphics/spectrum.h"
#include "graphics/tile_graphics_objects.h"
#include "user_interface/message.h"
}
#include "graphics/graphical_element.hpp"
#include "graphics/graphics_object_private.hpp"
#include "graphics/scene.hpp"
#include "graphics/material.hpp"
#include "graphics/render.hpp"

/****************** Render_graphics_compile_members **********************/

int Render_graphics_compile_members::Scene_compile(Scene *scene)
{
	return Scene_compile_members(scene, this);
}

int Render_graphics_compile_members::Scene_object_compile(Scene_object *scene_object)
{
	return Scene_object_compile_members(scene_object, this);
}

int Render_graphics_compile_members::Graphical_element_group_compile(
	GT_element_group *graphical_element_group)
{
	return Graphical_element_group_compile_members(graphical_element_group, this);
}

