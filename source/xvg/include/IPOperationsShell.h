/***********************************************************************
*
*  Name:          IPOperationsShell.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef IPOperationsShell_h
#define IPOperationsShell_h
extern void AddNewOpAfterCallback(void);
extern swidget create_IPOperationsShell(void);
extern void draw_mesg(char *p);
extern void ExecuteIPOpList(void);
extern void    LoadCosImgMenu(void);
extern void    LoadSinImgMenu(void);
extern boolean_t MallocScratchDouble(void);
extern void InitDoubleBuffering();
extern double *GetCurrentOutputIPBuffer();
extern void RegisterIPOp(char *OpName, void (*f)(), int IPOpSubmenuID, 
			 boolean_t RepsAllowed, boolean_t EdgeFixFlag,
			 ...);
#endif
