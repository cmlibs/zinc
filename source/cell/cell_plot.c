/*******************************************************************************
FILE : cell_plot.c

LAST MODIFIED : 11 June 2001

DESCRIPTION :
The object used to draw plots in Cell.
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
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/PanedW.h>
#include <Xm/ScrolledW.h>
#include <Xm/RowColumn.h>

#include "cell/cell_plot.h"
#include "general/object.h"
#include "general/list.h"
#include "general/indexed_list_private.h"
#include "general/compare.h"

/*
Module objects
--------------
*/
enum Text_vertical_alignment
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
The different methods of aligning text vertically, relative to a given set of
coordinates (x,y)
==============================================================================*/
{
  UP,     /* (x,y) is above the text */
  MIDDLE, /* (x,y) is in the vertical centre of the text */
  DOWN    /* (x,y) is on the baseline of the text */
};

enum Text_horizontal_alignment
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
The different methods of aligning text horizontally, relative to a given set of
coordinates (x,y)
==============================================================================*/
{
  LEFT,   /* (x,y) is at the left end of the text */
  CENTER, /* (x,y) is in the centre of the text */
  RIGHT   /* (x,y) is at the right end of the text */
};

struct Cell_plot_data
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
An object used to hold information about a specific data set in a plot
==============================================================================*/
{
  /* The component's ID, used for the indexed list because the name is
   * not guaranteed to be unique - not used??
   */
  int id;
  /* The access counter */
  int access_count;
  /* The name of the variable */
  char *name;
  /* A pointer to the actual variable */
  struct Cell_variable *variable;
  /* The drawing area used to hold the label pixmap */
  Widget drawing_area;
  /* and the label pixmap */
  Pixmap pixmap;
  /* The colour used for this data set */
  char *colour;
  /* The graphics context to use */
  GC gc;
}; /* struct Cell_plot_data */

struct Cell_plot
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
The cell plot object
==============================================================================*/
{
  /* The paned window containing the plot */
  Widget pane;
  /* The drawing area which holds the plot's pixmap */
  Widget drawing_area;
  /* The plot's pixmap */
  Pixmap pixmap;
  /* Keep a graphics context here to save creating it all the time */
  GC gc;
  /* The window for the plot key */
  Widget key_window;
  /* The list of data sets */
  struct LIST(Cell_plot_data) *data_list;
}; /* struct Cell_plot */

DECLARE_LIST_TYPES(Cell_plot_data);
FULL_DECLARE_INDEXED_LIST_TYPE(Cell_plot_data);

/*
Module Variables
================
*/
char normal_font_name[] = {"-*-tekton-*-*-*-*-14-*-*-*-*-*-*-*"};
char small_font_name[] = {"-*-tekton-*-*-*-*-10-*-*-*-*-*-*-*"};

/*
Module Functions
================
*/
PROTOTYPE_OBJECT_FUNCTIONS(Cell_plot_data);
PROTOTYPE_LIST_FUNCTIONS(Cell_plot_data);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cell_plot_data,name,char *);

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cell_plot_data,name,char *,strcmp)

static char *get_data_colour(void)
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Returns a string specifying the colour to use for a data set
==============================================================================*/
{
  char *colour;
  char *colours[] = {"red","blue","green","white","yellow"};
  int number_of_colours = 5;
  static int number = 0;

  ENTER(get_data_colour);
  if (number >= number_of_colours)
  {
    number = 0;
  }
  if (ALLOCATE(colour,char,strlen(colours[number])+1))
  {
    strcpy(colour,colours[number]);
    number++;
  }
  else
  {
    colour = (char *)NULL;
  }
  LEAVE;
  return(colour);
} /* get_data_colour() */

static void set_foreground_colour(Display *display,GC gc,char *colour)
/*******************************************************************************
LAST MODIFIED : 22 March 2001

DESCRIPTION :
Sets the foreground colour of the given graphics context <gc> to be the
specified <colour>.
==============================================================================*/
{
  Colormap colour_map;
  XColor screen_colour,exact_colour;

  ENTER(set_foreground_colour);
  if (display && gc && colour)
  {
    /* set the default colour map */
    if (colour_map = DefaultColormap(display,DefaultScreen(display)))
    {
      /* allocate the specified colour */
      XAllocNamedColor(display,colour_map,colour,&screen_colour,&exact_colour);
      /* set the foreground colour */
      XSetForeground(display,gc,screen_colour.pixel);
    }
    else
    {
      display_message(ERROR_MESSAGE,"set_foreground_colour.  "
        "Unable to get the default colour map");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_foreground_colour.  "
      "Invalid argument(s)");
  }
  LEAVE;
}/* set_foreground_colour() */

static int get_label_width(Display *display,char *font,char *label)
/*******************************************************************************
LAST MODIFIED : 22 March 2001

DESCRIPTION :
Returns the width of the given <label> when drawn with the specified <font>
==============================================================================*/
{
  XFontStruct *font_info;
  int width;

  ENTER(get_label_width);
  if (display && label && font)
  {
    /* load the font and font structure */
    if (font_info = XLoadQueryFont(display,font))
    {
      /* get the width of the label */
      width = XTextWidth(font_info,label,strlen(label));
      XUnloadFont(display,font_info->fid);
    }
    else
    {
      display_message(ERROR_MESSAGE,"get_label_width.  "
        "Unable to load the specified font (%s)",font);
      width = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"get_label_width.  "
      "Invalid argument(s)");
    width = 0;
  }
  LEAVE;
  return(width);
} /* get_label_width() */

static int get_label_height(Display *display,char *font,char *label)
/*******************************************************************************
LAST MODIFIED : 22 March 2001

DESCRIPTION :
Returns the height of the given <label> when drawn with the specified <font>
==============================================================================*/
{
  XFontStruct *font_info;
  int height;

  ENTER(get_label_height);
  if (display && label && font)
  {
    /* load the font and font structure */
    if (font_info = XLoadQueryFont(display,font))
    {
      /* get the height of the label */
      height = font_info->ascent + font_info->descent;
      XUnloadFont(display,font_info->fid);
    }
    else
    {
      display_message(ERROR_MESSAGE,"get_label_height.  "
        "Unable to load the specified font (%s)",font);
      height = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"get_label_height.  "
      "Invalid argument(s)");
    height = 0;
  }
  LEAVE;
  return(height);
} /* get_label_height() */

static int draw_label(Display *display,Drawable drawable,GC gc,
  char *text_colour,int x,int y,enum Text_vertical_alignment Valign,
  enum Text_horizontal_alignment Halign,char *label,char *label_sub,
  char *label_sup,char *label_font,char *label_sub_font,Region *region,int *end)
/*******************************************************************************
LAST MODIFIED : 22 March 2001

DESCRIPTION :
Draws the specified label and subscript/superscript to the specified drawable.
If required, will define a rectangular region around the text. Returns the lower
extremity of the text.

Parameters :
Display *display - the current display
Drawable drawable - the drawable to be drawn to
GC gc - the GC to use for the drawing
char *text_colour - the colour of the text
int x,y - used to position the text
int v_position - specifies the position of the label relative to x,y - can be:
                 UP, DOWN, or MIDDLE
int h_position - specifies the position of the label relative to x,y - can be:
                 LEFT, RIGHT, or CENTER
char *label - the text for the label
char *label_sub - the label's subscript
char *label_sup - the label's superscript
char *label_font - name of the font to be used for the label
char *label_sub_font - name of the font to be used for the label's subscript
Region *region - if non-null on entry, returns the region defined around the text
int *end - returns the position of the end of the label
==============================================================================*/
{
  XFontStruct *font_info,*font_info_sub;
  int width_label,width_all,height_all;
  int origin_x,origin_y,ret;
  unsigned long offset_x;
  XPoint reg_pts[4];

  ENTER(draw_label);
  if (display && drawable && gc && text_colour && label && label_font)
  {
    /* load the font and font structure */
    if ((font_info = XLoadQueryFont(display,label_font)) &&
      (font_info_sub = XLoadQueryFont(display,label_sub_font)))
    {
      /* place the font into the GC */
      XSetFont(display,gc,font_info->fid);
      /* set the text colour */
      set_foreground_colour(display,gc,text_colour);
      /* if there is a subscript */
      if (label_sub != (char *)NULL)
      {
        /* get the width of the label */
        width_label = XTextWidth(font_info,label,strlen(label));
        /* get the the subscript distance from the font structure */
        XGetFontProperty(font_info,XA_SUBSCRIPT_X,&offset_x);
        /*offset_x = XTextWidth(font_info,"I",1);*/
        /* find the origin of the text */
        if (label_sup != (char *)NULL)
        {
          width_all = width_label +
            XTextWidth(font_info_sub,label_sub,strlen(label_sub)) +
            XTextWidth(font_info_sub,label_sub,strlen(label_sup));
        }
        else
        {
          width_all = width_label +
            XTextWidth(font_info_sub,label_sub,strlen(label_sub));
        }
        if (Halign == LEFT)
        {
          origin_x = x;
        }
        else if (Halign == CENTER)
        {
          origin_x = x-(width_all/2);
        }
        else
        {
          origin_x = x-width_all;
        }
        if (Valign == UP)
        {
          origin_y = y+(font_info->ascent);
        }
        else if (Valign == MIDDLE)
        {
          origin_y = y+(((font_info->ascent)+(font_info->descent))/2)-
            (font_info->descent);
        }
        else
        {
          origin_y = y-(font_info->descent);
        }
        if (label_sup != (char *)NULL)
        {
          height_all = 2*(int)offset_x + (int)(font_info->descent +
            font_info->ascent);
        }
        else
        {
          height_all = (int)(font_info->ascent + offset_x + font_info->descent);
        }
        /* draw on the label */
        XDrawString(display,drawable,gc,origin_x,origin_y,label,strlen(label));
        if (label_sup != (char *)NULL)
        {
          XDrawString(display,drawable,gc,origin_x+width_label,
            (int)(origin_y-offset_x),label_sup,strlen(label_sup));
        }
        /* place the font into the GC */
        XSetFont(display,gc,font_info_sub->fid);
        /* draw on the subscript */
        XDrawString(display,drawable,gc,origin_x+width_label,
          (int)(origin_y+offset_x),label_sub,strlen(label_sub));
        /* evaluate return value */
        ret = origin_y + (int)(font_info->descent + offset_x);
      }
      else /* No subscript */
      {
        /* get the width of the label */
        width_label = XTextWidth(font_info,label,strlen(label));
        /* get the the superscript distance from the font structure */
        offset_x = XTextWidth(font_info,"I",1);
        /* get the width of the label */
        if (label_sup != (char *)NULL)
        {
          width_all = XTextWidth(font_info,label,strlen(label)) +
            XTextWidth(font_info,label_sup,strlen(label_sup));
          height_all = (int)(font_info->descent +
            offset_x + font_info->ascent);
        }
        else
        {
          width_all = XTextWidth(font_info,label,strlen(label));
          height_all = font_info->ascent + font_info->descent;
        }
        /* find the origin of the text */
        if (Halign == LEFT)
        {
          origin_x = x;
        }
        else if (Halign == CENTER)
        {
          origin_x = x-(width_all/2);
        }
        else
        {
          origin_x = x-width_all;
        }
        if (Valign == UP)
        {
          origin_y = y+(font_info->ascent);
        }
        else if (Valign == MIDDLE)
        {
          origin_y = y+(((font_info->ascent)+(font_info->descent))/2)-
            (font_info->descent);
        }
        else 
        {
          origin_y = y-(font_info->descent);
        }
        /* draw on the label */
        XDrawString(display,drawable,gc,origin_x,origin_y,label,strlen(label));
        if (label_sup != (char *)NULL)
        {
          XDrawString(display,drawable,gc,origin_x+width_label,
            origin_y-((int)offset_x),label_sup,strlen(label_sup));
        }
        /* evaluate return value */
        ret = origin_y + (int)font_info->descent;
      }
      if (region != (Region *)NULL)
      {
        /* create the region */
        reg_pts[0].x = origin_x;
        reg_pts[0].y = origin_y - font_info->ascent;
        reg_pts[1].x = reg_pts[0].x + width_all;
        reg_pts[1].y = reg_pts[0].y;
        reg_pts[2].x = reg_pts[1].x;
        reg_pts[2].y = reg_pts[0].y + height_all;
        reg_pts[3].x = reg_pts[0].x;
        reg_pts[3].y = reg_pts[2].y;
        *region = XPolygonRegion(reg_pts,4,WindingRule);
      }
      if (end != (int *)NULL)
      {
        *end = origin_x + width_all;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"draw_label.  "
        "Unable to load the specified font(s)");
      ret = -1;
    }
    XUnloadFont(display,font_info->fid);
    XUnloadFont(display,font_info_sub->fid);
  }
  else
  {
    display_message(ERROR_MESSAGE,"draw_label.  "
      "Invalid argument(s)");
    ret = -1;
  }
  LEAVE;
  return (ret);
}/* draw_label() */

static int DESTROY(Cell_plot_data)(struct Cell_plot_data **data_address)
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Destroys a Cell pl;ot data object.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_plot_data *data;
  
  ENTER(DESTROY(Cell_plot_data));
  if (data_address && (data = *data_address))
  {
    if (data->access_count == 0)
    {
      if (data->name)
      {
        DEALLOCATE(data->name);
      }
      if (data->drawing_area)
      {
        if (data->pixmap)
        {
          XFreePixmap(XtDisplay(data->drawing_area),data->pixmap);
        }
        if (data->gc)
        {
          XFreeGC(XtDisplay(data->drawing_area),data->gc);
        }
        XtDestroyWidget(data->drawing_area);
        data->drawing_area = (Widget)NULL;
      }
      DEALLOCATE(*data_address);
      *data_address = (struct Cell_plot_data *)NULL;
      return_code = 1;
    }
    else
    {
      display_message(WARNING_MESSAGE,"DESTROY(Cell_plot_data).  "
        "Access count is not zero - cannot destroy object");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"DESTROY(Cell_plot_data).  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* DESTROY(Cell_plot_data)() */

static struct Cell_plot_data *CREATE(Cell_plot_data)(
  struct Cell_variable *variable)
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Creates a Cell_plot_data object.
==============================================================================*/
{
  struct Cell_plot_data *data = (struct Cell_plot_data *)NULL;
  char *name;

  ENTER(CREATE(Cell_plot_data));
  if (variable)
  {
    if (ALLOCATE(data,struct Cell_plot_data,1))
    {
      /* Initialise */
      data->id = -1;
      data->access_count = 0;
      data->name = (char *)NULL;
      data->colour = get_data_colour();
      data->variable = variable;
      data->drawing_area = (Widget)NULL;
      data->pixmap = (Pixmap)NULL;
      data->gc = (GC)NULL;
      if (name = Cell_variable_get_name(variable))
      {
        if (ALLOCATE(data->name,char,strlen(name)+1))
        {
          strcpy(data->name,name);
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_plot_data).  "
            "Unable to allocate memory for the data name");
          DESTROY(Cell_plot_data)(&data);
          data = (struct Cell_plot_data *)NULL;
        }
        DEALLOCATE(name);
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_plot_data).  "
          "Unable to get the name of the variable");
        DESTROY(Cell_plot_data)(&data);
        data = (struct Cell_plot_data *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_plot_data).  "
        "Unable to allocate memory for the Cell plot data object");
      data = (struct Cell_plot_data *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_plot_data).  "
      "Invalid argument(s)");
    data = (struct Cell_plot_data *)NULL;
  }
  LEAVE;
  return(data);
} /* CREATE(Cell_plot_data)() */

static void drawing_area_CB(Widget widget,XtPointer plot_void,
  XtPointer cbs_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2001

DESCRIPTION :
Callback function for the main plot drawing area for expose/resize events.
==============================================================================*/
{
  struct Cell_plot *plot = (struct Cell_plot *)NULL;
  XmDrawingAreaCallbackStruct *cbs;
  int x_return,y_return;
  unsigned int width_return,height_return,border,depth_return;
  Window root_return;
  
  ENTER(drawing_area_CB);
  if (widget && (cbs = (XmDrawingAreaCallbackStruct *)cbs_void) &&
    (plot = (struct Cell_plot *)plot_void))
  {
    /* only continue if we have a window... */
    if (cbs->window)
    {
      /* If the plot has been resized or does not exist, need to creatge it */
      if ((cbs->reason == XmCR_RESIZE) || (plot->pixmap == (Pixmap)NULL))
      {
        /* get the current width and height */
        XGetGeometry(XtDisplay(widget),cbs->window,&root_return,&x_return,
          &y_return,&width_return,&height_return,&border,&depth_return);
        /* draw the pixmap */
        XSetForeground(XtDisplay(widget),plot->gc,
          BlackPixelOfScreen(XtScreen(widget)));
        if (plot->pixmap)
        {
          XFreePixmap(XtDisplay(widget),plot->pixmap);
        }
        plot->pixmap = XCreatePixmap(XtDisplay(widget),root_return,width_return,
          height_return,depth_return);
        XFillRectangle(XtDisplay(widget),plot->pixmap,plot->gc,0,0,width_return,
          height_return);
        XSetForeground(XtDisplay(widget),plot->gc,
          WhitePixelOfScreen(XtScreen(widget)));
        XFillRectangle(XtDisplay(widget),plot->pixmap,plot->gc,width_return/4,
          height_return/4,width_return/2,height_return/2);
        XCopyArea(XtDisplay(widget),plot->pixmap,cbs->window,plot->gc,0,0,
          width_return,height_return,0,0);
      }
      else
      {
        /* get the current width and height */
        XGetGeometry(XtDisplay(widget),cbs->window,&root_return,&x_return,
          &y_return,&width_return,&height_return,&border,&depth_return);
        XCopyArea(XtDisplay(widget),plot->pixmap,cbs->window,plot->gc,0,0,
          width_return,height_return,0,0);
      }
    } /* if (cbs->window) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"drawing_area_CB.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* drawing_area_CB() */

static void data_drawing_area_CB(Widget widget,XtPointer data_void,
  XtPointer cbs_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Callback function for the data set drawing area for expose/resize events.
==============================================================================*/
{
  struct Cell_plot_data *data = (struct Cell_plot_data *)NULL;
  XmDrawingAreaCallbackStruct *cbs;
  int x_return,y_return;
  unsigned int width_return,height_return,border,depth_return;
  Window root_return;
  
  ENTER(data_drawing_area_CB);
  if (widget && (cbs = (XmDrawingAreaCallbackStruct *)cbs_void) &&
    (data = (struct Cell_plot_data *)data_void))
  {
    /* only continue if we have a window... */
    if (cbs->window)
    {
      /* If the key has been resized or does not exist, need to creatge it */
      if ((cbs->reason == XmCR_RESIZE) || (data->pixmap == (Pixmap)NULL))
      {
        /* get the current width and height */
        XGetGeometry(XtDisplay(widget),cbs->window,&root_return,&x_return,
          &y_return,&width_return,&height_return,&border,&depth_return);
        /* draw the pixmap */
        if (data->pixmap)
        {
          XFreePixmap(XtDisplay(widget),data->pixmap);
        }
        data->pixmap = XCreatePixmap(XtDisplay(widget),root_return,
          width_return,height_return,depth_return);
        set_foreground_colour(XtDisplay(widget),data->gc,"black");
        XFillRectangle(XtDisplay(widget),data->pixmap,data->gc,0,0,
          width_return,height_return);
/*          set_foreground_colour(XtDisplay(widget),data->gc,data->colour); */
/*          XFillRectangle(XtDisplay(widget),data->pixmap,data->gc,width_return/4, */
/*            height_return/4,width_return/2,height_return/2); */
        draw_label(XtDisplay(widget),data->pixmap,data->gc,data->colour,
          width_return/2,height_return/2,MIDDLE,CENTER,data->name,(char *)NULL,
          (char *)NULL,normal_font_name,small_font_name,(Region *)NULL,
          (int *)NULL);
        XCopyArea(XtDisplay(widget),data->pixmap,cbs->window,data->gc,0,0,
          width_return,height_return,0,0);
      }
      else
      {
        /* get the current width and height */
        XGetGeometry(XtDisplay(widget),cbs->window,&root_return,&x_return,
          &y_return,&width_return,&height_return,&border,&depth_return);
        /* and copy over the pixmap */
        XCopyArea(XtDisplay(widget),data->pixmap,cbs->window,data->gc,0,0,
          width_return,height_return,0,0);
      }
    } /* if (cbs->window) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"data_drawing_area_CB.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* data_drawing_area_CB() */

static void data_drawing_area_input_CB(Widget widget,XtPointer data_void,
  XtPointer cbs_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Callback function for the data set drawing area for input events.
==============================================================================*/
{
  struct Cell_plot_data *data = (struct Cell_plot_data *)NULL;
  XmDrawingAreaCallbackStruct *cbs;
  
  ENTER(data_drawing_area_input_CB);
  if (widget && (cbs = (XmDrawingAreaCallbackStruct *)cbs_void) &&
    (data = (struct Cell_plot_data *)data_void))
  {
    /* FYI...
     * if ((event->xany.type == ButtonPress)&&(event->xbutton.button != 3))
     */
    if (cbs->event && (cbs->event->xany.type == ButtonRelease))
    {
      display_message(INFORMATION_MESSAGE,"data_drawing_area_input_CB.  "
        "Making the data set for %s bold!!\n",data->name);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"data_drawing_area_input_CB.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* data_drawing_area_input_CB() */

static int create_data_label(struct Cell_plot_data *data,Widget parent)
/*******************************************************************************
LAST MODIFIED : 12 March 2001

DESCRIPTION :
Creates the label for the given <data> set and puts it into the key widget
(<parent>).
==============================================================================*/
{
  int return_code = 0;
  int width,height;

  ENTER(create_data_label);
  if (data && parent && data->colour)
  {
    /* Get the size of the label */
    width = get_label_width(XtDisplay(parent),normal_font_name,data->name);
    height = get_label_height(XtDisplay(parent),normal_font_name,
      data->name);
    width *= 2;
    height *=2;
    if (data->drawing_area = XtVaCreateWidget("cell_plot_drawing_area",
      xmDrawingAreaWidgetClass,parent,
      XmNwidth,width,
      XmNheight,height,
      XmNresizePolicy,XmRESIZE_NONE,
      NULL))
    {
      /* And add the callbacks */
      XtAddCallback(data->drawing_area,XmNexposeCallback,
        data_drawing_area_CB,(XtPointer)data);
      XtAddCallback(data->drawing_area,XmNresizeCallback,
        data_drawing_area_CB,(XtPointer)data);
      XtAddCallback(data->drawing_area,XmNinputCallback,
        data_drawing_area_input_CB,(XtPointer)data);
      if (data->gc = XCreateGC(XtDisplay(data->drawing_area),
        RootWindowOfScreen(XtScreen(data->drawing_area)),0,NULL))
      {
        XtManageChild(data->drawing_area);
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_data_label.  "
          "Unable to create the GC for the label");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"create_data_label.  "
        "Invalid argument(s)");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_data_label.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* create_data_label() */

DECLARE_OBJECT_FUNCTIONS(Cell_plot_data)
DECLARE_INDEXED_LIST_FUNCTIONS(Cell_plot_data)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cell_plot_data,name,
  char *,strcmp)

/*
Global Functions
================
*/
struct Cell_plot *CREATE(Cell_plot)(Widget parent)
/*******************************************************************************
LAST MODIFIED : 08 March 2001

DESCRIPTION :
Creates a cell plot object. <parent> must be a form??
==============================================================================*/
{
  struct Cell_plot *plot = (struct Cell_plot *)NULL;
  Widget window;

  ENTER(CREATE(Cell_plot));
  if (parent)
  {
    if (ALLOCATE(plot,struct Cell_plot,1))
    {
      /* Initialise the object fields */
      plot->pane = (Widget)NULL;
      plot->drawing_area = (Widget)NULL;
      plot->pixmap = (Pixmap)NULL;
      plot->gc = (GC)NULL;
      plot->key_window = (Widget)NULL;
      plot->data_list = (struct LIST(Cell_plot_data) *)NULL;
      /* Create the form to hold the drawing areas */
      if (plot->pane = XtVaCreateWidget("cell_plot_form",
        xmPanedWindowWidgetClass,parent,
        XmNresizePolicy,XmRESIZE_ANY,
        XmNtopAttachment,XmATTACH_FORM,
        XmNbottomAttachment,XmATTACH_FORM,
        XmNleftAttachment,XmATTACH_FORM,
        XmNrightAttachment,XmATTACH_FORM,
        NULL))
      {
        /* Create the plot drawing area */
        if (plot->drawing_area = XtVaCreateWidget("cell_plot_drawing_area",
          xmDrawingAreaWidgetClass,plot->pane,
          NULL))
        {
          /* And add the callbacks */
          XtAddCallback(plot->drawing_area,XmNexposeCallback,
            drawing_area_CB,(XtPointer)plot);
          XtAddCallback(plot->drawing_area,XmNresizeCallback,
            drawing_area_CB,(XtPointer)plot);
          /* Create the plot key scrolled window */
          if (
            (window = XtVaCreateWidget("cell_plot_key_window",
              xmScrolledWindowWidgetClass,plot->pane,
              XmNscrollBarDisplayPolicy,XmSTATIC,
              XmNscrollingPolicy,XmAUTOMATIC,
              NULL)) &&
            (plot->key_window = XtVaCreateWidget("cell_plot_key_window",
              xmRowColumnWidgetClass,window,
              XmNpacking,XmPACK_TIGHT,
              XmNorientation,XmHORIZONTAL,
              NULL)))
          {
            /* Set the work window for the scrolled window */
            XtVaSetValues(window,
              XmNworkWindow,plot->key_window,
              NULL);
            /* Create a graphics context to use */
            if (plot->gc = XCreateGC(XtDisplay(plot->drawing_area),
              RootWindowOfScreen(XtScreen(plot->drawing_area)),0,NULL))
            {
              /* Create the list of data sets */
              if (plot->data_list = CREATE(LIST(Cell_plot_data))())
              {
                /* Manage the widgets */
                XtManageChild(plot->drawing_area);
                XtManageChild(plot->key_window);
                XtManageChild(window);
                XtManageChild(plot->pane);
              }
              else
              {
                display_message(ERROR_MESSAGE,"CREATE(Cell_plot).  "
                  "Unable to create the data set list");
                DESTROY(Cell_plot)(&plot);
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"CREATE(Cell_plot).  "
                "Unable to create the main plot GC");
              DESTROY(Cell_plot)(&plot);
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"CREATE(Cell_plot).  "
              "Unable to create the plot key drawing area");
            DESTROY(Cell_plot)(&plot);
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_plot).  "
            "Unable to create the main plot drawing area");
          DESTROY(Cell_plot)(&plot);
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_plot).  "
          "Unable to create the main plot form");
        DESTROY(Cell_plot)(&plot);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_plot).  "
        "Unable to allocate memory for the plot object");
      plot = (struct Cell_plot *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_plot).  "
      "Invalid argument(s)");
    plot = (struct Cell_plot *)NULL;
  }
  LEAVE;
  return(plot);
} /* CREATE(Cell_plot)() */

int DESTROY(Cell_plot)(struct Cell_plot **plot_address)
/*******************************************************************************
LAST MODIFIED : 08 March 2001

DESCRIPTION :
Destroys the given <plot>.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_plot *plot;

  ENTER(DESTROY(Cell_plot));
  if (plot_address && (plot = *plot_address))
  {
    /* Destroy the main pane (and all its children) */
    if (plot->pane)
    {
      /* Destroy the pixmap */
      if (plot->pixmap)
      {
        XFreePixmap(XtDisplay(plot->pane),plot->pixmap);
      }
      if (plot->gc)
      {
        XFreeGC(XtDisplay(plot->pane),plot->gc);
      }
      XtDestroyWidget(plot->pane);
    }
    /* Destroy the list of data sets */
    if (plot->data_list)
    {
      REMOVE_ALL_OBJECTS_FROM_LIST(Cell_plot_data)(plot->data_list);
      DESTROY(LIST(Cell_plot_data))(&(plot->data_list));
    }
    DEALLOCATE(*plot_address);
    *plot_address = (struct Cell_plot *)NULL;
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"DESTROY(Cell_plot).  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* DESTROY(Cell_plot)() */

int Cell_plot_set_pane_sizes(struct Cell_plot *plot,Widget shell)
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Sets the sizes of the plot and key drawing area's, proportional to the height of
the given <shell> widget. The <shell> will also be resized to accomodate the new
dimensions of the paned window.
==============================================================================*/
{
  int return_code = 0;
  Dimension height;

  ENTER(Cell_plot_set_pane_sizes);
  if (plot && shell)
  {
    if (plot->drawing_area)
    {
      XtVaGetValues(shell,
        XmNheight,&height,
        NULL);
      XtVaSetValues(plot->drawing_area,
        XmNheight,(Dimension)(5*((int)height)/8),
        NULL);
/*        XtVaSetValues(plot->key_drawing_area, */
/*          XmNheight,(Dimension)(((int)height)/8), */
/*          NULL); */
/*        XtUnmanageChild(XtParent(plot->pane)); */
/*        XtManageChild(XtParent(plot->pane)); */
/*        XtVaGetValues(plot->pane, */
/*          XmNheight,&new_height, */
/*          NULL); */
/*        XtVaSetValues(shell, */
/*          XmNheight,height+new_height, */
/*          NULL); */
/*        XtUnmanageChild(shell); */
/*        XtManageChild(shell); */
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_plot_set_pane_sizes.  "
        "Missing drawing area(s)");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_plot_set_pane_sizes.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_plot_set_pane_sizes() */

int Cell_plot_add_variable(struct Cell_plot *plot,
  struct Cell_variable *variable)
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Adds the given <variable> to the <plot>'s current data sets.
==============================================================================*/
{
  int return_code = 0;
  char *name;
  struct Cell_plot_data *data;

  ENTER(Cell_plot_add_variable);
  if (plot && variable)
  {
    if (name = Cell_variable_get_name(variable))
    {
      /* If the variable is not already in the list, then add it */
      if (!(FIND_BY_IDENTIFIER_IN_LIST(Cell_plot_data,name)(name,
        plot->data_list)))
      {
        if (data = CREATE(Cell_plot_data)(variable))
        {
          if (ADD_OBJECT_TO_LIST(Cell_plot_data)(data,plot->data_list))
          {
            create_data_label(data,plot->key_window);
          }
          else
          {
            display_message(ERROR_MESSAGE,"Cell_plot_add_variable.  "
              "Unable to add the variable (%s) to the data list",name);
            DESTROY(Cell_plot_data)(&data);
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"Cell_plot_add_variable.  "
            "Unable to create the data object for the variable: %s",name);
        }
      }
      DEALLOCATE(name);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_plot_add_variable.  "
        "Unable to get the variable's name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_plot_add_variable.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_plot_add_variable() */

int Cell_plot_remove_variable(struct Cell_plot *plot,
  struct Cell_variable *variable)
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Removes the given <variable> from the <plot>'s current data sets.
==============================================================================*/
{
  int return_code = 0;
  char *name;
  struct Cell_plot_data *data = (struct Cell_plot_data *)NULL;

  ENTER(Cell_plot_remove_variable);
  if (plot && variable)
  {
    if (name = Cell_variable_get_name(variable))
    {
      /* If the variable is in the list, then destroy it */
      if (data = FIND_BY_IDENTIFIER_IN_LIST(Cell_plot_data,name)(name,
        plot->data_list))
      {
        /* Removing the data set from the list should destroy it, since there
           should be nothing else ACCESS'ing it ?? */
        REMOVE_OBJECT_FROM_LIST(Cell_plot_data)(data,plot->data_list);
      }
      DEALLOCATE(name);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_plot_remove_variable.  "
        "Unable to get the variable's name");
      return_code = 0;
    }    
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_plot_remove_variable.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_plot_remove_variable() */

