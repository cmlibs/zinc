/*******************************************************************************
FILE : finite_element_to_iges.c

LAST MODIFIED : 4 March 2003

DESCRIPTION :
Functions for building IGES information from finite elements and exporting
to file.
==============================================================================*/
/* for IGES */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_wrappers.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_to_iges.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

struct IGES_entity_info
/******************************************************************************
LAST MODIFIED : 7 June 2002

DESCRIPTION :
==============================================================================*/
{
	struct CM_element_information cm;
	int directory_pointer,parameter_pointer,type;
	struct
	{
		char label[9];
		int color,form,label_display_associativity,level,line_font_pattern,
			line_weight,parameter_line_count,subscript_number,structure,
			transformation_matrix,view;
		struct
		{
			int blank_status,entity_use_flag,hierarchy,subordinate_entity_switch;
		} status;
	} directory;
	union
	{
		struct
		{
			/* composite curve entity */
			int *directory_pointers,number_of_entities;
		} type_102;
		struct
		{
			/* line entity */
			float end[3],start[3];
		} type_110;
		struct
		{
			/* parametric spline curve entity */
			int degree_of_continuity,n,number_of_dimensions,spline_type;
			/*???DB.  Assuming 1 cubic curve */
			float tu[2],x[4],y[4],z[4];
		} type_112;
		struct
		{
			/* parametric spline surface entity */
			int m,n,patch_type,spline_boundary_type;
			/*???DB.  Assuming 1 bicubic surface */
			float tu[2],tv[2],x[16],y[16],z[16];
		} type_114;
		struct
		{
			/* curve on parametric surface entity */
			int how_curve_created,material_curve_directory_pointer,
				preferred_representation,surface_directory_pointer,
				world_curve_directory_pointer;
		} type_142;
		struct
		{
			/* trimmed parametric surface entity */
			int *inner_boundary_directory_pointers,number_of_inner_boundary_curves,
				outer_boundary_directory_pointer,outer_boundary_type,
				surface_directory_pointer;
		} type_144;
	} parameter;
	struct IGES_entity_info *next;
}; /* struct IGES_entity_info */

#define NUMBER_OF_NODES (16)

struct Get_iges_entity_info_data
/******************************************************************************
LAST MODIFIED : 6 June 2002

DESCRIPTION :
==============================================================================*/
{
	struct IGES_entity_info *head,*tail;
	struct Computed_field *field;
	struct FE_element *element;
	struct FE_field *fe_field;
	struct FE_node *nodes[NUMBER_OF_NODES];
	struct FE_region *fe_region;
	int extra_line_number;
}; /* struct Get_iges_entity_info_data */

static struct IGES_entity_info *create_iges_entity_info(
	struct FE_element *element,struct IGES_entity_info **head,
	struct IGES_entity_info **tail)
/******************************************************************************
LAST MODIFIED : 6 June 2002

DESCRIPTION :
==============================================================================*/
{
	int directory_pointer,parameter_pointer;
	struct IGES_entity_info *entity;

	ENTER(create_iges_entity_info);
	entity=(struct IGES_entity_info *)NULL;
	if (element&&head&&tail)
	{
		if (ALLOCATE(entity,struct IGES_entity_info,1))
		{
			/* put into list */
			if ((*head)&&(*tail))
			{
				directory_pointer=(*tail)->directory_pointer+2;
				parameter_pointer=(*tail)->parameter_pointer+
					((*tail)->directory).parameter_line_count;
				(*tail)->next=entity;
				*tail=entity;
			}
			else
			{
				directory_pointer=1;
				parameter_pointer=1;
				*head=entity;
				*tail=entity;
			}
			entity->next=(struct IGES_entity_info *)NULL;
			/* fill in values */
			get_FE_element_identifier(element, &(entity->cm));
			entity->type=0;
			entity->directory_pointer=directory_pointer;
			entity->parameter_pointer=parameter_pointer;
			(entity->directory).structure=0;
			(entity->directory).line_font_pattern=0;
			(entity->directory).level=0;
			(entity->directory).view=0;
			(entity->directory).transformation_matrix=0;
			(entity->directory).label_display_associativity=0;
			(entity->directory).status.blank_status=0;
			(entity->directory).status.subordinate_entity_switch=0;
			(entity->directory).status.entity_use_flag=0;
			(entity->directory).status.hierarchy=0;
			(entity->directory).line_weight=0;
			(entity->directory).color=0;
			(entity->directory).parameter_line_count=0;
			(entity->directory).form=0;
			((entity->directory).label)[0]='\0';
			(entity->directory).subscript_number=0;
		}
	}
	LEAVE;

	return (entity);
} /* create_iges_entity_info */

static int get_iges_entity_info(struct FE_element *element,
	void *get_data_void)
/******************************************************************************
LAST MODIFIED : 7 August 2003

DESCRIPTION :
==============================================================================*/
{
	FE_value *destination, *source, *source_ptr;
	int *faces_material,*faces_world,i,j,k,material_curve_directory_pointer,
		monomial_x[1 + MAXIMUM_ELEMENT_XI_DIMENSIONS],
		monomial_y[1 + MAXIMUM_ELEMENT_XI_DIMENSIONS],
		monomial_z[1 + MAXIMUM_ELEMENT_XI_DIMENSIONS],
		number_of_component_values, number_of_faces,number_of_parents,
		outer_boundary_directory_pointer,*reorder_faces,return_code,
		surface_directory_pointer,world_curve_directory_pointer;
	struct CM_element_information cm;
	struct FE_element *face;
	struct FE_field *coordinate_field;
	struct FE_element_field_values *coordinate_element_field_values;
	struct Get_iges_entity_info_data *get_data;
	struct IGES_entity_info *entity,*surface_entity;

	ENTER(get_iges_entity_info);
	return_code=0;
	if (element&&(get_data=(struct Get_iges_entity_info_data *)get_data_void))
	{
		return_code=1;
		coordinate_element_field_values = (struct FE_element_field_values *)NULL;
		if ((2==get_FE_element_dimension(element))&&
			get_FE_element_number_of_parents(element, &number_of_parents) &&
			(1>=number_of_parents)&&(coordinate_field=get_data->fe_field)&&
			(3==get_FE_field_number_of_components(coordinate_field))&&
			(coordinate_element_field_values = CREATE(FE_element_field_values)()) &&
			(calculate_FE_element_field_values(element,coordinate_field,
			/*time*/(FE_value)0,/*calculate_derivatives*/(char)0,coordinate_element_field_values,
			(struct FE_element *)NULL))&&
			FE_element_field_values_get_monomial_component_info(
				coordinate_element_field_values, /*component_number*/0, monomial_x) &&
			(2 == monomial_x[0]) && (monomial_x[1] <= 3) && (monomial_x[2] <= 3) &&
			FE_element_field_values_get_monomial_component_info(
				coordinate_element_field_values, /*component_number*/1, monomial_y) &&
			(2 == monomial_y[0]) && (monomial_y[1] <= 3) && (monomial_y[2] <= 3) &&
			FE_element_field_values_get_monomial_component_info(
				coordinate_element_field_values, /*component_number*/2, monomial_z) &&
			(2 == monomial_z[0]) && (monomial_z[1] <= 3) && (monomial_z[2] <= 3))
		{
			if (surface_entity=create_iges_entity_info(element,&(get_data->head),
				&(get_data->tail)))
			{
				surface_directory_pointer=surface_entity->directory_pointer;
				/* blanked */
				(surface_entity->directory).status.blank_status=1;
				/* physically dependent */
				(surface_entity->directory).status.subordinate_entity_switch=1;
				/* parametric spline surface entity */
				surface_entity->type=114;
				/* use one line for integer flags, one line for the breakpoints and
					4 values per line for coefficients */
				(surface_entity->directory).parameter_line_count=14;
				/* cubic */
				(surface_entity->parameter).type_114.spline_boundary_type=3;
				/* cartesian product */
				(surface_entity->parameter).type_114.patch_type=1;
				(surface_entity->parameter).type_114.m=1;
				(surface_entity->parameter).type_114.n=1;
				((surface_entity->parameter).type_114.tu)[0]=0.;
				((surface_entity->parameter).type_114.tu)[1]=1.;
				((surface_entity->parameter).type_114.tv)[0]=0.;
				((surface_entity->parameter).type_114.tv)[1]=1.;
				if (FE_element_field_values_get_component_values(
					coordinate_element_field_values, /*component_number*/0,
					&number_of_component_values, &source) &&
					(0 < number_of_component_values) && source)
				{
					source_ptr = source;
					destination=(surface_entity->parameter).type_114.x;
					k=0;
					for (j=0;j<=monomial_x[2];j++)
					{
						for (i=0;i<=monomial_x[1];i++)
						{
							*destination= *source_ptr;
							destination++;
							source_ptr++;
							k++;
						}
						while (i<=3)
						{
							*destination=(float)0;
							destination++;
							k++;
							i++;
						}
					}
					while (k<16)
					{
						*destination=(float)0;
						destination++;
						k++;
					}
					DEALLOCATE(source);
				}
				if (FE_element_field_values_get_component_values(
					coordinate_element_field_values, /*component_number*/1,
					&number_of_component_values, &source) &&
					(0 < number_of_component_values) && source)
				{
					source_ptr = source;
					destination=(surface_entity->parameter).type_114.y;
					k=0;
					for (j=0;j<=monomial_y[2];j++)
					{
						for (i=0;i<=monomial_y[1];i++)
						{
							*destination= *source_ptr;
							destination++;
							source_ptr++;
							k++;
						}
						while (i<=3)
						{
							*destination=(float)0;
							destination++;
							k++;
							i++;
						}
					}
					while (k<16)
					{
						*destination=(float)0;
						destination++;
						k++;
					}
					DEALLOCATE(source);
				}
				if (FE_element_field_values_get_component_values(
					coordinate_element_field_values, /*component_number*/2,
					&number_of_component_values, &source) &&
					(0 < number_of_component_values) && source)
				{
					source_ptr = source;
					destination=(surface_entity->parameter).type_114.z;
					k=0;
					for (j=0;j<=monomial_z[2];j++)
					{
						for (i=0;i<=monomial_z[1];i++)
						{
							*destination= *source_ptr;
							destination++;
							source_ptr++;
							k++;
						}
						while (i<=3)
						{
							*destination=(float)0;
							destination++;
							k++;
							i++;
						}
					}
					while (k<16)
					{
						*destination=(float)0;
						destination++;
						k++;
					}
					DEALLOCATE(source);
				}
				/* add information for edges so that can be joined together */
				if (get_FE_element_number_of_faces(element, &number_of_faces) &&
					(4 == number_of_faces))
				{
					ALLOCATE(faces_material,int,number_of_faces);
					ALLOCATE(faces_world,int,number_of_faces);
					ALLOCATE(reorder_faces,int,number_of_faces);
					if (faces_material&&faces_world)
					{
						/* reorder the faces to go around the surface (not cmiss way) */
						reorder_faces[0]=0;
						reorder_faces[1]=2;
						reorder_faces[2]=3;
						reorder_faces[3]=1;
						i=0;
						while (return_code&&(i<number_of_faces))
						{
							if (get_FE_element_face(element, i, &face) && face &&
								get_FE_element_identifier(face, &cm))
							{
								/* create an entity for the edge in material coordinates */
								if (entity=create_iges_entity_info(face,&(get_data->head),
									&(get_data->tail)))
								{
									faces_material[reorder_faces[i]]=entity->directory_pointer;
									/* blanked */
									(entity->directory).status.blank_status=1;
									/* physically dependent */
									(entity->directory).status.subordinate_entity_switch=1;
									/* 2d parametric.  In xi coordinates */
									(entity->directory).status.entity_use_flag=5;
									/* line entity */
									entity->type=110;
									/* use one line for type, one line for start and one line
										for end */
									(entity->directory).parameter_line_count=3;
									/* cmiss order is xi1=0, xi1=1, xi2=0, xi2=1 */
									switch (i)
									{
										case 0:
										{
											/* xi1=0 */
											((entity->parameter).type_110.start)[0]=0.;
											((entity->parameter).type_110.start)[1]=0.;
											((entity->parameter).type_110.end)[0]=0.;
											((entity->parameter).type_110.end)[1]=1.;
										} break;
										case 1:
										{
											/* xi1=1 */
											((entity->parameter).type_110.start)[0]=1.;
											((entity->parameter).type_110.start)[1]=0.;
											((entity->parameter).type_110.end)[0]=1.;
											((entity->parameter).type_110.end)[1]=1.;
										} break;
										case 2:
										{
											/* xi2=0 */
											((entity->parameter).type_110.start)[0]=0.;
											((entity->parameter).type_110.start)[1]=0.;
											((entity->parameter).type_110.end)[0]=1.;
											((entity->parameter).type_110.end)[1]=0.;
										} break;
										case 3:
										{
											/* xi2=1 */
											((entity->parameter).type_110.start)[0]=0.;
											((entity->parameter).type_110.start)[1]=1.;
											((entity->parameter).type_110.end)[0]=1.;
											((entity->parameter).type_110.end)[1]=1.;
										} break;
									}
									((entity->parameter).type_110.start)[2]=0.;
									((entity->parameter).type_110.end)[2]=0.;
									entity=get_data->head;
									while (entity && !((cm.type == (entity->cm).type) &&
										(cm.number == (entity->cm).number) &&
										(112 == entity->type)))
									{
										entity=entity->next;
									}
									if (entity)
									{
										faces_world[reorder_faces[i]]=entity->directory_pointer;
									}
									else
									{
										/* create an entity for the edge in material coordinates */
										if (entity=create_iges_entity_info(face,&(get_data->head),
											&(get_data->tail)))
										{
											faces_world[reorder_faces[i]]=entity->directory_pointer;
											/* blanked */
											(entity->directory).status.blank_status=1;
											/* physically dependent */
											(entity->directory).status.subordinate_entity_switch=1;
											/* parametric spline curve entity */
											entity->type=112;
											/* use one line for integer flags, one line for the
												breakpoints and 4 values per line for coefficients */
											(entity->directory).parameter_line_count=5;
											/* cubic */
											(entity->parameter).type_112.spline_type=3;
											/* value and slope continuous at breakpoints */
											(entity->parameter).type_112.degree_of_continuity=1;
											(entity->parameter).type_112.number_of_dimensions=3;
											(entity->parameter).type_112.n=1;
											((entity->parameter).type_112.tu)[0]=0.;
											((entity->parameter).type_112.tu)[1]=1.;
											switch (i)
											{
												case 0:
												{
													/* xi1=0 */
													for (j=0;j<4;j++)
													{
														((entity->parameter).type_112.x)[j]=
															((surface_entity->parameter).type_114.x)[4*j];
														((entity->parameter).type_112.y)[j]=
															((surface_entity->parameter).type_114.y)[4*j];
														((entity->parameter).type_112.z)[j]=
															((surface_entity->parameter).type_114.z)[4*j];
													}
												} break;
												case 1:
												{
													/* xi1=1 */
													for (j=0;j<4;j++)
													{
														((entity->parameter).type_112.x)[j]=
															((surface_entity->parameter).type_114.x)[4*j];
														((entity->parameter).type_112.y)[j]=
															((surface_entity->parameter).type_114.y)[4*j];
														((entity->parameter).type_112.z)[j]=
															((surface_entity->parameter).type_114.z)[4*j];
														for (k=1;k<4;k++)
														{
															((entity->parameter).type_112.x)[j] +=
																((surface_entity->parameter).type_114.x)[4*j+k];
															((entity->parameter).type_112.y)[j] +=
																((surface_entity->parameter).type_114.y)[4*j+k];
															((entity->parameter).type_112.z)[j] +=
																((surface_entity->parameter).type_114.z)[4*j+k];
														}
													}
												} break;
												case 2:
												{
													/* xi2=0 */
													for (j=0;j<4;j++)
													{
														((entity->parameter).type_112.x)[j]=
															((surface_entity->parameter).type_114.x)[j];
														((entity->parameter).type_112.y)[j]=
															((surface_entity->parameter).type_114.y)[j];
														((entity->parameter).type_112.z)[j]=
															((surface_entity->parameter).type_114.z)[j];
													}
												} break;
												case 3:
												{
													/* xi2=1 */
													for (j=0;j<4;j++)
													{
														((entity->parameter).type_112.x)[j]=
															((surface_entity->parameter).type_114.x)[j];
														((entity->parameter).type_112.y)[j]=
															((surface_entity->parameter).type_114.y)[j];
														((entity->parameter).type_112.z)[j]=
															((surface_entity->parameter).type_114.z)[j];
														for (k=4;k<16;k += 4)
														{
															((entity->parameter).type_112.x)[j] +=
																((surface_entity->parameter).type_114.x)[j+k];
															((entity->parameter).type_112.y)[j] +=
																((surface_entity->parameter).type_114.y)[j+k];
															((entity->parameter).type_112.z)[j] +=
																((surface_entity->parameter).type_114.z)[j+k];
														}
													}
												} break;
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
							}
							i++;
						}
						if (return_code)
						{
							/* create a composite curve entity to describe the boundary of
								the element in material coordinates */
							if ((entity=create_iges_entity_info(element,&(get_data->head),
								&(get_data->tail)))&&ALLOCATE((entity->parameter).type_102.
								directory_pointers,int,number_of_faces))
							{
								material_curve_directory_pointer=entity->directory_pointer;
								/* blanked */
								(entity->directory).status.blank_status=1;
								/* physically dependent */
								(entity->directory).status.subordinate_entity_switch=1;
								/* 2d parametric.  In xi coordinates */
								(entity->directory).status.entity_use_flag=5;
								/* composite curve entity */
								entity->type=102;
								(entity->directory).parameter_line_count=1;
								(entity->parameter).type_102.number_of_entities=number_of_faces;
								for (j=0;j<number_of_faces;j++)
								{
									((entity->parameter).type_102.directory_pointers)[j]=
										faces_material[j];
								}
								/* create a composite curve entity to describe the boundary of
									the element in world coordinates */
								if ((entity=create_iges_entity_info(element,&(get_data->head),
									&(get_data->tail)))&&ALLOCATE((entity->parameter).type_102.
									directory_pointers,int,number_of_faces))
								{
									world_curve_directory_pointer=entity->directory_pointer;
									/* blanked */
									(entity->directory).status.blank_status=1;
									/* physically dependent */
									(entity->directory).status.subordinate_entity_switch=1;
									/* composite curve entity */
									entity->type=102;
									(entity->directory).parameter_line_count=1;
									(entity->parameter).type_102.number_of_entities=
										number_of_faces;
									for (j=0;j<number_of_faces;j++)
									{
										((entity->parameter).type_102.directory_pointers)[j]=
											faces_world[j];
									}
									/* combine the world and material boundary descriptions */
									if (entity=create_iges_entity_info(element,&(get_data->head),
										&(get_data->tail)))
									{
										outer_boundary_directory_pointer=entity->directory_pointer;
										/* blanked */
										(entity->directory).status.blank_status=1;
										/* physically dependent */
										(entity->directory).status.subordinate_entity_switch=1;
										/* 2d parametric.  In xi coordinates */
										(entity->directory).status.entity_use_flag=5;
										/* curve on parametric surface entity */
										entity->type=142;
										(entity->directory).parameter_line_count=1;
										/* projection of a given curve on the surface */
										(entity->parameter).type_142.how_curve_created=1;
										(entity->parameter).type_142.surface_directory_pointer=
											surface_directory_pointer;
										(entity->parameter).type_142.
											material_curve_directory_pointer=
											material_curve_directory_pointer;
										(entity->parameter).type_142.world_curve_directory_pointer=
											world_curve_directory_pointer;
										/* material specification is preferred */
										(entity->parameter).type_142.preferred_representation=1;
										/* create the trimmed surface that can be stitched
											together */
										if (entity=create_iges_entity_info(element,
											&(get_data->head),&(get_data->tail)))
										{
											/* trimmed parametric surface entity */
											entity->type=144;
											(entity->directory).parameter_line_count=1;
											(entity->parameter).type_144.surface_directory_pointer=
												surface_directory_pointer;
											/* outer boundary is the one specified (not the boundary
												of <surface_directory_pointer> */
											(entity->parameter).type_144.outer_boundary_type=1;
											(entity->parameter).type_144.
												outer_boundary_directory_pointer=
												outer_boundary_directory_pointer;
											(entity->parameter).type_144.
												number_of_inner_boundary_curves=0;
											(entity->parameter).type_144.
												inner_boundary_directory_pointers=(int *)NULL;
										}
										else
										{
											return_code=0;
										}
									}
									else
									{
										return_code=0;
									}
								}
								else
								{
									return_code=0;
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
					DEALLOCATE(reorder_faces);
					DEALLOCATE(faces_world);
					DEALLOCATE(faces_material);
				}
			}
			else
			{
				return_code=0;
			}
		}
		if (coordinate_element_field_values)
		{
			DESTROY(FE_element_field_values)(&coordinate_element_field_values);
		}
	}
	LEAVE;

	return (return_code);
} /* get_iges_entity_info */

static int get_iges_entity_as_cubic_from_any_2D_element(struct FE_element *element,
	void *get_data_void)
/******************************************************************************
LAST MODIFIED : 5 August 2003

DESCRIPTION :
SAB This function uses a template bicubic element patch to generate an IGES
representation of any 2D element.  The actual element is evaluated for nodal 
positions and  derivatives at each corner and these are just put into the template
nodes. This means that the iges code does not need to be implemented for each 
basis type, however every element type will be converted to a cubic.
==============================================================================*/
{
	FE_value values[3], xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int i, j, number_of_parents, number_of_values, return_code;
	struct CM_element_information cm;
	struct FE_element *face, *true_face;
	struct FE_element_shape *element_shape, *face_shape;
	struct FE_node_field_creator *node_field_creator;
	struct Get_iges_entity_info_data *get_data;

	ENTER(get_iges_entity_info);
	return_code = 0;
	/* check arguments */
	if (element && (get_data = (struct Get_iges_entity_info_data *)get_data_void))
	{
		return_code = 1;
		if ((2 == get_FE_element_dimension(element)) &&
			get_FE_element_number_of_parents(element, &number_of_parents) &&
			(1>=number_of_parents))
		{
			/* Create the node and element templates */
			if (!get_data->element)
			{
				/* Then we presume we have made nothing */
				/* Make a suitable FE_field */
				if ((get_data->fe_field = CREATE(FE_field)("coordinates", get_data->fe_region)) &&
					set_FE_field_value_type(get_data->fe_field, FE_VALUE_VALUE) &&
					set_FE_field_number_of_components(get_data->fe_field, /*number_of_components*/3))
				{
					ACCESS(FE_field)(get_data->fe_field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_iges_entity_as_cubic_from_any_2D_element.  "
						"Could not create fe field");
					DEACCESS(FE_field)(&(get_data->fe_field));
					return_code=0;
				}

				if (node_field_creator = CREATE(FE_node_field_creator)(
						 /*number_of_components*/3))
				{
#if defined (OLD_CODE)
					for (i = 0 ; i < 3 ; i++)
					{
						FE_node_field_creator_define_derivative(
							node_field_creator, /*component_number*/i,
							FE_NODAL_D_DS1);
						FE_node_field_creator_define_derivative(
							node_field_creator, /*component_number*/i,
							FE_NODAL_D_DS2);
						FE_node_field_creator_define_derivative(
							node_field_creator, /*component_number*/i,
							FE_NODAL_D2_DS1DS2);
					}
#endif /* defined (OLD_CODE) */
					for (i = 0 ; return_code && (i < NUMBER_OF_NODES) ; i++)
					{
						if (get_data->nodes[i] = CREATE(FE_node)(/*node_number*/0,
							 get_data->fe_region, (struct FE_node *)NULL))
						{
							ACCESS(FE_node)(get_data->nodes[i]);
							if (!define_FE_field_at_node(get_data->nodes[i], get_data->fe_field,
									 (struct FE_time_sequence *)NULL, node_field_creator))
							{
								display_message(ERROR_MESSAGE,
									"get_iges_entity_as_cubic_from_any_2D_element.  "
									"Could not define coordinate field in node_1");
								DEACCESS(FE_node)(&(get_data->nodes[i]));
								return_code=0;
							}
						}
					}
					DESTROY(FE_node_field_creator)(&(node_field_creator));
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_iges_entity_as_cubic_from_any_2D_element.  "
						"Could not create node field creator");
					return_code=0;
				}
				if (return_code)
				{
					if ((get_data->element = create_FE_element_with_line_shape(
							  /*identifier*/1,get_data->fe_region, /*dimension*/2)) &&
						FE_element_define_tensor_product_basis(get_data->element,
						/*dimension*/2,/*basis_type*/CUBIC_LAGRANGE, get_data->fe_field))
					{
						ACCESS(FE_element)(get_data->element);
						for (i = 0 ; return_code && (i < NUMBER_OF_NODES) ; i++)
						{
							if (!set_FE_element_node(get_data->element, i, get_data->nodes[i]))
							{
								display_message(ERROR_MESSAGE,
									"get_iges_entity_as_cubic_from_any_2D_element.  "
									"Could not set node %d into element", i+1);
								return_code=0;
							}
						}
					}
					else
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* Add four faces */
					get_FE_element_shape(element, &element_shape);
					cm.type = CM_LINE;
					cm.number = 0;
					for (i = 0 ; return_code && (i < 4) ; i++)
					{
						face_shape = get_FE_element_shape_of_face(element_shape, i, get_data->fe_region);
						if (face = CREATE(FE_element)(&cm, face_shape,
								 get_data->fe_region, (struct FE_element *)NULL))
						{
							/* must put the face in the element to inherit fields */
							set_FE_element_face(get_data->element, i, face);
						}
						else
						{
							return_code = 0;
						}
					}
				}
			}
			xi[2] = 0.0;
			/* Fill in the nodal values */
			for (i = 0 ; return_code && (i < 4) ; i++)
			{
				for (j = 0 ; return_code && (j < 4) ; j++)
				{
					xi[0] = (float)j / 3.0;
					xi[1] = (float)i / 3.0;
					if (Computed_field_evaluate_in_element(get_data->field, element, xi, 
						 /*time*/0.0, /*top_level_element*/(struct FE_element *)NULL,
						 values, (FE_value *)NULL))
					{
						if (!set_FE_nodal_field_FE_value_values(get_data->fe_field,
							get_data->nodes[i * 4 + j], values, &number_of_values))
						{
							display_message(ERROR_MESSAGE,
								"get_iges_entity_as_cubic_from_any_2D_element.  "
								"Unable to set values in node %d", i * 4 + j);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"get_iges_entity_as_cubic_from_any_2D_element.  "
							"Unable to evaluate values in node %d", i * 4 + j);
						return_code=0;
					}
				}
			}
			/* Set the correct element number into the template element */
			get_FE_element_identifier(element, &cm);
			cm.type = CM_ELEMENT;
			set_FE_element_identifier(get_data->element, &cm);
			/* Set the correct element numbers into the faces */
			for (i = 0 ; i < 4 ; i++)
			{
				get_FE_element_face(element, i, &true_face);
				get_FE_element_face(get_data->element, i, &face);
				if (!(true_face && get_FE_element_identifier(true_face, &cm)))
				{
					cm.type = CM_LINE;
					cm.number = FE_region_get_next_FE_element_identifier(get_data->fe_region,
						CM_LINE, get_data->extra_line_number + 1);
					get_data->extra_line_number = cm.number;
				}
				set_FE_element_identifier(face, &cm);
			}
			/* Add the element entity info for the element we just made */
			if (return_code)
			{
				get_iges_entity_info(get_data->element, get_data_void);
			}
		}
	}
	LEAVE;
	
	return (return_code);
} /* get_iges_entity_as_cubic_from_any_2D_element */
	
/*
Global functions
----------------
*/

struct Write_iges_parameter_data_data
{
	FILE *iges;
	int count;
}; /* struct Write_iges_parameter_data_data */

int export_to_iges(char *file_name, struct FE_region *fe_region,
	char *region_path, struct Computed_field *field)
/******************************************************************************
LAST MODIFIED : 5 August 2003

DESCRIPTION :
Write bicubic elements to an IGES file.
==============================================================================*/
{
	char *out_string, numeric_string[20], *temp_string, time_string[14];
	FILE *iges;
	int count, i, global_count, length, out_length, parameter_pointer,
		return_code, sub_count;
	struct Get_iges_entity_info_data iges_entity_info_data;
	struct IGES_entity_info *entity;
	time_t coded_time;
	struct tm *time_struct;

	ENTER(export_to_iges);
	return_code=0;
	USE_PARAMETER(field);
	if (file_name && fe_region && region_path)
	{
		if (iges=fopen(file_name,"w"))
		{
			return_code=1;
			/* write IGES header */
			/* start section */
			fprintf(iges,"%-72sS      1\n","cmgui");
			/* global section */
			global_count=0;
			out_string=(char *)NULL;
			out_length=0;
#define WRITE_STRING_PARAMETER( parameter ) \
{ \
	length=strlen(parameter); \
	if (length>0) \
	{ \
		length += (int)ceil(log10((double)length))+2; \
		if (REALLOCATE(temp_string,out_string,char,out_length+length+2)) \
		{ \
			out_string=temp_string; \
			sprintf(out_string+out_length,"%dH",(int)strlen(parameter)); \
			strcat(out_string,parameter); \
			strcat(out_string,","); \
			out_length=strlen(out_string); \
			while (out_length>72) \
			{ \
				global_count++; \
				fprintf(iges,"%.72sG%7d\n",out_string,global_count); \
				out_length -= 72; \
				temp_string=out_string; \
				for (i=0;i<=out_length;i++) \
				{ \
					*temp_string=temp_string[72]; \
					temp_string++; \
				} \
			} \
		} \
	} \
	else \
	{ \
		if (REALLOCATE(temp_string,out_string,char,out_length+2)) \
		{ \
			out_string=temp_string; \
			strcat(out_string,","); \
			out_length=strlen(out_string); \
		} \
	} \
}
#define WRITE_INTEGER_PARAMETER( parameter ) \
{ \
	sprintf(numeric_string,"%d",parameter); \
	length=strlen(numeric_string)+1; \
	if (REALLOCATE(temp_string,out_string,char,out_length+length+1)) \
	{ \
		out_string=temp_string; \
		if (out_length+length>72) \
		{ \
			global_count++; \
			fprintf(iges,"%-72sG%7d\n",out_string,global_count); \
			out_string[0]='\0'; \
			out_length=0; \
		} \
		strcat(out_string,numeric_string); \
		strcat(out_string,","); \
		out_length=strlen(out_string); \
	} \
}
#define WRITE_REAL_PARAMETER( parameter ) \
{ \
	sprintf(numeric_string,"%.6e",parameter); \
	length=strlen(numeric_string)+1; \
	if (REALLOCATE(temp_string,out_string,char,out_length+length+1)) \
	{ \
		out_string=temp_string; \
		if (out_length+length>72) \
		{ \
			global_count++; \
			fprintf(iges,"%-72sG%7d\n",out_string,global_count); \
			out_string[0]='\0'; \
			out_length=0; \
		} \
		strcat(out_string,numeric_string); \
		strcat(out_string,","); \
		out_length=strlen(out_string); \
	} \
}
			/* parameter delimiter */
			WRITE_STRING_PARAMETER(",");
			/* record delimiter */
			WRITE_STRING_PARAMETER(";");
			/* product identification from sending system */
			WRITE_STRING_PARAMETER(region_path);
			/* file name */
			WRITE_STRING_PARAMETER(file_name);
			/* system ID */
			WRITE_STRING_PARAMETER("cmgui");
			/* version */
			WRITE_STRING_PARAMETER("unknown");
			/* number of binary bits for integer representation */
			WRITE_INTEGER_PARAMETER((int)(8*sizeof(int)));
			/* maximum power of ten representable in a single precision floating point
				number on the sending system */
			WRITE_INTEGER_PARAMETER(FLT_MAX_10_EXP);
			/* number of significant digits in a single precision floating point
				number on the sending system */
			WRITE_INTEGER_PARAMETER(FLT_DIG);
			/* maximum power of ten representable in a double precision floating point
				number on the sending system */
			WRITE_INTEGER_PARAMETER(DBL_MAX_10_EXP);
			/* number of significant digits in a double precision floating point
				number on the sending system */
			WRITE_INTEGER_PARAMETER(DBL_DIG);
			/* product identification for the receiving system */
			WRITE_STRING_PARAMETER(region_path);
			/* model space scale (example:  .125 indicates a ratio of 1 unit model
				space to 8 units real world) */
			WRITE_REAL_PARAMETER(1.);
			/* unit flag.  millimetres */
			WRITE_INTEGER_PARAMETER(2);
			/* units abreviation */
			WRITE_STRING_PARAMETER("MM");
			/* maximum number of line weight gradations */
			WRITE_INTEGER_PARAMETER(1);
			/* width of maximum line weight in units */
			WRITE_REAL_PARAMETER(0.125);
			/* date & time of exchange file generation  YYMMDD.HHNNSS */
			time(&coded_time);
			time_struct=localtime(&coded_time);
			sprintf(time_string,"%02d%02d%02d.%02d%02d%02d",
				(time_struct->tm_year)%100,time_struct->tm_mday,(time_struct->tm_mon)+1,
				time_struct->tm_hour,time_struct->tm_min,time_struct->tm_sec);
			WRITE_STRING_PARAMETER(time_string);
			/* minimum user-intended resolution or granularity of the model expressed
				in units */
			WRITE_REAL_PARAMETER(1.e-8);
			/* approximate maximum coordinate value occurring in the model expressed
				in units */
			WRITE_REAL_PARAMETER(1.e4);
			/* name of author */
			WRITE_STRING_PARAMETER("");
			/* author's organization */
			WRITE_STRING_PARAMETER("");
			/* integer value corresponding to the version of the Specification used to
				create this file */
			WRITE_INTEGER_PARAMETER(5);
			/* drafting standard in compliance to which the data encoded in this file
				was generated */
			WRITE_INTEGER_PARAMETER(0);
			/* date and time the model was created or last modified, whichever
				occurred last, YYMMDD.HHNNSS */
			WRITE_STRING_PARAMETER("");
			global_count++;
			out_string[strlen(out_string)-1]=';';
			fprintf(iges,"%-72sG%7d\n",out_string,global_count);
			/* get entity information */
			iges_entity_info_data.head=(struct IGES_entity_info *)NULL;
			iges_entity_info_data.tail=(struct IGES_entity_info *)NULL;
			iges_entity_info_data.field=Computed_field_begin_wrap_coordinate_field(field);
			iges_entity_info_data.fe_field=(struct FE_field *)NULL;
			iges_entity_info_data.fe_region=fe_region;
			iges_entity_info_data.element=(struct FE_element *)NULL;
			iges_entity_info_data.extra_line_number = 999000; /* Start somewhere random although
							                              we will still check that it doesn't conflict */
         for (i = 0 ; i < NUMBER_OF_NODES ; i++)
         {
				iges_entity_info_data.nodes[i]=(struct FE_node *)NULL;
			}
#if defined (NEW_CODE)
         FE_region_get_default_coordinate_FE_field(
				fe_region,&(iges_entity_info_data.fe_field));
			FE_region_for_each_FE_element(fe_region, get_iges_entity_info,
				&iges_entity_info_data);
         USE_PARAMETER(get_iges_entity_as_cubic_from_any_2D_element);
#endif /* defined (NEW_CODE) */
			FE_region_for_each_FE_element(fe_region, get_iges_entity_as_cubic_from_any_2D_element,
				&iges_entity_info_data);
         Computed_field_end_wrap(&(iges_entity_info_data.field));
			if (iges_entity_info_data.fe_field)
			{
				DEACCESS(FE_field)(&iges_entity_info_data.fe_field);
			}
			if (iges_entity_info_data.element)
			{
				DEACCESS(FE_element)(&iges_entity_info_data.element);
			}
         for (i = 0 ; i < NUMBER_OF_NODES ; i++)
         {
				DEACCESS(FE_node)(&iges_entity_info_data.nodes[i]);
			}
			/* We no longer require the template element and nodes */
			/* directory entry section */
			entity=iges_entity_info_data.head;
			while (entity)
			{
				fprintf(iges,"%8d%8d%8d%8d%8d%8d%8d%8d%02d%02d%02d%02dD%7d\n",
					entity->type,entity->parameter_pointer,
					(entity->directory).structure,(entity->directory).line_font_pattern,
					(entity->directory).level,(entity->directory).view,
					(entity->directory).transformation_matrix,
					(entity->directory).label_display_associativity,
					(entity->directory).status.blank_status,
					(entity->directory).status.subordinate_entity_switch,
					(entity->directory).status.entity_use_flag,
					(entity->directory).status.hierarchy,
					entity->directory_pointer);
				fprintf(iges,"%8d%8d%8d%8d%8d%8s%8s%8s%8dD%7d\n",
					entity->type,(entity->directory).line_weight,
					(entity->directory).color,(entity->directory).parameter_line_count,
					(entity->directory).form," "," ",(entity->directory).label,
					(entity->directory).subscript_number,(entity->directory_pointer)+1);
				entity=entity->next;
			}
			/* parameter data section */
			entity=iges_entity_info_data.head;
			while (entity)
			{
				switch (entity->type)
				{
					case 102:
					{
						/* composite curve entity */
						parameter_pointer=entity->parameter_pointer;
						fprintf(iges,"%d,%d%n",entity->type,
							(entity->parameter).type_102.number_of_entities,&count);
						count=64-count;
						for (i=0;i<(entity->parameter).type_102.number_of_entities;i++)
						{
							fprintf(iges,",%d%n",
								((entity->parameter).type_102.directory_pointers)[i],
								&sub_count);
							count -= sub_count;
						}
						fprintf(iges,";");
						count--;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
					} break;
					case 110:
					{
						/* line entity */
						parameter_pointer=entity->parameter_pointer;
						fprintf(iges,"%d,%n",entity->type,&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
						fprintf(iges,"%.6e,%.6e,%.6e,%n",
							((entity->parameter).type_110.start)[0],
							((entity->parameter).type_110.start)[1],
							((entity->parameter).type_110.start)[2],&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						fprintf(iges,"%.6e,%.6e,%.6e;%n",
							((entity->parameter).type_110.end)[0],
							((entity->parameter).type_110.end)[1],
							((entity->parameter).type_110.end)[2],&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
					} break;
					case 112:
					{
						/* parametric spline curve entity */
						parameter_pointer=entity->parameter_pointer;
						fprintf(iges,"%d,%d,%d,%d,%d,%n",
							entity->type,(entity->parameter).type_112.spline_type,
							(entity->parameter).type_112.degree_of_continuity,
							(entity->parameter).type_112.number_of_dimensions,
							(entity->parameter).type_112.n,&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
						fprintf(iges,"%.6e,%.6e,%n",
							((entity->parameter).type_112.tu)[0],
							((entity->parameter).type_112.tu)[1],&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
						fprintf(iges,"%.6e,%.6e,%.6e,%.6e,%n",
							((entity->parameter).type_112.x)[0],
							((entity->parameter).type_112.x)[1],
							((entity->parameter).type_112.x)[2],
							((entity->parameter).type_112.x)[3],&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
						fprintf(iges,"%.6e,%.6e,%.6e,%.6e,%n",
							((entity->parameter).type_112.y)[0],
							((entity->parameter).type_112.y)[1],
							((entity->parameter).type_112.y)[2],
							((entity->parameter).type_112.y)[3],&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
						fprintf(iges,"%.6e,%.6e,%.6e,%.6e;%n",
							((entity->parameter).type_112.z)[0],
							((entity->parameter).type_112.z)[1],
							((entity->parameter).type_112.z)[2],
							((entity->parameter).type_112.z)[3],&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
					} break;
					case 114:
					{
						/* parametric spline surface entity */
						parameter_pointer=entity->parameter_pointer;
						fprintf(iges,"%d,%d,%d,%d,%d,%n",
							entity->type,(entity->parameter).type_114.spline_boundary_type,
							(entity->parameter).type_114.patch_type,
							(entity->parameter).type_114.m,(entity->parameter).type_114.n,
							&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
						fprintf(iges,"%.6e,%.6e,%.6e,%.6e,%n",
							((entity->parameter).type_114.tu)[0],
							((entity->parameter).type_114.tu)[1],
							((entity->parameter).type_114.tv)[0],
							((entity->parameter).type_114.tv)[1],&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
						for (i=0;i<16;i += 4)
						{
							fprintf(iges,"%.6e,%.6e,%.6e,%.6e,%n",
								((entity->parameter).type_114.x)[i],
								((entity->parameter).type_114.x)[i+1],
								((entity->parameter).type_114.x)[i+2],
								((entity->parameter).type_114.x)[i+3],&count);
							count=64-count;
							fprintf(iges,"%*s%8dP%7d\n",count," ",
								entity->directory_pointer,parameter_pointer);
							parameter_pointer++;
						}
						for (i=0;i<16;i += 4)
						{
							fprintf(iges,"%.6e,%.6e,%.6e,%.6e,%n",
								((entity->parameter).type_114.y)[i],
								((entity->parameter).type_114.y)[i+1],
								((entity->parameter).type_114.y)[i+2],
								((entity->parameter).type_114.y)[i+3],&count);
							count=64-count;
							fprintf(iges,"%*s%8dP%7d\n",count," ",
								entity->directory_pointer,parameter_pointer);
							parameter_pointer++;
						}
						for (i=0;i<16;i += 4)
						{
							fprintf(iges,"%.6e,%.6e,%.6e,%.6e%n",
								((entity->parameter).type_114.z)[i],
								((entity->parameter).type_114.z)[i+1],
								((entity->parameter).type_114.z)[i+2],
								((entity->parameter).type_114.z)[i+3],&count);
							if (12==i)
							{
								fprintf(iges,";");
							}
							else
							{
								fprintf(iges,",");
							}
							count++;
							count=64-count;
							fprintf(iges,"%*s%8dP%7d\n",count," ",
								entity->directory_pointer,parameter_pointer);
							parameter_pointer++;
						}
					} break;
					case 142:
					{
						/* curve on parametric surface entity */
						parameter_pointer=entity->parameter_pointer;
						fprintf(iges,"%d,%d,%d,%d,%d,%d;%n",entity->type,
							(entity->parameter).type_142.how_curve_created,
							(entity->parameter).type_142.surface_directory_pointer,
							(entity->parameter).type_142.material_curve_directory_pointer,
							(entity->parameter).type_142.world_curve_directory_pointer,
							(entity->parameter).type_142.preferred_representation,&count);
						count=64-count;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
					} break;
					case 144:
					{
						/* trimmed parametric surface entity */
						parameter_pointer=entity->parameter_pointer;
						fprintf(iges,"%d,%d,%d,%d,%d%n",entity->type,
							(entity->parameter).type_144.surface_directory_pointer,
							(entity->parameter).type_144.outer_boundary_type,
							(entity->parameter).type_144.number_of_inner_boundary_curves,
							(entity->parameter).type_144.outer_boundary_directory_pointer,
							&count);
						count=64-count;
						for (i=0;
							i<(entity->parameter).type_144.number_of_inner_boundary_curves;
							i++)
						{
							fprintf(iges,",%d%n",((entity->parameter).type_144.
								inner_boundary_directory_pointers)[i],&sub_count);
							count -= sub_count;
						}
						fprintf(iges,";");
						count--;
						fprintf(iges,"%*s%8dP%7d\n",count," ",
							entity->directory_pointer,parameter_pointer);
						parameter_pointer++;
					} break;
				}
				entity=entity->next;
			}
			/* terminate section */
			entity=iges_entity_info_data.tail;
			if (entity)
			{
				fprintf(iges,"S%7dG%7dD%7dP%7d%40sT%7d\n",1,global_count,
					(entity->directory_pointer)+1,(entity->parameter_pointer)+
					((entity->directory).parameter_line_count)-1," ",1);
			}
			while (iges_entity_info_data.head)
			{
				entity=iges_entity_info_data.head;
				iges_entity_info_data.head=entity->next;
				DEALLOCATE(entity);
			}
			fclose(iges);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_iges.  Could not open %s",
				file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_iges.  "
			"Invalid argument(s).  %p %p %p",file_name,fe_region,region_path);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* export_to_iges */
