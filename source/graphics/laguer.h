/*******************************************************************************
FILE : laguer.h

LAST MODIFIED : 24 November 1994

DESCRIPTION :
==============================================================================*/
#if !defined (LAGUER_H)
#define LAGUER_H
void laguer(fcomplex a[], int m, fcomplex *x, int *its);
void zroots(fcomplex a[], int m, fcomplex roots[], int polish);
#endif
