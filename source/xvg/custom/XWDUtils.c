/*****************************************************************************/
/*                                                                           */
/*    File:      XWDUtils.c                                                  */
/*    Author:    Misc.                                                       */
/*    Modified:  25 September 1993                                           */
/*                                                                           */
/*    Purpose:   code to pick an image from the screen. Source code          */
/*               taken from "xwd.c" and "dsimple.c"                          */
/*                                                                           */
/*    Note:      This code does not work with a GL window. Although the      */
/*               number of colormap entries in the Visual structure of the   */
/*               grabbed window is 4096, the number of distinct colors in    */
/*               the cmap is only 256, but more importantly, the depth of    */
/*               the grabbed image is only 8 (not the correct 16 or 24).     */
/*                                                                           */
/*****************************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include "XvgGlobals.h"
#include <X11/cursorfont.h>
#include <X11/Xmu/WinUtil.h>

#define FEEP_VOLUME 0
#define lowbit(x) ((x) & (~(x) + 1))

/* Get the XColors of all pixels in image - returns # of colors */
static int Get_XColors(XWindowAttributes *win_info, XColor **colors)
{
    int i, ncolors;
    Colormap WinCMap;

    if (!(WinCMap = win_info->colormap)) {
      ErrorMsg("Null color map field in GrabWindow()");
      return(0);
    }
    ncolors = win_info->visual->map_entries;
    *colors = (XColor *) UxRealloc(*colors, sizeof(XColor) * ncolors);

    if (win_info->visual->class == DirectColor ||
	win_info->visual->class == TrueColor) {
	Pixel red, green, blue, red1, green1, blue1;

	red = green = blue = 0;
	red1 = lowbit(win_info->visual->red_mask);
	green1 = lowbit(win_info->visual->green_mask);
	blue1 = lowbit(win_info->visual->blue_mask);
	for (i=0; i<ncolors; i++) {
	  (*colors)[i].pixel = red|green|blue;
	  (*colors)[i].pad = 0;
	  red += red1;
	  if (red > win_info->visual->red_mask)
	    red = 0;
	  green += green1;
	  if (green > win_info->visual->green_mask)
	    green = 0;
	  blue += blue1;
	  if (blue > win_info->visual->blue_mask)
	    blue = 0;
	}
    } else {
	for (i=0; i<ncolors; i++) {
	  (*colors)[i].pixel = i;
	  (*colors)[i].pad = 0;
	}
    }

    XQueryColors(UxDisplay, WinCMap, *colors, ncolors);
    return(ncolors);
}

/* Beep: Routine to beep the display. */
static void Beep(void)
{
	XBell(UxDisplay, 50);
}


/* Routine to let user select a window using the mouse */
static Window Select_Window(void)
{
  int status;
  Cursor cursor;
  XEvent event;
  Window target_win = None, root = XDefaultRootWindow(UxDisplay);
  int buttons = 0;

  /* Make the target cursor */
  cursor = XCreateFontCursor(UxDisplay, XC_crosshair);

  /* Grab the pointer using target cursor, letting it room all over */
  status = XGrabPointer(UxDisplay, root, B_FALSE,
			ButtonPressMask|ButtonReleaseMask, GrabModeSync,
			GrabModeAsync, root, cursor, CurrentTime);
  if (status != GrabSuccess) {
    ErrorMsg("Can't grab the mouse in GrabWindow()");
    return(target_win);
  }

  /* Let the user select a window... */
  while ((target_win == None) || (buttons != 0)) {
    /* allow one more event */
    XAllowEvents(UxDisplay, SyncPointer, CurrentTime);
    XWindowEvent(UxDisplay, root, ButtonPressMask|ButtonReleaseMask, &event);
    switch (event.type) {
    case ButtonPress:
      if (target_win == None) {
	target_win = event.xbutton.subwindow; /* window selected */
	if (target_win == None)
	  target_win = root;
      }
      buttons++;
      break;
    case ButtonRelease:
      if (buttons > 0) /* there may have been some down before we started */
	buttons--;
       break;
    }
  } 

  XUngrabPointer(UxDisplay, CurrentTime);      /* Done with pointer */

  return(target_win);
}


/* GrabWindow: grab a window with the pointer */
boolean_t GrabWindow(double **pixels, int *heightp, int *widthp,
		   Colormap WinCMap, XColor *Cells, boolean_t verbose)
{
  Window window, dummywin, root;
  XWindowAttributes win_info;
  XImage *image;
  XColor *colors;
  double *dp;
  int ncolors, i, j, k, dummyi, dummy, absx, absy, x, y, dwidth,dheight;
  int table[65536];
  char s[256];

  /* free any allocated color cells if required */
  if (PagedCmap == B_FALSE) {
    XFreeColors(UxDisplay, WinCMap, LutCells, AllocatedCmapCells, 0xffffffff);
    AllocatedCmapCells = 0;
  }

  /* allow the user to select the window */
  if ((window = Select_Window()) != None) {
    if (XGetGeometry (UxDisplay, window, &root, &dummyi, &dummyi,
		      (unsigned int *) &dummy, (unsigned int *) &dummy,
		      (unsigned int *) &dummy, (unsigned int *) &dummy)
	&& window != root)
      window = XmuClientWindow (UxDisplay, window);
  }
  else {
    ErrorMsg("Could not select window in GrabWindow()");
    return(B_FALSE);
  }

  /* Inform the user not to alter the screen and raise the window */
  XRaiseWindow(UxDisplay, window);
  XSync(UxDisplay, B_FALSE);
  Beep();
  
  /* Get the parameters of the window being dumped */
  if(!XGetWindowAttributes(UxDisplay, window, &win_info)) {
    ErrorMsg("Can't get target window attributes in GrabWindow()");
    return(B_FALSE);
  }
  
  /* handle any frame window */
  if (!XTranslateCoordinates (UxDisplay, window,
			      XDefaultRootWindow (UxDisplay), 0, 0,
			      &absx, &absy, &dummywin)) {
    sprintf(s, "GrabWindow():  unable to translate window coords (%d,%d)\n",
	    absx, absy);
    ErrorMsg(s);
    return(B_FALSE);
  }
  win_info.x = absx;
  win_info.y = absy;

  /* clip to window */
  dwidth = XDisplayWidth(UxDisplay, UxScreen);
  dheight = XDisplayHeight(UxDisplay, UxScreen);
  if (absx < 0) win_info.width += absx, absx = 0;
  if (absy < 0) win_info.height += absy, absy = 0;
  if (absx + win_info.width > dwidth) win_info.width = dwidth - absx;
  if (absy + win_info.height > dheight) win_info.height = dheight - absy;
  
  /* Snarf the pixmap with XGetImage */
  x = absx - win_info.x;
  y = absy - win_info.y;
  image = XGetImage (UxDisplay, window, x, y, win_info.width, win_info.height,
		     AllPlanes, ZPixmap);
  if (!image) {
    sprintf (s, "GrabWindow:  unable to get image at %dx%d+%d+%d\n",
	     win_info.width, win_info.height, x, y);
    ErrorMsg(s);
    return(B_FALSE);
  }
  
  /* Get image colors, check for GL type inconsistencies */
  if (((ncolors = Get_XColors(&win_info, &colors)) > 256)
      && (image->depth == 8)) {
    sprintf(s, "GrabWindow() : %d colors in the window Visual structure but only 8 bits/pixel in the image!", ncolors);
    ErrorMsg(s);
    return(B_FALSE);
  }
  
  /* Inform the user that the image has been retrieved */
  XBell(UxDisplay, FEEP_VOLUME);
  XBell(UxDisplay, FEEP_VOLUME);
  XFlush(UxDisplay);
  
  /* allocate storage for the image if required */
  HGCursor(B_TRUE);
  if ((*widthp != win_info.width) || (*heightp != win_info.height)) {
    *widthp = win_info.width;
    *heightp = win_info.height;
    if ((*pixels = (double *) XvgMM_Alloc((void *) *pixels,
					  (*heightp) * (*widthp)
					  * sizeof(double))) == NULL) {
      sprintf(s, "Could not allocate storage in GrabWindow()");
      ErrorMsg(s);
      return(B_FALSE);
    }
  }
  
  /* convert data to double precision and correct lut indicies */
  if ((win_info.depth == 8) || (win_info.depth == 16)) {
    if (win_info.depth == 8) 
      for (i = 0; i < 256; i++)
	table[i] = -1;
    else
      for (i = 0; i < 65536; i++)
	table[i] = -1;

    for (y = 0, k = 0, dp = *pixels; y < image->height; y++)
      for (x = 0; x < image->width; x++) {
	/* if the color is new, load it into the Cells array */
	if (table[image->data[y*image->bytes_per_line + x]] == -1) {
	  if ((k > NPAGEDLUTCELLS) && (PagedCmap)) {
	    ErrorMsg("Maximum number of colors exceeded in GrabWindow()");
	    return(B_FALSE);
	  }
	  for (j = 0; (j < win_info.visual->map_entries)
	       && (image->data[y*image->bytes_per_line + x]
		   != colors[j].pixel); j++);
	  Cells[k].red = colors[j].red;
	  Cells[k].green = colors[j].green;
	  Cells[k].blue = colors[j].blue;
	  if (PagedCmap == B_FALSE) {
	    XAllocColor(UxDisplay, WinCMap, &(Cells[k]));
	    LutCells[AllocatedCmapCells++] = Cells[k].pixel;
	  }
	  table[image->data[y*image->bytes_per_line + x]] = Cells[k++].pixel;
	}
	*dp++ = table[image->data[y*image->bytes_per_line + x]];
      }
  }
  else {
    for (i = 0; i < ncolors; i++)
      colors[i].flags = 0;
    
    for (y = 0, k = 0, dp = *pixels; y < image->height; y++)
      for (x = 0; x < image->width; x++) {
	/* find correct color cell */
	for (j = 0; (j < win_info.visual->map_entries)
	     && (image->data[y*image->bytes_per_line + x] != colors[j].pixel);
	     j++);
	/* if the color is new, load it into the Cells array */
	if (colors[j].flags == 0) {
	  if ((k > NPAGEDLUTCELLS)  && (PagedCmap)) {
	    ErrorMsg("Maximum number of colors exceeded in GrabWindow()");
	    return(B_FALSE);
	  }
	  Cells[k].red = colors[j].red;
	  Cells[k].green = colors[j].green;
	  Cells[k].blue = colors[j].blue;
	  if (PagedCmap == B_FALSE) {
	    XAllocColor(UxDisplay, WinCMap, &(Cells[k]));
	    LutCells[AllocatedCmapCells++] = Cells[k].pixel;
	    colors[j].flags = Cells[k].pixel;
	    k++;
	  }
	  else
	    colors[j].flags = k++;
	}
	*dp++ = Cells[colors[j].flags].pixel;
      }
  }

  /* free storage */
  if(ncolors > 0)
    UxFree(colors);
  XDestroyImage(image);

  /* return success */
  HGCursor(B_FALSE);
  return(B_TRUE);
}





