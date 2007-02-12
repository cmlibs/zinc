







extern "C" {
#include <stdio.h>
// #include <Xm/Xm.h>
// #include <Xm/XmP.h>
// #include <Xm/ArrowB.h>
// #include <Xm/Form.h>
// #include <Xm/FormP.h>
// #include <Xm/Frame.h>
// #include <Xm/PanedW.h>
// #include <Xm/Protocols.h>
// #include <Xm/PushB.h>
// #include <Xm/Label.h>
// #include <Xm/RowColumn.h>
// #include <Xm/ScrolledW.h>
// #include <Xm/Separator.h>
// #include <Xm/ToggleB.h>
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
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include "wx/xrc/xmlres.h"
#include "graphics/scene_editor_wx.xrch"
#endif /* defined (WX_USER_INTERFACE)*/
}
/*
Module types
------------
*/

#if defined (WX_USER_INTERFACE)
class wxSceneEditor;
#endif /* defined (WX_USER_INTERFACE) */

struct Scene_editor
/*******************************************************************************
LAST MODIFIED : 02 Febuary 2007

DESCRIPTION :
==============================================================================*/
{
#if defined (WX_USER_INTERFACE)
	wxSceneEditor *wx_scene_editor;
#endif /*defined (WX_USER_INTERFACE)*/
}; /*struct Scene_editor*/

/*
Module functions
----------------
*/

#if defined (WX_USER_INTERFACE)
class wxSceneEditor : public wxFrame
{																								
	Scene_editor *scene_editor;

public:

  wxSceneEditor(Scene_editor *scene_editor): 
    scene_editor(scene_editor)
  {	 
  };

  wxSceneEditor()
  {
  };


  DECLARE_DYNAMIC_CLASS(wxSceneEditor);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxSceneEditor, wxFrame)

BEGIN_EVENT_TABLE(wxSceneEditor, wxFrame)
END_EVENT_TABLE()

#endif /* defined (WX_USER_INTERFACE) */



/*
Global functions
----------------
*/

	struct Scene_editor *CREATE(Scene_editor)(	
  struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 2 Febuary 2007

DESCRIPTION :
==============================================================================*/
{
	struct Scene_editor *scene_editor;


	ENTER(CREATE(Scene_editor));
	if (user_interface)
	{
		if (ALLOCATE(scene_editor,struct Scene_editor,1))
		{		
#if defined (WX_USER_INTERFACE)
			wxXmlInit_scene_editor();
			scene_editor->wx_scene_editor = new 
				wxSceneEditor(scene_editor);
			wxXmlResource::Get()->LoadFrame(scene_editor->wx_scene_editor,
				(wxWindow *)NULL, _T("CmguiSceneEditor"));
#endif /*  (WX_USER_INTERFACE) */
		}
	}
	LEAVE;

	return (scene_editor);
} /* CREATE(Scene_editor_wx) */
