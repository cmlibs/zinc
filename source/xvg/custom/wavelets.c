/*
WARNING: ARRAY LENGHTS MUST BE POWERS OF 2 ONLY!!!!!!
*/

#define C0 0.4829629131445341
#define C1 0.8365163037378079
#define C2 0.2241438680420134
#define C3 -0.1294095225512604

void DAUB4(double *a, double *wksp, unsigned long n, int isign)
{
  unsigned long nh1, i, j;

  /* check input parms */
  if (n < 4)
    return;
  
  /* init parameters */
  nh1 = (n >> 1) + 1;

  /* forward wavelet transform */
  if (isign >= 0) {
    for (i=1, j=1; j <= n-3; j+=2,i++) {
      wksp[i]   = C0*a[j] + C1*a[j+1] + C2*a[j+2] + C3*a[j+3];
      wksp[i+n] = C3*a[j] - C2*a[j+1] + C1*a[j+2] - C0*a[j+3];
    }
    wksp[i]   = C0*a[n-1] + C1*a[n] + C2*a[1] + C3*a[2];
    wksp[i+n] = C3*a[n-1] - C2*a[n] + C1*a[1] - C0*a[2];
  }

  /* inverse wavelet transform */
  else {
    wksp[1] = C2*a[n] + C1*a[n] + C0*a[1] + C3*a[nh1];
    wksp[2] = C3*a[n] - C0*a[n] + C1*a[1] - C2*a[nh1];
    for (i=1, j=3; i < n; i++) {
      wksp[j++] = C2*a[i] + C1*a[i+n] + C0*a[i+1] + C3*a[i+nh1];
      wksp[j++] = C3*a[i] - C0*a[i+n] + C1*a[i+1] - C2*a[i+nh1];
    }
  }

  /* load results into output array */
  for (i=1; i <= n; i++)
    a[i] = wksp[i];
}

void WaveletTransform(double *inpixels, double *outpixels, int height, int width,
		      void *ParmPtrs[])
{
  double *buf, *wksp;
  int isign;
  unsigned long i, j, k, n, nmax;
  
  /* allocate storage */
  nmax = (width > height ? width : height);
  if ((buf = (double *) XvgMM_Alloc((void *) buf, nmax*sizeof(double))) == NULL) {
    ErrorMsg("Could not allocate storage in wt1()");
    return;
  }
  if ((wksp = (double *) XvgMM_Alloc((void *) wksp, nmax*sizeof(double))) == NULL) {
    ErrorMsg("Could not allocate storage in wt1()");
    return;
  }
  
  /* transform along the X axis */
  if (width > 4) {
    for (j = 0; j < height; j++) {
      /* load input image line */
      for (i = j*width, k=1; k <= width; k++, i++)
	wksp[k] = inpixels[i];
      /* forward transform */
      if (isign >= 0) {
	for(n = width; n >= 4; n /= 2)
	  DAUB4(wksp, buf, n, isign);
      }
      /* inverse transform */
      else {
	for(n = 4; n <= width; n /= 2)
	  DAUB4(wksp, buf, n, isign);
      }
      /* copy transformed line */
      for (i = j*width, k=1; k <= width; k++, i++)
	outpixels[i] = wksp[k];
    }
  }

  /* transform along the Y axis */
  if (height > 4) {
    for (j = 0; j < width; j++) {
      /* load input image line */
      for (i = j, k=1; k <= height; k++, i+=width)
	wksp[k] = inpixels[i];
      /* forward transform */
      if (isign >= 0) {
	for(n = height; n >= 4; n /= 2)
	  DAUB4(wksp, buf, n, isign);
      }
      /* inverse transform */
      else {
	for(n = 4; n <= height; n /= 2)
	  DAUB4(wksp, buf, n, isign);
      }
      /* copy transformed line */
      for (i = j, k=1; k <= height; k++, i+=width)
	outpixels[i] = wksp[k];
    }
  }

  /* free storage */
  XvgMM_Free(buf);
  XvgMM_Free(wksp);
}


