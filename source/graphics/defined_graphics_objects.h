/*******************************************************************************
FILE : defined_graphics_objects.h

LAST MODIFIED : 20 June 2002

DESCRIPTION :
Routines which construct graphics objects. (but do not depend on finite elements)
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
#if !defined (DEFINED_GRAPHICS_OBJECTS_H)
#define DEFINED_GRAPHICS_OBJECTS_H

/*
Global Functions
----------------
*/

int create_Spectrum_colour_bar(struct GT_object **graphics_object_address,
	char *name,struct Spectrum *spectrum,Triple bar_centre,Triple bar_axis,
	Triple side_axis,float bar_length,float bar_radius,float extend_length,
	int tick_divisions,float tick_length,char *number_format,
	struct Graphical_material *bar_material,
	struct Graphical_material *tick_label_material);
/*******************************************************************************
LAST MODIFIED : 13 October 2000

DESCRIPTION :
Creates a coloured bar with annotation for displaying the scale of <spectrum>.
First makes a graphics object named <name> consisting of a cylinder of size
<bar_length>, <bar_radius> centred at <bar_centre> and aligned with the
<bar_axis>, combining the <bar_material> with the <spectrum>. The bar is
coloured by the current range of the spectrum. An extra <extend_length> is added
on to each end of the bar and given the colour the spectrum returns outside its
range at that end.
The <side_axis> must not be collinear with <bar_axis> and is used to calculate
points around the bar and the direction of the <tick_divisions>+1 ticks at which
spectrum values are written.
Attached to the bar graphics_object are two graphics objects using the
<tick_label_material>, one containing the ticks, the other the labels. The
labels are written using the <number_format>.
On successful return a pointer to the bar_graphics_object is put at
<*graphics_object_address>. If there is already a colour_bar at this address it
is cleared and redefined.

The side_axis is made to be orthogonal to the bar_axis, and both are made unit
length by this function.
==============================================================================*/

#endif /* !defined (DEFINED_GRAPHICS_OBJECTS_H) */
