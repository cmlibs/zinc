/*******************************************************************************
FILE : ThreeDDraw.h

LAST MODIFIED : 19 September 2002

DESCRIPTION :
Public header file for the 3-D drawing widget.
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
#if !defined (_ThreeDDraw_h)
#define _ThreeDDraw_h

#include "three_d_drawing/X3d.h"

/*******************************************************************************
Resources :

Name               Class              RepType             Default Value
----               -----              -------             -------------
exposeCallback     ExposeCallback     XtPointer           NULL
initializeCallback InitializeCallback XtPointer           NULL
inputCallback      InputCallback      XtPointer           NULL
resizeCallback     ResizeCallback     XtPointer           NULL
==============================================================================*/

#define X3dNexposeCallback "exposeCallback"
#define X3dCExposeCallback "ExposeCallback"
#define X3dNinitializeCallback "initializeCallback"
#define X3dCInitializeCallback "InitializeCallback"
#define X3dNinputCallback "inputCallback"
#define X3dCInputCallback "InputCallback"
#define X3dNresizeCallback "resizeCallback"
#define X3dCResizeCallback "ResizeCallback"

/* class record constants */
extern WidgetClass threeDDrawingWidgetClass;
typedef struct _ThreeDDrawingClassRec *ThreeDDrawingWidgetClass;
typedef struct _ThreeDDrawingRec *ThreeDDrawingWidget;

/*
Global macros
-------------
*/
#if !defined IsThreeDDrawing
#define IsThreeDDrawing(w)  (XtIsSubclass(w,threeDDrawingWidgetClass))
#endif

/*
Global functions
----------------
*/
int X3dThreeDisInitialised(Widget widget);
/*******************************************************************************
LAST MODIFIED : 5 November 2001

DESCRIPTION :
Returns true if the X3dThreeD <widget> is initialised correctly.  This enables 
us to fail nicely if the Initialise routine was unable to complete properly, 
i.e. it couldn't create a valid rendering context.
==============================================================================*/

#endif /* !defined (_ThreeDDraw_h) */
