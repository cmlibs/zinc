/***********************************************************************
*
*  Name:          NRC_FFT.c
*
*  Author:        Paul Charette
*
*  Last Modified: 26 August 1997
*
*  Purpose:       Numerical Recipies in C FFT routines
*
*                      NRCFFT()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include "XvgGlobals.h"

extern double ***f3tensor(long nrl, long nrh, long ncl, long nch, long ndl,
			 long ndh);
extern double **matrix(long nrl, long nrh, long ncl, long nch);
extern void free_matrix(double **m, long nrl, long nrh, long ncl, long nch);
extern void free_f3tensor(double ***t, long nrl, long nrh, long ncl, long nch,
			  long ndl, long ndh);
extern void nrerror(char error_text[]);

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

static N1, N2, N3;
static double ***data, **speq;

void NRC_FFT_Init()
{
  N1 = -1;
  N2 = -1;
  N3 = -1;
  data = NULL;
  speq = NULL;
}

static void fourn(double data[], unsigned long nn[], int ndim, int isign)
{
  int idim;
  unsigned long i1,i2,i3,i2rev,i3rev,ip1,ip2,ip3,ifp1,ifp2;
  unsigned long ibit,k1,k2,n,nprev,nrem,ntot;
  double tempi,tempr;
  double theta,wi,wpi,wpr,wr,wtemp;
  
  for (ntot=1,idim=1;idim<=ndim;idim++)
    ntot *= nn[idim];
  nprev=1;
  for (idim=ndim;idim>=1;idim--) {
    n=nn[idim];
    nrem=ntot/(n*nprev);
    ip1=nprev << 1;
    ip2=ip1*n;
    ip3=ip2*nrem;
    i2rev=1;
    for (i2=1;i2<=ip2;i2+=ip1) {
      if (i2 < i2rev) {
	for (i1=i2;i1<=i2+ip1-2;i1+=2) {
	  for (i3=i1;i3<=ip3;i3+=ip2) {
	    i3rev=i2rev+i3-i2;
	    SWAP(data[i3],data[i3rev]);
	    SWAP(data[i3+1],data[i3rev+1]);
	  }
	}
      }
      ibit=ip2 >> 1;
      while (ibit >= ip1 && i2rev > ibit) {
	i2rev -= ibit;
	ibit >>= 1;
      }
      i2rev += ibit;
    }
    ifp1=ip1;
    while (ifp1 < ip2) {
      ifp2=ifp1 << 1;
      theta=isign*6.28318530717959/(ifp2/ip1);
      wtemp=sin(0.5*theta);
      wpr = -2.0*wtemp*wtemp;
      wpi=sin(theta);
      wr=1.0;
      wi=0.0;
      for (i3=1;i3<=ifp1;i3+=ip1) {
	for (i1=i3;i1<=i3+ip1-2;i1+=2) {
	  for (i2=i1;i2<=ip3;i2+=ifp2) {
	    k1=i2;
	    k2=k1+ifp1;
	    tempr=(double)wr*data[k2]-(double)wi*data[k2+1];
	    tempi=(double)wr*data[k2+1]+(double)wi*data[k2];
	    data[k2]=data[k1]-tempr;
	    data[k2+1]=data[k1+1]-tempi;
	    data[k1] += tempr;
	    data[k1+1] += tempi;
	  }
	}
	wr=(wtemp=wr)*wpr-wi*wpi+wr;
	wi=wi*wpr+wtemp*wpi+wi;
      }
      ifp1=ifp2;
    }
    nprev *= n;
  }
}

static void rlft3(double ***data, double **speq,
		  unsigned long nn1, unsigned long nn2,
		  unsigned long nn3, int isign)
{
  void fourn(double data[], unsigned long nn[], int ndim, int isign);
  void nrerror(char error_text[]);
  unsigned long i1,i2,i3,j1,j2,j3,nn[4],ii3;
  double theta,wi,wpi,wpr,wr,wtemp;
  double c1,c2,h1r,h1i,h2r,h2i;
  
  if (1+&data[nn1][nn2][nn3]-&data[1][1][1] != nn1*nn2*nn3)
    nrerror("rlft3: problem with dimensions or contiguity of data array\n");
  c1=0.5;
  c2 = -0.5*isign;
  theta=isign*(6.28318530717959/nn3);
  wtemp=sin(0.5*theta);
  wpr = -2.0*wtemp*wtemp;
  wpi=sin(theta);
  nn[1]=nn1;
  nn[2]=nn2;
  nn[3]=nn3 >> 1;
  if (isign == 1) {
    fourn(&data[1][1][1]-1,nn,3,isign);
    for (i1=1;i1<=nn1;i1++)
      for (i2=1,j2=0;i2<=nn2;i2++) {
	speq[i1][++j2]=data[i1][i2][1];
	speq[i1][++j2]=data[i1][i2][2];
      }
  }
  for (i1=1;i1<=nn1;i1++) {
    j1=(i1 != 1 ? nn1-i1+2 : 1);
    wr=1.0;
    wi=0.0;
    for (ii3=1,i3=1;i3<=(nn3>>2)+1;i3++,ii3+=2) {
      for (i2=1;i2<=nn2;i2++) {
	if (i3 == 1) {
	  j2=(i2 != 1 ? ((nn2-i2)<<1)+3 : 1);
	  h1r=c1*(data[i1][i2][1]+speq[j1][j2]);
	  h1i=c1*(data[i1][i2][2]-speq[j1][j2+1]);
	  h2i=c2*(data[i1][i2][1]-speq[j1][j2]);
	  h2r= -c2*(data[i1][i2][2]+speq[j1][j2+1]);
	  data[i1][i2][1]=h1r+h2r;
	  data[i1][i2][2]=h1i+h2i;
	  speq[j1][j2]=h1r-h2r;
	  speq[j1][j2+1]=h2i-h1i;
	} else {
	  j2=(i2 != 1 ? nn2-i2+2 : 1);
	  j3=nn3+3-(i3<<1);
	  h1r=c1*(data[i1][i2][ii3]+data[j1][j2][j3]);
	  h1i=c1*(data[i1][i2][ii3+1]-data[j1][j2][j3+1]);
	  h2i=c2*(data[i1][i2][ii3]-data[j1][j2][j3]);
	  h2r= -c2*(data[i1][i2][ii3+1]+data[j1][j2][j3+1]);
	  data[i1][i2][ii3]=h1r+wr*h2r-wi*h2i;
	  data[i1][i2][ii3+1]=h1i+wr*h2i+wi*h2r;
	  data[j1][j2][j3]=h1r-wr*h2r+wi*h2i;
	  data[j1][j2][j3+1]= -h1i+wr*h2i+wi*h2r;
	}
      }
      wr=(wtemp=wr)*wpr-wi*wpi+wr;
      wi=wi*wpr+wtemp*wpi+wi;
    }
  }
  if (isign == -1)
    fourn(&data[1][1][1]-1,nn,3,isign);
}

/*
isign =  1 => FFT
isign = -1 => inverse FFT
*/
NRC_FFT(double *RealPixels, double *ImPixels,
	int height, int width, int isign)
{
  int i, j, k, depth, offset;

  /* 2D transform */
  depth = 1;

  /* check if memory must be allocated */
  if ((N1 != depth) || (N2 != height) || (N3 != width)) {
    /* free previously allocated memory, if required */
    if ((N1 != -1) && (N2 != -1) && (N3 != -1)) {
      free_matrix(speq, 1, N1, 1, 2*N2);
      free_f3tensor(data, 1, N1, 1, N2, 1, N3);
    }
    N1 = depth;
    N2 = height;
    N3 = width;

    /* allocate memory */
    data = f3tensor(1, N1, 1, N2, 1, N3);
    speq = matrix(1, N1, 1, 2*N2);
  }

  /* load the data array */
  for (j = 1, k = 0; j <= N2; j++) {
    for (i = 1; i <= N3; i++, k++) {
      data[1][j][i] = RealPixels[k];
    }
  }

  /* calculate the FFT */
  rlft3(data, speq, 1, N2, N3, isign);

  /* transfer the data to the output buffer */
  for (j = 1; j <= N2/2; j++) {
    offset = (N3/2 + j - 1)*width + width/2;
    for (i = 1, k = 0; i <= N3; i+=2, k++) {
      RealPixels[offset + k] = data[1][j][i+0];
      ImPixels[offset + k]   = data[1][j][i+1];
    }
  }
  for (; j <= N2; j++) {
    offset = (j - N3/2 - 1)*width + width/2;
    for (i = 1, k = 0; i <= N3; i+=2, k++) {
      RealPixels[offset + k] = data[1][j][i+0];
      ImPixels[offset + k]   = data[1][j][i+1];
    }
  }
}



