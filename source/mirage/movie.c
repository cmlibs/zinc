/*******************************************************************************
FILE : movie.c

LAST MODIFIED : 28 April 2000

DESCRIPTION :
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "finite_element/import_finite_element.h"
#include "finite_element/export_finite_element.h"
#include "general/debug.h"
#include "general/image_utilities.h"
#include "general/object.h"
#include "general/mystring.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "graphics/texture.h"
#include "mirage/movie.h"
#include "mirage/mirage_node_editor.h"
#include "mirage/tracking_editor_data.h"
#include "user_interface/message.h"

#define MIRAGE_POINT_SIZE 2.0

/*
Module functions
----------------
*/

/*
Global functions
----------------
*/

struct Mirage_view *CREATE(Mirage_view)(void)
/*******************************************************************************
LAST MODIFIED : 23 April 1998

DESCRIPTION :
Allocates space for and initializes the Mirage_view structure.
==============================================================================*/
{
	struct Mirage_view *mirage_view;

	ENTER(CREATE(Mirage_view));
	if (ALLOCATE(mirage_view,struct Mirage_view,1)&&
		(mirage_view->placed_list=CREATE(LIST(Node_status))()))
	{
		mirage_view->name=(char *)NULL;
		mirage_view->image_file_name_template=(char *)NULL;
		mirage_view->number_of_nodes=0;
		mirage_view->node_numbers=(int *)NULL;
		mirage_view->com_radius=(int *)NULL;
		mirage_view->com_colour_indices=(int *)NULL;
		mirage_view->pending_nodes=(struct GROUP(FE_node) *)NULL;
		mirage_view->placed_nodes=(struct GROUP(FE_node) *)NULL;
		mirage_view->problem_nodes=(struct GROUP(FE_node) *)NULL;
		mirage_view->pending_elements=(struct GROUP(FE_element) *)NULL;
		mirage_view->placed_elements=(struct GROUP(FE_element) *)NULL;
		mirage_view->problem_elements=(struct GROUP(FE_element) *)NULL;
		mirage_view->scene=(struct Scene *)NULL;
		mirage_view->texture=(struct Texture *)NULL;
		mirage_view->element_group_manager=
			(struct MANAGER(GROUP(FE_element)) *)NULL;
		mirage_view->node_group_manager=(struct MANAGER(GROUP(FE_node)) *)NULL;
		mirage_view->scene_manager=(struct MANAGER(Scene) *)NULL;
		mirage_view->texture_manager=(struct MANAGER(Texture) *)NULL;
	}
	else
	{
		if (mirage_view)
		{
			DEALLOCATE(mirage_view);
		}
		display_message(ERROR_MESSAGE,
			"CREATE(Mirage_view).  Could not allocate space");
		mirage_view=(struct Mirage_view *)NULL;
	}
	LEAVE;

	return (mirage_view);
} /* CREATE(Mirage_view) */

int DESTROY(Mirage_view)(struct Mirage_view **mirage_view_address)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Cleans up space used by Mirage_view structure.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Mirage_view));
	if (mirage_view_address&&*mirage_view_address)
	{
		DESTROY(LIST(Node_status))(&((*mirage_view_address)->placed_list));
		if ((*mirage_view_address)->name)
		{
			DEALLOCATE((*mirage_view_address)->name);
		}
		if ((*mirage_view_address)->image_file_name_template)
		{
			DEALLOCATE((*mirage_view_address)->image_file_name_template);
		}
		if ((*mirage_view_address)->node_numbers)
		{
			DEALLOCATE((*mirage_view_address)->node_numbers);
		}
		if ((*mirage_view_address)->com_radius)
		{
			DEALLOCATE((*mirage_view_address)->com_radius);
		}
		if ((*mirage_view_address)->com_colour_indices)
		{
			DEALLOCATE((*mirage_view_address)->com_colour_indices);
		}
		/* remove elements first for the sake of graphical_finite_elements */
		if ((*mirage_view_address)->pending_elements)
		{
			DEACCESS(GROUP(FE_element))(
				&((*mirage_view_address)->pending_elements));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(
				(*mirage_view_address)->pending_elements,
				(*mirage_view_address)->element_group_manager);
		}
		if ((*mirage_view_address)->placed_elements)
		{
			DEACCESS(GROUP(FE_element))(
				&((*mirage_view_address)->placed_elements));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(
				(*mirage_view_address)->placed_elements,
				(*mirage_view_address)->element_group_manager);
		}
		if ((*mirage_view_address)->problem_elements)
		{
			DEACCESS(GROUP(FE_element))(
				&((*mirage_view_address)->problem_elements));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(
				(*mirage_view_address)->problem_elements,
				(*mirage_view_address)->element_group_manager);
		}
		if ((*mirage_view_address)->pending_nodes)
		{
			DEACCESS(GROUP(FE_node))(&((*mirage_view_address)->pending_nodes));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(
				(*mirage_view_address)->pending_nodes,
				(*mirage_view_address)->node_group_manager);
		}
		if ((*mirage_view_address)->placed_nodes)
		{
			DEACCESS(GROUP(FE_node))(&((*mirage_view_address)->placed_nodes));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(
				(*mirage_view_address)->placed_nodes,
				(*mirage_view_address)->node_group_manager);
		}
		if ((*mirage_view_address)->problem_nodes)
		{
			DEACCESS(GROUP(FE_node))(&((*mirage_view_address)->problem_nodes));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(
				(*mirage_view_address)->problem_nodes,
				(*mirage_view_address)->node_group_manager);
		}
		if ((*mirage_view_address)->scene)
		{
			DEACCESS(Scene)(&((*mirage_view_address)->scene));
			REMOVE_OBJECT_FROM_MANAGER(Scene)(
				(*mirage_view_address)->scene,
				(*mirage_view_address)->scene_manager);
		}
		if ((*mirage_view_address)->texture)
		{
			DEACCESS(Texture)(&((*mirage_view_address)->texture));
			REMOVE_OBJECT_FROM_MANAGER(Texture)(
				(*mirage_view_address)->texture,
				(*mirage_view_address)->texture_manager);
		}
		DEALLOCATE(*mirage_view_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Mirage_view).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Mirage_view) */

struct Mirage_movie *CREATE(Mirage_movie)(void)
/*******************************************************************************
LAST MODIFIED : 22 April 1999

DESCRIPTION :
Allocates space for and initializes the Mirage_movie structure.
==============================================================================*/
{
	struct Mirage_movie *mirage_movie;

	ENTER(CREATE(Mirage_movie));
	if (ALLOCATE(mirage_movie,struct Mirage_movie,1)&&
		(mirage_movie->placed_list=CREATE(LIST(Node_status))())&&
		(mirage_movie->pending_list=CREATE(LIST(Node_status))())&&
		(mirage_movie->problem_list=CREATE(LIST(Node_status))()))
	{
		mirage_movie->name=(char *)NULL;
		mirage_movie->working_directory_name=(char *)NULL;
		mirage_movie->all_node_file_name=(char *)NULL;
		mirage_movie->all_element_file_name=(char *)NULL;
		mirage_movie->all_node_group_name=(char *)NULL;
		mirage_movie->current_frame_no=0;
		mirage_movie->number_of_frames=0;
		mirage_movie->start_frame_no=0;
		mirage_movie->total_nodes=0;
		mirage_movie->node_file_name_template=(char *)NULL;
		mirage_movie->number_of_views=0;
		mirage_movie->views=(struct Mirage_view **)NULL;
		mirage_movie->pending_elements_3d=(struct GROUP(FE_element) *)NULL;
		mirage_movie->placed_elements_3d=(struct GROUP(FE_element) *)NULL;
		mirage_movie->problem_elements_3d=(struct GROUP(FE_element) *)NULL;
		mirage_movie->pending_nodes_3d=(struct GROUP(FE_node) *)NULL;
		mirage_movie->placed_nodes_3d=(struct GROUP(FE_node) *)NULL;
		mirage_movie->problem_nodes_3d=(struct GROUP(FE_node) *)NULL;
		mirage_movie->all_element_group=(struct GROUP(FE_element) *)NULL;
		mirage_movie->all_node_group=(struct GROUP(FE_node) *)NULL;
		mirage_movie->scene=(struct Scene *)NULL;
		mirage_movie->computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
		mirage_movie->element_manager=(struct MANAGER(FE_element) *)NULL;
		mirage_movie->element_group_manager=
			(struct MANAGER(GROUP(FE_element)) *)NULL;
		mirage_movie->fe_field_manager=(struct MANAGER(FE_field) *)NULL;
		mirage_movie->node_manager=(struct MANAGER(FE_node) *)NULL;
		mirage_movie->node_group_manager=(struct MANAGER(GROUP(FE_node)) *)NULL;
		mirage_movie->data_manager=(struct MANAGER(FE_node) *)NULL;
		mirage_movie->data_group_manager=(struct MANAGER(GROUP(FE_node)) *)NULL;
		mirage_movie->scene_manager=(struct MANAGER(Scene) *)NULL;
		mirage_movie->scene=(struct Scene *)NULL;
		mirage_movie->node_editor=(struct Mirage_node_editor *)NULL;
		mirage_movie->placed_points_material=(struct Graphical_material *)NULL;
		mirage_movie->pending_points_material=(struct Graphical_material *)NULL;
		mirage_movie->problem_points_material=(struct Graphical_material *)NULL;
		mirage_movie->lines_2d_material=(struct Graphical_material *)NULL;
		mirage_movie->lines_3d_material=(struct Graphical_material *)NULL;
		mirage_movie->surfaces_2d_material=(struct Graphical_material *)NULL;
		mirage_movie->surfaces_3d_material=(struct Graphical_material *)NULL;
		mirage_movie->modifiers = MIRAGE_MOVIE_MODIFIERS_NONE;
	}
	else
	{
		if (mirage_movie)
		{
			if (mirage_movie->placed_list)
			{
				DESTROY(LIST(Node_status))(&(mirage_movie->placed_list));
				DESTROY(LIST(Node_status))(&(mirage_movie->pending_list));
			}
			DEALLOCATE(mirage_movie);
		}
		display_message(ERROR_MESSAGE,
			"CREATE(Mirage_movie).  Could not allocate space");
		mirage_movie=(struct Mirage_movie *)NULL;
	}
	LEAVE;

	return (mirage_movie);
} /* CREATE(Mirage_movie) */

int DESTROY(Mirage_movie)(struct Mirage_movie **mirage_movie_address)
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Cleans up space used by Mirage_movie structure.
==============================================================================*/
{
	int return_code,view_no;
	struct Mirage_view **views,*destroy_view;
	struct Mirage_movie *movie;

	ENTER(DESTROY(Mirage_movie));
	if (mirage_movie_address&&(movie=*mirage_movie_address))
	{
		DESTROY(LIST(Node_status))(&(movie->placed_list));
		DESTROY(LIST(Node_status))(&(movie->pending_list));
		DESTROY(LIST(Node_status))(&(movie->problem_list));
		if (movie->node_editor)
		{
			DESTROY(Mirage_node_editor)(&(movie->node_editor));
		}
		if (movie->name)
		{
			DEALLOCATE(movie->name);
		}
		if (movie->working_directory_name)
		{
			DEALLOCATE(movie->working_directory_name);
		}
		if (movie->all_node_file_name)
		{
			DEALLOCATE(movie->all_node_file_name);
		}
		if (movie->all_element_file_name)
		{
			DEALLOCATE(movie->all_element_file_name);
		}
		if (movie->all_node_group_name)
		{
			DEALLOCATE(movie->all_node_group_name);
		}
		if (movie->node_file_name_template)
		{
			DEALLOCATE(movie->node_file_name_template);
		}
		if (views=movie->views)
		{
			for (view_no=movie->number_of_views;0<view_no;view_no--)
			{
				if (destroy_view=*views)
				{
					DESTROY(Mirage_view)(&destroy_view);
				}
				views++;
			}
			DEALLOCATE(movie->views);
		}
		/* must remove elements first for the sake of graphical finite elements */
		if (movie->pending_elements_3d)
		{
			DEACCESS(GROUP(FE_element))(
				&(movie->pending_elements_3d));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(
				movie->pending_elements_3d,movie->element_group_manager);
		}
		if (movie->placed_elements_3d)
		{
			DEACCESS(GROUP(FE_element))(
				&(movie->placed_elements_3d));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(
				movie->placed_elements_3d,movie->element_group_manager);
		}
		if (movie->problem_elements_3d)
		{
			DEACCESS(GROUP(FE_element))(&(movie->problem_elements_3d));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(
				movie->problem_elements_3d,movie->element_group_manager);
		}
		if (movie->pending_nodes_3d)
		{
			DEACCESS(GROUP(FE_node))(&(movie->pending_nodes_3d));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(
				movie->pending_nodes_3d,movie->node_group_manager);
		}
		if (movie->placed_nodes_3d)
		{
			DEACCESS(GROUP(FE_node))(&(movie->placed_nodes_3d));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(
				movie->placed_nodes_3d,movie->node_group_manager);
		}
		if (movie->problem_nodes_3d)
		{
			DEACCESS(GROUP(FE_node))(&(movie->problem_nodes_3d));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(
				movie->problem_nodes_3d,movie->node_group_manager);
		}
		/* deaccess materials */
		if (movie->placed_points_material)
		{
			DEACCESS(Graphical_material)(&(movie->placed_points_material));
		}
		if (movie->pending_points_material)
		{
			DEACCESS(Graphical_material)(&(movie->pending_points_material));
		}
		if (movie->problem_points_material)
		{
			DEACCESS(Graphical_material)(&(movie->problem_points_material));
		}
		if (movie->lines_2d_material)
		{
			DEACCESS(Graphical_material)(&(movie->lines_2d_material));
		}
		if (movie->lines_3d_material)
		{
			DEACCESS(Graphical_material)(&(movie->lines_3d_material));
		}
		if (movie->surfaces_2d_material)
		{
			DEACCESS(Graphical_material)(&(movie->surfaces_2d_material));
		}
		if (movie->surfaces_3d_material)
		{
			DEACCESS(Graphical_material)(&(movie->surfaces_3d_material));
		}
		/* remove all elements */
		if (movie->all_element_group)
		{
			DEACCESS(GROUP(FE_element))(&(movie->all_element_group));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(
				movie->all_element_group,movie->element_group_manager);
			REMOVE_ALL_OBJECTS_FROM_MANAGER(FE_element)(movie->element_manager);
		}
		/* remove all nodes */
		if (movie->all_node_group)
		{
			DEACCESS(GROUP(FE_node))(&(movie->all_node_group));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(
				movie->all_node_group,movie->node_group_manager);
			REMOVE_ALL_OBJECTS_FROM_MANAGER(FE_node)(movie->node_manager);
		}
		if (movie->scene)
		{
			DEACCESS(Scene)(&(movie->scene));
		}

		DEALLOCATE(*mirage_movie_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Mirage_movie).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Mirage_movie) */

static char *get_next_line(FILE *input_file)
/*******************************************************************************
LAST MODIFIED : 3 February 1998

DESCRIPTION :
Returns a string containing the next line in input_file not containing a ! as
the first character.
==============================================================================*/
{
	char *line;
	int next_line_found;

	ENTER(get_next_line);
	line=(char *)NULL;
	if (input_file)
	{
		next_line_found=0;
		while (!next_line_found)
		{
			if (feof(input_file))
			{
				next_line_found=1;
			}
			else
			{
				if (read_string(input_file,"[^\n]",&line))
				{
					if (line)
					{
						fscanf(input_file,"\n");
					}
					if ('!'==line[0])
					{
						DEALLOCATE(line);
					}
					else
					{
						next_line_found=1;
					}
				}
				else
				{
					if (line)
					{
						DEALLOCATE(line);
					}
					next_line_found=1;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_next_line.  Missing file");
	}
	LEAVE;

	return (line);
} /* get_next_line */

char *make_Mirage_file_name(char *template,int number)
/*******************************************************************************
LAST MODIFIED : 4 February 1998

DESCRIPTION :
Copies the template string and replaces the characters from the first to the
last # with the <number> as text with leading zeros.
eg. "imafish.#####.v1.rgb" with number 53 gives: "imafish.00053.v1.rgb".
Note: number of # characters must be less than 100!
==============================================================================*/
{
	char *file_name,format_string[10],*start_pos,*end_pos;
	int characters_allowed,characters_required;

	ENTER(make_Mirage_file_name);
	/* check arguments: especially that there is enough room to put the number */
	if (0<number)
	{
		characters_required=log10(number)+1;
	}
	else
	{
		characters_required=1;
	}
	if (template&&(0<=number)&&(start_pos=strchr(template,'#'))&&
		(end_pos=strrchr(template,'#')+1)&&
		((characters_allowed=(int)(end_pos-start_pos))<100)&&
		(characters_allowed>=characters_required))
	{
		if (ALLOCATE(file_name,char,strlen(template)+1))
		{
			sprintf(format_string,"%%0%dd",characters_allowed);
			strcpy(file_name,template);
			start_pos=strchr(file_name,'#');
			sprintf(start_pos,format_string,number);
			/* the above sprintf puts \0 at the end of the number. Hence need: */
			strcat(file_name,end_pos);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"make_Mirage_file_name.  Could not allocate space for file_name");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_Mirage_file_name.  Invalid argument(s)");
		file_name=(char *)NULL;
	}
	LEAVE;

	return (file_name);
} /* make_Mirage_file_name */

struct Mirage_movie *read_Mirage_movie(char *file_name)
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
Creates and fills a Mirage movie structure from file <file_name>.
==============================================================================*/
{
	char *line,*movie_name,*view_name;
	double dist_centre_x,dist_centre_y,dist_factor_k1;
	FILE *input_file;
	float com_factor;
	int number_of_views,view_no,row_no,node_no,i,number_of_nodes,*node_numbers,
		return_code,*com_colour_indices,com_colour_index,*com_radius,
		this_com_radius,use_factors;
	struct Mirage_movie *mirage_movie;
	struct Mirage_view **mirage_views,*view;
	struct Node_status *node_status;

	ENTER(read_Mirage_movie);
	/* check arguments */
	if (file_name)
	{
		if (input_file=fopen(file_name,"r"))
		{
			if (mirage_movie=CREATE(Mirage_movie)())
			{
				return_code=1;
				/* store the filename with the movie structure */
				if (return_code=(return_code&&
					ALLOCATE(movie_name,char,strlen(file_name)+1)))
				{
					strcpy(movie_name,file_name);
					mirage_movie->name=movie_name;
					/*???debug*/printf("movie_name = %s\n",movie_name);
				}
				/* read working directory */
				if (return_code=(return_code&&(line=get_next_line(input_file))))
				{
					mirage_movie->working_directory_name=line;
					/*???debug*/printf("Working directory name: %s\n",line);
				}
				/* read information about groups of all nodes and elements */
				if (return_code=(return_code&&(line=get_next_line(input_file))))
				{
					mirage_movie->all_node_file_name=line;
					/*???debug*/printf("All node file name: %s\n",line);
				}
				if (return_code=(return_code&&(line=get_next_line(input_file))))
				{
					mirage_movie->all_element_file_name=line;
					/*???debug*/printf("All element file name: %s\n",line);
				}
				if (return_code=(return_code&&(line=get_next_line(input_file))))
				{
					mirage_movie->all_node_group_name=line;
					/*???debug*/printf("All node group name: %s\n",line);
				}
				/* read the number of frames in the movie and the start frame no. */
				if (return_code=(return_code&&(line=get_next_line(input_file))))
				{
					sscanf(line,"%d %d",
						&(mirage_movie->number_of_frames),
						&(mirage_movie->start_frame_no));
					DEALLOCATE(line);
					/*???debug*/printf("frames %d %d\n",
						mirage_movie->number_of_frames,
						mirage_movie->start_frame_no);
					/* valid current_frame_no set in read_Mirage_movie_frame */
					mirage_movie->current_frame_no=mirage_movie->start_frame_no-1;
					return_code=(0<mirage_movie->number_of_frames);
				}
				/* read node file name template */
				if (return_code=(return_code&&(line=get_next_line(input_file))))
				{
					mirage_movie->node_file_name_template=line;
					/*???debug*/printf("node file format: %s\n",line);
				}
				/* read total number of nodes that can be tracked (not used) */
				if (return_code=(return_code&&(line=get_next_line(input_file))))
				{
					sscanf(line,"%d",&(mirage_movie->total_nodes));
					DEALLOCATE(line);
					/*???debug*/printf("total nodes to track: %d\n",
						mirage_movie->total_nodes);
				}
				/* read global centre of mass filter information */
				if (return_code=(return_code&&(line=get_next_line(input_file))))
				{
					sscanf(line,"%d %lf %d %lf",
						&(mirage_movie->com_radius1),&(mirage_movie->com_threshold1),
						&(mirage_movie->com_radius2),&(mirage_movie->com_threshold2));
					DEALLOCATE(line);
					/*???debug*/
					printf("Centre of Mass info: (1) %d %f  (2) %d %f\n",
						mirage_movie->com_radius1,mirage_movie->com_threshold1,
						mirage_movie->com_radius2,mirage_movie->com_threshold2);
					if ((mirage_movie->com_radius1<mirage_movie->com_radius2)||
						(0>mirage_movie->com_radius2)||
						(mirage_movie->com_threshold1<mirage_movie->com_threshold2)||
						(0.0>mirage_movie->com_threshold2))
					{
						printf("Invalid centre of mass info\n");
						return_code=0;
					}
				}

				/* read view information */
				if (return_code=(return_code&&(line=get_next_line(input_file))))
				{
					sscanf(line,"%d",&number_of_views);
					DEALLOCATE(line);
					/*???debug*/printf("number_of_views = %d\n",number_of_views);
					if ((0<number_of_views)&&
						ALLOCATE(mirage_views,struct Mirage_view *,number_of_views))
					{
						mirage_movie->number_of_views=number_of_views;
						for (view_no=0;view_no<number_of_views;view_no++)
						{
							mirage_views[view_no]=(struct Mirage_view *)NULL;
						}
						mirage_movie->views=mirage_views;
						for (view_no=0;return_code&&(view_no<number_of_views);view_no++)
						{
							if (view=mirage_views[view_no]=CREATE(Mirage_view)())
							{
								/* Give the view a text name from its number + 1 */
								if (return_code=(return_code&&
									ALLOCATE(view_name,char,3+(int)log10(view_no+1))))
								{
									sprintf(view_name,"%d",view_no+1);
									view->name=view_name;
									/*???debug*/printf("view: %s\n",view_name);
								}
								/* read image file name template */
								if (return_code=(return_code&&(line=get_next_line(input_file))))
								{
									view->image_file_name_template=line;
									/*???debug*/printf("Image file format: %s\n",line);
								}
								/* read radial distortion correction information. Note that
									I now calculate values for each image from this and the
									image size and placement stuff later */
								/* these distortion values refer to positions and K-factors
									 on the image placement area, not the original or cropped
									 texture. That way the same K-factors can be applied to
									 non-square-texel textures for which the placement position
									 is adjusted to achieve the correct image shape */
								if (return_code=(return_code&&(line=get_next_line(input_file))))
								{
									sscanf(line,"%lf %lf %lf",
										&dist_centre_x,&dist_centre_y,&dist_factor_k1);
									/* store these values for later writing to the movie file */
									view->dist_centre_x=dist_centre_x;
									view->dist_centre_y=dist_centre_y;
									view->dist_factor_k1=dist_factor_k1;
									DEALLOCATE(line);
									/*???debug*/printf("Distortion_info: %f %f %12e\n",
										dist_centre_x,dist_centre_y,dist_factor_k1);
								}

								/* read image cropping information for even frames */
								if (return_code=(return_code&&(line=get_next_line(input_file))))
								{
									sscanf(line,"%d %d %d %d",
										&(view->crop0_left),&(view->crop0_bottom),
										&(view->crop0_width),&(view->crop0_height));
									DEALLOCATE(line);
									/*???debug*/printf("Crop0_info: %d %d %d %d\n",
										view->crop0_left,view->crop0_bottom,
										view->crop0_width,view->crop0_height);
								}
								/* read image cropping information for odd frames */
								if (return_code=(return_code&&(line=get_next_line(input_file))))
								{
									sscanf(line,"%d %d %d %d",
										&(view->crop1_left),&(view->crop1_bottom),
										&(view->crop1_width),&(view->crop1_height));
									DEALLOCATE(line);
									/*???debug*/printf("Crop0_info: %d %d %d %d\n",
										view->crop1_left,view->crop1_bottom,
										view->crop1_width,view->crop1_height);
								}
								/* read image placement information for even frames */
								if (return_code=(return_code&&(line=get_next_line(input_file))))
								{
									sscanf(line,"%lf %lf %lf %lf",
										&(view->image0_left),&(view->image0_bottom),
										&(view->image0_width),&(view->image0_height));
									DEALLOCATE(line);
									/*???debug*/printf("Image0_info: %f %f %f %f\n",
										view->image0_left,view->image0_bottom,
										view->image0_width,view->image0_height);
								}
								/* read image placement information for odd frames */
								if (return_code=(return_code&&(line=get_next_line(input_file))))
								{
									sscanf(line,"%lf %lf %lf %lf",
										&(view->image1_left),&(view->image1_bottom),
										&(view->image1_width),&(view->image1_height));
									DEALLOCATE(line);
									/*???debug*/printf("Image0_info: %f %f %f %f\n",
										view->image1_left,view->image1_bottom,
										view->image1_width,view->image1_height);
								}
#if defined (OLD_CODE)
								/* get centre of distortion and k1 on small images */
								enlargement_ratio_x = view->image0_width / view->crop0_width;
								enlargement_ratio_y = view->image0_height / view->crop0_height;
								view->dist0_centre_x = -0.5 + view->crop0_left +
									(dist_centre_x - view->image0_left)/enlargement_ratio_x;
								view->dist0_centre_y = -0.5 + view->crop0_bottom +
									(dist_centre_y - view->image0_bottom)/enlargement_ratio_y;
								view->dist0_factor_k1 =
									dist_factor_k1*enlargement_ratio_y*enlargement_ratio_y;
								/*???debug*/printf("Distortion0_info: %f %f %12e\n",
									view->dist0_centre_x,view->dist0_centre_y,
									view->dist0_factor_k1);

								/* get centre of distortion and k1 on small images */
								enlargement_ratio_x = view->image1_width / view->crop1_width;
								enlargement_ratio_y = view->image1_height / view->crop1_height;
								view->dist1_centre_x = -0.5 + view->crop1_left +
									(dist_centre_x - view->image1_left)/enlargement_ratio_x;
								view->dist1_centre_y = -0.5 + view->crop1_bottom +
									(dist_centre_y - view->image1_bottom)/enlargement_ratio_y;
								view->dist1_factor_k1 =
									dist_factor_k1*enlargement_ratio_y*enlargement_ratio_y;
								/*???debug*/printf("Distortion1_info: %f %f %12e\n",
									view->dist1_centre_x,view->dist1_centre_y,
									view->dist1_factor_k1);
#endif /* defined (OLD_CODE) */

								/* read NDC_information */
								if (return_code=(return_code&&(line=get_next_line(input_file))))
								{
									sscanf(line,"%lf %lf %lf %lf",
										&(view->NDC_left),&(view->NDC_bottom),
										&(view->NDC_width),&(view->NDC_height));
									DEALLOCATE(line);
									/*???debug*/printf("NDC_info: %f %f %f %f\n",
										view->NDC_left,view->NDC_bottom,
										view->NDC_width,view->NDC_height);
								}
								/* read near and far clipping planes */
								if (return_code=(return_code&&
									(line=get_next_line(input_file))))
								{
									sscanf(line,"%lf %lf",
										&(view->near_clipping_plane),
										&(view->far_clipping_plane));
									DEALLOCATE(line);
									/*???debug*/printf("%f %f\n",
										view->near_clipping_plane,
										view->far_clipping_plane);
								}
								/* read in 4x3 Photogrammetry transformation matrix */
								/* each row of 3 is on a separate line */
								for (row_no=0;return_code&&(4>row_no);row_no++)
								{
									if (return_code=(return_code&&
										(line=get_next_line(input_file))))
									{
										sscanf(line,"%lf %lf %lf",
											&(view->transformation43[row_no*3]),
											&(view->transformation43[row_no*3+1]),
											&(view->transformation43[row_no*3+2]));
										DEALLOCATE(line);
										/*???debug*/printf("%f %f %f\n",
											view->transformation43[row_no*3],
											view->transformation43[row_no*3+1],
											view->transformation43[row_no*3+2]);
									}
								}
								/* read in the nodes to be placed in this view */
								if (return_code=(return_code&&(line=get_next_line(input_file))))
								{
									use_factors=(1==sscanf(line,"%d %d",&number_of_nodes,&this_com_radius));
									printf("use com factors=%i\n",use_factors);
									DEALLOCATE(line);
									/*???debug*/printf("number_of_nodes = %d\n",number_of_nodes);
									node_numbers=(int *)NULL;
									if ((0<number_of_nodes)&&
										ALLOCATE(node_numbers,int,number_of_nodes)&&
										ALLOCATE(com_radius,int,number_of_nodes)&&
										ALLOCATE(com_colour_indices,int,number_of_nodes))
									{
										view->number_of_nodes=number_of_nodes;
										view->node_numbers=node_numbers;
										view->com_radius=com_radius;
										view->com_colour_indices=com_colour_indices;
										for (i=0;return_code&&(i<number_of_nodes);i++)
										{
											if (return_code=((line=get_next_line(input_file))&&
												(3==sscanf(line,"%d %f %d",&node_no,&com_factor,
													&com_colour_index))))
											{
												DEALLOCATE(line);
												view->node_numbers[i]=node_no;
												if (use_factors)
												{
													view->com_radius[i]=(int)(com_factor*mirage_movie->com_radius2);
												}
												else
												{
													view->com_radius[i]=(int)(com_factor+0.1);
												}
												view->com_colour_indices[i]=com_colour_index;
												/*???debug*//*printf("%d %d %d\n",node_no,com_factor,
													com_colour_index);*/
												/* make entries in Node_status lists */
												if (node_status=CREATE(Node_status)(node_no))
												{
													if (!ADD_OBJECT_TO_LIST(Node_status)(
														node_status,view->placed_list))
													{
														DESTROY(Node_status)(&node_status);
														return_code=0;
													}
												}
												else
												{
													return_code=0;
												}
												/* ensure entry is in movie placed list */
												if (return_code&&
													!FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
													node_no,mirage_movie->placed_list))
												{
													if (node_status=CREATE(Node_status)(node_no))
													{
														if (!ADD_OBJECT_TO_LIST(Node_status)(
															node_status,mirage_movie->placed_list))
														{
															DESTROY(Node_status)(&node_status);
															return_code=0;
														}
													}
													else
													{
														return_code=0;
													}
												}
												/* ensure entry is in movie pending list */
												if (return_code&&
													!FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
													node_no,mirage_movie->pending_list))
												{
													if (node_status=CREATE(Node_status)(node_no))
													{
														if (!ADD_OBJECT_TO_LIST(Node_status)(
															node_status,mirage_movie->pending_list))
														{
															DESTROY(Node_status)(&node_status);
															return_code=0;
														}
													}
													else
													{
														return_code=0;
													}
												}
												/* ensure entry is in movie problem list */
												if (return_code&&
													!FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
													node_no,mirage_movie->problem_list))
												{
													if (node_status=CREATE(Node_status)(node_no))
													{
														if (!ADD_OBJECT_TO_LIST(Node_status)(
															node_status,mirage_movie->problem_list))
														{
															DESTROY(Node_status)(&node_status);
															return_code=0;
														}
													}
													else
													{
														return_code=0;
													}
												}
											}
										}
									}
									else
									{
										if (node_numbers)
										{
											if (com_radius)
											{
												DEALLOCATE(com_radius);
											}
											DEALLOCATE(node_numbers);
										}
										display_message(ERROR_MESSAGE,
											"read_Mirage_movie.  Invalid node numbers");
										return_code=0;
									}
								}
							}
							else
							{
								return_code=0;
							}
						} /* end of views loop */
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_Mirage_movie.  Invalid views");
						return_code=0;
					}
				}
				if (return_code)
				{
					return_code=Mirage_movie_read_node_status_lists(mirage_movie,"");
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"read_Mirage_movie.  Error reading movie: destroying");
					DESTROY(Mirage_movie)(&mirage_movie);
				}
			}
			fclose(input_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_Mirage_movie.  Could not open file %s",file_name);
			mirage_movie=(struct Mirage_movie *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_Mirage_movie.  Invalid argument(s)");
		mirage_movie=(struct Mirage_movie *)NULL;
	}
	LEAVE;

	return (mirage_movie);
} /* read_Mirage_movie */

int write_Mirage_movie(struct Mirage_movie *movie,char *extra_extension)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Writes mirage_movie to its file name with the extra_extension added on the end.
If the extra_extension is the empty string "" it first writes the movie to
the movie's file name with the string "_tmp" appended on the end,
then it uses a sys "cp" command to overwrite the actual file.
If extra_extension is not an empty string, the file is written normally.
==============================================================================*/
{
#define VERIFY_NON_NEGATIVE(func) return_code=(return_code&&(0<=(func)))
	char *file_name,*sys_command;
	FILE *out_file;
	int error_code,i,return_code,view_no;
	struct Mirage_view *view;

	ENTER(write_Mirage_movie);
	if (movie&&movie->name&&extra_extension)
	{
		if (ALLOCATE(file_name,char,strlen(movie->name)+strlen(extra_extension)+5)&&
			ALLOCATE(sys_command,char,(strlen(movie->name)+strlen(extra_extension)+5)*2+5))
		{
			strcpy(file_name,movie->name);
			if (0==strlen(extra_extension))
			{
				strcat(file_name,"_tmp");
			}
			else
			{
				strcat(file_name,extra_extension);
			}
			if (out_file=fopen(file_name,"w"))
			{
				return_code=1;
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!CMGUI/Mirage movie tracking description file\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"! Working directory (for XVG)\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"%s\n",movie->working_directory_name));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!File containing base group of all nodes\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"%s\n",movie->all_node_file_name));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!File containing base group of all elements\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"%s\n",movie->all_element_file_name));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!name of base group containing all nodes (also elements)\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"%s\n",movie->all_node_group_name));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!Number of frames and first frame number\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"%i %i\n",movie->number_of_frames,movie->start_frame_no));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!Frame node filename template\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"%s\n",movie->node_file_name_template));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!Total number of nodes to track (may be less than number in base exnode file):\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"%i\n",movie->total_nodes));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!  Global centre of mass parameters. The actual centre of mass radius used\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!  is given with each node listed under each view. A zero radius turns it\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!  off while 1 gives 3x3 pixels etc. Centre of mass is done in 2 passes with a\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!  different threshold on intensity in each. (A zero threshold uses the median\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!  value while 1.0 uses only the most intense pixel(s) for that colour index.)\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!  The first pass uses at least Std_CoM_Radius and Com_Threshold1 while the\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!  second uses CoM_Threshold2. The program automatically calculates CoM radii\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!  based on Std_CoM_Radius and no greater than Max_CoM_Radius.\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!Max_CoM_Radius CoM_Threshold1 Std_CoM_Radius CoM_Threshold2\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"%i %.3f %i %.3f\n",movie->com_radius1,movie->com_threshold1,
					movie->com_radius2,movie->com_threshold2));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!Definition of camera views\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!Number of views\n"));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"%i\n",movie->number_of_views));
				VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
				for (view_no=0;return_code&&(view_no < movie->number_of_views);view_no++)
				{
					if (movie->views&&(view=movie->views[view_no])&&
						(NULL!=view->node_numbers)&&(NULL!=view->com_radius)&&
						(NULL!=view->com_colour_indices))
					{
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!View %i\n",view_no+1));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!Image filename template\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"%s\n",view->image_file_name_template));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
						/* verbose descriptions only put out with first view */
						if (0==view_no)
						{
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  The following lines supply the parameters for correcting barrel/pincushion\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  distortion in the raw images so that they correctly line up with the\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  projected 3-D coordinates. The values are the positions and correction\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  factors for the space that the images are projected on to. The program\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  automatically calculates the centres and factors needed for each image,\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  whatever their size and placement position.\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  !!! NOTE !!!\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  At present these calculations rely on the aspect ratio of both the\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  images and the area they are placed onto being of the same aspect ratio!\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
						}
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!Radial distortion centre and correction factor:\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!distortion_centreX distortion_centreY distortion_factor_k1\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"%g %g %12e\n",view->dist_centre_x,view->dist_centre_y,view->dist_factor_k1));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
						if (0==view_no)
						{
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  The following lines specify which part of the above image to crop for use\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  in this view. Different crop values are permitted in even and odd frames.\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  Making crop width or height powers of 2 reduces texture memory required.\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  Specify zero crop width or height to use the whole original image.\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
						}
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!Image cropping parameters for even frame numbers:\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!crop_left crop_bottom crop_width crop_height\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"%i %i %i %i\n",
							view->crop0_left,view->crop0_bottom,view->crop0_width,view->crop0_height));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!Image cropping parameters for odd frame numbers:\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!crop_left crop_bottom crop_width crop_height\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"%i %i %i %i\n",
							view->crop1_left,view->crop1_bottom,view->crop1_width,view->crop1_height));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
						if (0==view_no)
						{
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  The following lines specify where the cropped image is placed in this view\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  by its bottom, left position and the width and height you would like it to\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  be. Making the height twice that in the cropped image will stretch it\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  accordingly, and similarly with the width.\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  There are 2 lines specifying the same information. They will be different\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  only if the images are interlaced so that every second frame is displaced\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  vertically by half a texel from the former frame.\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  Images from odd/upper scan lines of interlaced images take place 1/60 second\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  before even/lower scan lines. Currently we give the odd/upper scan lines a\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  frame number twice that of the original frame no, while frame numbers for\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  lower scan lines will be one higher than that. For compatibility with the\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  standard we end up using, which parameters to use are specified according to\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  whether the final frame number is even or odd, with even frame numbers first.\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
						}
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!Image placement parameters for even frame numbers\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!image_left image_bottom image_width image_height\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"%g %g %g %g\n",
							view->image0_left,view->image0_bottom,view->image0_width,view->image0_height));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!Image placement parameters for odd frame numbers\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!image_left image_bottom image_width image_height\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"%g %g %g %g\n",
							view->image1_left,view->image1_bottom,view->image1_width,view->image1_height));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
						if (0==view_no)
						{
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  The following parameters define the region of interest for digitising off\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  this view. It does not have to be exact, but merely helps the program find\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  the appropriate place. Internally, they are taken as the position and range\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!  of the normalised device coordinates for projection.\n"));
							VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
						}
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!NDC_left NDC_bottom NDC_width NDC_height\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"%g %g %g %g\n",
							view->NDC_left,view->NDC_bottom,view->NDC_width,view->NDC_height));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!near_clipping_plane far_clipping_plane (distance from eye)\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"%g %g\n",view-> near_clipping_plane,view->far_clipping_plane));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!PTM\n"));
						for (i=0;return_code&&(i<4);i++)
						{
							VERIFY_NON_NEGATIVE(fprintf(out_file,"%12e %12e %12e\n",view->transformation43[i*3],
								view->transformation43[i*3+1],view->transformation43[i*3+2]));
						}
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!number of nodes to be placed in this view\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"%i 1\n",view->number_of_nodes));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!nodes to be placed in this view in placement order,\n"));
						VERIFY_NON_NEGATIVE(fprintf(out_file,"!  centre of mass radius, colour index (0=black,1=white)\n"));
						for (i=0;return_code&&(i<view->number_of_nodes);i++)
						{
							VERIFY_NON_NEGATIVE(fprintf(out_file,"%i %i %i\n",
								view->node_numbers[i],view->com_radius[i],view->com_colour_indices[i]));
						}
					}
					else
					{
						return_code=0;
					}
				} /* view loop */
				fclose(out_file);
				if (return_code)
				{
					if (0==strlen(extra_extension))
					{
						/* copy temp movie file to actual movie file */
						sprintf(sys_command,"cp %s %s",file_name,movie->name);
						if (error_code=system(sys_command))
						{
							display_message(ERROR_MESSAGE,"write_Mirage_movie.  "
								"Error code %d encountered when copying to %s",error_code,movie->name);
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_Mirage_movie.  Error writing file");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_Mirage_movie.  Could not open file %s",file_name);
				return_code=0;
			}
			DEALLOCATE(file_name);
			DEALLOCATE(sys_command);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_Mirage_movie.  Could not allocate strings");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_Mirage_movie.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_Mirage_movie */

int Mirage_movie_read_node_status_lists(struct Mirage_movie *movie,
	char *extra_extension)
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Clears and reads the node_status_lists for <movie>. The filename for each list
is the movie name with a predefined extension added, eg. "_pending" for the
pending list and "_view1" for a view named "1". The <extra_extension> is added
after that to enable temporary saves of these lists to be restored as an undo
function.
==============================================================================*/
{
	char *file_name;
	int return_code,view_no;
	struct Mirage_view *view;

	ENTER(Mirage_movie_read_node_status_lists);
	/* check arguments */
	if (movie&&movie->name&&extra_extension)
	{
		if (ALLOCATE(file_name,char,strlen(movie->name)+strlen(extra_extension)+10))
		{
			return_code=1;
			/*???RC.  Error if files not there? */
			sprintf(file_name,"%s_placed%s",movie->name,extra_extension);
			Node_status_list_clear(movie->placed_list);
			Node_status_list_read(movie->placed_list,file_name);
			sprintf(file_name,"%s_pending%s",movie->name,extra_extension);
			Node_status_list_clear(movie->pending_list);
			Node_status_list_read(movie->pending_list,file_name);
			sprintf(file_name,"%s_problem%s",movie->name,extra_extension);
			Node_status_list_clear(movie->problem_list);
			Node_status_list_read(movie->problem_list,file_name);
			for (view_no=0;view_no<movie->number_of_views;view_no++)
			{
				if (view=movie->views[view_no])
				{
					sprintf(file_name,"%s_view%s%s",movie->name,view->name,
						extra_extension);
					Node_status_list_clear(view->placed_list);
					Node_status_list_read(view->placed_list,file_name);
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Mirage_movie_read_node_status_lists.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_read_node_status_lists */

int Mirage_movie_write_node_status_lists(struct Mirage_movie *movie,
	char *extra_extension)
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Writes the node_status_lists for <movie>. The filename for each list
is the movie name with a predefined extension added, eg. "_pending" for the
pending list and "_view1" for a view named "1". The <extra_extension> is added
after that to enable temporary saves of these lists to be later restored as an
undo function.
==============================================================================*/
{
	char *file_name;
	static char header_text[]="!Op\tNode\tStart\tStop\n";
	static char line_format[]="?\t%d\t%d\t%d\n";
	int return_code,view_no;
	struct Mirage_view *view;

	ENTER(Mirage_movie_write_node_status_lists);
	/* check arguments */
	if (movie&&movie->name&&extra_extension)
	{
		if (ALLOCATE(file_name,char,strlen(movie->name)+strlen(extra_extension)+10))
		{
			return_code=1;
			if (return_code)
			{
				sprintf(file_name,"%s_placed%s",movie->name,extra_extension);
				return_code=Node_status_list_write(movie->placed_list,file_name,
					header_text,line_format);
			}
			if (return_code)
			{
				sprintf(file_name,"%s_pending%s",movie->name,extra_extension);
				return_code=Node_status_list_write(movie->pending_list,file_name,
					header_text,line_format);
			}
			if (return_code)
			{
				sprintf(file_name,"%s_problem%s",movie->name,extra_extension);
				return_code=Node_status_list_write(movie->problem_list,file_name,
					header_text,line_format);
			}
			for (view_no=0;return_code&&(view_no<movie->number_of_views);view_no++)
			{
				if (view=movie->views[view_no])
				{
					sprintf(file_name,"%s_view%s%s",movie->name,view->name,
						extra_extension);
					return_code=Node_status_list_write(view->placed_list,file_name,
						header_text,line_format);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Mirage_movie_write_node_status_lists.  Invalid view");
					return_code=0;
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Mirage_movie_write_node_status_lists.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_write_node_status_lists */

static int Mirage_movie_graphics_show_points(struct Scene *scene,
	struct GROUP(FE_element) *element_group,struct Graphical_material *material,
	struct GT_object *glyph,float point_size,struct Computed_field *label_field)
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Adds points in the given <material>, possibly with node numbers on to the
graphical finite element for <element_group> on <scene>.
Regenerates the GFE but does not update the scene.
==============================================================================*/
{
	int return_code;
	struct Computed_field *orientation_scale_field;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;
	Triple glyph_centre,glyph_size,glyph_scale_factors;

	ENTER(Mirage_movie_graphics_make_points);
	if (scene&&element_group&&material&&glyph&&(0.0<=point_size))
	{
		if (gt_element_group=Scene_get_graphical_element_group(scene,element_group))
		{
			if (settings=CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_NODE_POINTS))
			{
				GT_element_settings_set_material(settings,material);
				GT_element_settings_set_select_mode(settings,GRAPHICS_SELECT_ON);
				glyph_centre[0]=0.0;
				glyph_centre[1]=0.0;
				glyph_centre[2]=0.0;
				glyph_size[0]=point_size;
				glyph_size[1]=point_size;
				glyph_size[2]=point_size;
				orientation_scale_field=(struct Computed_field *)NULL;
				glyph_scale_factors[0]=1.0;
				glyph_scale_factors[1]=1.0;
				glyph_scale_factors[2]=1.0;
				GT_element_settings_set_glyph_parameters(settings,glyph,
					glyph_centre,glyph_size,orientation_scale_field,glyph_scale_factors);
				GT_element_settings_set_label_field(settings,label_field);
				if (GT_element_group_add_settings(gt_element_group,settings,1))
				{
					GT_element_group_build_graphics_objects(gt_element_group,
						(struct FE_element *)NULL,(struct FE_node *)NULL);
					return_code=1;
				}
				else
				{
					DESTROY(GT_element_settings)(&settings);
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
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_show_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_show_points */

static int Mirage_movie_graphics_show_lines(struct Scene *scene,
	struct GROUP(FE_element) *element_group,struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Adds lines in the given <material> to the graphical finite element for
<element_group> on <scene>. Regenerates the GFE but does not update the scene.
==============================================================================*/
{
	int return_code;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;

	ENTER(Mirage_movie_graphics_make_lines);
	if (scene&&element_group&&material)
	{
		if (gt_element_group=Scene_get_graphical_element_group(scene,element_group))
		{
			if (settings=CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_LINES))
			{
				GT_element_settings_set_material(settings,material);
				if (GT_element_group_add_settings(gt_element_group,settings,2))
				{
					GT_element_group_build_graphics_objects(gt_element_group,
						(struct FE_element *)NULL,(struct FE_node *)NULL);
					return_code=1;
				}
				else
				{
					DESTROY(GT_element_settings)(&settings);
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
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_show_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_show_lines */

static int Mirage_movie_graphics_show_surfaces(struct Scene *scene,
	struct GROUP(FE_element) *element_group,struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Adds surfaces in the given <material> to the graphical finite element for
<element_group> on <scene>. Regenerates the GFE but does not update the scene.
==============================================================================*/
{
	int return_code;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;

	ENTER(Mirage_movie_graphics_make_surfaces);
	if (scene&&element_group&&material)
	{
		if (gt_element_group=Scene_get_graphical_element_group(scene,element_group))
		{
			if (settings=CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES))
			{
				GT_element_settings_set_material(settings,material);
				if (GT_element_group_add_settings(gt_element_group,settings,0))
				{
					GT_element_group_build_graphics_objects(gt_element_group,
						(struct FE_element *)NULL,(struct FE_node *)NULL);
					return_code=1;
				}
				else
				{
					DESTROY(GT_element_settings)(&settings);
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
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_show_surfaces.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_show_surfaces */

static int Mirage_movie_graphics_hide_points(struct Scene *scene,
	struct GROUP(FE_element) *element_group)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Removes all point settings from the graphical finite element for
<element_group> on <scene>. Regenerates the GFE but does not update the scene.
==============================================================================*/
{
	int return_code;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;

	ENTER(Mirage_movie_graphics_hide_points);
	if (scene&&element_group)
	{
		if (gt_element_group=Scene_get_graphical_element_group(scene,element_group))
		{
			return_code=1;
			while (return_code&&(settings=first_settings_in_GT_element_group_that(
				gt_element_group,GT_element_settings_type_matches,
				(void *)GT_ELEMENT_SETTINGS_NODE_POINTS)))
			{
				return_code=GT_element_group_remove_settings(gt_element_group,settings);
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_hide_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_hide_points */

static int Mirage_movie_graphics_hide_lines(struct Scene *scene,
	struct GROUP(FE_element) *element_group)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Removes all line settings from the graphical finite element for
<element_group> on <scene>. Regenerates the GFE but does not update the scene.
==============================================================================*/
{
	int return_code;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;

	ENTER(Mirage_movie_graphics_hide_lines);
	if (scene&&element_group)
	{
		if (gt_element_group=Scene_get_graphical_element_group(scene,element_group))
		{
			return_code=1;
			while (return_code&&(settings=first_settings_in_GT_element_group_that(
				gt_element_group,GT_element_settings_type_matches,
				(void *)GT_ELEMENT_SETTINGS_LINES)))
			{
				return_code=GT_element_group_remove_settings(gt_element_group,settings);
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_hide_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_hide_lines */

static int Mirage_movie_graphics_hide_surfaces(struct Scene *scene,
	struct GROUP(FE_element) *element_group)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Removes all surface settings from the graphical finite element for
<element_group> on <scene>. Regenerates the GFE but does not update the scene.
==============================================================================*/
{
	int return_code;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;

	ENTER(Mirage_movie_graphics_hide_surfaces);
	if (scene&&element_group)
	{
		if (gt_element_group=Scene_get_graphical_element_group(scene,element_group))
		{
			return_code=1;
			while (return_code&&(settings=first_settings_in_GT_element_group_that(
				gt_element_group,GT_element_settings_type_matches,
				(void *)GT_ELEMENT_SETTINGS_SURFACES)))
			{
				return_code=GT_element_group_remove_settings(gt_element_group,settings);
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_hide_surfaces.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_hide_surfaces */

int enable_Mirage_movie_graphics(struct Mirage_movie *movie,
	struct MANAGER(FE_basis) *basis_manager,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_graphical_material,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct MANAGER(Scene) *scene_manager,
	struct Scene *default_scene,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
From an already-created movie - eg. read in from read_Mirage_movie - creates
the node and element groups, textures and scenes for each view and for the
resulting 3-D display.
==============================================================================*/
{
	char *temp_name,temp_name2[20];
	int return_code,view_no;
	static struct Colour
		placed_colour={0,1,0},
		problem_colour={0,0,1},
		pending_colour={0,1,1},
		lines_2d_colour={0,0,0},
		lines_3d_colour={1,1,1},
		surfaces_ambient={0,0,0},
		surfaces_diffuse={0.4,0.6,1},
		surfaces_specular={1,1,1};
	MATERIAL_PRECISION surfaces_shininess=0.5;
	MATERIAL_PRECISION surfaces_2d_alpha=0.5;
	struct Element_discretization minimum_discretization={2,2,2};
	struct File_read_FE_element_group_data *read_element_data;
	struct File_read_FE_node_group_data *read_node_data;
	struct Graphical_material *temp_material;
	struct GT_element_group *gt_element_group;
	struct Mirage_view *view;
	Triple axis_lengths;

	ENTER(enable_Mirage_movie_graphics);
	if (movie&&element_manager&&element_group_manager&&fe_field_manager&&
		glyph_list&&graphical_material_manager&&default_graphical_material&&
		light_manager&&node_manager&&node_group_manager&&data_manager&&
		data_group_manager&&element_point_ranges_selection&&
		element_selection&&node_selection&&data_selection&&
		scene_manager&&default_scene&&spectrum_manager&&
		default_spectrum&&texture_manager&&user_interface&&computed_field_manager)
	{
		return_code=1;
		movie->computed_field_manager=computed_field_manager;
		movie->element_manager=element_manager;
		movie->element_group_manager=element_group_manager;
		movie->fe_field_manager=fe_field_manager;
		movie->glyph_list=glyph_list;
		movie->node_manager=node_manager;
		movie->node_group_manager=node_group_manager;
		movie->data_manager=data_manager;
		movie->data_group_manager=data_group_manager;
		movie->scene_manager=scene_manager;
		movie->scene=ACCESS(Scene)(default_scene);
		/* create materials for points, lines and surfaces */
		if (return_code)
		{
			if (!(temp_material=FIND_BY_IDENTIFIER_IN_MANAGER(
				Graphical_material,name)("placed_points",graphical_material_manager)))
			{
				if (!((temp_material=CREATE(Graphical_material)("placed_points"))&&
					Graphical_material_set_ambient(temp_material,&placed_colour)&&
					Graphical_material_set_diffuse(temp_material,&placed_colour)&&
					ADD_OBJECT_TO_MANAGER(Graphical_material)(temp_material,
						graphical_material_manager)))
				{
					DESTROY(Graphical_material)(&temp_material);
					return_code=0;
				}
			}
			if (temp_material)
			{
				movie->placed_points_material=ACCESS(Graphical_material)(temp_material);
			}
		}
		if (return_code)
		{
			if (!(temp_material=FIND_BY_IDENTIFIER_IN_MANAGER(
				Graphical_material,name)("pending_points",graphical_material_manager)))
			{
				if (!((temp_material=CREATE(Graphical_material)("pending_points"))&&
					Graphical_material_set_ambient(temp_material,&pending_colour)&&
					Graphical_material_set_diffuse(temp_material,&pending_colour)&&
					ADD_OBJECT_TO_MANAGER(Graphical_material)(temp_material,
						graphical_material_manager)))
				{
					DESTROY(Graphical_material)(&temp_material);
					return_code=0;
				}
			}
			if (temp_material)
			{
				movie->pending_points_material=
					ACCESS(Graphical_material)(temp_material);
			}
		}
		if (return_code)
		{
			if (!(temp_material=FIND_BY_IDENTIFIER_IN_MANAGER(
				Graphical_material,name)("problem_points",graphical_material_manager)))
			{
				if (!((temp_material=CREATE(Graphical_material)("problem_points"))&&
					Graphical_material_set_ambient(temp_material,&problem_colour)&&
					Graphical_material_set_diffuse(temp_material,&problem_colour)&&
					ADD_OBJECT_TO_MANAGER(Graphical_material)(temp_material,
						graphical_material_manager)))
				{
					DESTROY(Graphical_material)(&temp_material);
					return_code=0;
				}
			}
			if (temp_material)
			{
				movie->problem_points_material=
					ACCESS(Graphical_material)(temp_material);
			}
		}
		if (return_code)
		{
			if (!(temp_material=FIND_BY_IDENTIFIER_IN_MANAGER(
				Graphical_material,name)("lines_2d",graphical_material_manager)))
			{
				if (!((temp_material=CREATE(Graphical_material)("lines_2d"))&&
					Graphical_material_set_ambient(temp_material,&lines_2d_colour)&&
					Graphical_material_set_diffuse(temp_material,&lines_2d_colour)&&
					ADD_OBJECT_TO_MANAGER(Graphical_material)(temp_material,
						graphical_material_manager)))
				{
					DESTROY(Graphical_material)(&temp_material);
					return_code=0;
				}
			}
			if (temp_material)
			{
				movie->lines_2d_material=ACCESS(Graphical_material)(temp_material);
			}
		}
		if (return_code)
		{
			if (!(temp_material=FIND_BY_IDENTIFIER_IN_MANAGER(
				Graphical_material,name)("lines_3d",graphical_material_manager)))
			{
				if (!((temp_material=CREATE(Graphical_material)("lines_3d"))&&
					Graphical_material_set_ambient(temp_material,&lines_3d_colour)&&
					Graphical_material_set_diffuse(temp_material,&lines_3d_colour)&&
					ADD_OBJECT_TO_MANAGER(Graphical_material)(temp_material,
						graphical_material_manager)))
				{
					DESTROY(Graphical_material)(&temp_material);
					return_code=0;
				}
			}
			if (temp_material)
			{
				movie->lines_3d_material=ACCESS(Graphical_material)(temp_material);
			}
		}
		if (return_code)
		{
			if (!(temp_material=FIND_BY_IDENTIFIER_IN_MANAGER(
				Graphical_material,name)("surfaces_2d",graphical_material_manager)))
			{
				if (!((temp_material=CREATE(Graphical_material)("surfaces_2d"))&&
					Graphical_material_set_ambient(temp_material,&surfaces_ambient)&&
					Graphical_material_set_diffuse(temp_material,&surfaces_diffuse)&&
					Graphical_material_set_specular(temp_material,&surfaces_specular)&&
					Graphical_material_set_shininess(temp_material,surfaces_shininess)&&
					Graphical_material_set_alpha(temp_material,surfaces_2d_alpha)&&
					ADD_OBJECT_TO_MANAGER(Graphical_material)(temp_material,
						graphical_material_manager)))
				{
					DESTROY(Graphical_material)(&temp_material);
					return_code=0;
				}
			}
			if (temp_material)
			{
				movie->surfaces_2d_material=ACCESS(Graphical_material)(temp_material);
			}
		}
		if (return_code)
		{
			if (!(temp_material=FIND_BY_IDENTIFIER_IN_MANAGER(
				Graphical_material,name)("surfaces_3d",graphical_material_manager)))
			{
				if (!((temp_material=CREATE(Graphical_material)("surfaces_3d"))&&
					Graphical_material_set_ambient(temp_material,&surfaces_ambient)&&
					Graphical_material_set_diffuse(temp_material,&surfaces_diffuse)&&
					Graphical_material_set_specular(temp_material,&surfaces_specular)&&
					Graphical_material_set_shininess(temp_material,surfaces_shininess)&&
					ADD_OBJECT_TO_MANAGER(Graphical_material)(temp_material,
						graphical_material_manager)))
				{
					DESTROY(Graphical_material)(&temp_material);
					return_code=0;
				}
			}
			if (temp_material)
			{
				movie->surfaces_3d_material=ACCESS(Graphical_material)(temp_material);
			}
		}

		/* do not want all_element_group to be drawn in default scene */
		Scene_set_graphical_element_mode(movie->scene,
			GRAPHICAL_ELEMENT_INVISIBLE,computed_field_manager,element_manager,
			element_group_manager,fe_field_manager,node_manager,node_group_manager,
			data_manager,data_group_manager,element_point_ranges_selection,
			element_selection,node_selection,data_selection,user_interface);

		/* enlarge axes in default scene to fit size of face */
		axis_lengths[0]=25.0;
		axis_lengths[1]=25.0;
		axis_lengths[2]=25.0;
		Scene_set_axis_lengths(movie->scene,axis_lengths);

		/* read base node and element groups for movie */
		if (return_code)
		{
			if (ALLOCATE(read_node_data,struct File_read_FE_node_group_data,1))
			{
				read_node_data->fe_field_manager=fe_field_manager;
				read_node_data->element_group_manager=element_group_manager;
				read_node_data->node_manager=node_manager;
				read_node_data->element_manager=element_manager;
				read_node_data->node_group_manager=node_group_manager;
				read_node_data->data_group_manager=data_group_manager;
				return_code=file_read_FE_node_group(
					movie->all_node_file_name,(void *)read_node_data);
				DEALLOCATE(read_node_data);
			}
			else
			{
				return_code=0;
			}
		}
		if (return_code)
		{
			if (ALLOCATE(read_element_data,struct File_read_FE_element_group_data,1))
			{
				read_element_data->element_manager=element_manager;
				read_element_data->element_group_manager=element_group_manager;
				read_element_data->fe_field_manager=fe_field_manager;
				read_element_data->node_manager=node_manager;
				read_element_data->node_group_manager=node_group_manager;
				read_element_data->data_group_manager=data_group_manager;
				read_element_data->basis_manager=basis_manager;
				return_code=file_read_FE_element_group(
					movie->all_element_file_name,(void *)read_element_data);
				DEALLOCATE(read_element_data);
			}
			else
			{
				return_code=0;
			}
		}
		if (return_code)
		{
			/* get pointer to all element group */
			if (movie->all_element_group=
				FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_element),name)(
				movie->all_node_group_name,element_group_manager))
			{
				ACCESS(GROUP(FE_element))(movie->all_element_group);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"enable_Mirage_movie_graphics.  All element group %s not found",
					movie->all_node_group_name);
				return_code=0;
			}
		}
		if (return_code)
		{
			/* get pointer to all node group */
			if (movie->all_node_group=
				FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
				movie->all_node_group_name,node_group_manager))
			{
				ACCESS(GROUP(FE_node))(movie->all_node_group);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"enable_Mirage_movie_graphics.  All node group %s not found",
					movie->all_node_group_name);
				return_code=0;
			}
		}

		/* do want 3-D node/element groups to be drawn in default scene */
		Scene_set_graphical_element_mode(movie->scene,
			GRAPHICAL_ELEMENT_LINES,computed_field_manager,element_manager,
			element_group_manager,fe_field_manager,node_manager,node_group_manager,
			data_manager,data_group_manager,element_point_ranges_selection,
			element_selection,node_selection,data_selection,user_interface);

		if (return_code)
		{
			sprintf(temp_name2,"pending_3d");
			return_code=(
				(movie->pending_nodes_3d=CREATE(GROUP(FE_node))(temp_name2))
				&&ACCESS(GROUP(FE_node))(movie->pending_nodes_3d)&&
				ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(
					movie->pending_nodes_3d,node_group_manager)&&
				(movie->pending_elements_3d=
					CREATE(GROUP(FE_element))(temp_name2))
				&&ACCESS(GROUP(FE_element))(movie->pending_elements_3d)&&
				ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(
					movie->pending_elements_3d,element_group_manager));
		}
		if (return_code)
		{
			sprintf(temp_name2,"placed_3d");
			return_code=(
				(movie->placed_nodes_3d=CREATE(GROUP(FE_node))(temp_name2))
				&&ACCESS(GROUP(FE_node))(movie->placed_nodes_3d)&&
				ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(
					movie->placed_nodes_3d,node_group_manager)&&
				(movie->placed_elements_3d=
					CREATE(GROUP(FE_element))(temp_name2))
				&&ACCESS(GROUP(FE_element))(movie->placed_elements_3d)&&
				ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(
					movie->placed_elements_3d,element_group_manager));
		}
		if (return_code)
		{
			sprintf(temp_name2,"problem_3d");
			return_code=(
				(movie->problem_nodes_3d=CREATE(GROUP(FE_node))(temp_name2))
				&&ACCESS(GROUP(FE_node))(movie->problem_nodes_3d)&&
				ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(
					movie->problem_nodes_3d,node_group_manager)&&
				(movie->problem_elements_3d=
					CREATE(GROUP(FE_element))(temp_name2))
				&&ACCESS(GROUP(FE_element))(movie->problem_elements_3d)&&
				ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(
					movie->problem_elements_3d,element_group_manager));
		}

		/* reduce detail of all_element_group GFE and make it invisible */
		if (return_code&&(gt_element_group=Scene_get_graphical_element_group(
			movie->scene,movie->all_element_group)))
		{
			GT_element_group_set_element_discretization(gt_element_group,
				&minimum_discretization,user_interface);
			Scene_set_element_group_visibility(movie->scene,
				movie->all_element_group,g_INVISIBLE);
		}

		/* make sure pending points can be seen over problem then placed in
			3-D scene */
		if (return_code)
		{
			return_code=
				Scene_set_element_group_position(movie->scene,
					movie->placed_elements_3d,1)&&
				Scene_set_element_group_position(movie->scene,
					movie->problem_elements_3d,1)&&
				Scene_set_element_group_position(movie->scene,
					movie->pending_elements_3d,1);
		}

		/* remove lines and set discretization of 2*2*2 for 3-D placed group */
		if (return_code&&(gt_element_group=
			Scene_get_graphical_element_group(movie->scene,
				movie->placed_elements_3d)))
		{
			return_code=Mirage_movie_graphics_hide_lines(movie->scene,
				movie->placed_elements_3d)&&
				GT_element_group_set_element_discretization(gt_element_group,
					&minimum_discretization,user_interface);
		}

		/* don't want any of the following GFEs shown in default_scene */
		Scene_set_graphical_element_mode(movie->scene,
			GRAPHICAL_ELEMENT_INVISIBLE,computed_field_manager,element_manager,
			element_group_manager,fe_field_manager,node_manager,node_group_manager,
			data_manager,data_group_manager,element_point_ranges_selection,
			element_selection,node_selection,data_selection,user_interface);

		/* Views */
		for (view_no=0;return_code&&(view_no<movie->number_of_views);
			view_no++)
		{
			if ((view=movie->views[view_no])&&(view->name))
			{
				if (ALLOCATE(temp_name,char,20+strlen(view->name)))
				{
					view->element_group_manager=element_group_manager;
					view->node_group_manager=node_group_manager;
					view->scene_manager=scene_manager;
					view->texture_manager=texture_manager;
					if (return_code)
					{
						sprintf(temp_name,"view%s_placed",view->name);
						return_code=(
							(view->placed_nodes=CREATE(GROUP(FE_node))(temp_name))&&
							ACCESS(GROUP(FE_node))(view->placed_nodes)&&
							ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(
								view->placed_nodes,node_group_manager)&&
							(view->placed_elements=CREATE(GROUP(FE_element))(temp_name))&&
							ACCESS(GROUP(FE_element))(view->placed_elements)&&
							ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(
								view->placed_elements,element_group_manager));
					}
					if (return_code)
					{
						sprintf(temp_name,"view%s_problem",view->name);
						return_code=(
							(view->problem_nodes=CREATE(GROUP(FE_node))(temp_name))&&
							ACCESS(GROUP(FE_node))(view->problem_nodes)&&
							ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(
								view->problem_nodes,node_group_manager)&&
							(view->problem_elements=CREATE(GROUP(FE_element))(temp_name))&&
							ACCESS(GROUP(FE_element))(view->problem_elements)&&
							ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(
								view->problem_elements,element_group_manager));
					}
					if (return_code)
					{
						sprintf(temp_name,"view%s_pending",view->name);
						return_code=(
							(view->pending_nodes=CREATE(GROUP(FE_node))(temp_name))&&
							ACCESS(GROUP(FE_node))(view->pending_nodes)&&
							ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(
								view->pending_nodes,node_group_manager)&&
							(view->pending_elements=CREATE(GROUP(FE_element))(temp_name))&&
							ACCESS(GROUP(FE_element))(view->pending_elements)&&
							ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(
								view->pending_elements,element_group_manager));
					}
					if (return_code)
					{
						sprintf(temp_name,"view%s",view->name);
						return_code=(
							(view->scene=CREATE(Scene)(temp_name))&&
							ACCESS(Scene)(view->scene)&&
							ADD_OBJECT_TO_MANAGER(Scene)(view->scene,scene_manager)&&
							Scene_enable_graphics(view->scene,glyph_list,
								graphical_material_manager,default_graphical_material,
								light_manager,spectrum_manager,default_spectrum,
								texture_manager)&&
							Scene_set_graphical_element_mode(view->scene,
								GRAPHICAL_ELEMENT_INVISIBLE,
								computed_field_manager,element_manager,element_group_manager,
								fe_field_manager,node_manager,node_group_manager,
								data_manager,data_group_manager,element_point_ranges_selection,
								element_selection,node_selection,data_selection,
								user_interface)&&
							/* turn off axes in each views scene */
							Scene_set_axis_visibility(view->scene,g_INVISIBLE)&&
							(gt_element_group=Scene_get_graphical_element_group(
								view->scene,view->placed_elements))&&
							GT_element_group_set_element_discretization(gt_element_group,
								&minimum_discretization,user_interface)&&
							Scene_set_element_group_visibility(view->scene,
								view->placed_elements,g_VISIBLE)&&
							Scene_set_element_group_visibility(view->scene,
								view->problem_elements,g_VISIBLE)&&
							Scene_set_element_group_visibility(view->scene,
								view->pending_elements,g_VISIBLE)&&
							Scene_set_element_group_position(view->scene,
								view->placed_elements,1)&&
							Scene_set_element_group_position(view->scene,
								view->problem_elements,1)&&
							Scene_set_element_group_position(view->scene,
								view->pending_elements,1));
					}
					if (return_code)
					{
						sprintf(temp_name,"view%s",view->name);
						if (return_code=((view->texture=CREATE(Texture)(temp_name))&&
							ADD_OBJECT_TO_MANAGER(Texture)(view->texture,texture_manager)))
						{
							ACCESS(Texture)(view->texture);
						}
					}
					DEALLOCATE(temp_name);
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

		/* re-enable visibility and line renditions for new GFEs in default_scene */
		Scene_set_graphical_element_mode(movie->scene,
			GRAPHICAL_ELEMENT_LINES,computed_field_manager,element_manager,
			element_group_manager,fe_field_manager,node_manager,node_group_manager,
			data_manager,data_group_manager,element_point_ranges_selection,
			element_selection,node_selection,data_selection,user_interface);

		if (return_code)
		{
			if (!(movie->node_editor=CREATE(Mirage_node_editor)(movie)))
			{
				return_code=0;
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"enable_Mirage_movie_graphics.  Could not enable graphics");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"enable_Mirage_movie_graphics.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* enable_Mirage_movie_graphics */

int Mirage_movie_graphics_show_2d_points(struct Mirage_movie *movie,
	int show_node_numbers)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Turn on placed, pending and problem points on the 2-D views. Regenerates the
GFE graphics and updates the 3-D scenes. The other option allows node numbers to
be displayed with the placed points. Node numbers are never shown with the
pending points, but always with the problem points.
==============================================================================*/
{
	int return_code,view_no;
	struct Computed_field *label_field,*optional_label_field;
	struct GT_object *glyph;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Mirage_view *view;

	ENTER(Mirage_movie_graphics_show_2d_points);
	if (movie&&(computed_field_manager=movie->computed_field_manager))
	{
		return_code=1;
		glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("cross",movie->glyph_list);
		label_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			"cmiss_number",computed_field_manager);
		if (show_node_numbers)
		{
			optional_label_field=label_field;
		}
		else
		{
			optional_label_field=(struct Computed_field *)NULL;
		}
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				Mirage_movie_graphics_show_points(view->scene,view->placed_elements,
					movie->placed_points_material,glyph,MIRAGE_POINT_SIZE,
					optional_label_field);
				Mirage_movie_graphics_show_points(view->scene,view->pending_elements,
					movie->pending_points_material,glyph,MIRAGE_POINT_SIZE,
					(struct Computed_field *)NULL);
				/* always show cmiss_numbers beside problem points */
				Mirage_movie_graphics_show_points(view->scene,view->problem_elements,
					movie->problem_points_material,glyph,MIRAGE_POINT_SIZE,label_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_show_2d_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_show_2d_points */

int Mirage_movie_graphics_hide_2d_points(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Turn on placed, pending and problem points on the 3-D view. Regenerates the
GFE graphics and updates the 3-D scene. The other option allows node numbers to
be displayed with the placed points. Node numbers are never shown with the
pending points, but always with the problem points.
==============================================================================*/
{
	int return_code,view_no;
	struct Mirage_view *view;

	ENTER(Mirage_movie_graphics_show_2d_points);
	if (movie)
	{
		return_code=1;
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				return_code=1;
				Mirage_movie_graphics_hide_points(view->scene,view->placed_elements);
				Mirage_movie_graphics_hide_points(view->scene,view->pending_elements);
				Mirage_movie_graphics_hide_points(view->scene,view->problem_elements);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_hide_2d_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_hide_2d_points */

int Mirage_movie_graphics_show_2d_lines(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Turns on lines for the placed element GFEs in the 2-D views.
==============================================================================*/
{
	int return_code,view_no;
	struct Mirage_view *view;

	ENTER(Mirage_movie_graphics_show_2d_lines);
	if (movie)
	{
		return_code=1;
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				Mirage_movie_graphics_show_lines(view->scene,view->placed_elements,
					movie->lines_2d_material);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_show_2d_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_show_2d_lines */

int Mirage_movie_graphics_hide_2d_lines(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Turns off lines for the placed element GFEs in the 2-D views.
==============================================================================*/
{
	int return_code,view_no;
	struct Mirage_view *view;

	ENTER(Mirage_movie_graphics_show_2d_lines);
	if (movie)
	{
		return_code=1;
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				return_code=1;
				Mirage_movie_graphics_hide_lines(view->scene,view->placed_elements);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_hide_2d_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_hide_2d_lines */

int Mirage_movie_graphics_show_2d_surfaces(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Turns on surfaces for the placed element GFEs in the 2-D views.
==============================================================================*/
{
	int return_code,view_no;
	struct Mirage_view *view;

	ENTER(Mirage_movie_graphics_show_2d_surfaces);
	if (movie)
	{
		return_code=1;
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				Mirage_movie_graphics_show_surfaces(view->scene,view->placed_elements,
					movie->surfaces_2d_material);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_show_2d_surfaces.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_show_2d_surfaces */

int Mirage_movie_graphics_hide_2d_surfaces(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Turns off surfaces for the placed element GFEs in the 2-D views.
==============================================================================*/
{
	int return_code,view_no;
	struct Mirage_view *view;

	ENTER(Mirage_movie_graphics_show_2d_surfaces);
	if (movie)
	{
		return_code=1;
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				return_code=1;
				Mirage_movie_graphics_hide_surfaces(view->scene,view->placed_elements);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_hide_2d_surfaces.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_hide_2d_surfaces */

int Mirage_movie_graphics_show_3d_points(struct Mirage_movie *movie,
	int show_node_numbers)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Turn on placed, pending and problem points on the 3-D view. Regenerates the
GFE graphics and updates the 3-D scene. The other option allows node numbers to
be displayed with the placed points. Node numbers are never shown with the
pending points, but always with the problem points.
==============================================================================*/
{
	int return_code;
	struct Computed_field *label_field,*optional_label_field;
	struct GT_object *glyph;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(Mirage_movie_graphics_show_3d_points);
	if (movie&&(computed_field_manager=movie->computed_field_manager))
	{
		return_code=1;
		glyph=FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("cross",movie->glyph_list);
		label_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			"cmiss_number",computed_field_manager);
		if (show_node_numbers)
		{
			optional_label_field=label_field;
		}
		else
		{
			optional_label_field=(struct Computed_field *)NULL;
		}
		Mirage_movie_graphics_show_points(movie->scene,movie->placed_elements_3d,
			movie->placed_points_material,glyph,MIRAGE_POINT_SIZE,
			optional_label_field);
		Mirage_movie_graphics_show_points(movie->scene,movie->pending_elements_3d,
			movie->pending_points_material,glyph,MIRAGE_POINT_SIZE,
			(struct Computed_field *)NULL);
		Mirage_movie_graphics_show_points(movie->scene,movie->problem_elements_3d,
			movie->problem_points_material,glyph,MIRAGE_POINT_SIZE,label_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_show_3d_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_show_3d_points */

int Mirage_movie_graphics_hide_3d_points(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Turn on placed, pending and problem points on the 3-D view. Regenerates the
GFE graphics and updates the 3-D scene. The other option allows node numbers to
be displayed with the placed points. Node numbers are never shown with the
pending points, but always with the problem points.
==============================================================================*/
{
	int return_code;

	ENTER(Mirage_movie_graphics_hide_3d_points);
	if (movie)
	{
		return_code=1;
		Mirage_movie_graphics_hide_points(movie->scene,movie->placed_elements_3d);
		Mirage_movie_graphics_hide_points(movie->scene,movie->pending_elements_3d);
		Mirage_movie_graphics_hide_points(movie->scene,movie->problem_elements_3d);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_hide_3d_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_hide_3d_points */

int Mirage_movie_graphics_show_3d_lines(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Turns on lines for the placed elements in the 3-D view, updates scene.
==============================================================================*/
{
	int return_code;

	ENTER(Mirage_movie_graphics_show_3d_lines);
	if (movie)
	{
		return_code=1;
		Mirage_movie_graphics_show_lines(movie->scene,movie->placed_elements_3d,
			movie->lines_3d_material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_show_3d_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_show_3d_lines */

int Mirage_movie_graphics_hide_3d_lines(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Turns off lines for the placed elements in the 3-D view, updates scene.
==============================================================================*/
{
	int return_code;

	ENTER(Mirage_movie_graphics_hide_3d_lines);
	if (movie)
	{
		return_code=1;
		Mirage_movie_graphics_hide_lines(movie->scene,movie->placed_elements_3d);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_hide_3d_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_hide_3d_lines */

int Mirage_movie_graphics_show_3d_surfaces(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Turns on surfaces for the placed elements in the 3-D view, updates scene.
==============================================================================*/
{
	int return_code;

	ENTER(Mirage_movie_graphics_show_3d_surfaces);
	if (movie)
	{
		return_code=1;
		Mirage_movie_graphics_show_surfaces(movie->scene,movie->placed_elements_3d,
			movie->surfaces_3d_material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_show_3d_surfaces.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_show_3d_surfaces */

int Mirage_movie_graphics_hide_3d_surfaces(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Turns off surfaces for the placed elements in the 3-D view, updates scene.
==============================================================================*/
{
	int return_code;

	ENTER(Mirage_movie_graphics_hide_3d_surfaces);
	if (movie)
	{
		return_code=1;
		Mirage_movie_graphics_hide_surfaces(movie->scene,movie->placed_elements_3d);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_graphics_hide_3d_surfaces.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_graphics_hide_3d_surfaces */

int read_Mirage_movie_frame(struct Mirage_movie *movie,
	int frame_no)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Reads the images and nodes for frame <frame_no> in <movie>.
This will become the new current_frame_no of the movie.
==============================================================================*/
{
	char *image_file_name,*last_image_file_name_template;
	enum Texture_storage_type texture_storage;
	int crop_bottom_margin,crop_height,crop_left_margin,crop_width,
		number_of_bytes_per_component,number_of_components,return_code,
		view_no,original_width_texels,original_height_texels;
	long int image_width,image_height;
	struct Mirage_view *view;
	struct Texture *temp_texture;
	unsigned long *image,*cropped_image;

	ENTER(read_Mirage_movie_frame);
	if (movie)
	{
		if (return_code=Mirage_movie_read_frame_nodes(movie,frame_no))
		{
			movie->current_frame_no=frame_no;
			/* read images for each view */
			image=(unsigned long *)NULL;
			last_image_file_name_template=(char *)NULL;
			if (temp_texture=CREATE(Texture)("temp"))
			{
				for (view_no=0;return_code&&(view_no<movie->number_of_views);
						view_no++)
				{
					if ((view=movie->views[view_no])&&(image_file_name=
						make_Mirage_file_name(view->image_file_name_template,frame_no)))
					{
						/* get the image for this view */
						if (image)
						{
							/* try to reuse images if possible */
							if (strcmp(view->image_file_name_template,
								last_image_file_name_template))
							{
								DEALLOCATE(image);
							}
						}
						last_image_file_name_template=view->image_file_name_template;
						if (0==(movie->current_frame_no % 2))
						{
							/* even frame numbers */
							crop_left_margin=view->crop0_left;
							crop_bottom_margin=view->crop0_bottom;
							crop_width=view->crop0_width;
							crop_height=view->crop0_height;
						}
						else
						{
							/* odd frame numbers */
							crop_left_margin=view->crop1_left;
							crop_bottom_margin=view->crop1_bottom;
							crop_width=view->crop1_width;
							crop_height=view->crop1_height;
						}
						if (NULL==image)
						{
							/*???debug*/
							printf("  Reading texture image file: '%s'\n",image_file_name);
							return_code=(
								read_image_file(image_file_name,&number_of_components,
									&number_of_bytes_per_component,&image_height,&image_width,&image)&&
								(0<number_of_components));
						}
						if (image&&return_code)
						{
							original_width_texels=image_width;
							original_height_texels=image_height;
							cropped_image=(unsigned long *)NULL;
							/*???DB.  Guessing at texture storage for 4.  Should
								read_image_file return.  Is something like this already
								defined for images ? */
							switch (number_of_components)
							{
								case 1:
								{
									texture_storage=TEXTURE_LUMINANCE;
								} break;
								case 2:
								{
									texture_storage=TEXTURE_LUMINANCE_ALPHA;
								} break;
								case 3:
								{
									texture_storage=TEXTURE_RGB;
								} break;
								case 4:
								{
									texture_storage=TEXTURE_RGBA;
								} break;
								default:
								{
									texture_storage=TEXTURE_UNDEFINED_STORAGE;
								} break;
							}
							if ((cropped_image=copy_image(image,number_of_components,
								image_width,image_height))&&
								crop_image(&cropped_image,number_of_components,
									number_of_bytes_per_component,
									&original_width_texels,&original_height_texels,
									crop_left_margin,crop_bottom_margin,crop_width,crop_height)&&
								Texture_set_image(temp_texture,cropped_image,
									texture_storage,number_of_bytes_per_component,original_width_texels,
									original_height_texels,image_file_name,
									crop_left_margin,crop_bottom_margin,crop_width,crop_height))
							{
								/* the texture stores the distortion centre and factor k1.
									 Since the centre jiggles between even and odd frames with
									 interlacing, must set it again each time. */
								if (0==(movie->current_frame_no % 2))
								{
									/* even frame numbers */
									Texture_set_physical_size(temp_texture,
										(float)view->image0_width,
										(float)view->image0_height);
									Texture_set_distortion_info(temp_texture,
										(float)view->dist_centre_x-view->image0_left,
										(float)view->dist_centre_y-view->image0_bottom,
										(float)view->dist_factor_k1);
								}
								else
								{
									/* odd frame numbers */
									Texture_set_physical_size(temp_texture,
										(float)view->image1_width,
										(float)view->image1_height);
									Texture_set_distortion_info(temp_texture,
										(float)view->dist_centre_x-view->image1_left,
										(float)view->dist_centre_y-view->image1_bottom,
										(float)view->dist_factor_k1);
								}
								return_code=
									MANAGER_MODIFY_NOT_IDENTIFIER(Texture,name)(view->texture,
										temp_texture,view->texture_manager);
							}
							else
							{
								return_code=0;
							}
							if (cropped_image)
							{
								DEALLOCATE(cropped_image);
							}
						}
						DEALLOCATE(image_file_name);
					}
					else
					{
						return_code=0;
					}
				}
				DESTROY(Texture)(&temp_texture);
			}
			else
			{
				return_code=0;
			}
			if (image)
			{
				DEALLOCATE(image);
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"read_Mirage_movie_frame.  Error reading frame");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_Mirage_movie_frame.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_Mirage_movie_frame */

int Mirage_movie_frame_nodes_exist_or_error(struct Mirage_movie *movie,
	int frame_no)
/*******************************************************************************
LAST MODIFIED : 22 April 1998

DESCRIPTION :
Returns true if there exists a node file for <frame_no> or there is an error.
If this routine returns 0, you know there are no nodes for frame <frame_no>,
useful for creating initial meshes.
==============================================================================*/
{
	char *node_file_name;
	FILE *input_file;
	int return_code;

	ENTER(Mirage_movie_read_frame_nodes);
	return_code=1;
	if (movie&&(frame_no >= movie->start_frame_no)&&
		(frame_no < movie->start_frame_no+movie->number_of_frames))
	{
		if (node_file_name=
			make_Mirage_file_name(movie->node_file_name_template,frame_no))
		{
			if (input_file=fopen(node_file_name,"r"))
			{
				fclose(input_file);
				return_code=1;
			}
			else
			{
				return_code=0;
			}
			DEALLOCATE(node_file_name);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_read_frame_nodes.  Invalid argument(s)");
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_read_frame_nodes */

int Mirage_movie_read_frame_nodes(struct Mirage_movie *movie,
	int frame_no)
/*******************************************************************************
LAST MODIFIED : 6 September 1999

DESCRIPTION :
Reads the nodes for frame <frame_no> in <movie>. If <frame_no> is valid but
there is no node file for it the function returns successfully since the
current node positions in memory may be used.
The frame_no does not have to be the current_frame_number of the movie although
it is dangerous (but possibly useful) if it is not.
==============================================================================*/
{
	char *node_file_name;
	FILE *input_file;
	int return_code;
	struct File_read_FE_node_group_data *read_node_data;

	ENTER(Mirage_movie_read_frame_nodes);
	if (movie&&(frame_no >= movie->start_frame_no)&&
		(frame_no < movie->start_frame_no+movie->number_of_frames))
	{
		if (node_file_name=
			make_Mirage_file_name(movie->node_file_name_template,frame_no))
		{
			/* check there is a node file at this frame_no */
			if (input_file=fopen(node_file_name,"r"))
			{
				fclose(input_file);
				if (ALLOCATE(read_node_data,struct File_read_FE_node_group_data,1))
				{
					/*printf("> Reading node file '%s'\n",node_file_name);*/
					read_node_data->fe_field_manager=movie->fe_field_manager;
					read_node_data->element_group_manager=movie->element_group_manager;
					read_node_data->node_manager=movie->node_manager;
					read_node_data->element_manager=movie->element_manager;
					read_node_data->node_group_manager=movie->node_group_manager;
					read_node_data->data_group_manager=movie->data_group_manager;
					return_code=file_read_FE_node_group(
						node_file_name,(void *)read_node_data);
					DEALLOCATE(read_node_data);
				}
				else
				{
					display_message(ERROR_MESSAGE,"Mirage_movie_read_frame_nodes.  "
						"Could not allocate read_node_data");
					return_code=0;
				}
			}
			else
			{
				/* use existing node positions in memory */
				return_code=1;
			}
			DEALLOCATE(node_file_name);
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_read_frame_nodes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_read_frame_nodes */

int Mirage_movie_write_frame_nodes(struct Mirage_movie *movie,
	int frame_no)
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Writes the nodes in memory to the file for frame <frame_no> of <movie>.
The frame_no does not have to be the current_frame_number of the movie although
it is dangerous (but possibly useful) if it is not.
==============================================================================*/
{
	char *node_file_name;
	int return_code;
	struct Fwrite_FE_node_group_data data;

	ENTER(Mirage_movie_write_frame_nodes);
	if (movie&&(frame_no >= movie->start_frame_no)&&
		(frame_no < movie->start_frame_no+movie->number_of_frames))
	{
		if (node_file_name=
			make_Mirage_file_name(movie->node_file_name_template,frame_no))
		{
			data.field = (struct FE_field *)NULL;
			data.node_group = movie->all_node_group;
			return_code=
				file_write_FE_node_group(node_file_name,&data);
			DEALLOCATE(node_file_name);
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_write_frame_nodes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_write_frame_nodes */

int Mirage_movie_full_save(struct Mirage_movie *movie,char *extra_extension)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :
Writes the movie file and node status lists with names based on the movie->name
and the <extra_extension> added on the end. The current node file is also
written - but with its usual name.
==============================================================================*/
{
	int return_code;

	ENTER(Mirage_movie_full_save);
	if (movie&&extra_extension)
	{
		return_code=1;
		if (!write_Mirage_movie(movie,extra_extension))
		{
			return_code=0;
		}
		if (!Mirage_movie_write_node_status_lists(movie,extra_extension))
		{
			return_code=0;
		}
		if (!Mirage_movie_write_frame_nodes(movie,movie->current_frame_no))
		{
			return_code=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Mirage_movie_full_save.  Could not save movie");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_full_save.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_full_save */

struct Mirage_movie_add_node_to_group_data
{
	struct LIST(Node_status) *node_status_list,*node_status_list2;
	struct GROUP(FE_node) *node_group;
	int frame_no;
}; /* Mirage_movie_add_node_to_group_data */

static int Mirage_movie_add_node_to_group(struct FE_node *node,
	void *add_node_data_void)
/*******************************************************************************
LAST MODIFIED : 4 November 1998

DESCRIPTION :
Adds node to supplied group if it is in the node_status_list for the
specified view and frame. If there was a Node_status for <node> in the
<node_status_list>, but the node was not in a range, add it if it is in a
range in the node_status_list2 (if that list is not NULL).
The reason for this is that placed lists for each view are only utilised
while a point is not placed fully in 3-D.
==============================================================================*/
{
	int return_code;
	struct Node_status *node_status;
	struct Mirage_movie_add_node_to_group_data *add_node_data;

	ENTER(Mirage_movie_add_node_to_group);
	if (node&&(add_node_data=
		(struct Mirage_movie_add_node_to_group_data *)add_node_data_void)&&
		add_node_data->node_group&&add_node_data->node_status_list)
	{
		return_code=1;
		if (node_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			get_FE_node_cm_node_identifier(node),add_node_data->node_status_list))
		{
			if (Node_status_is_value_in_range(node_status,add_node_data->frame_no)||
				(add_node_data->node_status_list2&&
				(node_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
				get_FE_node_cm_node_identifier(node),add_node_data->node_status_list2))&&
				Node_status_is_value_in_range(node_status,add_node_data->frame_no)))
			{
				return_code=ADD_OBJECT_TO_GROUP(FE_node)(node,
					add_node_data->node_group);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_add_node_to_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_add_node_to_group */

static int FE_node_not_in_group(struct FE_node *node,void *node_group_void)
/*******************************************************************************
LAST MODIFIED : 24 February 1998

DESCRIPTION :
Returns 1 if node is not in node_group.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_not_in_group);
	return_code=!IS_OBJECT_IN_GROUP(FE_node)(node,
		(struct GROUP(FE_node) *)node_group_void);
	LEAVE;

	return (return_code);
} /* FE_node_not_in_group */

struct add_completed_FE_element_to_group_data
{
	int max_dimension;
	struct GROUP(FE_node) *node_group;
	struct GROUP(FE_element) *element_group;
};

static int add_completed_FE_element_to_group(
	struct FE_element *element,void *add_element_data_void)
/*******************************************************************************
LAST MODIFIED : 28 February 1998

DESCRIPTION :
If the dimension of the element is less than or equal to max_dimension, and
all the nodes in the element are in the specified node_group, then the element
is added to the passed element group.
==============================================================================*/
{
	int return_code;
	struct add_completed_FE_element_to_group_data *add_element_data;
	struct LIST(FE_node) *node_list;

	ENTER(add_completed_FE_element_to_group);
	if (element&&(element->shape)&&(add_element_data=
		(struct add_completed_FE_element_to_group_data *)add_element_data_void)&&
		add_element_data->node_group&&add_element_data->element_group)
	{
		if (element->shape->dimension <= add_element_data->max_dimension)
		{
			/* get list of nodes in element to compare with = SLOW */
			if ((node_list=CREATE(LIST(FE_node))())&&
				calculate_FE_element_field_nodes(element,(struct FE_field *)NULL,
					node_list))
			{
				if (!FIRST_OBJECT_IN_LIST_THAT(FE_node)(FE_node_not_in_group,
					(void *)(add_element_data->node_group),node_list))
				{
					return_code=ADD_OBJECT_TO_GROUP(FE_element)(element,
						add_element_data->element_group);
				}
				else
				{
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"add_completed_FE_element_to_group.  Could not make node list");
				return_code=0;
			}
			DESTROY(LIST(FE_node))(&node_list);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_completed_FE_element_to_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_completed_FE_element_to_group */

struct add_FE_node_to_group_if_in_group_data
{
	struct GROUP(FE_node) *node_group_must_be_in,*node_group_to_modify;
}; /* add_FE_node_to_group_if_in_group_data */

static int add_FE_node_to_group_if_in_group(struct FE_node *node,
	void *add_node_if_in_group_data_void)
/*******************************************************************************
LAST MODIFIED : 4 November 1998

DESCRIPTION :
Adds node to <node_group_to_modify> if it is in <node_group_must_be_in>.
specified view and frame.
==============================================================================*/
{
	int return_code;
	struct add_FE_node_to_group_if_in_group_data *add_node_if_in_group_data;

	ENTER(add_FE_node_to_group_if_in_group);
	if (node&&(add_node_if_in_group_data=
		(struct add_FE_node_to_group_if_in_group_data *)
		add_node_if_in_group_data_void))
	{
		if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
			get_FE_node_cm_node_identifier(node),
			add_node_if_in_group_data->node_group_must_be_in))
		{
			return_code=ADD_OBJECT_TO_GROUP(FE_node)(node,
				add_node_if_in_group_data->node_group_to_modify);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_FE_node_to_group_if_in_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_FE_node_to_group_if_in_group */

int Mirage_movie_refresh_node_groups(struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Refreshes the placed, pending and problem node groups so that they match the
entries in the Node_status_lists for the current_frame_no of the movie.
Should be called after reading and changing frames.
==============================================================================*/
{
	int return_code,view_no;
	struct Mirage_view *view;
	struct add_completed_FE_element_to_group_data add_element_data;
	struct add_FE_node_to_group_if_in_group_data add_node_if_in_group_data;
	struct Mirage_movie_add_node_to_group_data add_node_data;

	ENTER(Mirage_movie_refresh_node_groups);
	if (movie)
	{
		add_node_data.frame_no=movie->current_frame_no;
		add_element_data.max_dimension=2;

		/* update pending problem and placed groups in 3-D */
		add_node_data.node_status_list2=(struct LIST(Node_status) *)NULL;
		add_node_data.node_status_list=movie->pending_list;
		add_node_data.node_group=movie->pending_nodes_3d;
		MANAGED_GROUP_BEGIN_CACHE(FE_node)(movie->pending_nodes_3d);
		REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(movie->pending_nodes_3d);
		FOR_EACH_OBJECT_IN_GROUP(FE_node)(Mirage_movie_add_node_to_group,
			(void *)&add_node_data,movie->all_node_group);
		MANAGED_GROUP_END_CACHE(FE_node)(movie->pending_nodes_3d);
		add_node_data.node_status_list=movie->problem_list;
		add_node_data.node_group=movie->problem_nodes_3d;
		MANAGED_GROUP_BEGIN_CACHE(FE_node)(movie->problem_nodes_3d);
		REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(movie->problem_nodes_3d);
		FOR_EACH_OBJECT_IN_GROUP(FE_node)(Mirage_movie_add_node_to_group,
			(void *)&add_node_data,movie->all_node_group);
		MANAGED_GROUP_END_CACHE(FE_node)(movie->problem_nodes_3d);
		add_node_data.node_status_list=movie->placed_list;
		add_node_data.node_group=movie->placed_nodes_3d;
		MANAGED_GROUP_BEGIN_CACHE(FE_node)(movie->placed_nodes_3d);
		REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(movie->placed_nodes_3d);
		FOR_EACH_OBJECT_IN_GROUP(FE_node)(Mirage_movie_add_node_to_group,
			(void *)&add_node_data,movie->all_node_group);
		MANAGED_GROUP_END_CACHE(FE_node)(movie->placed_nodes_3d);

		/* also add placed elements in 3-D */
		add_element_data.node_group=movie->placed_nodes_3d;
		add_element_data.element_group=movie->placed_elements_3d;
		MANAGED_GROUP_BEGIN_CACHE(FE_element)(movie->placed_elements_3d);
		REMOVE_ALL_OBJECTS_FROM_GROUP(FE_element)(movie->placed_elements_3d);
		FOR_EACH_OBJECT_IN_GROUP(FE_element)(add_completed_FE_element_to_group,
			(void *)&add_element_data,movie->all_element_group);
		MANAGED_GROUP_END_CACHE(FE_element)(movie->placed_elements_3d);

		/* update placed, pending and problem groups in each view */
		add_node_data.node_status_list2=movie->placed_list;
		for (view_no=0;view_no<movie->number_of_views;view_no++)
		{
			if (view=movie->views[view_no])
			{
				/* add placed nodes for view */
				add_node_data.node_status_list=view->placed_list;
				add_node_data.node_group=view->placed_nodes;
				MANAGED_GROUP_BEGIN_CACHE(FE_node)(view->placed_nodes);
				REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(view->placed_nodes);
				FOR_EACH_OBJECT_IN_GROUP(FE_node)(Mirage_movie_add_node_to_group,
					(void *)&add_node_data,movie->all_node_group);
				MANAGED_GROUP_END_CACHE(FE_node)(view->placed_nodes);

				/* also add placed elements in view */
				add_element_data.node_group=view->placed_nodes;
				add_element_data.element_group=view->placed_elements;
				MANAGED_GROUP_BEGIN_CACHE(FE_element)(view->placed_elements);
				REMOVE_ALL_OBJECTS_FROM_GROUP(FE_element)(view->placed_elements);
				FOR_EACH_OBJECT_IN_GROUP(FE_element)(
					add_completed_FE_element_to_group,
					(void *)&add_element_data,movie->all_element_group);
				MANAGED_GROUP_END_CACHE(FE_element)(view->placed_elements);

				/* make pending and problem groups in each view from intersection of
					placed group in that view with placed/pending groups in 3-D */
				add_node_if_in_group_data.node_group_must_be_in=movie->pending_nodes_3d;
				add_node_if_in_group_data.node_group_to_modify=view->pending_nodes;
				MANAGED_GROUP_BEGIN_CACHE(FE_node)(view->pending_nodes);
				REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(view->pending_nodes);
				FOR_EACH_OBJECT_IN_GROUP(FE_node)(add_FE_node_to_group_if_in_group,
					(void *)&add_node_if_in_group_data,view->placed_nodes);
				MANAGED_GROUP_END_CACHE(FE_node)(view->pending_nodes);

				add_node_if_in_group_data.node_group_must_be_in=movie->problem_nodes_3d;
				add_node_if_in_group_data.node_group_to_modify=view->problem_nodes;
				MANAGED_GROUP_BEGIN_CACHE(FE_node)(view->problem_nodes);
				REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(view->problem_nodes);
				FOR_EACH_OBJECT_IN_GROUP(FE_node)(add_FE_node_to_group_if_in_group,
					(void *)&add_node_if_in_group_data,view->placed_nodes);
				MANAGED_GROUP_END_CACHE(FE_node)(view->problem_nodes);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_refresh_node_groups.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_refresh_node_groups */

int add_elements_with_node_to_group(struct FE_element *element,
	void *add_data_void)
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Add_elements_with_node_data *add_data;
	struct LIST(FE_node) *node_list;

	ENTER(add_elements_with_node_to_group);
	if (element&&(element->shape)&&
		(add_data=(struct Add_elements_with_node_data *)add_data_void))
	{
		if (add_data->max_dimension >= element->shape->dimension)
		{
			if (add_data->node&&add_data->node_group&&add_data->element_group)
			{
				return_code=1;
				/* trivial rejection: element or its parent contains node and
					not already in group */
				if (FE_element_or_parent_contains_node(element,(void *)add_data->node)&&
					!FIND_BY_IDENTIFIER_IN_GROUP(FE_element,identifier)(
					element->identifier,add_data->element_group))
				{
					/* compare against complete list of nodes in element = SLOW */
					if ((node_list=CREATE(LIST(FE_node))())&&
						calculate_FE_element_field_nodes(element,(struct FE_field *)NULL,
						node_list))
					{
						if (!FIRST_OBJECT_IN_LIST_THAT(FE_node)(FE_node_not_in_group,
							(void *)(add_data->node_group),node_list))
						{
							return_code=ADD_OBJECT_TO_GROUP(FE_element)(element,
								add_data->element_group);
						}
					}
					else
					{
						return_code=0;
					}
					DESTROY(LIST(FE_node))(&node_list);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"add_elements_with_node_to_group.  Invalid argument(s)");
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
			"add_elements_with_node_to_group.  Invalid element");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* add_elements_with_node_to_group */

int remove_elements_with_node_from_group(struct FE_element *element,
	void *rem_data_void)
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct LIST(FE_node) *node_list;
	struct Remove_elements_with_node_data *rem_data;

	ENTER(remove_elements_with_node_from_group);
	if (element&&(rem_data=(struct Remove_elements_with_node_data *)
		rem_data_void)&&rem_data->node&&rem_data->element_group)
	{
		return_code=1;
		/* trivial rejection: element or its parent contains node & is in group */
		if (FE_element_or_parent_contains_node(element,(void *)rem_data->node)&&
			FIND_BY_IDENTIFIER_IN_GROUP(FE_element,identifier)(
			element->identifier,rem_data->element_group))
		{
			/* compare against complete list of nodes in element = SLOW */
			if ((node_list=CREATE(LIST(FE_node))())&&
				calculate_FE_element_field_nodes(element,(struct FE_field *)NULL,
				node_list))
			{
				if (IS_OBJECT_IN_LIST(FE_node)(rem_data->node,node_list))
				{
					return_code=REMOVE_OBJECT_FROM_GROUP(FE_element)(element,
						rem_data->element_group);
				}
			}
			else
			{
				return_code=0;
			}
			DESTROY(LIST(FE_node))(&node_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"remove_elements_with_node_from_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* remove_elements_with_node_from_group */
