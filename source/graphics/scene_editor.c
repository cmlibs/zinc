/*******************************************************************************
FILE : scene_editor.c

LAST MODIFIED : 5 December 2001

DESCRIPTION :
Widgets for editing scene, esp. changing visibility of members.
==============================================================================*/
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/ArrowB.h>
#include <Xm/Form.h>
#include <Xm/FormP.h>
#include <Xm/Frame.h>
#include <Xm/PanedW.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include "choose/choose_scene.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/graphical_element.h"
#include "graphics/graphical_element_editor.h"
#include "graphics/graphics_object.h"
#include "graphics/scene_editor.h"
#include "graphics/volume_texture.h"
#include "transformation/transformation_editor.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Scene_editor_object
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
==============================================================================*/
{
	struct Scene_editor *scene_editor;

	/* name of object: scene or scene_object being viewed */
	char *name;

	/* parent_scene, if any, plus the scene this object represents, if any */
	struct Scene *parent_scene, *scene;
	struct Scene_object *scene_object;
	/* if there is a parent_scene, whether the widget shows object is visible */
	int visible;
	/* if object is a scene, whether its childrens' widgets are displayed */
	int expanded;
	/* list of child objects to put in expand_form if expanded scene list */
	struct LIST(Scene_editor_object) *scene_editor_objects;

	Widget child_form, expand_button, form, select_button, previous_widget,
		visibility_button;
	int in_use;
	int access_count;
}; /* struct Scene_editor_object */

PROTOTYPE_OBJECT_FUNCTIONS(Scene_editor_object);
DECLARE_LIST_TYPES(Scene_editor_object);
PROTOTYPE_LIST_FUNCTIONS(Scene_editor_object);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Scene_editor_object, name, \
	char *);

FULL_DECLARE_INDEXED_LIST_TYPE(Scene_editor_object);

struct Scene_editor
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
==============================================================================*/
{
	/* if autoapply flag is set, any changes to the currently edited graphical
		 element will automatically be applied globally */
	int auto_apply, child_edited, child_expanded, transformation_edited,
		transformation_expanded;

	/* access gt_element_group for current_object if applicable */
	struct GT_element_group *gt_element_group;

	struct Scene_editor_object *current_object;
	/* keep address of pointer to editor so can self-destroy */
	struct Scene_editor **scene_editor_address;
	void *scene_manager_callback_id;
	struct LIST(Scene_editor_object) *scene_editor_objects;
	struct MANAGER(Scene) *scene_manager;
	struct User_interface *user_interface;
	Widget down_button, list_form, main_form, object_form, object_label,
		auto_apply_button, apply_button, revert_button,
		scene_entry, scene_form, scene_label, scene_widget, up_button, window_shell;
	Widget child_button, child_form, content_form, content_frame, content_rowcolumn,
		graphical_element_editor, transformation_button, transformation_editor,
		transformation_form;
}; /* struct Scene_editor */

struct Scene_editor_update_data
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Data for function Scene_update_Scene_editor_object and
Scene_object_update_Scene_editor_object.
==============================================================================*/
{
	struct Scene_editor *scene_editor;
	struct LIST(Scene_editor_object) *scene_editor_objects;
	Widget parent_form, previous_widget;
}; /* struct Scene_object_update_widget_data */

/*
Module prototypes
-----------------
*/

static int Scene_object_update_Scene_editor_object(
	struct Scene_object *scene_object, void *update_data_void);
/*******************************************************************************
LAST MODIFIED : 29 October 2001

DESCRIPTION :
Scene_object iterator function for calling Scene_editor_object_update.
<update_data_void> should point at a struct Scene_editor_update_data.
Prototype.
==============================================================================*/

int DESTROY(Scene_editor_object)(
	struct Scene_editor_object **scene_editor_object_address);
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
Prototype.
==============================================================================*/

static int Scene_editor_set_current_object(struct Scene_editor *scene_editor,
	struct Scene_editor_object *scene_editor_object);
/*******************************************************************************
LAST MODIFIED : 14 November 2001

DESCRIPTION :
Sets the current_object in the <scene_editor> for editing. Updates widgets.
Prototype.
==============================================================================*/

static void Scene_editor_transformation_change(
	struct Scene_object *scene_object, gtMatrix *transformation_matrix, 
	void *scene_editor_void);
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Responds to changes in the transformation of the scene object in the
current object.
Prototype
==============================================================================*/

static int Scene_editor_graphical_element_change(
	struct GT_element_group *gt_element_group, void *scene_editor_void);
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Responds to changes in the GT_element_groups for the current_object.
Prototype
==============================================================================*/

/*
Module functions
----------------
*/

static void XmForm_resize(Widget form)
/*******************************************************************************
LAST MODIFIED : 28 November 2001

DESCRIPTION :
Calls a private function of the <form> to force a resize of the widget.
???RC Can't find any other way of getting this to work properly!
???RC Also in <graphical_element_editor>. Put in common place?
==============================================================================*/
{
	ENTER(XmForm_resize);
	if (form && XmIsForm(form))
	{
		(((XmFormWidgetClass)(form->core.widget_class))->composite_class.change_managed)(form);
	}
	else
	{
		display_message(ERROR_MESSAGE, "XmForm_resize.  Missing or invalid form");
	}
	LEAVE;
} /* XmForm_resize */

struct Scene_editor_object *CREATE(Scene_editor_object)(
	struct Scene_editor *scene_editor, struct Scene *scene,
	struct Scene_object *scene_object)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Creates a scene_editor_object for editing either the <scene> or a
<scene_object>. Note Scene_editor_object_update function is responsible for
creating and updating widgets.
==============================================================================*/
{
	struct Scene *parent_scene;
	struct Scene_editor_object *scene_editor_object;

	ENTER(CREATE(Scene_editor_object));
	parent_scene = (struct Scene *)NULL;
	if (scene_editor && ((scene && (!scene_object)) ||
		((!scene) && scene_object &&
			(parent_scene = Scene_object_get_parent_scene(scene_object)))))
	{
		if (ALLOCATE(scene_editor_object, struct Scene_editor_object, 1))
		{
			scene_editor_object->scene_editor = scene_editor;
			scene_editor_object->parent_scene = parent_scene;
			scene_editor_object->name = (char *)NULL;
			if (scene)
			{
				scene_editor_object->scene = scene;
				scene_editor_object->scene_object = (struct Scene_object *)NULL;
				GET_NAME(Scene)(scene, &(scene_editor_object->name));
				scene_editor_object->visible = 1;
				scene_editor_object->expanded = 0;
			}
			else
			{
				scene_editor_object->scene_object = ACCESS(Scene_object)(scene_object);
				if (SCENE_OBJECT_SCENE == Scene_object_get_type(scene_object))
				{
					scene_editor_object->scene =
						Scene_object_get_child_scene(scene_object);
				}
				else
				{
					scene_editor_object->scene = (struct Scene *)NULL;
				}
				GET_NAME(Scene_object)(scene_object, &(scene_editor_object->name));
				scene_editor_object->visible =
					(g_VISIBLE == Scene_object_get_visibility(scene_object));
				scene_editor_object->expanded = 0;
			}
			if (scene_editor_object->scene)
			{
				scene_editor_object->scene_editor_objects =
					CREATE(LIST(Scene_editor_object))();
			}
			else
			{
				scene_editor_object->scene_editor_objects =
					(struct LIST(Scene_editor_object) *)NULL;
			}
			scene_editor_object->in_use = 0;
			scene_editor_object->access_count = 0;

			scene_editor_object->child_form = (Widget)NULL;
			scene_editor_object->expand_button = (Widget)NULL;
			scene_editor_object->form = (Widget)NULL;
			scene_editor_object->select_button = (Widget)NULL;
			scene_editor_object->previous_widget = (Widget)NULL;
			scene_editor_object->visibility_button = (Widget)NULL;

			if ((!(scene_editor_object->name)) || (scene_editor_object->scene &&
				(!scene_editor_object->scene_editor_objects)))
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Scene_editor_object).  Could not fill object");
				DESTROY(Scene_editor_object)(&scene_editor_object);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Scene_editor_object).  Could not allocate space for object");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Scene_editor_object).  Invalid argument(s)");
		scene_editor_object = (struct Scene_editor_object *)NULL;
	}
	LEAVE;

	return (scene_editor_object);
} /* CREATE(Scene_editor_object) */

int DESTROY(Scene_editor_object)(
	struct Scene_editor_object **scene_editor_object_address)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
==============================================================================*/
{
	struct Scene_editor_object *scene_editor_object;
	int return_code;

	ENTER(DESTROY(Scene_editor_object));
	if (scene_editor_object_address &&
		(scene_editor_object = *scene_editor_object_address))
	{
		if (0 == scene_editor_object->access_count)
		{
			if (scene_editor_object->scene_object)
			{
				DEACCESS(Scene_object)(&(scene_editor_object->scene_object));
			}
			DEALLOCATE(scene_editor_object->name);
			if (scene_editor_object->scene_editor_objects)
			{
				DESTROY(LIST(Scene_editor_object))(
					&(scene_editor_object->scene_editor_objects));
			}
			if (scene_editor_object->form)
			{
				XtDestroyWidget(scene_editor_object->form);
			}
			DEALLOCATE(*scene_editor_object_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Scene_editor_object).  Non-zero access_count");
			return_code = 0;
		}
		*scene_editor_object_address = (struct Scene_editor_object *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Scene_editor_object).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Scene_editor_object) */

DECLARE_OBJECT_FUNCTIONS(Scene_editor_object)

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Scene_editor_object, name, char *, \
	strcmp)

DECLARE_INDEXED_LIST_FUNCTIONS(Scene_editor_object)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Scene_editor_object, \
	name, char *, strcmp)

static int Scene_editor_object_clear_in_use(
	struct Scene_editor_object *scene_editor_object, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 30 October 2001

DESCRIPTION :
Clears scene_editor_object->in_use flag to zero.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_editor_object_clear_in_use);
	USE_PARAMETER(dummy_void);
	if (scene_editor_object)
	{
		scene_editor_object->in_use = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_object_clear_in_use.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_object_clear_in_use */

static int Scene_editor_object_not_in_use(
	struct Scene_editor_object *scene_editor_object, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 30 October 2001

DESCRIPTION :
Returns true if scene_editor_object->in_use is not set.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_editor_object_not_in_use);
	USE_PARAMETER(dummy_void);
	if (scene_editor_object)
	{
		return_code = !(scene_editor_object->in_use);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_object_not_in_use.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_object_not_in_use */

static int Scene_editor_apply_edits(struct Scene_editor *scene_editor)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Applies any changes made to local copies of objects -- such as in the graphical
element editor -- to the global objects.
==============================================================================*/
{
	int return_code;
	gtMatrix transformation_matrix;
	struct GT_element_group *edited_gt_element_group;

	ENTER(Scene_editor_apply_edits);
	if (scene_editor && scene_editor->current_object)
	{
		return_code = 1;
		if (scene_editor->current_object->scene_object)
		{
			busy_cursor_on((Widget)NULL, scene_editor->user_interface);
			if (scene_editor->transformation_edited)
			{
				/* note: turn off callbacks while changes made */
				Scene_object_remove_transformation_callback(
					scene_editor->current_object->scene_object,
					Scene_editor_transformation_change, (void *)scene_editor);
				if (!(transformation_editor_get_transformation(
					scene_editor->transformation_editor, &transformation_matrix) &&
					Scene_object_set_transformation(
						scene_editor->current_object->scene_object,
						&transformation_matrix)))
				{
					display_message(ERROR_MESSAGE,
						"Scene_editor_apply_edits.  Could not modify transformation");
					return_code = 0;
				}
				Scene_object_add_transformation_callback(
					scene_editor->current_object->scene_object,
					Scene_editor_transformation_change, (void *)scene_editor);
			}
			if (scene_editor->child_edited)
			{
				switch (Scene_object_get_type(
					scene_editor->current_object->scene_object))
				{
					case SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP:
					{
						if (scene_editor->gt_element_group)
						{
							/* note: turn off callbacks while changes made */
							GT_element_group_remove_callback(scene_editor->gt_element_group,
								Scene_editor_graphical_element_change, (void *)scene_editor);
							if (edited_gt_element_group =
								graphical_element_editor_get_gt_element_group(
									scene_editor->graphical_element_editor))
							{
								if (!GT_element_group_modify(scene_editor->gt_element_group,
									edited_gt_element_group))
								{
									display_message(ERROR_MESSAGE, "Scene_editor_apply_edits.  "
										"Could not modify graphical element");
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE, "Scene_editor_apply_edits.  "
									"Missing edited graphical element");
								return_code = 0;
							}
							GT_element_group_add_callback(scene_editor->gt_element_group,
								Scene_editor_graphical_element_change, (void *)scene_editor);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Scene_editor_apply_edits.  Missing graphical element");
							return_code = 0;
						}
					} break;
					case SCENE_OBJECT_GRAPHICS_OBJECT:
					case SCENE_OBJECT_SCENE:
					{
						/* nothing to do */
					} break;
				}
			}
			if (return_code)
			{
				scene_editor->child_edited = 0;
				scene_editor->transformation_edited = 0;
				XtSetSensitive(scene_editor->apply_button, FALSE);
				XtSetSensitive(scene_editor->revert_button, FALSE);
			}
			busy_cursor_off((Widget)NULL, scene_editor->user_interface);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_apply_edits.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_apply_edits */

static int Scene_editor_revert_transformation(struct Scene_editor *scene_editor)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Takes the transformation from the current scene object and puts it in the
transformation editor. Clears the transformation_edited flag.
==============================================================================*/
{
	gtMatrix transformation_matrix;
	int return_code;

	ENTER(Scene_editor_revert_transformation);
	if (scene_editor && scene_editor->current_object &&
		scene_editor->current_object->scene_object)
	{
		if (Scene_object_get_transformation(
			scene_editor->current_object->scene_object, &transformation_matrix) &&
			transformation_editor_set_transformation(
				scene_editor->transformation_editor, &transformation_matrix))
		{
			scene_editor->transformation_edited = 0;
			if (!scene_editor->child_edited)
			{
				XtSetSensitive(scene_editor->apply_button, FALSE);
				XtSetSensitive(scene_editor->revert_button, FALSE);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_editor_revert_transformation.  Could not revert transformation");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_revert_transformation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_revert_transformation */

static int Scene_editor_revert_child(struct Scene_editor *scene_editor)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Takes the child object from the current scene object and puts it in the
appropriate child object editor. Clears the child_edited flag.
==============================================================================*/
{
	int return_code;
	struct GT_element_group *gt_element_group;

	ENTER(Scene_editor_revert_child);
	if (scene_editor && scene_editor->current_object &&
		scene_editor->current_object->scene_object)
	{
		return_code = 1;
		switch (Scene_object_get_type(scene_editor->current_object->scene_object))
		{
			case SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP:
			{
				if (gt_element_group = Scene_object_get_graphical_element_group(
					scene_editor->current_object->scene_object))
				{
					if (!graphical_element_editor_set_gt_element_group(
						scene_editor->graphical_element_editor, gt_element_group))
					{
						display_message(ERROR_MESSAGE, "Scene_editor_revert_child.  "
							"Could not revert graphical element");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Scene_editor_revert_child.  Missing graphical element");
					return_code = 0;
				}
			} break;
			case SCENE_OBJECT_GRAPHICS_OBJECT:
			case SCENE_OBJECT_SCENE:
			{
				/* nothing to do */
			} break;
		}
		if (return_code)
		{
			scene_editor->child_edited = 0;
			if (!scene_editor->transformation_edited)
			{
				XtSetSensitive(scene_editor->apply_button, FALSE);
				XtSetSensitive(scene_editor->revert_button, FALSE);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_revert_child.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_revert_child */

struct Scene_editor_object *Scene_editor_get_first_object(
	struct Scene_editor *scene_editor)
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Returns the first scene_editor_object in the <scene_editor>'s list, actually
that of the first one in the scene.
==============================================================================*/
{
	char *name;
	struct Scene *scene;
	struct Scene_editor_object *scene_editor_object;
	struct Scene_object *scene_object;

	ENTER(Scene_editor_get_first_object);
	scene_editor_object = (struct Scene_editor_object *)NULL;
	if (scene_editor)
	{
		if ((scene = Scene_editor_get_scene(scene_editor)) &&
			(scene_object = first_Scene_object_in_Scene_that(scene,
				(LIST_CONDITIONAL_FUNCTION(Scene_object) *)NULL, (void *)NULL)))
		{
			/* get the first scene_editor_object in scene -- can't get first in
				 scene_editor_objects list since ordered by name */
			GET_NAME(Scene_object)(scene_object, &name);
			scene_editor_object =
				FIND_BY_IDENTIFIER_IN_LIST(Scene_editor_object, name)(name,
					scene_editor->scene_editor_objects);
			DEALLOCATE(name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_get_first_object.  Invalid argument(s)");
	}
	LEAVE;

	return (scene_editor_object);
} /* Scene_editor_get_first_object */

int Scene_editor_Scene_update_Scene_editor_objects(
	struct Scene_editor *scene_editor, struct Scene *scene,
	struct LIST(Scene_editor_object) *scene_editor_objects,
	Widget parent_widget)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Updates the <scene_editor_objects> to match the contents of scene, within the
<parent_widget> form.
==============================================================================*/
{
	int return_code;
	struct Scene_editor_update_data update_data;

	ENTER(Scene_editor_Scene_update_Scene_editor_objects);
	if (scene_editor && scene && scene_editor_objects && parent_widget)
	{
		return_code = 1;

		/* clear in-use flags so we know which ones are used later */
		FOR_EACH_OBJECT_IN_LIST(Scene_editor_object)(
			Scene_editor_object_clear_in_use, (void *)NULL, scene_editor_objects);

		update_data.scene_editor = scene_editor;
		update_data.parent_form = parent_widget;
		update_data.previous_widget = (Widget)NULL;
		update_data.scene_editor_objects = scene_editor_objects;

		for_each_Scene_object_in_Scene(scene,
			Scene_object_update_Scene_editor_object, (void *)&update_data);

		REMOVE_OBJECTS_FROM_LIST_THAT(Scene_editor_object)(
			Scene_editor_object_not_in_use, (void *)NULL, scene_editor_objects);

		/* if current_object is invalid, choose another one */
		if ((!scene_editor->current_object) ||
			(1 == scene_editor->current_object->access_count))
		{
			Scene_editor_set_current_object(scene_editor,
				Scene_editor_get_first_object(scene_editor));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_Scene_update_Scene_editor_objects.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_Scene_update_Scene_editor_objects */

static void Scene_editor_scene_change(
	struct MANAGER_MESSAGE(Scene) *message, void *scene_editor_void)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Something has changed globally in the scene manager. Update the affected
<scene_editor_objects>.
==============================================================================*/
{
	struct Scene *scene;
	struct Scene_editor *scene_editor;

	ENTER(Scene_editor_scene_change);
	if (message && (scene_editor = (struct Scene_editor *)scene_editor_void))
	{
		scene = Scene_editor_get_scene(scene_editor);
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT(Scene):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Scene):
			{
				if (IS_OBJECT_IN_LIST(Scene)(scene, message->changed_object_list))
				{
					Scene_editor_Scene_update_Scene_editor_objects(scene_editor, scene,
						scene_editor->scene_editor_objects, scene_editor->list_form);
				}
			} break;
			case MANAGER_CHANGE_IDENTIFIER(Scene):
			case MANAGER_CHANGE_ADD(Scene):
			{
				/* do nothing */
			} break;
			case MANAGER_CHANGE_REMOVE(Scene):
			{
				if (IS_OBJECT_IN_LIST(Scene)(scene, message->changed_object_list))
				{
					/* a bit nasty if these are left around after scene, hence empty */
					REMOVE_ALL_OBJECTS_FROM_LIST(Scene_editor_object)(
						scene_editor->scene_editor_objects);
				}
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_scene_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_scene_change */

static void Scene_editor_transformation_change(
	struct Scene_object *scene_object, gtMatrix *transformation_matrix, 
	void *scene_editor_void)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Responds to changes in the transformation of the scene object in the
current object.
==============================================================================*/
{
	struct Scene_editor *scene_editor;

	ENTER(Scene_editor_transformation_change);
	USE_PARAMETER(transformation_matrix);
	if (scene_object && (scene_editor = (struct Scene_editor *)scene_editor_void)
		&& scene_editor->current_object &&
		scene_editor->current_object->scene_object)
	{
		scene_editor->transformation_edited = 1;
		if (scene_editor->auto_apply)
		{
			Scene_editor_revert_transformation(scene_editor);
		}
		else
		{
			XtSetSensitive(scene_editor->apply_button, TRUE);
			XtSetSensitive(scene_editor->revert_button, TRUE);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_transformation_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_transformation_change */

static int Scene_editor_graphical_element_change(
	struct GT_element_group *gt_element_group, void *scene_editor_void)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Responds to changes in the GT_element_groups for the current_object.
==============================================================================*/
{
	int return_code;
	struct GT_element_group *edit_gt_element_group;
	struct Scene_editor *scene_editor;

	ENTER(Scene_editor_graphical_element_change);
	if (gt_element_group &&
		(scene_editor = (struct Scene_editor *)scene_editor_void) &&
		scene_editor->current_object &&
		(edit_gt_element_group = graphical_element_editor_get_gt_element_group(
			scene_editor->graphical_element_editor)))
	{
		return_code = 1;
		/* work out if global object is different from that in editor */
		if (!GT_element_groups_match(gt_element_group, edit_gt_element_group))
		{
			scene_editor->child_edited = 1;
			if (scene_editor->auto_apply)
			{
				Scene_editor_revert_child(scene_editor);
			}
			else
			{
				XtSetSensitive(scene_editor->apply_button, TRUE);
				XtSetSensitive(scene_editor->revert_button, TRUE);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_graphical_element_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_graphical_element_change */

static int Scene_editor_set_current_object(struct Scene_editor *scene_editor,
	struct Scene_editor_object *scene_editor_object)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Sets the current_object in the <scene_editor> for editing. Updates widgets.
==============================================================================*/
{
	char *full_name, *name;
	gtMatrix transformation_matrix;
	int error, return_code;
	struct GT_element_group *gt_element_group;
	struct GT_object *graphics_object;
	XmString label_string;

	ENTER(Scene_editor_set_current_object);
	if (scene_editor)
	{
		if (scene_editor_object != scene_editor->current_object)
		{
			if (scene_editor->current_object)
			{
				XmToggleButtonSetState(scene_editor->current_object->select_button,
					/*state*/FALSE, /*notify*/FALSE);
				if (scene_editor->current_object->scene_object)
				{
					Scene_object_remove_transformation_callback(
						scene_editor->current_object->scene_object,
						Scene_editor_transformation_change, (void *)scene_editor);
				}
				if (scene_editor->gt_element_group)
				{
					GT_element_group_remove_callback(scene_editor->gt_element_group,
						Scene_editor_graphical_element_change, (void *)scene_editor);
					if ((struct Scene_editor_object *)NULL == scene_editor_object)
					{
						graphical_element_editor_set_gt_element_group(
							scene_editor->graphical_element_editor,
							(struct GT_element_group *)NULL);
					}
					DEACCESS(GT_element_group)(&(scene_editor->gt_element_group));
					scene_editor->gt_element_group = (struct GT_element_group *)NULL;
				}
				DEACCESS(Scene_editor_object)(&(scene_editor->current_object));
			}

			if (scene_editor_object)
			{
				scene_editor->current_object =
					ACCESS(Scene_editor_object)(scene_editor_object);

				XmToggleButtonSetState(scene_editor_object->select_button,
					/*state*/TRUE, /*notify*/FALSE);
				label_string = XmStringCreateSimple(scene_editor_object->name);
				XtVaSetValues(scene_editor->object_label,
					XmNlabelString, label_string, NULL);
				XmStringFree(label_string);

				name = (char *)NULL;
				full_name = (char *)NULL;
				error = 0;
				gt_element_group = (struct GT_element_group *)NULL;
				if (scene_editor_object->scene_object)
				{
					Scene_object_get_transformation(scene_editor_object->scene_object,
						&transformation_matrix);
					transformation_editor_set_transformation(
						scene_editor->transformation_editor, &transformation_matrix);
					Scene_object_add_transformation_callback(
						scene_editor->current_object->scene_object,
						Scene_editor_transformation_change, (void *)scene_editor);
					XtManageChild(scene_editor->transformation_editor);

					switch (Scene_object_get_type(scene_editor_object->scene_object))
					{
						case SCENE_OBJECT_GRAPHICS_OBJECT:
						{
							if (graphics_object =
								Scene_object_get_gt_object(scene_editor_object->scene_object))
							{
								full_name = duplicate_string("Graphics object: ");
								GET_NAME(GT_object)(graphics_object, &name);
								append_string(&full_name, name, &error);
								DEALLOCATE(name);
							}
						} break;
						case SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP:
						{
							full_name = duplicate_string("Graphical element: ");
							gt_element_group = Scene_object_get_graphical_element_group(
								scene_editor_object->scene_object);
							GET_NAME(GROUP(FE_element))(
								GT_element_group_get_element_group(gt_element_group), &name);
							append_string(&full_name, name, &error);
							DEALLOCATE(name);
						} break;
						case SCENE_OBJECT_SCENE:
						{
							full_name = duplicate_string("Child scene: ");
							GET_NAME(Scene)(Scene_object_get_child_scene(
								scene_editor_object->scene_object), &name);
							append_string(&full_name, name, &error);
							DEALLOCATE(name);
						} break;
					}
				}
				if (full_name)
				{
					label_string = XmStringCreateSimple(full_name);
					DEALLOCATE(full_name);
				}
				else
				{
					label_string = XmStringCreateSimple("Unknown child");
				}
				XtVaSetValues(scene_editor->child_button,
					XmNlabelString, label_string, NULL);
				XmStringFree(label_string);
		
				if (gt_element_group)
				{
					scene_editor->gt_element_group =
						ACCESS(GT_element_group)(gt_element_group);
					graphical_element_editor_set_gt_element_group(
						scene_editor->graphical_element_editor, gt_element_group);
					GT_element_group_add_callback(gt_element_group,
						Scene_editor_graphical_element_change, (void *)scene_editor);
					XtManageChild(scene_editor->graphical_element_editor);
				}
				else
				{
					XtUnmanageChild(scene_editor->graphical_element_editor);
				}
				XtManageChild(scene_editor->object_form);
			}
			else
			{
				XtUnmanageChild(scene_editor->object_form);
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_set_current_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_set_current_object */

static void Scene_editor_object_visibility_callback(Widget widget,
	XtPointer scene_editor_object_void, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Callback for when the visibility toggle button state changes.
==============================================================================*/
{
	enum GT_visibility_type visibility;
	struct Scene_editor_object *scene_editor_object;

	ENTER(Scene_editor_object_visibility_callback);
	USE_PARAMETER(call_data);
	if (widget && (scene_editor_object =
		(struct Scene_editor_object *)scene_editor_object_void) &&
		scene_editor_object->scene_object)
	{
		if (XmToggleButtonGetState(scene_editor_object->visibility_button))
		{
			scene_editor_object->visible = 1;
			visibility = g_VISIBLE;
		}
		else
		{
			scene_editor_object->visible = 0;
			visibility = g_INVISIBLE;
		}
		Scene_editor_set_current_object(scene_editor_object->scene_editor,
			scene_editor_object);
		Scene_object_set_visibility(scene_editor_object->scene_object, visibility);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_object_visibility_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_object_visibility_callback */

static void Scene_editor_object_expand_callback(Widget widget,
	XtPointer scene_editor_object_void, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Callback for when the expand toggle button state changes.
==============================================================================*/
{
	struct Scene_editor_object *scene_editor_object;
	XmString label_string;

	ENTER(Scene_editor_object_expand_callback);
	USE_PARAMETER(call_data);
	if (widget && (scene_editor_object =
		(struct Scene_editor_object *)scene_editor_object_void))
	{
		if (scene_editor_object->expanded)
		{
			scene_editor_object->expanded = 0;
			label_string = XmStringCreateSimple("+");
			XtUnmanageChild(scene_editor_object->child_form);
		}
		else
		{
			scene_editor_object->expanded = 1;
			label_string = XmStringCreateSimple("-");
			XtManageChild(scene_editor_object->child_form);
			XmForm_resize(scene_editor_object->child_form);
		}
		XtVaSetValues(scene_editor_object->expand_button,
			XmNlabelString, label_string, NULL);
		XmStringFree(label_string);
		Scene_editor_set_current_object(scene_editor_object->scene_editor,
			scene_editor_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_object_expand_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_object_expand_callback */

static void Scene_editor_object_select_callback(Widget widget,
	XtPointer scene_editor_object_void, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Callback for when the name toggle button state changes.
==============================================================================*/
{
	struct Scene_editor_object *scene_editor_object;

	ENTER(Scene_editor_object_select_callback);
	USE_PARAMETER(call_data);
	if (widget && (scene_editor_object =
		(struct Scene_editor_object *)scene_editor_object_void) &&
		scene_editor_object->scene_editor)
	{
 		if (scene_editor_object ==
			scene_editor_object->scene_editor->current_object)
		{
			/* make sure the object is highlighted properly */
			XmToggleButtonSetState(scene_editor_object->select_button,
				/*state*/TRUE, /*notify*/FALSE);
		}
		Scene_editor_set_current_object(scene_editor_object->scene_editor,
			scene_editor_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_object_select_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_object_select_callback */

static void Scene_editor_close_CB(Widget widget,
	void *scene_editor_void, void *call_data)
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
==============================================================================*/
{
	struct Scene_editor *scene_editor;

	ENTER(Scene_editor_close_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (scene_editor = (struct Scene_editor *)scene_editor_void)
	{
		DESTROY(Scene_editor)(scene_editor->scene_editor_address);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_close_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_close_CB */

static void Scene_editor_up_button_callback(Widget widget,
	void *scene_editor_void, void *call_data)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
==============================================================================*/
{
	int position;
	struct Scene *parent_scene;
	struct Scene_editor *scene_editor;
	struct Scene_editor_object *scene_editor_object;

	ENTER(Scene_editor_up_button_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((scene_editor = (struct Scene_editor *)scene_editor_void) &&
		(scene_editor_object = scene_editor->current_object) &&
		(parent_scene = scene_editor_object->parent_scene) &&
		scene_editor_object->scene_object)
	{
		position = Scene_get_scene_object_position(parent_scene,
			scene_editor_object->scene_object);
		if (1 < position)
		{
			Scene_set_scene_object_position(parent_scene,
				scene_editor_object->scene_object, position - 1);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_up_button_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_up_button_callback */

static void Scene_editor_down_button_callback(Widget widget,
	void *scene_editor_void, void *call_data)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
==============================================================================*/
{
	int position;
	struct Scene *parent_scene;
	struct Scene_editor *scene_editor;
	struct Scene_editor_object *scene_editor_object;

	ENTER(Scene_editor_down_button_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if ((scene_editor = (struct Scene_editor *)scene_editor_void) &&
		(scene_editor_object = scene_editor->current_object) &&
		(parent_scene = scene_editor_object->parent_scene) &&
		scene_editor_object->scene_object)
	{
		position = Scene_get_scene_object_position(parent_scene,
			scene_editor_object->scene_object);
		if (Scene_get_number_of_scene_objects(parent_scene) > position)
		{
			Scene_set_scene_object_position(parent_scene,
				scene_editor_object->scene_object, position + 1);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_down_button_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_down_button_callback */

static void Scene_editor_update_transformation(
	Widget widget,void *scene_editor_void, void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Callback for when changes are made in the graphical element editor. If autoapply
is on, changes are applied globally, otherwise nothing happens.
==============================================================================*/
{
	struct Scene_editor *scene_editor;

	ENTER(Scene_editor_update_transformation);
	USE_PARAMETER(widget);
	USE_PARAMETER(gt_element_group_void);
	if (scene_editor = (struct Scene_editor *)scene_editor_void)
	{
		scene_editor->transformation_edited = 1;
		if (scene_editor->auto_apply)
		{
			Scene_editor_apply_edits(scene_editor);
		}
		else
		{
			XtSetSensitive(scene_editor->apply_button, TRUE);
			XtSetSensitive(scene_editor->revert_button, TRUE);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_update_transformation.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_update_transformation */

static void Scene_editor_update_graphical_element(
	Widget widget,void *scene_editor_void, void *gt_element_group_void)
/*******************************************************************************
LAST MODIFIED : 14 November 2001

DESCRIPTION :
Callback for when changes are made in the graphical element editor. If autoapply
is on, changes are applied globally, otherwise nothing happens.
==============================================================================*/
{
	struct Scene_editor *scene_editor;

	ENTER(Scene_editor_update_graphical_element);
	USE_PARAMETER(widget);
	USE_PARAMETER(gt_element_group_void);
	if (scene_editor = (struct Scene_editor *)scene_editor_void)
	{
		scene_editor->child_edited = 1;
		if (scene_editor->auto_apply)
		{
			Scene_editor_apply_edits(scene_editor);
		}
		else
		{
			XtSetSensitive(scene_editor->apply_button, TRUE);
			XtSetSensitive(scene_editor->revert_button, TRUE);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_update_graphical_element.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_update_graphical_element */

static void Scene_editor_auto_apply_button_callback(Widget widget,
	void *scene_editor_void, void *call_data)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Auto apply/revert toggle button state changed.
==============================================================================*/
{
	struct Scene_editor *scene_editor;

	ENTER(Scene_editor_auto_apply_button_callback);
	USE_PARAMETER(call_data);
	if (widget && (scene_editor = (struct Scene_editor *)scene_editor_void))
	{
		if (XmToggleButtonGetState(widget))
		{
			scene_editor->auto_apply = 1;
			Scene_editor_apply_edits(scene_editor);
		}
		else
		{
			scene_editor->auto_apply = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_auto_apply_button_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_auto_apply_button_callback */

static void Scene_editor_apply_button_callback(Widget widget,
	void *scene_editor_void, void *call_data)
/*******************************************************************************
LAST MODIFIED : 14 November 2001

DESCRIPTION :
Apply button press.
==============================================================================*/
{
	struct Scene_editor *scene_editor;

	ENTER(Scene_editor_apply_button_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (scene_editor = (struct Scene_editor *)scene_editor_void)
	{
		Scene_editor_apply_edits(scene_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_apply_button_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_apply_button_callback */

static void Scene_editor_revert_button_callback(Widget widget,
	void *scene_editor_void, void *call_data)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Revert button press. Restore global state to local objects being edited.
==============================================================================*/
{
	struct Scene_editor *scene_editor;

	ENTER(Scene_editor_revert_button_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (scene_editor = (struct Scene_editor *)scene_editor_void)
	{
		Scene_editor_revert_transformation(scene_editor);
		Scene_editor_revert_child(scene_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_revert_button_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_revert_button_callback */

static void Scene_editor_content_rowcolumn_entry_callback(Widget widget,
	void *scene_editor_void, void *call_data)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Transformation/child editor toggle button value changed.
Entry callback from content_rowcolun XmRowColumn widget.
==============================================================================*/
{
	struct Scene_editor *scene_editor;
	Widget toggle_button;

	ENTER(Scene_editor_content_rowcolumn_entry_callback);
	USE_PARAMETER(widget);
	if ((scene_editor = (struct Scene_editor *)scene_editor_void) &&
		(toggle_button = ((XmRowColumnCallbackStruct *)call_data)->widget))
	{
		if (XmToggleButtonGetState(toggle_button))
		{
			if (scene_editor->transformation_button == toggle_button)
			{
				XtManageChild(scene_editor->transformation_form);
				scene_editor->transformation_expanded = 1;
			}
			else if (scene_editor->child_button == toggle_button)
			{
				XtManageChild(scene_editor->child_form);
				scene_editor->child_expanded = 1;
			}
		}
		else
		{
			if (scene_editor->transformation_button == toggle_button)
			{
				XtUnmanageChild(scene_editor->transformation_form);
				scene_editor->transformation_expanded = 0;
			}
			else if (scene_editor->child_button == toggle_button)
			{
				XtUnmanageChild(scene_editor->child_form);
				scene_editor->child_expanded = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_content_rowcolumn_entry_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_content_rowcolumn_entry_callback */

static void Scene_editor_update_scene(Widget scene_widget,
	void *scene_editor_void, void *scene_void)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Called when a scene is selected in the chooser widget.
==============================================================================*/
{
	struct Scene_editor *scene_editor;

	ENTER(Scene_editor_update_scene);
	if (scene_widget && (scene_editor = (struct Scene_editor *)scene_editor_void))
	{
		Scene_editor_Scene_update_Scene_editor_objects(
			scene_editor, (struct Scene *)scene_void,
			scene_editor->scene_editor_objects, scene_editor->list_form);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_update_scene.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_editor_update_scene */

static int Scene_editor_object_update(struct Scene_editor *scene_editor,
	struct Scene *scene, struct Scene_object *scene_object,
	struct LIST(Scene_editor_object) *scene_editor_objects,
	Widget parent_form, Widget *previous_widget_address,
	int update_attachments_only)
/*******************************************************************************
LAST MODIFIED : 20 November 2001

DESCRIPTION :
Tries to find by name a Scene_editor_object in <scene_editor_objects> that
could be for the <scene> or <scene_object>, only one of which can be given.
If there is none, one is created and added to the <scene_editor_objects>.
Then goes through each widget in the scene_editor_object and makes sure it is
created and displaying the values from the respective scene/scene_object and
with the correct previous widget. Marks the object as "in_use" so that those
that are not are removed.
If <update_attachments_only> is set, any <scene> or <scene_object> that
already has a Scene_editor_object has it updated to have the correct attachments
to the previous widget, but its widgets and child widgets are not updated.
==============================================================================*/
{
	Arg args[12];
	char *expand_label, *name;
	int num_args, return_code, update_children, visible;
	struct Scene *new_scene, *parent_scene;
	struct Scene_editor_object *scene_editor_object;

	ENTER(Scene_editor_object_update);
	parent_scene = (struct Scene *)NULL;
	if (((scene && (!scene_object)) || ((!scene) && scene_object &&
		(parent_scene = Scene_object_get_parent_scene(scene_object)))) &&
		parent_form && previous_widget_address)
	{
		return_code = 1;
		update_children = !update_attachments_only;
		if (scene)
		{
			GET_NAME(Scene)(scene, &name);
			new_scene = scene;
		}
		else
		{
			GET_NAME(Scene_object)(scene_object, &name);
			if (SCENE_OBJECT_SCENE == Scene_object_get_type(scene_object))
			{
				new_scene = Scene_object_get_child_scene(scene_object);
			}
			else
			{
				new_scene = (struct Scene *)NULL;
			}
		}
		if (name)
		{
			if (scene_editor_object = FIND_BY_IDENTIFIER_IN_LIST(
				Scene_editor_object, name)(name, scene_editor_objects))
			{
				/* discard current one if fundamentally different */
				if ((parent_scene != scene_editor_object->parent_scene) ||
					(new_scene != scene_editor_object->scene))
				{
					REMOVE_OBJECT_FROM_LIST(Scene_editor_object)(scene_editor_object,
						scene_editor_objects);
					scene_editor_object = (struct Scene_editor_object *)NULL;
				}
			}
			if (!scene_editor_object)
			{
				update_children = 1;
				scene_editor_object = CREATE(Scene_editor_object)(scene_editor,
					scene, scene_object);
				if (!ADD_OBJECT_TO_LIST(Scene_editor_object)(scene_editor_object,
					scene_editor_objects))
				{
					DESTROY(Scene_editor_object)(&scene_editor_object);
					scene_editor_object = (struct Scene_editor_object *)NULL;
				}
			}
			if (scene_editor_object)
			{
				/* create/update the widgets */

				/* form widget */
				if ((!(scene_editor_object->form)) ||
					(scene_editor_object->previous_widget != *previous_widget_address))
				{
					num_args = 0;
					if (*previous_widget_address)
					{
						XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_WIDGET);
						num_args++;
						XtSetArg(args[num_args], XmNtopWidget,
							(XtPointer)(*previous_widget_address));
						num_args++;
					}
					else
					{
						XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_FORM);
						num_args++;
					}
					if (scene_editor_object->form)
					{
						XtSetValues(scene_editor_object->form, args, num_args);
					}
					else
					{
						XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_FORM);
						num_args++;
						scene_editor_object->form =
							XmCreateForm(parent_form, "form", args, num_args);
						XtManageChild(scene_editor_object->form);
					}
					scene_editor_object->previous_widget = *previous_widget_address;
				}
				*previous_widget_address = scene_editor_object->form;

				if (update_children)
				{
					/* visibility_button */
					if (parent_scene)
					{
						visible = (g_VISIBLE == Scene_object_get_visibility(scene_object));
						if (scene_editor_object->visibility_button)
						{
							if (visible != scene_editor_object->visible)
							{
								XmToggleButtonSetState(scene_editor_object->visibility_button,
									/*state*/visible, /*notify*/FALSE);
							}
						}
						else
						{
							num_args = 0;
							XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_FORM);
							num_args++;
							XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_FORM);
							num_args++;
							XtSetArg(args[num_args], XmNset, visible);
							num_args++;
							XtSetArg(args[num_args], XmNindicatorOn, TRUE);
							num_args++;
							XtSetArg(args[num_args], XmNindicatorSize, 16);
							num_args++;
							XtSetArg(args[num_args], XmNmarginWidth, 0);
							num_args++;
							XtSetArg(args[num_args], XmNmarginHeight, 0);
							num_args++;
							XtSetArg(args[num_args], XmNspacing, 0);
							num_args++;
							XtSetArg(args[num_args], XmNfontList,
								(XtPointer)scene_editor->user_interface->normal_fontlist);
							num_args++;
							scene_editor_object->visibility_button = XmCreateToggleButton(
								scene_editor_object->form, "", args, num_args);
							XtAddCallback(scene_editor_object->visibility_button,
								XmNvalueChangedCallback,
								Scene_editor_object_visibility_callback,
								(XtPointer)scene_editor_object);
							XtManageChild(scene_editor_object->visibility_button);
						}
						scene_editor_object->visible = visible;
					}

					/* expand button -- if object contains a scene */
					if (scene_editor_object->scene)
					{
						if (!(scene_editor_object->expand_button))
						{
							num_args = 0;
							XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_FORM);
							num_args++;
							if (parent_scene)
							{
								XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_WIDGET);
								num_args++;
								XtSetArg(args[num_args], XmNleftWidget,
									scene_editor_object->visibility_button);
								num_args++;
							}
							else
							{
								XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_FORM);
								num_args++;
							}
#if defined (OLD_CODE)
							XtSetArg(args[num_args], XmNset, scene_editor_object->expanded);
							num_args++;
							XtSetArg(args[num_args], XmNindicatorOn, FALSE);
							num_args++;
							XtSetArg(args[num_args], XmNindicatorType, XmONE_OF_MANY);
							num_args++;
							XtSetArg(args[num_args], XmNtopOffset, 2);
							num_args++;
							XtSetArg(args[num_args], XmNleftOffset, 2);
							num_args++;
#endif /* defined (OLD_CODE) */
							XtSetArg(args[num_args], XmNshadowThickness, 1);
							num_args++;
							XtSetArg(args[num_args], XmNborderWidth, 0);
							num_args++;
							XtSetArg(args[num_args], XmNmarginHeight, 0);
							num_args++;
							XtSetArg(args[num_args], XmNmarginWidth, 3);
							num_args++;
#if defined (OLD_CODE)
							XtSetArg(args[num_args], XmNheight, 21);
							num_args++;
							XtSetArg(args[num_args], XmNwidth, 21);
							num_args++;
#endif /* defined (OLD_CODE) */
							XtSetArg(args[num_args], XmNfontList,
								(XtPointer)scene_editor->user_interface->normal_fontlist);
							num_args++;
							if (scene_editor_object->expanded)
							{
								expand_label = "-";
							}
							else
							{
								expand_label = "+";
							}
							scene_editor_object->expand_button = XmCreatePushButton(
								scene_editor_object->form, expand_label, args, num_args);
							XtAddCallback(scene_editor_object->expand_button,
								XmNactivateCallback,
								Scene_editor_object_expand_callback,
								(XtPointer)scene_editor_object);
							XtManageChild(scene_editor_object->expand_button);
						}
					}

					/* select_button */
					if (!(scene_editor_object->select_button))
					{
						num_args = 0;
						XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_FORM);
						num_args++;
						XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_WIDGET);
						num_args++;
						if (scene_editor_object->scene)
						{
							XtSetArg(args[num_args], XmNleftWidget,
								scene_editor_object->expand_button);
						}
						else
						{
							XtSetArg(args[num_args], XmNleftWidget,
								scene_editor_object->visibility_button);
						}
						num_args++;
						XtSetArg(args[num_args], XmNindicatorOn, FALSE);
						num_args++;
						XtSetArg(args[num_args], XmNindicatorType, XmN_OF_MANY);
						num_args++;
						XtSetArg(args[num_args], XmNshadowThickness, 0);
						num_args++;
						XtSetArg(args[num_args], XmNfontList,
							(XtPointer)scene_editor->user_interface->normal_fontlist);
						num_args++;
						scene_editor_object->select_button = XmCreateToggleButton(
							scene_editor_object->form, name, args, num_args);
						XtAddCallback(scene_editor_object->select_button,
							XmNvalueChangedCallback,
							Scene_editor_object_select_callback,
							(XtPointer)scene_editor_object);
						/*XtAddCallback(scene_editor_object->select_button,
							XmNarmCallback,
							Scene_editor_object_select_callback,
							(XtPointer)scene_editor_object);*/
						XtManageChild(scene_editor_object->select_button);
					}

					/* expanded scene contents */
					if (scene_editor_object->scene)
					{
						/* child_form */
						if (!(scene_editor_object->child_form))
						{
							num_args = 0;
							XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_WIDGET);
							num_args++;
							XtSetArg(args[num_args], XmNtopWidget,
								scene_editor_object->select_button);
							num_args++;
							if (scene_editor_object->parent_scene)
							{
								XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_WIDGET);
								num_args++;
								XtSetArg(args[num_args], XmNleftWidget,
									scene_editor_object->visibility_button);
								num_args++;
							}
							else
							{
								XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_FORM);
								num_args++;
							}
							scene_editor_object->child_form = XmCreateForm(
								scene_editor_object->form, "child", args, num_args);
						}

						Scene_editor_Scene_update_Scene_editor_objects(
							scene_editor, scene_editor_object->scene,
							scene_editor_object->scene_editor_objects,
							scene_editor_object->child_form);

						if (scene_editor_object->expanded)
						{
							XtManageChild(scene_editor_object->child_form);
						}
						else
						{
							XtUnmanageChild(scene_editor_object->child_form);
						}
					}
				}
				scene_editor_object->in_use = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE, "Scene_editor_object_update.  "
					"Could not find or create scene_editor_object");
				return_code = 0;
			}
			DEALLOCATE(name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_editor_object_update.  Could not get name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_object_update.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_object_update */

static int Scene_object_update_Scene_editor_object(
	struct Scene_object *scene_object, void *update_data_void)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Scene_object iterator function for calling Scene_editor_object_update.
<update_data_void> should point at a struct Scene_editor_update_data.
==============================================================================*/
{
	int return_code;
	struct Scene_editor_update_data *update_data;

	ENTER(Scene_object_update_Scene_editor_object);
	if (scene_object && (update_data =
		(struct Scene_editor_update_data *)update_data_void))
	{
		return_code = Scene_editor_object_update(update_data->scene_editor,
			(struct Scene *)NULL, scene_object, update_data->scene_editor_objects,
			update_data->parent_form, &(update_data->previous_widget),
			/*update_attachments_only*/0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_update_Scene_editor_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_update_Scene_editor_object */

/*
Global functions
----------------
*/

struct Scene_editor *CREATE(Scene_editor)(
	struct Scene_editor **scene_editor_address, Widget parent,
	struct MANAGER(Scene) *scene_manager, struct Scene *scene,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Note on successful return the dialog is put at <*scene_editor_address>.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	struct Callback_data callback;
	struct Scene_editor *scene_editor;
	Widget content_form, content_frame, controls_form, down_arrow_button,
		paned_window, scrolled_window, up_arrow_button;

	ENTER(CREATE(Scene_editor));
	scene_editor = (struct Scene_editor *)NULL;
	if (scene_manager && scene && computed_field_package && element_manager &&
		fe_field_manager && graphical_material_manager && default_material &&
		glyph_list && spectrum_manager && default_spectrum &&
		volume_texture_manager && user_interface)
	{
		if (ALLOCATE(scene_editor, struct Scene_editor, 1))
		{
			scene_editor->auto_apply = 1;
			scene_editor->child_edited = 0;
			scene_editor->child_expanded = 1;
			scene_editor->transformation_edited = 0;
			scene_editor->transformation_expanded = 0;
			scene_editor->current_object = (struct Scene_editor_object *)NULL;
			/* access gt_element_group for current_object if applicable */
			scene_editor->gt_element_group = (struct GT_element_group *)NULL;
			scene_editor->scene_manager = scene_manager;
			/* get callbacks for scene changes */
			scene_editor->scene_manager_callback_id = MANAGER_REGISTER(Scene)(
				Scene_editor_scene_change, (void *)scene_editor, scene_manager);
			scene_editor->user_interface = user_interface;
			scene_editor->scene_editor_objects = CREATE(LIST(Scene_editor_object))();
			scene_editor->scene_editor_address = (struct Scene_editor **)NULL;
			scene_editor->window_shell = (Widget)NULL;
			scene_editor->main_form = (Widget)NULL;
			scene_editor->list_form = (Widget)NULL;
			scene_editor->object_form = (Widget)NULL;
			scene_editor->up_button = (Widget)NULL;
			scene_editor->down_button = (Widget)NULL;
			scene_editor->object_label = (Widget)NULL;
			scene_editor->revert_button = (Widget)NULL;
			scene_editor->apply_button = (Widget)NULL;
			scene_editor->auto_apply_button = (Widget)NULL;
			scene_editor->scene_entry = (Widget)NULL;
			scene_editor->scene_label = (Widget)NULL;
			scene_editor->scene_form = (Widget)NULL;
			scene_editor->scene_widget = (Widget)NULL;
			scene_editor->child_button = (Widget)NULL;
			scene_editor->child_form = (Widget)NULL;
			scene_editor->content_rowcolumn = (Widget)NULL;
			scene_editor->graphical_element_editor = (Widget)NULL;
			scene_editor->transformation_button = (Widget)NULL;
			scene_editor->transformation_editor = (Widget)NULL;
			scene_editor->transformation_form = (Widget)NULL;

			scene_editor->window_shell = XtVaCreatePopupShell(
				"Scene Editor", topLevelShellWidgetClass, parent,
				XmNdeleteResponse, XmDO_NOTHING,
				XmNallowShellResize, FALSE, NULL);
			/* Set up window manager callback for close window message */
			WM_DELETE_WINDOW = XmInternAtom(XtDisplay(scene_editor->window_shell),
				"WM_DELETE_WINDOW", False);
			XmAddWMProtocolCallback(scene_editor->window_shell,
				WM_DELETE_WINDOW, Scene_editor_close_CB, scene_editor);
			/* Register the shell with the busy signal list */
			create_Shell_list_item(&(scene_editor->window_shell),
				scene_editor->user_interface);

			scene_editor->main_form = XtVaCreateWidget("main",
				xmFormWidgetClass, scene_editor->window_shell,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);
			XtManageChild(scene_editor->main_form);

			/* create the scene chooser */
			scene_editor->scene_entry = XtVaCreateWidget("scene_entry",
				xmFormWidgetClass, scene_editor->main_form,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				NULL);
			XtManageChild(scene_editor->scene_entry);

			scene_editor->scene_label = XtVaCreateWidget("Scene:",
				xmLabelWidgetClass, scene_editor->scene_entry,
				XmNtopAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNfontList, (XtPointer)user_interface->normal_fontlist,
				NULL);
			XtManageChild(scene_editor->scene_label);

			scene_editor->scene_form = XtVaCreateWidget("scene_form",
				xmFormWidgetClass, scene_editor->scene_entry,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, scene_editor->scene_label,
				NULL);
			XtManageChild(scene_editor->scene_form);

			scene_editor->scene_widget = CREATE_CHOOSE_OBJECT_WIDGET(Scene)(
				scene_editor->scene_form, scene, scene_manager,
				(MANAGER_CONDITIONAL_FUNCTION(Scene) *)NULL, (void *)NULL,
				user_interface);

			/* create a paned window to separate the scene list from object editor */

			paned_window = XtVaCreateWidget("panes",
				xmPanedWindowWidgetClass, scene_editor->main_form,
				XmNsashWidth, 50,
				XmNmarginHeight, 0,
				XmNmarginWidth, 0,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, scene_editor->scene_entry,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);
			XtManageChild(paned_window);

			/* create a scrolled_window containing a form to put items in */

			scrolled_window = XtVaCreateWidget("scroll",
				xmScrolledWindowWidgetClass, paned_window,
				XmNheight, 150,
				XmNwidth, 280,
				XmNscrollingPolicy, XmAUTOMATIC,
				XmNallowResize, TRUE,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);
			XtManageChild(scrolled_window);

			scene_editor->list_form = XtVaCreateWidget("list_form",
				xmFormWidgetClass, scrolled_window,
				NULL);
			XtManageChild(scene_editor->list_form);

			scene_editor->object_form = XtVaCreateWidget("object_form",
				xmFormWidgetClass, paned_window,
				XmNallowResize, TRUE,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);
			XtManageChild(scene_editor->object_form);

			controls_form = XtVaCreateWidget("controls",
				xmFormWidgetClass, scene_editor->object_form,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				NULL);
			XtManageChild(controls_form);

			up_arrow_button = XtVaCreateWidget("up",
				xmArrowButtonWidgetClass, controls_form,
				XmNtopAttachment, XmATTACH_FORM,
				XmNtopOffset, 2,
				XmNleftAttachment, XmATTACH_FORM,
				XmNarrowDirection, XmARROW_UP,
				XmNshadowThickness, 1,
				XmNborderWidth, 0,
				NULL);
			XtAddCallback(up_arrow_button, XmNactivateCallback,
				Scene_editor_up_button_callback, (XtPointer)scene_editor);
			XtManageChild(up_arrow_button);

			down_arrow_button = XtVaCreateWidget("down",
				xmArrowButtonWidgetClass, controls_form,
				XmNtopAttachment, XmATTACH_FORM,
				XmNtopOffset, 2,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, up_arrow_button,
				XmNarrowDirection, XmARROW_DOWN,
				XmNshadowThickness, 1,
				XmNborderWidth, 0,
				NULL);
			XtAddCallback(down_arrow_button, XmNactivateCallback,
				Scene_editor_down_button_callback, (XtPointer)scene_editor);
			XtManageChild(down_arrow_button);

			scene_editor->object_label = XtVaCreateWidget("name",
				xmLabelWidgetClass, controls_form,
				XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
				XmNbottomWidget, down_arrow_button,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, down_arrow_button,
				XmNfontList, (XtPointer)user_interface->normal_fontlist,
				NULL);
			XtManageChild(scene_editor->object_label);

			scene_editor->revert_button = XtVaCreateWidget("Revert",
				xmPushButtonWidgetClass, controls_form,
				XmNtopAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNsensitive, (0 == scene_editor->auto_apply),
				XmNshadowThickness, 1,
				XmNborderWidth, 0,
				XmNmarginHeight, 2,
				XmNmarginWidth, 2,
				XmNfontList, (XtPointer)scene_editor->user_interface->normal_fontlist,
				NULL);
			XtAddCallback(scene_editor->revert_button, XmNactivateCallback,
				Scene_editor_revert_button_callback, (XtPointer)scene_editor);
			XtManageChild(scene_editor->revert_button);

			scene_editor->apply_button = XtVaCreateWidget("Apply",
				xmPushButtonWidgetClass, controls_form,
				XmNtopAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_WIDGET,
				XmNrightWidget, scene_editor->revert_button,
				XmNsensitive, (0 == scene_editor->auto_apply),
				XmNshadowThickness, 1,
				XmNborderWidth, 0,
				XmNmarginHeight, 2,
				XmNmarginWidth, 2,
				XmNfontList, (XtPointer)scene_editor->user_interface->normal_fontlist,
				NULL);
			XtAddCallback(scene_editor->apply_button, XmNactivateCallback,
				Scene_editor_apply_button_callback, (XtPointer)scene_editor);
			XtManageChild(scene_editor->apply_button);

			scene_editor->auto_apply_button = XtVaCreateWidget("Auto",
				xmToggleButtonWidgetClass, controls_form,
				XmNtopAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_WIDGET,
				XmNrightWidget, scene_editor->apply_button,
				XmNset, (0 != scene_editor->auto_apply),
				XmNfontList, (XtPointer)scene_editor->user_interface->normal_fontlist,
				NULL);
			XtAddCallback(scene_editor->auto_apply_button, XmNvalueChangedCallback,
				Scene_editor_auto_apply_button_callback, (XtPointer)scene_editor);
			XtManageChild(scene_editor->auto_apply_button);

			scene_editor->content_rowcolumn = XtVaCreateWidget("content_rowcol",
				xmRowColumnWidgetClass, scene_editor->object_form,
				XmNentryBorder, 0,
				XmNorientation, XmHORIZONTAL,
				XmNpacking, XmPACK_TIGHT,
				XmNradioBehavior, TRUE,
				XmNradioAlwaysOne, TRUE,
				XmNspacing, 0,
				XmNmarginWidth, 0,
				XmNmarginHeight, 0,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, controls_form,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_NONE,
				NULL);
			XtAddCallback(scene_editor->content_rowcolumn, XmNentryCallback,
				Scene_editor_content_rowcolumn_entry_callback, (XtPointer)scene_editor);
			XtManageChild(scene_editor->content_rowcolumn);

			scene_editor->transformation_button = XtVaCreateWidget("Transformation",
				xmToggleButtonWidgetClass, scene_editor->content_rowcolumn,
				XmNindicatorOn, FALSE,
				XmNindicatorType, XmN_OF_MANY,
				XmNshadowThickness, 1,
				XmNborderWidth, 0,
				XmNset, (0 != scene_editor->transformation_expanded),
				XmNspacing, 0,
				XmNfontList, (XtPointer)user_interface->normal_fontlist,
				NULL);
			XtManageChild(scene_editor->transformation_button);

			scene_editor->child_button = XtVaCreateWidget("Child object",
				xmToggleButtonWidgetClass, scene_editor->content_rowcolumn,
				XmNindicatorOn, FALSE,
				XmNindicatorType, XmN_OF_MANY,
				XmNshadowThickness, 1,
				XmNborderWidth, 0,
				XmNset, (0 != scene_editor->child_expanded),
				XmNspacing, 0,
				XmNfontList, (XtPointer)user_interface->normal_fontlist,
				NULL);
			XtManageChild(scene_editor->child_button);

			content_frame = XtVaCreateWidget("content_frame",
				xmFrameWidgetClass, scene_editor->object_form,
				XmNshadowType, XmSHADOW_IN,
				XmNmarginWidth, 0,
				XmNmarginHeight, 0,
				XmNshadowThickness, 1,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, scene_editor->content_rowcolumn,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);
			XtManageChild(content_frame);

			content_form = XtVaCreateWidget("content_form",
				xmFormWidgetClass, content_frame,
				NULL);
			XtManageChild(content_form);

			scene_editor->transformation_form = XtVaCreateWidget(
				"transformation_form",
				xmFormWidgetClass, content_form,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, content_frame,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);

			create_transformation_editor_widget(
				&(scene_editor->transformation_editor),
				scene_editor->transformation_form,
				(gtMatrix *)NULL);
			XtManageChild(scene_editor->transformation_editor);

			scene_editor->child_form = XtVaCreateWidget("child_form",
				xmFormWidgetClass, content_form,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, content_frame,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				NULL);

			create_graphical_element_editor_widget(
				&(scene_editor->graphical_element_editor),
				scene_editor->child_form,
				(struct GT_element_group *)NULL,
				computed_field_package, element_manager, fe_field_manager,
				graphical_material_manager, default_material, glyph_list,
				spectrum_manager, default_spectrum,
				volume_texture_manager, user_interface);
			XtUnmanageChild(scene_editor->graphical_element_editor);

			if (scene_editor->scene_editor_objects && scene_editor->window_shell &&
				scene_editor->scene_widget && scene_editor->list_form &&
				scene_editor->transformation_editor &&
				scene_editor->graphical_element_editor)
			{
				Pixel pixel;
				Widget clip_window;

				/* copy background from list_form into scrolled_window's clip_window */
				XtVaGetValues(scene_editor->list_form, XmNbackground, &pixel, NULL);
				XtVaGetValues(scrolled_window, XmNclipWindow, &clip_window, NULL);
				XtVaSetValues(clip_window, XmNbackground, pixel, NULL);

				/* make the widgets for this scene */
				Scene_editor_Scene_update_Scene_editor_objects(scene_editor, scene,
					scene_editor->scene_editor_objects, scene_editor->list_form);

				if (scene_editor->transformation_expanded)
				{
					XtManageChild(scene_editor->transformation_form);
				}
				if (scene_editor->child_expanded)
				{
					XtManageChild(scene_editor->child_form);
				}

				Scene_editor_set_current_object(scene_editor,
					Scene_editor_get_first_object(scene_editor));

				if ((struct Scene_editor_object *)NULL == scene_editor->current_object)
				{
					XtUnmanageChild(scene_editor->object_form);
				}

				/* get callback for change of scene */
				callback.data = (void *)scene_editor;
				callback.procedure = Scene_editor_update_scene;
				CHOOSE_OBJECT_SET_CALLBACK(Scene)(
					scene_editor->scene_widget, &callback);

				/* get callbacks from transformation editor for AutoApply */
				callback.procedure = Scene_editor_update_transformation;
				callback.data = (void *)scene_editor;
				transformation_editor_set_callback(
					scene_editor->transformation_editor, &callback);

				/* get callbacks from graphical element editor for AutoApply */
				callback.procedure = Scene_editor_update_graphical_element;
				callback.data = (void *)scene_editor;
				graphical_element_editor_set_callback(
					scene_editor->graphical_element_editor, &callback);

				/* realize the editor widgets */
				XtRealizeWidget(scene_editor->window_shell);		
				XtPopup(scene_editor->window_shell, XtGrabNone);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Scene_editor).  Could not create widgets");
				DESTROY(Scene_editor)(&scene_editor);
				scene_editor = (struct Scene_editor *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Scene_editor).  Could not allocate editor structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Scene_editor).  Invalid argument(s)");
	}
	if (scene_editor_address && scene_editor)
	{
		scene_editor->scene_editor_address = scene_editor_address;
		*scene_editor_address = scene_editor;
	}
	LEAVE;

	return (scene_editor);
} /* CREATE(Scene_editor) */

int DESTROY(Scene_editor)(struct Scene_editor **scene_editor_address)
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Scene_editor *scene_editor;

	ENTER(DESTROY(Scene_editor));
	if (scene_editor_address && (scene_editor = *scene_editor_address) &&
		(scene_editor->scene_editor_address == scene_editor_address))
	{
		/* must unset the current_object, if any, to remove callbacks */
		Scene_editor_set_current_object(scene_editor,
			(struct Scene_editor_object *)NULL);
		if (scene_editor->scene_manager_callback_id)
		{
			MANAGER_DEREGISTER(Scene)(
				scene_editor->scene_manager_callback_id,
				scene_editor->scene_manager);
			scene_editor->scene_manager_callback_id = (void *)NULL;
		}
		DESTROY(LIST(Scene_editor_object))(&(scene_editor->scene_editor_objects));
		if (scene_editor->window_shell)
		{
			destroy_Shell_list_item_from_shell(&(scene_editor->window_shell),
				scene_editor->user_interface);
			XtDestroyWidget(scene_editor->window_shell);
		}
		DEALLOCATE(*scene_editor_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Scene_editor).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Scene_editor) */

int Scene_editor_bring_to_front(struct Scene_editor *scene_editor)
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
De-iconifies and brings the scene editor to the front.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_editor_bring_to_front);
	if (scene_editor)
	{
		/* bring up the dialog */
		XtPopup(scene_editor->window_shell,XtGrabNone);
		XtVaSetValues(scene_editor->window_shell,XmNiconic,False,NULL);
		return_code=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_bring_to_front.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_bring_to_front */

struct Scene *Scene_editor_get_scene(struct Scene_editor *scene_editor)
/*******************************************************************************
LAST MODIFIED : 5 November 2001

DESCRIPTION :
Returns the root scene of the <scene_editor>.
==============================================================================*/
{
	struct Scene *scene;

	ENTER(Scene_editor_get_scene);
	if (scene_editor)
	{
		scene = CHOOSE_OBJECT_GET_OBJECT(Scene)(scene_editor->scene_widget);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_get_scene.  Invalid argument(s)");
		scene = (struct Scene *)NULL;
	}
	LEAVE;

	return (scene);
} /* Scene_editor_get_scene */

int Scene_editor_set_scene(struct Scene_editor *scene_editor,
	struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
Sets the root scene of the <scene_editor>. Updates widgets.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_editor_set_scene);
	if (scene_editor && scene)
	{
		if (scene == Scene_editor_get_scene(scene_editor))
		{
			return_code = 1;
		}
		else
		{
			if (CHOOSE_OBJECT_SET_OBJECT(Scene)(scene_editor->scene_widget, scene))
			{
				REMOVE_ALL_OBJECTS_FROM_LIST(Scene_editor_object)(
					scene_editor->scene_editor_objects);
				return_code = Scene_editor_Scene_update_Scene_editor_objects(
					scene_editor, scene,
					scene_editor->scene_editor_objects, scene_editor->list_form);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_editor_set_scene.  Could not set new scene");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_set_scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_set_scene */
