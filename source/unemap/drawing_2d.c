/*******************************************************************************
FILE : drawing_2d.c

LAST MODIFIED : 24 November 1999

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "unemap/drawing_2d.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
struct Colour_map_pixel
/*******************************************************************************
LAST MODIFIED : 20 May 1998

DESCRIPTION :
Used in get_Drawing_2d_image, very similar to type in general/image_utilities.c
==============================================================================*/
{
	Pixel pixel;
	unsigned char blue,green,red;
	int access_count;
}; /* struct Colour_map_pixel */

DECLARE_LIST_TYPES(Colour_map_pixel);

FULL_DECLARE_INDEXED_LIST_TYPE(Colour_map_pixel);

/*
Module functions
----------------
*/
DECLARE_DEFAULT_DESTROY_OBJECT_FUNCTION(Colour_map_pixel)

DECLARE_OBJECT_FUNCTIONS(Colour_map_pixel)

static int compare_Pixel(Pixel pixel_1,Pixel pixel_2)
{
	int return_code;

	ENTER(compare_Pixel);
	if (pixel_1<pixel_2)
	{
		return_code= -1;
	}
	else
	{
		if (pixel_1==pixel_2)
		{
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* compare_Pixel */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Colour_map_pixel,pixel,Pixel, \
	compare_Pixel)

DECLARE_INDEXED_LIST_FUNCTIONS(Colour_map_pixel)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Colour_map_pixel,pixel, \
	Pixel,compare_Pixel)

/*
Global functions
----------------
*/
struct Drawing_2d *create_Drawing_2d(Widget widget,int width,int height,
	enum Create_drawing_2d_image create_image,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
This function allocates memory for a drawing and initializes the fields to the
specified values.  It returns a pointer to the created drawing if successful
and NULL if unsuccessful.
==============================================================================*/
{
	char *image_data;
	Display *display;
#if defined (OLD_CODE)
	int bit_map_bit_order,image_byte_order;
#endif /* defined (OLD_CODE) */
	int bit_map_pad,bit_map_unit,scan_line_bytes;
	struct Drawing_2d *drawing;

	ENTER(create_Drawing_2d);
	if (widget&&user_interface)
	{
		/* create drawing */
		if (ALLOCATE(drawing,struct Drawing_2d,1))
		{
			drawing->widget=widget;
			drawing->width=width;
			drawing->height=height;
			drawing->user_interface=user_interface;
			display=user_interface->display;
			/* create pixel map */
			XtVaGetValues(widget,
				XmNdepth,&(drawing->depth),
				NULL);
			if (drawing->pixel_map=XCreatePixmap(display,
				XRootWindow(display,XDefaultScreen(display)),
				(unsigned int)width,(unsigned int)height,
				(unsigned int)(drawing->depth)))
			{
				if (DRAWING_IMAGE==create_image)
				{
#if defined (OLD_CODE)
					bit_map_bit_order=BitmapBitOrder(display);
					image_byte_order=ImageByteOrder(display);
#endif /* defined (OLD_CODE) */
					/* the number of bits for each pixel */
					bit_map_unit=BitmapUnit(display);
					/* each scan line occupies a multiple of this number of bits */
					bit_map_pad=BitmapPad(display);
					/* create image */
					if (!((ALLOCATE(image_data,char,drawing->height*
						(scan_line_bytes=(((drawing->width*bit_map_unit-1)/
						bit_map_pad+1)*bit_map_pad-1)/8+1)))&&
						(drawing->image=XCreateImage(display,XDefaultVisual(display,
						XDefaultScreen(display)),drawing->depth,ZPixmap,0,image_data,
						drawing->width,drawing->height,bit_map_pad,scan_line_bytes))))
							/*???DB.  Should screen_visual be in user_interface ? */
					{
						display_message(ERROR_MESSAGE,
							"create_Drawing_2d.  Could not create image");
						DEALLOCATE(image_data);
						drawing->image=(XImage *)NULL;
					}
				}
				else
				{
					/* do not create image */
					drawing->image=(XImage *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Drawing_2d.  Could not create pixel map");
				DEALLOCATE(drawing);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Drawing_2d.  Could not allocate drawing");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Drawing_2d.  Invalid argument(s)");
	}
	LEAVE;

	return (drawing);
} /* create_Drawing_2d */

int destroy_Drawing_2d(struct Drawing_2d **drawing)
/*******************************************************************************
LAST MODIFIED : 26 December 1996

DESCRIPTION :
This function frees the memory associated with the fields of <**drawing>, frees
the memory for <**drawing> and changes <*drawing> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Drawing_2d);
	if (drawing&&(*drawing)&&((*drawing)->user_interface))
	{
		/* free the pixel map */
		XFreePixmap((*drawing)->user_interface->display,(*drawing)->pixel_map);
		/* free the image */
		if ((*drawing)->image)
		{
			DEALLOCATE((*drawing)->image->data);
			XFree((char *)((*drawing)->image));
		}
		/* free the drawing structure */
		DEALLOCATE(*drawing);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_Drawing_2d */

unsigned long *get_Drawing_2d_image(struct Drawing_2d *drawing)
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Allocates an image, suitable for passing to the image_utilities, and fills it
with the current contents of the <drawing>.
==============================================================================*/
{
	XImage *pixel_image;
	Display *display;
	int column,row;
	Pixel pixel;
	struct Colour_map_pixel *colour_map_entry;
	struct LIST(Colour_map_pixel) *colour_map_list;
	unsigned char *image_entry;
	unsigned long *image;
	XColor colour;
	XWindowAttributes window_attributes;
#if defined (OLD_CODE)
	Display *display;
	int column,count,row;
	Pixel pixel;
	unsigned char *colour_blue,*colour_green,*colour_red,*blue,*green,
		*image_entry,*red;
	unsigned long *image;
	XColor *colour,*colours;
	XImage *pixel_image;
	XVisualInfo *visual_info,visual_info_template;
	XWindowAttributes window_attributes;
#endif /* defined (OLD_CODE) */

	ENTER(get_Drawing_2d_image);
	image=(unsigned long *)NULL;
	/* check arguments */
	if (drawing)
	{
		display=drawing->user_interface->display;
		if (pixel_image=XGetImage(display,drawing->pixel_map,0,0,drawing->width,
			drawing->height,(unsigned long)0xffffffff,ZPixmap))
		{
			/* find the colour map and its size */
			XGetWindowAttributes(display,XtWindow(drawing->widget),
				&window_attributes);
			if (colour_map_list=CREATE(LIST(Colour_map_pixel))())
			{
				if (ALLOCATE(image,unsigned long,(drawing->width)*(drawing->height)))
				{
					image_entry=(unsigned char *)image;
					row=drawing->height;
					while (image&&(row>0))
					{
						row--;
						column=0;
						while (image&&(column<drawing->width))
						{
							pixel=XGetPixel(pixel_image,column,row);
							if (!(colour_map_entry=FIND_BY_IDENTIFIER_IN_LIST(
								Colour_map_pixel,pixel)(pixel,colour_map_list)))
							{
								colour.pixel=pixel;
								colour.pad=0;
								XQueryColor(display,window_attributes.colormap,&colour);
								if (ALLOCATE(colour_map_entry,struct Colour_map_pixel,1))
								{
									colour_map_entry->pixel=pixel;
									colour_map_entry->access_count=0;
									colour_map_entry->red=(unsigned char)((colour.red)>>8);
									colour_map_entry->green=(unsigned char)((colour.green)>>8);
									colour_map_entry->blue=(unsigned char)((colour.blue)>>8);
									if (!ADD_OBJECT_TO_LIST(Colour_map_pixel)(colour_map_entry,
										colour_map_list))
									{
										DEALLOCATE(image);
									}
								}
								else
								{
									DEALLOCATE(image);
								}
							}
							if (image)
							{
								*image_entry=colour_map_entry->red;
								image_entry++;
								*image_entry=colour_map_entry->green;
								image_entry++;
								*image_entry=colour_map_entry->blue;
								image_entry += 2;
							}
							column++;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_Drawing_2d_image.  Could not allocate image");
				}
				DESTROY(LIST(Colour_map_pixel))(&colour_map_list);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_Drawing_2d_image.  Could not allocate colour_map_list");
			}
#if defined (OLD_CODE)
			visual_info_template.visualid=
				XVisualIDFromVisual(window_attributes.visual);
			colours=(XColor *)NULL;
			colour_red=(unsigned char *)NULL;
			colour_green=(unsigned char *)NULL;
			colour_blue=(unsigned char *)NULL;
			if ((visual_info=XGetVisualInfo(display,VisualIDMask,
				&visual_info_template,&count))&&(1==count)&&
				ALLOCATE(colours,XColor,visual_info->colormap_size)&&
				ALLOCATE(colour_red,unsigned char,visual_info->colormap_size)&&
				ALLOCATE(colour_green,unsigned char,visual_info->colormap_size)&&
				ALLOCATE(colour_blue,unsigned char,visual_info->colormap_size))
			{
				colour=colours+(visual_info->colormap_size);
				for (count=visual_info->colormap_size;count>0;)
				{
					colour--;
					count--;
					colour->pixel=count;
					colour->pad=0;
				}
				XQueryColors(display,window_attributes.colormap,colours,
					visual_info->colormap_size);
				colour=colours;
				red=colour_red;
				green=colour_green;
				blue=colour_blue;
				for (count=visual_info->colormap_size;count>0;count--)
				{
					*red=(unsigned char)((colour->red)>>8);
					*green=(unsigned char)((colour->green)>>8);
					*blue=(unsigned char)((colour->blue)>>8);
					colour++;
					red++;
					green++;
					blue++;
				}
				if (ALLOCATE(image,unsigned long,(drawing->width)*(drawing->height)))
				{
					image_entry=(unsigned char *)image;
					for (row=drawing->height-1;row>=0;row--)
					{
						for (column=0;column<drawing->width;column++)
						{
							pixel=XGetPixel(pixel_image,column,row);
							*image_entry=colour_red[pixel];
							image_entry++;
							*image_entry=colour_green[pixel];
							image_entry++;
							*image_entry=colour_blue[pixel];
							image_entry += 2;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"get_Drawing_2d_image.  Could not allocate image");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_Drawing_2d_image.  Could not retrieve colour map");
			}
			DEALLOCATE(colours);
			DEALLOCATE(colour_red);
			DEALLOCATE(colour_green);
			DEALLOCATE(colour_blue);
#endif /* defined (OLD_CODE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_Drawing_2d_image.  Could not get pixel image");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_Drawing_2d_image.  Missing drawing");
	}
	LEAVE;

	return (image);
} /* get_Drawing_2d_image */
