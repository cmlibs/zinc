/*******************************************************************************
FILE : gtk_cmiss_scene_viewer.c

LAST MODIFIED : 10 September 2002

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
#include <stdarg.h>
#include <gtk/gtk.h>
#include "general/debug.h"
#include "api/cmiss_scene_viewer.h"
#include "graphics/scene_viewer.h"
#include "user_interface/message.h"
#include "gtk/gtk_cmiss_scene_viewer.h"

/*
Module variables
----------------
*/

static GtkBinClass *parent_class = NULL;

/*
Module functions
----------------
*/

void gtk_cmiss_scene_viewer_package_destroy_callback(
	struct Cmiss_scene_viewer_package *scene_viewer_package, void *dummy_void,
	void *gtk_cmiss_scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Clear the scene viewer reference when it is no longer valid.
==============================================================================*/
{
	GtkCmissSceneViewer *gtk_cmiss_scene_viewer;

	USE_PARAMETER(scene_viewer_package);
	USE_PARAMETER(dummy_void);
	ENTER(Computed_field_window_projection_scene_viewer_destroy_callback);
	if (gtk_cmiss_scene_viewer = (GtkCmissSceneViewer *)gtk_cmiss_scene_viewer_void)
	{
		gtk_cmiss_scene_viewer->cmiss_scene_viewer = (Cmiss_scene_viewer_id)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_window_projection_scene_viewer_callback.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_window_projection_scene_viewer_callback */

static void gtk_cmiss_scene_viewer_resize(GtkWidget *widget,
	GtkAllocation *allocation)
/*******************************************************************************
LAST MODIFIED : 19 August 2002

DESCRIPTION :
==============================================================================*/
{
	ENTER(gtk_cmiss_scene_viewer_resize);
	if (widget)
	{
#if GTK_MAJOR_VERSION >= 2
		gtk_widget_size_allocate (gtk_bin_get_child(GTK_BIN(widget)), allocation);
#else /* GTK_MAJOR_VERSION >= 2 */
		gtk_widget_size_allocate (GTK_BIN(widget)->child, allocation);
#endif /* GTK_MAJOR_VERSION >= 2 */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gtk_cmiss_scene_viewer_resize.  Invalid argument(s)");
	}
	LEAVE;
} /* gtk_cmiss_scene_viewer_resize */

static void gtk_cmiss_scene_viewer_destroy(GtkObject *object)
/*******************************************************************************
LAST MODIFIED : 19 August 2002

DESCRIPTION:
==============================================================================*/
{
  GtkCmissSceneViewer *gtk_cmiss_scene_viewer;

  ENTER(gtk_cmiss_scene_viewer_destroy);

  if (object && (GTK_IS_CMISS_SCENE_VIEWER(object)))
  {
	  gtk_cmiss_scene_viewer = GTK_CMISS_SCENE_VIEWER(object);
	  
	  if (gtk_cmiss_scene_viewer->cmiss_scene_viewer)
	  {
		  destroy_Scene_viewer_from_package(
			  &gtk_cmiss_scene_viewer->cmiss_scene_viewer,
			  gtk_cmiss_scene_viewer->cmiss_scene_viewer_package);
	  
		  Cmiss_scene_viewer_package_remove_destroy_callback(
			  gtk_cmiss_scene_viewer->cmiss_scene_viewer_package,
			  gtk_cmiss_scene_viewer_package_destroy_callback,
			  gtk_cmiss_scene_viewer);
	  }
	  
	  if (GTK_OBJECT_CLASS (parent_class)->destroy)
		  (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
  }
  else
  {
	  display_message(ERROR_MESSAGE,"gtk_cmiss_scene_viewer_destroy.  "
		  "Invalid argument(s)");
  }
  LEAVE;
}

static void gtk_cmiss_scene_viewer_class_init (GtkCmissSceneViewerClass *klass)
/*******************************************************************************
LAST MODIFIED : 19 August 2002

DESCRIPTION:
==============================================================================*/
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  ENTER(gtk_cmiss_scene_viewer_class_init);

#if GTK_MAJOR_VERSION >= 2
  parent_class = g_type_class_peek_parent(klass);
#else /* GTK_MAJOR_VERSION >= 2 */
  parent_class = gtk_type_class(gtk_bin_get_type());
#endif /* GTK_MAJOR_VERSION >= 2 */
  object_class = (GtkObjectClass*) klass;
  widget_class = (GtkWidgetClass*) klass;
  
  object_class->destroy = gtk_cmiss_scene_viewer_destroy;

  widget_class->size_allocate = gtk_cmiss_scene_viewer_resize;

  LEAVE;
}

static void gtk_cmiss_scene_viewer_init(GtkCmissSceneViewer *cmiss_scene_viewer)
/*******************************************************************************
LAST MODIFIED : 19 August 2002

DESCRIPTION:
==============================================================================*/
{
	ENTER(gtk_cmiss_scene_viewer_init);

#if GTK_MAJOR_VERSION >= 2
	/* We don't want gtk to double buffer the widget for us */
	gtk_widget_set_double_buffered(GTK_WIDGET(cmiss_scene_viewer), FALSE);
#else /* GTK_MAJOR_VERSION >= 2 */
	USE_PARAMETER(cmiss_scene_viewer);
#endif /* GTK_MAJOR_VERSION >= 2 */

	LEAVE;
}

/*
Global functions
----------------
*/

GtkWidget *gtk_cmiss_scene_viewer_new(
	struct Cmiss_scene_viewer_package *scene_viewer_package)
/*******************************************************************************
LAST MODIFIED : 19 August 2002

DESCRIPTION:
==============================================================================*/
{
	GtkCmissSceneViewer *gtk_cmiss_scene_viewer;

	ENTER(gtk_cmiss_scene_viewer_new);

#if GTK_MAJOR_VERSION >= 2
	gtk_cmiss_scene_viewer = g_object_new(GTK_TYPE_CMISS_SCENE_VIEWER, NULL);
#else /* GTK_MAJOR_VERSION >= 2 */
	gtk_cmiss_scene_viewer = gtk_type_new(gtk_cmiss_scene_viewer_get_type());
#endif /* GTK_MAJOR_VERSION >= 2 */

	gtk_cmiss_scene_viewer->cmiss_scene_viewer = create_Cmiss_scene_viewer_gtk(
		scene_viewer_package,
		GTK_CONTAINER(gtk_cmiss_scene_viewer), CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
		CMISS_SCENE_VIEWER_STEREO_MONO, /*minimum_colour_buffer_depth*/0, 
		/*minimum_depth_buffer_depth*/8, /*minimum_accumulation_buffer_depth*/8);
	gtk_cmiss_scene_viewer->cmiss_scene_viewer_package = scene_viewer_package;
	Cmiss_scene_viewer_package_add_destroy_callback(scene_viewer_package,
		gtk_cmiss_scene_viewer_package_destroy_callback,
		gtk_cmiss_scene_viewer);

	LEAVE;

	return GTK_WIDGET(gtk_cmiss_scene_viewer);	
}

#if GTK_MAJOR_VERSION >= 2
GType gtk_cmiss_scene_viewer_get_type(void)
/*******************************************************************************
LAST MODIFIED : 23 September 2002

DESCRIPTION :
==============================================================================*/
#else /* GTK_MAJOR_VERSION >= 2 */
GtkType gtk_cmiss_scene_viewer_get_type(void)
/*******************************************************************************
LAST MODIFIED : 23 September 2002

DESCRIPTION :
==============================================================================*/
#endif /* GTK_MAJOR_VERSION >= 2 */
{
  static GtkType object_type = 0;

  ENTER(gtk_cmiss_scene_viewer_get_type);
  if (!object_type)
    {
      static const GtkTypeInfo object_info =
      {
			"GtkCmissSceneViewer",
			sizeof (GtkCmissSceneViewer),
			sizeof (GtkCmissSceneViewerClass),
			(GtkClassInitFunc) gtk_cmiss_scene_viewer_class_init,
			(GtkObjectInitFunc) gtk_cmiss_scene_viewer_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
      };
      
      object_type = gtk_type_unique (GTK_TYPE_BIN, &object_info);
    }
  LEAVE;

  return object_type;
}

Cmiss_scene_viewer_id gtk_cmiss_scene_viewer_get_cmiss_scene_viewer(
	GtkCmissSceneViewer *gtk_cmiss_scene_viewer)
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION:
Returns a handle to the Cmiss_scene_viewer object which this
<gtk_cmiss_scene_viewer> wraps.  This handle can then be used to interact with
the scene_viewer.
==============================================================================*/
{
	Cmiss_scene_viewer_id cmiss_scene_viewer;
  
	ENTER(gtk_cmiss_scene_viewer_get_cmiss_scene_viewer);
	if (gtk_cmiss_scene_viewer)
	{
		cmiss_scene_viewer = gtk_cmiss_scene_viewer->cmiss_scene_viewer;
	}
	else
	{
		display_message(ERROR_MESSAGE,"gtk_cmiss_scene_viewer_get_cmiss_scene_viewer.  "
			"Invalid arguments.");
		cmiss_scene_viewer = (struct Cmiss_scene_viewer *)NULL;
	}

	LEAVE;

	return(cmiss_scene_viewer);
}
