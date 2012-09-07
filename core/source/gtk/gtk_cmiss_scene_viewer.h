/*******************************************************************************
FILE : gtk_cmiss_scene_viewer.h

LAST MODIFIED : 23 September 2002

DESCRIPTION :
The gtk interface to the Cmiss_scene_viewer object for rendering cmiss
scenes.  This creates a GtkWidget that represents the scene viewer and allows
the Cmiss_scene_viewer to be integrated with other Gtk widgets.  To control
the scene viewer get the contained Cmiss_scene_viewer object and use its api.
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
#ifndef __GTK_CMISS_SCENE_VIEWER_H__
#define __GTK_CMISS_SCENE_VIEWER_H__

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "api/cmiss_scene_viewer.h"


#if GTK_MAJOR_VERSION >= 2

G_BEGIN_DECLS

#define GTK_TYPE_CMISS_SCENE_VIEWER \
   (gtk_cmiss_scene_viewer_get_type())
#define GTK_CMISS_SCENE_VIEWER(obj) \
   (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CMISS_SCENE_VIEWER, GtkCmissSceneViewer))
#define GTK_CMISS_SCENE_VIEWER_CLASS(klass) \
   (G_TYPE_CHECK_CLASS_CAST (klass, GTK_TYPE_CMISS_SCENE_VIEWER, GtkCmissSceneViewerClass))
#define GTK_IS_CMISS_SCENE_VIEWER(obj) \
   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CMISS_SCENE_VIEWER))
#define GTK_IS_CMISS_SCENE_VIEWER_CLASS(klass) \
   (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CMISS_SCENE_VIEWER))
#define GTK_CMISS_SCENE_VIEWER_GET_CLASS(obj) \
   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CMISS_SCENE_VIEWER, GtkCmissSceneViewer))

#else /* GTK_MAJOR_VERSION >= 2 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_TYPE_CMISS_SCENE_VIEWER \
   (gtk_cmiss_scene_viewer_get_type())
#define GTK_CMISS_SCENE_VIEWER(obj) \
   (GTK_CHECK_CAST ((obj), GTK_TYPE_CMISS_SCENE_VIEWER, GtkCmissSceneViewer))
#define GTK_CMISS_SCENE_VIEWER_CLASS(klass) \
   (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_CMISS_SCENE_VIEWER, GtkCmissSceneViewerClass))
#define GTK_IS_CMISS_SCENE_VIEWER(obj) \
   (GTK_CHECK_TYPE ((obj), GTK_TYPE_CMISS_SCENE_VIEWER))
#define GTK_IS_CMISS_SCENE_VIEWER_CLASS(klass) \
   (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CMISS_SCENE_VIEWER))

#endif /* GTK_MAJOR_VERSION >= 2 */

typedef struct _GtkCmissSceneViewer GtkCmissSceneViewer;
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION :
==============================================================================*/

typedef struct _GtkCmissSceneViewerClass GtkCmissSceneViewerClass;
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION :
==============================================================================*/

struct _GtkCmissSceneViewer
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION :
==============================================================================*/
{
	GtkBin bin;

	Cmiss_scene_viewer_id cmiss_scene_viewer;
	struct Cmiss_scene_viewer_package* cmiss_scene_viewer_package;
};

struct _GtkCmissSceneViewerClass
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION :
==============================================================================*/
{
	GtkBinClass parent_class;
};

#if GTK_MAJOR_VERSION >= 2
GType gtk_cmiss_scene_viewer_get_type(void);
/*******************************************************************************
LAST MODIFIED : 23 September 2002

DESCRIPTION :
==============================================================================*/
#else /* GTK_MAJOR_VERSION >= 2 */
GtkType gtk_cmiss_scene_viewer_get_type(void);
/*******************************************************************************
LAST MODIFIED : 23 September 2002

DESCRIPTION :
==============================================================================*/
#endif /* GTK_MAJOR_VERSION >= 2 */

GtkWidget* gtk_cmiss_scene_viewer_new(
	struct Cmiss_scene_viewer_package *scene_viewer_package);
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION :
Create a new gtk_cmiss_scene_viewer widget with all the default settings.
To modify this object get the Cmiss_scene_viewer handle and then interact with
that.
==============================================================================*/

Cmiss_scene_viewer_id gtk_cmiss_scene_viewer_get_cmiss_scene_viewer(
	GtkCmissSceneViewer *gtk_cmiss_scene_viewer);
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION:
Returns a handle to the Cmiss_scene_viewer object which this
<gtk_cmiss_scene_viewer> wraps.  This handle can then be used to interact with
the scene_viewer.
==============================================================================*/

#if GTK_MAJOR_VERSION >= 2
G_END_DECLS
#else /* GTK_MAJOR_VERSION >= 2 */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* __GTK_CMISS_SCENE_VIEWER_H__ */
