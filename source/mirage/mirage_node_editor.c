/*******************************************************************************
FILE : mirage_node_editor.c

LAST MODIFIED : 28 September 1999

DESCRIPTION :
Special graphical node editor for mirage digitiser windows.
==============================================================================*/
#include <math.h>
#include "general/debug.h"
#include "general/geometry.h"
#include "general/image_utilities.h"
#include "graphics/element_group_settings.h"
#include "graphics/graphical_element.h"
#include "graphics/scene.h"
#include "mirage/movie.h"
#include "mirage/mirage_node_editor.h"
#include "mirage/photogrammetry.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/

/*
Module types
------------
*/
struct Mirage_node_editor
/*******************************************************************************
LAST MODIFIED : 19 February 1997

DESCRIPTION :
==============================================================================*/
{
	struct Mirage_movie *movie;
	double node_position[3],node_position_window[3],start_cursor_x,start_cursor_y;
	enum Mirage_node_editor_mode mode;
	struct GROUP(FE_node) *selected_nodes;
	struct FE_node *node,*working_node;
	struct GT_object *graphics_object;
}; /* struct Mirage_node_editor */

/*
Module functions
----------------
*/
int Texture_get_centre_of_mass(struct Texture *texture,int mx,int my,int radius,
	int colour_index,double threshold,double *cmx,double *cmy)
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
Returns the byte values in the texture at x,y.
The threshold controls the cut-off intensity. Its value may range from
0.0 (median_intensity) to 1.0 (min_intensity/max_intensity - see below).
If <colour_index> is 0, it performs the centre of mass on dark elements in the
image only. If it is 1 light elements are used. In future it may be replaced
with an RGB filter value.
Returns in num_used the number of points that were included in the calcs.
???RC 28 Sept. 1999. To handle images of non-square aspect ratio, the function
first calculates for the texture:
aspect_x = texture_width / width_texels
aspect_y = texture_height / height_texels
The given <radius> is assumed to work in the direction with the highest aspect,
and a separate radius is calculated in the other direction to make it the same
size in physical space.
==============================================================================*/
{
	float aspect_x,aspect_y,texture_height,texture_width;
	int return_code,*buffer,width_texels,height_texels,x,y,i,k,swap,
		number_of_components,v,w,median_intensity,cutoff_intensity,radius_x,
		radius_y;
	unsigned char values[4];

	ENTER(Texture_get_centre_of_mass);
	if (texture&&cmx&&cmy)
	{
		if (return_code=(
			Texture_get_original_size(texture,&width_texels,&height_texels)&&
			(0<width_texels)&&(0<height_texels)&&
			Texture_get_physical_size(texture,&texture_width,&texture_height)&&
			(0.0<texture_width)&&(0.0<texture_height)&&
			(number_of_components=Texture_get_number_of_components(texture))))
		{
			radius_x=radius_y=radius;
			aspect_x = texture_width / (float)width_texels;
			aspect_y = texture_height / (float)height_texels;
			if (aspect_x < aspect_y)
			{
				radius_x = radius*aspect_y/aspect_x;
			}
			else
			{
				radius_y = radius*aspect_x/aspect_y;
			}
			if ((0<mx)&&(mx<width_texels)&&(0<my)&&(my<height_texels))
			{
				if (radius <= 0)
				{
					*cmx=(double)mx;
					*cmy=(double)my;
					return_code=1;
				}
				else
				{
					if (ALLOCATE(buffer,int,(radius_x*2+1)*(radius_y*2+1)))
					{
						/* calculate the median intensity */
						/* bubble sort the left node neighbourhood (yeah, I know...) */
						k=0;
						for (y=my-radius_y;y <= my+radius_y;y++)
						{
							for (x=mx-radius_x;x <= mx+radius_x;x++)
							{
								if ((0 <= x)&&(x<width_texels)&&(0 <= y)&&(y<height_texels))
								{
									if (Texture_get_raw_pixel_values(texture,x,y,values))
									{
										if (number_of_components<3)
										{
											v=(int)values[0];
										}
										else
										{
											/* Colour: use 30% red 59% green 11% blue */
											v=30*values[0]+59*values[1]+11*values[2];
										}
										buffer[k]=v;
										/* shift the new value down to keep values ordered from
											min to max */
										swap=1;
										for (i=k;(0<i)&&swap;i--)
										{
											if (buffer[i] < buffer[i-1])
											{
												v=buffer[i-1];
												buffer[i-1]=buffer[i];
												buffer[i]=v;
											}
											else
											{
												swap = 0;
											}
										}
										/* increment insertion index */
										k++;
									}
								}
							}
						}
						median_intensity=buffer[k/2];
						if (0==colour_index)
						{
							cutoff_intensity=(1.0-threshold)*median_intensity+
								threshold*buffer[0];
							/* ensure cutoff_intensity is at least lighter than min. */
							if (cutoff_intensity <= buffer[0])
							{
								cutoff_intensity=buffer[0]+1;
							}
						}
						else
						{
							cutoff_intensity=(1.0-threshold)*median_intensity+
								threshold*buffer[k-1];
							/* ensure cutoff_intensity is at least darker than max. */
							if (cutoff_intensity >= buffer[k-1])
							{
								cutoff_intensity=buffer[k-1]-1;
							}
						}
						/*???debug*/
						/*printf("CoM: intensity median=%i, min=%i, max=%i, cutoff=%i\n",
							median_intensity,buffer[0],buffer[k-1],cutoff_intensity);*/

						/* calculate the center of mass  */
						*cmx=0.0;
						*cmy=0.0;
						w=0;
						for (y=my-radius_y;y <= my+radius_y;y++)
						{
							for (x=mx-radius_x;x <= mx+radius_x;x++)
							{
								if ((0 <= x)&&(x<width_texels)&&(0 <= y)&&(y<height_texels))
								{
									if (Texture_get_raw_pixel_values(texture,x,y,values))
									{
										if (number_of_components<3)
										{
											v=(int)values[0];
										}
										else
										{
											/* Use 30% red 59% green 11% blue */
											v=30*values[0]+59*values[1]+11*values[2];
										}
										/* calculate weight */
										if (0==colour_index)
										{
											v=cutoff_intensity-v;
										}
										else
										{
											v=v-cutoff_intensity;
										}
										if (0>v)
										{
											v=0;
										}
										/* update running sums */
										*cmx += (double)x * (double)v;
										*cmy += (double)y * (double)v;
										w += v;
									}
								}
							}
						}
						if (0<w)
						{
							*cmx /= (double) w;
							*cmy /= (double) w;
						}
						else
						{
							*cmx=mx;
							*cmy=my;
						}
						return_code=1;
						DEALLOCATE(buffer);
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Texture_get_centre_of_mass.  Initial centre not in texture");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Texture_get_centre_of_mass.  Missing texture");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Texture_get_centre_of_mass */

static int Mirage_movie_3d_to_texture_coordinates(struct Mirage_movie *movie,
	int view_no,double position[3],double *texel_x,double *texel_y)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Converts 3-D <position> to texture [radially distorted] coordinates.
==============================================================================*/
{
	double corr_x,corr_y,pos2[2],dist_x,dist_y,image_left,image_bottom,
		image_width,image_height;
	int return_code,width_texels,height_texels;
	struct Mirage_view *view;

	ENTER(Mirage_movie_3d_to_texture_coordinates);
	if (movie&&movie->views&&(view=movie->views[view_no])&&texel_x&&texel_y)
	{
		if (return_code=(point_3d_to_2d_view(view->transformation43,position,pos2)&&
			Texture_get_original_size(view->texture,&width_texels,&height_texels)))
		{
			corr_x=pos2[0];
			corr_y=pos2[1];
			return_code=get_radial_distortion_distorted_coordinates(corr_x,corr_y,
				view->dist_centre_x,view->dist_centre_y,view->dist_factor_k1,
				0.001/*tolerance*/,&dist_x,&dist_y);
			if (0==(movie->current_frame_no % 2))
			{
				/* even frame numbers */
				image_left  =view->image0_left;
				image_bottom=view->image0_bottom;
				image_width =view->image0_width;
				image_height=view->image0_height;
			}
			else
			{
				/* odd frame numbers */
				image_left  =view->image1_left;
				image_bottom=view->image1_bottom;
				image_width =view->image1_width;
				image_height=view->image1_height;
			}
			*texel_x=(dist_x-image_left)*width_texels/image_width;
			*texel_y=(dist_y-image_bottom)*height_texels/image_height;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_3d_to_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_3d_to_texture_coordinates */

static int Mirage_movie_texture_to_3d_coordinates(struct Mirage_movie *movie,
	int view_no,double texel_x,double texel_y,double fact,double position[3])
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Converts texture [radially distorted] coordinates to 3-D positions on the
near plane.
==============================================================================*/
{
	double corr_x,corr_y,pos2[2],dist_x,dist_y,image_left,image_bottom,
		image_width,image_height,near_pos[3],far_pos[3];
	int return_code,width_texels,height_texels;
	struct Mirage_view *view;

	ENTER(Mirage_movie_texture_to_3d_coordinates);
	if (movie&&movie->views&&(view=movie->views[view_no]))
	{
		if (Texture_get_original_size(view->texture,&width_texels,&height_texels))
		{
			if (0==(movie->current_frame_no % 2))
			{
				/* even frame numbers */
				image_left  =view->image0_left;
				image_bottom=view->image0_bottom;
				image_width =view->image0_width;
				image_height=view->image0_height;
			}
			else
			{
				/* odd frame numbers */
				image_left  =view->image1_left;
				image_bottom=view->image1_bottom;
				image_width =view->image1_width;
				image_height=view->image1_height;
			}
			dist_x=image_left+(texel_x+0.5)*image_width/width_texels;
			dist_y=image_bottom+(texel_y+0.5)*image_height/height_texels;
			if (return_code=get_radial_distortion_corrected_coordinates(
				dist_x,dist_y,view->dist_centre_x,view->dist_centre_y,
				view->dist_factor_k1,&corr_x,&corr_y))
			{
				pos2[0]=corr_x;
				pos2[1]=corr_y;
				if (return_code=photogrammetry_unproject(view->transformation43,
					view->near_clipping_plane,view->far_clipping_plane,
					view->NDC_left,view->NDC_bottom,
					view->NDC_width,view->NDC_height,pos2,near_pos,far_pos))
				{
					position[0]=fact*near_pos[0]+(1.0-fact)*far_pos[0];
					position[1]=fact*near_pos[1]+(1.0-fact)*far_pos[1];
					position[2]=fact*near_pos[2]+(1.0-fact)*far_pos[2];
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_texture_to_3d_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_texture_to_3d_coordinates */

static int Mirage_movie_auto_two_pass_centre_of_mass(struct Mirage_movie *movie,
	int view_no,double position[3],int edit_node_index,double fact,
	double new_position[3])
/*******************************************************************************
LAST MODIFIED : 26 July 1999

DESCRIPTION :
This routine assumes the initial position is fairly close to the desired centre
of mass point, so uses a two pass centre of mass with the second radius used
for both passes to get the centre to be returned. It then iteratively increases
the first pass radius from 1 to the maximum from the movie file until it finds
the optimum value (ie. it must be as large as possible but still lock on to the
same centre). The new radius is converted into a com_factor for edit_node_index.
Note: new_position may have the same address as position.
==============================================================================*/
{
	/* centre of mass must be within the following distance to be same */
	double cmx,*cmx_rad,cmy,*cmy_rad,com_threshold1,com_threshold2,dist,
		dist_difference,min_dist_difference,real_cmx,real_cmy;
	int com_colour_index,com_radius1,i,mx,my,return_code,radius;
	struct Mirage_view *view;

	ENTER(Mirage_movie_two_pass_centre_of_mass);
	if (movie&&movie->views&&(view=movie->views[view_no])&&(0<=edit_node_index)&&
		(view->number_of_nodes>edit_node_index)&&position&&new_position)
	{
		com_colour_index=view->com_colour_indices[edit_node_index];
		com_threshold1=movie->com_threshold1;
		com_threshold2=movie->com_threshold2;
		if (Mirage_movie_3d_to_texture_coordinates(movie,view_no,position,
			&cmx,&cmy))
		{
			/* get integer centre: */
			mx=(int)floor(cmx+0.5);
			my=(int)floor(cmy+0.5);
			return_code=1;
			/* determine position returned for range of com_radii */
			if (ALLOCATE(cmx_rad,double,movie->com_radius1+1)&&
				ALLOCATE(cmy_rad,double,movie->com_radius1+1))
			{
				for (i=0;return_code&&(i<=movie->com_radius1);i++)
				{
					/* first pass radius is at least movie->com_radius2 */
					com_radius1=i;
					if (com_radius1 < movie->com_radius2)
					{
						com_radius1=movie->com_radius2;
					}
					/* 2 pass centre of mass at increasing radii */
					return_code=
						Texture_get_centre_of_mass(view->texture,mx,my,
							com_radius1,com_colour_index,com_threshold1,&cmx,&cmy)&&
						Texture_get_centre_of_mass(view->texture,(int)floor(cmx+0.5),
							(int)floor(cmy+0.5),i,com_colour_index,com_threshold2,
							&(cmx_rad[i]),&(cmy_rad[i]));
				}
				if (return_code)
				{
					/* assume movie->com_radius2 gives position close to centre */
					real_cmx=cmx_rad[movie->com_radius2];
					real_cmy=cmy_rad[movie->com_radius2];
					radius=0;
					min_dist_difference=movie->com_radius1*2;
					for (i=movie->com_radius2;i<=movie->com_radius1;i++)
					{
						/* point must be close to the original centre... */
						dist=sqrt((cmx_rad[i]-real_cmx)*(cmx_rad[i]-real_cmx)+
							(cmy_rad[i]-real_cmy)*(cmy_rad[i]-real_cmy));
						if (0.5*movie->com_radius2 > dist)
						{
							/* ...and have shifted little from the last radius CoM */
							dist_difference=sqrt(
								(cmx_rad[i]-cmx_rad[i-1])*(cmx_rad[i]-cmx_rad[i-1])+
								(cmy_rad[i]-cmy_rad[i-1])*(cmy_rad[i]-cmy_rad[i-1]));
							if (dist_difference<min_dist_difference)
							{
								min_dist_difference=dist_difference;
								radius=i;
							}
						}
					}
					real_cmx=cmx_rad[radius];
					real_cmy=cmy_rad[radius];
#if defined (OLD_CODE)
					centre_OK=0;
					/* now check clicking around the boundary does not lock on to
						a neighbouring point */
					for (i=radius-movie->com_radius2;(i>0)&&!centre_OK&&return_code;i--)
					{
						centre_OK=1;
						for (j=0;(j<4)&&centre_OK&&return_code;j++)
						{
							if (j<2)
							{
								edge_mx=mx-i;
							}
							else
							{
								edge_mx=mx+i;
							}
							if (0==j%2)
							{
								edge_my=my-i;
							}
							else
							{
								edge_my=my+i;
							}
							if (return_code=
								Texture_get_centre_of_mass(view->texture,edge_mx,edge_my,
									i,com_colour_index,com_threshold2,&cmx,&cmy)&&
								Texture_get_centre_of_mass(view->texture,(int)floor(cmx+0.5),
									(int)floor(cmy+0.5),i,com_colour_index,com_threshold2,
									&cmx,&cmy))
							{
								if (!(centre_OK=0.5*movie->com_radius2 >
									sqrt((cmx-real_cmx)*(cmx-real_cmx)+
										(cmy-real_cmy)*(cmy-real_cmy))))
								{
									radius--;
								}
							}
						}
					}
#endif /*defined (OLD_CODE)*/
					if (return_code)
					{
						view->com_radius[edit_node_index]=radius;
						/*???debug*/
						/*printf("new centre of mass radius = %i\n",radius);*/
						return_code=Mirage_movie_texture_to_3d_coordinates(movie,
							view_no,cmx_rad[radius],cmy_rad[radius],fact,new_position);
					}
				}
				DEALLOCATE(cmx_rad);
				DEALLOCATE(cmy_rad);
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Mirage_movie_auto_two_pass_centre_of_mass.  "
					"Could not get new radius");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Mirage_movie_auto_two_pass_centre_of_mass.  Could not get texture size");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Mirage_movie_auto_two_pass_centre_of_mass.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Mirage_movie_auto_two_pass_centre_of_mass */

static int place_node_in_Mirage_view(struct Mirage_movie *movie,int view_no,
	struct FE_node *node,double position[3])
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
If there are still nodes to be placed in view<view_no> of the <movie>, this
function will add the next node to be placed at the position specified - or
use photogrammetry calculations if the point is already in another view.
It also takes care of adding elements that have all their nodes in the
placed group to the placed_elements group, and for nodes that are now defined
in 3-D, it adds the nodes and the appropriate elements to the placed_3d group.
==============================================================================*/
{
	double view_pos[2],other_position[3],other_view_pos[2],new_position[3],
		win_pos[3],new_view_pos[2],scale_x,scale_y;
	int return_code,other_view_no;
	struct Mirage_view *view,*other_view;
	struct Add_elements_with_node_data add_data;
	struct FE_node *working_node;
	FE_value posx,posy,posz;

	ENTER(place_node_in_Mirage_view);
	if (movie&&(view=movie->views[view_no])&&node&&position)
	{
		/* find first other view the node is in, if any */
		other_view=(struct Mirage_view *)NULL;
		for (other_view_no=0;(!other_view)&&
					(other_view_no<movie->number_of_views);other_view_no++)
		{
			if (other_view_no != view_no)
			{
				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					get_FE_node_cm_node_identifier(node),
					movie->views[other_view_no]->placed_nodes))
				{
					other_view=movie->views[other_view_no];
				}
			}
		}
		if (other_view)
		{
			if (FE_node_get_position_cartesian(node,(struct FE_field *)NULL,&posx,
				&posy,&posz,(FE_value *)NULL))
			{
				other_position[0]=(double)posx;
				other_position[1]=(double)posy;
				other_position[2]=(double)posz;
				if (0==(movie->current_frame_no % 2))
				{
					scale_x=view->image0_width/view->crop0_width;
					scale_y=view->image0_height/view->crop0_height;
				}
				else
				{
					scale_x=view->image1_width/view->crop1_width;
					scale_y=view->image1_height/view->crop1_height;
				}
				return_code=(
					point_3d_to_2d_view(view->transformation43,position,view_pos)&&
					point_3d_to_2d_view(other_view->transformation43,
						other_position,other_view_pos)&&
					point_pair_to_3d(view_pos,view->transformation43,
						other_view_pos,other_view->transformation43,new_position)&&
					/* check the point has not shifted too far from first position */
					point_3d_to_2d_view(view->transformation43,new_position,
						new_view_pos)&&
					(fabs(view_pos[0]-new_view_pos[0]) < 4*scale_x*movie->com_radius1)&&
					(fabs(view_pos[1]-new_view_pos[1]) < 4*scale_y*movie->com_radius1)&&
					/* check the projected position is within the near-far range
						for the other view */
					photogrammetry_project(other_view->transformation43,
						other_view->near_clipping_plane,other_view->far_clipping_plane,
						other_view->NDC_left,other_view->NDC_bottom,
						other_view->NDC_width,other_view->NDC_height,new_position,win_pos)&&
					(0.01<win_pos[2])&&(0.99>win_pos[2]));
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			new_position[0]=position[0];
			new_position[1]=position[1];
			new_position[2]=position[2];
			return_code=1;
		}
		/* check the projected position is within the near-far range
			for the view */
		if (return_code)
		{
			posx=(FE_value)new_position[0];
			posy=(FE_value)new_position[1];
			posz=(FE_value)new_position[2];
			if (return_code=photogrammetry_project(view->transformation43,
				view->near_clipping_plane,view->far_clipping_plane,
				view->NDC_left,view->NDC_bottom,view->NDC_width,view->NDC_height,
				new_position,win_pos))
			{
				return_code=((0.01<win_pos[2])&&(0.99>win_pos[2]));
			}
		}
		if (return_code)
		{
			if ((working_node=CREATE(FE_node)(0,(struct FE_node *)NULL))&&
				MANAGER_COPY_WITHOUT_IDENTIFIER(FE_node,cm_node_identifier)(working_node,
				node))
			{
				if (return_code=(FE_node_set_position_cartesian(working_node,
					(struct FE_field *)NULL,posx,posy,posz)&&
					MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(node,working_node,
					movie->node_manager)))
				{
					if (return_code=
						ADD_OBJECT_TO_GROUP(FE_node)(node,view->placed_nodes))
					{
						add_data.max_dimension=2;
						add_data.node=node;
						add_data.node_group=view->placed_nodes;
						add_data.element_group=view->placed_elements;
						MANAGED_GROUP_BEGIN_CACHE(FE_element)(view->placed_elements);
						FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							add_elements_with_node_to_group,(void *)&add_data,
							movie->all_element_group);
						MANAGED_GROUP_END_CACHE(FE_element)(view->placed_elements);
					}
#if defined (OLD_CODE)
					if (return_code&&other_view)
					{
						add_data.node_group=movie->placed_nodes_3d;
						add_data.element_group=movie->placed_elements_3d;
						/* add node and associated elements to 3-D placed group */
						MANAGED_GROUP_BEGIN_CACHE(FE_element)(movie->placed_elements_3d);
						return_code=(
							ADD_OBJECT_TO_GROUP(FE_node)(node,movie->placed_nodes_3d)&&
							ADD_OBJECT_TO_GROUP(FE_node)(node,movie->pending_nodes_3d)&&
							ADD_OBJECT_TO_GROUP(FE_node)(node,view->pending_nodes)&&
							ADD_OBJECT_TO_GROUP(FE_node)(node,other_view->pending_nodes)&&
							FOR_EACH_OBJECT_IN_GROUP(FE_element)(
								add_elements_with_node_to_group,(void *)&add_data,
								movie->all_element_group));
						MANAGED_GROUP_END_CACHE(FE_element)(movie->placed_elements_3d);
					}
#endif /* defined (OLD_CODE) */
				}
			}
			else
			{
				return_code=0;
			}
			DESTROY(FE_node)(&working_node);
		}
		else
		{
			if (!return_code)
			{
				printf("\a");
				fflush(stdout);
			}
			display_message(ERROR_MESSAGE,
				"place_node_in_Mirage_view.  Could not get valid 3-D position");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"place_node_in_Mirage_view.  Invalid element");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* place_node_in_Mirage_view */

static int select_node_in_Mirage_view(struct Mirage_movie *movie,
	int view_no,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 6 July 1998

DESCRIPTION : To add a node to the pending lists without changing its position.
==============================================================================*/
{
	int return_code;
	struct Mirage_view *view;
	struct FE_node *working_node;

	ENTER(reposition_node_in_Mirage_view);
	if (movie&&(view=movie->views[view_no])&&node)
	{
#if defined (DEBUG)
		display_message(INFORMATION_MESSAGE,
			"select_node_in_Mirage_view.  Selecting node\n");
#endif /* defined (DEBUG) */
		
		if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
			get_FE_node_cm_node_identifier(node), view->placed_nodes))
		{
			if ((working_node=CREATE(FE_node)(0,(struct FE_node *)NULL))&&
				MANAGER_COPY_WITHOUT_IDENTIFIER(FE_node,cm_node_identifier)(
					working_node,node))
			{
				movie->modifiers = MIRAGE_MOVIE_MODIFIERS_TOGGLE_SELECT;
				if (MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
						node,working_node,movie->node_manager))
				{
					movie->modifiers = MIRAGE_MOVIE_MODIFIERS_NONE;
				}
				DESTROY(FE_node)(&working_node);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"select_node_in_Mirage_view.  Node not placed in view");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"select_node_in_Mirage_view.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* select_node_in_Mirage_view */

static int reposition_node_in_Mirage_view(struct Mirage_movie *movie,
	int view_no,struct FE_node *node,double position[3])
/*******************************************************************************
LAST MODIFIED : 4 November 1998

DESCRIPTION :
==============================================================================*/
{
	double view_pos[2],other_position[3],other_view_pos[2],new_position[3],
		win_pos[3];
	int return_code,other_view_no;
	struct Mirage_view *view,*other_view;
	struct FE_node *working_node;
	FE_value posx,posy,posz;

	ENTER(reposition_node_in_Mirage_view);
	if (movie&&(view=movie->views[view_no])&&node&&position)
	{
		if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
			get_FE_node_cm_node_identifier(node),view->placed_nodes))
		{
			/* find first other view the node is in, if any */
			other_view=(struct Mirage_view *)NULL;
			for (other_view_no=0;(!other_view)&&
				(other_view_no<movie->number_of_views);other_view_no++)
			{
				if (other_view_no != view_no)
				{
					if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
						get_FE_node_cm_node_identifier(node),
						movie->views[other_view_no]->placed_nodes))
					{
						other_view=movie->views[other_view_no];
					}
				}
			}
			if (other_view)
			{
				if (FE_node_get_position_cartesian(node,(struct FE_field *)NULL,&posx,
					&posy,&posz,(FE_value *)NULL))
				{
					other_position[0]=(double)posx;
					other_position[1]=(double)posy;
					other_position[2]=(double)posz;
					/* weight equations for first view 1000x those for second view */
					return_code=(
						point_3d_to_2d_view(view->transformation43,position,view_pos)&&
						point_3d_to_2d_view(other_view->transformation43,
							other_position,other_view_pos)&&
						weighted_point_pair_to_3d(view_pos,view->transformation43,1000.0,
							other_view_pos,other_view->transformation43,1.0,new_position)&&
						/* check the projected position is within the near-far range
							for the other view */
						photogrammetry_project(other_view->transformation43,
							other_view->near_clipping_plane,other_view->far_clipping_plane,
							other_view->NDC_left,other_view->NDC_bottom,
							other_view->NDC_width,other_view->NDC_height,
							new_position,win_pos)&&
						(0.01<win_pos[2])&&(0.99>win_pos[2]));
#if defined (DEBUG)
					/*???debug*/
					printf("2d positions 1: %13.6e %13.6e\n",view_pos[0],view_pos[1]);
					printf("T1:\n");
					print_matrix(4,3,view->transformation43,"%13.6e");
					printf("2d positions 2: %13.6e %13.6e\n",other_view_pos[0],other_view_pos[1]);
					printf("T2:\n");
					print_matrix(4,3,other_view->transformation43,"%13.6e");
					printf("-->  %13.6e %13.6e %13.6e\n",new_position[0],new_position[1],
						new_position[2]);
#endif /* defined (DEBUG) */
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				new_position[0]=position[0];
				new_position[1]=position[1];
				new_position[2]=position[2];
				return_code=1;
			}
			/* check the projected position is within the near-far range
				for the view */
			if (return_code)
			{
				posx=(FE_value)new_position[0];
				posy=(FE_value)new_position[1];
				posz=(FE_value)new_position[2];
				if (return_code=photogrammetry_project(view->transformation43,
					view->near_clipping_plane,view->far_clipping_plane,
					view->NDC_left,view->NDC_bottom,view->NDC_width,view->NDC_height,
					new_position,win_pos))
				{
					return_code=((0.01<win_pos[2])&&(0.99>win_pos[2]));
				}
			}
			if (return_code)
			{
				if ((working_node=CREATE(FE_node)(0,(struct FE_node *)NULL))&&
					MANAGER_COPY_WITHOUT_IDENTIFIER(FE_node,cm_node_identifier)(working_node,
					node))
				{
					if (return_code=(FE_node_set_position_cartesian(working_node,
						(struct FE_field *)NULL,posx,posy,posz)&&
						MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
						node,working_node,movie->node_manager)))
					{
					}
					DESTROY(FE_node)(&working_node);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"reposition_node_in_Mirage_view.  Could not get valid 3-D position");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"reposition_node_in_Mirage_view.  node not placed in view");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"reposition_node_in_Mirage_view.  Invalid element");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* reposition_node_in_Mirage_view */

static int remove_node_from_Mirage_view(struct Mirage_movie *movie,int view_no,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 4 November 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code,other_view_no;
	struct Mirage_view *other_view,*view;
	struct Remove_elements_with_node_data rem_data;

	ENTER(remove_node_from_Mirage_view);
	if (movie&&(view=movie->views[view_no])&&node)
	{
		return_code=1;
		if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
			get_FE_node_cm_node_identifier(node),view->placed_nodes))
		{
			rem_data.node=node;
			rem_data.element_group=view->placed_elements;
			MANAGED_GROUP_BEGIN_CACHE(FE_element)(view->placed_elements);
			return_code=(
				REMOVE_OBJECT_FROM_GROUP(FE_node)(node,view->placed_nodes)&&
				FOR_EACH_OBJECT_IN_GROUP(FE_element)(
					remove_elements_with_node_from_group,(void *)&rem_data,
					movie->all_element_group));
			MANAGED_GROUP_END_CACHE(FE_element)(view->placed_elements);
			if (return_code)
			{
				/* find other view the node is in, if any */
				other_view=(struct Mirage_view *)NULL;
				for (other_view_no=0;(!other_view)&&
					(other_view_no<movie->number_of_views);other_view_no++)
				{
					if (other_view_no != view_no)
					{
						if (IS_OBJECT_IN_GROUP(FE_node)(
							node,movie->views[other_view_no]->placed_nodes))
						{
							other_view=movie->views[other_view_no];
						}
					}
				}
#if defined (OLD_CODE)
				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node->cm_node_identifier,movie->pending_nodes_3d))
				{
					REMOVE_OBJECT_FROM_GROUP(FE_node)(node,movie->pending_nodes_3d);
					REMOVE_OBJECT_FROM_GROUP(FE_node)(node,view->pending_nodes);
					/* also remove from pending group in other view */
					REMOVE_OBJECT_FROM_GROUP(FE_node)(node,other_view->pending_nodes);
				}
				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node->cm_node_identifier,movie->problem_nodes_3d))
				{
					REMOVE_OBJECT_FROM_GROUP(FE_node)(node,movie->problem_nodes_3d);
					REMOVE_OBJECT_FROM_GROUP(FE_node)(node,view->problem_nodes);
					/* also remove from problem group in other view */
					REMOVE_OBJECT_FROM_GROUP(FE_node)(node,other_view->problem_nodes);
				}

				if (FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					node->cm_node_identifier,movie->placed_nodes_3d))
				{
					rem_data.element_group=movie->placed_elements_3d;
					MANAGED_GROUP_BEGIN_CACHE(FE_element)(movie->placed_elements_3d);
					return_code=
						REMOVE_OBJECT_FROM_GROUP(FE_node)(node,movie->placed_nodes_3d)&&
						FOR_EACH_OBJECT_IN_GROUP(FE_element)(
							remove_elements_with_node_from_group,(void *)&rem_data,
							movie->all_element_group);
					MANAGED_GROUP_END_CACHE(FE_element)(movie->placed_elements_3d);
				}
#endif /* defined (OLD_CODE) */
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"remove_node_from_Mirage_view.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* remove_node_from_Mirage_view */

#if defined (OLD_CODE_TO_KEEP)
struct Remove_node_iterator_data
{
	struct Mirage_movie *movie;
	int view_no;
};

static int remove_node_from_Mirage_view_iterator(struct FE_node *node,
	void *remove_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Not used at present, but may be needed for rubber banding to remove nodes.
==============================================================================*/
{
	int return_code;
	struct Remove_node_iterator_data *remove_data;

	ENTER(remove_node_from_Mirage_view_iterator);
	if (node&&(remove_data=(struct Remove_node_data *)remove_data_void))
	{
		return_code=remove_node_from_Mirage_view(remove_data->movie,
			remove_data->view_no,node);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"remove_node_from_Mirage_view_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* remove_node_from_Mirage_view_iterator */
#endif /* defined (OLD_CODE_TO_KEEP) */

struct Select_node_data
{
	struct GROUP(FE_node) *selected_nodes;
	struct MANAGER(FE_node) *node_manager;
}; /* Select_node_data */

static int Scene_picked_object_select_node(
	struct Scene_picked_object *picked_object,void *select_node_data_void)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Adds picked nodes to the selected_nodes group if they are not already there.
==============================================================================*/
{
	int node_number,return_code;
	struct FE_node *picked_node;
	struct Scene_object *scene_object;
	struct Select_node_data *select_node_data;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *settings;

	ENTER(Scene_picked_object_select_node);
	if (picked_object&&
		(select_node_data=(struct Select_node_data *)select_node_data_void))
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
				node_number,select_node_data->node_manager))
			{
				if (!FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
					get_FE_node_cm_node_identifier(picked_node),
					select_node_data->selected_nodes))
				{
					return_code=ADD_OBJECT_TO_GROUP(FE_node)(picked_node,
						select_node_data->selected_nodes);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_picked_object_select_node.  Node number not in manager");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_select_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_select_node */

struct Get_nearest_projected_node_data
{
	double transformation43[12],pointer[2],nearest_distance;
	struct FE_node *nearest_node;
}; /* Get_nearest_projected_node_data */

static int get_nearest_projected_node(struct FE_node *node,
	void *nearest_node_data_void)
/*******************************************************************************
LAST MODIFIED : 4 November 1998

DESCRIPTION :
Node iterator function for returning the nearest node to the pointer when
projected onto 2-D space with transformation43.
==============================================================================*/
{
	double pos3[3],pos2[2],distance;
	FE_value node_x,node_y,node_z;
	int return_code;
	struct Get_nearest_projected_node_data *nearest_node_data;

	ENTER(get_nearest_projected_node);
	if (node&&(nearest_node_data=
		(struct Get_nearest_projected_node_data *)nearest_node_data_void))
	{
		/* get the projected node position */
		if (FE_node_get_position_cartesian(node,(struct FE_field *)NULL,&node_x,
			&node_y,&node_z,(FE_value *)NULL))
		{
			pos3[0]=(double)node_x;
			pos3[1]=(double)node_y;
			pos3[2]=(double)node_z;
			if (return_code=
				point_3d_to_2d_view(nearest_node_data->transformation43,pos3,pos2))
			{
				pos2[0] -= nearest_node_data->pointer[0];
				pos2[1] -= nearest_node_data->pointer[1];
				distance=sqrt(pos2[0]*pos2[0]+pos2[1]*pos2[1]);
				if (!(nearest_node_data->nearest_node) ||
					(distance < nearest_node_data->nearest_distance))
				{
					nearest_node_data->nearest_distance=distance;
					nearest_node_data->nearest_node=node;
				}
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
			"get_nearest_projected_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_nearest_projected_node */

enum MNE_scene_input_drag_mode
{
	MNE_DRAG_NOTHING,
	MNE_DRAG_PLACE_COM,
	MNE_DRAG_PLACE_AUTO_COM,
	MNE_DRAG_PLACE_EXACT,
	MNE_DRAG_MOVE_COM,
	MNE_DRAG_MOVE_EXACT,
	MNE_DRAG_UNPLACE,
	MNE_DRAG_SELECT_ONLY
};

static void MNE_scene_input_callback(struct Scene *scene,
	void *mirage_node_editor_void,
	struct Scene_input_callback_data *scene_input_data)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Receives mouse button press, motion and release events from <scene>, and
processes them into node movements as necessary.
==============================================================================*/
{
	double position[3],fact;
	int i,node_no,view_no,shift_key_down,control_key_down,alt_key_down;
	static int edit_node_index,last_edit_node_no=-1;
	struct Mirage_node_editor *node_editor;
	static enum MNE_scene_input_drag_mode drag_mode=MNE_DRAG_NOTHING;
	struct Select_node_data select_node_data;
	static struct FE_node *edit_node;
	struct Mirage_movie *movie;
	struct Mirage_view *view;
#if defined (OLD_CODE_TO_KEEP)
	struct Remove_node_iterator_data remove_data;
#endif /* defined (OLD_CODE_KEEP) */
	struct Get_nearest_projected_node_data nearest_node_data;

	if (scene&&(node_editor=(struct Mirage_node_editor *)
		mirage_node_editor_void)&&scene_input_data&&(movie=node_editor->movie))
	{
		/* determine which view the scene is from */
		view=(struct Mirage_view *)NULL;
		for (i=0;!view&&(i<movie->number_of_views);i++)
		{
			if (scene==(movie->views[i])->scene)
			{
				view=movie->views[i];
				view_no=i;
			}
		}
		if (view)
		{
			/* determine state of shift, control and alt keys */
			shift_key_down=SCENE_INPUT_MODIFY_SHIFT&scene_input_data->input_modifier;
			control_key_down=
				SCENE_INPUT_MODIFY_CONTROL&scene_input_data->input_modifier;
			alt_key_down=SCENE_INPUT_MODIFY_ALT&scene_input_data->input_modifier;
			fact=0.95;
			position[0]=
				fact*scene_input_data->nearx+(1.0-fact)*scene_input_data->farx;
			position[1]=
				fact*scene_input_data->neary+(1.0-fact)*scene_input_data->fary;
			position[2]=
				fact*scene_input_data->nearz+(1.0-fact)*scene_input_data->farz;
			switch (scene_input_data->input_type)
			{
				case SCENE_BUTTON_PRESS:
				{
					drag_mode=MNE_DRAG_NOTHING;
					/*???debug *//*printf("MNE: button press!\n");*/
					/* get list of picked nodes */
					select_node_data.selected_nodes=node_editor->selected_nodes;
					select_node_data.node_manager=movie->node_manager;
					REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(node_editor->selected_nodes);
					FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
						Scene_picked_object_select_node,(void *)&select_node_data,
						scene_input_data->picked_object_list);
					edit_node=(struct FE_node *)NULL;
					if (0<NUMBER_IN_GROUP(FE_node)(node_editor->selected_nodes))
					{
						/* get nearest picked node to edit */
						if (point_3d_to_2d_view(view->transformation43,position,
							nearest_node_data.pointer))
						{
							for (i=0;i<12;i++)
							{
								nearest_node_data.transformation43[i]=
									view->transformation43[i];
							}
							nearest_node_data.nearest_distance=0.0;
							nearest_node_data.nearest_node=(struct FE_node *)NULL;
							/* preferentially choose the last edited node */
							if ((nearest_node_data.nearest_node=FIND_BY_IDENTIFIER_IN_GROUP(
								FE_node,cm_node_identifier)(last_edit_node_no,
								node_editor->selected_nodes))||
								FOR_EACH_OBJECT_IN_GROUP(FE_node)(get_nearest_projected_node,
								(void *)&nearest_node_data,node_editor->selected_nodes))
							{
								edit_node=nearest_node_data.nearest_node;
								last_edit_node_no=get_FE_node_cm_node_identifier(edit_node);
								/* get edit_node_index */
								edit_node_index= -1;
								for (i=0;(0>edit_node_index)&&(i<view->number_of_nodes);i++)
								{
									if (view->node_numbers[i]==
										get_FE_node_cm_node_identifier(edit_node))
									{
										edit_node_index=i;
									}
								}
								if (0>edit_node_index)
								{
									display_message(ERROR_MESSAGE,
										"MNE_scene_input_callback.  Missing node index");
									drag_mode=MNE_DRAG_NOTHING;
									edit_node=(struct FE_node *)NULL;
								}
								else
								{
									if (alt_key_down)
									{
										drag_mode=MNE_DRAG_UNPLACE;
									}
									else
									{
										if (shift_key_down)
										{
											drag_mode=MNE_DRAG_MOVE_EXACT;
											/* turn off centre of mass */
											view->com_radius[edit_node_index]=0;
										}
										else
										{
											if ( control_key_down )
											{
												drag_mode=MNE_DRAG_SELECT_ONLY;
											}
											else
											{
												drag_mode=MNE_DRAG_MOVE_COM;
											}
										}
									}
								}
							}
						}
					}
					else
					{
						if (!alt_key_down && !control_key_down)
						{
							/* If alt_key_down don't select the node as this is delete mode
							   if control_key_down don't select node as this is 
								make_pending but don't move mode*/
							/* get the next node to add to this view, if any */
							edit_node=(struct FE_node *)NULL;
							last_edit_node_no=-1;
							for (i=0;!edit_node&&(i<view->number_of_nodes);i++)
							{
								node_no=view->node_numbers[i];
								if (!FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
									node_no,view->placed_nodes))
								{
									edit_node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
										node_no,movie->node_manager);
									edit_node_index=i;
								}
							}
						}
						if (edit_node)
						{
							last_edit_node_no=get_FE_node_cm_node_identifier(edit_node);
							if (shift_key_down)
							{
								drag_mode=MNE_DRAG_PLACE_EXACT;
								/* turn off centre of mass */
								view->com_radius[edit_node_index]=0;
							}
							else
							{
								drag_mode=MNE_DRAG_PLACE_COM;
							}
						}
					}
				} break;
				case SCENE_MOTION_NOTIFY:
				{
					/*???debug *//*printf("MNE: motion notify!\n");*/
				} break;
				case SCENE_BUTTON_RELEASE:
				{
					/*???debug *//*printf("MNE: button release!\n");*/
					if (edit_node)
					{
						switch (drag_mode)
						{
						case MNE_DRAG_UNPLACE:
							{
								/* remove edit node from view */
								remove_node_from_Mirage_view(movie,view_no,edit_node);
#if defined (OLD_CODE_TO_KEEP)
								/* remove all selected nodes from view */
								remove_data.movie=movie;
								remove_data.view_no=view_no;
								FOR_EACH_OBJECT_IN_GROUP(FE_node)(
									remove_node_from_Mirage_view_iterator,(void *)&remove_data,
									node_editor->selected_nodes);
#endif /* defined (OLD_CODE_TO_KEEP) */
							} break;
						case MNE_DRAG_MOVE_COM:
							{
								if (Mirage_movie_auto_two_pass_centre_of_mass(movie,view_no,
									position,edit_node_index,fact,position))
								{
									reposition_node_in_Mirage_view(movie,view_no,edit_node,
										position);
								}
							} break;
						case MNE_DRAG_MOVE_EXACT:
							{
								reposition_node_in_Mirage_view(movie,view_no,edit_node,
									position);
							} break;
						case MNE_DRAG_PLACE_COM:
							{
								if (Mirage_movie_auto_two_pass_centre_of_mass(movie,view_no,
									position,edit_node_index,fact,position))
								{
									place_node_in_Mirage_view(movie,view_no,edit_node,position);
								}
							} break;
						case MNE_DRAG_PLACE_EXACT:
							{
								place_node_in_Mirage_view(movie,view_no,edit_node,position);
							} break;
						case MNE_DRAG_SELECT_ONLY:
							{
								select_node_in_Mirage_view(movie,view_no,edit_node);
							} break;
						default:
							{
								/* should not be here */
							} break;
						}
					}
					edit_node=(struct FE_node *)NULL;
					drag_mode=MNE_DRAG_NOTHING;
				} break;
			default:
				{
					display_message(ERROR_MESSAGE,
						"MNE_scene_input_callback.  Invalid input_type");
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"MNE_scene_input_callback.  Scene not owned by a view");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MNE_scene_input_callback.  Invalid argument(s)");
	}
} /* MNE_scene_input_callback */

/*
Global functions
----------------
*/
struct Mirage_node_editor *CREATE(Mirage_node_editor)(
	struct Mirage_movie *movie)
/*******************************************************************************
LAST MODIFIED : 4 November 1998

DESCRIPTION :
Creates a mirage node editor and sets it up so that all view->scenes in the
movie structure send input to this graphical node editor.
This editor is only for use with digitiser_windows.
Note that it may only be created after calling enable_Mirage_movie_graphics
for the movie, and must be destroyed before the views.
==============================================================================*/
{
	int view_no;
	struct Mirage_node_editor *mirage_node_editor;
	struct Scene_input_callback input_callback;

	ENTER(CREATE(Mirage_node_editor));
	/* check arguments */
	if (movie&&movie->views&&movie->node_manager)
	{
		if (ALLOCATE(mirage_node_editor,struct Mirage_node_editor,1)&&
			(mirage_node_editor->working_node=CREATE(FE_node)(0,(struct FE_node *)NULL))&&
			(mirage_node_editor->selected_nodes=CREATE(GROUP(FE_node))(
			"Selected")))
		{
			mirage_node_editor->movie=movie;
			input_callback.procedure=MNE_scene_input_callback;
			input_callback.data=(void *)mirage_node_editor;
			for (view_no=0;view_no<movie->number_of_views;view_no++)
			{
				Scene_set_input_callback((movie->views[view_no])->scene,
					&input_callback);
			}
			mirage_node_editor->mode=MNE_EDIT_OFF;
			mirage_node_editor->node=(struct FE_node *)NULL;
			mirage_node_editor->graphics_object=(struct GT_object *)NULL;
		}
		else
		{
			if (mirage_node_editor)
			{
				DESTROY(FE_node)(&(mirage_node_editor->working_node));
				DEALLOCATE(mirage_node_editor);
			}
			display_message(ERROR_MESSAGE,
				"CREATE(Mirage_node_editor).  Could not ALLOCATE mirage_node_editor");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Mirage_node_editor).  Invalid argument(s)");
		mirage_node_editor=(struct Mirage_node_editor *)NULL;
	}
	LEAVE;

	return (mirage_node_editor);
} /* CREATE(Mirage_node_editor) */

int DESTROY(Mirage_node_editor)(
	struct Mirage_node_editor **mirage_node_editor_address)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Cleans up space used by Mirage_node_editor structure.
==============================================================================*/
{
	int return_code,view_no;
	struct Mirage_node_editor *node_editor;
	struct Scene_input_callback input_callback;

	ENTER(DESTROY(Mirage_node_editor));
	if (mirage_node_editor_address&&(node_editor=*mirage_node_editor_address))
	{
		/* turn off scene input callbacks */
		input_callback.procedure=(Scene_input_callback_procedure *)NULL;
		input_callback.data=(void *)NULL;
		for (view_no=0;view_no<node_editor->movie->number_of_views;view_no++)
		{
			Scene_set_input_callback((node_editor->movie->views[view_no])->scene,
				&input_callback);
		}
		DESTROY(GROUP(FE_node))(&node_editor->selected_nodes);
		DESTROY(FE_node)(&node_editor->working_node);
		DEALLOCATE(*mirage_node_editor_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Mirage_node_editor).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Mirage_node_editor) */
