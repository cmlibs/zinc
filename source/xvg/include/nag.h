/***********************************************************************
*
*  Name:          nag_externs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       Nag function call prototypes
*                     c06fuf()
*                     c06gcf()
*                     e02aef()
*                     e02caf()
*                     e02cbf()
*                     e02daf()
*                     e02dcf()
*                     e02ddf()
*                     e02dff()
*                     e04gef()
*                     e04ucf()
*                     e04udm()
*                     f04amf()
*                     f04jaf()
*                     f04atf()
*                     f04faf()
*                     lsfun2()
*
***********************************************************************/
#ifndef nag_externs_h
#define nag_externs_h
extern void c06fuf(int *, int *,
		   double *, double *,
		   char *, double *, double *,
		   double *, int *);
extern void c06gcf(double *, int *, int *);
extern void e02aef(int *, double *, double *, double *,
		   int *);
extern void e02caf(int *, int *, int *K, int *L, double *X1, double *Y1,
		   double *, double *, int *, double *, int *,
		   double *, double *, double *, int *,
		   double *, int *, double *, int *,
		   int *);
extern void e02cbf(int *, int *, int *, int *,
		   double *, double *, double *,
		   double *, double *, double *,
		   double *, double *, int *,
		   double *, int *, int *);
extern void e02daf(int *, int *, int *,
		   double *, double *, double*, double *,
		   double *, double *,
		   int *, int *, double *, double *,
		   int *, double *, int *,
		   double *, double *, int *, int *);
extern void e02dcf(char *, int *, double *, int *, double *,
		   double *, double *, int *, int *, 
		   int *, double *, int *,
		   double *, double *,
		   double *, double *, int *, int *, int *,
		   int *);
extern void e02ddf(char *, int *, double *, double *,
		   double *, double *, double *, int *, int *, 
		   int *, double *,
		   int *, double *, double *,
		   double *, int *, double *, int *,
		   int *, int *, int *);
extern void e02dff(int *, int *, int *, int *,
		   double *, double *, double *, double *,
		   double *, double *,
		   double *, int *, int *, int *, int *);
extern void e02zaf(int *, int *,
		   double *, double *, int *,
		   double *, double*,
		   int *, int *, int *, int *,
		   int *);
extern void e04bbf(void (*CrossCorrObjFuncEval_f),
		   double *, double *, double *, double *, int *,
		   double *, double *, double *, int *);
extern void e04gef(int *, int *, double *, double *,
		   int *, int *, double *, int *, int *);
extern void e04ucf(int *, int *, int *,
		   int *ONE1, int *ONE2, int *N2, double *NULLPTR,
		   double *BL, double *BU, void (*e04udm_f), void (*OBJFUN_f),
		   int *ITER, int *ISTATE, double *C, double *CJAC,
		   double *CLAMBDA, double *F, double *G, double *R,
		   double *X, int *IWORK, int *LIWORK,
		   double *WORK, int *LWORK, int *IUSER, double *USER,
		   int *);
extern void e04udm(void);
extern void e04uef(char  *);
extern void f04amf(double *, int *, double *, int *, double *,
		   int *, int *, int *, int *, double *,
		   double *, int *, double *, double *,
		   double *, double *, double *, int *, int *);
extern void f04jaf(int *, int *, double *, int *, double *, 
		   double *, double *, int *,
		   double *, int *, int *);
extern void f04atf(double *, int *, double *, int *, double *,
		   double *, int *, double *, double *,
		   int *);
extern void f04faf(int *, int *, double *, double *, double *,
		   int *);
extern void lsfun2(int *, int *, double *, double *, double *,
		   int *);
#endif



