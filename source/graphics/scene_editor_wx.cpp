extern "C" {
#include <stdio.h>
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "user_interface/message.h"
}
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include "wx/xrc/xmlres.h"
#include "choose/choose_class.hpp"
#include <wx/collpane.h>
#include <wx/splitter.h>
extern "C" {
#include "graphics/scene_editor_wx.h"
}
#include "graphics/scene_editor_wx.xrch"
#endif /* defined (WX_USER_INTERFACE)*/

/*
Module types
------------
*/

#if defined (WX_USER_INTERFACE)
class wxSceneEditor;
#endif /* defined (WX_USER_INTERFACE) */

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
LAST MODIFIED : 02 Febuary 2007

DESCRIPTION :
==============================================================================*/
{
	/* if autoapply flag is set, any changes to the currently edited graphical
		 element will automatically be applied globally */
	int auto_apply, child_edited, child_expanded, transformation_edited,
		transformation_expanded;

	/* access gt_element_group for current_object if applicable */
	struct GT_element_group *gt_element_group;
	Scene *scene;
	Scene_object *scene_object;
	/* keep address of pointer to editor so can self-destroy */
	struct Scene_editor **scene_editor_address;
	void *scene_manager_callback_id;
	struct MANAGER(Scene) *scene_manager;
	struct User_interface *user_interface;
	struct Computed_field_package *computed_field_package;
#if defined (WX_USER_INTERFACE)
	wxSceneEditor *wx_scene_editor;
	wxScrolledWindow *scene_check_box;
	wxPanel *graphical_element_panel;
	wxPanel *lower_panel;
	wxScrolledWindow *sceneediting;
	wxFrame *frame;
#endif /*defined (WX_USER_INTERFACE)*/
}; /*struct Scene_editor*/


int DESTROY(Scene_editor_object)(
	struct Scene_editor_object **scene_editor_object_address);
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
Prototype.
==============================================================================*/

static int add_scene_object_to_scene_check_box(struct Scene_object *scene_object,
																							 void *scene_editor_void);
/*******************************************************************************
LAST MODIFIED : 2 Match 2007

DESCRIPTION :
Prototype.
==============================================================================*/
/*
Module functions
----------------
*/

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
//			scene_editor_object =
//				FIND_BY_IDENTIFIER_IN_LIST(Scene_editor_object, name)(name,
//					scene_editor->scene_editor_objects);
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


#if defined (WX_USER_INTERFACE)

class wxSceneEditor : public wxFrame
{																								
	Scene_editor *scene_editor;
	wxScrolledWindow *sceneediting;
	wxFrame *frame;
	wxSplitterWindow *lowersplitter;
	wxSplitterWindow *topsplitter;


	DEFINE_MANAGER_CLASS(Scene);
	Managed_object_chooser<Scene,MANAGER_CLASS(Scene)>
		*scene_chooser;

public:

  wxSceneEditor(Scene_editor *scene_editor): 
    scene_editor(scene_editor)
  {
	{	
		wxXmlInit_scene_editor_wx();
	}
		scene_editor->wx_scene_editor = (wxSceneEditor *)NULL;
		wxXmlResource::Get()->LoadFrame(this,
			(wxWindow *)NULL, _T("CmguiSceneEditor"));

 /* Set the chooser panel  in the secne editor */
  wxPanel *scene_object_chooser_panel = 
		XRCCTRL(*this, "SceneObjectChooserPanel", wxPanel);
	scene_chooser = 
		new Managed_object_chooser<Scene,MANAGER_CLASS(Scene)>
	  (scene_object_chooser_panel, scene_editor->scene, scene_editor->scene_manager,
	  (MANAGER_CONDITIONAL_FUNCTION(Scene) *)NULL, (void *)NULL, scene_editor->user_interface);
	  Callback_base<Scene> *scene_object_callback = 
		  new Callback_member_callback< Scene, 
		  wxSceneEditor, int (wxSceneEditor::*)(Scene *) >
		  (this, &wxSceneEditor::scene_object_callback);
      scene_chooser->set_callback(scene_object_callback);

 /* Set the collapsible pane in the secne editor */
  wxCollapsiblePane *collpane = XRCCTRL(*this, "CollapsiblePane", wxCollapsiblePane);
  wxPanel *GeneralSettingPanel = new wxPanel;
  wxWindow *win = collpane->GetPane();
	wxXmlResource::Get()->LoadPanel(GeneralSettingPanel,
			win, _T("CmguiSceneEditorGeneralSettings"));
   wxSizer *paneSz = new wxBoxSizer(wxVERTICAL);
	paneSz->Add(GeneralSettingPanel, 1, wxEXPAND|wxALL, 2);
    win->SetSizer(paneSz);
    paneSz->SetSizeHints(win);
	 collpane->Collapse(1);
	 wxPanel *lowestpanel = 
		 XRCCTRL(*this, "CmguiSceneEditor", wxPanel);
	 lowestpanel->Fit();
	wxFrame *frame=XRCCTRL(*this, "CmguiSceneEditor", wxFrame);
			frame->Fit();

 Show();
};

  wxSceneEditor()
  {
  };

  ~wxSceneEditor()
  {
	  delete scene_chooser;
  }

	int scene_object_callback(Scene *scene)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Callback from wxChooser<Scene> when choice is made.
==============================================================================*/
	{
		Scene_editor_set_scene(scene_editor, scene);
		scene_editor->lower_panel->Hide();
		wxPanel *panel = scene_editor-> scene_check_box;
		panel->DestroyChildren();
		for_each_Scene_object_in_Scene(scene,
																	 add_scene_object_to_scene_check_box, (void *)scene_editor);
		return 1;
	}


	int setSceneObject(Scene *scene)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Set the selected option in the Scene Object chooser.
==============================================================================*/
	{
		scene_chooser->set_object(scene);
		return 1;
	}

      void ResetScrolledWindow(wxCollapsiblePaneEvent& event)
  	{
			lowersplitter=XRCCTRL(*this,"LowerSplitter",wxSplitterWindow);
			  lowersplitter->Fit();
				lowersplitter->Layout();	
			frame = 
					XRCCTRL(*this, "CmguiSceneEditor", wxFrame);
			frame->Fit();	
			frame->Layout();
			frame->SetMinSize(wxSize(50,500));
			frame->SetMaxSize(wxSize(2000,2000));
	}

      void ResetWindow(wxSplitterEvent& event)
  	{
			sceneediting = 
					XRCCTRL(*this, "SceneEditing", wxScrolledWindow);
			sceneediting->Fit();	
			sceneediting->Layout();
			sceneediting->SetScrollbars(10,10,40,40);
			lowersplitter=XRCCTRL(*this,"LowerSplitter",wxSplitterWindow);
			  lowersplitter->Fit();
				lowersplitter->Layout();	
			frame = 
					XRCCTRL(*this, "CmguiSceneEditor", wxFrame);
			frame->Fit();	
			frame->Layout();
			frame->SetMinSize(wxSize(50,100));
		}

  DECLARE_DYNAMIC_CLASS(wxSceneEditor);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxSceneEditor, wxFrame)

BEGIN_EVENT_TABLE(wxSceneEditor, wxFrame)
	EVT_SPLITTER_SASH_POS_CHANGED(XRCID("LowerSplitter"),wxSceneEditor::ResetWindow)
	EVT_COLLAPSIBLEPANE_CHANGED(XRCID("CollapsiblePane"), wxSceneEditor::ResetScrolledWindow)
END_EVENT_TABLE()

class wxInteractiveCheckBox : public wxCheckBox
{
   Scene_object *object;
  	Scene_editor *scene_editor;
  
  
public:

  wxInteractiveCheckBox(Scene_object *object, Scene_editor *scene_editor) :
    object(object), scene_editor(scene_editor)
  {
  }

  ~wxInteractiveCheckBox()
  {
  }


  void OnInteractiveCheckBoxPressed(wxCommandEvent& Event)
 {
		if (this->GetValue())
			{
				Scene_object_set_visibility(object, g_VISIBLE);
			}
		else
			{
				Scene_object_set_visibility(object, g_INVISIBLE);
			}
  }

};


static int add_scene_object_to_scene_check_box(struct Scene_object *scene_object,
					      void *scene_editor_void)
/*******************************************************************************
LAST MODIFIED : 2 Match 2007

DESCRIPTION :
Add scene_object as checkbox item into the box
==============================================================================*/
{
	Scene_editor *scene_editor = static_cast<Scene_editor*>(scene_editor_void);
	wxPanel *panel = scene_editor-> scene_check_box;
	wxSizer *sizer = panel->GetSizer();
	char *name;
	int visible;
	wxInteractiveCheckBox *checkbox = new wxInteractiveCheckBox(scene_object,scene_editor);

	GET_NAME(Scene_object)(scene_object, &name);
	checkbox->Create(panel,/*id*/-1,name); 
	checkbox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,wxCommandEventHandler(wxInteractiveCheckBox::OnInteractiveCheckBoxPressed));
	sizer->Add(checkbox,wxSizerFlags(0).Align(0).Expand().Border(wxALL,2));
	panel->Layout();
  	DEALLOCATE(name);
		visible =(g_VISIBLE == Scene_object_get_visibility(scene_object));
		if ( visible ==1)
			{
				checkbox->SetValue(1);
			}
		scene_editor->lower_panel->Show();
return(1);
 };
#endif /* defined (WX_USER_INTERFACE) */

/*
Global functions
----------------
*/

struct Scene_editor *CREATE(Scene_editor)(	
	struct Scene_editor **scene_editor_address,
	struct MANAGER(Scene) *scene_manager, struct Scene *scene,
   	struct Scene_object *scene_object,
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct Graphics_font *default_font,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
  struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 2 Febuary 2007

DESCRIPTION :
==============================================================================*/
{
	struct Scene_editor *scene_editor;

	ENTER(CREATE(Scene_editor));
	scene_editor = (struct Scene_editor *)NULL;
	if (scene_manager && scene && computed_field_package && root_region &&
		graphical_material_manager && default_material &&
		glyph_list && spectrum_manager && default_spectrum &&
		volume_texture_manager && user_interface)
	{
		if (ALLOCATE(scene_editor,struct Scene_editor,1))
		{		
			scene_editor->auto_apply = 1;
			scene_editor->child_edited =1;
			scene_editor->child_expanded=1; 
			scene_editor->transformation_edited=1;
			scene_editor->transformation_expanded=1;		
			scene_editor->gt_element_group = (struct GT_element_group *)NULL;
			scene_editor->scene = scene;
    		scene_editor->scene_object = scene_object,
			scene_editor->scene_editor_address = (struct Scene_editor **)NULL;
			scene_editor->scene_manager_callback_id = (void *)NULL;
			scene_editor->scene_manager = scene_manager;
			scene_editor->user_interface=user_interface;
			scene_editor->computed_field_package=(struct Computed_field_package *)NULL;
		
#if defined (WX_USER_INTERFACE)
		
	scene_editor->wx_scene_editor = (wxSceneEditor *)NULL;
	scene_editor->wx_scene_editor = new 
	wxSceneEditor(scene_editor);
	
 	scene_editor->scene_check_box = 
 	  XRCCTRL(*scene_editor->wx_scene_editor, "SceneCheckBox", wxScrolledWindow);
 	wxBoxSizer *scene_check_box_sizer = new wxBoxSizer( wxVERTICAL );
	scene_editor->graphical_element_panel=
		XRCCTRL(*scene_editor->wx_scene_editor, "GraphicalElementPanel", wxPanel);
	scene_editor->lower_panel=
		XRCCTRL(*scene_editor->wx_scene_editor, "LowerPanel", wxPanel);
	scene_editor->scene_check_box->SetSizer(scene_check_box_sizer);
	scene_editor->lower_panel->Hide();
	scene_editor->scene_check_box->SetScrollbars(10,10,100,100);
	for_each_Scene_object_in_Scene(scene,
		add_scene_object_to_scene_check_box, (void *)scene_editor);
 	scene_editor->frame = 
 	  XRCCTRL(*scene_editor->wx_scene_editor, "CmguiSceneEditor", wxFrame);
	scene_editor->frame->Fit();
	scene_editor->frame->Layout();
	scene_editor->frame->SetMinSize(wxSize(50,-1));
 	scene_editor->sceneediting = 
 	  XRCCTRL(*scene_editor->wx_scene_editor, "SceneEditing", wxScrolledWindow);
	scene_editor->sceneediting->Fit();
	scene_editor->sceneediting->Layout();
	scene_editor->sceneediting->SetScrollbars(10,10,40,40);
#endif /*  (WX_USER_INTERFACE) */

		}
	}

	LEAVE;

	return (scene_editor);
} /* CREATE(Scene_editor_wx) */

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
		//Scene_editor_set_current_object(scene_editor,
		//	(struct Scene_editor_object *)NULL);
		if (scene_editor->scene_manager_callback_id)
		{
			MANAGER_DEREGISTER(Scene)(
				scene_editor->scene_manager_callback_id,
				scene_editor->scene_manager);
			scene_editor->scene_manager_callback_id = (void *)NULL;
		}
	//DESTROY(LIST(Scene_editor_object))(&(scene_editor->scene_editor_objects));
		//if (scene_editor->window_shell)
		//{
		//	destroy_Shell_list_item_from_shell(&(scene_editor->window_shell),
		//		scene_editor->user_interface);
		//}
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
		scene_editor->wx_scene_editor->Raise();
		return_code=1;
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
	//	scene = CHOOSE_OBJECT_GET_OBJECT(Scene)(scene_editor->scene_widget);
		//scene_editor->wx_scene_editor->Raise();
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
#if defined (WX_USER_INTERFACE)
			if (scene_editor->wx_scene_editor)
			{
				scene_editor->wx_scene_editor->setSceneObject(scene);
			}
#endif /* defined (WX_USER_INTERFACE) */

	 {
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


