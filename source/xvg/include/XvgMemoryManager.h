/***********************************************************************
*
*  Name:          XvgMemoryManager.h
*
*  Author:        Paul Charette
*
*  Last Modified: 3 March 1997
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef XvgMemoryManager_h
#define XvgMemoryManager_h
extern void    XvgMM_Free(void *ptr);
extern void ShowXvgMMStats(double *inpixels, double *outpixels,
			   int height, int width, void *ParmPtrs[]);
extern void     XvgMM_FreeAllBlocks(void);
extern int      XvgMM_GetBlockSize(void *ptr);
extern int      XvgMM_GetMemSize(void);
extern void     XvgMM_Init(void);
extern void    *XvgMM_Alloc(void *ptr, int nbytes);
extern void     XvgMM_MemoryCheck(char *msg);
#endif
