/* xlc -c -O -D_POSIX_SOURCE sample.c */
#include <math.h>
#include <stdio.h>
#include <sys/lock.h>                   /* to pin program in memory			*/
#include <sys/m_intr.h>			/* includes interrupts levels			*/
#include <sys/errno.h>			/* AIX error codes				*/
#if !defined (CHSTKSIZ)
#include <ulimit.h>
#include <sys/resource.h>
#endif

#include "UNIMA/uim.h"
#include "UNIMA/unimax.h"
#include "UNIMA/un05.h"
#include "UNIMA/uadapterboard.h"



#define CHANNELS 32
#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)
#define GETSWAP16(target,value) {\
        unsigned int z;                  \
        z = (volatile unsigned int) value;       \
        target = ((short int)( ((z>>8)&0x0000ff00) | (z>>24) )); \
        }

extern unsigned int volatile *UnimaBusPort;
extern unsigned int UnimaFile;

int errno;
Sample(short int *buffer,int card, int N,double Ts)
{
   short int *bp;
   short int dummy;
   int i,j,k,n,pri,x,m=0xffff;
   int user_enable(int pri), user_disable(int pri);
   double t, t0, z, second();
   FILE *fp;
#if !defined (CHSTKSIZ)
	struct rlimit resource_limits;
#endif

#if defined (CHSTKSIZ)
   if (chstksiz(512*1024) < 0)                /* shrink stack to pinnable size*/
        perror("main:chstksiz");             /*                              */

   /* lock program in memory for the real-time work */
   if ( plock(PROCLOCK)==-1 ) {
     perror("main:plock");
     return(-1);
     }
#else
	/* shrink the stack and lock the process code, data and stack */
	resource_limits.rlim_max=resource_limits.rlim_cur=512*1024;
	if ((setrlimit(RLIMIT_STACK,&resource_limits)==0)&&(plock(PROCLOCK)==0))
	{
#endif

   Unima(1,0);  

   usleep(2000000);

#ifndef AUTO_START_AD
   UUserConfigRegister[0]=UUserConfigRegister[1];
   USetControl(0,A,1); /* Throw away first conversion  */
   USetControl(0,A,0);
   usleep(1000);
   USetControl(0,A,1); /* Throw away first conversion  */
   USetControl(0,A,0);
   usleep(1000);
#else
   for(k=1;k<=16;k++)
     i = (int)U05ReadData(k); /* Throw away first conversion  */
   usleep(1000);
   for(k=1;k<=16;k++)
     i = (int)U05ReadData(k); /* Throw away second conversion  */
   usleep(1000);
#endif

   usleep(2000000);

   pri = disable_interrupts(INTCLASS0);

   for(t=second()+Ts; second()<t;) {};

   bp = buffer;
   t0 = second();
   for( i=0; i<N; i++) {
    t = t0;
    for(j=0;j<CHANNELS;j++) {
/*    for(j=0;j<CHANNELS/2;j++) {*/
#ifndef AUTO_START_AD
     USetControl(0,A,1);
     USetControl(0,A,0); 
     for(t=second()+20.e-6; second()<t;) {};
/*     USetControl(0,A,1);
     USetControl(0,A,0); 
     for(t=second()+20.e-6; second()<t;) {};*/
#endif
     for(k=1;k<=16;k++) {
       GETSWAP16(dummy,*(UnimaBusPort+(8*k)+UN05_CHANNEL0));
       if(k==card+1) *(bp++)=dummy;
       }
/*     for(k=1;k<=16;k++) {
       GETSWAP16(dummy,*(UnimaBusPort+(8*k)+UN05_CHANNEL0));
       if(k==card+1) *(bp++)=dummy;
       }*/
     }
     for(t0=t0+Ts; t0>second();) {};
   }

   pri = enable_interrupts(pri);
   i = plock(UNLOCK);
   if (i == -1)
     printf("Error in unpinning pages\n");

   printf("%d\n",*(bp-1));
 
   U00SysReset();
   UAReset();
   UASetAdapter(DISABLE);

#if !defined (CHSTKSIZ)
	}
	else
	{
		printf("could not lock process in memory\n");
	}
#endif
}
#define NUMBER_OF_SAMPLES 4000

main()
{
	FILE *output;
	int i,j;
	short int *bp,buffer[32*NUMBER_OF_SAMPLES];

	Sample(buffer,0,NUMBER_OF_SAMPLES,0.001);
	if (output=fopen("multicast.dat","w"))
	{
		bp=buffer+18;
		for (i=0;i<NUMBER_OF_SAMPLES;i++)
		{
			fprintf(output,"%d\n",*bp);
			bp += 32;
		}
		fclose(output);
	}
}
