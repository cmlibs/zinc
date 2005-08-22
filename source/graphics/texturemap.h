/*******************************************************************************
FILE : texturemap.h

LAST MODIFIED : 15 September 1997

DESCRIPTION :
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
#if !defined (TEXTUREMAP_H)
#define TEXTUREMAP_H

#include "finite_element/finite_element.h"
#include "graphics/graphics_window.h"

/*
Global types
------------
*/
struct Image_buffer
/*******************************************************************************
LAST MODIFIED : 7 April 1997

DESCRIPTION :
==============================================================================*/
{
	short xsize,ysize,zsize;
	short **rbuf,**gbuf,**bbuf,**abuf;	
}; /* struct Image_buffer */

/*
Global functions
----------------
*/
int generate_textureimage_from_FE_element(
	struct Graphics_window *graphics_window,char *infile,char *outfile, 
	struct FE_element *element,double ximax[3],
	struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 15 September 1997

DESCRIPTION :
Creates texture map segment (range[0,ximax[3]]) SGI rgb image <out_image> from
<in_image> by interpolating and projecting FE element surface onto normalized
image space.
==============================================================================*/
#endif /* !defined (TEXTUREMAP_H) */
