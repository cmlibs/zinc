#ifndef matlab_h
#define matlab_h

#include <stdio.h>

typedef double Real;

/* MATRIX STRUCTURE DEFINTION */
#define mxMAXNAM        20
typedef struct {
  char    name[mxMAXNAM];         /* name is now an array */
  int     dummy;                  /* type: 0 - matrix, 1 - string */
  int     m;                      /* row dimension */
  int     n;                      /* column dimension */
  Real    *pr;                    /* pointer to real part */
  Real    *pi;                    /* pointer to imag part */
  int     dummy6;
  int     dummy7;
  int     dummy8;
} Matrix;

#define TEXT    1                       /* mat.type indicating text */
#define MATRIX  0                       /* mat.type indicating matrix */
#define REAL    0
#define COMPLEX 1

#define matREAD		0
#define matWRITE	1
#define matAPPEND	2


typedef FILE MATFile;

typedef struct {
  long type;
  long mrows;
  long ncols;
  long imagf;
  long namlen;
} Fmatrix;


Matrix	*mxCreateFull(int m, int n, int	cmplx_flg);
void mxFreeMatrix(Matrix *pmat);
char *mxGetName(const Matrix *pm);
void mxSetName(Matrix *pm, const char *s);
int mxGetM(const Matrix *pm);
void mxSetM(Matrix *pm, int m);
int mxGetN(const Matrix *pm);
void mxSetN(Matrix *pm, int n);
Real *mxGetPr(const Matrix *pm);
void mxSetPr(Matrix *pm, Real *pr);
Real *mxGetPi(const Matrix *pm);
void mxSetPi(Matrix *pm, Real *pi);
int mxGetNzmax(const Matrix *pm);
void mxSetNzmax(Matrix *pm, int nzmax);
int *mxGetIr(const Matrix *pm);
void mxSetIr(Matrix *pm, int	*ir);
int *mxGetJc(const Matrix *pm);
void mxSetJc(Matrix *pm, int *c);
int mxGetString(const Matrix *pm, char *str_ptr, int str_len);
Matrix *mxCreateString(const char *str_ptr);
Real mxGetScalar(const Matrix *pm);
int mxIsFull(const Matrix *pm);
int mxIsSparse(const Matrix *pm);
int mxIsDouble(const Matrix *pm);
int mxIsString(const Matrix *pm);
int mxIsNumeric(const Matrix *pm);
int mxIsComplex(const Matrix *pm);
Matrix *mxCreateSparse(int m, int n, int nzmax, int cmplx_flg);
MATFile *matOpen(char *filename, char *mode);
int matClose(MATFile *ph);
FILE *matGetFp(MATFile *ph);
char *NextMatrix(MATFile *ph, char *mname, int *mp, int *np,
		 double **pr, double **pi);
char **matGetDir(MATFile *ph, int *num);
void matFreeDir(char **dir, int nentries);
int matDeleteMatrix(MATFile *ph, char *name);
Matrix *matGetMatrix(MATFile *ph, char *name);
int matPutMatrix(MATFile *ph, Matrix *mp);
Matrix *matGetNextMatrix(MATFile *ph);
int matGetFull(MATFile *ph, char *name, int *m, int *n,
	       double **pr, double **pi);
void matPutFull(MATFile*ph, char *name, int m, int n, double *pr, double *pi);
void matGetString(MATFile *ph, char *name, char *string, int str_len);
int matPutString(MATFile *ph, char *name, char *string);


#endif
