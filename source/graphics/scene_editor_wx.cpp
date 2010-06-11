/*******************************************************************************
FILE : scene_editor_wx.cpp

LAST MODIFIED : 26 February 2007

DESCRIPTION :
codes used to build scene editor with wxWidgets.
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

#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif
#include <string>
extern "C" {
#include <stdio.h>
#include "api/cmiss_rendition.h"
#include "api/cmiss_graphic.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "user_interface/message.h"
}
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include <wx/treectrl.h>
#include "wx/xrc/xmlres.h"
#include "choose/choose_manager_class.hpp"
#include "choose/choose_enumerator_class.hpp"
#include "choose/choose_list_class.hpp"
#include "choose/text_choose_from_fe_element.hpp"
#include "transformation/transformation_editor_wx.hpp"
#include <wx/collpane.h>
#include <wx/splitter.h>
#include <wx/imaglist.h>
#include "icon/cmiss_icon.xpm"
#include "icon/tickbox.xpm"
#include "icon/unticked_box.xpm"
extern "C" {
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field.h"
#include "graphics/scene_editor_wx.h"
#include "graphics/font.h"
#include "graphics/cmiss_rendition.h"
#include "graphics/cmiss_graphic.h"
}
#include "graphics/scene_editor_wx.xrch"

#define GENERAL_SETTINGS
/*
Module types
------------
*/
class wxSceneEditor;

enum
{
	CmguiTree_MenuItem1,
	CmguiTree_MenuItem2,
	CmguiTree_MenuItem3,
	CmguiTree_MenuItem4,
	AddMenuItemNode,
	AddMenuItemData,
	AddMenuItemLines,
	AddMenuItemCylinders,
	AddMenuItemSurfaces,
	AddMenuItemIsoSurfaces,
	AddMenuItemElement,
	AddMenuItemStreamlines,
	AddMenuItemStaticGraphic,
	CmguiTree_Ctrl = 1000
};

class wxCmguiHierachicalTree;

class wxCmguiHierachicalTreeItemData :public wxTreeItemData
{
	struct Cmiss_region *region;
 	wxCmguiHierachicalTree *treectrl;

public:
	wxCmguiHierachicalTreeItemData(struct Cmiss_region *input_region,
		wxCmguiHierachicalTree *input_treectrl) : treectrl(input_treectrl)
	{
		region = ACCESS(Cmiss_region)(input_region);
		treectrl = input_treectrl;
		Cmiss_region_add_callback(region,
			propagate_region_change, (void *)this);
	}
	
	virtual ~wxCmguiHierachicalTreeItemData()
	{
		Cmiss_region_remove_callback(region,
			propagate_region_change, (void *)this);
		DEACCESS(Cmiss_region)(&region);
	}

	Cmiss_region *GetRegion()
	{
		return region;
	}

	static void propagate_region_change(
		struct Cmiss_region *region,struct Cmiss_region_changes *region_changes, void *data_void);
};

class wxCmguiHierachicalTree : public wxTreeCtrl
{
	wxSceneEditor *scene_editor_widget;
public:
	wxCmguiHierachicalTree(wxSceneEditor *scene_editor_widget, wxPanel *parent) :
		wxTreeCtrl(parent, CmguiTree_Ctrl, wxDefaultPosition, wxDefaultSize,
							 wxTR_HAS_BUTTONS|wxTR_MULTIPLE), scene_editor_widget(scene_editor_widget)
	{
		wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(this, wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
		parent->SetSizer(sizer);
		Show();
		Connect(wxEVT_LEFT_DOWN,
			wxMouseEventHandler(wxCmguiHierachicalTree::SendLeftDownEvent),
			NULL,this);
	}
	
	wxCmguiHierachicalTree()
	{
	};

	~wxCmguiHierachicalTree()
	{
	};

	void SetTreeIdRegionWithCallback(wxTreeItemId id, Cmiss_region *region)
	{
		wxCmguiHierachicalTreeItemData *data = 
			new wxCmguiHierachicalTreeItemData(region, this);
		SetItemData(id, data);
	}

	void remove_child_with_region(wxTreeItemId parent_id, Cmiss_region *child_region)
	{
		wxTreeItemIdValue cookie;
		wxTreeItemId child_id = GetFirstChild(parent_id, cookie);
		Cmiss_region *parent_region = dynamic_cast<wxCmguiHierachicalTreeItemData *>
			(GetItemData(parent_id))->GetRegion();
		while (child_id.IsOk())
		{
			/* if child_region is NULL then find an item with region that does not
				 belong to the parent region and remove it from the tree */
			Cmiss_region * current_region = dynamic_cast<wxCmguiHierachicalTreeItemData *>
				(GetItemData(child_id))->GetRegion();
			if ((child_region && child_region == current_region)
				|| (!child_region && !Cmiss_region_contains_subregion(parent_region, current_region)))
			{
				if (IsSelected(child_id))
					SelectItem(child_id, false);
				Delete(child_id);
			}
			child_id = GetNextChild(parent_id, cookie);
		}
	}

	void update_current_tree_item(wxTreeItemId parent_id)
	{
		int i = 0, return_code = 0;
		Cmiss_region *parent_region = dynamic_cast<wxCmguiHierachicalTreeItemData *>
			(GetItemData(parent_id))->GetRegion();
		char *child_name;
		Cmiss_region *child_region = NULL;
		wxTreeItemIdValue cookie;
		child_region = Cmiss_region_get_first_child(parent_region);
		while (child_region)
		{
			child_name = Cmiss_region_get_name(child_region);
			wxTreeItemId child_id = GetFirstChild(parent_id, cookie);
			while (child_id.IsOk() && !return_code)
			{
				Cmiss_region *current_region = dynamic_cast<wxCmguiHierachicalTreeItemData *>
					(GetItemData(child_id))->GetRegion();
				if (current_region == child_region)
				{
					return_code = 1;
				}
				child_id = GetNextChild(parent_id, cookie);
			}
			if (!return_code)
			{
				child_id = InsertItem(parent_id, i, child_name,0,0);
				SetTreeIdRegionWithCallback(child_id, child_region);
			}
			DEALLOCATE(child_name);
			Cmiss_region_reaccess_next_sibling(&child_region);
			i++;
		}
	}

	void region_change(struct Cmiss_region *region,
		struct Cmiss_region_changes *region_changes, wxCmguiHierachicalTreeItemData *data)
	{
		ENTER(region_change);

		if (region && region_changes && data)
		{
			if (region_changes->children_changed)
			{
				const wxTreeItemId parent_id = data->GetId();
				struct Cmiss_region *child_region = NULL;
				if (region_changes->child_added)
				{
					child_region = region_changes->child_added;
					char *name = Cmiss_region_get_name(child_region);
					wxTreeItemId child_id = AppendItem(parent_id,name,0,0);
					SetTreeIdRegionWithCallback(child_id, child_region);
					DEALLOCATE(name);
				}
				else if (region_changes->child_removed)
				{
					child_region = region_changes->child_removed;
					remove_child_with_region(parent_id, child_region);
				}
				else
				{
					remove_child_with_region(parent_id, NULL);
					update_current_tree_item(parent_id);
				}
			}	
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_Cmiss_region_change.  Invalid argument(s)");
		}
		LEAVE;
	}

void add_all_child_regions_to_tree_item(wxTreeItemId parent_id)
{
	char *child_name;
	wxTreeItemId child_id;
	Cmiss_region *parent_region, *child_region;
	ENTER(Scene_edito);
	
	parent_region = dynamic_cast<wxCmguiHierachicalTreeItemData *>
			(GetItemData(parent_id))->GetRegion();
	child_region = Cmiss_region_get_first_child(parent_region);
	while (child_region)
	{
		child_name = Cmiss_region_get_name(child_region);
		if (child_name)
		{
			child_id = AppendItem(parent_id,child_name,0,0);
			SetTreeIdRegionWithCallback(child_id, child_region);
			add_all_child_regions_to_tree_item(child_id);
			DEALLOCATE(child_name);
		}
		Cmiss_region_reaccess_next_sibling(&child_region);
	}
	

	LEAVE;
}

private:
	void SendLeftDownEvent(wxMouseEvent& event);

// 	void SendRightDownEvent(wxMousEvent& event);

	DECLARE_DYNAMIC_CLASS(wxCmguiHierachicalTree);
};

IMPLEMENT_DYNAMIC_CLASS(wxCmguiHierachicalTree, wxFrame)

void wxCmguiHierachicalTreeItemData::propagate_region_change(
	struct Cmiss_region *region,struct Cmiss_region_changes *region_changes, void *data_void)
{
	wxCmguiHierachicalTreeItemData *data = 
		static_cast<wxCmguiHierachicalTreeItemData *>(data_void);
	data->treectrl->region_change(region,
		region_changes, data);
}

struct Scene_editor
/*******************************************************************************
LAST MODIFIED : 02 Febuary 2007

DESCRIPTION :
==============================================================================*/
{
	/* if autoapply flag is set, any changes to the currently edited graphical
		 element will automatically be applied globally */
	int auto_apply, child_edited, child_expanded,
		transformation_expanded, transformation_callback_flag,
		gt_element_group_callback_flag, rendition_callback_flag;
	/* access gt_element_group for current_object if applicable */
	struct Cmiss_rendition *rendition, *edit_rendition;
	Scene *scene;
	/* keep address of pointer to editor so can self-destroy */
	struct Scene_editor **scene_editor_address;
	 struct MANAGER(Graphical_material) *graphical_material_manager;
	 struct Graphical_material *default_material, *selected_material;
	 struct Graphics_font *default_font;
	 struct MANAGER(Scene) *scene_manager;
	 struct User_interface *user_interface;
	 enum Cmiss_graphic_type current_graphic_type;
	 struct Cmiss_graphic *current_graphic;
	 struct LIST(GT_object) *glyph_list;	
	 struct MANAGER(VT_volume_texture) *volume_texture_manager;
	 struct MANAGER(Computed_field) *field_manager;
	 struct MANAGER(Graphics_font) *font_manager;
	 struct FE_field *native_discretization_field;
	 struct Computed_field *default_coordinate_field, *coordinate_field, *radius_scalar_field ;
	struct Cmiss_region *root_region, *current_region;
	 enum Graphics_select_mode select_mode;
	 enum Use_element_type use_element_type;
	 enum Xi_discretization_mode xi_discretization_mode;
	 enum Streamline_type streamline_type;
	 enum Streamline_data_type streamline_data_type;
	 float constant_radius,radius_scale_factor;
	 struct MANAGER(Spectrum) *spectrum_manager;
	 struct Spectrum *spectrum;
	 enum Render_type render_type;
	 struct FE_element *fe_element;
#if defined (WX_USER_INTERFACE)
	 Transformation_editor *transformation_editor;
	 wxSceneEditor *wx_scene_editor;
	 wxPanel *top_collpane_panel;
	 wxScrolledWindow *sceneediting;
	 wxFrame *frame;
	wxCheckListBox *checklistbox;
	wxCheckListBox *graphiclistbox;
	 wxCmguiHierachicalTree *testing_tree_ctrl;
	 wxImageList *ImageList;
	 wxSplitterWindow *lowersplitter;
	 wxSplitterWindow *topsplitter;
	 wxCheckBox *autocheckbox;
	 wxButton *applybutton;
	 wxButton *revertbutton;
	 wxCollapsiblePane *collpane;
	 wxCollapsiblePane *top_collpane;
#endif /*defined (WX_USER_INTERFACE)*/
}; /*struct Scene_editor*/

static int set_general_graphic(struct Scene_editor *scene_editor);
/*******************************************************************************
LAST MODIFIED : 16 Match 2007

DESCRIPTION :
Prototype.
==============================================================================*/

void Scene_editor_wx_transformation_change(struct Cmiss_rendition *rendition,
	 gtMatrix *transformation_matrix, void *scene_editor_void);

/***************************************************************************//**
* Revert changes done on the edit gt element group.
* 
*/
int Scene_editor_revert_changes(Scene_editor *scene_editor);

void Scene_editor_set_active_rendition(
	struct Scene_editor *scene_editor, struct Cmiss_rendition *rendition);

// new prototypes
static int Scene_editor_add_graphic_item(
	struct Cmiss_graphic *graphic, void *scene_editor_void);

/***************************************************************************//**
 *Get and set the display of graphic
 */
static int get_and_set_Cmiss_graphic_widgets(void *scene_editor_void);

/***************************************************************************//**
 * Iterator function for Rendition_editor_update_Graphic_item.
 */
static int Scene_editor_add_graphic(
	struct Cmiss_graphic *graphic, void *scene_editor_void);

void Scene_editor_set_graphic_widgets_for_rendition(Scene_editor *scene_editor);

/*
Module functions
----------------
*/

/***************************************************************************//**
 * This function will be called whenever there are global changes on rendition
 */
static int Scene_editor_wx_rendition_change(
	struct Cmiss_rendition *rendition, void *scene_editor_void);

/***************************************************************************//**
* Callback function in scene_editor_wx when object's transformation has been
* changed
*
*/
void Scene_editor_wx_transformation_change(struct Cmiss_rendition *rendition,
	 gtMatrix *transformation_matrix, void *scene_editor_void)
{
	 struct Scene_editor *scene_editor;
	 
	 if (scene_editor = (struct Scene_editor *)scene_editor_void)
	 {
		 if (rendition == scene_editor->rendition)
		 {
			 /* transformation_matrix can be null here which acutally indicates that
					the scene object has not been transformed. */
			 scene_editor->transformation_editor->
				 set_transformation(transformation_matrix);
		 }
	 }
	 else
	 {
		 display_message(ERROR_MESSAGE,
			 "Scene_editor_wx_transformation_change.  Invalid argument(s)");
	 }
}

#if defined (WX_USER_INTERFACE)

#if defined (__WXMSW__)
struct Scene_editor_size
{
	int previous_width, previous_height, current_width, current_height;
};
#endif /* defined (__WXMSW__) */

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_TREE_IMAGE_CLICK_EVENT, -1)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(wxEVT_TREE_IMAGE_CLICK_EVENT)

void wxCmguiHierachicalTree::SendLeftDownEvent(wxMouseEvent& event)
{
	int flags;
	wxTreeItemId id = HitTest( event.GetPosition(), flags ); 
	if(flags & wxTREE_HITTEST_ONITEMICON)
	{
		this->SelectItem(id);
		wxTreeEvent event(wxEVT_TREE_IMAGE_CLICK_EVENT, this, id);
		event.SetEventObject( this );
		// Send it
		GetEventHandler()->ProcessEvent(event);
	}
	else
	{
		event.Skip();
	}
};


class wxSceneEditor : public wxFrame
{	
	Scene_editor *scene_editor;
#if defined (__WXMSW__)
	Scene_editor_size scene_editor_size;
#endif /* defined (__WXMSW__) */
	wxScrolledWindow *lowest_panel, *sceneediting;
	wxFrame *frame;
	wxSplitterWindow *lowersplitter,*topsplitter;
	wxCheckListBox *scenechecklist,*graphicalitemschecklist;
	wxListBox *scenelistbox,*graphicalitemslistbox;
	wxStaticText *currentsceneobjecttext,*constantradius,*scalefactor,
		*isoscalartext, *glyphtext, *centretext, *baseglyphsizetext, *selectmodetext,
		*glyphscalefactorstext, *useelementtypetext, *xidiscretizationmodetext,
		*discretizationtext, *circlediscretizationtext,*densityfieldtext,*xitext, 
		*streamtypetext, *streamlengthtext, *streamwidthtext, *streamvectortext, 
		*linewidthtext, *streamlinedatatypetext, *spectrumtext, *rendertypetext, *fonttext;
	wxButton *sceneupbutton, scenedownbutton, *applybutton, *revertbutton;
	wxCheckBox *nativediscretizationcheckbox,*autocheckbox,	*coordinatefieldcheckbox,
		*radiusscalarcheckbox, *orientationscalecheckbox,*variablescalecheckbox,
		*labelcheckbox,*visibility_field_checkbox,*nativediscretizationfieldcheckbox,
		*reversecheckbox,*datacheckbox,*texturecoordinatescheckbox,*exteriorcheckbox,
		*facecheckbox, *seedelementcheckbox;	
	wxRadioButton *isovaluelistradiobutton, *isovaluesequenceradiobutton;
	wxPanel *isovalueoptionspane;
	wxTextCtrl *elementdiscretisationpanel, *nametextfield, *circlediscretisationpanel,
		*constantradiustextctrl, *scalefactorstextctrl, *isoscalartextctrl, *centretextctrl,
		*baseglyphsizetextctrl,*glyphscalefactorstextctrl,*discretizationtextctrl,
		*circlediscretizationtextctrl,*xitextctrl,*lengthtextctrl,*widthtextctrl,
		*linewidthtextctrl,*isovaluesequencenumbertextctrl, *isovaluesequencefirsttextctrl,
		*isovaluesequencelasttextctrl;
	wxPanel  *FE_chooser_panel,	*coordinate_field_chooser_panel, *data_chooser_panel,
		*radius_scalar_chooser_panel, *iso_scalar_chooser_panel, *glyph_chooser_panel,
		*orientation_scale_field_chooser_panel, *variable_scale_field_chooser_panel,
		 *label_chooser_panel, *font_chooser_panel, *select_mode_chooser_panel,
		 *use_element_type_chooser_panel, *xi_discretization_mode_chooser_panel,
		 *native_discretization_field_chooser_panel, *density_field_chooser_panel, 
		 *streamline_type_chooser_panel, *stream_vector_chooser_panel, 
		 *streamline_data_type_chooser_panel, *spectrum_chooser_panel,
		 *texture_coordinates_chooser_panel, *render_type_chooser_panel,
		 *seed_element_panel, *visibility_field_chooser_panel;
	wxWindow *glyphbox,*glyphline;
	wxChoice *facechoice;
	wxString TempText;
	DEFINE_MANAGER_CLASS(Scene);
	Managed_object_chooser<Scene,MANAGER_CLASS(Scene)>
		*scene_chooser;
	DEFINE_MANAGER_CLASS(Computed_field);
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 *default_coordinate_field_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 *FE_field_chooser;	
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*coordinate_field_chooser;
	DEFINE_MANAGER_CLASS(Graphical_material);
	Managed_object_chooser<Graphical_material,MANAGER_CLASS(Graphical_material)>
	*graphical_material_chooser;
	Managed_object_chooser<Graphical_material,MANAGER_CLASS(Graphical_material)>
	*selected_material_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Cmiss_graphic_type);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphic_type)>
	*graphic_type_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Graphics_select_mode);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Graphics_select_mode)>
	*select_mode_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*data_field_chooser;
	DEFINE_MANAGER_CLASS(Spectrum);
	Managed_object_chooser<Spectrum,MANAGER_CLASS(Spectrum)>
	 *spectrum_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 *radius_scalar_chooser;	
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 *iso_scalar_chooser;	
	DEFINE_LIST_CLASS(GT_object);
	List_chooser<GT_object,LIST_CLASS(GT_object)>
	 *glyph_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*orientation_scale_field_chooser;		
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*variable_scale_field_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*label_field_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*visibility_field_chooser;
	 DEFINE_MANAGER_CLASS(Graphics_font); 
	 	 Managed_object_chooser<Graphics_font,MANAGER_CLASS(Graphics_font)>
	 *font_chooser;	
	DEFINE_ENUMERATOR_TYPE_CLASS(Use_element_type);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Use_element_type)>
		*use_element_type_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Xi_discretization_mode);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Xi_discretization_mode)>
		*xi_discretization_mode_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 *native_discretization_field_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 *xi_point_density_field_chooser;
	 wxFeElementTextChooser *seed_element_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Streamline_type);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Streamline_type)>
	*streamline_type_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*stream_vector_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Streamline_data_type);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Streamline_data_type)>
	*streamline_data_type_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 *texture_coord_field_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Render_type);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Render_type)>
	*render_type_chooser;

public:

  wxSceneEditor(Scene_editor *scene_editor): 
		scene_editor(scene_editor)
  {
		 wxXmlInit_scene_editor_wx();
		 scene_editor->wx_scene_editor = (wxSceneEditor *)NULL;
		 wxXmlResource::Get()->LoadFrame(this,
				(wxWindow *)NULL, _T("CmguiSceneEditor"));
		 this->SetIcon(cmiss_icon_xpm);

#if defined (__WXMSW__)
		 scene_editor_size.previous_width = 0;
		 scene_editor_size.previous_height = 0;
		 scene_editor_size.current_width = 0;
		 scene_editor_size.current_height = 0;
#endif /* defined (__WXMSW__) */

 /* Set the collapsible pane in the secne editor */
  scene_editor->collpane = XRCCTRL(*this, "CollapsiblePane", wxCollapsiblePane);
  wxPanel *GeneralSettingPanel = new wxPanel;
  wxWindow *win = scene_editor->collpane->GetPane();
  wxXmlResource::Get()->LoadPanel(GeneralSettingPanel,
	  win, _T("CmguiSceneEditorGeneralSettings"));
  wxSizer *paneSz = new wxBoxSizer(wxVERTICAL);
  paneSz->Add(GeneralSettingPanel, 1, wxEXPAND|wxALL, 2);
  win->SetSizer(paneSz);
  paneSz->SetSizeHints(win);
  scene_editor->collpane->Collapse(1);

	/* Set the default_coordinate_field_chooser_panel*/
	default_coordinate_field_chooser = NULL;
	/* Set the native_discretisation_chooser_panel*/
	FE_field_chooser = NULL;
	/* Set the coordinate_field_chooser_panel*/
	coordinate_field_chooser = NULL;
	/* Set the graphical_material_chooser_panel*/
	wxPanel *graphical_material_chooser_panel =
		 XRCCTRL(*this, "GraphicalMaterialChooserPanel",wxPanel);
	graphical_material_chooser = 
		 new Managed_object_chooser<Graphical_material,MANAGER_CLASS(Graphical_material)>
		 (graphical_material_chooser_panel, scene_editor->default_material, scene_editor->graphical_material_manager,
				(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL, (void *)NULL, scene_editor->user_interface);
	Callback_base< Graphical_material* > *graphical_material_callback = 
		 new Callback_member_callback< Graphical_material*, 
				wxSceneEditor, int (wxSceneEditor::*)(Graphical_material *) >
		 (this, &wxSceneEditor::graphical_material_callback);
	graphical_material_chooser->set_callback(graphical_material_callback);
	graphical_material_chooser_panel->Fit();
	/* Set the selected_material_chooser_panel*/
	wxPanel *selected_material_chooser_panel =
		 XRCCTRL(*this, "SelectedMaterialChooserPanel",wxPanel);
	selected_material_chooser = 
		 new Managed_object_chooser<Graphical_material,MANAGER_CLASS(Graphical_material)>
		 (selected_material_chooser_panel, scene_editor->selected_material, scene_editor->graphical_material_manager,
				(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL, (void *)NULL, scene_editor->user_interface);
	Callback_base< Graphical_material* > *selected_material_callback = 
		 new Callback_member_callback< Graphical_material*, 
				wxSceneEditor, int (wxSceneEditor::*)(Graphical_material *) >
		 (this, &wxSceneEditor::selected_material_callback);
	selected_material_chooser->set_callback(selected_material_callback);
	selected_material_chooser_panel->Fit();
	wxPanel *graphic_type_chooser_panel =
		 XRCCTRL(*this, "TypeFormChooser", wxPanel);
	graphic_type_chooser = 
		 new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphic_type)>
		 (graphic_type_chooser_panel, 
				scene_editor->current_graphic_type,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_graphic_type) *)NULL,
				(void *)NULL, scene_editor->user_interface);
  graphic_type_chooser_panel->Fit();
	Callback_base< enum Cmiss_graphic_type > *graphic_type_callback = 
		 new Callback_member_callback< enum Cmiss_graphic_type, 
				wxSceneEditor, int (wxSceneEditor::*)(enum Cmiss_graphic_type) >
		 (this, &wxSceneEditor::Scene_editor_graphic_type_callback);
	graphic_type_chooser->set_callback(graphic_type_callback);
	graphic_type_chooser->set_value(scene_editor->current_graphic_type);

	/* Set the data_chooser*/
	Spectrum *temp_spectrum = (Spectrum *)NULL;
	Computed_field *temp_data_field = (Computed_field *)NULL;
	if (scene_editor->current_graphic != NULL)
	{
		Cmiss_graphic_get_data_spectrum_parameters(scene_editor->current_graphic,
			&temp_data_field,&temp_spectrum);
	}

	data_field_chooser = NULL;
	/* Set the spectrum_chooser*/
	wxPanel *spectrum_chooser_panel =
		 XRCCTRL(*this,"SpectrumChooserPanel", wxPanel);
	spectrum_chooser = 
		 new Managed_object_chooser<Spectrum,MANAGER_CLASS(Spectrum)>
		 (spectrum_chooser_panel, scene_editor->spectrum, scene_editor->spectrum_manager,
				(MANAGER_CONDITIONAL_FUNCTION(Spectrum) *)NULL, (void *)NULL, scene_editor->user_interface);
	Callback_base< Spectrum* > *spectrum_callback = 
		 new Callback_member_callback< Spectrum*, 
				wxSceneEditor, int (wxSceneEditor::*)(Spectrum *) >
		 (this, &wxSceneEditor::spectrum_callback);
	spectrum_chooser->set_callback(spectrum_callback);

	spectrum_chooser_panel->Fit();
	select_mode_chooser = NULL;
	radius_scalar_chooser = NULL;	
	iso_scalar_chooser = NULL;
	glyph_chooser = NULL;
	orientation_scale_field_chooser = NULL;		
	variable_scale_field_chooser = NULL;
	label_field_chooser= NULL;
	visibility_field_chooser= NULL;
	font_chooser=NULL;
	use_element_type_chooser = NULL;
	xi_discretization_mode_chooser = NULL;
	native_discretization_field_chooser = NULL;
	xi_point_density_field_chooser =NULL;
	streamline_type_chooser = NULL;
	stream_vector_chooser = NULL;
	streamline_data_type_chooser = NULL;
	texture_coord_field_chooser = NULL;
	render_type_chooser = NULL;
	seed_element_chooser = NULL;
	graphicalitemschecklist = NULL;

	XRCCTRL(*this,"CircleDiscretisationPanel", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::CircleDiscretisationUpdate),
		NULL, this);
	XRCCTRL(*this,"ElementDiscretisationPanel", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::ElementDiscretisationUpdate),
		NULL, this);
	XRCCTRL(*this,"NameTextField", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::GraphicEditorNameText),
		NULL, this);
	XRCCTRL(*this,"ConstantRadiusTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterRadius),
		NULL, this);
	XRCCTRL(*this,"ScaleFactorsTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterRadius),
		NULL, this);
	XRCCTRL(*this,"IsoScalarTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterIsoScalar),
		NULL, this);
	XRCCTRL(*this,"IsoValueSequenceNumberTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterIsoScalar),
		NULL, this);
	XRCCTRL(*this,"IsoValueSequenceFirstTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterIsoScalar),
		NULL, this);
	XRCCTRL(*this,"IsoValueSequenceLastTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterIsoScalar),
		NULL, this);
	XRCCTRL(*this,"CentreTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterGlyphCentre),
		NULL, this);
	XRCCTRL(*this,"BaseGlyphSizeTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterGlyphSize),
		NULL, this);
	XRCCTRL(*this,"GlyphScaleFactorsTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterGlyphScale),
		NULL, this);
	XRCCTRL(*this,"DiscretizationTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterElementDiscretization),
		NULL, this);
	XRCCTRL(*this,"XiTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterSeedXi),
		NULL, this);
	XRCCTRL(*this,"LengthTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterLength),
		NULL, this);
	XRCCTRL(*this,"WidthTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterWidth),
		NULL, this);
	XRCCTRL(*this,"LineWidthTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxSceneEditor::EnterLineWidth),
		NULL, this);
	frame=XRCCTRL(*this, "CmguiSceneEditor", wxFrame);
	currentsceneobjecttext=XRCCTRL(*this,"CurrentSceneObjectText",wxStaticText);
#if defined (__WXMSW__)
	frame->GetSize(&(scene_editor_size.current_width), &(scene_editor_size.current_height));
#endif /*defined (__WXMSW__)*/

	frame->Fit();
	Show();
};

  wxSceneEditor()
  {
  };

  ~wxSceneEditor()
	 {
			if (font_chooser)
				 delete font_chooser;
			if (default_coordinate_field_chooser)
				 delete default_coordinate_field_chooser;
			if (FE_field_chooser)
				 delete FE_field_chooser;
			if (coordinate_field_chooser)
				 delete coordinate_field_chooser;
			if (graphical_material_chooser)
				 delete graphical_material_chooser;
			if (selected_material_chooser)
				 delete selected_material_chooser;
			if (graphic_type_chooser)		
				 delete graphic_type_chooser;
			if (select_mode_chooser)
				 delete select_mode_chooser;
			if (data_field_chooser)
				 delete data_field_chooser;
			if (spectrum_chooser)
				 delete spectrum_chooser;
			if (radius_scalar_chooser)
				 delete radius_scalar_chooser;
			if (iso_scalar_chooser)
				 delete iso_scalar_chooser;
			if (glyph_chooser)
				 delete glyph_chooser;
			if (orientation_scale_field_chooser)
				 delete orientation_scale_field_chooser;
			if (variable_scale_field_chooser)
				 delete variable_scale_field_chooser;
			if (label_field_chooser)
				 delete label_field_chooser;
			if (visibility_field_chooser)
				 delete visibility_field_chooser;
			if (use_element_type_chooser)
				 delete use_element_type_chooser;
			if (xi_discretization_mode_chooser)
				 delete xi_discretization_mode_chooser;
			if (native_discretization_field_chooser)
				 delete	 native_discretization_field_chooser;
			if (xi_point_density_field_chooser)
				 delete  xi_point_density_field_chooser;
			if (streamline_type_chooser)
				 delete streamline_type_chooser;
			if (stream_vector_chooser)
				 delete stream_vector_chooser;
			if (streamline_data_type_chooser)
				 delete streamline_data_type_chooser;
			if (texture_coord_field_chooser)
				 delete texture_coord_field_chooser;
			if (render_type_chooser)
				 delete render_type_chooser;
			if (seed_element_chooser)
				 delete seed_element_chooser;
	 }

/***************************************************************************//**
* Set manager in different field manager object choosers.
* 
* @param scene_editor scene editor to be modify
*/
void Scene_editor_wx_set_manager_in_field_choosers(struct Scene_editor *scene_editor)
{
	 if (default_coordinate_field_chooser != NULL)
			default_coordinate_field_chooser->set_manager(scene_editor->field_manager);
	 if (FE_field_chooser != NULL)
			FE_field_chooser->set_manager(scene_editor->field_manager);
	 if (coordinate_field_chooser != NULL)
			coordinate_field_chooser->set_manager(scene_editor->field_manager);
	 if (data_field_chooser != NULL)
			data_field_chooser->set_manager(scene_editor->field_manager);
	 if (radius_scalar_chooser != NULL)
			radius_scalar_chooser->set_manager(scene_editor->field_manager);
	 if (iso_scalar_chooser != NULL)
			iso_scalar_chooser->set_manager(scene_editor->field_manager);
	 if (orientation_scale_field_chooser != NULL)
			orientation_scale_field_chooser->set_manager(scene_editor->field_manager);
	 if (variable_scale_field_chooser != NULL)
			variable_scale_field_chooser->set_manager(scene_editor->field_manager);
	 if (label_field_chooser != NULL)
			label_field_chooser->set_manager(scene_editor->field_manager);
	 if (visibility_field_chooser != NULL)
		  visibility_field_chooser->set_manager(scene_editor->field_manager);
	 if (native_discretization_field_chooser != NULL)
			native_discretization_field_chooser->set_manager(scene_editor->field_manager);
	 if (xi_point_density_field_chooser != NULL)
			xi_point_density_field_chooser->set_manager(scene_editor->field_manager);
	 if (stream_vector_chooser != NULL)
			stream_vector_chooser->set_manager(scene_editor->field_manager);
	 if (texture_coord_field_chooser != NULL)
			texture_coord_field_chooser->set_manager(scene_editor->field_manager);
}

	/***************************************************************************//**
 * Callback from wxChooser<Scene> when choice is made.
 */
	int default_coordinate_field_callback(Computed_field *default_coordinate_field)
 {
	if (default_coordinate_field&&scene_editor)
	{
		Cmiss_rendition_set_default_coordinate_field(
			scene_editor->edit_rendition,
			default_coordinate_field);
		/* inform the client of the change */
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
		if (scene_editor->current_graphic && coordinate_field_chooser)
		{
			coordinate_field_chooser->set_object(
				Cmiss_graphic_get_coordinate_field(scene_editor->current_graphic));
			Scene_editor_renew_label_on_list(scene_editor->current_graphic);
			coordinatefieldcheckbox->SetValue(1);
			coordinate_field_chooser_panel->Enable();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"default_coordinate_field_callback.  "
			"Invalid argument(s)");
	}
		return 1;
 } /* graphical_element_editor_update_default_coordinate_field */
	
	/***************************************************************************//**
 * Callback from wxChooser<Scene> when choice is made.
 */
	int native_discretization_field_callback(Computed_field *native_discretization_field)
	{
		int return_code;
		if (return_code = Computed_field_get_type_finite_element(native_discretization_field,
			&scene_editor->native_discretization_field))
		{
			Cmiss_rendition_set_native_discretization_field(
				scene_editor->edit_rendition, 
				scene_editor->native_discretization_field);
			Scene_editor_autoapply(scene_editor->rendition,
				scene_editor->edit_rendition);
			if (scene_editor->current_graphic && native_discretization_field_chooser)
			{
				native_discretization_field_chooser->set_object(
					FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
						Computed_field_is_read_only_with_fe_field,
						Cmiss_graphic_get_native_discretization_field(
							scene_editor->current_graphic), scene_editor->field_manager));
				Scene_editor_renew_label_on_list(scene_editor->current_graphic);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_update_native_discretization_field.  "
				"Invalid argument(s)");
		}	

		return (return_code);
	}

int coordinate_field_callback(Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 19 March 2007

DESCRIPTION :
Callback from wxChooser<Coordinate Field> when choice is made.
==============================================================================*/
	{
		Cmiss_graphic_set_coordinate_field(
			 scene_editor->current_graphic, coordinate_field);
		Scene_editor_autoapply(scene_editor->rendition,
			 scene_editor->edit_rendition);
		Scene_editor_renew_label_on_list(scene_editor->current_graphic);
		return(1);
	}

int graphical_material_callback(Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 19 March 2007

DESCRIPTION :
Callback from wxChooser<Graphical Material> when choice is made.
==============================================================================*/
	{
		Cmiss_graphic_set_material(scene_editor->current_graphic,
			material);
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
		Scene_editor_wx_set_list_string(scene_editor->current_graphic);
		return(1);
	}

int selected_material_callback(Graphical_material *selected_material)
/*******************************************************************************
LAST MODIFIED : 20 March 2007

DESCRIPTION :
Callback from wxChooser<Selected Material> when choice is made.
==============================================================================*/
{
	Cmiss_graphic_set_selected_material(scene_editor->current_graphic,
		selected_material);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_wx_set_list_string(scene_editor->current_graphic);
	return(1);
}

/***************************************************************************//**
 *Callback from graphic enumerator chooser when choice is made.
 */
int Scene_editor_graphic_type_callback(enum Cmiss_graphic_type new_value)
{
	int return_code;
	
	if (scene_editor)
	{
		if (scene_editor->current_graphic_type != new_value)
		{
			if (!graphicalitemschecklist)
				graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
			int number= graphicalitemschecklist->GetCount();
			graphicalitemschecklist->SetSelection(wxNOT_FOUND);
			Cmiss_graphic *graphic = NULL;
			scene_editor->current_graphic_type = new_value;
			graphic = first_graphic_in_Cmiss_rendition_that(
				scene_editor->edit_rendition,
				Cmiss_graphic_type_matches,
				(void*)scene_editor->current_graphic_type);
			if (graphic)
			{
				for (int i=0;number>i;i++) 
				{
					Cmiss_graphic *newgraphic;
					newgraphic = Cmiss_rendition_get_graphic_at_position(
						scene_editor->edit_rendition, i+1);
					if (graphic == newgraphic)
					{
						graphicalitemschecklist->SetSelection(i);
						Scene_editor_wx_update_graphic_type(newgraphic);
						Scene_editor_wx_update_graphic_widgets();
						break;
					}
					if (newgraphic)
					{
						Cmiss_graphic_destroy(&newgraphic);
					}
				}
				if (graphic != scene_editor->current_graphic)
				{
					if (scene_editor->current_graphic)
						REACCESS(Cmiss_graphic)(&(scene_editor->current_graphic),
							graphic);
					/* if graphic_type changed, select it in graphic_type option menu */
					Cmiss_graphic_type graphic_type = Cmiss_graphic_get_graphic_type(graphic);
					if (graphic_type != scene_editor->current_graphic_type)
					{
						scene_editor->current_graphic_type = graphic_type;
					}
					return_code = 1;
				}
			}
			else
			{
				if (scene_editor->current_graphic)
					REACCESS(Cmiss_graphic)(&(scene_editor->current_graphic),
						NULL);
			}
		}
		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"setting type callback.  Invalid argument(s)");
		return_code =  0;
	}
	return (return_code);	
}

int select_mode_callback(enum Graphics_select_mode select_mode)
/*******************************************************************************
LAST MODIFIED : 19 March 2007

DESCRIPTION :
Callback from wxChooser<select mode> when choice is made.
==============================================================================*/
{
	Cmiss_graphic_set_select_mode(
		scene_editor->current_graphic, select_mode);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
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
		 if (scene_editor->lowersplitter)
		 {
				int width, height;
				scene_editor->lowersplitter->GetSize(&width, &height);
				scene_editor->lowersplitter->SetSize(width-1, height-1);
				scene_editor->lowersplitter->SetSize(width+1, height+1);
		 }
		 return 1;
	}
int radius_scalar_callback(Computed_field *radius_scalar_field)
/*******************************************************************************
LAST MODIFIED : 20 March 2007

DESCRIPTION :
Callback from wxChooser<Radius Scalar> when choice is made.
==============================================================================*/
{
	float constant_radius,scale_factor; 
	double temp;

	constantradiustextctrl=XRCCTRL(*this, "ConstantRadiusTextCtrl",wxTextCtrl);
	(constantradiustextctrl->GetValue()).ToDouble(&temp);
	constant_radius=(float)temp;
	scalefactorstextctrl=XRCCTRL(*this,"ScaleFactorsTextCtrl",wxTextCtrl);
	(scalefactorstextctrl->GetValue()).ToDouble(&temp);
	scale_factor=(float)temp;	
	Cmiss_graphic_set_radius_parameters(scene_editor->current_graphic,constant_radius,
		scale_factor,radius_scalar_field);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	return 1;
}


int iso_scalar_callback(Computed_field *iso_scalar_field)
/*******************************************************************************
LAST MODIFIED : 21 March 2007

DESCRIPTION :
Callback from wxChooser<Iso Scalar> when choice is made.
==============================================================================*/
{
	double decimation_threshold, *iso_values,
	first_iso_value, last_iso_value;
	int number_of_iso_values;
	struct Computed_field *scalar_field;

	Cmiss_graphic_get_iso_surface_parameters(
		scene_editor->current_graphic,&scalar_field,&number_of_iso_values,
		&iso_values,&first_iso_value,&last_iso_value,
		&decimation_threshold);
	Cmiss_graphic_set_iso_surface_parameters(
		scene_editor->current_graphic,iso_scalar_field,number_of_iso_values,
		iso_values,first_iso_value,last_iso_value,
		decimation_threshold);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);

	return 1;
}

int glyph_callback(GT_object *glyph)
/*******************************************************************************
LAST MODIFIED : 21 March 2007

DESCRIPTION :
Callback from wxChooser<Glyph> when choice is made.
==============================================================================*/
{
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *old_glyph;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	if (scene_editor->current_graphic)
	{
		enum Graphic_glyph_scaling_mode glyph_scaling_mode;
		if (Cmiss_graphic_get_glyph_parameters(
					scene_editor->current_graphic, &old_glyph, &glyph_scaling_mode,
					glyph_centre, glyph_size, &orientation_scale_field, glyph_scale_factors,
					&variable_scale_field)&&
			Cmiss_graphic_set_glyph_parameters(
				scene_editor->current_graphic, glyph, glyph_scaling_mode,
				glyph_centre, glyph_size,	orientation_scale_field, glyph_scale_factors,
				variable_scale_field))
		{
			/* inform the client of the change */
			Scene_editor_autoapply(scene_editor->rendition,
				scene_editor->edit_rendition);
			Scene_editor_renew_label_on_list(scene_editor->current_graphic);
			return 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"glyph_callback.  Cannot get and set glyph parameters");
			return 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"glyph_callback.  Invalid argument(s)");
		return 0;
	}
}

int orientation_scale_callback(Computed_field *orientation_scale_field)
/*******************************************************************************
LAST MODIFIED : 21 March 2007
DESCRIPTION :
Callback from wxChooser<Orientation Scale> when choice is made.
==============================================================================*/
{
	struct Computed_field *temp_orientation_scale_field;
	struct Computed_field *variable_scale_field;
	struct GT_object *glyph;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	if (scene_editor->current_graphic)
	{
		enum Graphic_glyph_scaling_mode glyph_scaling_mode;
		if (Cmiss_graphic_get_glyph_parameters(
		     scene_editor->current_graphic, &glyph, &glyph_scaling_mode,
		     glyph_centre, glyph_size, &temp_orientation_scale_field, glyph_scale_factors,
		     &variable_scale_field)&&
		    Cmiss_graphic_set_glyph_parameters(
		     scene_editor->current_graphic, glyph, glyph_scaling_mode,
		     glyph_centre, glyph_size,orientation_scale_field, glyph_scale_factors,
		     variable_scale_field))
		{
			/* inform the client of the change */
			Scene_editor_autoapply(scene_editor->rendition,
				scene_editor->edit_rendition);
			Scene_editor_renew_label_on_list(scene_editor->current_graphic);
			return 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"orientation_scale_callback.  Cannot get and set glyph parameters");
			return 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"orientation_scale_callback.  Invalid argument(s)");
		return 0;
	}
}

int variable_scale_callback(Computed_field *variable_scale_field)
/*******************************************************************************
LAST MODIFIED : 21 March 2007

DESCRIPTION :
Callback from wxChooser<Variable Scale> when choice is made.
==============================================================================*/
{
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *temp_variable_scale_field;
	struct GT_object *glyph;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	Cmiss_graphic_get_glyph_parameters(
	  scene_editor->current_graphic, &glyph, &glyph_scaling_mode, glyph_centre,
	  glyph_size, &orientation_scale_field, glyph_scale_factors,
	  &temp_variable_scale_field);
	Cmiss_graphic_set_glyph_parameters(
	  scene_editor->current_graphic,glyph, glyph_scaling_mode, glyph_centre,
	  glyph_size, orientation_scale_field, glyph_scale_factors,
	  variable_scale_field);		
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	return 1;
}

int label_callback(Computed_field *label_field)
/*******************************************************************************
LAST MODIFIED : 22 March 2007

DESCRIPTION :
Callback from wxChooser<label> when choice is made.
==============================================================================*/
{
	struct Computed_field *temp_label_field;
	struct Graphics_font *font;
	Cmiss_graphic_get_label_field(scene_editor->current_graphic, 
		&temp_label_field, &font);
	Cmiss_graphic_set_label_field(scene_editor->current_graphic, 
		 label_field, font);
	Scene_editor_autoapply(scene_editor->rendition,
	    scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	return 1;
}

int visibility_field_callback(Computed_field *visibility_field)
{
	Cmiss_graphic_set_visibility_field(scene_editor->current_graphic, 
		visibility_field);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);

	return 1;
}

int font_callback(Graphics_font *graphics_font)
/*******************************************************************************
LAST MODIFIED : 22 May 2007

DESCRIPTION :
Callback from wxChooser<font> when choice is made.
==============================================================================*/
{
	struct Computed_field *temp_label_field;
	struct Graphics_font *font;
	Cmiss_graphic_get_label_field(scene_editor->current_graphic, 
		&temp_label_field, &font);
	Cmiss_graphic_set_label_field(scene_editor->current_graphic, 
		 temp_label_field, graphics_font);
	Scene_editor_autoapply(scene_editor->rendition,
	  scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	return 1;
}



int use_element_type_callback(enum Use_element_type use_element_type)
/*******************************************************************************
LAST MODIFIED : 22 March 2007

DESCRIPTION :
Callback from wxChooser<Use Element Type> when choice is made.
==============================================================================*/
 {
	 int face;

	 Cmiss_graphic_set_use_element_type(
		scene_editor->current_graphic,use_element_type);
	 exteriorcheckbox=XRCCTRL(*this,"ExteriorCheckBox",wxCheckBox);
	 facecheckbox=XRCCTRL(*this, "FaceCheckBox",wxCheckBox);
	 facechoice=XRCCTRL(*this, "FaceChoice",wxChoice);

	 if (CMISS_GRAPHIC_ISO_SURFACES==scene_editor->current_graphic_type)
	 {
		 linewidthtextctrl=XRCCTRL(*this,"LineWidthTextCtrl",wxTextCtrl);
		 if (USE_FACES != use_element_type) 
		 {
			 linewidthtextctrl->Disable();
		 }
		 else
		 {
			 linewidthtextctrl->Enable();
		 }
	 }
	 if  (USE_ELEMENTS != use_element_type) 
	 {
		 exteriorcheckbox->Enable();
		 facecheckbox->Enable();
		 Cmiss_graphic_set_exterior(scene_editor->current_graphic,
			 exteriorcheckbox->IsChecked());
		 if (facecheckbox->IsChecked()) 
		 {
			 facechoice->Enable();
			 face = facechoice->GetSelection();
		 }
		 else
		 {
			 facechoice->Disable();
			 face= -1;
		 }
		 Cmiss_graphic_set_face(scene_editor->current_graphic,face);
	 }
	else
	 {
		 exteriorcheckbox->Disable();
		 facecheckbox->Disable();
		 facechoice->Disable();
		 Cmiss_graphic_set_exterior(scene_editor->current_graphic,0);
		 Cmiss_graphic_set_face(scene_editor->current_graphic,-1);
	 }

	 Scene_editor_autoapply(scene_editor->rendition,
	   scene_editor->edit_rendition);
	 Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	 return 1;
 }

int xi_discretization_mode_callback(enum Xi_discretization_mode xi_discretization_mode)
/*******************************************************************************
LAST MODIFIED : 22 March 2007

DESCRIPTION :
Callback from wxChooser<xi Discretization Mode> when choice is made.
==============================================================================*/
{
	enum Xi_discretization_mode old_xi_discretization_mode;
	struct Computed_field *old_xi_point_density_field, *xi_point_density_field;

	discretizationtext=XRCCTRL(*this,"DiscretizationText",wxStaticText);
	discretizationtextctrl=XRCCTRL(*this,"DiscretizationTextCtrl",wxTextCtrl);
	nativediscretizationfieldcheckbox=XRCCTRL(*this,"NativeDiscretizationFieldCheckBox",wxCheckBox);
	native_discretization_field_chooser_panel=XRCCTRL(*this, "NativeDiscretizationFieldChooserPanel",wxPanel);
	densityfieldtext=XRCCTRL(*this, "DensityFieldText",wxStaticText);
	density_field_chooser_panel=XRCCTRL(*this,"DensityFieldChooserPanel",wxPanel);
	xitext=XRCCTRL(*this,"XiText",wxStaticText);
	xitextctrl=XRCCTRL(*this,"XiTextCtrl",wxTextCtrl);
	if (Cmiss_graphic_get_xi_discretization(
			scene_editor->current_graphic, &old_xi_discretization_mode,
			&old_xi_point_density_field))
		{
			xi_point_density_field = old_xi_point_density_field;
			if ((XI_DISCRETIZATION_CELL_DENSITY == xi_discretization_mode) ||
				(XI_DISCRETIZATION_CELL_POISSON == xi_discretization_mode))
			{
				if (!xi_point_density_field)
				{
					/* get xi_point_density_field from widget */
					xi_point_density_field=xi_point_density_field_chooser->get_object();	
				}
			}
			else
			{
				xi_point_density_field = (struct Computed_field *)NULL;
			}
			if (Cmiss_graphic_set_xi_discretization(
				scene_editor->current_graphic, xi_discretization_mode,
				xi_point_density_field))
			{
				/* inform the client of the change */
				Scene_editor_autoapply(scene_editor->rendition,
				  scene_editor->edit_rendition);
				Scene_editor_renew_label_on_list(scene_editor->current_graphic);
			}
			else
			{
				xi_discretization_mode = old_xi_discretization_mode;
				xi_point_density_field = old_xi_point_density_field;
				xi_discretization_mode_chooser->set_value(xi_discretization_mode);
			}
			if (XI_DISCRETIZATION_EXACT_XI != xi_discretization_mode)
			{
				FE_field *native_discretization_field=
				Cmiss_graphic_get_native_discretization_field(scene_editor->current_graphic);
				discretizationtextctrl->Enable();	
				discretizationtext->Enable();
				nativediscretizationfieldcheckbox->Enable();
				native_discretization_field_chooser_panel->Enable();
				xitext->Disable();
				xitextctrl->Disable();
				if 	((struct FE_field *)NULL != native_discretization_field)
				{
				 native_discretization_field_chooser_panel->Enable();
				}
				else
				{
				 native_discretization_field_chooser_panel->Disable();
				}
			}			
			else
			{	
				discretizationtextctrl->Disable();	
				discretizationtext->Disable();
				nativediscretizationfieldcheckbox->Disable();
				native_discretization_field_chooser_panel->Disable();
				xitext->Enable();
				xitextctrl->Enable();
			}
			if ((XI_DISCRETIZATION_CELL_DENSITY == xi_discretization_mode)||
			    (XI_DISCRETIZATION_CELL_POISSON == xi_discretization_mode))
			{
				densityfieldtext->Enable();
				density_field_chooser_panel->Enable();
			}
			else
			{
				densityfieldtext->Disable();
				density_field_chooser_panel->Disable();
			}
		}


	return 1;
}

int native_discretization_callback(Computed_field *native_discretization_field)
/*******************************************************************************
LAST MODIFIED : 22 March 2007

DESCRIPTION :
Callback from wxChooser<Native discretization> when choice is made.
==============================================================================*/
{
	FE_field *temp_native_discretization_field;

	if (Computed_field_get_type_finite_element(native_discretization_field,
			&temp_native_discretization_field))
	{
	  Cmiss_graphic_set_native_discretization_field(
	    scene_editor->current_graphic, temp_native_discretization_field);
	  Scene_editor_autoapply(scene_editor->rendition,
	    scene_editor->edit_rendition);
	  Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	}
	else
	{
	  display_message(ERROR_MESSAGE, "wxSceneEditor::native_discretization_callback.  "
	    "Could not modify native discretization field");
	}
	return 1;
}

int xi_point_density_callback(Computed_field *xi_point_density_field)
/*******************************************************************************
LAST MODIFIED : 22 March 2007

DESCRIPTION :

Callback from wxChooser<xi Point Denstiy Field> when choice is made.
==============================================================================*/
{
	enum Xi_discretization_mode xi_discretization_mode;
	struct Computed_field *temp_xi_point_density_field;

	Cmiss_graphic_get_xi_discretization(
		scene_editor->current_graphic, &xi_discretization_mode,
		&temp_xi_point_density_field);
        Cmiss_graphic_set_xi_discretization(
		scene_editor->current_graphic, xi_discretization_mode,
		xi_point_density_field);
	Scene_editor_autoapply(scene_editor->rendition,
	  scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	return 1;
}

int seed_element_callback(FE_element *seed_element)
/*******************************************************************************
LAST MODIFIED : 30 March 2007

DESCRIPTION :
Callback from wxChooser<Seed Element> when choice is made.
==============================================================================*/
{
	Cmiss_graphic_set_seed_element(scene_editor->current_graphic,
		seed_element);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	return 1;
}

int streamline_type_callback(enum Streamline_type streamline_type)
/*******************************************************************************
LAST MODIFIED : 23 March 2007

DESCRIPTION :
Callback from wxChooser<Streamline Type> when choice is made.
==============================================================================*/
{
	enum Streamline_type temp_streamline_type;
	float streamline_length,streamline_width;
	int reverse_track;
	struct Computed_field *stream_vector_field;

	if (Cmiss_graphic_get_streamline_parameters(
				scene_editor->current_graphic, &temp_streamline_type, &stream_vector_field,
				&reverse_track, &streamline_length, &streamline_width) &&
		Cmiss_graphic_set_streamline_parameters(
		  scene_editor->current_graphic, streamline_type, stream_vector_field,
		  reverse_track, streamline_length, streamline_width))
	{
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
		Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	}
	return 1;
}

	int stream_vector_callback(Computed_field *stream_vector_field)
/*******************************************************************************
LAST MODIFIED : 23 March 2007

DESCRIPTION :
Callback from wxChooser<Stream Vector> when choice is made.
==============================================================================*/
{
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int reverse_track;
	struct Computed_field *temp_stream_vector_field;

	if (Cmiss_graphic_get_streamline_parameters(
		    scene_editor->current_graphic,&streamline_type,
			 &temp_stream_vector_field,&reverse_track,
			 &streamline_length,&streamline_width)&&
		 Cmiss_graphic_set_streamline_parameters(
			 scene_editor->current_graphic,streamline_type,
			 stream_vector_field,reverse_track,
		    streamline_length,streamline_width))
	{
 		/* inform the client of the change */
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
		Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	}

	return 1;
}

int streamline_data_type_callback(enum Streamline_data_type streamline_data_type)
/*******************************************************************************
LAST MODIFIED : 26 March 2007

DESCRIPTION :
Callback from wxChooser<Stream Data Type> when choice is made.
==============================================================================*/
{
	enum Streamline_data_type old_streamline_data_type;
	struct Computed_field *data_field;
	struct Spectrum *spectrum;
	datacheckbox=XRCCTRL(*this, "DataCheckBox", wxCheckBox);
	data_chooser_panel=XRCCTRL(*this,"DataChooserPanel",wxPanel);
	spectrumtext=XRCCTRL(*this, "SpectrumText", wxStaticText);
	spectrum_chooser_panel=XRCCTRL(*this,"SpectrumChooserPanel", wxPanel);

 	if (Cmiss_graphic_get_data_spectrum_parameters_streamlines(
		 scene_editor->current_graphic,&old_streamline_data_type,
		 &data_field,&spectrum))
		{
			if (streamline_data_type != old_streamline_data_type)
			{
				if (STREAM_FIELD_SCALAR==old_streamline_data_type)
				{
					data_field=(struct Computed_field *)NULL;
				}
				old_streamline_data_type=streamline_data_type;
				if (STREAM_FIELD_SCALAR==streamline_data_type)
				{
					/* get data_field from widget */
					data_field=data_field_chooser->get_object();
					if (!data_field)
					{
						streamline_data_type=STREAM_NO_DATA;
					}
				}
				if ((STREAM_NO_DATA != streamline_data_type)&&!spectrum)
				{
					/* get spectrum from widget */
					spectrum=spectrum_chooser->get_object();
				}
				Cmiss_graphic_set_data_spectrum_parameters_streamlines(
					scene_editor->current_graphic,streamline_data_type,
					data_field,spectrum);
				if (streamline_data_type != old_streamline_data_type)
				{
					/* update the choose_enumerator for streamline_data_type */
					streamline_data_type_chooser->set_value(streamline_data_type);
				}
				/* set grayed status of data_field/spectrum widgets */
				if ((struct Computed_field *)NULL != data_field)
				{
					datacheckbox->SetValue(1);
					data_chooser_panel->Enable();	
				}
				else
				{
					datacheckbox->SetValue(0);
					data_chooser_panel->Disable();	
				}

				if (STREAM_NO_DATA != streamline_data_type)
				{
					spectrumtext->Enable();
					spectrum_chooser_panel->Enable();
				}
				else
				{
					spectrumtext->Disable();
					spectrum_chooser_panel->Disable();
				}
				/* inform the client of the change */
				Scene_editor_autoapply(scene_editor->rendition,
					scene_editor->edit_rendition);	 
				Scene_editor_renew_label_on_list(scene_editor->current_graphic);
			}
		}
	return 1;
}

	int data_field_callback(Computed_field *data_field)
/*******************************************************************************
LAST MODIFIED : 26 March 2007

DESCRIPTION :
Callback from wxChooser<Data Field> when choice is made.
==============================================================================*/
{
	enum Streamline_data_type streamline_data_type;
	struct Computed_field *temp_data_field;
	struct Spectrum *spectrum;

	if (CMISS_GRAPHIC_STREAMLINES==Cmiss_graphic_get_graphic_type
			(scene_editor->current_graphic))
		{
			Cmiss_graphic_get_data_spectrum_parameters_streamlines(
				scene_editor->current_graphic,&streamline_data_type,
				&temp_data_field,&spectrum);
			Cmiss_graphic_set_data_spectrum_parameters_streamlines(
				scene_editor->current_graphic,streamline_data_type,
				data_field,spectrum);
		}
		else
		{
			Cmiss_graphic_get_data_spectrum_parameters(
				scene_editor->current_graphic,&temp_data_field,&spectrum);
			Cmiss_graphic_set_data_spectrum_parameters(
				scene_editor->current_graphic,data_field,spectrum);
		}
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);	 	
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	return 1;
}


	int spectrum_callback(Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 26 March 2007

DESCRIPTION :
Callback from wxChooser<Spectrum> when choice is made.
==============================================================================*/
{
	enum Streamline_data_type streamline_data_type;
	struct Computed_field *data_field;
	struct Spectrum *temp_spectrum;

	if (CMISS_GRAPHIC_STREAMLINES==Cmiss_graphic_get_graphic_type
			(scene_editor->current_graphic))
	{
		Cmiss_graphic_get_data_spectrum_parameters_streamlines(
			scene_editor->current_graphic,&streamline_data_type,
			&data_field,&temp_spectrum);
		Cmiss_graphic_set_data_spectrum_parameters_streamlines(
			scene_editor->current_graphic,streamline_data_type,
			data_field,spectrum);
	}
	else
	{
		Cmiss_graphic_get_data_spectrum_parameters(
			scene_editor->current_graphic,&data_field,&temp_spectrum);
		Cmiss_graphic_set_data_spectrum_parameters(
			scene_editor->current_graphic,data_field,spectrum);
	}
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);	 
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	return 1;
}

int texture_coord_field_callback(Computed_field *texture_coord_field)
/*******************************************************************************
LAST MODIFIED : 26 March 2007

DESCRIPTION :
Callback from wxChooser<Texture Coord Field> when choice is made.
==============================================================================*/
{
	Cmiss_graphic_set_texture_coordinate_field(
		scene_editor->current_graphic, texture_coord_field);
			/* inform the client of the change */
	Scene_editor_autoapply(scene_editor->rendition,
			       scene_editor->edit_rendition);	 	
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	return 1;
}

int render_type_callback(enum Render_type render_type)
/*******************************************************************************
LAST MODIFIED : 26 March 2007

DESCRIPTION :
Callback from wxChooser<Render Type> when choice is made.
==============================================================================*/
{
	Cmiss_graphic_set_render_type(
		scene_editor->current_graphic, render_type);
			/* inform the client of the change */
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	return 1;
}

/***************************************************************************//**
*	Refresh the widgets upon collapsible pane change event.																																						
* WXMSW only: Revert the size of the frame as this event will change the size
*             of the window somehow.
*/
void CollapsiblepaneChangedEvent(wxCollapsiblePaneEvent& event)
{
	USE_PARAMETER(event);
	frame=XRCCTRL(*this, "CmguiSceneEditor", wxFrame);
	if (frame)
	{
		frame->Freeze();
		lowersplitter=XRCCTRL(*this,"LowerSplitter",wxSplitterWindow);
		lowersplitter->Layout();	
		topsplitter=XRCCTRL(*this,"TopSplitter",wxSplitterWindow);
		topsplitter->Layout();	
		frame = 
			XRCCTRL(*this, "CmguiSceneEditor", wxFrame);
		frame->SetMinSize(wxSize(50,100));
		frame->SetMaxSize(wxSize(2000,2000));
		frame->Layout();
#if defined (__WXMSW__)
		frame->SetSize(scene_editor_size.previous_width, scene_editor_size.previous_height);
		frame->GetSize(&scene_editor_size.current_width, &scene_editor_size.current_height);
#endif /* defined (__WXMSW__) */
		frame->Layout();
		frame->Thaw();
	}
}

#if defined (__WXMSW__)
/***************************************************************************//**
* Get the size of the frame in case the collapsible panes change the size
* of the frame.
* 
*/
void FrameGetSize(wxSizeEvent &event)
{
	frame=XRCCTRL(*this, "CmguiSceneEditor", wxFrame);
	if (frame)
	{
		scene_editor_size.previous_width = scene_editor_size.current_width;
		scene_editor_size.previous_height = scene_editor_size.current_height;
		frame->GetSize(&scene_editor_size.current_width, &scene_editor_size.current_height);
		event.Skip();
	}
}
#endif /* defined (__WXMSW__) */


/***************************************************************************//**
 *Check if the auto apply clicked or not, if clicked, apply the current changes
 */
void Scene_editor_autoapply(Cmiss_rendition *destination, Cmiss_rendition *source)
{
	 if(scene_editor->auto_apply)
	 {
		 if (scene_editor->rendition_callback_flag)
		 {
			 if (Cmiss_rendition_remove_callback(scene_editor->rendition,
					 Scene_editor_wx_rendition_change, (void *)scene_editor))
			 {
				 scene_editor->rendition_callback_flag = 0;
			 }
		 }
		 if (!Cmiss_rendition_modify(destination,source))
		 {
			 display_message(ERROR_MESSAGE, "wxSceneEditor::Scene_editor_autoapply"
				 "Could not modify rendition");
		 }
		 if (Cmiss_rendition_add_callback(scene_editor->rendition,
				 Scene_editor_wx_rendition_change, (void *)scene_editor))
		 {
			 scene_editor->rendition_callback_flag = 1;
		 }
	 }
	 else
	 {
		 applybutton->Enable();
		 revertbutton->Enable();
	 }
}

void Scene_editor_wx_set_list_string(Cmiss_graphic *graphic)
{
	unsigned int selection;
	char *graphic_string;
	unsigned int check;
	graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
	selection=graphicalitemschecklist->GetSelection();
	check = graphicalitemschecklist->IsChecked(selection);
	graphic_string = Cmiss_graphic_string(graphic,
		 GRAPHIC_STRING_COMPLETE_PLUS);
	graphicalitemschecklist->SetString(selection, graphic_string);
	graphicalitemschecklist->Check(selection,check);
	DEALLOCATE(graphic_string);
}

/***************************************************************************//**
 * When changes have been made by the user, renew the label on the list
 */
	void Scene_editor_renew_label_on_list(Cmiss_graphic *graphic)
{
	int position, check;
	char *graphic_string;
	
	graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);

	position = Cmiss_rendition_get_graphic_position(
		scene_editor->edit_rendition, graphic);
	check=graphicalitemschecklist->IsChecked(position - 1);
	graphic_string = Cmiss_graphic_string(scene_editor->current_graphic,
		GRAPHIC_STRING_COMPLETE_PLUS);
	graphicalitemschecklist->SetString(position-1, graphic_string);
	graphicalitemschecklist->Check(position-1, check);
	DEALLOCATE(graphic_string);
}

void ResetWindow(wxSplitterEvent& event)
{
	USE_PARAMETER(event);
	 frame = 
			XRCCTRL(*this, "CmguiSceneEditor", wxFrame);
	 frame->Layout();
	 frame->SetMinSize(wxSize(50,100));
	 topsplitter=XRCCTRL(*this,"TopSplitter",wxSplitterWindow);
	 topsplitter->Layout();	
	 lowersplitter=XRCCTRL(*this,"LowerSplitter",wxSplitterWindow);
	 lowersplitter->Layout();
	 sceneediting = 
			XRCCTRL(*this, "SceneEditing", wxScrolledWindow);
	 sceneediting->Layout();
	 sceneediting->SetScrollbars(10,10,40,40);
}

	void ElementDiscretisationUpdate(wxCommandEvent &event)
	{
		struct Parse_state *temp_state;
		struct Element_discretization element_discretization, discretization;
		char *text_entry, temp_string[20];

	USE_PARAMETER(event);
		elementdiscretisationpanel=XRCCTRL(*this, "ElementDiscretisationPanel",wxTextCtrl);
		if (text_entry = duplicate_string(
					const_cast<char *>(elementdiscretisationpanel->GetValue().c_str())))
		{
			if (NULL != (temp_state=create_Parse_state(text_entry)))
			{
				if (set_Element_discretization(temp_state,
						(void *)&element_discretization,
						(void *)scene_editor->user_interface) &&
					Cmiss_rendition_set_element_discretization(
						scene_editor->edit_rendition,
						&element_discretization))
				{
					/* inform the client of the changes */
					if (scene_editor->current_graphic && discretizationtextctrl &&
						Cmiss_graphic_get_discretization(
							scene_editor->current_graphic,&discretization))
					{
						/* always restore constant_radius to actual value in use */
						sprintf(temp_string,"%d*%d*%d",discretization.number_in_xi1,
							discretization.number_in_xi2,discretization.number_in_xi3);
						discretizationtextctrl->ChangeValue(temp_string);
						Scene_editor_renew_label_on_list(scene_editor->current_graphic);
					}
				}
				destroy_Parse_state(&temp_state);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"graphical_element_editor_element_disc_text_CB.  "
					"Could not create parse state");
			}
			set_general_graphic(scene_editor);
			DEALLOCATE(text_entry);
		}
		else
		{
			 display_message(ERROR_MESSAGE,
					"graphical_element_editor_element_disc_text_CB.  Invalid argument(s)");
		}
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
	}

	void NativeDiscretisationFieldChecked(wxCommandEvent &event)
	{
		Computed_field *temp_native_discretization_field;
		FE_field *native_discretization_field;
	USE_PARAMETER(event);
		nativediscretizationcheckbox = XRCCTRL(*this, "NativeDiscretisationFieldCheckBox",wxCheckBox);
		FE_chooser_panel = XRCCTRL(*this, "NativeDiscretisationFieldChooser",wxPanel);
		if (nativediscretizationcheckbox->IsChecked())
		{
			temp_native_discretization_field=FE_field_chooser->get_object();
			Computed_field_get_type_finite_element(temp_native_discretization_field,
				 &native_discretization_field);
			FE_chooser_panel->Enable();
		}
		else
		{
			FE_chooser_panel->Disable();
			native_discretization_field=(FE_field *)NULL;
		}
	  Cmiss_rendition_set_native_discretization_field(
	    scene_editor->edit_rendition, native_discretization_field);
	  Scene_editor_autoapply(scene_editor->rendition,
	    scene_editor->edit_rendition);
	}

	void AutoChecked(wxCommandEvent &event)
	{
	USE_PARAMETER(event);
		autocheckbox = XRCCTRL(*this, "AutoCheckBox", wxCheckBox);
		applybutton = XRCCTRL(*this, "ApplyButton", wxButton);
		revertbutton = XRCCTRL(*this,"RevertButton", wxButton);
		if(autocheckbox->IsChecked())
		{
			 applybutton->Disable();
			 revertbutton->Disable();
			 scene_editor->auto_apply = 1;
			 Scene_editor_autoapply(scene_editor->rendition,
				 scene_editor->edit_rendition);
		}
		else
		{
			 applybutton->Disable();
			 revertbutton->Disable();
			 scene_editor->auto_apply = 0;
		}
	}

void RevertClicked(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	Scene_editor_revert_changes(scene_editor);		
}

void ApplyClicked(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	if (scene_editor->rendition_callback_flag)
	{
		if (Cmiss_rendition_remove_callback(scene_editor->rendition,
				Scene_editor_wx_rendition_change, (void *)scene_editor))
		{
			scene_editor->rendition_callback_flag = 0;
		}
	}
	if (scene_editor->transformation_editor)
	{
		scene_editor->transformation_editor->ApplyTransformation(/*force_apply*/1);
	}
	if (!Cmiss_rendition_modify(scene_editor->rendition,
			scene_editor->edit_rendition))
	{
		display_message(ERROR_MESSAGE, "wxSceneEditor::ApplyClicked.  "
			"Could not modify rendition");
	}
	if (Cmiss_rendition_add_callback(scene_editor->rendition,
			Scene_editor_wx_rendition_change, (void *)scene_editor))
	{
		scene_editor->rendition_callback_flag = 1;
	}
}

void CircleDiscretisationUpdate(wxCommandEvent &event)
{
	 USE_PARAMETER(event);
	 int circle_discretization;
	 circlediscretisationpanel = XRCCTRL(*this, "CircleDiscretisationPanel",wxTextCtrl);
	 char temp_string[10];
	 TempText = circlediscretisationpanel->GetValue();
	 if (TempText)
	 {
			circle_discretization = atoi(const_cast<char *>(TempText.c_str()));
			if (Cmiss_rendition_set_circle_discretization(
						scene_editor->edit_rendition, circle_discretization))
			{
				if (scene_editor->current_graphic && circlediscretizationtextctrl &&
					CMISS_GRAPHIC_CYLINDERS==scene_editor->current_graphic_type)
				{
					sprintf(temp_string,"%d",Cmiss_graphic_get_circle_discretization(
										scene_editor->current_graphic));
					circlediscretizationtextctrl->SetValue(temp_string);
					Scene_editor_renew_label_on_list(scene_editor->current_graphic);
				}
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "graphical_element_editor_circle_disc_text_CB.  Missing text");
	 }
	 /* always redisplay discretization to show assumed values */
	 set_general_graphic(scene_editor);

	 Scene_editor_autoapply(scene_editor->rendition,
				scene_editor->edit_rendition);
}

void Scene_editor_wx_update_graphic_type(Cmiss_graphic *graphic)
{
	 wxScrolledWindow *sceneeditingpanel= XRCCTRL(*this, "SceneEditing",wxScrolledWindow);
	 sceneeditingpanel->Enable();
	 sceneeditingpanel->Show();
	 graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
	 REACCESS(Cmiss_graphic)(&scene_editor->current_graphic, graphic);
	 if (graphic)
	 {
		 scene_editor->current_graphic_type = Cmiss_graphic_get_graphic_type(graphic);
		 graphic_type_chooser->set_value(scene_editor->current_graphic_type);
	 }
	 else
	 {
		 graphic_type_chooser->set_value(CMISS_GRAPHIC_LINES);
	 }
}

void Scene_editor_wx_update_graphic_widgets()
{
	 wxScrolledWindow *sceneeditingpanel = XRCCTRL(*this, "SceneEditing",wxScrolledWindow);
	 sceneeditingpanel->Freeze();
	 get_and_set_Cmiss_graphic_widgets((void *)scene_editor);
	 sceneeditingpanel->Thaw();
	 sceneeditingpanel->Layout();
	 if (scene_editor->lowersplitter)
	 {
			int width, height;
			scene_editor->lowersplitter->GetSize(&width, &height);
			scene_editor->lowersplitter->SetSize(width-1, height-1);
				 scene_editor->lowersplitter->SetSize(width+1, height+1);
	 }
}

void GraphicListBoxProcessSelection(int selection)
{
	if (-1 != selection)
	{
		Cmiss_graphic *temp_graphic = Cmiss_rendition_get_graphic_at_position(
			scene_editor->edit_rendition, selection+1);
		Scene_editor_wx_update_graphic_type(temp_graphic);
		Scene_editor_wx_update_graphic_widgets();
		if (temp_graphic)
		{
			Cmiss_graphic_destroy(&temp_graphic);
		}
	}
}

void GraphicListBoxChecked(wxCommandEvent &event)
{
	int selection = event.GetInt();
	graphicalitemschecklist->SetSelection(selection);
	GraphicListBoxProcessSelection(selection);
	Cmiss_graphic *temp_graphic = Cmiss_rendition_get_graphic_at_position(
		scene_editor->edit_rendition, selection+1);
	Cmiss_graphic_set_visibility(temp_graphic, 
		graphicalitemschecklist->IsChecked(selection));
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Cmiss_graphic_destroy(&temp_graphic);
}


void GraphicListBoxClicked(wxCommandEvent &event)
{
	GraphicListBoxProcessSelection(event.GetInt());
}


void RenewGeneralSettingChooser(Cmiss_rendition *edit_rendition)
{
	if (default_coordinate_field_chooser)
	{
		default_coordinate_field_chooser->set_object(
			Cmiss_rendition_get_default_coordinate_field(edit_rendition));
	}
	if (FE_field_chooser)
	{
		FE_chooser_panel = XRCCTRL(*this, "NativeDiscretisationFieldChooser",wxPanel);
		nativediscretizationcheckbox = XRCCTRL(
			*this, "NativeDiscretisationFieldCheckBox",wxCheckBox);
		Computed_field *native_discretization_field=FE_field_chooser->get_object();
		FE_field_chooser->set_object(
			FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_read_only_with_fe_field, native_discretization_field,
				scene_editor->field_manager));
		if (nativediscretizationcheckbox->IsChecked())
		{
			FE_chooser_panel->Enable();
		}
		else
		{
			FE_chooser_panel->Disable();
		}
	}
}

void AddGraphic(Cmiss_graphic *graphic_to_copy, enum Cmiss_graphic_type graphic_type)
{
	Cmiss_graphic *graphic;
	int return_code;

	if (graphic =
		CREATE(Cmiss_graphic)(graphic_type))
	{
		return_code = 1;
		if (graphic_to_copy)
		{
			/* copy current graphic into new graphic */
			return_code = Cmiss_graphic_copy_without_graphics_object(graphic,
				graphic_to_copy);
			/* make sure new graphic is visible */
			Cmiss_graphic_set_visibility(graphic,1);
		}
		else
		{
			/* set materials for all graphic */
			Cmiss_graphic_set_material(graphic,
				scene_editor->default_material);
			Cmiss_graphic_set_label_field(graphic,
				(struct Computed_field *)NULL, scene_editor->default_font);
			Cmiss_graphic_set_selected_material(graphic,
				FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
					"default_selected",scene_editor->graphical_material_manager));
			/* for data_points, ensure either there are points with
				 default_coordinate defined at them. If not, and any have
				 the element_xi_coordinate field defined over them, use that */
			if (CMISS_GRAPHIC_DATA_POINTS==graphic_type)
			{
				FE_region *data_fe_region = FE_region_get_data_FE_region(
					Cmiss_region_get_FE_region(
						Cmiss_rendition_get_region(
							scene_editor->edit_rendition)));
				Computed_field *default_coordinate_field=
					Cmiss_rendition_get_default_coordinate_field(
						scene_editor->edit_rendition);
				if (!FE_region_get_first_FE_node_that(data_fe_region,
						FE_node_has_Computed_field_defined,
						(void *)default_coordinate_field))
				{
					MANAGER(Computed_field) *computed_field_manager=  
						Cmiss_region_get_Computed_field_manager(
							Cmiss_rendition_get_region(scene_editor->edit_rendition));
					Computed_field *element_xi_coordinate_field;
					if ((element_xi_coordinate_field=
							FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
								"element_xi_coordinate",computed_field_manager)) &&
						FE_region_get_first_FE_node_that(data_fe_region,
							FE_node_has_Computed_field_defined,
							(void *)element_xi_coordinate_field))
					{
						Cmiss_graphic_set_coordinate_field(graphic,
							element_xi_coordinate_field);
					}
				}
			}
			/* set iso_scalar_field for iso_surfaces */
			if (CMISS_GRAPHIC_ISO_SURFACES==graphic_type)
			{
				Computed_field *iso_scalar_field;
				MANAGER(Computed_field) *computed_field_manager = 
					Cmiss_region_get_Computed_field_manager(
						Cmiss_rendition_get_region(scene_editor->edit_rendition));
				if (iso_scalar_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
							Computed_field_is_scalar,(void *)NULL,computed_field_manager))
				{
					double iso_value_default = 0;
					if (!Cmiss_graphic_set_iso_surface_parameters(graphic,
							iso_scalar_field,/*number_of_iso_values*/1,&iso_value_default,
							/*first_iso_value*/0.0, /*last_iso_value*/0.0,
							/*decimation_threshold*/0.0))
					{
						return_code=0;
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"No scalar fields defined");
					return_code=0;
				}
			}
			/* set initial glyph for graphic types that use them */
			if ((CMISS_GRAPHIC_NODE_POINTS==graphic_type)||
				(CMISS_GRAPHIC_DATA_POINTS==graphic_type)||
				(CMISS_GRAPHIC_ELEMENT_POINTS==graphic_type)||
				(CMISS_GRAPHIC_STATIC==graphic_type))
			{
				/* default to point glyph for fastest possible display */
				GT_object *glyph, *old_glyph;
				Graphic_glyph_scaling_mode glyph_scaling_mode;
				Triple glyph_centre,glyph_scale_factors,glyph_size;
				Computed_field *orientation_scale_field, *variable_scale_field; ;
				glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
					scene_editor->glyph_list);
				if (!(Cmiss_graphic_get_glyph_parameters(graphic,
							&old_glyph, &glyph_scaling_mode ,glyph_centre, glyph_size,
							&orientation_scale_field, glyph_scale_factors,
							&variable_scale_field) &&
						Cmiss_graphic_set_glyph_parameters(graphic,glyph,
							glyph_scaling_mode, glyph_centre, glyph_size,
							orientation_scale_field, glyph_scale_factors,
							variable_scale_field)))
				{
					display_message(WARNING_MESSAGE,"No glyphs defined");
					return_code=0;
				}
			}
			if (CMISS_GRAPHIC_VOLUMES==graphic_type)
			{
				/* must have a volume texture */
				VT_volume_texture *volume_texture;
				if (volume_texture=FIRST_OBJECT_IN_MANAGER_THAT(VT_volume_texture)(
							(MANAGER_CONDITIONAL_FUNCTION(VT_volume_texture) *)NULL,
							(void *)NULL,scene_editor->volume_texture_manager))
				{
					if (!Cmiss_graphic_set_volume_texture(graphic,
							volume_texture))
					{
						return_code=0;
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"No volume textures defined");
					return_code=0;
				}
			}
			/* set stream_vector_field for STREAMLINES */
			if (CMISS_GRAPHIC_STREAMLINES==graphic_type)
			{
				Streamline_type streamline_type;
				Computed_field *stream_vector_field;
				float streamline_length,streamline_width;
				int reverse_track;
				MANAGER(Computed_field) *computed_field_manager = 
					Cmiss_region_get_Computed_field_manager(
						Cmiss_rendition_get_region(scene_editor->edit_rendition));
				Cmiss_graphic_get_streamline_parameters(
					graphic,&streamline_type,&stream_vector_field,&reverse_track,
					&streamline_length,&streamline_width);
				if (stream_vector_field=
					FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
						Computed_field_is_stream_vector_capable,(void *)NULL,
						computed_field_manager))
				{
					if (!Cmiss_graphic_set_streamline_parameters(
								graphic,streamline_type,stream_vector_field,reverse_track,
								streamline_length,streamline_width))
					{
						return_code=0;
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"No vector fields defined");
					return_code=0;
				}
			}
			/* set use_element_type for element_points */
			if (return_code && (CMISS_GRAPHIC_ELEMENT_POINTS == graphic_type))
			{
				Cmiss_graphic_set_use_element_type(graphic,USE_ELEMENTS);
			}
		}
		if (return_code && Cmiss_rendition_add_graphic(
					scene_editor->edit_rendition, graphic, 0))
		{
			//Update the list of graphic
			wxCheckListBox *graphicalitemschecklist =  XRCCTRL(*this, "CmissGraphicListBox",wxCheckListBox);
			graphicalitemschecklist->SetSelection(wxNOT_FOUND);
			graphicalitemschecklist->Clear();
			for_each_graphic_in_Cmiss_rendition(scene_editor->edit_rendition,
				Scene_editor_add_graphic_item, (void *)scene_editor);
			graphicalitemschecklist->SetSelection((graphicalitemschecklist->GetCount()-1));
			sceneediting = 
				XRCCTRL(*this, "SceneEditing", wxScrolledWindow);
			sceneediting->Freeze();
			Cmiss_graphic *temp_graphic = Cmiss_rendition_get_graphic_at_position(
				scene_editor->edit_rendition, graphicalitemschecklist->GetCount());
			Scene_editor_wx_update_graphic_type(temp_graphic);
			Cmiss_graphic_destroy(&temp_graphic);
			Scene_editor_wx_update_graphic_widgets();
			Scene_editor_autoapply(scene_editor->rendition,
				scene_editor->edit_rendition);
			sceneediting->Thaw();
			sceneediting->Layout();
		}
		if (!return_code)
		{
			DESTROY(Cmiss_graphic)(&graphic);
		}
	}
}

void AddToGraphicList(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	AddGraphic(scene_editor->current_graphic,
		scene_editor->current_graphic_type);
}

void AddGraphicItemFromMenu(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	Cmiss_graphic *graphic = NULL;
	enum Cmiss_graphic_type current_graphic_type = CMISS_GRAPHIC_LINES;

	if (AddMenuItemNode == event.GetId())
	{
		current_graphic_type = CMISS_GRAPHIC_NODE_POINTS;
	} 
	else if (AddMenuItemData == event.GetId())
	{
		current_graphic_type = CMISS_GRAPHIC_DATA_POINTS;
	}
	else if (AddMenuItemLines == event.GetId())
	{
		current_graphic_type = CMISS_GRAPHIC_LINES;
	}
	else if (AddMenuItemCylinders == event.GetId())
	{
		current_graphic_type = CMISS_GRAPHIC_CYLINDERS;
	}
	else if (AddMenuItemSurfaces == event.GetId())
	{
		current_graphic_type = CMISS_GRAPHIC_SURFACES;
	}
	else if (AddMenuItemIsoSurfaces == event.GetId())
	{
		current_graphic_type = CMISS_GRAPHIC_ISO_SURFACES;
	}
	else if (AddMenuItemElement == event.GetId())
	{
		current_graphic_type = CMISS_GRAPHIC_ELEMENT_POINTS;
	}
	else if (AddMenuItemStreamlines == event.GetId())
	{
		current_graphic_type = CMISS_GRAPHIC_STREAMLINES;
	}
	else if (AddMenuItemStaticGraphic == event.GetId())
	{
		current_graphic_type = CMISS_GRAPHIC_STATIC;
	}
	graphic = first_graphic_in_Cmiss_rendition_that(
		scene_editor->edit_rendition, Cmiss_graphic_type_matches,
		(void *)current_graphic_type);
	AddGraphic(graphic,	current_graphic_type);
}

void RemoveFromGraphicList(wxCommandEvent &event)
{
	 unsigned int position;

	 USE_PARAMETER(event);

	 if 	(scene_editor->edit_rendition)
	 {
			graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
			position = Cmiss_rendition_get_graphic_position(
				 scene_editor->edit_rendition, scene_editor->current_graphic);
			Cmiss_rendition_remove_graphic(
				 scene_editor->edit_rendition, scene_editor->current_graphic);
			/* inform the client of the changes */
			if (graphicalitemschecklist->GetCount()>1)
			{
				graphicalitemschecklist->SetSelection(wxNOT_FOUND);
				graphicalitemschecklist->Clear();
				for_each_graphic_in_Cmiss_rendition(scene_editor->edit_rendition,
					Scene_editor_add_graphic_item, (void *)scene_editor);
				 if (position>1)
				 {
					 Cmiss_graphic *temp_graphic = NULL;
					 if (graphicalitemschecklist->GetCount()>=position)
					 {
						 graphicalitemschecklist->SetSelection(position-1);
						 temp_graphic = Cmiss_rendition_get_graphic_at_position(
							 scene_editor->edit_rendition, position);
					 }
					 else
					 {
						 graphicalitemschecklist->SetSelection(position-2);
						 temp_graphic = Cmiss_rendition_get_graphic_at_position(
							 scene_editor->edit_rendition, position - 1);
					 }
					 if (temp_graphic)
					 {
						 Scene_editor_wx_update_graphic_type(temp_graphic);
						 Scene_editor_wx_update_graphic_widgets();
						 Cmiss_graphic_destroy(&temp_graphic);
					 }
						Scene_editor_autoapply(scene_editor->rendition,
							scene_editor->edit_rendition);
				 }
			}
			else
			{
				graphicalitemschecklist->SetSelection(wxNOT_FOUND);
				graphicalitemschecklist->Clear();
				wxScrolledWindow *sceneeditingpanel= XRCCTRL(*this, "SceneEditing",wxScrolledWindow);
				sceneeditingpanel->Disable();
				sceneeditingpanel->Hide();
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				"graphical_element_editor_delete_button_CB.  Invalid argument(s)");
	 }
	 /*check if autoapply*/
	 Scene_editor_autoapply(scene_editor->rendition,
		 scene_editor->edit_rendition);
}

void MoveUpInGraphicList(wxCommandEvent &event)
{
	 USE_PARAMETER(event);
	 int position;
	 Cmiss_graphic *graphic;
	 if (scene_editor->edit_rendition)
	 {
		 if (1 < (position = Cmiss_rendition_get_graphic_position(
								scene_editor->edit_rendition, scene_editor->current_graphic)))
		 {
			 graphic = scene_editor->current_graphic;
			 ACCESS(Cmiss_graphic)(graphic);
			 Cmiss_rendition_remove_graphic(scene_editor->edit_rendition,
				 scene_editor->current_graphic);
			 Cmiss_rendition_add_graphic(scene_editor->edit_rendition,
				 scene_editor->current_graphic, position - 1);
			 DEACCESS(Cmiss_graphic)(&graphic);
			 graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
			 graphicalitemschecklist->SetSelection(wxNOT_FOUND);
			 graphicalitemschecklist->Clear();
			 for_each_graphic_in_Cmiss_rendition(scene_editor->edit_rendition,
				 Scene_editor_add_graphic_item, (void *)scene_editor);
			 graphicalitemschecklist->SetSelection(position-2);
			 Cmiss_graphic *temp_graphic = Cmiss_rendition_get_graphic_at_position(
				 scene_editor->edit_rendition, position-1);
			 Scene_editor_wx_update_graphic_type(temp_graphic);
			 Scene_editor_wx_update_graphic_widgets();
			 Cmiss_graphic_destroy(&temp_graphic);
			 Scene_editor_autoapply(scene_editor->rendition,
				 scene_editor->edit_rendition);
			 /* By default the graphic name is the position, so it needs to be updated
					even though the graphic hasn't actually changed */
			 /* inform the client of the change */
		 }
	 }
	 else
	 {
		 display_message(ERROR_MESSAGE,
			 "MoveUpInGraphicList.  Invalid argument(s)");
	 }
}

	void MoveDownInGraphicList(wxCommandEvent &event)
	{

		int position;
		Cmiss_graphic *graphic;

	  USE_PARAMETER(event);

		if 	(scene_editor->edit_rendition)
		{
			if (Cmiss_rendition_get_number_of_graphic(
						scene_editor->edit_rendition) >
				(position = Cmiss_rendition_get_graphic_position(
					 scene_editor->edit_rendition, scene_editor->current_graphic)))
			{
				graphic = scene_editor->current_graphic;
				ACCESS(Cmiss_graphic)(graphic);
				Cmiss_rendition_remove_graphic(scene_editor->edit_rendition,
					scene_editor->current_graphic);
				Cmiss_rendition_add_graphic(scene_editor->edit_rendition,
					scene_editor->current_graphic, position + 1);
				DEACCESS(Cmiss_graphic)(&graphic);
				//		Graphical_element_editor_update_graphic_list(scene_editor);
				graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
				graphicalitemschecklist->SetSelection(wxNOT_FOUND);
				graphicalitemschecklist->Clear();
				for_each_graphic_in_Cmiss_rendition(scene_editor->edit_rendition,
					Scene_editor_add_graphic_item, (void *)scene_editor);
				graphicalitemschecklist->SetSelection(position);
				Cmiss_graphic *temp_graphic = Cmiss_rendition_get_graphic_at_position(
					scene_editor->edit_rendition, position+1);
				Scene_editor_wx_update_graphic_type(temp_graphic);
				Scene_editor_wx_update_graphic_widgets();
				Cmiss_graphic_destroy(&temp_graphic);
				Scene_editor_autoapply(scene_editor->rendition,
					scene_editor->edit_rendition);
				/* By default the graphic name is the position, so it needs to be updated
					 even though the graphic hasn't actually changed */
				/* inform the client of the change */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"MoveDownInGraphicList.  Invalid argument(s)");
		}
	}

	void GraphicEditorNameText(wxCommandEvent &event)
	{
		char new_name[200],*text_entry;
		const char *name = NULL;
		wxString temp;

	  USE_PARAMETER(event);

		if (scene_editor->current_graphic)
		{
			graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
			Cmiss_graphic_get_name(scene_editor->current_graphic,
				&name);
			nametextfield = XRCCTRL(*this, "NameTextField", wxTextCtrl);
			temp = nametextfield->GetValue();
			text_entry = const_cast<char *>(temp.c_str());
			if (text_entry)
			{
				sscanf(text_entry, "%s", new_name);
				if (strcmp(name, new_name))
				{
					Cmiss_graphic_set_name(
						scene_editor->current_graphic, new_name);
						/* inform the client of the change */
					nametextfield->SetValue(new_name);
					Scene_editor_autoapply(scene_editor->rendition,
						scene_editor->edit_rendition);
					Scene_editor_renew_label_on_list(scene_editor->current_graphic);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"graphic_editor_constant_radius_text_CB.  Missing text");
			}
			DEALLOCATE(name);
		}
		else
		{
		 	display_message(ERROR_MESSAGE,
		      "graphic_editor_constant_radius_text_CB.  Invalid argument(s)");
		}
	}

	void CoordinateFieldChecked(wxCommandEvent &event)
	{
		Computed_field *coordinate_field;
		USE_PARAMETER(event);
		coordinatefieldcheckbox = XRCCTRL(*this, "CoordinateFieldCheckBox",wxCheckBox);
		coordinate_field_chooser_panel = XRCCTRL(*this, "CoordinateFieldChooserPanel",wxPanel);
		if (coordinatefieldcheckbox->IsChecked())
		{
			coordinate_field=coordinate_field_chooser->get_object();
			coordinate_field_chooser_panel->Enable();
		}
		else
		{
			coordinate_field=(Computed_field *)NULL;
			coordinate_field_chooser_panel->Disable();
		}
		Cmiss_graphic_set_coordinate_field(
			scene_editor->current_graphic, coordinate_field);
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
		Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	}

	void EnterRadius(wxCommandEvent &event)
 	{
		float constant_radius, scale_factor;
		double temp;
		struct Computed_field *radius_scalar_field;

		USE_PARAMETER(event);

		constantradiustextctrl=XRCCTRL(*this, "ConstantRadiusTextCtrl",wxTextCtrl);
		(constantradiustextctrl->GetValue()).ToDouble(&temp);
		constant_radius=(float)temp;
		scalefactorstextctrl=XRCCTRL(*this,"ScaleFactorsTextCtrl",wxTextCtrl);
		(scalefactorstextctrl->GetValue()).ToDouble(&temp);
		scale_factor=(float)temp;	
		radiusscalarcheckbox=XRCCTRL(*this, "RadiusScalarCheckBox",wxCheckBox);
		radius_scalar_chooser_panel=XRCCTRL(*this, "RadiusScalarChooserPanel",wxPanel);
		scalefactor = XRCCTRL(*this,"ScaleFactorLabel", wxStaticText);
		if (radiusscalarcheckbox->IsChecked())
			{
				scalefactorstextctrl->Enable();
				radius_scalar_chooser_panel->Enable();
				scalefactor ->Enable();
		  	   radius_scalar_field=radius_scalar_chooser->get_object();
			}
		else	
			{
				radius_scalar_field=(Computed_field *)NULL;
				radius_scalar_chooser_panel->Disable();
				scalefactorstextctrl->Disable();	
				scalefactor ->Disable();
			}
		Cmiss_graphic_set_radius_parameters(scene_editor->current_graphic,constant_radius,
																							scale_factor,radius_scalar_field);
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
		Scene_editor_renew_label_on_list(scene_editor->current_graphic);
 	}

	void EnterIsoScalar(wxCommandEvent &event)
	{
		double *current_iso_values = NULL,decimation_threshold, *iso_values = NULL,
			current_first_iso_value, current_last_iso_value, first_iso_value,
			last_iso_value;
		char *text_entry = NULL, temp_string[50], *vector_temp_string = NULL;
		int allocated_length, changed_value = 0, error, i, length,
			new_number_of_iso_values, number_of_iso_values,
			offset, valid_value;
		struct Computed_field *scalar_field = NULL;
#define VARIABLE_LENGTH_ALLOCATION_STEP (10)

	  USE_PARAMETER(event);
		
		if (scene_editor && scene_editor->current_graphic)
		{
			Cmiss_graphic_get_iso_surface_parameters(
				scene_editor->current_graphic,&scalar_field,&number_of_iso_values,
				&current_iso_values,
				&current_first_iso_value, &current_last_iso_value,
				&decimation_threshold);
			isovaluelistradiobutton=XRCCTRL(*this,"IsoValueListRadioButton",wxRadioButton);
			isovaluesequenceradiobutton=XRCCTRL(*this,"IsoValueSequenceRadioButton",wxRadioButton);
			
			isoscalartextctrl=XRCCTRL(*this,"IsoScalarTextCtrl",wxTextCtrl);
			isovaluesequencenumbertextctrl=XRCCTRL(*this,"IsoValueSequenceNumberTextCtrl",wxTextCtrl);
			isovaluesequencefirsttextctrl=XRCCTRL(*this,"IsoValueSequenceFirstTextCtrl",wxTextCtrl);
			isovaluesequencelasttextctrl=XRCCTRL(*this,"IsoValueSequenceLastTextCtrl",wxTextCtrl);
			
			if (isovaluelistradiobutton->GetValue())
			{
				isoscalartextctrl->Enable();
				isovaluesequencenumbertextctrl->Disable();
				isovaluesequencefirsttextctrl->Disable();
				isovaluesequencelasttextctrl->Disable();
				text_entry = const_cast<char *>(isoscalartextctrl->GetValue().c_str());
				if (text_entry)
				{
					i = 0;
					valid_value = 1;
					changed_value = 0;
					offset = 0;
					iso_values = (double *)NULL;
					allocated_length = 0;
					while (valid_value)
					{
						if (i >= allocated_length)
						{
							REALLOCATE(iso_values, iso_values, double, 
								allocated_length + VARIABLE_LENGTH_ALLOCATION_STEP);
							allocated_length += VARIABLE_LENGTH_ALLOCATION_STEP;
						}
						if (0 < sscanf(text_entry+offset,"%lg%n",&iso_values[i], &length))
						{
							offset += length;
							if ((i >= number_of_iso_values) || (!current_iso_values) ||
								(iso_values[i] != current_iso_values[i]))
							{
								changed_value = 1;
							}
							i++;
						}
						else
						{
							valid_value = 0;
						}
					}
					if (changed_value || (number_of_iso_values != i))
					{
						number_of_iso_values = i;
						Cmiss_graphic_set_iso_surface_parameters(
							scene_editor->current_graphic,scalar_field,
							number_of_iso_values, iso_values,
							first_iso_value, last_iso_value, decimation_threshold);
						Scene_editor_autoapply(scene_editor->rendition,
							scene_editor->edit_rendition);
						Scene_editor_renew_label_on_list(scene_editor->current_graphic);		/* inform the client of the change */
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"EnterIsoScalar.  Missing text");
				}
				if (current_iso_values)
				{
					DEALLOCATE(current_iso_values);
				}
				
				vector_temp_string = (char *)NULL;
				error = 0;
				for (i = 0 ; !error && (i < number_of_iso_values) ; i++)
				{
					sprintf(temp_string,"%g ",iso_values[i]);
					append_string(&vector_temp_string, temp_string, &error);
				}
				if (vector_temp_string)
				{
					isoscalartextctrl->SetValue(vector_temp_string);
					DEALLOCATE(vector_temp_string);
				}
				if (iso_values)
				{
					DEALLOCATE(iso_values);
				}
			}
			else if (isovaluesequenceradiobutton->GetValue())
			{
				if (current_iso_values)
				{
					DEALLOCATE(current_iso_values);
				}
				isoscalartextctrl->Disable();
				isovaluesequencenumbertextctrl->Enable();
				isovaluesequencefirsttextctrl->Enable();
				isovaluesequencelasttextctrl->Enable();
				text_entry=const_cast<char *>(isovaluesequencenumbertextctrl->GetValue().c_str());
				if (text_entry)
				{
					sscanf(text_entry,"%d",&new_number_of_iso_values);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"graphic_editor_streamline_width_text_CB.  Missing text");
				}
				
				text_entry=const_cast<char *>(isovaluesequencefirsttextctrl->GetValue().c_str());
				if (text_entry)
				{
					sscanf(text_entry,"%lg",&first_iso_value);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"graphic_editor_streamline_width_text_CB.  Missing text");
				}
				
				text_entry=const_cast<char *>(isovaluesequencelasttextctrl->GetValue().c_str());
				if (text_entry)
				{
					sscanf(text_entry,"%lg",&last_iso_value);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"graphic_editor_streamline_width_text_CB.  Missing text");
				}
				
				if ((changed_value != number_of_iso_values) ||
					(current_first_iso_value != first_iso_value) ||
					(current_last_iso_value != last_iso_value))
				{
					Cmiss_graphic_set_iso_surface_parameters(
						scene_editor->current_graphic,scalar_field,
						new_number_of_iso_values, /*iso value list*/(double *)NULL,
					first_iso_value, last_iso_value, decimation_threshold);
					
					/* inform the client of the change */
					Scene_editor_autoapply(scene_editor->rendition,
						scene_editor->edit_rendition);
					Scene_editor_renew_label_on_list(scene_editor->current_graphic);
				}
				
				/* always restore strings to actual value in use */
				sprintf(temp_string,"%d",new_number_of_iso_values);
				isovaluesequencenumbertextctrl->SetValue(temp_string);			 
				
				sprintf(temp_string,"%g",first_iso_value);
				isovaluesequencefirsttextctrl->SetValue(temp_string);			 
				
				sprintf(temp_string,"%g",last_iso_value);
				isovaluesequencelasttextctrl->SetValue(temp_string);			 
				
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"wx_graphic_editor_enter_iso_scalar.  Missing iso value option.");
				
			}
		}
	}

void EnterGlyphCentre(wxCommandEvent &event)
{
	char *text_entry, temp_string[50];
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	static int number_of_components=3;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Parse_state *temp_state;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	USE_PARAMETER(event);

	centretextctrl=XRCCTRL(*this,"CentreTextCtrl",wxTextCtrl);
	
	if (scene_editor->current_graphic)
	{
		if (Cmiss_graphic_get_glyph_parameters(
					scene_editor->current_graphic, &glyph, &glyph_scaling_mode, glyph_centre,
					glyph_size, &orientation_scale_field, glyph_scale_factors,
					&variable_scale_field))
		{
			/* Get the text string */
			if (text_entry = duplicate_string(
						const_cast<char *>(centretextctrl->GetValue().c_str())))
			{
				/* clean up spaces? */
				if (temp_state=create_Parse_state(text_entry))
				{
					set_float_vector(temp_state,glyph_centre,
						(void *)&number_of_components);
					Cmiss_graphic_set_glyph_parameters(
						scene_editor->current_graphic, glyph, glyph_scaling_mode,
						glyph_centre, glyph_size, orientation_scale_field,
						glyph_scale_factors, variable_scale_field);
					Scene_editor_autoapply(scene_editor->rendition,
						scene_editor->edit_rendition);
					Scene_editor_renew_label_on_list(scene_editor->current_graphic);
					destroy_Parse_state(&temp_state);
					Cmiss_graphic_get_glyph_parameters(
						scene_editor->current_graphic, &glyph, &glyph_scaling_mode, glyph_centre,
						glyph_size, &orientation_scale_field, glyph_scale_factors,
						&variable_scale_field);
					sprintf(temp_string,"%g,%g,%g",
						glyph_centre[0],glyph_centre[1],glyph_centre[2]);
					centretextctrl->ChangeValue(temp_string);
				}
				DEALLOCATE(text_entry);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_glyph_centre_text_CB.  Missing text");
			}
		}
	}
}

void EnterGlyphSize(wxCommandEvent &event)
{
	char *text_entry, temp_string[50];
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Parse_state *temp_state;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	USE_PARAMETER(event);

	baseglyphsizetextctrl=XRCCTRL(*this,"BaseGlyphSizeTextCtrl",wxTextCtrl);
	if (scene_editor->current_graphic)
	{
		if (Cmiss_graphic_get_glyph_parameters(
					scene_editor->current_graphic, &glyph, &glyph_scaling_mode, glyph_centre,
					glyph_size, &orientation_scale_field, glyph_scale_factors,
					&variable_scale_field))
		{
			/* Get the text string */
			if (text_entry = 
				duplicate_string(const_cast<char *>(baseglyphsizetextctrl->GetValue().c_str())))
			{
				/* clean up spaces? */
				if (temp_state=create_Parse_state(text_entry))
				{
					set_special_float3(temp_state,glyph_size,const_cast<char *>("*"));
					Cmiss_graphic_set_glyph_parameters(
						scene_editor->current_graphic, glyph, glyph_scaling_mode,
						glyph_centre, glyph_size, orientation_scale_field,
						glyph_scale_factors, variable_scale_field);
					/* inform the client of the change */
					Scene_editor_autoapply(scene_editor->rendition,
						scene_editor->edit_rendition);
					Scene_editor_renew_label_on_list(scene_editor->current_graphic);
					Cmiss_graphic_get_glyph_parameters(
						scene_editor->current_graphic, &glyph, &glyph_scaling_mode, glyph_centre,
						glyph_size, &orientation_scale_field, glyph_scale_factors,
						&variable_scale_field);
					sprintf(temp_string,"%g*%g*%g",
						glyph_size[0],glyph_size[1],glyph_size[2]);
					baseglyphsizetextctrl->ChangeValue(temp_string);
					destroy_Parse_state(&temp_state);
				}
				DEALLOCATE(text_entry);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_glyph_size_text_CB.  Missing text");
			}
		}
	}
}

	void   EnterGlyphScale(wxCommandEvent &event)
	{
	char *text_entry,temp_string[50];
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	struct Parse_state *temp_state;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	USE_PARAMETER(event);
	glyphscalefactorstextctrl=XRCCTRL(*this,"GlyphScaleFactorsTextCtrl",wxTextCtrl);
	orientation_scale_field_chooser_panel=XRCCTRL(*this,	"OrientationScaleChooserPanel",wxPanel);
	glyphscalefactorstext	=XRCCTRL(*this,"GlyphScaleFactorsText",wxStaticText);
	glyphscalefactorstextctrl=XRCCTRL(*this,"GlyphScaleFactorsTextCtrl",wxTextCtrl);
	orientationscalecheckbox=XRCCTRL(*this,"OrientationScaleCheckBox",wxCheckBox);

		if (scene_editor->current_graphic)
		{
			if (Cmiss_graphic_get_glyph_parameters(
				scene_editor->current_graphic, &glyph, &glyph_scaling_mode, glyph_centre,
				glyph_size, &orientation_scale_field, glyph_scale_factors,
				&variable_scale_field))
			{
				/* Get the text string */
				if (text_entry = duplicate_string(
							const_cast<char *>(glyphscalefactorstextctrl->GetValue().c_str())))
				{
					/* clean up spaces? */
					if (temp_state=create_Parse_state(text_entry))
					{
						set_special_float3(temp_state,glyph_scale_factors,const_cast<char *>("*"));
						if (orientationscalecheckbox->IsChecked())
						{
							orientation_scale_field_chooser_panel->Enable();
							glyphscalefactorstext->Enable();
							glyphscalefactorstextctrl->Enable();					
							orientation_scale_field=orientation_scale_field_chooser->get_object();
						}
						else
						{
							orientation_scale_field_chooser_panel->Disable();
							glyphscalefactorstext->Disable();
							glyphscalefactorstextctrl->Disable();					
							orientation_scale_field=(Computed_field *)NULL;
						}
						Cmiss_graphic_set_glyph_parameters(
							scene_editor->current_graphic, glyph, glyph_scaling_mode,
							glyph_centre, glyph_size, orientation_scale_field,
							glyph_scale_factors, variable_scale_field);
						Scene_editor_autoapply(scene_editor->rendition,
						        scene_editor->edit_rendition);
						Scene_editor_renew_label_on_list(scene_editor->current_graphic);
						destroy_Parse_state(&temp_state);
						Cmiss_graphic_get_glyph_parameters(
							scene_editor->current_graphic, &glyph, &glyph_scaling_mode, glyph_centre,
							glyph_size, &orientation_scale_field, glyph_scale_factors,
							&variable_scale_field);
						sprintf(temp_string,"%g*%g*%g",
							glyph_scale_factors[0],glyph_scale_factors[1],glyph_scale_factors[2]);
						glyphscalefactorstextctrl->ChangeValue(temp_string);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"settings_editor_glyph_scale_factors_text_CB.  Missing text");
				}
				DEALLOCATE(text_entry);
			}
		}	}

   void OverlayChecked(wxCommandEvent &event)
	{
		wxTextCtrl *overlay_textctrl = XRCCTRL(*this, "OverlayTextCtrl", wxTextCtrl);
		char temp_string[50];
		Cmiss_graphic_enable_overlay(scene_editor->current_graphic,event.IsChecked());
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
		if (event.IsChecked())
		{
			overlay_textctrl->Enable();
			sprintf(temp_string,"%d ",Cmiss_graphic_get_overlay_order(
								scene_editor->current_graphic));
			overlay_textctrl->SetValue(temp_string);
		}
		else
		{
			overlay_textctrl->Disable();	
		}
	}

  void OverlayEntered(wxCommandEvent &event)
  {
		wxTextCtrl *overlay_textctrl = XRCCTRL(*this, "OverlayTextCtrl", wxTextCtrl);
		int new_overlay_order;

		USE_PARAMETER(event);
		const char *text_entry=const_cast<char *>(overlay_textctrl->GetValue().c_str());
		sscanf(text_entry,"%d",&new_overlay_order);
		Cmiss_graphic_set_overlay_order(scene_editor->current_graphic, new_overlay_order);
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
	}


   void VariableScaleChecked(wxCommandEvent &event)
{
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct GT_object *glyph;
	Triple glyph_centre,glyph_scale_factors,glyph_size;

	USE_PARAMETER(event);

	Cmiss_graphic_get_glyph_parameters(
		 scene_editor->current_graphic, &glyph, &glyph_scaling_mode, glyph_centre,
		 glyph_size, &orientation_scale_field, glyph_scale_factors,
		 &variable_scale_field);
	variable_scale_field_chooser_panel=XRCCTRL(*this,"VariableScaleChooserPanel",wxPanel);
	variablescalecheckbox=XRCCTRL(*this,"VariableScaleCheckBox",wxCheckBox);
	if (variablescalecheckbox->IsChecked())
	{
		variable_scale_field_chooser_panel->Enable();				
		variable_scale_field=variable_scale_field_chooser->get_object();
	}
	else
	{
		variable_scale_field_chooser_panel->Disable();		
		variable_scale_field=(Computed_field *)NULL;
	}
        Cmiss_graphic_set_glyph_parameters(
	  scene_editor->current_graphic, glyph, glyph_scaling_mode,
	  glyph_centre, glyph_size, orientation_scale_field,
	  glyph_scale_factors, variable_scale_field);
	Scene_editor_autoapply(scene_editor->rendition,
	  scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
}

void LabelChecked(wxCommandEvent &event)
{
	struct Computed_field *label_field;
	struct Graphics_font *font;

	USE_PARAMETER(event);
	labelcheckbox=XRCCTRL(*this,"LabelCheckBox",wxCheckBox);
	label_chooser_panel = XRCCTRL(*this,"LabelChooserPanel",wxPanel);
	Cmiss_graphic_get_label_field(scene_editor->current_graphic, 
		&label_field, &font);
	 if (labelcheckbox->IsChecked())
	 {
		 label_chooser_panel->Enable();				
		 label_field=label_field_chooser->get_object();
	 }
	 else
	 {
		 label_chooser_panel->Disable();		
		 label_field=(Computed_field *)NULL;
	 }
	Cmiss_graphic_set_label_field(scene_editor->current_graphic,
		label_field, font);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
}

	void VisibilityFieldChecked(wxCommandEvent &event)
{
	struct Computed_field *visibility_field;

	USE_PARAMETER(event);
	visibility_field_checkbox=XRCCTRL(*this,"VisibilityFieldCheckBox",wxCheckBox);
	visibility_field_chooser_panel = XRCCTRL(*this,"VisibilityFieldChooserPanel",wxPanel);
	if (visibility_field_checkbox->IsChecked())
	{
		Cmiss_graphic_get_visibility_field(scene_editor->current_graphic, 
			&visibility_field);
		visibility_field_chooser_panel->Enable();				
		visibility_field=visibility_field_chooser->get_object();
	}
	else
	{
		visibility_field_chooser_panel->Disable();		
		visibility_field=(Computed_field *)NULL;
	}
	Cmiss_graphic_set_visibility_field(scene_editor->current_graphic, 
		visibility_field);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
}

void EnterElementDiscretization(wxCommandEvent &event)
{
	char *text_entry,temp_string[50];
	struct Element_discretization discretization;
	struct Parse_state *temp_state;

	USE_PARAMETER(event);
	discretizationtextctrl=XRCCTRL(*this,"DiscretizationTextCtrl",wxTextCtrl);

	if (scene_editor->current_graphic)
	{
		/* Get the text string */
		discretizationtextctrl=XRCCTRL(*this,"DiscretizationTextCtrl",wxTextCtrl);
		if (text_entry =duplicate_string(
					const_cast<char *>(discretizationtextctrl->GetValue().c_str())))
		{
			if (temp_state=create_Parse_state(text_entry))
			{
				if (set_Element_discretization(temp_state,(void *)&discretization,
						(void *)scene_editor->user_interface)&&
					Cmiss_graphic_set_discretization(
						scene_editor->current_graphic,&discretization))
				{
					/* inform the client of the change */
					Scene_editor_autoapply(scene_editor->rendition,
						scene_editor->edit_rendition);
					Scene_editor_renew_label_on_list(scene_editor->current_graphic);
				}
				destroy_Parse_state(&temp_state);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"settings_editor_discretization_text_CB.  "
					"Could not create parse state");
			}
			DEALLOCATE(text_entry);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"settings_editor_discretization_text_CB.  Missing text");
		}
		if (Cmiss_graphic_get_discretization(
					scene_editor->current_graphic,&discretization))
		{
			/* always restore constant_radius to actual value in use */
			sprintf(temp_string,"%d*%d*%d",discretization.number_in_xi1,
				discretization.number_in_xi2,discretization.number_in_xi3);
			discretizationtextctrl->ChangeValue(temp_string);
		}
	}
}

void EnterCircleDiscretization(wxCommandEvent &event)
{
	char temp_string[10];
	int circle_discretization;

	USE_PARAMETER(event);
	
	if (scene_editor->current_graphic)
	{		
		circle_discretization = atoi(const_cast<char *>(
			circlediscretizationtextctrl->GetValue().c_str()));
		/* Get the text string */
		Cmiss_graphic_set_circle_discretization(
			scene_editor->current_graphic,circle_discretization);
		/* inform the client of the change */
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
		Scene_editor_renew_label_on_list(scene_editor->current_graphic);	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"EnterCircleDiscretization. Invalid argument");
	}
 
	/* always restore constant_radius to actual value in use */
	sprintf(temp_string,"%d", 
		Cmiss_graphic_get_circle_discretization(scene_editor->current_graphic));
	circlediscretizationtextctrl->ChangeValue(temp_string);
}

void NativeDiscretizationChecked(wxCommandEvent &event)
{
	nativediscretizationfieldcheckbox=XRCCTRL(
		*this,"NativeDiscretizationFieldCheckBox",wxCheckBox);
	native_discretization_field_chooser_panel=XRCCTRL(
		*this, "NativeDiscretizationFieldChooserPanel",wxPanel);
	FE_field *temp_native_discretization_field;

	USE_PARAMETER(event);

	if (nativediscretizationfieldcheckbox->IsChecked())
	{	
		native_discretization_field_chooser_panel->Enable();
		Computed_field_get_type_finite_element(
			native_discretization_field_chooser->get_object(),
			&temp_native_discretization_field);
	}
	else
	{
		native_discretization_field_chooser_panel->Disable();
		temp_native_discretization_field=(FE_field *)NULL;
	}
	Cmiss_graphic_set_native_discretization_field(
		scene_editor->current_graphic, temp_native_discretization_field);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
}

void SeedElementChecked(wxCommandEvent &event)
{
	 seed_element_panel = XRCCTRL(*this, "SeedElementPanel", wxPanel);
	 seedelementcheckbox = XRCCTRL(*this, "SeedElementCheckBox", wxCheckBox);

	USE_PARAMETER(event);
	 if (seedelementcheckbox->IsChecked())
	 {
			Cmiss_graphic_set_seed_element(scene_editor->current_graphic, seed_element_chooser->get_object());
			seed_element_panel->Enable();
	 }
	 else
	 {
			Cmiss_graphic_set_seed_element(scene_editor->current_graphic,(FE_element*)NULL);
			seed_element_panel->Disable();
	 }
	 Scene_editor_autoapply(scene_editor->rendition,
		 scene_editor->edit_rendition);
	 Scene_editor_renew_label_on_list(scene_editor->current_graphic);

}

void EnterSeedXi(wxCommandEvent &event)
{
	char *text_entry,temp_string[50];
	static int number_of_components=3;
	struct Parse_state *temp_state;
	Triple seed_xi;

	USE_PARAMETER(event);
	xitext=XRCCTRL(*this,"XiText",wxStaticText);
	xitextctrl=XRCCTRL(*this,"XiTextCtrl",wxTextCtrl);
	if (Cmiss_graphic_get_seed_xi(
				scene_editor->current_graphic,seed_xi))
	{
		/* Get the text string */
		if (text_entry = duplicate_string(const_cast<char *>(xitextctrl->GetValue().c_str())))
		{
			/* clean up spaces? */
			if (temp_state=create_Parse_state(text_entry))
			{
				set_float_vector(temp_state,seed_xi,
					(void *)&number_of_components);
				Cmiss_graphic_set_seed_xi(
					scene_editor->current_graphic,seed_xi);
				/* inform the client of the change */
				Scene_editor_autoapply(scene_editor->rendition,
					scene_editor->edit_rendition);
				Scene_editor_renew_label_on_list(scene_editor->current_graphic);
				destroy_Parse_state(&temp_state);
			}
			DEALLOCATE(text_entry);
		}
		else
		{
			display_message(ERROR_MESSAGE,
			   "settings_editor_seed_xi_text_CB.  Missing text");
		}
		/* always re-display the values actually set */
		sprintf(temp_string,"%g,%g,%g",seed_xi[0],seed_xi[1],seed_xi[2]);
		xitextctrl->ChangeValue(temp_string);
	}
}

void EnterLength(wxCommandEvent &event)
{
	char *text_entry,temp_string[50];
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int reverse_track;
	struct Computed_field *stream_vector_field;
	
	USE_PARAMETER(event);
	lengthtextctrl=XRCCTRL(*this,"LengthTextCtrl",wxTextCtrl);
	Cmiss_graphic_get_streamline_parameters(
		scene_editor->current_graphic,&streamline_type,
		&stream_vector_field,&reverse_track,&streamline_length,
		&streamline_width);
	text_entry=const_cast<char *>(lengthtextctrl->GetValue().c_str());
	if (text_entry)
	{
		sscanf(text_entry,"%g",&streamline_length);
		Cmiss_graphic_set_streamline_parameters(
			scene_editor->current_graphic,streamline_type,
			stream_vector_field,reverse_track,streamline_length,
			streamline_width);
		/* inform the client of the change */
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
		Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	}
	else
	 {
		 display_message(ERROR_MESSAGE,
			"settings_editor_streamline_length_text_CB.  Missing text");
	 }
	sprintf(temp_string,"%g",streamline_length);
	lengthtextctrl->SetValue(temp_string);		
}

void EnterWidth(wxCommandEvent &event)
{
	char *text_entry,temp_string[50];
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int reverse_track;
	struct Computed_field *stream_vector_field;

	USE_PARAMETER(event);
	widthtextctrl=XRCCTRL(*this,"WidthTextCtrl",wxTextCtrl);
	Cmiss_graphic_get_streamline_parameters(
		scene_editor->current_graphic,&streamline_type,
		&stream_vector_field,&reverse_track,&streamline_length,
		&streamline_width);
	/* Get the text string */
	text_entry=const_cast<char *>(widthtextctrl->GetValue().c_str());
	if (text_entry)
	{
		sscanf(text_entry,"%g",&streamline_width);
		Cmiss_graphic_set_streamline_parameters(
			scene_editor->current_graphic,streamline_type,
			stream_vector_field,reverse_track,streamline_length,streamline_width);
		/* inform the client of the change */
		Scene_editor_autoapply(scene_editor->rendition,
			scene_editor->edit_rendition);
		Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	}
	else
	{
		display_message(ERROR_MESSAGE,
		   "settings_editor_streamline_width_text_CB.  Missing text");
	}
	/* always restore streamline_width to actual value in use */
	sprintf(temp_string,"%g",streamline_width);
	widthtextctrl->SetValue(temp_string);			 
}

void ReverseChecked(wxCommandEvent &event)
{
	enum Streamline_type streamline_type;
	float streamline_length,streamline_width;
	int reverse_track;
	struct Computed_field *stream_vector_field;
	USE_PARAMETER(event);
	reversecheckbox=XRCCTRL(*this,"ReverseCheckBox",wxCheckBox);
	Cmiss_graphic_get_streamline_parameters(
		scene_editor->current_graphic,&streamline_type,
		&stream_vector_field,&reverse_track,&streamline_length,
		&streamline_width);
	reverse_track = !reverse_track;
	Cmiss_graphic_set_streamline_parameters(
		scene_editor->current_graphic,streamline_type,
		stream_vector_field,reverse_track,streamline_length,
		streamline_width);
	/* inform the client of the change */
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
	reversecheckbox->SetValue(reverse_track);
}

void EnterLineWidth(wxCommandEvent &event)
{
	char *text_entry,temp_string[50];
	int line_width, new_line_width;

	USE_PARAMETER(event);
	linewidthtextctrl=XRCCTRL(*this,"LineWidthTextCtrl",wxTextCtrl);
	line_width = Cmiss_graphic_get_line_width(scene_editor->current_graphic);
	new_line_width = line_width;
	/* Get the text string */
	text_entry=const_cast<char *>(linewidthtextctrl->GetValue().c_str());
	if (text_entry)
	{
		sscanf(text_entry,"%d",&new_line_width);
		if (new_line_width != line_width)
		{
			Cmiss_graphic_set_line_width(
				scene_editor->current_graphic, new_line_width);
			/* inform the client of the change */
			Scene_editor_autoapply(scene_editor->rendition,
				       scene_editor->edit_rendition);	 
			Scene_editor_renew_label_on_list(scene_editor->current_graphic);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
		  "settings_editor_line_width_text_CB.  Missing text");
	}
	/* always restore streamline_width to actual value in use */
	sprintf(temp_string,"%d",new_line_width);
	linewidthtextctrl->SetValue(temp_string);
}

void DataFieldChecked(wxCommandEvent &event)
{
	struct Computed_field *data_field;
	struct Spectrum *spectrum;
	USE_PARAMETER(event);
	datacheckbox=XRCCTRL(*this, "DataCheckBox", wxCheckBox);
	data_chooser_panel=XRCCTRL(*this,"DataChooserPanel",wxPanel);
	spectrumtext=XRCCTRL(*this, "SpectrumText", wxStaticText);
	spectrum_chooser_panel=XRCCTRL(*this,"SpectrumChooserPanel", wxPanel);
	Cmiss_graphic_get_data_spectrum_parameters(
		scene_editor->current_graphic,&data_field,&spectrum);
	if (datacheckbox->IsChecked())
	{
		data_chooser_panel->Enable();
		spectrumtext->Enable();
		spectrum_chooser_panel->Enable();
		data_field = data_field_chooser->get_object();
		spectrum = spectrum_chooser->get_object();
	}
	else
	{
		data_chooser_panel->Disable();
		spectrumtext->Disable();
		spectrum_chooser_panel->Disable();
		data_field=(Computed_field *)NULL;
		spectrum = (Spectrum*)NULL;
	}
	Cmiss_graphic_set_data_spectrum_parameters(
		scene_editor->current_graphic,data_field,spectrum);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);	 
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
}

void TextureCoordFieldChecked(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	struct Computed_field *texture_coord_field;
	texturecoordinatescheckbox=XRCCTRL(*this, "TextureCoordinatesCheckBox", wxCheckBox);
	texture_coordinates_chooser_panel =XRCCTRL(*this, "TextrueCoordinatesChooserPanel", wxPanel);

	if (texturecoordinatescheckbox->IsChecked())
	{
		texture_coordinates_chooser_panel->Enable();
		texture_coord_field= texture_coord_field_chooser->get_object();
	}
	else
	{
		texture_coordinates_chooser_panel->Disable();
		texture_coord_field= (Computed_field *)NULL;
	}
	Cmiss_graphic_set_texture_coordinate_field(
		scene_editor->current_graphic, texture_coord_field);
	/* inform the client of the change */
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
}

void ExteriorChecked(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	exteriorcheckbox=XRCCTRL(*this,"ExteriorCheckBox",wxCheckBox); 
	Cmiss_graphic_set_exterior(scene_editor->current_graphic,
		exteriorcheckbox->IsChecked());
		/* inform the client of the change */
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
}

void FaceChecked(wxCommandEvent &event)
{
	int face;
	USE_PARAMETER(event);
	facecheckbox=XRCCTRL(*this, "FaceCheckBox",wxCheckBox);
	facechoice=XRCCTRL(*this, "FaceChoice",wxChoice);
	if (facecheckbox->IsChecked()) 
	{
		facechoice->Enable();
		face = facechoice->GetSelection();
	}
	else
	{
		facechoice->Disable();
		face= -1;
	}
	Cmiss_graphic_set_face(scene_editor->current_graphic,face);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
 }

void FaceChosen(wxCommandEvent &event)
{
	int face;
	USE_PARAMETER(event);
	facechoice=XRCCTRL(*this, "FaceChoice",wxChoice);
	face = facechoice->GetSelection();
	Cmiss_graphic_set_face(scene_editor->current_graphic,face);
	Scene_editor_autoapply(scene_editor->rendition,
		scene_editor->edit_rendition);
	Scene_editor_renew_label_on_list(scene_editor->current_graphic);
}

void SetBothMaterialChooser(Cmiss_graphic *graphic)
{
	graphical_material_chooser->set_object(Cmiss_graphic_get_material
		(graphic));	
	selected_material_chooser->set_object(Cmiss_graphic_get_selected_material
		(graphic));
}

void SetGraphic(Cmiss_graphic *graphic)
{
		int error,number_of_iso_values, i,reverse_track,line_width, face;
		double decimation_threshold, *iso_values, first_iso_value,
			last_iso_value;
		char temp_string[50], *vector_temp_string;
		struct Computed_field *radius_scalar_field, *iso_scalar_field, 
			*orientation_scale_field, *variable_scale_field,	*label_field, 
			*visibility_field, *xi_point_density_field, *stream_vector_field,
			*data_field, *texture_coord_field;
		float constant_radius,scale_factor,streamline_length,streamline_width;
		struct GT_object *glyph;
		enum Graphic_glyph_scaling_mode glyph_scaling_mode;
		enum Xi_discretization_mode xi_discretization_mode;
		Triple glyph_centre, glyph_size, glyph_scale_factors, seed_xi;
		struct Graphics_font *font;
		struct Element_discretization discretization;
		struct FE_field *native_discretization_field;
		enum Streamline_type streamline_type;
		enum Streamline_data_type streamline_data_type;
		struct Spectrum *spectrum;
		enum Render_type render_type;
		struct FE_element *seed_element;
		struct FE_region *fe_region;

		if (default_coordinate_field_chooser ==NULL)
		{
			scene_editor->default_coordinate_field=
				Cmiss_rendition_get_default_coordinate_field(scene_editor->edit_rendition);
			wxPanel *default_coordinate_field_chooser_panel =
				XRCCTRL(*this, "DefaultCoordinateFieldChooser",wxPanel);
			default_coordinate_field_chooser = 
				new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				(default_coordinate_field_chooser_panel, scene_editor->default_coordinate_field, scene_editor->field_manager,
					(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL, (void *)NULL, scene_editor->user_interface);
			Callback_base< Computed_field* > *default_coordinate_field_callback = 
				new Callback_member_callback< Computed_field*, 
				wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
				(this, &wxSceneEditor::default_coordinate_field_callback);
			default_coordinate_field_chooser->set_callback(default_coordinate_field_callback);
			default_coordinate_field_chooser_panel->Fit();	
		}

	if (FE_field_chooser ==NULL)
	{
		if (scene_editor->edit_rendition != NULL)
		{
			scene_editor->native_discretization_field = Cmiss_rendition_get_native_discretization_field(scene_editor->edit_rendition);
		}
		wxPanel *FE_chooser_panel =
			XRCCTRL(*this, "NativeDiscretisationFieldChooser",wxPanel);			
		FE_field_chooser =
			new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
			(FE_chooser_panel,(Computed_field*)NULL, scene_editor->field_manager,
				Computed_field_is_type_finite_element_iterator, (void *)NULL, scene_editor->user_interface);
		Callback_base< Computed_field* > *native_discretization_field_callback = 
			new Callback_member_callback< Computed_field*, 
			wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
			(this, &wxSceneEditor::native_discretization_field_callback);
		FE_field_chooser->set_callback(native_discretization_field_callback);
		FE_chooser_panel->Fit();
	}
	
	coordinatefieldcheckbox = XRCCTRL(*this, "CoordinateFieldCheckBox",wxCheckBox);
	coordinate_field_chooser_panel =
		XRCCTRL(*this, "CoordinateFieldChooserPanel",wxPanel);
	if (CMISS_GRAPHIC_STATIC != scene_editor->current_graphic_type)
	{
		if (scene_editor->current_graphic != NULL)
		{
			scene_editor->coordinate_field=
				Cmiss_graphic_get_coordinate_field(scene_editor->current_graphic);
		}
		struct Computed_field *temp_coordinate_field;
		if (scene_editor->coordinate_field)
		{
			temp_coordinate_field = scene_editor->coordinate_field;
			coordinatefieldcheckbox->SetValue(1);
			coordinate_field_chooser_panel->Enable();
		}
		else
		{
			temp_coordinate_field = scene_editor->default_coordinate_field;
			coordinatefieldcheckbox->SetValue(0);
			coordinate_field_chooser_panel->Disable();
		}
		if (coordinate_field_chooser ==NULL)
		{
			coordinate_field_chooser = 
				new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				(coordinate_field_chooser_panel, temp_coordinate_field, scene_editor->field_manager,
					(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL, (void *)NULL, scene_editor->user_interface);
			Callback_base< Computed_field* > *coordinate_field_callback = 
				new Callback_member_callback< Computed_field*, 
				wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
				(this, &wxSceneEditor::coordinate_field_callback);
			coordinate_field_chooser->set_callback(coordinate_field_callback);
			coordinate_field_chooser_panel->Fit();
		}
		coordinate_field_chooser->set_object(temp_coordinate_field);
		coordinatefieldcheckbox->Show();
		coordinate_field_chooser_panel->Show();
	}
	else
	{
		coordinatefieldcheckbox->Hide();
		coordinate_field_chooser_panel->Hide();
	}

		constantradiustextctrl=XRCCTRL(*this, "ConstantRadiusTextCtrl",wxTextCtrl);
		radiusscalarcheckbox=XRCCTRL(*this, "RadiusScalarCheckBox",wxCheckBox);
		scalefactorstextctrl=XRCCTRL(*this,"ScaleFactorsTextCtrl",wxTextCtrl);
		 radius_scalar_chooser_panel=XRCCTRL(*this, "RadiusScalarChooserPanel",wxPanel);
		 constantradius = XRCCTRL(*this,"ConstantRadiusText",wxStaticText);
		 scalefactor = XRCCTRL(*this,"ScaleFactorLabel", wxStaticText);
		 if ((CMISS_GRAPHIC_CYLINDERS==scene_editor->current_graphic_type)&& 
				 Cmiss_graphic_get_radius_parameters(graphic,
				 &constant_radius,&scale_factor,&radius_scalar_field))
		 {
				constantradiustextctrl->Show();
				radiusscalarcheckbox->Show();
				scalefactorstextctrl->Show();
				constantradius->Show();
				radius_scalar_chooser_panel->Show();
				scalefactor->Show();
				sprintf(temp_string,"%g",constant_radius);
				constantradiustextctrl->SetValue(temp_string);
				sprintf(temp_string,"%g",scale_factor);
				scalefactorstextctrl->SetValue(temp_string);
				
				if (radius_scalar_chooser == NULL)
				{
					 radius_scalar_chooser = 
							new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
							(radius_scalar_chooser_panel, scene_editor->radius_scalar_field, scene_editor->field_manager,
								 Computed_field_is_scalar, (void *)NULL, scene_editor->user_interface);
					 Callback_base< Computed_field* > *radius_scalar_callback = 
							new Callback_member_callback< Computed_field*, 
							wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
							(this, &wxSceneEditor::radius_scalar_callback);
					 radius_scalar_chooser->set_callback(radius_scalar_callback);
					 radius_scalar_chooser_panel->Fit();
				}
				if ((struct Computed_field *)NULL!=radius_scalar_field)
				{
					 radius_scalar_chooser->set_object(radius_scalar_field);
					 scalefactorstextctrl->Enable();
					 radiusscalarcheckbox->SetValue(1);
					 radius_scalar_chooser_panel->Enable();
					 scalefactor->Enable();
				}
				else
				{
					 scalefactorstextctrl->Disable();
					 radiusscalarcheckbox->SetValue(0);
					 radius_scalar_chooser_panel->Disable();
					 scalefactor->Disable();
				}
		 }
		 else
		 {
				constantradiustextctrl->Hide();
				radiusscalarcheckbox->Hide();
				scalefactorstextctrl->Hide();
				radius_scalar_chooser_panel->Hide();
				scalefactor->Hide();
				constantradius->Hide();
		 }

		 //iso-surface
		 iso_scalar_chooser_panel=XRCCTRL(*this, "IsoScalarChooserPanel",wxPanel);
		 isoscalartextctrl = XRCCTRL(*this, "IsoScalarTextCtrl",wxTextCtrl);
		 isoscalartext=XRCCTRL(*this,"IsoScalarText",wxStaticText);
		 isovalueoptionspane=XRCCTRL(*this,"IsoValueOptions",wxPanel);

		 isovaluelistradiobutton=XRCCTRL(*this,"IsoValueListRadioButton",wxRadioButton);
		 isovaluesequenceradiobutton=XRCCTRL(*this,"IsoValueSequenceRadioButton",wxRadioButton);
		 
		 isoscalartextctrl=XRCCTRL(*this,"IsoScalarTextCtrl",wxTextCtrl);
		 isovaluesequencenumbertextctrl=XRCCTRL(*this,"IsoValueSequenceNumberTextCtrl",wxTextCtrl);
		 isovaluesequencefirsttextctrl=XRCCTRL(*this,"IsoValueSequenceFirstTextCtrl",wxTextCtrl);
		 isovaluesequencelasttextctrl=XRCCTRL(*this,"IsoValueSequenceLastTextCtrl",wxTextCtrl);

		if ((CMISS_GRAPHIC_ISO_SURFACES==scene_editor->current_graphic_type)&&
				Cmiss_graphic_get_iso_surface_parameters(graphic,
					 &iso_scalar_field,&number_of_iso_values,&iso_values,
					 &first_iso_value,&last_iso_value,
					 &decimation_threshold)&&iso_scalar_field)
			{
				iso_scalar_chooser_panel->Show();
				isoscalartextctrl->Show();
				isoscalartext->Show();
				isovalueoptionspane->Show();
				if (iso_scalar_chooser == NULL)
				{
					 iso_scalar_chooser = 
							new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
							(iso_scalar_chooser_panel, iso_scalar_field, scene_editor->field_manager,
								 Computed_field_is_scalar, (void *)NULL, scene_editor->user_interface);
					 Callback_base< Computed_field* > *iso_scalar_callback = 
							new Callback_member_callback< Computed_field*, 
							wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
							(this, &wxSceneEditor::iso_scalar_callback);
					 iso_scalar_chooser->set_callback(iso_scalar_callback);
					 iso_scalar_chooser_panel->Fit();
				}
				if (iso_values)
				{
					iso_scalar_chooser->set_object(iso_scalar_field);
					isovaluelistradiobutton->SetValue(true);
					isoscalartextctrl->Enable();
					isovaluesequencenumbertextctrl->Disable();
					isovaluesequencefirsttextctrl->Disable();
					isovaluesequencelasttextctrl->Disable();
					vector_temp_string = (char *)NULL;
					error = 0;
					for (i = 0 ; !error && (i < number_of_iso_values) ; i++)
					{
						sprintf(temp_string,"%g ",iso_values[i]);
						append_string(&vector_temp_string, temp_string, &error);
					}
					if (vector_temp_string)
					{
						isoscalartextctrl ->SetValue(vector_temp_string);
						DEALLOCATE(vector_temp_string);
					}
					DEALLOCATE(iso_values);
				}
				else
				{
					isoscalartextctrl->Disable();
					isovaluesequencenumbertextctrl->Enable();
					isovaluesequencefirsttextctrl->Enable();
					isovaluesequencelasttextctrl->Enable();
					isovaluesequenceradiobutton->SetValue(true);
					sprintf(temp_string,"%d", number_of_iso_values);
					isovaluesequencenumbertextctrl->SetValue(temp_string);
					sprintf(temp_string,"%g", first_iso_value);
					isovaluesequencefirsttextctrl->SetValue(temp_string);
					sprintf(temp_string,"%g", last_iso_value);
					isovaluesequencelasttextctrl->SetValue(temp_string);
				}
			}
		else
			{			
				iso_scalar_chooser_panel->Hide();
				isoscalartextctrl->Hide();
				isoscalartext->Hide();
				isovalueoptionspane->Hide();
			}

		/* node_points, data_points, element_points */
		/* glyphs */
		glyph_chooser_panel=XRCCTRL(*this,"GlyphChooserPanel",wxPanel);
		orientation_scale_field_chooser_panel=XRCCTRL(*this,	"OrientationScaleChooserPanel",wxPanel);
		variable_scale_field_chooser_panel=XRCCTRL(*this,"VariableScaleChooserPanel",wxPanel);
		glyphtext=XRCCTRL(*this,"GlyphText",wxStaticText);
		centretext=XRCCTRL(*this,"CentreText",wxStaticText);
		centretextctrl=XRCCTRL(*this,"CentreTextCtrl",wxTextCtrl);
		baseglyphsizetext=XRCCTRL(*this,"BaseGlyphSizeText",wxStaticText);
		baseglyphsizetextctrl=XRCCTRL(*this,"BaseGlyphSizeTextCtrl",wxTextCtrl);
		orientationscalecheckbox=XRCCTRL(*this,"OrientationScaleCheckBox",wxCheckBox);
		glyphscalefactorstext	=XRCCTRL(*this,"GlyphScaleFactorsText",wxStaticText);
		glyphscalefactorstextctrl=XRCCTRL(*this,"GlyphScaleFactorsTextCtrl",wxTextCtrl);
		variablescalecheckbox=XRCCTRL(*this,"VariableScaleCheckBox",wxCheckBox);
		glyphbox=XRCCTRL(*this,"GlyphBox",wxWindow);
		glyphline=XRCCTRL(*this,"GlyphLine",wxWindow);

		wxCheckBox *overlay_checkbox = XRCCTRL(*this, "OverlayCheckBox", wxCheckBox);
		wxTextCtrl *overlay_textctrl = XRCCTRL(*this, "OverlayTextCtrl", wxTextCtrl);
		if (CMISS_GRAPHIC_STATIC == scene_editor->current_graphic_type)
		{
			overlay_checkbox->SetValue(Cmiss_graphic_is_overlay(graphic));
			overlay_checkbox->Show();
			if (Cmiss_graphic_is_overlay(graphic))
			{
				overlay_textctrl->Enable();
			}
			else
			{
				overlay_textctrl->Disable();		
			}
			sprintf(temp_string,"%d ",Cmiss_graphic_get_overlay_order(graphic));
			overlay_textctrl->SetValue(temp_string);
			overlay_textctrl->Show();
		}
		else
		{
			overlay_textctrl->Hide();
			overlay_textctrl->Disable();
			overlay_checkbox->Hide();	
		}
		
		if (((CMISS_GRAPHIC_NODE_POINTS == scene_editor->current_graphic_type) ||
				(CMISS_GRAPHIC_DATA_POINTS == scene_editor->current_graphic_type) ||
				(CMISS_GRAPHIC_ELEMENT_POINTS == scene_editor->current_graphic_type) ||
				(CMISS_GRAPHIC_STATIC == scene_editor->current_graphic_type)) &&
			 Cmiss_graphic_get_glyph_parameters(graphic,
				  &glyph, &glyph_scaling_mode,glyph_centre, glyph_size,
				  &orientation_scale_field, glyph_scale_factors,&variable_scale_field))
			{
				/* turn on callbacks */
				glyph_chooser_panel->Show();
				orientation_scale_field_chooser_panel->Show();
				variable_scale_field_chooser_panel->Show();
				glyphtext->Show();
				centretext->Show();
				centretextctrl->Show();
				baseglyphsizetext->Show();
				baseglyphsizetextctrl->Show();
				orientationscalecheckbox->Show();
				glyphscalefactorstext->Show();
				glyphscalefactorstextctrl->Show();
				variablescalecheckbox->Show();
				glyphbox->Show();
				glyphline->Show();
				if (CMISS_GRAPHIC_STATIC == scene_editor->current_graphic_type)
				{
					orientation_scale_field_chooser_panel->Hide();
					orientationscalecheckbox->Hide();
					glyphscalefactorstext->Hide();
					glyphscalefactorstextctrl->Hide();
					variable_scale_field_chooser_panel->Hide();
					variablescalecheckbox->Hide();
				}
				if (glyph_chooser == NULL)
				{
					 glyph_chooser = 
							new List_chooser<GT_object, LIST_CLASS(GT_object)>
							(glyph_chooser_panel, glyph, scene_editor->glyph_list,
								 (LIST_CONDITIONAL_FUNCTION(GT_object) *)NULL, (void *)NULL, 
								 scene_editor->user_interface);
					 Callback_base< GT_object* > *glyph_callback = 
							new Callback_member_callback< GT_object*, 
							wxSceneEditor, int (wxSceneEditor::*)(GT_object *) >
							(this, &wxSceneEditor::glyph_callback);
					 glyph_chooser->set_callback(glyph_callback);	
					 glyph_chooser_panel->Fit();
				}
			
				if (orientation_scale_field_chooser == NULL)
				{
					 orientation_scale_field_chooser=
							new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
							(orientation_scale_field_chooser_panel, orientation_scale_field, scene_editor->field_manager,
								 Computed_field_is_orientation_scale_capable, NULL,
								 scene_editor->user_interface);
					 Callback_base< Computed_field* > *orientation_scale_callback = 
							new Callback_member_callback< Computed_field*, 
							wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
							(this, &wxSceneEditor::orientation_scale_callback);
					 orientation_scale_field_chooser->set_callback(orientation_scale_callback);
					 orientation_scale_field_chooser_panel->Fit();
				}
				if (variable_scale_field_chooser  ==NULL)
				{
					 variable_scale_field_chooser=
							new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
							(variable_scale_field_chooser_panel, variable_scale_field, scene_editor->field_manager,
								 Computed_field_has_up_to_3_numerical_components, (void *)NULL, 
								 scene_editor->user_interface);
					 Callback_base< Computed_field* > *variable_scale_callback = 
							new Callback_member_callback< Computed_field*, 
							wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
							(this, &wxSceneEditor::variable_scale_callback);
					 variable_scale_field_chooser->set_callback(variable_scale_callback);
					 variable_scale_field_chooser_panel->Fit();
				}

				if (glyph != NULL)
				{
					 glyph_chooser ->set_object(glyph);
				}
					 
				sprintf(temp_string,"%g,%g,%g",
					glyph_centre[0],glyph_centre[1],glyph_centre[2]);
				centretextctrl->SetValue(temp_string);
				sprintf(temp_string,"%g*%g*%g",
					glyph_size[0],glyph_size[1],glyph_size[2]);
				baseglyphsizetextctrl->SetValue(temp_string);
				sprintf(temp_string,"%g*%g*%g",glyph_scale_factors[0],
					glyph_scale_factors[1],glyph_scale_factors[2]);
				glyphscalefactorstextctrl->SetValue(temp_string);
			
				if ((struct Computed_field *)NULL!=orientation_scale_field)
				{
				  orientation_scale_field_chooser->set_object(orientation_scale_field);
				  glyphscalefactorstextctrl->Enable();
				  orientationscalecheckbox->SetValue(1);
				  orientation_scale_field_chooser_panel->Enable();
				  glyphscalefactorstext->Enable();
				}
				else
				{
				  glyphscalefactorstextctrl->Disable();
				  orientationscalecheckbox->SetValue(0);
				  orientation_scale_field_chooser_panel->Disable();
				  glyphscalefactorstext->Disable();
				}
				if ((struct Computed_field *)NULL!=variable_scale_field)
				{
				  variable_scale_field_chooser->set_object(variable_scale_field);
				  variablescalecheckbox->SetValue(1);
				  variable_scale_field_chooser_panel->Enable();
				}
				else
				{
				  variablescalecheckbox->SetValue(0);
				  variable_scale_field_chooser_panel->Disable();
				}
			}
		else
			{
				glyph_chooser_panel->Hide();
				orientation_scale_field_chooser_panel->Hide();
				variable_scale_field_chooser_panel->Hide();
				glyphtext->Hide();
				centretext->Hide();
				centretextctrl->Hide();
				baseglyphsizetext->Hide();
				baseglyphsizetextctrl->Hide();
				orientationscalecheckbox->Hide();
				glyphscalefactorstext->Hide();
				glyphscalefactorstextctrl->Hide();
				variablescalecheckbox->Hide();
				glyphbox->Hide();
				glyphline->Hide();
			}

		/* label field */
		labelcheckbox=XRCCTRL(*this,"LabelCheckBox",wxCheckBox);
		label_chooser_panel = XRCCTRL(*this,"LabelChooserPanel",wxPanel);
		fonttext=XRCCTRL(*this,"FontText",wxStaticText);
		font_chooser_panel = XRCCTRL(*this,"FontChooserPanel",wxPanel);	

		if (((CMISS_GRAPHIC_NODE_POINTS==scene_editor->current_graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==scene_editor->current_graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==scene_editor->current_graphic_type)||
				(CMISS_GRAPHIC_STATIC==scene_editor->current_graphic_type)) &&
				Cmiss_graphic_get_label_field(graphic,&label_field, &font))
			{
				 if (label_field_chooser == NULL)
				 {
						label_field_chooser =
							 new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
							 (label_chooser_panel, label_field, scene_editor->field_manager,
									(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL , (void *)NULL, 
									scene_editor->user_interface);
						Callback_base< Computed_field* > *label_callback = 
							 new Callback_member_callback< Computed_field*, 
							 wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
							 (this, &wxSceneEditor::label_callback);
						label_field_chooser->set_callback(label_callback);
						label_chooser_panel->Fit();
				 }
				 if (font_chooser == NULL)
				 {
						font_chooser =
							 new Managed_object_chooser<Graphics_font,MANAGER_CLASS(Graphics_font)>
							 (font_chooser_panel, font, scene_editor->font_manager,
									(MANAGER_CONDITIONAL_FUNCTION(Graphics_font) *)NULL , (void *)NULL, 
									scene_editor->user_interface);
						Callback_base< Graphics_font* > *font_callback = 
							 new Callback_member_callback< Graphics_font*, 
							 wxSceneEditor, int (wxSceneEditor::*)(Graphics_font *) >
							 (this, &wxSceneEditor::font_callback);
						font_chooser->set_callback(font_callback);
						font_chooser_panel->Fit();
				 }
				 if (CMISS_GRAPHIC_STATIC!=scene_editor->current_graphic_type)
				 {
					 labelcheckbox->Show();
					 label_chooser_panel->Show();
				 }
				 else
				 {
					 labelcheckbox->Hide();
					 label_chooser_panel->Hide();
				 }
				 fonttext->Show();
				 font_chooser_panel->Show();
				 if ((struct Computed_field *)NULL!=label_field)
				 {
						label_field_chooser->set_object(label_field);
						labelcheckbox->SetValue(1);
						label_chooser_panel->Enable();
				 }
				 else
				 {
						labelcheckbox->SetValue(0);
						label_chooser_panel->Disable();
				 }
				 if ((struct Graphics_font *)NULL!=font)
				 {
						font_chooser->set_object(font);
						font_chooser_panel->Enable();
				 }
				 else
				 {
						font_chooser_panel->Disable();
				 }

			}
		else
		{
			 labelcheckbox->Hide();
			 label_chooser_panel->Hide();
			 font_chooser_panel->Hide();
			 fonttext->Hide();
		}
		
		/* Visibility field */
		visibility_field_checkbox=XRCCTRL(*this,"VisibilityFieldCheckBox",wxCheckBox);
		visibility_field_chooser_panel = XRCCTRL(*this,"VisibilityFieldChooserPanel",wxPanel);

		if (((CMISS_GRAPHIC_NODE_POINTS==scene_editor->current_graphic_type)||
				(CMISS_GRAPHIC_DATA_POINTS==scene_editor->current_graphic_type)) &&
			Cmiss_graphic_get_visibility_field(graphic,&visibility_field))
		{
			if (visibility_field_chooser == NULL)
			{
				visibility_field_chooser =
					new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
					(visibility_field_chooser_panel, visibility_field, scene_editor->field_manager,
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL , (void *)NULL, 
						scene_editor->user_interface);
				Callback_base< Computed_field* > *visibility_field_callback = 
					new Callback_member_callback< Computed_field*, 
					wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
					(this, &wxSceneEditor::visibility_field_callback);
				visibility_field_chooser->set_callback(visibility_field_callback);
				visibility_field_chooser_panel->Fit();
			}
			visibility_field_checkbox->Show();
			visibility_field_chooser_panel->Show();
			if ((struct Computed_field *)NULL!=visibility_field)
			{
				visibility_field_chooser->set_object(visibility_field);
				visibility_field_checkbox->SetValue(1);
				visibility_field_chooser_panel->Enable();
			}
			else
			{
				visibility_field_checkbox->SetValue(0);
				visibility_field_chooser_panel->Disable();
			}
		}
		else
		{
			visibility_field_checkbox->Hide();
			visibility_field_chooser_panel->Hide();
		}
		
		/* Set the select_mode_chooser_panel*/
		select_mode_chooser_panel = 
			XRCCTRL(*this, "SelectModeChooserPanel", wxPanel);
		selectmodetext=XRCCTRL(*this,"SelectModeText",wxStaticText);
		if (CMISS_GRAPHIC_STATIC!=scene_editor->current_graphic_type)
		{
			if (select_mode_chooser == NULL)
			{
				select_mode_chooser = 
					new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Graphics_select_mode)>
					(select_mode_chooser_panel, 
						scene_editor->select_mode,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
						(void *)NULL, scene_editor->user_interface);
				select_mode_chooser_panel->Fit();
				Callback_base< enum Graphics_select_mode > *select_mode_callback = 
					new Callback_member_callback< enum Graphics_select_mode, 
					wxSceneEditor, int (wxSceneEditor::*)(enum Graphics_select_mode) >
					(this, &wxSceneEditor::select_mode_callback);
				select_mode_chooser->set_callback(select_mode_callback);
			}
			select_mode_chooser_panel->Show();
			selectmodetext->Show();
			select_mode_chooser->set_value(Cmiss_graphic_get_select_mode(graphic));
		}
		else
		{
			select_mode_chooser_panel->Hide();
			selectmodetext->Hide();
		}

		/* element_points and iso_surfaces */
		/*use_element_type*/
			use_element_type_chooser_panel = 
				XRCCTRL(*this, "UseElementTypeChooserPanel", wxPanel);
			useelementtypetext=XRCCTRL(*this,"UseElementTypeText",wxStaticText);
			if ((CMISS_GRAPHIC_ELEMENT_POINTS==scene_editor->current_graphic_type)||
			     (CMISS_GRAPHIC_ISO_SURFACES==scene_editor->current_graphic_type)) 
			{
				 if (use_element_type_chooser == NULL)
				 {
						use_element_type_chooser = 
							 new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Use_element_type)>
							 (use_element_type_chooser_panel, 
									scene_editor->use_element_type,
									(ENUMERATOR_CONDITIONAL_FUNCTION(Use_element_type) *)NULL,
									(void *)NULL, scene_editor->user_interface);
						use_element_type_chooser_panel->Fit();
						Callback_base< enum Use_element_type > *use_element_type_callback = 
							 new Callback_member_callback< enum Use_element_type, 
							 wxSceneEditor, int (wxSceneEditor::*)(enum Use_element_type) >
							 (this, &wxSceneEditor::use_element_type_callback);
						use_element_type_chooser->set_callback(use_element_type_callback);
				 }
				 use_element_type_chooser_panel->Show();
				 useelementtypetext->Show();
				 use_element_type_chooser->set_value(Cmiss_graphic_get_use_element_type(graphic));
			}
			else
			{
				 use_element_type_chooser_panel->Hide();
				 useelementtypetext->Hide();	 
			}

		discretizationtext=XRCCTRL(*this,"DiscretizationText",wxStaticText);
		discretizationtextctrl=XRCCTRL(*this,"DiscretizationTextCtrl",wxTextCtrl);
		circlediscretizationtext=XRCCTRL(*this,"CircleDiscretizationText",wxStaticText);
		circlediscretizationtextctrl=XRCCTRL(*this,"CircleDiscretizationTextCtrl",wxTextCtrl);
		nativediscretizationfieldcheckbox=XRCCTRL(*this,"NativeDiscretizationFieldCheckBox",wxCheckBox);
		native_discretization_field_chooser_panel=XRCCTRL(*this, "NativeDiscretizationFieldChooserPanel",wxPanel);

		if ((CMISS_GRAPHIC_DATA_POINTS!=scene_editor->current_graphic_type) &&
			(CMISS_GRAPHIC_NODE_POINTS!=scene_editor->current_graphic_type) &&
			(CMISS_GRAPHIC_STREAMLINES!=scene_editor->current_graphic_type) &&
			(CMISS_GRAPHIC_STATIC!=scene_editor->current_graphic_type))
		{
			Cmiss_graphic_get_discretization(graphic,
				&discretization);
			sprintf(temp_string,"%d*%d*%d",discretization.number_in_xi1,
				discretization.number_in_xi2,discretization.number_in_xi3);
			discretizationtextctrl->SetValue(temp_string);
			discretizationtextctrl->Show();	
			discretizationtext->Show();
		}
		else
		{
			discretizationtextctrl->Hide();	
			discretizationtext->Hide();
		}

		if (CMISS_GRAPHIC_CYLINDERS==scene_editor->current_graphic_type)
		{
			sprintf(temp_string,"%d",Cmiss_graphic_get_circle_discretization(graphic));
			circlediscretizationtextctrl->SetValue(temp_string);
			circlediscretizationtextctrl->Show();	
			circlediscretizationtext->Show();
		}
		else
		{
			circlediscretizationtextctrl->Hide();	
			circlediscretizationtext->Hide();
		}

		/* element_points */
		xidiscretizationmodetext = XRCCTRL(*this,"XiDiscretizationModeText",wxStaticText);
		xi_discretization_mode_chooser_panel= XRCCTRL(*this,"XiDiscretizationModeChooserPanel",wxPanel);
		densityfieldtext=XRCCTRL(*this, "DensityFieldText",wxStaticText);
		density_field_chooser_panel=XRCCTRL(*this,"DensityFieldChooserPanel",wxPanel);

		if ((CMISS_GRAPHIC_ELEMENT_POINTS==scene_editor->current_graphic_type)&&
				Cmiss_graphic_get_xi_discretization(graphic,&xi_discretization_mode, &xi_point_density_field))	
		{
			native_discretization_field=
				Cmiss_graphic_get_native_discretization_field(graphic);
			if (native_discretization_field_chooser==NULL)
			{
				native_discretization_field_chooser =
					new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
					(native_discretization_field_chooser_panel,(Computed_field*)NULL, 
						scene_editor->field_manager,
						Computed_field_is_type_finite_element_iterator, (void *)NULL, scene_editor->user_interface);
				Callback_base< Computed_field* > *native_discretization_callback = 
					new Callback_member_callback< Computed_field*, 
					wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
					(this, &wxSceneEditor::native_discretization_callback);
				native_discretization_field_chooser->set_callback(native_discretization_callback);
				native_discretization_field_chooser_panel->Fit();
			}
			nativediscretizationfieldcheckbox->Show();
			native_discretization_field_chooser_panel->Show();
			if (native_discretization_field != NULL)
			{
				native_discretization_field_chooser->set_object(
					FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
						Computed_field_is_read_only_with_fe_field, native_discretization_field,
						scene_editor->field_manager));
				nativediscretizationfieldcheckbox->SetValue(1);
				native_discretization_field_chooser_panel->Enable();
			}
			else
			{
				nativediscretizationfieldcheckbox->SetValue(0);
				native_discretization_field_chooser_panel->Disable();
			}

			if (xi_discretization_mode_chooser ==NULL)
			{
				xi_discretization_mode_chooser = 
					new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Xi_discretization_mode)>
					(xi_discretization_mode_chooser_panel, xi_discretization_mode,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Xi_discretization_mode) *)NULL,
						(void *)NULL, scene_editor->user_interface);
				xi_discretization_mode_chooser_panel->Fit();
				Callback_base< enum Xi_discretization_mode > *xi_discretization_mode_callback = 
					new Callback_member_callback< enum Xi_discretization_mode, 
					wxSceneEditor, int (wxSceneEditor::*)(enum Xi_discretization_mode) >
					(this, &wxSceneEditor::xi_discretization_mode_callback);
				xi_discretization_mode_chooser->set_callback(xi_discretization_mode_callback);
			}
			xi_discretization_mode_chooser_panel->Show();
			xidiscretizationmodetext->Show();
			xi_discretization_mode_chooser->set_value(xi_discretization_mode);
			
			if (xi_point_density_field_chooser == NULL)
			{
				xi_point_density_field_chooser =
					new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
					(density_field_chooser_panel, xi_point_density_field, scene_editor->field_manager,
						Computed_field_is_scalar, (void *)NULL, 
						scene_editor->user_interface);
				Callback_base< Computed_field* > *xi_point_density_callback = 
					new Callback_member_callback< Computed_field*, 
					wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
					(this, &wxSceneEditor::xi_point_density_callback);
				xi_point_density_field_chooser->set_callback(xi_point_density_callback);
				density_field_chooser_panel->Fit();
			}
			densityfieldtext->Show();
			density_field_chooser_panel->Show();
			xi_point_density_field_chooser->set_object(xi_point_density_field);
			
			if (XI_DISCRETIZATION_EXACT_XI != xi_discretization_mode)
			{
				discretizationtextctrl->Enable();	
				discretizationtext->Enable();
				nativediscretizationfieldcheckbox->Enable();
				if ((struct FE_field *)NULL != native_discretization_field)
				{
					native_discretization_field_chooser_panel->Enable();
				}
				else
				{
					native_discretization_field_chooser_panel->Disable();
				}
			}			
			else
			{	
				discretizationtextctrl->Disable();	
				discretizationtext->Disable();
				nativediscretizationfieldcheckbox->Disable();
				native_discretization_field_chooser_panel->Disable();
			}
			if ((XI_DISCRETIZATION_CELL_DENSITY == xi_discretization_mode)||
				(XI_DISCRETIZATION_CELL_POISSON == xi_discretization_mode))
			{
				densityfieldtext->Enable();
				density_field_chooser_panel->Enable();
			}
			else
			{
				densityfieldtext->Disable();
				density_field_chooser_panel->Disable();
			}
		}
		else
		{	
			nativediscretizationfieldcheckbox->Hide();
			native_discretization_field_chooser_panel->Hide();
			xidiscretizationmodetext->Hide();
			xi_discretization_mode_chooser_panel->Hide();
			densityfieldtext->Hide();	
			density_field_chooser_panel->Hide();
		}

		/* seed element */
		seed_element_panel = XRCCTRL(*this, "SeedElementPanel", wxPanel);
		seedelementcheckbox = XRCCTRL(*this, "SeedElementCheckBox", wxCheckBox);

		if (CMISS_GRAPHIC_STREAMLINES==scene_editor->current_graphic_type)
		{
			seed_element_panel->Show();
			seedelementcheckbox->Show();
			fe_region = Cmiss_region_get_FE_region(scene_editor->root_region);
			seed_element =
				 Cmiss_graphic_get_seed_element(graphic);
			if (seed_element_chooser == NULL)
			{
				 seed_element_chooser = new wxFeElementTextChooser(seed_element_panel,
						seed_element, fe_region, FE_element_is_top_level,(void *)NULL); 
				 Callback_base<FE_element *> *seed_element_callback = 
						new Callback_member_callback< FE_element*, 
						wxSceneEditor, int (wxSceneEditor::*)(FE_element *) >
						(this, &wxSceneEditor::seed_element_callback);
				 seed_element_chooser->set_callback(seed_element_callback);
				 seed_element_panel->Fit();
			}
			if (NULL != seed_element)
			{
				 seedelementcheckbox->SetValue(1);
				 seed_element_panel->Enable();
			}
			else
			{
				 seedelementcheckbox->SetValue(0);
				 seed_element_panel->Disable();
			}
			seed_element_chooser->set_object(seed_element);
		}
		else
		{
			seed_element_panel->Hide();
			seedelementcheckbox->Hide();
		}

		/* seed xi */
		xitext=XRCCTRL(*this,"XiText",wxStaticText);
		xitextctrl=XRCCTRL(*this,"XiTextCtrl",wxTextCtrl);
		if ((CMISS_GRAPHIC_ELEMENT_POINTS==scene_editor->current_graphic_type)||
			(CMISS_GRAPHIC_STREAMLINES==scene_editor->current_graphic_type))
		{
			xitext->Show();
			xitextctrl->Show();
			Cmiss_graphic_get_seed_xi(scene_editor->current_graphic,seed_xi);
			sprintf(temp_string,"%g,%g,%g",seed_xi[0],seed_xi[1],seed_xi[2]);
			xitextctrl->SetValue(temp_string);
			if (CMISS_GRAPHIC_ELEMENT_POINTS==scene_editor->current_graphic_type)
			{
				if (XI_DISCRETIZATION_EXACT_XI == xi_discretization_mode)
				{
					xitext->Enable();
					xitextctrl->Enable();
				}
				else
				{
					xitext->Disable();
					xitextctrl->Disable();
				}
			}
			else
			{
				xitext->Enable();
				xitextctrl->Enable();
			}
		}
		else
			{
				xitext->Hide();
				xitextctrl->Hide();
			}

		/*StreamLine */
		streamtypetext=XRCCTRL(*this, "StreamTypeText",wxStaticText);
		streamline_type_chooser_panel=XRCCTRL(*this, "StreamlineTypeChooserPanel",wxPanel);
		streamlengthtext=XRCCTRL(*this, "StreamLengthText",wxStaticText);
		lengthtextctrl=XRCCTRL(*this,"LengthTextCtrl",wxTextCtrl);
		streamwidthtext=XRCCTRL(*this, "StreamWidthText",wxStaticText);
		widthtextctrl=XRCCTRL(*this,"WidthTextCtrl",wxTextCtrl);
		streamvectortext=XRCCTRL(*this, "StreamVectorText",wxStaticText);
		stream_vector_chooser_panel=XRCCTRL(*this, "StreamVectorChooserPanel",wxPanel);
		reversecheckbox=XRCCTRL(*this,"ReverseCheckBox",wxCheckBox);

		if (CMISS_GRAPHIC_STREAMLINES==scene_editor->current_graphic_type)
			{
				streamtypetext->Show();
				streamline_type_chooser_panel->Show();
				streamlengthtext->Show();
				lengthtextctrl->Show();
				streamwidthtext->Show();
				widthtextctrl->Show();
				streamvectortext->Show();
				stream_vector_chooser_panel->Show();
				reversecheckbox->Show();
				Cmiss_graphic_get_streamline_parameters(graphic,
					&streamline_type,&stream_vector_field,&reverse_track,
					&streamline_length,&streamline_width);
				if (streamline_type_chooser == NULL)
				{
					 streamline_type_chooser = 
							new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Streamline_type)>
							(streamline_type_chooser_panel, streamline_type,
								 (ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_type) *)NULL,
								 (void *)NULL, scene_editor->user_interface);
					 streamline_type_chooser_panel->Fit();
					 Callback_base< enum Streamline_type > *streamline_type_callback = 
							new Callback_member_callback< enum Streamline_type, 
							wxSceneEditor, int (wxSceneEditor::*)(enum Streamline_type) >
							(this, &wxSceneEditor::streamline_type_callback);
					 streamline_type_chooser->set_callback(streamline_type_callback);
				}
				streamline_type_chooser->set_value(streamline_type);
				sprintf(temp_string,"%g",streamline_length);
				lengthtextctrl->SetValue(temp_string);
				sprintf(temp_string,"%g",streamline_width);
				widthtextctrl->SetValue(temp_string);
				if (stream_vector_chooser == NULL)
				{
					 stream_vector_chooser =
							new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
							(stream_vector_chooser_panel, stream_vector_field, scene_editor->field_manager,
								 Computed_field_is_stream_vector_capable, (void *)NULL, 
								 scene_editor->user_interface);
					 Callback_base< Computed_field* > *stream_vector_callback = 
							new Callback_member_callback< Computed_field*, 
							wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
							(this, &wxSceneEditor::stream_vector_callback);
					 stream_vector_chooser->set_callback(stream_vector_callback);
					 stream_vector_chooser_panel->Fit();
				}
				stream_vector_chooser ->set_object(stream_vector_field);
				reversecheckbox->SetValue(reverse_track);
			}
			else
			{
				streamtypetext->Hide();
				streamline_type_chooser_panel->Hide();
				streamlengthtext->Hide();
				lengthtextctrl->Hide();
				streamwidthtext->Hide();
				widthtextctrl->Hide();
				streamvectortext->Hide();
				stream_vector_chooser_panel->Hide();
				reversecheckbox->Hide();
           }
		/* line_width */
		linewidthtext=XRCCTRL(*this,"LineWidthText",wxStaticText);
		linewidthtextctrl=XRCCTRL(*this,"LineWidthTextCtrl",wxTextCtrl);
		
		if (CMISS_GRAPHIC_LINES==scene_editor->current_graphic_type)
		{
			linewidthtext->Show();
			linewidthtextctrl->Show();
			line_width = Cmiss_graphic_get_line_width(graphic);
			sprintf(temp_string,"%d",line_width);
			linewidthtextctrl->SetValue(temp_string);
			linewidthtextctrl->Enable();
			if (CMISS_GRAPHIC_ISO_SURFACES==scene_editor->current_graphic_type &&
				USE_FACES != Cmiss_graphic_get_use_element_type(graphic))
			{
				linewidthtextctrl->Disable();
			}
		}
		else
		{
			linewidthtext->Hide();
			linewidthtextctrl->Hide();
		}
		
		streamlinedatatypetext=XRCCTRL(*this, "StreamlineDataTypeText", wxStaticText);
		streamline_data_type_chooser_panel = XRCCTRL(*this,"StreamlineDataTypeChooserPanel",wxPanel);
		spectrumtext=XRCCTRL(*this, "SpectrumText", wxStaticText);
		spectrum_chooser_panel=XRCCTRL(*this,"SpectrumChooserPanel", wxPanel);
		datacheckbox=XRCCTRL(*this, "DataCheckBox", wxCheckBox);
		data_chooser_panel=XRCCTRL(*this,"DataChooserPanel",wxPanel);

		if (CMISS_GRAPHIC_STATIC!=scene_editor->current_graphic_type)
		{
			if (data_field_chooser == NULL)
			{
				Spectrum *temp_spectrum = (Spectrum *)NULL;
				Computed_field *temp_data_field = (Computed_field *)NULL;
				if (scene_editor->current_graphic != NULL)
				{
					Cmiss_graphic_get_data_spectrum_parameters(scene_editor->current_graphic,
						&temp_data_field,&temp_spectrum);
				}
				data_field_chooser = 
					new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
					(data_chooser_panel,temp_data_field, scene_editor->field_manager,
						Computed_field_has_numerical_components, (void *)NULL, scene_editor->user_interface);
				Callback_base< Computed_field* > *data_field_callback = 
					new Callback_member_callback< Computed_field*, 
					wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
					(this, &wxSceneEditor::data_field_callback);
				data_field_chooser->set_callback(data_field_callback);
				data_chooser_panel->Fit();
			}
			datacheckbox->Show();
			data_chooser_panel->Show();
		}
		else
		{
			datacheckbox->Hide();
			data_chooser_panel->Hide();
		}
		

		if (CMISS_GRAPHIC_STREAMLINES==scene_editor->current_graphic_type)
		{
			streamlinedatatypetext->Show();
			streamline_data_type_chooser_panel->Show();
			Cmiss_graphic_get_data_spectrum_parameters_streamlines(
				graphic,&streamline_data_type,&data_field,&spectrum);
			if (streamline_data_type_chooser == NULL)
			{
				streamline_data_type_chooser=
					new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Streamline_data_type)>
					(streamline_data_type_chooser_panel, streamline_data_type,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_data_type) *)NULL,
						(void *)NULL, scene_editor->user_interface);
				Callback_base< enum Streamline_data_type > *streamline_data_type_callback = 
					new Callback_member_callback< enum Streamline_data_type, 
					wxSceneEditor, int (wxSceneEditor::*)(enum Streamline_data_type) >
					(this, &wxSceneEditor::streamline_data_type_callback);
				streamline_data_type_chooser->set_callback(streamline_data_type_callback);
				streamline_data_type_chooser_panel->Fit();
			}
			if ((struct Computed_field *)NULL != data_field)
			{
				data_chooser_panel->Enable();
				spectrumtext->Enable();
				spectrum_chooser_panel->Enable();
			}
			else
			{
				data_chooser_panel->Disable();
				spectrumtext->Disable();
				spectrum_chooser_panel->Disable();
			}
			datacheckbox->Disable();
			streamline_data_type_chooser->set_value(streamline_data_type);
		}
		else
		{
			/* set scalar data field & spectrum */
			streamlinedatatypetext->Hide();
			streamline_data_type_chooser_panel->Hide();
			if (CMISS_GRAPHIC_STATIC!=scene_editor->current_graphic_type)
			{
				Cmiss_graphic_get_data_spectrum_parameters(graphic,
					&data_field,&spectrum);
				if	((struct Computed_field *)NULL != data_field)
				{
					datacheckbox->SetValue(1);
					data_field_chooser->set_object(data_field);
					spectrum_chooser->set_object(spectrum);
					data_chooser_panel->Enable();
					spectrumtext->Enable();
					spectrum_chooser_panel->Enable();
				}
				else
				{
					datacheckbox->SetValue(0);
					data_chooser_panel->Disable();
					spectrumtext->Disable();
					spectrum_chooser_panel->Disable();
				}
				datacheckbox->Enable();
			}
		}
		
		texturecoordinatescheckbox=XRCCTRL(*this, "TextureCoordinatesCheckBox", wxCheckBox);
		texture_coordinates_chooser_panel =XRCCTRL(*this, "TextrueCoordinatesChooserPanel", wxPanel);
		if ((CMISS_GRAPHIC_CYLINDERS == scene_editor->current_graphic_type) ||
			(CMISS_GRAPHIC_SURFACES == scene_editor->current_graphic_type) ||
			(CMISS_GRAPHIC_ISO_SURFACES == scene_editor->current_graphic_type))
		{
			/* set texture_coordinate field */
			texture_coordinates_chooser_panel->Show();
			texturecoordinatescheckbox->Show();
			texture_coord_field = Cmiss_graphic_get_texture_coordinate_field(graphic);
			if (texture_coord_field_chooser == NULL)
			{
				 texture_coord_field_chooser =
						new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				    (texture_coordinates_chooser_panel,texture_coord_field, scene_editor->field_manager,
							 Computed_field_has_up_to_3_numerical_components, (void *)NULL, 
							 scene_editor->user_interface);
				 Callback_base< Computed_field* > *texture_coord_field_callback = 
						new Callback_member_callback< Computed_field*, 
					wxSceneEditor, int (wxSceneEditor::*)(Computed_field *) >
						(this, &wxSceneEditor::texture_coord_field_callback);
				 texture_coord_field_chooser->set_callback(texture_coord_field_callback);
				 texture_coordinates_chooser_panel->Fit();
			}
			if ((struct Computed_field *)NULL != texture_coord_field)
			{
				texturecoordinatescheckbox->SetValue(1);
				texture_coord_field_chooser->set_object(texture_coord_field);
			 	texture_coordinates_chooser_panel->Enable();
			 }
			else
			{
				texturecoordinatescheckbox->SetValue(0);
			 	texture_coordinates_chooser_panel->Disable();
			}
		}
		else
		{
			texture_coordinates_chooser_panel->Hide();
			texturecoordinatescheckbox->Hide();
		}

		/* render_type */		
		rendertypetext=XRCCTRL(*this, "RenderTypeText",wxStaticText);
		render_type_chooser_panel=XRCCTRL(*this,"RenderTypeChooserPanel",wxPanel);
		if ((CMISS_GRAPHIC_CYLINDERS == scene_editor->current_graphic_type) ||
			(CMISS_GRAPHIC_SURFACES==scene_editor->current_graphic_type) ||
			(CMISS_GRAPHIC_ISO_SURFACES==scene_editor->current_graphic_type))
		{
			/* set texture_coordinate field */
			rendertypetext->Show();
			render_type_chooser_panel->Show();
			render_type=Cmiss_graphic_get_render_type(graphic);
			if (render_type_chooser == NULL)
			{
				 render_type_chooser = 
						new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Render_type)>
						(render_type_chooser_panel, render_type,
							 (ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL,
							 (void *)NULL, scene_editor->user_interface);
				 render_type_chooser_panel->Fit();
				 Callback_base< enum Render_type > *render_type_callback = 
						new Callback_member_callback< enum Render_type, 
						wxSceneEditor, int (wxSceneEditor::*)(enum Render_type) >
						(this, &wxSceneEditor::render_type_callback);
				 render_type_chooser->set_callback(render_type_callback);
			}
			render_type_chooser->set_value(render_type);
		}
		else
		{
			rendertypetext->Hide();
			render_type_chooser_panel->Hide();
		}
		
		exteriorcheckbox=XRCCTRL(*this,"ExteriorCheckBox",wxCheckBox);
		facecheckbox=XRCCTRL(*this, "FaceCheckBox",wxCheckBox);
		facechoice=XRCCTRL(*this, "FaceChoice",wxChoice);
		if ((CMISS_GRAPHIC_DATA_POINTS!=scene_editor->current_graphic_type) &&
			(CMISS_GRAPHIC_NODE_POINTS!=scene_editor->current_graphic_type) &&
			(CMISS_GRAPHIC_STREAMLINES!=scene_editor->current_graphic_type) &&
			(CMISS_GRAPHIC_STATIC!=scene_editor->current_graphic_type))
		{
			exteriorcheckbox->Show();
			facecheckbox->Show();
			facechoice->Show();

 			if ((CMISS_GRAPHIC_ISO_SURFACES!=scene_editor->current_graphic_type) &&
   				(CMISS_GRAPHIC_ELEMENT_POINTS!=scene_editor->current_graphic_type))
			{
					exteriorcheckbox->Enable();
					facecheckbox->Enable();				
			}
			else
			{
				if  (USE_ELEMENTS != Cmiss_graphic_get_use_element_type(graphic))
				{
					exteriorcheckbox->Enable();
					facecheckbox->Enable();
				}
				else
				{
					exteriorcheckbox->Disable();
					facecheckbox->Disable();
					facechoice->Disable();
				}
			}
			if (Cmiss_graphic_get_exterior(graphic))
			{
				exteriorcheckbox->SetValue(1);
			}
			else
			{
				exteriorcheckbox->SetValue(0);
			}		
			if (Cmiss_graphic_get_face(graphic,&face))
			{
				facecheckbox->SetValue(1);
				facechoice->Enable();
				facechoice->SetSelection(face);
			}
			else
			{
				facecheckbox->SetValue(0);
				facechoice->Disable();
			}
		}
		else
		{
			exteriorcheckbox->Hide();
			facecheckbox->Hide();
			facechoice->Hide();
		}
	} /*SetGraphic*/

const char *GetRegionPathFromTreeItemId(wxTreeItemId current_item_id)
{
	wxTreeItemId parent_id, root_item_id;
	std::string path_name, current_region_name;

	path_name = scene_editor->testing_tree_ctrl->
		GetItemText(current_item_id);
	root_item_id = 
		scene_editor->testing_tree_ctrl->GetRootItem();
	if (current_item_id != root_item_id)
	{
		parent_id = 
			scene_editor->testing_tree_ctrl->GetItemParent(current_item_id);
		while (parent_id != root_item_id)
		{
			current_region_name = scene_editor->testing_tree_ctrl->
				GetItemText(parent_id);
			path_name = "/" + path_name;
			path_name = current_region_name + path_name;
			parent_id = 
				scene_editor->testing_tree_ctrl->GetItemParent(parent_id);
		}
		path_name = scene_editor->testing_tree_ctrl->
			GetItemText(root_item_id) + path_name;
	}
	return duplicate_string(path_name.c_str());
}

void TreeControlSelectionChanged(wxTreeEvent &event)
{
	ENTER(TreeControlSelectionChanged);

	struct Cmiss_region *region;
	struct Cmiss_rendition *rendition;
	int width, height;
	wxTreeItemId new_id= event.GetItem();
	wxCmguiHierachicalTreeItemData* data = NULL;
	wxArrayTreeItemIds array;

	if (new_id.IsOk())
	{ 
		data = dynamic_cast<wxCmguiHierachicalTreeItemData*>(
			scene_editor->testing_tree_ctrl->GetItemData(new_id));
	}
	if (scene_editor->testing_tree_ctrl->GetSelections(array) 
		&& data && (region = data->GetRegion()))
	{
		rendition = Cmiss_region_get_rendition(region);
		if (rendition)
		{
			Scene_editor_set_active_rendition(scene_editor, rendition);
			scene_editor->collpane->Enable();
			scene_editor->collpane->Show();
			scene_editor->lowersplitter->Enable();
			scene_editor->lowersplitter->Show();
			Scene_editor_set_graphic_widgets_for_rendition(scene_editor);
			if (scene_editor->sceneediting)
			{
				if (!Cmiss_rendition_get_number_of_graphic(rendition))
				{
					scene_editor->sceneediting->Hide();
				}
				else
				{
					scene_editor->sceneediting->Show();
				}
			}
			DEACCESS(Cmiss_rendition)(&rendition);
		}
	}
	else
	{
		Scene_editor_set_active_rendition(
				scene_editor, NULL);
		if (!scene_editor->graphiclistbox)
			scene_editor->graphiclistbox = XRCCTRL(
				*this, "CmissGraphicListBox",wxCheckListBox);
		scene_editor->graphiclistbox->SetSelection(wxNOT_FOUND);
		scene_editor->graphiclistbox->Clear();
		scene_editor->collpane->Disable();
		scene_editor->collpane->Hide();
		scene_editor->lowersplitter->Disable();
		scene_editor->lowersplitter->Hide();
		scene_editor->sceneediting->Hide();
		Scene_editor_set_graphic_widgets_for_rendition(scene_editor);
	}

	if (scene_editor->lowersplitter)
	{
		scene_editor->lowersplitter->GetSize(&width, &height);
		scene_editor->lowersplitter->SetSize(width-1, height-1);
		scene_editor->lowersplitter->SetSize(width+1, height+1);
	}
	lowest_panel = XRCCTRL(*this, "LowestPanel",wxScrolledWindow);
	if (lowest_panel)
	{
		lowest_panel->GetSize(&width, &height);
		lowest_panel->SetSize(width-1, height-1);
		lowest_panel->SetSize(width+1, height+1);
	}

	LEAVE;
}

void SetTreeItemImage(wxTreeItemId current_item_id, int id)
{
	scene_editor->testing_tree_ctrl->SetItemImage(
		current_item_id,id,
		wxTreeItemIcon_Normal);
	scene_editor->testing_tree_ctrl->SetItemImage(
		current_item_id,id,
		wxTreeItemIcon_Selected);
	scene_editor->testing_tree_ctrl->SetItemImage(
		current_item_id,id,
		wxTreeItemIcon_Expanded);
	scene_editor->testing_tree_ctrl->SetItemImage(
		current_item_id,id,
		wxTreeItemIcon_SelectedExpanded);
}

void ShowMenu(wxTreeItemId id, const wxPoint& pt)
{
	const char *full_path = GetRegionPathFromTreeItemId(
		id);
	wxMenu menu(full_path);
	wxMenu *add_menu= new wxMenu("Graphic");
	add_menu->Append(AddMenuItemNode, wxT("node_points"));
	add_menu->Append(AddMenuItemData, wxT("data_points"));
	add_menu->Append(AddMenuItemLines, wxT("lines"));
	add_menu->Append(AddMenuItemCylinders, wxT("cylinders"));
	add_menu->Append(AddMenuItemSurfaces, wxT("surfaces"));
	add_menu->Append(AddMenuItemIsoSurfaces, wxT("iso_surfaces"));
	add_menu->Append(AddMenuItemElement, wxT("element_points"));
	add_menu->Append(AddMenuItemStreamlines, wxT("streamlines"));
	add_menu->Append(AddMenuItemStaticGraphic, wxT("static_graphic"));
	menu.Append(CmguiTree_MenuItem1, wxT("Turn on visibililty"));
	menu.Append(CmguiTree_MenuItem2, wxT("Turn off visibililty"));
	menu.AppendSubMenu(add_menu, wxT("Add"), wxT("Graphic Type"));
	PopupMenu(&menu, pt);
	DEALLOCATE(full_path);
}

void OnItemMenu(wxTreeEvent &event)
{
	wxTreeItemId itemId = event.GetItem();
	wxPoint clientpt = event.GetPoint();
	wxPoint screenpt = ClientToScreen(clientpt);
	
	ShowMenu(itemId, clientpt);
	event.Skip();
}

void SetVisibilityOfTreeId(wxTreeItemId current_item_id, int flag)
{
	struct Cmiss_region *region;
	struct Cmiss_rendition *rendition;

	wxCmguiHierachicalTreeItemData* data = 
		dynamic_cast<wxCmguiHierachicalTreeItemData*>(
			scene_editor->testing_tree_ctrl->GetItemData(current_item_id));
	if ((region = data->GetRegion()))
	{
		rendition = Cmiss_region_get_rendition(region);
		if (rendition)
		{
			Cmiss_rendition_set_visibility(rendition,flag);
			SetTreeItemImage(current_item_id, !flag);
			DEACCESS(Cmiss_rendition)(&rendition);
		}
	}
}

void PropagateChanges(wxTreeItemId current_item_id, int flag)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child_id = scene_editor->testing_tree_ctrl->GetFirstChild(
		current_item_id, cookie);
	while (child_id.IsOk())
	{
			SetVisibilityOfTreeId(child_id,flag);
			PropagateChanges(child_id, flag);
			child_id = scene_editor->testing_tree_ctrl->GetNextChild(
				current_item_id, cookie);
	}
}

void TreeControlImageClicked(wxEvent &event)
{
	ENTER(TreeControlImageClicked);

	wxTreeEvent *tree_event = (wxTreeEvent *)&event;  

	if (Cmiss_rendition_get_visibility(
				scene_editor->edit_rendition))
	{
		SetVisibilityOfTreeId(tree_event->GetItem(),0);
		PropagateChanges(tree_event->GetItem(),0);
	}
	else
	{
		SetVisibilityOfTreeId(tree_event->GetItem(),1);
		PropagateChanges(tree_event->GetItem(),1);
	}
	LEAVE;
}

void OnMenuSelectionOn(wxCommandEvent &event)
{
	wxArrayTreeItemIds array;

	USE_PARAMETER(event);

	int num = scene_editor->testing_tree_ctrl->GetSelections(array);
	for (int i = 0; i < num; i ++)
	{
		SetVisibilityOfTreeId(array[i], 1);
	}
}

void OnMenuSelectionOff(wxCommandEvent &event)
{
	wxArrayTreeItemIds array;

	USE_PARAMETER(event);

	int num = scene_editor->testing_tree_ctrl->GetSelections(array);
	for (int i = 0; i < num; i ++)
	{
		SetVisibilityOfTreeId(array[i], 0);
	}
}

void CloseSceneEditor(wxCloseEvent &event)
{
	 ENTER(CloseSceneEditor);
	USE_PARAMETER(event);
	 DESTROY(Scene_editor)(scene_editor->scene_editor_address);
	 LEAVE;
}

  DECLARE_DYNAMIC_CLASS(wxSceneEditor);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxSceneEditor, wxFrame)

BEGIN_EVENT_TABLE(wxSceneEditor, wxFrame)
	EVT_SPLITTER_SASH_POS_CHANGED(XRCID("LowerSplitter"),wxSceneEditor::ResetWindow)
	EVT_COLLAPSIBLEPANE_CHANGED(XRCID("CollapsiblePane"), wxSceneEditor::CollapsiblepaneChangedEvent)
	EVT_COLLAPSIBLEPANE_CHANGED(XRCID("SceneEditorTopCollapsiblePane"), wxSceneEditor::CollapsiblepaneChangedEvent)
	EVT_TEXT_ENTER(XRCID("CircleDiscretisationPanel"), wxSceneEditor::CircleDiscretisationUpdate)
	EVT_TEXT_ENTER(XRCID("ElementDiscretisationPanel"), wxSceneEditor::ElementDiscretisationUpdate)
	EVT_CHECKBOX(XRCID("NativeDiscretisationFieldCheckBox"),wxSceneEditor::NativeDiscretisationFieldChecked)
	EVT_CHECKBOX(XRCID("AutoCheckBox"),wxSceneEditor::AutoChecked)
	EVT_BUTTON(XRCID("ApplyButton"),wxSceneEditor::ApplyClicked)
	EVT_BUTTON(XRCID("RevertButton"),wxSceneEditor::RevertClicked)
	EVT_BUTTON(XRCID("AddButton"),wxSceneEditor::AddToGraphicList)
	EVT_CHECKLISTBOX(XRCID("CmissGraphicListBox"), wxSceneEditor::GraphicListBoxChecked)
	EVT_TEXT_ENTER(XRCID("NameTextField"),wxSceneEditor::GraphicEditorNameText)
	EVT_LISTBOX(XRCID("CmissGraphicListBox"), wxSceneEditor::GraphicListBoxClicked)
	EVT_BUTTON(XRCID("DelButton"),wxSceneEditor::RemoveFromGraphicList)
	EVT_BUTTON(XRCID("UpButton"),wxSceneEditor::MoveUpInGraphicList)
	EVT_BUTTON(XRCID("DownButton"),wxSceneEditor::MoveDownInGraphicList)
	EVT_CHECKBOX(XRCID("CoordinateFieldCheckBox"),wxSceneEditor::CoordinateFieldChecked)
	EVT_TEXT_ENTER(XRCID("ConstantRadiusTextCtrl"), wxSceneEditor::EnterRadius)
	EVT_TEXT_ENTER(XRCID("ScaleFactorsTextCtrl"), wxSceneEditor::EnterRadius)
	EVT_CHECKBOX(XRCID("RadiusScalarCheckBox"), wxSceneEditor::EnterRadius)
	EVT_RADIOBUTTON(XRCID("IsoValueListRadioButton"), wxSceneEditor::EnterIsoScalar)
	EVT_RADIOBUTTON(XRCID("IsoValueSequenceRadioButton"), wxSceneEditor::EnterIsoScalar)
	EVT_TEXT_ENTER(XRCID("IsoScalarTextCtrl"),wxSceneEditor::EnterIsoScalar)
	EVT_TEXT_ENTER(XRCID("IsoValueSequenceNumberTextCtrl"),wxSceneEditor::EnterIsoScalar)
	EVT_TEXT_ENTER(XRCID("IsoValueSequenceFirstTextCtrl"),wxSceneEditor::EnterIsoScalar)
	EVT_TEXT_ENTER(XRCID("IsoValueSequenceLastTextCtrl"),wxSceneEditor::EnterIsoScalar)
	EVT_TEXT_ENTER(XRCID("CentreTextCtrl"),wxSceneEditor::EnterGlyphCentre)
	EVT_TEXT_ENTER(XRCID("BaseGlyphSizeTextCtrl"),wxSceneEditor::EnterGlyphSize)
	EVT_TEXT_ENTER(XRCID("GlyphScaleFactorsTextCtrl"),wxSceneEditor::EnterGlyphScale)
	EVT_CHECKBOX(XRCID("OrientationScaleCheckBox"),wxSceneEditor::EnterGlyphScale)
	EVT_CHECKBOX(XRCID("OverlayCheckBox"),wxSceneEditor::OverlayChecked)
	EVT_TEXT_ENTER(XRCID("OverlayTextCtrl"),wxSceneEditor::OverlayEntered)
	EVT_CHECKBOX(XRCID("VariableScaleCheckBox"),wxSceneEditor::VariableScaleChecked)
	EVT_CHECKBOX(XRCID("LabelCheckBox"),wxSceneEditor::LabelChecked)
	EVT_CHECKBOX(XRCID("VisibilityFieldCheckBox"),wxSceneEditor::VisibilityFieldChecked)
	EVT_TEXT_ENTER(XRCID("DiscretizationTextCtrl"),wxSceneEditor::EnterElementDiscretization)
	EVT_TEXT_ENTER(XRCID("CircleDiscretizationTextCtrl"),wxSceneEditor::EnterCircleDiscretization)
	EVT_CHECKBOX(XRCID("NativeDiscretizationFieldCheckBox"),wxSceneEditor::NativeDiscretizationChecked)
	EVT_CHECKBOX(XRCID("SeedElementCheckBox"),wxSceneEditor::SeedElementChecked)
	EVT_TEXT_ENTER(XRCID("XiTextCtrl"),wxSceneEditor::EnterSeedXi)
	EVT_TEXT_ENTER(XRCID("LengthTextCtrl"),wxSceneEditor::EnterLength)
	EVT_TEXT_ENTER(XRCID("WidthTextCtrl"),wxSceneEditor::EnterWidth)
	EVT_CHECKBOX(XRCID("ReverseCheckBox"),wxSceneEditor::ReverseChecked)
	EVT_TEXT_ENTER(XRCID("LineWidthTextCtrl"),wxSceneEditor::EnterLineWidth)
	EVT_CHECKBOX(XRCID("DataCheckBox"),wxSceneEditor::DataFieldChecked)
	EVT_CHECKBOX(XRCID("TextureCoordinatesCheckBox"),wxSceneEditor::TextureCoordFieldChecked)
	EVT_CHECKBOX(XRCID("ExteriorCheckBox"),wxSceneEditor::ExteriorChecked)
	EVT_CHECKBOX(XRCID("FaceCheckBox"),wxSceneEditor::FaceChecked)
	EVT_CHOICE(XRCID("FaceChoice"),wxSceneEditor::FaceChosen)
	EVT_TREE_SEL_CHANGED(wxID_ANY, wxSceneEditor::TreeControlSelectionChanged)
	EVT_CUSTOM(wxEVT_TREE_IMAGE_CLICK_EVENT, wxID_ANY, wxSceneEditor::TreeControlImageClicked)
	EVT_TREE_ITEM_MENU(CmguiTree_Ctrl, wxSceneEditor::OnItemMenu)
	EVT_MENU(CmguiTree_MenuItem1, wxSceneEditor::OnMenuSelectionOn)
	EVT_MENU(CmguiTree_MenuItem2, wxSceneEditor::OnMenuSelectionOff)
	EVT_MENU(AddMenuItemNode, wxSceneEditor::AddGraphicItemFromMenu)
	EVT_MENU(AddMenuItemData, wxSceneEditor::AddGraphicItemFromMenu)
	EVT_MENU(AddMenuItemLines, wxSceneEditor::AddGraphicItemFromMenu)
	EVT_MENU(AddMenuItemCylinders, wxSceneEditor::AddGraphicItemFromMenu)
	EVT_MENU(AddMenuItemSurfaces, wxSceneEditor::AddGraphicItemFromMenu)
	EVT_MENU(AddMenuItemIsoSurfaces, wxSceneEditor::AddGraphicItemFromMenu)
	EVT_MENU(AddMenuItemElement, wxSceneEditor::AddGraphicItemFromMenu)
	EVT_MENU(AddMenuItemStreamlines, wxSceneEditor::AddGraphicItemFromMenu)
	EVT_MENU(AddMenuItemStaticGraphic, wxSceneEditor::AddGraphicItemFromMenu)
#if defined (__WXMSW__)
	 EVT_SIZE(wxSceneEditor::FrameGetSize)
#endif /*!defined (__WXMSW__)*/
	 EVT_CLOSE(wxSceneEditor::CloseSceneEditor)
END_EVENT_TABLE()

int Scene_editor_revert_changes(Scene_editor *scene_editor)
{
	int return_code = 0;
 	gtMatrix transformation_matrix;
	if (scene_editor && scene_editor->wx_scene_editor)
	{
		return_code = 1;

		if (scene_editor->rendition)
		{
			if (!scene_editor->graphiclistbox)
			{
				scene_editor->graphiclistbox =  XRCCTRL(*scene_editor->wx_scene_editor,
					"CmissGraphicListBox",wxCheckListBox);
			}
			
			int selection=	scene_editor->graphiclistbox->GetSelection();
			scene_editor->graphiclistbox->SetSelection(wxNOT_FOUND);
			scene_editor->graphiclistbox->Clear();
			for_each_graphic_in_Cmiss_rendition(scene_editor->rendition,
				Scene_editor_add_graphic_item, (void *)scene_editor);
			REACCESS(Cmiss_rendition)(&scene_editor->edit_rendition,
				create_editor_copy_Cmiss_rendition(scene_editor->rendition));
			int num = 	scene_editor->graphiclistbox->GetCount();
			if (selection >= num)
			{
				selection = 0;
			}
			scene_editor->graphiclistbox->SetSelection(selection);
			scene_editor->collpane->Show();
			scene_editor->lowersplitter->Show();
			scene_editor->wx_scene_editor->
				Scene_editor_wx_set_manager_in_field_choosers(scene_editor);
			set_general_graphic(scene_editor);
			Cmiss_graphic *temp_graphic = Cmiss_rendition_get_graphic_at_position(
				scene_editor->edit_rendition, selection+1);
			scene_editor->wx_scene_editor->Scene_editor_wx_update_graphic_type(temp_graphic);
			scene_editor->wx_scene_editor->Scene_editor_wx_update_graphic_widgets();
			Cmiss_graphic_destroy(&temp_graphic);
			if (scene_editor->transformation_editor)
			{
				Cmiss_rendition_get_transformation(scene_editor->rendition,
					&transformation_matrix);
				scene_editor->transformation_editor->set_transformation(&transformation_matrix);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_editor_revert_changes.  Missing rendition");
			return_code = 0;
		}

		if (return_code)
		{
			wxButton *applybutton;
			wxButton *revertbutton;
			scene_editor->child_edited = 0;
			applybutton = XRCCTRL(*scene_editor->wx_scene_editor, "ApplyButton", wxButton);
			revertbutton = XRCCTRL(*scene_editor->wx_scene_editor,"RevertButton", wxButton);
			applybutton->Disable();
			revertbutton->Disable();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene Editor Revert Changes.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/***************************************************************************//**
*AW: Reaccess both pointers of scene_editor->rendition and 
*scene_editor->edit_rendition to rendition if scene object is 
*this also change the current field manager in scene editor.
* 
* @param scene_editor scene editor to be modified
* @param scene_object Currently active object 
*/

void Scene_editor_set_active_rendition(
	 struct Scene_editor *scene_editor, struct Cmiss_rendition *rendition)
{
	if (scene_editor->rendition)
	{
		if (scene_editor->transformation_callback_flag)
		{
			scene_editor->transformation_editor->set_rendition(NULL);
			if (Cmiss_rendition_remove_transformation_callback(scene_editor->rendition,
					Scene_editor_wx_transformation_change, (void *)scene_editor))
			{
				scene_editor->transformation_callback_flag = 0;
			}
		}
		if (scene_editor->rendition_callback_flag)
		{
			if (Cmiss_rendition_remove_callback(scene_editor->rendition,
					Scene_editor_wx_rendition_change, (void *)scene_editor))
			{
				scene_editor->rendition_callback_flag = 0;
			}
		}
	}
	 if (scene_editor && rendition)
	 {
		 int previous_selection;
		 if (scene_editor->edit_rendition && scene_editor->current_graphic)
		 {
			 previous_selection = Cmiss_rendition_get_graphic_position(
				 scene_editor->edit_rendition, scene_editor->current_graphic);
		 }
		 else
		 {
			 previous_selection = 0;
		 }
		 REACCESS(Cmiss_rendition)(&scene_editor->rendition, rendition);
		 Cmiss_rendition *edit_rendition;
		 if (scene_editor->rendition)
		 {
			 edit_rendition =
				 create_editor_copy_Cmiss_rendition(scene_editor->rendition);
			 if (!edit_rendition)
			 {
				 display_message(ERROR_MESSAGE,
					 "Rendition_editor_set_rendition.  "
					 "Could not copy rendition");
			 }
		 }
		 else
		 {
			 edit_rendition = (struct Cmiss_rendition *)NULL;
		 }
		 REACCESS(Cmiss_rendition)(&(scene_editor->edit_rendition),
			 edit_rendition);
		 DEACCESS(Cmiss_rendition)(&edit_rendition);
		 if (previous_selection == 0)
		 {
			 previous_selection = 1;
		 }
		 if (!scene_editor->graphiclistbox)
		 {
			 scene_editor->graphiclistbox = XRCCTRL(
				 *scene_editor->wx_scene_editor, "CmissGraphicListBox",wxCheckListBox);
		 }
		 scene_editor->graphiclistbox->SetSelection(wxNOT_FOUND);
		 scene_editor->graphiclistbox->Clear();
		 if (scene_editor->edit_rendition)
		 {
			 for_each_graphic_in_Cmiss_rendition(scene_editor->edit_rendition,
				 Scene_editor_add_graphic, (void *)scene_editor);
			 int num = Cmiss_rendition_get_number_of_graphic(
				 scene_editor->edit_rendition);
			 Cmiss_graphic *temp_graphic = NULL;
			 if (num > previous_selection) 
			 {
				 num = previous_selection;
			 }
			 temp_graphic = Cmiss_rendition_get_graphic_at_position(
				 scene_editor->edit_rendition, num);
			 scene_editor->wx_scene_editor->Scene_editor_wx_update_graphic_type(
				 temp_graphic);
			 scene_editor->graphiclistbox->SetSelection(num-1);
			 REACCESS(Cmiss_graphic)(&scene_editor->current_graphic, temp_graphic);
			 if (temp_graphic)
				 Cmiss_graphic_destroy(&temp_graphic);
			 if (scene_editor->current_graphic)
			 {
				 scene_editor->current_graphic_type = Cmiss_graphic_get_graphic_type(
					 scene_editor->current_graphic);
			 }
			 scene_editor->collpane->Enable();
			 scene_editor->collpane->Show();
			 scene_editor->lowersplitter->Enable();
			 scene_editor->lowersplitter->Show();
			 scene_editor->field_manager = Cmiss_region_get_Computed_field_manager(
				 Cmiss_rendition_get_region(
					 scene_editor->edit_rendition));
		 }
		 if (scene_editor->rendition)
		 {
			 gtMatrix transformation_matrix;
			 Cmiss_rendition_get_transformation(scene_editor->rendition,
				 &transformation_matrix);
			 scene_editor->transformation_editor->set_rendition(
				 scene_editor->rendition);
			 scene_editor->transformation_editor->set_transformation(
				 &transformation_matrix);
			 if (Cmiss_rendition_add_transformation_callback(scene_editor->rendition,
					 Scene_editor_wx_transformation_change, (void *)scene_editor))
			 {
				 scene_editor->transformation_callback_flag = 1;
			 }
			 if (Cmiss_rendition_add_callback(scene_editor->rendition,
					 Scene_editor_wx_rendition_change, (void *)scene_editor))
			 {
				 scene_editor->rendition_callback_flag = 1;
			 }
		 }
	 }
	 else if (scene_editor)
	 {
		 REACCESS(Cmiss_rendition)(&scene_editor->rendition, NULL);
		 REACCESS(Cmiss_rendition)(&scene_editor->edit_rendition, NULL);
		 REACCESS(Cmiss_graphic)(&scene_editor->current_graphic,NULL);
	 }
}

static int set_general_graphic(struct Scene_editor *scene_editor)
/*******************************************************************************
LAST MODIFIED : 16 Match 2007

DESCRIPTION :
Renew the display of  the general settings
==============================================================================*/
{
	struct Element_discretization element_discretization;
	wxTextCtrl *circlediscretisationpanel =	XRCCTRL(*scene_editor->wx_scene_editor,
				"CircleDiscretisationPanel",wxTextCtrl);
	wxTextCtrl *elementdiscretisationpanel = XRCCTRL(*scene_editor->wx_scene_editor, 
		 "ElementDiscretisationPanel",wxTextCtrl);
	char temp_string[80];
	scene_editor->wx_scene_editor->RenewGeneralSettingChooser(scene_editor->edit_rendition);
	int temp = Cmiss_rendition_get_circle_discretization(scene_editor->edit_rendition);
	wxString circle_discretisation = wxString::Format(wxT("%d"), (int)temp);
	circlediscretisationpanel->SetValue(circle_discretisation);
	Cmiss_rendition_get_element_discretization(
	   scene_editor->edit_rendition,&element_discretization);
	sprintf(temp_string,"%d*%d*%d",
		element_discretization.number_in_xi1,
		element_discretization.number_in_xi2,
		element_discretization.number_in_xi3);
	 elementdiscretisationpanel->SetValue(temp_string);
	return 1;
}

/***************************************************************************//**
 *Get and set the display of graphic
 */
static int get_and_set_Cmiss_graphic_widgets(void *scene_editor_void)
{
	Scene_editor *scene_editor = static_cast<Scene_editor*>(scene_editor_void);

	/*for the text field*/
	if (scene_editor->current_graphic)
	{
		 const char *name;
		 Cmiss_graphic_get_name(scene_editor->current_graphic,
				&name);
		 wxTextCtrl *nametextfield = XRCCTRL(*scene_editor->wx_scene_editor, "NameTextField", wxTextCtrl);
		 nametextfield->SetValue(name);
		 DEALLOCATE(name);

		 /*for the selected and material chooser*/
		 scene_editor->wx_scene_editor->SetBothMaterialChooser(scene_editor->current_graphic);
		 scene_editor->wx_scene_editor->SetGraphic(scene_editor->current_graphic);
	}
	wxFrame *frame=XRCCTRL(*scene_editor->wx_scene_editor, "CmguiSceneEditor", wxFrame);
	frame->Layout();
	return 1;
}

/***************************************************************************//**
 * Iterator function for Graphical_element_editor_update_Graphic_item.
 */
static int Scene_editor_add_graphic_item(
	 struct Cmiss_graphic *graphic, void *scene_editor_void)
{
	char *graphic_string;
	int return_code;
	struct Scene_editor *scene_editor;
	ENTER(Scene_editor_add_graphic_item);
	if (graphic && (scene_editor = static_cast<Scene_editor*>(scene_editor_void)))
	{
		 graphic_string = Cmiss_graphic_string(graphic,
				GRAPHIC_STRING_COMPLETE_PLUS);
		 wxCheckListBox *graphicalitemschecklist =  XRCCTRL(*scene_editor->wx_scene_editor, "CmissGraphicListBox",wxCheckListBox);
		 graphicalitemschecklist->Append(graphic_string);
		 if (  Cmiss_graphic_get_visibility(graphic) ==1)
		 {
				graphicalitemschecklist->Check((graphicalitemschecklist->GetCount()-1),1);
		 }
		 DEALLOCATE(graphic_string);
		 return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_add_graphic_item.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_add_graphic_item */

#endif /* defined (WX_USER_INTERFACE) */

void Scene_editor_region_tree_region_add_child(Scene_editor *scene_editor, Cmiss_region *parent_region,
       wxTreeItemId parent)
{
	char *child_name;
	wxTreeItemId current;
	Cmiss_region *current_region;
	ENTER(Scene_edito);
	
	current_region = Cmiss_region_get_first_child(parent_region);
	while (current_region)
	{
		child_name = Cmiss_region_get_name(current_region);
		if (child_name)
		{
			current = scene_editor->testing_tree_ctrl->AppendItem(parent,child_name,0,0);
			Scene_editor_region_tree_region_add_child(scene_editor,current_region,current);
			DEALLOCATE(child_name);
		}
		Cmiss_region_reaccess_next_sibling(&current_region);
	}

	LEAVE;
}

void Scene_editor_setup_region_tree(Scene_editor *scene_editor)
{
	wxTreeItemId current;
	char *root_region_path;
	struct Cmiss_rendition *rendition;
	
	ENTER(Scene_editor_setup_region_tree);
	
	root_region_path = Cmiss_region_get_root_region_path();
	if (root_region_path)
	{
		current = scene_editor->testing_tree_ctrl->AddRoot(root_region_path,0,0);
		scene_editor->testing_tree_ctrl->SetTreeIdRegionWithCallback(
			current, scene_editor->root_region);
		scene_editor->testing_tree_ctrl->add_all_child_regions_to_tree_item(current);
		rendition = Cmiss_region_get_rendition(scene_editor->root_region);
		REACCESS(Cmiss_rendition)(&scene_editor->rendition,
			rendition);
		DEACCESS(Cmiss_rendition)(&rendition);
		DEALLOCATE(root_region_path);
	}

	LEAVE;
}

/*
Global functions
----------------
*/

struct Scene_editor *CREATE(Scene_editor)(	
	struct Scene_editor **scene_editor_address,
	struct MANAGER(Scene) *scene_manager, struct Scene *scene,
	struct Cmiss_region *root_region,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct Graphics_font *default_font,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct Graphics_font_package *font_package,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 2 Febuary 2007

DESCRIPTION :
==============================================================================*/
{
	struct Scene_editor *scene_editor;
 
	ENTER(CREATE(Scene_editor));
	scene_editor = (struct Scene_editor *)NULL;
	if (scene_manager && scene && root_region &&
		graphical_material_manager && default_material &&
		glyph_list && spectrum_manager && default_spectrum &&
		volume_texture_manager && font_package && user_interface)
	{
		if (ALLOCATE(scene_editor,struct Scene_editor,1))
		{		
			 scene_editor->auto_apply = 1;
			 scene_editor->child_edited =1;
			 scene_editor->child_expanded=1; 
			 scene_editor->transformation_expanded=1;		
			 scene_editor->transformation_callback_flag = 0;
			 scene_editor->gt_element_group_callback_flag = 0;
			 scene_editor->rendition_callback_flag = 0;
			 scene_editor->scene = scene;
			 scene_editor->graphical_material_manager = graphical_material_manager;
			 scene_editor->scene_editor_address = (struct Scene_editor **)NULL;
			 scene_editor->default_material=default_material;
			 scene_editor->selected_material=default_material;
			 scene_editor->default_font=default_font;
			 scene_editor->glyph_list=glyph_list;
			 scene_editor->scene_manager = scene_manager;
			 scene_editor->user_interface=user_interface;
			 scene_editor->current_graphic_type=CMISS_GRAPHIC_LINES;
			 scene_editor->current_graphic = (Cmiss_graphic *)NULL;
			 scene_editor->default_coordinate_field=(Computed_field *)NULL;
			 scene_editor->volume_texture_manager=volume_texture_manager;
			 scene_editor->field_manager=(MANAGER(Computed_field)*)NULL;
 			 scene_editor->font_manager=Graphics_font_package_get_font_manager(font_package);
			 scene_editor->native_discretization_field=(FE_field*)NULL ;
			 scene_editor->coordinate_field=(Computed_field *)NULL;	
			 scene_editor->select_mode=(Graphics_select_mode)NULL;
			 scene_editor->constant_radius=0.0;
			 scene_editor->radius_scale_factor=1.0;
			 scene_editor->radius_scalar_field = (Computed_field *)NULL;
			 scene_editor->use_element_type= (Use_element_type)NULL;
			 scene_editor->xi_discretization_mode= (Xi_discretization_mode)NULL;
			 scene_editor->streamline_type=(Streamline_type)NULL;
			 scene_editor->streamline_data_type=(Streamline_data_type)NULL;
			 scene_editor->spectrum_manager=spectrum_manager;
			 scene_editor->spectrum = default_spectrum;
			 scene_editor->render_type =(Render_type)NULL;
			 scene_editor->fe_element =(FE_element *)NULL;
			 scene_editor->root_region = root_region;
			 scene_editor->current_region = NULL;
			 scene_editor->wx_scene_editor = (wxSceneEditor *)NULL;
			 scene_editor->graphiclistbox = (wxCheckListBox *)NULL;
			 scene_editor->rendition = (Cmiss_rendition *)NULL;
			 scene_editor->edit_rendition = (Cmiss_rendition *)NULL;
			 wxLogNull logNo;
			 scene_editor->wx_scene_editor = new 
					wxSceneEditor(scene_editor);
			 scene_editor->lowersplitter=XRCCTRL(*scene_editor->wx_scene_editor,"LowerSplitter",wxSplitterWindow);
			 scene_editor->top_collpane = XRCCTRL(*scene_editor->wx_scene_editor, 
					"SceneEditorTopCollapsiblePane", wxCollapsiblePane);
			 wxWindow *top_collpane_win = scene_editor->top_collpane->GetPane();
			 scene_editor->top_collpane_panel = new wxPanel(top_collpane_win);
			 scene_editor->transformation_editor = new Transformation_editor(
 					scene_editor->top_collpane_panel, "transformation_editor", NULL,
					&scene_editor->auto_apply);
			 wxSizer *top_collpane_sizer = new wxBoxSizer(wxVERTICAL);
			 top_collpane_sizer->Add(scene_editor->top_collpane_panel, 1, wxEXPAND|wxALL, 2);
			 top_collpane_win->SetSizer(top_collpane_sizer);
			 top_collpane_sizer->SetSizeHints(top_collpane_win);
			 top_collpane_sizer->Layout();
			 top_collpane_win->Layout();
			 scene_editor->top_collpane->Collapse(1);
			 scene_editor->autocheckbox = 
					XRCCTRL(*scene_editor->wx_scene_editor, "AutoCheckBox", wxCheckBox);
			 scene_editor->autocheckbox->SetValue(true);
			 scene_editor->applybutton = 
					XRCCTRL(*scene_editor->wx_scene_editor, "ApplyButton", wxButton);
			 scene_editor->applybutton->Disable();
			 scene_editor->revertbutton = 
					XRCCTRL(*scene_editor->wx_scene_editor, "RevertButton", wxButton);
			 scene_editor->revertbutton->Disable();
			 scene_editor->frame=
					XRCCTRL(*scene_editor->wx_scene_editor, "CmguiSceneEditor", wxFrame);
			 scene_editor->frame->Layout();
			 scene_editor->frame->SetMinSize(wxSize(50,100));
			 scene_editor->frame->SetSize(wxSize(600,800));
			 scene_editor->frame->SetMaxSize(wxSize(2000,2000));
			 scene_editor->sceneediting = 
					XRCCTRL(*scene_editor->wx_scene_editor, "SceneEditing", wxScrolledWindow);
			 scene_editor->graphiclistbox =
					XRCCTRL(*scene_editor->wx_scene_editor, "CmissGraphicListBox",wxCheckListBox);
			 scene_editor->sceneediting->Layout();
			 scene_editor->sceneediting->SetScrollbars(10,10,40,40);
			 scene_editor->topsplitter=XRCCTRL(*scene_editor->wx_scene_editor,"TopSplitter",wxSplitterWindow);
			 scene_editor->topsplitter->SetSashPosition(150);
			 scene_editor->lowersplitter->SetSashPosition(160);
			 scene_editor->lowersplitter->Layout();
			 scene_editor->lowersplitter->Hide();
			 scene_editor->topsplitter->Layout();
			 scene_editor->testing_tree_ctrl = new wxCmguiHierachicalTree(scene_editor->wx_scene_editor,
				 XRCCTRL(*scene_editor->wx_scene_editor,"TreeControlPanel",wxPanel));
			 scene_editor->ImageList = new wxImageList(13,13);
			 scene_editor->ImageList->Add(wxIcon(tickbox_xpm));
			 scene_editor->ImageList->Add(wxIcon(unticked_box_xpm));
			 scene_editor->testing_tree_ctrl->AssignImageList(scene_editor->ImageList);
			 Scene_editor_setup_region_tree(scene_editor);
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
		if (scene_editor->current_graphic)
			DEACCESS(Cmiss_graphic)(&scene_editor->current_graphic);
		if (scene_editor->edit_rendition)
			DEACCESS(Cmiss_rendition)(&scene_editor->edit_rendition);
		if (scene_editor->rendition)
		{
			scene_editor->transformation_editor->set_rendition(NULL);
			if (scene_editor->transformation_callback_flag &&
				Cmiss_rendition_remove_transformation_callback(scene_editor->rendition,
					Scene_editor_wx_transformation_change, (void *)scene_editor))
			{
				scene_editor->transformation_callback_flag = 0;
			}
			if (scene_editor->rendition_callback_flag)
			{
				if (Cmiss_rendition_remove_callback(scene_editor->rendition,
						Scene_editor_wx_rendition_change, (void *)scene_editor))
				{
					scene_editor->rendition_callback_flag = 0;
				}
			}
		}
		if (scene_editor->rendition)
			DEACCESS(Cmiss_rendition)(&scene_editor->rendition);
		delete scene_editor->transformation_editor;
		delete scene_editor->wx_scene_editor;
		DEALLOCATE(*scene_editor_address);
		*scene_editor_address = NULL;
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
	scene = (struct Scene*)NULL;
	if (scene_editor)
	{
		 scene = scene_editor->scene;
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
				if(scene_editor->wx_scene_editor)
				{
					 scene_editor->wx_scene_editor->setSceneObject(scene);
					 return_code = 1;
				}
				else
				{
					 display_message(ERROR_MESSAGE,
							"Scene_editor_set_scene.  Could not set new scene");
					 return_code = 0;
				}
		 }
		 if (return_code)
		 {
				scene_editor->scene = scene;
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

/***************************************************************************//**
* Setup the graphic widgets.
* 
* @param scene_editor scene editor to be be modified
*/
void Scene_editor_set_graphic_widgets_for_rendition(Scene_editor *scene_editor)
{
	 ENTER(Scene_editor_set_graphic_widgets_for_rendition);
	 if (scene_editor && scene_editor->rendition)
	 {
		 scene_editor->wx_scene_editor->
			 Scene_editor_wx_set_manager_in_field_choosers(scene_editor);
		 set_general_graphic(scene_editor);
		 get_and_set_Cmiss_graphic_widgets((void *)scene_editor);
		 scene_editor->collpane->Enable();
		 scene_editor->collpane->Show();
		 scene_editor->lowersplitter->Enable();
		 scene_editor->lowersplitter->Show();
	 }
	 LEAVE;
}

static int Scene_editor_wx_rendition_change(
	struct Cmiss_rendition *rendition, void *scene_editor_void)
{
	Scene_editor *scene_editor;
	int return_code;
	//	wxCheckListBox *cmiss_graphic_checklist;
	
	if (rendition && 
		(scene_editor = (struct Scene_editor *)scene_editor_void))
	{
		return_code = 1;
		if (!Cmiss_renditions_match(
					rendition, scene_editor->edit_rendition))
		{
			if (scene_editor->auto_apply)
			{
				Scene_editor_revert_changes(scene_editor);
			}
			else
			{
				wxButton *applybutton;
				wxButton *revertbutton;
				scene_editor->child_edited = 0;
				applybutton = XRCCTRL(*scene_editor->wx_scene_editor, "ApplyButton", wxButton);
				revertbutton = XRCCTRL(*scene_editor->wx_scene_editor,"RevertButton", wxButton);
				applybutton->Enable();
				revertbutton->Enable();
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_wx_rendition_change.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

static int Scene_editor_add_graphic(
	 struct Cmiss_graphic *graphic, void *scene_editor_void)
{
	char *graphic_string;
	int return_code;
	struct Scene_editor *scene_editor;
	ENTER(Scene_editor_add_graphic);
	if (graphic && (scene_editor = static_cast<Scene_editor*>(scene_editor_void)))
	{
		 graphic_string = Cmiss_graphic_string(graphic,
				GRAPHIC_STRING_COMPLETE_PLUS);
		 wxCheckListBox *graphicchecklist =  XRCCTRL(*scene_editor->wx_scene_editor, "CmissGraphicListBox",wxCheckListBox);
		 graphicchecklist->Append(graphic_string);
		 if (Cmiss_graphic_get_visibility(graphic) ==1)
		 {
			graphicchecklist->Check((graphicchecklist->GetCount()-1),1);
		 }
		 DEALLOCATE(graphic_string);
		 return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_editor_add_graphic.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_editor_add_element_graphic_item */
