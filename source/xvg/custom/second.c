/*--------------------------------------------------------------------------------------*/
/*                                                                                      */
/*  MODULE NAME:  second.c                                                              */
/*                                                                                      */
/*  DESCRIPTION:  AIX 3.1 subroutine to read to hardware timer registers                */
/*                                                                                      */
/*  COMPILATION:  cc second.c -c                                                        */
/*                                                                                      */
/*  ARGUMENTS:    N/A                                                                   */
/*                                                                                      */
/*  COMMENTS:     Uses the assembler program 'timer.s' to access the hardware timer.    */
/*                                                                                      */
/*  COPYRIGHT IBM CORP. 1990 - all rights reserved                                      */
/*                                                                                      */
/*--------------------------------------------------------------------------------------*/
#include "../include/second.h"
double second(void)                                /* start of second                       */
{                                              /*                                       */
   double  tl, tu, tc;                         /* variables to hold the secs and nsecs  */

   do {
     tc = rtc_upper();                         /* read the register that holds the secs */
     tl = rtc_lower();                         /* read the register that holds nsecs    */
     tu = rtc_upper();                         /* read the register that holds the secs */
     } while ( tc!=tu);
   return(tu+tl/1000000000.);                  /* convert it to decimal form            */
}                                              /* and return it                         */
