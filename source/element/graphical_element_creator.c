/*******************************************************************************
FILE : graphical_element_creator.c

LAST MODIFIED : 20 January 2000

DESCRIPTION :
Mouse controlled element creator/creator.
==============================================================================*/
/*#include <math.h>*/
#include "general/debug.h"
/*#include "general/geometry.h"*/
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "element/graphical_element_creator.h"
#include "finite_element/finite_element.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
struct Graphical_element_creator
/*******************************************************************************
LAST MODIFIED : 21 October 1999

DESCRIPTION :
==============================================================================*/
{
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
	/* scene only accessed while element_creator gets scene input callbacks */
	struct Scene *scene;
	/* put current scene input callback in stored_input_callback while
		 element_creator gets scene input callbacks - restore once element made */
	struct Scene_input_callback stored_input_callback;
}; /* struct Graphical_element_creator */

/*
Module functions
----------------
*/

char *Mesh_editor_create_mode_string(
	enum Mesh_editor_create_mode mesh_editor_create_mode)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Returns a pointer to a static string describing the mesh_editor_create_mode, eg.
MESH_EDITOR_CREATE_ELEMENTS = "create_elements".
This string should match the command used to create that mode.
The returned string must not be DEALLOCATEd!
==============================================================================*/
{
	char *return_string;

	ENTER(Mesh_editor_create_mode_string);
	switch (mesh_editor_create_mode)
	{
		case MESH_EDITOR_CREATE_ELEMENTS:
		{
			return_string="create_elements";
		} break;
		case MESH_EDITOR_CREATE_ONLY_NODES:
		{
			return_string="create_only_nodes";
		} break;
		case MESH_EDITOR_CREATE_NODES_AND_ELEMENTS:
		{
			return_string="create_nodes_and_elements";
		} break;
		case MESH_EDITOR_NO_CREATE:
		{
			return_string="no_create";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Mesh_editor_create_mode_string.  Unknown mode");
			return_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (return_string);
} /* Mesh_editor_create_mode_string */

char **Mesh_editor_create_mode_get_valid_strings(int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Mesh_editor_create_modes - obtained from function
Mesh_editor_create_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;
	enum Mesh_editor_create_mode mesh_editor_create_mode;
	int i;

	ENTER(Mesh_editor_create_mode_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=0;
		mesh_editor_create_mode=MESH_EDITOR_CREATE_MODE_BEFORE_FIRST;
		mesh_editor_create_mode++;
		while (mesh_editor_create_mode<MESH_EDITOR_CREATE_MODE_AFTER_LAST)
		{
			(*number_of_valid_strings)++;
			mesh_editor_create_mode++;
		}
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			mesh_editor_create_mode=MESH_EDITOR_CREATE_MODE_BEFORE_FIRST;
			mesh_editor_create_mode++;
			i=0;
			while (mesh_editor_create_mode<MESH_EDITOR_CREATE_MODE_AFTER_LAST)
			{
				valid_strings[i]=
					Mesh_editor_create_mode_string(mesh_editor_create_mode);
				i++;
				mesh_editor_create_mode++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Mesh_editor_create_mode_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mesh_editor_create_mode_get_valid_strings.  Invalid argument");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Mesh_editor_create_mode_get_valid_strings */

enum Mesh_editor_create_mode Mesh_editor_create_mode_from_string(
	char *mesh_editor_create_mode_string)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Returns the <Mesh_editor_create_mode> described by
<mesh_editor_create_mode_string>.
==============================================================================*/
{
	enum Mesh_editor_create_mode mesh_editor_create_mode;

	ENTER(Mesh_editor_create_mode_from_string);
	if (mesh_editor_create_mode_string)
	{
		mesh_editor_create_mode=MESH_EDITOR_CREATE_MODE_BEFORE_FIRST;
		mesh_editor_create_mode++;
		while ((mesh_editor_create_mode<MESH_EDITOR_CREATE_MODE_AFTER_LAST)&&
			(!fuzzy_string_compare_same_length(mesh_editor_create_mode_string,
				Mesh_editor_create_mode_string(mesh_editor_create_mode))))
		{
			mesh_editor_create_mode++;
		}
		if (MESH_EDITOR_CREATE_MODE_AFTER_LAST==mesh_editor_create_mode)
		{
			mesh_editor_create_mode=MESH_EDITOR_CREATE_MODE_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mesh_editor_create_mode_from_string.  Invalid argument");
		mesh_editor_create_mode=MESH_EDITOR_CREATE_MODE_INVALID;
	}
	LEAVE;

	return (mesh_editor_create_mode);
} /* Mesh_editor_create_mode_from_string */

int Graphical_element_creator_make_template_node(
	struct Graphical_element_creator *element_creator)
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

	ENTER(Graphical_element_creator_make_template_node);
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
				"Graphical_element_creator_make_template_node.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_creator_make_template_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_make_template_node */

int Graphical_element_creator_make_template_element(
	struct Graphical_element_creator *element_creator)
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

	ENTER(Graphical_element_creator_make_template_element);
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
			return_code=Graphical_element_creator_make_template_node(element_creator);
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
						"Graphical_element_creator_make_template_element.  "
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
									"Graphical_element_creator_make_template_element.  "
									"Could not define coordinate field at template_element");
								DESTROY(FE_element)(&(element_creator->template_element));
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Graphical_element_creator_make_template_element.  "
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
							"Graphical_element_creator_make_template_element.  "
							"Could not allocate components");
						DESTROY(FE_element)(&(element_creator->template_element));
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Graphical_element_creator_make_template_element.  "
						"Could not set element shape and field info");
					DESTROY(FE_element)(&(element_creator->template_element));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Graphical_element_creator_make_template_element.  "
					"Could not create template element");
			}
		}			
		if (0==return_code)
		{
			display_message(ERROR_MESSAGE,
				"Graphical_element_creator_make_template_element.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_creator_make_template_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_make_template_element */

struct Scene_picked_object_get_nearest_picked_node_data
{
	struct FE_node *picked_node;
	struct MANAGER(FE_node) *node_manager;
	/* "nearest" value from Scene_picked_object for picked_node */
	unsigned int nearest;
};

static int Scene_picked_object_get_nearest_picked_node(
	struct Scene_picked_object *picked_object,void *picked_node_data_void)
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
If the <picked_object> refers to a node, the "nearest" value is compared with
that for the currently picked_node in the <picked_node_data>. I there was no
currently picked_node or the new node is nearer, it becomes the picked node and
its "nearest" value is stored in the picked_node_data.
==============================================================================*/
{
	int node_number,return_code;
	struct FE_node *picked_node;
	struct Scene_object *scene_object;
	struct Scene_picked_object_get_nearest_picked_node_data *picked_node_data;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;
	unsigned int nearest;
 
	ENTER(Scene_picked_object_get_nearest_picked_node);
	if (picked_object&&(picked_node_data=
		(struct Scene_picked_object_get_nearest_picked_node_data *)
		picked_node_data_void))
	{
		return_code=1;
		/* is the last scene_object a Graphical_element wrapper, and does the
			 settings for the graphic refer to node_glyphs? */
		if ((scene_object=Scene_picked_object_get_Scene_object(picked_object,
			Scene_picked_object_get_number_of_scene_objects(picked_object)-1))&&
			(SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP==
				Scene_object_get_type(scene_object))&&(gt_element_group=
					Scene_object_get_graphical_element_group(scene_object))&&
			(3==Scene_picked_object_get_number_of_subobjects(picked_object))&&
			(settings=get_settings_at_position_in_GT_element_group(gt_element_group,
				Scene_picked_object_get_subobject(picked_object,0)))&&
			(GT_ELEMENT_SETTINGS_NODE_POINTS==
				GT_element_settings_get_settings_type(settings)))
		{
			node_number=Scene_picked_object_get_subobject(picked_object,2);
			if (picked_node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
				node_number,picked_node_data->node_manager))
			{
				nearest=Scene_picked_object_get_nearest(picked_object);
				if (((struct FE_node *)NULL==picked_node_data->picked_node)||
					(nearest < picked_node_data->nearest))
				{
					picked_node_data->picked_node = picked_node;
					picked_node_data->nearest = nearest;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_picked_object_get_nearest_picked_node.  "
					"Node number %d not in manager",node_number);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_nearest_picked_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_get_nearest_picked_node */

static int Graphical_element_creator_add_element(
	struct Graphical_element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 12 October 1999

DESCRIPTION :
Adds the just created element to the manager and group, adding faces to these
as necessary.
Note: It is up to the calling function to call
Graphical_element_creator_release_input.
==============================================================================*/
{
	int return_code;
	struct Add_FE_element_and_faces_to_manager_data *add_element_data;

	ENTER(Graphical_element_creator_add_element);
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
					"Graphical_element_creator_add_element.  "
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
			"Graphical_element_creator_add_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_add_element */

static void Graphical_element_creator_scene_input_callback(struct Scene *scene,
	void *graphical_element_creator_void,
	struct Scene_input_callback_data *scene_input_callback_data)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Receives mouse button press, motion and release events from <scene>, and
creates nodes and elements from the entered positions.
==============================================================================*/
{
	struct CM_element_information element_identifier;
	FE_value node_coordinates[3];
	int i,node_number,return_code;
	struct FE_field_component field_component;
	struct FE_node *node;
	struct Graphical_element_creator *element_creator;
	struct Scene_picked_object_get_nearest_picked_node_data picked_node_data;

	ENTER(Graphical_element_creator_scene_input_callback);
	if (scene&&(element_creator=(struct Graphical_element_creator *)
		graphical_element_creator_void)&&scene_input_callback_data)
	{
		switch (scene_input_callback_data->input_type)
		{
			case SCENE_BUTTON_PRESS:
			{
				return_code=
					Graphical_element_creator_make_template_node(element_creator);
				if (return_code)
				{
					/*???RC must create node on button press because currently only
						perform picking then */
#if defined (DEBUG)
					/*???debug */
					printf("Graphical_element_creator: button press!\n");
					/*???debug end */
#endif /* defined (DEBUG) */
					/* get picked node nearest to the near plane, if any */
					picked_node_data.picked_node=(struct FE_node *)NULL;
					picked_node_data.node_manager=element_creator->node_manager;
					FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
						Scene_picked_object_get_nearest_picked_node,
						(void *)&picked_node_data,
						scene_input_callback_data->picked_object_list);
					if (!(node=picked_node_data.picked_node))
					{
						if (MESH_EDITOR_CREATE_ELEMENTS !=
							element_creator->mesh_editor_create_mode)
						{
							/* create a new node, based on the template node */
							if (element_creator->coordinate_field)
							{
								node_coordinates[0]=0.5*(scene_input_callback_data->nearx+
									scene_input_callback_data->farx);
								node_coordinates[1]=0.5*(scene_input_callback_data->neary+
									scene_input_callback_data->fary);
								node_coordinates[2]=0.5*(scene_input_callback_data->nearz+
									scene_input_callback_data->farz);
								/*???RC get_next_FE_node_number is pretty inefficient */
								node_number=
									get_next_FE_node_number(element_creator->node_manager,1);
								if (node=CREATE(FE_node)(node_number,
									element_creator->template_node))
								{
									/* fill the field with coordinates half way between point on
										 near and far plane */
									field_component.field=element_creator->coordinate_field;
									for (i=0;i</*coordinate_dimension*/3;i++)
									{
										field_component.number=i;								
										if (!set_FE_nodal_FE_value_value(node,&field_component,
											/*version*/0,FE_NODAL_VALUE,node_coordinates[i]))
										{
											return_code=0;
										}
									}
									/* add new node to manager and node_group */
									if (!(return_code&&ADD_OBJECT_TO_MANAGER(FE_node)(node,
										element_creator->node_manager)&&
										((!element_creator->node_group)||
											ADD_OBJECT_TO_GROUP(FE_node)(
												node,element_creator->node_group))))
									{
										REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,
											element_creator->node_manager);
										DESTROY(FE_node)(&node);
									}
								}
							}
							if (!node)
							{
								display_message(ERROR_MESSAGE,
									"Graphical_element_creator_scene_input_callback.  "
									"Could not create node");
								return_code=0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"Make element: You did not click on a node");
						}
					}
					if (node&&(MESH_EDITOR_CREATE_ONLY_NODES !=
						element_creator->mesh_editor_create_mode))
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
							if (Graphical_element_creator_make_template_element(
								element_creator)&&(element_creator->element=CREATE(FE_element)(
								&element_identifier,element_creator->template_element)))
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
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Graphical_element_creator_scene_input_callback.  "
									"Could not set node");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Graphical_element_creator_scene_input_callback.  "
								"Could not create element");
							return_code=0;
						}
					}
					if (!return_code)
					{
						display_message(WARNING_MESSAGE,
							"Graphical_element_creator_scene_input_callback.  "
							"Aborting element creation");
						DEACCESS(FE_element)(&(element_creator->element));
					}
				}
			} break;
			case SCENE_MOTION_NOTIFY:
			{
#if defined (DEBUG)
				/*???debug */
				/*printf("Graphical_element_creator: motion notify!\n");*/
				/*???debug end */
#endif /* defined (DEBUG) */
			} break;
			case SCENE_BUTTON_RELEASE:
			{
#if defined (DEBUG)
				/*???debug */
				printf("Graphical_element_creator: button release!\n");
				/*???debug end */
#endif /* defined (DEBUG) */
				/*???RC must call Graphical_element_creator_release_input on
					button release only - otherwise the button release will go to
					the graphical node editor! */
				if (element_creator->element&&
					(element_creator->number_of_clicked_nodes ==
						element_creator->element->information->number_of_nodes))
				{
					return_code=Graphical_element_creator_add_element(element_creator);
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphical_element_creator_scene_input_callback.  "
					"Invalid input_type");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_creator_scene_input_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphical_element_creator_scene_input_callback */

static int Graphical_element_creator_has_input(
	struct Graphical_element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_element_creator_has_input);
	if (element_creator)
	{
		return_code=((struct Scene *)NULL != element_creator->scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_creator_has_input.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_has_input */

static int Graphical_element_creator_grab_input(
	struct Graphical_element_creator *element_creator,struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Stores current scene input callback for <scene> then diverts it to the
<element_creator>. 
==============================================================================*/
{
	int return_code;
	struct Scene_input_callback scene_input_callback;

	ENTER(Graphical_element_creator_grab_input);
	return_code=0;
	if (element_creator&&scene)
	{
		if (Graphical_element_creator_has_input(element_creator))
		{
			display_message(ERROR_MESSAGE,
				"Graphical_element_creator_grab_input.  Editor already has input");
		}
		else
		{
			REACCESS(Scene)(&(element_creator->scene),scene);
			/* save current input callback for scene so it can be restored later */
			Scene_get_input_callback(scene,
				&(element_creator->stored_input_callback));
			/* establish new scene input callback for defining nodes and element */
			scene_input_callback.procedure=
				Graphical_element_creator_scene_input_callback;
			scene_input_callback.data=(void *)element_creator;
			Scene_set_input_callback(scene,&scene_input_callback);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_creator_grab_input.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_grab_input */

static int Graphical_element_creator_release_input(
	struct Graphical_element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Whether an element has just been successfully created, or creation is being
aborted early, call this function to restore previous scene input callbacks and
clean up the nodes array.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_element_creator_release_input);
	if (element_creator)
	{
		if (element_creator->scene)
		{
			element_creator->number_of_clicked_nodes=0;
			/* restore the stored_input_callback since scene may have changed */
			Scene_set_input_callback(element_creator->scene,
				&(element_creator->stored_input_callback));
			DEACCESS(Scene)(&(element_creator->scene));
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_creator_release_input.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_release_input */

/*
Global functions
----------------
*/
struct Graphical_element_creator *CREATE(Graphical_element_creator)(
	struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Creates a Graphical_element_creator, giving it the element_manager to put new
elements in, and the node_manager for new nodes, and the fe_field_manager to
enable the creation of a coordinate field.
==============================================================================*/
{
	struct Graphical_element_creator *element_creator;

	ENTER(CREATE(Graphical_element_creator));
	if (basis_manager&&element_manager&&fe_field_manager&&node_manager)
	{
		if (ALLOCATE(element_creator,struct Graphical_element_creator,1))
		{
			element_creator->mesh_editor_create_mode=MESH_EDITOR_NO_CREATE;
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
			element_creator->scene=(struct Scene *)NULL;
			element_creator->stored_input_callback.procedure=
				(Scene_input_callback_procedure *)NULL;
			element_creator->stored_input_callback.data=(void *)NULL;
		}
		else
		{
			DEALLOCATE(element_creator);
			display_message(ERROR_MESSAGE,
				"CREATE(Graphical_element_creator).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Graphical_element_creator).  Invalid argument(s)");
		element_creator=(struct Graphical_element_creator *)NULL;
	}
	LEAVE;

	return (element_creator);
} /* CREATE(Graphical_element_creator) */

int DESTROY(Graphical_element_creator)(
	struct Graphical_element_creator **element_creator_address)
/*******************************************************************************
LAST MODIFIED : 12 October 1999

DESCRIPTION :
Deaccesses objects and frees memory used by the Graphical_element_creator at
<*element_creator_address>.
==============================================================================*/
{
	struct Graphical_element_creator *element_creator;
	int return_code;

	ENTER(DESTROY(Graphical_element_creator));
	if (element_creator_address)
	{
		if (element_creator= *element_creator_address)
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
			if (element_creator->scene)
			{
				DEACCESS(Scene)(&(element_creator->scene));
			}
			DEALLOCATE(*element_creator_address);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Graphical_element_creator).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Graphical_element_creator) */

int Graphical_element_creator_get_mesh_editor_create_mode(
	struct Graphical_element_creator *element_creator,
	enum Mesh_editor_create_mode *mesh_editor_create_mode)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_element_creator_get_mesh_editor_create_mode);
	if (element_creator&&mesh_editor_create_mode)
	{
		*mesh_editor_create_mode = element_creator->mesh_editor_create_mode;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_creator_get_mesh_editor_create_mode.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_get_mesh_editor_create_mode */

int Graphical_element_creator_set_mesh_editor_create_mode(
	struct Graphical_element_creator *element_creator,
	enum Mesh_editor_create_mode mesh_editor_create_mode,
	struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_element_creator_set_mesh_editor_create_mode);
	if (element_creator&&scene)
	{
		return_code=1;
		if (mesh_editor_create_mode != element_creator->mesh_editor_create_mode)
		{
			if ((MESH_EDITOR_NO_CREATE==mesh_editor_create_mode)||
				(MESH_EDITOR_CREATE_ONLY_NODES==mesh_editor_create_mode))
			{
				if (element_creator->element)
				{
					display_message(WARNING_MESSAGE,"Aborting current element creation");
					DEACCESS(FE_element)(&(element_creator->element));
				}
			}
			if (MESH_EDITOR_NO_CREATE==mesh_editor_create_mode)
			{
				Graphical_element_creator_release_input(element_creator);
			}
			else
			{
				if (!Graphical_element_creator_has_input(element_creator))
				{
					Graphical_element_creator_grab_input(element_creator,scene);
				}
			}
			element_creator->mesh_editor_create_mode = mesh_editor_create_mode;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_creator_set_mesh_editor_create_mode.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_set_mesh_editor_create_mode */

int Graphical_element_creator_get_element_dimension(
	struct Graphical_element_creator *element_creator)
/*******************************************************************************
LAST MODIFIED : 21 October 1999

DESCRIPTION :
Returns the dimension of elements to be created by the <element_creator>.
==============================================================================*/
{
	int element_dimension;

	ENTER(Graphical_element_creator_get_element_dimension);
	if (element_creator)
	{
		element_dimension=element_creator->element_dimension;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_creator_get_element_dimension.  Invalid argument(s)");
		element_dimension=0;
	}
	LEAVE;

	return (element_dimension);
} /* Graphical_element_creator_get_element_dimension */

int Graphical_element_creator_set_element_dimension(
	struct Graphical_element_creator *element_creator,int element_dimension)
/*******************************************************************************
LAST MODIFIED : 21 October 1999

DESCRIPTION :
Sets the <element_dimension> of elements to be created by <element_creator>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_element_creator_set_element_dimension);
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
					"Graphical_element_creator_set_element_dimension.  "
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
			"Graphical_element_creator_set_element_dimension.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_set_element_dimension */

int Graphical_element_creator_get_groups(
	struct Graphical_element_creator *element_creator,
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

	ENTER(Graphical_element_creator_get_groups);
	if (element_creator&&element_group&&node_group)
	{
		*element_group=element_creator->element_group;
		*node_group=element_creator->node_group;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_element_creator_get_groups.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_get_groups */

int Graphical_element_creator_set_groups(
	struct Graphical_element_creator *element_creator,
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

	ENTER(Graphical_element_creator_set_groups);
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
			"Graphical_element_creator_set_groups.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_element_creator_set_groups */

