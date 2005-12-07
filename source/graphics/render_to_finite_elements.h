/*******************************************************************************
FILE : render_to_finite_elements.c

LAST MODIFIED : 5 January 1998

DESCRIPTION :
Renders gtObjects to VRML file
==============================================================================*/
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
#if !defined (RENDER_TO_FINITE_ELEMENTS_H)
#define RENDER_TO_FINITE_ELEMENTS_H

#include "general/enumerator.h"

/*
Global types
------------
*/

enum Render_to_finite_elements_mode
/******************************************************************************
LAST MODIFIED : 7 December 2005

DESCRIPTION :
Renders the visible objects as finite elements into the specified <fe_region>.
==============================================================================*/
{
	/* Renders the scene as square linear product elements.  A collapsed linear
		product element will be used to represent a triangle. */
	RENDER_TO_FINITE_ELEMENTS_LINEAR_PRODUCT,
	RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD
};

PROTOTYPE_ENUMERATOR_FUNCTIONS(Render_to_finite_elements_mode);

/*
Global functions
----------------
*/

int render_to_finite_elements(struct Scene *scene, struct FE_region *fe_region,
	enum Render_to_finite_elements_mode render_mode, 
	struct Computed_field *coordinate_field);
/******************************************************************************
LAST MODIFIED : 7 December 2005

DESCRIPTION :
Renders the visible objects as finite elements into the specified <fe_region>.
==============================================================================*/
#endif /* !defined (RENDER_TO_FINITE_ELEMENTS_H) */
