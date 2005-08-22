/*******************************************************************************
FILE : ThreeDDraP.h

LAST MODIFIED : 03 May 2004

DESCRIPTION :
Private header file for the 3-D drawing widget.
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
#if !defined (_ThreeDDraP_h)
#define _ThreeDDraP_h

/* private header files for superclasses */
#include <X11/CoreP.h>
#include <X11/CompositeP.h>
/* public header file for 3-D drawing widget */
#include "ThreeDDraw.h"
/* header for 3-D graphics application programming interface */

/*
Class types
-----------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 14 April 1994

DESCRIPTION :
==============================================================================*/
{
	/* a structure can't be empty */
	int dummy;
} ThreeDDrawingClassPart;

typedef struct _ThreeDDrawingClassRec
/*******************************************************************************
LAST MODIFIED : 14 April 1994

DESCRIPTION :
==============================================================================*/
{
	/* superclass parts */
	CoreClassPart core_class;
	CompositeClassPart composite_class;
	/* 3-D drawing widget part */
	ThreeDDrawingClassPart threeDDrawing_class;
} ThreeDDrawingClassRec;

/*
Class record
------------
*/
extern ThreeDDrawingClassRec threeDDrawingClassRec;

/*
Instance types
--------------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 03 May 2004

DESCRIPTION :
==============================================================================*/
{
	/* specify if the buffer is present for the widget */
	Boolean present;
	Colormap colour_map;
	Visual *visual;
	XtCallbackList expose_callback;
	XtCallbackList initialize_callback;
} X3dOutputBuffer;

typedef struct
/*******************************************************************************
LAST MODIFIED : 27 April 1994

DESCRIPTION :
???DB.  At present, only implementing the normal buffer
==============================================================================*/
{
	/* resources */
	XtCallbackList input_callback;
	XtCallbackList resize_callback;
	/* private state */
	X3dOutputBuffer normal_buffer;
} ThreeDDrawingPart;

typedef struct _ThreeDDrawingRec
/*******************************************************************************
LAST MODIFIED : 14 April 1994

DESCRIPTION :
==============================================================================*/
{
	/* superclass parts */
	CorePart core;
	CompositePart composite;
	/* 3-D drawing widget part */
	ThreeDDrawingPart three_d_drawing;
} ThreeDDrawingRec;

#endif
