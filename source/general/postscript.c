/*******************************************************************************
FILE : postscript.c

LAST MODIFIED : 22 September 2004

DESCRIPTION :
Functions for creating postscript output from the mapping system.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>
#include "general/postscript.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global variables
----------------
*/
#if !defined (NO_ALIGNMENT)
/*???DB.  Change to module variables ? */
enum Horizontal_alignment horizontal_alignment=CENTRE_HORIZONTAL_ALIGNMENT;
enum Vertical_alignment vertical_alignment=CENTRE_VERTICAL_ALIGNMENT;
#endif

/*
Global macros
-------------
*/
#if !defined (NO_ALIGNMNET)
#define SET_HORIZONTAL_ALIGNMENT(alignment) \
	horizontal_alignment=alignment

#define SET_VERTICAL_ALIGNMENT(alignment) \
	vertical_alignment=alignment
#endif

/*
Module types
------------
*/
struct Postscript
{
	Colormap colour_map;
	FILE *file;
	Pixel *spectrum_pixels;
	int number_of_spectrum_colours;
	Pixel background_drawing_colour,foreground_drawing_colour;
	XFontStruct *font;
	float page_height,page_width,page_window_bottom,page_window_height,
		page_window_left,page_window_width,world_left,world_top,world_width,
		world_height;
	struct Colour background_printer_colour;
};

/*
Module variables
----------------
*/
char first_draw_line,first_draw_lines,first_draw_string,first_fill_arc,
	first_fill_rectangle,first_put_image,first_set_clip_rectangles;
struct Postscript postscript=
{
	(Colormap)NULL,
	(FILE *)NULL,
	(Pixel *)NULL,
	0,
	(Pixel)NULL,(Pixel)NULL,
	(XFontStruct *)NULL,
	0,0,0,0,0,0,0,0,0,0,
	{0,0,0}
};
float world_origin_x,world_origin_y;

/*
Global functions
----------------
*/
int open_postscript(char *file_name,enum Page_orientation page_orientation,
	Colormap colour_map,Pixel *spectrum_pixels,int number_of_spectrum_colours,
	Pixel background_drawing_colour,struct Colour *background_printer_colour,
	Pixel foreground_drawing_colour,XFontStruct *font,float printer_page_width_mm,
	float printer_page_height_mm,float printer_page_left_margin_mm,
	float printer_page_right_margin_mm,float printer_page_top_margin_mm,
	float printer_page_bottom_margin_mm,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Opens a file with the specified <file_name> for writing.  Writes the PostScript
file heading and some standard routines to the file.  If the <page_orientation>
is <PORTRAIT> then the short axis is x and the long axis is y.  If the
<page_orientation> is <LANDSCAPE> then the long axis is x and the short axis is
y.  The axes are scaled so that the units are mm.
???DB.  <background_printer_colour> could be from a different <colour_map>
==============================================================================*/
{
	Atom family_name,point_size,weight;
	char *font_name=(char *)NULL,*font_weight=(char *)NULL;
	float scale_mm_to_default;
	int bounding_box_bottom,bounding_box_left,bounding_box_right,bounding_box_top,
		font_point_size,i,return_code;
	XFontProp *font_property;

	ENTER(open_postscript);
	/* check arguments */
	if (file_name&&colour_map&&font&&user_interface)
	{
		if (!(postscript.file))
		{
			if (postscript.file=fopen(file_name,"wt"))
			{
				postscript.colour_map=colour_map;
				postscript.spectrum_pixels=spectrum_pixels;
				postscript.background_drawing_colour=background_drawing_colour;
				postscript.background_printer_colour.red=background_printer_colour->red;
				postscript.background_printer_colour.green=
					background_printer_colour->green;
				postscript.background_printer_colour.blue=
					background_printer_colour->blue;
				postscript.foreground_drawing_colour=foreground_drawing_colour;
				postscript.number_of_spectrum_colours=number_of_spectrum_colours;
				postscript.font=font;
				/* the default postscript unit is a 72nd of an inch */
				scale_mm_to_default=72./25.4;
				bounding_box_left=(int)(scale_mm_to_default*
					printer_page_left_margin_mm);
				bounding_box_bottom=(int)(scale_mm_to_default*
					printer_page_bottom_margin_mm);
				bounding_box_right=(int)(scale_mm_to_default*(printer_page_width_mm-
					printer_page_right_margin_mm));
				bounding_box_top=(int)(scale_mm_to_default*(printer_page_height_mm-
					printer_page_top_margin_mm));
				/* output encapsulated postscript header */
				(void)fprintf(postscript.file,"%%!PS-Adobe-3.0 EPSF-3.0\n");
				(void)fprintf(postscript.file,"%%%%BoundingBox: %d %d %d %d\n",
					bounding_box_left,bounding_box_bottom,bounding_box_right,
					bounding_box_top);
				(void)fprintf(postscript.file,"%%%%Title: %s\n",file_name);
				(void)fprintf(postscript.file,"%%%%Creator: emap\n");
				(void)fprintf(postscript.file,"%%%%EndComments\n");
				(void)fprintf(postscript.file,"%%\n");
				(void)fprintf(postscript.file,"%% Constants\n");
				(void)fprintf(postscript.file,"/red_in_gray 0.299 def\n");
				(void)fprintf(postscript.file,"/green_in_gray 0.587 def\n");
				(void)fprintf(postscript.file,"/blue_in_gray 0.114 def\n");
				(void)fprintf(postscript.file,"%%\n");
				/* declare PostScript global variables */
				(void)fprintf(postscript.file,"%% Global variables\n");
				(void)fprintf(postscript.file,"/buffer 512 string def\n");
				(void)fprintf(postscript.file,
					"/default_transformation_matrix matrix def\n");
				(void)fprintf(postscript.file,"/transformation_matrix matrix def\n");
				(void)fprintf(postscript.file,
					"/unscaled_transformation_matrix matrix def\n");
				(void)fprintf(postscript.file,"%%\n");
				/* set the default display transformation.  This is dependent on the
					page orientation */
				if (((PORTRAIT==page_orientation)&&
					(printer_page_width_mm>printer_page_height_mm))||
					((LANDSCAPE==page_orientation)&&
					(printer_page_width_mm<printer_page_height_mm)))
				{
					/* page height is x and page width is y */
					(void)fprintf(postscript.file,
						"%% page height is x and page width is y\n");
					/* change the origin to the bottom right of the bounding box */
					(void)fprintf(postscript.file,
						"%% change the origin to the bottom right of the bounding box\n");
					(void)fprintf(postscript.file,"%d %d translate\n",bounding_box_right,
						bounding_box_bottom);
					/* rotate the axes by 90 degrees counterclockwise */
					(void)fprintf(postscript.file,
						"%% rotate the axes by 90 degrees counterclockwise\n");
					(void)fprintf(postscript.file,"90 rotate\n");
					/* save the postscript page width and height */
					postscript.page_width=bounding_box_top-bounding_box_bottom;
					postscript.page_height=bounding_box_right-bounding_box_left;
				}
				else
				{
					/* page height is y and page width is x */
					(void)fprintf(postscript.file,
						"%% page height is y and page width is x\n");
					/* change the origin to the bottom left of the bounding box */
					(void)fprintf(postscript.file,
						"%% change the origin to the bottom left of the bounding box\n");
					(void)fprintf(postscript.file,"%d %d translate\n",bounding_box_left,
						bounding_box_bottom);
					/* save the postscript page width and height */
					postscript.page_width=bounding_box_right-bounding_box_left;
					postscript.page_height=bounding_box_top-bounding_box_bottom;
				}
				/* save the default display transformation */
				(void)fprintf(postscript.file,
					"%% save the default display transformation\n");
				(void)fprintf(postscript.file,
					"default_transformation_matrix currentmatrix pop\n");
				/* set the current transformation and the unscaled current
					transformation */
				(void)fprintf(postscript.file,
"%% set the current transformation and the unscaled current transformation\n");
				(void)fprintf(postscript.file,
					"%%   (used for making line thickness independent of orientation)\n");
				(void)fprintf(postscript.file,
					"transformation_matrix currentmatrix pop\n");
				(void)fprintf(postscript.file,
					"unscaled_transformation_matrix currentmatrix pop\n");
				/* set the font */
				/* get the family name, weight and point size */
				family_name=XInternAtom(User_interface_get_display(user_interface),"FAMILY_NAME",True);
				weight=XInternAtom(User_interface_get_display(user_interface),"WEIGHT_NAME",True);
				point_size=XInternAtom(User_interface_get_display(user_interface),"POINT_SIZE",True);
				font_property=font->properties;
				for (i=font->n_properties;i>0;i--)
				{
					if (font_property->name==family_name)
					{
						font_name=XGetAtomName(User_interface_get_display(user_interface),
							(Atom)(font_property->card32));
					}
					else
					{
						if (font_property->name==weight)
						{
							font_weight=XGetAtomName(User_interface_get_display(user_interface),
								(Atom)(font_property->card32));
						}
						else
						{
							if (font_property->name==point_size)
							{
								font_point_size=(unsigned int)(font_property->card32);
							}
						}
					}
					font_property++;
				}
				(void)fprintf(postscript.file,"%% set the default font\n");
				/* the font point size is in tenths of a point */
				(void)fprintf(postscript.file,
					"/%s-%s findfont %.3g scalefont setfont\n",font_name,font_weight,
					((float)font_point_size)/10.);
				/* set all the PostScript functions to be uncalled */
				first_draw_line=1;
				first_draw_lines=1;
				first_draw_string=1;
				first_fill_arc=1;
				first_fill_rectangle=1;
				first_put_image=1;
				first_set_clip_rectangles=1;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"open_postscript.  Could not open file");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"open_postscript.  Already open");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_postscript.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_postscript */

int close_postscript(void)
/*******************************************************************************
LAST MODIFIED : 28 February 1993

DESCRIPTION :
Write the terminating postscript commands and closes the <postscript> file.
==============================================================================*/
{
	int return_code;

	ENTER(close_postscript);
	if (postscript.file)
	{
		(void)fprintf(postscript.file,"showpage\n");
		(void)fprintf(postscript.file,"%%%%EOF\n");
		(void)fclose(postscript.file);
		postscript.file=(FILE *)NULL;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"close_postscript.  Missing file");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* close_postscript */

int get_postscript_page_size(float *width,float *height)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Gets the <width> and <height> of the postscript page.  Used in conjunction with
set_postscript_display_transfor .
==============================================================================*/
{
	int return_code;

	ENTER(get_postscript_page_size);
	if (width&&height)
	{
		if (postscript.file)
		{
			*width=postscript.page_width;
			*height=postscript.page_height;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_postscript_page_size.  Postscript closed");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_postscript_page_size.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_postscript_page_size */

int new_postscript_page(void)
/*******************************************************************************
LAST MODIFIED : 29 September 1993

DESCRIPTION :
Write the postscript commands for printing the current drawing (page) and then
clearing the drawing (page).  NB  This resets the display transformation to the
default display transformation.
==============================================================================*/
{
	int return_code;

	ENTER(new_postscript_page);
	if (postscript.file)
	{
		(void)fprintf(postscript.file,"showpage\n");
		/* showpage executes initgraphics so that the default display transformation
			has to be reset */
		(void)fprintf(postscript.file,"%% set the display transformation\n");
		(void)fprintf(postscript.file,"default_transformation_matrix setmatrix\n");
		/* reset the current transformation and the unscaled current
			transformation */
		(void)fprintf(postscript.file,
"%% reset the current transformation and the unscaled current transformation\n"
			);
		(void)fprintf(postscript.file,
			"%%   (used for making line thickness independent of orientation)\n");
		(void)fprintf(postscript.file,"transformation_matrix currentmatrix pop\n");
		(void)fprintf(postscript.file,
			"unscaled_transformation_matrix currentmatrix pop\n");
	}
	else
	{
		display_message(ERROR_MESSAGE,"new_postscript_page.  Missing file");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* new_postscript_page */

int set_postscript_display_transfor(float page_window_left,
	float page_window_bottom,float page_window_width,float page_window_height,
	float world_left,float world_top,float world_width,float world_height)
/*******************************************************************************
LAST MODIFIED : 8 February 1997

DESCRIPTION :
Sets the display transformation so that the contents of the world rectangle are
displayed in the page rectangle.
==============================================================================*/
{
	float scale_x,scale_y;
	int return_code;

	ENTER(set_postscript_display_transfor);
	if (postscript.file)
	{
		if ((page_window_width>0)&&(page_window_height>0)&&(world_width>0)&&
			(world_height>0))
		{
			scale_x=page_window_width/world_width;
			scale_y=page_window_height/world_height;
			(void)fprintf(postscript.file,"%% set the display transformation\n");
			(void)fprintf(postscript.file,
				"default_transformation_matrix setmatrix\n");
			(void)fprintf(postscript.file,"%.5g %.5g translate\n",page_window_left,
				page_window_bottom);
			(void)fprintf(postscript.file,
				"unscaled_transformation_matrix currentmatrix pop\n");
			(void)fprintf(postscript.file,"%.5g %.5g scale\n",scale_x,scale_y);
			(void)fprintf(postscript.file,
				"transformation_matrix currentmatrix pop\n");
			/*???Get from X ? */
			/* the font point size is in tenths of a point */
/*      (void)fprintf(postscript.file,
				"/%s-%s findfont [%.5g 0 0 %.5g 0 0] makefont setfont\n",font_name,
				font_weight,(float)font_point_size/(10*scale_x),
				(float)font_point_size/(10*scale_y));*/
			postscript.page_window_left=page_window_left;
			postscript.page_window_bottom=page_window_bottom;
			postscript.page_window_width=page_window_width;
			postscript.page_window_height=page_window_height;
			postscript.world_left=world_left;
			postscript.world_top=world_top;
			postscript.world_width=world_width;
			postscript.world_height=world_height;
			world_origin_x=world_left;
			world_origin_y=world_top+world_height;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_postscript_display_transfor.  Invalid rectangles");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_postscript_display_transfor.  Missing file");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_postscript_display_transfor */

int get_postscript_display_transfor(float *page_window_left,
	float *page_window_bottom,float *page_window_width,float *page_window_height,
	float *world_left,float *world_top,float *world_width,float *world_height)
/*******************************************************************************
LAST MODIFIED : 22 June 1997

DESCRIPTION :
Gets the display transformation.
==============================================================================*/
{
	int return_code;

	ENTER(set_postscript_display_transfor);
	if (page_window_left&&page_window_bottom&&page_window_width&&
		page_window_height&&world_left&&world_top&&world_width&&world_height)
	{
		if (postscript.file)
		{
			*page_window_left=postscript.page_window_left;
			*page_window_bottom=postscript.page_window_bottom;
			*page_window_width=postscript.page_window_width;
			*page_window_height=postscript.page_window_height;
			*world_left=postscript.world_left;
			*world_top=postscript.world_top;
			*world_width=postscript.world_width;
			*world_height=postscript.world_height;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_postscript_display_transfor.  Missing file");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_postscript_display_transfor.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_postscript_display_transfor */

float get_pixel_aspect_ratio(Display *display)
/*******************************************************************************
LAST MODIFIED : 8 February 1997

DESCRIPTION :
Returns the aspect ratio (height/width) of a pixel in display space.
==============================================================================*/
{
	float pixel_aspect_ratio;
	int screen_number;

	ENTER(get_pixel_aspect_ratio);
	screen_number=DefaultScreen(display);
	pixel_aspect_ratio=((float)DisplayHeightMM(display,screen_number)/
		(float)DisplayHeight(display,screen_number))/
		((float)DisplayWidthMM(display,screen_number)/
		(float)DisplayWidth(display,screen_number));
	LEAVE;

	return (pixel_aspect_ratio);
} /* get_pixel_aspect_ratio */

void XPSDrawLine(Display *display,Drawable drawable,GC gc,int x1,int y1,int x2,
	int y2)
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
If the <postscript> file is open then PostScript for drawing the line is
written to the file, otherwise XDrawLine is called.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
???At present only retrieves the foreground colour from the graphics context.
==============================================================================*/
{
	char **statement;
	static char *draw_line_postscript[]=
	{
		"%",
		"% Routine for drawing a line",
		"%",
		"/DrawLine",
		"{",
		"  % Draw a line between two points",
		"  %",
		"  % Parameters:",
		"  %   red, green, blue components for the line colour (each value between",
		"  %     0 and 1 inclusive)",
		"  %   x1 & y1  coordinates of first point",
		"  %   x2 & y2  coordinates of second point",
		"  %",
		"  currentfile buffer readline pop",
		"  token pop /red exch def",
		"  token pop /green exch def",
		"  token pop /blue exch def pop",
		"  systemdict /setrgbcolor known",
		"  {",
		"    red green blue setrgbcolor",
		"  }",
		"  {",
		"    red red_in_gray mul",
		"    green green_in_gray mul add",
		"    blue blue_in_gray mul add",
		"    setgray",
		"  } ifelse",
		"  currentfile buffer readline pop",
		"  token pop /x exch def",
		"  token pop /y exch def pop",
		"  x y moveto",
		"  currentfile buffer readline pop",
		"  token pop /x exch def",
		"  token pop /y exch def pop",
		"  x y lineto",
		"% to make line thickness independent of orientation",
		"  unscaled_transformation_matrix setmatrix",
		"  stroke",
		"  transformation_matrix setmatrix",
		"} bind def",
		NULL
	};
	XColor colour;
	XGCValues values;

	ENTER(XPSDrawLine);
	if (postscript.file)
	{
		if (first_draw_line)
		{
			/* output postscript for the DrawLine routine */
			for (statement=draw_line_postscript;*statement;statement++)
			{
				(void)fprintf(postscript.file,"%s\n",*statement);
			}
			first_draw_line=0;
		}
		/* call the PostScript routine for drawing the line */
		(void)fprintf(postscript.file,"DrawLine\n");
		/* specify the line colour */
		XGetGCValues(display,gc,GCForeground,&values);
		/* set the background drawing colour to the background postscript colour */
		if (values.foreground==postscript.background_drawing_colour)
		{
			colour.red=
				(unsigned short)(postscript.background_printer_colour.red*65535.);
			colour.green=
				(unsigned short)(postscript.background_printer_colour.green*65535.);
			colour.blue=
				(unsigned short)(postscript.background_printer_colour.blue*65535.);
		}
		else
		{
			colour.pixel=values.foreground;
			colour.pad=0;
			XQueryColor(display,postscript.colour_map,&colour);
		}
		(void)fprintf(postscript.file,"%.5g %.5g %.5g\n",
			(float)(colour.red>>8)/(float)0xff,(float)(colour.green>>8)/(float)0xff,
			(float)(colour.blue>>8)/(float)0xff);
		/* specify the start point */
		(void)fprintf(postscript.file,"%.5g %.5g\n",(float)x1-world_origin_x,
			world_origin_y-(float)y1);
		/* specify the end point */
		(void)fprintf(postscript.file,"%.5g %.5g\n",(float)x2-world_origin_x,
			world_origin_y-(float)y2);
	}
	else
	{
		XDrawLine(display,drawable,gc,x1,y1,x2,y2);
	}
	LEAVE;
} /* XPSDrawLine */

void XPSDrawLineFloat(Display *display,Drawable drawable,GC gc,float x1,
	float y1,float x2,float y2)
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
If the <postscript> file is open then PostScript for drawing the line is
written to the file, otherwise XDrawLine is called.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
???At present only retrieves the foreground colour from the graphics context.
???DB.  Quick fix so that I can get smoother constant width contours for emap.
???It is a duplicate of XPSDrawLine except that it takes floats for the
???coordinates.
???X.  Round coordinates to integers.
???Postscript.  Use floats.
==============================================================================*/
{
	char **statement;
	static char *draw_line_postscript[]=
	{
		"%",
		"% Routine for drawing a line",
		"%",
		"/DrawLine",
		"{",
		"  % Draw a line between two points",
		"  %",
		"  % Parameters:",
		"  %   red, green, blue components for the line colour (each value between",
		"  %     0 and 1 inclusive)",
		"  %   x1 & y1  coordinates of first point",
		"  %   x2 & y2  coordinates of second point",
		"  %",
		"  currentfile buffer readline pop",
		"  token pop /red exch def",
		"  token pop /green exch def",
		"  token pop /blue exch def pop",
		"  systemdict /setrgbcolor known",
		"  {",
		"    red green blue setrgbcolor",
		"  }",
		"  {",
		"    red red_in_gray mul",
		"    green green_in_gray mul add",
		"    blue blue_in_gray mul add",
		"    setgray",
		"  } ifelse",
		"  currentfile buffer readline pop",
		"  token pop /x exch def",
		"  token pop /y exch def pop",
		"  x y moveto",
		"  currentfile buffer readline pop",
		"  token pop /x exch def",
		"  token pop /y exch def pop",
		"  x y lineto",
		"% to make line thickness independent of orientation",
		"  unscaled_transformation_matrix setmatrix",
		"  stroke",
		"  transformation_matrix setmatrix",
		"} bind def",
		NULL
	};
	XColor colour;
	XGCValues values;

	ENTER(XPSDrawLineFloat);
	if (postscript.file)
	{
		if (first_draw_line)
		{
			/* output postscript for the DrawLine routine */
			for (statement=draw_line_postscript;*statement;statement++)
			{
				(void)fprintf(postscript.file,"%s\n",*statement);
			}
			first_draw_line=0;
		}
		/* call the PostScript routine for drawing the line */
		(void)fprintf(postscript.file,"DrawLine\n");
		/* specify the line colour */
		XGetGCValues(display,gc,GCForeground,&values);
		/* set the background drawing colour to the background postscript colour */
		if (values.foreground==postscript.background_drawing_colour)
		{
			colour.red=
				(unsigned short)(postscript.background_printer_colour.red*65535.);
			colour.green=
				(unsigned short)(postscript.background_printer_colour.green*65535.);
			colour.blue=
				(unsigned short)(postscript.background_printer_colour.blue*65535.);
		}
		else
		{
			colour.pixel=values.foreground;
			colour.pad=0;
			XQueryColor(display,postscript.colour_map,&colour);
		}
		(void)fprintf(postscript.file,"%.5g %.5g %.5g\n",
			(float)(colour.red>>8)/(float)0xff,(float)(colour.green>>8)/(float)0xff,
			(float)(colour.blue>>8)/(float)0xff);
		/* specify the start point */
		(void)fprintf(postscript.file,"%.6g %.6g\n",x1-world_origin_x,
			world_origin_y-y1);
		/* specify the end point */
		(void)fprintf(postscript.file,"%.6g %.6g\n",x2-world_origin_x,
			world_origin_y-y2);
	}
	else
	{
		XDrawLine(display,drawable,gc,(int)(x1+0.5),(int)(y1+0.5),(int)(x2+0.5),
			(int)(y2+0.5));
	}
	LEAVE;
} /* XPSDrawLineFloat */

void XPSDrawLines(Display *display,Drawable drawable,GC gc,XPoint *points,
	int num_points,int mode)
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
If the <postscript> file is open then PostScript for drawing the polyline
specified by <points> is written to the file, otherwise XDrawLines is called.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
???At present only retrieves the foreground colour from the graphics context.
==============================================================================*/
{
	char **statement;
	int i;
	static char *draw_lines_postscript[]=
	{
		"%",
		"% Routine for drawing a polyline",
		"%",
		"/DrawLines",
		"{",
		"  % Draw a polyline",
		"  %",
		"  % Parameters:",
		"  %   red, green, blue components for the line colour (each value between",
		"  %     0 and 1 inclusive)",
		"  %   mode  either all points are given in absolute coordinates (0) or",
		"  %     the coordinates of points after the first are given relative to",
		"  %     the previous point",
		"  %   number of points",
		"  %   a list of (x,y) pairs, each pair on a separate line",
		"  %",
		"  currentfile buffer readline pop",
		"  token pop /red exch def",
		"  token pop /green exch def",
		"  token pop /blue exch def pop",
		"  systemdict /setrgbcolor known",
		"  {",
		"    red green blue setrgbcolor",
		"  }",
		"  {",
		"    red red_in_gray mul",
		"    green green_in_gray mul add",
		"    blue blue_in_gray mul add",
		"    setgray",
		"  } ifelse",
		"  currentfile buffer readline pop",
		"  token pop /mode exch def pop",
		"  currentfile buffer readline pop",
		"  token pop /num_points exch def pop",
		"  currentfile buffer readline pop",
		"  token pop /x exch def",
		"  token pop /y exch def pop",
		"  mode 0 eq",
		"  {",
		"    num_points 1 sub 1000 idiv -1 1",
		"    {",
		"      pop",
		"      x y moveto",
		"      1 1 1000",
		"      {",
		"        pop",
		"        currentfile buffer readline pop",
		"        token pop /x exch def",
		"        token pop /y exch def pop",
		"        x y lineto",
		"      } for",
		"%     to make line thickness independent of orientation",
		"      unscaled_transformation_matrix setmatrix",
		"      stroke",
		"      transformation_matrix setmatrix",
		"    } for",
		"    num_points 1 sub 1000 mod 0 gt",
		"    {",
		"      x y moveto",
		"      num_points 1 sub 1000 mod -1 1",
		"      {",
		"        pop",
		"        currentfile buffer readline pop",
		"        token pop /x exch def",
		"        token pop /y exch def pop",
		"        x y lineto",
		"      } for",
		"%     to make line thickness independent of orientation",
		"      unscaled_transformation_matrix setmatrix",
		"      stroke",
		"      transformation_matrix setmatrix",
		"    } if",
		"  }",
		"  {",
		"    num_points 1 sub 1000 idiv -1 1",
		"    {",
		"      pop",
		"      x y moveto",
		"      1 1 1000",
		"      {",
		"        pop",
		"        currentfile buffer readline pop",
		"        token pop /x exch def",
		"        token pop /y exch def pop",
		"        x y rlineto",
		"      } for",
		"      currentpoint /y exch def /x exch def",
		"%     to make line thickness independent of orientation",
		"      unscaled_transformation_matrix setmatrix",
		"      stroke",
		"      transformation_matrix setmatrix",
		"    } for",
		"    num_points 1 sub 1000 mod 0 gt",
		"    {",
		"      x y moveto",
		"      num_points 1 sub 1000 mod -1 1",
		"      {",
		"        pop",
		"        currentfile buffer readline pop",
		"        token pop /x exch def",
		"        token pop /y exch def pop",
		"        x y rlineto",
		"      } for",
		"%     to make line thickness independent of orientation",
		"      unscaled_transformation_matrix setmatrix",
		"      stroke",
		"      transformation_matrix setmatrix",
		"    } if",
		"  } ifelse",
		"} bind def",
		NULL
	};
	XColor colour;
	XGCValues values;
	XPoint *point;

	ENTER(XPSDrawLines);
	if (postscript.file)
	{
		if (first_draw_lines)
		{
			/* output postscript for the DrawLines routine */
			for (statement=draw_lines_postscript;*statement;statement++)
			{
				(void)fprintf(postscript.file,"%s\n",*statement);
			}
			first_draw_lines=0;
		}
		/* call the PostScript routine for drawing the line */
		(void)fprintf(postscript.file,"DrawLines\n");
		/* specify the line colour */
		XGetGCValues(display,gc,GCForeground,&values);
		/* set the background drawing colour to the background postscript colour */
		if (values.foreground==postscript.background_drawing_colour)
		{
			colour.red=
				(unsigned short)(postscript.background_printer_colour.red*65535.);
			colour.green=
				(unsigned short)(postscript.background_printer_colour.green*65535.);
			colour.blue=
				(unsigned short)(postscript.background_printer_colour.blue*65535.);
		}
		else
		{
			colour.pixel=values.foreground;
			colour.pad=0;
			XQueryColor(display,postscript.colour_map,&colour);
		}
		(void)fprintf(postscript.file,"%.5g %.5g %.5g\n",
			(float)(colour.red>>8)/(float)0xff,(float)(colour.green>>8)/(float)0xff,
			(float)(colour.blue>>8)/(float)0xff);
		/* specify the mode */
		switch (mode)
		{
			case CoordModeOrigin: default:
			{
				(void)fprintf(postscript.file,"0\n");
			} break;
			case CoordModePrevious:
			{
				(void)fprintf(postscript.file,"1\n");
			} break;
		}
		/* specify the number of points */
		(void)fprintf(postscript.file,"%d\n",num_points);
		/* specify the coordinates of the points */
		point=points;
		switch (mode)
		{
			case CoordModeOrigin: default:
			{
				for (i=num_points;i>0;i--)
				{
					(void)fprintf(postscript.file,"%.5g %.5g\n",
						(float)(point->x)-world_origin_x,world_origin_y-(float)(point->y));
					point++;
				}
			} break;
			case CoordModePrevious:
			{
				(void)fprintf(postscript.file,"%.5g %.5g\n",
					(float)(point->x)-world_origin_x,world_origin_y-(float)(point->y));
				for (i=num_points-1;i>0;i--)
				{
					point++;
					(void)fprintf(postscript.file,"%d %d\n",point->x,-(point->y));
				}
			} break;
		}
	}
	else
	{
		XDrawLines(display,drawable,gc,points,num_points,mode);
	}
	LEAVE;
} /* XPSDrawLines */

void XPSDrawString(Display *display,Drawable drawable,GC gc,int x,int y,
	char *string,int length)
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
#if defined (NO_ALIGNMENT)
If the <postscript> file is open then PostScript for drawing the string is
written to the file, otherwise XDrawString is called.
#else
Draws the <string> relative to the given point using the current horizontal and
vertical alignments.  If the <postscript> file is open then PostScript for
drawing the string is written to the file, otherwise XTextExtents and
XDrawString are used.
#endif
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
==============================================================================*/
{
	char **statement;
#if !defined (NO_ALIGNMENT)
	int x_string,y_string;
#endif
	static char *draw_string_postscript[]=
	{
		"%",
		"% Routine for drawing a string",
		"%",
		"/DrawString",
		"{",
		"  % Draw a string at a specified point",
		"  %",
		"  % Parameters:",
		"  %   red, green, blue components for the string colour (each value",
		"  %     between 0 and 1 inclusive)",
#if !defined (NO_ALIGNMENT)
		"  %   horizontal and vertical text alignments",
#endif
		"  %   x & y  coordinates of position",
		"  %   string length",
		"  %   string",
		"  %",
		"  currentfile buffer readline pop",
		"  token pop /red exch def",
		"  token pop /green exch def",
		"  token pop /blue exch def pop",
		"  systemdict /setrgbcolor known",
		"  {",
		"    red green blue setrgbcolor",
		"  }",
		"  {",
		"    red red_in_gray mul",
		"    green green_in_gray mul add",
		"    blue blue_in_gray mul add",
		"    setgray",
		"  } ifelse",
#if !defined (NO_ALIGNMENT)
		"  currentfile buffer readline pop",
		"  token pop /x_align exch def",
		"  token pop /y_align exch def pop",
#endif
		"  currentfile buffer readline pop",
		"  token pop /x exch def",
		"  token pop /y exch def pop",
		"  currentfile buffer readline pop",
		"  token pop /out_length exch def pop",
		"  /out_string out_length string def",
		"  currentfile out_string readstring pop pop",
		"  currentfile buffer readline pop pop",
#if !defined (NO_ALIGNMENT)
		"  % allow for the horizontal and vertical alignment",
		"  newpath",
#endif
		"  x y moveto",
#if !defined (NO_ALIGNMENT)
		"  out_string false charpath",
		"  pathbbox",
		"  pop /upper_right_x exch def",
		"  pop /upper_right_y exch def",
		"  pop /lower_left_x exch def",
		"  pop /lower_left_y exch def",
		"  x_align 0 eq",
		"  {",
		"    % left",
		"    x x add lower_left_x sub",
		"  }",
		"  {",
		"    x_align 1 eq",
		"    {",
		"      % centre",
		"      x x add upper_right_x lower_left_x sub sub",
		"    }",
		"    {",
		"      % right",
		"      x x add upper_right_x sub",
		"    }",
		"    elseif",
		"  }",
		"  elseif",
		"  y_align 0 eq",
		"  {",
		"    % top",
		"    y y add upper_right_y sub",
		"  }",
		"  {",
		"    y_align 1 eq",
		"    {",
		"      % centre",
		"      y y add lower_left_y upper_right_y sub sub",
		"    }",
		"    {",
		"      % bottom",
		"      y y add lower_left_y sub",
		"    }",
		"    elseif",
		"  }",
		"  elseif",
		"  moveto",
#endif
		"  unscaled_transformation_matrix setmatrix",
		"  out_string show",
		"  transformation_matrix setmatrix",
		"} bind def",
		NULL
	};
#if !defined (NO_ALIGNMENT)
	XCharStruct bounds;
#endif
	XColor colour;
	XGCValues values;

	ENTER(XPSDrawString);
	if (postscript.file)
	{
		if (first_draw_string)
		{
			/* output postscript for some put_image routines */
			for (statement=draw_string_postscript;*statement;statement++)
			{
				(void)fprintf(postscript.file,"%s\n",*statement);
			}
			first_draw_string=0;
		}
		/* call the PostScript routine for drawing the string */
		(void)fprintf(postscript.file,"DrawString\n");
		/* specify the line colour */
		XGetGCValues(display,gc,GCForeground,&values);
		/* set the background drawing colour to the background postscript colour */
		if (values.foreground==postscript.background_drawing_colour)
		{
			colour.red=
				(unsigned short)(postscript.background_printer_colour.red*65535.);
			colour.green=
				(unsigned short)(postscript.background_printer_colour.green*65535.);
			colour.blue=
				(unsigned short)(postscript.background_printer_colour.blue*65535.);
		}
		else
		{
			colour.pixel=values.foreground;
			colour.pad=0;
			XQueryColor(display,postscript.colour_map,&colour);
		}
		(void)fprintf(postscript.file,"%.5g %.5g %.5g\n",
			(float)(colour.red>>8)/(float)0xff,(float)(colour.green>>8)/(float)0xff,
			(float)(colour.blue>>8)/(float)0xff);
#if !defined (NO_ALIGNMENT)
		/* specify the alignment */
		(void)fprintf(postscript.file,"%d %d\n",horizontal_alignment,
			vertical_alignment);
#endif
		/* specify the start point */
		(void)fprintf(postscript.file,"%.5g %.5g\n",
			(float)(x)-world_origin_x,world_origin_y-(float)y);
		/* specify the string length */
		(void)fprintf(postscript.file,"%d\n",length);
		/* specify the string */
		(void)fprintf(postscript.file,"%s\n",string);
	}
	else
	{
#if defined (NO_ALIGNMENT)
		XDrawString(display,drawable,gc,x,y,string,length);
#else
		/* allow for the vertical and horizontal alignment */
		XTextExtents(postscript.font,string,length,&direction,&ascent,&descent,
			&bounds);
		x_string=x;
		switch (horizontal_alignment)
		{
			case LEFT_ALIGNMENT:
			{
				x_string += bounds.lbearing;
			} break;
			case CENTRE_HORIZONTAL_ALIGNMENT:
			{
				x_string += (bounds.lbearing-bounds.rbearing)/2;
			} break;
			case RIGHT_ALIGNMENT:
			{
				x_string -= bounds.rbearing;
			} break;
		}
		y_string=y;
		switch (vertical_alignment)
		{
			case TOP_ALIGNMENT:
			{
				y_string += bounds.ascent;
			} break;
			case CENTRE_VERTICAL_ALIGNMENT:
			{
				y_string += (bounds.ascent-bounds.descent)/2;
			} break;
			case BOTTOM_ALIGNMENT:
			{
				y_string -= bounds.descent;
			} break;
		}
		XDrawString(display,drawable,gc,x_string,y_string,string,length);
#endif
	}
	LEAVE;
} /* XPSDrawString */

void XPSFillRectangle(Display *display,Drawable drawable,GC gc,int x,int y,
	unsigned int width,unsigned int height)
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
If the <postscript> file is open then PostScript for filling the rectangle is
written to the file, otherwise XFillRectangle is called.
???In the future the idea is for it to switch based on either the <display> or
the drawable (not sure which)
???Sometimes XFillRectangle is used for clearing the background ?
==============================================================================*/
{
	char **statement;
	static char *fill_rectangle_postscript[]=
	{
		"%",
		"% Routine for filling a rectangle",
		"%",
		"/FillRectangle",
		"{",
		"  % Fill a rectangle",
		"  %",
		"  % Parameters:",
		"  %   red, green, blue components for the fill colour (each value between",
		"  %     0 and 1 inclusive)",
		"  %   left, top, width, height for rectangle",
		"  %",
		"  currentfile buffer readline pop",
		"  token pop /red exch def",
		"  token pop /green exch def",
		"  token pop /blue exch def pop",
		"  systemdict /setrgbcolor known",
		"  {",
		"    red green blue setrgbcolor",
		"  }",
		"  {",
		"    red red_in_gray mul",
		"    green green_in_gray mul add",
		"    blue blue_in_gray mul add",
		"    setgray",
		"  } ifelse",
		"  currentfile buffer readline pop",
		"  token pop /x exch def",
		"  token pop /y exch def",
		"  token pop /width exch def",
		"  token pop /height exch def pop",
		"  newpath",
		"  x y moveto",
		"  width 0 rlineto",
		"  0 height neg rlineto",
		"  width neg 0 rlineto",
		"  fill",
		"} bind def",
		NULL
	};
	XColor colour;
	XGCValues values;

	ENTER(XPSFillRectangle);
	if (postscript.file)
	{
		if (first_fill_rectangle)
		{
			/* output postscript for the FillRectangle routine */
			for (statement=fill_rectangle_postscript;*statement;statement++)
			{
				(void)fprintf(postscript.file,"%s\n",*statement);
			}
			first_fill_rectangle=0;
		}
		/* call the PostScript routine for drawing the line */
		(void)fprintf(postscript.file,"FillRectangle\n");
		/* specify the line colour */
		XGetGCValues(display,gc,GCForeground,&values);
		/* set the background drawing colour to the background postscript colour */
		if (values.foreground==postscript.background_drawing_colour)
		{
			colour.red=
				(unsigned short)(postscript.background_printer_colour.red*65535.);
			colour.green=
				(unsigned short)(postscript.background_printer_colour.green*65535.);
			colour.blue=
				(unsigned short)(postscript.background_printer_colour.blue*65535.);
		}
		else
		{
			colour.pixel=values.foreground;
			colour.pad=0;
			XQueryColor(display,postscript.colour_map,&colour);
		}
		(void)fprintf(postscript.file,"%.5g %.5g %.5g\n",
			(float)(colour.red>>8)/(float)0xff,(float)(colour.green>>8)/(float)0xff,
			(float)(colour.blue>>8)/(float)0xff);
		/* specify the rectangle */
		(void)fprintf(postscript.file,"%.5g %.5g %u %u\n",(float)x-world_origin_x,
			world_origin_y-(float)y,width,height);
	}
	else
	{
		XFillRectangle(display,drawable,gc,x,y,width,height);
	}
	LEAVE;
} /* XPSFillRectangle */

void XPSFillArc(Display *display,Drawable drawable,GC gc,int x,int y,
	unsigned int width,unsigned int height,int angle1,int angle2)
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
If the <postscript> file is open then PostScript for filling the arc is written
to the file, otherwise XFillArc is called.  Fills a single elliptical arc using
the bounding rectangle and the specified angle.  <angle1> and <angle2> are
specified in units of degrees*64.  <angle1> indicates the starting position of
the arc, and <angle2> indicates the ending position.  If <angle1> is positive,
the direction is counterclockwise from the three o'clock position.  Negative
values indicate clockwise motion.  Positive values for <angle2> indicate
counterclockwise motion relative to the starting position, and negative values
indicate clockwise motion.  If the value of <angle2> is greater than 360*64, it
is truncated to 360*64.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
==============================================================================*/
{
	char **statement;
	int temp_int;
	static char *fill_arc_postscript[]=
	{
		"%",
		"% Routine for filling an arc of an ellipse",
		"%",
		"/FillArc",
		"{",
		"  % Fill an arc of an ellipse",
		"  %",
		"  % Parameters:",
		"  %   red, green, blue components for the fill colour (each value between",
		"  %     0 and 1 inclusive)",
		"  %   x, y, width, height, angle1, angle2 for arc",
		"  %",
		"  currentfile buffer readline pop",
		"  token pop /red exch def",
		"  token pop /green exch def",
		"  token pop /blue exch def pop",
		"  systemdict /setrgbcolor known",
		"  {",
		"    red green blue setrgbcolor",
		"  }",
		"  {",
		"    red red_in_gray mul",
		"    green green_in_gray mul add",
		"    blue blue_in_gray mul add",
		"    setgray",
		"  } ifelse",
		"  currentfile buffer readline pop",
		"  token pop /x exch def",
		"  token pop /y exch def",
		"  token pop /width exch def",
		"  token pop /height exch def",
		"  token pop 64 div /angle1 exch def",
		"  token pop 64 div /angle2 exch def pop",
		"  % Calculate ellipse centre",
		"  x width 2 div add /xc exch def",
		"  y height 2 div sub /yc exch def",
		"  % Represent elliptical section with two Bezier curves",
		"  xc angle1 cos width mul 2 div add /x1 exch def",
		"  yc angle1 sin height mul 2 div add /y1 exch def",
		"  x1 angle1 sin width mul 2 div sub /x12 exch def",
		"  y1 angle1 cos height mul 2 div add /y12 exch def",
		"  xc angle1 angle2 2 div add cos width mul 2 div add /x2 exch def",
		"  yc angle1 angle2 2 div add sin height mul 2 div add /y2 exch def",
		"  x2 angle1 angle2 2 div add sin width mul 2 div add /x21 exch def",
		"  y2 angle1 angle2 2 div add cos height mul 2 div sub /y21 exch def",
		"  x2 angle1 angle2 2 div add sin width mul 2 div sub /x23 exch def",
		"  y2 angle1 angle2 2 div add cos height mul 2 div add /y23 exch def",
		"  xc angle1 angle2 add cos width mul 2 div add /x3 exch def",
		"  yc angle1 angle2 add sin height mul 2 div add /y3 exch def",
		"  x3 angle1 angle2 add sin width mul 2 div add /x32 exch def",
		"  y3 angle1 angle2 add cos height mul 2 div sub /y32 exch def",
		"  newpath",
		"  xc yc moveto",
		"  x1 y1 lineto",
		"  x12 y12 x21 y21 x2 y2 curveto",
		"  x23 y23 x32 y32 x3 y3 curveto",
		"  xc yc lineto",
		"  fill",
		"} bind def",
		NULL
	};
	XColor colour;
	XGCValues values;

	ENTER(XPSFillArc);
	if (postscript.file)
	{
		if (first_fill_arc)
		{
			/* output postscript for the FillArc routine */
			for (statement=fill_arc_postscript;*statement;statement++)
			{
				(void)fprintf(postscript.file,"%s\n",*statement);
			}
			first_fill_arc=0;
		}
		/* call the PostScript routine for drawing the line */
		(void)fprintf(postscript.file,"FillArc\n");
		/* specify the line colour */
		XGetGCValues(display,gc,GCForeground,&values);
		/* set the background drawing colour to the background postscript colour */
		if (values.foreground==postscript.background_drawing_colour)
		{
			colour.red=
				(unsigned short)(postscript.background_printer_colour.red*65535.);
			colour.green=
				(unsigned short)(postscript.background_printer_colour.green*65535.);
			colour.blue=
				(unsigned short)(postscript.background_printer_colour.blue*65535.);
		}
		else
		{
			colour.pixel=values.foreground;
			colour.pad=0;
			XQueryColor(display,postscript.colour_map,&colour);
		}
		(void)fprintf(postscript.file,"%.5g %.5g %.5g\n",
			(float)(colour.red>>8)/(float)0xff,(float)(colour.green>>8)/(float)0xff,
			(float)(colour.blue>>8)/(float)0xff);
		/* specify the arc */
		if (angle2>360*64)
		{
			temp_int=360*64;
		}
		else
		{
			if (angle2< -360*64)
			{
				temp_int= -360*64;
			}
			else
			{
				temp_int=angle2;
			}
		}
		(void)fprintf(postscript.file,"%.5g %.5g %u %u %d %d\n",
			(float)x-world_origin_x,world_origin_y-(float)y,width,height,angle1,
			temp_int);
	}
	else
	{
		XFillArc(display,drawable,gc,x,y,width,height,angle1,angle2);
	}
	LEAVE;
} /* XPSFillArc */

void XPSPutImage(Display *display,Drawable drawable,GC gc,XImage *image,
	int src_x,int src_y,int dest_x,int dest_y,unsigned int width,
	unsigned int height)
/*******************************************************************************
LAST MODIFIED : 22 September 2004

DESCRIPTION :
If the <postscript> file is open then PostScript for displaying the <image> is
written to the file, otherwise XPutImage is called.
???In the future the idea is for it to switch based on either the <display> or
the drawable (not sure which)
==============================================================================*/
{
	char black_and_white;
	unsigned int column,count,i,return_code,row,scan_line_length;
	Pixel background_pixel,*spectrum_pixel;
	unsigned char bit_mask,byte;
	unsigned short background_blue,background_green,background_red,
		foreground_blue,foreground_green,foreground_red;
	Visual *screen_visual;
	XColor colour;

	ENTER(XPSPutImage);
	if (postscript.file)
	{
		screen_visual=XDefaultVisual(display,XDefaultScreen(display));
		/*???Should get visual from graphics context ? */
		/*???Use src, dest and size ? */
		if (image&&screen_visual&&(postscript.spectrum_pixels)&&
			(0<postscript.number_of_spectrum_colours)&&
			((unsigned int)postscript.number_of_spectrum_colours<=
			screen_visual->map_entries))
		{
			return_code=1;
			background_pixel=postscript.background_drawing_colour;
			colour.pad=0;
			/* determine if colour or black and white */
			colour.pixel=postscript.foreground_drawing_colour;
			XQueryColor(display,postscript.colour_map,&colour);
			foreground_red=colour.red;
			foreground_green=colour.green;
			foreground_blue=colour.blue;
			colour.pixel=background_pixel;
			XQueryColor(display,postscript.colour_map,&colour);
			background_red=colour.red;
			background_green=colour.green;
			background_blue=colour.blue;
			black_and_white=1;
			spectrum_pixel=postscript.spectrum_pixels;
			i=postscript.number_of_spectrum_colours;
			while (return_code&&black_and_white&&(i>0))
			{
				colour.pixel= *spectrum_pixel;
				XQueryColor(display,postscript.colour_map,&colour);
				if (((foreground_red==colour.red)&&
					(foreground_green==colour.green)&&
					(foreground_blue==colour.blue))||
					((background_red==colour.red)&&
					(background_green==colour.green)&&
					(background_blue==colour.blue)))
				{
					i--;
					spectrum_pixel++;
				}
				else
				{
					black_and_white=0;
				}
			}
			if (black_and_white)
			{
				/* write the postscript for displaying the image */
				scan_line_length=width/8;
				if (0!=width%8)
				{
					scan_line_length++;
				}
				(void)fprintf(postscript.file,"%% PutImage\n");
				(void)fprintf(postscript.file,"gsave\n");
				(void)fprintf(postscript.file,"/scan_line %d string def\n",
					(int)scan_line_length);
				(void)fprintf(postscript.file,"0 setgray\n");
				(void)fprintf(postscript.file,"%d %d translate\n",dest_x,dest_y);
				(void)fprintf(postscript.file,"%u %u\n",width,height);
				(void)fprintf(postscript.file,"true\n");
				(void)fprintf(postscript.file,"[1 0 0 -1 0 %u]\n",height);
				(void)fprintf(postscript.file,"{\n");
				(void)fprintf(postscript.file,
					"  currentfile scan_line readhexstring pop\n");
				(void)fprintf(postscript.file,"}\n");
				(void)fprintf(postscript.file,"imagemask\n");
				/* output image data */
				count=0;
				byte=0x00;
				bit_mask=0x80;
				for (row=0;row<height;row++)
				{
					for (column=0;column<width;column++)
					{
						colour.pixel=XGetPixel(image,column+src_x,row+src_y);
						XQueryColor(display,postscript.colour_map,&colour);
						if ((foreground_red==colour.red)&&
							(foreground_green==colour.green)&&
							(foreground_blue==colour.blue))
						{
							byte=byte|bit_mask;
						}
						bit_mask=bit_mask>>1;
						if (0x00==bit_mask)
						{
							(void)fprintf(postscript.file,"%02x",byte);
							byte=0x00;
							bit_mask=0x80;
							count++;
							if (count>=40)
							{
								count=0;
								(void)fprintf(postscript.file,"\n");
							}
						}
					}
					/* rows must start on a character boundary */
					if (0x80!=bit_mask)
					{
						(void)fprintf(postscript.file,"%02x",byte);
						byte=0x00;
						bit_mask=0x80;
						count++;
						if (count>=40)
						{
							count=0;
							(void)fprintf(postscript.file,"\n");
						}
					}
				}
				if (count!=0)
				{
					(void)fprintf(postscript.file,"\n");
				}
				(void)fprintf(postscript.file,"grestore\n");
				(void)fprintf(postscript.file,"\n");
			}
			else
			{
				/* set the background drawing colour to the background postscript
					colour */
				background_red=
					(unsigned short)(postscript.background_printer_colour.red*255.);
				background_green=
					(unsigned short)(postscript.background_printer_colour.green*255.);
				background_blue=
					(unsigned short)(postscript.background_printer_colour.blue*255.);
				/* write the postscript for displaying the image */
				scan_line_length=3*width;
				(void)fprintf(postscript.file,"%% PutImage\n");
				(void)fprintf(postscript.file,"gsave\n");
				(void)fprintf(postscript.file,"/scan_line %d string def\n",
					(int)scan_line_length);
				(void)fprintf(postscript.file,"%d %d translate\n",dest_x,dest_y);
				(void)fprintf(postscript.file,"%u %u\n",width,height);
				(void)fprintf(postscript.file,"8\n");
				(void)fprintf(postscript.file,"[1 0 0 -1 0 %u]\n",height);
				(void)fprintf(postscript.file,"{\n");
				(void)fprintf(postscript.file,
					"  currentfile scan_line readhexstring pop\n");
				(void)fprintf(postscript.file,"}\n");
				(void)fprintf(postscript.file,"false\n");
				(void)fprintf(postscript.file,"3\n");
				(void)fprintf(postscript.file,"colorimage\n");
				/* output image data */
				count=0;
				for (row=0;row<height;row++)
				{
					for (column=0;column<width;column++)
					{
						colour.pixel=XGetPixel(image,column+src_x,row+src_y);
						if (background_pixel==colour.pixel)
						{
							(void)fprintf(postscript.file,"%02x%02x%02x",
								background_red,background_green,background_blue);
						}
						else
						{
							XQueryColor(display,postscript.colour_map,&colour);
							(void)fprintf(postscript.file,"%02x%02x%02x",
								(colour.red)>>8,(colour.green)>>8,(colour.blue)>>8);
						}
						count++;
						if (count>=13)
						{
							count=0;
							(void)fprintf(postscript.file,"\n");
						}
					}
				}
				if (count!=0)
				{
					(void)fprintf(postscript.file,"\n");
				}
				(void)fprintf(postscript.file,"grestore\n");
				(void)fprintf(postscript.file,"\n");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"XPSPutImage.  Missing image or could not access colour map");
		}
	}
	else
	{
		XPutImage(display,drawable,gc,image,src_x,src_y,dest_x,dest_y,width,height);
	}
	LEAVE;
} /* XPSPutImage */

#if defined (CODE_FRAGMENTS)
void XPSPutImage(Display *display,Drawable drawable,GC gc,XImage *image,
	int src_x,int src_y,int dest_x,int dest_y,unsigned int width,
	unsigned int height)
/*******************************************************************************
LAST MODIFIED : 16 October 1993

DESCRIPTION :
If the <postscript> file is open then PostScript for displaying the <image> is
written to the file, otherwise XPutImage is called.
???In the future the idea is for it to switch based on either the <display> or
the drawable (not sure which)
==============================================================================*/
{
	static char *put_image_postscript[]=
	{
		"%",
		"% Variables used in the display of images",
		"%",
		"/byte 1 string def",
		"/color_packet 3 string def",
		"/compression 1 string def",
		"/gray_packet 1 string def",
		"/pixels 768 string def",
		"",
		"%",
		"% Routines used in the display of images",
		"%",
		"/GrayPseudoClassPacket",
		"{",
		"  %",
		"  % Get a PseudoClass packet;  convert to grayscale.",
		"  %",
		"  % Parameters: ",
		"  %   index: index into the colormap.",
		"  %   length: number of pixels minus one of this color (optional).",
		"  %",
		"  currentfile byte readhexstring pop 0 get",
		"  /offset exch 3 mul def",
		"  /color_packet colormap offset 3 getinterval def",
		"  color_packet 0 get red_in_gray mul",
		"  color_packet 1 get green_in_gray mul add",
		"  color_packet 2 get blue_in_gray mul add",
		"  cvi",
		"  /gray_packet exch def",
		"  compression 0 gt",
		"  {",
		"    /number_pixels 1 def",
		"  }",
		"  {",
		"    currentfile byte readhexstring pop 0 get",
		"    /number_pixels exch 1 add def",
		"  } ifelse",
		"  0 1 number_pixels 1 sub",
		"  {",
		"    pixels exch gray_packet put",
		"  } for",
		"  pixels 0 number_pixels getinterval",
		"} bind def",
		"",
		"/PseudoClassPacket",
		"{",
		"  %",
		"  % Get a PseudoClass packet.",
		"  %",
		"  % Parameters: ",
		"  %   index: index into the colormap.",
		"  %   length: number of pixels minus one of this color (optional).",
		"  %",
		"  currentfile byte readhexstring pop 0 get",
		"  /offset exch 3 mul def",
		"  /color_packet colormap offset 3 getinterval def",
		"  compression 0 gt",
		"  {",
		"    /number_pixels 3 def",
		"  }",
		"  {",
		"    currentfile byte readhexstring pop 0 get",
		"    /number_pixels exch 1 add 3 mul def",
		"  } ifelse",
		"  0 3 number_pixels 1 sub",
		"  {",
		"    pixels exch color_packet putinterval",
		"  } for",
		"  pixels 0 number_pixels getinterval",
		"} bind def",
		"",
		"/set_colour_map",
		"{",
		"  %",
		"  % Sets up a colour map for displaying images",
		"  % Parameters: ",
		"  %",
		"  %   colors: number of colors in the colormap.",
		"  %   colormap: red, green, blue color packets.",
		"  %",
		"  currentfile buffer readline pop",
		"  token pop /colors exch def pop",
		"  /colors colors 3 mul def",
		"  /colormap colors string def",
		"  currentfile colormap readhexstring pop pop",
		"} bind def",
		"",
		"/PseudoClassImage",
		"{",
		"  %",
		"  % Display a PseudoClass image.",
		"  %",
		"  systemdict /colorimage known",
		"  {",
		"    columns rows 8",
		"    [",
		"      1 0",
		"      0 -1",
		"      0 rows",
		"    ]",
		"    { PseudoClassPacket } false 3 colorimage",
		"  }",
		"  {",
		"    %",
		"    % No colorimage operator;  convert to grayscale.",
		"    %",
		"    columns rows 8",
		"    [",
		"      1 0",
		"      0 -1",
		"      0 rows",
		"    ]",
		"    { GrayPseudoClassPacket } image",
		"  } ifelse",
		"} bind def",
		"",
		"/DisplayImage",
		"{",
		"  %",
		"  % Display a PseudoClass image.",
		"  %",
		"  % Parameters: ",
		"  %   x & y translation.",
		"  %   image columns & rows.",
		"  %   compression: 0-RunlengthEncodedCompression or 1-NoCompression.",
		"  %   hex color packets.",
		"  %",
		"  gsave",
		"  currentfile buffer readline pop",
		"  token pop /x exch def",
		"  token pop /y exch def pop",
		"  x y translate",
		"  currentfile buffer readline pop",
		"  token pop /columns exch def",
		"  token pop /rows exch def pop",
		"  currentfile buffer readline pop",
		"  token pop /compression exch def pop",
		"  PseudoClassImage",
		"  grestore",
		"} bind def",
		"",
		NULL
	};
	char **statement;
	int colour,column,count,number_of_colours,return_code,row;
	XColor *colours;

	/*???DB. temp */
	char bit_mask,*blue,byte,*green,*parity,*red;
	Pixel pixel;
	XColor contour_colour,pixel_colour;

	ENTER(XPSPutImage);
	if (postscript.file)
	{
#endif
#if defined (CODE_FRAGMENTS)
/* remove while testing black and white version */
		if (first_put_image)
		{
			/* output postscript for some put_image routines */
			for (statement=put_image_postscript;*statement;statement++)
			{
				(void)fprintf(postscript.file,"%s\n",*statement);
			}
			first_put_image=0;
		}
		/*???Should get visual from graphics context ? */
		/*???Use src, dest and size ? */
		if (image&&screen_visual&&
			ALLOCATE(colours,XColor,(number_of_colours=screen_visual->map_entries)))
		{
			/* write the postscript for the colour map.  NB the drawing background
				colour is changed to the PostScript background colour */
			(void)fprintf(postscript.file,"set_colour_map\n");
			/*??? not for true or direct colour.  See XReadColormap in
				ImageMagick/X.c */
			for (colour=0;colour<number_of_colours;colour++)
			{
				colours[colour].pixel=colour;
				colours[colour].pad=0;
			}
			XQueryColors(display,postscript.colour_map,colours,number_of_colours);
			/* set the background drawing colour to the background postscript
				colour */
			colours[postscript.background_drawing_colour].red=
				colours[postscript.background_printer_colour].red;
			colours[postscript.background_drawing_colour].blue=
				colours[postscript.background_printer_colour].blue;
			colours[postscript.background_drawing_colour].green=
				colours[postscript.background_printer_colour].green;
			(void)fprintf(postscript.file,"%d\n",number_of_colours);
			for (colour=0;colour<number_of_colours;colour++)
			{
				(void)fprintf(postscript.file,"%02x%02x%02x\n",(colours[colour].red)>>8,
					(colours[colour].green)>>8,(colours[colour].blue)>>8);
			}
			(void)fprintf(postscript.file,"\n");
			DEALLOCATE(colours);
			/* write the postscript for displaying the image */
			(void)fprintf(postscript.file,"DisplayImage\n");
			/* output pseudo colour uncompressed image */
			(void)fprintf(postscript.file,"%d %d\n%u %u\n1\n",dest_x,dest_y,width,
				height);
			count=0;
			for (row=0;row<height;row++)
			{
				for (column=0;column<width;column++)
				{
					count++;
					(void)fprintf(postscript.file,"%02x",
						(unsigned short)XGetPixel(image,column+src_x,row+src_y));
					if (count>=36)
					{
						count=0;
						(void)fprintf(postscript.file,"\n");
					}
				}
			}
			if (count!=0)
			{
				(void)fprintf(postscript.file,"\n");
			}
			(void)fprintf(postscript.file,"\n");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"XPSPutImage.  Missing image or could not access colour map");
		}
#endif
#if defined (CODE_FRAGMENTS)
		/* testing black and white */
		if (image&&screen_visual&&
			ALLOCATE(colours,XColor,(number_of_colours=screen_visual->map_entries))&&
			ALLOCATE(parity,char,number_of_colours))
		{
			for (colour=0;colour<number_of_colours;colour++)
			{
				colours[colour].pixel=colour;
				colours[colour].pad=0;
			}
			XQueryColors(display,postscript.colour_map,colours,number_of_colours);
			contour_colour.pixel=postscript.contour_colour;
			XQueryColor(display,postscript.colour_map,&contour_colour);
			for (colour=0;colour<number_of_colours;colour++)
			{
				if ((contour_colour.red==colours[colour].red)&&
					(contour_colour.green==colours[colour].green)&&
					(contour_colour.blue==colours[colour].blue))
				{
					parity[colour]=1;
				}
				else
				{
					parity[colour]=0;
				}
			}
			DEALLOCATE(colours);
			/* write the postscript for displaying the image */
			(void)fprintf(postscript.file,"gsave\n");
			(void)fprintf(postscript.file,"0 setgray\n");
			(void)fprintf(postscript.file,"%d %d translate\n",dest_x,dest_y);
			(void)fprintf(postscript.file,"%u %u\n",width,height);
			(void)fprintf(postscript.file,"[1 0 0 -1 0 %u]\n",height);
			(void)fprintf(postscript.file,"true\n");
			(void)fprintf(postscript.file,"{\n");
			(void)fprintf(postscript.file,"  <\n");
			/* output image data */
			count=0;
			for (row=0;row<height;row++)
			{
				for (column=0;column<width;column++)
				{
					if (parity[XGetPixel(image,column+src_x,row+src_y)])
					{
						byte=byte|bit_mask;
					}
					bit_mask=bit_mask>>1;
					if (0x00==bit_mask)
					{
						(void)fprintf(postscript.file,"%02x",byte);
						byte=0x00;
						bit_mask=0x80;
						count++;
						if (count>=40)
						{
							count=0;
							(void)fprintf(postscript.file,"\n");
						}
					}
				}
				/* rows must start on a character boundary */
				if (0x80!=bit_mask)
				{
					(void)fprintf(postscript.file,"%02x",byte);
					byte=0x00;
					bit_mask=0x80;
					count++;
					if (count>=40)
					{
						count=0;
						(void)fprintf(postscript.file,"\n");
					}
				}
			}
			if (0x00!=bit_mask)
			{
				(void)fprintf(postscript.file,"%02x",byte);
				byte=0x00;
				bit_mask=0x80;
				count++;
				if (count>=40)
				{
					count=0;
					(void)fprintf(postscript.file,"\n");
				}
			}
			else
			{
				if (count!=0)
				{
					(void)fprintf(postscript.file,"\n");
				}
			}
			(void)fprintf(postscript.file,"  >\n");
			(void)fprintf(postscript.file,"}\n");
			(void)fprintf(postscript.file,"imagemask\n");
			(void)fprintf(postscript.file,"grestore\n");
			(void)fprintf(postscript.file,"\n");
			DEALLOCATE(parity);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"XPSPutImage.  Missing image or could not access colour map");
		}
#endif
#if defined (CODE_FRAGMENTS)
		/* testing colour */
		if (image&&screen_visual&&
			ALLOCATE(colours,XColor,(number_of_colours=screen_visual->map_entries))&&
			ALLOCATE(red,char,number_of_colours)&&
			ALLOCATE(green,char,number_of_colours)&&
			ALLOCATE(blue,char,number_of_colours))
		{
			for (colour=0;colour<number_of_colours;colour++)
			{
				colours[colour].pixel=colour;
				colours[colour].pad=0;
			}
			XQueryColors(display,postscript.colour_map,colours,number_of_colours);
			/* set the background drawing colour to the background postscript
				colour */
			colours[postscript.background_drawing_colour].red=
				colours[postscript.background_printer_colour].red;
			colours[postscript.background_drawing_colour].blue=
				colours[postscript.background_printer_colour].blue;
			colours[postscript.background_drawing_colour].green=
				colours[postscript.background_printer_colour].green;
			for (colour=0;colour<number_of_colours;colour++)
			{
				red[colour]=(char)((colours[colour].red)>>8);
				green[colour]=(char)((colours[colour].green)>>8);
				blue[colour]=(char)((colours[colour].blue)>>8);
			}
			DEALLOCATE(colours);
			/* write the postscript for displaying the image */
			(void)fprintf(postscript.file,"gsave\n");
			(void)fprintf(postscript.file,"/put_image_buffer 39 string def\n");
			(void)fprintf(postscript.file,"%d %d translate\n",dest_x,dest_y);
			(void)fprintf(postscript.file,"%u %u\n",width,height);
			(void)fprintf(postscript.file,"8\n");
			(void)fprintf(postscript.file,"[1 0 0 -1 0 %u]\n",height);
			(void)fprintf(postscript.file,"{\n");
			(void)fprintf(postscript.file,
				"  currentfile put_image_buffer readhexstring pop\n");
			(void)fprintf(postscript.file,"}\n");
			(void)fprintf(postscript.file,"false\n");
			(void)fprintf(postscript.file,"3\n");
			(void)fprintf(postscript.file,"colorimage\n");
			/* output image data */
			count=0;
			for (row=0;row<height;row++)
			{
				for (column=0;column<width;column++)
				{
					pixel=XGetPixel(image,column+src_x,row+src_y);
					(void)fprintf(postscript.file,"%02x%02x%02x",red[pixel],green[pixel],
						blue[pixel]);
					count++;
					if (count>=13)
					{
						count=0;
						(void)fprintf(postscript.file,"\n");
					}
				}
			}
			if (count!=0)
			{
				while (count<13)
				{
					(void)fprintf(postscript.file,"000000");
					count++;
				}
				(void)fprintf(postscript.file,"\n");
			}
			(void)fprintf(postscript.file,"grestore\n");
			(void)fprintf(postscript.file,"\n");
			DEALLOCATE(parity);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"XPSPutImage.  Missing image or could not access colour map");
		}
#endif
#if defined (CODE_FRAGMENTS)
	}
	else
	{
		XPutImage(display,drawable,gc,image,src_x,src_y,dest_x,dest_y,width,height);
	}
	LEAVE;
} /* XPSPutImage */
#endif

void XPSSetClipRectangles(Display *display,GC gc,int x,int y,
	XRectangle *rectangles,int num_rectangles,int order)
/*******************************************************************************
LAST MODIFIED : 10 May 1993

DESCRIPTION :
If the <postscript> file is open then PostScript for setting the clipping
rectangles is written to the file, otherwise XSetClipRectangles is called.
???Initially, for postscript, only one rectangle will be used and the clipping
will be for all GCs.
==============================================================================*/
{
	static char *put_image_postscript[]=
	{
		"/SetClipRectangles",
		"{",
		"  %",
		"  % Set the clipping mask to a set of rectangles.",
		"  %",
		"  % Parameters: ",
		"  %   number of clip rectangles",
		"  %   left, bottom, width, height quadruples (one to a line)",
		"  %",
		"  currentfile buffer readline pop",
		"  token pop /number_of_rectangles exch def",
		"  1 1 number_of_rectangles",
		"  {",
		"    pop",
		"    currentfile buffer readline pop",
		"    token pop /left exch def",
		"    token pop /bottom exch def",
		"    token pop /width exch def",
		"    token pop /height exch def",
		"    newpath",
		"    left bottom moveto",
		"    width 0 rlineto",
		"    0 height rlineto",
		"    width neg 0 rlineto",
		"    closepath",
		"    clip",
		"    newpath",
/*    "    left bottom width height rectclip",*/
		"  } for",
		"} bind def",
		"",
		NULL
	};
	char **statement;
	int i;
	XRectangle *clip_rectangle;

	ENTER(XPSSetClipRectangles);
	if (postscript.file)
	{
		if (num_rectangles>0)
		{
			if (first_set_clip_rectangles)
			{
				/* output postscript for some put_image routines */
				for (statement=put_image_postscript;*statement;statement++)
				{
					(void)fprintf(postscript.file,"%s\n",*statement);
				}
				first_set_clip_rectangles=0;
			}
			/* call the PostScript routine for  setting the clip rectangles */
			(void)fprintf(postscript.file,"SetClipRectangles\n");
			/* specify the number of rectangles */
			(void)fprintf(postscript.file,"%d\n",num_rectangles);
			/* specify the clip rectangles */
			clip_rectangle=rectangles;
			for (i=num_rectangles;i>0;i--)
			{
				fprintf(postscript.file,"%.5g %.5g %.5g %.5g\n",
					(float)(clip_rectangle->x)-world_origin_x,world_origin_y-
					(float)(clip_rectangle->y+(int)(clip_rectangle->height)-1),
					(float)(clip_rectangle->width-1),(float)(clip_rectangle->height-1));
				clip_rectangle++;
			}
		}
	}
	else
	{
		XSetClipRectangles(display,gc,x,y,rectangles,num_rectangles,order);
	}
	LEAVE;
} /* XPSSetClipRectangles */

void XPSSetClipMask(Display *display,GC gc,Pixmap pixmap)
/*******************************************************************************
LAST MODIFIED : 10 May 1993

DESCRIPTION :
If the <postscript> file is open then PostScript for setting the clip mask
attribute of the <gc> is written to the file, otherwise XSetClipMask is called.
???Initially, for postscript, this will turn off clipping.
==============================================================================*/
{
	ENTER(XPSSetClipMask);
	if (postscript.file)
	{
		fprintf(postscript.file,"%% Clear clipping rectangle\n");
		fprintf(postscript.file,"initclip\n");
	}
	else
	{
		XSetClipMask(display,gc,pixmap);
	}
	LEAVE;
} /* XPSSetClipMask */
