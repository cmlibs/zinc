/***************************************************************************//**
 * cmiss_rendition.hpp
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
#if !defined (CMISS_RENDITION_HPP)
#define CMISS_RENDITION_HPP

#include "general/callback_class.hpp"

struct Cmiss_rendition;
class Render_graphics_compile_members;
class Render_graphics_opengl;

int Cmiss_rendition_compile_rendition(Cmiss_rendition *cmiss_rendition,
	Render_graphics_compile_members *renderer);

int Cmiss_rendition_compile_members_rendition(Cmiss_rendition *cmiss_rendition,
	Render_graphics_compile_members *renderer);

int Cmiss_rendition_update_non_distorted_ndc_objects(Cmiss_rendition *rendition,
	Render_graphics_compile_members *renderer);

int execute_Cmiss_rendition(Cmiss_rendition *cmiss_rendition,
	Render_graphics_opengl *renderer);

int Cmiss_rendition_render_opengl(struct Cmiss_rendition *cmiss_rendition,
	Render_graphics_opengl *renderer);

int Cmiss_rendition_render_child_rendition(struct Cmiss_rendition *rendition,
	Render_graphics_opengl *renderer);

#endif /* !defined (CMISS_RENDITION_HPP) */
