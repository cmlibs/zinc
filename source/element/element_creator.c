/*******************************************************************************
FILE : element_creator.c

LAST MODIFIED : 14 May 2003

DESCRIPTION :
Dialog for choosing the type of element constructed in response to node
selections. Elements are created in this way while dialog is open.
==============================================================================*/
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>
#include "choose/choose_fe_field.h"
#include "region/cmiss_region_chooser.h"
#include "element/element_creator.h"
#include "element/element_creator.uidh"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/

#if defined (MOTIF)
static int element_creator_hierarchy_open=0;
static MrmHierarchy element_creator_hierarchy;
#endif /* defined (MOTIF) */

/*
Module types
------------
*/

struct Element_creator
/*******************************************************************************
LAST MODIFIED : 10 January 2003

DESCRIPTION :
==============================================================================*/
{
	struct Element_creator **element_creator_address;
	struct FE_field *coordinate_field;
	/* maintain a template node for creating new nodes */
	struct FE_node *template_node;
	/* the dimension of the elements being created - user settable */
	int element_dimension;
	/* maintain template element for creating new elements */
	struct FE_element *template_element;
	struct User_interface *user_interface;

	/* indicates whether elements are created in response to node selections */
	int create_enabled;

	/* the element being created */
	struct FE_element *element;
	/* number of nodes that have been set in the element being created */
	int number_of_clicked_nodes;

	/* root_region and region/fe_region to put new elements in */
	struct Cmiss_region *region, *root_region;
	struct FE_region *fe_region;

	/* selections */
	struct FE_element_selection *element_selection;
	struct FE_node_selection *node_selection;

	struct Cmiss_region_chooser *region_chooser;

	Widget coordinate_field_form,coordinate_field_widget,create_button,
		element_dimension_text, region_form,
		node_list_widget;
	Widget widget,window_shell;
}; /* struct Element_creator */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(element_creator,Element_creator, \
	create_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_creator,Element_creator, \
	region_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_creator,Element_creator, \
	element_dimension_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_creator,Element_creator, \
	coordinate_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_creator,Element_creator, \
	node_list_widget)

static int Element_creator_make_template_node(
	struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Ensures there is a template node defined with the coordinate_field in the
<element_creator> for creating new nodes as copies of.
==============================================================================*/
{
	int return_code;
	struct FE_node_field_creator *node_field_creator;

	ENTER(Element_creator_make_template_node);
	if (element_creator)
	{
		if (element_creator->coordinate_field)
		{
			if (element_creator->fe_region)
			{
				return_code = 1;
				if (!element_creator->template_node)
				{
					if ((node_field_creator = CREATE(FE_node_field_creator)(
						/*number_of_components*/3))&&
						(element_creator->template_node = CREATE(FE_node)(/*node_number*/0,
							element_creator->fe_region, (struct FE_node *)NULL)))
					{
						/* template_node is accessed by element_creator but not managed */
						ACCESS(FE_node)(element_creator->template_node);
						if (!define_FE_field_at_node(element_creator->template_node,
							element_creator->coordinate_field,
							(struct FE_time_version *)NULL, node_field_creator))
						{
							display_message(ERROR_MESSAGE,
								"Element_creator_make_template_node.  "
								"Could not define coordinate field in template_node");
							DEACCESS(FE_node)(&(element_creator->template_node));
							return_code=0;
						}
						DESTROY(FE_node_field_creator)(&(node_field_creator));
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Element_creator_make_template_node.  "
							"Could not create template node");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_creator_make_template_node.  "
					"Must specify a region first");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_creator_make_template_node.  No coordinate_field specified");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_make_template_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_make_template_node */

static int Element_creator_make_template_element(
	struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 4 November 2002

DESCRIPTION :
This function checks if <element_creator> has a coordinate_field, element shape,
basis, field_info and node_scale_field_info. If not, it creates them for
generating 2-D bilinear elements.
==============================================================================*/
{
	int *basis_type,element_dimension,i,j,number_of_components,
		number_of_nodes,return_code,*type,*type_entry,*xi_basis_type;
	struct CM_element_information element_identifier;
	struct FE_basis *element_basis;
	struct FE_element_field_component *component,**components;
			struct FE_element_shape *element_shape;
	struct Standard_node_to_element_map *standard_node_map;

	ENTER(Element_creator_make_template_element);
	if (element_creator)
	{
		return_code=1;
		if (!element_creator->template_element)
		{
			element_dimension=element_creator->element_dimension;
			/* make template_node */
			if (!Element_creator_make_template_node(element_creator))
			{
				display_message(ERROR_MESSAGE,"Element_creator_make_template_element.  "
					"Could not make template node");
				return_code=0;
			}
			/* make shape */
			element_shape=(struct FE_element_shape *)NULL;
			if (return_code)
			{
				/* make default n-D "square" element shape */
				if (ALLOCATE(type,int,(element_dimension*(element_dimension+1))/2))
				{
					/* retrieve a "square" element of the specified element_dimension */
					type_entry=type;
					for (i=element_dimension-1;i>=0;i--)
					{
						*type_entry=LINE_SHAPE;
						type_entry++;
						for (j=i;j>0;j--)
						{
							*type_entry=0;
							type_entry++;
						}
					}
					if (element_shape=CREATE(FE_element_shape)(element_dimension,type,
						element_creator->fe_region))
					{
						ACCESS(FE_element_shape)(element_shape);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Element_creator_make_template_element.  Error creating shape");
						return_code=0;
					}
					DEALLOCATE(type);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_creator_make_template_element.  Not enough memory");
					return_code=0;
				}
			}
			/* make basis */
			element_basis=(struct FE_basis *)NULL;
			if (return_code)
			{
				/* make default N-linear basis */
				if (ALLOCATE(basis_type,int,
					1+(element_dimension*(1+element_dimension))/2))
				{
					xi_basis_type=basis_type;
					*xi_basis_type=element_dimension;
					xi_basis_type++;
					for (i=element_dimension;0<i;i--)
					{
						for (j=i;0<j;j--)
						{
							if (i==j)
							{
								*xi_basis_type=LINEAR_LAGRANGE;
							}
							else
							{
								*xi_basis_type=NO_RELATION;
							}
							xi_basis_type++;
						}
					}
					if (element_basis =
						make_FE_basis(basis_type,
							FE_region_get_basis_manager(element_creator->fe_region)))
					{
						ACCESS(FE_basis)(element_basis);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Element_creator_make_template_element.  Error creating shape");
						return_code=0;
					}
					DEALLOCATE(basis_type);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_creator_make_template_element.  Not enough memory");
					return_code=0;
				}
			}
			/* make template_element */
			if (return_code)
			{
				element_identifier.type=CM_ELEMENT;
				element_identifier.number=0;
				if (element_creator->template_element = CREATE(FE_element)(
					&element_identifier, element_shape, element_creator->fe_region,
					(struct FE_element *)NULL))
				{
					number_of_nodes=1;
					for (i=0;i<element_dimension;i++)
					{
						number_of_nodes *= 2;
					}
					if (set_FE_element_number_of_nodes(element_creator->template_element,
							number_of_nodes) &&
						set_FE_element_number_of_scale_factor_sets(
							element_creator->template_element,
							/*number_of_scale_factor_sets*/1,
							/*scale_factor_set_identifiers*/(void *)&element_basis,
							/*numbers_in_scale_factor_sets*/&number_of_nodes))
					{
						number_of_components = get_FE_field_number_of_components(
							element_creator->coordinate_field);
						if (ALLOCATE(components,struct FE_element_field_component *,
							number_of_components))
						{
							for (i=0;i<number_of_components;i++)
							{
								components[i]=(struct FE_element_field_component *)NULL;
							}
							for (i=0;(i<number_of_components)&&return_code;i++)
							{
								if (component=CREATE(FE_element_field_component)(
									STANDARD_NODE_TO_ELEMENT_MAP,number_of_nodes,
									element_basis,(FE_element_field_component_modify)NULL))
								{
									for (j=0;j<number_of_nodes;j++)
									{
										if (standard_node_map =
											CREATE(Standard_node_to_element_map)(
												/*node_index*/j, /*number_of_values*/1))
										{
											if (!(Standard_node_to_element_map_set_nodal_value_index(
												standard_node_map, 0, 0) &&
												Standard_node_to_element_map_set_scale_factor_index(
													standard_node_map, 0, j) &&
												/* set scale_factors to 1 */
												set_FE_element_scale_factor(
													element_creator->template_element,
													/*scale_factor_number*/j, 1.0) &&
												FE_element_field_component_set_standard_node_map(
													component, /*node_number*/j, standard_node_map)))
											{
												DESTROY(Standard_node_to_element_map)(
													&standard_node_map);
												return_code = 0;
											}
										}
										else
										{
											return_code=0;
										}
									}
								}
								else
								{
									return_code=0;
								}
								components[i]=component;
							}
							if (return_code)
							{
								if (define_FE_field_at_element(
									element_creator->template_element,
									element_creator->coordinate_field,components))
								{
									/* template_element is accessed by element_creator but not
										 placed in manager */
									ACCESS(FE_element)(element_creator->template_element);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Element_creator_make_template_element.  "
										"Could not define coordinate field at template_element");
									DESTROY(FE_element)(&(element_creator->template_element));
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Element_creator_make_template_element.  "
									"Could not create components");
								DESTROY(FE_element)(&(element_creator->template_element));
							}
							for (i=0;i<number_of_components;i++)
							{
								DESTROY(FE_element_field_component)(&(components[i]));
							}
							DEALLOCATE(components);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Element_creator_make_template_element.  "
								"Could not allocate components");
							DESTROY(FE_element)(&(element_creator->template_element));
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Element_creator_make_template_element.  "
							"Could not set element shape and field info");
						DESTROY(FE_element)(&(element_creator->template_element));
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_creator_make_template_element.  "
						"Could not create template element");
					return_code=0;
				}
			}
			/* deaccess basis and shape so at most used by template element */
			if (element_basis)
			{
				DEACCESS(FE_basis)(&element_basis);
			}
			if (element_shape)
			{
				DEACCESS(FE_element_shape)(&element_shape);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_make_template_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_make_template_element */

static int Element_creator_end_element_creation(
	struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
DEACCESSes the element being created, if any, and if it is unmanaged, warns that
the creation was aborted. Also clears the node list.
Call this function whether element is successfully created or not.
==============================================================================*/
{
	int return_code;

	ENTER(Element_creator_end_element_creation);
	if (element_creator)
	{
		if (element_creator->element)
		{
			if (!FE_region_contains_FE_element(element_creator->fe_region,
				element_creator->element))
			{
				display_message(WARNING_MESSAGE,
					"Element_creator: destroying incomplete element");
			}
			DEACCESS(FE_element)(&(element_creator->element));
			XmListDeleteAllItems(element_creator->node_list_widget);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_end_element_creation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_end_element_creation */

static int Element_creator_add_element(struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 14 May 2003

DESCRIPTION :
Adds the just created element to the fe_region, adding faces as necessary.
==============================================================================*/
{
	int return_code;

	ENTER(Element_creator_add_element);
	if (element_creator && element_creator->fe_region && element_creator->element)
	{
		FE_region_begin_change(element_creator->fe_region);
		FE_region_begin_define_faces(element_creator->fe_region);
		return_code = FE_region_merge_FE_element_and_faces_and_nodes(
			element_creator->fe_region, element_creator->element);
		FE_region_end_define_faces(element_creator->fe_region);
		FE_region_end_change(element_creator->fe_region);
		Element_creator_end_element_creation(element_creator);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_add_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_add_element */

static void Element_creator_node_selection_change(
	struct FE_node_selection *node_selection,
	struct FE_node_selection_changes *changes,void *element_creator_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Callback for change in the global node selection.
==============================================================================*/
{
	char temp_string[50];
	int number_of_nodes;
	struct CM_element_information element_identifier;
	struct Element_creator *element_creator;
	struct FE_node *node;
	XmString new_string;

	ENTER(Element_creator_node_selection_change);
	if (node_selection&&changes&&(element_creator=
		(struct Element_creator *)element_creator_void))
	{
		/* if exactly 1 node selected, add it to element */
		if (1==NUMBER_IN_LIST(FE_node)(changes->newly_selected_node_list))
		{
			/* get the last selected node and check it is in the FE_region */
			node = FIRST_OBJECT_IN_LIST_THAT(FE_node)(
				(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
				changes->newly_selected_node_list);
			if (FE_region_contains_FE_node(element_creator->fe_region, node))
			{
				if (!element_creator->element)
				{
					/* get next unused element identifier from fe_region */
					element_identifier.type = CM_ELEMENT;
					element_identifier.number = FE_region_get_next_FE_element_identifier(
						element_creator->fe_region, CM_ELEMENT, 1);
					if (Element_creator_make_template_element(element_creator) &&
						(element_creator->element = CREATE(FE_element)(&element_identifier,
							(struct FE_element_shape *)NULL, (struct FE_region *)NULL,
							element_creator->template_element)))
					{
						ACCESS(FE_element)(element_creator->element);
						element_creator->number_of_clicked_nodes=0;
					}
				}
				if (element_creator->element)
				{
					/* When we make more than linear elements we will need
						to check that the derivatives exist and are the correct ones */
					if (set_FE_element_node(element_creator->element,
						element_creator->number_of_clicked_nodes,node))
					{
						sprintf(temp_string,"%d. Node %d",
							element_creator->number_of_clicked_nodes+1,
							get_FE_node_identifier(node));
						new_string=XmStringCreateSimple(temp_string);
						XmListAddItem(element_creator->node_list_widget,new_string,0);
						XmStringFree(new_string);
						element_creator->number_of_clicked_nodes++;
						if (get_FE_element_number_of_nodes(element_creator->element,
							&number_of_nodes) &&
							(element_creator->number_of_clicked_nodes == number_of_nodes))
						{
							Element_creator_add_element(element_creator);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Element_creator_node_selection_change.  Could not set node");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_creator_scene_input_callback.  Could not create element");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element creator: Selected node not from current region");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_node_selection_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_creator_node_selection_change */

static int Element_creator_set_region(struct Element_creator *element_creator,
	struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 14 January 2003

DESCRIPTION :
Sets the <region> containing the FE_region where elements created by
<element_creator> are placed.
The <element_creator> assumes its nodes are to come from the same FE_region.
==============================================================================*/
{
	int return_code;

	ENTER(Element_creator_set_region);
	if (element_creator)
	{
		return_code = 1;
		if (region != element_creator->region)
		{
			Element_creator_end_element_creation(element_creator);
			element_creator->region = region;
			/* lose the current template element and node, if any */
			REACCESS(FE_element)(&(element_creator->template_element),
			(struct FE_element *)NULL);
			if (region)
			{
				element_creator->fe_region = Cmiss_region_get_FE_region(region);
			}
			else
			{
				element_creator->fe_region = (struct FE_region *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_set_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_set_region */

static int Element_creator_refresh_element_dimension_text(
	struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Updates what is shown on the dimension text field.
==============================================================================*/
{
	char temp_string[20],*value_string;
	int return_code;
 
	ENTER(Element_creator_refresh_element_dimension_text);
	if (element_creator)
	{
		return_code=1;
		if (value_string=
			XmTextFieldGetString(element_creator->element_dimension_text))
		{
			sprintf(temp_string,"%d",element_creator->element_dimension);
			/* only set string if different from that shown */
			if (strcmp(temp_string,value_string))
			{
				XmTextFieldSetString(element_creator->element_dimension_text,
					temp_string);
			}
			XtFree(value_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_refresh_element_dimension_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_refresh_element_dimension_text */

static void Element_creator_create_button_CB(Widget widget,
	void *element_creator_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Callback from toggle button controlling whether elements are created in
response to node selection.
==============================================================================*/
{
	struct Element_creator *element_creator;

	ENTER(Element_creator_create_button_CB);
	USE_PARAMETER(call_data);
	if (element_creator=(struct Element_creator *)element_creator_void)
	{
		Element_creator_set_create_enabled(element_creator,
			XmToggleButtonGadgetGetState(widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_create_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_creator_create_button_CB */

static void Element_creator_update_region(Widget widget,
	void *element_creator_void,void *region_void)
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Callback for change of region to put the new elements in.
==============================================================================*/
{
	struct Cmiss_region *region;
	struct Element_creator *element_creator;

	ENTER(Element_creator_update_region);
	USE_PARAMETER(widget);
	if (element_creator=(struct Element_creator *)element_creator_void)
	{
		region = (struct Cmiss_region *)region_void;
		Element_creator_set_region(element_creator, region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_update_region.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_creator_update_region */

static void Element_creator_element_dimension_text_CB(Widget widget,
	void *element_creator_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Called when entry is made into the element_dimension_text field.
==============================================================================*/
{
	char *value_string;
	int element_dimension;
	struct Element_creator *element_creator;

	ENTER(Element_creator_element_dimension_text_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_creator=(struct Element_creator *)element_creator_void)
	{
		if (value_string=XmTextFieldGetString(widget))
		{
			if (1==sscanf(value_string,"%d",&element_dimension))
			{
				Element_creator_set_element_dimension(element_creator,
					element_dimension);
			}
			XtFree(value_string);
		}
		/* always restore element_dimension_text to actual value stored */
		Element_creator_refresh_element_dimension_text(element_creator);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_element_dimension_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_creator_element_dimension_text_CB */

static void Element_creator_update_coordinate_field(Widget widget,
	void *element_creator_void,void *coordinate_field_void)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Callback for change of coordinate field.
==============================================================================*/
{
	struct FE_field *coordinate_field;
	struct Element_creator *element_creator;

	ENTER(Element_creator_update_coordinate_field);
	USE_PARAMETER(widget);
	if (element_creator=(struct Element_creator *)element_creator_void)
	{
		coordinate_field=(struct FE_field *)coordinate_field_void;
		Element_creator_set_coordinate_field(element_creator,coordinate_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_update_coordinate_field.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_creator_update_coordinate_field */

static void Element_creator_abort_creation_CB(Widget widget,
	void *element_creator_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Called when abort element creation button is pressed.
==============================================================================*/
{
	struct Element_creator *element_creator;

	ENTER(Element_creator_abort_creation_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_creator=(struct Element_creator *)element_creator_void)
	{
		Element_creator_end_element_creation(element_creator);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_abort_creation_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_creator_abort_creation_CB */

static void Element_creator_close_CB(Widget widget,
	void *element_creator_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Callback when "close" is selected from the window menu, or it is double
clicked. How this is made to occur is as follows. The dialog has its
XmNdeleteResponse == XmDO_NOTHING, and a window manager protocol callback for
WM_DELETE_WINDOW has been set up with XmAddWMProtocolCallback to call this
function in response to the close command. See CREATE for more details.
==============================================================================*/
{
	struct Element_creator *element_creator;

	ENTER(Element_creator_close_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_creator=
		(struct Element_creator *)element_creator_void)
	{
		DESTROY(Element_creator)(
			element_creator->element_creator_address);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Element_creator_close_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_creator_close_CB */

/*
Global functions
----------------
*/

struct Element_creator *CREATE(Element_creator)(
	struct Element_creator **element_creator_address,
	struct Cmiss_region *root_region, char *initial_region_path,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 14 January 2003

DESCRIPTION :
Creates an Element_creator.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	int init_widgets;
	MrmType element_creator_dialog_class;
	static MrmRegisterArg callback_list[]=
	{
		{"elem_cre_id_create_btn",(XtPointer)
			DIALOG_IDENTIFY(element_creator,create_button)},
		{"elem_cre_id_region_form",(XtPointer)
			DIALOG_IDENTIFY(element_creator,region_form)},
		{"elem_cre_id_dimension_text",(XtPointer)
			DIALOG_IDENTIFY(element_creator,element_dimension_text)},
		{"elem_cre_id_coord_field_form",(XtPointer)
			DIALOG_IDENTIFY(element_creator,coordinate_field_form)},
		{"elem_cre_id_node_list",(XtPointer)
			DIALOG_IDENTIFY(element_creator,node_list_widget)},
		{"elem_cre_create_btn_CB",
		 (XtPointer)Element_creator_create_button_CB},
		{"elem_cre_dimension_text_CB",
		 (XtPointer)Element_creator_element_dimension_text_CB},
		{"elem_cre_abort_creation_CB",
		 (XtPointer)Element_creator_abort_creation_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"elem_cre_structure",(XtPointer)NULL}
	};
	struct Callback_data callback;
	struct Cmiss_region *region;
	struct Element_creator *element_creator;

	ENTER(CREATE(Element_creator));
	element_creator = (struct Element_creator *)NULL;
	if (element_creator_address && root_region && initial_region_path &&
		element_selection && node_selection && user_interface)
	{
		if (MrmOpenHierarchy_base64_string(element_creator_uidh,
			&element_creator_hierarchy, &element_creator_hierarchy_open))
		{
			if (ALLOCATE(element_creator, struct Element_creator, 1))
			{
				element_creator->element_creator_address = element_creator_address;
				element_creator->user_interface = user_interface;
				element_creator->coordinate_field = (struct FE_field *)NULL;
				element_creator->template_node = (struct FE_node *)NULL;
				/* by default create 2-D elements */
				element_creator->element_dimension = 2;
				element_creator->template_element = (struct FE_element *)NULL;
				element_creator->create_enabled = 0;

				element_creator->element = (struct FE_element *)NULL;
				element_creator->number_of_clicked_nodes = 0;

				element_creator->root_region = root_region;
				element_creator->region = (struct Cmiss_region *)NULL;
				element_creator->fe_region = (struct FE_region *)NULL;
				element_creator->element_selection = element_selection;
				element_creator->node_selection = node_selection;
				/* initialise widgets */
				element_creator->region_chooser = (struct Cmiss_region_chooser *)NULL;
				element_creator->create_button = (Widget)NULL;
				element_creator->region_form = (Widget)NULL;
				element_creator->element_dimension_text = (Widget)NULL;
				element_creator->coordinate_field_form = (Widget)NULL;
				element_creator->coordinate_field_widget = (Widget)NULL;
				element_creator->node_list_widget = (Widget)NULL;
				element_creator->widget = (Widget)NULL;
				element_creator->window_shell = (Widget)NULL;
				/* make the dialog shell */
				if (element_creator->window_shell =
					XtVaCreatePopupShell("Element Creator",
						topLevelShellWidgetClass,
						User_interface_get_application_shell(user_interface),
						XmNdeleteResponse, XmDO_NOTHING,
						XmNmwmDecorations, MWM_DECOR_ALL,
						XmNmwmFunctions, MWM_FUNC_ALL,
						/*XmNtransient, FALSE,*/
						XmNallowShellResize, False,
						XmNtitle, "Element Creator",
						NULL))
				{
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW =
						XmInternAtom(XtDisplay(element_creator->window_shell),
							"WM_DELETE_WINDOW", False);
					XmAddWMProtocolCallback(element_creator->window_shell,
						WM_DELETE_WINDOW, Element_creator_close_CB,
						element_creator);
					/* Register the shell with the busy signal list */
					create_Shell_list_item(&(element_creator->window_shell),
						user_interface);
					/* register the callbacks */
					if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
						element_creator_hierarchy,callback_list, XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value = (XtPointer)element_creator;
						if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
							element_creator_hierarchy, identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch element creator widgets */
							if (MrmSUCCESS == MrmFetchWidget(element_creator_hierarchy,
								"element_creator", element_creator->window_shell,
								&(element_creator->widget), &element_creator_dialog_class))
							{
								init_widgets = 1;
								if (element_creator->region_chooser =
									CREATE(Cmiss_region_chooser)(element_creator->region_form,
										root_region, initial_region_path))
								{
									if (Cmiss_region_chooser_get_region(
										element_creator->region_chooser, &region))
									{
										Element_creator_set_region(element_creator, region);
									}
									Cmiss_region_chooser_set_callback(
										element_creator->region_chooser,
										Element_creator_update_region, (void *)element_creator);
								}
								else
								{
									init_widgets = 0;
								}
								if (element_creator->coordinate_field_widget =
									CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET(FE_field)(
										element_creator->coordinate_field_form,
										element_creator->fe_region,
										element_creator->coordinate_field,
										FE_field_is_coordinate_field, (void *)NULL,
										user_interface))
								{
									element_creator->coordinate_field =
										FE_REGION_CHOOSE_OBJECT_GET_OBJECT(FE_field)(
											element_creator->coordinate_field_widget);
									callback.data = (void *)element_creator;
									callback.procedure = Element_creator_update_coordinate_field;
									FE_REGION_CHOOSE_OBJECT_SET_CALLBACK(FE_field)(
										element_creator->coordinate_field_widget, &callback);
								}
								else
								{
									init_widgets = 0;
								}
								if (init_widgets)
								{
									Element_creator_refresh_element_dimension_text(
										element_creator);
									XtManageChild(element_creator->widget);
									XtRealizeWidget(element_creator->window_shell);
									XtPopup(element_creator->window_shell,XtGrabNone);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"CREATE(Element_creator).  Could not init widgets");
									DESTROY(Element_creator)(&element_creator);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"CREATE(Element_creator).  "
									"Could not fetch element_creator");
								DESTROY(Element_creator)(&element_creator);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Element_creator).  Could not register identifiers");
							DESTROY(Element_creator)(&element_creator);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Element_creator).  Could not register callbacks");
						DESTROY(Element_creator)(&element_creator);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Element_creator).  Could not create Shell");
					DESTROY(Element_creator)(&element_creator);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Element_creator).  Not enough memory");
				DEALLOCATE(element_creator);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_creator).  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Element_creator).  Invalid argument(s)");
	}
	if (element_creator_address)
	{
		*element_creator_address=element_creator;
	}
	LEAVE;

	return (element_creator);
} /* CREATE(Element_creator) */

int DESTROY(Element_creator)(
	struct Element_creator **element_creator_address)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Deaccesses objects and frees memory used by the Element_creator at
<*element_creator_address>.
==============================================================================*/
{
	struct Element_creator *element_creator;
	int return_code;

	ENTER(DESTROY(Element_creator));
	if (element_creator_address&&
		(element_creator= *element_creator_address))
	{
		Element_creator_end_element_creation(element_creator);
		/* lose the current template element and node, if any */
		REACCESS(FE_element)(&(element_creator->template_element),
			(struct FE_element *)NULL);
		REACCESS(FE_node)(&(element_creator->template_node),
			(struct FE_node *)NULL);
		if (element_creator->create_enabled)
		{
			/* remove callbacks from the global node_selection */
			FE_node_selection_remove_callback(element_creator->node_selection,
				Element_creator_node_selection_change,
				(void *)element_creator);
		}
		if (element_creator->window_shell)
		{
			destroy_Shell_list_item_from_shell(
				&(element_creator->window_shell),
				element_creator->user_interface);
			XtDestroyWidget(element_creator->window_shell);
		}
		DEALLOCATE(*element_creator_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Element_creator).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Element_creator) */

struct FE_field *Element_creator_get_coordinate_field(
	struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Returns the coordinate field interpolated by elements created with
<element_creator>.
==============================================================================*/
{
	struct FE_field *coordinate_field;

	ENTER(Element_creator_get_coordinate_field);
	if (element_creator)
	{
		coordinate_field=element_creator->coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_get_coordinate_field.  Invalid argument(s)");
		coordinate_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (coordinate_field);
} /* Element_creator_get_coordinate_field */

int Element_creator_set_coordinate_field(
	struct Element_creator *element_creator,struct FE_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Sets the coordinate field interpolated by elements created with
<element_creator>.
==============================================================================*/
{
	int return_code;

	ENTER(Element_creator_set_coordinate_field);
	if (element_creator)
	{
		return_code=1;
		if (coordinate_field != element_creator->coordinate_field)
		{
			if (coordinate_field)
			{
				if ((3<get_FE_field_number_of_components(coordinate_field))||
					(FE_VALUE_VALUE != get_FE_field_value_type(coordinate_field)))
				{
					display_message(ERROR_MESSAGE,
						"Element_creator_set_coordinate_field.  "
						"Invalid number of components or value type");
					return_code=0;
				}
			}
			if (return_code)
			{
				element_creator->coordinate_field=coordinate_field;
				Element_creator_end_element_creation(element_creator);
				/* lose the current template element and node, if any */
				REACCESS(FE_element)(&(element_creator->template_element),
					(struct FE_element *)NULL);
				REACCESS(FE_node)(&(element_creator->template_node),
					(struct FE_node *)NULL);
				/* make sure the current field is shown on the widget */
				FE_REGION_CHOOSE_OBJECT_SET_OBJECT(FE_field)(
					element_creator->coordinate_field_widget,
					element_creator->coordinate_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_set_coordinate_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_set_coordinate_field */

int Element_creator_get_create_enabled(struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/
{
	int create_enabled;

	ENTER(Element_creator_get_create_enabled);
	if (element_creator)
	{
		create_enabled=element_creator->create_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_get_create_enabled.  Invalid argument(s)");
		create_enabled=0;
	}
	LEAVE;

	return (create_enabled);
} /* Element_creator_get_create_enabled */

int Element_creator_set_create_enabled(struct Element_creator *element_creator,
	int create_enabled)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Sets flag controlling whether elements are created in response to
node selection.
==============================================================================*/
{
	int button_state,return_code;

	ENTER(Element_creator_set_create_enabled);
	if (element_creator)
	{
		/* make sure value of flag is 1 */
		if (create_enabled)
		{
			create_enabled=1;
		}
		if (create_enabled != element_creator->create_enabled)
		{
			element_creator->create_enabled=create_enabled;
			if (create_enabled)
			{
				/* request callbacks from the global node_selection */
				FE_node_selection_add_callback(element_creator->node_selection,
					Element_creator_node_selection_change,
					(void *)element_creator);
			}
			else
			{
				Element_creator_end_element_creation(element_creator);
				/* end callbacks from the global node_selection */
				FE_node_selection_remove_callback(element_creator->node_selection,
					Element_creator_node_selection_change,
					(void *)element_creator);
			}
			/* make sure button shows current state */
			if (XmToggleButtonGadgetGetState(element_creator->create_button))
			{
				button_state=1;
			}
			else
			{
				button_state=0;
			}
			if (button_state != element_creator->create_enabled)
			{
				XmToggleButtonGadgetSetState(element_creator->create_button,
					/*state*/element_creator->create_enabled,/*notify*/False);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_set_create_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_set_create_enabled */

int Element_creator_get_element_dimension(
	struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 21 October 1999

DESCRIPTION :
Returns the dimension of elements to be created by the <element_creator>.
==============================================================================*/
{
	int element_dimension;

	ENTER(Element_creator_get_element_dimension);
	if (element_creator)
	{
		element_dimension=element_creator->element_dimension;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_get_element_dimension.  Invalid argument(s)");
		element_dimension=0;
	}
	LEAVE;

	return (element_dimension);
} /* Element_creator_get_element_dimension */

int Element_creator_set_element_dimension(
	struct Element_creator *element_creator,int element_dimension)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Sets the <element_dimension> of elements to be created by <element_creator>.
==============================================================================*/
{
	int return_code;

	ENTER(Element_creator_set_element_dimension);
	if (element_creator)
	{
		if ((0<element_dimension)&&(element_dimension<=3))
		{
			return_code=1;
			if (element_creator->element_dimension != element_dimension)
			{
				element_creator->element_dimension=element_dimension;
				Element_creator_end_element_creation(element_creator);
				/* lose the current template element and node, if any */
				REACCESS(FE_element)(&(element_creator->template_element),
					(struct FE_element *)NULL);
				REACCESS(FE_node)(&(element_creator->template_node),
					(struct FE_node *)NULL);
			}
			Element_creator_refresh_element_dimension_text(element_creator);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Dimension must be from 1 to 3");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_set_element_dimension.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_set_element_dimension */

int Element_creator_get_region_path(struct Element_creator *element_creator,
	char **path_address)
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Returns in <path_address> the path to the Cmiss_region where elements created by
the <element_creator> are put.
Up to the calling function to DEALLOCATE the returned path.
==============================================================================*/
{
	int return_code;

	ENTER(Element_creator_get_region_path);
	if (element_creator && path_address)
	{
		return_code = Cmiss_region_chooser_get_path(element_creator->region_chooser,
			path_address);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_get_region_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_get_region_path */

int Element_creator_set_region_path(struct Element_creator *element_creator,
	char *path)
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Sets the <path> to the region/FE_region where elements created by
<element_creator> are placed.
The <element_creator> assumes its nodes are to come from the same FE_region.
==============================================================================*/
{
	int return_code;
	struct Cmiss_region *region;

	ENTER(Element_creator_set_region_path);
	if (element_creator && path)
	{
		Cmiss_region_chooser_set_path(element_creator->region_chooser, path);
		region = (struct Cmiss_region *)NULL;
		Cmiss_region_chooser_get_region(element_creator->region_chooser, &region);
		return_code = Element_creator_set_region(element_creator, region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_set_region_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_set_region_path */

int Element_creator_bring_window_to_front(
	struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Pops the window for <element_creator> to the front of those visible.
??? De-iconify as well?
==============================================================================*/
{
	int return_code;

	ENTER(Element_creator_bring_window_to_front);
	if (element_creator)
	{
		XtPopup(element_creator->window_shell,XtGrabNone);
		XtVaSetValues(element_creator->window_shell,XmNiconic,False,NULL);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_bring_window_to_front.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_bring_window_to_front */

