/*******************************************************************************
FILE : motif/image_utilities.c

LAST MODIFIED : 5 July 2002

DESCRIPTION :
Utilities for handling images with X and Motif.
==============================================================================*/
#include <Xm/Xm.h>
#include <stdio.h>
#include <stdlib.h>
#include "general/debug.h"
#include "general/image_utilities.h"
#include "motif/image_utilities.h"
#include "user_interface/message.h"

/*
Module functions
----------------
*/
#if defined (OLD_CODE)
static int find_pixel_offset(unsigned long mask, int *offset, unsigned long *word_mask)
/*******************************************************************************
LAST MODIFIED : 8 July 2002

DESCRIPTION :
Creates a single Cmgui_image which stores the data from the X <pixmap>.
==============================================================================*/
{
	int i, return_code;

	ENTER(find_pixel_offset);

	printf( "mask %lu\n",
		mask);
	if (mask)
	{
		for (i = 0 ; !(*offset) && (i < 32) ; i++)
		{
			if (1 & mask)
			{
				*offset = i;
			}
			mask >>= 1;
		}
		if (*offset)
		{
			*word_mask = mask;
		}
	}
	else
	{
		*offset = 0;
		*word_mask = 0;
	}
	printf( "mask %lu offset %d word_mask %lu\n",
		mask, *offset, *word_mask);
	return_code = 1;

	LEAVE;

	return(return_code);
} /* find_pixel_offset */
#endif /* defined (OLD_CODE) */

/*
Global functions
----------------
*/

struct Cmgui_image *create_Cmgui_image_from_Pixmap(Display *display,
	Pixmap pixmap)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Creates a single Cmgui_image which stores the data from the X <pixmap>.
==============================================================================*/
{
	int k, number_of_allocated_colours, number_of_colours, x, y;
	struct Cmgui_image *image;
	unsigned char *char_ximage_data, *pixels, *pixel_data;
	unsigned int border_width, depth, height, i, j, width;
	Window root_window;
	XColor xcolour, *xcolour_array;
	XImage *ximage;
	XWindowAttributes window_attributes;

	ENTER(create_Cmgui_image_from_Pixmap);
	
	if (display && pixmap)
	{
		XGetWindowAttributes(display, DefaultRootWindow(display),
			&window_attributes);
		XGetGeometry(display, pixmap, &root_window, &x,
			&y, &width, &height, &border_width, &depth);
		if (ximage = XGetImage(display, pixmap, 0, 0, width, height,
			(unsigned long)0xffffffff,ZPixmap))
		{
			if (ALLOCATE(pixels, unsigned char, width*height*3))
			{
				if ((32 == ximage->bits_per_pixel) && (ximage->format == ZPixmap)
					&& (MSBFirst == ximage->byte_order))
				{
					pixel_data = pixels;
					for (j = 0 ; j < height ; j++)
					{
						char_ximage_data = (unsigned char *)(ximage->data + j * ximage->bytes_per_line);
						for (i = 0 ; i < width ; i++)
						{
							pixel_data[0] = char_ximage_data[1];
							pixel_data[1] = char_ximage_data[2];
							pixel_data[2] = char_ximage_data[3];
							pixel_data += 3;
							char_ximage_data += 4;
						}
					}
				}
				else if ((32 == ximage->bits_per_pixel) && (ximage->format == ZPixmap)
					&& (MSBFirst != ximage->byte_order))
				{
					pixel_data = pixels;
					for (j = 0 ; j < height ; j++)
					{
						char_ximage_data = (unsigned char *)(ximage->data + j * ximage->bytes_per_line);
						for (i = 0 ; i < width ; i++)
						{
							pixel_data[0] = char_ximage_data[2];
							pixel_data[1] = char_ximage_data[1];
							pixel_data[2] = char_ximage_data[0];
							pixel_data += 3;
							char_ximage_data += 4;
						}
					}
				}
				else 
				{
					if ((8 == ximage->bits_per_pixel) && (ximage->format == ZPixmap))
					{
						xcolour_array = NULL;
						number_of_colours = 0;
						number_of_allocated_colours = 0;
						pixel_data = pixels;
						for (j = 0 ; j < height ; j++)
						{
							char_ximage_data = (unsigned char *)(ximage->data + j * ximage->bytes_per_line);
							for (i = 0 ; i < width ; i++)
							{
								xcolour.pixel = (unsigned long)(*char_ximage_data);
								k = 0;
								while ((k < number_of_colours) && 
									(xcolour.pixel != xcolour_array[k].pixel))
								{
									k++;
								}
								if (k >= number_of_colours)
								{
									number_of_colours++;
									if (number_of_colours >= number_of_allocated_colours)
									{
										number_of_allocated_colours += 10;
										REALLOCATE(xcolour_array, xcolour_array, XColor,
											number_of_allocated_colours);
									}
									xcolour_array[k].pixel = xcolour.pixel;
								}
								/* Store the index number for later */
								pixel_data[0] = k;
								pixel_data += 3;
								char_ximage_data++;
							}
						}
					}
					else
					{
						/* This is the default, it is general but slow */
						xcolour_array = NULL;
						number_of_colours = 0;
						number_of_allocated_colours = 0;
						pixel_data = pixels;
						for (j = 0 ; j < height ; j++)
						{
							for (i = 0 ; i < width ; i++)
							{
								xcolour.pixel = XGetPixel(ximage, i, j);
								k = 0;
								while ((k < number_of_colours) && 
									(xcolour.pixel != xcolour_array[k].pixel))
								{
									k++;
								}
								if (k >= number_of_colours)
								{
									number_of_colours++;
									if (number_of_colours >= number_of_allocated_colours)
									{
										number_of_allocated_colours += 10;
										REALLOCATE(xcolour_array, xcolour_array, XColor,
											number_of_allocated_colours);
									}
									xcolour_array[k].pixel = xcolour.pixel;
								}
								/* Store the index number for later */
								pixel_data[0] = k;
								pixel_data += 3;
							}
						}
					}
					XQueryColors(display, window_attributes.colormap,
						xcolour_array, number_of_colours);
					pixel_data = pixels;
					for (j = 0 ; j < height ; j++)
					{
						for (i = 0 ; i < width ; i++)
						{
							k = pixel_data[0];
							pixel_data[0] =
								(unsigned char)(xcolour_array[k].red >> 8);
							pixel_data[1] =
								(unsigned char)(xcolour_array[k].green >> 8);
							pixel_data[2] =
								(unsigned char)(xcolour_array[k].blue >> 8);
							pixel_data += 3;
						}
					}
					DEALLOCATE(xcolour_array);
				}
				image = Cmgui_image_constitute(width, height,
					/*number_of_components*/3, /*number_of_bytes_per_component*/1,
					/*source_width_bytes*/3*width, pixels);
				DEALLOCATE(pixels);
			}
			else
			{
				display_message(WARNING_MESSAGE, "create_Cmgui_image_from_Pixmap.  "
					"Unable to allocate pixel memory.");
				image=(struct Cmgui_image *)NULL;
			}
			XDestroyImage(ximage);
		}
		else
		{
			display_message(WARNING_MESSAGE, "create_Cmgui_image_from_Pixmap.  "
				"Unable to create an X Image.");
			image=(struct Cmgui_image *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Cmgui_image_from_Pixmap.  "
			"Invalid argument(s)");
		image=(struct Cmgui_image *)NULL;
	}
	LEAVE;

	return(image);
} /* create_Cmgui_image_from_Pixmap */

Pixmap create_Pixmap_from_Cmgui_image(Display *display,
	struct Cmgui_image *cmgui_image, int depth)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Generates an X Pixmap which represents the <cmgui_image>.
==============================================================================*/
{
	char *ximage_data;
	GC gc;
	int height, i, j, k, number_of_allocated_colours, number_of_colours, width;
	Pixmap pixmap;
	unsigned char *char_ximage_data, *pixels, *pixel_data;
	XColor xcolour, *xcolour_array;
	XImage *ximage;
	XWindowAttributes window_attributes;

	ENTER(create_Cmgui_image_from_Pixmap);

	if (cmgui_image)
	{
		XGetWindowAttributes(display, DefaultRootWindow(display),
			&window_attributes);
		width = Cmgui_image_get_width(cmgui_image);
		height = Cmgui_image_get_height(cmgui_image);
		/* Allocate the ximage data using malloc as we are
			going to pass it into the ximage and then it manages
			the free.  I am making it 4 * width * height as this
			is sufficient for the visuals and displays we are using */
		if (ALLOCATE(pixels, unsigned char, width * height * 3) &&
			(ximage_data = (char *)malloc(width * height * 4)))
		{
			if (Cmgui_image_dispatch(cmgui_image, 0, 0, 0, width, height,
				0, 0, NULL, pixels))
			{
				if (pixmap = XCreatePixmap(display, DefaultRootWindow(display),
					width, height, depth))
				{
					gc = XCreateGC(display, pixmap, 0, NULL);
					XSetForeground(display, gc, BlackPixel(display, 
						DefaultScreen(display)));
					XSetBackground(display, gc, WhitePixel(display, 
						DefaultScreen(display)));
					XSetFunction(display, gc, GXcopy);
					ximage = XCreateImage(display, XDefaultVisual(display,
						XDefaultScreen(display)), depth, ZPixmap,
						0, ximage_data, width, height, 32, 0);
					if ((32 == ximage->bits_per_pixel) && (ximage->format == ZPixmap)
						&& (MSBFirst == ximage->byte_order))
					{
						pixel_data = pixels;
						for (j = 0 ; j < height ; j++)
						{
							char_ximage_data = (unsigned char *)(ximage->data + j * ximage->bytes_per_line);
							for (i = 0 ; i < width ; i++)
							{
								char_ximage_data[0] = 0;
								char_ximage_data[1] = pixel_data[0];
								char_ximage_data[2] = pixel_data[1];
								char_ximage_data[3] = pixel_data[2];
								pixel_data += 3;
								char_ximage_data += 4;
							}
						}
					}
					else if ((32 == ximage->bits_per_pixel) && (ximage->format == ZPixmap)
						&& (MSBFirst != ximage->byte_order))
					{
						pixel_data = pixels;
						for (j = 0 ; j < height ; j++)
						{
							char_ximage_data = (unsigned char *)(ximage->data + j * ximage->bytes_per_line);
							for (i = 0 ; i < width ; i++)
							{
								char_ximage_data[0] = pixel_data[2];
								char_ximage_data[1] = pixel_data[1];
								char_ximage_data[2] = pixel_data[0];
								char_ximage_data[3] = 0;
								pixel_data += 3;
								char_ximage_data += 4;
							}
						}
					}
					else
					{
						xcolour_array = NULL;
						number_of_allocated_colours = 0;
						number_of_colours = 0;
						pixel_data = pixels;
						for (j = 0 ; j < height ; j++)
						{
							for (i = 0 ; i < width ; i++)
							{
								xcolour.red = (pixel_data[0] << 8);
								xcolour.green = (pixel_data[1] << 8);
								xcolour.blue = (pixel_data[2] << 8);
								k = 0;
								while ((k < number_of_colours) && 
									((xcolour.red != xcolour_array[k].red) ||
									(xcolour.green != xcolour_array[k].green) ||
									(xcolour.blue != xcolour_array[k].blue)))
								{
									k++;
								}
								if (k >= number_of_colours)
								{
									number_of_colours++;
									if (number_of_colours >= number_of_allocated_colours)
									{
										number_of_allocated_colours += 10;
										REALLOCATE(xcolour_array, xcolour_array, XColor,
											number_of_allocated_colours);
									}
									xcolour_array[k].red = xcolour.red;
									xcolour_array[k].green = xcolour.green;
									xcolour_array[k].blue = xcolour.blue;									
								}
								/* Store the index number for later */
								pixel_data[0] = k;
								pixel_data += 3;
							}
						}
						for (k = 0 ; k < number_of_colours ; k++)
						{
							XAllocColor(display, window_attributes.colormap,
								xcolour_array + k);
						}
						if ((8 == ximage->bits_per_pixel) && (ximage->format == ZPixmap))
						{
							pixel_data = pixels;
							for (j = 0 ; j < height ; j++)
							{
								char_ximage_data = (unsigned char *)(ximage->data + j * ximage->bytes_per_line);
								for (i = 0 ; i < width ; i++)
								{
									k = pixel_data[0];
									*char_ximage_data = (unsigned char)(0xFF & xcolour_array[k].pixel);
									pixel_data += 3;
									char_ximage_data++;
								}
							}
						}
						else
						{
							pixel_data = pixels;
							for (j = 0 ; j < height ; j++)
							{
								for (i = 0 ; i < width ; i++)
								{
									k = pixel_data[0];
									XPutPixel(ximage, i, j, xcolour_array[k].pixel);
									pixel_data += 3;
								}
							}
						}
						DEALLOCATE(xcolour_array);
					}
					XPutImage(display, pixmap, gc,
						ximage, 0, 0, 0, 0, width, height);
					XFreeGC(display, gc);
					XDestroyImage(ximage);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Cmgui_image_from_Pixmap.  "
						"Could not create pixmap.");
					pixmap = (Pixmap)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Cmgui_image_from_Pixmap.  "
					"Could not dispatch Cmgui_image.");
				pixmap = (Pixmap)NULL;
			}
			DEALLOCATE(pixels);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Cmgui_image_from_Pixmap.  "
				"Could not allocate pixel data.");
			pixmap = (Pixmap)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Cmgui_image_from_Pixmap.  "
			"Missing widget data");
		pixmap = (Pixmap)NULL;
	}
	LEAVE;

	return (pixmap);
} /* create_Pixmap_from_Cmgui_image */

int convert_Colour_to_Pixel(Display *display, struct Colour *colour,
	Pixel *pixel)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Finds an X pixel value that represents the <colour>.
==============================================================================*/
{
	int return_code;
	XColor xcolour;
	XWindowAttributes window_attributes;

	ENTER(convert_Colour_to_Pixel);
	
	if (colour)
	{
		XGetWindowAttributes(display, DefaultRootWindow(display),
			&window_attributes);
		xcolour.red = 0xFFFF * colour->red;
		xcolour.green = 0xFFFF * colour->green;
		xcolour.blue = 0xFFFF * colour->blue;
		XAllocColor(display, window_attributes.colormap, &xcolour);
		*pixel = xcolour.pixel;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"convert_Colour_to_Pixel.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return(return_code);
} /* convert_Colour_to_Pixel */

int convert_Pixel_to_Colour(Display *display, Pixel pixel,
	struct Colour *colour)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Fills in the <colour> based on the pixel.
==============================================================================*/
{
	int return_code;
	XColor xcolour;
	XWindowAttributes window_attributes;

	ENTER(convert_Pixel_to_Colour);
	
	if (colour)
	{
		XGetWindowAttributes(display, DefaultRootWindow(display),
			&window_attributes);
		xcolour.pixel = pixel;
		XQueryColor(display, window_attributes.colormap,
			&xcolour);
		colour->red = ((COLOUR_PRECISION)xcolour.red) / 65535.0;
		colour->green = ((COLOUR_PRECISION)xcolour.green) / 65535.0;
		colour->blue = ((COLOUR_PRECISION)xcolour.blue) / 65535.0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"convert_Pixel_to_Colour.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return(return_code);
} /* convert_Pixel_to_Colour */
