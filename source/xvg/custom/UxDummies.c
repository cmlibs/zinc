/****************************************************************************
*
*    File:      UxDummies.c
*    Author:    Paul Charette
*    Modified:  29 December 1995
*
*    Routines:
*           UxCalloc()
*           UxFree()
*           UxGetDirectory()
*           UxGetHeight()
*           UxGetItemCount()
*           UxGetLabelString()
*           UxGetMaximum()
*           UxGetMinimum()
*           UxGetSet()
*           UxGetText()
*           UxGetValue()
*           UxGetWidth()
*           UxDelayUpdate()
*           UxMap()
*           UxMalloc()
*           UxPutBottomOffset()
*           UxPutLabelString()
*           UxPutHeight()
*           UxPutMaxHeight()
*           UxPutMaximum()
*           UxPutMaxWidth()
*           UxPutMessageString()
*           UxPutPattern()
*           UxPutRightOffset()
*           UxPutSelectionLabelString()
*           UxPutSet()
*           UxPutText()
*           UxPutTextString()
*           UxPutTitle()
*           UxPutTitleString()
*           UxPutValue()
*           UxPutWidth()
*           UxPutX()
*           UxRealloc()
*           UxUnmap()
*           UxUpdate()
*
*???DB.  Is XmStringCreateLocalized in Motif 1.2 ?
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "UxXt.h"
#include "XvgGlobals.h"

void *UxRealloc(void *ptr, size_t size)
{
  ptr = (void *) XvgMM_Alloc((void *) ptr, size);
  return(ptr);
}

void *UxCalloc(size_t NumberOfElements, size_t ElementSize)
{
  void *ptr;

  printf("UxCALLOC(%d, %d) USED.\n", NumberOfElements, ElementSize);

  ptr = calloc(NumberOfElements, ElementSize);
  if (ptr == NULL) {
    sprintf(cbuf, "Could not UxCalloc(%d,%d)", NumberOfElements, ElementSize);
    ErrorMsg(cbuf);
  }
  return(ptr);
}

void *UxMalloc(size_t size)
{
  void *ptr;

  printf("UxMALLOC(%d) USED.\n", size);

  ptr = malloc(size);
  if (ptr == NULL) {
    sprintf(cbuf, "Could not UxMalloc(%d)", size);
    ErrorMsg(cbuf);
  }
  return(ptr);
}

void UxPutLabelString(Widget sw, char *s) /* works */
{
  XtVaSetValues( sw, XmNlabelString,
		XmStringCreateSimple(s), NULL);
#ifdef DEBUG
  printf("UxPutLabelString(%s)\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutText(Widget sw, char *s) /* works */
{
  XtVaSetValues( sw, XmNvalue, s, NULL);
#ifdef DEBUG
  printf("UxPutText(%s)\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutMessageString(Widget sw, char *s) /* works */
{
  XtVaSetValues( sw, XmNmessageString,
	XmStringCreateSimple(s), NULL);
#ifdef DEBUG
  printf("UxPutMessageString(%s)\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutHeight(Widget sw, int d) /* works */
{
  XtVaSetValues( sw, XmNheight, (Dimension) d, NULL);
#ifdef DEBUG
  printf("UxPutHeight(%d)\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutWidth(Widget sw, int d)  /* works */
{
  XtVaSetValues( sw, XmNwidth, (Dimension) d, NULL);
#ifdef DEBUG
  printf("UxPutWidth(%d)\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutMaxHeight(Widget sw, int d) /* works */
{
  XtVaSetValues( sw, XmNmaxHeight, d, NULL);
#ifdef DEBUG
  printf("UxPutMaxHeight(%d)\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutMaxWidth(Widget sw, int d) /* works */
{
  XtVaSetValues( sw, XmNmaxWidth, d, NULL);
#ifdef DEBUG
  printf("UxPutMaxWidth(%d)\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutTitle(Widget sw, char *s) /* works */
{
  XtVaSetValues( sw, XmNtitle, s, NULL);
#ifdef DEBUG
  printf("UxPutTitle(%s)\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutX(Widget sw, int d) /* works */
{
  XtVaSetValues( sw, XmNx, (Position) d, NULL);
#ifdef DEBUG
  printf("UxPutX(%d)\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutSelectionLabelString(Widget sw, char *s) /* works */
{
  XtVaSetValues( sw, XmNselectionLabelString,
	XmStringCreateSimple(s), NULL);
#ifdef DEBUG
  printf("UxPutSelectionLabelString(%s)\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutPattern(Widget sw, char *s) /* works */
{
  XtVaSetValues( sw, XmNpattern,
		XmStringCreateSimple(s), NULL);
#ifdef DEBUG
  printf("UxPutPattern(%s)\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutTextString(Widget sw, char *s) /* works */
{ 
  XtVaSetValues( sw, XmNtextString,
		XmStringCreateSimple(s), NULL);
#ifdef DEBUG
  printf("UxPutTextString(%s)\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutTitleString(Widget sw, char *s) /* works */
{
  XtVaSetValues( sw, XmNtitleString,
		XmStringCreateSimple(s), NULL);
#ifdef DEBUG
  printf("UxPutTitleString(%s)\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutMaximum(Widget sw, int d)
{
  XtVaSetValues( sw, XmNmaximum, d, NULL);
#ifdef DEBUG
  printf("UxPutMaximum(%d)\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutValue(Widget sw, int d) /* works */
{
  XtVaSetValues( sw, XmNvalue, d, NULL);
#ifdef DEBUG
  printf("UxPutValue(%d)\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutBottomOffset(Widget sw, int d)
{
  XtVaSetValues( sw, XmNbottomOffset, d, NULL);
#ifdef DEBUG
  printf("UxPutBottomOffset(%d)\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutRightOffset(Widget sw, int d)
{
  XtVaSetValues( sw, XmNrightOffset, d, NULL);
#ifdef DEBUG
  printf("UxPutRightOffset(%d)\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxPutSet(Widget sw, char *s) /* works */
{
  if (strcmp(s, "true") == 0)
    XtVaSetValues( sw, XmNset, (Boolean) 1, NULL);
  else
    XtVaSetValues( sw, XmNset, (Boolean) 0, NULL);
#ifdef DEBUG
  printf("UxPutSet(%s)\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

char *UxGetSet(Widget sw) /* works */
{
  Boolean v;

  XtVaGetValues( sw, XmNset, &v, NULL);
#ifdef DEBUG
  printf("UxGetSet() = %d\n", v);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
  if (v == 0)
    return("false");
  else
    return("true");
}

int UxGetWidth(Widget sw) /* works */
{
  Dimension d;
 
  XtVaGetValues( sw, XmNwidth, &d, NULL);
#ifdef DEBUG
  printf("UxGetWidth() = %d\n", (int) d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
  return((int) d);
}

int UxGetValue(Widget sw) /* works */
{
  int d;

  XtVaGetValues( sw, XmNvalue, &d, NULL);
#ifdef DEBUG
  printf("UxGetValue() = %d\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
  return(d);
}

int UxGetHeight(Widget sw) /* works */
{
  Dimension d;

  XtVaGetValues( sw, XmNheight, &d, NULL);
#ifdef DEBUG
  printf("UxGetHeight() = %d\n", (int) d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
  return((int) d);
}

int UxGetItemCount(Widget sw)
{
  int d;

  XtVaGetValues( sw, XmNitemCount, &d, NULL);
#ifdef DEBUG
  printf("UxGetItemCount() = %d\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
  return(d);
}

char *UxGetText(Widget sw) /* works */
{
  char *s;
  
  XtVaGetValues( sw, XmNvalue, &s, NULL);
#ifdef DEBUG
  printf("UxGetText() = %s\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
  return(s);
}

char *UxGetLabelString(Widget sw)
{
  XmString xs;
  char *s;
  
  XtVaGetValues( sw, XmNlabelString, &xs, NULL);
  XmStringGetLtoR(xs, XmSTRING_DEFAULT_CHARSET, &s);
  XmStringFree(xs);
#ifdef DEBUG
  printf("UxGetLabelString() = %s\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
  return(s);
}

int UxGetMaximum(Widget sw) /* works */
{
  int d;

  XtVaGetValues( sw, XmNmaximum, &d, NULL);
#ifdef DEBUG
  printf("UxGetMaximum() = %d\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
  return(d);
}

int UxGetMinimum(Widget sw) /* works */
{
  int d;

  XtVaGetValues( sw, XmNminimum, &d, NULL);
#ifdef DEBUG
  printf("UxGetMinimum() = %d\n", d);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
  return(d);
}

char *UxGetDirectory(Widget sw)
{
  XmString xs;
  char *s;
  
  XtVaGetValues( sw, XmNdirectory, &xs, NULL);
  XmStringGetLtoR(xs, XmSTRING_DEFAULT_CHARSET, &s);
  XmStringFree(xs);
#ifdef DEBUG
  printf("UxGetDirectory() : %s\n", s);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
  return(s);
}

void UxFree(void *p)
{
  XvgMM_Free(p);
#ifdef DEBUG
  printf("UxFree()\n");
#endif
}

void UxMap(Widget sw) /* works */
{
  XtMapWidget( sw);
#ifdef DEBUG
  printf("UxMap(%d)\n", sw);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxUnmap(Widget sw) /* works */
{
  if (XtIsRealized( sw))
    XtUnmapWidget( sw);
#ifdef DEBUG
  printf("UxUnmap(%d)\n", sw);
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
#endif
}

void UxDelayUpdate(Widget sw)
{
#ifdef DEBUG
  printf("UxDelayUpdate()\n");
#endif
}
void UxUpdate(Widget sw)
{
#ifdef DEBUG
  printf("UxUpdate()\n");
#endif
#ifdef XSYNCS
  XSync(UxDisplay, B_FALSE);
#endif
}



