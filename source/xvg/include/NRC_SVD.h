#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_

#define TOL 1.0e-5

extern void svdcmp(double **a, int m, int n, double *w, double **v,
		   double *c);
extern void svbksb(double **u, double *w, double **v, int m, int n,
		   double *b, double *x, double *c);

#endif /* _NR_UTILS_H_ */
