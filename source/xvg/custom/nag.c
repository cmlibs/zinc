/***********************************************************************
*
*  Name:          nag.c
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       Nag dummy function calls
*                     c06fuf()
*                     c06gcf()
*                     e02aef()
*                     e02caf()
*                     e02cbf()
*                     e02daf()
*                     e02dcf()
*                     e02ddf()
*                     e02dff()
*                     e04bbf()
*                     e04gef()
*                     e04ucf()
*                     e04udm()
*                     e04uef()
*                     f04amf()
*                     f04jaf()
*                     f04atf()
*                     f04faf()
*                     lsfun2()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include "XvgGlobals.h"

void c06fuf(int *width, int *height,
	    double *FFTRealBuffer, double *FFTImBuffer,
	    char *startupflag, double *FFTTrigm, double *FFTTrign,
	    double *FFTWork, int *IFAIL)
{
  printf("Called c06fuf()\n");
}

void c06gcf(double *FFTImBuffer, int *n, int *IFAIL)
{
  printf("Called c06gcf()\n");
}

void e02aef(int *OPLUS1, double *A, double *XCAP, double *P,
	    int *IFAIL)
{
  printf("Called e02aef()\n");
}

void e02caf(int *M, int *N, int *K, int *L, double *X1, double *Y1,
	    double *F, double *W, int *MTOT, double *A, int *NA,
	    double *XMIN, double *XMAX, double *X2, int *INUXP1,
	    double *X3, int *INUYP1, double *WORK, int *NWORK,
	    int *IFAIL)
{
  printf("Called e02caf()\n");
}

void e02cbf(int *MFIRST, int *MLAST, int *K, int *L,
	    double *X, double *XMIN, double *XMAX,
	    double *Y, double *YMIN, double *YMAX,
	    double *FF, double *A, int *NA,
	    double *WORK, int *NWORK, int *IFAIL)
{
  printf("Called e02cbf()\n");
}

void e02daf(int *M, int *BSplineNKj, int *BSplineNKi,
	    double *Y, double *X, double* F, double *W,
	    double *BSplineKj, double *BSplineKi,
	    int *POINT, int *NPOINT, double *DL, double *BSplineCij,
	    int *NC, double *WS, int *NWS,
	    double *EPS, double *SIGMA, int *RANK, int *IFAIL)
{
  printf("Called e02daf()\n");
}

void e02dcf(char *startflag, int *MY, double *Y, int *MX, double *X,
	    double *inpixels, double *S, int *NYEST, int *NXEST, 
	    int *BSplineNKj, double *BSplineKj, int *BSplineNKi,
	    double *BSplineKi, double *BSplineCij,
	    double *FP, double *WRK, int *LWRK, int *IWRK, int *LIWRK,
	    int *IFAIL)
{
  printf("Called e02dcf()\n");
}

void e02ddf(char *startflag, int *k, double *YC, double *XC,
	    double *F, double *W, double *S, int *NYEST, int *NXEST, 
	    int *BSplineNKj, double *BSplineKj,
	    int *BSplineNKi, double *BSplineKi, double *BSplineCij,
	    double *FP, int *RANK, double *WRK, int *LWRK,
	    int *IWRK, int *LIWRK, int *IFAIL)
{
  printf("Called e02ddf()\n");
}

void e02dff(int *MY, int *MX, int *BSplineNKj, int *BSplineNKi,
	    double *Y, double *X, double *BSplineKj, double *BSplineKi,
	    double *BSplineCij, double *outpixels,
	    double *WRK, int *LWRK, int *IWRK, int *LIWRK, int *IFAIL)
{
  printf("Called e02dff()\n");
}

void e02zaf(int *BSplineNKj, int *BSplineNKi,
	    double *BSplineKj, double *BSplineKi, int *M,
	    double *Y, double* X,
	    int *POINT, int *NPOINT, int *ADRES, int *NADRES,
	    int *IFAIL)
{
  printf("Called e02zaf()\n");
}

void e04bbf(void (*CrossCorrObjFuncEval_f),
	    double *E1, double *E2, double *LB, double *UB, int *MAXCAL,
	    double *root, double *f, double *df, int *IFAIL)
{
  printf("Called e04bbf()\n");
}

void e04gef(int *M, int *N, double *X, double *FSUMSQ,
	    int *IW, int *LIW, double *W, int *LW, int *IFAIL)
{
  printf("Called e04gef()\n");
}

void e04ucf(int *N1, int *ZERO1, int *ZERO2,
	    int *ONE1, int *ONE2, int *N2, double *NULLPTR,
	    double *BL, double *BU, void (*e04udm_f), void (*OBJFUN_f),
	    int *ITER, int *ISTATE, double *C, double *CJAC,
	    double *CLAMBDA, double *F, double *G, double *R,
	    double *X, int *IWORK, int *LIWORK,
	    double *WORK, int *LWORK, int *IUSER, double *USER,
	    int *IFAIL)
{
  printf("Called e04ucf()\n");
}

void e04udm(void)
{
  printf("Called e04udm()\n");
}

void e04uef(char  *s)
{
  printf("Called e04uef()\n");
}


void f04amf(double *AX, int *N1, double *XSOLN, int *N2, double *BX,
	    int *N3, int *N4, int *N5, int *IR, double *EPS,
	    double *QR, int *N6, double *ALPHA, double *E,
	    double *YY, double *Z, double *R, int *IPIV, int *IFAIL)
{
  printf("Called f04amf()\n");
}

void f04jaf(int *M1, int *N, double *A, int *M2, double *B, 
	    double *tolerance, double *sigma, int *rank,
	    double *work, int *lwork, int *IFAIL)
{
  printf("Called f04jaf()\n");
}

void f04atf(double *A, int *N1, double *B, int *N, double *C,
	    double *AA, int *N2, double *WKS1, double *WKS2,
	    int *IFAIL)
{
  printf("Called f04atf()\n");
}

void f04faf(int *JOB, int *N, double *D, double *E, double *PX,
	    int *IFAIL)
{
  printf("Called f04faf()\n");
}
