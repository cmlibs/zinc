/***********************************************************************
*
*  Name:          matlab.c
*
*  Author:        Paul Charette
*
*  Last Modified: 12 August 1994
*
*  Purpose:       Dummy MATLAB declarations
*
*                 WARNING: Accessing the "VerboseFlag" here makes
*                 an SGI machine crash (type incompatibilites
*                 between int and boolean_t?)!!!!!
*
***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matlab.h"

/* include XvgGlobals.h here clashes with CMGUI */
extern int      XvgMM_Free(void *ptr);
extern void    *XvgMM_Alloc(void *ptr, int nbytes);
extern void     ErrorMsg(char *);

Matrix	*mxCreateFull(int m, int n, int	cmplx_flg)
{
  Matrix *mm;
  char s[256];

  if ((mm = (Matrix *) XvgMM_Alloc(NULL, sizeof(Matrix))) == NULL) {
    sprintf(s, "mxCreateFull(): Could not allocate Matrix");
    ErrorMsg(s);
    return(NULL);
  }
  mm->m = m;
  mm->n = n;
  if ((mm->pr = (double *) XvgMM_Alloc(NULL, n*m*sizeof(double)))
      == NULL) {
    sprintf(s, "mxCreateFull(): Could not allocate %d byte Real buffer",
	    m*n*sizeof(double));
    ErrorMsg(s);
    XvgMM_Free(mm);
    return(NULL);
  }
  if (cmplx_flg == COMPLEX) {
    ErrorMsg("mxCreateFull() : Suspicious complex data in matrix");
    if ((mm->pi = (double *) XvgMM_Alloc(NULL, n*m*sizeof(double)))
	== NULL) {
      sprintf(s, "mxCreateFull(): Could not allocate %d byte Im buffer",
	      m*n*sizeof(double));
      ErrorMsg(s);
      XvgMM_Free(mm->pr);
      XvgMM_Free(mm);
      return(NULL);
    }
  }
  else
    mm->pi = NULL;
#ifdef DEBUG
  printf("mxCreateFull(%d x %d, flag:%d)\n", m, n, cmplx_flg);
#endif
  return(mm);
}

void mxFreeMatrix(Matrix *pmat)
{
  if (pmat != NULL) {
    XvgMM_Free(pmat->pr);
    XvgMM_Free(pmat->pi);
  }
  XvgMM_Free(pmat);
#ifdef DEBUG
  printf("mxFreeMatrix()\n");
#endif
}

char *mxGetName(const Matrix *pm)
{
#ifdef DEBUG
  printf("** MATLAB: mxGetName()=\"%s\"\n", pm->name);
#endif
  return((char *) (pm->name));
}

void mxSetName(Matrix *pm, const char *s)
{
  char *b, ss[256];

  if ((b = (char *) XvgMM_Alloc(NULL, strlen(s) + 1)) == NULL) {
    sprintf(ss, "mxSetName(): Could not allocate %d bytes", strlen(s));
    ErrorMsg(ss);
    return;
  }
  sprintf(b, "%s", s);
  if (strlen(s) >= mxMAXNAM)
    b[mxMAXNAM-1] = '\0';
  sprintf(pm->name, "%s", b);
#ifdef DEBUG
  printf("mxSetName(%s)\n", pm->name);
#endif
}

int mxGetM(const Matrix *pm)
{
#ifdef DEBUG
  printf("** MATLAB: mxGetM()=%d\n", pm->m);
#endif
  return(pm->m);
}

void mxSetM(Matrix *pm, int m)
{
#ifdef DEBUG
  printf("** MATLAB: mxSetM(%d)\n", m);
#endif
  pm->m = m;
}

int mxGetN(const Matrix *pm)
{
#ifdef DEBUG
  printf("** MATLAB: mxGetN()=%d\n", pm->n);
#endif
  return(pm->n);
}

void mxSetN(Matrix *pm, int n)
{
#ifdef DEBUG
  printf("** MATLAB: mxSetN(%d)\n", n);
#endif
  pm->n = n;
}

Real *mxGetPr(const Matrix *pm)
{
#ifdef DEBUG
  printf("mxGetPr()\n");
#endif
  return(pm->pr);
}

void mxSetPr(Matrix *pm, Real *pr)
{
#ifdef DEBUG
  printf("** MATLAB: mxSetPr()\n");
#endif
  pm->pr = pr;
}

Real *mxGetPi(const Matrix *pm) 
{
#ifdef DEBUG
  printf("** MATLAB: mxGetPi()\n");
#endif
  return(pm->pi);
}

void mxSetPi(Matrix *pm, Real *pi)
{
#ifdef DEBUG
  printf("** MATLAB: mxSetPi()\n");
#endif
  pm->pi = pi;
}

int mxGetNzmax(const Matrix *pm)
{
  printf("** MATLAB(UNIMPLEMENTED): mxGetNzmax()\n");
  return(-1);
}

void mxSetNzmax(Matrix *pm, int nzmax)
{
  printf("** MATLAB(UNIMPLEMENTED): mxSetNzmax()\n");
}

int *mxGetIr(const Matrix *pm)
{
  printf("** MATLAB(UNIMPLEMENTED): mxGetIr()\n");
  return(NULL);
}

void mxSetIr(Matrix *pm, int	*ir)
{
  printf("** MATLAB(UNIMPLEMENTED): mxSetIr()\n");
}

int *mxGetJc(const Matrix *pm)
{
  printf("** MATLAB(UNIMPLEMENTED): mxGetJc()\n");
  return(NULL);
}

void mxSetJc(Matrix *pm, int *c)
{
  printf("** MATLAB(UNIMPLEMENTED): mxSetJc()\n");
}

int mxGetString(const Matrix *pm, char *str_ptr, int str_len)
{
  printf("** MATLAB(UNIMPLEMENTED): mxGetString()\n");
  return(-1);
}

Matrix *mxCreateString(const char *str_ptr)
{
  printf("** MATLAB(UNIMPLEMENTED): mxCreateString()\n");
  return(NULL);
}

Real mxGetScalar(const Matrix *pm)
{
  printf("** MATLAB(UNIMPLEMENTED): mxGetScalar()\n");
  return(-1);
}

int mxIsFull(const Matrix *pm)
{
  printf("** MATLAB(UNIMPLEMENTED): mxIsFull()\n");
  return(-1);
}

int mxIsSparse(const Matrix *pm)
{
  printf("** MATLAB(UNIMPLEMENTED): mxIsSparse()\n");
  return(-1);
}

int mxIsDouble(const Matrix *pm)
{
  printf("** MATLAB(UNIMPLEMENTED): mxIsDouble()\n");
  return(-1);
}

int mxIsString(const Matrix *pm)
{
  printf("** MATLAB(UNIMPLEMENTED): mxIsString()\n");
  return(-1);
}

int mxIsNumeric(const Matrix *pm)
{
  printf("** MATLAB(UNIMPLEMENTED): mxIsNumeric()\n");
  return(-1);
}

int mxIsComplex(const Matrix *pm)
{
  printf("** MATLAB(UNIMPLEMENTED): mxIsComplex()\n");
  return(-1);
}

Matrix *mxCreateSparse(int m, int n, int nzmax, int cmplx_flg)
{
  printf("** MATLAB(UNIMPLEMENTED): mxCreateSparse()\n");
  return(NULL);
}

MATFile *matOpen(char *filename, char *mode) /* works */
{
  return((MATFile *) fopen(filename, mode));
}

int matClose(MATFile *ph) /* works */
{
  fclose((FILE *) ph);
  return(0);
}

FILE *matGetFp(MATFile *ph) /* works */
{
  return((FILE *) ph);
}

char *NextMatrix(MATFile *ph, char *mname, int *mp, int *np,
		 double **pr, double **pi)
{
  extern int VerboseFlag;
  static char *lpr=NULL, *lpi=NULL;
  Fmatrix mh;
  double *dptr, *ddptr;
  float *fptr;
  int M, O, P, T, el, i, *iptr, rc;
  short *sptr;
  unsigned short *usptr;
  char s[512], *cptr, *name;

  /* return if EOF */
  if (fread(&mh, sizeof(Fmatrix), 1, matGetFp(ph)) != 1)
    return(NULL);
#ifdef DEBUG
  printf("NextMatrix() : variable name length : %d\n", mh.namlen);
#endif

  /* fetch matrix name */
  if ((name = (char *) XvgMM_Alloc(NULL, mh.namlen)) == NULL) {
    sprintf(s, "NextMatrix() : Could not allocate storage for name (%d bytes)",
	    mh.namlen);
    ErrorMsg(s);
    return(NULL);
  }
  if ((rc = fread(name, 1, mh.namlen, matGetFp(ph))) != mh.namlen) {
    sprintf(s, "NextMatrix() : Could not read the variable name (%d/%d bytes read)",
	    rc, mh.namlen);
    ErrorMsg(s);
    XvgMM_Free(name);
    return(NULL);
  }
#ifdef DEBUG
if (mname)
  printf("NextMatrix() : variable name : %s (looking for %s)\n", name, mname);
else
  printf("NextMatrix() : variable name : %s\n", name);
#endif
  
  /* parse the header */
  M = mh.type / 1000;
  O = (mh.type - M*1000) / 100;
  P = (mh.type - M*1000 - O*100) / 10;
  T = (mh.type - M*1000 - O*100 - P*10);

  if (VerboseFlag)
    printf("NextMatrix() = \"%s(%ld x %ld)\" M:%d,O:%d,P:%d,T:%d, imagf:%d\n",
	 name, mh.mrows, mh.ncols, M, O, P, T, mh.imagf);

  /* if the matrix is text or sparse, flag error */
  if (T != 0) {
    sprintf(s, "NextMatrix() : Matrix \"%s\" has an unsupported type (%d)\n",
	    name, T);
    ErrorMsg(s);
    XvgMM_Free(name);
    return(NULL);
  }

  /* determine the element storage size */
  switch (P) {
  case 0:
    el = 8;
    break;
  case 1:
  case 2:
    el = 4;
    break;
  case 3:
  case 4:
    el = 2;
    break;
  case 5:
  default:
    el = 1;
    break;
  }

  /* load matrix data if required */
  if (mname) {
    if (strcmp(name, mname) == 0) {
#ifdef DEBUG
      printf("NextMatrix() : Reading in real data from disk file\n");
#endif
      
      /* set matrix dimensions */
      *mp = mh.mrows;
      *np = mh.ncols;
      /* load in real data */
      if (((*pr = (double *) XvgMM_Alloc((void *) *pr,
					 mh.mrows * mh.ncols * sizeof(double)))
	   == NULL)
	  || ((lpr = (char *) XvgMM_Alloc((void *) lpr,
					  mh.mrows * mh.ncols * el))==NULL)) {
	sprintf(s, "NextMatrix(): Could not allocate buffs (%d) for real data",
		mh.mrows*mh.ncols);
	ErrorMsg(s);
	XvgMM_Free(name);
	return(NULL);
      }
      if (fread(lpr, el, mh.mrows*mh.ncols, matGetFp(ph))!= mh.mrows*mh.ncols){
	sprintf(s, "NextMatrix() : Could not read in real data (%d)",
		mh.mrows*mh.ncols);
	ErrorMsg(s);
	XvgMM_Free(name);
	return(NULL);
      }
      switch (P) {
      case 0:
	for (i = 0, ddptr = *pr, dptr = (double *) lpr;
	     i < mh.ncols*mh.mrows; i++)
	  *ddptr++ = *dptr++;
	break;
      case 1:
	for (i = 0, ddptr = *pr, fptr = (float *) lpr;
	     i < mh.ncols*mh.mrows; i++)
	  *ddptr++ = *fptr++;
	break;
      case 2:
	for (i = 0, ddptr = *pr, iptr = (int *) lpr;
	     i < mh.ncols*mh.mrows; i++)
	  *ddptr++ = *iptr++;
	break;
      case 3:
	for (i = 0, ddptr = *pr, sptr = (short *) lpr;
	     i < mh.ncols*mh.mrows; i++)
	  *ddptr++ = *sptr++;
	break;
      case 4:
	for (i = 0, ddptr = *pr, usptr = (unsigned short *) lpr;
	     i < mh.ncols*mh.mrows; i++)
	  *ddptr++ = *usptr++;
	break;
      case 5:
      default:
	for (i = 0, ddptr = *pr, cptr = lpr; i < mh.ncols*mh.mrows; i++)
	  *ddptr++ = *cptr++;
	break;
      }
      
      /* load in imaginary data */
      if ((mh.imagf) && (*pi)) {
	sprintf(s, "NextMatrix() : Matrix \"%s\" contains imaginary data");
	ErrorMsg(s);
#ifndef DISABLED
#ifdef DEBUG
	printf("NextMatrix() : Reading in imaginary data from disk file\n");
#endif
	if (((*pi = (double *) XvgMM_Alloc(*pi,
					   mh.mrows * mh.ncols * sizeof(double)))
	     == NULL)
	    || ((lpi = (char *) XvgMM_Alloc(lpi, mh.mrows * mh.ncols * el))
		== NULL)) {
	  sprintf(s, "NextMatrix() : Could not allocate bufs (%d) for Im data",
		  mh.mrows*mh.ncols);
	  ErrorMsg(s);
	  XvgMM_Free(name);
	  return(NULL);
	}
	if (fread(lpi,el,mh.mrows*mh.ncols,matGetFp(ph)) != mh.mrows*mh.ncols) {
	  sprintf(s, "NextMatrix() : Could not read in real data (%d)",
		  mh.mrows*mh.ncols);
	  ErrorMsg(s);
	  XvgMM_Free(name);
	  return(NULL);
	}
	switch (P) {
	case 0:
	  for (i = 0, ddptr = *pi, dptr = (double *) lpi;
	       i < mh.ncols*mh.mrows; i++)
	    *ddptr++ = *dptr++;
	  break;
	case 1:
	  for (i = 0, ddptr = *pi, fptr = (float *) lpi;
	       i < mh.ncols*mh.mrows; i++)
	    *ddptr++ = *fptr++;
	  break;
	case 2:
	  for (i = 0, ddptr = *pi, iptr = (int *) lpi;
	       i < mh.ncols*mh.mrows; i++)
	    *ddptr++ = *iptr++;
	  break;
	case 3:
	  for (i = 0, ddptr = *pi, sptr = (short *) lpi;
	       i < mh.ncols*mh.mrows; i++)
	    *ddptr++ = *sptr++;
	  break;
	case 4:
	  for (i = 0, ddptr = *pi, usptr = (unsigned short *) lpi;
	       i < mh.ncols*mh.mrows; i++)
	    *ddptr++ = *usptr++;
	  break;
	case 5:
	default:
	  for (i = 0, ddptr = *pi, cptr = lpi; i < mh.ncols*mh.mrows; i++)
	    *ddptr++ = *cptr++;
	  break;
	}
#endif /* DISABLED */
      }
    }
    /* else just advance the file pointer */
    else {
#ifdef DEBUG
      printf("NextMatrix() : Advancing file pointer\n");
#endif
      fseek(matGetFp(ph), el*mh.mrows*mh.ncols, 1);
      if (mh.imagf)
	fseek(matGetFp(ph), el*mh.mrows*mh.ncols, 1);
    }
  }
  
  /* else just advance the file pointer */
  else {
#ifdef DEBUG
    printf("NextMatrix() : Advancing file pointer\n");
#endif
    fseek(matGetFp(ph), el*mh.mrows*mh.ncols, 1);
    if (mh.imagf)
      fseek(matGetFp(ph), el*mh.mrows*mh.ncols, 1);
  }
  
  /* return matrix name */
#ifdef DEBUG
  printf("NextMatrix() : Returning with \"%s\"\n", name);
#endif
  return(name);
}

char **matGetDir(MATFile *ph, int *num)
{
  Fmatrix mh;
  int i;
  char **dir, s[256], *n;

  /* loop to create directory list */
  fseek(matGetFp(ph), 0, 0);
  for (*num = 0; (n = NextMatrix(ph, NULL, NULL, NULL, NULL, NULL)) != NULL;
       (*num)++, XvgMM_Free(n));
  if ((dir = (char **) XvgMM_Alloc(NULL, *num * sizeof(char **)))
      == NULL) {
    sprintf(s, "matGetDir() : Could not alloc space for directory (%d bytes)",
	    *num * sizeof(char **));
    ErrorMsg(s);
    return(NULL);
  }
#ifdef DEBUG
    printf("matGetDir() %d directory entries\n", *num);
#endif
  for (i = 0, fseek(matGetFp(ph), 0, 0); i < *num; i++) {
    if ((n = NextMatrix(ph, NULL, NULL, NULL, NULL, NULL)) == NULL) {
      sprintf(s,"matGetDir() : Internal error - Null directory entry\n");
      ErrorMsg(s);
      XvgMM_Free(n);
      return(NULL);
    }
#ifdef DEBUG
    printf("matGetDir() directory entry %d : %s\n", i, n);
#endif
    dir[i] = NULL;
    if ((dir[i] = (char *) XvgMM_Alloc(NULL, strlen(n)+1))
	== NULL) {
      sprintf(s,"matGetDir() : Could not alloc space for dir entry (%d bytes)",
	      strlen(n));
      ErrorMsg(s);
      XvgMM_Free(n);
      return(NULL);
    }
#ifdef DEBUG
    printf("matGetDir() directory entry %d : %s\n", i, n);
#endif
    sprintf(dir[i], "%s", n);
#ifdef DEBUG
    printf("matGetDir() directory entry %d : %s (%s)\n", i, dir[i], n);
#endif
    XvgMM_Free(n);
  }
  
  /* reset file pointer and return */
  fseek(matGetFp(ph), 0, 0);
  return(dir);
}

void matFreeDir(char **dir, int nentries)
{
  int i;
  
  /* loop to delete directory entries */
  if (dir) {
    for (i = 0; i < nentries; i++)
      XvgMM_Free(dir[i]);
    XvgMM_Free(dir);
  }
}

int matDeleteMatrix(MATFile *ph, char *name)
{
  printf("** MATLAB(UNIMPLEMENTED): matDeleteMatrix()\n");
  return(-1);
}

Matrix *matGetMatrix(MATFile *ph, char *name)
{
  printf("** MATLAB: matGetMatrix()\n");
  return(NULL);
}

/* warning:: If matrix already exists in file, it is not replaced! */
int matPutMatrix(MATFile *ph, Matrix *mp)
{
  Fmatrix m;
  char b[512], *mname;

  /* loop to see if matrix exists in this file */
  for (fseek(matGetFp(ph), 0, 0);
       (mname = NextMatrix(ph, NULL, NULL, NULL, NULL, NULL)) != NULL;
       XvgMM_Free(mname))
    if (strcmp(mp->name, mname) == 0) {
      sprintf(b, "matPutMatrix() : Matrix \"%s\" already exists in output file", mp->name);
      ErrorMsg(b);
      return(0);
    }
  
  /* go to EOF */
  fseek(matGetFp(ph), 0, 2);

  /* write header */
  m.type = 1000;
  m.mrows = mp->m;
  m.ncols = mp->n;
  m.imagf = (mp->pi ? 1 : 0);
  m.namlen = strlen(mp->name) + 1;
  fwrite(&m, sizeof(Fmatrix), 1, matGetFp(ph));

  /* write name */
  fwrite(mp->name, 1, strlen(mp->name) + 1, matGetFp(ph));

  /* write real data */
  fwrite(mp->pr, sizeof(double), mp->m * mp->n, matGetFp(ph));

  /* write imaginary data */
  if (mp->pi != NULL)
    fwrite(mp->pi, sizeof(double), mp->m * mp->n, matGetFp(ph));

#ifdef DEBUG
  printf("matPutMatrix(%s : %dx%d)\n", mp->name, mp->m, mp->n);
#endif
  return(1);
}

Matrix *matGetNextMatrix(MATFile *ph)
{
  printf("** MATLAB(UNIMPLEMENTED): matGetNextMatrix()\n");
  return(NULL);
}

int matGetFull(MATFile *ph, char *name, int *m, int *n,
	       double **pr, double **pi)
{
  char *mname;

#ifdef DEBUG
  printf("matGetFull(%s)\n", name);
#endif

  /* loop to find matrix */
  for (fseek(matGetFp(ph), 0, 0);
       (mname = NextMatrix(ph, name, m, n, pr, pi)) != NULL;) {
    if (strcmp(name, mname) == 0) {
      XvgMM_Free(mname);
#ifdef DEBUG
      printf("matGetFull(%s), size:%dx%d, done.\n", name, *m, *n);
#endif
      return(0);
    }
    else {
#ifdef DEBUG
      printf("matGetFull(%s) : name(%s) != mname(%s)\n", name, mname);
#endif
      XvgMM_Free(mname);
    }
  }
#ifdef DEBUG
      printf("matGetFull(%s), Failed.\n", name);
#endif
  return(1);
}

void matPutFull(MATFile*ph, char *name, int m, int n, double *pr, double *pi)
{
  printf("** MATLAB(UNIMPLEMENTED): matPutFull()\n");
}

void matGetString(MATFile *ph, char *name, char *string, int str_len)
{
  printf("** MATLAB(UNIMPLEMENTED): matGetString()\n");
}

int matPutString(MATFile *ph, char *name, char *string)
{
  printf("** MATLAB(UNIMPLEMENTED): matPutString()\n");
  return(-1);
}

