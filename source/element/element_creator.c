/*******************************************************************************
FILE : element_creator.c

LAST MODIFIED : 9 May 2000

DESCRIPTION :
Dialog for choosing the type of element constructed in response to node
selections. Elements are created in this way while dialog is open.
==============================================================================*/
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include "general/debug.h"
#include "element/element_creator.h"
#if defined (TEMPORARY)
#include "element/element_creator.uidh"
#endif /* defined (TEMPORARY) */
#include "finite_element/finite_element.h"
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

enum Mesh_editor_create_mode
/*******************************************************************************
LAST MODIFIED : 1 May 2000

DESCRIPTION :
Kill this.
==============================================================================*/
{
	MESH_EDITOR_CREATE_ELEMENTS,
	MESH_EDITOR_CREATE_ONLY_NODES,
	MESH_EDITOR_CREATE_NODES_AND_ELEMENTS
};

struct Element_creator
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
==============================================================================*/
{
	struct Element_creator **element_creator_address;

	char *tool_title;
	enum Mesh_editor_create_mode mesh_editor_create_mode;

	struct FE_field *coordinate_field;

	/* maintain a template node for creating new nodes */
	struct FE_node *template_node;

	/* the dimension of the elements being created - user settable */
	int element_dimension;

	/* shape - incl. dimension - of elements being created */
	struct FE_element_shape *element_shape;
	/* basis type of elements being created */
	struct FE_basis *element_basis;
	/* maintain template element for creating new elements */
	struct FE_element *template_element;
	struct User_interface *user_interface;

	/* the element being created */
	struct FE_element *element;
	/* number of nodes that have been set in the element being created */
	int number_of_clicked_nodes;

	/* the element and node groups new objects are put in */
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *node_group;
	/* node and element managers */
	struct MANAGER(FE_basis) *basis_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *node_selection;

	Widget widget,window_shell;
}; /* struct Element_creator */

/*
Module functions
----------------
*/

int Element_creator_make_template_node(
	struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
==============================================================================*/
{
	char *component_names[3]=
	{
		"x","y","z"
	};
	enum FE_nodal_value_type *components_nodal_value_types[3]=
	{
		{FE_NODAL_VALUE},
		{FE_NODAL_VALUE},
		{FE_NODAL_VALUE}
	};
	int components_number_of_derivatives[3]={0,0,0},
		components_number_of_versions[3]={1,1,1},return_code;
	struct CM_field_information field_info;
	struct Coordinate_system rect_cart_coords;

	ENTER(Element_creator_make_template_node);
	if (element_creator)
	{
		return_code=1;
		/* default coordinate field is called "coordinates" with 3 coordinates
			 named x, y and z, ie. RC coordinate system */
		if (!element_creator->coordinate_field)
		{
			set_CM_field_information(&field_info,CM_COORDINATE_FIELD,(int *)NULL);
			rect_cart_coords.type=RECTANGULAR_CARTESIAN;
			if (!(element_creator->coordinate_field=
				get_FE_field_manager_matched_field(
					element_creator->fe_field_manager,"coordinates",
					GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
					/*number_of_indexed_values*/0,&field_info,
					&rect_cart_coords,FE_VALUE_VALUE,
					/*number_of_components*/3,component_names,
					/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE)))
			{
				return_code=0;
			}
		}
		/* make template_node */
		if (return_code&&!element_creator->template_node)
		{
			if (element_creator->template_node=CREATE(FE_node)(
				/*node_number*/0,(struct FE_node *)NULL))
			{
				/* template_node is accessed by element_creator but not placed
					 in manager */
				ACCESS(FE_node)(element_creator->template_node);
				if (!define_FE_field_at_node(element_creator->template_node,
					element_creator->coordinate_field,
					components_number_of_derivatives,
					components_number_of_versions,
					components_nodal_value_types))
				{
					DEACCESS(FE_node)(&(element_creator->template_node));
					return_code=0;
				}
			}
			else
			{
				return_code=0;
			}
		}			
		if (0==return_code)
		{
			display_message(ERROR_MESSAGE,
				"Element_creator_make_template_node.  Failed");
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

int Element_creator_make_template_element(
	struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
This function checks if <element_creator> has a coordinate_field, element shape,
basis, field_info and node_scale_field_info. If not, it creates them for
generating 2-D bilinear elements.
==============================================================================*/
{
	int *basis_type,element_dimension,i,j,number_of_components,
		number_of_nodes,return_code,*type,*type_entry,*xi_basis_type;
	struct CM_element_information element_identifier;
	struct FE_element_field_component *component,**components;
	struct Standard_node_to_element_map **standard_node_map;

	ENTER(Element_creator_make_template_element);
	if (element_creator)
	{
		/* clear up the existing template information if dimension changed */
		if (element_creator->element_shape)
		{
			if (element_creator->element_shape->dimension
				!= element_creator->element_dimension)
			{
				if (element_creator->template_element)
				{
					DEACCESS(FE_element)(&(element_creator->template_element));
				}
				if (element_creator->element_basis)
				{
					DEACCESS(FE_basis)(&(element_creator->element_basis));
				}
				if (element_creator->element_shape)
				{
					DEACCESS(FE_element_shape)(&(element_creator->element_shape));
				}
			}
		}
		element_dimension=element_creator->element_dimension;

		return_code=1;
		if (!element_creator->template_node)
		{
			return_code=Element_creator_make_template_node(element_creator);
		}

		if (return_code&&!(element_creator->element_shape))
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
				if (!(element_creator->element_shape=
					CREATE(FE_element_shape)(element_dimension,type)))
				{
					display_message(ERROR_MESSAGE,
						"Element_creator_make_template_element.  "
						"Error creating shape");
					return_code=0;
				}
				DEALLOCATE(type);
			}
			else
			{
				return_code=0;
			}
			if (element_creator->element_shape)
			{
				ACCESS(FE_element_shape)(element_creator->element_shape);
			}
		}
		if (return_code&&(!element_creator->element_basis))
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

				if(!(element_creator->element_basis=make_FE_basis(basis_type,
					element_creator->basis_manager)))
				{
					return_code=0;
				}
				DEALLOCATE(basis_type);
			}
			if (element_creator->element_basis)
			{
				ACCESS(FE_basis)(element_creator->element_basis);
			}
		}
		if (return_code&&(!element_creator->template_element))
		{
			element_identifier.type=CM_ELEMENT;
			element_identifier.number=0;
			if (element_creator->template_element=CREATE(FE_element)(
				&element_identifier,(struct FE_element *)NULL))
			{
				number_of_nodes=1;
				for (i=0;i<element_dimension;i++)
				{
					number_of_nodes *= 2;
				}
				if (set_FE_element_shape(element_creator->template_element,
					element_creator->element_shape)&&
					set_FE_element_node_scale_field_info(
						element_creator->template_element,
						/*number_of_scale_factor_sets*/1,
						/*scale_factor_set_identifiers*/
						(void *)&(element_creator->element_basis),
						/*numbers_in_scale_factor_sets*/&number_of_nodes,number_of_nodes))
				{
					number_of_components=get_FE_field_number_of_components(
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
								element_creator->element_basis,
								(FE_element_field_component_modify)NULL))
							{
								standard_node_map=
									component->map.standard_node_based.node_to_element_maps;
								for (j=0;j<number_of_nodes;j++)
								{
									if (*standard_node_map=
										CREATE(Standard_node_to_element_map)(
											/*node_index*/j,/*number_of_values*/1))
									{
										(*standard_node_map)->nodal_value_indices[0]=0;
										(*standard_node_map)->scale_factor_indices[0]=j;
										/* set scale_factors to 1 */
										element_creator->template_element->information->
											scale_factors[(*standard_node_map)->
												scale_factor_indices[0]]=1.0;
									}
									else
									{
										return_code=0;
									}
									standard_node_map++;
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
							if (define_FE_field_at_element(element_creator->template_element,
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
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_creator_make_template_element.  "
						"Could not set element shape and field info");
					DESTROY(FE_element)(&(element_creator->template_element));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_creator_make_template_element.  "
					"Could not create template element");
			}
		}			
		if (0==return_code)
		{
			display_message(ERROR_MESSAGE,
				"Element_creator_make_template_element.  Failed");
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

static int Element_creator_add_element(
	struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 12 October 1999

DESCRIPTION :
Adds the just created element to the manager and group, adding faces to these
as necessary.
Note: It is up to the calling function to call
Element_creator_release_input.
==============================================================================*/
{
	int return_code;
	struct Add_FE_element_and_faces_to_manager_data *add_element_data;

	ENTER(Element_creator_add_element);
	if (element_creator&&element_creator->element&&
		(element_creator->number_of_clicked_nodes ==
			element_creator->element->information->number_of_nodes))
	{
		/* create user_data for add_FE_element_and_faces_to_manager, which
			 helps efficiently find existing faces to fit new element. Don't
			 forget to destroy it afterwards as it can be huge! */
		if (add_element_data=CREATE(Add_FE_element_and_faces_to_manager_data)(
			element_creator->element_manager))
		{
			MANAGER_BEGIN_CACHE(FE_element)(element_creator->element_manager);
			if (element_creator->element_group)
			{
				MANAGED_GROUP_BEGIN_CACHE(FE_element)(
					element_creator->element_group);
			}
			if (add_FE_element_and_faces_to_manager(element_creator->element,
				(void *)add_element_data)&&((!element_creator->element_group)||
					add_FE_element_and_faces_to_group(element_creator->element,
						element_creator->element_group)))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_creator_add_element.  "
					"Error adding element to element_manager or element_group");
				if (FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
					element_creator->element->identifier,
					element_creator->element_manager))
				{
					if (element_creator->element_group&&
						FIND_BY_IDENTIFIER_IN_GROUP(FE_element,identifier)(
							element_creator->element->identifier,
							element_creator->element_group))
					{
						remove_FE_element_and_faces_from_group(element_creator->element,
							element_creator->element_group);
					}
					remove_FE_element_and_faces_from_manager(element_creator->element,
						element_creator->element_manager);
				}
				return_code=0;
			}
			if (element_creator->element_group)
			{
				MANAGED_GROUP_END_CACHE(FE_element)(element_creator->element_group);
			}
			MANAGER_END_CACHE(FE_element)(element_creator->element_manager);
			/* Destroy add_element_data without fail - it can be huge! */
			DESTROY(Add_FE_element_and_faces_to_manager_data)(&add_element_data);
		}
		DEACCESS(FE_element)(&(element_creator->element));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_add_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_add_element */

static void Element_creator_node_selection_change(
	struct FE_node_selection *node_selection,
	struct FE_node_selection_changes *changes,void *element_creator_void)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Callback for change in the global node selection.
==============================================================================*/
{
	struct CM_element_information element_identifier;
	struct FE_node *node;
	struct Element_creator *element_creator;

	ENTER(Element_creator_node_selection_change);
	if (node_selection&&changes&&(element_creator=
		(struct Element_creator *)element_creator_void))
	{
		/* if exactly 1 node selected, add it to element */
		if (1==NUMBER_IN_LIST(FE_node)(changes->newly_selected_node_list))
		{
			if ((MESH_EDITOR_CREATE_ELEMENTS ==
				element_creator->mesh_editor_create_mode) ||
				(MESH_EDITOR_CREATE_NODES_AND_ELEMENTS ==
					element_creator->mesh_editor_create_mode))
			{
				node=FIRST_OBJECT_IN_LIST_THAT(FE_node)(
					(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
					changes->newly_selected_node_list);
				if (IS_OBJECT_IN_GROUP(FE_node)(node,element_creator->node_group))
				{
					if (!element_creator->element)
					{
						element_identifier.type=CM_ELEMENT;
						element_identifier.number=1;
						while (element_creator->element=
							FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
								&element_identifier,element_creator->element_manager))
						{
							element_identifier.number++;
						}
						if (Element_creator_make_template_element(
							element_creator)&&
							(element_creator->element=CREATE(FE_element)(&element_identifier,
								element_creator->template_element)))
						{
							ACCESS(FE_element)(element_creator->element);
							element_creator->number_of_clicked_nodes=0;
						}
					}
					if (element_creator->element)
					{
						if (set_FE_element_node(element_creator->element,
							element_creator->number_of_clicked_nodes,node))
						{
							element_creator->number_of_clicked_nodes++;
							if (element_creator->number_of_clicked_nodes ==
								element_creator->element->information->number_of_nodes)
							{
								Element_creator_add_element(element_creator);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Element_creator_scene_input_callback.  "
								"Could not set node");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Element_creator_scene_input_callback.  "
							"Could not create element");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element creator: Selected node not from current group");
				}
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
	struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Creates a Element_creator, giving it the element_manager to put new
elements in, and the node_manager for new nodes, and the fe_field_manager to
enable the creation of a coordinate field.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	struct Element_creator *element_creator;

	ENTER(CREATE(Element_creator));
	if (element_creator_address&&basis_manager&&element_manager&&
		fe_field_manager&&node_manager&&element_selection&&node_selection&&
		user_interface)
	{
		if (ALLOCATE(element_creator,struct Element_creator,1))
		{
			element_creator->element_creator_address=
				element_creator_address;
			element_creator->mesh_editor_create_mode=
				MESH_EDITOR_CREATE_NODES_AND_ELEMENTS;
			element_creator->user_interface=user_interface;
			element_creator->coordinate_field=(struct FE_field *)NULL;
			element_creator->template_node=(struct FE_node *)NULL;
			/* by default create 2-D elements */
			element_creator->element_dimension=2;
			element_creator->element_shape=(struct FE_element_shape *)NULL;
			element_creator->element_basis=(struct FE_basis *)NULL;
			element_creator->template_element=(struct FE_element *)NULL;

			element_creator->element=(struct FE_element *)NULL;
			element_creator->number_of_clicked_nodes=0;

			element_creator->element_group=(struct GROUP(FE_element) *)NULL;
			element_creator->node_group=(struct GROUP(FE_node) *)NULL;
			element_creator->basis_manager=basis_manager;
			element_creator->element_manager=element_manager;
			element_creator->fe_field_manager=fe_field_manager;
			element_creator->node_manager=node_manager;
			element_creator->element_selection=element_selection;
			element_creator->node_selection=node_selection;
			/* initialise widgets */
			element_creator->widget=(Widget)NULL;
			element_creator->window_shell=(Widget)NULL;
			/* make the dialog shell */
			if (element_creator->window_shell=
				XtVaCreatePopupShell("Element Creator",
					topLevelShellWidgetClass,
					user_interface->application_shell,
					XmNdeleteResponse,XmDO_NOTHING,
					XmNmwmDecorations,MWM_DECOR_ALL,
					XmNmwmFunctions,MWM_FUNC_ALL,
					/*XmNtransient,FALSE,*/
					XmNallowShellResize,False,
					XmNtitle,"Element Creator",
					NULL))
			{
				/* Set up window manager callback for close window message */
				WM_DELETE_WINDOW=
					XmInternAtom(XtDisplay(element_creator->window_shell),
						"WM_DELETE_WINDOW",False);
				XmAddWMProtocolCallback(element_creator->window_shell,
					WM_DELETE_WINDOW,Element_creator_close_CB,
					element_creator);
				/* Register the shell with the busy signal list */
				create_Shell_list_item(&(element_creator->window_shell),
					user_interface);
				/* request callbacks from the global node_selection */
				FE_node_selection_add_callback(node_selection,
					Element_creator_node_selection_change,
					(void *)element_creator);
				XtRealizeWidget(element_creator->window_shell);
				XtPopup(element_creator->window_shell,XtGrabNone);
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
			"CREATE(Element_creator).  Invalid argument(s)");
		element_creator=(struct Element_creator *)NULL;
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
LAST MODIFIED : 9 May 2000

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
		if (element_creator->coordinate_field)
		{
			DEACCESS(FE_field)(&(element_creator->coordinate_field));
		}
		if (element_creator->template_node)
		{
			DEACCESS(FE_node)(&(element_creator->template_node));
		}
		if (element_creator->element_shape)
		{
			DEACCESS(FE_element_shape)(&(element_creator->element_shape));
		}
		if (element_creator->element_basis)
		{
			DEACCESS(FE_basis)(&(element_creator->element_basis));
		}
		if (element_creator->template_element)
		{
			DEACCESS(FE_element)(&(element_creator->template_element));
		}
		if (element_creator->element_group)
		{
			DEACCESS(GROUP(FE_element))(&(element_creator->element_group));
		}
		if (element_creator->node_group)
		{
			DEACCESS(GROUP(FE_node))(&(element_creator->node_group));
		}
		/* remove callbacks from the global node_selection */
		FE_node_selection_remove_callback(element_creator->node_selection,
			Element_creator_node_selection_change,
			(void *)element_creator);
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

struct FE_field *Element_creator_get_coordinate_field(struct Element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the coordinate field of nodes created by <element_creator>.
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

#if defined (NEW_CODE)
int Element_creator_set_coordinate_field(struct Element_creator *element_creator,
	struct FE_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the coordinate field of nodes created by <element_creator>.
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
						"Element_creator_set_coordinate_field.  Invalid argument(s)");
					return_code=0;
				}
			}
			if (return_code)
			{
				REACCESS(FE_field)(&(element_creator->coordinate_field),coordinate_field);
				/* lose the current template element, if any */
				REACCESS(FE_node)(&(element_creator->template_node),
					(struct FE_node *)NULL);
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
#endif /* defined (NEW_CODE) */

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
LAST MODIFIED : 21 October 1999

DESCRIPTION :
Sets the <element_dimension> of elements to be created by <element_creator>.
==============================================================================*/
{
	int return_code;

	ENTER(Element_creator_set_element_dimension);
	if (element_creator&&(0<element_dimension)&&(element_dimension<=3))
	{
		if (element_creator->element_dimension != element_dimension)
		{
			/*???RC not really essential to disallow this while creating elements */
			if (0==element_creator->number_of_clicked_nodes)
			{
				element_creator->element_dimension=element_dimension;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_creator_set_element_dimension.  "
					"May not change dimension while element is being made");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
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

int Element_creator_get_groups(
	struct Element_creator *element_creator,
	struct GROUP(FE_element) **element_group,struct GROUP(FE_node) **node_group)
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Returns the current <element_group> and <node_group> where elements and nodes
created by the <element_creator> are placed.
???RC Eventually this will get the Region.
==============================================================================*/
{
	int return_code;

	ENTER(Element_creator_get_groups);
	if (element_creator&&element_group&&node_group)
	{
		*element_group=element_creator->element_group;
		*node_group=element_creator->node_group;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_get_groups.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_get_groups */

int Element_creator_set_groups(
	struct Element_creator *element_creator,
	struct GROUP(FE_element) *element_group,struct GROUP(FE_node) *node_group)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Sets the <element_group> and <node_group> where elements and nodes created by
<element_creator> are placed. Either or both groups may be omitted, however,
it pays to supply a node_group and element_group of the same name with a
corresponding GT_element_group displaying node_points, lines and possibly
surfaces - that way the user will automatically see the new objects. Also, it
enables the user to export the nodes/elements as a group.
???RC Eventually this will set the Region.
==============================================================================*/
{
	int return_code;

	ENTER(Element_creator_set_groups);
	if (element_creator)
	{
		REACCESS(GROUP(FE_element))(&(element_creator->element_group),
			element_group);
		REACCESS(GROUP(FE_node))(&(element_creator->node_group),node_group);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_set_groups.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_set_groups */

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

