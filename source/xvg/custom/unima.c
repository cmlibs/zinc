/***********************************************************************
*
*  Name:          unima.c
*
*  Author:        Paul Charette
*
*  Last Modified: 3 March 1997
*
*  Purpose:       UNIMA AND LCR Utility routines.
*
*                      UnimaInit()
*                      UnimaOff()
*                      UnimaPSGalvosOut()
*                      UnimaSample()
*                      UnimaVOut()
*                      LCRTempSample()
*                      LCRValOut()
*                      LightSourceSelect()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <memory.h>
#include <search.h>
#include <values.h>
#include <termio.h>
#include <malloc.h>
#include "XvgGlobals.h"

/************************** UNIMA stuff ************************************/
/*                                                                         */
/* Unima interface code.                                                   */
/*                                                                         */
/* Code taken from /u/paul/unima/udac/udac.c                               */
/*             and /u/paul/unima/emap/Scope/sample.c                       */
/*                                                                         */
/* Note : uinterface.h must be modified to download the appropriate        */
/*        scan lists.                                                      */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
#ifdef UNIMA_CODE

/* LCR stuff */
#define D1040_DIS1 0x010000
#define D1040_DIS2 0x020000
#define D1040_TSEL 0x040000
#define D1040_GSEL 0x080000
#define D1040_OLUP 0x100000
#define D1040_OLDN 0x200000
#define D1040_OLAL 0x300000

/* UNIMA stuff */
#define USYSTEM_CONFIG1 1
#define WITHOUT_MENU 0
#define WITH_MENU    1
#define MAXDAOUT 524287
#define MINDAOUT -524288

#define STARTCHANNEL 0
#define LASTCHANNEL 15
#define NKLUDGE     14
#define DISABLE     0

#define NADCHANNELS 32
#define GETSWAP16(target,value) {\
        unsigned int z;                  \
        z = (volatile unsigned int) value;       \
        target = ((short int)( ((z>>8)&0x0000ff00) | (z>>24) )); \
        }

/* NOTE : DAC dynamic range = +3V -> -3V. Step approx = 5.7 microvolts. */
/* NOTE : ADC dynamic range = +4.5V <-> -4.5V */

#include "/u/paul/unima/udac/unima_syscall.c"

/* Initialize UNIMA */
void UnimaInit(void)
{
  int i;

  /* check that already done */
  if (UnimaInitialized)
    return(0);
  UnimaInitialized = B_TRUE;

  /* init the hardware */
  Unima(USYSTEM_CONFIG1, WITHOUT_MENU);
  sleep_us(1500000);

  /* scrap first two AD conversions */
  U05ReadData(1);
  sleep_us(1000);
  U05ReadData(1);
  sleep_us(1000);

  /* zero out DA values array */
  for (i = STARTCHANNEL, DA0CurrentVal = 0, DA4CurrentVal = 0;
       i <= LASTCHANNEL; i++) {
    DAValuesOut[i] = 0;
    DACurrentVals[i] = 0;
    U05WriteData(1, 0);
    U05SendData(1);
  }

  /* load DA output extrema arrays */
  DAMax[0] = DA00MAX; DAMin[0] = DA00MIN;
  DAMax[1] = DA01MAX; DAMin[1] = DA01MIN;
  DAMax[2] = DA02MAX; DAMin[2] = DA02MIN;
  DAMax[3] = DA03MAX; DAMin[3] = DA03MIN;
  DAMax[4] = DA04MAX; DAMin[4] = DA04MIN;
  DAMax[5] = DA05MAX; DAMin[5] = DA05MIN;
  DAMax[6] = DA06MAX; DAMin[6] = DA06MIN;
  DAMax[7] = DA07MAX; DAMin[7] = DA07MIN;
  DAMax[8] = DA08MAX; DAMin[8] = DA08MIN;
  DAMax[9] = DA09MAX; DAMin[9] = DA09MIN;
  DAMax[10] = DA10MAX; DAMin[10] = DA10MIN;
  DAMax[11] = DA11MAX; DAMin[11] = DA11MIN;
  DAMax[12] = DA12MAX; DAMin[12] = DA12MIN;
  DAMax[13] = DA13MAX; DAMin[13] = DA13MIN;
  DAMax[14] = DA14MAX; DAMin[14] = DA14MIN;
  DAMax[15] = DA15MAX; DAMin[15] = DA15MIN;

  /* load "uV to DA values" scale fator arrays */
  for (i = 0; i < 16; i++) {
    DASlopes[i] = 1048575.0 / (DAMax[i] - DAMin[i]);
    DAOffsets[i] = 524287.0 - DASlopes[i]*DAMax[i];
  }
  
  /* kludge to reset DA scannet to 0 */
  for (i = STARTCHANNEL; i <= NKLUDGE; i++) {
    U05WriteData(1, 0);
    U05SendData(1);
  }

  /* initialize parallel ports */
  USetPort(1, 0, PORT_LATCHED_OUTPUT);
  USetPort(1, 1, PORT_LATCHED_OUTPUT);
  USetPort(1, 2, PORT_LATCHED_OUTPUT);
  UWritePort(1, 0x7fff | D1040_OLAL);
}

/* Read value from AD */
int UnimaSample(void)
{
  extern volatile unsigned int *port;
  double t;
  short dummy;

  for (t = second()+0.000015; t>second(););
  GETSWAP16(dummy,*(port+8));
  return((int) -dummy);
}

/* Output to 16 DA channels (uV inputs) */
extern Widget DAValuesTBG;
void UnimaVOut(void)
{
  int i, j, val;

  /* kludge to replace channel 13 with channel 5, because of spike noise */
  DAValuesOut[5] = DAValuesOut[13];

  for (i = STARTCHANNEL; i <= LASTCHANNEL; i++) {
    val = DAValuesOut[i]*DASlopes[i] + DAOffsets[i];
    DACurrentVals[i] = val;
    if (i == 0)
      DA0CurrentVal = val;
    else if (i == 4)
      DA4CurrentVal = val;
    U05WriteData(1, val);
    U05SendData(1);
  }
}

/* Output to channel 0 & 4 only of the 16 DA channels (digital step inputs) */
void UnimaPSGalvosOut(int NewVal)
{
  int i, j, val;

  for (i = STARTCHANNEL; i <= LASTCHANNEL; i++) {
    if (i == 0) {
      val = DA0CurrentVal = NewVal;
      DAValuesOut[0] = (val - DAOffsets[0])/DASlopes[0];
    }
    else if (i == 4) {
      val = DA4CurrentVal = -NewVal;
      DAValuesOut[4] = (val - DAOffsets[4])/DASlopes[4];
    }
    else
      val = DAValuesOut[i]*DASlopes[i] + DAOffsets[i];
    DACurrentVals[i] = val;
    U05WriteData(1, val);
    U05SendData(1);
  }
}

/* disable UNIMA hardware */
void UnimaOff(void)
{
  int i;

  /* null DA outputs */
  for (i = STARTCHANNEL; i <= LASTCHANNEL; i++) {
    U05WriteData(1, 0);
    U05SendData(1);
  }
  U00SysReset();                      /* Reset the UNIMA system   */
  UAReset();                          /* Reset the UNIMA Adapter  */
  UASetAdapter(DISABLE);
}


/************************** LCR control stuff ******************************/
/*                                                                         */
/*  The parallel ports on interface module #1 are used as follows          */
/*  for controlling the D1040 LCR controller :                             */
/*      - PORT 0 : D1040 data bits 0-7                                     */
/*      - PORT 1 : D1040 data bits 8-15                                    */
/*      - PORT 2 : Control bits for D1040                                  */
/*                   0  DIS1 (clock disable 1)                             */
/*                   1  DIS2 (clock disable 2)                             */
/*                   2  Temp Select (active high)                          */
/*                   3  Gain Select (active high)                          */
/*                   4  Output Latch (upper byte) (active high)            */
/*                   5  Output Latch (lower byte) (active high)            */
/*                                                                         */
/***************************************************************************/
void LCRValOut(int v)
{
  double t;
  int cob;

  /* set ports 0,1,2 to OUTPUT */
  USetPort(1, 0, PORT_LATCHED_OUTPUT);
  USetPort(1, 1, PORT_LATCHED_OUTPUT);
  USetPort(1, 2, PORT_LATCHED_OUTPUT);

  /* pulse latch select control bits and send voltage command */
  cob = (((v * 0x8000)/20000) + 0x7fff) & 0xffff;
  UWritePort(1, cob | D1040_OLAL);
  for (t = second()+0.000050; t>second(););
  UWritePort(1, cob | 0); 
}

double LCRTempSample(void)
{
  double t, v;
  int i;

  /* set ports 0,1 to INPUT and 2 to OUTPUT (required because of UNIMA bug) */
  USetPort(1, 0, PORT_DIRECT_INPUT);
  USetPort(1, 1, PORT_DIRECT_INPUT);
  USetPort(1, 2, PORT_LATCHED_OUTPUT);
  
  /* read temperature data and average (30 times recommended by MeadowLark) */
  for (i = 0, v = 0; i < 30; i++) {
    /* set temperature select control bits and wait a bit */
    UWritePort(1, D1040_TSEL);
    for (t = second()+0.000050; t>second(););
    /* read the temperature data */
    v += (UReadPort(1) & 0x03ff);
    sleep_us(1000);
    /* reset D1040 to idle state */
    UWritePort(1, 0);
  }

  /* return value */
  return(v/300.0);
}

#else
void UnimaInit(void) {}
int UnimaSample(void){return(-1);}
void UnimaVOut(void){}
void UnimaOff(void){}
void UnimaPSGalvosOut(int NewVal){}
double LCRTempSample(void){return(-1);}
void LCRValOut(int v){}
#endif

